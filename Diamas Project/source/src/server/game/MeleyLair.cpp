#include "MeleyLair.h"

#include "ChatUtil.hpp"

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "db.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "utils.h"
#include "party.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "regen.h"
#include "config.h"
#include "motion.h"
#include "item_manager.h"
#include "guild_manager.h"
#include "start_position.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "locale_service.h"
#include "desc.h"
#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>

namespace MeleyLair
{
int stoneSpawnPos[MOBCOUNT_RESPAWN_STONE_STEP2][2] =
{
    {140, 131},
    {130, 122},
    {121, 128},
    {128, 140},
    {130, 150},
    {147, 128},
    {125, 117},
    {112, 131},

};

int monsterSpawnPos[MOBCOUNT_RESPAWN_COMMON_STEP][2] =
{
    {140, 131},
    {130, 122},
    {121, 128},
    {128, 140},
    {128, 142},
    {128, 131},
    {135, 130},
    {141, 126},
    {128, 122},
    {117, 127},
    {118, 136},
    {126, 142}
};

void RemoveStatueAffects(CHARACTER *ch)
{
    ch->RemoveAffect(AFFECT_STATUE_1);
    ch->RemoveAffect(AFFECT_STATUE_2);
    ch->RemoveAffect(AFFECT_STATUE_3);
    ch->RemoveAffect(AFFECT_STATUE_4);
}

EVENTINFO(r_meleystatues_info)
{
    bool bFirst;
    CMgrMap *pMap;
};

EVENTFUNC(r_meleystatues_event)
{
    auto *pEventInfo = dynamic_cast<r_meleystatues_info *>(event->info);
    if (pEventInfo)
    {
        if (pEventInfo->bFirst)
        {
            CMgrMap *pMap = pEventInfo->pMap;
            if (pMap)
                pMap->DungeonResult();
        }
        else
        {
            pEventInfo->bFirst = true;
            return THECORE_SECS_TO_PASSES(TIME_LIMIT_TO_KILL_STATUE);
        }
    }

    return 0;
}

EVENTINFO(r_meleylimit_info)
{
    bool bWarp;
    CMgrMap *pMap;
};

EVENTFUNC(r_meleylimit_event)
{
    auto *pEventInfo = dynamic_cast<r_meleylimit_info *>(event->info);
    if (pEventInfo)
    {
        CMgrMap *pMap = pEventInfo->pMap;
        if (pMap)
        {
            if (pEventInfo->bWarp)
                pMap->EndDungeonWarp();
            else
                pMap->EndDungeon(false, false);
        }
    }

    return 0;
}

EVENTINFO(r_meleyspawn_info)
{
    CMgrMap *pMap;
    bool bFirst;
    uint8_t bStep;
    uint32_t dwTimeReload, dwMobVnum, dwMobCount;
};

EVENTFUNC(r_meleyspawn_event)
{
    auto *pEventInfo = dynamic_cast<r_meleyspawn_info *>(event->info);
    if (!pEventInfo)
        return 0;

    CMgrMap *pMap = pEventInfo->pMap;
    if (!pMap)
        return 0;

    if (!pMap->GetMapSectree())
        return 0;

    const auto evTimeReload = pEventInfo->dwTimeReload;
    const auto evMobVnum = pEventInfo->dwMobVnum;
    const auto evMobCount = pEventInfo->dwMobCount;

    const auto mobCount = SECTREE_MANAGER::instance().GetMonsterCountInMap(pMap->GetMapIndex(), evMobVnum, true);

    if (evMobCount > mobCount)
    {
        // Respawn mobs when some where killed
        uint32_t dwDiff = evMobCount - mobCount;
        for (uint32_t i = 0; i < dwDiff; ++i)
        {
            uint32_t dwRandom = Random::get(3, 10);
            pMap->Spawn(evMobVnum, monsterSpawnPos[i][0], monsterSpawnPos[i][1] + dwRandom, 0);
        }
    }

    if (pEventInfo->bStep == 2 || pEventInfo->bStep == 3)
    {
        size_t stones_count = SECTREE_MANAGER::instance().GetMonsterCountInMap(
            pMap->GetMapIndex(), (uint32_t)MeleyLair::MOBVNUM_RESPAWN_STONE_STEP2, true);
        if (stones_count == 0)
        {
            uint32_t dwTime = get_global_time();
            if (dwTime >= pMap->GetLastStoneKilledTime())
            {
                for (uint32_t i = 0; i < MOBCOUNT_RESPAWN_STONE_STEP2; ++i)
                {
                    uint32_t dwRandom = Random::get(3, 10);
                    pMap->Spawn((uint32_t)MeleyLair::MOBVNUM_RESPAWN_STONE_STEP2, stoneSpawnPos[i][0],
                                stoneSpawnPos[i][1] + dwRandom, 0);
                }

                if (pEventInfo->bStep == 3)
                {
                    for (auto i = 0; i < 8; ++i)
                    {
                        if (!pMap->GetStatueChar(i)->FindAffect(AFFECT_STATUE_3))
                        {
                            RemoveStatueAffects(pMap->GetStatueChar(i));
                            pMap->GetStatueChar(i)->AddAffect(AFFECT_STATUE_1, POINT_NONE, 0, INFINITE_AFFECT_DURATION,
                              0, true);
                        }
                    }
                    pMap->SetKillCountStones(0);
                }
                else
                {
                    for (auto i = 0; i < 8; ++i)
                    {
                        if (!pMap->GetStatueChar(i)->FindAffect(AFFECT_STATUE_2))
                        {
                            RemoveStatueAffects(pMap->GetStatueChar(i));
                            pMap->GetStatueChar(i)->AddAffect(AFFECT_STATUE_1, POINT_NONE, 0, INFINITE_AFFECT_DURATION,
                              0, true);
                        }
                    }
                }
            }
        }

        if (pEventInfo->bStep == 3 && pEventInfo->bFirst)
        {
            for (auto &stoneSpawnPo : stoneSpawnPos)
            {
                uint32_t dwRandom = Random::get(3, 10);
                pMap->Spawn(MOBVNUM_RESPAWN_BOSS_STEP3, stoneSpawnPo[0],
                            stoneSpawnPo[1] + dwRandom, 0);
                pMap->Spawn(MOBVNUM_RESPAWN_SUBBOSS_STEP3, stoneSpawnPo[0],
                            stoneSpawnPo[1] + dwRandom, 0);
                pMap->Spawn(MOBVNUM_RESPAWN_SUBBOSS_STEP3, stoneSpawnPo[0],
                            stoneSpawnPo[1] + dwRandom, 0);
            }

            pEventInfo->bFirst = false;
        }
    }

    return THECORE_SECS_TO_PASSES(evTimeReload);
}

EVENTINFO(r_meleyeffect_info)
{
    CMgrMap *pMap;
    uint8_t bStep;
    uint8_t bEffectStep;
};

EVENTFUNC(r_meleyeffect_event)
{
    const auto pEventInfo = dynamic_cast<r_meleyeffect_info *>(event->info);
    if (!pEventInfo)
        return 0;

    CMgrMap *pMap = pEventInfo->pMap;
    if (!pMap)
        return 0;

    if (!pMap->GetBossChar() || !pMap->StatuesExist())
        return 0;

    uint8_t bStep = pEventInfo->bStep, bEffectStep = pEventInfo->bEffectStep;
    if (bStep == 1)
    {
        if (bEffectStep == 1)
        {
#ifdef __LASER_EFFECT_ON_75HP__
            auto timeNow = static_cast<time_t>(get_dword_time());

            if (pMap->GetMapSectree())
            {
                PIXEL_POSITION pos = {130 * 100, 77 * 100, 0};
                for (int i = 0; i < 8; ++i) { pMap->GetStatueChar(i)->SetRotationToXY(pos.x, pos.y); }
            }

            for (int i = 0; i < 8; ++i)
            {
                pMap->GetStatueChar(i)->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatueChar(i)->GetX(),
                                                       pMap->GetStatueChar(i)->GetY(), 0, timeNow);
            }

#endif

            pEventInfo->bEffectStep = 2;
            return THECORE_SECS_TO_PASSES(1);
        }
        else
        {
            pMap->StartDungeonStep(2);
            return 0;
        }
    }

