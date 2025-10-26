#include "MasterPackets.hpp"
#include "MasterSocket.hpp"
#include "ClientUtil.hpp"
#include "Server.hpp"

#include <game/MasterPackets.hpp>




bool HandleLoginSuccessPacket(MasterSocket* master,
                              const MaLoginSuccessPacket& p)
{
	spdlog::trace("MASTER: Received login success: {0} {1}",
	           p.handle, p.sessionId);

	const auto client = master->GetServer().GetByHandle(p.handle);
	if (!client)
		return true;

	SendAuthSuccessPacket(client.get(), p.sessionId);
	return true;
}

bool HandleLoginFailurePacket(MasterSocket* master,
                              const MaLoginFailurePacket& p)
{
	spdlog::trace("MASTER: Received login failure: {0} {1}",
	           p.handle, p.status);

	const auto client = master->GetServer().GetByHandle(p.handle);
	if (!client)
		return true;

	SendAuthFailurePacket(client.get(), p.status);
	return true;
}


