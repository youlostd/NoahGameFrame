#include "StdAfx.h"
#include "PropertyManager.h"

#include "Property.h"

#include <storm/io/File.hpp>
#include <storm/io/StreamUtil.hpp>

/*
 *	CProperty 파일 포맷
 *
 *  0 ~ 4 bytes: fourcc
 *  5 ~ 6 bytes: \r\n
 *
 *  그 이후의 바이트들은 텍스트 파일 로더와 같은 구조
 */
CProperty::CProperty(const std::string &c_pszFileName)
    : m_stFileName(c_pszFileName), mc_pFileName(NULL), m_dwCRC(0)
{
    StringPath(m_stFileName);

    mc_pFileName = strrchr(m_stFileName.c_str(), '/');

    if (!mc_pFileName)
        mc_pFileName = m_stFileName.c_str();
    else
        ++mc_pFileName;
}

CProperty::~CProperty()
{
}

uint32_t CProperty::GetCRC()
{
    return m_dwCRC;
}

const char *CProperty::GetFileName()
{
    return (m_stFileName.c_str());
}

bool CProperty::GetString(const char *c_pszKey, const char **c_ppString)
{
    std::string stTempKey = c_pszKey;
    stl_lowers(stTempKey);
    CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());

    //	printf("GetString %s %d\n", stTempKey.c_str(), m_stTokenMap.size());

    if (m_stTokenMap.end() == it)
        return false;

    *c_ppString = it->second[0].c_str();
    return true;
}

uint32_t CProperty::GetSize()
{
    return m_stTokenMap.size();
}

bool CProperty::GetVector(const char *c_pszKey, CTokenVector &rTokenVector)
{
    std::string stTempKey = c_pszKey;
    stl_lowers(stTempKey);
    CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());

    if (m_stTokenMap.end() == it)
        return false;

    // NOTE : 튕김 현상 발견
    //	std::copy(rTokenVector.begin(), it->second.begin(), it->second.end());
    // NOTE : 레퍼런스에는 이런 식으로 하게끔 되어 있음
    ///////////////////////////////////////////////////////////////////////////////
    //	template <class InputIterator, class OutputIterator>
    //	OutputIterator copy(InputIterator first, InputIterator last,
    //                    OutputIterator result);
    //
    //	vector<int> V(5);
    //	iota(V.begin(), V.end(), 1);
    //	list<int> L(V.size());
    //	copy(V.begin(), V.end(), L.begin());
    //	assert(equal(V.begin(), V.end(), L.begin()));
    ///////////////////////////////////////////////////////////////////////////////
    // 헌데 그래도 튕김. - [levites]
    //	std::copy(it->second.begin(), it->second.end(), rTokenVector.begin());

    // 결국 이렇게.. - [levites]
    // 현재 사용하는 곳 : WorldEditor/Dialog/MapObjectPropertyPageBuilding.cpp
    CTokenVector &rSourceTokenVector = it->second;
    CTokenVector::iterator itor = rSourceTokenVector.begin();
    for (; itor != rSourceTokenVector.end(); ++itor)
    {
        rTokenVector.push_back(*itor);
    }

    return true;
}

void CProperty::PutString(const char *c_pszKey, const char *c_pszString)
{
    std::string stTempKey = c_pszKey;
    stl_lowers(stTempKey);

    // 이미 있는걸 지움
    CTokenVectorMap::iterator itor = m_stTokenMap.find(stTempKey);

    if (itor != m_stTokenMap.end())
        m_stTokenMap.erase(itor);

    CTokenVector tokenVector;
    tokenVector.push_back(c_pszString);

    m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, tokenVector));
}

void CProperty::PutVector(const char *c_pszKey, const CTokenVector &c_rTokenVector)
{
    std::string stTempKey = c_pszKey;
    stl_lowers(stTempKey);

    m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, c_rTokenVector));
}

void GetTimeString(char *str, time_t ct)
{
    struct tm tm;
    tm = *localtime(&ct);

    _snprintf(str, 15, "%04d%02d%02d%02d%02d%02d",
              tm.tm_year + 1900,
              tm.tm_mon + 1,
              tm.tm_mday,
              tm.tm_hour,
              tm.tm_min,
              tm.tm_sec);
}

bool CProperty::Save()
{
    storm::File fp;

    bsys::error_code ec;
    fp.Open(m_stFileName, ec,
            storm::AccessMode::kWrite,
            storm::CreationDisposition::kCreateAlways,
            storm::ShareMode::kNone,
            storm::UsageHint::kSequential);

    if (ec)
    {
        SPDLOG_ERROR("Failed to open {0} for writing with {1}",
                      m_stFileName, ec);
        return false;
    }

    if (!m_dwCRC)
    {
        char szCRC[MAX_PATH + 16 + 1];

        GetTimeString(szCRC, time(0));
        strcpy(szCRC + strlen(szCRC), m_stFileName.c_str());

        m_dwCRC = CPropertyManager::Instance().GetUniqueCRC(szCRC);
    }

    storm::WriteString(fp, "{0}\r\n{1}\r\n",
                       "YPRT",
                       m_dwCRC);

    for (const auto &p : m_stTokenMap)
    {
        auto &tokenVector = p.second;

        storm::WriteString(fp, ec, "{0}\t", p.first);

        for (const auto &str : tokenVector)
            storm::WriteString(fp, ec, "\t\"{0}\"", str);

        storm::WriteString(fp, ec, "\r\n");
    }

    return !ec;
}

bool CProperty::ReadFromMemory(const void *c_pvData, int iLen, const char *c_pszFileName)
{
    if (iLen < 4 + 2)
        return false;

    const char *pcData = (const char *)c_pvData;
    if (0 != std::memcmp(pcData, "YPRT", 4))
    {
        SPDLOG_ERROR("Property fourCC mismatch");
        return false;
    }

    pcData += sizeof(DWORD);
    iLen -= 4;

    if (*pcData == '\r')
    {
        ++pcData;
        --iLen;
    }

    if (*pcData == '\n')
    {
        ++pcData;
        --iLen;
    }

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind(std::string_view(pcData, iLen));

    storm::ParseNumber(textFileLoader.GetLineString(0), m_dwCRC);

    CTokenVector stTokenVector;
    for (DWORD i = 1; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLine(i, &stTokenVector))
            continue;

        stl_lowers(stTokenVector[0]);
        std::string stKey = stTokenVector[0];

        stTokenVector.erase(stTokenVector.begin());
        PutVector(stKey.c_str(), stTokenVector);
    }

    return true;
}

bool CProperty::ReadFromXML(const char *c_pszCRC)
{
    m_dwCRC = atoi(c_pszCRC);

    return true;
}

void CProperty::Clear()
{
    m_stTokenMap.clear();
}