    if (bStep == 2)
    {
        if (bEffectStep == 1)
        {
#ifdef __LASER_EFFECT_ON_75HP__
            auto timeNow = static_cast<time_t>(get_dword_time());

            if (pMap->GetMapSectree())
            {
                PIXEL_POSITION pos = {130 * 100, 77 * 100, 0};
                for (int i = 0; i < 8; ++i) { pMap->GetStatueChar(i)->SetRotationToXY(pos.x, pos.y); }
            }

            for (int i = 0; i < 8; ++i)
            {
                pMap->GetStatueChar(i)->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatueChar(i)->GetX(),
                                                       pMap->GetStatueChar(i)->GetY(), 0, timeNow);
            }

#endif

            pEventInfo->bEffectStep = 2;
            return THECORE_SECS_TO_PASSES(1);
        }
        else
        {
            pMap->StartDungeonStep(3);
            return 0;
        }
    }

    return THECORE_SECS_TO_PASSES(1);
}

CMgrMap::CMgrMap(long lMapIndex, uint32_t dwGuildID)
    : m_mapIndex(lMapIndex), m_uniqueId(dwGuildID), m_sectreeMap(SECTREE_MANAGER::instance().GetMap(lMapIndex))
{
    m_participants.clear();
    m_alreadyActed.clear();
    m_alreadyRewarded.clear();
    m_dungeonStep = 0;
    m_reward = 0;

    memset(m_statues, 0, sizeof m_statues);

    Start();
}

CMgrMap::~CMgrMap()
{
    if (e_pEndEvent != nullptr)
        event_cancel(&e_pEndEvent);

    if (e_pWarpEvent != nullptr)
        event_cancel(&e_pWarpEvent);

    if (e_SpawnEvent != nullptr)
        event_cancel(&e_SpawnEvent);

    if (e_SEffectEvent != nullptr)
        event_cancel(&e_SEffectEvent);

    if (e_DestroyStatues != nullptr)
        event_cancel(&e_DestroyStatues);
}

