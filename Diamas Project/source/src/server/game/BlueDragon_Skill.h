#ifndef METIN2_SERVER_GAME_BLUEDRAGON_SKILL_H
#define METIN2_SERVER_GAME_BLUEDRAGON_SKILL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "motion.h"

struct FSkillBreath
{
    EJobs Set1;
    EJobs Set2;
    ESex gender;
    CHARACTER *pAttacker;

    FSkillBreath(CHARACTER *p)
    {
        pAttacker = p;

        Set1 = static_cast<EJobs>(Random::get(0, 3));
        Set2 = static_cast<EJobs>(Random::get(0, 3));
        gender = static_cast<ESex>(Random::get(0, 2));
    }

    void operator()(CEntity *ent)
    {
        if (nullptr != ent)
        {
            if (true == ent->IsType(ENTITY_CHARACTER))
            {
                CHARACTER *ch = static_cast<CHARACTER *>(ent);

                if (true == ch->IsPC() && false == ch->IsDead())
                {
                    if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
                        return;

                    if ((signed)BlueDragon_GetSkillFactor(2, "Skill0", "damage_area") < DISTANCE_APPROX(
                            pAttacker->GetX() - ch->GetX(), pAttacker->GetY() - ch->GetY()))
                    {
                        SPDLOG_TRACE("BlueDragon: Breath too far (%d)",
                                     DISTANCE_APPROX(pAttacker->GetX()-ch->GetX(), pAttacker->GetY()-ch->GetY()));
                        return;
                    }

                    int overlapDamageCount = 0;

                    int pct = 0;
                    if (ch->GetJob() == Set1)
                    {
                        const char *ptr = nullptr;

                        switch (Set1)
                        {
                        case JOB_WARRIOR:
                            ptr = "musa";
                            break;
                        case JOB_ASSASSIN:
                            ptr = "assa";
                            break;
                        case JOB_SURA:
                            ptr = "sura";
                            break;
                        case JOB_SHAMAN:
                            ptr = "muda";
                            break;
                        case JOB_WOLFMAN:
                            ptr = "wolf";
                            break; // TODO: 블루드래곤 수인족 전투 처리
                        default:
                        case JOB_MAX_NUM:
                            return;
                        }

                        int firstDamagePercent = Random::get(
                            BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "min"),
                            BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "max"));
                        pct += firstDamagePercent;

                        if (firstDamagePercent > 0)
                            overlapDamageCount++;
                    }

                    if (ch->GetJob() == Set2)
                    {
                        const char *ptr = nullptr;

                        switch (Set2)
                        {
                        case JOB_WARRIOR:
                            ptr = "musa";
                            break;
                        case JOB_ASSASSIN:
                            ptr = "assa";
                            break;
                        case JOB_SURA:
                            ptr = "sura";
                            break;
                        case JOB_SHAMAN:
                            ptr = "muda";
                            break;
                        case JOB_WOLFMAN:
                            ptr = "wolf";
                            break; // TODO: 블루드래곤 수인족 전투 처리
                        default:
                            return;
                        }

                        int secondDamagePercent = Random::get(
                            BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "min"),
                            BlueDragon_GetSkillFactor(4, "Skill0", "damage", ptr, "max"));
                        pct += secondDamagePercent;

                        if (secondDamagePercent > 0)
                            overlapDamageCount++;
                    }

                    if (GetSexByRace(ch->GetRaceNum()) == gender)
                    {
                        const char *ptr = nullptr;

                        switch (gender)
                        {
                        case SEX_MALE:
                            ptr = "male";
                            break;
                        case SEX_FEMALE:
                            ptr = "female";
                            break;
                        default:
                            return;
                        }

                        int thirdDamagePercent = Random::get(
                            BlueDragon_GetSkillFactor(4, "Skill0", "gender", ptr, "min"),
                            BlueDragon_GetSkillFactor(4, "Skill0", "gender", ptr, "max"));
                        pct += thirdDamagePercent;

                        if (thirdDamagePercent > 0)
                            overlapDamageCount++;
                    }

                    switch (overlapDamageCount)
                    {
                    case 1:
                        ch->EffectPacket(SE_PERCENT_DAMAGE1);
                        break;
                    case 2:
                        ch->EffectPacket(SE_PERCENT_DAMAGE2);
                        break;
                    case 3:
                        ch->EffectPacket(SE_PERCENT_DAMAGE3);
                        break;
                    }

                    int addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

                    pct += addPct;

                    int dam = Random::get(BlueDragon_GetSkillFactor(3, "Skill0", "default_damage", "min"),
                                          BlueDragon_GetSkillFactor(3, "Skill0", "default_damage", "max"));

                    dam += (dam * addPct) / 100;
                    dam += (ch->GetMaxHP() * pct) / 100;

                    ch->Damage(pAttacker, dam, DAMAGE_TYPE_ICE);

                    SPDLOG_TRACE(
                        "BlueDragon: Breath to {} pct({}) dam({}) "
                        "overlap({})",
                        ch->GetName(), pct, dam, overlapDamageCount);
                }
            }
        }
    }
};

