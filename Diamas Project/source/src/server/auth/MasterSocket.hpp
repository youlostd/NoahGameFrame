#ifndef METIN2_SERVER_AUTH_MASTERSOCKET_HPP
#define METIN2_SERVER_AUTH_MASTERSOCKET_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/Socket.hpp>
#include <net/ClientSocketAdapter.hpp>

#include <storm/String.hpp>

#include <boost/asio/ip/tcp.hpp>



struct PacketHeader;

class Server;

class MasterSocket : public ClientSocketAdapter<Socket>
{
	public:
		MasterSocket(Server& server, boost::asio::io_service& ioService);

		void Disconnect() override;

		Server& GetServer() const { return m_server; }

	protected:
		void OnConnectSuccess() override;
		void OnConnectFailure(const boost::system::error_code& ec) override;

		bool HandlePacket(const PacketHeader& header,
		                          const boost::asio::const_buffer& data) override final;

		Server& m_server;
};



#endif
