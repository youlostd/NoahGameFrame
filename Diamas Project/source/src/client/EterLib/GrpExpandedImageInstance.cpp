#include "GrpExpandedImageInstance.h"
#include "../EterBase/CRC32.h"
#include "StdAfx.h"

#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"

void CGraphicExpandedImageInstance::OnRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 0, 255, 0), L"** CGraphicExpandedImageInstance::OnRender **");
    DWORD cullMode = STATEMANAGER.GetRenderState(D3DRS_CULLMODE);

    auto pImage = m_roImage;
    CGraphicTexture *pTexture = pImage->GetTexturePointer();

    const RECT &c_rRect = pImage->GetRectReference();
    float texReverseWidth = 1.0f / float(pTexture->GetWidth());
    float texReverseHeight = 1.0f / float(pTexture->GetHeight());
    float su = (c_rRect.left - m_RenderingRect.left) * texReverseWidth;
    float sv = (c_rRect.top - m_RenderingRect.top) * texReverseHeight;
    float eu = (c_rRect.left + m_RenderingRect.right + (c_rRect.right - c_rRect.left)) * texReverseWidth;
    float ev = (c_rRect.top + m_RenderingRect.bottom + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

    TPDTVertex vertices[4];
    vertices[0].position.x = m_v2Position.x - 0.5f;
    vertices[0].position.y = m_v2Position.y - 0.5f;
    vertices[0].position.z = m_fDepth;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = m_DiffuseColor.BGRA().c;

    vertices[1].position.x = m_v2Position.x - 0.5f;
    vertices[1].position.y = m_v2Position.y - 0.5f;
    vertices[1].position.z = m_fDepth;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = m_DiffuseColor.BGRA().c;

    vertices[2].position.x = m_v2Position.x - 0.5f;
    vertices[2].position.y = m_v2Position.y - 0.5f;
    vertices[2].position.z = m_fDepth;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = m_DiffuseColor.BGRA().c;

    vertices[3].position.x = m_v2Position.x - 0.5f;
    vertices[3].position.y = m_v2Position.y - 0.5f;
    vertices[3].position.z = m_fDepth;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = m_DiffuseColor.BGRA().c;

    if (m_bIsInverse)
    {
        vertices[0].texCoord = TTextureCoordinate(eu, ev);
        vertices[1].texCoord = TTextureCoordinate(su, ev);
        vertices[2].texCoord = TTextureCoordinate(eu, sv);
        vertices[3].texCoord = TTextureCoordinate(su, sv);
    }

    if (0.0f == m_fRotation)
    {
        float fimgWidth = float(pImage->GetWidth()) * m_v2Scale.x;
        float fimgHeight = float(pImage->GetHeight()) * m_v2Scale.y;

        vertices[0].position.x -= m_RenderingRect.left;
        vertices[0].position.y -= m_RenderingRect.top;
        vertices[1].position.x += fimgWidth + m_RenderingRect.right;
        vertices[1].position.y -= m_RenderingRect.top;
        vertices[2].position.x -= m_RenderingRect.left;
        vertices[2].position.y += fimgHeight + m_RenderingRect.bottom;
        vertices[3].position.x += fimgWidth + m_RenderingRect.right;
        vertices[3].position.y += fimgHeight + m_RenderingRect.bottom;
        if ((0.0f < m_v2Scale.x && 0.0f > m_v2Scale.y) || (0.0f > m_v2Scale.x && 0.0f < m_v2Scale.y))
        {
            STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        }
    }
    else
    {
        float fimgHalfWidth = float(pImage->GetWidth()) / 2.0f * m_v2Scale.x;
        float fimgHalfHeight = float(pImage->GetHeight()) / 2.0f * m_v2Scale.y;

        for (int i = 0; i < 4; ++i)
        {
            vertices[i].position.x += m_v2Origin.x;
            vertices[i].position.y += m_v2Origin.y;
        }

        float fRadian = DirectX::XMConvertToRadians(m_fRotation);
        vertices[0].position.x += (-fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
        vertices[0].position.y += (-fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));
        vertices[1].position.x += (+fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
        vertices[1].position.y += (+fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));
        vertices[2].position.x += (-fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
        vertices[2].position.y += (-fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));
        vertices[3].position.x += (+fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
        vertices[3].position.y += (+fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));
    }

    DWORD isAlphaBlend = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);

    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    switch (m_iRenderingMode)
    {
    case RENDERING_MODE_SCREEN:
    case RENDERING_MODE_COLOR_DODGE:
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
    case RENDERING_MODE_MODULATE:
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

        break;
    }
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
    STATEMANAGER.SetTexture(1, NULL);
    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, vertices, 4);
    // 2004.11.18.myevan.ctrl+alt+del �ݺ� ���� ƨ��� ����
    // if (Engine::GetDevice().SetPDTStream(vertices, 4))
    //{
    //    Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

    //    Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    //}
    // Engine::GetDevice().DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16,
    // vertices, sizeof(TPDTVertex));
    /////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////

    switch (m_iRenderingMode)
    {
    case RENDERING_MODE_SCREEN:
    case RENDERING_MODE_COLOR_DODGE:
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        break;

    case RENDERING_MODE_MODULATE:
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
        break;
    }

    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, cullMode);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, isAlphaBlend);

    D3DPERF_EndEvent();
}

