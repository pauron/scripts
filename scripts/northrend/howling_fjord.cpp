/* Copyright (C) 2006 - 2013 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Howling_Fjord
SD%Complete: ?
SDComment: Quest support: 11300, 11464, 11343
SDCategory: Howling Fjord
EndScriptData */

/* ContentData
npc_ancient_male_vrykul
at_ancient_male_vrykul
npc_daegarn
npc_silvermoon_harry
EndContentData */

#include "precompiled.h"

enum
{
    SPELL_ECHO_OF_YMIRON                    = 42786,
    SPELL_SECRET_OF_NIFFELVAR               = 43458,
    QUEST_ECHO_OF_YMIRON                    = 11343,
    NPC_MALE_VRYKUL                         = 24314,
    NPC_FEMALE_VRYKUL                       = 24315,

    SAY_VRYKUL_CURSED                       = -1000635,
    EMOTE_VRYKUL_POINT                      = -1000636,
    EMOTE_VRYKUL_SOB                        = -1000637,
    SAY_VRYKUL_DISPOSE                      = -1000638,
    SAY_VRYKUL_BEG                          = -1000639,
    SAY_VRYKUL_WHAT                         = -1000640,
    SAY_VRYKUL_HIDE                         = -1000641,
};

struct MANGOS_DLL_DECL npc_ancient_male_vrykulAI : public ScriptedAI
{
    npc_ancient_male_vrykulAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bEventInProgress;
    uint32 m_uiPhase;
    uint32 m_uiPhaseTimer;

    void Reset() override
    {
        m_bEventInProgress = false;
        m_uiPhase = 0;
        m_uiPhaseTimer = 0;
    }

    void StartEvent()
    {
        if (m_bEventInProgress)
            return;

        m_bEventInProgress = true;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_bEventInProgress)
            return;

        if (m_uiPhaseTimer < uiDiff)
            m_uiPhaseTimer = 5000;
        else
        {
            m_uiPhaseTimer -= uiDiff;
            return;
        }

        Creature* pFemale = GetClosestCreatureWithEntry(m_creature, NPC_FEMALE_VRYKUL, 10.0f);

        switch (m_uiPhase)
        {
            case 0:
                DoScriptText(SAY_VRYKUL_CURSED, m_creature);
                DoScriptText(EMOTE_VRYKUL_POINT, m_creature);
                break;
            case 1:
                if (pFemale)
                    DoScriptText(EMOTE_VRYKUL_SOB, pFemale);
                DoScriptText(SAY_VRYKUL_DISPOSE, m_creature);
                break;
            case 2:
                if (pFemale)
                    DoScriptText(SAY_VRYKUL_BEG, pFemale);
                break;
            case 3:
                DoScriptText(SAY_VRYKUL_WHAT, m_creature);
                break;
            case 4:
                if (pFemale)
                    DoScriptText(SAY_VRYKUL_HIDE, pFemale);
                break;
            case 5:
                DoCastSpellIfCan(m_creature, SPELL_SECRET_OF_NIFFELVAR);
                break;
            case 6:
                Reset();
                return;
        }

        ++m_uiPhase;
    }
};

CreatureAI* GetAI_npc_ancient_male_vrykul(Creature* pCreature)
{
    return new npc_ancient_male_vrykulAI(pCreature);
}

bool AreaTrigger_at_ancient_male_vrykul(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    if (pPlayer->isAlive() && pPlayer->GetQuestStatus(QUEST_ECHO_OF_YMIRON) == QUEST_STATUS_INCOMPLETE &&
            pPlayer->HasAura(SPELL_ECHO_OF_YMIRON))
    {
        if (Creature* pCreature = GetClosestCreatureWithEntry(pPlayer, NPC_MALE_VRYKUL, 20.0f))
        {
            if (npc_ancient_male_vrykulAI* pVrykulAI = dynamic_cast<npc_ancient_male_vrykulAI*>(pCreature->AI()))
                pVrykulAI->StartEvent();
        }
    }

    return true;
}

/*######
## npc_daegarn
######*/

