#include "utils.h"
#include "config.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "GBattle.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "vector.h"
#include <game/GamePacket.hpp>
#include "pvp.h"
#include "guild.h"
#include <game/AffectConstants.hpp>
#include "lua_incl.h"
#include "GArena.h"
#include "sectree.h"
#include "locale_service.h"
#include "questmanager.h"
#include "main.h"
#include <game/MotionTypes.hpp>
#include <game/MobConstants.hpp>
#include <algorithm>
#include "motion.h"

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

#define DEF_STR(x) #x

int battle_hit(CHARACTER *pkAttacker, CHARACTER *pkVictim);

bool timed_event_cancel(CHARACTER *ch)
{
    if (ch->m_pkTimedEvent)
    {
        event_cancel(&ch->m_pkTimedEvent);
        return true;
    }

    /* RECALL_DELAY
       Â÷ÈÄ ÀüÅõ·Î ÀÎÇØ ±ÍÈ¯ºÎ µô·¹ÀÌ°¡ Ãë¼Ò µÇ¾î¾ß ÇÒ °æ¿ì ÁÖ¼® ÇØÁ¦
       if (ch->m_pk_RecallEvent)
       {
       event_cancel(&ch->m_pkRecallEvent);
       return true;
       }
       END_OF_RECALL_DELAY */

    return false;
}

bool battle_distance_valid(CHARACTER *ch, CHARACTER *victim)
{
    if (victim->IsBuilding())
        return true;

    const uint32_t distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

    uint32_t max = gConfig.pcMaxHitRange;

    if (!ch->IsPC()) { max = ch->GetMobAttackRange() * 2; }
    else
    {
        if (!victim->IsPC() && BATTLE_TYPE_MELEE == victim->GetMobBattleType())
            max = std::max<uint32_t>(max, victim->GetMobAttackRange() * 2);
    }

    return distance <= max;
}

bool battle_is_attackable(CHARACTER *ch, CHARACTER *victim)
{
    if (victim->GetRaceNum() == 20387)
        return false;

    if (victim->IsDead() || victim->IsObserverMode())
        return false;

    if (victim->IsBuffBot())
        return false;

    // ¾ÈÀüÁö´ë¸é Áß´Ü
    if (!ch->IsGuardNPC())
    {
        SECTREE *sectree = nullptr;

        sectree = ch->GetSectree();
        if (sectree && sectree->IsAttr(ch->GetX(), ch->GetY(), ATTR_BANPK))
            return false;

        sectree = victim->GetSectree();
        if (sectree && sectree->IsAttr(victim->GetX(), victim->GetY(), ATTR_BANPK))
            return false;
    }

    // ³»°¡ Á×¾úÀ¸¸é Áß´ÜÇÑ´Ù.
    if (ch->IsStun() || ch->IsDead())
        return false;

    if (ch->IsPC() && victim->IsPC())
    {
        CGuild *g1 = ch->GetGuild();
        CGuild *g2 = victim->GetGuild();

        if (g1 && g2)
        {
            auto attackerHasGuildMode = ch->GetPKMode() == PK_MODE_GUILD;
            auto victimHasGuildMode = victim->GetPKMode() == PK_MODE_GUILD;

            if (g1->GetID() == g2->GetID())
            {
                if (attackerHasGuildMode || victimHasGuildMode)
                    return false;
            }

            if (g1->UnderWar(g2->GetID()))
                return true;
        }
    }

    if (CArenaManager::instance().CanAttack(ch, victim) == true)
        return true;

#ifdef ENABLE_HYDRA_DUNGEON
    if (CHydraDungeonManager::instance().CanAttack(ch, victim))
        return true;
#endif

    return CPVPManager::instance().CanAttack(ch, victim);
}

