#ifndef INC_METIN_II_GAME_CHARACTER_MANAGER_H__
#define INC_METIN_II_GAME_CHARACTER_MANAGER_H__



#include <game/DbPackets.hpp>
#include <base/Singleton.hpp>

#include "vid.h"
#include <base/robin_hood.h>


struct regen;
class CDungeon;
class CGrid;
class CHARACTER;
class CharacterVectorInteractor;
class CharacterSetSnapshot;

class CHARACTER_MANAGER : public singleton<CHARACTER_MANAGER>
{
public:
    typedef robin_hood::unordered_map<std::string, CHARACTER *> NAME_MAP;

    CHARACTER_MANAGER();
    virtual ~CHARACTER_MANAGER() = default;

    void Destroy();

    void GracefulShutdown(); // 정상적 셧다운할 때 사용. PC를 모두 저장시키고 Destroy 한다.

    uint32_t AllocVID();

    CHARACTER *CreateCharacter(std::string stName, uint32_t dwPID = 0);
#ifndef DEBUG_ALLOC
    void DestroyCharacter(CHARACTER *ch);
#else
		void DestroyCharacter(CHARACTER* ch, const char* file, size_t line);
#endif

    void Update(int iPulse);

    CHARACTER *SpawnMob(uint32_t dwVnum, long lMapIndex, long x, long y, long z, bool bSpawnMotion = false,
                        int iRot = -1, bool bShow = true, bool bAggressive = false, uint16_t scale = 100,
                        bool ignoreMobChecks = false);
    CHARACTER *SpawnMobRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, bool bIsException = false,
                             bool bSpawnMotion = false, bool bAggressive = false, uint16_t scale = 100);
    CHARACTER *SpawnGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, regen *pkRegen = nullptr,
                          bool bAggressive_ = false, CDungeon *pDungeon = nullptr, bool bRandom = false,
                          CHARACTER *aggressiveTo = nullptr);
    bool SpawnGroupGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, regen *pkRegen = nullptr,
                         bool bAggressive_ = false, CDungeon *pDungeon = nullptr);
    bool SpawnMoveGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, int tx, int ty,
                        regen *pkRegen = nullptr, bool bAggressive_ = false);
    CHARACTER *SpawnMobRandomPosition(uint32_t dwVnum, long lMapIndex);

    void SelectStone(CHARACTER *pkChrStone);
    bool IsWorldBoss( uint32_t raceVnum ) const
    {
        for(const auto& info : m_vec_kWorldBossInfo) {
            if(info.dwRaceNum == raceVnum)
                return true;
        }
        return false;
    }
    NAME_MAP &GetPCMap() { return m_map_pkPCChr; }

    CHARACTER *Find(uint32_t dwVID);
    CHARACTER *Find(const VID &vid);
    void RegisterKill(uint32_t pid, uint64_t hwid);
    bool HasPlayerKilledHwid(uint32_t pid, uint64_t hwid);
    CHARACTER *FindPC(std::string stName);
    CHARACTER *FindByPID(uint32_t dwPID);

    bool AddToStateList(CHARACTER *ch);
    void RemoveFromStateList(CHARACTER *ch);

    // DelayedSave: 어떠한 루틴 내에서 저장을 해야 할 짓을 많이 하면 저장
    // 쿼리가 너무 많아지므로 "저장을 한다" 라고 표시만 해두고 잠깐
    // (예: 1 frame) 후에 저장시킨다.
    void DelayedSave(CHARACTER *ch);
    bool FlushDelayedSave(CHARACTER *ch); // Delayed 리스트에 있다면 지우고 저장한다. 끊김 처리시 사용 됨.
    void ProcessDelayedSave();

    template <class Func>
    Func for_each_pc(Func f);
template <class Func> Func for_each(Func f);

