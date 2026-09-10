#include "BossLootResolver.h"

#include "Chat.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "Player.h"
#include "QueryResult.h"
#include "ScriptMgr.h"

class BossArchivePlayerScript : public PlayerScript
{
public:
    BossArchivePlayerScript() : PlayerScript("BossArchivePlayerScript",
    {
        PLAYERHOOK_ON_CREATURE_KILL,
        PLAYERHOOK_CAN_SELL_ITEM
    }) {}

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        if (!killer || !killed)
            return;

        if (!killed->IsDungeonBoss())
            return;

        uint32 guid = killer->GetGUID().GetCounter();
        uint32 bossEntry = killed->GetEntry();
        uint16 lootMode = killed->GetLootMode();

        BossLootResolver::ItemSet items =
            BossLootResolver::ResolveBossLoot(bossEntry, lootMode);

        CharacterDatabase.Execute(
            "INSERT IGNORE INTO character_boss_archive "
            "(guid, boss_entry, loot_mode) VALUES ({}, {}, {})",
            guid, bossEntry, lootMode
        );

        for (uint32 itemEntry : items)
        {
            CharacterDatabase.Execute(
                "INSERT IGNORE INTO character_boss_archive_item "
                "(guid, boss_entry, loot_mode, item_entry) "
                "VALUES ({}, {}, {}, {})",
                guid, bossEntry, lootMode, itemEntry
            );
        }

        ChatHandler(killer->GetSession()).PSendSysMessage(
            "Boss Archive: boss unlocked, entry = {}, loot mode = {}, items = {}",
            bossEntry, lootMode, items.size()
        );
    }

    bool OnPlayerCanSellItem(
        Player* player,
        Item* item,
        Creature* /*creature*/) override
    {
        if (!player || !item)
            return true;

        uint32 guid = player->GetGUID().GetCounter();
        uint32 itemGuid = item->GetGUID().GetCounter();

        QueryResult archiveResult = CharacterDatabase.Query(
            "SELECT 1 "
            "FROM character_boss_archive_copy "
            "WHERE guid = {} "
            "AND item_guid = {} "
            "LIMIT 1",
            guid,
            itemGuid
        );

        if (!archiveResult)
            return true;

        ChatHandler(player->GetSession()).SendSysMessage(
            "Boss Archive: archive copies cannot be sold."
        );

        return false;
    }
};

void AddBossArchivePlayerScripts()
{
    new BossArchivePlayerScript();
}