int battle_melee_attack(CHARACTER *ch, CHARACTER *victim)
{
    if (!victim || ch == victim)
    {
        SPDLOG_DEBUG("battle_melee_attack Invalid victim ch: {0} victim: {1}", (void *)ch, (void *)victim);
        return BATTLE_NONE;
    }

    if (!battle_is_attackable(ch, victim))
    {
        SPDLOG_DEBUG("Victim not attackable {0} -> {1}", ch->GetName(), victim->GetName());
        return BATTLE_NONE;
    }

    int distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

    if (!victim->IsBuilding())
    {
        int max = gConfig.pcMaxHitRange;

        const auto mapIndex = ch->GetMapIndex();

        if (false == ch->IsPC())
        {
            // 몬스터의 경우 몬스터 공격 거리를 사용
            max = (int)(ch->GetMobAttackRange() * 1.15f);
        }
        else
        {
            // PC일 경우 상대가 melee 몹일 경우 몹의 공격 거리가 최대 공격 거리
            if (false == victim->IsPC() && BATTLE_TYPE_MELEE == victim->GetMobBattleType())
                max = std::max(300, (int)(victim->GetMobAttackRange() * 1.15f));
        }

#ifdef ENABLE_HYDRA_DUNGEON
        if (victim->GetRaceNum() == HYDRA_BOSS_VNUM)
            max = 1000;
#endif

        if (distance > max)
        {
            if (gConfig.testServer)
                SPDLOG_INFO("VICTIM_FAR: {0} distance: {1} max: {2}", ch->GetName(), distance, max);

            return BATTLE_NONE;
        }
    }

    if (timed_event_cancel(ch))
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "전투가 시작 되어 취소 되었습니다.");

    if (timed_event_cancel(victim))
        SendI18nChatPacket(victim, CHAT_TYPE_INFO, "전투가 시작 되어 취소 되었습니다.");

    ch->SetPosition(POS_FIGHTING);
    ch->SetVictim(victim);

    const PIXEL_POSITION &vpos = victim->GetXYZ();
    ch->SetRotationToXY(vpos.x, vpos.y);

    return battle_hit(ch, victim);
}

// ½ÇÁ¦ GET_BATTLE_VICTIMÀ» NULL·Î ¸¸µé°í ÀÌº¥Æ®¸¦ Äµ½½ ½ÃÅ²´Ù.
void battle_end_ex(CHARACTER *ch)
{
    if (ch->IsPosition(POS_FIGHTING))
        ch->SetPosition(POS_STANDING);
}

void battle_end(CHARACTER *ch) { battle_end_ex(ch); }

// AG = Attack Grade
// AL = Attack Limit
PointValue CalcBattleDamage(PointValue iDam, int iAttackerLev, int iVictimLev)
{
    if (iDam < 3)
        iDam = Random::get(1, 5);

    //return CALCULATE_DAMAGE_LVDELTA(iAttackerLev, iVictimLev, iDam);
    return iDam;
}

PointValue CalcMagicDamageWithValue(PointValue iDam, CHARACTER *pkAttacker, CHARACTER *pkVictim)
{
    return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

int CalcMagicDamage(CHARACTER *pkAttacker, CHARACTER *pkVictim)
{
    PointValue iDam = 0;

    if (pkAttacker->IsNPC()) { iDam = CalcMeleeDamage(pkAttacker, pkVictim, false, false); }

    return CalcMagicDamageWithValue(iDam, pkAttacker, pkVictim);
}

float CalcAttackRating(CHARACTER *pkAttacker, CHARACTER *pkVictim, bool bIgnoreTargetRating)
{
    return 1.0f;
    /*
     *
    int attacker_dx = pkAttacker->GetPolymorphPoint(POINT_DX);
    int attacker_lv = pkAttacker->GetLevel();

    int victim_dx = pkVictim->GetPolymorphPoint(POINT_DX);
    int victim_lv = pkAttacker->GetLevel();

    int iARSrc = std::min(90, (attacker_dx * 4 + attacker_lv * 2) / 6);
    int iERSrc = std::min(90, (victim_dx * 4 + victim_lv * 2) / 6);

    float fAR = ((float) iARSrc + 210.0f) / 300.0f; // fAR = 0.7 ~ 1.0

    if (bIgnoreTargetRating)
        return fAR;

    // ((Edx * 2 + 20) / (Edx + 110)) * 0.3
    float fER = ((float) (iERSrc * 2 + 5) / (iERSrc + 95)) * 3.0f / 10.0f;

    return fAR - fER;
         */
}

PointValue ApplyAttackBonus(CHARACTER *pkAttacker, CHARACTER *pkVictim, PointValue iAtk)
{
    if (!pkVictim->IsPC())
        iAtk += pkAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_ATTACK_BONUS);

#ifdef __FAKE_PC__
    if (!pkAttacker->IsPC() && !pkAttacker->FakePC_Check())
#else
    if (!pkAttacker->IsPC())
#endif
    {
        PointValue iReduceDamagePct = pkVictim->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_TRANSFER_DAMAGE);
        iAtk = iAtk * (100.f + iReduceDamagePct) / 100.f;
    }

#ifdef __FAKE_PC__
    if (pkAttacker->IsNPC() && !pkAttacker->FakePC_Check() && pkVictim->IsPC())
#else
    if (pkAttacker->IsNPC() && pkVictim->IsPC())
#endif
    {
        iAtk = (iAtk * g_pCharManager->GetMobDamageRate(pkAttacker)) / 100;
    }

    auto index = pkVictim->GetMapIndex();
    if (index > 10000)
        index /= 10000;

#ifdef __FAKE_PC__
    if (pkVictim->IsNPC() && !pkVictim->FakePC_Check())
