#ifndef METIN2_CLIENT_MILESLIB_SAMPLEFILE_HPP
#define METIN2_CLIENT_MILESLIB_SAMPLEFILE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <memory>
#include <pak/VfsFile.hpp>
#include <storm/io/View.hpp>
#include <string_view>

class SampleFile
{
  public:
    SampleFile(std::string_view filename, std::unique_ptr<VfsFile> file);

    const std::string &GetFilename() const
    {
        return m_filename;
    }

    const void *GetData() const;
    uint32_t GetSize() const;

  private:
    std::string m_filename;
    std::unique_ptr<VfsFile> m_file;
    storm::View m_view;
};

using SampleFilePtr = std::shared_ptr<SampleFile>;

#endif