void CMgrMap::Destroy()
{
    if (e_pEndEvent != nullptr)
        event_cancel(&e_pEndEvent);

    if (e_pWarpEvent != nullptr)
        event_cancel(&e_pWarpEvent);

    if (e_SpawnEvent != nullptr)
        event_cancel(&e_SpawnEvent);

    if (e_SEffectEvent != nullptr)
        event_cancel(&e_SEffectEvent);

    if (e_DestroyStatues != nullptr)
        event_cancel(&e_DestroyStatues);

    e_pEndEvent = nullptr;
    e_pWarpEvent = nullptr;
    e_SpawnEvent = nullptr;
    e_SEffectEvent = nullptr;
    e_DestroyStatues = nullptr;
    m_participants.clear();
    m_alreadyActed.clear();
    m_alreadyRewarded.clear();
    m_mapIndex = 0;
    m_uniqueId = 0;
    m_dungeonStep = 0;
    m_reward = 0;
    m_sectreeMap = nullptr;
    m_mainNpc = nullptr, m_gate = nullptr, m_boss = nullptr;
    memset(m_statues, 0, sizeof m_statues);
}

uint32_t CMgrMap::GetParticipantsCount() const { return m_participants.size(); }

void CMgrMap::Participant(bool bInsert, PlayerId pid)
{
    if (bInsert)
    {
        bool bCheck = std::find(m_participants.begin(), m_participants.end(), pid) != m_participants.end();
        if (!bCheck)
            m_participants.push_back(pid);
    }
    else
        m_participants.erase(std::remove(m_participants.begin(), m_participants.end(), pid), m_participants.end());
}

bool CMgrMap::IsParticipant(PlayerId pid)
{
    bool bCheck = std::find(m_participants.begin(), m_participants.end(), pid) != m_participants.end();
    return bCheck;
}

CHARACTER *CMgrMap::Spawn(uint32_t dwVnum, int iX, int iY, int iDir, bool bSpawnMotion)
{
    if (!m_sectreeMap)
        return nullptr;

    CHARACTER *pkMob = g_pCharManager->SpawnMob(dwVnum, GetMapIndex(), iX * 100, iY * 100, 0,
                                                bSpawnMotion, iDir == 0 ? -1 : (iDir - 1) * 45);
    if (pkMob)
        SPDLOG_INFO("<MeleyLair> Spawn: {0} (map index: {1}).", pkMob->GetName(), GetMapIndex());

    return pkMob;
}

void CMgrMap::SetDungeonStep(uint8_t bStep)
{
    if (bStep == m_dungeonStep)
        return;

    m_dungeonStep = bStep;

    if (e_SpawnEvent != nullptr)
        event_cancel(&e_SpawnEvent);

    e_SpawnEvent = nullptr;

    if (bStep == 1)
    {
        SetDungeonTimeStart(get_global_time());
        SetKillCountStones(0);
        auto *pEventInfo = AllocEventInfo<r_meleyspawn_info>();
        pEventInfo->pMap = this;
        pEventInfo->bFirst = false;
        pEventInfo->bStep = bStep;
        pEventInfo->dwTimeReload = TIME_RESPAWN_COMMON_STEP1;
        pEventInfo->dwMobVnum = MOBVNUM_RESPAWN_COMMON_STEP1;
        pEventInfo->dwMobCount = MOBCOUNT_RESPAWN_COMMON_STEP;
        e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, THECORE_SECS_TO_PASSES(1));
    }
    else if (bStep == 2)
    {
        if (!m_boss || !StatuesExist())
            EndDungeon(false, true);

        if (e_SEffectEvent != nullptr)
            event_cancel(&e_SEffectEvent);

        e_SEffectEvent = nullptr;

        auto *pEventInfo = AllocEventInfo<r_meleyeffect_info>();
        pEventInfo->pMap = this;
        pEventInfo->bStep = 1;
        pEventInfo->bEffectStep = 1;
        e_SEffectEvent = event_create(r_meleyeffect_event, pEventInfo, THECORE_SECS_TO_PASSES(1));
    }
    else if (bStep == 3)
    {
        if (!m_boss || !StatuesExist())
            EndDungeon(false, true);

        if (e_SEffectEvent != nullptr)
            event_cancel(&e_SEffectEvent);

        e_SEffectEvent = nullptr;

        auto *pEventInfo = AllocEventInfo<r_meleyeffect_info>();
        pEventInfo->pMap = this;
        pEventInfo->bStep = 2;
        pEventInfo->bEffectStep = 1;
        e_SEffectEvent = event_create(r_meleyeffect_event, pEventInfo, THECORE_SECS_TO_PASSES(1));
    }
    else if (bStep == 4)
    {
        if (!m_boss || !StatuesExist())
            EndDungeon(false, true);

        if (e_SEffectEvent != nullptr)
            event_cancel(&e_SEffectEvent);

        if (e_DestroyStatues != nullptr)
            event_cancel(&e_DestroyStatues);

        e_SEffectEvent = nullptr;
        e_DestroyStatues = nullptr;

        m_alreadyActed.clear();

        for (auto &statue : m_statues) { statue->SetArmada(); }

        GetMapSectree()->for_each([&](CEntity *ent)
        {
            if (ent->IsType(ENTITY_CHARACTER))
            {
                auto *pkChar = static_cast<CHARACTER *>(ent);
                if (pkChar)
                {
                    SendI18nChatPacket(pkChar, CHAT_TYPE_NOTICE,
                                       "You have %d second(s) to destroy the statues, hurry up!",
                                       TIME_LIMIT_TO_KILL_STATUE);
                }
            }
        });

        auto *pEventInfo = AllocEventInfo<r_meleystatues_info>();
        pEventInfo->bFirst = false;
        pEventInfo->pMap = this;
        e_DestroyStatues = event_create(r_meleystatues_event, pEventInfo, THECORE_SECS_TO_PASSES(3));
    }
}

