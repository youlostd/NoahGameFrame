#ifndef METIN2_CLIENT_ETERLIB_GRPSHADOWTEXTURE_HPP
#define METIN2_CLIENT_ETERLIB_GRPSHADOWTEXTURE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpTexture.h"

METIN2_BEGIN_NS

class CGraphicShadowTexture : public CGraphicTexture
{
public:
    CGraphicShadowTexture();
    virtual ~CGraphicShadowTexture();

    void Destroy();

    bool Create(int width, int height);

    void Begin();
    void End();
    void Set(int stage = 0) const;

    const Matrix &GetLightVPMatrixReference() const;
    LPDIRECT3DTEXTURE9 GetD3DTexture() const;

protected:
    void Initialize();

protected:
    Matrix m_d3dLightVPMatrix;
    D3DVIEWPORT9 m_d3dOldViewport;

    LPDIRECT3DTEXTURE9 m_lpd3dShadowTexture;
    LPDIRECT3DSURFACE9 m_lpd3dShadowSurface;
    LPDIRECT3DSURFACE9 m_lpd3dDepthSurface;

    LPDIRECT3DSURFACE9 m_lpd3dOldBackBufferSurface;
    LPDIRECT3DSURFACE9 m_lpd3dOldDepthBufferSurface;
};

METIN2_END_NS
#endif
