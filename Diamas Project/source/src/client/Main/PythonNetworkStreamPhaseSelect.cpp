#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include <game/GamePacket.hpp>
#include "../eterBase/Timer.h"


// Select Character ---------------------------------------------------------------------------
void CPythonNetworkStream::SetSelectPhase()
{
    if ("Select" != m_strPhase)
        m_phaseLeaveFunc.Run();

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - Select Phase ##");
    SPDLOG_DEBUG("");

    m_strPhase = "Select";

    m_dwChangingPhaseTime = ELTimer_GetMSec();
    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::SelectPhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveSelectPhase);

    if (m_autoSelectCharacter)
    {
        if (m_firstHandShakeCompleted)
        {
            SendSelectCharacterPacket(m_selectedCharacterSlot);
            //PyCallClassMemberFunc(m_poHandler, "SetLoadingPhase");
            m_autoSelectCharacter = false;
        }
        else
        {
            SendTimeSyncReqPacket();
        }
    }
    else
    {
        /*
        if (IsSelectedEmpire())
            PyCallClassMemberFunc(m_poHandler, "SetSelectCharacterPhase");
        else
            PyCallClassMemberFunc(m_poHandler, "SetSelectEmpirePhase");*/

         PyCallClassMemberFunc(m_poHandler, "SetSelectCharacterPhase"); //nevo hotfix for empire
    }
}

bool CPythonNetworkStream::SelectPhase(const PacketHeader &header, const boost::asio::const_buffer &data)
{
  #define HANDLE_PACKET(id, fn, type) \
	case id: return this->fn(ReadPacket<type>(data))

    switch (header.id)
    {
        HANDLE_PACKET(HEADER_GC_LOGIN_SUCCESS, __RecvLoginSuccessPacket, TPacketGCLoginSuccess);
        HANDLE_PACKET(HEADER_GC_CHARACTER_CREATE_FAILURE, __RecvPlayerCreateFailurePacket, GcCharacterCreateFailurePacket);
        HANDLE_PACKET(HEADER_GC_CHARACTER_CREATE_SUCCESS, __RecvPlayerCreateSuccessPacket, TPacketGCPlayerCreateSuccess);
        HANDLE_PACKET(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID, __RecvPlayerDestroyFailurePacket, BlankPacket);
        HANDLE_PACKET(HEADER_GC_CHARACTER_DELETE_SUCCESS, __RecvPlayerDestroySuccessPacket, GcCharacterDeleteSuccessPacket);
        HANDLE_PACKET(HEADER_GC_CHANGE_NAME, __RecvChangeName, GcChangeNamePacket);
        HANDLE_PACKET(HEADER_GC_CHANGE_EMPIRE, __RecvChangeEmpire, GcChangeEmpirePacket);
        HANDLE_PACKET(HEADER_GC_HANDSHAKE, RecvHandshakePacket, TPacketGCHandshake);


    case HEADER_GC_TIME_SYNC:
        BlankPacket p;
        if (!RecvHandshakeOKPacket(p))
            break;

        // If we didn't complete at least one handshake at selection time,
        // the call to SendSelectCharacterPacket() will have been delayed
        // until now.
        if (m_autoSelectCharacter && !m_firstHandShakeCompleted)
        {
            SendSelectCharacterPacket(m_selectedCharacterSlot);
            //PyCallClassMemberFunc(m_poHandler, "SetLoadingPhase");
            m_autoSelectCharacter = false;
        }

        // This is always true after receiving this packet.
        m_firstHandShakeCompleted = true;
        return true;

    }

    return true;
}

bool CPythonNetworkStream::SendSelectCharacterPacket(uint8_t Index)
{
    TPacketCGPlayerSelect p;
    p.index = Index;
    p.localeName = LocaleService_GetLocaleName();
    Send(HEADER_CG_CHARACTER_SELECT, p);

    isInputEnabled = 0;

    return true;
}

