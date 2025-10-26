#ifndef METIN2_SERVER_DB_GUILDSTORAGEMANAGER_H
#define METIN2_SERVER_DB_GUILDSTORAGEMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#ifdef ENABLE_GUILD_STORAGE
#include <base/Singleton.hpp>
#include "GuildStorage.h"

typedef std::map <uint32_t, CGuildStorage *> TGuildStorages;

class CGuildStorageManager : public singleton<CGuildStorageManager>
{
public:
	CGuildStorageManager();
	~CGuildStorageManager();

	CGuildStorage * getStorage(uint32_t guildID);

private:
	TGuildStorages m_Storages;
};

#endif


#endif /* METIN2_SERVER_DB_GUILDSTORAGEMANAGER_H */
