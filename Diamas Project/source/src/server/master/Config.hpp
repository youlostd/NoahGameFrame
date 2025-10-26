#ifndef METIN2_SERVER_MASTER_CONFIG_HPP
#define METIN2_SERVER_MASTER_CONFIG_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>



struct Config
{
	std::string gameBindAddr;
	std::string gameBindPort;

	std::string authBindAddr;
	std::string authBindPort;

	uint8_t logLevel = 3;
};

bool LoadFile(Config& c, const std::string& filename);



#endif
