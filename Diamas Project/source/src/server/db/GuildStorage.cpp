#include "ClientManager.h"
#include "DBManager.h"
#include <string>
#include <vector>


#ifdef ENABLE_GUILD_STORAGE

#include "grid.h"
#include "GuildStorage.h"
// Constructor
CGuildStorage::CGuildStorage(uint32_t guildID)
{	// Init grid
	m_pkGrid = new CGrid(GUILDSTORAGE_SLOT_X_COUNT, GUILDSTORAGE_SLOT_Y_COUNT * GUILDSTORAGE_TAB_COUNT + 1);	// Set grid

	dwGuildID = guildID;	// Set Guild ID
	locked = false;	// Lock mode?
	loadItems();	// Load items on create
}

// Destructor
CGuildStorage::~CGuildStorage()
{
	__Destroy();
}

// Destroy
void CGuildStorage::__Destroy()
{
	if (m_pkGrid != nullptr)
	{
		delete m_pkGrid;
		m_pkGrid = nullptr;
	}
}

// Reload items
void CGuildStorage::reloadItems() 
{
	m_Items.clear();	// Clear items
	m_pkGrid->Clear();	// Clear grid
	loadItems();		// Reload items
}

// Is empty
bool CGuildStorage::IsEmpty(uint16_t dwPos, uint8_t bSize)
{
	if (!m_pkGrid) {
		spdlog::error("Grid is not defined");
		return false;
	}
	return m_pkGrid->IsEmpty(dwPos, 1, bSize);
}

// Remove item
void CGuildStorage::removeItemRequest(CPeer * peering, TPlayerItem item, int wPos, uint32_t queID) {
	if (!peering) {
		spdlog::error("Peering is undefined");
		return;
	}

	auto it = m_Items.find(wPos);
	if (it != m_Items.end()) {
		// Get item infos
		if (it->second.vnum != item.vnum || item.id != it->second.id) {
			spdlog::error("item missmatch for remove request  vnum %d/%d  id %d/%d", it->second.vnum, item.vnum, it->second.id, item.id);
			eraseQue(peering, queID);	// Erase que (item is not here)
			return;
		}

		const TItemTable* pProto = CClientManager::Instance().GetItemTable(it->second.vnum);
		if (!pProto)
		{
			spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
			eraseQue(peering, queID);	// Erase que (item is not here)
			return;
		}
		
		TQueueItem returnItem = {};
		returnItem.item = it->second;
		returnItem.bQueueType = 2;

		m_Items.erase(it);					// Remove item from space
		m_pkGrid->Get(wPos, 1, pProto->bSize);	// Free grid
		

		TPacketGuildStorageRemoveItemReturn packet;
		packet.queItem = returnItem;
		packet.queID = queID;
		packet.wPos = wPos;
		packet.guildID = dwGuildID;

		peering->EncodeHeader(HEADER_DG_GUILDSTORAGE_REMOVE_ITEM, 0, sizeof(TPacketGuildStorageRemoveItemReturn));
		peering->Encode(&packet, sizeof(TPacketGuildStorageRemoveItemReturn));

		TPacketGuildStorageRemoveSyncItem syncPacket;
		syncPacket.guildID = dwGuildID;
		syncPacket.pos = wPos;

		CClientManager::instance().ForwardPacket(HEADER_DG_GUILDSTORAGE_REMOVE_SYNC_ITEM, &syncPacket, sizeof(TPacketGuildStorageRemoveSyncItem), 0, peering);
		SPDLOG_INFO( "CGuildStorage::removeItemRequest: Item removed (id: %d) vnum: %d ", returnItem.item.id, returnItem.item.vnum);
	}
	else {
		eraseQue(peering, queID);	// Erase que (item is not here)
	}
}