void CGraphicExpandedImageInstance::OnRenderWithCoolTimeBox(float fTime)
{
    if (fTime >= 1.0)
        fTime = 1.0;

    auto pImage = m_roImage;
    CGraphicTexture *pTexture = pImage->GetTexturePointer();
    DWORD cullMode = STATEMANAGER.GetRenderState(D3DRS_CULLMODE);

    const RECT &c_rRect = pImage->GetRectReference();
    float texReverseWidth = 1.0f / float(pTexture->GetWidth());
    float texReverseHeight = 1.0f / float(pTexture->GetHeight());
    auto su = (c_rRect.left - m_RenderingRect.left) * texReverseWidth;
    auto sv = (c_rRect.top - m_RenderingRect.top) * texReverseHeight;
    auto eu = texReverseWidth * (c_rRect.right + m_RenderingRect.right);
    auto ev = texReverseHeight * (c_rRect.bottom + m_RenderingRect.bottom);
    float halfImageWidth = (c_rRect.right - c_rRect.left);
    float halfImageHeight = (c_rRect.bottom - c_rRect.top);
    auto fxCenter = halfImageWidth * 0.5 + m_v2Position.x;
    auto fyCenter = halfImageHeight * 0.5 + m_v2Position.y;

    if (fTime >= 1.0)
        return;

    if (fTime < 0.0)
        fTime = 0.0;

    int iTriCount = int(8 - 8.0f * fTime);
    float fLastPercentage = (8 - 8.0f * fTime) - iTriCount;

    static Vector2 s_v2BoxPos[8] = {
        Vector2(-1.0f, -1.0f), Vector2(-1.0f, 0.0f), Vector2(-1.0f, +1.0f), Vector2(0.0f, +1.0f),
        Vector2(+1.0f, +1.0f), Vector2(+1.0f, 0.0f), Vector2(+1.0f, -1.0f), Vector2(0.0f, -1.0f),
    };

    static Vector2 s_v2BoxPos2[8] = {
        Vector2(su, sv), Vector2(su, (ev + sv) * 0.5f), Vector2(su, ev), Vector2(0.5f * (su + eu), ev),
        Vector2(eu, ev), Vector2(eu, (ev + sv) * 0.5f), Vector2(eu, sv), Vector2(0.5f * (su + eu), sv),
    };

    std::vector<TPDTVertex> vertices;
    TPDTVertex vertex;
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter;
    vertex.position.z = m_fDepth;
    vertex.diffuse = m_DiffuseColor.BGRA().c;
    vertex.texCoord.x = (eu + su) * 0.5f;
    vertex.texCoord.y = (ev + sv) * 0.5f;
    vertices.push_back(vertex);
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter - (c_rRect.bottom - c_rRect.top) * 0.5;
    vertex.position.z = m_fDepth;
    vertex.diffuse = m_DiffuseColor.BGRA().c;
    vertex.texCoord.x = (eu + su) * 0.5f;
    vertex.texCoord.y = sv;
    vertices.push_back(vertex);

    for (int j = 0; j < iTriCount; ++j)
    {
        vertex.position.x = s_v2BoxPos[j].x * halfImageWidth * 0.5f + fxCenter;
        vertex.position.y = s_v2BoxPos[j].y * halfImageHeight * 0.5f + fyCenter;
        vertex.position.z = m_fDepth;
        vertex.diffuse = m_DiffuseColor.BGRA().c;
        vertex.texCoord.x = s_v2BoxPos2[(j & 7)].x;
        vertex.texCoord.y = s_v2BoxPos2[(j & 7)].y;
        vertices.push_back(vertex);
    }

    if (fLastPercentage > 0.0f)
    {
        Vector2 *pv2LastPos;
        Vector2 *pv2Pos;

        assert((iTriCount - 1 + 8) % 8 >= 0 && (iTriCount - 1 + 8) % 8 < 8);
        assert((iTriCount + 8) % 8 >= 0 && (iTriCount + 8) % 8 < 8);

        pv2Pos = &s_v2BoxPos[(iTriCount + 8) % 8];

        pv2LastPos = &s_v2BoxPos[(iTriCount + 7) % 8];
        auto v75 = (pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x;
        vertex.position.x = halfImageWidth * v75 * 0.5 + fxCenter + 0.5;
        auto v76 = fLastPercentage * (pv2Pos->y - pv2LastPos->y) + pv2LastPos->y;
        vertex.position.y = halfImageHeight * v76 * 0.5 + fyCenter + 0.5;
        vertex.position.z = m_fDepth;
        vertex.diffuse = m_DiffuseColor.BGRA().c;
        vertex.texCoord.x = su + (eu - su) * 0.5f + v75 * (eu - su) * 0.5f;
        vertex.texCoord.y = 0.5f * (v76 * (ev - sv)) + sv + (ev - sv) * 0.5f;
        vertices.push_back(vertex);
        ++iTriCount;
    }

    DWORD isAlphaBlend = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);

    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    switch (m_iRenderingMode)
    {
    case RENDERING_MODE_SCREEN:
    case RENDERING_MODE_COLOR_DODGE:
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
    case RENDERING_MODE_MODULATE:
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

        break;
    }

    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);

    STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
    STATEMANAGER.SetTexture(1, NULL);

    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, iTriCount, vertices.data(), vertices.size());

    /*// 2004.11.18.myevan.ctrl+alt+del �ݺ� ���� ƨ��� ����
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
    {
        Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);


        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }*/
    // Engine::GetDevice().DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16,
    // vertices, sizeof(TPDTVertex));
    /////////////////////////////////////////////////////////////

    switch (m_iRenderingMode)
    {
    case RENDERING_MODE_SCREEN:
    case RENDERING_MODE_COLOR_DODGE:
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        break;

    case RENDERING_MODE_MODULATE:
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
        break;
    }

    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, cullMode);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, isAlphaBlend);
}

