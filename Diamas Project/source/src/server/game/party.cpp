#include "utils.h"
#include "char.h"
#include "party.h"
#include "char_manager.h"
#include "config.h"

#include "DbCacheSocket.hpp"
#include "dungeon.h"
#include "log.h"
#include "TextTagUtil.hpp"

#include "desc.h"
#include "desc_manager.h"
#include "item.h"
#include "ItemUtils.h"
#include "game/MasterPackets.hpp"

CPartyManager::CPartyManager() { Initialize(); }

CPartyManager::~CPartyManager()
{
}

void CPartyManager::Initialize() { m_bEnablePCParty = false; }

void CPartyManager::DeleteAllParty()
{
    TPCPartySet::iterator it = m_set_pkPCParty.begin();

    while (it != m_set_pkPCParty.end())
    {
        DeleteParty(*it);
        it = m_set_pkPCParty.begin();
    }
}

bool CPartyManager::SetParty(CHARACTER *ch) // PC만 사용해야 한다!!
{
    TPartyMap::iterator it = m_map_pkParty.find(ch->GetPlayerID());

    if (it == m_map_pkParty.end())
        return false;

    CParty *pParty = it->second;
    pParty->Link(ch);
    return true;
}

void CPartyManager::P2PLogin(uint32_t pid, const char *name, uint32_t mapIndex, uint32_t channel)
{
    TPartyMap::iterator it = m_map_pkParty.find(pid);

    if (it == m_map_pkParty.end())
        return;

    it->second->UpdateOnlineState(pid, name, mapIndex, channel);
}

void CPartyManager::P2PLogout(uint32_t pid, uint32_t mapIndex, uint32_t channel)
{
    TPartyMap::iterator it = m_map_pkParty.find(pid);

    if (it == m_map_pkParty.end())
        return;

    it->second->UpdateOfflineState(pid, mapIndex, channel);
}

void CPartyManager::P2PJoinParty(uint32_t leader, uint32_t pid, uint8_t role)
{
    auto it = m_map_pkParty.find(leader);

    if (it != m_map_pkParty.end())
    {
        it->second->P2PJoin(pid);

        if (role >= PARTY_ROLE_MAX_NUM)
            role = PARTY_ROLE_NORMAL;

        it->second->SetRole(pid, role, true);
    }
    else { SPDLOG_ERROR("No such party with leader [%d]", leader); }
}

void CPartyManager::P2PUnlink(uint32_t leader, uint32_t pid, uint32_t vid)
{
    TPartyMap::iterator it = m_map_pkParty.find(leader);

    if (it != m_map_pkParty.end()) { it->second->SendPartyUnlinkOneToAllP2P(pid, vid); }
    else { SPDLOG_ERROR("No such party with leader [%d]", leader); }
}

void CPartyManager::P2PQuitParty(uint32_t pid)
{
    TPartyMap::iterator it = m_map_pkParty.find(pid);

    if (it != m_map_pkParty.end()) { it->second->P2PQuit(pid); }
    else { SPDLOG_ERROR("No such party with member [%d]", pid); }
}

CParty *CPartyManager::P2PCreateParty(uint32_t pid)
{
    auto it = m_map_pkParty.find(pid);
    if (it != m_map_pkParty.end())
        return it->second;

    auto pParty = new CParty;

    m_set_pkPCParty.insert(pParty);

    SetPartyMember(pid, pParty);
    pParty->SetPCParty(true);
    pParty->P2PJoin(pid);

    return pParty;
}

void CPartyManager::P2PDeleteParty(uint32_t pid)
{
    TPartyMap::iterator it = m_map_pkParty.find(pid);

    if (it != m_map_pkParty.end())
    {
        m_set_pkPCParty.erase(it->second);
        delete (it->second);
    }
    else
        SPDLOG_ERROR("PARTY P2PDeleteParty Cannot find party [%u]", pid);
}

CParty *CPartyManager::CreateParty(CHARACTER *pLeader)
{
    if (pLeader->GetParty())
        return pLeader->GetParty();

    CParty *pParty = new CParty;

    if (pLeader->IsPC())
    {
        //TPacketGGParty p;
        //p.header	= HEADER_GG_PARTY;
        //p.subheader	= PARTY_SUBHEADER_GG_CREATE;
        //p.pid		= pLeader->GetPlayerID();
        //P2P_MANAGER::instance().Send(&p, sizeof(p));
        TPacketPartyCreate p;
        p.dwLeaderPID = pLeader->GetPlayerID();

        db_clientdesc->DBPacket(HEADER_GD_PARTY_CREATE, 0, &p, sizeof(TPacketPartyCreate));

        SPDLOG_INFO("PARTY: Create %s pid %u", pLeader->GetName(), pLeader->GetPlayerID());
        LogManager::instance().CharLog(pLeader, 0, "PARTY_CREATE", "");
        pParty->SetPCParty(true);
        pParty->Join(pLeader->GetPlayerID());

        m_set_pkPCParty.insert(pParty);
    }
    else
    {
        pParty->SetPCParty(false);
        pParty->Join(pLeader->GetVID());
    }

    pParty->Link(pLeader);
    return (pParty);
}

void CPartyManager::DeleteParty(CParty *pParty)
{
    //TPacketGGParty p;
    //p.header = HEADER_GG_PARTY;
    //p.subheader = PARTY_SUBHEADER_GG_DESTROY;
    //p.pid = pParty->GetLeaderPID();
    //P2P_MANAGER::instance().Send(&p, sizeof(p));
    TPacketPartyDelete p;
    p.dwLeaderPID = pParty->GetLeaderPID();
    db_clientdesc->DBPacket(HEADER_GD_PARTY_DELETE, 0, &p, sizeof(TPacketPartyDelete));
    m_set_pkPCParty.erase(pParty);
    if (pParty->GetLeaderPID() != 0)
        LogManager::instance().CharLog(pParty->GetLeader(), 0, "PARTY_DESTROY", "");
    delete (pParty);
}

