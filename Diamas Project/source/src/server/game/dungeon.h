#ifndef METIN2_SERVER_GAME_DUNGEON_H
#define METIN2_SERVER_GAME_DUNGEON_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cstdint>
#include <memory>

#include "sectree_manager.h"
#include "party.h"
#include <base/robin_hood.h>

class CHARACTER;
class CParty;
struct regen;

class CDungeon
{
    friend class CDungeonManager;

public:
    // <Factor> Non-persistent identifier type
    typedef uint32_t IdType;

    CDungeon(IdType id, int32_t originalMapIndex, int32_t mapIndex);
    virtual ~CDungeon();
 

    IdType GetId() const { return m_id; }

    int32_t GetMapIndex() const { return m_lMapIndex; }

    int32_t GetOriginalMapIndex() const { return m_lOrigMapIndex; }

    void Notice(const char *msg);
    void ClearMissionMessage();
    void SetMissionMessage(const char *msg);
    void SetMissionSubMessage(const char *msg);

    // These two functions only warp the characters into the dungeon
    // They're not added to the dungeon's member set.
    void Join(CHARACTER *ch);
    void JoinParty(CParty *pParty);

    void JumpAll(int32_t lFromMapIndex, int x, int y);
    void WarpAll(int32_t lFromMapIndex, int x, int y);
    void JumpParty(CParty *pParty, int32_t lFromMapIndex, int x, int y);
	void	SkipPlayerSaveDungeonOnce() { m_bSkipSaveWarpOnce = true; }

    void ExitAll();
    void ExitAllToStartPosition();

    void Purge();
    void KillAll();

    void Spawn(uint32_t vnum, const char *pos);
    CHARACTER *SpawnMob(uint32_t vnum, int x, int y, int dir = 0);
    CHARACTER *SpawnMob_ac_dir(uint32_t vnum, int x, int y, int dir = 0);
    CHARACTER *SpawnGroup(uint32_t vnum, int32_t x, int32_t y, float radius,
                          bool bAggressive = false, int count = 1);

    void SpawnNameMob(uint32_t vnum, int x, int y, const char *name);
    void SpawnGotoMob(int32_t lFromX, int32_t lFromY, int32_t lToX,
                      int32_t lToY);
    void SpawnMoveGroup(uint32_t vnum, const char *pos_from, const char *pos_to,
                        int count = 1);

    void SpawnRegen(const char *filename, bool bOnce = true);
    void AddRegen(regen *r);
    void ClearRegen();
    bool IsValidRegen(regen *r, size_t regen_id);
    void Completed();

	void 	SetMaxDmgOnBoss(uint32_t dwVnum, uint32_t dwDmg);
	uint32_t 	GetMaxDmgOnBoss(uint32_t vnum);
   void AddBossVnum(uint32_t dwVnum);
    uint32_t IsBossVnum(uint32_t vnum);

    virtual void OnEnter(CHARACTER *ch);
    virtual void OnLeave(CHARACTER *ch);
    virtual void OnKill(CHARACTER *killer, CHARACTER *victim) = 0;

    template <class Func> Func ForEachMember(Func &&f);
    void Rejoin(CHARACTER* ch);

    protected:
    IdType m_id;
    uint32_t m_lOrigMapIndex;
    uint32_t m_lMapIndex;
    uint32_t m_rejoinX = 0;
    uint32_t m_rejoinY = 0;
    bool m_completed = false;
	bool m_bSkipSaveWarpOnce = false;

    std::unordered_set<CHARACTER *> m_players;
    const TAreaMap *m_map_Area;

    std::vector<regen *> m_regen;
    size_t regen_id_;
	std::unordered_map<uint32_t, DamageValue> m_bossDamage;
};

template <class Func> Func CDungeon::ForEachMember(Func &&f)
{
    for (auto it = m_players.begin(); it != m_players.end(); ++it) { f(*it); }

    return f;
}

class QuestDungeon : public CDungeon
{
    friend EVENTFUNC(dungeon_dead_event);
    friend EVENTFUNC(dungeon_exit_all_event);
    friend EVENTFUNC(dungeon_jump_to_event);
    friend EVENTFUNC(dungeon_jump_all_event);

public:
    QuestDungeon(IdType id, int32_t originalMapIndex, int32_t mapIndex);
    virtual ~QuestDungeon();

    typedef std::vector<std::pair<uint32_t, int>> ItemGroup;

    int GetFlag(const std::string &name);
    void SetFlag(const std::string &name, int value);

    uint8_t GetDungeonStage() const { return m_dungeonStage; }

    uint8_t GetPrevDungeonStage() const { return m_prevDungeonStage; }

    void IncMonster();
    void DecMonster();

    int CountMonster() { return m_iMonsterCount; }

    int CountRealMonster();
    void JumpAllDelayed(int x, int y, int delay);
    void CheckEliminated();

    void IncKillCount(CHARACTER *pkKiller, CHARACTER *pkVictim);
    int GetKillMobCount();
    int GetKillStoneCount();

    void SetExitAllAtEliminate(int32_t time);
    void SetEventAtEliminate(const char * regen_file);
    void SetDungeonExitLocation(uint32_t mapIndex, uint32_t x, uint32_t y);
    void SetWarpAtEliminate(int32_t      time, int32_t lMapIndex, int x, int y,
                            const char * regen_file);
    void JumpToEliminateLocation();

