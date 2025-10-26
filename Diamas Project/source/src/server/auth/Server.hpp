#ifndef METIN2_SERVER_AUTH_SERVER_HPP
#define METIN2_SERVER_AUTH_SERVER_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/Acceptor.hpp>

#include <boost/asio/ip/tcp.hpp>
namespace asio = boost::asio;

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>



struct Config;

class ClientSocket;

class MasterSocket;
class DbCacheSocket;

using ClientSocketPtr = std::shared_ptr<ClientSocket>;

class Server
{
	public:
		using ClientSockets = std::unordered_set<ClientSocketPtr>;

		Server(boost::asio::io_service& ioService);

		bool Start(const Config& config);
		void Quit();

		const ClientSockets& GetClientSockets() const
		{ return m_clientSockets; }

		ClientSocketPtr GetByHandle(uint32_t handle);

		uint64_t GetVersion() const 
		{ return m_version; }

		const std::shared_ptr<MasterSocket>& GetMasterSocket() const
		{ return m_masterSocket; }

		const std::shared_ptr<DbCacheSocket>& GetDbCacheSocket() const
		{ return m_dbCacheSocket; }

		bool InsertPendingLogin(const std::string& login);
		void ErasePendingLogin(const std::string& login);
		    void UnregisterClientSocket(ClientSocket* p);
    void PrintPendingLogins();

    private:
		bool StartClientService(const Config& config);

		void RegisterClientSocket(asio::ip::tcp::socket socket);

    asio::io_service& m_ioService;
		Acceptor m_acceptor;

		ClientSockets m_clientSockets;
		uint32_t m_currentHandle;
		std::unordered_map<uint32_t, ClientSocketPtr> m_clientSocketsByHandle;

		std::shared_ptr<MasterSocket> m_masterSocket;
		std::shared_ptr<DbCacheSocket> m_dbCacheSocket;

		// Set containing all names of accounts whose clients
		// are currently connected to this auth server.
		std::unordered_set<std::string> m_pendingLogins;

		uint32_t m_version = 0;
};



#endif
