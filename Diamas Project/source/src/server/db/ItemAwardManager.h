// vim:ts=8 sw=4

#ifndef METIN2_SERVER_DB_ITEMAWARDMANAGER_H
#define METIN2_SERVER_DB_ITEMAWARDMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <map>
#include <set>
#include "Peer.h"

#include <boost/asio/basic_waitable_timer.hpp>
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <game/DbPackets.hpp>

typedef struct SItemAward
{
	uint32_t dwID = 0;
	char szLogin[LOGIN_MAX_LEN + 1]{};
	ClientItemData data{};
	char szWhy[ITEM_AWARD_WHY_MAX_LEN + 1]{};
	bool bTaken = false;
	bool bMall = false;
} TItemAward;

class ItemAwardManager : public singleton<ItemAwardManager>
{
public:
	ItemAwardManager(asio::io_service& ioService);

	/// Graceful shutdown
	void Quit();


	void RequestLoad() const;
	void Load(SQLMsg* pMsg);
	std::set<TItemAward *>* GetByLogin(const char* c_pszLogin);

	void Taken(uint32_t dwAwardID, uint32_t dwItemID);
	// gift notify
	std::map<uint32_t, TItemAward *>& GetMapAward();
	std::map<std::string, std::set<TItemAward *>>& GetMapkSetAwardByLogin();

	void StartReloadTimer();


private:
	asio::basic_waitable_timer<std::chrono::steady_clock> m_reloadTimer;
	// ID, ItemAward pair
	std::map<uint32_t, TItemAward *> m_map_award;
	// PID, ItemAward pair
	std::map<std::string, std::set<TItemAward *>> m_map_kSetAwardByLogin;
};

#endif /* METIN2_SERVER_DB_ITEMAWARDMANAGER_H */
