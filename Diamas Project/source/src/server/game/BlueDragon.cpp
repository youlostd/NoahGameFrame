#include "BlueDragon.h"
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include "main.h"

#include "vector.h"
#include "utils.h"
#include "char.h"
#include "mob_manager.h"
#include "sectree_manager.h"
#include "GBattle.h"
#include <game/AffectConstants.hpp>
#include "BlueDragon_Binder.h"
#include "BlueDragon_Skill.h"
#include <game/GamePacket.hpp>
#include "motion.h"

time_t UseBlueDragonSkill(CHARACTER *pChar, unsigned int idx)
{
    SECTREE_MAP *pSecMap = SECTREE_MANAGER::instance().GetMap(pChar->GetMapIndex());

    if (nullptr == pSecMap)
        return 0;

    int nextUsingTime = 0;

    switch (idx)
    {
    case 0: {
        SPDLOG_DEBUG("BlueDragon: Using Skill Breath");

        FSkillBreath f(pChar);

        pSecMap->for_each(f);

        nextUsingTime = Random::get(BlueDragon_GetSkillFactor(3, "Skill0", "period", "min"),
                                    BlueDragon_GetSkillFactor(3, "Skill0", "period", "max"));
    }
    break;

    case 1: {
        SPDLOG_DEBUG("BlueDragon: Using Skill Weak Breath");

        FSkillWeakBreath f(pChar);

        pSecMap->for_each(f);

        nextUsingTime = Random::get(BlueDragon_GetSkillFactor(3, "Skill1", "period", "min"),
                                    BlueDragon_GetSkillFactor(3, "Skill1", "period", "max"));
    }
    break;

    case 2: {
        SPDLOG_DEBUG("BlueDragon: Using Skill EarthQuake");

        FSkillEarthQuake f(pChar);

        pSecMap->for_each(f);

        nextUsingTime = Random::get(BlueDragon_GetSkillFactor(3, "Skill2", "period", "min"),
                                    BlueDragon_GetSkillFactor(3, "Skill2", "period", "max"));

        if (nullptr != f.pFarthestChar) { pChar->BeginFight(f.pFarthestChar); }
    }
    break;

    default:
        SPDLOG_ERROR("BlueDragon: Wrong Skill Index: {0}", idx);
        return 0;
    }

    auto addPct = BlueDragon_GetRangeFactor("hp_period", pChar->GetHPPct());

    nextUsingTime += (nextUsingTime * addPct) / 100;

    return nextUsingTime;
}

int BlueDragon_StateBattle(CHARACTER *pChar)
{
    if (pChar->GetHPPct() > 98)
        return THECORE_SECS_TO_PASSES(1);

    const int SkillCount = 3;
    int SkillPriority[SkillCount];

    if (pChar->GetHPPct() > 76)
    {
        SkillPriority[0] = 1;
        SkillPriority[1] = 0;
        SkillPriority[2] = 2;
    }
    else if (pChar->GetHPPct() > 31)
    {
        SkillPriority[0] = 0;
        SkillPriority[1] = 1;
        SkillPriority[2] = 2;
    }
    else
    {
        SkillPriority[0] = 0;
        SkillPriority[1] = 2;
        SkillPriority[2] = 1;
    }

    auto timeNow = static_cast<time_t>(get_dword_time());
    auto &motionManager = GetMotionManager();
    for (int i = 0; i < SkillCount; ++i)
    {
        const int SkillIndex = SkillPriority[i];

        if (pChar->GetMobSkillCooldown(SkillIndex) < timeNow)
        {
            int SkillUsingDuration = 0;

            auto key = MakeMotionKey(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + SkillIndex);
            const auto* motion = motionManager.Get(pChar->GetRaceNum(), key);
            if (motion)
                SkillUsingDuration = motion->duration / 1000;

            pChar->TouchMobSkillCooldown(
                SkillIndex, timeNow + (UseBlueDragonSkill(pChar, SkillIndex) * 1000) + SkillUsingDuration + 3000);

            pChar->SendMovePacket(FUNC_MOB_SKILL, SkillIndex, pChar->GetX(),
                                  pChar->GetY(), 0, timeNow);

            return 0 == SkillUsingDuration ? THECORE_SECS_TO_PASSES(1) : THECORE_SECS_TO_PASSES(SkillUsingDuration);
        }
    }

    return THECORE_SECS_TO_PASSES(1);
}

int BlueDragon_Damage(CHARACTER *me, CHARACTER *pAttacker, int dam)
{
    if (nullptr == me || nullptr == pAttacker)
        return dam;

    if (true == pAttacker->IsMonster() && 2493 == pAttacker->GetMobTable().dwVnum)
    {
        for (int i = 1; i <= 4; ++i)
        {
            if (ATK_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
            {
                uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
                size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
                size_t cnt = SECTREE_MANAGER::instance().
                    GetMonsterCountInMap(pAttacker->GetMapIndex(), dwDragonStoneID);

                dam += dam * (val * cnt) / 100;

                break;
            }
        }
    }

    if (true == me->IsMonster() && 2493 == me->GetMobTable().dwVnum)
    {
        for (int i = 1; i <= 4; ++i)
        {
            if (DEF_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
            {
                uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
                size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
                size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap(me->GetMapIndex(), dwDragonStoneID);

                dam -= dam * (val * cnt) / 100;

                if (dam <= 0)
                    dam = 1;

                break;
            }
        }
    }

    if (true == me->IsStone() && 0 != pAttacker->GetMountVnum())
    {
        for (int i = 1; i <= 4; ++i)
        {
            if (me->GetMobTable().dwVnum == BlueDragon_GetIndexFactor("DragonStone", i, "vnum"))
            {
                if (pAttacker->GetMountVnum() == BlueDragon_GetIndexFactor("DragonStone", i, "enemy"))
                {
                    size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "enemy_val");

                    dam *= val;

                    break;
                }
            }
        }
    }

    return dam;
}