#else
    if (pkVictim->IsNPC())
#endif
    {
        if (pkVictim->IsRaceFlag(RACE_FLAG_ANIMAL))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ANIMAL)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_UNDEAD))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_UNDEAD)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_DEVIL))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DEVIL)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_HUMAN))
        {
#ifdef SHELIA_BUILD
            iAtk += (iAtk *
                     (pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN) + pkAttacker->GetPoint(POINT_PARTY_DESTROYER_BONUS))) /
                    100;
#else
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN)) / 100.f;
#endif
        }
        if (pkVictim->IsRaceFlag(RACE_FLAG_ORC))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ORC)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_MILGYO))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_MILGYO)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_INSECT))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_INSECT)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_FIRE))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_FIRE)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_ICE))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ICE)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_DESERT))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_DESERT)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_TREE))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_TRENT)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_CZ))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_CZ)) / 100.f;
        if (pkVictim->IsRaceFlag(RACE_FLAG_SHADOW))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SHADOW)) / 100.f;

        if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_ELEC))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_ELECT)) / 10000.f;
        else if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_FIRE))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_FIRE)) / 10000.f;
        else if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_ICE))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_ICE)) / 10000.f;
        else if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_WIND))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_WIND)) / 10000.f;
        else if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_EARTH))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_EARTH)) / 10000.f;
        else if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_DARK))
            iAtk += (iAtk * 30 * pkAttacker->GetPoint(POINT_ENCHANT_DARK)) / 10000.f;

        if ((pkVictim->GetMobRank() == MOB_RANK_BOSS || pkVictim->GetMobRank() == MOB_RANK_KING) &&
            !pkVictim->IsStone())
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_BOSS)) / 100.0f;

        if ((pkVictim->GetMobRank() == MOB_RANK_LEGENDARY))
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_LEGEND)) / 100.0f;

        if (pkVictim->IsStone())
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_METIN)) / 100.0f;

        iAtk += (iAtk * (pkAttacker->GetPoint(POINT_ATTBONUS_MONSTER) + pkAttacker->GetPoint(POINT_ENERGY))) / 100.f;

#ifdef ENABLE_PVM_BOOST_FOR_SHAMAN_AND_NINJA
        if (pkAttacker->GetJob() == JOB_ASSASSIN || pkAttacker->GetJob() == JOB_SHAMAN)
            iAtk += (iAtk * 25) / 100;
#endif
    }
#ifdef __FAKE_PC__
    else if (pkVictim->IsPC() || pkVictim->FakePC_Check())
#else
    else if (pkVictim->IsPC())
#endif
    {
#ifdef SHELIA_BUILD
        iAtk +=
            (iAtk * (pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN) + pkAttacker->GetPoint(POINT_PARTY_DESTROYER_BONUS))) /
            100;
#else
        iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN)) / 100.f;
#endif

        switch (pkVictim->GetJob())
        {
        case JOB_WARRIOR:
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WARRIOR)) / 100.f;
            break;

        case JOB_ASSASSIN:
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ASSASSIN)) / 100.f;
            break;

        case JOB_SURA:
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SURA)) / 100.f;
            break;

        case JOB_SHAMAN:
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SHAMAN)) / 100.f;
            break;

        case JOB_WOLFMAN:
            iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WOLFMAN)) / 100.f;
            break;
        }
    }

    return iAtk;
}

PointValue ApplyResistBonus(CHARACTER *attacker, CHARACTER *victim, PointValue atk)
{
    switch (attacker->GetJob())
    {
    case JOB_WARRIOR:
        atk -= (atk * victim->GetPoint(POINT_RESIST_WARRIOR)) / 100.0f;
        break;

    case JOB_ASSASSIN:
        atk -= (atk * victim->GetPoint(POINT_RESIST_ASSASSIN)) / 100.0f;
        break;

    case JOB_SURA:
        atk -= (atk * victim->GetPoint(POINT_RESIST_SURA)) / 100.0f;
        break;

    case JOB_SHAMAN:
        atk -= (atk * victim->GetPoint(POINT_RESIST_SHAMAN)) / 100.0f;
        break;

    case JOB_WOLFMAN:
        atk -= (atk * victim->GetPoint(POINT_RESIST_WOLFMAN)) / 100.0f;
        break;
    }

    if (attacker->IsRaceFlag(RACE_FLAG_ATT_ELEC))
        atk -= (atk * victim->GetPoint(POINT_RESIST_ELEC)) / 100.0f;
    else if (attacker->IsRaceFlag(RACE_FLAG_ATT_FIRE))
        atk -= (atk * victim->GetPoint(POINT_RESIST_FIRE)) / 100.0f;
    else if (attacker->IsRaceFlag(RACE_FLAG_ATT_ICE))
        atk -= (atk * victim->GetPoint(POINT_RESIST_ICE)) / 100.0f;
    else if (attacker->IsRaceFlag(RACE_FLAG_ATT_WIND))
        atk -= (atk * victim->GetPoint(POINT_RESIST_WIND)) / 100.0f;
    else if (attacker->IsRaceFlag(RACE_FLAG_ATT_EARTH))
        atk -= (atk * victim->GetPoint(POINT_RESIST_EARTH)) / 100.0f;
    else if (attacker->IsRaceFlag(RACE_FLAG_ATT_DARK))
        atk -= (atk * victim->GetPoint(POINT_RESIST_DARK)) / 100.0f;

#ifdef SHELIA_BUILD
    atk -= (atk * victim->GetPoint(POINT_RESIST_HUMAN) + victim->GetPoint(POINT_PARTY_DEFENDER_BONUS)) / 100.0f;
#else
    atk -= (atk * victim->GetPoint(POINT_RESIST_HUMAN)) / 100.0f;
#endif

    return atk;
}

