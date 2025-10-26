// vim:ts=4 sw=4
#include "ClientManager.h"
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "GuildManager.h"
#ifdef ENABLE_GUILD_STORAGE
#include "GuildStorage.h"
#include "GuildStorageManager.h"
#endif


void CClientManager::GuildCreate(CPeer * peer, uint32_t dwGuildID)
{
	SPDLOG_INFO( "GuildCreate {0}", dwGuildID);
	ForwardPacket(HEADER_DG_GUILD_LOAD, &dwGuildID, sizeof(uint32_t));

	CGuildManager::instance().Load(dwGuildID);
}

void CClientManager::GuildChangeGrade(CPeer* peer, TPacketGuild* p)
{
	SPDLOG_INFO( "GuildChangeGrade {0} {1}", p->dwGuild, p->dwInfo);
	ForwardPacket(HEADER_DG_GUILD_CHANGE_GRADE, p, sizeof(TPacketGuild));
}

void CClientManager::GuildAddMember(CPeer* peer, TPacketGDGuildAddMember * p)
{
	CGuildManager::instance().TouchGuild(p->dwGuild);
	SPDLOG_INFO( "GuildAddMember {0} {1}", p->dwGuild, p->dwPID);


	std::unique_ptr<SQLMsg> pmsg_insert(CDBManager::instance().DirectQuery(fmt::format("INSERT INTO guild_member VALUES({}, {}, {}, 0, 0)",
		p->dwPID, p->dwGuild, p->bGrade)));

	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("SELECT pid, grade, is_general, offer, level, job, name FROM guild_member, player WHERE guild_id = {} and pid = id and pid = {}",  p->dwGuild, p->dwPID)));

	if (pmsg->Get()->uiNumRows == 0) {
		spdlog::error( "Query failed when getting guild member data {0}", pmsg->stQuery.c_str());
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

	if (!row[0] || !row[1])
		return;

	TPacketDGGuildMember dg;

	dg.dwGuild = p->dwGuild;
	str_to_number(dg.dwPID, row[0]);
	str_to_number(dg.bGrade, row[1]);
	str_to_number(dg.isGeneral, row[2]);
	str_to_number(dg.dwOffer, row[3]);
	str_to_number(dg.bLevel, row[4]);
	str_to_number(dg.bJob, row[5]);
	strlcpy(dg.szName, row[6], sizeof(dg.szName));

	ForwardPacket(HEADER_DG_GUILD_ADD_MEMBER, &dg, sizeof(TPacketDGGuildMember));
}

void CClientManager::GuildRemoveMember(CPeer* peer, TPacketGuild* p)
{
	SPDLOG_INFO( "GuildRemoveMember {0} {1}", p->dwGuild, p->dwInfo);

	CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM guild_member WHERE pid={} and guild_id={}", p->dwInfo, p->dwGuild));
	CDBManager::instance().AsyncQuery(fmt::format("REPLACE INTO quest (dwPID, szName, szState, lValue) VALUES({}, 'guild_manage', 'withdraw_time', {})",  p->dwInfo, (uint32_t)GetCurrentTime()));

	ForwardPacket(HEADER_DG_GUILD_REMOVE_MEMBER, p, sizeof(TPacketGuild));
}

void CClientManager::GuildSkillUpdate(CPeer* peer, TPacketGuildSkillUpdate* p)
{
	SPDLOG_INFO( "GuildSkillUpdate {0}", p->amount);
	ForwardPacket(HEADER_DG_GUILD_SKILL_UPDATE, p, sizeof(TPacketGuildSkillUpdate));
}

void CClientManager::GuildExpUpdate(CPeer* peer, TPacketGuildExpUpdate* p)
{
	SPDLOG_INFO( "GuildExpUpdate {0}", p->amount);
	ForwardPacket(HEADER_DG_GUILD_EXP_UPDATE, p, sizeof(TPacketGuildExpUpdate), 0, peer);
}

