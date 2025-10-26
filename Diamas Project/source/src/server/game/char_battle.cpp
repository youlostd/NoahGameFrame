#include "char.h"

#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "GBattle.h"
#include "pvp.h"
#include "skill.h"
#include "start_position.h"
#include "dungeon.h"
#include "log.h"
#include "priv_manager.h"
#include "vector.h"
#include "marriage.h"
#include "GArena.h"
#include "exchange.h"
#include "shop_manager.h"
#include "party.h"
#include "guild.h"
#include "guild_manager.h"
#include "questmanager.h"
#include "questlua.h"
#include "threeway_war.h"
#include "BlueDragon.h"
#include "constants.h"
#include "PetSystem.h"
#include "CharUtil.hpp"
#include "cube.h"
#include <game/HuntingManager.hpp>
#include "item.h"
#include "war_map.h"

#include <game/GamePacket.hpp>
#include <game/AffectConstants.hpp>
#include <game/MotionTypes.hpp>
#include <game/MobConstants.hpp>
#include <game/ItemConstants.hpp>

#include <storm/Util.hpp>

#include "PacketUtils.hpp"

#ifdef ENABLE_BATTLE_PASS
#include "battle_pass.h"
#endif

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

PointValue AdjustExpByLevel(const CHARACTER *ch, PointValue exp)
{
    if (ch->GetLevel() > PLAYER_EXP_TABLE_MAX)
    {
        double factor = 1.0;
        auto ret = exp;

        for (auto i = 0; i < ch->GetLevel() - 135; ++i)
            factor -= 0.05;

        ret = ret * factor;

        if (ret < 1.0)
            return 1;

        return llround(ret);
    }
    return llround(exp);
}

// TODO: move to own .cpp/.hpp
void CharacterHitLog::Clear() { m_count.clear(); }

uint32_t CharacterHitLog::Hit(uint32_t vid) { return m_count[vid]++; }

bool CHARACTER::CanBeginFight() const
{
    if (!CanMove())
        return false;

    return m_pointsInstant.position == POS_STANDING && !IsDead() && !IsStun();
}

void CHARACTER::BeginFight(CHARACTER *pkVictim)
{
    if (pkVictim->IsPetPawn())
        return;

#ifdef __FAKE_PC__
    if (FakePC_Check() && !CPVPManager::instance().CanAttack(this, pkVictim))
        return;
#endif

    if (pkVictim && pkVictim->IsPet())
    {
        const auto owner = pkVictim->GetPet()->GetOwnerPtr();

        SetVictim(owner);
        SetPosition(POS_FIGHTING);
        SetNextStatePulse(1);
        return;
    }

    SetVictim(pkVictim);
    SetPosition(POS_FIGHTING);
    SetNextStatePulse(1);
}

bool CHARACTER::CanFight() const { return m_pointsInstant.position >= POS_FIGHTING ? true : false; }

void CHARACTER::CreateFly(uint8_t bType, CHARACTER *pkVictim)
{
    TPacketGCCreateFly packFly;
    packFly.bType = bType;
    packFly.dwStartVID = GetVID();
    packFly.dwEndVID = pkVictim->GetVID();
    PacketAround(m_map_view, this, HEADER_GC_CREATE_FLY, packFly);
}

void CHARACTER::DistributeSP(CHARACTER *pkKiller, int iMethod)
{
    if (pkKiller->GetSP() >= pkKiller->GetMaxSP())
        return;

    const auto now = get_dword_time();

    const bool bAttacking = (now - GetLastAttackTime()) < 3000;
    const bool bMoving = (now - GetLastMoveTime()) < 3000;

    if (iMethod == 1)
    {
        const int num = Random::get(0, 3);

        if (!num)
        {
            const int iLvDelta = GetLevel() - pkKiller->GetLevel();
            int iAmount = 0;

            if (iLvDelta >= 5)
                iAmount = 10;
            else if (iLvDelta >= 0)
                iAmount = 6;
            else if (iLvDelta >= -3)
                iAmount = 2;

            if (iAmount != 0)
            {
                iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;

                if (iAmount >= 11)
                    CreateFly(FLY_SP_BIG, pkKiller);
                else if (iAmount >= 7)
                    CreateFly(FLY_SP_MEDIUM, pkKiller);
                else
                    CreateFly(FLY_SP_SMALL, pkKiller);

                pkKiller->PointChange(POINT_SP, iAmount);
            }
        }
    }
    else
    {
        if (pkKiller->GetJob() == JOB_SHAMAN || (pkKiller->GetJob() == JOB_SURA && pkKiller->GetSkillGroup() == 2))
        {
            int iAmount;

            if (bAttacking)
                iAmount = 2 + GetMaxSP() / 100;
            else if (bMoving)
                iAmount = 3 + GetMaxSP() * 2 / 100;
            else
                iAmount = 10 + GetMaxSP() * 3 / 100; // Æò»ó½Ã

            iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
            pkKiller->PointChange(POINT_SP, iAmount);
        }
        else
        {
            int iAmount;

            if (bAttacking)
                iAmount = 2 + pkKiller->GetMaxSP() / 200;
            else if (bMoving)
                iAmount = 2 + pkKiller->GetMaxSP() / 100;
            else
            {
                // Æò»ó½Ã
                if (pkKiller->GetHP() < pkKiller->GetMaxHP())
                    iAmount = 2 + (pkKiller->GetMaxSP() / 100); // ÇÇ ´Ù ¾ÈÃ¡À»¶§
                else
                    iAmount = 9 + (pkKiller->GetMaxSP() / 100); // ±âº»
            }

            iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
            pkKiller->PointChange(POINT_SP, iAmount);
        }
    }
}

#ifdef __FAKE_PC__
struct FCollectFakePCVictimList
{
    FCollectFakePCVictimList(CHARACTER *pkFakePC, std::vector<CHARACTER *> &rvec_VictimList)
        : m_pkFakePC(pkFakePC), m_rvec_VictimList(rvec_VictimList)
    {
    }

    void operator()(CEntity *ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER *pkChr = static_cast<CHARACTER *>(ent);

        if (pkChr == m_pkFakePC)
            return;

        const float fDist = DISTANCE_APPROX(m_pkFakePC->GetX() - pkChr->GetX(), m_pkFakePC->GetY() - pkChr->GetY());
        if (fDist > m_pkFakePC->GetMobAttackRange() * 1.25f)
            return;

        const float fMaxRotationDif = 100.0f;

        float fRealRotation = m_pkFakePC->GetRotation();
        float fHitRotation =
            GetDegreeFromPositionXY(m_pkFakePC->GetX(), m_pkFakePC->GetY(), pkChr->GetX(), pkChr->GetY());

        if (fRealRotation > 180.0f)
            fRealRotation = 360.0f - fRealRotation;
        if (fHitRotation > 180.0f)
            fHitRotation = 360.0f - fHitRotation;

        const float fDif = abs(fRealRotation - fHitRotation);
        if (fDif > fMaxRotationDif)
            return;

        if (!CPVPManager::instance().CanAttack(m_pkFakePC, pkChr))
            return;

        m_rvec_VictimList.push_back(pkChr);
    }

    CHARACTER *m_pkFakePC;
    std::vector<CHARACTER *> &m_rvec_VictimList;
};
#endif

bool CHARACTER::Attack(CHARACTER *victim, uint32_t motionKey, uint32_t time, uint8_t type)
{

    if (gConfig.testServer)
    {
        SPDLOG_INFO("[TEST_SERVER] Attack : {0} type {1}, MobBattleType {2}", GetName(), type,
                    !GetMobBattleType() ? 0 : GetMobAttackRange());
    }

    const uint32_t dwCurrentTime = get_dword_time();

    if (!CanMove())
    {
        return false;
    }

    if (victim->IsPet())
        return false;

    int iRet;

    if (type == 0)
    {
        if (IsPC())
        {
			if(gConfig.IsHackCheckedMap(GetMapIndex()))
			{
				if(IS_SPEED_HACK(this, victim, time, motionKey))
					return false;
			} else {
				IS_SPEED_HACK(this, victim, time, motionKey);

			}
        }
        std::vector<CHARACTER *> vec_pkVictimList;
        vec_pkVictimList.push_back(victim);

#ifdef __FAKE_PC__
        if (FakePC_Check())
        {
            const auto pkWeapon = GetWear(WEAR_WEAPON);
            if (pkWeapon && pkWeapon->GetSubType() != WEAPON_ARROW && pkWeapon->GetSubType() != WEAPON_QUIVER)
            {
                FCollectFakePCVictimList f(this, vec_pkVictimList);
                GetSectree()->ForEachAround(f);
            }
        }
#endif
        switch (GetMobBattleType())
        {
        case BATTLE_TYPE_MELEE:
        case BATTLE_TYPE_POWER:
        case BATTLE_TYPE_TANKER:
        case BATTLE_TYPE_SUPER_POWER:
        case BATTLE_TYPE_SUPER_TANKER:
            iRet = battle_melee_attack(this, victim);
            break;

        case BATTLE_TYPE_RANGE:
            FlyTarget(victim->GetVID(), victim->GetX(), victim->GetY(), HEADER_CG_FLY_TARGETING);

            iRet = Shoot(0, motionKey) ? BATTLE_DAMAGE : BATTLE_NONE;
            break;

        case BATTLE_TYPE_MAGIC:
            FlyTarget(victim->GetVID(), victim->GetX(), victim->GetY(), HEADER_CG_FLY_TARGETING);

            iRet = Shoot(1, motionKey) ? BATTLE_DAMAGE : BATTLE_NONE;
            break;

        default:
            SPDLOG_ERROR("Unhandled battle type {0}", GetMobBattleType());
            iRet = BATTLE_NONE;
            break;
        }
    }
    else
    {
        /*
         * TODO: Fix this to work with the new game loop
         *if(IsPC() && time - m_dwLastSkillTime > 1500) {
            SPDLOG_WARN(
                "hack: skill use time: name {0} PID: {1} delta: {2}",
                GetName(), GetPlayerID(),
                time - m_dwLastSkillTime);
            this->ChatInfoCond(gConfig.testServer, "Attack canceled: Detected skill use time hack");
            return false;
        }*/

        SPDLOG_DEBUG("Attack call ComputeSkill {0} {1}", type, victim ? victim->GetName() : "");
        iRet = ComputeSkill(type, victim);
    }

    // if (test_server && IsPC())
    //	STORM_LOG(Info, "{0} Attack {1} type {2} ret {3}", GetName(), victim->GetName(), type, iRet);
    if (iRet == BATTLE_DAMAGE || iRet == BATTLE_DEAD)
    {
        victim->SetSyncOwner(this);

        if (victim->CanBeginFight())
        {
            victim->BeginFight(this);
        }

        if (!IsPC())
        {
            MonsterChat(MONSTER_CHAT_ATTACK);
        }

        OnMove(true);
        victim->OnMove();

        // only pc sets victim null. For npc, state machine will reset this.
        if (BATTLE_DEAD == iRet && IsPC())
            SetVictim(nullptr);
#ifdef __FAKE_PC__
        else if (IsPC() && !victim->IsPC())
            FakePC_Owner_ForceFocus(victim);
#endif
        if (BATTLE_DEAD != iRet && IsPC())
        {
            if (m_petSystem)
                m_petSystem->LaunchAttack(victim);
        }

        return true;
    }

    return false;
}

void CHARACTER::DeathPenalty(uint8_t bTown)
{
#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex()))
        return;
#endif
    SPDLOG_TRACE("DEATH_PERNALY_CHECK(%s) town(%d)", GetName(), bTown);

    Cube_close(this);

    if (GetLevel() < 10)
    {
        SPDLOG_DEBUG("NO_DEATH_PENALTY_LESS_LV10({})", GetName());
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "용신의 가호로 경험치가 떨어지지 않았습니다.");
        return;
    }

    if (Random::get(0, 2))
    {
        SPDLOG_DEBUG("NO_DEATH_PENALTY_LUCK({0})", GetName());
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "용신의 가호로 경험치가 떨어지지 않았습니다.");
        return;
    }

    if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY))
    {
        REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

        if (!bTown) // ±¹Á¦ ¹öÀü¿¡¼­´Â Á¦ÀÚ¸® ºÎÈ°½Ã¸¸ ¿ë½ÅÀÇ °¡È£¸¦ »ç¿ëÇÑ´Ù. (¸¶À» º¹±Í½Ã´Â °æÇèÄ¡ ÆÐ³ÎÆ¼ ¾øÀ½)
        {
            if (FindAffect(AFFECT_NO_DEATH_PENALTY))
            {
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "용신의 가호로 경험치가 떨어지지 않았습니다.");
                RemoveAffect(AFFECT_NO_DEATH_PENALTY);
                return;
            }
        }

        auto iLoss = std::min<PointValue>(
            800000,
            ((GetNextExp() * aiExpLossPercents[std::clamp<int64_t>(GetLevel(), 1, PLAYER_EXP_TABLE_MAX)]) / 100));

        if (bTown)
            iLoss = 0;

        if (IsEquipUniqueItem(UNIQUE_ITEM_TEARDROP_OF_GODNESS))
            iLoss /= 2;

        PointChange(POINT_EXP, -iLoss, true);
    }
}

bool CHARACTER::IsStun() const
{
    if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN))
        return true;

    return false;
}

EVENTFUNC(StunEvent)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("StunEvent> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;

    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }
    ch->m_pkStunEvent = nullptr;
    ch->Dead();
    return 0;
}

void CHARACTER::Stun()
{
    if (IsStun())
        return;

    if (IsDead())
        return;

    if (!IsPC() && m_pkParty)
    {
        m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);
    }

    PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
    PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));

    event_cancel(&m_pkRecoveryEvent); // È¸º¹ ÀÌº¥Æ®¸¦ Á×ÀÎ´Ù.

    TPacketGCStun pack;
    pack.vid = m_vid;
    PacketAround(m_map_view, this, HEADER_GC_STUN, pack);

    SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

    if (m_pkStunEvent)
        return;

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;

    m_pkStunEvent = event_create(StunEvent, info, THECORE_SECS_TO_PASSES(3));
}

EVENTINFO(SCharDeadEventInfo)
{
    bool isPC;
    uint32_t dwID;

    SCharDeadEventInfo() : isPC(0), dwID(0) {}
};

EVENTFUNC(dead_event)
{
    const SCharDeadEventInfo *info = static_cast<SCharDeadEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("dead_event: null event info");
        return 0;
    }

    CHARACTER *ch;

    if (true == info->isPC)
    {
        ch = g_pCharManager->FindByPID(info->dwID);
    }
    else
    {
        ch = g_pCharManager->Find(info->dwID);
    }

    if (nullptr == ch)
        return 0;

    ch->m_pkDeadEvent = nullptr;

    if (ch->GetDesc())
    {
        ch->GetDesc()->SetPhase(PHASE_GAME);
        ch->SetPosition(POS_STANDING);

        // Nation war
        if (g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex()))
        {
            if (g_pThreeWayWar->GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
            {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "성지에서 부활 기회를 모두 잃었습니다! 마을로 이동합니다!");
                ch->GoHome();
            }
            else
            {
                ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
            }

            ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
            ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

            ch->ReviveInvisible(5);

            ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
            return 0;
        }
        else if (g_pThreeWayWar->IsThreeWayWarMapIndex(ch->GetMapIndex())) // NW index, but not sungzi -> we are @ pass
        {
            ch->Show(ch->GetMapIndex(), GetPassStartX(ch->GetEmpire()), GetPassStartY(ch->GetEmpire()));

            ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
            ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
            ch->ReviveInvisible(5);

            ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
            return 0;
        }

        // Otherwise...
        PIXEL_POSITION pos;

        if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
            ch->WarpSet(ch->GetMapIndex(), pos.x, pos.y);
        else
        {
            SPDLOG_ERROR("cannot find spawn position (name {0})", ch->GetName());
            ch->WarpSet(EMPIRE_START_MAP(ch->GetMapIndex()), EMPIRE_START_X(ch->GetEmpire()),
                        EMPIRE_START_Y(ch->GetEmpire()));
        }

        // 1 3 21 23 41 43 63
        const auto mapIndex = ch->GetMapIndex();
        if (mapIndex == 1 || mapIndex == 3 || mapIndex == 21 || mapIndex == 23 || mapIndex == 41 || mapIndex == 43)
        {
            ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
        }
        else
        {
            ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
            ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
        }

        ch->DeathPenalty(0);

        ch->StartRecoveryEvent();

        ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
    }
    else
    {
        /*
         *
        if (ch->IsMonster())
        {
            if (!ch->IsRevive() && ch->HasReviverInParty())
            {
                ch->SetPosition(POS_STANDING);
                ch->SetHP(ch->GetMaxHP());

                ch->ViewReencode();

                ch->SetAggressive();
                ch->SetRevive(true);


                return 0;
            }
        }
                 */

        if (!ch->IsToggleMount())
            M2_DESTROY_CHARACTER(ch);
    }

    return 0;
}