bool IsValidMeleeWeapon(const TItemTable *p)
{
    if (p->bType != ITEM_WEAPON)
        return false;

    switch (p->bSubType)
    {
    case WEAPON_SWORD:
    case WEAPON_DAGGER:
    case WEAPON_TWO_HANDED:
    case WEAPON_BELL:
    case WEAPON_FAN:
    case WEAPON_CLAW:
    case WEAPON_MOUNT_SPEAR:
        return true;
    }

    return false;
}

DamageValue CalcMeleeDamage(CHARACTER *pkAttacker, CHARACTER *pkVictim, bool bIgnoreDefense, bool bIgnoreTargetRating)
{
    CItem *pWeapon = pkAttacker->GetWear(WEAR_WEAPON);
    bool bPolymorphed = pkAttacker->IsPolymorphed();

    PointValue iDam = 0;
    float fAR = CalcAttackRating(pkAttacker, pkVictim, bIgnoreTargetRating);
    PointValue iDamMin = 0, iDamMax = 0, damBonus = 0;

    // TESTSERVER_SHOW_ATTACKINFO
    PointValue DEBUG_iDamCur = 0;
    PointValue DEBUG_iDamBonus = 0;
    // END_OF_TESTSERVER_SHOW_ATTACKINFO

    if (pWeapon && IsValidMeleeWeapon(pWeapon->GetProto()))
    {
        iDamMin = pWeapon->GetValue(3);
        iDamMax = pWeapon->GetValue(4);
        damBonus = pWeapon->GetValue(5);
    }
    else if (pkAttacker->IsNPC())
    {
        iDamMin = pkAttacker->GetMobDamageMin();
        iDamMax = pkAttacker->GetMobDamageMax();
    }

    const auto *sash = pkAttacker->GetWear(WEAR_COSTUME_ACCE);
    if (sash && sash->GetSocket(1) != 0)
    {
        const ItemVnum vnum = sash->GetSocket(1);
        const auto f = sash->GetSocket(0);

        const auto* p = ITEM_MANAGER::instance().GetTable(vnum);
        if (p && IsValidMeleeWeapon(p))
        {
            iDamMin += std::max<DamageValue>(0, p->alValues[3] * f / 100.0);
            iDamMax += std::max<DamageValue>(0, p->alValues[4] * f / 100.0);
            damBonus += std::max<DamageValue>(0, p->alValues[5] * f / 100.0);
        }
    }

    if (bPolymorphed && !pkAttacker->IsPolyMaintainStat())
    {
        uint32_t dwMobVnum = pkAttacker->GetPolymorphVnum();
        const auto* pMob = CMobManager::instance().Get(dwMobVnum);
        if (pMob)
        {
            int iPower = pkAttacker->GetPolymorphPower();
            iDamMin += pMob->dwDamageRange[0] * iPower / 100.0;
            iDamMax += pMob->dwDamageRange[1] * iPower / 100.0;
        }
    }

    iDam = Random::get(iDamMin, iDamMax) * 2.0;

    DEBUG_iDamCur = iDam;

    PointValue iAtk = 0;

    // level must be ignored when multiply by fAR, so subtract it before calculation.
    iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) + pkAttacker->GetPoint(POINT_ATT_GRADE_BONUS) + iDam - (pkAttacker->GetLevel() * 2.0);
    iAtk = (iAtk * fAR);
    iAtk += pkAttacker->GetLevel() * 2.0; // and add again

    // Bonus damage
    iAtk += damBonus * 2.0;
    DEBUG_iDamBonus = damBonus * 2;

    iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS); // party attacker role bonus
    iAtk =
        (iAtk *
              (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) /
              100);

    iAtk = ApplyAttackBonus(pkAttacker, pkVictim, iAtk);
    iAtk = ApplyResistBonus(pkAttacker, pkVictim, iAtk);

    PointValue iDef = 0;

    if (!bIgnoreDefense)
    {
        iDef = pkVictim->GetPoint(POINT_DEF_GRADE) * (100 + pkVictim->GetPoint(POINT_DEF_BONUS)) / 100;

        if (!pkAttacker->IsPC())
            iDef += pkVictim->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_DEFENSE_BONUS);
    }