bool CPythonNetworkStream::SendDestroyCharacterPacket(uint8_t index, const char *szPrivateCode)
{
    TPacketCGPlayerDelete DestroyCharacterPacket;
    DestroyCharacterPacket.index = index;
    DestroyCharacterPacket.private_code = szPrivateCode;
    Send(HEADER_CG_CHARACTER_DELETE, DestroyCharacterPacket);
    return true;
}

bool CPythonNetworkStream::SendCreateCharacterPacket(uint8_t index,
                                                     const char *name,
                                                     uint8_t empire,
                                                     uint8_t job,
                                                     uint8_t shape,
                                                     uint8_t ht, uint8_t iq,
                                                     uint8_t st, uint8_t dx)
{
    TPacketCGPlayerCreate p;
    p.index = index;
    p.name = name;
    p.empire = empire;
    p.job = job;
    p.shape = shape;
    p.Con = ht;
    p.Int = iq;
    p.Str = st;
    p.Dex = dx;

    Send(HEADER_CG_CHARACTER_CREATE, p);

    return true;
}

bool CPythonNetworkStream::SendChangeNamePacket(uint8_t slot, const char *name)
{
    CgChangeNamePacket p;
    p.index = slot;
    p.name = name;
    Send(HEADER_CG_CHANGE_NAME, p) ;
    return true;
}

bool CPythonNetworkStream::SendChangeEmpirePacket(uint8_t slot, uint8_t empire)
{
    CgChangeEmpirePacket p;
    p.slot = slot;
    p.empire = empire;
     Send(HEADER_CG_CHANGE_EMPIRE, p);
    return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateSuccessPacket(const TPacketGCPlayerCreateSuccess& p)
{
    if (p.slot >= PLAYER_PER_ACCOUNT)
    {
        SPDLOG_ERROR(
            "CPythonNetworkStream::RecvPlayerCreateSuccessPacket - OUT OF RANGE SLOT({0}) > PLATER_PER_ACCOUNT({1})",
            p.slot, PLAYER_PER_ACCOUNT);
        return true;
    }

    m_players[p.slot] = p.player;
    m_playerNames[p.slot] = p.player.name;
    m_adwGuildID[p.slot] = 0;
    m_astrGuildName[p.slot] = "";

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateSuccess");
    return true;
}

bool CPythonNetworkStream::__RecvPlayerCreateFailurePacket(const GcCharacterCreateFailurePacket& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_CREATE], "OnCreateFailure",
                          p.type);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnCreateFailure",
                          p.type);
    return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroySuccessPacket(const GcCharacterDeleteSuccessPacket& p)
{

    memset(&m_players[p.slot], 0, sizeof(m_players[p.slot]));
    m_adwGuildID[p.slot] = 0;
    m_astrGuildName[p.slot] = "";
    m_playerNames[p.slot] = "";

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteSuccess", p.slot);
    return true;
}

bool CPythonNetworkStream::__RecvPlayerDestroyFailurePacket(const BlankPacket&p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "OnDeleteFailure");
    return true;
}

bool CPythonNetworkStream::__RecvChangeName(const GcChangeNamePacket&p)
{
    if (p.index >= PLAYER_PER_ACCOUNT)
    {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT],
                              "OnCreateFailure", 100);
        return true;
    }

    m_players[p.index].bChangeName = false;
    CopyStringSafe(m_players[p.index].name, p.name);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT],
                          "OnChangeName",
                          p.index, p.name);
    return true;
}

bool CPythonNetworkStream::__RecvChangeEmpire(const GcChangeEmpirePacket& p)
{
    if (p.slot >= PLAYER_PER_ACCOUNT)
        return true;

    m_players[p.slot].bChangeEmpire = false;
    m_players[p.slot].empire = p.empire;
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT],
                          "OnChangeEmpire",
                          p.slot, p.empire);
    return true;
}
