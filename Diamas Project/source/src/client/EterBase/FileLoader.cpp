#include "StdAfx.h"
#include "FileLoader.h"
#include <cassert>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <storm/Tokenizer.hpp>

CMemoryTextFileLoader::CMemoryTextFileLoader()
= default;

CMemoryTextFileLoader::~CMemoryTextFileLoader()
= default;

bool CMemoryTextFileLoader::SplitLineByTab(uint32_t dwLine, CTokenVector *pstTokenVector)
{
    pstTokenVector->reserve(10);
    pstTokenVector->clear();

    const std::string &c_rstLine = GetLineString(dwLine);
    const int c_iLineLength = c_rstLine.length();

    if (0 == c_iLineLength)
        return false;

    int basePos = 0;

    do
    {
        const int beginPos = c_rstLine.find_first_of('\t', basePos);

        pstTokenVector->push_back(c_rstLine.substr(basePos, beginPos - basePos));

        basePos = beginPos + 1;
    }
    while (basePos < c_iLineLength && basePos > 0);

    return true;
}

int CMemoryTextFileLoader::SplitLine2(uint32_t dwLine, CTokenVector *pstTokenVector, const char *c_szDelimeter)
{
    pstTokenVector->reserve(10);
    pstTokenVector->clear();

    //boost::split(pstTokenVector, GetLineString(dwLine), boost::is_any_of("\t"));

    const auto &c_rstLine = GetLineString(dwLine);

    uint32_t basePos = 0;

    do
    {
        int beginPos = c_rstLine.find_first_not_of(c_szDelimeter, basePos);

        if (beginPos < 0)
            return -1;

        int endPos;

        if (c_rstLine[beginPos] == '"')
        {
            ++beginPos;
            endPos = c_rstLine.find_first_of('\"', beginPos);

            if (endPos < 0)
                return -2;

            basePos = endPos + 1;
        }
        else
        {
            endPos = c_rstLine.find_first_of(c_szDelimeter, beginPos);
            basePos = endPos;
        }

        pstTokenVector->push_back(c_rstLine.substr(beginPos, endPos - beginPos));

        // �߰� �ڵ�. �ǵڿ� ���� �ִ� ��츦 üũ�Ѵ�. - [levites]
        if (int(c_rstLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
            break;
    }
    while (basePos < c_rstLine.length());

    return 0;
}

bool CMemoryTextFileLoader::SplitLine(uint32_t dwLine, CTokenVector *pstTokenVector, const char *c_szDelimeter)
{
    pstTokenVector->reserve(10);
    pstTokenVector->clear();

    const std::string &c_rstLine = GetLineString(dwLine);

    uint32_t basePos = 0;

    do
    {
        int beginPos = c_rstLine.find_first_not_of(c_szDelimeter, basePos);
        if (beginPos < 0)
            return false;

        int endPos;

        if (c_rstLine[beginPos] == '"')
        {
            ++beginPos;
            endPos = c_rstLine.find_first_of('\"', beginPos);

            if (endPos < 0)
                return false;

            basePos = endPos + 1;
        }
        else
        {
            endPos = c_rstLine.find_first_of(c_szDelimeter, beginPos);
            basePos = endPos;
        }

        pstTokenVector->push_back(c_rstLine.substr(beginPos, endPos - beginPos));

        // �߰� �ڵ�. �ǵڿ� ���� �ִ� ��츦 üũ�Ѵ�. - [levites]
        if (int(c_rstLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
            break;
    }
    while (basePos < c_rstLine.length());

    return true;
}

uint32_t CMemoryTextFileLoader::GetLineCount() const
{
    return m_stLineVector.size();
}

bool CMemoryTextFileLoader::CheckLineIndex(uint32_t dwLine) const
{
    return dwLine < m_stLineVector.size();
}

const std::string &CMemoryTextFileLoader::GetLineString(uint32_t dwLine)
{
    assert(CheckLineIndex(dwLine));
    return m_stLineVector[dwLine];
}

void CMemoryTextFileLoader::Bind(std::string_view data)
{
    m_stLineVector.reserve(128);
    m_stLineVector.clear();

    storm::Tokenize(data, "\r\n", m_stLineVector);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int CMemoryFileLoader::GetSize() const
{
    return m_size;
}

int CMemoryFileLoader::GetPosition() const
{
    return m_pos;
}

bool CMemoryFileLoader::IsReadableSize(int size) const
{
    return m_pos + size <= m_size;
}

bool CMemoryFileLoader::Read(int size, void *pvDst)
{
    if (!IsReadableSize(size))
        return false;

    memcpy(pvDst, GetCurrentPositionPointer(), size);
    m_pos += size;
    return true;
}

const char *CMemoryFileLoader::GetCurrentPositionPointer() const
{
    assert(m_pcBase != NULL);
    return (m_pcBase + m_pos);
}

CMemoryFileLoader::CMemoryFileLoader(int size, const void *c_pvMemoryFile)
{
    assert(c_pvMemoryFile != NULL);

    m_pos = 0;
    m_size = size;
    m_pcBase = static_cast<const char *>(c_pvMemoryFile);
}

CMemoryFileLoader::~CMemoryFileLoader()
= default;

//////////////////////////////////////////////////////////////////////////////////////////////////
int CDiskFileLoader::GetSize() const
{
    return m_size;
}

bool CDiskFileLoader::Read(int size, void *pvDst) const
{
    assert(m_fp != NULL);

    const int ret = fread(pvDst, size, 1, m_fp);

    return ret > 0;
}

bool CDiskFileLoader::Open(const char *c_szFileName)
{
    Close();

    if (!c_szFileName[0])
        return false;

    m_fp = fopen(c_szFileName, "rbe");

    if (!m_fp)
        return false;

    fseek(m_fp, 0, SEEK_END);
    m_size = ftell(m_fp);
    fseek(m_fp, 0, SEEK_SET);
    return true;
}

void CDiskFileLoader::Close()
{
    if (m_fp)
        fclose(m_fp);

    Initialize();
}

void CDiskFileLoader::Initialize()
{
    m_fp = NULL;
    m_size = 0;
}

CDiskFileLoader::CDiskFileLoader()
{
    Initialize();
}

CDiskFileLoader::~CDiskFileLoader()
{
    Close();
}