bool CHARACTER::IsDead() const
{
    if (m_pointsInstant.position == POS_DEAD)
        return true;

    return false;
}

void CHARACTER::RewardGold(CHARACTER *pkAttacker)
{
    if (!m_pkMobData)
        return;


    const auto isAutoLoot = pkAttacker->GetPremiumRemainSeconds(PREMIUM_AUTOLOOT) > 0 ||
                            pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_AUTOLOOT);

    PIXEL_POSITION pos;

    if (!isAutoLoot)
    {
        if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
            return;
    }

    int iGoldPercent = MobRankStats[GetMobRank()].iGoldPercent;

    if (pkAttacker->IsPC())
        iGoldPercent = iGoldPercent * (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD_DROP)) / 100;

    if (pkAttacker->GetPoint(POINT_MALL_GOLDBONUS))
        iGoldPercent += (iGoldPercent * pkAttacker->GetPoint(POINT_MALL_GOLDBONUS) / 100);

    iGoldPercent = iGoldPercent * g_pCharManager->GetMobGoldDropRate(pkAttacker) / 100;

    // ADD_PREMIUM
    if (pkAttacker->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0 ||
        pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_LUCKY_GOLD))
        iGoldPercent += iGoldPercent;
    // END_OF_ADD_PREMIUM

    if (IsEquipUniqueItem(UNIQUE_CHRISTMAS_LOLLIPOP))
        iGoldPercent += 10;

    if (iGoldPercent > 100)
        iGoldPercent = 100;

    int iPercent;

    if (GetMobRank() >= MOB_RANK_BOSS)
        iPercent = ((iGoldPercent * PERCENT_LVDELTA_BOSS(pkAttacker->GetLevel(), GetLevel())) / 100);
    else
        iPercent = ((iGoldPercent * PERCENT_LVDELTA(pkAttacker->GetLevel(), GetLevel())) / 100);

    if (Random::get(1, 100) > iPercent)
        return;

    int iGoldMultipler = 1;

    // °³ÀÎ Àû¿ë
    if (pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
        if (Random::get(1, 100) <= pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
            iGoldMultipler *= 2;

    if (gConfig.testServer)
        pkAttacker->ChatPacket(CHAT_TYPE_PARTY, "rate %d", g_pCharManager->GetMobGoldAmountRate(pkAttacker));

    {
        Gold iGold = Random::get(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
        iGold = iGold * g_pCharManager->GetMobGoldAmountRate(pkAttacker) / 100;
        iGold = iGold * iGoldMultipler;

        if (iGold > 0)
            pkAttacker->GiveGold(iGold);
    }

    // DBManager::instance().SendMoneyLog(MONEY_LOG_MONSTER, GetRaceNum(), iTotalGold);
}

void CHARACTER::Reward(bool bItemDrop)
{
    const auto raceNum = GetRaceNum();

    if (raceNum == 5001) // ¿Ö±¸´Â µ·À» ¹«Á¶°Ç µå·Ó
    {
        PIXEL_POSITION pos;

        if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
            return;

        CItem *item;
        Gold iGold = Random::get(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
        iGold = iGold * g_pCharManager->GetMobGoldAmountRate(nullptr) / 100;
        const Gold iSplitCount = Random::get(25, 35);

        SPDLOG_INFO("WAEGU Dead gold {} split {}", iGold, iSplitCount);

        for (int i = 1; i <= iSplitCount; ++i)
        {
            if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold / iSplitCount)))
            {
                if (i != 0)
                {
                    pos.x = Random::get(-7, 7) * 20;
                    pos.y = Random::get(-7, 7) * 20;

                    pos.x += GetX();
                    pos.y += GetY();
                }

                item->AddToGround(GetMapIndex(), pos);
                item->StartDestroyEvent(gConfig.itemGroundTime);
            }
        }
        return;
    }

    // PROF_UNIT puReward("Reward");
    CHARACTER *pkAttacker = DistributeExp();

    if (!pkAttacker)
        return;

    // PROF_UNIT pu1("r1");
    if (pkAttacker->IsPC())
    {

        if (GetLevel() - pkAttacker->GetLevel() >= -10)
        {
            // if (pkAttacker->GetRealAlignment() < 0)
            //{
            //    if (pkAttacker->IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_KILL))
            //    {
            //        pkAttacker->UpdateAlignment(0);
            //    }
            //    else { pkAttacker->UpdateAlignment(0); }
            //}
            // else { pkAttacker->UpdateAlignment(0); }

            pkAttacker->UpdateAlignment(0);
        }

        pkAttacker->SetQuestNPCID(GetVID());

        quest::CQuestManager::instance().Kill(pkAttacker->GetPlayerID(), raceNum);
        g_pCharManager->KillLog(raceNum);

#ifdef ENABLE_BATTLE_PASS
        const uint8_t bBattlePassId = pkAttacker->GetBattlePassId();
        if (bBattlePassId && GetMobRank() < MOB_RANK_BOSS && !IsStone())
        {
            uint32_t dwMonsterVnum, dwToKillCount;
            if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, MONSTER_KILL, &dwMonsterVnum,
                                                                 &dwToKillCount))
            {
                if ((dwMonsterVnum == raceNum  || dwMonsterVnum == 0) &&
                    pkAttacker->GetMissionProgress(MONSTER_KILL, bBattlePassId) < dwToKillCount)
                    pkAttacker->UpdateMissionProgress(MONSTER_KILL, bBattlePassId, 1, dwToKillCount);
            }
        }

        if (bBattlePassId && GetMobRank() >= MOB_RANK_BOSS && !IsStone())
        {
            uint32_t dwMonsterVnum, dwToKillCount;
            if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, BOSS_KILL, &dwMonsterVnum,
                                                                 &dwToKillCount))
            {
                if ((dwMonsterVnum == raceNum  || dwMonsterVnum == 0) &&
                    pkAttacker->GetMissionProgress(BOSS_KILL, bBattlePassId) < dwToKillCount)
                    pkAttacker->UpdateMissionProgress(BOSS_KILL, bBattlePassId, 1, dwToKillCount);
            }
        }

        if (bBattlePassId && IsStone())
        {
            uint32_t dwMonsterVnum, dwToKillCount;
            if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, METIN_KILL, &dwMonsterVnum,
                                                                 &dwToKillCount))
            {
                if ((dwMonsterVnum == raceNum  || dwMonsterVnum == 0) &&
                    pkAttacker->GetMissionProgress(METIN_KILL, bBattlePassId) < dwToKillCount)
                    pkAttacker->UpdateMissionProgress(METIN_KILL, bBattlePassId, 1, dwToKillCount);
            }
        }
#endif


        const auto &v = HuntingManager::instance().GetHuntingMissionByMob(raceNum);
        for (const auto &m : v)
        {
            if (m.isDaily && !pkAttacker->IsHuntingMissionForCurrentDay(m.id))
                continue;

            bool fulfilled = true;
            for (const auto &reqId : m.required)
            {
                fulfilled = pkAttacker->IsHuntingMissionCompleted(reqId);
            }
            if (fulfilled && pkAttacker->GetHuntingMissionProgress(m.id) < m.needCount &&
                pkAttacker->GetLevel() >= m.minLevel)
                if (pkAttacker->UpdateHuntingMission(m.id, 1, m.needCount))
                    break;
        }

        if (!Random::get(0, 9))
        {
            if (pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY))
            {
                const int64_t iHP = pkAttacker->GetMaxHP() * pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY) / 100;
                pkAttacker->PointChange(POINT_HP, iHP);
                CreateFly(FLY_HP_SMALL, pkAttacker);
            }

            if (pkAttacker->GetPoint(POINT_KILL_SP_RECOVER))
            {
                const int64_t iSP = pkAttacker->GetMaxSP() * pkAttacker->GetPoint(POINT_KILL_SP_RECOVER) / 100;
                pkAttacker->PointChange(POINT_SP, iSP);
                CreateFly(FLY_SP_SMALL, pkAttacker);
            }
        }
    }
    // pu1.Pop();

    if (!bItemDrop)
        return;

    PIXEL_POSITION pos = GetXYZ();

    if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), pos.x, pos.y, pos))
        return;

    //
    // µ· µå·Ó
    //
    // PROF_UNIT pu2("r2");
    if (gConfig.testServer)
        SPDLOG_INFO("{}: Drop money for attacker {}", GetName(), pkAttacker->GetName());
    if (auto d = pkAttacker->GetDesc(); d)
    {
        if (!d->GetDropsAllowed())
            return;
    }

    RewardGold(pkAttacker);
    // pu2.Pop();

    //
    // ¾ÆÀÌÅÛ µå·Ó
    //
    // PROF_UNIT pu3("r3");
    CItem *item;

    static std::vector<CItem *> s_vec_item;
    s_vec_item.clear();

    if (ITEM_MANAGER::instance().CreateDropItem(this, pkAttacker, s_vec_item))
    {
        if (s_vec_item.empty())
            ;
        else if (s_vec_item.size() == 1)
        {
            item = s_vec_item[0];
            item->AddToGround(GetMapIndex(), pos);

            if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
                item->StartDestroyEvent(gConfig.itemGroundTimeLong);
                item->SetOwnership(pkAttacker, gConfig.itemOwnershipTimeLong);
            }
            else {
                item->StartDestroyEvent(gConfig.itemGroundTime);
                item->SetOwnership(pkAttacker, gConfig.itemOwnershipTime);
            }


            pos.x = Random::get(-7, 7) * 20;
            pos.y = Random::get(-7, 7) * 20;
            pos.x += GetX();
            pos.y += GetY();

            SPDLOG_INFO("DROP_ITEM: {} {} {} from {}", item->GetName(), pos.x, pos.y, GetName());
        }
        else
        {
            if (CHARACTER_MANAGER::instance().IsWorldBoss(raceNum))
            {
                // Everyone shares ownership
                int iItemIdx = s_vec_item.size() - 1;

                std::priority_queue<std::pair<int, CHARACTER *>> pq;

                int total_dam = 0;

                for (auto &it : m_map_kDamage)
                {
                    int iDamage = it.second.iTotalDamage;

                    {
                        CHARACTER *ch = g_pCharManager->Find(it.first);

                        if (ch)
                        {
                            pq.push(std::make_pair(iDamage, ch));
                            total_dam += iDamage;
                        }
                    }
                }

                std::vector<CHARACTER *> v;

                while (!pq.empty())
                {
                    v.push_back(pq.top().second);
                    pq.pop();
                }


                while (iItemIdx >= 0)
                {
                    item = s_vec_item[iItemIdx--];

                    if (!item)
                    {
                        SPDLOG_ERROR("{}: Item null in vector idx {}", GetName(), iItemIdx + 1);
                        continue;
                    }

                    item->AddToGround(GetMapIndex(), pos);

                   const auto randomIt = Random::get( v );
                   auto ch = (*randomIt);
                 

                    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
                        item->StartDestroyEvent(gConfig.itemGroundTimeLong);
                        item->SetOwnership(ch->IsPet() ? ch->GetPet()->GetOwnerPtr() : ch, gConfig.itemOwnershipTimeLong);
                    }
                    else {
                        item->StartDestroyEvent(gConfig.itemGroundTime);
                        item->SetOwnership(ch->IsPet() ? ch->GetPet()->GetOwnerPtr() : ch, gConfig.itemOwnershipTime);
                    }

                    pos.x = Random::get(-7, 7) * 20;
                    pos.y = Random::get(-7, 7) * 20;
                    pos.x += GetX();
                    pos.y += GetY();

                    SPDLOG_INFO("{}: Drop item: {} {} {}", GetName(), item->GetName(), pos.x, pos.y);
                }
            }
            else
            {
                int iItemIdx = s_vec_item.size() - 1;

                std::priority_queue<std::pair<DamageValue, CHARACTER *>> pq;

                DamageValue total_dam = 0;

                for (const auto& [vid, battleInfo] : m_map_kDamage)
                {
                    DamageValue iDamage = battleInfo.iTotalDamage;
#ifdef __FAKE_PC__
                    iDamage += it.second.iTotalFakePCDamage;
#endif
                    if (iDamage > 0)
                    {
                        CHARACTER *ch = g_pCharManager->Find(vid);

                        if (ch)
                        {
                            pq.emplace(iDamage, ch);
                            total_dam += iDamage;
                        }
                    }
                }

                std::vector<CHARACTER *> v;

                while (!pq.empty() && pq.top().first * 10 >= total_dam)
                {
                    v.push_back(pq.top().second);
                    pq.pop();
                }

                if (v.empty())
                {
                    // No one has given so much damage, no ownership
                    while (iItemIdx >= 0)
                    {
                        item = s_vec_item[iItemIdx--];

                        if (!item)
                        {
                            SPDLOG_ERROR("{}: Item null in vector idx {}", GetName(), iItemIdx + 1);
                            continue;
                        }

                        item->AddToGround(GetMapIndex(), pos);
                        // Less than 10% Ownership of damage
                        // item->SetOwnership(pkAttacker);
                        item->StartDestroyEvent(gConfig.itemGroundTime);

                        pos.x = Random::get(-7, 7) * 20;
                        pos.y = Random::get(-7, 7) * 20;
                        pos.x += GetX();
                        pos.y += GetY();

                        SPDLOG_INFO("{}: Drop item: {} {} {}", GetName(), item->GetName(), pos.x, pos.y);
                    }
                }
                else
                {
                    // Only those who gave a lot of damage share ownership
                    auto it = v.begin();

                    while (iItemIdx >= 0)
                    {
                        item = s_vec_item[iItemIdx--];

                        if (!item)
                        {
                            SPDLOG_ERROR("item null in vector idx %d", iItemIdx + 1);
                            continue;
                        }

                        item->AddToGround(GetMapIndex(), pos);

                        CHARACTER *ch = *it;

                        if (ch->GetParty())
                            ch = ch->GetParty()->GetNextOwnership(ch, GetX(), GetY(), GetMapIndex());

                        ++it;

                        if (it == v.end())
                            it = v.begin();

                        if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
                            item->StartDestroyEvent(gConfig.itemGroundTimeLong);
                            item->SetOwnership(
                                ch->IsPet() ? ch->GetPet()->GetOwnerPtr() : ch,
                                gConfig.itemOwnershipTimeLong);
                        } else {
                            item->StartDestroyEvent(gConfig.itemGroundTime);
                            item->SetOwnership(
                                ch->IsPet() ? ch->GetPet()->GetOwnerPtr() : ch,
                                gConfig.itemOwnershipTime);
                        }


                        pos.x = Random::get(-7, 7) * 20;
                        pos.y = Random::get(-7, 7) * 20;
                        pos.x += GetX();
                        pos.y += GetY();

                        SPDLOG_INFO("{}: Drop item: {} {} {}", GetName(), item->GetName(), pos.x, pos.y);
                    }
                }
            }
        }
    }

    m_map_kDamage.clear();
}

