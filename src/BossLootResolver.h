#pragma once

#include "Define.h"

#include <unordered_set>

class BossLootResolver
{
public:
    using ItemSet = std::unordered_set<uint32>;

    // Returns every archive-eligible item that belongs to the boss loot table
    // for the specified loot mode.
    static ItemSet ResolveBossLoot(uint32 bossEntry, uint16 lootMode);

private:
    static void ResolveCreatureLoot(
        uint32 lootId,
        uint16 lootMode,
        ItemSet& result,
        std::unordered_set<uint64>& visitedReferences
    );

    static void ResolveReferenceLoot(
        uint32 referenceId,
        uint16 lootMode,
        uint32 requestedGroup,
        ItemSet& result,
        std::unordered_set<uint64>& visitedReferences
    );
};