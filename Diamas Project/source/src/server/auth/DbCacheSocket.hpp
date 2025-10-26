#ifndef METIN2_SERVER_AUTH_DBCACHESOCKET_HPP
#define METIN2_SERVER_AUTH_DBCACHESOCKET_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/SocketBase.hpp>
#include <net/ClientSocketAdapter.hpp>

#include <storm/String.hpp>

#include <boost/asio/ip/tcp.hpp>
namespace asio = boost::asio;



class Server;

class DbCacheSocket
	: public ClientSocketAdapter<SocketBase<asio::ip::tcp::socket>>
{
	public:
		DbCacheSocket(Server& server, asio::io_service& ioService);

		void Disconnect() override final;

		void Send(uint8_t id, uint32_t handle,
		          const void* data, uint32_t size);

		Server& GetServer() const { return m_server; }

	protected:
		void OnConnectSuccess() override final;
		void OnConnectFailure(const boost::system::error_code& ec)
			override final;

		uint32_t ProcessData(const asio::const_buffer& data)
			override final;

		Server& m_server;
};



#endif
