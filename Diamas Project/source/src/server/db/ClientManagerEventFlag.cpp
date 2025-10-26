// vim:ts=4 sw=4
#include "ClientManager.h"
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include <storm/Util.hpp>

void CClientManager::LoadEventFlag()
{
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery("SELECT szName, lValue FROM quest WHERE dwPID = 0"));

	SQLResult* pRes = pmsg->Get();
	if (pRes->uiNumRows)
	{
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(pRes->pSQLResult)))
		{
			TPacketSetEventFlag p{};
			strlcpy(p.szFlagName, row[0], sizeof(p.szFlagName));
			str_to_number(p.lValue, row[1]);
			SPDLOG_INFO( "EventFlag Load {0} {1}", p.szFlagName, p.lValue);
			m_map_lEventFlag.insert(std::make_pair(std::string(p.szFlagName), p.lValue));
			ForwardPacket(HEADER_DG_SET_EVENT_FLAG, &p, sizeof(TPacketSetEventFlag));
		}
	}
}

void CClientManager::SetEventFlag(TPacketSetEventFlag* p)
{
	ForwardPacket(HEADER_DG_SET_EVENT_FLAG, p, sizeof(TPacketSetEventFlag));

	bool bChanged = false;

	auto it = m_map_lEventFlag.find(p->szFlagName);
	if (it == m_map_lEventFlag.end())
	{
		bChanged = true;
		m_map_lEventFlag.insert(std::make_pair(std::string(p->szFlagName), p->lValue));
	}
	else if (it->second != p->lValue)
	{
		bChanged = true;
		it->second = p->lValue;
	}

	if (bChanged)
	{
		std::string query; ;
		char flag[sizeof(p->szFlagName) * 4];
		p->szFlagName[STORM_ARRAYSIZE(p->szFlagName) - 1] = '\0';
		CDBManager::instance().EscapeString(flag, p->szFlagName, strlen(p->szFlagName));

		if (p->lValue != 0)
			query = fmt::format("REPLACE INTO quest (dwPID, szName, szState, lValue) VALUES(0, '{}', '', {})",
				flag, p->lValue);
		else
			query = fmt::format(
				"DELETE FROM quest WHERE dwPID = 0 AND szName = '{}'",
				flag);


		//CDBManager::instance().ReturnQuery(szQuery, QID_QUEST_SAVE, 0, NULL);
		CDBManager::instance().AsyncQuery(query);
		SPDLOG_INFO( "HEADER_GD_SET_EVENT_FLAG : Changed CClientmanager::SetEventFlag({} {}) ", p->szFlagName, p->lValue);
		return;
	}
	SPDLOG_INFO( "HEADER_GD_SET_EVENT_FLAG : No Changed CClientmanager::SetEventFlag({} {}) ", p->szFlagName, p->lValue);
}

void CClientManager::SendEventFlagsOnSetup(CPeer* peer)
{
	decltype(m_map_lEventFlag.begin()) it;
	for (it = m_map_lEventFlag.begin(); it != m_map_lEventFlag.end(); ++it)
	{
		TPacketSetEventFlag p;
		strlcpy(p.szFlagName, it->first.c_str(), sizeof(p.szFlagName));
		p.lValue = it->second;
		peer->EncodeHeader(HEADER_DG_SET_EVENT_FLAG, 0, sizeof(TPacketSetEventFlag));
		peer->Encode(&p, sizeof(TPacketSetEventFlag));
	}
}