void CGraphicExpandedImageInstance::SetDepth(float fDepth) { m_fDepth = fDepth; }

void CGraphicExpandedImageInstance::SetOrigin() { SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f); }

void CGraphicExpandedImageInstance::SetOrigin(float fx, float fy)
{
    m_v2Origin.x = fx;
    m_v2Origin.y = fy;
}

void CGraphicExpandedImageInstance::SetRotation(float fRotation) { m_fRotation = fRotation; }

void CGraphicExpandedImageInstance::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
    if (IsEmpty())
    {
        return;
    }

    float fWidth = float(GetWidth());
    float fHeight = float(GetHeight());

    m_RenderingRect.left = fWidth * fLeft;
    m_RenderingRect.top = fHeight * fTop;
    m_RenderingRect.right = fWidth * fRight;
    m_RenderingRect.bottom = fHeight * fBottom;
}

void CGraphicExpandedImageInstance::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
    if (IsEmpty())
    {
        return;
    }

    float fWidth = float(GetWidth()) * m_v2Scale.x;
    float fHeight = float(GetHeight()) * m_v2Scale.y;

    m_RenderingRect.left = fWidth * fLeft;
    m_RenderingRect.top = fHeight * fTop;
    m_RenderingRect.right = fWidth * fRight;
    m_RenderingRect.bottom = fHeight * fBottom;
}

void CGraphicExpandedImageInstance::SetRenderingMode(int iMode) { m_iRenderingMode = iMode; }

DWORD CGraphicExpandedImageInstance::Type()
{
    static DWORD s_dwType = GetCRC32("CGraphicExpandedImageInstance", strlen("CGraphicExpandedImageInstance"));
    return (s_dwType);
}

void CGraphicExpandedImageInstance::OnSetImagePointer()
{
    if (IsEmpty())
    {
        return;
    }

    SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f);
}

bool CGraphicExpandedImageInstance::OnIsType(DWORD dwType)
{
    if (CGraphicExpandedImageInstance::Type() == dwType)
    {
        return TRUE;
    }

    return CGraphicImageInstance::IsType(dwType);
}

void CGraphicExpandedImageInstance::Destroy() { CGraphicImageInstance::Destroy(); }

CGraphicExpandedImageInstance::CGraphicExpandedImageInstance(std::string_view filename) { m_fileName = filename; }

CGraphicExpandedImageInstance::~CGraphicExpandedImageInstance() { CGraphicExpandedImageInstance::Destroy(); }