struct FSkillWeakBreath
{
    CHARACTER *pAttacker;

    FSkillWeakBreath(CHARACTER *p) { pAttacker = p; }

    void operator()(CEntity *ent)
    {
        if (nullptr != ent)
        {
            if (true == ent->IsType(ENTITY_CHARACTER))
            {
                CHARACTER *ch = static_cast<CHARACTER *>(ent);

                if (true == ch->IsPC() && false == ch->IsDead())
                {
                    if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
                        return;

                    if ((signed)BlueDragon_GetSkillFactor(2, "Skill1", "damage_area") < DISTANCE_APPROX(
                            pAttacker->GetX() - ch->GetX(), pAttacker->GetY() - ch->GetY()))
                    {
                        SPDLOG_TRACE("BlueDragon: Breath too far (%d)",
                                     DISTANCE_APPROX(pAttacker->GetX() - ch->GetX(),
                                         pAttacker->GetY() - ch->GetY()));
                        return;
                    }

                    int addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

                    int dam = Random::get(BlueDragon_GetSkillFactor(3, "Skill1", "default_damage", "min"),
                                          BlueDragon_GetSkillFactor(3, "Skill1", "default_damage", "max"));
                    dam += (dam * addPct) / 100;

                    ch->Damage(pAttacker, dam, DAMAGE_TYPE_ICE);

                    SPDLOG_TRACE(
                        "BlueDragon: WeakBreath to %s addPct(%d) dam(%d)",
                        ch->GetName(), addPct, dam);
                }
            }
        }
    }
};

struct FSkillEarthQuake
{
    EJobs Set1;
    EJobs Set2;
    ESex gender;
    long MaxDistance;
    CHARACTER *pAttacker;
    CHARACTER *pFarthestChar;

    FSkillEarthQuake(CHARACTER *p)
    {
        pAttacker = p;

        MaxDistance = 0;
        pFarthestChar = nullptr;

        Set1 = static_cast<EJobs>(Random::get(0, 3));
        Set2 = static_cast<EJobs>(Random::get(0, 3));
        gender = static_cast<ESex>(Random::get(0, 2));
    }

