#ifndef METIN2_SERVER_GAME_PARTY_H
#define METIN2_SERVER_GAME_PARTY_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "TextTagUtil.hpp"
#include "char.h"
#include "locale.hpp"
#include <base/robin_hood.h>
class CParty;

class CPartyManager : public singleton<CPartyManager>
{
public:
    typedef std::unordered_map<uint32_t, CParty *> TPartyMap;
    typedef robin_hood::unordered_set<CParty *> TPCPartySet;

public:
    CPartyManager();
    virtual ~CPartyManager();

    void Initialize();

    // void		SendPartyToDB();

    void EnablePCParty() { m_bEnablePCParty = true; }

    void DisablePCParty() { m_bEnablePCParty = false; }

    bool IsEnablePCParty() { return m_bEnablePCParty; }

    CParty *CreateParty(CHARACTER *pkLeader);
    void DeleteParty(CParty *pParty);
    void DeleteAllParty();
    bool SetParty(CHARACTER *pkChr);

    void SetPartyMember(uint32_t dwPID, CParty *pParty);

    void P2PLogin(uint32_t pid, const char *name, uint32_t mapIndex, uint32_t channel);
    void P2PLogout(uint32_t pid, uint32_t mapIndex, uint32_t channel);

    CParty *P2PCreateParty(uint32_t pid);
    void P2PDeleteParty(uint32_t pid);
    void P2PJoinParty(uint32_t leader, uint32_t pid, uint8_t role = 0);
    void P2PUnlink(uint32_t leader, uint32_t pid, uint32_t vid);
    void P2PQuitParty(uint32_t pid);

private:
    TPartyMap m_map_pkParty;    // PID로 어느 파티에 있나 검색하기 위한 컨테이너
    TPartyMap m_map_pkMobParty; // Mob 파티는 PID 대신 VID 로 따로 관리한다.

    TPCPartySet m_set_pkPCParty; // 사람들의 파티 전체 집합

    bool m_bEnablePCParty; // 디비가 켜져있지 않으면 사람들의 파티 상태가 변경불가
};

enum EPartyMessages
{
    PM_ATTACK,
    // Attack him
    PM_RETURN,
    // Return back to position
    PM_ATTACKED_BY,
    // I was attacked by someone
    PM_AGGRO_INCREASE,
    // My aggro is increased
};

class CParty
{
public:
    typedef struct SMember
    {
        std::string strName;
        CHARACTER *pCharacter;
        uint32_t mapIndex;
        uint32_t channel;
        uint32_t race;
        bool bNear;
        uint8_t bRole;
        uint8_t bIsLeader;
        uint8_t bLevel;
    } TMember;

    typedef std::unordered_map<uint32_t, TMember> TMemberMap;

    typedef std::unordered_map<std::string, int> TFlagMap;

public:
    CParty();
    virtual ~CParty();

    void P2PJoin(uint32_t dwPID);
    void P2PQuit(uint32_t dwPID);
    virtual void Join(uint32_t dwPID);
    void Quit(uint32_t dwPID);
    void Link(CHARACTER *pkChr);
    void Unlink(CHARACTER *pkChr);

    template <typename... Args> void ChatPacketToAllMember(uint8_t type, const char *format, Args ... args);

    void UpdateOnlineState(uint32_t dwPID, const char *name, uint32_t mapIndex, uint32_t channel);
    void UpdateOfflineState(uint32_t dwPID, uint32_t mapIndex, uint32_t channel);

    uint32_t GetLeaderPID();
    CHARACTER *GetLeaderCharacter();

    CHARACTER *GetLeader() { return m_pkChrLeader; }

    uint32_t GetMemberCount();

    uint32_t GetNearMemberCount() { return m_iCountNearPartyMember; }

    bool IsMember(uint32_t pid) const { return m_memberMap.find(pid) != m_memberMap.end(); }

    bool IsMemberExluding(uint32_t pid, uint32_t excludedPid)
    {
        return std::find_if(m_memberMap.begin(), m_memberMap.end(),
                            [&excludedPid, &pid](const TMemberMap::value_type &item)
                            {
                                return item.first == pid && item.first != excludedPid;
                            }) != m_memberMap.end();
    }

    bool IsNearLeader(uint32_t pid);

    bool IsPositionNearLeader(CHARACTER *ch);

    void SendMessage(CHARACTER *ch, uint8_t bMsg, uint32_t dwArg1, uint32_t dwArg2);

    void SendPartyJoinOneToAll(uint32_t dwPID);
    void SendPartyJoinAllToOne(CHARACTER *ch);
    void SendPartyRemoveOneToAll(uint32_t dwPID);

    void SendPartyInfoOneToAll(uint32_t pid);
    void SendPartyInfoOneToAll(CHARACTER *ch);
    void SendPartyInfoAllToOne(CHARACTER *ch);
    void SendPartyPositionOneToAll(CHARACTER *ch);
    void SendPartyPositionAllToOne(CHARACTER *ch);

    void SendPartyLinkOneToAll(CHARACTER *ch);
    void SendPartyLinkAllToOne(CHARACTER *ch);
    void SendPartyUnlinkOneToAll(CHARACTER *ch);
    void SendPartyUnlinkOneToAllP2P(uint32_t pid, uint32_t vid);

    int GetPartyBonusExpPercent() { return m_iExpBonus; }

    int GetPartyBonusAttackGrade() { return m_iAttBonus; }