void CMgrMap::StartDungeonStep(uint8_t bStep)
{
    if (e_SpawnEvent != nullptr)
        event_cancel(&e_SpawnEvent);

    if (e_SEffectEvent != nullptr)
        event_cancel(&e_SEffectEvent);

    e_SpawnEvent = nullptr;
    e_SEffectEvent = nullptr;

    if (bStep == 2)
    {
        SetLastStoneKilledTime(0);
        auto *pEventInfo = AllocEventInfo<r_meleyspawn_info>();
        pEventInfo->pMap = this;
        pEventInfo->bFirst = false;
        pEventInfo->bStep = bStep;
        pEventInfo->dwTimeReload = (uint32_t)TIME_RESPAWN_COMMON_STEP2;
        pEventInfo->dwMobVnum = (uint32_t)MOBVNUM_RESPAWN_COMMON_STEP2;
        pEventInfo->dwMobCount = (uint32_t)MOBCOUNT_RESPAWN_COMMON_STEP;
        e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, THECORE_SECS_TO_PASSES(1));
    }
    else if (bStep == 3)
    {
        SetLastStoneKilledTime(0);
        SetKillCountStones(0);
        SetKillCountBosses(0);
        auto *pEventInfo = AllocEventInfo<r_meleyspawn_info>();
        pEventInfo->pMap = this;
        pEventInfo->bFirst = true;
        pEventInfo->bStep = bStep;
        pEventInfo->dwTimeReload = (uint32_t)TIME_RESPAWN_COMMON_STEP3;
        pEventInfo->dwMobVnum = (uint32_t)MOBVNUM_RESPAWN_COMMON_STEP3;
        pEventInfo->dwMobCount = (uint32_t)MOBCOUNT_RESPAWN_COMMON_STEP;
        e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, THECORE_SECS_TO_PASSES(1));
    }
}

void CMgrMap::Start()
{
    if (!m_sectreeMap)
    {
        EndDungeon(false, true);
        return;
    }

    m_mainNpc = Spawn((uint32_t)NPC_VNUM, 135, 179, 8);
    m_gate = Spawn((uint32_t)GATE_VNUM, 129, 175, 5);
    m_boss = Spawn((uint32_t)BOSS_VNUM, 130, 77, 1);
    m_boss->SetMeley(this);

    m_statues[0] = Spawn(STATUE_VNUM, 123, 137, 5, true);
    m_statues[1] = Spawn(STATUE_VNUM, 123, 124, 5, true);
    m_statues[2] = Spawn(STATUE_VNUM, 136, 123, 5, true);
    m_statues[3] = Spawn(STATUE_VNUM, 137, 137, 5, true);
    m_statues[4] = Spawn(STATUE_VNUM, 130, 150, 5, true);
    m_statues[5] = Spawn(STATUE_VNUM, 147, 128, 5, true);
    m_statues[6] = Spawn(STATUE_VNUM, 125, 117, 5, true);
    m_statues[7] = Spawn(STATUE_VNUM, 112, 131, 5, true);
}

void CMgrMap::StartDungeon(CHARACTER *pkChar)
{
    if (!IsParticipant(pkChar->GetPlayerID()))
        return;
    if (e_pEndEvent != nullptr)
    {
        EndDungeon(false, true);
        return;
    }
    if (!m_mainNpc || !m_gate || !m_boss || !StatuesExist())
    {
        EndDungeon(false, true);
        return;
    }

    SendI18nNoticeMap("The fight against %s just start.", GetMapIndex(), false, TextTag::mobname(m_boss->GetRaceNum()));
    SendI18nNoticeMap("The shelter's doors are now opened. The time limit is %d second(s).", GetMapIndex(), false,
                      static_cast<int>(TIME_LIMIT_DUNGEON));

    pkChar->SetQuestNPCID(0);
    m_gate->Dead();
    m_gate = nullptr;

    SetDungeonStep(1);
    if (e_pEndEvent != nullptr)
        event_cancel(&e_pEndEvent);

    auto *pEventInfo = AllocEventInfo<r_meleylimit_info>();
    pEventInfo->pMap = this;
    pEventInfo->bWarp = false;
    e_pEndEvent = event_create(r_meleylimit_event, pEventInfo, THECORE_SECS_TO_PASSES(TIME_LIMIT_DUNGEON));
}

void CMgrMap::EndDungeon(bool bSuccess, bool bGiveBack)
{
    int32_t iWarpTime = 0;
    if (bGiveBack)
    {
        if (bSuccess)
            iWarpTime = 60;
        else
            iWarpTime = 10;
    }
    else
        iWarpTime = 10;

    if (!iWarpTime)
        EndDungeonWarp();
    else
    {
        if (e_pEndEvent != nullptr)
            event_cancel(&e_pEndEvent);

        e_pEndEvent = nullptr;

        if (e_pWarpEvent != nullptr)
            event_cancel(&e_pWarpEvent);

        r_meleylimit_info *pEventInfo = AllocEventInfo<r_meleylimit_info>();
        pEventInfo->pMap = this;
        pEventInfo->bWarp = true;
        e_pWarpEvent = event_create(r_meleylimit_event, pEventInfo, THECORE_SECS_TO_PASSES(iWarpTime));
    }
}

