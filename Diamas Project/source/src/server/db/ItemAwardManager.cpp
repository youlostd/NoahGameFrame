#include "QID.h"
#include "DBManager.h"
#include "ItemAwardManager.h"
#include "Peer.h"

#include "ClientManager.h"
#include <fmt/format.h>


ItemAwardManager::ItemAwardManager(asio::io_service& ioService)
	: m_reloadTimer(ioService)
{
	// ctor
}

void ItemAwardManager::Quit()
{
	m_reloadTimer.cancel();
}

uint32_t g_dwLastCachedItemAwardID = 0;

void ItemAwardManager::RequestLoad() const
{
	CDBManager::instance().ReturnQuery(
		fmt::format(
			"SELECT id,login,vnum,count,socket0,socket1,socket2,"
			"attrtype0, attrvalue0,"
			"attrtype1, attrvalue1,"
			"attrtype2, attrvalue2,"
			"attrtype3, attrvalue3,"
			"attrtype4, attrvalue4,"
			"attrtype5, attrvalue5,"
			"attrtype6, attrvalue6,"
			"mall, why "
			"FROM item_award WHERE taken_time IS NULL and id > {}",
			g_dwLastCachedItemAwardID), QID_ITEM_AWARD_LOAD, 0, nullptr);
}

void ItemAwardManager::Load(SQLMsg* pMsg)
{
	MYSQL_RES* pRes = pMsg->Get()->pSQLResult;

	for (uint i = 0; i < pMsg->Get()->uiNumRows; ++i)
	{
		const auto row = mysql_fetch_row(pRes);
		int col = 0;

		uint32_t dwID = 0;
		str_to_number(dwID, row[col++]);

		if (m_map_award.find(dwID) != m_map_award.end())
			continue;

		auto kData = new TItemAward;
		kData->dwID = dwID;

		trim_and_lower(row[col++], kData->szLogin, sizeof(kData->szLogin));
		str_to_number(kData->data.vnum, row[col++]);
		str_to_number(kData->data.count, row[col++]);
		str_to_number(kData->data.sockets[0], row[col++]);
		str_to_number(kData->data.sockets[1], row[col++]);
		str_to_number(kData->data.sockets[2], row[col++]);
		for (auto& j : kData->data.attrs)
		{
			str_to_number(j.bType, row[col++]);
			str_to_number(j.sValue, row[col++]);
		}
		str_to_number(kData->bMall, row[col++]);

		m_map_award.emplace(dwID, kData);

		SPDLOG_INFO( "ITEM_AWARD: load id {0} login {1} vnum {2} count {3} socket {4}", kData->dwID, kData->szLogin,
			kData->data.vnum, kData->data, kData->data.sockets[0]);
		auto& kSet = m_map_kSetAwardByLogin[kData->szLogin];
		kSet.insert(kData);

		if (dwID > g_dwLastCachedItemAwardID)
			g_dwLastCachedItemAwardID = dwID;
	}
}

std::set<TItemAward *>* ItemAwardManager::GetByLogin(const char* c_pszLogin)
{
	auto it = m_map_kSetAwardByLogin.find(c_pszLogin);
	if (it == m_map_kSetAwardByLogin.end())
		return nullptr;

	return &it->second;
}

void ItemAwardManager::Taken(uint32_t dwAwardID, uint32_t dwItemID)
{
	const auto it = m_map_award.find(dwAwardID);
	if (it == m_map_award.end())
	{
		SPDLOG_INFO( "ITEM_AWARD: Taken ID not exist {0}", dwAwardID);
		return;
	}

	auto k = it->second;
	k->bTaken = true;

	//
	// Update taken_time in database to prevent not to give him again.
	// 
	const auto takeQuery = fmt::format(
		"UPDATE item_award SET taken_time=NOW(),item_id={} WHERE id={} AND taken_time IS NULL", dwItemID, dwAwardID);
	CDBManager::instance().ReturnQuery(takeQuery, QID_ITEM_AWARD_TAKEN, 0, nullptr);
	SPDLOG_INFO("ItemAwardManager::Taken {} {}", dwAwardID, dwItemID);
}

std::map<uint32_t, TItemAward *>& ItemAwardManager::GetMapAward()
{
	return m_map_award;
}

std::map<std::string, std::set<TItemAward *>>& ItemAwardManager::GetMapkSetAwardByLogin()
{
	return m_map_kSetAwardByLogin;
}

void ItemAwardManager::StartReloadTimer()
{
	m_reloadTimer.expires_from_now(chrono::seconds(5));
	m_reloadTimer.async_wait([this] (const boost::system::error_code& ec) {
		if (ec == asio::error::operation_aborted)
			return;

		if (ec) {
			spdlog::error("Error waiting for reload timer");
			return;
		}

		RequestLoad();
		StartReloadTimer();
	});
}
