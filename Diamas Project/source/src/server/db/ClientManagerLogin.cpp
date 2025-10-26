

#include "ClientManager.h"

#include "Main.h"
#include "Config.h"
#include "QID.h"
#include "Cache.h"
#include <game/GamePacket.hpp>


void CreateAccountPlayerDataFromRes(MYSQL_RES * pRes, TAccountTable& pkTab)
{
	if (!pRes)
		return;

	for (uint32_t i = 0; i < mysql_num_rows(pRes); ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(pRes);
		int col = 0;

		uint32_t pid = 0;
		str_to_number(pid, row[col++]);

		uint32_t slot = 0;
		str_to_number(slot, row[col++]);

		if (pid == 0 || slot >= PLAYER_PER_ACCOUNT)
			continue;

		auto& player = pkTab.players[slot];
		player.id = pid;


		const auto pc = CClientManager::instance().GetPlayerCache().Get(pid);
		if (pc) {
			const auto& tab = pc->GetTable();
			strlcpy(player.name, tab.name, sizeof(player.name));
			player.empire = tab.empire;
			player.byJob = tab.job;
			player.byLevel = tab.level;
			player.dwPlayMinutes = tab.playtime;
			player.byST = tab.st;
			player.byHT = tab.ht;
			player.byDX = tab.dx;
			player.byIQ = tab.iq;
			std::memcpy(player.parts, tab.parts,
				sizeof(player.parts));
			player.mapIndex = tab.lMapIndex;
			player.x = tab.x;
			player.y = tab.y;
			player.skill_group = tab.skill_group;
			player.last_play = tab.last_play;
			str_to_number(player.bChangeName, row[27]);
			str_to_number(player.bChangeEmpire, row[28]);

			// Cached player is likely unlocked, so it's lifetime is
			// expiring. We don't want to free it if there's a good chance that
			// it might be needed again soon (i.e. account logs in again)
			pc->Touch();
		}
		else {
			if (!row[col++])
				*player.name = '\0';
			else
				strlcpy(player.name, row[col - 1], sizeof(player.name));

			player.byJob = 0;
			player.byLevel = 0;
			player.dwPlayMinutes = 0;
			player.byST = 0;
			player.byHT = 0;
			player.byDX = 0;
			player.byIQ = 0;
			std::memset(player.parts, 0,
				sizeof(player.parts));
			player.x = 0;
			player.y = 0;
			player.skill_group = 0;
			player.bChangeName = 0;

			str_to_number(player.empire, row[col++]);
			str_to_number(player.byJob, row[col++]);
			str_to_number(player.byLevel, row[col++]);
			str_to_number(player.dwPlayMinutes, row[col++]);
			str_to_number(player.byST, row[col++]);
			str_to_number(player.byHT, row[col++]);
			str_to_number(player.byDX, row[col++]);
			str_to_number(player.byIQ, row[col++]);

			str_to_number(player.parts[PART_MAIN].vnum, row[col++]);
			str_to_number(player.parts[PART_MAIN].appearance, row[col++]);
			str_to_number(player.parts[PART_MAIN].costume, row[col++]);
			str_to_number(player.parts[PART_MAIN].costume_appearance, row[col++]);

			str_to_number(player.parts[PART_HAIR].vnum, row[col++]);
			str_to_number(player.parts[PART_HAIR].appearance, row[col++]);
			str_to_number(player.parts[PART_HAIR].costume, row[col++]);
			str_to_number(player.parts[PART_HAIR].costume_appearance, row[col++]);

			str_to_number(player.parts[PART_ACCE].vnum, row[col++]);
			str_to_number(player.parts[PART_ACCE].appearance, row[col++]);
			str_to_number(player.parts[PART_ACCE].costume, row[col++]);
			str_to_number(player.parts[PART_ACCE].costume_appearance, row[col++]);

			str_to_number(player.mapIndex, row[col++]);
			str_to_number(player.x, row[col++]);
			str_to_number(player.y, row[col++]);
			str_to_number(player.skill_group, row[col++]);
			str_to_number(player.bChangeName, row[col++]);
			str_to_number(player.bChangeEmpire, row[col++]);
			str_to_number(player.last_play, row[col++]);
		}
	}
}


struct AuthLoginQueryInfo
{
	uint32_t handle;
	std::string login;
	std::string hwid;
};

void CClientManager::HandleAuthSetPinQuery(CPeer* peer, uint32_t handle,
                                          const GdAuthSetPinQueryPacket& p)
{
	CDBManager::instance().AsyncQuery(fmt::format("UPDATE account SET securitycode = {} WHERE id = {}", p.code, p.aid), SQL_ACCOUNT);
}