void CClientManager::GuildChangeMemberData(CPeer* peer, TPacketGuildChangeMemberData* p)
{
	SPDLOG_INFO( "GuildChangeMemberData {0} {1} {2} {3}", p->pid, p->offer, p->level, p->grade);
	ForwardPacket(HEADER_DG_GUILD_CHANGE_MEMBER_DATA, p, sizeof(TPacketGuildChangeMemberData), 0, peer);
}

void CClientManager::GuildDisband(CPeer* peer, TPacketGuild* p)
{
	SPDLOG_INFO( "GuildDisband {0}", p->dwGuild);


	CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM guild WHERE id={}", p->dwGuild));
	CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM guild_grade WHERE guild_id={}", p->dwGuild));
	CDBManager::instance().AsyncQuery(fmt::format("REPLACE INTO quest (dwPID, szName, szState, lValue) SELECT pid, 'guild_manage', 'withdraw_time', {} FROM guild_member WHERE guild_id = {}",  (uint32_t)GetCurrentTime(),  p->dwGuild));
	CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM guild_member WHERE guild_id={}",  p->dwGuild));
	CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM guild_comment WHERE guild_id={}", p->dwGuild));

	ForwardPacket(HEADER_DG_GUILD_DISBAND, p, sizeof(TPacketGuild));
}

const char* __GetWarType(int n)
{
	switch (n)
	{
	case 0:
		return "Field";
	case 1:
		return "Arena";
	case 2:
		return "Flag";
	default:
		return "Unknown";
	}
}