void CPartyManager::SetPartyMember(uint32_t dwPID, CParty *pParty)
{
    auto it = m_map_pkParty.find(dwPID);

    if (pParty == nullptr)
    {
        if (it != m_map_pkParty.end())
            m_map_pkParty.erase(it);
    }
    else
    {
        if (it != m_map_pkParty.end())
        {
            if (it->second != pParty)
            {
                it->second->Quit(dwPID);
                it->second = pParty;
            }
        }
        else
            m_map_pkParty.emplace(dwPID, pParty);
    }
}

EVENTINFO(party_update_event_info)
{
    uint32_t pid;

    party_update_event_info()
        : pid(0)
    {
    }
};

/////////////////////////////////////////////////////////////////////////////
//
// CParty begin!
//
/////////////////////////////////////////////////////////////////////////////
EVENTFUNC(party_update_event)
{
    party_update_event_info *info = static_cast<party_update_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("party_update_event> <Factor> Null pointer");
        return 0;
    }

    uint32_t pid = info->pid;
    CHARACTER *leader = g_pCharManager->FindByPID(pid);

    if (leader && leader->GetDesc())
    {
        CParty *pParty = leader->GetParty();

        if (pParty)
            pParty->Update();
    }

    return THECORE_SECS_TO_PASSES(3);
}

CParty::CParty() { Initialize(); }

CParty::~CParty() { Destroy(); }

void CParty::Initialize()
{
    SPDLOG_TRACE("Party::Initialize");

    m_iExpDistributionMode = PARTY_EXP_DISTRIBUTION_NON_PARITY;
    m_pkChrExpCentralize = nullptr;

    m_dwLeaderPID = 0;

    m_eventUpdate = nullptr;

    memset(&m_anRoleCount, 0, sizeof(m_anRoleCount));
    memset(&m_anMaxRole, 0, sizeof(m_anMaxRole));
    m_anMaxRole[PARTY_ROLE_LEADER] = 1;
    m_anMaxRole[PARTY_ROLE_NORMAL] = 32;

    m_dwPartyStartTime = get_dword_time();
    m_iLongTimeExpBonus = 0;

    m_iLeadership = 0;
    m_iExpBonus = 0;
    m_iAttBonus = 0;
    m_iDefBonus = 0;

    m_itNextOwner = m_memberMap.begin();

    m_iCountNearPartyMember = 0;

    m_pkChrLeader = nullptr;
    m_bPCParty = false;
}

void CParty::Destroy()
{
    // PC가 만든 파티면 파티매니저에 맵에서 PID를 삭제해야 한다.
    if (m_bPCParty)
    {
        for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
            CPartyManager::instance().SetPartyMember(it->first, nullptr);
    }

    event_cancel(&m_eventUpdate);

    RemoveBonus();

    TMemberMap::iterator it = m_memberMap.begin();

    uint32_t dwTime = get_dword_time();

    for (auto &elem : m_memberMap)
    {
        TMember &rMember = elem.second;
        if (rMember.pCharacter)
        {
            SendPartyRemoveOneToAll(rMember.pCharacter->GetPlayerID());

            if (rMember.pCharacter->GetDesc())
            {
                TPacketGCPartyRemove p;
                p.pid = rMember.pCharacter->GetPlayerID();
                rMember.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_REMOVE, p);

                SendI18nChatPacket(rMember.pCharacter, CHAT_TYPE_INFO, "<파티> 파티가 해산 되었습니다.");
            }
            else
            {
                // NPC일 경우 일정 시간 후 전투 중이 아닐 때 사라지게 하는 이벤트를 시작시킨다.
                rMember.pCharacter->SetLastAttacked(dwTime);
                rMember.pCharacter->StartDestroyWhenIdleEvent();
            }

            rMember.pCharacter->SetParty(nullptr);
        }
    }

    m_memberMap.clear();
    m_itNextOwner = m_memberMap.begin();
}

uint32_t CParty::GetLeaderPID() { return m_dwLeaderPID; }

uint32_t CParty::GetMemberCount() { return m_memberMap.size(); }

void CParty::P2PJoin(uint32_t dwPID)
{
    TMemberMap::iterator it = m_memberMap.find(dwPID);

    if (it == m_memberMap.end())
    {
        TMember Member;

        Member.pCharacter = nullptr;
        Member.bNear = false;

        if (m_memberMap.empty())
        {
            Member.bRole = PARTY_ROLE_NORMAL;
            Member.bIsLeader = true;
            m_dwLeaderPID = dwPID;
        }
        else {
            Member.bRole = PARTY_ROLE_NORMAL;
            Member.bIsLeader = false;
        }

        if (m_bPCParty)
        {
            CHARACTER *ch = g_pCharManager->FindByPID(dwPID);

            if (ch)
            {
                SPDLOG_INFO("PARTY: Join %s pid %u leader %u", ch->GetName(), dwPID, m_dwLeaderPID);
                Member.strName = ch->GetName();
                Member.mapIndex = ch->GetMapIndex();
                Member.race = ch->GetRaceNum();
                Member.channel = gConfig.channel;

                if (Member.bIsLeader)
                    m_iLeadership = ch->GetLeadershipSkillLevel();
            }
            else
            {
                const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(dwPID);
                if (op && op->channel == gConfig.channel)
                {
                    Member.strName = op->name;
                    Member.mapIndex = op->mapIndex;
                    Member.channel = op->channel;
                    Member.race = 0; // TODO
                }
            }
        }

        SPDLOG_TRACE("PARTY[{0}] MemberCountChange {1} -> {2}", GetLeaderPID(), GetMemberCount(), GetMemberCount() + 1);

        m_memberMap.emplace(dwPID, Member);

        if (m_memberMap.size() == 1)
            m_itNextOwner = m_memberMap.begin();

        if (m_bPCParty)
        {
            CPartyManager::instance().SetPartyMember(dwPID, this);
            SendPartyJoinOneToAll(dwPID);

            CHARACTER *ch = g_pCharManager->FindByPID(dwPID);

            if (ch)
                SendParameter(ch);
        }
    }
}

