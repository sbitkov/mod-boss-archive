#include "ArchiveService.h"

#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"

class BossArchiveCraftScript : public PlayerScript
{
public:
    BossArchiveCraftScript() : PlayerScript("BossArchiveCraftScript",
    {
        PLAYERHOOK_ON_CREATE_ITEM
    }) {}

    void OnPlayerCreateItem(
        Player* player,
        Item* item,
        uint32 /*count*/) override
    {
        if (!player || !item)
            return;

        uint32 itemEntry = item->GetEntry();

        BossArchive::UnlockItem(
            player,
            itemEntry,
            BossArchive::ArchiveSourceType::CRAFT,
            itemEntry
        );
        
        BossArchive::UnlockVariant(player, item);
    }
};

void AddBossArchiveCraftScripts()
{
    new BossArchiveCraftScript();
}