void CMgrMap::EndDungeonWarp()
{
    if (m_sectreeMap)
    {
        m_sectreeMap->for_each([](CEntity *ent)
        {
            if (!ent->IsType(ENTITY_CHARACTER))
                return;

            if (auto *ch = static_cast<CHARACTER *>(ent); ch->IsPC())
            {
                if (const PIXEL_POSITION posSub = CMgr::instance().GetSubXYZ(); posSub.x)
                {
                    ch->WarpSet(MeleyLair::SUBMAP_INDEX, posSub.x, posSub.y);
                    ch->SetMeley(nullptr);
                    return;
                }

                ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()), EMPIRE_START_X(ch->GetEmpire()),
                            EMPIRE_START_Y(ch->GetEmpire()));
                ch->SetMeley(nullptr);
            }
        });
    }

    SECTREE_MANAGER::instance().DestroyPrivateMap(GetMapIndex());
    Destroy();
    CMgr::instance().Remove(m_uniqueId);
}

bool CMgrMap::AllStatuesHaveAffect(uint32_t affect)
{
    for (auto &statue : m_statues)
    {
        if (!statue->FindAffect(affect))
            return false;
    }

    return true;
}

void CMgrMap::NextDungeonStep(CHARACTER *pkStatue, uint32_t affect, uint32_t nextStep)
{
    if (!pkStatue->FindAffect(affect))
        pkStatue->AddAffect(affect, POINT_NONE, 0, INFINITE_AFFECT_DURATION, 0, true);

    if (AllStatuesHaveAffect(affect))
        SetDungeonStep(nextStep);
}

bool CMgrMap::IsExistingStatue(CHARACTER *checkStatue)
{
    for (auto &statue : m_statues)
    {
        if (statue == checkStatue)
            return true;
    }
    return false;
}

bool CMgrMap::Damage(CHARACTER *pkStatue, CHARACTER *pkAttacker, uint32_t damage)
{
    uint8_t bStep = GetDungeonStep();
    if (!bStep || !IsExistingStatue(pkStatue))
        return false;

    if (pkStatue->FindAffect(AFFECT_STATUE_1) ||
        pkStatue->FindAffect(AFFECT_STATUE_2) ||
        pkStatue->FindAffect(AFFECT_STATUE_3) ||
        pkStatue->FindAffect(AFFECT_STATUE_4))
    {
        pkStatue->SendDamagePacket(pkAttacker, 0, DAMAGE_BLOCK);
        return false;
    }

    const auto prevHp = pkStatue->GetHP();
    if (prevHp <= 0) { SPDLOG_ERROR("The statue has zero or less HP"); }

    int64_t newHp;
    switch (bStep)
    {
    case 1:
        newHp = std::clamp<int64_t>(prevHp - damage, pkStatue->GetMaxHP() * 75 / 100, pkStatue->GetMaxHP());
        break;
    case 2:
        newHp = std::clamp<int64_t>(prevHp - damage, pkStatue->GetMaxHP() * 50 / 100, pkStatue->GetMaxHP());
        break;
    case 3:
        newHp = std::clamp<int64_t>(prevHp - damage, pkStatue->GetMaxHP() * 1 / 100, pkStatue->GetMaxHP());
        break;
    default:
        SPDLOG_DEBUG("Unknown step {} using 1%");
        newHp = std::clamp<int64_t>(prevHp - damage, pkStatue->GetMaxHP() * 1 / 100, pkStatue->GetMaxHP());
        break;
    }

    pkStatue->SetHP(newHp);
    pkStatue->BroadcastTargetPacket(prevHp);
    pkStatue->SendDamagePacket(pkAttacker, prevHp - newHp, DAMAGE_NORMAL);

    const auto hpPercentage = pkStatue->GetHPPct();

    uint8_t reqHpPercentage;
    switch (bStep)
    {
    case 1:
        reqHpPercentage = 75;
        break;
    case 2:
        reqHpPercentage = 50;
        break;
    case 3:
        reqHpPercentage = 1;
        break;
    default:
        reqHpPercentage = 75;
        break;
    }

    if (hpPercentage <= reqHpPercentage)
    {
        switch (bStep)
        {
        case 1:
            NextDungeonStep(pkStatue, AFFECT_STATUE_1, 2);
            break;
        case 2:
            NextDungeonStep(pkStatue, AFFECT_STATUE_2, 3);
            break;
        case 3:
            NextDungeonStep(pkStatue, AFFECT_STATUE_3, 4);
            break;
        default:
            break;
        }
    }

    return false;
}