    void operator()(CEntity *ent)
    {
        if (nullptr != ent)
        {
            if (true == ent->IsType(ENTITY_CHARACTER))
            {
                CHARACTER *ch = static_cast<CHARACTER *>(ent);

                if (true == ch->IsPC() && false == ch->IsDead())
                {
                    if (nullptr != ch->FindAffect(AFFECT_REVIVE_INVISIBLE, APPLY_NONE))
                        return;

                    if ((signed)BlueDragon_GetSkillFactor(2, "Skill2", "damage_area") < DISTANCE_APPROX(
                            pAttacker->GetX() - ch->GetX(), pAttacker->GetY() - ch->GetY()))
                    {
                        SPDLOG_TRACE("BlueDragon: Breath too far (%d)",
                                     DISTANCE_APPROX(pAttacker->GetX() - ch->GetX(),
                                         pAttacker->GetY() - ch->GetY()));
                        return;
                    }

                    int sec = Random::get(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", "default", "min"),
                                          BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", "default", "max"));

                    if (ch->GetJob() == Set1)
                    {
                        const char *ptr = nullptr;

                        switch (Set1)
                        {
                        case JOB_WARRIOR:
                            ptr = "musa";
                            break;
                        case JOB_ASSASSIN:
                            ptr = "assa";
                            break;
                        case JOB_SURA:
                            ptr = "sura";
                            break;
                        case JOB_SHAMAN:
                            ptr = "muda";
                            break;
                        case JOB_WOLFMAN:
                            ptr = "wolf";
                            break; // TODO: 블루드래곤 수인족 전투 처리
                        default:
                        case JOB_MAX_NUM:
                            return;
                        }

                        sec += Random::get(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "min"),
                                           BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "max"));
                    }

                    if (ch->GetJob() == Set2)
                    {
                        const char *ptr = nullptr;

                        switch (Set2)
                        {
                        case JOB_WARRIOR:
                            ptr = "musa";
                            break;
                        case JOB_ASSASSIN:
                            ptr = "assa";
                            break;
                        case JOB_SURA:
                            ptr = "sura";
                            break;
                        case JOB_SHAMAN:
                            ptr = "muda";
                            break;
                        case JOB_WOLFMAN:
                            ptr = "wolf";
                            break; // TODO: 블루드래곤 수인족 전투 처리
                        default:
                        case JOB_MAX_NUM:
                            return;
                        }

                        sec += Random::get(BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "min"),
                                           BlueDragon_GetSkillFactor(4, "Skill2", "stun_time", ptr, "max"));
                    }

                    if (GetSexByRace(ch->GetRaceNum()) == gender)
                    {
                        const char *ptr = nullptr;

                        switch (gender)
                        {
                        case SEX_MALE:
                            ptr = "male";
                            break;
                        case SEX_FEMALE:
                            ptr = "female";
                            break;
                        default:
                            return;
                        }

                        sec += Random::get(BlueDragon_GetSkillFactor(4, "Skill2", "gender", ptr, "min"),
                                           BlueDragon_GetSkillFactor(4, "Skill2", "gender", ptr, "max"));
                    }

                    int addPct = BlueDragon_GetRangeFactor("hp_damage", pAttacker->GetHPPct());

                    int dam = Random::get(BlueDragon_GetSkillFactor(3, "Skill2", "default_damage", "min"),
                                          BlueDragon_GetSkillFactor(3, "Skill2", "default_damage", "max"));
                    dam += (dam * addPct) / 100;

                    ch->Damage(pAttacker, dam, DAMAGE_TYPE_ICE);

                    SkillAttackAffect(ch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, sec, "BDRAGON_STUN");

                    SPDLOG_TRACE(
                        "BlueDragon: EarthQuake to %s addPct(%d) dam(%d) "
                        "sec(%d)",
                        ch->GetName(), addPct, dam, sec);

                    Eigen::Vector3f vec;
                    vec.x() = static_cast<float>(pAttacker->GetX() - ch->GetX());
                    vec.y() = static_cast<float>(pAttacker->GetY() - ch->GetY());
                    vec.z() = 0.0f;

                    vec.normalize();

                    const int nFlyDistance = 1000;

                    long tx = (long)(ch->GetX() + vec.x() * nFlyDistance);
                    long ty = (long)(ch->GetY() + vec.y() * nFlyDistance);

                    for (int i = 0; i < 5; ++i)
                    {
                        if (true == SECTREE_MANAGER::instance().IsMovablePosition(ch->GetMapIndex(), tx, ty)) { break; }

                        switch (i)
                        {
                        case 0:
                            tx = (long)(ch->GetX() + vec.x() * nFlyDistance * -1);
                            ty = (long)(ch->GetY() + vec.y() * nFlyDistance * -1);
                            break;
                        case 1:
                            tx = (long)(ch->GetX() + vec.x() * nFlyDistance * -1);
                            ty = (long)(ch->GetY() + vec.y() * nFlyDistance);
                            break;
                        case 2:
                            tx = (long)(ch->GetX() + vec.x() * nFlyDistance);
                            ty = (long)(ch->GetY() + vec.y() * nFlyDistance * -1);
                            break;
                        case 3:
                            tx = (long)(ch->GetX() + vec.x() * Random::get(1, 100));
                            ty = (long)(ch->GetY() + vec.y() * Random::get(1, 100));
                            break;
                        case 4:
                            tx = (long)(ch->GetX() + vec.x() * Random::get(1, 10));
                            ty = (long)(ch->GetY() + vec.y() * Random::get(1, 10));
                            break;
                        }
                    }

                    ch->Goto(tx, ty);

                    ch->Sync(tx, ty);
                    ch->SyncPacket();

                    int32_t dist = DISTANCE_APPROX(pAttacker->GetX() - ch->GetX(),
                                                   pAttacker->GetY() - ch->GetY());

                    if (dist > MaxDistance)
                    {
                        MaxDistance = dist;
                        pFarthestChar = ch;
                    }
                }
            }
        }
    }
};

#endif /* METIN2_SERVER_GAME_BLUEDRAGON_SKILL_H */
