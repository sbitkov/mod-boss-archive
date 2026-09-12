#include "ArchiveService.h"

#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "WorldPacket.h"

#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr char const* ArchivePrefix = "BossArchive";

    std::vector<std::string> Split(std::string const& text, char delimiter)
    {
        std::vector<std::string> parts;
        std::stringstream stream(text);
        std::string part;

        while (std::getline(stream, part, delimiter))
            parts.push_back(part);

        return parts;
    }

    void SendArchiveAddonMessage(
        Player* player,
        std::string const& payload)
    {
        if (!player)
            return;

        std::string message =
            std::string(ArchivePrefix) + "\t" + payload;

        WorldPacket data;

        ChatHandler::BuildChatPacket(
            data,
            CHAT_MSG_WHISPER,
            LANG_ADDON,
            player,
            player,
            message
        );

        player->SendDirectMessage(&data);
    }

    BossArchive::ArchiveRarityFilter ParseRarity(
        std::string const& value)
    {
        if (value == "UNCOMMON")
            return BossArchive::ArchiveRarityFilter::UNCOMMON;

        if (value == "RARE")
            return BossArchive::ArchiveRarityFilter::RARE;

        if (value == "EPIC")
            return BossArchive::ArchiveRarityFilter::EPIC;

        if (value == "LEGENDARY")
            return BossArchive::ArchiveRarityFilter::LEGENDARY;

        if (value == "ARTIFACT")
            return BossArchive::ArchiveRarityFilter::ARTIFACT;

        if (value == "HEIRLOOM")
            return BossArchive::ArchiveRarityFilter::HEIRLOOM;

        return BossArchive::ArchiveRarityFilter::ALL;
    }

    void HandleListRequest(
        Player* player,
        std::vector<std::string> const& parts)
    {
        uint32 page = 0;
        BossArchive::ArchiveRarityFilter rarity =
            BossArchive::ArchiveRarityFilter::ALL;

        std::string search;

        if (parts.size() >= 2)
        {
            try
            {
                page = static_cast<uint32>(
                    std::stoul(parts[1])
                );
            }
            catch (...)
            {
                page = 0;
            }
        }

        if (parts.size() >= 3)
            rarity = ParseRarity(parts[2]);

        if (parts.size() >= 4)
            search = parts[3];

        BossArchive::ArchiveQueryResult result =
            BossArchive::QueryArchiveItems(
                player,
                page,
                rarity,
                search
            );

        SendArchiveAddonMessage(
            player,
            "BEGIN|" +
                std::to_string(result.totalCount) + "|" +
                std::to_string(result.page) + "|" +
                std::to_string(result.totalPages)
        );

        for (BossArchive::ArchiveQueryItem const& item : result.items)
        {
            SendArchiveAddonMessage(
                player,
                "ITEM|" +
                    std::to_string(item.itemEntry) + "|" +
                    std::to_string(item.quality)
            );
        }

        SendArchiveAddonMessage(
            player,
            "END"
        );
    }

    void HandleCopyRequest(
        Player* player,
        std::vector<std::string> const& parts)
    {
        if (parts.size() < 2)
        {
            SendArchiveAddonMessage(
                player,
                "COPY|INVALID_REQUEST|0"
            );

            return;
        }

        uint32 itemEntry = 0;

        try
        {
            itemEntry = static_cast<uint32>(
                std::stoul(parts[1])
            );
        }
        catch (...)
        {
            SendArchiveAddonMessage(
                player,
                "COPY|INVALID_REQUEST|0"
            );

            return;
        }

        BossArchive::CreateCopyOutcome outcome =
            BossArchive::CreateArchiveCopy(
                player,
                itemEntry
            );

        switch (outcome.result)
        {
            case BossArchive::CreateCopyResult::SUCCESS:
                SendArchiveAddonMessage(
                    player,
                    "COPY|SUCCESS|" +
                        std::to_string(itemEntry) + "|" +
                        std::to_string(outcome.itemGuid)
                );
                return;

            case BossArchive::CreateCopyResult::NOT_UNLOCKED:
                SendArchiveAddonMessage(
                    player,
                    "COPY|NOT_UNLOCKED|" +
                        std::to_string(itemEntry)
                );
                return;

            case BossArchive::CreateCopyResult::ITEM_TEMPLATE_NOT_FOUND:
                SendArchiveAddonMessage(
                    player,
                    "COPY|ITEM_NOT_FOUND|" +
                        std::to_string(itemEntry)
                );
                return;


            case BossArchive::CreateCopyResult::CANNOT_STORE:
                SendArchiveAddonMessage(
                    player,
                    "COPY|CANNOT_STORE|" +
                        std::to_string(itemEntry)
                );
                return;

            case BossArchive::CreateCopyResult::CREATE_FAILED:
                SendArchiveAddonMessage(
                    player,
                    "COPY|CREATE_FAILED|" +
                        std::to_string(itemEntry)
                );
                return;

            case BossArchive::CreateCopyResult::INVALID_PLAYER:
                SendArchiveAddonMessage(
                    player,
                    "COPY|INVALID_PLAYER|" +
                        std::to_string(itemEntry)
                );
                return;
        }
    }
}

class BossArchiveProtocolScript : public PlayerScript
{
public:
    BossArchiveProtocolScript()
        : PlayerScript(
            "BossArchiveProtocolScript",
            {
                PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT
            })
    {
    }

    bool OnPlayerCanUseChat(
        Player* player,
        uint32 /*type*/,
        uint32 language,
        std::string& msg,
        Player* receiver) override
    {
        if (!player || !receiver)
            return true;

        if (language != LANG_ADDON)
            return true;

        if (player != receiver)
            return true;

        std::string prefix =
            std::string(ArchivePrefix) + "\t";

        if (msg.rfind(prefix, 0) != 0)
            return true;

        std::string payload =
            msg.substr(prefix.size());

        std::vector<std::string> parts =
            Split(payload, '|');

        if (parts.empty())
            return false;

        if (parts[0] == "LIST")
        {
            HandleListRequest(player, parts);
            return false;
        }

        if (parts[0] == "COPY")
        {
            HandleCopyRequest(player, parts);
            return false;
        }

        return false;
    }
};

void AddBossArchiveProtocolScripts()
{
    new BossArchiveProtocolScript();
}