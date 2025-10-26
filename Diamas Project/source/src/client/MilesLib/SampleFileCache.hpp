#ifndef METIN2_CLIENT_MILESLIB_SAMPLEFILECACHE_HPP
#define METIN2_CLIENT_MILESLIB_SAMPLEFILECACHE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SampleFile.hpp"

#include <unordered_map>

class SampleFileCache
{
public:
    SampleFilePtr Get(std::string_view filename);

private:
    std::unordered_map<std::string_view, SampleFilePtr> m_samples;
};

#endif
