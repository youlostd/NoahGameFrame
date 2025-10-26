#include "StdAfx.h"
#include "ScreenFilter.h"


#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"

CScreenFilter::CScreenFilter()
    : m_enabled(false)
      , m_srcType(D3DBLEND_SRCALPHA)
      , m_dstType(D3DBLEND_INVSRCALPHA)
      , m_color(0.0f, 0.0f, 0.0f, 0.0f)
{
    // ctor
}

void CScreenFilter::SetEnable(bool value)
{
    m_enabled = value;
}

void CScreenFilter::SetBlendType(uint8_t srcType, uint8_t dstType)
{
    m_srcType = srcType;
    m_dstType = dstType;
}

void CScreenFilter::SetColor(const DirectX::SimpleMath::Color &color)
{
    m_color = color;
}

void CScreenFilter::Render()
{
    if (!m_enabled)
        return;

    DWORD lighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
    Engine::GetDevice().SetDepthEnable(false, false);

    STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, false);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, m_srcType);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, m_dstType);

    Matrix proj = Matrix::CreateOrthographicOffCenter(0, CScreen::ms_iWidth, CScreen::ms_iHeight, 0, 0, 400.0f);

    STATEMANAGER.SaveTransform(D3DTS_PROJECTION, reinterpret_cast<Matrix *>(&proj));
    STATEMANAGER.SaveTransform(D3DTS_VIEW, &ms_matIdentity);
    STATEMANAGER.SetTransform(D3DTS_WORLD, &ms_matIdentity);

    SetDiffuseColor(m_color.BGRA().c);
    RenderBar2d(0, 0, CScreen::ms_iWidth, CScreen::ms_iHeight);

    STATEMANAGER.RestoreTransform(D3DTS_VIEW);
    STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    Engine::GetDevice().SetDepthEnable(true, true);

}
