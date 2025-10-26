#include "StdAfx.h"
#include <pak/Vfs.hpp>

#include "Property.h"
#include "PropertyManager.h"

#include "base/GroupTextTree.hpp"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <storm/io/View.hpp>

#include "pak/Util.hpp"

CPropertyManager::CPropertyManager() : m_isFileMode(true)
{
}

CPropertyManager::~CPropertyManager()
{
    Clear();
}

bool CPropertyManager::Initialize(const char *c_pszPackFileName)
{
    auto data = LoadFileToString(GetVfs(), "property/config.txt");
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        SPDLOG_ERROR("Failed to parse config file property/config.txt");
        return false;
    }

    auto files = reader.GetList("Files");
    if (files != nullptr)
    {
#ifdef _DEBUG
        SPDLOG_INFO("Property config contains {0} files", files->GetLineCount());
#endif
        for (uint32_t i = 0; i < files->GetLineCount(); ++i)
        {
            auto path = files->GetLine(i);

            if (!Register(path))
                SPDLOG_ERROR("Failed to register property {0}", path, path);
        }
    }

    auto reserved = reader.GetList("ReservedCRC");
    if (reserved != nullptr)
    {
#ifdef _DEBUG
        SPDLOG_INFO("Property config contains {0} reserved CRCs", reserved->GetLineCount());
#endif
        for (uint32_t i = 0; i < reserved->GetLineCount(); ++i)
        {
            auto crc = reserved->GetLine(i);

            uint32_t val;
            if (!storm::ParseNumber(crc, val))
            {
                SPDLOG_ERROR("{0} is not a valid reserved CRC", crc);
                continue;
            }

            ReserveCRC(val);
        }
    }

    return true;
}

bool CPropertyManager::Put(CProperty *property)
{
    auto r = m_PropertyByCRCMap.insert(std::make_pair(property->GetCRC(), property));
    if (!r.second)
    {
        SPDLOG_INFO("Property already registered, replace {0} to {1}", r.first->second->GetFileName(),
                    property->GetFileName());

        r.first->second.reset(property);
    }

    return true;
}

bool CPropertyManager::Erase(const storm::String &filename)
{
    CProperty *property;
    if (!Get(filename, &property))
        return false;

    m_PropertyByCRCMap.erase(property->GetCRC());
    delete property;

    ::DeleteFileA(filename.c_str());
    return false;
}

bool CPropertyManager::BuildPack()
{
    return false;

    /*if (!m_pack.Create(m_fileDict, "property", ""))
        return false;

    WIN32_FIND_DATA fdata;
    HANDLE hFind = FindFirstFile("property\\*", &fdata);

    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        char szSourceFileName[256 + 1];
        _snprintf(szSourceFileName, sizeof(szSourceFileName), "property\\%s", fdata.cFileName);

        m_pack.Put(fdata.cFileName, szSourceFileName,COMPRESSED_TYPE_NONE,"");
    }
    while (FindNextFile(hFind, &fdata));

    FindClose(hFind);
    return true;*/
}

bool CPropertyManager::LoadReservedCRC(const char *c_pszFileName)
{

    auto data = LoadFileToString(GetVfs(), c_pszFileName);
    if (!data)
        return false;

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(data.value());

    for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        auto pszLine = kTextFileLoader.GetLineString(i);

        if (pszLine.empty())
            continue;

        ReserveCRC(std::stoul(pszLine));
    }

    return true;
}

void CPropertyManager::ReserveCRC(uint32_t dwCRC)
{
    m_ReservedCRCSet.insert(dwCRC);
}

uint32_t CPropertyManager::GetUniqueCRC(const char *c_szSeed)
{
    std::string stTmp = c_szSeed;

    while (true)
    {
        DWORD dwCRC = ComputeCrc32(0, stTmp.c_str(), stTmp.length());

        if (m_ReservedCRCSet.find(dwCRC) == m_ReservedCRCSet.end() &&
            m_PropertyByCRCMap.find(dwCRC) == m_PropertyByCRCMap.end())
            return dwCRC;

        stTmp += fmt::format("{}", Random::get(0, 9));
    }
}


bool CPropertyManager::Register(std::string_view filename, CProperty **ppProperty)
{
    auto fp = GetVfs().Open(filename);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    auto pProperty = std::make_unique<CProperty>(filename.data());

    if (!pProperty->ReadFromMemory(data.GetData(), size, filename.data()))
    {
        return false;
    }

    uint32_t dwCRC = pProperty->GetCRC();

    CProperty *rawPtr = pProperty.get();

    auto itor = m_PropertyByCRCMap.find(dwCRC);

    if (m_PropertyByCRCMap.end() != itor)
    {
        SPDLOG_ERROR("Property already registered, replace {0} to {1}", itor->second->GetFileName(),
                     filename);

        itor->second = std::move(pProperty);
    }
    else
        m_PropertyByCRCMap.emplace(dwCRC, std::move(pProperty));

    if (ppProperty)
        *ppProperty = rawPtr;

    return true;
}

bool CPropertyManager::Get(const std::string &c_pszFileName, CProperty **ppProperty)
{
    return Register(c_pszFileName, ppProperty);
}

bool CPropertyManager::Get(uint32_t dwCRC, CProperty **ppProperty)
{
    auto itor = m_PropertyByCRCMap.find(dwCRC);

    if (m_PropertyByCRCMap.end() == itor)
        return false;

    *ppProperty = itor->second.get();
    return true;
}

void CPropertyManager::Clear()
{
    m_PropertyByCRCMap.clear();
}
