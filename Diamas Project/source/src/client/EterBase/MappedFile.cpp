#include "StdAfx.h"
#include "MappedFile.h"
#include "Debug.h"

CMappedFile::CMappedFile()
    : m_hFM(NULL)
      , m_lpMapData(NULL)
      , m_dataOffset(0)
      , m_mapSize(0)
      , m_seekPosition(0)
      , m_pbBufLinkData(NULL)
      , m_dwBufLinkSize(0)
{
}

CMappedFile::~CMappedFile()
{
    Destroy();
}

bool CMappedFile::Create(const std::string &filename)
{
    Destroy();

    if (!CFileBase::Create(filename, FILEMODE_READ))
        return false;

    return Map(0, 0) > 0;
}

void CMappedFile::Create(size_t size)
{
    m_ownedBuffer.reset(new uint8_t[size]);

    m_pbBufLinkData = m_ownedBuffer.get();
    m_dwBufLinkSize = size;
}

const void *CMappedFile::Get() const
{
    return m_pbBufLinkData;
}

void CMappedFile::Destroy()
{
    if (NULL != m_lpMapData)
    {
        Unmap(m_lpMapData);
        m_lpMapData = NULL;
    }

    if (NULL != m_hFM)
    {
        CloseHandle(m_hFM);
        m_hFM = NULL;
    }

    m_pbBufLinkData = NULL;
    m_dwBufLinkSize = 0;

    m_seekPosition = 0;
    m_dataOffset = 0;
    m_mapSize = 0;

    CFileBase::Destroy();
}

uint32_t CMappedFile::Size() const
{
    return m_dwBufLinkSize;
}

int CMappedFile::Map(int offset, int size)
{
    m_dataOffset = offset;

    if (size == 0)
        m_mapSize = m_dwSize;
    else
        m_mapSize = size;

    if (m_dataOffset + m_mapSize > m_dwSize)
        return NULL;

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);
    uint32_t dwSysGran = SysInfo.dwAllocationGranularity;
    uint32_t dwFileMapStart = (m_dataOffset / dwSysGran) * dwSysGran;
    uint32_t dwMapViewSize = (m_dataOffset % dwSysGran) + m_mapSize;
    INT iViewDelta = m_dataOffset - dwFileMapStart;

    m_hFM = CreateFileMapping(m_hFile,                  // handle
                              NULL,                     // security
                              PAGE_READONLY,            // flProtect
                              0,                        // high
                              m_dataOffset + m_mapSize, // low
                              NULL);                    // name

    if (!m_hFM)
    {
        OutputDebugString("CMappedFile::Map !m_hFM\n");
        return 0;
    }

    m_lpMapData = MapViewOfFile(m_hFM,
                                FILE_MAP_READ,
                                0,
                                dwFileMapStart,
                                dwMapViewSize);

    if (!m_lpMapData)
    {
        SPDLOG_ERROR("CMappedFile::Map !m_lpMapData {0}", GetLastError());
        return 0;
    }

    m_seekPosition = 0;
    m_dwBufLinkSize = m_mapSize;
    m_pbBufLinkData = static_cast<uint8_t *>(m_lpMapData) + iViewDelta;
    return m_mapSize;
}

int CMappedFile::Seek(uint32_t offset, int iSeekType)
{
    switch (iSeekType)
    {
    case SEEK_TYPE_BEGIN:
        if (offset > m_dwSize)
            offset = m_dwSize;

        m_seekPosition = offset;
        break;

    case SEEK_TYPE_CURRENT:
        m_seekPosition = std::min(m_seekPosition + offset, Size());
        break;

    case SEEK_TYPE_END:
        m_seekPosition = (Size() >= offset) ? Size() - offset : 0;
        break;
    }

    return m_seekPosition;
}

bool CMappedFile::Read(void *dest, uint32_t bytes)
{
    if (m_seekPosition + bytes > Size())
        return FALSE;

    memcpy(dest, GetCurrentSeekPoint(), bytes);
    m_seekPosition += bytes;
    return TRUE;
}

bool CMappedFile::Read(size_t seekPos, void *dest, uint32_t bytes)
{
    if (seekPos + bytes > Size())
        return FALSE;

    memcpy(dest, m_pbBufLinkData + seekPos, bytes);
    return TRUE;
}

uint32_t CMappedFile::GetSeekPosition(void)
{
    return m_seekPosition;
}

uint8_t *CMappedFile::GetCurrentSeekPoint()
{
    return m_pbBufLinkData + m_seekPosition;
}

void CMappedFile::Unmap(LPCVOID data)
{
    if (!UnmapViewOfFile(data))
        SPDLOG_ERROR("CMappedFile::Unmap - Error");
}