struct TItemDropPenalty
{
    int iInventoryPct; // Range: 1 ~ 1000
    int iInventoryQty; // Range: --
    int iEquipmentPct; // Range: 1 ~ 100
    int iEquipmentQty; // Range: --
};

TItemDropPenalty aItemDropPenalty_kor[9] = {
    {0, 0, 0, 0},    // ¼±¿Õ
    {0, 0, 0, 0},    // ¿µ¿õ
    {0, 0, 0, 0},    // ¼ºÀÚ
    {0, 0, 0, 0},    // ÁöÀÎ
    {0, 0, 0, 0},    // ¾ç¹Î
    {25, 1, 5, 1},   // ³¶ÀÎ
    {50, 2, 10, 1},  // ¾ÇÀÎ
    {75, 4, 15, 1},  // ¸¶µÎ
    {100, 8, 20, 1}, // ÆÐ¿Õ
};

void CHARACTER::ItemDropPenalty(CHARACTER *pkKiller)
{
#ifdef INSTINCT_BUILD
    return;
#else

    if (IsShop())
        return;

    if (GetLevel() < 50)
        return;

    if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
        return;

    struct TItemDropPenalty *table = &aItemDropPenalty_kor[0];

    if (GetLevel() < 10)
        return;

    int iAlignIndex;

    if (GetRealAlignment() >= 120000)
        iAlignIndex = 0;
    else if (GetRealAlignment() >= 80000)
        iAlignIndex = 1;
    else if (GetRealAlignment() >= 40000)
        iAlignIndex = 2;
    else if (GetRealAlignment() >= 10000)
        iAlignIndex = 3;
    else if (GetRealAlignment() >= 0)
        iAlignIndex = 4;
    else if (GetRealAlignment() > -40000)
        iAlignIndex = 5;
    else if (GetRealAlignment() > -80000)
        iAlignIndex = 6;
    else if (GetRealAlignment() > -120000)
        iAlignIndex = 7;
    else
        iAlignIndex = 8;

    std::vector<std::pair<CItem *, int>> vec_item;
    CItem *pkItem;
    int i;
    bool isDropAllEquipments = false;

    TItemDropPenalty &r = table[iAlignIndex];
    SPDLOG_INFO("{} align {} inven_pct {} equip_pct {}", GetName(), iAlignIndex, r.iInventoryPct, r.iEquipmentPct);

    bool bDropInventory = r.iInventoryPct >= Random::get(1, 1000);
    bool bDropEquipment = r.iEquipmentPct >= Random::get(1, 100);
    bool bDropAntiDropUniqueItem = false;

    if ((bDropInventory || bDropEquipment) && IsEquipUniqueItem(UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY))
    {
        bDropInventory = false;
        bDropEquipment = false;
        bDropAntiDropUniqueItem = true;
    }

    if (bDropInventory) // Drop Inventory
    {
        std::vector<uint8_t> vec_bSlots;

        for (i = 0; i < INVENTORY_MAX_NUM; ++i)
            if (GetInventoryItem(i))
                vec_bSlots.push_back(i);

        if (!vec_bSlots.empty())
        {
            random_shuffle(vec_bSlots.begin(), vec_bSlots.end());

            int iQty = std::min<int>(vec_bSlots.size(), r.iInventoryQty);

            if (iQty)
                iQty = Random::get(1, iQty);

            for (i = 0; i < iQty; ++i)
            {
                pkItem = GetInventoryItem(vec_bSlots[i]);

                if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP) ||
                    !CanTradeRestrictedItem(pkItem, this))
                    continue;

                if (pkItem->IsSealed())
                    continue;

                SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], std::numeric_limits<uint16_t>::max());
                vec_item.emplace_back(pkItem->RemoveFromCharacter(), INVENTORY);
            }
        }
        else if (iAlignIndex == 8)
            isDropAllEquipments = true;
    }

    if (bDropEquipment) // Drop Equipment
    {
        std::vector<uint8_t> vec_bSlots;

        for (i = 0; i < WEAR_MAX_NUM; ++i)
            if (GetWear(i))
                vec_bSlots.push_back(i);

        if (!vec_bSlots.empty())
        {
            Random::shuffle(vec_bSlots.begin(), vec_bSlots.end());
            int iQty;

            if (isDropAllEquipments)
                iQty = vec_bSlots.size();
            else
                iQty = std::min<int>(vec_bSlots.size(), Random::get(1, r.iEquipmentQty));

            if (iQty)
                iQty = Random::get(1, iQty);

            for (i = 0; i < iQty; ++i)
            {
                pkItem = GetWear(vec_bSlots[i]);

                if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP) ||
                    !CanTradeRestrictedItem(pkItem, this))
                    continue;

                SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], std::numeric_limits<uint16_t>::max());
                vec_item.emplace_back(pkItem->RemoveFromCharacter(), EQUIPMENT);
            }
        }
    }

    if (bDropAntiDropUniqueItem)
    {
        CItem *item = GetWear(WEAR_UNIQUE1);

        if (item && item->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
        {
            SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE1, std::numeric_limits<uint16_t>::max());
            vec_item.emplace_back(item->RemoveFromCharacter(), EQUIPMENT);
        }

        item = GetWear(WEAR_UNIQUE2);

        if (item && item->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
        {
            SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE2, std::numeric_limits<uint16_t>::max());
            vec_item.emplace_back(item->RemoveFromCharacter(), EQUIPMENT);
        }
    }

    {
        PIXEL_POSITION pos;
        pos.x = GetX();
        pos.y = GetY();

        for (auto j = 0; j < vec_item.size(); ++j)
        {
            CItem *item = vec_item[j].first;
            int window = vec_item[j].second;

            item->AddToGround(GetMapIndex(), pos);
            item->StartDestroyEvent();

            SPDLOG_INFO("DROP_ITEM_PK: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
            LogManager::instance().ItemLog(this, item, "DEAD_DROP",
                                           (window == INVENTORY) ? "INVENTORY"
                                                                 : ((window == EQUIPMENT) ? "EQUIPMENT" : ""));

            pos.x = GetX() + Random::get(-7, 7) * 20;
            pos.y = GetY() + Random::get(-7, 7) * 20;
        }
    }
#endif
}

class FPartyAlignmentCompute
{
  public:
    FPartyAlignmentCompute(int iAmount, int x, int y, int32_t map) : m_iKillerMap{map}
    {
        m_iAmount = iAmount;
        m_iCount = 0;
        m_iStep = 0;
        m_iKillerX = x;
        m_iKillerY = y;
    }

    void operator()(CHARACTER *pkChr)
    {
        if (DISTANCE_APPROX(pkChr->GetX() - m_iKillerX, pkChr->GetY() - m_iKillerY) < PARTY_DEFAULT_RANGE &&
            pkChr->GetMapIndex() == m_iKillerMap)
        {
            if (m_iStep == 0)
            {
                ++m_iCount;
            }
            else
            {
                pkChr->UpdateAlignment(0);
            }
        }
    }

    int m_iAmount;
    int m_iCount;
    int m_iStep;

    int m_iKillerX;
    int m_iKillerY;
    int32_t m_iKillerMap;
};

void CHARACTER::Dead(CHARACTER *pkKiller, bool bImmediateDead)
{
    if (IsDead())
        return;

    SetPosition(POS_DEAD);

    Unmount(this);

#ifdef __FAKE_PC__
    if (FakePC_IsSupporter())
    {
        // if (auto pkItem = FakePC_GetOwnerItem())
        //	pkItem->SetItemCooltime(30);

        SendI18nChatPacket(FakePC_GetOwner(), CHAT_TYPE_INFO, "Your fake pc died.");
    }
#endif

    if (!pkKiller && m_dwKillerPID)
        pkKiller = g_pCharManager->FindByPID(m_dwKillerPID);

#ifdef __FAKE_PC__
    if (pkKiller && pkKiller->FakePC_IsSupporter())
        pkKiller = FakePC_GetOwner();
#endif

    m_dwKillerPID = 0;

    bool isAgreedPVP = false;
    bool isUnderGuildWar = false;
    bool isDuel = false;
    bool isForked = false;

    if (pkKiller && pkKiller->IsPC())
    {
        if (pkKiller->m_pkChrTarget == this)
            pkKiller->SetTarget(nullptr);

        isAgreedPVP = CPVPManager::instance().Dead(this, pkKiller->GetPlayerID(), pkKiller);
        isDuel = CArenaManager::instance().OnDead(pkKiller, this);

      	const auto mapIndex = pkKiller->GetMapIndex();
        if (gConfig.IsDuelOnlyMap(mapIndex)) {
            if (isAgreedPVP || isDuel) {
                pkKiller->PointChange(POINT_HP,
                                      pkKiller->GetMaxHP() - pkKiller->GetHP());
                pkKiller->PointChange(POINT_SP,
                                      pkKiller->GetMaxSP() - pkKiller->GetSP());
            }
        }

#if defined(WJ_COMBAT_ZONE)
        CCombatZoneManager::instance().OnDead(pkKiller, this);
#endif
        if (IsPC())
        {
            CGuild *g1 = GetGuild();
            CGuild *g2 = pkKiller->GetGuild();

            if (g1 && g2)
                if (g1->UnderWar(g2->GetID()))
                    isUnderGuildWar = true;

            pkKiller->SetQuestNPCID(GetVID());
            quest::CQuestManager::instance().Kill(pkKiller->GetPlayerID(), quest::QUEST_NO_NPC);
            CGuildManager::instance().Kill(pkKiller, this);
        }
    }

#ifdef ENABLE_HYDRA_DUNGEON
    if (pkKiller)
        CHydraDungeonManager::instance().OnKill(pkKiller, this);
#endif

    // CHECK_FORKEDROAD_WAR
    if (IsPC())
    {
        if (g_pThreeWayWar->IsThreeWayWarMapIndex(GetMapIndex()))
            isForked = true;
    }
    // END_CHECK_FORKEDROAD_WAR

    if (pkKiller && !isAgreedPVP && !isUnderGuildWar && IsPC() && !isDuel && !isForked)
    {
        if (GetGMLevel() == GM_PLAYER || gConfig.testServer)
        {
            ItemDropPenalty(pkKiller);
        }
    }

    if (true == isForked)
    {
        g_pThreeWayWar->onDead(this, pkKiller);
    }


    if (pkKiller && pkKiller->IsPC() && !gConfig.IsRestartWithAffectsMap(pkKiller->GetMapIndex())) {
        RemoveGoodAffect();
        RemoveBadAffect();
        RemoveAffect(AFFECT_PREVENT_GOOD);
    }
    ClearTarget();

    if (pkKiller && IsPC())
    {
        if (!pkKiller->IsPC())
        {
            if (!isForked)
            {
                SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
                LogManager::instance().CharLog(this, pkKiller->GetRaceNum(), "DEAD_BY_NPC",
                                               pkKiller->GetName().c_str());
            }
        }
        else
        {

            REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
                if (pkKiller->GetDesc()->GetHwid() != GetDesc()->GetHwid())
                {
                    if (!g_pCharManager->HasPlayerKilledHwid(pkKiller->GetPlayerID(), GetDesc()->GetHwid()))
                    {
                        const uint8_t bBattlePassId = pkKiller->GetBattlePassId();
                        if (bBattlePassId)
                        {
                            uint32_t dwMinLevel, dwToKillCount;
                            if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, PLAYER_KILL,
                                                                                 &dwMinLevel, &dwToKillCount))
                            {
                                if (GetLevel() >= dwMinLevel &&
                                    pkKiller->GetMissionProgress(PLAYER_KILL, bBattlePassId) < dwToKillCount)
                                    pkKiller->UpdateMissionProgress(PLAYER_KILL, bBattlePassId, 1, dwToKillCount);
                            }
                        }
                    }
                    g_pCharManager->RegisterKill(pkKiller->GetPlayerID(), GetDesc()->GetHwid());
                }

            if (GetEmpire() != pkKiller->GetEmpire())
            {
                LogManager::instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC",
                                               fmt::sprintf("%d %d %d %s %d %d %d %s", GetEmpire(), GetAlignment(),
                                                            GetPKMode(), GetName(), pkKiller->GetEmpire(),
                                                            pkKiller->GetAlignment(), pkKiller->GetPKMode(),
                                                            pkKiller->GetName())
                                                   .c_str());
            }
            else
            {
                // PREVENT REFLECT BUG ABUSE
                if (!isAgreedPVP && !isUnderGuildWar && !IsKillerMode() && GetAlignment() >= 0 && !isDuel &&
                    !isForked && pkKiller->GetPKMode() != PK_MODE_PEACE)
                {
                    int iNoPenaltyProb;

                    if (pkKiller->GetAlignment() >= 0) // 1/3 percent down
                        iNoPenaltyProb = 33;
                    else // 4/5 percent down
                        iNoPenaltyProb = 20;

                    if (Random::get(1, 100) < iNoPenaltyProb)
                        SendI18nChatPacket(pkKiller, CHAT_TYPE_INFO, "용신의 보호로 아이템이 떨어지지 않았습니다.");
                    else
                    {
                        if (pkKiller->GetParty())
                        {
                            FPartyAlignmentCompute f(-20000, pkKiller->GetX(), pkKiller->GetY(),
                                                     pkKiller->GetMapIndex());
                            pkKiller->GetParty()->ForEachOnlineMember(f);

                            if (f.m_iCount == 0)
                                pkKiller->UpdateAlignment(0);
                            else
                            {
                                SPDLOG_INFO("ALIGNMENT PARTY count %d amount %d", f.m_iCount, f.m_iAmount);

                                f.m_iStep = 1;
                                pkKiller->GetParty()->ForEachOnlineMember(f);
                            }
                        }
                        else
                            pkKiller->UpdateAlignment(0);
                    }
                }

                LogManager::instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC",
                                               fmt::sprintf("%d %d %d %s %d %d %d %s", GetEmpire(), GetAlignment(),
                                                            GetPKMode(), GetName(), pkKiller->GetEmpire(),
                                                            pkKiller->GetAlignment(), pkKiller->GetPKMode(),
                                                            pkKiller->GetName())
                                                   .c_str());

#ifdef ENABLE_BATTLE_PASS


#endif
            }
        }
    }
    else
    {
        REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
    }

    ClearSync();

    // SPDLOG_WARN( "stun cancel %s[%d]", GetName(), (uint32_t)GetVID());
    event_cancel(&m_pkStunEvent); // ±âÀý ÀÌº¥Æ®´Â Á×ÀÎ´Ù.

    if (IsPC())
    {
        m_dwLastDeadTime = get_dword_time();
        SetKillerMode(false);
        GetDesc()->SetPhase(PHASE_DEAD);
		quest::CQuestManager::instance().Dead(GetPlayerID());

    }
#ifdef __FAKE_PC__
    else if (!FakePC_IsSupporter())
#else
    else