void CMgrMap::OnKill(uint32_t dwVnum)
{
    uint8_t bStep = GetDungeonStep();
    if (!bStep || !StatuesExist())
        return;

    if ((bStep == 2 || bStep == 3) && dwVnum == (uint32_t)MOBVNUM_RESPAWN_STONE_STEP2)
    {
        uint32_t dwLimit = (uint32_t)MOBCOUNT_RESPAWN_STONE_STEP2 - 1;
        if (GetKillCountStones() >= dwLimit)
        {
            if (bStep == 2)
                SetKillCountStones(0);
            else
                SetKillCountStones(GetKillCountStones() + 1);

            SetLastStoneKilledTime(get_global_time() + 60);

            bool bDoAff = true;
            if (bStep == 3)
            {
                auto dwLimit2 = (uint32_t)MOBCOUNT_RESPAWN_BOSS_STEP3;
                if (GetKillCountBosses() >= dwLimit2)
                    SetKillCountStones(0);
                else
                    bDoAff = false;
            }

            if (bDoAff)
            {
                for (auto statue : m_statues)
                {
                    if (statue->FindAffect(AFFECT_STATUE_1) ||
                        (bStep == 3 && 
                            (statue->FindAffect(AFFECT_STATUE_2) ||
                             statue->FindAffect(AFFECT_STATUE_1)))
                        )
                    {
                        statue->RemoveAffect(AFFECT_STATUE_1);
                        statue->RemoveAffect(AFFECT_STATUE_2);
                    }
                }
            }
        }
        else { SetKillCountStones(GetKillCountStones() + 1); }
    }
    else if (bStep == 3 && dwVnum == (uint32_t)MOBVNUM_RESPAWN_BOSS_STEP3)
    {
        uint32_t dwLimit = (uint32_t)MOBCOUNT_RESPAWN_BOSS_STEP3 - 1;
        if (GetKillCountBosses() >= dwLimit)
        {
            SetKillCountBosses(GetKillCountBosses() + 1);

            bool bDoAff = true;
            auto dwLimit2 = (uint32_t)MOBCOUNT_RESPAWN_STONE_STEP2;
            if (GetKillCountStones() >= dwLimit2)
                SetKillCountStones(0);
            else
                bDoAff = false;

            if (bDoAff)
            {
                for (auto statue : m_statues)
                {
                    if (statue->FindAffect(AFFECT_STATUE_2) || statue->FindAffect(AFFECT_STATUE_1))
                    {
                        statue->RemoveAffect(AFFECT_STATUE_1);
                        statue->RemoveAffect(AFFECT_STATUE_2);
                    }
                }
            }
        }
        else
            SetKillCountBosses(GetKillCountBosses() + 1);
    }
}

void CMgrMap::OnKillStatue(CItem *pkItem, CHARACTER *pkChar, CHARACTER *pkStatue)
{
    uint8_t bStep = GetDungeonStep();
    if (!pkItem || !pkChar || !pkStatue || bStep != 4 || !m_boss || !StatuesExist())
        return;

    if (!pkStatue->FindAffect(AFFECT_STATUE_3))
        return;

    if (pkStatue->FindAffect(AFFECT_STATUE_4))
        return;

    auto pid = pkChar->GetPlayerID();
    bool bCheck = std::find(m_alreadyActed.begin(), m_alreadyActed.end(), pid) != m_alreadyActed.end();

#ifdef __DESTROY_INFINITE_STATUES_GM__
    if (bCheck && pkChar->GetGMLevel() == GM_PLAYER)
#else
		if (bCheck)
#endif
        return;
    else
    {
#ifdef __DESTROY_INFINITE_STATUES_GM__
        if (!bCheck)
            m_alreadyActed.push_back(pid);
#else
			v_Already.push_back(dwPlayerID);
#endif
    }

    ITEM_MANAGER::instance().RemoveItem(pkItem);
    pkStatue->RemoveAffect(AFFECT_STATUE_1);
    pkStatue->RemoveAffect(AFFECT_STATUE_2);
    pkStatue->RemoveAffect(AFFECT_STATUE_3);
    pkStatue->RemoveAffect(AFFECT_STATUE_4);

    pkStatue->AddAffect(AFFECT_STATUE_4, POINT_NONE, 0, INFINITE_AFFECT_DURATION, 0, true);

    if (AllStatuesHaveAffect(AFFECT_STATUE_4))
        DungeonResult();
}

void CMgrMap::DungeonResult()
{
    if (e_DestroyStatues != nullptr)
        event_cancel(&e_DestroyStatues);

    e_DestroyStatues = nullptr;

    if (const auto bNextStep = AllStatuesHaveAffect(AFFECT_STATUE_4); !bNextStep)
    {
        SetDungeonStep(5);
        EndDungeon(false, false);
    }
    else
    {
        m_alreadyRewarded.clear();
        SetRewardTime(get_global_time() + 10);

        for (auto &statue : m_statues) { statue->Dead(); }

        m_boss->Dead();

        Spawn(CHEST_VNUM, 130, 130, 1);
        EndDungeon(true, true);
    }
}

bool CMgrMap::CheckRewarder(PlayerId id)
{
    bool bCheck = std::find(m_alreadyRewarded.begin(), m_alreadyRewarded.end(), id) != m_alreadyRewarded.end();
    return bCheck;
}

void CMgrMap::GiveReward(CHARACTER *pkChar, uint8_t bReward)
{
    auto id = pkChar->GetPlayerID();
    if (!IsParticipant(id))
        return;

    if (CheckRewarder(id))
        return;

    m_alreadyRewarded.push_back(id);
    uint32_t dwVnumReward = bReward == 1 ? REWARD_ITEMCHEST_VNUM_1 : REWARD_ITEMCHEST_VNUM_2;
    pkChar->AutoGiveItem(dwVnumReward, 1);
}