void CParty::Join(uint32_t dwPID)
{
    P2PJoin(dwPID);

    if (m_bPCParty)
    {
        TPacketPartyAdd p;
        p.dwLeaderPID = GetLeaderPID();
        p.dwPID = dwPID;
        p.bState = PARTY_ROLE_NORMAL; // #0000790: [M2EU] CZ 크래쉬 증가: 초기화 중요! 
        db_clientdesc->DBPacket(HEADER_GD_PARTY_ADD, 0, &p,
                                sizeof(TPacketPartyAdd));

        if (dwPID != 0)
            LogManager::instance().CharLog(g_pCharManager->FindByPID(dwPID), 0, "PARTY_JOIN", "");
    }
}

void CParty::P2PQuit(uint32_t dwPID)
{
    TMemberMap::iterator it = m_memberMap.find(dwPID);

    if (it == m_memberMap.end())
        return;

    if (m_bPCParty)
        SendPartyRemoveOneToAll(dwPID);

    if (it == m_itNextOwner)
        IncreaseOwnership();

    if (m_bPCParty)
        RemoveBonusForOne(dwPID);

    CHARACTER *ch = it->second.pCharacter;
    uint8_t bRole = it->second.bRole;
    bool isLeader = it->second.bIsLeader;

    m_memberMap.erase(it);

    SPDLOG_TRACE("PARTY[{0}] MemberCountChange {1} -> {2}", GetLeaderPID(),
                 GetMemberCount(), GetMemberCount() - 1);

    if (bRole < PARTY_ROLE_MAX_NUM) { --m_anRoleCount[bRole]; }
    else
    {
        SPDLOG_ERROR("ROLE_COUNT_QUIT_ERROR: INDEX({0}) > MAX({1})", bRole,
                     PARTY_ROLE_MAX_NUM);
    }

    if (ch)
    {
        ch->SetParty(nullptr);
        ComputeRolePoint(ch, bRole, false);
    }

    if (m_bPCParty)
        CPartyManager::instance().SetPartyMember(dwPID, nullptr);

    if (isLeader)
        CPartyManager::instance().DeleteParty(this);
}

void CParty::Quit(uint32_t dwPID)
{
    // Always PC
    P2PQuit(dwPID);

    if (m_bPCParty && dwPID != GetLeaderPID())
    {
        //TPacketGGParty p;
        //p.header = HEADER_GG_PARTY;
        //p.subheader = PARTY_SUBHEADER_GG_QUIT;
        //p.pid = dwPID;
        //p.leaderpid = GetLeaderPID();
        //P2P_MANAGER::instance().Send(&p, sizeof(p));
        TPacketPartyRemove p;
        p.dwPID = dwPID;
        p.dwLeaderPID = GetLeaderPID();
        db_clientdesc->DBPacket(HEADER_GD_PARTY_REMOVE, 0, &p, sizeof(p));
    }
}

void CParty::Link(CHARACTER *pkChr)
{
    TMemberMap::iterator it;

    if (pkChr->IsPC())
        it = m_memberMap.find(pkChr->GetPlayerID());
    else
        it = m_memberMap.find(pkChr->GetVID());

    if (it == m_memberMap.end())
    {
        SPDLOG_ERROR("%s is not member of this party", pkChr->GetName());
        return;
    }

    // 플레이어 파티일 경우 업데이트 이벤트 생성
    if (m_bPCParty && !m_eventUpdate)
    {
        party_update_event_info *info = AllocEventInfo<party_update_event_info>();
        info->pid = m_dwLeaderPID;
        m_eventUpdate = event_create(party_update_event, info, THECORE_SECS_TO_PASSES(3));
    }

    if (it->second.bIsLeader)
        m_pkChrLeader = pkChr;

    SPDLOG_TRACE("PARTY[%d] %s linked to party", GetLeaderPID(), pkChr->GetName());

    it->second.pCharacter = pkChr;
    pkChr->SetParty(this);

    if (pkChr->IsPC())
    {
        if (it->second.strName.empty()) { it->second.strName = pkChr->GetName(); }

        SendPartyJoinOneToAll(pkChr->GetPlayerID());

        SendPartyJoinAllToOne(pkChr);
        SendPartyLinkOneToAll(pkChr);
        SendPartyLinkAllToOne(pkChr);
        SendPartyInfoAllToOne(pkChr);
        SendPartyInfoOneToAll(pkChr);
        SendPartyPositionAllToOne(pkChr);
        SendPartyPositionOneToAll(pkChr);

        SendParameter(pkChr);

        RequestSetMemberLevel(pkChr->GetPlayerID(), pkChr->GetLevel());
    }
}

void CParty::RequestSetMemberLevel(uint32_t pid, uint8_t level)
{
    TPacketPartySetMemberLevel p;
    p.dwLeaderPID = GetLeaderPID();
    p.dwPID = pid;
    p.bLevel = level;
    db_clientdesc->DBPacket(HEADER_GD_PARTY_SET_MEMBER_LEVEL, 0, &p, sizeof(TPacketPartySetMemberLevel));
}

void CParty::P2PSetMemberLevel(uint32_t pid, uint8_t level)
{
    if (!m_bPCParty)
        return;

    TMemberMap::iterator it;

    SPDLOG_INFO("PARTY P2PSetMemberLevel leader %d pid %d level %d", GetLeaderPID(), pid, level);

    it = m_memberMap.find(pid);
    if (it != m_memberMap.end()) { it->second.bLevel = level; }
}

namespace
{
struct FExitDungeon
{
    void operator()(CHARACTER *ch) { ch->ExitToSavedLocation(); }
};
}

