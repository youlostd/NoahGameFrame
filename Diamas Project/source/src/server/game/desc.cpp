#include "desc.h"
#include "CharUtil.hpp"
#include "DbCacheSocket.hpp"
#include "GBufferManager.h"
#include "MasterUtil.hpp"
#include "char.h"
#include "config.h"
#include "desc_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "locale.h"
#include "log.h"
#include "messenger_manager.h"
#include "sectree_manager.h"
#include "utils.h"

#include <game/GamePacket.hpp>
#include <net/Util.hpp>

EVENTFUNC(ping_event)
{
    auto info = static_cast<DESC::desc_event_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("ping_event> <Factor> Null pointer");
        return 0;
    }

    DESC* desc = info->desc;
    if (desc->IsAdminMode())
        return THECORE_SECS_TO_PASSES(gConfig.pingTimeout);

    if (!desc->IsPong()) {
        SPDLOG_INFO("PING_EVENT: {0} has not responded in time disconnecting",
                    desc->GetHostName());
        desc->SetDisconnectReason("Ping timeout");
        desc->Disconnect();
        return THECORE_SECS_TO_PASSES(gConfig.pingTimeout);
    } else {
        TPacketGCPing p{};
        desc->Send(HEADER_GC_PING, p);
        desc->SetPong(false);
    }

    desc->SendHandshake(get_dword_time(), 0);
    return THECORE_SECS_TO_PASSES(gConfig.pingTimeout);
}

DESC::DESC(asio::ip::tcp::socket socket, uint32_t handle, uint32_t handshake)
    : Socket(std::move(socket))
    , m_iPhase(PHASE_CLOSE)
    , m_dwHandle(handle)
    , m_pInputProcessor(nullptr)
    , m_dwHandshake(handshake)
    , m_dwHandshakeSentTime(0)
    , m_iHandshakeRetry(0)
    , m_dwClientTime(0)
    , m_bHandshaking(false)
    , m_pkPingEvent(nullptr)
    , m_pkDisconnectEvent(nullptr)
    , m_bAdminMode(false)
    , m_bPong(true)
    , m_aid(0)
    , m_character(nullptr)
{

}

void DESC::Setup()
{
    boost::system::error_code ec;
    const auto s = m_socket.remote_endpoint(ec);
    if (ec) {
        if (ec) {
            SPDLOG_ERROR("Could not read address: {} handle {} socket {}",
                         ec.message(), m_dwHandle, m_socket);
        }
        m_socketAddress = "0.0.0.0";
    } else {
        m_socketAddress = s.address().to_string(ec);
        if (ec) {
            SPDLOG_ERROR("Could not read address: {} handle {} socket {}",
                         ec.message(), m_dwHandle, m_socket);
        }
    }

    boost::asio::ip::tcp::no_delay option(true);
    boost::system::error_code noDelayEc;
    m_socket.set_option(option, noDelayEc);
    if (noDelayEc) {
        SPDLOG_ERROR("Could no set tcp::no_delay {} socket {} handle {}",
                     noDelayEc.message(), m_socket, m_dwHandle);
        return;
    }



    // Ping Event
    assert(m_pkPingEvent == NULL);
    auto* info = AllocEventInfo<desc_event_info>();
    info->desc = this;
    m_pkPingEvent = event_create(ping_event, info,
                                 THECORE_SECS_TO_PASSES(gConfig.pingTimeout));

    StartReadSome();

    SendHandshake(get_dword_time(), 0);

    SetPhase(PHASE_LOGIN);
}

void DESC::SetDisconnectPhase(EPhase phase)
{
    m_disconnectPhase = phase;
}

/*virtual*/
void DESC::Disconnect()
{
    SPDLOG_INFO("Connection {0} closed.", m_socket);
    SPDLOG_INFO("DESC::Disconnect {} for reason {}",
                m_character ? m_character->GetName() : m_socketAddress,
                m_disconnectReason.value_or("<notset>"));

    if (m_character) {
        m_character->Disconnect("DESC::Disconnect");
        m_character = nullptr;
    }

    Logout();

    event_cancel(&m_pkPingEvent);
    event_cancel(&m_pkDisconnectEvent);

    SetPhase(m_disconnectPhase);
    SocketBase::Disconnect();

    DESC_MANAGER::instance().DestroyDesc(this);
}

void DESC::SetDisconnectReason(const std::string& reason)
{
    m_disconnectReason = reason;
}

void DESC::SetPhase(int phase)
{
    m_iPhase = phase;

    TPacketGCPhase pack;
    pack.phase = phase;
    Send(HEADER_GC_PHASE, pack);

    // |m_inputPhase| calls SetInputProcessor() with our actual phase
    // once the ACK has been received.
    m_pInputProcessor = &m_inputPhase;
}

void DESC::SetInputProcessor(int phase)
{
    switch (phase) {
        case PHASE_CLOSE:
            m_pInputProcessor = &m_inputClose;
            break;

        case PHASE_HANDSHAKE:
            m_pInputProcessor = &m_inputHandshake;
            break;

        case PHASE_SELECT:
        case PHASE_LOGIN:
            m_pInputProcessor = &m_inputLogin;
            break;

        case PHASE_LOADING:
            m_pInputProcessor = &m_inputLogin;
            break;

        case PHASE_DEAD:
        case PHASE_GAME:
            m_pInputProcessor = &m_inputMain;
            break;
    }
}

bool DESC::IsPong()
{
    return m_bPong;
}

void DESC::SetPong(bool b)
{
    m_bPong = b;
}

