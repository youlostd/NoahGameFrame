#ifndef METIN2_NET_SOCKETBASE_HPP
#define METIN2_NET_SOCKETBASE_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/asio/buffer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/noncopyable.hpp>

#include <memory>


class SocketBaseBuffers : private boost::noncopyable
{
	public:
		SocketBaseBuffers(uint32_t initialSendBufferSize,
		                  uint32_t initialRecvBufferSize,
		                  uint32_t maxSendBufferSize = 0xffffffff,
		                  uint32_t maxRecvBufferSize = 0xffffffff);

		SocketBaseBuffers(SocketBaseBuffers&& other) noexcept;

		void ResetBuffers();

		bool CanSend(uint32_t size) const;

		bool AutoFlushSendBuffer() const { return m_autoFlushSendBuffer; }
		void SetAutoFlushSendBuffer(bool flush) { m_autoFlushSendBuffer = flush; }

	protected:
		// TODO(tim): Evaluate whether a generic buffer class might be better here
		// So far, it doesn't seem that way.
		// http://think-async.com/Asio/asio-1.11.0/doc/asio/reference/DynamicBufferSequence.html

		/// The normal send buffer which is always appended to.
		std::vector<uint8_t> m_sendBuffer;

		/// The locked send buffer, this is what gets passed to
		/// asio and which remains immutable during the send-operation
		/// TODO(tim): Only needs to be immutable when a reallocation would occur.
		std::vector<uint8_t> m_lockedSendBuffer;

		uint32_t m_maxSendBufferSize;
	    mutable uint8_t m_resizeSendBuffer = false;

		bool m_autoFlushSendBuffer;

		std::vector<uint8_t> m_recvBuffer;
		uint32_t m_maxRecvBufferSize;

		/// Size of the send buffer's actively used part.
		/// Only the range [0, m_recvFillSize) contains valid data.
		uint32_t m_recvFillSize;
};

template <class Socket>
class SocketBase
	: public SocketBaseBuffers
	, public std::enable_shared_from_this<SocketBase<Socket>>
{
	public:
		typedef Socket SocketType;

		SocketBase(Socket s,
		           uint32_t initialSendBufferSize,
		           uint32_t initialRecvBufferSize,
		           uint32_t maxSendBufferSize = 0xffffffff,
		           uint32_t maxRecvBufferSize = 0xffffffff);

		virtual ~SocketBase();

		Socket& GetSocket();
		const Socket& GetSocket() const;

		bool IsClosed() const;

		virtual void Disconnect();
	    virtual void OnDisconnect() {};
	    virtual void OnRemoteDisconnect() {};

		void* Send(uint32_t size);
		void Send(const void* packet, uint32_t size);

		void Flush();

	protected:
		bool VerifySend(uint32_t size) const;

		bool GrowRecvBuffer(uint32_t spaceNeeded);

		void StartWriteSome();
		void StartReadSome();

		void HandleWriteSome(const boost::system::error_code& error,
		                     size_t size);

		void HandleReadSome(const boost::system::error_code& error,
		                    size_t size);

		/// Returns the amount of @c data processed
		virtual uint32_t ProcessData(const boost::asio::const_buffer& data) = 0;

		Socket m_socket;
};



#include <net/SocketBase-impl.hpp>

#endif