void CParty::Unlink(CHARACTER *pkChr)
{
    TMemberMap::iterator it;

    if (pkChr->IsPC())
        it = m_memberMap.find(pkChr->GetPlayerID());
    else
        it = m_memberMap.find(pkChr->GetVID());

    if (it == m_memberMap.end())
    {
        SPDLOG_ERROR("%s is not member of this party", pkChr->GetName());
        return;
    }

    if (pkChr->IsPC())
    {
        SendPartyUnlinkOneToAll(pkChr);
        //SendPartyUnlinkAllToOne(pkChr); // 끊기는 것이므로 구지 Unlink 패킷을 보낼 필요 없다.

        if (it->second.bIsLeader)
        {
            // NOTE: Remember this on: Dungeon leave time refactoring 
            RemoveBonus();

            if (it->second.pCharacter->GetDungeon())
            {
                FExitDungeon f;
                ForEachNearMember(f);
            }
        }
    }

    if (it->second.bIsLeader)
        m_pkChrLeader = nullptr;

    it->second.pCharacter = nullptr;
    pkChr->SetParty(nullptr);
}

void CParty::SendPartyRemoveOneToAll(uint32_t pid)
{

    TPacketGCPartyRemove p;
    p.pid = pid;

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_REMOVE, p);
    }
}

void CParty::SendPartyJoinOneToAll(uint32_t pid)
{
    const TMember &r = m_memberMap[pid];

    TPacketGCPartyAdd p;
    p.pid = pid;
    p.name = r.strName;
    p.mapIndex = r.mapIndex;
    p.channel = r.channel;
    p.race = r.race;

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_ADD, p);
    }
}

void CParty::SendPartyJoinAllToOne(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCPartyAdd p;
    for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        p.pid = it->first;
        p.name = it->second.strName;
        p.mapIndex = it->second.mapIndex;
        p.race = it->second.race;
        p.channel = it->second.channel;
        ch->GetDesc()->Send(HEADER_GC_PARTY_ADD, p);
    }
}

void CParty::SendPartyUnlinkOneToAll(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCPartyUnlink p;
    p.pid = ch->GetPlayerID();
    p.vid = (uint32_t)ch->GetVID();

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
        {
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_UNLINK, p);
        }
    }

    /*TPacketGGPartyUnlinkOneToAll p2 = {};
    p2.header = HEADER_GG_PARTY_UNLINK_ONE_TO_ALL;
    p2.pid = ch->GetPlayerID();
    p2.vid = (uint32_t)ch->GetVID();
    p2.leader = GetLeaderPID();
    P2P_MANAGER::instance().Send(&p2, sizeof(TPacketGGPartyUnlinkOneToAll));
    */
}

void CParty::SendPartyUnlinkOneToAllP2P(uint32_t pid, uint32_t vid)
{
    TPacketGCPartyUnlink p;
    p.pid = pid;
    p.vid = vid;

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
        {
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_UNLINK, p);
        }
    }
}

void CParty::SendPartyLinkOneToAll(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCPartyLink p = {};
    p.vid = ch->GetVID();
    p.pid = ch->GetPlayerID();
    p.mapIndex = ch->GetMapIndex();
    p.channel = gConfig.channel;
    p.race = ch->GetRaceNum();

    for (const auto &member : m_memberMap)
    {
        if (member.second.pCharacter && member.second.pCharacter->GetDesc())
            member.second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_LINK, p);
    }
}

void CParty::SendPartyLinkAllToOne(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCPartyLink p = {};

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter)
        {
            p.vid = it->second.pCharacter->GetVID();
            p.pid = it->second.pCharacter->GetPlayerID();
            p.mapIndex = it->second.pCharacter->GetMapIndex();
            p.race = it->second.pCharacter->GetRaceNum();
            p.channel = it->second.channel;

            ch->GetDesc()->Send(HEADER_GC_PARTY_LINK, p);
        }
    }
}

void CParty::SendPartyInfoOneToAll(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return;

    if (it->second.pCharacter)
    {
        SendPartyInfoOneToAll(it->second.pCharacter);
        return;
    }

    // Data Building
    TPacketGCPartyUpdate p = {};
    p.pid = pid;
    p.percent_hp = 255;
    p.role = it->second.bRole;
    p.race = it->second.race;
    p.is_leader = it->second.bIsLeader;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if ((it->second.pCharacter) && (it->second.pCharacter->GetDesc()))
        {
            //SPDLOG_TRACE("PARTY send info %s[%d] to %s[%d]", ch->GetName(), (uint32_t)ch->GetVID(), it->second.pCharacter->GetName(), (uint32_t)it->second.pCharacter->GetVID());
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_UPDATE, p);
        }
    }
}

void CParty::SendPartyInfoOneToAll(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    // Data Building
    TPacketGCPartyUpdate p;
    ch->BuildUpdatePartyPacket(p);

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if ((it->second.pCharacter) && (it->second.pCharacter->GetDesc()))
        {
            SPDLOG_TRACE("PARTY send info %s[%d] to %s[%d]", ch->GetName(), (uint32_t)ch->GetVID(),
                         it->second.pCharacter->GetName(), (uint32_t)it->second.pCharacter->GetVID());
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_UPDATE, p);
        }
    }
}

void CParty::SendPartyInfoAllToOne(CHARACTER *ch)
{
    TPacketGCPartyUpdate p = {};

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (!it->second.pCharacter)
        {
            uint32_t pid = it->first;
            p = {};
            p.pid = pid;
            p.percent_hp = 255;
            p.role = it->second.bRole;
            p.race = it->second.race;
            p.is_leader = it->second.bIsLeader;
            ch->GetDesc()->Send(HEADER_GC_PARTY_UPDATE, p);
            continue;
        }

        it->second.pCharacter->BuildUpdatePartyPacket(p);
        ch->GetDesc()->Send(HEADER_GC_PARTY_UPDATE, p);
    }
}