void CClientManager::HandleAuthLoginQuery(CPeer* peer, uint32_t handle,
                                          const GdAuthLoginQueryPacket& p)
{
	auto* info = new AuthLoginQueryInfo();
	info->handle = handle;
	info->login = p.login;
	info->hwid = p.hwid;

    const std::string query = fmt::format(
	         "SELECT `id`, `password`, `use_hwid_prot`, `securitycode`, `social_id`, `status`, availDt - NOW() > 0, "
	         "UNIX_TIMESTAMP(silver_expire), "
	         "UNIX_TIMESTAMP(gold_expire), "
	         "UNIX_TIMESTAMP(safebox_expire), "
	         "UNIX_TIMESTAMP(autoloot_expire), "
	         "UNIX_TIMESTAMP(fish_mind_expire), "
	         "UNIX_TIMESTAMP(marriage_fast_expire), "
	         "UNIX_TIMESTAMP(money_drop_rate_expire), "
             "UNIX_TIMESTAMP(vote_special_expire), "
	         "UNIX_TIMESTAMP(create_time) "
	         "FROM `account` WHERE `login`='{}';",
	          p.login, p.hwid);

	CDBManager::instance().ReturnQuery(query, QID_AUTH_LOGIN,
	                                   peer->GetHandle(), info,
	                                   SQL_ACCOUNT);
}


struct AuthRegisterQueryInfo
{
	uint32_t handle;
	GdAuthRegisterQueryPacket data;
};

void CClientManager::HandleAuthRegisterQuery(CPeer* peer, const uint32_t handle,
                                             const GdAuthRegisterQueryPacket& p)
{
    auto* info = new AuthRegisterQueryInfo();
    info->handle = handle;
    info->data = p;

    const std::string query =
        fmt::format("SELECT id FROM account WHERE login='{}';", p.login);
    CDBManager::instance().ReturnQuery(query, QID_AUTH_REGISTER_NAME_CHECK,
                                       peer->GetHandle(), info, SQL_ACCOUNT);
}


struct HGuardInfoQueryInfo
{
	uint32_t handle{};
	uint32_t aid{};
	std::string hwid;
	std::string macAddr;
	std::string guid;
	std::string cpuId;
};


void CClientManager::HandleHGuardInfoQuery(CPeer* peer, uint32_t handle, const GdHGuardInfoQueryPacket& p)
{
	auto info = new HGuardInfoQueryInfo();
	info->handle = handle;
	info->aid = p.aid;
	info->hwid = p.hwid;
	info->macAddr = p.macAddr;
	info->guid = p.guid;
	info->cpuId = p.cpuId;

	CDBManager::instance().ReturnQuery(fmt::format("SELECT validation_time IS NOT NULL, DATE_SUB(NOW(), INTERVAL 10 MINUTE) < creation_time as was_in_last_10_minutes FROM hguard WHERE "
	    "creation_time != 0 AND hg_hash = '{}' AND account_id = {} ORDER BY "
	    "validation_time DESC LIMIT 1",
	    p.hwid, p.aid),
	    QID_GET_HGUARD_INFO, peer->GetHandle(), info, SQL_ACCOUNT);
}

void CClientManager::HandleHwidInfoQuery(CPeer* peer, uint32_t dwHandle, const GdHwidInfoQueryPacket& data)
{
	CDBManager::instance().AsyncQuery(fmt::format("UPDATE account SET hwid = '{}' WHERE id = {}", data.hwid, data.aid), SQL_ACCOUNT);
}

struct HGuardVerifyQueryInfo
{
	uint32_t handle;
	uint32_t aid;
};

void CClientManager::HandleHGuardVerifyQuery(CPeer* peer, uint32_t handle, const GdHGuardValidationQueryPacket& data)
{
	auto info = new HGuardVerifyQueryInfo;
	info->aid = data.aid;
	info->handle = handle;

	CDBManager::instance().ReturnQuery(fmt::format( "UPDATE hguard SET validation_time = NOW(), activation_code = '' WHERE "
	    "hg_hash = '{}' AND account_id = '{}' AND activation_code = '{}'",
	    data.hgHash, data.aid, data.code),
	    QID_VALIDATE_HGUARD_CODE, peer->GetHandle(), info, SQL_ACCOUNT);

}

std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        std::string charset ="0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		return *Random::get(charset);
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

