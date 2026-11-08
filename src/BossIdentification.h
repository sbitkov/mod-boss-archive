#pragma once

#include "Creature.h"

namespace BossArchive
{
    // AzerothCore currently does not classify Scarlet Commander Mograine as a
    // dungeon boss, despite scripting him as part of the Cathedral encounter.
    // Keep this fallback narrowly scoped until the core classification is fixed.
    inline bool IsArchiveDungeonBoss(Creature const* creature)
    {
        if (!creature)
            return false;

        if (creature->IsDungeonBoss())
            return true;

        return creature->GetEntry() == 3976 && creature->GetMapId() == 189;
    }
}