    void SetUnique(const char *key, uint32_t vid);
    void SetRejoinPosition(uint32_t x, uint32_t y);
    void SpawnMoveUnique(const char *key, uint32_t vnum, const char *pos_from,
                         const char *pos_to);
    void SpawnUnique(const char *key, uint32_t vnum, const char *pos);
    void SpawnStoneDoor(const char *key, const char *pos);
    void SpawnWoodenDoor(const char *key, const char *pos);
    void KillUnique(const std::string &key);
    void PurgeUnique(const std::string &key);
    bool IsUniqueDead(const std::string &key);
    uint32_t GetUniqueVid(const std::string &key);
    PointValue GetUniqueHpPerc(const std::string &key);
    void UniqueSetMaxHP(const std::string &key, int64_t iMaxHP);
    void UniqueSetHP(const std::string &key, int64_t iHP);
    void UniqueSetHPPerc(const std::string &key, int iHPPct);

    void UniqueSetDefGrade(const std::string &key, int iGrade);
    void SetDungeonStage(int stage);;

    bool IsAllPCNearTo(int x, int y, int dist);

    void CreateItemGroup(const std::string &group_name, ItemGroup &item_group);
    const ItemGroup *GetItemGroup(const std::string &group_name);

    virtual void OnEnter(CHARACTER * ch) override final;
    virtual void OnLeave(CHARACTER * ch) override final;
    virtual void OnKill(CHARACTER * killer, CHARACTER * victim) override final;
    void         ExitAllDungeon();

private:
    LPEVENT deadEvent;
    LPEVENT exit_all_event_;
    LPEVENT jump_to_event_;
    LPEVENT jump_all_event_;

    int m_iMobKill;
    int m_iStoneKill;
    bool m_bUsePotion;
    bool m_bUseRevive;

    int m_iMonsterCount;

    bool m_bExitAllAtEliminate;
    bool m_bWarpAtEliminate;
    bool m_bEventAtEliminate;

    std::string m_stRegenFile;

    // 적 전멸시 워프하는 위치
    int m_iWarpDelay;
    int32_t m_lWarpMapIndex;
    int32_t m_lWarpX;
    int32_t m_lWarpY;
    int32_t m_lExitWarpMapIndex;
    int32_t m_lExitWarpX;
    int32_t m_lExitWarpY;
    uint8_t m_dungeonStage = 0;
    uint8_t m_prevDungeonStage = 0;
    uint32_t m_dungeonScriptIndex = 0;

    robin_hood::unordered_map<std::string, int> m_map_Flag;
    robin_hood::unordered_map<std::string, ItemGroup> m_map_ItemGroup;
    robin_hood::unordered_map<std::string, CHARACTER *> m_map_UniqueMob;
};

class CDungeonManager : public singleton<CDungeonManager>
{
public:
    CDungeonManager();
    ~CDungeonManager();

    template <class T> T *Create(int32_t lOriginalMapIndex);

    void Destroy(CDungeon::IdType dungeon_id);

    template <class T> T *Find(CDungeon::IdType dungeon_id);

    template <class T> T *FindByMapIndex(int32_t mapIndex);

    void		SetPlayerInfo(DWORD pid, uint32_t mapIndex);
	void		SetPlayerInfo(CHARACTER* ch);
	void		RemovePlayerInfoDungeon(uint32_t map_idx);
	void		RemovePlayerInfo(uint32_t pid);
	bool 		HasPlayerInfo(uint32_t pid) { return m_map_pkDungeonPlayerInfo.find(pid) != m_map_pkDungeonPlayerInfo.end(); }
	uint32_t GetPlayerInfo(uint32_t pid);
private:
    uint32_t CreateAux(uint32_t mapIndex);

    robin_hood::unordered_map<CDungeon::IdType, std::unique_ptr<CDungeon>> m_dungeons;
    robin_hood::unordered_map<uint32_t, CDungeon *> m_dungeonsByMap;
	robin_hood::unordered_map<uint32_t, uint32_t> m_map_pkDungeonPlayerInfo;

    // <Factor> Introduced unsigned 32-bit dungeon identifier
    CDungeon::IdType next_id_;
};

template <class T> T *CDungeonManager::Create(int32_t originalMapIndex)
{
    const auto mapIndex = CreateAux(originalMapIndex);
    if (!mapIndex)
    {
        SPDLOG_ERROR("Failed to create dungeon for %d", originalMapIndex);
        return nullptr;
    }

    // <Factor> TODO: Change id assignment, or drop it
    CDungeon::IdType id = next_id_++;
    while (Find<CDungeon>(id) != nullptr)
        id = next_id_++;

    auto dungeon = new T(id, originalMapIndex, mapIndex);

    const auto r = m_dungeons.emplace(id, std::unique_ptr<T>(dungeon));
    if (!r.second)
        return nullptr;

    m_dungeonsByMap.emplace(mapIndex, dungeon);
    return dungeon;
}

template <class T> T *CDungeonManager::Find(CDungeon::IdType dungeon_id)
{
    const auto it = m_dungeons.find(dungeon_id);
    if (it == m_dungeons.end())
        return nullptr;

#ifdef DEBUG
	if (!dynamic_cast<T*>(it->second.get())) {
		SPDLOG_ERROR("Invalid dungeon type specified.");
		return nullptr;
	}
#endif

    return static_cast<T *>(it->second.get());
}

template <class T> T *CDungeonManager::FindByMapIndex(int32_t mapIndex)
{
    const auto it = m_dungeonsByMap.find(mapIndex);
    if (it == m_dungeonsByMap.end())
        return nullptr;

#ifdef DEBUG
	if (!dynamic_cast<T*>(it->second)) {
		SPDLOG_ERROR("Invalid dungeon type specified.");
		return nullptr;
	}
#endif

    return static_cast<T *>(it->second);
}

#endif /* METIN2_SERVER_GAME_DUNGEON_H */
