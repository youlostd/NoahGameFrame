#include "StdAfx.h"
#include "BlockTexture.h"

#include "Engine.hpp"
#include "GrpBase.h"
#include "GrpDevice.h"
#include "GrpDib.h"
#include "../eterbase/Stl.h"
#include "../eterlib/StateManager.h"

void CBlockTexture::SetClipRect(const RECT &c_rRect)
{
    m_bClipEnable = true;
    m_clipRect = c_rRect;
}

void CBlockTexture::Render(int ix, int iy)
{
    int isx = ix + m_rect.left;
    int isy = iy + m_rect.top;
    int iex = ix + m_rect.left + m_dwWidth;
    int iey = iy + m_rect.top + m_dwHeight;

    float su = 0.0f;
    float sv = 0.0f;
    float eu = 1.0f;
    float ev = 1.0f;

    if (m_bClipEnable)
    {
        if (isx > m_clipRect.right)
            return;
        if (iex < m_clipRect.left)
            return;

        if (isy > m_clipRect.bottom)
            return;
        if (iey < m_clipRect.top)
            return;

        if (m_clipRect.left > isx)
        {
            int idx = m_clipRect.left - isx;
            isx += idx;
            su += float(idx) / float(m_dwWidth);
        }
        if (iex > m_clipRect.right)
        {
            int idx = iex - m_clipRect.right;
            iex -= idx;
            eu -= float(idx) / float(m_dwWidth);
        }

        if (m_clipRect.top > isy)
        {
            int idy = m_clipRect.top - isy;
            isy += idy;
            sv += float(idy) / float(m_dwHeight);
        }
        if (iey > m_clipRect.bottom)
        {
            int idy = iey - m_clipRect.bottom;
            iey -= idy;
            ev -= float(idy) / float(m_dwHeight);
        }
    }

    TPDTVertex vertices[4];
    vertices[0].position.x = isx - 0.5f;
    vertices[0].position.y = isy - 0.5f;
    vertices[0].position.z = 0.0f;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = 0xffffffff;

    vertices[1].position.x = iex - 0.5f;
    vertices[1].position.y = isy - 0.5f;
    vertices[1].position.z = 0.0f;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = 0xffffffff;

    vertices[2].position.x = isx - 0.5f;
    vertices[2].position.y = iey - 0.5f;
    vertices[2].position.z = 0.0f;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = 0xffffffff;

    vertices[3].position.x = iex - 0.5f;
    vertices[3].position.y = iey - 0.5f;
    vertices[3].position.z = 0.0f;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = 0xffffffff;

    STATEMANAGER.SetTexture(0, m_lpd3dTexture);
    STATEMANAGER.SetTexture(1, NULL);
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, vertices, 4);

}

void CBlockTexture::InvalidateRect(const RECT &c_rsrcRect)
{
    RECT dstRect = m_rect;
    if (c_rsrcRect.right < dstRect.left ||
        c_rsrcRect.left > dstRect.right ||
        c_rsrcRect.bottom < dstRect.top ||
        c_rsrcRect.top > dstRect.bottom)
    {
        SPDLOG_ERROR("InvalidateRect() - Strange rect");
        return;
    }

    // DIBBAR_LONGSIZE_BUGFIX
    const RECT clipRect = {
        std::max<LONG>(c_rsrcRect.left - dstRect.left, 0),
        std::max<LONG>(c_rsrcRect.top - dstRect.top, 0),
        std::min<LONG>(c_rsrcRect.right - dstRect.left, dstRect.right - dstRect.left),
        std::min<LONG>(c_rsrcRect.bottom - dstRect.top, dstRect.bottom - dstRect.top),
    };
    // END_OF_DIBBAR_LONGSIZE_BUGFIX

    uint32_t *pdwSrc;
    pdwSrc = (uint32_t *)m_pDIB->GetPointer();
    pdwSrc += dstRect.left + dstRect.top * m_pDIB->GetWidth();

    D3DLOCKED_RECT lockedRect;
    if (FAILED(m_lpd3dTexture->LockRect(0, &lockedRect, &clipRect, 0)))
    {
        SPDLOG_ERROR("InvalidateRect() - Failed to LockRect");
        return;
    }

    int iclipWidth = clipRect.right - clipRect.left;
    int iclipHeight = clipRect.bottom - clipRect.top;
    uint32_t *pdwDst = (uint32_t *)lockedRect.pBits;
    uint32_t dwDstWidth = lockedRect.Pitch >> 2;
    uint32_t dwSrcWidth = m_pDIB->GetWidth();
    for (int i = 0; i < iclipHeight; ++i)
    {
        for (int i = 0; i < iclipWidth; ++i)
        {
            if (pdwSrc[i])
                pdwDst[i] = pdwSrc[i] | 0xff000000;
            else
                pdwDst[i] = 0;
        }
        pdwDst += dwDstWidth;
        pdwSrc += dwSrcWidth;
    }

    m_lpd3dTexture->UnlockRect(0);
}

bool CBlockTexture::Create(CGraphicDib *pDIB, const RECT &c_rRect, uint32_t dwWidth, uint32_t dwHeight)
{
    if (FAILED(
            ms_lpd3dDevice->CreateTexture(dwWidth, dwHeight, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_lpd3dTexture,
                NULL))
    )
    {
        SPDLOG_ERROR("Failed to create block texture {0}, {1}", dwWidth, dwHeight);
        return false;
    }

    m_pDIB = pDIB;
    m_rect = c_rRect;
    m_dwWidth = dwWidth;
    m_dwHeight = dwHeight;
    m_bClipEnable = false;

    return true;
}

CBlockTexture::CBlockTexture()
{
    m_pDIB = NULL;
    m_lpd3dTexture = NULL;
}

CBlockTexture::~CBlockTexture()
{
    M2_SAFE_RELEASE_CHECK(m_lpd3dTexture);
    m_lpd3dTexture = NULL;
}
