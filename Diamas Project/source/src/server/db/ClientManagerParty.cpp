// vim:ts=4 sw=4
#include "stdafx.h"
#include "ClientManager.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"

void CClientManager::QUERY_PARTY_CREATE(CPeer* peer, TPacketPartyCreate* p)
{
	auto& pm = m_map_pkPartyMap;

	if (pm.find(p->dwLeaderPID) == pm.end())
	{
		pm.insert(make_pair(p->dwLeaderPID, TPartyMember()));
		ForwardPacket(HEADER_DG_PARTY_CREATE, p, sizeof(TPacketPartyCreate), 0, peer);
		SPDLOG_INFO( "PARTY Create [{0}]", p->dwLeaderPID);
	}
	else
	{
		spdlog::error( "PARTY Create - Already exists [{0}]", p->dwLeaderPID);
	}
}

void CClientManager::QUERY_PARTY_DELETE(CPeer* peer, TPacketPartyDelete* p)
{
	auto& pm = m_map_pkPartyMap;
	auto it = pm.find(p->dwLeaderPID);

	if (it == pm.end())
	{
		spdlog::error( "PARTY Delete - Non exists [{0}]", p->dwLeaderPID);
		return;
	}

	pm.erase(it);
	ForwardPacket(HEADER_DG_PARTY_DELETE, p, sizeof(TPacketPartyDelete), 0, peer);
	SPDLOG_INFO( "PARTY Delete [{0}]", p->dwLeaderPID);
}

void CClientManager::QUERY_PARTY_ADD(CPeer* peer, TPacketPartyAdd* p)
{
	auto& pm = m_map_pkPartyMap;
	auto it = pm.find(p->dwLeaderPID);

	if (it == pm.end())
	{
		spdlog::error( "PARTY Add - Non exists [{0}]", p->dwLeaderPID);
		return;
	}

	if (it->second.find(p->dwPID) == it->second.end())
	{
		it->second.insert(std::make_pair(p->dwPID, TPartyInfo()));
		ForwardPacket(HEADER_DG_PARTY_ADD, p, sizeof(TPacketPartyAdd), 0, peer);
		SPDLOG_INFO( "PARTY Add [{0}] to [{1}]", p->dwPID, p->dwLeaderPID);
	}
	else
		spdlog::error( "PARTY Add - Already [{0}] in party [{1}]", p->dwPID, p->dwLeaderPID);
}

void CClientManager::QUERY_PARTY_REMOVE(CPeer* peer, TPacketPartyRemove* p)
{
	auto& pm = m_map_pkPartyMap;
	auto it = pm.find(p->dwLeaderPID);

	if (it == pm.end())
	{
		spdlog::error( "PARTY Remove - Non exists [{0}] cannot remove [{1}]", p->dwLeaderPID, p->dwPID);
		return;
	}

	auto pit = it->second.find(p->dwPID);

	if (pit != it->second.end())
	{
		it->second.erase(pit);
		ForwardPacket(HEADER_DG_PARTY_REMOVE, p, sizeof(TPacketPartyRemove), 0, peer);
		SPDLOG_INFO( "PARTY Remove [{0}] to [{1}]", p->dwPID, p->dwLeaderPID);
	}
	else
		spdlog::error( "PARTY Remove - Cannot find [{0}] in party [{1}]", p->dwPID, p->dwLeaderPID);
}

void CClientManager::QUERY_PARTY_STATE_CHANGE(CPeer* peer, TPacketPartyStateChange* p)
{
	auto& pm = m_map_pkPartyMap;
	auto it = pm.find(p->dwLeaderPID);

	if (it == pm.end())
	{
		spdlog::error( "PARTY StateChange - Non exists [{0}] cannot state change [{1}]", p->dwLeaderPID, p->dwPID);
		return;
	}

	auto pit = it->second.find(p->dwPID);

	if (pit == it->second.end())
	{
		spdlog::error( "PARTY StateChange - Cannot find [{0}] in party [{1}]", p->dwPID, p->dwLeaderPID);
		return;
	}

	if (p->bFlag)
		pit->second.bRole = p->bRole;
	else
		pit->second.bRole = 0;

	ForwardPacket(HEADER_DG_PARTY_STATE_CHANGE, p, sizeof(TPacketPartyStateChange), 0, peer);
	SPDLOG_INFO( "PARTY StateChange [{0}] at [{1}] from {2} {3}", p->dwPID, p->dwLeaderPID, p->bRole, p->bFlag);
}

void CClientManager::QUERY_PARTY_SET_MEMBER_LEVEL(CPeer* peer, TPacketPartySetMemberLevel* p)
{
	auto& pm = m_map_pkPartyMap;
	auto it = pm.find(p->dwLeaderPID);

	if (it == pm.end())
	{
		spdlog::error( "PARTY SetMemberLevel - Non exists [{0}] cannot level change [{1}]", p->dwLeaderPID, p->dwPID);
		return;
	}

	auto pit = it->second.find(p->dwPID);

	if (pit == it->second.end())
	{
		spdlog::error( "PARTY SetMemberLevel - Cannot find [{0}] in party [{1}]", p->dwPID, p->dwLeaderPID);
		return;
	}

	pit->second.bLevel = p->bLevel;

	ForwardPacket(HEADER_DG_PARTY_SET_MEMBER_LEVEL, p, sizeof(TPacketPartySetMemberLevel), 0);
	SPDLOG_INFO( "PARTY SetMemberLevel pid [{0}] level {1}", p->dwPID, p->bLevel);
}
