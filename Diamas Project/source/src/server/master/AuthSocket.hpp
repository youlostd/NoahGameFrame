#ifndef METIN2_SERVER_MASTER_AUTHSOCKET_HPP
#define METIN2_SERVER_MASTER_AUTHSOCKET_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/Socket.hpp>

#include <boost/asio/ip/tcp.hpp>


class Server;

class AuthSocket : public Socket
{
	public:
		AuthSocket(Server& server, asio::ip::tcp::socket socket);

		void Setup();
		virtual void Disconnect();

		Server& GetServer() const { return m_server; }

	protected:
		virtual bool HandlePacket(const PacketHeader& header,
		                          const asio::const_buffer& data);

	private:
		Server& m_server;
};



#endif