enum
{
    QUEST_DEFEAT_AT_RING            = 11300,

    NPC_FIRJUS                      = 24213,
    NPC_JLARBORN                    = 24215,
    NPC_YOROS                       = 24214,
    NPC_OLUF                        = 23931,

    NPC_PRISONER_1                  = 24253,                // looks the same but has different abilities
    NPC_PRISONER_2                  = 24254,
    NPC_PRISONER_3                  = 24255,
};

static float afSummon[] = {838.81f, -4678.06f, -94.182f};
static float afCenter[] = {801.88f, -4721.87f, -96.143f};

// TODO: make prisoners help (unclear if summoned or using npc's from surrounding cages (summon inside small cages?))
struct MANGOS_DLL_DECL npc_daegarnAI : public ScriptedAI
{
    npc_daegarnAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bEventInProgress;
    ObjectGuid m_playerGuid;

    void Reset() override
    {
        m_bEventInProgress = false;
        m_playerGuid.Clear();
    }

    void StartEvent(Player* pPlayer)
    {
        if (m_bEventInProgress)
            return;

        m_playerGuid = pPlayer->GetObjectGuid();

        SummonGladiator(NPC_FIRJUS);
    }

    void JustSummoned(Creature* pSummon) override
    {
        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
        {
            if (pPlayer->isAlive())
            {
                pSummon->SetWalk(false);
                pSummon->GetMotionMaster()->MovePoint(0, afCenter[0], afCenter[1], afCenter[2]);
                return;
            }
        }

        Reset();
    }

    void SummonGladiator(uint32 uiEntry)
    {
        m_creature->SummonCreature(uiEntry, afSummon[0], afSummon[1], afSummon[2], 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 20 * IN_MILLISECONDS);
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*uiMotionType*/, uint32 /*uiPointId*/) override
    {
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

        // could be group, so need additional here.
        if (!pPlayer || !pPlayer->isAlive())
        {
            Reset();
            return;
        }

        if (pSummoned->IsWithinDistInMap(pPlayer, 75.0f))   // ~the radius of the ring
            pSummoned->AI()->AttackStart(pPlayer);
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        uint32 uiEntry = 0;

        // will eventually reset the event if something goes wrong
        switch (pSummoned->GetEntry())
        {
            case NPC_FIRJUS:    uiEntry = NPC_JLARBORN; break;
            case NPC_JLARBORN:  uiEntry = NPC_YOROS;    break;
            case NPC_YOROS:     uiEntry = NPC_OLUF;     break;
            case NPC_OLUF:      Reset();                return;
        }

        SummonGladiator(uiEntry);
    }
};

bool QuestAccept_npc_daegarn(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_DEFEAT_AT_RING)
    {
        if (npc_daegarnAI* pDaegarnAI = dynamic_cast<npc_daegarnAI*>(pCreature->AI()))
            pDaegarnAI->StartEvent(pPlayer);
    }

    return true;
}

CreatureAI* GetAI_npc_daegarn(Creature* pCreature)
{
    return new npc_daegarnAI(pCreature);
}

/*######
## npc_silvermoon_harry
######*/

enum
{
    QUEST_GAMBLING_DEBT         = 11464,

    SAY_AGGRO                   = -1000603,
    SAY_BEATEN                  = -1000604,

    GOSSIP_ITEM_GAMBLING_DEBT   = -3000101,
    GOSSIP_ITEM_PAYING          = -3000102,

    SPELL_BLAST_WAVE            = 15091,
    SPELL_SCORCH                = 50183,

    ITEM_HARRY_DEBT             = 34115,
    FACTION_HOSTILE_SH          = 90,                       // guessed, possibly not correct
};

struct MANGOS_DLL_DECL npc_silvermoon_harryAI : public ScriptedAI
{
    npc_silvermoon_harryAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_bHarryBeaten;
    uint32 m_uiBlastWaveTimer;
    uint32 m_uiScorchTimer;
    uint32 m_uiResetBeatenTimer;