#ifdef __FAKE_PC__
    if (pkAttacker->IsNPC() && !pkAttacker->FakePC_Check())
#else
    if (pkAttacker->IsNPC())
#endif
        iAtk = iAtk * pkAttacker->GetMobDamageMultiply();

    iDam = std::max(0.0, iAtk - iDef);

    if (gConfig.testServer && gConfig.damageDebug)
    {
        int DEBUG_iLV = pkAttacker->GetLevel() * 2;
        PointValue DEBUG_iST = int((pkAttacker->GetPoint(POINT_ATT_GRADE) - DEBUG_iLV) * fAR);
        PointValue DEBUG_iPT = pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);
        PointValue DEBUG_iWP = 0;
        PointValue DEBUG_iPureAtk = 0;
        PointValue DEBUG_iPureDam = 0;
        std::string szRB = "";
        std::string szGradeAtkBonus = "";

        DEBUG_iWP = (DEBUG_iDamCur * fAR);
        DEBUG_iPureAtk = DEBUG_iLV + DEBUG_iST + DEBUG_iWP + DEBUG_iDamBonus;
        DEBUG_iPureDam = iAtk - iDef;

        if (pkAttacker->IsNPC())
        {
            szGradeAtkBonus = fmt::format("={:.2f}*{:.2f}", DEBUG_iPureAtk, pkAttacker->GetMobDamageMultiply());
            DEBUG_iPureAtk = int(DEBUG_iPureAtk * pkAttacker->GetMobDamageMultiply());
        }

        if (DEBUG_iDamBonus != 0)
            szRB = fmt::format("+RB({:.2f})", DEBUG_iDamBonus);

        std::string szPT = "";

        if (DEBUG_iPT != 0)
            szPT = fmt::format(", PT={:.2f}", DEBUG_iPT);

        std::string szUnknownAtk = "";

        if (iAtk != DEBUG_iPureAtk)
            szUnknownAtk = fmt::format("+?({:.2f})", iAtk - DEBUG_iPureAtk);

        std::string szUnknownDam = "";

        if (iDam != DEBUG_iPureDam)
            szUnknownDam = fmt::format("+?({:.2f})", iDam - DEBUG_iPureDam);

        std::string szMeleeAttack;

        szMeleeAttack = fmt::format("{}({})-{}({})={}{}, ATK=LV({})+ST({})+WP({}){}{}{}, AR={}{}",
                                    pkAttacker->GetName(), iAtk, pkVictim->GetName(), iDef, iDam, szUnknownDam,
                                    DEBUG_iLV, DEBUG_iST, DEBUG_iWP, szRB, szUnknownAtk, szGradeAtkBonus, fAR, szPT);

        pkAttacker->ChatPacket(CHAT_TYPE_TALKING, "%s", szMeleeAttack);
        pkVictim->ChatPacket(CHAT_TYPE_TALKING, "%s", szMeleeAttack);
    }

    return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