std::string DESC::GetHostName() const
{
    return m_socketAddress;
}

void DESC::SendHandshake(uint32_t dwCurTime, int32_t lNewDelta)
{
    TPacketGCHandshake pack;
    pack.dwHandshake = m_dwHandshake;
    pack.dwTime = dwCurTime;
    pack.lDelta = lNewDelta;

    Send(HEADER_GC_HANDSHAKE, pack);

    m_dwHandshakeSentTime = dwCurTime;
    m_bHandshaking = true;
}

bool DESC::HandshakeProcess(uint32_t dwTime, int32_t lDelta,
                            bool bInfiniteRetry)
{
    uint32_t dwCurTime = get_dword_time();

    if (lDelta < 0) {
        SPDLOG_ERROR("Invalid lDelta {0} on {1}", lDelta, m_socket);
        return false;
    }

    int bias = (int)(dwCurTime - (dwTime + lDelta));

    if (bias >= 0 && bias <= 50) {
        BlankPacket p;
        Send(HEADER_GC_TIME_SYNC, p);

        if (GetCharacter())
            SPDLOG_INFO("Handshake: client_time {0} server_time {1} name: {2}",
                        m_dwClientTime, dwCurTime, GetCharacter()->GetName());
        else
            SPDLOG_INFO("Handshake: client_time {0} server_time {1}",
                        m_dwClientTime, dwCurTime, lDelta);

        m_dwClientTime = dwCurTime;
        m_bHandshaking = false;
        return true;
    }

    // TODO(tim): Replace /2 with *2 and compare lNewDelta to (dwCurTime -
    // m_dwHandshakeSentTime) / 2
    int32_t lNewDelta = (int32_t)(dwCurTime - dwTime) / 2;
    if (lNewDelta < 0) {
        SPDLOG_INFO("Handshake: lower than zero {0}", lNewDelta);
        lNewDelta = (dwCurTime - m_dwHandshakeSentTime) / 2;
    }

    SPDLOG_TRACE("Handshake: now {0} expected {1} sent-ts {2} new-delta {3} "
                 "new-delta2 {4}",
                 dwCurTime, dwTime, m_dwHandshakeSentTime, lNewDelta,
                 (dwCurTime - m_dwHandshakeSentTime) / 2);

    SendHandshake(dwCurTime, lNewDelta);
    return false;
}

bool DESC::IsHandshaking()
{
    return m_bHandshaking;
}

uint32_t DESC::GetClientTime()
{
    return m_dwClientTime;
}

void DESC::BindCharacter(CHARACTER* ch)
{
    m_character = ch;
}

EVENTFUNC(disconnect_event)
{
    auto info = static_cast<DESC::desc_event_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("disconnect_event> <Factor> Null pointer");
        return 0;
    }

    DESC* d = info->desc;

    d->m_pkDisconnectEvent = nullptr;
    d->SetDisconnectPhase(info->phase);
    d->Disconnect();
    return 0;
}

bool DESC::DelayedDisconnect(int iSec, std::optional<std::string> reason,
                             EPhase disconnectPhase)
{
    m_disconnectReason = reason;
    if (m_pkDisconnectEvent != nullptr)
        return false;

    desc_event_info* info = AllocEventInfo<desc_event_info>();
    info->desc = this;
    info->phase = disconnectPhase;
    m_pkDisconnectEvent =
        event_create(disconnect_event, info, THECORE_SECS_TO_PASSES(iSec));
    return true;
}

void DESC::DisconnectOfSameLogin()
{
    if (GetCharacter()) {
        if (m_pkDisconnectEvent)
            return;

        SendI18nChatPacket(GetCharacter(), CHAT_TYPE_INFO,
                           "다른 컴퓨터에서 로그인 하여 접속을 종료 합니다.");
        DelayedDisconnect(5, "SAME_LOGIN", PHASE_SAME_LOGIN);
    } else {
        SetDisconnectReason("SAME_LOGIN");
        SetDisconnectPhase(PHASE_SAME_LOGIN);
        Disconnect();
    }
}

void DESC::SetAdminMode()
{
    m_bAdminMode = true;
}

bool DESC::IsAdminMode()
{
    return m_bAdminMode;
}

void DESC::SendLoginSuccessPacket()
{
    const auto& tab = m_accountTable;

    TPacketGCLoginSuccess p;
    std::memcpy(p.players, tab.players, sizeof(tab.players));

    for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i) {
        CGuild* g = CGuildManager::instance().GetLinkedGuild(tab.players[i].id);
        if (g) {
            p.guild_id[i] = g->GetID();
            strlcpy(p.guild_name[i], g->GetName(), sizeof(p.guild_name[i]));
        } else {
            p.guild_id[i] = 0;
            p.guild_name[i][0] = '\0';
        }
    }

    Send(HEADER_GC_LOGIN_SUCCESS, p);
}

void DESC::Logout()
{
    if (0 == m_aid)
        return;

    DESC_MANAGER::instance().DisconnectAccount(m_aid);
    DESC_MANAGER::instance().DisconnectAccountDesc(this);

    SendLogoutPacket(m_aid);
    m_aid = 0;
}

bool DESC::HandlePacket(const PacketHeader& header,
                        const boost::asio::const_buffer& data)
{

    try {
        return m_pInputProcessor->Analyze(this, header, data);
    } catch (std::exception&) {
        return true;
    }
}

void DESC::SetDropsAllowed(bool v)
{
    m_dropsAllowed = v;
    BlankPacket p;
    p.dummydata = v;
    Send(HEADER_GC_MULTI_STATUS, p);
}
