#ifndef METIN2_SERVER_DB_PREFIXHEADER_HPP
#define METIN2_SERVER_DB_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#if VSTD_PLATFORM_WINDOWS
	#include <storm/WindowsPlatform.hpp>

	#include <winsock2.h>
#endif

#include <algorithm>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <cstdio>
#include <cstring>

#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL_USER_LEVEL 0
#else
#define SPDLOG_ACTIVE_LEVEL_USER_LEVEL 4
#endif

#include <SpdLog.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;
namespace asio = boost::asio;


#endif
