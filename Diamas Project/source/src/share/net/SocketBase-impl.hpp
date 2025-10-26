#ifndef METIN2_NET_SOCKETBASE_IMPL_HPP
#define METIN2_NET_SOCKETBASE_IMPL_HPP


#include <SpdLog.hpp>

#include <boost/asio/buffer.hpp>
#include <storm/ErrorReporting.hpp>
#include <net/Util.hpp>
#include <gsl/span>


BOOST_FORCEINLINE bool SocketBaseBuffers::CanSend(uint32_t size) const {
	const auto cur = m_sendBuffer.size() + m_lockedSendBuffer.size() + size;
	if(cur >= m_maxSendBufferSize)
		m_resizeSendBuffer = true;
	return true;
}

template <class Socket>
SocketBase<Socket>::SocketBase(Socket s,
                               uint32_t initialSendBufferSize,
                               uint32_t initialRecvBufferSize,
                               uint32_t maxSendBufferSize,
                               uint32_t maxRecvBufferSize)
	: SocketBaseBuffers(initialSendBufferSize, initialRecvBufferSize,
	                    maxSendBufferSize, maxRecvBufferSize)
	  , m_socket(std::move(s))
{
	// ctor
}

template <class Socket>
SocketBase<Socket>::~SocketBase() {

}

template <class Socket>
/*virtual*/ void SocketBase<Socket>::Disconnect()
{
	if(m_socket.is_open()) {
		boost::system::error_code ec;

		m_socket.shutdown(Socket::shutdown_both, ec);
		m_socket.close(ec);

	    OnDisconnect();
		// We really don't expect this one...
		if(ec)
			spdlog::critical("Failed to close socket: {}", ec.message());
	}
}

template <class Socket>
BOOST_FORCEINLINE Socket& SocketBase<Socket>::GetSocket()
{
	return m_socket;
}

template <class Socket>
BOOST_FORCEINLINE const Socket& SocketBase<Socket>::GetSocket() const
{
	return m_socket;
}

template <class Socket>
BOOST_FORCEINLINE bool SocketBase<Socket>::IsClosed() const
{
	return !m_socket.is_open();
}

template <class Socket>
void* SocketBase<Socket>::Send(uint32_t size)
{
	if(size == 0 || !m_socket.is_open() || !VerifySend(size))
		return nullptr;

	const auto oldSize = m_sendBuffer.size();
	m_sendBuffer.resize(oldSize + size);

	if(m_autoFlushSendBuffer) {
		auto self(this->shared_from_this());
		boost::asio::post(m_socket.get_executor() ,[this, self]
		{
			Flush();
		});

	}

	return &m_sendBuffer[oldSize];
}

template <class Socket>
void SocketBase<Socket>::Send(const void* packet, uint32_t size)
{
	// TODO(tim): Ignore sends on closed sockets?
	if(size == 0 || !m_socket.is_open() || !VerifySend(size))
		return;

	const auto packetBytes = gsl::make_span(static_cast<const uint8_t*>(packet), size);
	m_sendBuffer.insert(std::end(m_sendBuffer), std::begin(packetBytes), std::end(packetBytes));
	// What about std::length_error?

	if(m_autoFlushSendBuffer) {
		auto self(this->shared_from_this());
		boost::asio::post(m_socket.get_executor(), [this, self]
		{
			Flush();
		});
	}
}

template <class Socket>
void SocketBase<Socket>::Flush()
{
	if(m_lockedSendBuffer.empty())
		StartWriteSome();
}

template <class Socket>
bool SocketBase<Socket>::VerifySend(uint32_t size) const
{
	if(!CanSend(size)) {
		// At this point we cannot send because our buffer reached its maximum capacity
		// How should we handle this, currently we basically just report it and drop the packets
		SPDLOG_ERROR("{0}: Failed to send {1} bytes (pending {2} {3})",
		             m_socket, size, m_sendBuffer.size(),
		             m_lockedSendBuffer.size());
		return false;
	}

	return true;
}

