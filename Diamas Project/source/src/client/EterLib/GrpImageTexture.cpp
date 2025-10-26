#include "GrpImageTexture.h"
#include "../EterBase/DDSTextureLoader.h"
#include "../EterBase/WICTextureLoader9.hpp"
#include "../EterBase/dds.h"
#include "../eterBase/MappedFile.h"
#include "../eterBase/Utils.h"
#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include <storm/io/View.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <base/stb_image.h>

#include <d3dx9tex.h>

bool CGraphicImageTexture::Lock(int *pRetPitch, void **ppRetPixels, int level)
{
    D3DLOCKED_RECT lockedRect;
    if (FAILED(m_lpd3dTexture->LockRect(level, &lockedRect, NULL, 0)))
        return false;

    *pRetPitch = lockedRect.Pitch;
    *ppRetPixels = (void *)lockedRect.pBits;
    return true;
}

void CGraphicImageTexture::Unlock(int level)
{
    assert(m_lpd3dTexture != NULL);
    m_lpd3dTexture->UnlockRect(level);
}

void CGraphicImageTexture::Initialize()
{
    CGraphicTexture::Initialize();
    m_d3dFmt = D3DFMT_UNKNOWN;
    m_stFileName = "";
}

void CGraphicImageTexture::Destroy()
{
    CGraphicTexture::Destroy();

    Initialize();
}

bool CGraphicImageTexture::CreateDeviceObjects()
{
    assert(ms_lpd3dDevice != NULL);
    assert(m_lpd3dTexture == NULL);

    if (m_stFileName.empty())
    {
        // 폰트 텍스쳐
        if (FAILED(ms_lpd3dDevice->CreateTexture(m_width, m_height, 1, 0, m_d3dFmt, D3DPOOL_MANAGED, &m_lpd3dTexture,
                                                 NULL)))
            return false;
    }
    else
    {
        auto fp = GetVfs().Open(m_stFileName.c_str(), kVfsOpenFullyBuffered);

        if (!fp)
            return nullptr;

        const auto size = fp->GetSize();

        storm::View data(storm::GetDefaultAllocator());
        fp->GetView(0, data, size);

        return CreateFromMemoryFile(size, data.GetData());
    }

    m_bEmpty = false;
    return true;
}

bool CGraphicImageTexture::Create(UINT width, UINT height, D3DFORMAT d3dFmt)
{
    assert(ms_lpd3dDevice != NULL);
    Destroy();

    m_width = width;
    m_height = height;
    m_d3dFmt = d3dFmt;

    return CreateDeviceObjects();
}

void CGraphicImageTexture::CreateFromTexturePointer(const CGraphicTexture *c_pSrcTexture) {}

bool CGraphicImageTexture::CreateWithStbImage(UINT bufSize, const void *c_pvBuf)
{

    struct free_deleter
    {
        void operator()(void *p) const { free(p); }
    };
    using c_ptr = std::unique_ptr<uint8_t[], free_deleter>;

    IDirect3DTexture9 *ret;
    auto *data_uc = static_cast<const stbi_uc *>(c_pvBuf);

    int Width{}, Height{}, comp{};
    auto Levels = 1;
    auto Usage = 0;

    c_ptr tex_mem = c_ptr{stbi_load_from_memory(data_uc, bufSize, &Width, &Height, &comp, STBI_rgb_alpha)};

    auto *tex_ptr = tex_mem.get();

    if (!(Width && Height && comp && tex_mem))
    {
        m_bEmpty = true;
        return false;
    }

    D3DFORMAT format;
    if (comp == 4)
        format = D3DFMT_A8R8G8B8;
    else
        format = D3DFMT_X8R8G8B8;

    if (FAILED(ms_lpd3dDevice->CreateTexture(Width, Height, Levels, Usage, format, D3DPOOL_MANAGED, &ret, nullptr)))
        return false;

    D3DLOCKED_RECT LockedRect;
    if (FAILED(ret->LockRect(0, &LockedRect, nullptr, 0)))
    {
        return false;
    }
    auto *ptr = static_cast<unsigned char *>(LockedRect.pBits);
    for (int i{}; i < Width * Height; ++i)
    {
#define SET(offset)                                                                                                    \
    *ptr = *(tex_ptr + offset);                                                                                        \
    ++ptr
        // RGBA -> BGRA
        SET(2);
        SET(1);
        SET(0);
        SET(3);
        tex_ptr += 4;
#undef SET
    }
    if (FAILED(ret->UnlockRect(0)))
    {
        m_bEmpty = true;
        return false;
    }

    m_width = Width;
    m_height = Height;
    m_d3dFmt = format;
    m_bEmpty = false;
    m_lpd3dTexture = ret;
    return true;
}

