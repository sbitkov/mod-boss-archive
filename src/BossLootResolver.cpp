#include "BossLootResolver.h"

#include "DatabaseEnv.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "QueryResult.h"

BossLootResolver::ItemSet BossLootResolver::ResolveBossLoot(
    uint32 bossEntry,
    uint16 lootMode)
{
    ItemSet result;
    std::unordered_set<uint64> visitedReferences;

    QueryResult creatureResult = WorldDatabase.Query(
        "SELECT lootid "
        "FROM creature_template "
        "WHERE entry = {}",
        bossEntry
    );

    if (!creatureResult)
        return result;

    Field* fields = creatureResult->Fetch();
    uint32 lootId = fields[0].Get<uint32>();

    if (!lootId)
        return result;

    ResolveCreatureLoot(
        lootId,
        lootMode,
        result,
        visitedReferences
    );

    return result;
}

void BossLootResolver::ResolveCreatureLoot(
    uint32 lootId,
    uint16 lootMode,
    ItemSet& result,
    std::unordered_set<uint64>& visitedReferences)
{
    QueryResult lootResult = WorldDatabase.Query(
        "SELECT Item, Reference, QuestRequired, LootMode, GroupId "
        "FROM creature_loot_template "
        "WHERE Entry = {}",
        lootId
    );

    if (!lootResult)
        return;

    do
    {
        Field* fields = lootResult->Fetch();

        uint32 itemEntry   = fields[0].Get<uint32>();
        uint32 referenceId = fields[1].Get<uint32>();
        bool questRequired = fields[2].Get<uint8>() != 0;
        uint16 rowLootMode = fields[3].Get<uint16>();
        uint32 groupId     = fields[4].Get<uint32>();

        if ((rowLootMode & lootMode) == 0)
            continue;

        if (referenceId != 0)
        {
            ResolveReferenceLoot(
                referenceId,
                lootMode,
                groupId,
                result,
                visitedReferences
            );

            continue;
        }

        if (!itemEntry)
            continue;

        if (questRequired)
            continue;

        if (IsArchiveEligibleItem(itemEntry))
            result.insert(itemEntry);
    }
    while (lootResult->NextRow());
}

void BossLootResolver::ResolveReferenceLoot(
    uint32 referenceId,
    uint16 lootMode,
    uint32 requestedGroup,
    ItemSet& result,
    std::unordered_set<uint64>& visitedReferences)
{
    uint64 visitKey =
        (static_cast<uint64>(referenceId) << 32) |
        static_cast<uint64>(requestedGroup);

    if (!visitedReferences.insert(visitKey).second)
        return;

    QueryResult referenceResult;

    if (requestedGroup != 0)
    {
        referenceResult = WorldDatabase.Query(
            "SELECT Item, Reference, QuestRequired, LootMode, GroupId "
            "FROM reference_loot_template "
            "WHERE Entry = {} AND GroupId = {}",
            referenceId,
            requestedGroup
        );
    }
    else
    {
        referenceResult = WorldDatabase.Query(
            "SELECT Item, Reference, QuestRequired, LootMode, GroupId "
            "FROM reference_loot_template "
            "WHERE Entry = {}",
            referenceId
        );
    }

    if (!referenceResult)
        return;

    do
    {
        Field* fields = referenceResult->Fetch();

        uint32 itemEntry   = fields[0].Get<uint32>();
        uint32 nestedRefId = fields[1].Get<uint32>();
        bool questRequired = fields[2].Get<uint8>() != 0;
        uint16 rowLootMode = fields[3].Get<uint16>();
        uint32 groupId     = fields[4].Get<uint32>();

        if ((rowLootMode & lootMode) == 0)
            continue;

        if (nestedRefId != 0)
        {
            ResolveReferenceLoot(
                nestedRefId,
                lootMode,
                groupId,
                result,
                visitedReferences
            );

            continue;
        }

        if (!itemEntry)
            continue;

        if (questRequired)
            continue;

        if (IsArchiveEligibleItem(itemEntry))
            result.insert(itemEntry);
    }
    while (referenceResult->NextRow());
}

bool BossLootResolver::IsArchiveEligibleItem(uint32 itemEntry)
{
    ItemTemplate const* itemTemplate =
        sObjectMgr->GetItemTemplate(itemEntry);

    if (!itemTemplate)
        return false;

    return itemTemplate->InventoryType != INVTYPE_NON_EQUIP;
}