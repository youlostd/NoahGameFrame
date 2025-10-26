#include "MasterSocket.hpp"
#include "MasterPackets.hpp"


#include <net/Util.hpp>

#include <storm/StringUtil.hpp>

#include <cstring>
#include <game/MasterPackets.hpp>


MasterSocket::MasterSocket(Server& server, boost::asio::io_service& ioService)
	: ClientSocketAdapter(ioService)
	, m_server(server)
{
	// ctor
}

void MasterSocket::Disconnect()
{
	SPDLOG_INFO("MASTER: Connection {0} closed.", m_socket);

	SocketBase::Disconnect();
}

/*virtual*/ void MasterSocket::OnConnectSuccess()
{
	SPDLOG_INFO("MASTER: Successfully connected {0}", m_socket);

	StartReadSome();
}

/*virtual*/ void MasterSocket::OnConnectFailure(const boost::system::error_code& ec)
{
	if (ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
		return;

	spdlog::error("MASTER: Failed to connect to {0}:{1} with {2}",
	          m_host, m_service, ec.message());

	Reconnect();
}

/*virtual*/ bool MasterSocket::HandlePacket(const PacketHeader& header,
                                            const boost::asio::const_buffer& data)
{
#define HANDLE_PACKET(id, fn, type) \
	case id: return fn(this, ReadPacket<type>(data))

	switch (header.id) {
		HANDLE_PACKET(kMaLoginSuccess, HandleLoginSuccessPacket,
		              MaLoginSuccessPacket);
		HANDLE_PACKET(kMaLoginFailure, HandleLoginFailurePacket,
		              MaLoginFailurePacket);

		default:
			spdlog::error("Unhandled packet: {0}", header.id);
			return false;
	}

#undef HANDLE_PACKET
}


