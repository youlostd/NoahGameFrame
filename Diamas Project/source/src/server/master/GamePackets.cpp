#include "GamePackets.hpp"
#include "GameSocket.hpp"
#include "GameUtil.hpp"
#include "Server.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>



namespace
{

bool VerifyHostedMaps(Server& server, const GmSetupPacket& p)
{
	const auto& games = server.GetGameSockets();
	for (auto mapIndex : p.hostedMaps) {
		for (const auto& game : games) {
			const auto& maps = game->GetHostedMaps();
			const auto it = std::find(maps.begin(), maps.end(), mapIndex);

			// If the map isn't hosted on this server as well,
			// everything's fine.
			if (it == maps.end())
				continue;

			if(game->GetChannel() != p.channel)
				continue;

			SPDLOG_ERROR(
			          "Duplicate map {} on {}:{}, first seen on {}:{}",
			          mapIndex, p.publicAddr, p.publicPort,
			          game->GetPublicAddr(), game->GetPublicPort());

			return false;
		}
	}

	return true;
}

}

bool HandleSetupPacket(GameSocket* game, const GmSetupPacket& p)
{
	if (p.version != GmSetupPacket::kVersion) {
		SPDLOG_ERROR( "{0}: Version mismatch {1} != {2}",
		          game->GetSocket(), p.version, GmSetupPacket::kVersion);
		return true;
	}

	if (!VerifyHostedMaps(game->GetServer(), p)) {
		game->Disconnect();
		return true;
	}

	game->SetChannel(p.channel);
	game->SetPublicAddr(p.publicAddr);
	game->SetPublicPort(p.publicPort);
	game->SetHostedMaps(p.hostedMaps);

	// Exchange hosted maps:
	// * Broadcast our hosted maps
	// * Send hosted maps of our maps to us
	MgHostedMapsPacket hostedMapsPacket;
	hostedMapsPacket.channel = p.channel;
	hostedMapsPacket.publicAddr = p.publicAddr;
	hostedMapsPacket.publicPort = p.publicPort;
	hostedMapsPacket.hostedMaps = p.hostedMaps;

	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgHostedMaps, hostedMapsPacket, game);

	for (const auto& peer : game->GetServer().GetGameSockets()) {
		// Needed for auth server
		if (peer->GetHostedMaps().empty())
			continue;

		if (peer.get() == game)
			continue;

		hostedMapsPacket.channel = peer->GetChannel();
		hostedMapsPacket.publicAddr = peer->GetPublicAddr();
		hostedMapsPacket.publicPort = peer->GetPublicPort();
		hostedMapsPacket.hostedMaps = peer->GetHostedMaps();
		game->Send(kMgHostedMaps, hostedMapsPacket);
	}

	SPDLOG_INFO("Received setup from {0}:{1} ch {2}",
	          p.publicAddr, p.publicPort, p.channel);
	return true;
}

bool HandleShutdownBroadcastPacket(GameSocket* game,
                                   const GmShutdownBroadcastPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgShutdownBroadcast, p, game);
	return true;
}

bool HandleMaintenanceBroadcastPacket(GameSocket* game,
                                   const GmMaintenanceBroadcastPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMaintenanceBroadcast, p, game);
	return true;
}

bool HandleLoginPacket(GameSocket* game, const GmLoginPacket& p)
{
	spdlog::info( "GAME: LOGIN {0} {1}", p.login, p.sessionId);

	auto* session = game->GetServer().GetLoginSessions().Get(p.login);
	if (session == nullptr) {
		SPDLOG_ERROR( "No session for {0} with sessionId {1}", p.login, p.sessionId);
		SendLoginFailurePacket(game, p.handle, "BESAMEKEY");
		return true;
	}

	if (session->id != p.sessionId) {
		SPDLOG_ERROR( "Invalid session ID {0} != {1} for {2}",
		           session->id, p.sessionId, p.login);
		SendLoginFailurePacket(game, p.handle, "WRONGPWD");
		return true;
	}

	if (session->state != LoginSession::kAuthenticated) {
		SPDLOG_ERROR( "Session {0} already logged in user", session->id, p.login);
		SendLoginFailurePacket(game, p.handle, "ALREADY");
		game->GetServer().GetLoginSessions().Kill(session);
		return true;
	}

	SendLoginSuccessPacket(game, p.handle, session->aid,
	                       session->socialId, session->premiumTimes,
	                       session->hwid);

	assert(!session->game && "Why are we connected?");

	session->state = LoginSession::kLoggedIn;
	session->game = game;

	game->GetServer().GetLoginSessions().StopExpiring(session);
	return true;
}