void CParty::SendPartyPositionOneToAll(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCPartyPositionInfo p;
    p.pid = ch->GetPlayerID();
    p.x = ch->GetX();
    p.y = ch->GetY();
    p.rot = ch->GetRotation();
    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if ((it->second.pCharacter) && (it->second.pCharacter->GetDesc()) &&
            (it->second.pCharacter->GetMapIndex() == ch->GetMapIndex()))
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_POSITION_INFO, p);
    }
}

void CParty::SendPartyPositionAllToOne(CHARACTER *ch)
{
    TPacketGCPartyPositionInfo p;
    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetMapIndex() == ch->GetMapIndex())
        {
            uint32_t pid = it->first;
            p = {};
            p.pid = pid;
            p.x = it->second.pCharacter->GetX();
            p.y = it->second.pCharacter->GetX();
            p.rot = it->second.pCharacter->GetRotation();
            ch->GetDesc()->Send(HEADER_GC_PARTY_POSITION_INFO, p);
        }
    }
}

void CParty::SendMessage(CHARACTER *ch, uint8_t bMsg, uint32_t dwArg1, uint32_t dwArg2)
{
    if (ch->GetParty() != this)
    {
        SPDLOG_ERROR("{} is not member of this party {:p}", ch->GetName(), (void*)this);
        return;
    }

    switch (bMsg)
    {
    case PM_ATTACK:
        break;

    case PM_RETURN: {
        TMemberMap::iterator it = m_memberMap.begin();

        while (it != m_memberMap.end())
        {
            TMember &rMember = it->second;
            ++it;

            CHARACTER *pkChr;

            if ((pkChr = rMember.pCharacter) && ch != pkChr)
            {
                uint32_t x = dwArg1 + Random::get(-500, 500);
                uint32_t y = dwArg2 + Random::get(-500, 500);

                pkChr->SetVictim(nullptr);
                pkChr->SetRotationToXY(x, y);
                pkChr->Goto(x, y);
            }
        }
    }
    break;

    case PM_ATTACKED_BY: // 공격 받았음, 리더에게 도움을 요청
    {
        // 리더가 없을 때
        CHARACTER *pkChrVictim = ch->GetVictim();

        if (!pkChrVictim)
            return;

        TMemberMap::iterator it = m_memberMap.begin();

        while (it != m_memberMap.end())
        {
            TMember &rMember = it->second;
            ++it;

            CHARACTER *pkChr;

            if ((pkChr = rMember.pCharacter) && ch != pkChr)
            {
                if (pkChr->CanBeginFight())
                    pkChr->BeginFight(pkChrVictim);
            }
        }
    }
    break;

    case PM_AGGRO_INCREASE: {
        CHARACTER *victim = g_pCharManager->Find(dwArg2);

        if (!victim)
            return;

        TMemberMap::iterator it = m_memberMap.begin();

        while (it != m_memberMap.end())
        {
            TMember &rMember = it->second;
            ++it;

            CHARACTER *pkChr;

            if ((pkChr = rMember.pCharacter) && ch != pkChr)
            {
                pkChr->UpdateAggrPoint(victim, DAMAGE_TYPE_SPECIAL, dwArg1);
            }
        }
    }
    break;
    }
}

CHARACTER *CParty::GetLeaderCharacter() { return m_memberMap[GetLeaderPID()].pCharacter; }

bool CParty::SetRole(uint32_t dwPID, uint8_t bRole, bool bSet)
{
    TMemberMap::iterator it = m_memberMap.find(dwPID);

    if (it == m_memberMap.end()) { return false; }

    CHARACTER *ch = it->second.pCharacter;

    if (bSet)
    {
        if (m_anRoleCount[bRole] >= m_anMaxRole[bRole])
            return false;

        it->second.bRole = bRole;

        if (ch && GetLeader())
            ComputeRolePoint(ch, bRole, true);

        if (bRole < PARTY_ROLE_MAX_NUM) { ++m_anRoleCount[bRole]; }
        else { SPDLOG_ERROR("ROLE_COUNT_INC_ERROR: INDEX(%d) > MAX(%d)", bRole, PARTY_ROLE_MAX_NUM); }
    }
    else
    {
        if (it->second.bRole == PARTY_ROLE_NORMAL)
            return false;

        it->second.bRole = PARTY_ROLE_NORMAL;

        if (ch && GetLeader())
            ComputeRolePoint(ch, PARTY_ROLE_NORMAL, false);

        if (bRole < PARTY_ROLE_MAX_NUM) { --m_anRoleCount[bRole]; }
        else { SPDLOG_ERROR("ROLE_COUNT_DEC_ERROR: INDEX(%d) > MAX(%d)", bRole, PARTY_ROLE_MAX_NUM); }
    }

    SendPartyInfoOneToAll(dwPID);
    return true;
}

int8_t CParty::GetRace(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return -1;
    else
        return it->second.race;
}

uint8_t CParty::GetRole(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return PARTY_ROLE_NORMAL;
    else
        return it->second.bRole;
}

uint8_t CParty::IsLeaderByPid(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return 0;
    else
        return it->second.bIsLeader;
}

bool CParty::IsRole(uint32_t pid, uint8_t bRole)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return false;

    return it->second.bRole == bRole;
}

void CParty::RemoveBonus()
{
    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch;

        if ((ch = it->second.pCharacter)) { ComputeRolePoint(ch, it->second.bRole, false); }

        it->second.bNear = false;
    }
}

void CParty::RemoveBonusForOne(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return;

    CHARACTER *ch;

    if ((ch = it->second.pCharacter))
        ComputeRolePoint(ch, it->second.bRole, false);
}

