#pragma once

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraConfig.h"

class CHydraDungeon final
{
private:

public:
    enum EStages
    {
        STAGE_ONE,
        STAGE_TWO,
        STAGE_THREE,
        STAGE_FOUR,
        STAGE_FIVE,
        STAGE_SIX,
        STAGE_SEVEN,
        STAGE_EIGHT,
        STAGE_NINE,

        STAGE_COMPLETED = 253,
        STAGE_FAIL = 254,
        STAGE_WAITING = 255
    };

public:
    CHydraDungeon(uint32_t owner);
    ~CHydraDungeon();

    void OnLogin(CHARACTER *ch);
    void OnLogout(CHARACTER *ch);
    void OnKill(CHARACTER *ch, CHARACTER *victim);
    void ProcessDestroy(CHARACTER *ch);

    long CreateDungeon();
    long ProcessStage(bool isTimerEvent);

    void NotifyHydraDmg(CHARACTER *hydra, DamageValue *dmg);

    uint32_t GetOwner() { return m_dwOwnerID; }

    EStages GetStage() { return m_bStage; };

    void ClearStageEvent() { m_stageEvent = nullptr; }

private:

    void DestroyDungeon();
    void WarpToExit(CHARACTER *ch = nullptr);
    void ClearMonsters();
    void SetStage(EStages nextStage);
    void ProcessFailure();
    void SendStageNotice(CHARACTER *ch = nullptr);

    template <class Func> void ForEachMember(Func &f)
    {
        for (auto it = m_vecPC.begin(); it != m_vecPC.end(); ++it)
            f(*it);
    }

private:
    std::vector<CHARACTER *> m_vecPC;
    std::map<uint32_t, CHARACTER *> m_monsterKeep;
    std::map<uint32_t, CHARACTER *> m_hydraEggs;

    long m_lMapIndex;
    uint32_t m_dwOwnerID;

    LPEVENT m_destroyEvent;
    LPEVENT m_stageEvent;
    LPEVENT m_warpOutTimer;

    CHARACTER *m_mastNPC;
    CHARACTER *m_hydraBoss;
    CHARACTER *m_rewardChest;

    bool m_bEggsSpawned;
    EStages m_bStage;
    time_t m_runStart;
    time_t m_runEnd;

    CHARACTER *m_hydraHeads[HYDRA_HEADS_MAX_NUM];
};

class CHydraDungeonManager final : public singleton<CHydraDungeonManager>
{
public:
    CHydraDungeonManager();
    ~CHydraDungeonManager();
    void Destroy();

    bool CreateDungeon(CHARACTER *owner);
    void DestroyDungeon(long lMapIndex);
    void OnLogin(CHARACTER *ch);
    void OnDestroy(CHARACTER *ch);
    void OnKill(CHARACTER *ch, CHARACTER *victim);

    bool CanAttack(CHARACTER *ch, CHARACTER *victim);

    void NotifyHydraDmg(CHARACTER *hydra, DamageValue *dmg);

private:
    std::map<long, CHydraDungeon *> m_mapDungeon;
};

inline void CHydraDungeonManager::OnDestroy(CHARACTER *ch)
{
    if (ch->IsPrivateMap(HYDRA_DUNGEON_MAPINDEX))
    {
        auto it = m_mapDungeon.find(ch->GetMapIndex());
        if (it == m_mapDungeon.end())
            return;

        if (ch->IsPC())
        {
            SPDLOG_INFO("pc %d is about to log off from hydra map %d", ch->GetPlayerID(), ch->GetMapIndex());
            it->second->OnLogout(ch);
        }
        else
            it->second->ProcessDestroy(ch);
    }
}

inline void CHydraDungeonManager::OnKill(CHARACTER *ch, CHARACTER *victim)
{
    if (!ch || !victim || ch == victim)
        return;

    if (ch->IsPrivateMap(HYDRA_DUNGEON_MAPINDEX))
    {
        auto it = m_mapDungeon.find(ch->GetMapIndex());
        if (it == m_mapDungeon.end())
            return;

        it->second->OnKill(ch, victim);
    }
}

inline bool CHydraDungeonManager::CanAttack(CHARACTER *ch, CHARACTER *victim)
{
    if (ch == victim || !ch || !victim)
        return false;

    if (!ch->IsPrivateMap(HYDRA_DUNGEON_MAPINDEX) || (ch->IsPC() && !victim->IsMonster() && !victim->IsStone()))
        return false;

    if (ch->IsMonster() && (victim->IsMonster() || victim->IsStone()))
        return false;

    return true;
}

inline void CHydraDungeonManager::NotifyHydraDmg(CHARACTER *hydra, DamageValue *dmg)
{
    auto it = m_mapDungeon.find(hydra->GetMapIndex());
    if (it == m_mapDungeon.end())
        return;

    it->second->NotifyHydraDmg(hydra, dmg);
}

struct map_destroy_event_info : public event_info_data
{
    long lMapIndex;
};

struct stage_event_info : public event_info_data
{
    CHydraDungeon *pDungeon;
};

#endif