bool CGraphicImageTexture::CreateFromMemoryFile(UINT bufSize, const void *c_pvBuf)
{
    if (CGraphicBase::m_deviceNeedsReset)
        return false;

    assert(ms_lpd3dDevice != NULL);
    assert(m_lpd3dTexture == NULL);

    if (!m_stFileName.empty() && CFileNameHelper::GetExtension(m_stFileName) == "dds" ||
        CFileNameHelper::GetExtension(m_stFileName) == "DDS")
    {

        HRESULT hr = CreateDDSTextureFromMemoryEx(ms_lpd3dDevice, (const uint8_t *)c_pvBuf, bufSize, 0, D3DPOOL_MANAGED,
                                                  false, &m_lpd3dTexture);
        if (FAILED(hr))
        {
            D3DXIMAGE_INFO imageInfo;
            hr = D3DXCreateTextureFromFileInMemoryEx(
                ms_lpd3dDevice, c_pvBuf, bufSize, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, m_d3dFmt,
                D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0xffff00ff, &imageInfo, NULL, &m_lpd3dTexture);
            if (FAILED(hr))
            {
                spdlog::error("{0}: failed to create texture with {1}", m_stFileName, (hr));
                return false;
            }
            m_bEmpty = false;

            m_width = imageInfo.Width;
            m_height = imageInfo.Height;
            return true;
        }

        D3DSURFACE_DESC desc;
        m_lpd3dTexture->GetLevelDesc(0, &desc);

        m_width = desc.Width;
        m_height = desc.Height;
        m_d3dFmt = desc.Format;
        m_bEmpty = false;
        return true;
    }
    else
    {

        if (!CreateWithStbImage(bufSize, c_pvBuf))
        {

            auto hr = DirectX::CreateWICTextureFromMemoryEx(ms_lpd3dDevice, (const uint8_t *)c_pvBuf, bufSize, 0, 0,
                                                            D3DPOOL_MANAGED, 0, &m_lpd3dTexture);
            if (hr == S_OK)
            {
                D3DSURFACE_DESC desc;
                m_lpd3dTexture->GetLevelDesc(0, &desc);
                m_bEmpty = false;
                m_width = desc.Width;
                m_height = desc.Height;
                m_d3dFmt = desc.Format;
                return true;
            }
            else
            {
                m_bEmpty = true;

                spdlog::error("{0}: failed to create texture with {1}", m_stFileName,
                              std::system_category().message(hr));
                return false;
            }
        }
    }

    m_bEmpty = false;
    return true;
}

void CGraphicImageTexture::SetFileName(const char *c_szFileName) { m_stFileName = c_szFileName; }

bool CGraphicImageTexture::CreateFromDiskFile(const char *c_szFileName, D3DFORMAT d3dFmt)
{
    Destroy();
    SetFileName(c_szFileName);

    m_d3dFmt = d3dFmt;
    return CreateDeviceObjects();
}

CGraphicImageTexture::CGraphicImageTexture() { CGraphicImageTexture::Initialize(); }

CGraphicImageTexture::~CGraphicImageTexture() { Destroy(); }
