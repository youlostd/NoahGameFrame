#pragma once

#include "GrpBase.h"
#include "GrpTexture.h"

#include <wrl/client.h>

/*
D3DXProtectRenderTarget
Written by Matthew Fisher

The D3DXProtectRenderTarget class is used to simplify preserving a device's current render target and depth buffer
so new ones can easily be overlayed, and then the original render target and depth buffer restored.
CGraphicRenderTargetTexture is a texture that can be used as a render target.  It has its own associated depth buffer.

On D3DX8 is not tested properly.
*/
class D3DXProtectRenderTarget
{
public:
    D3DXProtectRenderTarget(LPDIRECT3DDEVICE9 Device, bool ProtectDepthBuffer, bool Enabled)

    {
        _Enabled = Enabled;
        _RenderTarget = NULL;
        _DepthStencilSurface = NULL;
        if (_Enabled)
        {
            _ProtectDepthBuffer = ProtectDepthBuffer;
            Device->GetViewport(&_Viewport);
            Device->GetRenderTarget(0, &_RenderTarget);

            if (ProtectDepthBuffer)
            {
                Device->GetDepthStencilSurface(&_DepthStencilSurface);
            }
            _Device = Device;
        }
    }

    ~D3DXProtectRenderTarget()
    {
        if (_Enabled)
        {
            if (_RenderTarget)
            {
                _Device->SetRenderTarget(0, _RenderTarget);
            }
            _Device->SetRenderTarget(1, NULL);
            _Device->SetRenderTarget(2, NULL);
            _Device->SetRenderTarget(3, NULL);

            if (_ProtectDepthBuffer && _DepthStencilSurface != NULL)
            {
                _Device->SetDepthStencilSurface(_DepthStencilSurface);
            }
            _Device->SetViewport(&_Viewport);

            if (_RenderTarget != NULL)
            {
                _RenderTarget->Release();
            }
            if (_ProtectDepthBuffer && _DepthStencilSurface != NULL)
            {
                _DepthStencilSurface->Release();
            }
        }
    }

private:
    LPDIRECT3DDEVICE9 _Device;
    LPDIRECT3DSURFACE9 _RenderTarget;
    LPDIRECT3DSURFACE9 _DepthStencilSurface;
    D3DVIEWPORT9 _Viewport;

    bool _Enabled;
    bool _ProtectDepthBuffer;
};

/*!
 * \brief
 * Interface of texture.
 */
class CGraphicRenderTargetTexture : public CGraphicBase
{
public:
    virtual ~CGraphicRenderTargetTexture();
public:
    bool Create(int width, int height, D3DFORMAT texFormat);

    bool SetRenderTarget();
    void ResetRenderTarget();

    void SetRenderingRect(RECT *rect);
    RECT *GetRenderingRect();


    bool OnRestoreDevice();
    void OnLostDevice();

    void Render() const;

protected:
    LPDIRECT3DTEXTURE9 m_texture = nullptr;
    LPDIRECT3DSURFACE9 m_surface = nullptr;
    LPDIRECT3DSURFACE9 m_depthStencil = nullptr; 

    LPDIRECT3DSURFACE9 m_backupSurface = nullptr;
    LPDIRECT3DSURFACE9 m_backupDepthStencil = nullptr;
    D3DFORMAT m_d3dFormat = D3DFMT_UNKNOWN;
    RECT m_renderRect = {};
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_init = false;
};
