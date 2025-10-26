#include "GameSocket.hpp"
#include "GamePackets.hpp"
#include "Server.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>

GameSocket::GameSocket(Server& server, asio::ip::tcp::socket socket)
    : Socket(std::move(socket))
    , m_server(server)
    , m_publicPort(0)
    , m_channel(0)
{
	// ctor
}

void GameSocket::Setup()
{
	// This implicitly calls shared_from_this() which cannot be done in
	// the constructor.
	StartReadSome();
}

void GameSocket::Disconnect()
{
	SPDLOG_INFO("GAME: Connection {0} closed.", m_socket);
	SocketBase::Disconnect();
}
void GameSocket::OnDisconnect()
{
	GetServer().GetLoginSessions().Kill(this);
	GetServer().GetOnlinePlayers().RemoveAll(this);
	GetServer().UnregisterGameSocket(this);
}
/*virtual*/ bool GameSocket::HandlePacket(const PacketHeader& header,
                                          const asio::const_buffer& data)
{
	try {
#define HANDLE_PACKET(id, fn, type)                                            \
	case id:                                                                   \
		return fn(this, ReadPacket<type>(data))

		switch (header.id) {
			HANDLE_PACKET(kGmSetup, HandleSetupPacket, GmSetupPacket);
			HANDLE_PACKET(kGmShutdownBroadcast, HandleShutdownBroadcastPacket,
			              GmShutdownBroadcastPacket);
			HANDLE_PACKET(kGmMaintenanceBroadcast,
			              HandleMaintenanceBroadcastPacket,
			              GmMaintenanceBroadcastPacket);
			HANDLE_PACKET(kGmLogin, HandleLoginPacket, GmLoginPacket);
			HANDLE_PACKET(kGmLogout, HandleLogoutPacket, GmLogoutPacket);

			HANDLE_PACKET(kGmCharacterEnter, HandleCharacterEnterPacket,
			              GmCharacterEnterPacket);
			HANDLE_PACKET(kGmCharacterLeave, HandleCharacterLeavePacket,
			              GmCharacterLeavePacket);
			HANDLE_PACKET(kGmCharacterDisconnect,
			              HandleCharacterDisconnectPacket,
			              GmCharacterDisconnectPacket);
			HANDLE_PACKET(kGmCharacterWarp, HandleCharacterWarpPacket,
			              GmCharacterWarpPacket);
			HANDLE_PACKET(kGmCharacterTransfer, HandleCharacterTransferPacket,
			              GmCharacterTransferPacket);
			HANDLE_PACKET(kGmMonarchCharacterWarp,
			              HandleMonarchCharacterWarpPacket,
			              GmCharacterWarpPacket);

			HANDLE_PACKET(kGmMessengerAdd, HandleMessengerAddPacket,
			              GmMessengerAddPacket);
			HANDLE_PACKET(kGmMessengerRequestAdd,
			              HandleMessengerRequestAddPacket,
			              GmMessengerRequestAddPacket);
			HANDLE_PACKET(kGmMessengerAuthAdd, HandleMessengerAuthAddPacket,
			              GmMessengerAuthAddPacket);
			HANDLE_PACKET(kGmMessengerDel, HandleMessengerDelPacket,
			              GmMessengerDelPacket);

			HANDLE_PACKET(kGmGuildChat, HandleGuildChatPacket,
			              GmGuildChatPacket);
			HANDLE_PACKET(kGmGuildMemberCountBonus,
			              HandleGuildMemberCountBonusPacket,
			              GmGuildMemberCountBonusPacket);
			HANDLE_PACKET(kGmGuildWarZoneInfo, HandleGuildWarZoneInfoPacket,
			              GmGuildWarZoneInfoPacket);

			HANDLE_PACKET(kGmChat, HandleChatPacket, GmChatPacket);
			HANDLE_PACKET(kGmWhisper, HandleWhisperPacket, GmWhisperPacket);
			HANDLE_PACKET(kGmWhisperError, HandleWhisperErrorPacket,
			              GmWhisperErrorPacket);
			HANDLE_PACKET(kGmShout, HandleShoutPacket, GmShoutPacket);
			HANDLE_PACKET(kGmNotice, HandleNoticePacket, GmNoticePacket);
			HANDLE_PACKET(kGmBlockChat, HandleBlockChatPacket,
			              GmBlockChatPacket);
			HANDLE_PACKET(kGmOfflineShopBuyInfo, HandleOfflineShopBuyInfoPacket,
			              GmOfflineShopBuyInfoPacket);
			HANDLE_PACKET(kGmOfflineShopTime, HandleOfflineTimePacket,
			              GmOfflineShopTimePacket);
			HANDLE_PACKET(kGmWorldBoss, HandleWorldBossPacket,
			              GmWorldBossPacket);
			HANDLE_PACKET(kGmHyperlinkItemRequest, HandleHyperlinkRequestPacket,
			              GmHyperlinkRequestPacket);
			HANDLE_PACKET(kGmHyperlinkResult, HandleHyperlinkResultPacket,
			              GmHyperlinkResultPacket);

			default:
				SPDLOG_ERROR("Unhandled packet: {0}", header.id);
				return true;
		}

#undef HANDLE_PACKET

	} catch (const std::exception& ex) {
		SPDLOG_ERROR("Error handling packet {}, {}", header.id, ex.what());
		return true;
	}
}
