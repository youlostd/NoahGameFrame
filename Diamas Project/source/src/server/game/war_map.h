#ifndef METIN2_SERVER_GAME_WAR_MAP_H
#define METIN2_SERVER_GAME_WAR_MAP_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "constants.h"
#include "event.h"

#include <unordered_set>
#include <unordered_map>
#include <base/Singleton.hpp>


#include "char.h"
#include "desc.h"
#include "net/Type.hpp"

class CHARACTER;
class DESC;
class CItem;
class CGuild;

enum EWarMapTypes
{
    WAR_MAP_TYPE_NORMAL,
    WAR_MAP_TYPE_FLAG,
};

typedef struct SWarMapInfo
{
    uint8_t bType;
    int32_t lMapIndex;
    PIXEL_POSITION posStart[3];
} TWarMapInfo;

namespace warmap
{
enum
{
    WAR_FLAG_VNUM_START = 20035,
    WAR_FLAG_VNUM_END = 20037,

    WAR_FLAG_VNUM0 = 20035,
    WAR_FLAG_VNUM1 = 20036,
    WAR_FLAG_VNUM2 = 20037,

    WAR_FLAG_BASE_VNUM = 20038
};

inline bool IsWarFlag(uint32_t dwVnum)
{
    if (dwVnum >= WAR_FLAG_VNUM_START && dwVnum <= WAR_FLAG_VNUM_END)
        return true;

    return false;
}

inline bool IsWarFlagBase(uint32_t dwVnum) { return dwVnum == WAR_FLAG_BASE_VNUM ? true : false; }
}

class CWarMap
{
public:
    friend class CGuild;

    CWarMap(int32_t lMapIndex, const TGuildWarInfo &r_info,
            TWarMapInfo *pkWarMapInfo,
            uint32_t dwGuildID1, uint32_t dwGuildID2);

    ~CWarMap();

    bool GetTeamIndex(uint32_t dwGuild, uint8_t &bIdx);

    void IncMember(CHARACTER *ch);
    void DecMember(CHARACTER *ch);

    CGuild *GetGuild(uint8_t bIdx);
    uint32_t GetGuildID(uint8_t bIdx);
#ifdef ENABLE_GUILD_WAR_BEGIN
    DWORD GetStartTime();
    bool GetBeginProtectionStatus();
    void SetBeginProtectionStatus(bool bNewStat);

#endif
#ifdef ENABLE_NEW_GUILD_WAR
    int GetCurrentPlayer(BYTE bIdx);
#endif
    uint8_t GetType();
    int32_t GetMapIndex();
    uint32_t GetGuildOpponent(CHARACTER *ch);

    uint32_t GetWinnerGuild();
    void UsePotion(CHARACTER *ch, CItem *item);

    void Draw(); // 강제 무승부 처리
    void Timeout();
    void CheckWarEnd();
    bool SetEnded();
    void ExitAll();

    void SetBeginEvent(LPEVENT pkEv);
    void SetTimeoutEvent(LPEVENT pkEv);
    void SetEndEvent(LPEVENT pkEv);
    void SetResetFlagEvent(LPEVENT pkEv);
#ifdef ENABLE_GUILD_WAR_BEGIN
    void SetBeginProtectionEvent(LPEVENT pkEv);
#endif
    void UpdateScore(uint32_t g1, int score1, uint32_t g2, int score2);
    bool CheckScore();

    int GetRewardGold(uint8_t bWinnerIdx);

    bool GetGuildIndex(uint32_t dwGuild, int &iIndex);
    template<typename T>
    void Packet(PacketId id, const T & p);

    template <typename ...Args>
    void Notice(const char *psz, Args ... args);
    void SendWarPacket(DESC *d);
    void SendScorePacket(uint8_t bIdx, DESC *d = nullptr);

    void OnKill(CHARACTER *killer, CHARACTER *ch);