#endif
    {
        // °¡µå¿¡°Ô °ø°Ý¹ÞÀº ¸ó½ºÅÍ´Â º¸»óÀÌ ¾ø¾î¾ß ÇÑ´Ù.
        if (!HasNoRewardFlag())
        {
            if (!(pkKiller && pkKiller->IsPC() && pkKiller->GetGuild() &&
                  pkKiller->GetGuild()->UnderAnyWar(GUILD_WAR_TYPE_FIELD)))
            {
                // ºÎÈ°ÇÏ´Â ¸ó½ºÅÍ´Â º¸»óÀ» ÁÖÁö ¾Ê´Â´Ù.
#ifdef __FAKE_PC__
                if (!FakePC_Check() && GetMobTable().dwResurrectionVnum)
#else
                if (GetMobTable().dwResurrectionVnum && !bImmediateDead)
#endif
                {
                    // DUNGEON_MONSTER_REBIRTH_BUG_FIX
                    CHARACTER *chResurrect =
                        g_pCharManager->SpawnMob(GetMobTable().dwResurrectionVnum, GetMapIndex(), GetX(), GetY(),
                                                 GetZ(), true, static_cast<int>(GetRotation()));
                    if (GetDungeon() && chResurrect)
                    {
                        chResurrect->SetDungeon(GetDungeon());
                    }
                    // END_OF_DUNGEON_MONSTER_REBIRTH_BUG_FIX

                    Reward(false);
                }
                else if (IsRevive() == true)
                {
                    Reward(false);
                }
                else
                {
                    Reward(true); // Drops gold, item, etc..
                }
            }
            else
            {
                if (pkKiller->m_dwUnderGuildWarInfoMessageTime < get_dword_time())
                {
                    pkKiller->m_dwUnderGuildWarInfoMessageTime = get_dword_time() + 60000;
                    SendI18nChatPacket(pkKiller, CHAT_TYPE_INFO, "<길드> 길드전중에는 사냥에 따른 이익이 없습니다.");
                }
            }
        }
        else if (pkKiller && pkKiller->IsPC())
        {
            pkKiller->SetQuestNPCID(GetVID());
            quest::CQuestManager::instance().Kill(pkKiller->GetPlayerID(), GetRaceNum());
            g_pCharManager->KillLog(GetRaceNum());
        }
    }

    if (pkKiller && pkKiller->GetDungeon())
        pkKiller->GetDungeon()->OnKill(pkKiller, this);
    // BOSS_KILL_LOG
    if (GetMobRank() >= MOB_RANK_BOSS && pkKiller && pkKiller->IsPC())
    {
        const std::string buf = fmt::sprintf("%d %ld", gConfig.channel, pkKiller->GetMapIndex());

        if (IsStone())
            LogManager::instance().CharLog(pkKiller, GetRaceNum(), "STONE_KILL", buf.c_str());
        else
            LogManager::instance().CharLog(pkKiller, GetRaceNum(), "BOSS_KILL", buf.c_str());
    }
    // END_OF_BOSS_KILL_LOG

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if ((IsStone()) || (IsMonster()))
    {
        if (pkKiller && pkKiller->IsPC())
        {
            if (((GetMobTable().dwVnum == static_cast<uint32_t>(MeleyLair::MOBVNUM_RESPAWN_STONE_STEP2)) ||
                 (GetMobTable().dwVnum == static_cast<uint32_t>(MeleyLair::MOBVNUM_RESPAWN_BOSS_STEP3))) &&
                (MeleyLair::CMgr::IsMeleyMap(pkKiller->GetMapIndex())))
                MeleyLair::CMgr::instance().OnKill(GetMobTable().dwVnum, pkKiller);
            else if (MeleyLair::CMgr::IsMeleyMap(pkKiller->GetMapIndex()))
                MeleyLair::CMgr::instance().OnKillCommon(this, pkKiller);
        }
    }
#endif

    TPacketGCDead pack;
    pack.vid = m_vid;
    PacketAround(m_map_view, this, HEADER_GC_DEAD, pack);

    REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

    //
    // Dead ÀÌº¥Æ® »ý¼º,
    //
    // Dead ÀÌº¥Æ®¿¡¼­´Â ¸ó½ºÅÍÀÇ °æ¿ì ¸îÃÊ ÈÄ¿¡ Destroy µÇµµ·Ï ÇØÁÖ¸ç,
    // PCÀÇ °æ¿ì 3ºÐ ÀÖ´Ù°¡ ¸¶À»¿¡¼­ ³ª¿Àµµ·Ï ÇØ ÁØ´Ù. 3ºÐ ³»¿¡´Â À¯Àú·ÎºÎÅÍ
    // ¸¶À»¿¡¼­ ½ÃÀÛÇÒ °ÇÁö, ¿©±â¼­ ½ÃÀÛÇÒ °ÇÁö °áÁ¤À» ¹Þ´Â´Ù.
    if (isDuel == false)
    {
        if (m_pkDeadEvent)
        {
            event_cancel(&m_pkDeadEvent);
        }

        if (IsStone())
            ClearStone();

        SCharDeadEventInfo *pEventInfo = AllocEventInfo<SCharDeadEventInfo>();

        if (IsPC())
        {
            pEventInfo->isPC = true;
            pEventInfo->dwID = this->GetPlayerID();

            m_pkDeadEvent = event_create(dead_event, pEventInfo, THECORE_SECS_TO_PASSES(180));
        }
        else
        {
            pEventInfo->isPC = false;
            pEventInfo->dwID = this->GetVID();

#ifndef ENABLE_ALWAYS_INSTANT_DEATH
            if (IsRevive() == false && HasReviverInParty() == true)
            {
                m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : THECORE_SECS_TO_PASSES(3));
            }
            else
            {
                m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : THECORE_SECS_TO_PASSES(10));
            }
#else
            m_pkDeadEvent = event_create(dead_event, pEventInfo, 1);

#endif
        }
    }

    if (m_pkExchange != nullptr)
    {
        m_pkExchange->Cancel();
    }

    if (IsCubeOpen() == true)
    {
        Cube_close(this);
    }

    if (GetShop())
    {
        GetShop()->RemoveGuest(this);
        SetShop(nullptr);
    }

    CloseSafebox();
}

void CHARACTER::SendDamagePacket(CHARACTER *attacker, int damage, uint32_t damageFlag)
{
    if (attacker->IsPet())
    {
        const auto *owner = attacker->GetPet()->GetOwnerPtr();
        if (owner)
        {
            TPacketGCDamageInfo damageInfo = {};
            damageInfo.dwVID = static_cast<uint32_t>(GetVID());
            damageInfo.flag = damageFlag;
            damageInfo.damage = damage;

            if (owner->GetDesc() != nullptr)
            {
                owner->GetDesc()->Send(HEADER_GC_DAMAGE_INFO, damageInfo);
            }
        }
    }

    if (IsPC() || (attacker->IsPC() && attacker->GetTarget() == this))
    {
        TPacketGCDamageInfo damageInfo = {};
        damageInfo.dwVID = static_cast<uint32_t>(GetVID());
        damageInfo.flag = damageFlag;
        damageInfo.damage = damage;

        if (GetDesc() != nullptr)
        {
            GetDesc()->Send(HEADER_GC_DAMAGE_INFO, damageInfo);
        }

        if (attacker->GetDesc() != nullptr)
        {
            attacker->GetDesc()->Send(HEADER_GC_DAMAGE_INFO, damageInfo);
        }
        /*
           if (GetArenaObserverMode() == false && GetArena() != NULL)
           {
           GetArena()->SendPacketToObserver(&damageInfo, sizeof(TPacketGCDamageInfo));
           }
         */
    }
}

void CHARACTER::PirateTanaka_Damage()
{
    if (GetRaceNum() == 5001)
    {
        bool bDropMoney = false;
        const int64_t iPercent = (GetHP() * 100) / GetMaxHP();

        if (iPercent <= 10 && GetMaxSP() < 5)
        {
            SetMaxSP(5);
            bDropMoney = true;
        }
        else if (iPercent <= 20 && GetMaxSP() < 4)
        {
            SetMaxSP(4);
            bDropMoney = true;
        }
        else if (iPercent <= 40 && GetMaxSP() < 3)
        {
            SetMaxSP(3);
            bDropMoney = true;
        }
        else if (iPercent <= 60 && GetMaxSP() < 2)
        {
            SetMaxSP(2);
            bDropMoney = true;
        }
        else if (iPercent <= 80 && GetMaxSP() < 1)
        {
            SetMaxSP(1);
            bDropMoney = true;
        }

        if (bDropMoney)
        {
            const uint32_t dwGold = 1000;
            const int iSplitCount = Random::get(10, 13);

            SPDLOG_INFO("WAEGU DropGoldOnHit %d times", GetMaxSP());

            for (int i = 1; i <= iSplitCount; ++i)
            {
                PIXEL_POSITION pos;
                CItem *item;

                if ((item = ITEM_MANAGER::instance().CreateItem(1, dwGold / iSplitCount)))
                {
                    if (i != 0)
                    {
                        pos.x = (Random::get(-14, 14) + Random::get(-14, 14)) * 20;
                        pos.y = (Random::get(-14, 14) + Random::get(-14, 14)) * 20;

                        pos.x += GetX();
                        pos.y += GetY();
                    }

                    item->AddToGround(GetMapIndex(), pos);
                    item->StartDestroyEvent(gConfig.itemGroundTime);
                }
            }
        }
    }
}

void CHARACTER::AfterDamage(CHARACTER *pAttacker, int dam, EDamageType type)
{
    // PROF_UNIT puRest2("Rest2");
    if (pAttacker && dam > 0 && IsNPC())
    {
        // PROF_UNIT puRest20("Rest20");
        auto it = m_map_kDamage.find(pAttacker->GetVID());

        if (it == m_map_kDamage.end())
        {
            m_map_kDamage.emplace(pAttacker->GetVID(), TBattleInfo(dam, 0));
            it = m_map_kDamage.find(pAttacker->GetVID());
        }
        else
        {
            it->second.iTotalDamage += dam;
        }

#ifdef __FAKE_PC__
        if (pAttacker->FakePC_IsSupporter())
        {
            const auto *pOwner = pAttacker->FakePC_GetOwner();

            it = m_map_kDamage.find(pOwner->GetVID());

            if (it == m_map_kDamage.end())
            {
                m_map_kDamage.insert(TDamageMap::value_type(pOwner->GetVID(), TBattleInfo(0, dam, 0)));
                it = m_map_kDamage.find(pOwner->GetVID());
            }
            else
            {
                it->second.iTotalFakePCDamage += dam;
            }
        }
#endif
        // puRest20.Pop();

        // PROF_UNIT puRest21("Rest21");
        StartRecoveryEvent(); // ¸ó½ºÅÍ´Â µ¥¹ÌÁö¸¦ ÀÔÀ¸¸é È¸º¹À» ½ÃÀÛÇÑ´Ù.
        // puRest21.Pop();

        // PROF_UNIT puRest22("Rest22");
        UpdateAggrPointEx(pAttacker, type, dam, it->second);
        // puRest22.Pop();
    }
    // puRest2.Pop();

    // PROF_UNIT puRest3("Rest3");
    if (GetHP() <= 0)
    {
        if (pAttacker && !pAttacker->IsNPC())
            m_dwKillerPID = pAttacker->GetPlayerID();
        else
            m_dwKillerPID = 0;

        Dead(pAttacker);
    }
}

