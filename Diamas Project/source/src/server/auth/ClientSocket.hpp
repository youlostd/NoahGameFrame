#ifndef METIN2_SERVER_AUTH_CLIENTSOCKET_HPP
#define METIN2_SERVER_AUTH_CLIENTSOCKET_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <net/Socket.hpp>

#include <vstl/string.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <game/Constants.hpp>
namespace asio = boost::asio;



class Server;

class ClientSocket : public Socket
{
	public:
	
		enum ClientState
		{
			kAuth,
			kPin,
			kHGuard,
		};
		ClientSocket(Server& server, uint32_t handle,
		             asio::ip::tcp::socket socket);

		void Setup();
		virtual void Disconnect();

		Server& GetServer() const { return m_server; }
		uint32_t GetHandle() const { return m_handle; }

		const std::string& GetLogin() const { return m_login; }
		void SetLogin(const std::string& login) { m_login = login; }

		const std::string& GetPassword() const { return m_password; }
		void SetPassword(const std::string& password)
		{ m_password = password; }

		const std::string& GetHwid() const { return m_hwid; }
		void SetHwid(const std::string& hwid)
		{ m_hwid = hwid; }

		const std::string& GetHGHash() const { return m_hgHash; }
		void SetHGHash(const std::string& hwid)
		{ m_hgHash = hwid; }

		uint32_t GetSecurityCode() const { return m_securityCode; }
		void SetSecurityCode(uint32_t code) { m_securityCode = code; }

		bool IsUsingHGuard() const { return m_useHwidProt; }
		void SetHGuardStatus(bool v) { m_useHwidProt = v; }


		uint32_t GetAid() const { return m_aid; }
		void SetAid(uint32_t aid) { m_aid = aid; }

		const std::string& GetSocialId() const { return m_socialId; }
		void SetSocialId(const std::string& socialId)
		{ m_socialId = socialId; }

		uint32_t GetState() const { return m_state; }
		void SetState(uint32_t state) { m_state = state; }

		const uint32_t* GetPremiumTimes() const { return m_premiumTimes; }
		void SetPremiumTimes(const uint32_t* premiumTimes)
		{ std::memcpy(m_premiumTimes, premiumTimes, sizeof(m_premiumTimes)); }

	protected:
		virtual bool HandlePacket(const PacketHeader& header,
		                          const asio::const_buffer& data);

	private:
		Server& m_server;
		uint32_t m_handle;

		std::string m_login;
		std::string m_password;
		std::string m_hwid;
		std::string m_hgHash;

		uint32_t m_securityCode;
		uint32_t m_state = kAuth;
		bool m_useHwidProt = false;


		uint32_t m_aid;
		std::string m_socialId;
		uint32_t m_premiumTimes[PREMIUM_MAX_NUM];
};



#endif