void CParty::SummonToLeader(uint32_t pid)
{
    if(pid == GetLeaderPID())
        return;

    if (m_memberMap.find(pid) == m_memberMap.end())
        return;
    
    if(!IsSummonableZone(m_memberMap[GetLeaderPID()].mapIndex))
        return;

    CHARACTER *ch = m_memberMap[pid].pCharacter;
    if(ch) {
        if(GetLeaderCharacter()) {
            ch->WarpSet(GetLeaderCharacter()->GetMapIndex(), GetLeaderCharacter()->GetX(), GetLeaderCharacter()->GetY());
        } else {
            ch->WarpToPID(GetLeaderPID());
        }
        return;
    }

    const auto* op = DESC_MANAGER::instance().GetOnlinePlayers().Get(pid);
    if (!op)
        return;

    GmCharacterTransferPacket p;
    p.sourcePid = pid;
    p.targetPid = GetLeaderPID();
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterTransfer, p);

}

void CParty::IncreaseOwnership()
{
    if (m_memberMap.empty())
    {
        m_itNextOwner = m_memberMap.begin();
        return;
    }

    if (m_itNextOwner == m_memberMap.end())
        m_itNextOwner = m_memberMap.begin();
    else
    {
        ++m_itNextOwner;

        if (m_itNextOwner == m_memberMap.end())
            m_itNextOwner = m_memberMap.begin();
    }
}

CHARACTER *CParty::GetNextOwnership(CHARACTER *ch, int32_t x, int32_t y, int32_t mapindex)
{
    if (m_itNextOwner == m_memberMap.end())
        return ch;

    int size = m_memberMap.size();
    while (size-- > 0)
    {
        CHARACTER *pkMember = m_itNextOwner->second.pCharacter;

        if (pkMember && DISTANCE_APPROX(pkMember->GetX() - x, pkMember->GetY() - y) < 3000)
        {
            IncreaseOwnership();
            return pkMember;
        }

        IncreaseOwnership();
    }

    return ch;
}

void CParty::ComputeRolePoint(CHARACTER *ch, uint8_t bRole, bool bAdd)
{
	if (!bAdd)
	{
		bool state = false;
		const DWORD partyBonus[] =
		{
			POINT_PARTY_ATTACKER_BONUS,
			POINT_PARTY_TANKER_BONUS,
			POINT_PARTY_BUFFER_BONUS,
			POINT_PARTY_SKILL_MASTER_BONUS,
			POINT_PARTY_DEFENDER_BONUS,
			POINT_PARTY_HASTE_BONUS
		};

		for (size_t i = 0; i < sizeof(partyBonus) / sizeof(DWORD); ++i)
		{
			if (ch->GetPoint(partyBonus[i]))
			{
				ch->PointChange(partyBonus[i], -ch->GetPoint(partyBonus[i]));
				state = true;
			}
		}

		if (state)
		{
			ch->ComputeBattlePoints();
            ch->ComputePoints();
            ch->ComputeMountPoints();
		}
		return;
	}

    //SKILL_POWER_BY_LEVEL
    float k = static_cast<float>(GetSkillPowerByLevel(std::min(static_cast<int>(SKILL_MAX_LEVEL), m_iLeadership))) /
              100.0f;
    //float k = (float) aiSkillPowerByLevel[MIN(SKILL_MAX_LEVEL, m_iLeadership)] / 100.0f;
    //
    //sys_log(0,"ComputeRolePoint %fi %d, %d ", k, SKILL_MAX_LEVEL, m_iLeadership ); 
    //END_SKILL_POWER_BY_LEVEL

    switch (bRole)
    {
    case PARTY_ROLE_ATTACKER: {
        //int iBonus = (int) (10 + 90 * k);
        int iBonus = (int)(10 + 60 * k);

        if (ch->GetPoint(POINT_PARTY_ATTACKER_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_ATTACKER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
            ch->ComputePoints();
            ch->ComputeMountPoints();
        }
    }
    break;

    case PARTY_ROLE_TANKER: {
        int iBonus = (int)(50 + 1450 * k);

        if (ch->GetPoint(POINT_PARTY_TANKER_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_TANKER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_TANKER_BONUS));
            ch->ComputePoints();
            ch->ComputeMountPoints();
        }
    }
    break;

    case PARTY_ROLE_BUFFER: {
        int iBonus = (int)(5 + 45 * k);

        if (ch->GetPoint(POINT_PARTY_BUFFER_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_BUFFER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_BUFFER_BONUS));
        }
    }
    break;

    case PARTY_ROLE_SKILL_MASTER: {
        int iBonus = (int)(25 + 600 * k);

        if (ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
            ch->ComputePoints();
            ch->ComputeMountPoints();
        }
    }
    break;
    case PARTY_ROLE_HASTE: {
        int iBonus = (int)(1 + 5 * k);
        if (ch->GetPoint(POINT_PARTY_HASTE_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_HASTE_BONUS, iBonus - ch->GetPoint(POINT_PARTY_HASTE_BONUS));
            ch->ComputePoints();
            ch->ComputeMountPoints();
        }
    }
    break;
    case PARTY_ROLE_DEFENDER: {
        int iBonus = (int)(5 + 30 * k);
        if (ch->GetPoint(POINT_PARTY_DEFENDER_BONUS) != iBonus)
        {
            ch->PointChange(POINT_PARTY_DEFENDER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
            ch->ComputePoints();
            ch->ComputeMountPoints();
        }
    }
    break;
    }
}