//
// CHARACTER::Damage ¸Þ¼Òµå´Â this°¡ µ¥¹ÌÁö¸¦ ÀÔ°Ô ÇÑ´Ù.
//
// Arguments
//    pAttacker		: °ø°ÝÀÚ
//    dam		: µ¥¹ÌÁö
//    EDamageType	: ¾î¶² Çü½ÄÀÇ °ø°ÝÀÎ°¡?
//
// Return value
//    true		: dead
//    false		: not dead yet
//
bool CHARACTER::Damage(CHARACTER *pAttacker, DamageValue dam, EDamageType type /*= DAMAGE_TYPE_NORMAL*/,
                       uint32_t skillVnum /*= 0*/)
{

    if (IsNPC() && pAttacker && (pAttacker->IsPC() && !pAttacker->GetDesc()->GetDropsAllowed())) {
        SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
        return false;
    }
    //@Adalet eğer saldıranda hava kılıcı veya büyülü silah varsa ve silah yoksa STR etkisi kalmaması ve damage atmaya çalışmaması için affecti siler.
    if (pAttacker && this) {

        if (pAttacker->GetPoint(SKILL_GWIGEOM) &&
            !pAttacker->GetWear(WEAR_WEAPON)) {
            pAttacker->RemoveAffect(SKILL_GWIGEOM);
            return false;
        }

        if (pAttacker->GetPoint(SKILL_GEOMKYUNG) &&
            !pAttacker->GetWear(WEAR_WEAPON)) {
            pAttacker->RemoveAffect(SKILL_GEOMKYUNG);
            return false;
        }
    }

    if (DAMAGE_TYPE_MAGIC == type)
    {
        dam = dam *
                  (100.0 + (pAttacker->GetPoint(POINT_MAGIC_ATT_BONUS_PER) +
                            pAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) /
                  100.0 +
              0.5;
    }

    PirateTanaka_Damage();

    // ÆòÅ¸°¡ ¾Æ´Ò ¶§´Â °øÆ÷ Ã³¸®
    if (type != DAMAGE_TYPE_NORMAL && type != DAMAGE_TYPE_NORMAL_RANGE)
    {
        if (FindAffect(SKILL_TERROR))
        {
            auto pct = GetSkillPower(SKILL_TERROR) / 400;

            if (Random::get(1, 100) <= pct)
                return false;
        }
    }

    auto iCurHP = GetHP();
    auto iCurSP = GetSP();

    bool IsCritical = false;
    bool IsPenetrate = false;
    bool IsDeathBlow = false;

    uint32_t damageEventsFlag = 0;

#ifdef ENABLE_NEMERE_DUNGEON_EXTRAS
    if (!IsPC() && pAttacker)
    {
        if ((IsMonster() || IsNPC() || IsStone()) && pAttacker->IsPC())
        {
            if (GetRaceNum() == 6151)
            {
                quest::CQuestManager &q = quest::CQuestManager::instance();
                QuestDungeon *pDungeon = q.GetCurrentDungeon();
                if (pDungeon)
                {
                    uint32_t dwDungeonLevel = pDungeon->GetFlag("level");

                    if (dwDungeonLevel == 4)
                    {
                        if (!pAttacker->FindAffect(SKILL_HOSIN))
                        {
                            SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                            return false;
                        }
                    }
                    else if (dwDungeonLevel == 7)
                    {
                        if (!pAttacker->FindAffect(SKILL_GICHEON))
                        {
                            SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                            return false;
                        }
                    }
                }
            }
            if (GetRaceNum() == 8058)
            {
                if (pAttacker->GetJob() != JOB_SHAMAN)
                {
                    SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                    return false;
                }
            }
            if (GetRaceNum() == 20399)
            {
                if (pAttacker->GetJob() == JOB_WARRIOR || pAttacker->GetJob() == JOB_SURA ||
                    pAttacker->GetJob() == JOB_WOLFMAN)
                {
                    SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                    return false;
                }
            }
        }
    }
#endif

    //
    // ¸¶¹ýÇü ½ºÅ³°ú, ·¹ÀÎÁöÇü ½ºÅ³Àº(±ÃÀÚ°´) Å©¸®Æ¼ÄÃ°ú, °üÅë°ø°Ý °è»êÀ» ÇÑ´Ù.
    // ¿ø·¡´Â ÇÏÁö ¾Ê¾Æ¾ß ÇÏ´Âµ¥ Nerf(´Ù¿î¹ë·±½º)ÆÐÄ¡¸¦ ÇÒ ¼ö ¾ø¾î¼­ Å©¸®Æ¼ÄÃ°ú
    // °üÅë°ø°ÝÀÇ ¿ø·¡ °ªÀ» ¾²Áö ¾Ê°í, /2 ÀÌ»óÇÏ¿© Àû¿ëÇÑ´Ù.
    //
    // ¹«»ç ÀÌ¾ß±â°¡ ¸¹¾Æ¼­ ¹Ð¸® ½ºÅ³µµ Ãß°¡
    //
    // 20091109 : ¹«»ç°¡ °á°úÀûÀ¸·Î ¾öÃ»³ª°Ô °­ÇØÁø °ÍÀ¸·Î °á·Ð³², µ¶ÀÏ ±âÁØ ¹«»ç ºñÀ² 70% À°¹Ú
    //

    bool damage_debug = gConfig.damageDebug;
    if (type == DAMAGE_TYPE_MELEE || type == DAMAGE_TYPE_RANGE || type == DAMAGE_TYPE_MAGIC)
    {
        if (pAttacker)
        {
            // Å©¸®Æ¼ÄÃ
            auto iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

            if (!IsPC())
                iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

            if (IsEquipUniqueItem(UNIQUE_CHRISTMAS_LOLLIPOP))
                iCriticalPct += 5.0;

            if (iCriticalPct > 0.0)
            {
                if (iCriticalPct >= 10.0) // 10º¸´Ù Å©¸é 5% + (4¸¶´Ù 1%¾¿ Áõ°¡), µû¶ó¼­ ¼öÄ¡°¡ 50ÀÌ¸é 20%
                    iCriticalPct = 5.0 + (iCriticalPct - 10.0) / 2.0;
                else // 10º¸´Ù ÀÛÀ¸¸é ´Ü¼øÈ÷ ¹ÝÀ¸·Î ±ðÀ½, 10 = 5%
                    iCriticalPct /= 2.0;

                //Å©¸®Æ¼ÄÃ ÀúÇ× °ª Àû¿ë.
                iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

                if (Random::get(1, 100) <= iCriticalPct)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::CRITICAL);

                    IsCritical = true;
                    dam *= 2;

                    // Think - NEW BONUS: Critical Boost
                    if (pAttacker->GetPoint(POINT_BOOST_CRITICAL) > 0.0)
                    {
                        int bonusDam = dam * pAttacker->GetPoint(POINT_BOOST_CRITICAL) / 100.0;
                        if (damage_debug)
                            SPDLOG_INFO("[TEST_SERVER] Critical boost! Base: %d, New: %d", dam, dam + bonusDam);

                        dam += bonusDam;
                    }

                    EffectPacket(SE_CRITICAL);

                    // We dont want critical hits to remove the reflect buff
                    // if (FindAffect(SKILL_REFLECT))
                    //	RemoveAffect(SKILL_REFLECT);
                }
            }

            // °üÅë°ø°Ý
            auto iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

            if (!IsPC())
                iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);

            if (IsEquipUniqueItem(UNIQUE_CHRISTMAS_LOLLIPOP))
                iPenetratePct += 5;

            if (iPenetratePct > 0.0)
            {
                {
                    CSkillProto *pkSk = CSkillManager::instance().Get(SKILL_RESIST_PENETRATE);

                    if (nullptr != pkSk)
                    {
                        pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

                        iPenetratePct -= static_cast<int>(pkSk->kPointPoly.Eval());
                    }
                }

                if (iPenetratePct >= 10.0)
                {
                    // If greater than 10, 5% + (increase by 1% every 4), so if the number is 50, 20%
                    iPenetratePct = 5.0 + (iPenetratePct - 10.0) / 2.0;
                }
                else
                {
                    // do nothing
                }

                //°üÅëÅ¸°Ý ÀúÇ× °ª Àû¿ë.
                iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

                if (Random::get(1, 100) <= iPenetratePct)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::PENETRATE);
                    IsPenetrate = true;

                    if (damage_debug)
                        SendI18nChatPacket(this, CHAT_TYPE_INFO, "관통 추가 데미지 %d",
                                           GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);

                    dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;

                    // Think - NEW BONUS: Pierce Boost
                    if (pAttacker->GetPoint(POINT_BOOST_PENETRATE))
                    {
                        int bonusDam = dam * pAttacker->GetPoint(POINT_BOOST_PENETRATE) / 100;
                        if (damage_debug)
                            SPDLOG_INFO("[TEST_SERVER] Pierce boost! Base: %d, New: %d", dam, dam + bonusDam);

                        dam += bonusDam;
                    }

                    // We dont want critical hits to remove the reflect buff
                    // if (FindAffect(SKILL_REFLECT))
                    //	RemoveAffect(SKILL_REFLECT);
                }
            }
        }
    }
    //
    // ÄÞº¸ °ø°Ý, È° °ø°Ý, Áï ÆòÅ¸ ÀÏ ¶§¸¸ ¼Ó¼º°ªµéÀ» °è»êÀ» ÇÑ´Ù.
    //
    else if (type == DAMAGE_TYPE_NORMAL || type == DAMAGE_TYPE_NORMAL_RANGE)
    {
        if (type == DAMAGE_TYPE_NORMAL)
        {
            // ±ÙÁ¢ ÆòÅ¸ÀÏ °æ¿ì ¸·À» ¼ö ÀÖÀ½
            if (GetPoint(POINT_BLOCK) && Random::get(1, 100) <= GetPoint(POINT_BLOCK))
            {
                if (damage_debug)
                {
                    SendI18nChatPacket(pAttacker, CHAT_TYPE_INFO, "%s 블럭! (%d%%)", GetName(), GetPoint(POINT_BLOCK));
                    SendI18nChatPacket(this, CHAT_TYPE_INFO, "%s 블럭! (%d%%)", GetName(), GetPoint(POINT_BLOCK));
                }

                SET_BIT(damageEventsFlag, DamageEvents::BLOCK);
                SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                return false;
            }
        }
        else if (type == DAMAGE_TYPE_NORMAL_RANGE)
        {
            // ¿ø°Å¸® ÆòÅ¸ÀÇ °æ¿ì ÇÇÇÒ ¼ö ÀÖÀ½
            if (GetPoint(POINT_DODGE) && Random::get(1, 100) <= GetPoint(POINT_DODGE))
            {
                if (damage_debug)
                {
                    SendI18nChatPacket(pAttacker, CHAT_TYPE_INFO, "%s 회피! (%d%%)", GetName(), GetPoint(POINT_DODGE));
                    SendI18nChatPacket(this, CHAT_TYPE_INFO, "%s 회피! (%d%%)", GetName(), GetPoint(POINT_DODGE));
                }

                SET_BIT(damageEventsFlag, DamageEvents::BLOCK);
                SendDamagePacket(pAttacker, 0, DAMAGE_DODGE);
                return false;
            }
        }

        /*if (FindAffect(SKILL_JEONGWI)) {
            SET_BIT(damageEventsFlag, DamageEvents::BERSEK);
            dam = (int)(dam * (100 + GetSkillPower(SKILL_JEONGWI) * 25 / 100) / 100);
        }*/

        if (FindAffect(SKILL_TERROR))
        {
            SET_BIT(damageEventsFlag, DamageEvents::FEAR);
            auto damBefore = dam;
            dam = (dam * (95 - GetSkillPower(SKILL_TERROR) / 5) / 100);
            if (gConfig.testServer)
            {
                if (pAttacker)
                {
                    pAttacker->ChatPacket(
                        CHAT_TYPE_INFO,
                        fmt::format("Enemy has TERROR Active Original damage {} After Terror {}", damBefore, dam)
                            .c_str());
                }
                ChatPacket(
                    CHAT_TYPE_INFO,
                    fmt::format("I have TERROR Active Original damage {} After Terror {}", damBefore, dam).c_str());
            }
        }

        if (FindAffect(SKILL_HOSIN))
        {
            SET_BIT(damageEventsFlag, DamageEvents::BLESSING);
            dam = dam * (100 - GetPoint(POINT_RESIST_NORMAL_DAMAGE)) / 100;
        }

        //
        // °ø°ÝÀÚ ¼Ó¼º Àû¿ë
        //
        if (pAttacker)
        {
            if (type == DAMAGE_TYPE_NORMAL)
            {
                // Yansıtma bütün herşey için kontrolsüz çalışması için düzeltildi. @Adalet
                // Eski hali : if (GetPoint(POINT_REFLECT_MELEE) && pAttacker->IsMonster())
                if (GetPoint(POINT_REFLECT_MELEE))
                {
                    uint64_t reflectDamage = llround(dam * GetPoint(POINT_REFLECT_MELEE) / 100.0f);

                    // NOTE: °ø°ÝÀÚ°¡ IMMUNE_REFLECT ¼Ó¼ºÀ» °®°íÀÖ´Ù¸é ¹Ý»ç¸¦ ¾È ÇÏ´Â °Ô
                    // ¾Æ´Ï¶ó 1/3 µ¥¹ÌÁö·Î °íÁ¤ÇØ¼­ µé¾î°¡µµ·Ï ±âÈ¹¿¡¼­ ¿äÃ».
                    if (pAttacker->IsImmune(IMMUNE_REFLECT))
                        reflectDamage = llround(reflectDamage / 3.0f + 0.5f);

                    pAttacker->Damage(this, reflectDamage, DAMAGE_TYPE_SPECIAL);

                    SET_BIT(damageEventsFlag, DamageEvents::REFLECT);
                }

                if (IsMonster() && IS_SET(GetMobTable().dwAIFlag, AIFLAG_REFLECT))
                {
                    uint64_t reflectDamage = llround(dam * 20.0f / 100.0f);

                    // NOTE: °ø°ÝÀÚ°¡ IMMUNE_REFLECT ¼Ó¼ºÀ» °®°íÀÖ´Ù¸é ¹Ý»ç¸¦ ¾È ÇÏ´Â °Ô
                    // ¾Æ´Ï¶ó 1/3 µ¥¹ÌÁö·Î °íÁ¤ÇØ¼­ µé¾î°¡µµ·Ï ±âÈ¹¿¡¼­ ¿äÃ».
                    if (pAttacker->IsImmune(IMMUNE_REFLECT))
                        reflectDamage = llround(reflectDamage / 3.0f + 0.5f);

                    pAttacker->Damage(this, reflectDamage, DAMAGE_TYPE_SPECIAL);

                    SET_BIT(damageEventsFlag, DamageEvents::REFLECT);
                }
            }

            // Å©¸®Æ¼ÄÃ
            auto iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

            if (!IsPC())
                iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

            if (iCriticalPct > 0.0)
            {
                //Å©¸®Æ¼ÄÃ ÀúÇ× °ª Àû¿ë.
                iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

                if (Random::get(1.0f, 100.0f) <= iCriticalPct)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::CRITICAL);

                    IsCritical = true;
                    dam *= 2.0f;

                    // Think - NEW BONUS: Critical Boost
                    if (pAttacker->GetPoint(POINT_BOOST_CRITICAL) > 0.0f)
                    {
                        auto bonusDam = dam * pAttacker->GetPoint(POINT_BOOST_CRITICAL) / 100.0f;
                        if (damage_debug)
                            SPDLOG_INFO("[TEST_SERVER] Critical boost! Base: %d, New: %d", dam, dam + bonusDam);

                        dam += bonusDam;
                    }

                    EffectPacket(SE_CRITICAL);
                }
            }

            // °üÅë°ø°Ý
            auto iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

            if (!IsPC())
                iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);

            {
                CSkillProto *pkSk = CSkillManager::instance().Get(SKILL_RESIST_PENETRATE);

                if (nullptr != pkSk)
                {
                    pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

                    iPenetratePct -= static_cast<int>(pkSk->kPointPoly.Eval());
                }
            }

            if (iPenetratePct > 0.0)
            {
                //°üÅëÅ¸°Ý ÀúÇ× °ª Àû¿ë.
                iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

                if (Random::get(1, 100) <= iPenetratePct)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::PENETRATE);
                    IsPenetrate = true;


                    dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;

                    // Think - NEW BONUS: Pierce Boost
                    if (pAttacker->GetPoint(POINT_BOOST_PENETRATE))
                    {
                        int bonusDam = dam * pAttacker->GetPoint(POINT_BOOST_PENETRATE) / 100;

                        dam += bonusDam;
                    }
                }
            }


             if (IsPC() && pAttacker->GetPoint(POINT_STEAL_HP) && Random::get(0, 4) > 0)
            {
                PointValue i =
                    std::min(dam, std::max<int64_t>(0, iCurHP)) * pAttacker->GetPoint(POINT_STEAL_HP) / 100;

                if (i)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::HP_RECOVER);
                    CreateFly(FLY_HP_SMALL, pAttacker);
                    pAttacker->PointChange(POINT_HP, i);
                }
            }

            if (!IsPC() && pAttacker->GetPoint(POINT_STEAL_SP))
            {
                auto pct = 1.f;

                if (Random::get(1, 10) <= pct)
                {
                    PointValue iCur;

                    if (IsPC())
                        iCur = iCurSP;
                    else
                        iCur = iCurHP;

                    PointValue iSP =
                        std::min(dam, std::max<int64_t>(0, iCur)) * pAttacker->GetPoint(POINT_STEAL_SP) / 100;

                    if (iSP > 0 && iCur >= iSP)
                    {
                        SET_BIT(damageEventsFlag, DamageEvents::STEAL_SP);
                        CreateFly(FLY_SP_SMALL, pAttacker);
                        pAttacker->PointChange(POINT_SP, iSP);

                        if (IsPC())
                            PointChange(POINT_SP, -iSP);
                    }
                }
            }

            // µ· ½ºÆ¿
            if (pAttacker->GetPoint(POINT_STEAL_GOLD))
            {
                if (Random::get(1, 100) <= pAttacker->GetPoint(POINT_STEAL_GOLD))
                {
                    Gold iAmount = Random::get<Gold>(1, GetLevel());
                    pAttacker->ChangeGold(iAmount);
                    // DBManager::instance().SendMoneyLog(MONEY_LOG_MISC, 1, iAmount);
                }
            }

            /**
             *
             */
            //if (!IsPC() && pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) && Random::get(0, 4) > 0) @Adalet correct eskiden sadece canavar koşuluydu.
            if (IsPC() && pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) && Random::get(0, 4) > 0)
            {
                PointValue i =
                    std::min(dam, std::max<int64_t>(0, iCurHP)) * pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) / 100;

                if (i)
                {
                    SET_BIT(damageEventsFlag, DamageEvents::HP_RECOVER);
                    CreateFly(FLY_HP_SMALL, pAttacker);
                    pAttacker->PointChange(POINT_HP, i);
                }
            }

            // Ä¥ ¶§¸¶´Ù SPÈ¸º¹
            if (!IsPC() && pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) && Random::get(0.0f, 4.0f) > 0.0f) // 80% È®·ü
            {
                PointValue i = std::min<PointValue>(dam, std::max<int64_t>(0, iCurHP)) *
                               pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) / 100;

                if (i)
                {
                    CreateFly(FLY_SP_SMALL, pAttacker);
                    pAttacker->PointChange(POINT_SP, i);
                }
            }

            // »ó´ë¹æÀÇ ¸¶³ª¸¦ ¾ø¾Ø´Ù.
            if (pAttacker->GetPoint(POINT_MANA_BURN_PCT))
            {
                if (Random::get(1, 100) <= pAttacker->GetPoint(POINT_MANA_BURN_PCT))
                {
                    SET_BIT(damageEventsFlag, DamageEvents::SP_BURN);
                    PointChange(POINT_SP, -50);
                }
            }
        }
    }

    //
    // ÆòÅ¸ ¶Ç´Â ½ºÅ³·Î ÀÎÇÑ º¸³Ê½º ÇÇÇØ/¹æ¾î °è»ê
    //
    switch (type)
    {
    case DAMAGE_TYPE_NORMAL:
    case DAMAGE_TYPE_NORMAL_RANGE:
        if (pAttacker)
            if (pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS))
                dam = dam * (100.0f + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 85.0f;

        dam = dam * (100.0f - std::min<PointValue>(99.0f, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100.0f;
        break;

    case DAMAGE_TYPE_MELEE:
    case DAMAGE_TYPE_RANGE:
    case DAMAGE_TYPE_FIRE:
    case DAMAGE_TYPE_ICE:
    case DAMAGE_TYPE_ELEC:
    case DAMAGE_TYPE_MAGIC:
    case DAMAGE_TYPE_EARTH:
        //@Adalet Beceri efsunu hasarı düzenlemesi 21 altıysa 2ye böler.
        if (pAttacker)
            if (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS))
            {


                if (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS) < 24.0f) {
                        dam = dam *
                              (100 +
                               (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS)) / 2) /
                              100.0f;
                } else {
                        dam = dam *
                              (100 +
                               pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS)) /
                              80.0f;
                }

            }

        dam = dam * (100 - std::min<PointValue>(99LL, GetPoint(POINT_SKILL_DEFEND_BONUS))) / 100.0f;
        break;

    default:
        break;
    }

    //
    // ¸¶³ª½¯µå(Èæ½Å¼öÈ£)
    //
    if (FindAffect(SKILL_MANASHIELD))
    {
        dam -= dam * GetPoint(POINT_MANASHIELD) / 100;
    }

    //
    // ÀüÃ¼ ¹æ¾î·Â »ó½Â (¸ô ¾ÆÀÌÅÛ)
    //
    if (GetPoint(POINT_MALL_DEFBONUS) > 0)
    {
        DamageValue dec_dam = std::min<DamageValue>(200, dam * GetPoint(POINT_MALL_DEFBONUS) / 100);
        dam -= dec_dam;
    }

    if (pAttacker)
    {
        //
        // ÀüÃ¼ °ø°Ý·Â »ó½Â (¸ô ¾ÆÀÌÅÛ)
        //
        if (pAttacker->GetPoint(POINT_MALL_ATTBONUS) > 0)
        {
            DamageValue add_dam = std::min<DamageValue>(300, dam * pAttacker->GetPoint(POINT_MALL_ATTBONUS) / 100);
            dam += add_dam;
        }

        //
        // Á¦±¹À¸·Î ÀÎÇÑ º¸³Ê½º (ÇÑ±¹ ¿Ãµå ¹öÀü¸¸ Àû¿ë)
        //

        if (pAttacker->IsPC())
        {
            if (!IsPC() && GetMonsterDrainSPPoint())
            {
                int iDrain = GetMonsterDrainSPPoint();

                if (iDrain <= pAttacker->GetSP())
                    pAttacker->PointChange(POINT_SP, -iDrain);
                else
                {
                    int iSP = pAttacker->GetSP();
                    pAttacker->PointChange(POINT_SP, -iSP);
                }
            }
        }
        else if (pAttacker->IsGuardNPC())
        {
            SetNoRewardFlag();
            Stun();
            return true;
        }
    }
    // puAttr.Pop();

    if (!GetSectree() || GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
        return false;

    if (!IsPC())
    {
        if (m_pkParty && m_pkParty->GetLeader())
            m_pkParty->GetLeader()->SetLastAttacked(get_dword_time());
        else
            SetLastAttacked(get_dword_time());

        // ¸ó½ºÅÍ ´ë»ç : ¸ÂÀ» ¶§
        MonsterChat(MONSTER_CHAT_ATTACKED);
    }

    if (IsStun())
    {
        Dead(pAttacker);
        return true;
    }

    if (IsDead())
        return true;

    // ------------------------
    // µ¶ÀÏ ÇÁ¸®¹Ì¾ö ¸ðµå
    // -----------------------
    if (pAttacker && pAttacker->IsPC())
    {
        auto iDmgPct = g_pCharManager->GetUserDamageRate(pAttacker);
        dam = dam * iDmgPct / 100;
    }

    // STONE SKIN : ÇÇÇØ ¹ÝÀ¸·Î °¨¼Ò
    if (IsMonster() && IsStoneSkinner())
    {
        if (GetHPPct() < GetMobTable().bStoneSkinPoint)
            dam /= 2;
    }

    // PROF_UNIT puRest1("Rest1");
    if (pAttacker)
    {
        if (pAttacker->IsMonster() && pAttacker->IsDeathBlower())
        {
            if (pAttacker->IsDeathBlow())
            {
                auto rate = 2 * std::clamp(51 - pAttacker->GetHPPct(), 1, 50);
                if (GetPoint(POINT_BLOCK) > 0 && Random::get(1, 100) <= GetPoint(POINT_BLOCK))
                {
                    rate -= std::clamp<PointValue>(GetPoint(POINT_BLOCK), 1, 75);
                }

                IsDeathBlow = true;
                dam *= static_cast<int>(1.5 + 1.5 * rate / 100.0);
            }
        }

        if (pAttacker->IsMonster() && 2493 == pAttacker->GetMobTable().dwVnum)
            dam = BlueDragon_Damage(this, pAttacker, dam);

        uint32_t damageFlag = 0;

        if (type == DAMAGE_TYPE_POISON)
            damageFlag = DAMAGE_POISON;
        else
            damageFlag = DAMAGE_NORMAL;

        if (IsCritical)
            damageFlag |= DAMAGE_CRITICAL;

        if (IsPenetrate)
            damageFlag |= DAMAGE_PENETRATE;

        //ÃÖÁ¾ µ¥¹ÌÁö º¸Á¤
        dam = dam * GetDamMul() + 0.5f;

#ifdef ENABLE_BATTLE_PASS
        if (dam > 0)
        {
            uint8_t bBattlePassId = pAttacker->GetBattlePassId();
            if (bBattlePassId)
            {

                    if (IsPC())
                    {
                        uint32_t dwMinLevel, dwDamage;
                        if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, PLAYER_DAMAGE, &dwMinLevel,
                                                                             &dwDamage))
                        {
                            uint32_t dwCurrentDamage = pAttacker->GetMissionProgress(PLAYER_DAMAGE, bBattlePassId);
                            if (GetLevel() >= dwMinLevel && dwCurrentDamage < dwDamage && dwCurrentDamage < dam)
                                pAttacker->UpdateMissionProgress(PLAYER_DAMAGE, bBattlePassId, dam, dwDamage, true);
                        }
                    }
                    else
                    {
                        uint32_t dwMonsterVnum, dwDamage;
                        if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, MONSTER_DAMAGE,
                                                                             &dwMonsterVnum, &dwDamage))
                        {
                            uint32_t dwCurrentDamage = pAttacker->GetMissionProgress(MONSTER_DAMAGE, bBattlePassId);
                            if ((dwMonsterVnum == GetRaceNum() || dwMonsterVnum == 0) && dwCurrentDamage < dwDamage && dwCurrentDamage < dam)
                                pAttacker->UpdateMissionProgress(MONSTER_DAMAGE, bBattlePassId, dam, dwDamage, true);
                        }
                    }
                
            }
        }
