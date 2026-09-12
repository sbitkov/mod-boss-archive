#include "ArchiveService.h"
#include "ArchiveEligibility.h"

#include "DatabaseEnv.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace BossArchive
{
    bool UnlockItem(
        Player* player,
        uint32 itemEntry,
        ArchiveSourceType sourceType,
        uint32 sourceEntry,
        uint32 sourceContext)
    {
        if (!player)
            return false;

        if (!IsArchiveEligible(itemEntry))
            return false;

        uint32 guid = player->GetGUID().GetCounter();

        CharacterDatabase.Execute(
            "INSERT IGNORE INTO character_archive_item "
            "(guid, item_entry, source_type, source_entry, source_context) "
            "VALUES ({}, {}, {}, {}, {})",
            guid,
            itemEntry,
            static_cast<uint8>(sourceType),
            sourceEntry,
            sourceContext
        );

        return true;
    }

    CreateCopyOutcome CreateArchiveCopy(Player* player, uint32 itemEntry)
    {
        CreateCopyOutcome outcome;

        if (!player)
        {
            outcome.result = CreateCopyResult::INVALID_PLAYER;
            return outcome;
        }

        uint32 guid = player->GetGUID().GetCounter();

        QueryResult unlockResult = CharacterDatabase.Query(
            "SELECT 1 "
            "FROM character_archive_item "
            "WHERE guid = {} "
            "AND item_entry = {} "
            "LIMIT 1",
            guid,
            itemEntry
        );

        if (!unlockResult)
        {
            outcome.result = CreateCopyResult::NOT_UNLOCKED;
            return outcome;
        }

        ItemTemplate const* itemTemplate =
            sObjectMgr->GetItemTemplate(itemEntry);

        if (!itemTemplate)
        {
            outcome.result = CreateCopyResult::ITEM_TEMPLATE_NOT_FOUND;
            return outcome;
        }


        ItemPosCountVec dest;

        InventoryResult msg = player->CanStoreNewItem(
            NULL_BAG,
            NULL_SLOT,
            dest,
            itemEntry,
            1
        );

        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(
                msg,
                nullptr,
                nullptr,
                itemEntry
            );

            outcome.result = CreateCopyResult::CANNOT_STORE;
            return outcome;
        }

        Item* item = player->StoreNewItem(
            dest,
            itemEntry,
            true
        );

        if (!item)
        {
            outcome.result = CreateCopyResult::CREATE_FAILED;
            return outcome;
        }

        item->SetBinding(true);

        uint32 itemGuid = item->GetGUID().GetCounter();

        CharacterDatabase.Execute(
            "INSERT INTO character_boss_archive_copy "
            "(guid, item_entry, item_guid) "
            "VALUES ({}, {}, {})",
            guid,
            itemEntry,
            itemGuid
        );

        player->SendNewItem(
            item,
            1,
            true,
            false
        );

        outcome.result = CreateCopyResult::SUCCESS;
        outcome.itemGuid = itemGuid;

        return outcome;
    }
    namespace
    {
        std::string ToLower(std::string value)
        {
            std::transform(
                value.begin(),
                value.end(),
                value.begin(),
                [](unsigned char c)
                {
                    return static_cast<char>(std::tolower(c));
                }
            );

            return value;
        }

        bool MatchesRarity(
            ItemTemplate const* itemTemplate,
            ArchiveRarityFilter rarity)
        {
            if (!itemTemplate)
                return false;

            switch (rarity)
            {
                case ArchiveRarityFilter::ALL:
                    return true;

                case ArchiveRarityFilter::UNCOMMON:
                    return itemTemplate->Quality == ITEM_QUALITY_UNCOMMON;

                case ArchiveRarityFilter::RARE:
                    return itemTemplate->Quality == ITEM_QUALITY_RARE;

                case ArchiveRarityFilter::EPIC:
                    return itemTemplate->Quality == ITEM_QUALITY_EPIC;

                case ArchiveRarityFilter::LEGENDARY:
                    return itemTemplate->Quality == ITEM_QUALITY_LEGENDARY;

                case ArchiveRarityFilter::ARTIFACT:
                    return itemTemplate->Quality == ITEM_QUALITY_ARTIFACT;

                case ArchiveRarityFilter::HEIRLOOM:
                    return itemTemplate->Quality == ITEM_QUALITY_HEIRLOOM;
            }

            return false;
        }
    }

    ArchiveQueryResult QueryArchiveItems(
        Player* player,
        uint32 page,
        ArchiveRarityFilter rarity,
        std::string const& search)
    {
        ArchiveQueryResult result;

        if (!player)
            return result;

        uint32 guid = player->GetGUID().GetCounter();

        QueryResult itemResult = CharacterDatabase.Query(
            "SELECT DISTINCT item_entry "
            "FROM character_archive_item "
            "WHERE guid = {}",
            guid
        );

        if (!itemResult)
            return result;

        struct Candidate
        {
            uint32 itemEntry;
            uint32 quality;
            std::string sortName;
        };

        std::vector<Candidate> candidates;
        std::string loweredSearch = ToLower(search);

        do
        {
            Field* fields = itemResult->Fetch();
            uint32 itemEntry = fields[0].Get<uint32>();

            ItemTemplate const* itemTemplate =
                sObjectMgr->GetItemTemplate(itemEntry);

            // An existing archive unlock is trusted as already eligible.
            // A missing ItemTemplate cannot be displayed, so skip it.
            if (!itemTemplate)
                continue;

            if (!MatchesRarity(itemTemplate, rarity))
                continue;

            std::string loweredName =
                ToLower(itemTemplate->Name1);

            if (!loweredSearch.empty() &&
                loweredName.find(loweredSearch) == std::string::npos)
            {
                continue;
            }

            candidates.push_back(
                {
                    itemEntry,
                    itemTemplate->Quality,
                    std::move(loweredName)
                }
            );
        }
        while (itemResult->NextRow());

        std::sort(
            candidates.begin(),
            candidates.end(),
            [](Candidate const& left, Candidate const& right)
            {
                if (left.sortName != right.sortName)
                    return left.sortName < right.sortName;

                return left.itemEntry < right.itemEntry;
            }
        );

        result.totalCount =
            static_cast<uint32>(candidates.size());

        if (result.totalCount == 0)
            return result;

        result.totalPages =
            (result.totalCount + ARCHIVE_PAGE_SIZE - 1) /
            ARCHIVE_PAGE_SIZE;

        if (page >= result.totalPages)
            page = result.totalPages - 1;

        result.page = page;

        uint32 beginIndex =
            page * ARCHIVE_PAGE_SIZE;

        uint32 endIndex = std::min(
            beginIndex + ARCHIVE_PAGE_SIZE,
            result.totalCount
        );

        result.items.reserve(endIndex - beginIndex);

        for (uint32 index = beginIndex; index < endIndex; ++index)
        {
            Candidate const& candidate = candidates[index];

            result.items.push_back(
                {
                    candidate.itemEntry,
                    candidate.quality
                }
            );
        }

        return result;
    }
}
