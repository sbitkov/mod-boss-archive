#include "BossLootResolver.h"
#include "ArchiveService.h"

#include "Chat.h"
#include "CommandScript.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"
#include "RBAC.h"

using namespace Acore::ChatCommands;

class BossArchiveCommandScript : public CommandScript
{
public:
    BossArchiveCommandScript() : CommandScript("BossArchiveCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "bossloot", HandleBossLootCommand, rbac::RBAC_PERM_COMMAND_SERVER_INFO, Console::No },
            { "archiveboss", HandleArchiveBossCommand, rbac::RBAC_PERM_COMMAND_SERVER_INFO, Console::No },
            { "archiveitem", HandleArchiveItemCommand, rbac::RBAC_PERM_COMMAND_SERVER_INFO, Console::No },
            { "archive", HandleArchiveCommand, rbac::RBAC_PERM_COMMAND_SERVER_INFO, Console::No }
        };

        return commandTable;
    }

    static bool HandleBossLootCommand(
        ChatHandler* handler,
        uint32 bossEntry,
        Optional<uint16> lootModeArg)
    {
        uint16 lootMode = lootModeArg.value_or(1);

        BossLootResolver::ItemSet items =
            BossLootResolver::ResolveBossLoot(bossEntry, lootMode);

        handler->PSendSysMessage(
            "Boss Archive loot for entry {} (loot mode {}):",
            bossEntry,
            lootMode
        );

        if (items.empty())
        {
            handler->SendSysMessage("No archive-eligible items found.");
            return true;
        }

        for (uint32 itemEntry : items)
        {
            ItemTemplate const* itemTemplate =
                sObjectMgr->GetItemTemplate(itemEntry);

            if (!itemTemplate)
                continue;

            handler->PSendSysMessage(
                "  {} - {}",
                itemEntry,
                itemTemplate->Name1
            );
        }

        handler->PSendSysMessage(
            "Boss Archive: {} items found.",
            items.size()
        );

        return true;
    }

    static bool HandleArchiveBossCommand(
        ChatHandler* handler,
        uint32 bossEntry,
        Optional<uint16> lootModeArg)
    {
        Player* player = handler->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage("Boss Archive: player not found.");
            return false;
        }

        uint32 guid = player->GetGUID().GetCounter();
        uint16 lootMode = lootModeArg.value_or(1);

        QueryResult bossResult = CharacterDatabase.Query(
            "SELECT 1 "
            "FROM character_boss_archive "
            "WHERE guid = {} "
            "AND boss_entry = {} "
            "AND loot_mode = {}",
            guid,
            bossEntry,
            lootMode
        );

        if (!bossResult)
        {
            handler->PSendSysMessage(
                "Boss Archive: boss entry {} (loot mode {}) is not unlocked.",
                bossEntry,
                lootMode
            );

            return true;
        }

        QueryResult itemResult = CharacterDatabase.Query(
            "SELECT item_entry "
            "FROM character_archive_item "
            "WHERE guid = {} "
            "AND source_type = {} "
            "AND source_entry = {} "
            "AND source_context = {} "
            "ORDER BY item_entry",
            guid,
            static_cast<uint8>(BossArchive::ArchiveSourceType::BOSS),
            bossEntry,
            lootMode
        );

        handler->PSendSysMessage(
            "Boss Archive: saved items for boss entry {} (loot mode {}):",
            bossEntry,
            lootMode
        );

        if (!itemResult)
        {
            handler->SendSysMessage("No saved items found.");
            return true;
        }

        uint32 count = 0;

        do
        {
            Field* fields = itemResult->Fetch();
            uint32 itemEntry = fields[0].Get<uint32>();

            ItemTemplate const* itemTemplate =
                sObjectMgr->GetItemTemplate(itemEntry);

            if (!itemTemplate)
            {
                handler->PSendSysMessage(
                    "  {} - [unknown item]",
                    itemEntry
                );

                ++count;
                continue;
            }

            handler->PSendSysMessage(
                "  {} - {}",
                itemEntry,
                itemTemplate->Name1
            );

            ++count;
        }
        while (itemResult->NextRow());

        handler->PSendSysMessage(
            "Boss Archive: {} saved items.",
            count
        );

        return true;
    }

    static bool HandleArchiveItemCommand(
        ChatHandler* handler,
        uint32 itemEntry)
    {
        Player* player = handler->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage("Boss Archive: player not found.");
            return false;
        }

        BossArchive::CreateCopyOutcome outcome =
            BossArchive::CreateArchiveCopy(player, itemEntry);

        ItemTemplate const* itemTemplate =
            sObjectMgr->GetItemTemplate(itemEntry);

        switch (outcome.result)
        {
            case BossArchive::CreateCopyResult::SUCCESS:
                if (itemTemplate)
                {
                    handler->PSendSysMessage(
                        "Boss Archive: created archive copy of {} - {} (item guid {}).",
                        itemEntry,
                        itemTemplate->Name1,
                        outcome.itemGuid
                    );
                }
                else
                {
                    handler->PSendSysMessage(
                        "Boss Archive: created archive copy of {} (item guid {}).",
                        itemEntry,
                        outcome.itemGuid
                    );
                }

                return true;

            case BossArchive::CreateCopyResult::NOT_UNLOCKED:
                handler->PSendSysMessage(
                    "Boss Archive: item {} is not unlocked.",
                    itemEntry
                );
                return true;

            case BossArchive::CreateCopyResult::ITEM_TEMPLATE_NOT_FOUND:
                handler->PSendSysMessage(
                    "Boss Archive: item template {} not found.",
                    itemEntry
                );
                return true;

            case BossArchive::CreateCopyResult::CANNOT_STORE:
                handler->SendSysMessage(
                    "Boss Archive: unable to create archive copy."
                );
                return true;

            case BossArchive::CreateCopyResult::CREATE_FAILED:
                handler->SendSysMessage(
                    "Boss Archive: failed to create item."
                );
                return true;

            case BossArchive::CreateCopyResult::INVALID_PLAYER:
                handler->SendSysMessage(
                    "Boss Archive: player not found."
                );
                return false;
        }

        return false;
    }
    static bool HandleArchiveCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage("Boss Archive: player not found.");
            return false;
        }

        uint32 guid = player->GetGUID().GetCounter();

        QueryResult result = CharacterDatabase.Query(
            "SELECT boss_entry, loot_mode, unlocked_at "
            "FROM character_boss_archive "
            "WHERE guid = {} "
            "ORDER BY unlocked_at, boss_entry, loot_mode",
            guid
        );

        handler->SendSysMessage("Boss Archive:");

        if (!result)
        {
            handler->SendSysMessage("No bosses unlocked.");
            return true;
        }

        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();

            uint32 bossEntry = fields[0].Get<uint32>();
            uint16 lootMode = fields[1].Get<uint16>();

            std::string bossName = "[unknown creature]";

            QueryResult creatureResult = WorldDatabase.Query(
                "SELECT name "
                "FROM creature_template "
                "WHERE entry = {}",
                bossEntry
            );

            if (creatureResult)
            {
                Field* creatureFields = creatureResult->Fetch();
                bossName = creatureFields[0].Get<std::string>();
            }

            handler->PSendSysMessage(
                "  {} - {} (loot mode {})",
                bossEntry,
                bossName,
                lootMode
            );

            ++count;
        }
        while (result->NextRow());

        handler->PSendSysMessage(
            "Boss Archive: {} bosses unlocked.",
            count
        );

        return true;
    }
};

void AddBossArchiveCommandScripts()
{
    new BossArchiveCommandScript();
}