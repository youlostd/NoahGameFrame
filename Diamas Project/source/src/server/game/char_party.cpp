#include "char.h"
#include "desc.h"
#include "GArena.h"
#include "gm.h"
#include "party.h"

struct FFindReviver
{
    FFindReviver()
    {
        pChar = nullptr;
        HasReviver = false;
    }

    void operator()(CHARACTER *ch)
    {
        if (ch->IsMonster() != true) { return; }

        if (ch->IsReviver() == true && pChar != ch && ch->IsDead() != true)
        {
            if (Random::get(1, 100) <= ch->GetMobTable().bRevivePoint)
            {
                HasReviver = true;
                pChar = ch;
            }
        }
    }

    CHARACTER *pChar;
    bool HasReviver;
};

bool CHARACTER::HasReviverInParty() const
{
    auto* party = GetParty();

    if (party != nullptr)
    {
        if (party->GetMemberCount() == 1)
            return false;

        FFindReviver f;
        party->ForEachMemberPtr(f);
        return f.HasReviver;
    }

    return false;
}

void CHARACTER::SetParty(CParty *pkParty)
{
    if (pkParty == m_pkParty)
        return;

    if (pkParty && m_pkParty)
        SPDLOG_ERROR(
        "{0} is trying to reassigning party (current {1:p}, new party "
        "{2:p})",
        GetName(), fmt::ptr(m_pkParty), fmt::ptr(pkParty));

    m_pkParty = pkParty;

    if (IsPC())
    {
        if (m_pkParty)
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);
        else
            REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);

        UpdatePacket();
    }
}

// PARTY_JOIN_BUG_FIX
/// 파티 가입 event 정보
EVENTINFO(TPartyJoinEventInfo)
{
    uint32_t dwGuestPID;  ///< 파티에 참여할 캐릭터의 PID
    uint32_t dwLeaderPID; ///< 파티 리더의 PID

    TPartyJoinEventInfo()
        : dwGuestPID(0)
          , dwLeaderPID(0)
    {
    }
};

EVENTFUNC(party_request_event)
{
    auto* info = static_cast<TPartyJoinEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("party_request_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = g_pCharManager->FindByPID(info->dwGuestPID);

    if (ch)
    {
        SPDLOG_INFO("{}: PartyRequestEvent", ch->GetName());
        ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
        ch->SetPartyRequestEvent(nullptr);
    }

    return 0;
}

bool CHARACTER::RequestToParty(CHARACTER *leader)
{
    if (leader->GetParty())
        leader = leader->GetParty()->GetLeaderCharacter();

    if (!leader)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "파티장이 접속 상태가 아니라서 요청을 할 수 없습니다."); // Groupleader not online
        return false;
    }

    if (m_pkPartyRequestEvent)
        return false;

    if (!IsPC() || !leader->IsPC())
        return false;

#if defined(WJ_COMBAT_ZONE)
	if (CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex()))
		return false;
#endif

    if (leader->IsBlockMode(BLOCK_PARTY_REQUEST, this))
        return false;

    PartyJoinErrCode errcode = IsPartyJoinableCondition(leader, this);

    switch (errcode)
    {
    case PERR_NONE:
        break;

    case PERR_SERVER:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다.");
        return false;

    case PERR_DIFFEMPIRE:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 다른 제국과 파티를 이룰 수 없습니다.");
        return false;

    case PERR_DUNGEON:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 던전 안에서는 파티 초대를 할 수 없습니다.");
        return false;

    case PERR_OBSERVER:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 관전 모드에선 파티 초대를 할 수 없습니다.");
        return false;

    case PERR_LVBOUNDARY:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다.");
        return false;

    case PERR_LOWLEVEL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다.");
        return false;

    case PERR_HILEVEL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다.");
        return false;

    case PERR_ALREADYJOIN:
        return false;

    case PERR_PARTYISFULL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 더 이상 파티원을 초대할 수 없습니다.");
        return false;

    default:
        SPDLOG_ERROR("Do not process party join error(%d)", errcode);
        return false;
    }

    TPartyJoinEventInfo *info = AllocEventInfo<TPartyJoinEventInfo>();

    info->dwGuestPID = GetPlayerID();
    info->dwLeaderPID = leader->GetPlayerID();

    SetPartyRequestEvent(event_create(party_request_event, info, THECORE_SECS_TO_PASSES(10)));

    leader->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequest %u %s", (uint32_t)GetVID(), GetName());
    SendI18nChatPacket(this, CHAT_TYPE_INFO, "%s 님에게 파티가입 신청을 했습니다.", leader->GetName());
    return true;
}

