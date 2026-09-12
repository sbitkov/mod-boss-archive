#include "ArchiveService.h"

#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"

class BossArchiveQuestScript : public PlayerScript
{
public:
    BossArchiveQuestScript() : PlayerScript("BossArchiveQuestScript",
    {
        PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST
    }) {}

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        if (!player || !quest)
            return;

        uint32 questId = quest->GetQuestId();

        for (uint8 i = 0; i < QUEST_REWARDS_COUNT; ++i)
        {
            uint32 itemEntry = quest->RewardItemId[i];

            if (!itemEntry)
                continue;

            BossArchive::UnlockItem(
                player,
                itemEntry,
                BossArchive::ArchiveSourceType::QUEST,
                questId
            );
        }

        for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        {
            uint32 itemEntry = quest->RewardChoiceItemId[i];

            if (!itemEntry)
                continue;

            BossArchive::UnlockItem(
                player,
                itemEntry,
                BossArchive::ArchiveSourceType::QUEST,
                questId
            );
        }
    }
};

void AddBossArchiveQuestScripts()
{
    new BossArchiveQuestScript();
}
