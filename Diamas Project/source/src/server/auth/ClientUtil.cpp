#include "ClientUtil.hpp"
#include "ClientSocket.hpp"
#include "Server.hpp"

#include <game/AuthPackets.hpp>

#include <net/Util.hpp>
#include <game/Types.hpp>
#include <thecore/sha256.hpp>

std::string GetHGuardHash(const CaHGuardInfoPacket& p, uint32_t accID)
{


	std::string chain;
	chain.append(p.cpuId)
			.append(p.guid)
			.append(p.macAddr)
			.append(std::to_string(accID))
			.append("Lik1[2K%*8}Az[B!Ez(<@\\S#");


	return sha256(chain);
}

std::string GetHWIDHash(const CaHGuardInfoPacket& p)
{
	std::string chain;
	chain.append(p.cpuId)
		.append(p.guid)
		.append(p.macAddr)
		.append("K,u$#<W|k(T0=;d%Tt!k");

	return sha256(chain);
}


void SendPinRequestPacket(ClientSocket* client)
{
	AcRequestPacket p;
	client->Send(kAcPinRequest, p);
}

void SendSetPinRequestPacket(ClientSocket* client)
{
	AcRequestPacket p;
	client->Send(kAcSetPinRequest, p);
}

void SendHGuardRequestPacket(ClientSocket* client)
{
	AcRequestPacket p;
	client->Send(kAcHGuardRequest, p);
}

void SendHGuardResultPacket(ClientSocket* client, uint8_t result)
{
	AcHGuardResultPacket p;
	p.code = result;
	client->Send(kAcHGuardResult, p);
}

void SendAuthFailurePacket(ClientSocket* client,
                           const std::string& status)
{
	AcAuthFailurePacket p;
	p.status = status;
	client->Send(kAcAuthFailure, p);
}

void SendAuthSuccessPacket(ClientSocket* client,
                           SessionId sessionId)
{
	AcAuthSuccessPacket p;
	p.sessionId = sessionId;
	client->Send(kAcAuthSuccess, p);
}