void CHARACTER::DenyToParty(CHARACTER *member)
{
    if (!member->m_pkPartyRequestEvent)
        return;

    auto *info = static_cast<TPartyJoinEventInfo *>(member->m_pkPartyRequestEvent->info);

    if (!info)
    {
        SPDLOG_ERROR("CHARACTER::DenyToParty> <Factor> Null pointer");
        return;
    }

    if (info->dwGuestPID != member->GetPlayerID())
        return;

    if (info->dwLeaderPID != GetPlayerID())
        return;

    event_cancel(&member->m_pkPartyRequestEvent);

    member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

void CHARACTER::AcceptToParty(CHARACTER *member)
{
    SPDLOG_DEBUG("AcceptToParty {} member {}", GetName(), member->GetName());

    if (!member->m_pkPartyRequestEvent)
        return;

    TPartyJoinEventInfo *info = static_cast<TPartyJoinEventInfo *>(member->m_pkPartyRequestEvent->info);

    if (!info)
    {
        SPDLOG_ERROR("CHARACTER::AcceptToParty> <Factor> Null pointer");
        return;
    }

    if (info->dwGuestPID != member->GetPlayerID())
        return;

    if (info->dwLeaderPID != GetPlayerID())
        return;

    event_cancel(&member->m_pkPartyRequestEvent);

    if (!GetParty())
        SendI18nChatPacket(member, CHAT_TYPE_INFO, "상대방이 파티에 속해있지 않습니다.");
    else
    {
        if (GetPlayerID() != GetParty()->GetLeaderPID())
            return;

        PartyJoinErrCode errcode = IsPartyJoinableCondition(this, member);
        switch (errcode)
        {
        case PERR_NONE:
            member->PartyJoin(this);
            return;
        case PERR_SERVER:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다.");
            break;
        case PERR_DUNGEON:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 던전 안에서는 파티 초대를 할 수 없습니다.");
            break;
        case PERR_OBSERVER:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 관전 모드에선 파티 초대를 할 수 없습니다.");
            break;
        case PERR_LVBOUNDARY:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다.");
            break;
        case PERR_LOWLEVEL:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다.");
            break;
        case PERR_HILEVEL:
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다.");
            break;
        case PERR_ALREADYJOIN:
            break;
        case PERR_PARTYISFULL: {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 더 이상 파티원을 초대할 수 없습니다.");
            SendI18nChatPacket(member, CHAT_TYPE_INFO, "<파티> 파티의 인원제한이 초과하여 파티에 참가할 수 없습니다.");
            break;
        }
        default: SPDLOG_ERROR("Do not process party join error(%d)", errcode);
        }
    }

    member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

/**
 * 파티 초대 event callback 함수.
 * event 가 발동하면 초대 거절로 처리한다.
 */
EVENTFUNC(party_invite_event)
{
    auto *pInfo = static_cast<TPartyJoinEventInfo *>(event->info);

    if (pInfo == nullptr)
    {
        SPDLOG_ERROR("party_invite_event> <Factor> Null pointer");
        return 0;
    }

    auto *pchInviter = g_pCharManager->FindByPID(pInfo->dwLeaderPID);

    if (pchInviter)
    {
        SPDLOG_TRACE("{}: PartyInviteEvent", pchInviter->GetName());
        pchInviter->PartyInviteDeny(pInfo->dwGuestPID);
    }

    return 0;
}

void CHARACTER::PartyInvite(CHARACTER *pchInvitee)
{
#if defined(WJ_COMBAT_ZONE)
	if (CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex()))
		return;
#endif

    if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티원을 초대할 수 있는 권한이 없습니다.");
        return;
    }
    else if (pchInvitee->IsBlockMode(BLOCK_PARTY_INVITE, this))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> %s 님이 파티 거부 상태입니다.", pchInvitee->GetName());
        return;
    }
    else if (CArenaManager::instance().IsArenaMap(pchInvitee->GetMapIndex()))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "대련장에서 사용하실 수 없습니다.");
        return;
    }

    if (IsGM() && !pchInvitee->IsGM() && !GM::check_allow(GetGMLevel(), GM_ALLOW_PLAYER_GROUP_INVITE))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You are not allowed to invite players into a group");
        return;
    }

    if (!IsGM() && pchInvitee->IsGM() && !GM::check_allow(GetGMLevel(), GM_ALLOW_GM_GROUP_INVITE))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You are not allowed to invite game masters into a group");
        return;
    }

    PartyJoinErrCode errcode = IsPartyJoinableCondition(this, pchInvitee);

    switch (errcode)
    {
    case PERR_NONE:
        break;

    case PERR_SERVER:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다.");
        return;

    case PERR_DIFFEMPIRE:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 다른 제국과 파티를 이룰 수 없습니다.");
        return;

    case PERR_DUNGEON:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 던전 안에서는 파티 초대를 할 수 없습니다.");
        return;

    case PERR_OBSERVER:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 관전 모드에선 파티 초대를 할 수 없습니다.");
        return;

    case PERR_LVBOUNDARY:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다.");
        return;

    case PERR_LOWLEVEL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다.");
        return;

    case PERR_HILEVEL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다.");
        return;

    case PERR_ALREADYJOIN:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 이미 %s님은 파티에 속해 있습니다.", pchInvitee->GetName());
        return;

    case PERR_PARTYISFULL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 더 이상 파티원을 초대할 수 없습니다.");
        return;

    default:
        SPDLOG_ERROR("{}: Do not process party join error ({})", GetName(), errcode);
        return;
    }

    if (m_PartyInviteEventMap.end() != m_PartyInviteEventMap.find(pchInvitee->GetPlayerID()))
        return;

    //
    // EventMap 에 이벤트 추가
    //
    TPartyJoinEventInfo *info = AllocEventInfo<TPartyJoinEventInfo>();

    info->dwGuestPID = pchInvitee->GetPlayerID();
    info->dwLeaderPID = GetPlayerID();

    m_PartyInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(),
                                                      event_create(party_invite_event, info,
                                                                   THECORE_SECS_TO_PASSES(10))));

    //
    // 초대 받는 character 에게 초대 패킷 전송
    //

    TPacketGCPartyInvite p;
    p.vid = GetVID();
    pchInvitee->GetDesc()->Send(HEADER_GC_PARTY_INVITE, p);
}

