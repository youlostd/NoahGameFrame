#include "Server.hpp"
#include "Config.hpp"
#include "AuthSocket.hpp"
#include "GameSocket.hpp"

#include <net/Util.hpp>



Server::Server(asio::io_service& ioService)
	: m_ioService(ioService)
	, m_authAcceptor(ioService)
	, m_gameAcceptor(ioService)
	, m_onlinePlayers(*this)
{
	// ctor
}


bool Server::Start(const Config& config)
{
	if (!StartAuthService(config))
		return false;

	if (!StartGameService(config))
		return false;

	return true;
}

void Server::Quit()
{
	if (!m_authSockets.empty())
		SPDLOG_ERROR(
		          "Auth servers should be closed before the master.");

	if (!m_gameSockets.empty())
		SPDLOG_ERROR(
		          "Game servers should be closed before the master.");

	for (const auto& p : m_authSockets)
		p->Disconnect();

	for (const auto& p : m_gameSockets)
	{	
		if(p)
			p->Disconnect();
	}
	m_authAcceptor.Close();
	m_gameAcceptor.Close();
}

bool Server::StartAuthService(const Config& config)
{
	boost::system::error_code ec;
	m_authAcceptor.Bind(config.authBindAddr, config.authBindPort, ec,
	                [this] (asio::ip::tcp::socket socket) {
	    SPDLOG_INFO("AUTH: Accepted {0}", socket);
		RegisterAuthSocket(std::move(socket));
	});

	if (ec) {
		SPDLOG_ERROR( "AUTH: Failed to bind on {0}:{1} with {2}",
		          config.authBindAddr, config.authBindPort, ec);
		return false;
	}

	SPDLOG_INFO("AUTH: Listening on {0}:{1}",
	          config.authBindAddr, config.authBindPort);
	return true;
}

bool Server::StartGameService(const Config& config)
{
	boost::system::error_code ec;
	m_gameAcceptor.Bind(config.gameBindAddr, config.gameBindPort, ec,
	                [this] (asio::ip::tcp::socket socket) {
	    SPDLOG_INFO("GAME: Accepted {0}", socket);
		RegisterGameSocket(std::move(socket));
	});

	if (ec) {
		SPDLOG_ERROR( "GAME: Failed to bind on {0}:{1} with {2}",
		          config.gameBindAddr, config.gameBindPort, ec);
		return false;
	}

	SPDLOG_INFO("GAME: Listening on {0}:{1}",
	          config.gameBindAddr, config.gameBindPort);
	return true;
}

void Server::RegisterAuthSocket(asio::ip::tcp::socket socket)
{
	auto p = std::make_shared<AuthSocket>(*this, std::move(socket));
	p->Setup();
	m_authSockets.emplace_back(std::move(p));
}

void Server::RegisterGameSocket(asio::ip::tcp::socket socket)
{
	auto p = std::make_shared<GameSocket>(*this, std::move(socket));
	p->Setup();
	m_gameSockets.emplace_back(std::move(p));
}

void Server::UnregisterGameSocket(GameSocket* game)
{
	m_gameSockets.erase(std::remove_if(m_gameSockets.begin(),
	                                   m_gameSockets.end(),
	                                   [game](GameSocketPtr gameSocket)
	                                   {
		                                   return gameSocket.get() == game;
	                                   }),
	                    m_gameSockets.end());
}