void CClientManager::HandleHGuardInfoResult(CPeer* peer, SQLMsg* msg)
{
	auto qi = static_cast<CQueryInfo*>(msg->pvUserData);
	std::unique_ptr<HGuardInfoQueryInfo> info(
	    static_cast<HGuardInfoQueryInfo*>(qi->pvData));


	// Already has an entry on db
	if (msg->Get()->uiNumRows > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(msg->Get()->pSQLResult);

		uint8_t isValid = 0;
		str_to_number(isValid, row[0]);

		uint8_t wasInLast10Minutes = 0;
		str_to_number(wasInLast10Minutes, row[1]);

		SPDLOG_DEBUG("[HGuard] Hash found, status: ({0})", isValid);

		if (isValid == 1) // All ok, we can proceed
		{
			// Success
			DgHGuardResult p { DgHGuardResult::kHGuardOk };
			peer->EncodeHeader(HEADER_DG_HGUARD_RESULT, info->handle, sizeof(p));
			peer->Encode(&p, sizeof(p));
			return;
		}

		// If the code was sent in the last 10 minutes allow the old code
		// Notify the client that it should switch to the special
		// screen for entering the code
		if (wasInLast10Minutes)
		{
			// REQUIRE_CODE
			DgHGuardResult p { DgHGuardResult::kHGuardCheck };
			peer->EncodeHeader(HEADER_DG_HGUARD_RESULT, info->handle, sizeof(p));
			peer->Encode(&p, sizeof(p));
			return;
		}
		// Otherwise, player got the code but didn't validate it.
		// We'll assume he could not validate it.  Proceed.
	}

	// No entry DB/no previously entered code.
	// 1st: New code
	std::string code = random_string(6);

	// 2nd: Replace the old code or simply insert the new code
	//TPacketCGHGuardSend* hData = (TPacketCGHGuardSend*)qi->pvData;
	std::string hash = info->hwid;
	CDBManager::instance().AsyncQuery(fmt::format("REPLACE INTO hguard (hg_hash, account_id, "
	      "activation_code, creation_time, mac, cpu_id, "
	      "guid) VALUES('{}', {}, "
		    "'{}', NOW(), '{}', '{}',"
	      " '{}')",
	      hash.c_str(), info->aid,
		code, info->macAddr, info->cpuId,
		info->guid), SQL_ACCOUNT);

	CDBManager::instance().AsyncQuery(fmt::format("REPLACE INTO hguard_mails(hash, account_id, sent) VALUES('{}', '{}', 0);", info->hwid, info->aid), SQL_ACCOUNT);

	// 4. Notify the client that it should switch to the special
	// screen for entering the code
		// HEADER_DG_HGUARD_RESULT
		// REQUIRE_CODE
	DgHGuardResult p { DgHGuardResult::kHGuardCheck };
	peer->EncodeHeader(HEADER_DG_HGUARD_RESULT, info->handle, sizeof(p));
	peer->Encode(&p, sizeof(p));
}

void CClientManager::HandleHGuardVerifyResult(CPeer* peer, SQLMsg* msg)
{
	const auto qi = static_cast<CQueryInfo*>(msg->pvUserData);
	const std::unique_ptr<HGuardVerifyQueryInfo> info(
		static_cast<HGuardVerifyQueryInfo*>(qi->pvData));


	if(msg->Get()->uiAffectedRows > 0) {
		DgHGuardResult p{DgHGuardResult::kHGuardOk};
		peer->EncodeHeader(HEADER_DG_HGUARD_RESULT, info->handle, sizeof(p));
		peer->Encode(&p, sizeof(p));
		return;
	}

	DgHGuardResult p{DgHGuardResult::kHGuardIncorrect};
	peer->EncodeHeader(HEADER_DG_HGUARD_RESULT, info->handle, sizeof(p));
	peer->Encode(&p, sizeof(p));
}