void CHARACTER::PartyInviteAccept(CHARACTER *pchInvitee)
{
    EventMap::iterator itFind = m_PartyInviteEventMap.find(pchInvitee->GetPlayerID());

    if (itFind == m_PartyInviteEventMap.end())
    {
        SPDLOG_WARN("{}: PartyInviteAccept from not invited character ({})", GetName(), pchInvitee->GetName());
        return;
    }

    event_cancel(&itFind->second);
    m_PartyInviteEventMap.erase(itFind);

    if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 파티원을 초대할 수 있는 권한이 없습니다.");
        return;
    }

    PartyJoinErrCode errcode = IsPartyJoinableMutableCondition(this, pchInvitee);

    switch (errcode)
    {
    case PERR_NONE:
        break;

    case PERR_SERVER:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다.");
        return;

    case PERR_DUNGEON:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 던전 안에서는 파티 초대에 응할 수 없습니다.");
        return;

    case PERR_OBSERVER:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 관전 모드에선 파티 초대를 할 수 없습니다.");
        return;

    case PERR_LVBOUNDARY:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다.");
        return;

    case PERR_LOWLEVEL:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다.");
        return;

    case PERR_HILEVEL:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다.");
        return;

    case PERR_ALREADYJOIN:
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 파티 초대에 응할 수 없습니다.");
        return;

    case PERR_PARTYISFULL:
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> 더 이상 파티원을 초대할 수 없습니다.");
        SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO, "<파티> 파티의 인원제한이 초과하여 파티에 참가할 수 없습니다.");
        return;

    default:
        SPDLOG_ERROR("ignore party join error(%d)", errcode);
        return;
    }

    //
    // 파티 가입 처리
    //

    if (GetParty())
        pchInvitee->PartyJoin(this);
    else
    {
        auto pParty = CPartyManager::instance().CreateParty(this);

        pParty->Join(pchInvitee->GetPlayerID());
        pParty->Link(pchInvitee);
        pParty->SendPartyInfoAllToOne(this);
    }
}