// Add item (step 2) removed from game with response (added to db server)
bool CGuildStorage::addItemRemoveGame(CPeer * peering, TPlayerItem item, int dwDestPos, uint32_t queID) {
	if (!peering) {
		spdlog::error("Peering is undefined");
		return false;
	}

	TGuildStorageQueue::iterator it = m_Queue.find(queID);
	if (it != m_Queue.end()) {
		// Get item infos
		const TItemTable* pProto = CClientManager::Instance().GetItemTable(item.vnum);
		if (!pProto)
		{
			spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
			return false;
		}

		// Todo: Mach ich später (meine eier sind am arsch)...
		if (IsEmpty(dwDestPos, pProto->bSize)) {
			spdlog::error("Fatal error", dwDestPos, pProto->bSize);
			return false;
		}

		if (it->second.item.vnum == item.vnum && it->second.item.id == item.id) {
			item.owner = dwGuildID;		// Guild id
			item.pos = dwDestPos;		// Dest pos
			item.window = GUILDSTORAGE;	// Guildstorage window

			m_Items.insert(std::make_pair(dwDestPos, item));	// Add item to map
			CClientManager::instance().RemoveCachedItem(item.id);	// Remove from cache

			SaveSingleItem(&item);

			// Send update packet to other cores
			TPacketGuildStorageItem p;
			p.guildID = dwGuildID;
			p.item = item;
			p.iTick = getTick();
			p.wPos = dwDestPos;

			CClientManager::Instance().ForwardPacket(HEADER_DG_GUILDSTORAGE_ITEM, &p, sizeof(TPacketGuildStorageItem));
			SPDLOG_INFO( "addItemRemoveGame (Response): item: %d id: %d que id: %d   guild_id: %d ", item.vnum, item.id, queID, dwGuildID);
			return true;
		}
		else {
			m_pkGrid->Get(dwDestPos, 1, pProto->bSize);	// Free space
			spdlog::error("Possible queue duplicate (WARNING - This is not casual!)");
		}
	}
	else {
		spdlog::error("Queue with id %d not exist", queID);
	}
	return false;
}


// Delete queue item @gamecore
void CGuildStorage::eraseQue(CPeer * peering, uint32_t queID) {
	if (!peering) {
		spdlog::error("Peering is undefined");
		return;
	}

	TPacketGuildStorageEraseQue packetDestroy;
	packetDestroy.queID = queID;
	packetDestroy.guildID = dwGuildID;
	peering->EncodeHeader(HEADER_DG_GUILDSTORAGE_DESTROY_QUE, 0, sizeof(TPacketGuildStorageEraseQue));
	peering->Encode(&packetDestroy, sizeof(TPacketGuildStorageEraseQue));
	SPDLOG_INFO( "CGuildStorage::eraseQue: Erasing que with id %d guild %d", queID, dwGuildID);
}

// Add item (step 1)
bool CGuildStorage::addItemByGame(CPeer * peering, TPlayerItem item, int dwDestPos, uint32_t queID) {
	if (!peering) {
		spdlog::error("Peering is undefined");
		return false;
	}

	// Get item infos
	const TItemTable* pProto = CClientManager::Instance().GetItemTable(item.vnum);
	if (!pProto)
	{
		spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
		eraseQue(peering, queID);
		return false;
	}

	// Todo: Mach ich später (meine eier sind am arsch)...
	if (!IsEmpty(dwDestPos, pProto->bSize)) {
		spdlog::error("Trying to add item to used pos %d (size %d).", dwDestPos, pProto->bSize);
		eraseQue(peering, queID);
		return false;
	}

	// Put item to grid (Use space)
	m_pkGrid->Put(dwDestPos, 1, pProto->bSize);

	// Create back response que
	TQueueItem queItem;
	queItem.bQueueType = 1;
	queItem.item = item;							// Item
	m_Queue.insert(std::make_pair(queID, queItem));	// Add to local que for wait remove response

	// Add item
	TPacketGuildStorageAddItem packet;
	packet.guildID = dwGuildID;
	packet.queID = queID;
	packet.queItem = queItem;
	packet.wPos = dwDestPos;

	// Send to explicit requester the informations
	peering->EncodeHeader(HEADER_DG_GUILDSTORAGE_ADD_ITEM_RESPONE_1, 0, sizeof(TPacketGuildStorageAddItem));
	peering->Encode(&packet, sizeof(TPacketGuildStorageAddItem));
	SPDLOG_INFO( "AddItemByGame: item: %d id: %d que id: %d   guild_id: %d ", item.vnum, item.id, queID, dwGuildID);
	return true;
}

// Move item
void CGuildStorage::moveItem(CPeer * peering, int dwSrcPos, int dwDestPos, uint32_t playerId)
{
	TGuildStorageItemTable::iterator it = m_Items.find(dwSrcPos);

	if (m_Items.end() != it) {
		TPlayerItem item = it->second;

		// Get item infos
		const TItemTable* pProto = CClientManager::Instance().GetItemTable(item.vnum);
		if (!pProto)
		{
			spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
			return;
		}

		// Todo: Mach ich später (meine eier sind am arsch)...
		if (!IsEmpty(dwDestPos, pProto->bSize)) {
			spdlog::error("Trying to move item to not empty position source %d dest %d (size %d).", dwSrcPos, dwDestPos, pProto->bSize);
			return;
		}
		
		addTick();	// Add change
		m_pkGrid->Get(dwSrcPos, 1, pProto->bSize);	// Remove item from grid
		item.pos = dwDestPos;
		m_Items.insert(std::make_pair(dwDestPos, item));
		m_pkGrid->Put(dwDestPos, 1, pProto->bSize);	// Add item to grid
		
		// Remove item from map
		m_Items.erase(dwSrcPos);

		// Save
		SaveSingleItem(&item);

		// Packet forward
		TPacketGuildStorageMoveItem p;
		p.dwDestPos = dwDestPos;
		p.dwSrcPos = dwSrcPos;
		p.guildID = dwGuildID;
		p.item = item;
		p.iTick = getTick();	// Tick for desyncronization detection
		p.playerId = playerId;

		// Client manager
		CClientManager::Instance().ForwardPacket(HEADER_DG_GUILDSTORAGE_MOVE, &p, sizeof(TPacketGuildStorageMoveItem));
		SPDLOG_INFO( "Moved item from %d to %d with vnum %d", dwSrcPos, dwDestPos, item.vnum);
	}
	else {
		sendItems(peering);	// Reload
		spdlog::error("Cannot find item at %d", dwSrcPos);
	}
	return;
}

