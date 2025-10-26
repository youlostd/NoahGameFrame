#include <net/SocketBase.hpp>

#include <storm/memory/Allocate.hpp>



SocketBaseBuffers::SocketBaseBuffers(uint32_t initialSendBufferSize,
                                     uint32_t initialRecvBufferSize,
                                     uint32_t maxSendBufferSize,
                                     uint32_t maxRecvBufferSize)
	: m_sendBuffer()
	, m_lockedSendBuffer()
	, m_maxSendBufferSize(maxSendBufferSize)
	, m_autoFlushSendBuffer(true)
	, m_recvBuffer(initialRecvBufferSize)
	, m_maxRecvBufferSize(maxRecvBufferSize)
	, m_recvFillSize(0)
{
	STORM_ASSERT(0 != initialSendBufferSize, "");
	STORM_ASSERT(0 != initialRecvBufferSize, "");
	STORM_ASSERT(0 != maxSendBufferSize, "");
	STORM_ASSERT(0 != maxRecvBufferSize, "");

	m_sendBuffer.reserve(initialSendBufferSize);
	m_lockedSendBuffer.reserve(initialSendBufferSize);
}

SocketBaseBuffers::SocketBaseBuffers(SocketBaseBuffers&& other) noexcept
	: m_sendBuffer(std::move(other.m_sendBuffer))
	, m_lockedSendBuffer(std::move(other.m_lockedSendBuffer))
	, m_maxSendBufferSize(other.m_maxSendBufferSize)
	, m_autoFlushSendBuffer(other.m_autoFlushSendBuffer)
	, m_recvBuffer(std::move(other.m_recvBuffer))
	, m_maxRecvBufferSize(other.m_maxRecvBufferSize)
	, m_recvFillSize(other.m_recvFillSize) 
{
	other.m_maxSendBufferSize = 0;
	other.m_maxRecvBufferSize = 0;
	other.m_recvFillSize = 0;
}

void SocketBaseBuffers::ResetBuffers()
{
	m_sendBuffer.clear();
	m_lockedSendBuffer.clear();

	m_recvFillSize = 0;
}


