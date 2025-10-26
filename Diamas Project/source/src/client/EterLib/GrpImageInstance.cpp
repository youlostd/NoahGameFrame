#include "GrpImageInstance.h"
#include "StdAfx.h"

#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"

#include "../eterBase/CRC32.h"

void CGraphicImageInstance::Render()
{
    if (IsEmpty())
        return;

    OnRender();
}

void CGraphicImageInstance::OnRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CGraphicImageInstance::OnRender **");

    const auto pTexture = m_roImage->GetTexturePointer();

    const auto fimgWidth = m_roImage->GetWidth() * m_v2Scale.x;
    const auto fimgHeight = m_roImage->GetHeight() * m_v2Scale.y;

    const auto &c_rRect = m_roImage->GetRectReference();
    const auto texReverseWidth = 1.0f / float(pTexture->GetWidth());
    const auto texReverseHeight = 1.0f / float(pTexture->GetHeight());
    const auto su = c_rRect.left * texReverseWidth;
    const auto sv = c_rRect.top * texReverseHeight;
    const auto eu = (c_rRect.left + (c_rRect.right - c_rRect.left)) * texReverseWidth;
    const auto ev = (c_rRect.top + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

    TPDTVertex vertices[4];
    vertices[0].position.x = m_v2Position.x - 0.5f;
    vertices[0].position.y = m_v2Position.y - 0.5f;
    vertices[0].position.z = 0.0f;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = m_DiffuseColor.BGRA().c;

    vertices[1].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[1].position.y = m_v2Position.y - 0.5f;
    vertices[1].position.z = 0.0f;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = m_DiffuseColor.BGRA().c;

    vertices[2].position.x = m_v2Position.x - 0.5f;
    vertices[2].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[2].position.z = 0.0f;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = m_DiffuseColor.BGRA().c;

    vertices[3].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[3].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[3].position.z = 0.0f;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = m_DiffuseColor.BGRA().c;

    if (m_bIsInverse)
    {
        vertices[0].texCoord = TTextureCoordinate(eu, sv);
        vertices[1].texCoord = TTextureCoordinate(su, sv);
        vertices[2].texCoord = TTextureCoordinate(eu, ev);
        vertices[3].texCoord = TTextureCoordinate(su, ev);
    }

    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    
        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, nullptr);
    // 2004.11.18.myevan.ctrl+alt+del �ݺ� ���� ƨ��� ����
    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, vertices, 4);

    
    //OLD: Engine::GetDevice().DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
    ////////////////////////////////////////////////////////////

    // if(m_clipRect)
    //	STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DPERF_EndEvent();
}

void CGraphicImageInstance::OnRenderWithCoolTimeBox(float fTime)
{
    if (fTime >= 1.0f)
        fTime = 1.0f;

    auto *const pTexture = m_roImage->GetTexturePointer();

    const auto fimgWidth = m_roImage->GetWidth() * m_v2Scale.x;
    const auto fimgHeight = m_roImage->GetHeight() * m_v2Scale.y;

    const auto &c_rRect = m_roImage->GetRectReference();
    const auto texReverseWidth = 1.0f / float(pTexture->GetWidth());
    const auto texReverseHeight = 1.0f / float(pTexture->GetHeight());
    const auto su = c_rRect.left * texReverseWidth;
    const auto sv = c_rRect.top * texReverseHeight;
    const auto eu = texReverseWidth * c_rRect.right;
    const auto ev = texReverseHeight * c_rRect.bottom;

    const auto fxCenter = fimgWidth * 0.5 + this->m_v2Position.x - 0.5;
    const auto halfHeight = fimgHeight * 0.5;
    const auto fyCenter = halfHeight + this->m_v2Position.y - 0.5;

    if (fTime >= 1.0)
        return;

    if (fTime < 0.0)
        fTime = 0.0;

    static Vector2 s_v2BoxPos[8] = {
        Vector2(-1.0f, -1.0f), Vector2(-1.0f, 0.0f), Vector2(-1.0f, +1.0f), Vector2(0.0f, +1.0f),
        Vector2(+1.0f, +1.0f), Vector2(+1.0f, 0.0f), Vector2(+1.0f, -1.0f), Vector2(0.0f, -1.0f),
    };

    auto iTriCount = int(8 - 8.0f * fTime);
    const auto fLastPercentage = (8 - 8.0f * fTime) - iTriCount;

    std::vector<TPDTVertex> vertices;
    TPDTVertex vertex;
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter;
    vertex.position.z = 0.0f;
    vertex.diffuse = m_DiffuseColor.BGRA().c;
    vertex.texCoord.y = ev * 0.5f;
    vertex.texCoord.x = 0.5f * eu;
    vertices.push_back(vertex);

    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter - halfHeight - 0.5f;
    vertex.position.z = 0.0f;
    vertex.diffuse = m_DiffuseColor.BGRA().c;
    vertex.texCoord.x = sv;
    vertex.texCoord.x = 0.5f * eu;
    vertices.push_back(vertex);

    vertex.position.z = 0.0f;
    for (auto j = 0; j < iTriCount; ++j)
    {
        vertex.position.x = s_v2BoxPos[j].x * fimgWidth * 0.5f + fxCenter - 0.5f;
        vertex.position.y = s_v2BoxPos[j].y * fimgHeight * 0.5f + fyCenter - 0.5f;
        vertex.diffuse = m_DiffuseColor.BGRA().c;
        vertex.texCoord.x = su + 2 * (j & 7);
        vertex.texCoord.y = sv + 2 * (j & 7);
        vertices.push_back(vertex);
    }

    if (fLastPercentage > 0.0f)
    {
        assert((iTriCount - 1 + 8) % 8 >= 0 && (iTriCount - 1 + 8) % 8 < 8);
        assert((iTriCount + 8) % 8 >= 0 && (iTriCount + 8) % 8 < 8);
        const auto pv2LastPos = &s_v2BoxPos[(iTriCount + 7) % 8];
        const auto pv2Pos = &s_v2BoxPos[(iTriCount + 8) % 8];

        vertex.position.x =
            fimgWidth * (pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x * 0.5f + fxCenter - 0.5f;
        vertex.position.y =
            fimgHeight * fLastPercentage * (pv2Pos->y - pv2LastPos->y) + pv2LastPos->y * 0.5f + fyCenter - 0.5f;
        vertex.position.z = 0.0f;
        vertex.diffuse = m_DiffuseColor.BGRA().c;
        vertex.texCoord.x = (pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x * eu * 0.5f + (0.5f * eu);
        vertex.texCoord.y = 0.5f * (fLastPercentage * (pv2Pos->y - pv2LastPos->y) + pv2LastPos->y * ev) + (ev * 0.5f);

        vertices.push_back(vertex);
        ++iTriCount;
    }

    if (vertices.empty())
    {
        return;
    }
    if (m_bScalePivotCenter)
    {
        vertices[0].texCoord = TTextureCoordinate(eu, sv);
        vertices[1].texCoord = TTextureCoordinate(su, sv);
        vertices[2].texCoord = TTextureCoordinate(eu, ev);
        vertices[3].texCoord = TTextureCoordinate(su, ev);
    }

    {
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, NULL);
        Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
        DWORD cullMode;
        STATEMANAGER.GetRenderState(D3DRS_CULLMODE, &cullMode);
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, NULL);

        Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, iTriCount, vertices.data(), vertices.size());

        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, cullMode);
    }
}