// Remove item
void CGuildStorage::removeItem(CPeer * peering, uint32_t dwVID, uint16_t wPos, uint32_t dwVnum, uint32_t queID)
{
	if (!peering) {
		spdlog::error("Peering is undefined");
		return;
	}

	if (dwVID && queID) {
		TGuildStorageItemTable::iterator it = m_Items.find(wPos);
		if (it != m_Items.end()) {
			addTick();	// Add tick

			TPacketGuildStorageAddItemToChar pChar;
			pChar.item = it->second;
			pChar.iTick = getTick();
			pChar.wPos = wPos;
			pChar.dwVID = dwVID;
			pChar.queID = queID;
			pChar.guildID = dwGuildID;

			// Send to explicit requester the informations
			peering->EncodeHeader(HEADER_DG_GUILDSTORAGE_ADD_ITEM_TO_CHARACTER, 0, sizeof(TPacketGuildStorageRemoveItem));
			peering->Encode(&pChar, sizeof(TPacketGuildStorageRemoveItem));

			TPacketGuildStorageRemoveItem p;
			p.wPos = wPos;
			p.iTick = getTick();
			p.guildID = dwGuildID;

			// Client manager
			CClientManager::Instance().ForwardPacket(HEADER_DG_GUILDSTORAGE_REMOVE, &p, sizeof(TPacketGuildStorageRemoveItem), 0, peering);
			m_Items.erase(it);	// Remove from map
		}
		else {
			spdlog::error("[Desync] Detected: item with vnum %d can not be owned by VID %d at position %d queid %d", dwVnum, dwVID, wPos, queID);
		}
	}
}

// Add item
void CGuildStorage::addItem(CPeer * peering, TPlayerItem item, uint16_t dwDestPos)
{
	if (!peering) {
		spdlog::error("Peering is undefined");
		return;
	}

	// Get item infos
	const TItemTable* pProto = CClientManager::Instance().GetItemTable(item.vnum);
	if (!pProto)
	{
		spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
		return;
	}

	if (IsEmpty(dwDestPos, pProto->bSize)) {
		m_pkGrid->Put(dwDestPos, 1, pProto->bSize);			// Add item to grid
		m_Items.insert(std::make_pair(dwDestPos, item));	// Add item to map
		CClientManager::instance().RemoveCachedItem(item.id);	// Remove from cache

		// Send update packet to other cores
		TPacketGuildStorageItem p;
		p.guildID = dwGuildID;
		p.item = item;
		p.iTick = getTick();
		p.wPos = dwDestPos;

		CClientManager::Instance().ForwardPacket(HEADER_DG_GUILDSTORAGE_ITEM, &p, sizeof(TPacketGuildStorageItem));
	}
	else {
		spdlog::error("[Desync] Detected: item (%d) vnum %d from guild %d is mabye lost.",item.id, item.vnum, dwGuildID);
	}
	return;
}

// Send items to specified peering
void CGuildStorage::sendItems(CPeer * peering) {
	if (!peering) {
		spdlog::error("Peering is undefined");
		return;
	}
	for (TGuildStorageItemTable::iterator it = m_Items.begin(); it != m_Items.end(); ++it) {
		TPacketGuildStorageItem p;
		p.guildID = dwGuildID;
		p.item = it->second;
		p.iTick = getTick();
		p.wPos = it->first;
		p.loadCount = m_Items.size();

		peering->EncodeHeader(HEADER_DG_GUILDSTORAGE_ITEM, 0, sizeof(TPacketGuildStorageItem));
		peering->Encode(&p, sizeof(TPacketGuildStorageItem));
	}
}


