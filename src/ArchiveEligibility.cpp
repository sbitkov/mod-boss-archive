#include "ArchiveEligibility.h"

#include "ItemTemplate.h"
#include "ObjectMgr.h"

namespace BossArchive
{
    bool IsArchiveEligible(uint32 itemEntry)
    {
        ItemTemplate const* itemTemplate =
            sObjectMgr->GetItemTemplate(itemEntry);

        if (!itemTemplate)
            return false;

        if (itemTemplate->Quality < ITEM_QUALITY_UNCOMMON)
            return false;

        if (itemTemplate->Class == ITEM_CLASS_CONSUMABLE)
            return false;

        if (itemTemplate->Class == ITEM_CLASS_RECIPE)
            return false;

        if (itemTemplate->Class == ITEM_CLASS_KEY)
            return false;

        if (itemTemplate->StartQuest != 0)
            return false;

        if (itemTemplate->Class == ITEM_CLASS_MISC &&
            itemTemplate->SubClass == ITEM_SUBCLASS_JUNK_MOUNT)
        {
            return false;
        }

        return true;
    }
}