bool HandleLogoutPacket(GameSocket* game, const GmLogoutPacket& p)
{
	spdlog::trace( "GAME: LOGOUT {0}", p.aid);

	auto* session = game->GetServer().GetLoginSessions().Get(p.aid);
	if (!session) {
		SPDLOG_DEBUG( "No session for {0}", p.aid);
		return true;
	}


	assert(session->state != LoginSession::kAuthenticated &&
	                 "Invalid state for operation");
	assert(session->game && "Not connected?");

	if (session->state != LoginSession::kInvalidated) {
		session->state = LoginSession::kAuthenticated;
		session->game = nullptr;

		game->GetServer().GetLoginSessions().StartExpiring(session);

		
	} else {
		session->game = nullptr;
		game->GetServer().GetLoginSessions().Kill(session);
	}

	
	return true;
}

bool HandleCharacterEnterPacket(GameSocket* game,
                                const GmCharacterEnterPacket& p)
{
	SPDLOG_INFO("Enter: {0} {1} ch {2} map {3}",
	          p.pid, p.name, p.channel, p.mapIndex);

	OnlinePlayer op;
	op.game = game;
	op.aid = p.aid;
	op.pid = p.pid;
	op.name = p.name;
	op.mapIndex = p.mapIndex;
	op.empire = p.empire;
	op.channel = p.channel;
	op.hwid = p.hwid;
	game->GetServer().GetOnlinePlayers().Enter(std::move(op));

	return true;
}

bool HandleCharacterLeavePacket(GameSocket* game,
                                const GmCharacterLeavePacket& p)
{
	SPDLOG_INFO("Leave: {0}", p.pid);

	game->GetServer().GetOnlinePlayers().Leave(p.pid);
	return true;
}

bool HandleCharacterDisconnectPacket(GameSocket* game,
                                     const GmCharacterDisconnectPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgCharacterDisconnect, p);
	return true;
}

bool HandleCharacterWarpPacket(GameSocket* game,
                               const GmCharacterWarpPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgCharacterWarp, p, game);
	return true;
}

bool HandleCharacterTransferPacket(GameSocket* game,
                                   const GmCharacterTransferPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgCharacterTransfer, p, game);
	return true;
}

bool HandleMonarchCharacterWarpPacket(GameSocket* game,
                                      const GmCharacterWarpPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMonarchCharacterWarp, p, game);
	return true;
}

bool HandleMessengerAddPacket(GameSocket* game,
                              const GmMessengerAddPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMessengerAdd, p, game);
	return true;
}

bool HandleMessengerRequestAddPacket(GameSocket* game,
                              const GmMessengerRequestAddPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMessengerRequestAdd, p, game);
	return true;
}

bool HandleMessengerAuthAddPacket(GameSocket* game,
                              const GmMessengerAuthAddPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMessengerAuthAdd, p);
	return true;
}


bool HandleMessengerDelPacket(GameSocket* game,
                              const GmMessengerDelPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgMessengerDel, p, game);
	return true;
}

bool HandleGuildChatPacket(GameSocket* game, const GmGuildChatPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgGuildChat, p, game);
	return true;
}

bool HandleGuildMemberCountBonusPacket(GameSocket* game,
                                       const GmGuildMemberCountBonusPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgGuildMemberCountBonus, p, game);
	return true;
}

bool HandleGuildWarZoneInfoPacket(GameSocket* game,
                                  const GmGuildWarZoneInfoPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgGuildWarZoneInfo, p, game);
	return true;
}

bool HandleChatPacket(GameSocket* game, const GmChatPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgChat, p, game);
	return true;
}

bool HandleWhisperPacket(GameSocket* game, const GmWhisperPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgWhisper, p, game);
	return true;
}

bool HandleWhisperErrorPacket(GameSocket* game, const GmWhisperErrorPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgWhisperError, p, game);
	return true;
}

bool HandleShoutPacket(GameSocket* game, const GmShoutPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgShout, p, game);
	return true;
}

bool HandleNoticePacket(GameSocket* game, const GmNoticePacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgNotice, p, game);
	return true;
}

bool HandleBlockChatPacket(GameSocket* game, const GmBlockChatPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgBlockChat, p, game);
	return true;
}

bool HandleOfflineShopBuyInfoPacket(GameSocket* game, const GmOfflineShopBuyInfoPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgOfflineShopBuyInfo, p, game);
	return true;
}

bool HandleOfflineTimePacket(GameSocket* game, const GmOfflineShopTimePacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgOfflineShopTime, p, game);
	return true;
}

bool HandleWorldBossPacket(GameSocket* game, const GmWorldBossPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgWorldBoss, p, game);
	return true;
}
bool HandleHyperlinkRequestPacket(GameSocket* game, const GmHyperlinkRequestPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgHyperlinkItemRequest, p);
	return true;
}
bool HandleHyperlinkResultPacket(GameSocket* game, const GmHyperlinkResultPacket& p)
{
	BroadcastPacket(game->GetServer().GetGameSockets(),
	                kMgHyperlinkResult, p);
	return true;
}
