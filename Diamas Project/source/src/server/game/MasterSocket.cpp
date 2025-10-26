#include "config.h"
#include "MasterSocket.hpp"
#include "MasterPackets.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>

#include <cstring>

MasterSocket::MasterSocket(asio::io_service &ioService)
    : ClientSocketAdapter(ioService)
{
    // ctor
}

void MasterSocket::Disconnect()
{
    SPDLOG_INFO("MASTER: Connection {0} closed.", m_socket);

    ClientSocketAdapter::Disconnect();
}

/*virtual*/
void MasterSocket::OnConnectSuccess()
{
    SPDLOG_INFO("MASTER: Successfully connected {0}", m_socket);

    GmSetupPacket p;
    p.version = GmSetupPacket::kVersion;
    p.channel = gConfig.channel;
    p.publicAddr = gConfig.gameIp;
    storm::ParseNumber(gConfig.gamePort.c_str(), p.publicPort);
    p.hostedMaps.insert(p.hostedMaps.end(),
                        gConfig.hostedMaps.begin(),
                        gConfig.hostedMaps.end());
    Send(kGmSetup, p);

    StartReadSome();
}

/*virtual*/
void MasterSocket::OnConnectFailure(const boost::system::error_code &ec)
{
    if (ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
        return;

    SPDLOG_ERROR("MASTER: Failed to connect to {0}:{1} with {2}",
                 m_host, m_service, ec);

    Reconnect();
}

/*virtual*/
bool MasterSocket::HandlePacket(const PacketHeader &header,
                                const asio::const_buffer &data)
{
#define HANDLE_PACKET(id, fn, type) \
	case id: return fn(this, ReadPacket<type>(data))

    switch (header.id)
    {
    HANDLE_PACKET(kMgShutdownBroadcast, HandleShutdownBroadcastPacket,
                      MgShutdownBroadcastPacket);
    HANDLE_PACKET(kMgMaintenanceBroadcast, HandleMaintenanceBroadcastPacket,
                      MgMaintenanceBroadcastPacket);

    HANDLE_PACKET(kMgHostedMaps, HandleHostedMapsPacket,
                      MgHostedMapsPacket);

    HANDLE_PACKET(kMgLoginSuccess, HandleLoginSuccessPacket,
                      MgLoginSuccessPacket);
    HANDLE_PACKET(kMgLoginFailure, HandleLoginFailurePacket,
                      MgLoginFailurePacket);
    HANDLE_PACKET(kMgLoginDisconnect, HandleLoginDisconnectPacket,
                      MgLoginDisconnectPacket);

    HANDLE_PACKET(kMgCharacterEnter, HandleCharacterEnterPacket,
                      MgCharacterEnterPacket);
    HANDLE_PACKET(kMgCharacterLeave, HandleCharacterLeavePacket,
                      MgCharacterLeavePacket);
    HANDLE_PACKET(kMgCharacterDisconnect, HandleCharacterDisconnectPacket,
                      MgCharacterDisconnectPacket);

    HANDLE_PACKET(kMgCharacterTransfer, HandleCharacterTransferPacket,
                      MgCharacterTransferPacket);
    HANDLE_PACKET(kMgCharacterWarp, HandleCharacterWarpPacket,
                      MgCharacterWarpPacket);

    HANDLE_PACKET(kMgMonarchCharacterWarp,
                      HandleMonarchCharacterWarpPacket,
                      MgCharacterWarpPacket);

    HANDLE_PACKET(kMgMessengerAdd, HandleMessengerAddPacket,
                      MgMessengerAddPacket);
    HANDLE_PACKET(kMgMessengerRequestAdd, HandleMessengerRequestAddPacket,
                      MgMessengerRequestAddPacket);

    HANDLE_PACKET(kMgMessengerAuthAdd, HandleMessengerAuthAddPacket,
                      MgMessengerAuthAddPacket);
    HANDLE_PACKET(kMgMessengerDel, HandleMessengerDelPacket,
                      MgMessengerDelPacket);

    HANDLE_PACKET(kMgGuildChat, HandleGuildChatPacket,
                      MgGuildChatPacket);
    HANDLE_PACKET(kMgGuildMemberCountBonus,
                      HandleGuildMemberCountBonusPacket,
                      MgGuildMemberCountBonusPacket);
    HANDLE_PACKET(kMgGuildWarZoneInfo, HandleGuildWarZoneInfoPacket,
                      MgGuildWarZoneInfoPacket);

    HANDLE_PACKET(kMgChat, HandleChatPacket,
                      MgChatPacket);
    HANDLE_PACKET(kMgWhisper, HandleWhisperPacket,
                      MgWhisperPacket);
    HANDLE_PACKET(kMgWhisperError, HandleWhisperErrorPacket,
                      MgWhisperErrorPacket);
    HANDLE_PACKET(kMgShout, HandleShoutPacket,
                      MgShoutPacket);
    HANDLE_PACKET(kMgNotice, HandleNoticePacket,
                      MgNoticePacket);
    HANDLE_PACKET(kMgBlockChat, HandleBlockChatPacket,
                      MgBlockChatPacket);
    HANDLE_PACKET(kMgOfflineShopBuyInfo, HandleOfflineShopBuyInfoPacket,
                      MgOfflineShopBuyInfoPacket);
    HANDLE_PACKET(kMgOfflineShopTime, HandleOfflineShopTimePacket,
                      MgOfflineShopTimePacket);
    HANDLE_PACKET(kMgWorldBoss, HandleWorldBossPacket,
                      MgWorldBossPacket);
    HANDLE_PACKET(kMgHyperlinkItemRequest, HandleHyperlinkItemRequest, GmHyperlinkRequestPacket);
    HANDLE_PACKET(kMgHyperlinkResult, HandleHyperlinkItemResult, GmHyperlinkResultPacket);
    HANDLE_PACKET(kMgDropStatus, HandleDropStatusPacket, MgDropStatusPacket);


    default:
        SPDLOG_ERROR("Unhandled packet: {0}", header.id);
        return false;
    }

#undef HANDLE_PACKET
}