void CClientManager::GuildWar(CPeer* peer, TPacketGuildWar* p)
{
	switch (p->bWar)
	{
	case GUILD_WAR_SEND_DECLARE:
		SPDLOG_INFO( "GuildWar: GUILD_WAR_SEND_DECLARE type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().AddDeclare(p->bType, p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_REFUSE:
		SPDLOG_INFO( "GuildWar: GUILD_WAR_REFUSE type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);
		break;
		/*
		   case GUILD_WAR_WAIT_START:
		   CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);

		   if (!CGuildManager::instance().WaitStart(p))
		   p->bWar = GUILD_WAR_CANCEL;

		   break;
		   */

	case GUILD_WAR_WAIT_START:
		SPDLOG_INFO( "GuildWar: GUILD_WAR_WAIT_START type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
	case GUILD_WAR_RESERVE:	// 길드전 예약
		if (p->bWar != GUILD_WAR_WAIT_START)
			SPDLOG_INFO( "GuildWar: GUILD_WAR_RESERVE type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);

		if (!CGuildManager::instance().ReserveWar(p))
			p->bWar = GUILD_WAR_CANCEL;
		else
			p->bWar = GUILD_WAR_RESERVE;

		break;

	case GUILD_WAR_ON_WAR:		// 길드전을 시작 시킨다. (필드전은 바로 시작 됨)
		SPDLOG_INFO( "GuildWar: GUILD_WAR_ON_WAR type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().RemoveDeclare(p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().StartWar(p->bType, p->dwGuildFrom, p->dwGuildTo);
		break;

	case GUILD_WAR_OVER:		// 길드전 정상 종료
		SPDLOG_INFO( "GuildWar: GUILD_WAR_OVER type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().RecvWarOver(p->dwGuildFrom, p->dwGuildTo, p->bType, p->isRanked);
		break;

	case GUILD_WAR_END:		// 길드전 비정상 종료
		SPDLOG_INFO( "GuildWar: GUILD_WAR_END type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().RecvWarEnd(p->dwGuildFrom, p->dwGuildTo);
		return; // NOTE: RecvWarEnd에서 패킷을 보내므로 따로 브로드캐스팅 하지 않는다.

	case GUILD_WAR_CANCEL:
		SPDLOG_INFO( "GuildWar: GUILD_WAR_CANCEL type({0}) guild({1} - {2})", __GetWarType(p->bType), p->dwGuildFrom, p->dwGuildTo);
		CGuildManager::instance().CancelWar(p->dwGuildFrom, p->dwGuildTo);
		break;
	}

	ForwardPacket(HEADER_DG_GUILD_WAR, p, sizeof(TPacketGuildWar));
}

void CClientManager::GuildWarScore(CPeer* peer, TPacketGuildWarScore * p)
{
	CGuildManager::instance().UpdateScore(p->dwGuildGainPoint, p->dwGuildOpponent, p->lScore, p->lBetScore);
}

void CClientManager::GuildChangeLadderPoint(TPacketGuildLadderPoint* p)
{
	SPDLOG_INFO( "GuildChangeLadderPoint Recv {0} {1}", p->dwGuild, p->lChange);
	CGuildManager::instance().ChangeLadderPoint(p->dwGuild, p->lChange);
}

void CClientManager::GuildUseSkill(TPacketGuildUseSkill* p)
{
	SPDLOG_INFO( "GuildUseSkill Recv {0} {1}", p->dwGuild, p->dwSkillVnum);
	CGuildManager::instance().UseSkill(p->dwGuild, p->dwSkillVnum, p->dwCooltime);
	SendGuildSkillUsable(p->dwGuild, p->dwSkillVnum, false);
}

void CClientManager::SendGuildSkillUsable(uint32_t guild_id, uint32_t dwSkillVnum, bool bUsable)
{
	SPDLOG_INFO( "SendGuildSkillUsable Send {0} {1} {2}", guild_id, dwSkillVnum, bUsable ? "true" : "false");

	TPacketGuildSkillUsableChange p;

	p.dwGuild = guild_id;
	p.dwSkillVnum = dwSkillVnum;
	p.bUsable = bUsable;

	ForwardPacket(HEADER_DG_GUILD_SKILL_USABLE_CHANGE, &p, sizeof(TPacketGuildSkillUsableChange));
}

void CClientManager::GuildChangeMaster(TPacketChangeGuildMaster* p)
{
	if (CGuildManager::instance().ChangeMaster(p->dwGuildID, p->idFrom, p->idTo) == true)
	{
		TPacketChangeGuildMaster packet;
		packet.dwGuildID = p->dwGuildID;
		packet.idFrom = 0;
		packet.idTo = 0;

		ForwardPacket(HEADER_DG_ACK_CHANGE_GUILD_MASTER, &packet, sizeof(packet));
	}
}

#ifdef ENABLE_GUILD_STORAGE
void CClientManager::GuildAddItem(TPacketGuildStorageAddItem* p, CPeer * peer)
{
	if (!peer)
		return;

	CGuildStorage * storage = CGuildStorageManager::instance().getStorage(p->guildID);	// Get guildstorage if exists
	if (storage) {
		storage->addItemByGame(peer, p->queItem.item, p->wPos, p->queID);
	}
}

void CClientManager::GuildRemoveItem(TPacketGuildStorageRemoveItemRequest* p, CPeer * peer)
{
	if (!peer)
		return;

	CGuildStorage * storage = CGuildStorageManager::instance().getStorage(p->guildID);	// Get guildstorage if exists
	if (storage) {
		storage->removeItemRequest(peer, p->queItem.item, p->wPos, p->queID);
	}
}

void CClientManager::GuildAddItemRemoveResponse(TPacketGuildStorageAddItem* p, CPeer * peer)
{
	if (!peer)
		return;

	CGuildStorage * storage = CGuildStorageManager::instance().getStorage(p->guildID);	// Get guildstorage if exists
	if (storage) {
		storage->addItemRemoveGame(peer, p->queItem.item, p->wPos, p->queID);
	}
}

void CClientManager::GuildMoveItem(TPacketGuildStorageMoveItem* p, CPeer * peer)
{
	if (!peer)
		return;

	CGuildStorage * storage = CGuildStorageManager::instance().getStorage(p->guildID);	// Get guildstorage if exists
	if (storage) {
		storage->moveItem(peer, p->dwSrcPos, p->dwDestPos, p->playerId);
	}
}

void CClientManager::GuildLoadItems(TPacketRequestGuildItems* p, CPeer * peer)
{
	if (!peer)
		return;

	CGuildStorage * storage = CGuildStorageManager::instance().getStorage(p->guildID);	// Get guildstorage if exists
	if (storage) {
		storage->sendItems(peer);	// Send items to peering
	}
}
#endif