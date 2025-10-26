#ifndef METIN2_SERVER_AUTH_CONFIG_HPP
#define METIN2_SERVER_AUTH_CONFIG_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <string>


struct Config
{
	std::string bindAddr;
	std::string bindPort;

	std::string masterAddr;
	std::string masterPort;

	std::string dbCacheAddr;
	std::string dbCachePort;

	uint64_t version;

	uint8_t logLevel;
};

bool LoadFile(Config& c, const std::string& filename);



#endif
