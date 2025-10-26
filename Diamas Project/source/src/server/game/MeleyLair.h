#ifndef METIN2_SERVER_GAME_MELEYLAIR_H
#define METIN2_SERVER_GAME_MELEYLAIR_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>
#include <game/Types.hpp>

#include "event.h"
#include "typedef.h"

class CItem;
class CGuild;
class CHARACTER;
class SECTREE_MAP;

namespace MeleyLair
{
enum eConfig
{
    MAP_INDEX = 215,
    SUBMAP_INDEX = 160,
    MIN_LVL = 22,
    PARTECIPANTS_LIMIT = 8,
    LADDER_POINTS_COST = 100,
    LADDER_POINTS_RETURN = 50,
    COOLDOWN_DUNGEON = 0,
    NPC_VNUM = 20429,
    GATE_VNUM = 20388,
    BOSS_VNUM = 6193,
    STATUE_VNUM = 6118,
    CHEST_VNUM = 20420,
    REWARD_ITEMCHEST_VNUM_1 = 50270,
    REWARD_ITEMCHEST_VNUM_2 = 50271,
    TIME_LIMIT_DUNGEON = 1800,
    SEAL_VNUM_KILL_STATUE = 30341,
    TIME_LIMIT_TO_KILL_STATUE = 20,
    TIME_RESPAWN_COMMON_STEP1 = 5,
    MOBCOUNT_RESPAWN_COMMON_STEP = 12,
    MOBVNUM_RESPAWN_COMMON_STEP1 = 6112,
    TIME_RESPAWN_COMMON_STEP2 = 30,
    MOBVNUM_RESPAWN_COMMON_STEP2 = 6113,
    MOBVNUM_RESPAWN_STONE_STEP2 = 20422,
    MOBCOUNT_RESPAWN_STONE_STEP2 = 8,
    TIME_RESPAWN_COMMON_STEP3 = 30,
    MOBVNUM_RESPAWN_COMMON_STEP3 = 6115,
    MOBVNUM_RESPAWN_BOSS_STEP3 = 6116,
    MOBVNUM_RESPAWN_SUBBOSS_STEP3 = 6117,
    MOBCOUNT_RESPAWN_BOSS_STEP3 = 8,
};

extern int stoneSpawnPos[MOBCOUNT_RESPAWN_STONE_STEP2][2];
extern int monsterSpawnPos[MOBCOUNT_RESPAWN_COMMON_STEP][2];

class CMgrMap
{
public:
    CMgrMap(long lMapIndex, uint32_t dwGuildID);
    ~CMgrMap();
    void Destroy();

    long GetMapIndex() const { return m_mapIndex; }

    uint32_t GetUniqueId() const { return m_uniqueId; }

    uint8_t GetDungeonStep() const { return m_dungeonStep; }

    void SetDungeonStep(uint8_t bStep);
    void StartDungeonStep(uint8_t bStep);

    uint32_t GetDungeonTimeStart() const { return m_startTime; }

    void SetDungeonTimeStart(uint32_t dwTime) { m_startTime = dwTime; }

    uint32_t GetLastStoneKilledTime() const { return m_lastStoneKilled; }

    void SetLastStoneKilledTime(uint32_t dwTime) { m_lastStoneKilled = dwTime; }

    uint32_t GetKillCountStones() const { return m_killedStoneCount; }

    void SetKillCountStones(uint32_t dwCount) { m_killedStoneCount = dwCount; }

    uint32_t GetKillCountBosses() const { return m_killedBossCount; }

    void SetKillCountBosses(uint32_t dwCount) { m_killedBossCount = dwCount; }

    uint32_t GetRewardTime() const { return m_reward; }

    void SetRewardTime(uint32_t dwTime) { m_reward = dwTime; }

