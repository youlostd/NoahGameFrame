#ifndef METIN2_CLIENT_GAMELIB_SNOWENVIRONMENT_H
#define METIN2_CLIENT_GAMELIB_SNOWENVIRONMENT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpScreen.h"

class CSnowParticle;

class CSnowEnvironment : public CScreen
{
  public:
    CSnowEnvironment();
    virtual ~CSnowEnvironment();

    bool Create();
    void Destroy();

    void Enable();
    void Disable();

    void Update(const Vector3 &c_rv3Pos);
    void Deform();
    void Render();

  protected:
    void __Initialize();
    bool __CreateBlurTexture();
    bool __CreateGeometry();
    void __BeginBlur();
    void __ApplyBlur();

  protected:
    LPDIRECT3DSURFACE9 m_lpOldSurface;
    LPDIRECT3DSURFACE9 m_lpOldDepthStencilSurface;

    LPDIRECT3DTEXTURE9 m_lpSnowTexture;
    LPDIRECT3DSURFACE9 m_lpSnowRenderTargetSurface;
    LPDIRECT3DSURFACE9 m_lpSnowDepthSurface;

    LPDIRECT3DTEXTURE9 m_lpAccumTexture;
    LPDIRECT3DSURFACE9 m_lpAccumRenderTargetSurface;
    LPDIRECT3DSURFACE9 m_lpAccumDepthSurface;

    VertexBufferHandle m_pVB;
    IndexBufferHandle m_pIB;

    Vector3 m_v3Center;

    uint16_t m_wBlurTextureSize;
    std::unique_ptr<CGraphicImageInstance> m_pImageInstance;
    std::vector<std::unique_ptr<CSnowParticle>> m_kVct_pkParticleSnow;

    uint32_t m_dwParticleMaxNum;
    bool m_bBlurEnable;

    bool m_bSnowEnable;
    float m_lastTime;
};
#endif /* METIN2_CLIENT_GAMELIB_SNOWENVIRONMENT_H */
