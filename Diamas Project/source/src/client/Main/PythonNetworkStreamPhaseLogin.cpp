#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include <game/GamePacket.hpp>
#include "Test.h"
#include "../eterBase/Timer.h"
#include "AuthSocket.hpp"

// Login ---------------------------------------------------------------------------
bool CPythonNetworkStream::LoginPhase(const PacketHeader &header, const boost::asio::const_buffer &data)
{
#define HANDLE_PACKET(id, fn, type)                                                                                    \
    case id:                                                                                                           \
        return this->fn(ReadPacket<type>(data))

    switch (header.id)
    {
        HANDLE_PACKET(HEADER_GC_LOGIN_SUCCESS, __RecvLoginSuccessPacket, TPacketGCLoginSuccess);
        HANDLE_PACKET(HEADER_GC_LOGIN_FAILURE, __RecvLoginFailurePacket, TPacketGCLoginFailure);

    default:
        break;
    }

#undef HANDLE_PACKET

    return true;
}

void CPythonNetworkStream::SetLoginPhase()
{
    if ("Login" != m_strPhase)
        m_phaseLeaveFunc.Run();

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - Login Phase ##");
    SPDLOG_DEBUG("");

    m_strPhase = "Login";

    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoginPhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoginPhase);

    m_dwChangingPhaseTime = ELTimer_GetMSec();

    m_firstHandShakeCompleted = false;

    SendLoginPacket(m_login.c_str(), m_sessionId);

    if (!m_autoSelectCharacter)
    {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginStart");
        __ClearSelectCharacterData();
    }
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket(const TPacketGCLoginSuccess& p)
{
    for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
    {
        m_players[i] = p.players[i];
        m_adwGuildID[i] = p.guild_id[i];
        m_astrGuildName[i] = p.guild_name[i];
        m_playerNames[i] = p.players[i].name;
    }

    return true;
}

void CPythonNetworkStream::OnConnectFailure()
{
    if (m_autoSelectCharacter)
        ClosePhase();
    else
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnConnectFailure");
}

bool CPythonNetworkStream::__RecvLoginFailurePacket(const TPacketGCLoginFailure& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure", py::bytes(p.szStatus));

    return true;
}

bool CPythonNetworkStream::SendLoginPacket(const char *login, uint64_t sessionId)
{
    CgKeyLoginPacket p;
    p.login = login;
    p.sessionId = sessionId;
    Send(HEADER_CG_KEY_LOGIN, p);
    return true;
}
