#include "StdAfx.h"
#include "FileBase.h"

CFileBase::CFileBase()
    : m_mode(0), m_hFile(nullptr)
      , m_dwSize(0)
{
}

CFileBase::~CFileBase()
{
    Destroy();
}

const std::string &CFileBase::GetFileName() const
{
    return m_filename;
}

void CFileBase::Destroy()
{
    Close();
    m_dwSize = 0;
}

void CFileBase::Close()
{
    if (m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = nullptr;
    }
}

bool CFileBase::Create(const std::string &filename, EFileMode mode)
{
    Destroy();

    m_filename = filename;

    uint32_t dwMode, dwShareMode = FILE_SHARE_READ;

    if (mode == FILEMODE_WRITE)
    {
        dwMode = GENERIC_READ | GENERIC_WRITE;
        dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
    else
        dwMode = GENERIC_READ;

    m_hFile = CreateFile(filename.c_str(),                                    // name of the file
                         dwMode,                                              // desired access
                         dwShareMode,                                         // share mode
                         nullptr,                                             // security attributes
                         mode == FILEMODE_READ ? OPEN_EXISTING : OPEN_ALWAYS, // creation disposition
                         FILE_ATTRIBUTE_NORMAL,                               // flags and attr
                         nullptr);                                            // template file

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_dwSize = GetFileSize(m_hFile, nullptr);
        m_mode = mode;
        return true;
    }

    m_hFile = nullptr;
    return false;
}

uint32_t CFileBase::Size() const
{
    return m_dwSize;
}

void CFileBase::SeekCur(uint32_t size)
{
    SetFilePointer(m_hFile, size, nullptr, FILE_CURRENT);
}

void CFileBase::Seek(uint32_t offset)
{
    if (offset > m_dwSize)
        offset = m_dwSize;

    SetFilePointer(m_hFile, offset, nullptr, FILE_BEGIN);
}

uint32_t CFileBase::GetPosition() const
{
    return SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);
}

bool CFileBase::Write(const void *src, uint32_t bytes)
{
    uint32_t written = 0;
    bool ret = WriteFile(m_hFile, src, bytes, (LPDWORD)&written, nullptr);
    if (!ret || written != bytes)
        return false;

    m_dwSize = GetFileSize(m_hFile, nullptr);
    return true;
}

bool CFileBase::Read(void *dest, uint32_t bytes) const
{
    uint32_t read = 0;
    return ReadFile(m_hFile, dest, bytes, (LPDWORD)&read, nullptr) && read == bytes;
}
