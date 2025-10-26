#include "GuildStorage.h"
#include "GuildStorageManager.h"
#ifdef ENABLE_GUILD_STORAGE


CGuildStorageManager::CGuildStorageManager()
{
}


CGuildStorageManager::~CGuildStorageManager()
{
}

CGuildStorage * CGuildStorageManager::getStorage(uint32_t guildID) 
{
	TGuildStorages::iterator it = m_Storages.find(guildID);
	if (it != m_Storages.end()) {
		return it->second;
	}
	else {
		//CGuildStorage guildStorage(guildID);
		CGuildStorage * guildStorage = new CGuildStorage(guildID);
		m_Storages.insert(std::make_pair(guildID, guildStorage));
		return guildStorage;
	}
	return nullptr;	// On a fatal error (U donowww u donowww)
}


#endif