#ifndef METIN2_BASE_DUMPVERSION_HPP
#define METIN2_BASE_DUMPVERSION_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

/// @file DumpVersion.hpp
/// Defines DumpVersion function.
///
/// @{



// Simple version string to be extracted by 'strings' (or similar tools)
// Format: @@VERSION@@<user>@<hostname>@<sha1>@<name>
extern const char* kMachineVersionString;

// Human-readable version string.
extern const char* kHumanVersionString;

// Dumps version information to stderr.
void DumpVersion();



/// @}

#endif