#endif

        if(pAttacker->IsPC() && pAttacker->GetDungeon())
		{
			if(pAttacker->GetDungeon()->IsBossVnum(this->GetRaceNum()))
			{
				if(dam > 0 && dam > pAttacker->GetDungeon()->GetMaxDmgOnBoss(this->GetRaceNum()))
				{
					pAttacker->GetDungeon()->SetMaxDmgOnBoss(this->GetRaceNum(), dam);
				}
			}
		}

        if (pAttacker && GetRaceNum() != MeleyLair::STATUE_VNUM)
            SendDamagePacket(pAttacker, dam, damageFlag);

        if (damage_debug)
        {
            if (pAttacker && damage_debug)
            {
                pAttacker->ChatPacket(CHAT_TYPE_INFO, "-> %s, DAM %d HP %d(%d%%) %s%s%s", GetName(), dam, GetHP(),
                                      (GetHP() * 100) / GetMaxHP(), IsCritical ? "crit " : "",
                                      IsPenetrate ? "pene " : "", IsDeathBlow ? "deathblow " : "");
            }

            ChatPacket(CHAT_TYPE_PARTY, "<- %s, DAM %d HP %d(%d%%) %s%s%s", pAttacker ? pAttacker->GetName() : nullptr,
                       dam, GetHP(), (GetHP() * 100) / GetMaxHP(), IsCritical ? "crit " : "",
                       IsPenetrate ? "pene " : "", IsDeathBlow ? "deathblow " : "");
        }

        if (m_bDetailLog && damage_debug)
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "%s[%d]가 공격 위치: %d %d", pAttacker->GetName(),
                               static_cast<uint32_t>(pAttacker->GetVID()), pAttacker->GetX(), pAttacker->GetY());
        }
    }

    //
    // !!!!!!!!! ½ÇÁ¦ HP¸¦ ÁÙÀÌ´Â ºÎºÐ !!!!!!!!!
    //
    if (!m_invincible)
    {
        if (IsMonster() && IS_SET(GetMobTable().dwAIFlag, AIFLAG_COUNT))
        {
            dam = 1;
        }

        if (type == DAMAGE_TYPE_POISON)
        {
            if (GetHP() - dam <= 0)
            {
                dam = GetHP() - 1;
            }
        }

        const auto now = get_dword_time();

#ifdef ENABLE_MELEY_LAIR_DUNGEON
        if (pAttacker)
        {
            if (GetRaceNum() == MeleyLair::STATUE_VNUM && MeleyLair::CMgr::IsMeleyMap(pAttacker->GetMapIndex()))
            {
                return MeleyLair::CMgr::instance().Damage(this, pAttacker, dam);
            }

            if (GetRaceNum() == MeleyLair::BOSS_VNUM && MeleyLair::CMgr::IsMeleyMap(pAttacker->GetMapIndex()))
            {
                SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
                return false;
            }
        }
#endif

#ifdef ENABLE_HYDRA_DUNGEON
        if (GetRaceNum() == HYDRA_BOSS_VNUM || GetRaceNum() == MAST_VNUM)
            CHydraDungeonManager::instance().NotifyHydraDmg(this, &dam);
#endif
        if (pAttacker)
        {
            CWarMap *pWarMap = pAttacker->GetWarMap();

            if (pWarMap && pWarMap == GetWarMap())
            {
                CWarMap::TMemberStats *pStats = pWarMap->GetMemberStats(pAttacker);
                pStats->ullDamage += dam;
                pWarMap->SendStats(pStats);
            }
        }

        if (GetHP() - dam <= 0)
        {
            dam = GetHP();
            PointChange(POINT_HP, -dam, false);
            MarkHitReceived(get_dword_time());

            if (pAttacker && pAttacker->IsPC())
            {
                MarkPlayerHitReceived(get_dword_time());
                if(IsPC())
                    pAttacker->MarkPlayerHit(get_dword_time());
            }

            if (pAttacker && !IsPC())
            {
                AfterDamage(pAttacker, dam, type);

                Dead(pAttacker);
                return false;
            }
        }
        else
        {
            PointChange(POINT_HP, -dam, false);
            if (pAttacker && pAttacker->IsPC())
            {
                MarkPlayerHitReceived(get_dword_time());
                if(IsPC())
                    pAttacker->MarkPlayerHit(get_dword_time());
            }
            MarkHitReceived(get_dword_time());
        }
    }

    // puRest1.Pop();
    AfterDamage(pAttacker, dam, type);

    return false;
}

void CHARACTER::DistributeHP(CHARACTER *pkKiller)
{
    if (pkKiller->GetDungeon()) // ´øÁ¯³»¿¡¼± ¸¸µÎ°¡³ª¿ÀÁö¾Ê´Â´Ù
        return;
}

static void GiveExp(CHARACTER *from, CHARACTER *to, PointValue iExp)
{
    if (!to || to->HasInstantFlag(INSTANT_FLAG_REFUSE_EXP) || to->IsEquipUniqueGroup(UNIQUE_GROUP_ANTI_EXP))
        return;

    const int expBlock = quest::CQuestManager::instance().GetEventFlag(fmt::format("exp_block_{}", from->GetRaceNum()));
    if (expBlock && to->GetLevel() < from->GetLevel())
        return;

    if (CMobManager::instance().MobHasExpLevelLimit(from->GetRaceNum()))
    {
        const auto &limit = CMobManager::instance().GetMobExpLevelLimit(from->GetRaceNum());
        if (to->GetLevel() < limit.first || to->GetLevel() > limit.second)
            return;
    }

    const int expTest = quest::CQuestManager::instance().GetEventFlag("exp_test");
    // ·¹º§Â÷ °æÇèÄ¡ °¡°¨ºñÀ²

    iExp = CALCULATE_VALUE_LVDELTA(to->GetLevel(), from->GetLevel(), iExp);

    // ¿ÜºÎ Å×½ºÆ® ¼­¹ö °æÇèÄ¡ 3¹è º¸³Ê½º

    const auto iBaseExp = iExp;

    // Á¡¼ú, È¸»ç °æÇèÄ¡ ÀÌº¥Æ® Àû¿ë
    iExp = iExp * (100 + CPrivManager::instance().GetPriv(to, PRIV_EXP_PCT)) / 100;

    // °ÔÀÓ³» ±âº» Á¦°øµÇ´Â °æÇèÄ¡ º¸³Ê½º
    {
        // Bonus exp in DT
        if (to->GetMapIndex() >= 660000 && to->GetMapIndex() < 670000)
            iExp += iExp * 20 / 100;

        // Bonus: Chance to get exp bonus
        if (to->GetPoint(POINT_EXP_DOUBLE_BONUS) > 0.0f)
        {
            if (Random::get(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
            {
                iExp += iExp * 30 / 100; // 1.3¹è (30%)
            }
        }

        // Experience ring 50%
        if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP))
        {
            iExp += iExp * 50 / 100;
        }

    }

    // ¾ÆÀÌÅÛ ¸ô: °æÇèÄ¡ °áÁ¦
    if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
    {
        iExp += (iExp * 50 / 100);
    }

    if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP))
    {
        iExp += (iExp * 50 / 100);
    }

    /*if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP2) == true)
    {
        iExp += (iExp * 50 / 100);
    }

    if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_ANTI_EXP) == true)
    {
        iExp = 0;
    }

    if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_DOUBLE_EXP) == true)
    {
        iExp = iExp * 2;
    }
*/

    iExp += (iExp * to->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS) / 100);
    iExp += (iExp * to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP) / 100);
    iExp += (iExp * to->GetPoint(POINT_MALL_EXPBONUS) / 100);
    iExp += (iExp * to->GetPoint(POINT_EXP) / 100);

    if (gConfig.testServer)
    {
        SPDLOG_INFO("Bonus Exp : Ramadan Candy: %d MallExp: %d PointExp: %d",
                    to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP), to->GetPoint(POINT_MALL_EXPBONUS),
                    to->GetPoint(POINT_EXP));
    }

    // ±âÈ¹Ãø Á¶Á¤°ª 2005.04.21 ÇöÀç 85%
    iExp = iExp * g_pCharManager->GetMobExpRate(to) / 100;

    // °æÇèÄ¡ ÇÑ¹ø È¹µæ·® Á¦ÇÑ
    iExp = std::min<PointValue>(to->GetNextExp() / 10, iExp);

    iExp = AdjustExpByLevel(to, iExp);

    iExp = std::clamp<PointValue>(iExp, 0, std::numeric_limits<PointValue>::max());
    to->PointChange(POINT_EXP, iExp, true);

    from->CreateFly(FLY_EXP, to);
    // Love points update
    {
        CHARACTER *you = to->GetMarryPartner();
        // ºÎºÎ°¡ ¼­·Î ÆÄÆ¼ÁßÀÌ¸é ±Ý½½ÀÌ ¿À¸¥´Ù
        if (you && to)
        {
            // 1¾ïÀÌ 100%
            uint32_t dwUpdatePoint = 2000 * iExp / to->GetLevel() / to->GetLevel() / 3;

            if (to->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0 ||
                you->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0)
                dwUpdatePoint = static_cast<uint32_t>(dwUpdatePoint * 3);

            marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(to->GetPlayerID());

            // DIVORCE_NULL_BUG_FIX
            if (pMarriage && pMarriage->IsNear())
                pMarriage->Update(dwUpdatePoint);
            // END_OF_DIVORCE_NULL_BUG_FIX
        }
    }
}

namespace NPartyExpDistribute
{
struct FPartyTotaler
{
    int total;
    int member_count;
    int x, y;
    int32_t map;

    FPartyTotaler(CHARACTER *center)
        : total(0), member_count(0), x(center->GetX()), y(center->GetY()), map(center->GetMapIndex()){};

