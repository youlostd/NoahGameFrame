#ifndef METIN2_SERVER_DB_CONFIG_H
#define METIN2_SERVER_DB_CONFIG_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <unordered_map>

struct DbConfig
{
	std::string addr;
	uint16_t port;
	std::string user;
	std::string password;
	std::string name;
};

struct Config
{
	bool testServer;
	bool verbose;
	bool chinaEventServer;

	std::string bindAddr;
	std::string bindPort;

	DbConfig playerDb;
	DbConfig accountDb;
	DbConfig commonDb;
	DbConfig logDb;

	// in s
	uint32_t playerCacheFlushTime;
	uint32_t itemCacheFlushTime;
	uint32_t itemPriceListCacheFlushTime;

	uint32_t playerDeleteLevelMin;
	uint32_t playerDeleteLevelMax;

	std::string guildPowerPoly;
	std::string guildHandicapPoly;

	uint32_t battlepassId;
};

bool LoadFile(Config& c, const std::string& filename);
#endif /* METIN2_SERVER_DB_CONFIG_H */
