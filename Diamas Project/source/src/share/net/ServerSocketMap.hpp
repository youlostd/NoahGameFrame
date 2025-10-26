#ifndef METIN2_NET_SERVERSOCKETMAP_HPP
#define METIN2_NET_SERVERSOCKETMAP_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <unordered_map>



template <class Socket>
class ServerSocketMap
{
	public:
		typedef typename Socket::Pointer SocketPtr;
		typedef typename Socket::native_handle_type
			SocketHandle;

		ServerSocketMap();

		Socket* GetSocket(const UniqueHandle& handle);

		void Insert(const SocketPtr& p);
		void Remove(Socket* p);

	private:
		typedef std::unordered_map<SocketHandle, SocketPtr> RealMap;

		RealMap m_sockets;
};



#include "ServerSocketMap-impl.hpp"

#endif