void CHARACTER::PartyInviteDeny(uint32_t dwPID)
{
    EventMap::iterator itFind = m_PartyInviteEventMap.find(dwPID);

    if (itFind == m_PartyInviteEventMap.end())
    {
        SPDLOG_DEBUG("PartyInviteDeny to not exist event(inviter PID: {0}, invitee PID: {1})", GetPlayerID(), dwPID);
        return;
    }

    event_cancel(&itFind->second);
    m_PartyInviteEventMap.erase(itFind);

    CHARACTER *pchInvitee = g_pCharManager->FindByPID(dwPID);
    if (pchInvitee)
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> %s님이 파티 초대를 거절하셨습니다.", pchInvitee->GetName());
}

void CHARACTER::PartyJoin(CHARACTER *pLeader)
{
    SendI18nChatPacket(pLeader, CHAT_TYPE_INFO, "<파티> %s님이 파티에 참가하셨습니다.", GetName());
    // "<Party> %s joined your group. "
    SendI18nChatPacket(this, CHAT_TYPE_INFO, "<파티> %s님의 파티에 참가하셨습니다.", pLeader->GetName());
    // "<Party> %s joined your group. "

    pLeader->GetParty()->Join(GetPlayerID());
    pLeader->GetParty()->Link(this);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableCondition(const CHARACTER *pchLeader, const CHARACTER *pchGuest)
{
    if (pchLeader->GetEmpire() != pchGuest->GetEmpire())
        return PERR_DIFFEMPIRE;

    return IsPartyJoinableMutableCondition(pchLeader, pchGuest);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableMutableCondition(const CHARACTER *pchLeader,
                                                                       const CHARACTER *pchGuest)
{
    if (!CPartyManager::instance().IsEnablePCParty())
        return PERR_SERVER;
    else if (pchLeader->GetDungeon())
        return PERR_DUNGEON;
    else if (pchGuest->IsObserverMode())
        return PERR_OBSERVER;
    else if (abs(pchLeader->GetLevel() - pchGuest->GetLevel()) > 30)
        return PERR_LVBOUNDARY;
    else if (pchGuest->GetParty())
        return PERR_ALREADYJOIN;
    else if (pchLeader->GetParty())
    {
        if (pchLeader->GetParty()->GetMemberCount() == PARTY_MAX_MEMBER)
            return PERR_PARTYISFULL;
    }

    return PERR_NONE;
}

// END_OF_PARTY_JOIN_BUG_FIX

bool CHARACTER::BuildUpdatePartyPacket(TPacketGCPartyUpdate &out)
{
    if (!GetParty())
        return false;

    out = {};
    out.pid = GetPlayerID();
    out.percent_hp = std::clamp<uint64_t>(GetHP() * 100 / GetMaxHP(), 0, 100);
    out.role = GetParty()->GetRole(GetPlayerID());
    out.race = GetParty()->GetRace(GetPlayerID());
    out.is_leader = GetParty()->IsLeaderByPid(GetPlayerID());

    SPDLOG_DEBUG("PARTY {0} role is {1}", GetName(), out.role);

    CHARACTER *l = GetParty()->GetLeaderCharacter();

    if (l && DISTANCE_APPROX(GetX() - l->GetX(), GetY() - l->GetY()) < PARTY_DEFAULT_RANGE)
    {
        out.affects[0] = GetParty()->GetPartyBonusExpPercent();
        out.affects[1] = GetPoint(POINT_PARTY_ATTACKER_BONUS);
        out.affects[2] = GetPoint(POINT_PARTY_TANKER_BONUS);
#ifdef SHELIA_BUILD
		out.affects[3] = GetPoint(POINT_PARTY_BLOCKER_BONUS);
		out.affects[5] = GetPoint(POINT_PARTY_DESTROYER_BONUS);
#else
        out.affects[3] = GetPoint(POINT_PARTY_BUFFER_BONUS);
        out.affects[5] = GetPoint(POINT_PARTY_HASTE_BONUS);
#endif

        out.affects[4] = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
        out.affects[6] = GetPoint(POINT_PARTY_DEFENDER_BONUS);
    }

    return true;
}
