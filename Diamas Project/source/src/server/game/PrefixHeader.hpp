#ifndef METIN2_SERVER_GAME_PREFIXHEADER_HPP
#define METIN2_SERVER_GAME_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#if VSTD_PLATFORM_WINDOWS
	#include <storm/WindowsPlatform.hpp>

	#include <winsock2.h>
#endif

#include <boost/asio.hpp>

#pragma warning(disable : 4800)
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#define isdigit isdigit
#define isspace isspace

#include <algorithm>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include <cstdio>
#include <cstring>

#include <fmt/format.h>
#include <fmt/printf.h>

#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL_USER_LEVEL 0
#else
#define SPDLOG_ACTIVE_LEVEL_USER_LEVEL 2
#endif

#include <SpdLog.hpp>
#include <effolkronium/random.hpp>
#include <mysql/AsyncSQL.h>

#include "event.h"
#include "typedef.h"
#define RMT_ENABLED 0
#include <base/Remotery.h>

using Random = effolkronium::random_static;
namespace asio = boost::asio;

#endif
