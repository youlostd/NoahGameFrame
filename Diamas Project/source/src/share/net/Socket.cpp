#include <net/Socket.hpp>
#include <net/Util.hpp>


Socket::Socket(boost::asio::ip::tcp::socket socket)
	: SocketBase(std::move(socket),
	             256, 256, 256 * 1024, 256 * 1024)
{
	// ctor
}

/*virtual*/ Socket::~Socket()
{
	// dtor
}

uint32_t Socket::ProcessData(const boost::asio::const_buffer& data)
{
	STORM_ASSERT(m_socket.is_open(), "Disconnected");

	boost::asio::const_buffer cur(data);
	while (buffer_size(cur) >= sizeof(PacketHeader)) {
		const auto* header = boost::asio::buffer_cast<const PacketHeader*>(cur);


		// Partial recv's are usual. We need to handle this.
		if (sizeof(PacketHeader) + header->size > boost::asio::buffer_size(cur))
			break;

		boost::asio::const_buffer wholePacket(boost::asio::buffer_cast<const void*>(cur),
		                               sizeof(PacketHeader) + header->size);

		if (!HandlePacket(*header, wholePacket + sizeof(PacketHeader))) {
			SPDLOG_CRITICAL(
			          "Packet handler indicated failure for "
			          "packet {0} on {1} at {2}",
			          header->id, m_socket,
			          buffer_size(data) - buffer_size(cur));

			Disconnect();
			return false;
		}

		// If the handler closed our socket, we'll just abort here
		if (!m_socket.is_open())
			break;

		cur = cur + sizeof(PacketHeader) + header->size;
	}

	return boost::asio::buffer_size(data) - boost::asio::buffer_size(cur);
}