void CParty::Update()
{
    SPDLOG_DEBUG("PARTY::Update");

    CHARACTER *l = GetLeaderCharacter();

    if (!l)
        return;

    TMemberMap::iterator it;

    int iNearMember = 0;
    bool bResendAll = false;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch = it->second.pCharacter;

        it->second.bNear = false;

        if (!ch)
            continue;

        if (l->GetDungeon())
            it->second.bNear = l->GetDungeon() == ch->GetDungeon();
        else
            it->second.bNear = (DISTANCE_APPROX(l->GetX() - ch->GetX(), l->GetY() - ch->GetY()) < PARTY_DEFAULT_RANGE);

        if (it->second.bNear)
        {
            ++iNearMember;
            //SPDLOG_INFO("NEAR %s", ch->GetName());
        }
    }

    if (iNearMember <= 1 && !l->GetDungeon())
    {
        for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
            it->second.bNear = false;

        iNearMember = 0;
    }

    if (iNearMember != m_iCountNearPartyMember)
    {
        m_iCountNearPartyMember = iNearMember;
        bResendAll = true;
    }

    m_iLeadership = l->GetLeadershipSkillLevel();
    int iNewExpBonus = ComputePartyBonusExpPercent();
    m_iAttBonus = ComputePartyBonusAttackGrade();
    m_iDefBonus = ComputePartyBonusDefenseGrade();

    if (m_iExpBonus != iNewExpBonus)
    {
        bResendAll = true;
        m_iExpBonus = iNewExpBonus;
    }

    bool bLongTimeExpBonusChanged = false;

    // 파티 결성 후 충분한 시간이 지나면 경험치 보너스를 받는다.
    if (!m_iLongTimeExpBonus && (get_dword_time() - m_dwPartyStartTime > PARTY_ENOUGH_MINUTE_FOR_EXP_BONUS * 60 * 1000))
    {
        bLongTimeExpBonusChanged = true;
        m_iLongTimeExpBonus = 5;
        bResendAll = true;
    }

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch = it->second.pCharacter;

        if (!ch)
            continue;

        if (bLongTimeExpBonusChanged && ch->GetDesc())
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "파티의 협동력이 높아져 지금부터 추가 경험치 보너스를 받습니다.");

        bool bNear = it->second.bNear;

        ComputeRolePoint(ch, it->second.bRole, bNear);

        if (bNear)
        {
            if (!bResendAll)
                SendPartyInfoOneToAll(ch);
        }
    }

    // PARTY_ROLE_LIMIT_LEVEL_BUG_FIX
    m_anMaxRole[PARTY_ROLE_ATTACKER] = m_iLeadership >= 10 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_HASTE] = m_iLeadership >= 20 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_TANKER] = m_iLeadership >= 20 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_BUFFER] = m_iLeadership >= 25 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_SKILL_MASTER] = m_iLeadership >= 35 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_DEFENDER] = m_iLeadership >= 40 ? 1 : 0;
    m_anMaxRole[PARTY_ROLE_ATTACKER] += m_iLeadership >= 40 ? 1 : 0;
    // END_OF_PARTY_ROLE_LIMIT_LEVEL_BUG_FIX

    if (bResendAll)
    {
        for (auto &it : m_memberMap)
            if (it.second.pCharacter)
                SendPartyInfoOneToAll(it.second.pCharacter);
    }
}

void CParty::UpdateOnlineState(uint32_t dwPID, const char *name, uint32_t mapIndex, uint32_t channel)
{
    TMember &r = m_memberMap[dwPID];

    TPacketGCPartyAdd p;
    p.pid = dwPID;
    r.strName = name;
    p.name = name;
    p.mapIndex = mapIndex;
    p.channel = channel;

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_ADD, p);
    }
}

void CParty::UpdateOfflineState(uint32_t dwPID, uint32_t mapIndex, uint32_t channel)
{
    TPacketGCPartyAdd p = {};
    p.pid = dwPID;
    p.name = "";
    p.mapIndex = mapIndex;
    p.channel = channel;

    for (auto it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (it->second.pCharacter && it->second.pCharacter->GetDesc())
            it->second.pCharacter->GetDesc()->Send(HEADER_GC_PARTY_ADD, p);
    }
}

int CParty::GetFlag(const std::string &name)
{
    TFlagMap::iterator it = m_map_iFlag.find(name);

    if (it != m_map_iFlag.end())
    {
        //SPDLOG_INFO("PARTY GetFlag %s %d", name.c_str(), it->second);
        return it->second;
    }

    //SPDLOG_INFO("PARTY GetFlag %s 0", name.c_str());
    return 0;
}

void CParty::SetFlag(const std::string &name, int value)
{
    TFlagMap::iterator it = m_map_iFlag.find(name);

    //SPDLOG_INFO("PARTY SetFlag %s %d", name.c_str(), value);
    if (it == m_map_iFlag.end()) { m_map_iFlag.insert(make_pair(name, value)); }
    else if (it->second != value) { it->second = value; }
}

bool CParty::IsPositionNearLeader(CHARACTER *ch)
{
    if (!m_pkChrLeader)
        return false;

    if (DISTANCE_APPROX(ch->GetX() - m_pkChrLeader->GetX(), ch->GetY() - m_pkChrLeader->GetY()) >= PARTY_DEFAULT_RANGE)
        return false;

    return true;
}

int CParty::GetExpBonusPercent()
{
    if (GetNearMemberCount() <= 1)
        return 0;

    return m_iExpBonus + m_iLongTimeExpBonus;
}

bool CParty::IsNearLeader(uint32_t pid)
{
    TMemberMap::iterator it = m_memberMap.find(pid);

    if (it == m_memberMap.end())
        return false;

    return it->second.bNear;
}

uint8_t CParty::CountMemberByVnum(uint32_t dwVnum)
{
    if (m_bPCParty)
        return 0;

    CHARACTER *tch;
    uint8_t bCount = 0;

    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        if (!(tch = it->second.pCharacter))
            continue;

        if (tch->IsPC())
            continue;

        if (tch->GetMobTable().dwVnum == dwVnum)
            ++bCount;
    }

    return bCount;
}

void CParty::SendParameter(CHARACTER *ch)
{
    TPacketGCPartyParameter p = {};
    p.bDistributeMode = m_iExpDistributionMode;

    DESC *d = ch->GetDesc();

    if (d) { d->Send(HEADER_GC_PARTY_PARAMETER, p); }
}

