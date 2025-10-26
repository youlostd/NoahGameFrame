#include "ClientManager.h"
#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include <storm/Util.hpp>

void CClientManager::LoadHwidFlag()
{
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery("SELECT hwid, name, value FROM hwid_flag"));

	SQLResult* pRes = pmsg->Get();
	if (pRes->uiNumRows)
	{
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(pRes->pSQLResult)))
		{
			TPacketSetHwidFlag p;
			strlcpy(p.szHwid, row[0], sizeof(p.szHwid));
			strlcpy(p.szFlagName, row[1], sizeof(p.szFlagName));
			str_to_number(p.lValue, row[2]);
			SPDLOG_INFO( "HwidFlag Load %s %d", p.szFlagName, p.lValue);
			m_map_lHwidFlag.insert(std::make_pair(std::string(p.szFlagName), std::make_pair(std::string(p.szHwid), p.lValue)));
			ForwardPacket(HEADER_DG_SET_HWID_FLAG, &p, sizeof(TPacketSetHwidFlag));
		}
	}
}

void CClientManager::SetHwidFlag(TPacketSetHwidFlag* p)
{
	ForwardPacket(HEADER_DG_SET_EVENT_FLAG, p, sizeof(TPacketSetHwidFlag));

	
	
	bool bChanged = false;

	using const_ref = std::multimap<std::string, THwidDataPair>::const_reference;
	auto it = std::find_if(m_map_lHwidFlag.begin(), m_map_lHwidFlag.end(),
		[&](const_ref a) -> bool {
		return a.first == std::string(p->szFlagName) && a.second.first == std::string(p->szHwid);
	});


	if (it == m_map_lHwidFlag.end())
	{
		bChanged = true;
		m_map_lHwidFlag.insert(std::make_pair(std::string(p->szFlagName), std::make_pair(std::string(p->szHwid), p->lValue)));
	}
	else if (it->second.second != p->lValue)
	{
		bChanged = true;
		it->second.second = p->lValue;
	}

	if (bChanged)
	{
		std::string query;
		char flag[sizeof(p->szFlagName) * 4];
		p->szFlagName[STORM_ARRAYSIZE(p->szFlagName) - 1] = '\0';
		CDBManager::instance().EscapeString(flag, p->szFlagName, strlen(p->szFlagName));

		char hwid[sizeof(p->szHwid) * 4];
		p->szHwid[STORM_ARRAYSIZE(p->szHwid) - 1] = '\0';
		CDBManager::instance().EscapeString(hwid, p->szHwid, strlen(p->szHwid));

		if (p->lValue != 0)
			query = fmt::format(
				"REPLACE INTO hwid_flag (hwid, name, value) VALUES('{}', '{}', {})",
					hwid, flag, p->lValue);
		else
			query =  fmt::format(
				"DELETE FROM hwid_flag WHERE hwid = '{}' AND name = '{}'",
				hwid,flag);

		//CDBManager::instance().ReturnQuery(szQuery, QID_QUEST_SAVE, 0, NULL);
		CDBManager::instance().AsyncQuery(query);
		SPDLOG_INFO( "HEADER_GD_SET_HWID_FLAG : Changed CClientmanager::SetHwidFlag({} {} {}) ", p->szHwid, p->szFlagName, p->lValue);
		return;
	}
	SPDLOG_INFO( "HEADER_GD_SET_HWID_FLAG : No Changed CClientmanager::SetHwidFlag({} {} {}) ", p->szHwid, p->szFlagName, p->lValue);
}

void CClientManager::SendHwidFlagsOnSetup(CPeer* peer)
{
	for (auto it = m_map_lHwidFlag.begin(); it != m_map_lHwidFlag.end(); ++it)
	{
		TPacketSetHwidFlag p;
		strlcpy(p.szHwid, it->second.first.c_str(), sizeof(p.szHwid));
		strlcpy(p.szFlagName, it->first.c_str(), sizeof(p.szFlagName));
		p.lValue = it->second.second;
		peer->EncodeHeader(HEADER_DG_SET_EVENT_FLAG, 0, sizeof(TPacketSetHwidFlag));
		peer->Encode(&p, sizeof(TPacketSetHwidFlag));
	}
}

