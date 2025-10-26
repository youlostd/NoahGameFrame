#ifndef METIN2_NET_SERVERSOCKETMAP_IMPL_HPP
#define METIN2_NET_SERVERSOCKETMAP_IMPL_HPP

#include <boost/static_assert.hpp>



template <class Socket>
ServerSocketMap<Socket>::ServerSocketMap()
{
	// ctor
}

template <class Socket>
const SocketPtr& ServerSocketMap<Socket>::GetSocket(const UniqueHandle& handle)
{
	BOOST_STATIC_ASSERT_MSG(sizeof(uint32_t) >= sizeof(SocketHandle),
	                        "UniqueHandle.id has to be capable of holding"
	                        "a complete native socket handle");

	const auto it = m_sockets.find(static_cast<SocketHandle>(handle.id));
	if (it != m_sockets.end()) {
		const auto& s = it->second;

		// Check whether the unique keys still match - socket could've been
		// re-used since the @c UniqueHandle was acquired.
		if (s->GetRandomKey() == handle.randomKey)
			return s;
	}

	return nullptr;
}

template <class Socket>
void ServerSocketMap<Socket>::Insert(const SocketPtr& p)
{
	auto r = m_sockets.insert(std::make_pair(p->GetNativeSocket(), p));
	STORM_ASSERT();
}

template <class Socket>
void ServerSocketMap<Socket>::Remove(Socket* p)
{
	m_sockets.erase(p->GetNativeSocket());
}



#endif