// Save item
void CGuildStorage::SaveSingleItem(TPlayerItem* pItem)
{
	std::string saveQuery = "";

	if (!pItem->owner)
	{
		SPDLOG_INFO( "CGuildStorage::SaveSingleItem delete item %u window %u", pItem->id, GUILDSTORAGE);
		saveQuery = fmt::sprintf("DELETE FROM item WHERE id = %u AND `window` = %u", pItem->id, GUILDSTORAGE);
		delete pItem;
	}
	else
	{
		SPDLOG_INFO( "CGuildStorage::SaveSingleItem id: %u owner: %u vnum: %u window: %u position: %u", pItem->id, pItem->owner, pItem->vnum, pItem->window, pItem->pos);
		saveQuery = fmt::sprintf(
			"REPLACE INTO item "
			" SET id = %u, "
				"owner_id = %d, "
				"`window` = %d, "
				"pos = %d, "
				"count = %u, "
				"vnum = %u, "
				"price = %lld,"
				"socket0 = %lld, "
				"socket1 = %lld, "
				"socket2 = %lld, "
				"socket3 = %lld, "
				"socket4 = %lld, "
				"socket5 = %lld, "
				"attrtype0 = %u, attrvalue0 = %d, "
				"attrtype1 = %u, attrvalue1 = %d, "
				"attrtype2 = %u, attrvalue2 = %d, "
				"attrtype3 = %u, attrvalue3 = %d, "
				"attrtype4 = %u, attrvalue4 = %d, "
				"attrtype5 = %u, attrvalue5 = %d, "
				"attrtype6 = %u, attrvalue6 = %d ",
			pItem->id, 
			pItem->owner, 
			pItem->window, 
			pItem->pos, 
			pItem->count, 
			pItem->vnum,
			pItem->price,
			pItem->alSockets[0], 
			pItem->alSockets[1], 
			pItem->alSockets[2], 
			pItem->alSockets[3],
			pItem->alSockets[4],
			pItem->alSockets[5],
			pItem->aAttr[0].bType, pItem->aAttr[0].sValue,
			pItem->aAttr[1].bType, pItem->aAttr[1].sValue, 
			pItem->aAttr[2].bType, pItem->aAttr[2].sValue, 
			pItem->aAttr[3].bType, pItem->aAttr[3].sValue, 
			pItem->aAttr[4].bType, pItem->aAttr[4].sValue, 
			pItem->aAttr[5].bType, pItem->aAttr[5].sValue,
			pItem->aAttr[6].bType, pItem->aAttr[6].sValue);
	}

	CDBManager::Instance().AsyncQuery(saveQuery, SQL_PLAYER);
}

// Load items
void CGuildStorage::loadItems()
{
	spdlog::error("Loading items");
	m_pkGrid->Clear();

	std::unique_ptr<SQLMsg> msg ( CDBManager::instance().DirectQuery(fmt::format("SELECT id, pos, count, vnum, price, socket0, socket1, socket2, socket3, socket4, socket5, "
		"attrtype0, attrvalue0, "
		"attrtype1, attrvalue1, "
		"attrtype2, attrvalue2, "
		"attrtype3, attrvalue3, "
		"attrtype4, attrvalue4, "
		"attrtype5, attrvalue5, "
		"attrtype6, attrvalue6 "
		"FROM item WHERE owner_id={} AND `window`='GUILDSTORAGE'", dwGuildID)));

	MYSQL_RES *res = msg->Get()->pSQLResult;

	if (!res)
		return;

	int rows;

	if ((rows = mysql_num_rows(res)) <= 0)
		return;

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem item;

		int cur = 0;

		str_to_number(item.id, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		item.owner = dwGuildID;
		item.window = GUILDSTORAGE;
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
		str_to_number(item.price, row[cur++]);
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);
		str_to_number(item.alSockets[3], row[cur++]);
		str_to_number(item.alSockets[4], row[cur++]);
		str_to_number(item.alSockets[5], row[cur++]);

		for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
		{
			str_to_number(item.aAttr[j].bType, row[cur++]);
			str_to_number(item.aAttr[j].sValue, row[cur++]);
		}

		const TItemTable* pProto = CClientManager::Instance().GetItemTable(item.vnum);
		if (!pProto)
		{
			spdlog::error("cannot get proto of vnum %u guild id %u", item.vnum, dwGuildID);
			return;
		}

		if (!IsEmpty(item.pos, pProto->bSize)) {
			spdlog::error("position overwrite item %d for guild %d", item.id, dwGuildID);
			continue;
		}

		SPDLOG_INFO( "CGuildStorage::loadItems(): Item %d owner: %d pos: %d window: %d count: %d ", item.vnum, item.owner, item.pos, item.window, item.count);

		m_pkGrid->Put(item.pos, 1, pProto->bSize);
		TGuildStorageItemTable::iterator it = m_Items.find(item.pos);

		if (it != m_Items.end()) {
			it->second = item;
		}
		else {
			m_Items.insert(std::make_pair(item.pos, item));
		}
	}
}

#endif
