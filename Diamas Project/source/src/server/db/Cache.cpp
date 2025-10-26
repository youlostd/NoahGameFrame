#include "Cache.h"
#include "QID.h"
#include "ClientManager.h"
#include "Main.h"
#include "Persistence.hpp"

#include <thecore/utils.hpp>

extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_test_server;
// MYSHOP_PRICE_LIST
extern int g_iItemPriceListTableCacheFlushSeconds;
// END_OF_MYSHOP_PRICE_LIST
extern int g_iShopCacheFlushSeconds;

extern int g_item_count;

CachedItem::CachedItem()
	: CachedObject(chrono::milliseconds(g_iItemCacheFlushSeconds * 1000))
	// Not strictly necessary (we shouldn't access uninitialized items)
	, m_item{}
{
	// ctor
}
CachedItem::~CachedItem()
{

}


void CachedItem::SetData(const TPlayerItem& item, bool setDirty)
{
	if (setDirty && std::memcmp(&item, &m_item, sizeof(m_item)) != 0)
		m_dirty = true;

	m_item = item;
}

void CachedItem::Flush()
{
	SPDLOG_TRACE("Flushing cached item {0} (vnum {1})",
		m_item.id, m_item.vnum);
	std::string query;
	CreateItemSaveQuery(m_item, query);
	CDBManager::instance().ReturnQuery(query, QID_ITEM_SAVE, 0, NULL);
}

bool IsOwnedByPlayer(const TPlayerItem& item)
{
	switch (item.window) {
	case SAFEBOX:
	case MALL:
		return false;

	default:
		return true;
	}
}

bool IsOwnedByPlayer(CachedItem* item)
{
	return IsOwnedByPlayer(item->GetData());
}

CachedPlayer::CachedPlayer()
	: CachedObject(chrono::milliseconds(g_iPlayerCacheFlushSeconds * 1000))
	// Not strictly necessary (we shouldn't access uninitialized player tables)
	, m_tab{}
{
	// ctor
}

CachedPlayer::~CachedPlayer()
{

	for (const auto& item : m_items)
		item->Unlock();
}


void CachedPlayer::SetTable(const TPlayerTable& table, bool setDirty)
{
	if (setDirty) {
		if (std::memcmp(&table, &m_tab, sizeof(m_tab)) != 0)
			m_dirty = true;

		for (uint32_t i = 0; i != SKILL_MAX_NUM; ++i) {
			if (m_tab.skills[i] != table.skills[i])
				m_dirtySkills.insert(i);
		}

		for (uint32_t i = 0; i != QUICKSLOT_MAX_NUM; ++i) {
			if (m_tab.quickslot[i] != table.quickslot[i])
				m_dirtyQuickslots.insert(i);
		}

		if(m_tab.title != table.title) {
			m_dirtyTitle = true;
		}

	}

	m_tab = table;
}

void CachedPlayer::SetItems(std::vector<CachedItem*> items, bool setDirty)
{
	m_items = std::move(items);

	if (setDirty)
		m_dirty = true;
}

void CachedPlayer::RemoveItem(CachedItem* item)
{
	const auto it = std::remove(m_items.begin(), m_items.end(), item);
	if (it != m_items.end()) {
		item->Unlock();
		m_items.erase(it, m_items.end());
	}
}

void CachedPlayer::AddItem(CachedItem* item)
{
	item->Lock();
	m_items.push_back(item);
}

void CachedPlayer::Flush()
{
	SPDLOG_TRACE("Flushing cached player {0}", m_tab.id);

	std::string query;
	CreatePlayerSaveQuery(m_tab, query);
	CDBManager::instance().ReturnQuery(query, QID_PLAYER_SAVE, 0, NULL);
	
	FlushSkills();
	FlushQuickslots();
	FlushTitle();
}

void CachedPlayer::FlushSkills()
{
	const TPlayerSkill emptySkill = {};

	std::string query;
	for (auto vnum : m_dirtySkills) {
		const auto& skill = m_tab.skills[vnum];

		SPDLOG_TRACE("Flushing skill {0}: master {1} level {2}",
			vnum, skill.bMasterType, skill.bLevel);

		if (skill == emptySkill)
			CreateSkillDeleteQuery(m_tab.id, vnum, query);
		else
			CreateSkillSaveQuery(m_tab.id, vnum, skill, query);
		CDBManager::instance().AsyncQuery(query.c_str());
		query.clear();
	}

	m_dirtySkills.clear();
}

