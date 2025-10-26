#ifndef METIN2_CLIENT_MAIN_PYTHONGUILDSTORAGE_H
#define METIN2_CLIENT_MAIN_PYTHONGUILDSTORAGE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PythonNetworkStream.h"

#ifdef ENABLE_GUILD_STORAGE
class CPythonGuildStorage : public CSingleton < CPythonGuildStorage >
{
public:
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
	typedef std::vector<ClientItemData> TItemInstanceVector;

public:
	CPythonGuildStorage();
	virtual ~CPythonGuildStorage();

	void SetItemData(uint32_t dwSlotIndex, const ClientItemData & rItemData);
	void DelItemData(uint32_t dwSlotIndex);

	bool GetSlotItemID(uint32_t dwSlotIndex, uint32_t* pdwItemID);
	bool GetSlotItemTransmutationVnum(uint32_t dwSlotIndex, uint32_t* pdwItemID);

	void OpenGuildStorage();

	int GetCurrentGuildStorageSize();
	bool GetItemDataPtr(uint32_t dwSlotIndex, ClientItemData ** ppInstance);

	void AddLog(TPacketGuildStorageLogGC pack);
	void ClearLog();
	void RequestLog();
	std::vector<TPacketGuildStorageLogGC> GetLogs();

protected:
	TItemInstanceVector m_ItemInstanceVector;
	std::vector<TPacketGuildStorageLogGC> m_LogVector;										   
};

#endif
#endif /* METIN2_CLIENT_MAIN_PYTHONGUILDSTORAGE_H */
