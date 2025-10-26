#include "GrpRenderTargetTexture.h"
#include "../EterBase/Stl.h"
#include "StdAfx.h"

#include "../EterBase/vk.h"
#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"

bool CGraphicRenderTargetTexture::Create(int width, int height,
                                         D3DFORMAT texFormat)
{
    OnLostDevice();

    m_height = height;
    m_width = width;
    m_d3dFormat = texFormat;

    return OnRestoreDevice();
}

void CGraphicRenderTargetTexture::OnLostDevice()
{
    M2_SAFE_RELEASE(m_texture, 0);
    M2_SAFE_RELEASE(m_surface, 0);
    M2_SAFE_RELEASE(m_depthStencil, 0);
    M2_SAFE_RELEASE(m_backupSurface, 0);
    M2_SAFE_RELEASE(m_backupDepthStencil, 0);
    m_init = false;
}

bool CGraphicRenderTargetTexture::OnRestoreDevice()
{
    if (FAILED(ms_lpd3dDevice->CreateTexture(
            m_width, m_height, 1, D3DUSAGE_RENDERTARGET, m_d3dFormat,
            D3DPOOL_DEFAULT, &m_texture, NULL))) {
        SPDLOG_ERROR("CGraphicRenderTargetTexture::CreateRenderTexture Failed");
        return false;
    }

    if (FAILED(m_texture->GetSurfaceLevel(0, &m_surface))) {
        SPDLOG_ERROR("CGraphicRenderTargetTexture::CreateRenderTexture "
                     "GetSurfaceLevel Failed");
        M2_SAFE_RELEASE(m_texture, 0);
        return false;
    }

    if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(
            m_width, m_height, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, false,
            &m_depthStencil, NULL))) {
        SPDLOG_ERROR("CGraphicRenderTargetTexture::CreateRenderTexture Failed "
                     "to create depth stencil");
        M2_SAFE_RELEASE(m_texture, 0);
        M2_SAFE_RELEASE(m_surface, 0);

        return false;
    }

    m_backupSurface = NULL;
    m_backupDepthStencil = NULL;
    m_init = true;
    return true;
}

bool CGraphicRenderTargetTexture::SetRenderTarget()
{
    if(!m_init)
        return false;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** "
                                                      L"CGraphicRenderTargetTex"
                                                      L"ture::SetRenderTarget "
                                                      L"**");

    if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_backupSurface))) {
        SPDLOG_ERROR("Failed to backup render target");
        return false;
    }

    if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_backupDepthStencil))) {
        SPDLOG_ERROR("Failed to backup depth buffer");
        return false;
    }

    if (FAILED(ms_lpd3dDevice->SetRenderTarget(0, m_surface))) {
        SPDLOG_ERROR("Failed to set Render Target");
        return false;
    }

    if (FAILED(ms_lpd3dDevice->SetDepthStencilSurface(m_depthStencil))) {
        SPDLOG_ERROR("Failed to set depth stencil surface");
        return false;
    }

    D3DPERF_EndEvent();
    return true;
}

void CGraphicRenderTargetTexture::ResetRenderTarget()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** "
                                                      L"CGraphicRenderTargetTex"
                                                      L"ture::"
                                                      L"ResetRenderTarget **");

    if (FAILED(ms_lpd3dDevice->SetRenderTarget(0, m_backupSurface))) {
        SPDLOG_ERROR("Failed to switch to backup render target");
    }
    if (FAILED(ms_lpd3dDevice->SetDepthStencilSurface(m_backupDepthStencil))) {
        SPDLOG_ERROR("Failed to switch to backup depth stencil");
    }

    M2_SAFE_RELEASE_CHECK(m_backupSurface);
    M2_SAFE_RELEASE_CHECK(m_backupDepthStencil);

    D3DPERF_EndEvent();
}

void CGraphicRenderTargetTexture::SetRenderingRect(RECT* rect)
{
    m_renderRect = *rect;
}

RECT* CGraphicRenderTargetTexture::GetRenderingRect()
{
    return &m_renderRect;
}

void CGraphicRenderTargetTexture::Render() const
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 0, 127, 0), L"** "
                                                      L"CGraphicRenderTargetTex"
                                                      L"ture::Render **");

    float sx = static_cast<float>(m_renderRect.left) - 0.5f;
    float sy = static_cast<float>(m_renderRect.top) - 0.5f;
    float ex = static_cast<float>(m_renderRect.left) +
               (static_cast<float>(m_renderRect.right) -
                static_cast<float>(m_renderRect.left)) -
               0.5f;
    float ey = static_cast<float>(m_renderRect.top) +
               (static_cast<float>(m_renderRect.bottom) -
                static_cast<float>(m_renderRect.top)) -
               0.5f;
    float z = 0.0f;

    float texReverseWidth = 1.0f / (static_cast<float>(m_renderRect.right) -
                                    static_cast<float>(m_renderRect.left));
    float texReverseHeight = 1.0f / (static_cast<float>(m_renderRect.bottom) -
                                     static_cast<float>(m_renderRect.top));

    float su = texReverseWidth;
    float sv = texReverseHeight;
    float eu = ((m_renderRect.right - m_renderRect.left)) * texReverseWidth;
    float ev = ((m_renderRect.bottom - m_renderRect.top)) * texReverseHeight;

    TPDTVertex pVertices[4];

    pVertices[0].position = TPosition(sx, sy, z);
    pVertices[0].texCoord = TTextureCoordinate(su, sv);
    pVertices[0].diffuse = 0xffffffff;

    pVertices[1].position = TPosition(ex, sy, z);
    pVertices[1].texCoord = TTextureCoordinate(eu, sv);
    pVertices[1].diffuse = 0xffffffff;

    pVertices[2].position = TPosition(sx, ey, z);
    pVertices[2].texCoord = TTextureCoordinate(su, ev);
    pVertices[2].diffuse = 0xffffffff;

    pVertices[3].position = TPosition(ex, ey, z);
    pVertices[3].texCoord = TTextureCoordinate(eu, ev);
    pVertices[3].diffuse = 0xffffffff;

    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);

    STATEMANAGER.SetTexture(0, m_texture);
    STATEMANAGER.SetTexture(1, nullptr);

    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, pVertices, 4);

    D3DPERF_EndEvent();
}

CGraphicRenderTargetTexture::~CGraphicRenderTargetTexture()
{
    OnLostDevice();
}