int CalcArrowDamage(CHARACTER *pkAttacker, CHARACTER *pkVictim, CItem *pkBow, CItem *pkArrow, bool bIgnoreDefense,
                    bool ignoreDistance)
{
    if (!pkBow || pkBow->GetItemType() != ITEM_WEAPON || pkBow->GetSubType() != WEAPON_BOW)
        return 0;

    if (!pkArrow)
        return 0;

    int iDist = DISTANCE_APPROX(pkAttacker->GetX() - pkVictim->GetX(), pkAttacker->GetY() - pkVictim->GetY());

    int iGap = (iDist / 100) - 5 - pkAttacker->GetPoint(POINT_BOW_DISTANCE);
    int iPercent = 100;

    if (!ignoreDistance && pkArrow->GetSubType() != WEAPON_QUIVER)
    {
        iPercent = std::clamp(100 - (iGap * 5), 0, 100);
        if (iPercent == 0)
            return 0;
    }

    PointValue minDam = pkBow->GetValue(3), maxDam = pkBow->GetValue(4);
    int bonusDam = pkBow->GetValue(5) * 2;

    const auto sash = pkAttacker->GetWear(WEAR_COSTUME_ACCE);
    if (sash && sash->GetSocket(1) != 0)
    {
        const auto vnum = sash->GetSocket(1);
        const auto f = sash->GetSocket(0);

        const auto p = ITEM_MANAGER::instance().GetTable(vnum);
        if (p && p->bType == ITEM_WEAPON && p->bSubType == WEAPON_BOW)
        {
            minDam += std::max<int32_t>(0, p->alValues[3] * f / 100);
            maxDam += std::max<int32_t>(0, p->alValues[4] * f / 100);
            bonusDam += std::max<int32_t>(0, p->alValues[5] * f / 100);
        }
    }

    PointValue iDam = Random::get(minDam, maxDam) * 2 + pkArrow->GetValue(3);

    float fAR = CalcAttackRating(pkAttacker, pkVictim, false);

    // level must be ignored when multiply by fAR, so subtract it before calculation.
    PointValue iAtk = (pkAttacker->GetPoint(POINT_ATT_GRADE) + pkAttacker->GetPoint(POINT_ATT_GRADE_BONUS)) + iDam -
                      (pkAttacker->GetLevel() * 2);
    iAtk = (int)(iAtk * fAR);
    iAtk += pkAttacker->GetLevel() * 2; // and add again

    // Refine Grade
    iAtk += bonusDam;

    iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);
    iAtk =
        (int)(iAtk *
              (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) /
              100);

    iAtk = ApplyAttackBonus(pkAttacker, pkVictim, iAtk);
    iAtk = ApplyResistBonus(pkAttacker, pkVictim, iAtk);

    int iDef = 0;

    if (!bIgnoreDefense)
        iDef = (pkVictim->GetPoint(POINT_DEF_GRADE) * (100.0 + pkAttacker->GetPoint(POINT_DEF_BONUS)) / 100.0);

    if (pkAttacker->IsNPC())
        iAtk = (int)(iAtk * pkAttacker->GetMobDamageMultiply());

    PointValue iPureDam = (std::max(0.0, iAtk - iDef) * iPercent) / 100.0;

    if (gConfig.testServer && gConfig.damageDebug)
    {
        SendChatPacket(pkAttacker, CHAT_TYPE_INFO,
                            fmt::format("ARROW {} -> {}, DAM {} DIST {} GAP "
                                        "{}% {}",
                                        pkAttacker->GetName(),
                               pkVictim->GetName(), iPureDam, iDist, iGap, iPercent));
    }

    return iPureDam;
}

void NormalAttackAffect(CHARACTER *pkAttacker, CHARACTER *pkVictim)
{
    // µ¶ °ø°ÝÀº Æ¯ÀÌÇÏ¹Ç·Î Æ¯¼ö Ã³¸®
    if (pkAttacker->GetPoint(POINT_POISON_PCT) > 0 && !pkVictim->FindAffect(AFFECT_POISON))
    {
        if (Random::get(1, 100) <= pkAttacker->GetPoint(POINT_POISON_PCT))
            pkVictim->AttackedByPoison(pkAttacker);
    }

    int iStunDuration = 2;
    if (pkAttacker->IsPC() && !pkVictim->IsPC())
        iStunDuration = 4;

    AttackAffect(pkAttacker, pkVictim, POINT_STUN_PCT, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, iStunDuration, "STUN");
    AttackAffect(pkAttacker, pkVictim, POINT_SLOW_PCT, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, 20, "SLOW");
}

void AttackAffect(CHARACTER *pkAttacker, CHARACTER *pkVictim, uint8_t att_point, uint32_t immune_flag,
                  uint32_t affect_idx, uint8_t affect_point, int32_t affect_amount, int time, const char *name)
{
    if (pkAttacker->GetPoint(att_point) && !pkVictim->FindAffect(affect_idx))
    {
        if (Random::get(1, 100) <= pkAttacker->GetPoint(att_point) && !pkVictim->IsImmune(immune_flag))
        {
            pkVictim->AddAffect(affect_idx, affect_point, affect_amount, time, 0, true);

            if (gConfig.testServer)
                pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s %s(%d%%) SUCCESS", pkAttacker->GetName(), name,
                                     pkAttacker->GetPoint(att_point));
        }
        else if (gConfig.testServer)
        {
            pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s %s(%d%%) FAIL", pkAttacker->GetName(), name,
                                 pkAttacker->GetPoint(att_point));
        }
    }
}

void SkillAttackAffect(CHARACTER *pkVictim, int success_pct, uint32_t immune_flag, uint32_t affect_idx,
                       uint8_t affect_point, int32_t affect_amount, int time, const char *name, bool force)
{
    if (success_pct && !pkVictim->FindAffect(affect_idx))
    {
        if (force || (Random::get(1, 1000) <= success_pct && !pkVictim->IsImmune(immune_flag)))
        {
            pkVictim->AddAffect(affect_idx, affect_point, affect_amount, time, 0, true);

            if (gConfig.testServer)
                pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s(%d%%) -> %s SUCCESS", name, success_pct, name);
        }
        else if (gConfig.testServer)
        {
            pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s(%d%%) -> %s FAIL", name, success_pct, name);
        }
    }
}