void CClientManager::HandleAuthLoginResult(CPeer* peer, SQLMsg* msg)
{
	auto* qi = static_cast<CQueryInfo*>(msg->pvUserData);
    const std::unique_ptr<AuthLoginQueryInfo> info(
	    static_cast<AuthLoginQueryInfo*>(qi->pvData));

	DgAuthLoginResultPacket p = {};

	const std::unique_ptr<SQLMsg> hwidMsg(CDBManager::instance().DirectQuery(
        fmt::format("SELECT hwid FROM hwid_ban WHERE hwid = '{}' ", info->hwid),
        SQL_ACCOUNT));

    if (hwidMsg->Get()->uiNumRows != 0) {
        storm::CopyStringSafe(p.status, "HWIDB");
    }

	else if (msg->Get()->uiNumRows != 0) {
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		int col = 0;
		str_to_number(p.id, row[col++]);
		storm::CopyStringSafe(p.password, row[col++]);
		str_to_number(p.useHwidProt, row[col++]);
		str_to_number(p.securityCode, row[col++]);
		storm::CopyStringSafe(p.socialId, row[col++]);
		storm::CopyStringSafe(p.status, row[col++]);

		int notAvail = 0;
		str_to_number(notAvail, row[col++]);
		if (0 != notAvail)
			storm::CopyStringSafe(p.status, "NOTAVAIL");


		str_to_number(p.premiumTimes[PREMIUM_EXP], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_ITEM], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_SAFEBOX], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_AUTOLOOT], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_FISH_MIND], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_MARRIAGE_FAST], row[col++]);
		str_to_number(p.premiumTimes[PREMIUM_GOLD], row[col++]);
	    str_to_number(p.premiumTimes[PREMIUM_VOTE], row[col++]);
		str_to_number(p.createTime, row[col++]);
	    storm::CopyStringSafe(p.hwidHash, info->hwid);

	} else {
		storm::CopyStringSafe(p.status, "NOID");
	}


	peer->EncodeHeader(HEADER_DA_AUTH_LOGIN, info->handle, sizeof(p));
	peer->Encode(&p, sizeof(p));
}

void CClientManager::HandleAuthRegisterNameCheckResult(CPeer* peer, SQLMsg* msg)
{
    auto* qi = static_cast<CQueryInfo*>(msg->pvUserData);
    std::unique_ptr<AuthRegisterQueryInfo> info(
        static_cast<AuthRegisterQueryInfo*>(qi->pvData));

	if (msg->Get()->uiNumRows != 0) {
        const uint8_t status = 0; // Kayýt mevcut
        peer->EncodeHeader(HEADER_DA_AUTH_REGISTER, info->handle,
                           sizeof(status));
        peer->EncodeBYTE(status);
    } else {
        const std::string query = fmt::format(
            "INSERT INTO {}.account SET "
            "login = '{}', "
            "password = PASSWORD('{}'), "
            "hwid = '{}', "
            "email = '{}', "
            "social_id = '{}';",
            m_config.accountDb.name, info->data.login, info->data.password,
            info->data.hwid, info->data.mailAdress, info->data.deleteCode);

        CDBManager::instance().ReturnQuery(query, QID_AUTH_REGISTER,
                                           peer->GetHandle(), info.release(),
                                           SQL_ACCOUNT);
    }
}

void CClientManager::HandleAuthRegister(CPeer* peer, SQLMsg* msg)
{
    const auto* qi = static_cast<CQueryInfo*>(msg->pvUserData);
    const std::unique_ptr<AuthRegisterQueryInfo> info(
        static_cast<AuthRegisterQueryInfo*>(qi->pvData));

    if (msg->Get()->uiAffectedRows != 0) {
        const uint8_t status = 1; // kayýt baþarýlý
        peer->EncodeHeader(HEADER_DA_AUTH_REGISTER, info->handle,
                           sizeof(status));
        peer->EncodeBYTE(status);
    } else {
        const uint8_t status = 2;
        peer->EncodeHeader(HEADER_DA_AUTH_REGISTER, info->handle,
                           sizeof(status));
        peer->EncodeBYTE(status);
    }
}

struct CharacterListQueryInfo
{
	uint32_t handle;
	TAccountTable tab;
};

void CClientManager::HandlePlayerListQuery(CPeer* peer, uint32_t handle,
                                           const GdPlayerListQueryPacket& p)
{
	auto info = new CharacterListQueryInfo();
	info->handle = handle;
	info->tab.id = p.aid;


	const auto query = fmt::format("SELECT id, slot, name, empire, job, level, playtime, st, ht, dx, iq, "
		"part_main, part_main_appearance, part_main_costume, part_main_costume_appearance, "
		"part_hair, part_hair_appearance, part_hair_costume, part_hair_costume_appearance, "
		"part_acce, part_acce_appearance, part_acce_costume, part_acce_costume_appearance, "
		"map_index, x, y, skill_group, change_name, change_empire, UNIX_TIMESTAMP(last_play) FROM player WHERE account_id={} AND deleted = 0",
		info->tab.id);

	//char query[QUERY_MAX_LEN];
	//snprintf(query, sizeof(query),
	//         "SELECT id, slot, name, empire, job, level, playtime, st, ht, dx, iq, "
	//         "part_main, part_main_appearance, part_hair, part_hair_appearance, "
	//         "part_weapon, part_weapon_appearance, "
	//         "map_index, x, y, skill_group, change_name, change_empire "
	//         "FROM player%s WHERE account_id=%u",
	//         GetTablePostfix(), info->tab.id);

	CDBManager::instance().ReturnQuery(query, QID_PLAYER_LIST, peer->GetHandle(), info);
}


