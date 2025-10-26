#include "MasterUtil.hpp"
#include "MasterSocket.hpp"
#include "Server.hpp"

#include <game/MasterPackets.hpp>

#include <cstring>



void SendLoginPacket(MasterSocket* master, uint32_t handle, uint32_t aid,
                     const std::string& login,
                     const std::string& socialId,
                     const uint32_t* premiumTimes,
                     const std::string& hwid)
{
	AmLoginPacket p;
	p.handle = handle;
	p.aid = aid;
	p.login = login;
	p.socialId = socialId;
	std::memcpy(p.premiumTimes, premiumTimes,
	            sizeof(p.premiumTimes));
	p.hwid = hwid;
	master->Send(kAmLogin, p);
}