void CParty::SendParameterToAll()
{
    if (!m_bPCParty)
        return;

    TMemberMap::iterator it;

    for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
        if (it->second.pCharacter)
            SendParameter(it->second.pCharacter);
}

void CParty::SetParameter(int iMode)
{
    if (iMode >= PARTY_EXP_DISTRIBUTION_MAX_NUM)
    {
        SPDLOG_ERROR("Invalid exp distribution mode %d", iMode);
        return;
    }

    m_iExpDistributionMode = iMode;
    SendParameterToAll();
}

int CParty::GetExpDistributionMode() { return m_iExpDistributionMode; }

void CParty::SetExpCentralizeCharacter(uint32_t dwPID)
{
    TMemberMap::iterator it = m_memberMap.find(dwPID);

    if (it == m_memberMap.end())
        return;

    m_pkChrExpCentralize = it->second.pCharacter;
}

CHARACTER *CParty::GetExpCentralizeCharacter() { return m_pkChrExpCentralize; }

uint8_t CParty::GetMemberMaxLevel()
{
    uint8_t bMax = 0;

    auto it = m_memberMap.begin();
    while (it != m_memberMap.end())
    {
        if (!it->second.bLevel)
        {
            ++it;
            continue;
        }

        if (!bMax)
            bMax = it->second.bLevel;
        else if (it->second.bLevel)
            bMax = std::max<int>(bMax, it->second.bLevel);
        ++it;
    }
    return bMax;
}

uint8_t CParty::GetMemberMinLevel()
{
    uint8_t bMin = gConfig.maxLevel;

    auto it = m_memberMap.begin();
    while (it != m_memberMap.end())
    {
        if (!it->second.bLevel)
        {
            ++it;
            continue;
        }

        if (!bMin)
            bMin = it->second.bLevel;
        else if (it->second.bLevel)
            bMin = std::min<int>(bMin, it->second.bLevel);
        ++it;
    }
    return bMin;
}

int CParty::ComputePartyBonusExpPercent()
{
    if (GetNearMemberCount() <= 1)
        return 0;

    CHARACTER *leader = GetLeaderCharacter();

    int iBonusPartyExpFromItem = 0;

    // UPGRADE_PARTY_BONUS
    int iMemberCount = std::min<int>(16, GetNearMemberCount());

    if (leader && (leader->IsEquipUniqueItem(UNIQUE_ITEM_PARTY_BONUS_EXP) || leader->IsEquipUniqueItem(
                       UNIQUE_ITEM_PARTY_BONUS_EXP_MALL)
                   || leader->IsEquipUniqueItem(UNIQUE_ITEM_PARTY_BONUS_EXP_GIFT) || leader->IsEquipUniqueGroup(10010)))
    {
        // 중국측 육도 적용을 확인해야한다.
        if (iMemberCount <= 8 && iMemberCount > 1)
            iBonusPartyExpFromItem = 30;
        else if (iMemberCount > 8)
            iBonusPartyExpFromItem = 60;
    }

    int iBonusPartyComputedBonusPercent = iBonusPartyExpFromItem + KOR_aiPartyBonusExpPercentByMemberCount[iMemberCount
                                          ];
    if (iBonusPartyComputedBonusPercent > 150)
        iBonusPartyComputedBonusPercent = 150;

    return iBonusPartyComputedBonusPercent;
    // END_OF_UPGRADE_PARTY_BONUS
}

bool CParty::IsPartyInDungeon(int mapIndex)
{
    // 파티원이 mapIndex인 던젼안에 있는지 순서대로 검사
    for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
    {
        CHARACTER *ch = it->second.pCharacter;

        if (nullptr == ch) { continue; }

        CDungeon *d = ch->GetDungeon();

        if (nullptr == d)
        {
            SPDLOG_INFO("not in dungeon");
            continue;
        }

        if (mapIndex == (d->GetMapIndex()) / 10000) { return true; }
    }
    return false;
}

CHARACTER *CParty::Dice(CHARACTER *oldCh, CItem *item) //item name
{
    ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "|cffffe6ba*** Rolling for the following item: %s ***",
                          TextTag::itemname(item->GetVnum()).c_str()); //CHAT_TYPE_DICE_INFO

    while (true)
    {
        CHARACTER *ch = oldCh;
        int iDice = 0, iDraw = 0;

        for (const auto &member : m_memberMap)
        {
            const auto memberCh = member.second.pCharacter;
            if (memberCh)
            {
                const auto desc = memberCh->GetDesc();
                if (desc)
                {
                    if (DISTANCE_APPROX(memberCh->GetX() - oldCh->GetX(), memberCh->GetY() - oldCh->GetY()) < 3000)
                    {
                        int iScore = Random::get(1, 10000);

                        if (iScore == iDice)
                            iDraw = iScore;
                        else if (iDice == 0 || iScore > iDice)
                        {
                            ch = memberCh;
                            iDice = iScore;
                        }

                        ChatPacketToAllMember(CHAT_TYPE_DICE_INFO,
                                              "|cffffe6ba*** ->\t\t\t\t\t%s - Dice score: %d ***",
                                              memberCh->GetName(),
                                              iScore); //CHAT_TYPE_DICE_INFO
                    }
                    else { memberCh->ChatPacket(CHAT_TYPE_DICE_INFO, "You're to far to take a part in item dice"); }
                }
            }
        }

        if (iDice != iDraw)
        {
            ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "|cffbb00ff*** Winner of the item %s is: %s ***",
                                  TextTag::itemname(item->GetVnum()).c_str(), ch->GetName()); //CHAT_TYPE_DICE_INFO
            ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "");
            return ch;
        }

        ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "");
        ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "|cffffe6ba*** Rolling again for the following item: %s ***",
                              TextTag::itemname(item->GetVnum()).c_str()); //CHAT_TYPE_DICE_INFO
    }
}
