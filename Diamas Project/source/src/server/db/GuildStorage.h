#ifndef METIN2_SERVER_DB_GUILDSTORAGE_H
#define METIN2_SERVER_DB_GUILDSTORAGE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#ifdef ENABLE_GUILD_STORAGE

#include "grid.h"
#include "Peer.h"
#include <game/DbPackets.hpp>
typedef std::map <uint32_t, TPlayerItem> TGuildStorageItemTable;
typedef std::map <uint32_t, TQueueItem> TGuildStorageQueue;

class CGrid;

class CGuildStorage
{
public:
	CGuildStorage(uint32_t guildID);
	~CGuildStorage();
	enum
	{
		GUILDSTORAGE_SLOT_X_COUNT = 17,
		GUILDSTORAGE_SLOT_Y_COUNT = 6,
		GUILDSTORAGE_PAGE_SIZE = GUILDSTORAGE_SLOT_X_COUNT * GUILDSTORAGE_SLOT_Y_COUNT,
		GUILDSTORAGE_TAB_COUNT = 10,
		GUILDSTORAGE_GLYPH_COUNT = 10,
		GUILDSTORAGE_TOTAL_SIZE = (GUILDSTORAGE_TAB_COUNT * GUILDSTORAGE_PAGE_SIZE) + GUILDSTORAGE_GLYPH_COUNT,
		GUILDSTORAGE_GLYPH_START = GUILDSTORAGE_TOTAL_SIZE - GUILDSTORAGE_GLYPH_COUNT
	};
public:
	void reloadItems();
	int getTick() { return iTickRate; };
	void addTick() { iTickRate++; }
	
	// Locking
	void lock(bool state) { locked = state; }
	bool isLocked() { return locked; }

	// Item suck thins
	bool addItemByGame(CPeer * peering, TPlayerItem item, int dwDestPos, uint32_t queID);
	bool addItemRemoveGame(CPeer * peering, TPlayerItem item, int dwDestPos, uint32_t queID);
	void moveItem(CPeer * peering, int dwSrcPos, int dwDestPos, uint32_t playerId);
	void addItem(CPeer * peering, TPlayerItem item, uint16_t dwDestPos);
	void removeItem(CPeer * peering, uint32_t dwVID, uint16_t wPos, uint32_t dwVnum, uint32_t queID);
	void eraseQue(CPeer * peering, uint32_t queID);
	void removeItemRequest(CPeer * peering, TPlayerItem item, int wPos, uint32_t queID);

	// Save item	
	void SaveSingleItem(TPlayerItem* pItem);

	// Send items
	void sendItems(CPeer * peering);
private:
	// Load items
	void loadItems();

	// ITEM Table
	TGuildStorageItemTable m_Items;
	TGuildStorageQueue	m_Queue;

	// Grid
	CGrid * m_pkGrid;
	bool IsEmpty(uint16_t dwPos, uint8_t bSize);

	// Guild informations
	uint32_t		dwGuildID;

	// Destory
	void __Destroy();

	// Last tick
	int iTickRate;

	// Locked storage
	bool locked;
};
#endif
#endif /* METIN2_SERVER_DB_GUILDSTORAGE_H */
