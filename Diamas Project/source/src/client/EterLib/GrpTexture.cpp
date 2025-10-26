#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpTexture.h"

#include <d3dx9tex.h>

#include "StateManager.h"

void CGraphicTexture::DestroyDeviceObjects()
{
    M2_SAFE_RELEASE_CHECK(m_lpd3dTexture);
}

void CGraphicTexture::Destroy()
{
    DestroyDeviceObjects();

    Initialize();
}

void CGraphicTexture::Initialize()
{
    m_lpd3dTexture = NULL;
    m_width = 0;
    m_height = 0;
    m_bEmpty = true;
}

bool CGraphicTexture::IsEmpty() const
{
    return m_bEmpty;
}

void CGraphicTexture::SetTextureStage(int stage) const
{
    assert(ms_lpd3dDevice != NULL);
    STATEMANAGER.SetTexture(stage, m_lpd3dTexture);
}

LPDIRECT3DTEXTURE9 CGraphicTexture::GetD3DTexture() const
{
    return m_lpd3dTexture;
}

bool CGraphicTexture::SetSubData(unsigned int level, unsigned int left, unsigned int top, unsigned int width,
                                 unsigned int height, unsigned int pitch, void *pData, D3DFORMAT pf)
{
    if (NULL == m_lpd3dTexture)
    {
        return false;
    }

    //
    IDirect3DSurface9 *pSurface = 0;
    m_lpd3dTexture->GetSurfaceLevel(0, &pSurface);
    if (NULL == pSurface)
    {
        return false;
    }

    //
    RECT rc = {0, 0, width, height};
    RECT drc = {left, top, width + left, height + top};

    //
    HRESULT hr = D3DXLoadSurfaceFromMemory(pSurface, 0, &drc, pData, pf,
                                           pitch, 0, &rc, D3DX_DEFAULT, 0);
    if (FAILED(hr))
    {
        return false;
    }
    pSurface->Release();

    //
    return false;
}

int CGraphicTexture::GetWidth() const
{
    return m_width;
}

int CGraphicTexture::GetHeight() const
{
    return m_height;
}

CGraphicTexture::CGraphicTexture()
{
    Initialize();
}

CGraphicTexture::~CGraphicTexture()
{
}
