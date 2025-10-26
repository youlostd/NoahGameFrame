#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpBase.h"

/*!
 * \brief
 * Interface of texture.
 */
class CGraphicTexture : public CGraphicBase
{
public:
    virtual bool IsEmpty() const;

    int GetWidth() const;
    int GetHeight() const;

    void SetTextureStage(int stage) const;
    LPDIRECT3DTEXTURE9 GetD3DTexture() const;
    bool SetSubData(unsigned level, unsigned left, unsigned top, unsigned width, unsigned height, unsigned pitch,
                    void *pData, D3DFORMAT pf);
    void DestroyDeviceObjects();

protected:
    CGraphicTexture();
    ~CGraphicTexture() override;

    virtual void Destroy();
    virtual void Initialize();

protected:
    bool m_bEmpty;

    uint32_t m_width;
    uint32_t m_height;

    LPDIRECT3DTEXTURE9 m_lpd3dTexture;
};