int battle_hit(CHARACTER *pkAttacker, CHARACTER *pkVictim)
{
    PointValue iDam = CalcMeleeDamage(pkAttacker, pkVictim);
    if (iDam <= 0.0)
        return BATTLE_DAMAGE;

    if (pkVictim->IsMonster())
    {
        if (IS_SET(pkVictim->GetMobTable().dwAIFlag, AIFLAG_RATTSPEED))
        {
            if (Random::get<bool>(0.5))
            {
                pkAttacker->AddAffect(AFFECT_ATT_SPEED_SLOW, POINT_ATT_SPEED, -20, 20, 0, false);
            }
        }

        if (IS_SET(pkVictim->GetMobTable().dwAIFlag, AIFLAG_RCASTSPEED))
        {
            if (Random::get<bool>(0.5))
            {
                pkAttacker->AddAffect(AFFECT_REDUCE_CAST_SPEED, POINT_CASTING_SPEED, -20, 20, 0, false);
            }
        }

        if (IS_SET(pkVictim->GetMobTable().dwAIFlag, AIFLAG_RHP_REGEN))
        {
            if (Random::get<bool>(0.5)) { pkAttacker->AddAffect(AFFECT_NO_RECOVERY, POINT_NONE, 0, 20, 0, false); }
        }
    }

    NormalAttackAffect(pkAttacker, pkVictim);

    // 데미지 계산
    // iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST)) / 100.0f;
    CItem *pkWeapon = pkAttacker->GetWear(WEAR_WEAPON);

    if (pkWeapon)
    {
        switch (pkWeapon->GetSubType())
        {
        case WEAPON_SWORD:
            iDam = iDam * (100.0 - pkVictim->GetPoint(POINT_RESIST_SWORD)) / 100.0;
            break;

        case WEAPON_TWO_HANDED:
            iDam = iDam * (100.0 - pkVictim->GetPoint(POINT_RESIST_TWOHAND)) / 100.0;
            break;

        case WEAPON_DAGGER:
            iDam = iDam * (100.0 - pkVictim->GetPoint(POINT_RESIST_DAGGER)) / 100.0;
            break;

        case WEAPON_FAN:
        case WEAPON_BELL:
            iDam = iDam * (100.0 - pkVictim->GetPoint(POINT_RESIST_BELL)) / 100.0;
            break;
            /*
            iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_FAN)) / 100.0f;
            break;
            */
        case WEAPON_BOW:
            iDam = iDam * (100.0 - pkVictim->GetPoint(POINT_RESIST_BOW)) / 100.0;
            break;

        case WEAPON_CLAW:
            iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_CLAW)) / 100.0f;
            break;

        case WEAPON_BOUQUET:
            iDam = 0;
            break;
        }
    }
    else
    {
        if(pkAttacker->IsPC()) {
          iDam = iDam * (100.0 - 99.0) / 100.0;
        }
    }

    iDam = pkAttacker->GetAttMul() * iDam + 0.5;

    SPDLOG_DEBUG("{0} hits {1}: dam {2}", pkAttacker->GetName(), pkVictim->GetName(), iDam);

    if (pkVictim->Damage(pkAttacker, iDam, DAMAGE_TYPE_NORMAL))
        return BATTLE_DEAD;

    return BATTLE_DAMAGE;
}

uint32_t GetAttackMotionInputTime(CHARACTER *ch, uint32_t motionKey)
{
    STORM_ASSERT(ch != nullptr, "Parameter check");

    const auto id = MakeMotionId(motionKey);
    const auto* motion = GetMotionManager().Get(ch->GetRaceNum(), motionKey);
    if (!motion)
    {
        SPDLOG_ERROR("Failed to find combo-motion for {0}: {1} {2} {3}", ch->GetRaceNum(), id.mode, id.index,
                     id.subIndex);
        return 1000;
    }

    const uint32_t speed = gConfig.speedHackLimitBonus + ch->GetLimitPoint(POINT_ATT_SPEED);

    if (id.index == MOTION_NORMAL_ATTACK)
        return CalculateDuration(speed, motion->duration) * 90 / 100;

    return CalculateDuration(speed, motion->inputTime);
}