void CachedPlayer::FlushTitle()
{
	const TPlayerTitle emptyTitle = {};

	std::string query;

	if(m_dirtyTitle) {
		const auto& title = m_tab.title;

		if (title == emptyTitle)
			CreateTitleDeleteQuery(m_tab.id, query);
		else
			CreateTitleSaveQuery(m_tab.id, title, query);

		CDBManager::instance().AsyncQuery(query.c_str());
		query.clear();
	}


	m_dirtyTitle = false;
}

void CachedPlayer::FlushQuickslots()
{
	std::string query;
	for (auto index : m_dirtyQuickslots) {
		const auto& q = m_tab.quickslot[index];

		SPDLOG_TRACE("Flushing quickslot {0}: type {1} pos {2}",
			index, q.type, q.pos);

		if (q.type != QUICKSLOT_TYPE_NONE)
			CreateQuickslotSaveQuery(m_tab.id, index, q, query);
		else
			CreateQuickslotDeleteQuery(m_tab.id, index, query);

		CDBManager::instance().AsyncQuery(query.c_str());
		query.clear();
	}

	m_dirtyQuickslots.clear();
}


#ifdef __OFFLINE_SHOP__
int g_iOfflineShopCacheTime = 5;

COfflineShopCache::COfflineShopCache()
{
	this->m_expireTime = g_iOfflineShopCacheTime;
	this->m_bShopWereDeleted = false;
}

COfflineShopCache::~COfflineShopCache() { }

void COfflineShopCache::OnFlush()
{
	std::string szQuery;

	if (this->m_data.byChannel == 0)
	{
		szQuery = fmt::format("DELETE FROM offline_shop WHERE pid = {}",
			this->m_data.dwOwnerPID);

		this->m_bShopWereDeleted = true;
	}
	else
	{
		static char szEscapedName[SHOP_SIGN_MAX_LEN * 2 + 1];
		CDBManager::instance().EscapeString(szEscapedName, static_cast<const char*>(this->m_data.szName), sizeof(this->m_data.szName));

		szQuery = fmt::format(
			"REPLACE INTO offline_shop ("
				"pid, "
				"name, "
				"gold, "
				"open, "
				"channel, "
				"map_index, "
				"x, "
				"y"
			") VALUES("
				"{},"
				"'{}',"
				"{},"
				"{},"
				"{},"
				"{},"
				"{},"
				"{}"
			")",
			this->m_data.dwOwnerPID,
			szEscapedName,
			this->m_data.llGold,
#ifdef __OFFLINE_SHOP_OPENING_TIME__
			this->m_data.iLeftOpeningTime,
#else
			(this->m_data.bOpen ? 1 : 0),
#endif
			this->m_data.byChannel,
			this->m_data.lMapIndex,
			this->m_data.lX,
			this->m_data.lY);
	}

	CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER);
}

bool COfflineShopCache::ShopWereDeleted()
{
	return this->m_bShopWereDeleted;
}

COfflineShopItemCache::COfflineShopItemCache()
{
	this->m_expireTime = g_iOfflineShopCacheTime;
	this->m_bItemWereDeleted = false;
}

COfflineShopItemCache::~COfflineShopItemCache() { }

void COfflineShopItemCache::Put(TOfflineShopItem* pkNew, uint32_t dwOwnerPID, bool bSkipQuery)
{
	memcpy(&this->m_data, pkNew, sizeof(this->m_data));
	this->m_dwOwnerPID = dwOwnerPID;
	this->m_lastUpdateTime = time(0);

	if (!bSkipQuery)
		this->m_bNeedQuery = true;
}

