#ifndef METIN2_SERVER_MASTER_LOGINSESSIONS_HPP
#define METIN2_SERVER_MASTER_LOGINSESSIONS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>
#include <vstl/string.hpp>


#include <boost/chrono/system_clocks.hpp>
#include <game/length.h>
#include <chrono>


class GameSocket;

struct LoginSession
{
	enum State
	{
		// User is authenticated
		kAuthenticated,

		// User is connected to one of the game servers
		kLoggedIn,

		// User is in-game
		kInGame,

		// Invalidated by another login
		// Used to defer destruction until the game server acknowledged logout.
		kInvalidated,
	};

	// TODO(tim): weak_ptr?
	GameSocket* game;
	SessionId id;
	State state;
	uint32_t aid;
	std::string login;
	std::string socialId;
	uint32_t premiumTimes[PREMIUM_MAX_NUM];
	std::string hwid;

	std::chrono::steady_clock::time_point expires;
};

class LoginSessions
{
	using SessionMap = std::unordered_map<uint32_t, std::unique_ptr<LoginSession>>;

	public:
		LoginSession* Login(uint32_t aid, const std::string& login);

        void StartExpiring(LoginSession *session);
		void StopExpiring(LoginSession* session);

		void Kill(LoginSession* session);
		void Kill(GameSocket* game);


		void Update();

		const LoginSession* Get(uint32_t aid) const;
		const LoginSession* Get(const std::string& login) const;

		LoginSession*              Get(uint32_t aid);
		LoginSession*              Get(const std::string& login);


    SessionMap& GetAllSessions() { return m_sessions; };
	private:
		void Remove(LoginSession* session);
		SessionId MakeNewId();

		SessionMap m_sessions;

		// key is owned by LoginSession*, which is owned by |m_sessions|
		std::unordered_map<std::string, LoginSession*> m_sessionsByLogin;

		std::vector<LoginSession*> m_expiringSessions;
};



#endif
