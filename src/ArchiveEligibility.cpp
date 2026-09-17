#include "ArchiveEligibility.h"

#include "ItemTemplate.h"
#include "ObjectMgr.h"

namespace BossArchive
{
    bool IsArchiveEligible(uint32 itemEntry)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry);
        if (!itemTemplate)
            return false;

        // Archive stores only uncommon-or-better items.
        if (itemTemplate->Quality < ITEM_QUALITY_UNCOMMON)
            return false;

        // Do not allow Archive copies to become repeatable quest starters.
        if (itemTemplate->StartQuest != 0)
            return false;

        switch (itemTemplate->Class)
        {
            case ITEM_CLASS_WEAPON:
            case ITEM_CLASS_ARMOR:
            case ITEM_CLASS_MISC:
                return true;

            default:
                return false;
        }
    }
}