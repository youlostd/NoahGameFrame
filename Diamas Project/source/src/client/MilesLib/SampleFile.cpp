#include "SampleFile.hpp"
#include "Stdafx.h"

SampleFile::SampleFile(std::string_view filename, std::unique_ptr<VfsFile> file)
    : m_filename(filename), m_file(std::move(file)), m_view(storm::GetDefaultAllocator())
{
    if (m_file)
        m_file->GetView(0, m_view, m_file->GetSize());
}

const void *SampleFile::GetData() const
{
    return m_view.GetData();
}

uint32_t SampleFile::GetSize() const
{
    if (m_file)
        return m_file->GetSize();
    return 0;
}