void CClientManager::HandlePlayerListResult(CPeer* peer, SQLMsg* msg)
{
	auto qi = static_cast<CQueryInfo*>(msg->pvUserData);
	std::unique_ptr<CharacterListQueryInfo> info(
	    static_cast<CharacterListQueryInfo*>(qi->pvData));

	DgPlayerListResultPacket p = {};

	if (msg->uiSQLErrno != 0)
		p.failed = true;

	if (!msg->Get()->pSQLResult)
		p.failed = true;

	if (!p.failed) {
		CreateAccountPlayerDataFromRes(msg->Get()->pSQLResult, info->tab);
		p.tab = info->tab;
	}

	peer->EncodeHeader(HEADER_DG_PLAYER_LIST_RESULT, info->handle, sizeof(p));
	peer->Encode(&p, sizeof(p));
}

void CClientManager::HandlePlayerDisconnect(CPeer* peer,
                                            const GdPlayerDisconnectPacket& p)
{
	auto c = m_playerCache.Get(p.pid);
	if (c) {
		if (!c->IsLocked())
			spdlog::error( "Player {0}:{1} was playing, but wasn't locked",
			          p.aid, p.pid);

		// Update access time since we're about to lose delete protection
		// (i.e. locked state)
		c->Touch();
		c->Unlock();
	} else {
		spdlog::error( "Disconnected player isn't in cache: {0}:{1}",
		          p.aid, p.pid);
	}
}

void CClientManager::HandleChangeName(CPeer * peer, uint32_t handle, GdChangeNamePacket* p)
{

	std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(fmt::format("SELECT id FROM player WHERE name = '{}' AND deleted = 0", p->name), SQL_PLAYER));
	if(msg->Get()->uiNumRows > 0) {
		DgChangeNamePacket p2;
		p2.failed = true;
		p2.pid = p->pid;
		strlcpy(p2.name, p->name, sizeof(p2.name));
		peer->EncodeHeader(HEADER_DG_CHANGE_NAME, handle, sizeof(p2));
		peer->Encode(&p2, sizeof(p2));
		return;
	}

	char queryStr[QUERY_MAX_LEN];
	snprintf(queryStr, sizeof(queryStr),
	         "UPDATE player SET name='%s', change_name=0 WHERE id=%u AND deleted = 0",
	          p->name, p->pid);
	std::unique_ptr<SQLMsg> msg2(CDBManager::instance().DirectQuery(queryStr, SQL_PLAYER));

	DgChangeNamePacket p2;
	if (msg2->uiSQLErrno != 0)
		p2.failed = true;

	auto* cachedPlayer = m_playerCache.Get(p->pid);
	if (!p2.failed && cachedPlayer) {
		auto tab = cachedPlayer->GetTable();
		storm::CopyStringSafe(tab.name, p->name);
		cachedPlayer->SetTable(tab, false);
	}

	p2.pid = p->pid;
	strlcpy(p2.name, p->name, sizeof(p2.name));
	peer->EncodeHeader(HEADER_DG_CHANGE_NAME, handle, sizeof(p2));
	peer->Encode(&p2, sizeof(p2));
}

void CClientManager::HandleChangeEmpire(CPeer * peer, uint32_t handle, GdChangeEmpirePacket* p)
{
	char queryStr[QUERY_MAX_LEN];
	snprintf(queryStr, sizeof(queryStr),
	         "UPDATE player SET empire=%u, change_empire=0 WHERE id=%u",
	          p->empire, p->pid);
	std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(queryStr, SQL_PLAYER));

	DgChangeEmpirePacket p2 = {};
	if (msg->uiSQLErrno != 0)
		p2.failed = true;

	auto cachedPlayer = m_playerCache.Get(p->pid);
	if (!p2.failed && cachedPlayer) {
		auto tab = cachedPlayer->GetTable();
		tab.empire = p->empire;
		cachedPlayer->SetTable(tab, false);
	}

	p2.pid = p->pid;
	p2.empire = p->empire;
	peer->EncodeHeader(HEADER_DG_CHANGE_EMPIRE, handle, sizeof(p2));
	peer->Encode(&p2, sizeof(p2));
}

