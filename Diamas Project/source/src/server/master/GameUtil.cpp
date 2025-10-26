#include "GameUtil.hpp"
#include "GameSocket.hpp"

#include <game/MasterPackets.hpp>



void SendLoginSuccessPacket(GameSocket* game, uint32_t handle, uint32_t aid,
                            const std::string& socialId,
                            const uint32_t* premiumTimes,
                            const std::string& hwid)
{
	MgLoginSuccessPacket p;
	p.handle = handle;
	p.aid = aid;
	p.socialId = socialId;
	std::memcpy(p.premiumTimes, premiumTimes, sizeof(p.premiumTimes));
	p.hwid = hwid;
	game->Send(kMgLoginSuccess, p);
}

void SendLoginFailurePacket(GameSocket* game, uint32_t handle,
                            const std::string& status)
{
	MgLoginFailurePacket p;
	p.handle = handle;
	p.status = status;
	game->Send(kMgLoginFailure, p);
}

void SendLoginDisconnectPacket(GameSocket* game, uint32_t aid)
{
	MgLoginDisconnectPacket p;
	p.aid = aid;
	game->Send(kMgLoginDisconnect, p);
}


