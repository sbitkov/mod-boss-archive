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
#include <unordered_map>
#include <unordered_set>
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

    CreateCopyOutcome CreateArchiveCopy(
    Player* player,
    uint32 itemEntry,
    int32 randomPropertyId)
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
        
        int32 effectiveRandomPropertyId = randomPropertyId;

        std::vector<ArchiveVariant> variants =
            GetArchiveVariants(player, itemEntry);

        if (randomPropertyId != 0)
        {
            bool variantUnlocked = false;

            for (ArchiveVariant const& variant : variants)
            {
                if (variant.randomPropertyId == randomPropertyId)
                {
                    variantUnlocked = true;
                    break;
                }
            }

            if (!variantUnlocked)
            {
                outcome.result = CreateCopyResult::VARIANT_NOT_UNLOCKED;
                return outcome;
            }
        }
        else
        {
            if (variants.size() == 1)
            {
                effectiveRandomPropertyId =
                    variants.front().randomPropertyId;
            }
            else if (variants.size() > 1)
            {
                outcome.result =
                    CreateCopyResult::VARIANT_SELECTION_REQUIRED;

                return outcome;
            }
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
            true,
            effectiveRandomPropertyId
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
        std::string const& search,
        ArchiveSourceFilter source)
    {
        ArchiveQueryResult result;
        if (!player)
            return result;

        uint32 guid = player->GetGUID().GetCounter();

        // Filter by provenance before collecting items; one item can have
        // several sources, but appears only once in the result.
        QueryResult itemResult;
        if (source == ArchiveSourceFilter::ALL)
        {
            itemResult = CharacterDatabase.Query(
                "SELECT item_entry, source_type, source_entry "
                "FROM character_archive_item WHERE guid = {}", guid
            );
        }
        else
        {
            itemResult = CharacterDatabase.Query(
                "SELECT item_entry, source_type, source_entry "
                "FROM character_archive_item WHERE guid = {} AND source_type = {}",
                guid, static_cast<uint8>(source)
            );
        }
        if (!itemResult)
            return result;

        struct Unlock
        {
            uint32 itemEntry;
            uint8 sourceType;
            uint32 sourceEntry;
        };

        std::vector<Unlock> unlocks;
        std::unordered_set<uint32> bossEntries;
        std::string loweredSearch = ToLower(search);

        do
        {
            Field* fields = itemResult->Fetch();
            Unlock unlock{
                fields[0].Get<uint32>(),
                fields[1].Get<uint8>(),
                fields[2].Get<uint32>()
            };
            unlocks.push_back(unlock);
            if (!loweredSearch.empty() &&
                unlock.sourceType == static_cast<uint8>(ArchiveSourceType::BOSS))
            {
                bossEntries.insert(unlock.sourceEntry);
            }
        }
        while (itemResult->NextRow());

        // Character and world data live in different DB connections.
        // Resolve boss names in batches instead of one DB query per item.
        std::unordered_map<uint32, bool> matchingBosses;
        if (!bossEntries.empty())
        {
            std::string idList;
            uint32 batchCount = 0;
            auto loadBatch = [&]()
            {
                if (idList.empty())
                    return;

                QueryResult bosses = WorldDatabase.Query(
                    "SELECT entry, name FROM creature_template WHERE entry IN ({})",
                    idList
                );
                if (bosses)
                {
                    do
                    {
                        Field* row = bosses->Fetch();
                        uint32 entry = row[0].Get<uint32>();
                        std::string name = ToLower(row[1].Get<std::string>());
                        matchingBosses[entry] =
                            name.find(loweredSearch) != std::string::npos;
                    }
                    while (bosses->NextRow());
                }
                idList.clear();
                batchCount = 0;
            };

            for (uint32 entry : bossEntries)
            {
                if (!idList.empty())
                    idList += ",";
                idList += std::to_string(entry);
                if (++batchCount >= 100)
                    loadBatch();
            }
            loadBatch();
        }

        struct Candidate
        {
            uint32 itemEntry;
            uint32 quality;
            std::string sortName;
        };

        std::vector<Candidate> candidates;
        std::unordered_set<uint32> included;
        for (Unlock const& unlock : unlocks)
        {
            if (included.find(unlock.itemEntry) != included.end())
                continue;

            ItemTemplate const* itemTemplate =
                sObjectMgr->GetItemTemplate(unlock.itemEntry);
            if (!itemTemplate || !MatchesRarity(itemTemplate, rarity))
                continue;

            std::string loweredName = ToLower(itemTemplate->Name1);
            bool matches = loweredSearch.empty() ||
                loweredName.find(loweredSearch) != std::string::npos;

            if (!matches &&
                unlock.sourceType == static_cast<uint8>(ArchiveSourceType::BOSS))
            {
                auto found = matchingBosses.find(unlock.sourceEntry);
                matches = (found != matchingBosses.end() && found->second) ||
                    std::to_string(unlock.sourceEntry) == loweredSearch;
            }
            if (!matches)
                continue;

            included.insert(unlock.itemEntry);
            candidates.push_back(
                {unlock.itemEntry, itemTemplate->Quality, std::move(loweredName)}
            );
        }

        std::sort(candidates.begin(), candidates.end(),
            [](Candidate const& left, Candidate const& right)
            {
                if (left.sortName != right.sortName)
                    return left.sortName < right.sortName;
                return left.itemEntry < right.itemEntry;
            }
        );

        result.totalCount = static_cast<uint32>(candidates.size());
        if (!result.totalCount)
            return result;

        result.totalPages =
            (result.totalCount + ARCHIVE_PAGE_SIZE - 1) / ARCHIVE_PAGE_SIZE;
        if (page >= result.totalPages)
            page = result.totalPages - 1;
        result.page = page;

        uint32 beginIndex = page * ARCHIVE_PAGE_SIZE;
        uint32 endIndex = std::min(
            beginIndex + ARCHIVE_PAGE_SIZE, result.totalCount
        );
        result.items.reserve(endIndex - beginIndex);

        for (uint32 index = beginIndex; index < endIndex; ++index)
        {
            Candidate const& candidate = candidates[index];
            result.items.push_back({candidate.itemEntry, candidate.quality});
        }
        return result;
    }

    bool UnlockVariant(Player* player, Item* item)
    {
        if (!player || !item)
            return false;

        uint32 itemEntry = item->GetEntry();

        if (!IsArchiveEligible(itemEntry))
            return false;

        int32 randomPropertyId = item->GetItemRandomPropertyId();

        // Non-affix items do not need a secondary variant row.
        if (randomPropertyId == 0)
            return true;

        uint32 suffixFactor = item->GetItemSuffixFactor();
        uint32 guid = player->GetGUID().GetCounter();

        CharacterDatabase.Execute(
            "INSERT INTO character_archive_variant "
            "(guid, item_entry, random_property_id, suffix_factor) "
            "VALUES ({}, {}, {}, {}) "
            "ON DUPLICATE KEY UPDATE "
            "suffix_factor = VALUES(suffix_factor)",
            guid,
            itemEntry,
            randomPropertyId,
            suffixFactor
        );

        return true;
    }
    
    std::vector<ArchiveVariant> GetArchiveVariants(
        Player* player,
        uint32 itemEntry)
        {
            std::vector<ArchiveVariant> variants;

            if (!player)
                return variants;

            uint32 guid = player->GetGUID().GetCounter();

            QueryResult result = CharacterDatabase.Query(
                "SELECT random_property_id, suffix_factor "
                "FROM character_archive_variant "
                "WHERE guid = {} "
                "AND item_entry = {} "
                "ORDER BY random_property_id",
                guid,
                itemEntry
            );

            if (!result)
                return variants;

            do
            {
                Field* fields = result->Fetch();

                ArchiveVariant variant;
                variant.randomPropertyId = fields[0].Get<int32>();
                variant.suffixFactor = fields[1].Get<uint32>();

                variants.push_back(variant);
            }
            while (result->NextRow());

            return variants;
        }
}