    void operator()(CHARACTER *ch)
    {
        if (ch->GetMapIndex() == map && DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
        {
            total += party_exp_distribute_table[std::min<int>(ch->GetLevel(),
                                                              STORM_ARRAYSIZE(party_exp_distribute_table) - 1)];
            ++member_count;
        }
    }
};

struct FPartyDistributor
{
    int total;
    CHARACTER *c;
    int x, y;
    uint32_t _iExp;
    int m_iMode;
    int m_iMemberCount;
    int32_t map;

    FPartyDistributor(CHARACTER *center, int member_count, int total, uint32_t iExp, int iMode)
        : total(total), c(center), x(center->GetX()), y(center->GetY()), _iExp(iExp), m_iMode(iMode),
          m_iMemberCount(member_count), map(center->GetMapIndex())
    {
        if (m_iMemberCount == 0)
            m_iMemberCount = 1;
    };

    void operator()(CHARACTER *ch)
    {
        if (ch->GetMapIndex() == map && DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
        {
            uint32_t iExp2;

            switch (m_iMode)
            {
            case PARTY_EXP_DISTRIBUTION_NON_PARITY:
                iExp2 = _iExp;
                break;

            case PARTY_EXP_DISTRIBUTION_PARITY:
                iExp2 = _iExp / m_iMemberCount;
                break;

            default:
                SPDLOG_ERROR("Unknown party exp distribution mode {0}", m_iMode);
                return;
            }

            GiveExp(c, ch, iExp2);
        }
    }
};
} // namespace NPartyExpDistribute

typedef struct SDamageInfo
{
    int iDam;
    CHARACTER *pAttacker;
    CParty *pParty;

    void Clear()
    {
        pAttacker = nullptr;
        pParty = nullptr;
    }

    inline void Distribute(CHARACTER *ch, uint32_t iExp)
    {
        if (pAttacker)
            GiveExp(ch, pAttacker, iExp);
        else if (pParty)
        {
            NPartyExpDistribute::FPartyTotaler f(ch);
            pParty->ForEachOnlineMember(f);

            if (pParty->GetExpDistributionMode() != PARTY_EXP_DISTRIBUTION_NON_PARITY)
            {
                if (pParty->IsPositionNearLeader(ch))
                    iExp = iExp * (100 + pParty->GetExpBonusPercent()) / 100;

                if (gConfig.testServer)
                {
                    if (quest::CQuestManager::instance().GetEventFlag("exp_bonus_log") && pParty->GetExpBonusPercent())
                        pParty->ChatPacketToAllMember(CHAT_TYPE_INFO, "exp party bonus %d%%",
                                                      pParty->GetExpBonusPercent());
                }

                // °æÇèÄ¡ ¸ô¾ÆÁÖ±â (ÆÄÆ¼°¡ È¹µæÇÑ °æÇèÄ¡¸¦ 5% »©¼­ ¸ÕÀú ÁÜ)
                if (pParty->GetExpCentralizeCharacter())
                {
                    CHARACTER *tch = pParty->GetExpCentralizeCharacter();

                    if (DISTANCE_APPROX(ch->GetX() - tch->GetX(), ch->GetY() - tch->GetY()) <= PARTY_DEFAULT_RANGE &&
                        ch->GetMapIndex() == tch->GetMapIndex())
                    {
                        const uint32_t iExpCenteralize = static_cast<uint32_t>(iExp * 0.05f);
                        iExp -= iExpCenteralize;

                        GiveExp(ch, pParty->GetExpCentralizeCharacter(), iExpCenteralize);
                    }
                }
            }

            NPartyExpDistribute::FPartyDistributor fDist(ch, f.member_count, f.total, iExp,
                                                         pParty->GetExpDistributionMode());
            pParty->ForEachOnlineMember(fDist);
        }
    }
} TDamageInfo;

CHARACTER *CHARACTER::DistributeExp()
{
    auto iExpToDistribute = GetExp();

    uint32_t iTotalDam = 0;
    CHARACTER *pkChrMostAttacked = nullptr;
    uint32_t iMostDam = 0;

    typedef std::vector<TDamageInfo> TDamageInfoTable;
    TDamageInfoTable damage_info_table;
    std::map<CParty *, TDamageInfo> map_party_damage;

    damage_info_table.reserve(m_map_kDamage.size());

    TDamageMap::iterator it = m_map_kDamage.begin();

    while (it != m_map_kDamage.end())
    {
        const VID &c_VID = it->first;
        const uint32_t iDam = it->second.iTotalDamage;

        ++it;

        CHARACTER *pAttacker = g_pCharManager->Find(c_VID);

        if (!pAttacker || pAttacker->IsNPC() ||
            DISTANCE_APPROX(GetX() - pAttacker->GetX(), GetY() - pAttacker->GetY()) > 5000 ||
            GetMapIndex() != pAttacker->GetMapIndex())
            continue;

        iTotalDam += iDam;
        if (!pkChrMostAttacked || iDam > iMostDam)
        {
            pkChrMostAttacked = pAttacker;
            iMostDam = iDam;
        }

        if (pAttacker->GetParty())
        {
            auto itPd = map_party_damage.find(pAttacker->GetParty());
            if (itPd == map_party_damage.end())
            {
                TDamageInfo di;
                di.iDam = iDam;
                di.pAttacker = nullptr;
                di.pParty = pAttacker->GetParty();
                map_party_damage.insert(std::make_pair(di.pParty, di));
            }
            else
            {
                itPd->second.iDam += iDam;
            }
        }
        else
        {
            TDamageInfo di;
            di.iDam = iDam;
            di.pAttacker = pAttacker;
            di.pParty = nullptr;
            damage_info_table.push_back(di);
        }
    }

    for (auto itPd = map_party_damage.begin(); itPd != map_party_damage.end(); ++itPd)
    {
        damage_info_table.push_back(itPd->second);
    }

    SetExp(0);
    // m_map_kDamage.clear();

    if (iTotalDam == 0)
        return nullptr;

    if (m_pkChrStone)
    {
        const PointValue iExp = iExpToDistribute / 2;
        m_pkChrStone->SetExp(m_pkChrStone->GetExp() + iExp);
        iExpToDistribute -= iExp;
    }

    if (damage_info_table.empty())
        return nullptr;

    DistributeHP(pkChrMostAttacked);

    {
        auto diBegin = damage_info_table.begin();
        {
            for (auto itDi = damage_info_table.begin(); itDi != damage_info_table.end(); ++itDi)
            {
                if (itDi->iDam > diBegin->iDam)
                    diBegin = itDi;
            }
        }

        uint32_t iExp = iExpToDistribute / 5;
        iExpToDistribute -= iExp;

        float fPercent = static_cast<float>(diBegin->iDam) / iTotalDam;
        if (fPercent > 1.0f)
        {
            SPDLOG_ERROR("DistributeExp percent over 1.0 (fPercent {0} name {1})", fPercent,
                         diBegin->pAttacker->GetName());
            fPercent = 1.0f;
        }

        iExp += static_cast<uint32_t>(iExpToDistribute * fPercent);

        // SPDLOG_INFO( "{0} given exp percent {1} + 20 dam {2}", GetName(),
        // fPercent * 100.0f, di.iDam);

        diBegin->Distribute(this, iExp);

        if (fPercent == 1.0f)
            return pkChrMostAttacked;

        diBegin->Clear();
    }

    {
        for (auto itDi = damage_info_table.begin(); itDi != damage_info_table.end(); ++itDi)
        {
            TDamageInfo &di = *itDi;

            float fPercent = static_cast<float>(di.iDam) / iTotalDam;

            if (fPercent > 1.0f)
            {
                SPDLOG_ERROR("DistributeExp percent over 1.0 (fPercent %f name "
                             "%s)",
                             fPercent, di.pAttacker->GetName());
                fPercent = 1.0f;
            }

            di.Distribute(this, static_cast<int>(iExpToDistribute * fPercent));
        }
    }

    return pkChrMostAttacked;
}

// È­»ì °³¼ö¸¦ ¸®ÅÏÇØ ÁÜ
int CHARACTER::GetArrowAndBow(CItem **ppkBow, CItem **ppkArrow, CountType iArrowCount /*= 1*/)
{
    const auto bow = GetWear(WEAR_WEAPON);
    if (!bow || bow->GetSubType() != WEAPON_BOW)
        return 0;

    const auto arrow = GetWear(WEAR_ARROW);
    if (!arrow || arrow->GetItemType() != ITEM_WEAPON)
        return 0;

    switch (arrow->GetSubType())
    {
    case WEAPON_BOW: {
        iArrowCount = std::min<int>(iArrowCount, arrow->GetCount());
        break;
    }

    case WEAPON_QUIVER: {
        iArrowCount = 1;
        break;
    }
    default:
        break;
    }

    *ppkBow = bow;
    *ppkArrow = arrow;
    return iArrowCount;
}

struct CFuncShoot
{
    CHARACTER *m_me;
    uint8_t m_bType;
    uint32_t m_motionKey;
    bool m_bSucceed;

    CFuncShoot(CHARACTER *ch, uint8_t bType, uint32_t motionKey)
        : m_me(ch), m_bType(bType), m_motionKey(motionKey), m_bSucceed(false)
    {
    }

    void operator()(uint32_t dwTargetVID)
    {
        if (m_bType > 1)
        {
            if (gConfig.disableSkills)
                return;

            m_me->m_SkillUseInfo[m_bType].SetMainTargetVID(dwTargetVID);
            /*if (m_bType == SKILL_BIPABU || m_bType == SKILL_KWANKYEOK)
              m_me->m_SkillUseInfo[m_bType].ResetHitCount();*/
        }

        CHARACTER *pkVictim = g_pCharManager->Find(dwTargetVID);

        if (!pkVictim)
            return;

        // 공격 불가
        if (!battle_is_attackable(m_me, pkVictim))
            return;

        if (m_me->IsNPC())
        {
            if (DISTANCE_APPROX(m_me->GetX() - pkVictim->GetX(), m_me->GetY() - pkVictim->GetY()) > 5000)
                return;
        }

        if (m_me->IsPolymorphed())
        {
            const auto proto = CMobManager::instance().Get(m_me->GetPolymorphVnum());
            if (!proto)
                return;

            if (DISTANCE_APPROX(m_me->GetX() - pkVictim->GetX(), m_me->GetY() - pkVictim->GetY()) > proto->wAttackRange)
                return;
        }

        CItem *pkBow;
        CItem *pkArrow;

        switch (m_bType)
        {
        case 0: {
            PointValue iDam;

            if (m_me->IsPC())
            {
                if (m_me->GetJob() != JOB_ASSASSIN)
                    return;

                if (0 == m_me->GetArrowAndBow(&pkBow, &pkArrow))
                    return;

                if (m_me->GetSkillGroup() != 0)
                    if (!m_me->IsNPC() && m_me->GetSkillGroup() != 2)
                    {
                        if (m_me->GetSP() < 5)
                            return;

                        m_me->PointChange(POINT_SP, -5);
                    }

                iDam = CalcArrowDamage(m_me, pkVictim, pkBow, pkArrow);
                // m_me->UseArrow(pkArrow, 1);
                // check speed hack
                const uint32_t dwCurrentTime = get_dword_time();
                if (IS_SPEED_HACK(m_me, pkVictim, dwCurrentTime, m_motionKey))
                    iDam = 0;
            }
            else
                iDam = CalcMeleeDamage(m_me, pkVictim);

            NormalAttackAffect(m_me, pkVictim);

            // 데미지 계산
            iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_BOW)) / 100;

            // SPDLOG_INFO( "{0} arrow {1} dam {2}", m_me->GetName(), pkVictim->GetName(), iDam);

            m_me->OnMove(true);
            pkVictim->OnMove();

            if (pkVictim->CanBeginFight())
                pkVictim->BeginFight(m_me);

            const bool isDead = pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_NORMAL_RANGE);

            if (!isDead && m_me->IsPC())
            {
                const auto &petSystem = m_me->GetPetSystem();
                if (petSystem)
                    petSystem->LaunchAttack(pkVictim);
            }

            // 타격치 계산부 끝
            break;
        }

        case 1: {
            int iDam;

            if (m_me->IsPC())
                return;

            iDam = CalcMagicDamage(m_me, pkVictim);

            NormalAttackAffect(m_me, pkVictim);

            iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_MAGIC)) / 100;

            // SPDLOG_INFO( "{0} arrow {1} dam {2}", m_me->GetName(), pkVictim->GetName(), iDam);

            m_me->OnMove(true);
            pkVictim->OnMove();

            if (pkVictim->CanBeginFight())
                pkVictim->BeginFight(m_me);

            pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_MAGIC);

            break;
        }

        case SKILL_YEONSA: {
            // 연사
            // int iUseArrow = 2 + (m_me->GetSkillPower(SKILL_YEONSA) *6/100);
            const int iUseArrow = 1;

            if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
            {
                m_me->OnMove(true);
                pkVictim->OnMove();

                if (pkVictim->CanBeginFight())
                    pkVictim->BeginFight(m_me);

                m_me->ComputeSkill(m_bType, pkVictim);
                // m_me->UseArrow(pkArrow, iUseArrow);
            }

            break;
        }

        case SKILL_KWANKYEOK: {
            const int iUseArrow = 1;
            if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
            {
                m_me->OnMove(true);
                pkVictim->OnMove();

                if (pkVictim->CanBeginFight())
                    pkVictim->BeginFight(m_me);

                SPDLOG_INFO("{0} kwankeyok {1}", m_me->GetName(), pkVictim->GetName());
                m_me->ComputeSkill(m_bType, pkVictim);
                // m_me->UseArrow(pkArrow, iUseArrow);
            }

            break;
        }

        case SKILL_GIGUNG: {
            const int iUseArrow = 1;
            if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
            {
                m_me->OnMove(true);
                pkVictim->OnMove();

                if (pkVictim->CanBeginFight())
                    pkVictim->BeginFight(m_me);

                SPDLOG_INFO("{0} gigung {1}", m_me->GetName(), pkVictim->GetName());
                m_me->ComputeSkill(m_bType, pkVictim);
                // m_me->UseArrow(pkArrow, iUseArrow);
            }

            break;
        }

        case SKILL_HWAJO: {
            const int iUseArrow = 1;
            if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
            {
                m_me->OnMove(true);
                pkVictim->OnMove();

                if (pkVictim->CanBeginFight())
                    pkVictim->BeginFight(m_me);

                SPDLOG_INFO("{0} hwajo {1}", m_me->GetName(), pkVictim->GetName());
                m_me->ComputeSkill(m_bType, pkVictim);
                // m_me->UseArrow(pkArrow, iUseArrow);
            }

            break;
        }

        case SKILL_HORSE_WILDATTACK_RANGE: {
            const int iUseArrow = 1;
            if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
            {
                m_me->OnMove(true);
                pkVictim->OnMove();

                if (pkVictim->CanBeginFight())
                    pkVictim->BeginFight(m_me);

                SPDLOG_INFO("{0} horse_wildattack {1}", m_me->GetName(), pkVictim->GetName());
                m_me->ComputeSkill(m_bType, pkVictim);
                // m_me->UseArrow(pkArrow, iUseArrow);
            }

            break;
        }

        case SKILL_MARYUNG:
        case SKILL_TUSOK:
        case SKILL_BIPABU:
        case SKILL_NOEJEON:
        case SKILL_GEOMPUNG:
        case SKILL_SANGONG:
        case SKILL_MAHWAN:
        case SKILL_PABEOB: {
            m_me->OnMove(true);
            pkVictim->OnMove();

            if (pkVictim->CanBeginFight())
                pkVictim->BeginFight(m_me);

            SPDLOG_INFO("{0} - Skill {1} -> {2}", m_me->GetName(), m_bType, pkVictim->GetName());
            m_me->ComputeSkill(m_bType, pkVictim);
            break;
        }

        case SKILL_CHAIN: {
            m_me->OnMove(true);
            pkVictim->OnMove();

            if (pkVictim->CanBeginFight())
                pkVictim->BeginFight(m_me);

            SPDLOG_INFO("{0} - Skill {1} -> {2}", m_me->GetName(), m_bType, pkVictim->GetName());
            m_me->ComputeSkill(m_bType, pkVictim);

            break;
        }

        case SKILL_YONGBI: {
            m_me->OnMove(true);
            break;
        }

            /*case SKILL_BUDONG: {
                m_me->OnMove(true);
                pkVictim->OnMove();

                uint32_t * pdw;
                uint32_t dwEI = AllocEventInfo(sizeof(uint32_t) * 2, &pdw);
                pdw[0] = m_me->GetVID();
                pdw[1] = pkVictim->GetVID();

                event_create(budong_event_func, dwEI, THECORE_SECS_TO_PASSES(1));
                break;
            }*/

        default:
            SPDLOG_ERROR("CFuncShoot: I don't know this type [{0}] of range attack.", static_cast<int>(m_bType));
            break;
        }

        m_bSucceed = true;
    }
};

