#include "LoginSessions.hpp"
#include "Server.hpp"
#include "GameSocket.hpp"
#include "GameUtil.hpp"

#include <game/MasterPackets.hpp>
#include <thecore/utils.hpp>
#include <net/Util.hpp>


#include <storm/StringUtil.hpp>

#include <random>


namespace
{

struct CompareSessionPtrs
{
	bool operator()(const LoginSession* a, const LoginSession* b) const
	{
		return a->expires < b->expires;
	}

	bool operator()(const LoginSession* a,
	                const std::chrono::steady_clock::time_point& b) const
	{
		return a->expires < b;
	}

	bool operator()(const std::chrono::steady_clock::time_point& a,
	                const LoginSession* b) const
	{
		return a < b->expires;
	}
};

static const std::chrono::minutes kExpireTime(30);

}

LoginSession* LoginSessions::Login(uint32_t aid,
                                   const std::string& login)
{
	const auto res = m_sessions.emplace(aid, std::unique_ptr<LoginSession>());
	auto& session = res.first->second;

	if (res.second) {
		session.reset(new LoginSession());
		session->aid = aid;
		session->login = login;

		
		const auto res2 = m_sessionsByLogin.emplace(session->login, session.get());
		if (!res2.second) {
			SPDLOG_ERROR( "Account {0} ({1}) is already registered",
			          login, aid);
		}
		SPDLOG_DEBUG("Registered account {}", session->login);
	} else {
		// If the user isn't logged-in, replace his active session.
		if (session->state != LoginSession::kAuthenticated) {
			SPDLOG_DEBUG("Warning: Account {0} ({1}) is already registered",
			           login, aid);
			return nullptr;
		}
	}

	session->id = MakeNewId();
	session->state = LoginSession::kAuthenticated;
	SPDLOG_DEBUG("Authenticated {} given session id {}", login, session->id);
	return session.get();
}

void LoginSessions::StartExpiring(LoginSession* session)
{
	session->expires = std::chrono::steady_clock::now() + kExpireTime;
	spdlog::trace("Session for {} expires {}", session->login, session->expires.time_since_epoch().count());

	const auto it = std::upper_bound(m_expiringSessions.begin(),
	                                 m_expiringSessions.end(),
	                                 session, CompareSessionPtrs());
	m_expiringSessions.insert(it, session);
}

void LoginSessions::StopExpiring(LoginSession* session)
{
	m_expiringSessions.erase(std::remove_if(m_expiringSessions.begin(), m_expiringSessions.end(), [session] (const LoginSession* session2) {
		return session2 == session;
	}), m_expiringSessions.end());


}

void LoginSessions::Kill(LoginSession* session)
{
	// Defer destruction until the user is no longer logged-in.
	if (session->game) {
		SendLoginDisconnectPacket(session->game, session->aid);
		session->state = LoginSession::kInvalidated;
		return;
	}

	m_expiringSessions.erase(std::remove_if(m_expiringSessions.begin(), m_expiringSessions.end(), [session] (const LoginSession* session2) {
		return session2 == session;
	}), m_expiringSessions.end());
	// Remove() doesn't do this for us...


	Remove(session);
}

void LoginSessions::Kill(GameSocket* game)
{
	std::unordered_set<LoginSession*> sessionDeletionSet;

	for(const auto& [aid, session] : m_sessions) {
		if(session.get()->game == game)
			sessionDeletionSet.insert(session.get());
	}


	for(auto& session : sessionDeletionSet) {
		m_expiringSessions.erase(std::remove_if(m_expiringSessions.begin(), m_expiringSessions.end(),
		                                        [session](const LoginSession* session2)
		                                        {
			                                        return session2 == session;
		                                        }), m_expiringSessions.end());
		// Remove() doesn't do this for us...
		Remove(session);
	}
}

void LoginSessions::Update()
{
	const auto now = std::chrono::steady_clock::now();
	const auto it = std::upper_bound(m_expiringSessions.begin(),
	                                 m_expiringSessions.end(),
	                                 now, CompareSessionPtrs());

	std::for_each(m_expiringSessions.begin(), it,
	              [this] (LoginSession* session) {
		SPDLOG_INFO("Removing expired session: {0} {1}",
		          session->aid, session->login);
		Remove(session);
	});

	m_expiringSessions.erase(m_expiringSessions.begin(), it);
}

const  LoginSession* LoginSessions::Get(uint32_t aid) const
{
	return const_cast<LoginSessions*>(this)->Get(aid);
}

const  LoginSession* LoginSessions::Get(const std::string& login) const
{
	return const_cast<LoginSessions*>(this)->Get(login);
}

LoginSession* LoginSessions::Get(uint32_t aid)
{
	const auto it = m_sessions.find(aid);
	if (it != m_sessions.end())
		return it->second.get();

	return nullptr;
}

LoginSession* LoginSessions::Get(const std::string& login)
{
	const auto it = m_sessionsByLogin.find(login);
	if(it != m_sessionsByLogin.end())
		return it->second;

	return nullptr;
}

void LoginSessions::Remove(LoginSession* session)
{
	SPDLOG_DEBUG("DEBUG:Removed session {} {} {}", session->login, session->aid, session->id);

	m_sessionsByLogin.erase(session->login);
    m_sessions.erase(session->aid);
}

SessionId LoginSessions::MakeNewId()
{
	return Random::get<SessionId>();
}

