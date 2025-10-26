#ifndef METIN2_TOOL_QC_PREFIXHEADER_HPP
#define METIN2_TOOL_QC_PREFIXHEADER_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#if VSTD_PLATFORM_WINDOWS
	#include <storm/WindowsPlatform.hpp>
#endif

extern "C" {

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

}

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include <cstdio>
#include <cstring>

#endif