bool CMgrMap::StatuesExist()
{
    for (auto &statue : m_statues)
    {
        if (!statue)
            return false;
    }

    return true;
}

void CMgrMap::Entry(CHARACTER *character)
{
    if (false) //StatuesExist()) 
    {
        character->SpecificTargetEffectPacket(m_statues[0], "d:/ymir work/effect/monster2/redd_moojuk_bluegreen.mse");
        character->SpecificTargetEffectPacket(m_statues[1], "d:/ymir work/effect/monster2/redd_moojuk_fullblue.mse");
        character->SpecificTargetEffectPacket(m_statues[2], "d:/ymir work/effect/monster2/redd_moojuk_fullgreen.mse");
        character->SpecificTargetEffectPacket(m_statues[3], "d:/ymir work/effect/monster2/redd_moojuk_irgendwas.mse");
        character->SpecificTargetEffectPacket(m_statues[4], "d:/ymir work/effect/monster2/redd_moojuk_irgendwas2.mse");
        character->SpecificTargetEffectPacket(m_statues[5], "d:/ymir work/effect/monster2/redd_moojuk_redblue.mse");
        character->SpecificTargetEffectPacket(m_statues[6], "d:/ymir work/effect/monster2/redd_moojuk_redgreen.mse");
        character->SpecificTargetEffectPacket(m_statues[7], "d:/ymir work/effect/monster2/redd_moojuk_white.mse");
    }
}

void CMgr::Initialize()
{
    m_mapMeleys.clear();
    SetXYZ(0, 0, 0);
    SetSubXYZ(0, 0, 0);
    dwUniqueID = 0;
}

void CMgr::Destroy()
{
    for (auto &it : m_mapMeleys)
    {
        auto pMap = it.second.get();
        if (pMap)
        {
            SECTREE_MANAGER::instance().DestroyPrivateMap(pMap->GetMapIndex());
            pMap->Destroy();
        }
    }

    m_mapMeleys.clear();
    SetXYZ(0, 0, 0);
    SetSubXYZ(0, 0, 0);
    dwUniqueID = 0;
}

bool CMgr::EnterUniqueId(uint32_t meleyId, CHARACTER *pkChar)
{
    if (!pkChar || !meleyId)
        return false;

    auto it = m_mapMeleys.find(meleyId);
    if (it == m_mapMeleys.end())
        return false;

    auto pMap = it->second.get();
    PIXEL_POSITION mPos;
    if (!SECTREE_MANAGER::instance().GetRecallPositionByEmpire((int32_t)(MAP_INDEX), pkChar->GetEmpire(), mPos))
        return false;

    pMap->Participant(true, pkChar->GetPlayerID());
    pkChar->SetMeley(pMap);
    pkChar->SaveExitLocation();
    pkChar->WarpSet(mPos.x, mPos.y, pMap->GetMapIndex());

    return true;
}

bool CMgr::Enter(CHARACTER *pkChar)
{
    if (!pkChar)
        return false;

    int32_t lNormalMapIndex = (int32_t)(MAP_INDEX);
    int32_t lMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(lNormalMapIndex);
    if (!lMapIndex)
        return false;

    PIXEL_POSITION pos = GetXYZ(), posSub = GetSubXYZ();
    if (!pos.x) { SetXYZ(130 * 100, 130 * 100, 0); }

    if (!posSub.x) { SetSubXYZ(87 * 100, 853 * 100, 0); }
    uint32_t dwID = ++dwUniqueID;

    CMgrMap *pMap = new CMgrMap(lMapIndex, dwID);
    if (!pMap)
        return false;

    m_mapMeleys.emplace(dwID, pMap);
    pkChar->SetMeley(pMap);
    PIXEL_POSITION mPos;
    if (!SECTREE_MANAGER::instance().GetRecallPositionByEmpire((int32_t)(MAP_INDEX), pkChar->GetEmpire(), mPos))
        return false;

    pMap->Participant(true, pkChar->GetPlayerID());
    pkChar->SaveExitLocation();
    pkChar->WarpSet(mPos.x, mPos.y, pMap->GetMapIndex());

    return true;
}

void CMgr::Leave(CHARACTER *pkChar, bool bSendOut)
{
    if (!pkChar)
        return;

    auto *pMap = pkChar->GetMeley();
    if (pMap)
    {
        if (!bSendOut)
        {
            if (pkChar->GetMapIndex() == pMap->GetMapIndex())
                pMap->Participant(false, pkChar->GetPlayerID());
        }
        else if (!pMap->IsParticipant(pkChar->GetPlayerID()))
            WarpOut(pkChar);
    }
    else
        WarpOut(pkChar);
}

void CMgr::LeaveRequest(CHARACTER *pkChar)
{
    if (!pkChar)
        return;

    auto *pMap = pkChar->GetMeley();
    if (pMap)
    {
        if (pMap->IsParticipant(pkChar->GetPlayerID()))
            pMap->Participant(false, pkChar->GetPlayerID());

        WarpOut(pkChar);
    }
    else
        WarpOut(pkChar);
}

