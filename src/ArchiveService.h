#pragma once

#include "Define.h"

#include <string>
#include <vector>

class Player;
class Item;

namespace BossArchive
{
    constexpr uint32 ARCHIVE_PAGE_SIZE = 10;

    enum class ArchiveSourceType : uint8
    {
        BOSS = 1,
        QUEST = 2,
        CRAFT = 3,
        VENDOR = 4,
        WORLD = 5
    };

    bool UnlockItem(
        Player* player,
        uint32 itemEntry,
        ArchiveSourceType sourceType,
        uint32 sourceEntry,
        uint32 sourceContext = 0
    );

    bool UnlockVariant(Player* player, Item* item);

    enum class CreateCopyResult : uint8
    {
        SUCCESS = 0,
        INVALID_PLAYER,
        NOT_UNLOCKED,
        ITEM_TEMPLATE_NOT_FOUND,
        CANNOT_STORE,
        CREATE_FAILED,
        VARIANT_NOT_UNLOCKED,
        VARIANT_SELECTION_REQUIRED
    };

    struct CreateCopyOutcome
    {
        CreateCopyResult result = CreateCopyResult::CREATE_FAILED;
        uint32 itemGuid = 0;
    };

    enum class ArchiveRarityFilter : uint8
    {
        ALL = 0,
        UNCOMMON,
        RARE,
        EPIC,
        LEGENDARY,
        ARTIFACT,
        HEIRLOOM
    };

    struct ArchiveQueryItem
    {
        uint32 itemEntry = 0;
        uint32 quality = 0;
    };

    struct ArchiveQueryResult
    {
        std::vector<ArchiveQueryItem> items;

        uint32 totalCount = 0;
        uint32 page = 0;
        uint32 totalPages = 0;
    };

    CreateCopyOutcome CreateArchiveCopy(
        Player* player,
        uint32 itemEntry,
        int32 randomPropertyId = 0
    );

    ArchiveQueryResult QueryArchiveItems(
        Player* player,
        uint32 page,
        ArchiveRarityFilter rarity,
        std::string const& search
    );
    
    struct ArchiveVariant
    {
        int32 randomPropertyId = 0;
        uint32 suffixFactor = 0;
    };
    
    std::vector<ArchiveVariant> GetArchiveVariants(
        Player* player,
        uint32 itemEntry
    );
}
