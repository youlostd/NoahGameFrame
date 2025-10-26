#include "DbCacheSocket.hpp"
#include "DbCachePackets.hpp"

#include <game/DbPackets.hpp>

#include <net/Util.hpp>

#include <storm/StringUtil.hpp>

#include <cstring>



DbCacheSocket::DbCacheSocket(Server& server, asio::io_service& ioService)
	: ClientSocketAdapter(ioService,
	                      256 * 1024, 256 * 1024)
	, m_server(server)
{
	// ctor
}

void DbCacheSocket::Disconnect()
{
	SPDLOG_INFO("Connection {0} closed.", m_socket);

	ClientSocketAdapter::Disconnect();
}

void DbCacheSocket::Send(uint8_t id, uint32_t handle,
                         const void* data, uint32_t size)
{
	DbPacketHeader hdr;
	hdr.id = id;
	hdr.handle = handle;
	hdr.size = size;
	SocketBase::Send(&hdr, sizeof(hdr));
	SocketBase::Send(data, size);
}

/*virtual*/ void DbCacheSocket::OnConnectSuccess()
{
	SPDLOG_INFO("Successfully connected {0}", m_socket);
	StartReadSome();
}

/*virtual*/ void DbCacheSocket::OnConnectFailure(const boost::system::error_code& ec)
{
	if (ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
		return;

	spdlog::error("Failed to connect to {0}:{1} with {2}",
	          m_host, m_service, ec.message());

	Reconnect();
}

/*virtual*/ uint32_t DbCacheSocket::ProcessData(const asio::const_buffer& data)
{
	STORM_ASSERT(m_socket.is_open(), "Disconnected");

	using asio::buffer_size;
	using asio::buffer_cast;

	asio::const_buffer cur(data);
	while (buffer_size(cur) >= sizeof(DbPacketHeader)) {
		auto p = buffer_cast<const DbPacketHeader*>(cur);

		if (buffer_size(cur) - sizeof(DbPacketHeader) < p->size)
			break;

		cur = cur + sizeof(DbPacketHeader);

		HandleDbPacket(this, *p, asio::buffer(cur, p->size));

		if (!m_socket.is_open())
			break;

		cur = cur + p->size;
	}

	return buffer_size(data) - buffer_size(cur);
}


