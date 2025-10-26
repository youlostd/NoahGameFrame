#include "AuthSocket.hpp"
#include "AuthPackets.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>

AuthSocket::AuthSocket(Server& server, asio::ip::tcp::socket socket)
	: Socket(std::move(socket))
	, m_server(server)
{
	// ctor
}

void AuthSocket::Setup()
{
	// This implicitly calls shared_from_this() which cannot be done in
	// the constructor.
	StartReadSome();
}

void AuthSocket::Disconnect()
{
	SPDLOG_INFO("AUTH: Connection {0} closed.", m_socket);
	SocketBase::Disconnect();
}

/*virtual*/ bool AuthSocket::HandlePacket(const PacketHeader& header,
                                          const asio::const_buffer& data)
{
#define HANDLE_PACKET(id, fn, type) \
	case id: return fn(this, ReadPacket<type>(data))

	switch (header.id) {
		HANDLE_PACKET(kAmLogin, HandleLoginPacket, AmLoginPacket);

		default:
			SPDLOG_ERROR( "Unhandled packet: {0}", header.id);
			return false;
	}

#undef HANDLE_PACKET
}

