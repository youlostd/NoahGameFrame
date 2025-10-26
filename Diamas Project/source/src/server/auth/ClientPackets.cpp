#include "ClientPackets.hpp"
#include "ClientSocket.hpp"
#include "ClientUtil.hpp"
#include "MasterUtil.hpp"
#include "Server.hpp"
#include "DbCacheSocket.hpp"

#include <game/AuthPackets.hpp>
#include <game/DbPackets.hpp>

#include <net/Util.hpp>

#include <storm/StringUtil.hpp>

#include <ctime>
#include <game/AuthPackets.hpp>

bool HandlePinResponsePacket(ClientSocket* client, const CaPinResponsePacket& p)
{
	if(client->GetState() != ClientSocket::kPin) {
	    SendAuthFailurePacket(client, "BESAMEKEY");
            return true;
	}

	if(client->GetSecurityCode() == 0) {
			
		GdAuthSetPinQueryPacket p2 = { client->GetAid(), p.code};
		client->GetServer().GetDbCacheSocket()->Send(HEADER_GD_AUTH_SET_PIN,
		                                             client->GetHandle(),
		                                             &p2, sizeof(p2));
		client->SetState(ClientSocket::kHGuard);
		SendHGuardRequestPacket(client);
		return true;
	}

	if (client->GetSecurityCode() == p.code) {
		client->SetState(ClientSocket::kHGuard);
		SendHGuardRequestPacket(client);
		return true;
	}

	SendAuthFailurePacket(client, "PIN");
	return true;
}

bool HandleRegisterPacket(ClientSocket* client, const CaRegisterPacket& p)
{
    SPDLOG_INFO("CLIENT: Received register: {0} {1} {2}", p.hwid, p.login,
                p.password);

    GdAuthRegisterQueryPacket p2;
    storm::CopyStringSafe(p2.login, p.login.c_str());
    storm::CopyStringSafe(p2.hwid, p.hwid.c_str());
    storm::CopyStringSafe(p2.password, p.password.c_str());
    storm::CopyStringSafe(p2.deleteCode, p.deleteCode.c_str());
    storm::CopyStringSafe(p2.mailAdress, p.mailAdress.c_str());

    client->GetServer().GetDbCacheSocket()->Send(
        HEADER_GD_AUTH_REGISTER, client->GetHandle(), &p2, sizeof(p2));
    return true;
}

bool HandleAuthPacket(ClientSocket* client, const CaAuthPacket& p)
{
	spdlog::trace("CLIENT: Received login: v{0} {1} {2}",
	           p.version, p.login, p.password);

	if (p.version != client->GetServer().GetVersion()) {
		SendAuthFailurePacket(client, "VERSION");
		return true;
	}

	// Don't allow a client to send a second auth packet
	if (!client->GetLogin().empty()) {
		SendAuthFailurePacket(client, "EINVAL");
		return true;
	}

	std::string login = p.login;

	std::transform(login.begin(), login.end(), login.begin(), ::tolower);

	client->SetLogin(login);

	// Don't allow a second login for the same account
	// using a different connection.
	if (!client->GetServer().InsertPendingLogin(client->GetLogin())) {
        SendAuthFailurePacket(client, "ALREADY");
		return true;
	}

	// Validate HWID string here so that we don't have to escape it everywhere.
	for (uint8_t ch : p.hwid) {
		if (!std::isalnum(ch)) {
			SendAuthFailurePacket(client, "EINVAL");
			return true;
		}
	}

	client->SetPassword(p.password);
	client->SetHwid(p.hwid);

	GdAuthLoginQueryPacket p2;
	storm::CopyStringSafe(p2.login, client->GetLogin().c_str());
	storm::CopyStringSafe(p2.hwid, p.hwid.c_str());
	client->GetServer().GetDbCacheSocket()->Send(HEADER_GD_AUTH_LOGIN,
	                                             client->GetHandle(),
	                                             &p2, sizeof(p2));
	return true;
}

bool HandleHGuardInfoPacket(ClientSocket* client, const CaHGuardInfoPacket& p)
{
	if(client->GetState() != ClientSocket::kHGuard) {
		return true;
	}
	
	const auto calcHwid = GetHWIDHash(p);

	GdHwidInfoQueryPacket p2 = {};
	p2.aid = client->GetAid();
	storm::CopyStringSafe(p2.hwid, calcHwid.c_str());
	client->GetServer().GetDbCacheSocket()->Send(HEADER_GD_HWID_INFO,
	                                             client->GetHandle(),
	                                             &p2, sizeof(p2));

	if(!client->IsUsingHGuard()) {
		SendLoginPacket(client->GetServer().GetMasterSocket().get(),
                client->GetHandle(), client->GetAid(), client->GetLogin(),
                client->GetSocialId(), client->GetPremiumTimes(),
                client->GetHwid());
		return true;
	}


	const auto hash = GetHGuardHash(p, client->GetAid());
	client->SetHGHash(hash);

	GdHGuardInfoQueryPacket p3 = {};
	p3.aid = client->GetAid();
	storm::CopyStringSafe(p3.cpuId, p.cpuId.c_str());
	storm::CopyStringSafe(p3.macAddr, p.macAddr.c_str());
	storm::CopyStringSafe(p3.guid, p.guid.c_str());
	storm::CopyStringSafe(p3.hwid, hash.c_str());
	client->GetServer().GetDbCacheSocket()->Send(HEADER_GD_HGUARD_INFO,
	                                             client->GetHandle(),
	                                             &p3, sizeof(p3));
	return true;
}

bool HandleHGuardCodePacket(ClientSocket* client, const CaHGuardCodePacket& p)
{
	if(client->GetState() != ClientSocket::kHGuard) {
		return true;

	}

	if(client->GetHGHash().empty()) {
		return true;
	}

	GdHGuardValidationQueryPacket p2 = {};
	p2.aid = client->GetAid();
	storm::CopyStringSafe(p2.hgHash, client->GetHGHash());
	storm::CopyStringSafe(p2.code, p.code);
	client->GetServer().GetDbCacheSocket()->Send(HEADER_GD_HGUARD_VERIFY,
	                                             client->GetHandle(),
	                                             &p2, sizeof(p2));
	return true;
}

