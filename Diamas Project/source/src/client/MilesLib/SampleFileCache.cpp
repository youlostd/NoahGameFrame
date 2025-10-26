#include "SampleFileCache.hpp"
#include "Stdafx.h"

#include <pak/Vfs.hpp>

SampleFilePtr SampleFileCache::Get(std::string_view filename)
{
    const auto it = m_samples.find(filename);
    if (it != m_samples.end())
        return it->second;

    auto fp = GetVfs().Open(filename);
    if (!fp)
    {
        SPDLOG_ERROR("Failed to open {0}", filename);
        return nullptr;
    }

	auto sample = std::make_shared<SampleFile>(filename, std::move(fp));
	m_samples.emplace(sample->GetFilename(), sample);
	return sample;
}
