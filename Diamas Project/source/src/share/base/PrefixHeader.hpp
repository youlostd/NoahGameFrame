#ifndef METIN2_BASE_PREFIXHEADER_HPP
#define METIN2_BASE_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#if VSTD_PLATFORM_WINDOWS
	#include <storm/WindowsPlatform.hpp>
#endif

#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <SpdLog.hpp>
#include <spdlog/fmt/ostr.h>

#endif