    void Reset() override
    {
        m_bHarryBeaten = false;

        // timers guessed
        m_uiScorchTimer = 5 * IN_MILLISECONDS;
        m_uiBlastWaveTimer = 7 * IN_MILLISECONDS;

        m_uiResetBeatenTimer = MINUTE * IN_MILLISECONDS;
    }

    void AttackedBy(Unit* pAttacker) override
    {
        if (m_creature->getVictim())
            return;

        if (m_creature->IsHostileTo(pAttacker))
            AttackStart(pAttacker);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage) override
    {
        if (uiDamage > m_creature->GetHealth() || (m_creature->GetHealth() - uiDamage) * 100 / m_creature->GetMaxHealth() < 20)
        {
            if (Player* pPlayer = pDoneBy->GetCharmerOrOwnerPlayerOrPlayerItself())
            {
                if (!m_bHarryBeaten && pPlayer->GetQuestStatus(QUEST_GAMBLING_DEBT) == QUEST_STATUS_INCOMPLETE)
                {
                    uiDamage = 0;                           // Take 0 damage

                    m_creature->RemoveAllAurasOnDeath();
                    m_creature->DeleteThreatList();
                    m_creature->CombatStop(true);

                    DoScriptText(SAY_BEATEN, m_creature);
                    m_bHarryBeaten = true;
                }
            }
        }
    }

    bool IsBeaten()
    {
        return m_bHarryBeaten;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bHarryBeaten)
        {
            if (m_uiResetBeatenTimer < uiDiff)
                EnterEvadeMode();
            else
                m_uiResetBeatenTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiScorchTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SCORCH);
            m_uiScorchTimer = 10 * IN_MILLISECONDS;
        }
        else
            m_uiScorchTimer -= uiDiff;

        if (m_uiBlastWaveTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_BLAST_WAVE);
            m_uiBlastWaveTimer = 50 * IN_MILLISECONDS;
        }
        else
            m_uiBlastWaveTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_silvermoon_harry(Creature* pCreature)
{
    return new npc_silvermoon_harryAI(pCreature);
}

bool GossipHello_npc_silvermoon_harry(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pCreature->isVendor())
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    if (pPlayer->GetQuestStatus(QUEST_GAMBLING_DEBT) == QUEST_STATUS_INCOMPLETE)
    {
        if (npc_silvermoon_harryAI* pHarryAI = dynamic_cast<npc_silvermoon_harryAI*>(pCreature->AI()))
        {
            if (!pHarryAI->IsBeaten())
                pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GAMBLING_DEBT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            else
                pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_PAYING, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        }
    }

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_silvermoon_harry(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_TRADE:
            pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();

            DoScriptText(SAY_AGGRO, pCreature, pPlayer);
            pCreature->SetFactionTemporary(FACTION_HOSTILE_SH, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_RESTORE_COMBAT_STOP);
            pCreature->AI()->AttackStart(pPlayer);
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            if (!pPlayer->HasItemCount(ITEM_HARRY_DEBT, 1))
            {
                if (Item* pItem = pPlayer->StoreNewItemInInventorySlot(ITEM_HARRY_DEBT, 1))
                {
                    pPlayer->SendNewItem(pItem, 1, true, false);
                    pPlayer->CLOSE_GOSSIP_MENU();
                    pCreature->AI()->EnterEvadeMode();
                }
            }
            break;
    }

    return true;
}

void AddSC_howling_fjord()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_ancient_male_vrykul";
    pNewScript->GetAI = &GetAI_npc_ancient_male_vrykul;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_ancient_male_vrykul";
    pNewScript->pAreaTrigger = &AreaTrigger_at_ancient_male_vrykul;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_daegarn";
    pNewScript->GetAI = &GetAI_npc_daegarn;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_daegarn;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_silvermoon_harry";
    pNewScript->GetAI = &GetAI_npc_silvermoon_harry;
    pNewScript->pGossipHello = &GossipHello_npc_silvermoon_harry;
    pNewScript->pGossipSelect = &GossipSelect_npc_silvermoon_harry;
    pNewScript->RegisterSelf();
}
