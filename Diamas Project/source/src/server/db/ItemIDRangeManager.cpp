#include "stdafx.h"
#include "ItemIDRangeManager.h"
#include "Main.h"
#include "DBManager.h"
#include "ClientManager.h"
#include "Peer.h"

namespace
{

	static const uint32_t kItemIdMax = 4290000000;
	static const uint32_t kItemIdMin = 10000000;
	static const uint32_t kMinimumIdCount = 10000;

}

void CItemIDRangeManager::Boot()
{
	for (uint32_t i = 1; ; ++i) {
		uint32_t min = kItemIdMin * (i + 0) + 1;
		uint32_t max = kItemIdMin * (i + 1);

		if (max == kItemIdMax)
			break;

		ItemIdRange range;
		if (BuildRange(min, max, range)) {
			SPDLOG_INFO( "Item ID range: [{0}, {1}] start {2}",
				range.min, range.max, range.start);

			m_ranges.push_back(range);
		}
	}
}

struct FCheckCollision
{
	bool hasCollision;
	ItemIdRange range;

	FCheckCollision(const ItemIdRange& data)
		: hasCollision(false)
		, range(data)
	{
		// ctor
	}

	void operator()(CPeer* peer)
	{
		if (!hasCollision)
			hasCollision = peer->CheckItemIDRangeCollision(range);
	}
};

bool CItemIDRangeManager::BuildRange(uint32_t min, uint32_t max, ItemIdRange& range)
{
	uint32_t start = 0;

	std::unique_ptr<SQLMsg> pMsg (CDBManager::instance().DirectQuery(fmt::format("SELECT MAX(id) FROM item WHERE id >= {} and id <= {}",
		min, max)));
	if (pMsg) {
		if (pMsg->Get()->uiNumRows > 0) {
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			str_to_number(start, row[0]);
		}

	}

	if (start == 0)
		start = min;
	else
		start++;

	if (max - start < kMinimumIdCount) {
		SPDLOG_WARN(
			"ItemIDRange [{0}, {1}] start: {2} is too small - min {3}",
			min, max, start, kMinimumIdCount);
		return false;
	}

	range.min = min;
	range.max = max;
	range.start = start;
	return true;
}

bool CItemIDRangeManager::GetRange(ItemIdRange& range)
{
	while (!m_ranges.empty()) {
		range = m_ranges.front();
		m_ranges.pop_front();

		// TODO(tim): Make assignment sticky
		FCheckCollision f(range);
		CClientManager::instance().for_each_peer(f);

		if (!f.hasCollision)
			return true;
	}

	// Out of ranges...
	return false;
}

void CItemIDRangeManager::ReturnRange(const ItemIdRange& range)
{
	ItemIdRange newRange;
	if (BuildRange(range.min, range.max, newRange))
		m_ranges.push_back(newRange);
}
