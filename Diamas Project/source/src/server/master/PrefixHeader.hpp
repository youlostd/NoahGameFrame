#ifndef METIN2_SERVER_MASTER_PREFIXHEADER_HPP
#define METIN2_SERVER_MASTER_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cstdio>
#include <cstring>

#include <fmt/format.h>
#include <fmt/ostream.h>
#define SPDLOG_ACTIVE_LEVEL_USER_LEVEL SPDLOG_LEVEL_TRACE
#include <SpdLog.hpp>


#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;

#endif