    int GetPartyBonusDefenseGrade() { return m_iDefBonus; }

    int ComputePartyBonusExpPercent();
    inline int ComputePartyBonusAttackGrade();
    inline int ComputePartyBonusDefenseGrade();

    template <class Func> void ForEachMember(Func &f);
    template <class Func> void ForEachMemberPtr(Func &f);
    template <class Func> void ForEachOnlineMember(Func&& f);
    template <class Func> void ForEachNearMember(Func & f);
    template <class Func> void ForEachOnMapMember(Func & f, long lMapIndex);
    template <class Func> bool ForEachOnMapMemberBool(Func & f, long lMapIndex);

    void Update();

    int GetExpBonusPercent();

    bool SetRole(uint32_t pid, uint8_t bRole, bool on);
int8_t GetRace(uint32_t pid);
uint8_t GetRole(uint32_t pid);
uint8_t IsLeaderByPid(uint32_t pid);
bool IsRole(uint32_t pid, uint8_t bRole);

    uint8_t GetMemberMaxLevel();
    uint8_t GetMemberMinLevel();

    void ComputeRolePoint(CHARACTER *ch, uint8_t bRole, bool bAdd);

    void SummonToLeader(uint32_t pid);

    void SetPCParty(bool b) { m_bPCParty = b; }

    CHARACTER *GetNextOwnership(CHARACTER *ch, int32_t x, int32_t y, int32_t mapindex);

    void SetFlag(const std::string &name, int value);
    int GetFlag(const std::string &name);

    uint8_t CountMemberByVnum(uint32_t dwVnum);

    void SetParameter(int iMode);
    int GetExpDistributionMode();

    void SetExpCentralizeCharacter(uint32_t pid);
    CHARACTER *GetExpCentralizeCharacter();

    void RequestSetMemberLevel(uint32_t pid, uint8_t level);
    void P2PSetMemberLevel(uint32_t pid, uint8_t level);

    bool IsPartyInDungeon(int mapIndex);
    CHARACTER *Dice(CHARACTER *, CItem *item);

protected:
    void IncreaseOwnership();

    virtual void Initialize();
    void Destroy();
    void RemovePartyBonus();

    void RemoveBonus();
    void RemoveBonusForOne(uint32_t pid);

    void SendParameter(CHARACTER *ch);
    void SendParameterToAll();

    TMemberMap m_memberMap;
    uint32_t m_dwLeaderPID;
    CHARACTER *m_pkChrLeader;

    LPEVENT m_eventUpdate;

    TMemberMap::iterator m_itNextOwner;

private:
    TFlagMap m_map_iFlag;
    int m_anRoleCount[PARTY_ROLE_MAX_NUM];
    int m_anMaxRole[PARTY_ROLE_MAX_NUM];

    int m_iExpDistributionMode;
    CHARACTER *m_pkChrExpCentralize;

    uint32_t m_dwPartyStartTime;

    int m_iLongTimeExpBonus;

    // used in Update
    int m_iLeadership;
    int m_iExpBonus;
    int m_iAttBonus;
    int m_iDefBonus;

    // changed only in Update
    int m_iCountNearPartyMember;

    bool m_bPCParty;
};

template <typename... Args> void CParty::ChatPacketToAllMember(uint8_t type, const char *format, Args ... args)
{
    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        TMember &rMember = it->second;

        if (rMember.pCharacter)
        {
            if (rMember.pCharacter->GetDesc())
            {
                SendI18nChatPacket(rMember.pCharacter, type, format, std::forward<Args>(args)...);
            }
        }
    }
}

template <class Func> void CParty::ForEachMember(Func &f)
{
    for (auto &ent : m_memberMap)
        f(ent.first);
}

template <class Func> void CParty::ForEachMemberPtr(Func &f)
{
    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
        f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachOnlineMember(Func &&f)
{
    TMemberMap::iterator it;
    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
        if (it->second.pCharacter)
            f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachNearMember(Func &f)
{
    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
        if (it->second.pCharacter && it->second.bNear)
            f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachOnMapMember(Func &f, long lMapIndex)
{
    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch = it->second.pCharacter;
        if (ch)
        {
            if (ch->GetMapIndex() == lMapIndex)
                f(ch);
        }
    }
}

template <class Func> bool CParty::ForEachOnMapMemberBool(Func &f, long lMapIndex)
{
    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch = it->second.pCharacter;
        if (ch) { if (ch->GetMapIndex() == lMapIndex) { if (f(ch) == false) { return false; } } }
    }
    return true;
}

inline int CParty::ComputePartyBonusAttackGrade()
{
    /*
    if (GetNearMemberCount() <= 1)
    return 0;

    int leadership = GetLeaderCharacter()->GetLeadershipSkillLevel();
    int n = GetNearMemberCount();

    if (n >= 3 && leadership >= 10)
    return 2;

    if (n >= 2 && leadership >= 4)
    return 1;
    */
    return 0;
}

inline int CParty::ComputePartyBonusDefenseGrade()
{
    /*
    if (GetNearMemberCount() <= 1)
    return 0;

    int leadership = GetLeaderCharacter()->GetLeadershipSkillLevel();
    int n = GetNearMemberCount();

    if (n >= 5 && leadership >= 24)
    return 2;

    if (n >= 4 && leadership >= 16)
    return 1;
    */
    return 0;
}

#endif /* METIN2_SERVER_GAME_PARTY_H */
