#ifndef METIN2_WINDOWS_NETWORKADAPTERS_HPP
#define METIN2_WINDOWS_NETWORKADAPTERS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>

#include <vector>

METIN2_BEGIN_NS

bool GetAdapters(std::vector<uint8_t>& adapters);

bool GetFirstAdapterMac(const std::vector<uint8_t>& adapters,
                        vstd::string& name,
                        std::vector<uint8_t>& mac);

METIN2_END_NS

#endif