template <class Socket>
bool SocketBase<Socket>::GrowRecvBuffer(uint32_t spaceNeeded)
{
	const uint32_t curSize = m_recvBuffer.size();
	if(curSize - m_recvFillSize >= spaceNeeded)
		return true;

	const auto newSize = std::min<uint32_t>(spaceNeeded + m_recvBuffer.size() * 2,
	                              m_maxRecvBufferSize);
	if(newSize == curSize) {
		SPDLOG_ERROR(
			"{0}: Failed to grow recv buffer to {1} limit {2} fill {3}",
			m_socket, newSize, m_maxRecvBufferSize, m_recvFillSize);
		return false;
	}

	m_recvBuffer.resize(newSize);
	return true;
}

template <class Socket>
void SocketBase<Socket>::StartWriteSome()
{
	std::swap(m_sendBuffer, m_lockedSendBuffer);

	auto self(this->shared_from_this());
	m_socket.async_write_some(boost::asio::buffer(m_lockedSendBuffer),
	                          std::bind(&SocketBase<Socket>::HandleWriteSome, self, std::placeholders::_1,
	                                    std::placeholders::_2));
}

template <class Socket>
void SocketBase<Socket>::StartReadSome()
{
	static const uint32_t kMinRecvSize = 128;
	if(!GrowRecvBuffer(kMinRecvSize)) {
		Disconnect();
		return;
	}

	boost::asio::mutable_buffer buf(m_recvBuffer.data() + m_recvFillSize,
	                                m_recvBuffer.size() - m_recvFillSize);

	auto self(this->shared_from_this());
	m_socket.async_read_some(boost::asio::buffer(buf),
	                         std::bind(&SocketBase<Socket>::HandleReadSome, self, std::placeholders::_1,
	                                   std::placeholders::_2));
}

template <class Socket>
void SocketBase<Socket>::HandleWriteSome(const boost::system::error_code& ec,
                                         size_t size)
{
	if(size == 0) {
		if(!m_sendBuffer.empty())
			StartWriteSome();
		return;
	}

	if(ec) {
		// Silently ignore aborts (socket closed, ...)
		if(ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
			return;

		SPDLOG_ERROR("Failed to write with {0}", ec.message());
		Disconnect();
		return;
	}

	STORM_ASSERT(m_lockedSendBuffer.size() >= size,
	             "Cannot send more than available");

	// Discard what we've sent - not needed anymore
	m_lockedSendBuffer.erase(m_lockedSendBuffer.begin(),
	                         m_lockedSendBuffer.begin() + size);

	// If the send buffer was used while we were locked,
	// we have to append the content to our unsent data.
	if(!m_sendBuffer.empty()) {
		m_lockedSendBuffer.insert(m_lockedSendBuffer.end(),
		                          std::make_move_iterator(m_sendBuffer.begin()),
		                          std::make_move_iterator(m_sendBuffer.end()));
		//std::move(m_sendBuffer.begin(), m_sendBuffer.end(), std::back_inserter(m_lockedSendBuffer));  // ##
		
		m_sendBuffer.clear();
	}

	std::swap(m_sendBuffer, m_lockedSendBuffer);

	// If we have something to send - we need to do so immediately
	if(!m_sendBuffer.empty())
		StartWriteSome();
}

template <class Socket>
void SocketBase<Socket>::HandleReadSome(const boost::system::error_code& ec,
                                        size_t size)
{
	if(ec) {
		// Silently ignore aborts (socket closed, ...)
		if(ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
			return;

		if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
			OnRemoteDisconnect();
        } else {
			SPDLOG_ERROR("Failed to read with {0}", ec.message());
        }

		
        Disconnect();
        return;
    }

    m_recvFillSize += size;

	// Buffer overflow risk control @Adalet
    if (m_recvFillSize > m_recvBuffer.size()) {
        SPDLOG_ERROR("Buffer overflow detected");
        Disconnect();
        return;
    }

    const auto processed = ProcessData(boost::asio::buffer(m_recvBuffer,
                                                           m_recvFillSize));

    // If the socket was closed, abort
    if(!m_socket.is_open())
        return;

    // Now that we have the chance, compact the recv buffer.
    if(processed != 0) {
        STORM_ASSERT(processed <= m_recvFillSize,
                     "Buffer overflow");

        m_recvBuffer.erase(m_recvBuffer.begin(),
                           m_recvBuffer.begin() + processed);

        m_recvFillSize -= processed;
    }

    // Unconditionally start receiving data again
    StartReadSome();
    }


#endif