    void AddFlag(uint8_t bIdx, uint32_t x = 0, uint32_t y = 0);
    void AddFlagBase(uint8_t bIdx, uint32_t x = 0, uint32_t y = 0);
    void RemoveFlag(uint8_t bIdx);
    bool IsFlagOnBase(uint8_t bIdx);
    void ResetFlag();
  std::unordered_set<CHARACTER *>& GetMembers() { return m_set_pkChr; }

private:
    void UpdateUserCount();

private:
    TWarMapInfo m_kMapInfo;
    bool m_bEnded;
#ifdef ENABLE_GUILD_WAR_BEGIN
    bool m_bBeginProtectionCompleted;
    DWORD m_dwFirstLogonTime;
#endif
    LPEVENT m_pkBeginEvent;
#ifdef ENABLE_GUILD_WAR_BEGIN
    LPEVENT m_pkBeginProtectionEvent;
#endif
    LPEVENT m_pkTimeoutEvent;
    LPEVENT m_pkEndEvent;
    LPEVENT m_pkResetFlagEvent;
public:
    typedef struct SMemberStats
    {
        std::string strPlayerName;
        uint32_t dwGuildId;

        uint8_t byLevel;
        uint32_t dwKills;
        uint32_t dwDeaths;
        uint64_t ullDamage;

        SMemberStats()
            : dwGuildId(0), byLevel(0), dwKills(0),
              dwDeaths(0), ullDamage(0)
        {
        }
    } TMemberStats;

    typedef std::map<uint32_t, TMemberStats *> TMemberStatsMap;

    TMemberStats *RegisterMemberStats(CHARACTER *ch);
    TMemberStats *GetMemberStats(CHARACTER *ch);
    void SendStats(TMemberStats *pStats);
    void SendStatusBoard(CHARACTER *ch);

private:
    TMemberStatsMap map_MemberStats;

    typedef struct STeamData
    {
        uint32_t dwID;
        CGuild *pkGuild;
        int iMemberCount;
        int iUsePotionPrice;
        int iScore;
        CHARACTER *pkChrFlag;
        CHARACTER *pkChrFlagBase;

        std::unordered_set<uint32_t> set_pidJoiner;

        void Initialize();

        int GetAccumulatedJoinerCount(); // 누적된 참가자 수
        int GetCurJointerCount();        // 현재 참가자 수

        void AppendMember(CHARACTER *ch);
        void RemoveMember(CHARACTER *ch);
    } TeamData;

    TeamData m_TeamData[2];
    int m_iObserverCount;
    uint32_t m_dwStartTime;
    uint8_t m_bTimeout;

    TGuildWarInfo m_WarInfo;
    uint32_t m_firstBloodPid;
    uint32_t m_killCount = 0;

    std::unordered_set<CHARACTER *> m_set_pkChr;
};

class CWarMapManager : public singleton<CWarMapManager>
{
public:
    CWarMapManager();
    ~CWarMapManager();

    bool IsWarMap(int32_t lMapIndex);
    TWarMapInfo *GetWarMapInfo(int32_t lMapIndex);
    bool GetStartPosition(int32_t lMapIndex, uint8_t bIdx, PIXEL_POSITION &pos);

    template <typename Func> Func for_each(Func f);
    int32_t CreateWarMap(const TGuildWarInfo &r_WarInfo, uint32_t dwGuildID1, uint32_t dwGuildID2);
    void DestroyWarMap(CWarMap *pMap);
    CWarMap *Find(int32_t lMapIndex);

    int CountWarMap() const { return m_mapWarMap.size(); }

    void OnShutdown();

private:
    std::unordered_map<int32_t, TWarMapInfo *> m_map_kWarMapInfo;
    std::unordered_map<int32_t, CWarMap *> m_mapWarMap;
};

template <typename T>
void CWarMap::Packet(PacketId id, const T& p) {
    std::for_each(m_set_pkChr.begin(), m_set_pkChr.end(), [id, p](CHARACTER *ch) { ch->GetDesc()->Send(id, p); });

}

template <typename ... Args> void CWarMap::Notice(const char * psz, Args ...args)
{
    for (auto *ch : m_set_pkChr) { SendI18nChatPacket(ch, CHAT_TYPE_NOTICE, psz, std::forward<Args>(args)...); }
}

template <typename Func>
Func CWarMapManager::for_each(Func f)
{
    for (auto it = m_mapWarMap.begin(); it != m_mapWarMap.end(); ++it)
        f(it->second);

    return f;
}

#endif /* METIN2_SERVER_GAME_WAR_MAP_H */