uint32_t GetAttackMotionDuration(CHARACTER *ch, uint32_t motionKey)
{
    STORM_ASSERT(ch != nullptr, "Parameter check");

    const auto* motion = GetMotionManager().Get(ch->GetRaceNum(), motionKey);
    if (!motion)
    {
        const auto id = MakeMotionId(motionKey);
        SPDLOG_ERROR("Failed to find combo-motion for {0}: {1} {2} {3}", ch->GetRaceNum(), id.mode, id.index,
                     id.subIndex);
        return 1000;
    }

    return CalculateDuration(ch->GetLimitPoint(POINT_ATT_SPEED), motion->duration);
}

void SET_ATTACK_TIME(CHARACTER *ch, CHARACTER *victim, uint32_t current_time)
{
    if (nullptr == ch || nullptr == victim)
        return;

    if (!ch->IsPC())
        return;

    ch->m_kAttackLog.dwVID = victim->GetVID();
    ch->m_kAttackLog.dwTime = current_time;
}

void SET_ATTACKED_TIME(CHARACTER *ch, CHARACTER *victim, uint32_t current_time)
{
    if (nullptr == ch || nullptr == victim)
        return;

    if (!ch->IsPC())
        return;

    victim->m_AttackedLog.dwPID = ch->GetPlayerID();
    victim->m_AttackedLog.dwAttackedTime = current_time;
}

uint32_t GET_ATTACK_SPEED(CHARACTER *ch, uint32_t motionKey)
{
    STORM_ASSERT(ch != nullptr, "Parameter check");

    // 유두리 공속(기본 80)
    uint32_t default_bonus = gConfig.speedHackLimitBonus;

    // 뭔가를 탔으면 추가공속 50
    uint32_t riding_bonus = ch->IsRiding() ? 50 : 0;

    const auto* motion = GetMotionManager().Get(ch->GetRaceNum(), motionKey);
    if (!motion)
    {
        const auto id = MakeMotionId(motionKey);
        SPDLOG_ERROR("Failed to find combo-motion for {0}: {1} {2} {3}", ch->GetRaceNum(), id.mode, id.index,
                     id.subIndex);
        return 1000;
    }

    uint32_t aniSpeed = default_bonus + ch->GetPoint(POINT_ATT_SPEED) + riding_bonus;
    uint32_t speed = CalculateDuration(aniSpeed, motion->duration);

    // 단검의 경우 공속 2배
    CItem *item = ch->GetWear(WEAR_WEAPON);
    if (item && (item->GetSubType() == WEAPON_DAGGER || item->GetSubType() == WEAPON_CLAW))
        speed /= 2;

    return speed;
}

bool IS_SPEED_HACK(CHARACTER *ch, CHARACTER *victim, uint32_t current_time, uint32_t motionKey)
{
    // Minimum time between successive attacks
    auto limit = GetAttackMotionInputTime(ch, motionKey);
    CItem *item = ch->GetWear(WEAR_WEAPON);
    if (item && (item->GetSubType() == WEAPON_DAGGER || item->GetSubType() == WEAPON_CLAW))
        limit /= 2;

    if (ch->m_kAttackLog.dwVID == victim->GetVID())
    {
        const auto delta = current_time - ch->m_kAttackLog.dwTime;
        if (delta < limit)
        {
            INCREASE_SPEED_HACK_COUNT(ch);

            SPDLOG_DEBUG("{0}: target attack speed hack: delta {1} limit {2}", ch->GetName(), delta, limit,
                          ch->m_speed_hack_count);

            if (gConfig.testServer && gConfig.damageDebug)
            {
                ch->ChatPacket(CHAT_TYPE_INFO, "%s: target attack speed hack: delta %u limit %u count %u",
                               ch->GetName(), delta, limit, ch->m_speed_hack_count);
            }

            SET_ATTACK_TIME(ch, victim, current_time);
            SET_ATTACKED_TIME(ch, victim, current_time);
            return true;
        }
    }

    SET_ATTACK_TIME(ch, victim, current_time);

    if (victim->m_AttackedLog.dwPID == ch->GetPlayerID())
    {
        const auto delta = current_time - victim->m_AttackedLog.dwAttackedTime;
        if (delta < limit)
        {
            INCREASE_SPEED_HACK_COUNT(ch);

            SPDLOG_DEBUG("{0}: victim attack speed hack: delta {1} limit {2}", ch->GetName(), delta, limit,
                          ch->m_speed_hack_count);

            if (gConfig.testServer && gConfig.damageDebug)
            {
                ch->ChatPacket(CHAT_TYPE_INFO, "%s: victim attack speed hack: delta %u limit %u count %u",
                               ch->GetName(), delta, limit, ch->m_speed_hack_count);
            }

            SET_ATTACKED_TIME(ch, victim, current_time);
            return true;
        }
    }

    SET_ATTACKED_TIME(ch, victim, current_time);
    return false;
}
