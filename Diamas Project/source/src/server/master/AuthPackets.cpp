#include "AuthPackets.hpp"
#include "AuthSocket.hpp"
#include "Server.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>

#include <game/AuthPackets.hpp>

bool HandleLoginPacket(AuthSocket* auth, const AmLoginPacket& p)
{
	spdlog::trace( "AUTH: Received login: {0} {1}",
	           p.aid, p.login);

	auto& sessions = auth->GetServer().GetLoginSessions();

	auto* session = sessions.Login(p.aid, p.login);
	if (!session) {
		MaLoginFailurePacket p2;
		p2.handle = p.handle;
		p2.status = "ALREADY";
		auth->Send(kMaLoginFailure, p2);

		auto* currentSession = sessions.Get(p.aid);
		if (currentSession)
			sessions.Kill(currentSession);
		else
			// If the player isn't logged-in, we should be able to register
			// a new session.
			SPDLOG_ERROR( "Unknown session failure for: {0} {1}",
			          p.aid, p.login);

		return true;
	}

	session->socialId = p.socialId;
	std::memcpy(session->premiumTimes, p.premiumTimes,
	            sizeof(session->premiumTimes));
	session->hwid = p.hwid;

	MaLoginSuccessPacket p2;
	p2.handle = p.handle;
	p2.sessionId = session->id;
	auth->Send(kMaLoginSuccess, p2);

	sessions.StartExpiring(session);
	return true;
}


