#ifndef METIN2_SERVER_GAME_MOB_MANAGER_H
#define METIN2_SERVER_GAME_MOB_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MobTypes.hpp>
#include <base/robin_hood.h>

class CHARACTER;

typedef struct SMobSplashAttackInfo
{
    uint32_t dwTiming;      // 스킬 사용 후 실제로 데미지 먹힐때까지 기다리는 시간 (ms)
    uint32_t dwHitDistance; // 스킬 사용시 실제로 스킬 계산이 되는 거리 (전방 몇cm)

    SMobSplashAttackInfo(uint32_t dwTiming, uint32_t dwHitDistance)
        : dwTiming(dwTiming)
          , dwHitDistance(dwHitDistance)
    {
    }
} TMobSplashAttackInfo;

class CMobInstance
{
public:
    CMobInstance();

    PIXEL_POSITION m_posLastAttacked{}; // 마지막 맞은 위치
    uint32_t m_dwLastAttackedTime = 0;  // 마지막 맞은 시간
    uint32_t m_dwLastWarpTime = 0;

    bool m_IsBerserk = false;
    bool m_IsGodSpeed = false;
    bool m_IsRevive = false;
};

class CMobGroupGroup
{
public:
    CMobGroupGroup(uint32_t dwVnum) { m_dwVnum = dwVnum; }

    // ADD_MOB_GROUP_GROUP_PROB
    void AddMember(uint32_t dwVnum, int prob = 1)
    {
        if (prob == 0)
            return;

        if (!m_vec_iProbs.empty())
            prob += m_vec_iProbs.back();

        m_vec_iProbs.push_back(prob);
        m_vec_dwMemberVnum.push_back(dwVnum);
    }

    // END_OF_ADD_MOB_GROUP_GROUP_PROB

    uint32_t GetMember()
    {
        if (m_vec_dwMemberVnum.empty())
            return 0;

        // ADD_MOB_GROUP_GROUP_PROB
        int n = Random::get(1, m_vec_iProbs.back());
        auto it = lower_bound(m_vec_iProbs.begin(), m_vec_iProbs.end(), n);

        return m_vec_dwMemberVnum[std::distance(m_vec_iProbs.begin(), it)];
        // END_OF_ADD_MOB_GROUP_GROUP_PROB
        //return m_vec_dwMemberVnum[number(1, m_vec_dwMemberVnum.size())-1];
    }

    uint32_t m_dwVnum;
    std::vector<uint32_t> m_vec_dwMemberVnum;

    // ADD_MOB_GROUP_GROUP_PROB
    std::vector<int> m_vec_iProbs;
    // END_OF_ADD_MOB_GROUP_GROUP_PROB
};

class CMobGroup
{
public:
    void Create(uint32_t dwVnum, std::string &r_stName)
    {
        m_dwVnum = dwVnum;
        m_stName = r_stName;
    }

    const std::vector<uint32_t> &GetMemberVector() { return m_vec_dwMemberVnum; }

    int GetMemberCount() { return m_vec_dwMemberVnum.size(); }

    void AddMember(uint32_t dwVnum) { m_vec_dwMemberVnum.push_back(dwVnum); }

protected:
    uint32_t m_dwVnum;
    std::string m_stName;
    std::vector<uint32_t> m_vec_dwMemberVnum;
};

using MobExpLimitPair = std::pair<uint32_t, uint32_t>;

class CMobManager : public singleton<CMobManager>
{
public:
    using ProtoMap = robin_hood::unordered_map<uint32_t, TMobTable>;

    bool ReloadMobProto();
    bool Initialize();

    bool LoadGroup(const char *c_pszFileName, bool isReloading = false);
    bool LoadGroupGroup(const char *c_pszFileName, bool isReloading = false);
    bool LoadNames(const std::string &filename);
    CMobGroup *GetGroup(uint32_t dwVnum);
    bool ReadMountSpeedLimitConfig(const char *c_pszFileName);
    bool ReadMobExpLevelLimitConfig(const char *c_pszFileName);
    uint32_t GetGroupFromGroupGroup(uint32_t dwVnum);

    const TMobTable *Get(uint32_t dwVnum);
    const std::string &GetName(uint32_t dwVnum) const;
    const TMobTable *Get(const std::string &name, bool isAbbrev) const;

    ProtoMap::iterator begin() { return m_protoMap.begin(); }

    ProtoMap::iterator end() { return m_protoMap.end(); }

    bool MobHasExpLevelLimit(uint32_t vnum);
    MobExpLimitPair GetMobExpLevelLimit(uint32_t vnum);
    bool MountHasSpeedLimit(uint32_t vnum) const;
    uint32_t GetMountSpeedLimit(uint32_t vnum) const;
#ifdef INGAME_WIKI
    std::vector<uint32_t> &GetMobWikiInfo(uint32_t vnum) { return m_wikiInfoMap[vnum]; }
#endif
private:
    ProtoMap m_protoMap;
    robin_hood::unordered_map<uint32_t, std::string> m_mobNames;

    std::string m_noName;

    std::map<uint32_t, CMobGroup *> m_map_pkMobGroup;
    std::map<uint32_t, CMobGroupGroup *> m_map_pkMobGroupGroup;
    std::unordered_map<uint32_t, MobExpLimitPair> m_map_mobExpLimit;
    std::unordered_map<uint32_t, uint32_t> m_mapMountSpeedLimit;
#ifdef INGAME_WIKI
    std::map<uint32_t, std::vector<uint32_t>> m_wikiInfoMap;
#endif
    std::map<uint32_t, double> m_mapRegenCount;
};

#endif /* METIN2_SERVER_GAME_MOB_MANAGER_H */