    uint32_t GetParticipantsCount() const;
    void Participant(bool bInsert, PlayerId pid);
    bool IsParticipant(PlayerId pid);
    CHARACTER *Spawn(uint32_t dwVnum, int iX, int iY, int iDir = 0, bool bSpawnMotion = false);
    void Start();
    void StartDungeon(CHARACTER *pkChar);
    void EndDungeon(bool bSuccess, bool bGiveBack);
    void EndDungeonWarp();
    bool AllStatuesHaveAffect(uint32_t affect);
    void NextDungeonStep(CHARACTER *pkStatue, uint32_t affect, uint32_t nextStep);
    bool IsExistingStatue(CHARACTER *checkStatue);
    bool Damage(CHARACTER *pkStatue, CHARACTER *pkAttacker, uint32_t damage);
    void OnKill(uint32_t dwVnum);
    void OnKillStatue(CItem *pkItem, CHARACTER *pkChar, CHARACTER *pkStatue);
    void DungeonResult();
    bool CheckRewarder(PlayerId id);
    void GiveReward(CHARACTER *pkChar, uint8_t bReward);
    bool StatuesExist();

    SECTREE_MAP *GetMapSectree() const { return m_sectreeMap; }

    CHARACTER *GetBossChar() const { return m_boss; }

    CHARACTER *GetStatueChar(uint8_t index) { return m_statues[index]; }

    void Entry(CHARACTER *character);
private:

    uint32_t m_mapIndex = 0;
    uint32_t m_uniqueId = 0;
    uint32_t m_startTime = 0;
    uint32_t m_lastStoneKilled = 0;
    uint32_t m_killedStoneCount = 0;
    uint32_t m_killedBossCount = 0;
    uint32_t m_reward = 0;
    uint8_t m_dungeonStep = 0;
    std::vector<PlayerId> m_participants;
    std::vector<PlayerId> m_alreadyActed;
    std::vector<PlayerId> m_alreadyRewarded;
    SECTREE_MAP *m_sectreeMap = nullptr;
    CHARACTER *m_mainNpc{};
    CHARACTER *m_gate{};
    CHARACTER *m_boss{};
    CHARACTER *m_statues[8] = {};

protected:
    LPEVENT e_pEndEvent, e_pWarpEvent, e_SpawnEvent, e_SEffectEvent, e_DestroyStatues;
};

class CMgr final : public singleton<CMgr>
{
public:
    void Initialize();
    void Destroy();
    bool EnterUniqueId(uint32_t meleyId, CHARACTER *pkChar);
    bool Enter(CHARACTER *pkChar);
    void Leave(CHARACTER *pkChar, bool bSendOut);
    void LeaveRequest(CHARACTER *pkChar);
    static bool IsMeleyMap(long lMapIndex);
    void Check(CHARACTER *pkChar);
    void WarpOut(CHARACTER *pkChar) const;

    void SetXYZ(long lX, long lY, long lZ) { m_mapCenterPos.x = lX, m_mapCenterPos.y = lY, m_mapCenterPos.z = lZ; }

    const PIXEL_POSITION &GetXYZ() const { return m_mapCenterPos; }

    void SetSubXYZ(long lX, long lY, long lZ) { m_subMapPos.x = lX, m_subMapPos.y = lY, m_subMapPos.z = lZ; }

    const PIXEL_POSITION &GetSubXYZ() const { return m_subMapPos; }

    void Start(CHARACTER *pkChar);
    bool Damage(CHARACTER *pkStatue, CHARACTER *pkAttacker, uint32_t damage);
    void Remove(uint32_t dwGuildID);
    void OnKill(uint32_t dwVnum, CHARACTER *pkKiller);
    void OnKillStatue(CItem *pkItem, CHARACTER *pkChar, CHARACTER *pkStatue);
    void OnKillCommon(CHARACTER *pkMonster, CHARACTER *pkChar);
    bool CanGetReward(CHARACTER *pkChar);
    void Reward(CHARACTER *pkChar, uint8_t bReward);
    void OpenRanking(CHARACTER *pkChar);

    void DungeonSet(CHARACTER *pkChar);
private:
    PIXEL_POSITION m_mapCenterPos = {};
    PIXEL_POSITION m_subMapPos = {};
    std::unordered_map<uint32_t, std::unique_ptr<CMgrMap>> m_mapMeleys;
    uint32_t dwUniqueID = 0;
};
};

#endif /* METIN2_SERVER_GAME_MELEYLAIR_H */
