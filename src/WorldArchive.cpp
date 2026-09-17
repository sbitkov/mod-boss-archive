#include "ArchiveService.h"

#include "Creature.h"
#include "GameObject.h"
#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"

class BossArchiveWorldScript : public PlayerScript
{
public:
    BossArchiveWorldScript() : PlayerScript("BossArchiveWorldScript",
    {
        PLAYERHOOK_ON_LOOT_ITEM
    }) {}

    void OnPlayerLootItem(
        Player* player,
        Item* item,
        uint32 /*count*/,
        ObjectGuid lootGuid) override
    {
        if (!player || !item)
            return;

        uint32 sourceEntry = 0;
        uint32 sourceContext = 0;

        if (lootGuid.IsCreature())
        {
            Creature* creature =
                player->GetMap()->GetCreature(lootGuid);

            if (!creature)
                return;

            // Boss loot already has BOSS provenance.
            if (creature->IsDungeonBoss() || creature->isWorldBoss())
                return;

            sourceEntry = creature->GetEntry();
            sourceContext = 1; // Creature
        }
        else if (lootGuid.IsGameObject())
        {
            GameObject* gameObject =
                player->GetMap()->GetGameObject(lootGuid);

            if (!gameObject)
                return;

            sourceEntry = gameObject->GetEntry();
            sourceContext = 2; // GameObject
        }
        else
        {
            return;
        }

        uint32 itemEntry = item->GetEntry();

        BossArchive::UnlockItem(
            player,
            itemEntry,
            BossArchive::ArchiveSourceType::WORLD,
            sourceEntry,
            sourceContext
        );

        BossArchive::UnlockVariant(player, item);
    }
};

void AddBossArchiveWorldScripts()
{
    new BossArchiveWorldScript();
}