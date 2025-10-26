#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include <game/GamePacket.hpp>
#include <pak/Vfs.hpp>

#ifdef USE_AHNLAB_HACKSHIELD
#include METIN2HS_INCLUDE_HSHIELD
#endif

// HandShake ---------------------------------------------------------------------------
bool CPythonNetworkStream::HandShakePhase(const PacketHeader &header, const boost::asio::const_buffer &data)
{
    switch (header.id)
    {
    case HEADER_GC_HANDSHAKE: {
        m_HandshakeData = ReadPacket<TPacketGCHandshake>(data);

        SPDLOG_DEBUG("HANDSHAKE RECV {0} {1}", m_HandshakeData.dwTime, m_HandshakeData.lDelta);

        ELTimer_SetServerMSec(m_HandshakeData.dwTime + m_HandshakeData.lDelta);

        m_HandshakeData.dwTime = m_HandshakeData.dwTime + m_HandshakeData.lDelta + m_HandshakeData.lDelta;
        m_HandshakeData.lDelta = 0;

        SPDLOG_DEBUG("HANDSHAKE SEND {0}", m_HandshakeData.dwTime);

        Send(HEADER_CG_HANDSHAKE, m_HandshakeData);
        return true;
    }
    }
    return true;
}

void CPythonNetworkStream::SetHandShakePhase()
{
    if ("HandShake" != m_strPhase)
        m_phaseLeaveFunc.Run();

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - Hand Shake Phase ##");
    SPDLOG_DEBUG("");

    m_strPhase = "HandShake";

    m_dwChangingPhaseTime = ELTimer_GetMSec();
    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::HandShakePhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveHandshakePhase);

    SetGameOnline();
}

bool CPythonNetworkStream::RecvHandshakePacket(const TPacketGCHandshake& p)
{
    SPDLOG_DEBUG("HANDSHAKE RECV {0} {1}", p.dwTime, p.lDelta);

    m_kServerTimeSync.m_dwChangeServerTime = p.dwTime + p.lDelta;
    m_kServerTimeSync.m_dwChangeClientTime = ELTimer_GetMSec();
    auto outp = p;
    outp.dwTime = p.dwTime + p.lDelta + p.lDelta;
    outp.lDelta = 0;

    SPDLOG_DEBUG("HANDSHAKE SEND {0}", p.dwTime);

    Send(HEADER_CG_TIME_SYNC, outp);


    return true;
}

bool CPythonNetworkStream::RecvHandshakeOKPacket(const BlankPacket& p)
{

    uint32_t dwDelta = ELTimer_GetMSec() - m_kServerTimeSync.m_dwChangeClientTime;
    ELTimer_SetServerMSec(m_kServerTimeSync.m_dwChangeServerTime + dwDelta);

    SPDLOG_DEBUG("HANDSHAKE OK RECV {0} {1}", m_kServerTimeSync.m_dwChangeServerTime, dwDelta);

    return true;
}

bool CPythonNetworkStream::SendTimeSyncReqPacket()
{
    BlankPacket p;
    Send(HEADER_CG_REQ_TIMESYNC, p);
    return true;
}
