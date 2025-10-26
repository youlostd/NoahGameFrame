#include "DbCachePackets.hpp"
#include "DbCacheSocket.hpp"
#include "MySqlPassword.hpp"
#include "ClientSocket.hpp"
#include "ClientUtil.hpp"
#include "MasterUtil.hpp"
#include "Server.hpp"
#include <game/DbPackets.hpp>
#include <thecore/sha1.h>
#include <boost/algorithm/string.hpp>
#include "game/AuthPackets.hpp"


void HandleHGuardResult(DbCacheSocket* dbCache, uint32_t handle,
                           const DgHGuardResult& p)
{
	SPDLOG_TRACE("DBCACHE: Received hguard result: {0} handle {1}",
	           p.code, handle);
	
	const auto client = dbCache->GetServer().GetByHandle(handle);
	if (!client)
		return;


	switch(p.code) {
	case DgHGuardResult::kHGuardOk:
		{
			SendLoginPacket(client->GetServer().GetMasterSocket().get(),
	                client->GetHandle(), client->GetAid(), client->GetLogin(),
	                client->GetSocialId(), client->GetPremiumTimes(),
	                client->GetHwid());
		}
		break;
	case DgHGuardResult::kHGuardCheck:
		{
			SendHGuardResultPacket(client.get(), AcHGuardResultPacket::kRequireCode);
		}
		break;
	case DgHGuardResult::kHGuardIncorrect:
		{
			SendHGuardResultPacket(client.get(), AcHGuardResultPacket::kIncorrectCode);
		}
		break;
        default:
            SendHGuardResultPacket(client.get(), AcHGuardResultPacket::kIncorrectCode);
			break;
    }

}

void HandleAuthRegisterResult(DbCacheSocket* dbCache, const uint32_t handle,
                              const uint8_t& status)
{
    const auto client = dbCache->GetServer().GetByHandle(handle);
    if (!client)
        return;

    if (status == 0)
        SendAuthFailurePacket(client.get(), "REG0");
    else if (status == 1)
        SendAuthFailurePacket(client.get(), "REG1");
    else
        SendAuthFailurePacket(client.get(), "REG2");
}

void HandleAuthLoginResult(DbCacheSocket* dbCache, uint32_t handle,
                           const DgAuthLoginResultPacket& p)
{
	SPDLOG_TRACE("DBCACHE: Received auth result: {0} handle {1}",
	           p.id, handle);

	const auto client = dbCache->GetServer().GetByHandle(handle);
	if (!client)
		return;

	if (p.id == 0) {
		SendAuthFailurePacket(client.get(), p.status);
		return;
	}

	char scrambledPassword[128];
	my_make_scrambled_password_sha1(scrambledPassword,
	                                client->GetPassword().c_str(),
	                                client->GetPassword().size());

	if (0 != std::strcmp(scrambledPassword, p.password)) {
		SendAuthFailurePacket(client.get(), "NOID");
		return;
	}

	if (0 != std::strcmp("OK", p.status)) {
		std::string status = p.status;
		SendAuthFailurePacket(client.get(), status);
		return;
	}

	// Needed for delayed AmLoginPacket
	client->SetAid(p.id);
	client->SetSocialId(p.socialId);
	client->SetPremiumTimes(p.premiumTimes);
	client->SetSecurityCode(p.securityCode);
	client->SetHGuardStatus(p.useHwidProt);
	client->SetHwid(p.hwidHash);
	client->SetState(ClientSocket::kPin);
	
	if (client->GetSecurityCode() == 0) {
		SendSetPinRequestPacket(client.get());
	} else {
		SendPinRequestPacket(client.get());
	}

}

void HandleDbPacket(DbCacheSocket* dbCache,
                    const DbPacketHeader& header,
                    const asio::const_buffer& data)
{
	switch (header.id) {
		case HEADER_DA_AUTH_LOGIN: {
			HandleAuthLoginResult(dbCache, header.handle,
			                      *asio::buffer_cast<const DgAuthLoginResultPacket*>(data));
			break;
		}
		case HEADER_DG_HGUARD_RESULT: {
			HandleHGuardResult(dbCache, header.handle,
			                      *asio::buffer_cast<const DgHGuardResult*>(data));
			break;
		}

		case HEADER_DA_AUTH_REGISTER: {
			HandleAuthRegisterResult(dbCache, header.handle,
			                      *asio::buffer_cast<const uint8_t*>(data));

			break;
		}
		

		default:
			break;
	}
}


