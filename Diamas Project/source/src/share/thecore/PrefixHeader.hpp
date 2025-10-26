#ifndef METIN2_THECORE_PREFIXHEADER_HPP
#define METIN2_THECORE_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#if VSTD_PLATFORM_WINDOWS
	#include <storm/WindowsPlatform.hpp>

	#include <winsock2.h>
#endif

#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;


#include <fmt/format.h>
#include <fmt/ostream.h>
#include <SpdLog.hpp>



#endif
