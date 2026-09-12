#pragma once

#include "Define.h"

#include <string>
#include <vector>

class Player;

namespace BossArchive
{
    constexpr uint32 ARCHIVE_PAGE_SIZE = 10;

    enum class ArchiveSourceType : uint8
    {
        BOSS = 1,
        QUEST = 2,
        CRAFT = 3,
        VENDOR = 4
    };

    bool UnlockItem(
        Player* player,
        uint32 itemEntry,
        ArchiveSourceType sourceType,
        uint32 sourceEntry,
        uint32 sourceContext = 0
    );

    enum class CreateCopyResult : uint8
    {
        SUCCESS = 0,
        INVALID_PLAYER,
        NOT_UNLOCKED,
        ITEM_TEMPLATE_NOT_FOUND,
        CANNOT_STORE,
        CREATE_FAILED
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
        uint32 itemEntry
    );

    ArchiveQueryResult QueryArchiveItems(
        Player* player,
        uint32 page,
        ArchiveRarityFilter rarity,
        std::string const& search
    );
}