bool CMgr::IsMeleyMap(long lMapIndex)
{
    if (lMapIndex >= 10000)
        lMapIndex /= 10000;

    return MAP_INDEX == lMapIndex;
}

void CMgr::Check(CHARACTER *pkChar)
{
    if (!pkChar)
        return;

    auto *pMap = pkChar->GetMeley();
    if (pMap)
    {
        if (!pMap->IsParticipant(pkChar->GetPlayerID()))
            WarpOut(pkChar);
    }
    else
        WarpOut(pkChar);
}

void CMgr::WarpOut(CHARACTER *pkChar) const
{
    if (!pkChar)
        return;

    if (const auto posSub = GetSubXYZ(); posSub.x)
    {
        pkChar->WarpSet(MeleyLair::SUBMAP_INDEX, posSub.x, posSub.y);
        return;
    }

    pkChar->WarpSet(EMPIRE_START_MAP(pkChar->GetMapIndex()), EMPIRE_START_X(pkChar->GetEmpire()),
                    EMPIRE_START_Y(pkChar->GetEmpire()));
}

void CMgr::Start(CHARACTER *pkChar)
{
    if (!pkChar)
        return;

    auto *pMap = pkChar->GetMeley();
    if (pMap && pMap->GetDungeonStep() < 1)
        pMap->StartDungeon(pkChar);
}

bool CMgr::Damage(CHARACTER *pkStatue, CHARACTER *pkAttacker, uint32_t damage)
{
    if (!pkStatue || !pkAttacker)
        return false;

    const auto guild = pkAttacker->GetGuild();
    if (!guild)
        return false;

    auto *pMap = pkAttacker->GetMeley();
    if (!pMap)
        return false;

    return pMap->Damage(pkStatue, pkAttacker, damage);
}

void CMgr::Remove(uint32_t dwID)
{
    auto it = m_mapMeleys.find(dwID);
    auto iterEnd = m_mapMeleys.end();
    if (it != iterEnd)
        m_mapMeleys.erase(it, iterEnd);
}

void CMgr::OnKill(uint32_t dwVnum, CHARACTER *pkKiller)
{
    if (!dwVnum)
        return;

    auto *pMap = pkKiller->GetMeley();
    if (!pMap)
        return;

    pMap->OnKill(dwVnum);
}

void CMgr::OnKillStatue(CItem *pkItem, CHARACTER *pkChar, CHARACTER *pkStatue)
{
    if (!pkItem || !pkChar || !pkStatue)
        return;

    if (pkItem->GetOriginalVnum() != SEAL_VNUM_KILL_STATUE)
        return;

    auto *pMap = pkChar->GetMeley();
    if (!pMap)
        return;

    if (!pMap->IsParticipant(pkChar->GetPlayerID()))
        return;

    pMap->OnKillStatue(pkItem, pkChar, pkStatue);
}

void CMgr::OnKillCommon(CHARACTER *pkMonster, CHARACTER *pkChar)
{
    if (!pkMonster || !pkChar)
        return;

    auto *pMap = pkChar->GetMeley();
    if (!pMap)
        return;
    if (!pMap->IsParticipant(pkChar->GetPlayerID()))
        return;
    if (pMap->GetDungeonStep() < 3)
        return;

    const int iChance = Random::get<bool>(0.1);
    //10% chance to drop
    if (iChance)
    {
        auto pkItem = ITEM_MANAGER::instance().CreateItem(SEAL_VNUM_KILL_STATUE);
        if (!pkItem)
            return;

        PIXEL_POSITION mPos;
        mPos.x = pkMonster->GetX();
        mPos.y = pkMonster->GetY();

        pkItem->AddToGround(pMap->GetMapIndex(), mPos);
        if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
            pkItem->StartDestroyEvent(gConfig.itemGroundTimeLong);
            pkItem->SetOwnership(pkChar, gConfig.itemOwnershipTimeLong);
        }
        else {
            pkItem->StartDestroyEvent(gConfig.itemGroundTime);
            pkItem->SetOwnership(pkChar, gConfig.itemOwnershipTime);
        }
    }
}

bool CMgr::CanGetReward(CHARACTER *pkChar)
{
    if (!pkChar)
        return false;

    auto mgrMap = pkChar->GetMeley();
    if (!mgrMap)
        return false;

    if (!mgrMap->IsParticipant(pkChar->GetPlayerID()))
        return false;

    if (mgrMap->GetDungeonStep() != 4)
        return false;

    if (mgrMap->GetRewardTime() > get_global_time())
        return false;

    if (mgrMap->CheckRewarder(pkChar->GetPlayerID()))
        return false;

    return true;
}

void CMgr::Reward(CHARACTER *pkChar, uint8_t bReward)
{
    if (!pkChar)
        return;

    if (auto mgrMap = pkChar->GetMeley(); mgrMap)
        mgrMap->GiveReward(pkChar, bReward);
}

void CMgr::OpenRanking(CHARACTER *pkChar)
{
    if (!pkChar)
        return;
}

void CMgr::DungeonSet(CHARACTER *pkChar)
{
    if (!pkChar)
        return;

    for (auto &it : m_mapMeleys)
    {
        auto pMap = it.second.get();
        if (pMap && pMap->IsParticipant(pkChar->GetPlayerID()))
            pkChar->SetMeley(pMap);
    }
}
};
#endif
