#include "StdAfx.h"
#include "GrpMarkInstance.h"

#include "Engine.hpp"
#include "GrpDevice.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include "../eterBase/CRC32.h"

CDynamicPool<CGraphicMarkInstance> CGraphicMarkInstance::ms_kPool;

void CGraphicMarkInstance::SetImageFileName(const char *c_szFileName)
{
    m_stImageFileName = c_szFileName;
}

const std::string &CGraphicMarkInstance::GetImageFileName()
{
    return m_stImageFileName;
}

void CGraphicMarkInstance::CreateSystem(UINT uCapacity)
{
    ms_kPool.Create(uCapacity);
}

void CGraphicMarkInstance::DestroySystem()
{
    ms_kPool.Destroy();
}

CGraphicMarkInstance *CGraphicMarkInstance::New()
{
    return ms_kPool.Alloc();
}

void CGraphicMarkInstance::Delete(CGraphicMarkInstance *pkImgInst)
{
    pkImgInst->Destroy();
    ms_kPool.Free(pkImgInst);
}

void CGraphicMarkInstance::Render()
{
    if (IsEmpty())
        return;

    assert(!IsEmpty());

    OnRender();
}

void CGraphicMarkInstance::OnRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderAfterLensFlare **");

    CGraphicTexture *pTexture = m_roImage->GetTexturePointer();

    UINT uColCount = m_roImage->GetWidth() / MARK_WIDTH;
    if (uColCount == 0)
        return;

    UINT uCol = m_uIndex % uColCount;
    UINT uRow = m_uIndex / uColCount;

    RECT kRect;
    kRect.left = uCol * MARK_WIDTH;
    kRect.top = uRow * MARK_HEIGHT;
    kRect.right = kRect.left + MARK_WIDTH;
    kRect.bottom = kRect.top + MARK_HEIGHT;

    float texReverseWidth = 1.0f / float(pTexture->GetWidth());
    float texReverseHeight = 1.0f / float(pTexture->GetHeight());
    float su = kRect.left * texReverseWidth;
    float sv = kRect.top * texReverseHeight;
    float eu = kRect.right * texReverseWidth;
    float ev = kRect.bottom * texReverseHeight;

    float fRenderWidth = MARK_WIDTH * m_fScale;
    float fRenderHeight = MARK_HEIGHT * m_fScale;

    TPDTVertex vertices[4];
    vertices[0].position.x = m_v3Position.x - 0.5f;
    vertices[0].position.y = m_v3Position.y - 0.5f;
    vertices[0].position.z = m_v3Position.z;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = m_DiffuseColor.BGRA().c;

    vertices[1].position.x = m_v3Position.x + fRenderWidth - 0.5f;
    vertices[1].position.y = m_v3Position.y - 0.5f;
    vertices[1].position.z = m_v3Position.z;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = m_DiffuseColor.BGRA().c;

    vertices[2].position.x = m_v3Position.x - 0.5f;
    vertices[2].position.y = m_v3Position.y + fRenderHeight - 0.5f;
    vertices[2].position.z = m_v3Position.z;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = m_DiffuseColor.BGRA().c;

    vertices[3].position.x = m_v3Position.x + fRenderWidth - 0.5f;
    vertices[3].position.y = m_v3Position.y + fRenderHeight - 0.5f;
    vertices[3].position.z = m_v3Position.z;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = m_DiffuseColor.BGRA().c;
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);

    STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
    STATEMANAGER.SetTexture(1, NULL);
    Engine::GetDevice().DrawPDTVertices(D3DPT_TRIANGLESTRIP, 2, vertices, 4);
    D3DPERF_EndEvent();
}

const CGraphicTexture &CGraphicMarkInstance::GetTextureReference() const
{
    return m_roImage->GetTextureReference();
}

CGraphicTexture *CGraphicMarkInstance::GetTexturePointer()
{
    return m_roImage->GetTexturePointer();
}

CGraphicImage::Ptr CGraphicMarkInstance::GetGraphicImagePointer()
{
    return m_roImage;
}

void CGraphicMarkInstance::SetScale(float fScale)
{
    m_fScale = fScale;
}

void CGraphicMarkInstance::SetIndex(UINT uIndex)
{
    m_uIndex = uIndex;
}

int CGraphicMarkInstance::GetWidth()
{
    if (IsEmpty())
        return 0;

    //return m_roImage->GetWidth();
    return 16;
}

int CGraphicMarkInstance::GetHeight()
{
    if (IsEmpty())
        return 0;

    //return m_roImage->GetHeight();
    return 12;
}

void CGraphicMarkInstance::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
    m_DiffuseColor.x = fr;
    m_DiffuseColor.y = fg;
    m_DiffuseColor.z = fb;
    m_DiffuseColor.w = fa;
}

void CGraphicMarkInstance::SetPosition(float fx, float fy, float fz)
{
    m_v3Position.x = fx;
    m_v3Position.y = fy;
    m_v3Position.z = fz;
}

void CGraphicMarkInstance::Load()
{
    if (GetImageFileName().empty())
        return;

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(GetImageFileName());
    SetImagePointer(r);
}

void CGraphicMarkInstance::SetImagePointer(CGraphicImage::Ptr pImage)
{
    m_roImage = pImage;
    OnSetImagePointer();
}

bool CGraphicMarkInstance::IsEmpty() const
{
    if (m_roImage)
        return false;

    return true;
}

bool CGraphicMarkInstance::operator ==(const CGraphicMarkInstance &rhs) const
{
    return m_roImage == rhs.m_roImage;
}

DWORD CGraphicMarkInstance::Type()
{
    static DWORD s_dwType = GetCRC32("CGraphicMarkInstance", strlen("CGraphicMarkInstance"));
    return (s_dwType);
}

BOOL CGraphicMarkInstance::IsType(DWORD dwType)
{
    return OnIsType(dwType);
}

BOOL CGraphicMarkInstance::OnIsType(DWORD dwType)
{
    if (CGraphicMarkInstance::Type() == dwType)
        return TRUE;

    return FALSE;
}

void CGraphicMarkInstance::OnSetImagePointer()
{
}

void CGraphicMarkInstance::Initialize()
{
    m_DiffuseColor.x = m_DiffuseColor.y = m_DiffuseColor.z = m_DiffuseColor.w = 1.0f;
    m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;
    m_uIndex = 0;
    m_fScale = 1.0f;
}

void CGraphicMarkInstance::Destroy()
{
    m_roImage = nullptr;
    Initialize();
}

CGraphicMarkInstance::CGraphicMarkInstance()
{
    Initialize();
}

CGraphicMarkInstance::~CGraphicMarkInstance()
{
    Destroy();
}
