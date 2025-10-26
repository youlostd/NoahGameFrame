#include "ClientSocket.hpp"
#include "ClientPackets.hpp"
#include "Server.hpp"

#include <game/AuthPackets.hpp>

#include <net/Util.hpp>


ClientSocket::ClientSocket(Server& server, uint32_t handle,
                           asio::ip::tcp::socket socket)
    : Socket(std::move(socket))
      , m_server(server)
      , m_handle(handle)
      , m_securityCode{0}, m_aid(0)
{
    std::memset(&m_premiumTimes, 0, sizeof(m_premiumTimes));
}

void ClientSocket::Setup()
{
	// This implicitly calls shared_from_this() which cannot be done in
	// the constructor.
	StartReadSome();
}

void ClientSocket::Disconnect()
{
	SPDLOG_INFO("CLIENT: Connection {0} closed.", m_socket);

	if (!m_login.empty())
		m_server.ErasePendingLogin(m_login);

	Socket::Disconnect();
	m_server.UnregisterClientSocket(this);

}

/*virtual*/ bool ClientSocket::HandlePacket(const PacketHeader& header,
                                            const asio::const_buffer& data)
{
#define HANDLE_PACKET(id, fn, type) \
	case id: return fn(this, ReadPacket<type>(data))

	switch (header.id) {
		HANDLE_PACKET(kCaAuth, HandleAuthPacket, CaAuthPacket);
		HANDLE_PACKET(kCaRegisterRequest, HandleRegisterPacket, CaRegisterPacket);

		HANDLE_PACKET(kCaPinResponse, HandlePinResponsePacket,
		              CaPinResponsePacket);
		HANDLE_PACKET(kCaHGuardInfo, HandleHGuardInfoPacket,
		              CaHGuardInfoPacket);
		HANDLE_PACKET(kCaHGuardCode, HandleHGuardCodePacket,
		              CaHGuardCodePacket);
		default:
			SPDLOG_CRITICAL("Unhandled packet: {0}", header.id);
			return false;
	}

#undef HANDLE_PACKET
}


