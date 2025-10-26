#ifndef METIN2_SERVER_MASTER_SERVER_HPP
#define METIN2_SERVER_MASTER_SERVER_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "LoginSessions.hpp"
#include "OnlinePlayers.hpp"

#include <net/Acceptor.hpp>

#include <boost/asio/ip/tcp.hpp>
namespace asio = boost::asio;

#include <vector>
#include <memory>



struct Config;

class AuthSocket;
class GameSocket;

using AuthSocketPtr = std::shared_ptr<AuthSocket>;
using GameSocketPtr = std::shared_ptr<GameSocket>;

class Server
{
	public:
		using GameSockets = std::vector<GameSocketPtr>;
		using AuthSockets = std::vector<AuthSocketPtr>;

		Server(asio::io_service& ioService);

		bool Start(const Config& config);
		void Quit();

		const AuthSockets& GetAuthSockets() const
		{ return m_authSockets; }

		const GameSockets& GetGameSockets() const
		{ return m_gameSockets; }

		LoginSessions& GetLoginSessions()
		{ return m_loginSessions; }

		OnlinePlayers& GetOnlinePlayers()
		{ return m_onlinePlayers; }

		void UnregisterGameSocket(GameSocket*);

	private:
		bool StartAuthService(const Config& config);
		bool StartGameService(const Config& config);

		void RegisterAuthSocket(asio::ip::tcp::socket socket);
		void RegisterGameSocket(asio::ip::tcp::socket socket);

		asio::io_service& m_ioService;
		Acceptor m_authAcceptor;
		Acceptor m_gameAcceptor;

		AuthSockets m_authSockets;
		GameSockets m_gameSockets;
		LoginSessions m_loginSessions;
		OnlinePlayers m_onlinePlayers;
};


#endif
