#ifndef METIN2_SERVER_MASTER_GAMESOCKET_HPP
#define METIN2_SERVER_MASTER_GAMESOCKET_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/Socket.hpp>

#include <boost/asio/ip/tcp.hpp>


class Server;

class GameSocket : public Socket
{
	public:
		GameSocket(Server& server, asio::ip::tcp::socket socket);

		void         Setup();
		virtual void Disconnect();
        void         OnDisconnect();

        Server& GetServer() const { return m_server; }

		const std::string& GetPublicAddr() const { return m_publicAddr; }

		void SetPublicAddr(const std::string& addr)
		{ m_publicAddr = addr; }

		uint16_t GetPublicPort() const { return m_publicPort; }
		void SetPublicPort(uint16_t port) { m_publicPort = port; }

		uint8_t GetChannel() const { return m_channel; }
		void SetChannel(uint8_t channel) { m_channel = channel; }

		const std::vector<uint32_t>& GetHostedMaps() const { return m_hostedMaps; }

		void SetHostedMaps(const std::vector<uint32_t>& hostedMaps)
		{ m_hostedMaps = hostedMaps; }

	protected:
		virtual bool HandlePacket(const PacketHeader& header,
		                          const asio::const_buffer& data);

	private:
		Server& m_server;
		std::string m_publicAddr;
		uint16_t m_publicPort;

		uint8_t m_channel;
		std::vector<uint32_t> m_hostedMaps;
};



#endif
