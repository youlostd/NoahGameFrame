#include "Server.hpp"
#include "Config.hpp"
#include "ClientSocket.hpp"
#include "MasterSocket.hpp"
#include "DbCacheSocket.hpp"

#include <net/Util.hpp>



Server::Server(asio::io_service& ioService)
	: m_ioService(ioService)
	, m_acceptor(ioService)
	, m_currentHandle(0)
	, m_masterSocket(std::make_shared<MasterSocket>(*this, m_ioService))
	, m_dbCacheSocket(std::make_shared<DbCacheSocket>(*this, m_ioService))
{
	// ctor
}

bool Server::Start(const Config& config)
{
	if (!StartClientService(config))
		return false;

	if (!m_masterSocket->Connect(config.masterAddr, config.masterPort))
		return false;

	if (!m_dbCacheSocket->Connect(config.dbCacheAddr, config.dbCachePort))
		return false;

	return true;
}

void Server::Quit()
{
	for (const auto& p : m_clientSockets)
		p->Disconnect();

	m_clientSocketsByHandle.clear();
    m_clientSockets.clear();
	

	m_acceptor.Close();
}

ClientSocketPtr Server::GetByHandle(uint32_t handle)
{
	const auto it = m_clientSocketsByHandle.find(handle);
	if (it == m_clientSocketsByHandle.end())
		return ClientSocketPtr();

	return it->second;
}

bool Server::InsertPendingLogin(const std::string& login)
{
	return m_pendingLogins.insert(login).second;
}

void Server::ErasePendingLogin(const std::string& login)
{
	const auto count = m_pendingLogins.erase(login);
	assert(count == 1 && "Sanity");
}

bool Server::StartClientService(const Config& config)
{
	m_version = config.version;

	boost::system::error_code ec;
	m_acceptor.Bind(config.bindAddr, config.bindPort, ec,
	                [this] (asio::ip::tcp::socket socket) {
	    SPDLOG_INFO("Accepted connection on {0}", socket);
		RegisterClientSocket(std::move(socket));
	});

	if (ec) {
		spdlog::error("Failed to bind on {0}:{1} with {2}",
		          config.bindAddr, config.bindPort, ec);
		return false;
	}

	SPDLOG_INFO("Listening for game connections on {0}:{1}",
	          config.bindAddr, config.bindPort);
	return true;
}

void Server::RegisterClientSocket(asio::ip::tcp::socket socket)
{
	// Seems like the connection was already closed just ignore it
    if (!socket.is_open())
        return;

    const auto handle = ++m_currentHandle;

	auto p = std::make_shared<ClientSocket>(*this, handle,
	                                        std::move(socket));
	p->Setup();

	m_clientSocketsByHandle.emplace(handle, p);
	m_clientSockets.emplace(std::move(p));
}

void Server::UnregisterClientSocket(ClientSocket* p)
{
    m_clientSockets.erase(std::static_pointer_cast<ClientSocket>(p->shared_from_this()));

    if (p->GetHandle() != 0)
        m_clientSocketsByHandle.erase(p->GetHandle());
}

void Server::PrintPendingLogins()
{
	SPDLOG_CRITICAL("START OF PENDING LIST");

	for(const auto& login : m_pendingLogins) {
		SPDLOG_CRITICAL(login);
	}

    SPDLOG_CRITICAL("END OF PENDING LIST");
}