const CGraphicTexture &CGraphicImageInstance::GetTextureReference() const { return m_roImage->GetTextureReference(); }

CGraphicTexture *CGraphicImageInstance::GetTexturePointer() const
{
    return m_roImage ? m_roImage->GetTexturePointer() : nullptr;
}

CGraphicImage::Ptr CGraphicImageInstance::GetGraphicImagePointer() const { return m_roImage; }

int CGraphicImageInstance::GetWidth() const
{
    if (IsEmpty())
    {
        return 0;
    }

    return m_roImage->GetWidth();
}

int CGraphicImageInstance::GetHeight() const
{
    if (IsEmpty())
    {
        return 0;
    }

    return m_roImage->GetHeight();
}

void CGraphicImageInstance::SetDiffuseColor(const Color &color) { m_DiffuseColor = color; }

void CGraphicImageInstance::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
    m_DiffuseColor.R(fr);
    m_DiffuseColor.G(fg);
    m_DiffuseColor.B(fb);
    m_DiffuseColor.A(fa);
}

void CGraphicImageInstance::SetScalePivotCenter(bool bScalePivotCenter) { m_bScalePivotCenter = bScalePivotCenter; }

void CGraphicImageInstance::SetPosition(float fx, float fy)
{
    m_v2Position.x = fx;
    m_v2Position.y = fy;
}

void CGraphicImageInstance::SetInverse() { m_bIsInverse = true; }

void CGraphicImageInstance::SetScale(float fx, float fy)
{
    m_v2Scale.x = fx;
    m_v2Scale.y = fy;
}

void CGraphicImageInstance::SetScale(Vector2 scale)
{
    m_v2Scale = scale;
}

void CGraphicImageInstance::SetImagePointer(CGraphicImage::Ptr r)
{
    m_roImage = r;
    OnSetImagePointer();
}

void CGraphicImageInstance::SetFilename(const std::string &r) { m_fileName = r; }

void CGraphicImageInstance::ReloadImagePointer(CGraphicImage::Ptr r)
{
    if (m_roImage)
        m_roImage->Reload();
    else
        SetImagePointer(r);
}

bool CGraphicImageInstance::IsEmpty() const
{
    if (m_roImage) {
        if(!m_roImage->GetTexturePointer())
            return true;

        if(m_roImage->GetTexturePointer()->IsEmpty())
            return true;
        return false;
    }

    return true;
}

bool CGraphicImageInstance::operator==(const CGraphicImageInstance &rhs) const { return m_roImage == rhs.m_roImage; }

DWORD CGraphicImageInstance::Type()
{
    static DWORD s_dwType = GetCRC32("CGraphicImageInstance", strlen("CGraphicImageInstance"));
    return (s_dwType);
}

bool CGraphicImageInstance::IsType(DWORD dwType) { return OnIsType(dwType); }

bool CGraphicImageInstance::OnIsType(DWORD dwType)
{
    if (CGraphicImageInstance::Type() == dwType)
    {
        return TRUE;
    }

    return FALSE;
}

void CGraphicImageInstance::OnSetImagePointer() {}

void CGraphicImageInstance::Destroy() { m_roImage.reset(); }

CGraphicImageInstance::CGraphicImageInstance(std::string_view filename) { m_fileName = filename; }

CGraphicImageInstance::~CGraphicImageInstance() { m_roImage.reset(); }
