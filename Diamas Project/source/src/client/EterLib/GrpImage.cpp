#include "StdAfx.h"
#include "GrpImage.h"
#include "../EterBase/Stl.h"
#include <base/GroupText.hpp>
#include "ResourceManager.h"
#include <base/GroupTextTree.hpp>
#include <storm/memory/NewAllocator.hpp>
#undef LoadString

CGraphicImage::CGraphicImage()
{
    m_rect.bottom = m_rect.right = m_rect.top = m_rect.left = 0;
}

void CGraphicImage::Clear()
{
    //	SPDLOG_DEBUG("Image Destroy : %s\n", m_pszFileName);
    m_imageTexture.Destroy();
    memset(&m_rect, 0, sizeof(m_rect));
}

bool CGraphicImage::CreateDeviceObjects()
{
    return m_imageTexture.CreateDeviceObjects();
}

void CGraphicImage::DestroyDeviceObjects()
{
    m_imageTexture.DestroyDeviceObjects();
}

int CGraphicImage::GetWidth() const
{
    return m_rect.right - m_rect.left;
}

int CGraphicImage::GetHeight() const
{
    return m_rect.bottom - m_rect.top;
}

const CGraphicTexture &CGraphicImage::GetTextureReference() const
{
    if (m_subImage)
    {
        return m_subImage->GetTextureReference();
    }
    return m_imageTexture;
}

CGraphicTexture *CGraphicImage::GetTexturePointer()
{
    if (m_subImage)
    {
        return m_subImage->GetTexturePointer();
    }
    return &m_imageTexture;
}

const RECT &CGraphicImage::GetRectReference() const
{
    return m_rect;
}

bool CGraphicImage::Load(int iSize, const void *c_pvBuf)
{
    if (!c_pvBuf)
        return false;

    if (m_filename.size() >= 4 && m_filename.substr(m_filename.size() - 4) == ".sub")
    {
        return LoadSubImage(iSize, c_pvBuf);
    }
    return LoadRawImage(iSize, c_pvBuf);
}

bool CGraphicImage::LoadRawImage(int iSize, const void *c_pvBuf)
{
    m_imageTexture.SetFileName(m_filename.c_str());

    // 특정 컴퓨터에서 Unknown으로 '안'하면 튕기는 현상이 있음-_-; -비엽
    if (!m_imageTexture.CreateFromMemoryFile(iSize, c_pvBuf))
        return false;

    m_rect.left = 0;
    m_rect.top = 0;
    m_rect.right = m_imageTexture.GetWidth();
    m_rect.bottom = m_imageTexture.GetHeight();
    return true;
}

bool CGraphicImage::LoadSubImage(int iSize, const void *c_pvBuf)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    const std::string_view data(static_cast<const char *>(c_pvBuf), iSize);

    if (!reader.LoadString(data))
    {
        return false;
    }
    if (reader.GetProperty("title") != "subImage")
    {
        SPDLOG_ERROR("title is not subImage while loading {0}", m_filename.c_str());
        return false;
    }

    std::string fileName;
    if (reader.GetProperty("version") == "2.0")
    {
        int nPos = m_filename.find_last_of('/');
        if (nPos < 0)
        {
            fileName.append(reader.GetProperty("image").data(), reader.GetProperty("image").length());
        }
        else
        {
            fileName.append(m_filename, 0, nPos + 1);
            fileName.append(reader.GetProperty("image").data(), reader.GetProperty("image").length());
        }
    }
    else if (reader.GetProperty("version") == "3.0")
    {
        fileName = "D:/Ymir Work/Effect/";
        fileName.append(reader.GetProperty("image").data(), reader.GetProperty("image").length());
    }
    else
    {
        fileName = "D:/Ymir Work/UI/";
        fileName.append(reader.GetProperty("image").data(), reader.GetProperty("image").length());
    }
    assert(fileName != m_filename);
    m_subImage = CResourceManager::instance().LoadResource<CGraphicImage>(fileName);

    if (!m_subImage)
    {
        SPDLOG_WARN("Failed to load {} while loading {}", fileName.c_str(), GetFileName());
        return false;
    }

    storm::ParseNumber(reader.GetProperty("left"), m_rect.left);
    storm::ParseNumber(reader.GetProperty("right"), m_rect.right);
    storm::ParseNumber(reader.GetProperty("top"), m_rect.top);
    storm::ParseNumber(reader.GetProperty("bottom"), m_rect.bottom);

    SPDLOG_DEBUG("Sucessfully loaded {0}", m_filename.c_str());
    return true;
}