void COfflineShopItemCache::OnFlush()
{
	std::string szQuery;

	if(this->m_data.data.info.vnum == 0) {
		szQuery = fmt::format(
			"DELETE FROM offline_shop_items WHERE id = {}",
			this->m_data.dwID);

		this->m_bItemWereDeleted = true;
	} else {
		szQuery = fmt::format("REPLACE INTO offline_shop_items ("
		                      "id, pid, vnum, trans_vnum, count, pos,"
		                      "socket0, socket1, socket2,"
		                      "socket3, socket4, socket5,"
		                      "attrtype0, attrvalue0,"
		                      "attrtype1, attrvalue1,"
		                      "attrtype2, attrvalue2,"
		                      "attrtype3, attrvalue3,"
		                      "attrtype4, attrvalue4,"
		                      "attrtype5, attrvalue5,"
		                      "attrtype6, attrvalue6,"
		                      "price"
		                      ") VALUES("
		                      "{id}, {pid}, {vnum}, {trans_vnum}, {count}, {pos},"
		                      "{socket0}, {socket1}, {socket2},"
		                      "{socket3}, {socket4}, {socket5},"
		                      "{attrtype0}, {attrvalue0},"
		                      "{attrtype1}, {attrvalue1},"
		                      "{attrtype2}, {attrvalue2},"
		                      "{attrtype3}, {attrvalue3},"
		                      "{attrtype4}, {attrvalue4},"
		                      "{attrtype5}, {attrvalue5},"
		                      "{attrtype6}, {attrvalue6},"
		                      "{price}"
		                      ");",
		                      fmt::arg("id", this->m_data.dwID), fmt::arg("pid", this->m_dwOwnerPID),
		                      fmt::arg("vnum", this->m_data.data.info.vnum),
		                      fmt::arg("trans_vnum", this->m_data.data.info.transVnum),
							  fmt::arg("count", this->m_data.data.info.count),
		                      fmt::arg("pos", this->m_data.data.dwPosition),
		                      fmt::arg("socket0", this->m_data.data.info.sockets[0]),
		                      fmt::arg("socket1", this->m_data.data.info.sockets[1]),
		                      fmt::arg("socket2", this->m_data.data.info.sockets[2]),
		                      fmt::arg("socket3", this->m_data.data.info.sockets[3]),
		                      fmt::arg("socket4", this->m_data.data.info.sockets[4]),
		                      fmt::arg("socket5", this->m_data.data.info.sockets[5]),
		                      fmt::arg("attrtype0", this->m_data.data.info.attrs[0].bType),
		                      fmt::arg("attrvalue0", this->m_data.data.info.attrs[0].sValue),
		                      fmt::arg("attrtype1", this->m_data.data.info.attrs[1].bType),
		                      fmt::arg("attrvalue1", this->m_data.data.info.attrs[1].sValue),
		                      fmt::arg("attrtype2", this->m_data.data.info.attrs[2].bType),
		                      fmt::arg("attrvalue2", this->m_data.data.info.attrs[2].sValue),
		                      fmt::arg("attrtype3", this->m_data.data.info.attrs[3].bType),
		                      fmt::arg("attrvalue3", this->m_data.data.info.attrs[3].sValue),
		                      fmt::arg("attrtype4", this->m_data.data.info.attrs[4].bType),
		                      fmt::arg("attrvalue4", this->m_data.data.info.attrs[4].sValue),
		                      fmt::arg("attrtype5", this->m_data.data.info.attrs[5].bType),
		                      fmt::arg("attrvalue5", this->m_data.data.info.attrs[5].sValue),
		                      fmt::arg("attrtype6", this->m_data.data.info.attrs[6].bType),
		                      fmt::arg("attrvalue6", this->m_data.data.info.attrs[6].sValue),
		                      fmt::arg("price", this->m_data.data.llPrice)
		);
	}

	CDBManager::instance().DirectQuery(szQuery, SQL_PLAYER);
}

bool COfflineShopItemCache::ItemWereDeleted()
{
	return this->m_bItemWereDeleted;
}

CDungeonInfoCache::CDungeonInfoCache()
{
	m_expireTime = std::min(1800, g_iItemCacheFlushSeconds);
}

CDungeonInfoCache::~CDungeonInfoCache()
{
}

void CDungeonInfoCache::OnFlush()
{
	auto query = fmt::format("REPLACE INTO dungeon_info (pid, dungeon_id, cooldown_end, complete_count, fastest_time, highest_damage)"
				" VALUES({pid}, {dungeon_id}, FROM_UNIXTIME({cooldown_end}), {completion_count}, {fastest_time}, {highest_damage});", 
				fmt::arg("pid", m_data.dwPlayerID),
		        fmt::arg("dungeon_id", m_data.bDungeonID),
		        fmt::arg("cooldown_end", m_data.dwCooldownEnd),
		        fmt::arg("completion_count", m_data.wCompleteCount),
		        fmt::arg("fastest_time", m_data.wFastestTime),
		        fmt::arg("highest_damage", m_data.dwHightestDamage)
	);
	SPDLOG_INFO("CDungeonInfoCache::Flush : {} - {}", m_data.dwPlayerID, szQuery);

	CDBManager::instance().AsyncQuery(query);
}
#endif