void RegisterForMonsterLog(CHARACTER *ch);
    void UnregisterForMonsterLog(CHARACTER *ch);


    void KillLog(uint32_t dwVnum);

    void RegisterRaceNum(uint32_t dwVnum);
    void RegisterRaceNumMap(CHARACTER *ch);
    void UnregisterRaceNumMap(CHARACTER *ch);
    CharacterSetSnapshot GetCharactersByRaceNum(uint32_t dwRaceNum);
    int CountCharactersByRaceNum(uint32_t dwRaceNum);

    CHARACTER *FindSpecifyPC(unsigned int uiJobFlag, long lMapIndex, CHARACTER *except = nullptr, int iMinLevel = 1,
                             int iMaxLevel = PLAYER_MAX_LEVEL_CONST);

    void SetMobItemRate(int value) { m_iMobItemRate = value; }

    void SetMobDamageRate(int value) { m_iMobDamageRate = value; }

    void SetMobGoldAmountRate(int value) { m_iMobGoldAmountRate = value; }

    void SetMobGoldDropRate(int value) { m_iMobGoldDropRate = value; }

    void SetMobExpRate(int value) { m_iMobExpRate = value; }

    void SetMobItemRatePremium(int value) { m_iMobItemRatePremium = value; }

    void SetMobGoldAmountRatePremium(int value) { m_iMobGoldAmountRatePremium = value; }

    void SetMobGoldDropRatePremium(int value) { m_iMobGoldDropRatePremium = value; }

    void SetMobExpRatePremium(int value) { m_iMobExpRatePremium = value; }

    void SetUserDamageRatePremium(int value) { m_iUserDamageRatePremium = value; }

    void SetUserDamageRate(int value) { m_iUserDamageRate = value; }

    int GetMobItemRate(CHARACTER *ch);
    int GetMobDamageRate(CHARACTER *ch);
    int GetMobGoldAmountRate(CHARACTER *ch);
    int GetMobGoldDropRate(CHARACTER *ch);
    int GetMobExpRate(CHARACTER *ch);

    int GetUserDamageRate(CHARACTER *ch);
    void SendScriptToMap(long lMapIndex, const std::string &s);

    bool IsPendingDestroy() const { return m_bUsePendingDestroy; }

    bool BeginPendingDestroy();
    void FlushPendingDestroy();

    void DestroyCharacterInMap(long lMapIndex);
    bool LoadAutoNotices();

private:
    int m_iMobItemRate;
    int m_iMobDamageRate;
    int m_iMobGoldAmountRate;
    int m_iMobGoldDropRate;
    int m_iMobExpRate;

    int m_iMobItemRatePremium;
    int m_iMobGoldAmountRatePremium;
    int m_iMobGoldDropRatePremium;
    int m_iMobExpRatePremium;

    int m_iUserDamageRate;
    int m_iUserDamageRatePremium;
    int m_iVIDCount;

    robin_hood::unordered_map<uint32_t, CHARACTER *> m_map_pkChrByVID;
    robin_hood::unordered_map<uint32_t, CHARACTER *> m_map_pkChrByPID;
    std::unordered_map<uint32_t, std::vector<uint64_t>> m_map_pidToHwidsKilled;
    NAME_MAP m_map_pkPCChr;

    std::unordered_set<CHARACTER *> m_set_pkChrState; // FSM이 돌아가고 있는 놈들
    robin_hood::unordered_set<CHARACTER *> m_set_pkChrForDelayedSave;
    robin_hood::unordered_set<CHARACTER *> m_set_pkChrMonsterLog;

    CHARACTER *m_pkChrSelectedStone;

    std::map<uint32_t, uint32_t> m_map_dwMobKillCount;

    std::unordered_set<uint32_t> m_set_dwRegisteredRaceNum;
    std::map<uint32_t, std::unordered_set<CHARACTER *>> m_map_pkChrByRaceNum;

    bool m_bUsePendingDestroy;
    robin_hood::unordered_set<CHARACTER *> m_set_pkChrPendingDestroy;

    std::vector<TWorldBossInfo> m_vec_kWorldBossInfo;
    std::vector<std::vector<std::string>> m_hourlyNotices;



#ifdef M2_USE_POOL
		ObjectPool<CHARACTER> pool_;
#endif

#ifdef __WORLD_BOSS__
public:
    bool LoadWorldBossInfo(const char *szFileName);
    void UpdateWorldBoss();
    bool SpawnWorldBoss(int iIndex, int iMapIndex = 0, uint32_t x = 0, uint32_t y = 0);

#endif
};

template <class Func>
Func CHARACTER_MANAGER::for_each_pc(Func f)
{
    for (auto it = m_map_pkChrByPID.begin(); it != m_map_pkChrByPID.end(); ++it)
        f(it->second);

    return f;
}

template <class Func>
Func CHARACTER_MANAGER::for_each(Func f)
{
    for (auto it = m_map_pkChrByVID.begin(); it != m_map_pkChrByVID.end(); ++it)
        f(it->second);

    return f;
}

class CharacterSnapshotGuard
{
public:
    CharacterSnapshotGuard();
    ~CharacterSnapshotGuard() noexcept;

private:
    bool m_hasPendingOwnership;
};

class CharacterSetSnapshot
{
public:
    CharacterSetSnapshot();
    CharacterSetSnapshot(const std::unordered_set<CHARACTER *> &chars);

    std::unordered_set<CHARACTER *>::const_iterator begin() const;
    std::unordered_set<CHARACTER *>::const_iterator end() const;

    bool empty() const { return !m_chars || m_chars->empty(); }

private:
    CharacterSnapshotGuard m_guard;
    const std::unordered_set<CHARACTER *> *m_chars;
};

#ifndef DEBUG_ALLOC
#define M2_DESTROY_CHARACTER(ptr) g_pCharManager->DestroyCharacter(ptr)
#else
#define M2_DESTROY_CHARACTER(ptr) g_pCharManager->DestroyCharacter(ptr, __FILE__, __LINE__)
#endif

#define g_pCharManager	singleton<CHARACTER_MANAGER>::InstancePtr()

#endif
