#include "AllSpellScript.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "Player.h"
#include "QueryResult.h"
#include "Spell.h"

class BossArchiveSpellScript : public AllSpellScript
{
public:
    BossArchiveSpellScript() : AllSpellScript(
        "BossArchiveSpellScript",
        {
            ALLSPELLHOOK_ON_SPELL_CHECK_CAST
        })
    {
    }

    void OnSpellCheckCast(
        Spell* spell,
        bool /*strict*/,
        SpellCastResult& res) override
    {
        if (!spell)
            return;

        if (res != SPELL_CAST_OK)
            return;

        SpellInfo const* spellInfo = spell->m_spellInfo;

        if (!spellInfo)
            return;

        bool isDisenchant = false;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_DISENCHANT)
            {
                isDisenchant = true;
                break;
            }
        }

        if (!isDisenchant)
            return;

        Player* player = spell->GetCaster()->ToPlayer();

        if (!player)
            return;

        Item* item = spell->m_targets.GetItemTarget();

        if (!item)
            return;

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
            return;

        res = SPELL_FAILED_CANT_BE_DISENCHANTED;
    }
};

void AddBossArchiveSpellScripts()
{
    new BossArchiveSpellScript();
}