bool CHARACTER::Shoot(uint8_t bType, uint32_t motionKey)
{
    SPDLOG_DEBUG("Shoot {0} type {1} flyTargets.size %zu", GetName(), bType, m_vec_dwFlyTargets.size());

    if (!CanMove())
        return false;

    CFuncShoot f(this, bType, motionKey);

    if (m_dwFlyTargetID != 0)
    {
        f(m_dwFlyTargetID);
        m_dwFlyTargetID = 0;
    }

    f = std::for_each(m_vec_dwFlyTargets.begin(), m_vec_dwFlyTargets.end(), f);
    m_vec_dwFlyTargets.clear();

    return f.m_bSucceed;
}

void CHARACTER::FlyTarget(uint32_t dwTargetVID, long x, long y, uint8_t bHeader)
{
    CHARACTER *pkVictim = g_pCharManager->Find(dwTargetVID);
    TPacketGCFlyTargeting pack;

    // pack.bHeader	= HEADER_GC_FLY_TARGETING;
    pack.dwShooterVID = GetVID();

    if (pkVictim)
    {
        pack.dwTargetVID = pkVictim->GetVID();
        pack.x = pkVictim->GetX();
        pack.y = pkVictim->GetY();

        if (bHeader == HEADER_CG_FLY_TARGETING)
            m_dwFlyTargetID = dwTargetVID;
        else
            m_vec_dwFlyTargets.push_back(dwTargetVID);
    }
    else
    {
        pack.dwTargetVID = 0;
        pack.x = x;
        pack.y = y;
    }

    SPDLOG_DEBUG("FlyTarget {0} vid {1} x {2} y {3}", GetName(), pack.dwTargetVID, pack.x, pack.y);
    PacketAround(m_map_view, this,
                 (bHeader == HEADER_CG_FLY_TARGETING) ? HEADER_GC_FLY_TARGETING : HEADER_GC_ADD_FLY_TARGETING, pack,
                 this);
}

CHARACTER *CHARACTER::GetNearestVictim(CHARACTER *pkChr)
{
    if (nullptr == pkChr)
        pkChr = this;

    float fMinDist = 99999.0f;
    CHARACTER *pkVictim = nullptr;

    auto it = m_map_kDamage.begin();

    // ÀÏ´Ü ÁÖÀ§¿¡ ¾ø´Â »ç¶÷À» °É·¯ ³½´Ù.
    while (it != m_map_kDamage.end())
    {
        const VID &c_VID = it->first;
        ++it;

        CHARACTER *pAttacker = g_pCharManager->Find(c_VID);

        if (!pAttacker)
            continue;

        if (pAttacker->IsPet())
            pAttacker = pAttacker->GetPet()->GetOwnerPtr();

        if (pAttacker->FindAffect(SKILL_EUNHYUNG) || pAttacker->FindAffect(AFFECT_INVISIBILITY) ||
            pAttacker->FindAffect(AFFECT_REVIVE_INVISIBLE))
            continue;

        const int dist = DISTANCE_APPROX(pAttacker->GetX() - pkChr->GetX(), pAttacker->GetY() - pkChr->GetY());
        if (dist < fMinDist && pAttacker->GetMapIndex() == GetMapIndex())
        {
            pkVictim = pAttacker;
            fMinDist = dist;
        }
    }

    return pkVictim;
}

void CHARACTER::SetVictim(CHARACTER *pkVictim)
{
    if (!pkVictim)
    {
        if (0 != static_cast<uint32_t>(m_kVIDVictim))
            MonsterLog("Can't find victim");

        m_kVIDVictim.Reset();
        battle_end(this);
    }
    else
    {
        if (pkVictim->IsPet())
            pkVictim = pkVictim->GetPet()->GetOwnerPtr();

        if (m_kVIDVictim != pkVictim->GetVID())
            MonsterLog("Set victim: %s", pkVictim->GetName());

        m_kVIDVictim = pkVictim->GetVID();
        m_dwLastVictimSetTime = get_dword_time();
    }
}

CHARACTER *CHARACTER::GetVictim() const { return g_pCharManager->Find(m_kVIDVictim); }

CHARACTER *CHARACTER::GetProtege() const // º¸È£ÇØ¾ß ÇÒ ´ë»óÀ» ¸®ÅÏ
{
    if (m_pkChrStone)
        return m_pkChrStone;

#ifdef __FAKE_PC__
    if (FakePC_IsSupporter())
        return FakePC_GetOwner();
#endif

    if (m_pkParty)
        return m_pkParty->GetLeader();

    return nullptr;
}

int CHARACTER::GetAlignment() const { return m_iAlignment; }

int CHARACTER::GetRealAlignment() const { return m_iRealAlignment; }

void CHARACTER::ShowAlignment(bool bShow)
{
    if (bShow)
    {
        if (m_iAlignment != m_iRealAlignment)
        {
            m_iAlignment = m_iRealAlignment;
            UpdatePacket();
        }
    }
    else
    {
        if (m_iAlignment != 0)
        {
            m_iAlignment = 0;
            UpdatePacket();
        }
    }
}

void CHARACTER::UpdateAlignment(int iAmount)
{
#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex()))
        return;
#endif

    bool bShow = false;

    if (m_iAlignment == m_iRealAlignment)
        bShow = true;

    const int i = m_iAlignment / 10;

    m_iRealAlignment = std::clamp(m_iRealAlignment + iAmount, -10000000, 10000000);

    if (bShow)
    {
        m_iAlignment = m_iRealAlignment;

        if (i != m_iAlignment / 10)
            UpdatePacket();
    }
}

void CHARACTER::SetKillerMode(bool isOn)
{
    if ((isOn ? ADD_CHARACTER_STATE_KILLER : 0) == IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER))
        return;

    if (isOn)
        SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
    else
        REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);

    m_iKillerModePulse = thecore_pulse();
    UpdatePacket();
    SPDLOG_INFO("SetKillerMode Update %s[%d]", GetName(), GetPlayerID());
}

bool CHARACTER::IsKillerMode() const { return IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER); }

void CHARACTER::UpdateKillerMode()
{
    if (!IsKillerMode())
        return;

    const int iKillerSeconds = 60;

    if (thecore_pulse() - m_iKillerModePulse >= THECORE_SECS_TO_PASSES(iKillerSeconds))
        SetKillerMode(false);
}

void CHARACTER::SetPKMode(uint8_t bPKMode)
{
    if (bPKMode >= PK_MODE_MAX_NUM)
        return;

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if ((MeleyLair::CMgr::instance().IsMeleyMap(GetMapIndex())) && (bPKMode != PK_MODE_GUILD))
        bPKMode = PK_MODE_GUILD;
#endif

    if (m_bPKMode == bPKMode)
        return;

    if (bPKMode == PK_MODE_GUILD && !GetGuild())
        bPKMode = PK_MODE_FREE;

    m_bPKMode = bPKMode;
    UpdatePacket();

    SPDLOG_INFO("PK_MODE: %s %d", GetName(), m_bPKMode);
}

uint8_t CHARACTER::GetPKMode() const { return m_bPKMode; }

struct FuncForgetMyAttacker
{
    CHARACTER *m_ch;

    FuncForgetMyAttacker(CHARACTER *ch) { m_ch = ch; }

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = static_cast<CHARACTER *>(ent);
            if (ch->IsPC())
                return;
            if (ch->m_kVIDVictim == m_ch->GetVID())
                ch->SetVictim(nullptr);
        }
    }
};

struct FuncAggregateMonster
{
    CHARACTER *m_ch;
    uint32_t m_range;

    FuncAggregateMonster(CHARACTER *ch, uint32_t range)
    {
        m_ch = ch;
        m_range = range;
    }

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = static_cast<CHARACTER *>(ent);
            if (ch->IsPC())
                return;
            if (!ch->IsMonster())
                return;
            if (ch->GetVictim())
                return;

            if (DISTANCE_APPROX(ch->GetX() - m_ch->GetX(), ch->GetY() - m_ch->GetY()) < m_range)
                if (ch->CanBeginFight())
                    ch->BeginFight(m_ch);
        }
    }
};

struct FuncAttractRanger
{
    CHARACTER *m_ch;

    FuncAttractRanger(CHARACTER *ch) { m_ch = ch; }

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = static_cast<CHARACTER *>(ent);
            if (ch->IsPC())
                return;
            if (!ch->IsMonster())
                return;
            if (ch->GetVictim() && ch->GetVictim() != m_ch)
                return;
            if (ch->GetMobAttackRange() > 150)
            {
                int iNewRange = 150; //(int)(ch->GetMobAttackRange() * 0.2);
                if (iNewRange < 150)
                    iNewRange = 150;

                ch->AddAffect(AFFECT_BOW_DISTANCE, POINT_BOW_DISTANCE, iNewRange - ch->GetMobAttackRange(), 3 * 60, 0,
                              false);
            }
        }
    }
};

struct FuncPullMonster
{
    CHARACTER *m_ch;
    int m_iLength;

    FuncPullMonster(CHARACTER *ch, int iLength = 300)
    {
        m_ch = ch;
        m_iLength = iLength;
    }

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = static_cast<CHARACTER *>(ent);
            if (ch->IsPC())
                return;
            if (!ch->IsMonster())
                return;
            // if (ch->GetVictim() && ch->GetVictim() != m_ch)
            // return;
            const float fDist =
                static_cast<float>(DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY()));
            if (fDist > 3000 || fDist < 100)
                return;

            float fNewDist = fDist - m_iLength;
            if (fNewDist < 100)
                fNewDist = 100;

            const float degree = GetDegreeFromPositionXY(ch->GetX(), ch->GetY(), m_ch->GetX(), m_ch->GetY());
            float fx;
            float fy;

            GetDeltaByDegree(degree, fDist - fNewDist, &fx, &fy);
            const long tx = static_cast<long>(ch->GetX() + fx);
            const long ty = static_cast<long>(ch->GetY() + fy);

            ch->Sync(tx, ty);
            ch->Goto(tx, ty);

            ch->SyncPacket();
        }
    }
};

void CHARACTER::ForgetMyAttacker()
{
    FuncForgetMyAttacker f(this);
    ForEachSeen(f);

    ReviveInvisible(30);
}

void CHARACTER::AggregateMonster(uint32_t range)
{
    FuncAggregateMonster f(this, range);
    ForEachSeen(f);
}

void CHARACTER::AttractRanger()
{
    FuncAttractRanger f(this);
    ForEachSeen(f);
}

void CHARACTER::PullMonster()
{
    FuncPullMonster f(this);
    ForEachSeen(f);
}

void CHARACTER::UpdateAggrPointEx(CHARACTER *pAttacker, EDamageType type, int dam, CHARACTER::TBattleInfo &info)
{
    // Æ¯Á¤ °ø°ÝÅ¸ÀÔ¿¡ µû¶ó ´õ ¿Ã¶ó°£´Ù
    switch (type)
    {
    case DAMAGE_TYPE_NORMAL_RANGE:
        dam = static_cast<int>(dam * 1.2f);
        break;

    case DAMAGE_TYPE_RANGE:
        dam = static_cast<int>(dam * 1.5f);
        break;

    case DAMAGE_TYPE_MAGIC:
        dam = static_cast<int>(dam * 1.2f);
        break;

    default:
        break;
    }

    // °ø°ÝÀÚ°¡ ÇöÀç ´ë»óÀÎ °æ¿ì º¸³Ê½º¸¦ ÁØ´Ù.
    if (pAttacker == GetVictim())
        dam = static_cast<int>(dam * 1.2f);

    info.iAggro += dam;

    if (info.iAggro < 0)
        info.iAggro = 0;

    if (GetParty() && dam > 0 && type != DAMAGE_TYPE_SPECIAL)
    {
        auto pParty = GetParty();

        // ¸®´õÀÎ °æ¿ì ¿µÇâ·ÂÀÌ Á»´õ °­ÇÏ´Ù
        int iPartyAggroDist = dam;

        if (pParty->GetLeaderPID() == static_cast<uint32_t>(GetVID()))
            iPartyAggroDist /= 2;
        else
            iPartyAggroDist /= 3;

        pParty->SendMessage(this, PM_AGGRO_INCREASE, iPartyAggroDist, pAttacker->GetVID());
    }

    if (type != DAMAGE_TYPE_POISON)
        ChangeVictimByAggro(info.iAggro, pAttacker);
}

void CHARACTER::UpdateAggrPoint(CHARACTER *pAttacker, EDamageType type, int dam)
{
    if (IsDead() || IsStun())
        return;

    TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());
    if (it == m_map_kDamage.end())
    {
        m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(0, dam)));
        it = m_map_kDamage.find(pAttacker->GetVID());
    }

    UpdateAggrPointEx(pAttacker, type, dam, it->second);
}

void CHARACTER::ChangeVictimByAggro(int iNewAggro, CHARACTER *pNewVictim)
{
    if (get_dword_time() - m_dwLastVictimSetTime < 3000) // 3ÃÊ´Â ±â´Ù·Á¾ßÇÑ´Ù
        return;

    if (pNewVictim == GetVictim())
    {
        if (m_iMaxAggro < iNewAggro)
        {
            m_iMaxAggro = iNewAggro;
            return;
        }

        // Aggro°¡ °¨¼ÒÇÑ °æ¿ì
        TDamageMap::iterator it;
        auto itFind = m_map_kDamage.end();

        for (it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
        {
            if (it->second.iAggro > iNewAggro)
            {
                CHARACTER *ch = g_pCharManager->Find(it->first);

                if (ch && (!ch->IsDead() && !ch->IsPet() && !ch->IsPetPawn()) &&
                    DISTANCE_APPROX(ch->GetX() - GetX(), ch->GetY() - GetY()) < 5000 &&
                    ch->GetMapIndex() == GetMapIndex())
                {
                    itFind = it;
                    iNewAggro = it->second.iAggro;
                }
            }
        }

        if (itFind != m_map_kDamage.end())
        {
            auto victim = g_pCharManager->Find(itFind->first);
            if (victim->IsPet())
                victim = victim->GetPet()->GetOwnerPtr();

            m_iMaxAggro = iNewAggro;
            SetVictim(victim);
            m_dwStateDuration = 1;
        }
    }
#ifdef ENABLE_HYDRA_DUNGEON
    else if (!m_bLockTarget)
#else
    else
#endif
    {
        if (m_iMaxAggro < iNewAggro)
        {
            m_iMaxAggro = iNewAggro;
            auto victim = pNewVictim;
            if (victim->IsPet())
                victim = victim->GetPet()->GetOwnerPtr();

            SetVictim(victim);
            m_dwStateDuration = 1;
        }
    }
}
