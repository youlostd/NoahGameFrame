#include "StdAfx.h"
#include "EffectMeshInstance.h"

#include "../EterBase/StepTimer.h"
#include "Util.hpp"
#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"
#include "../EterLib/GrpMath.h"

using namespace DirectX::SimpleMath;

CDynamicPool<CEffectMeshInstance> CEffectMeshInstance::ms_kPool;

void CEffectMeshInstance::DestroySystem()
{
    ms_kPool.Destroy();
}

CEffectMeshInstance *CEffectMeshInstance::New()
{
    return ms_kPool.Alloc();
}

void CEffectMeshInstance::Delete(CEffectMeshInstance *pkMeshInstance)
{
    pkMeshInstance->Destroy();
    ms_kPool.Free(pkMeshInstance);
}

bool CEffectMeshInstance::isActive()
{
    if (!m_MeshFrameController.isActive())
        return false;

    for (uint32_t j = 0; j < m_TextureInstanceVector.size(); ++j)
    {
        int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
        if (m_TextureInstanceVector[j].TextureFrameController.isActive(iCurrentFrame))
            return true;
    }

    return false;
}

bool CEffectMeshInstance::OnUpdate(double fElapsedTime)
{
    if (!isActive() || !m_isActive || (isHiddenByLod() && !m_isAlwaysShow))
        return false;

    if (m_MeshFrameController.isActive())
        m_MeshFrameController.Update(fElapsedTime);

    for (uint32_t j = 0; j < m_TextureInstanceVector.size(); ++j)
    {
        int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
        if (m_TextureInstanceVector[j].TextureFrameController.isActive(iCurrentFrame))
            m_TextureInstanceVector[j].TextureFrameController.Update(fElapsedTime);
    }

    return true;
}

void CEffectMeshInstance::OnRender(uint32_t addColor)
{
    if (!isActive() || !m_isActive || (isHiddenByLod() && !m_isAlwaysShow))
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CEffectMeshInstance::OnRender **");

    for (uint32_t i = 0; i < m_pEffectMesh->GetMeshCount(); ++i)
    {
        assert(i < m_TextureInstanceVector.size());

        CFrameController &rTextureFrameController = m_TextureInstanceVector[i].TextureFrameController;
        if (!rTextureFrameController.isActive(m_MeshFrameController.GetCurrentFrame()))
            continue;

        int iBillboardType = m_pMeshScript->GetBillboardType(i);

        Matrix m_matWorld = Matrix::Identity;


        switch (iBillboardType)
        {
        case MESH_BILLBOARD_TYPE_ALL: {
            Matrix matTemp = Matrix::CreateRotationX(90.0f);
            m_matWorld = CScreen::GetViewMatrix().Invert();
            m_matWorld = matTemp * m_matWorld;
        }
        break;

        case MESH_BILLBOARD_TYPE_Y: {
            Matrix matTemp = CScreen::GetViewMatrix().Invert();
            m_matWorld._11 = matTemp._11;
            m_matWorld._12 = matTemp._12;
            m_matWorld._21 = matTemp._21;
            m_matWorld._22 = matTemp._22;
        }
        break;

        case MESH_BILLBOARD_TYPE_MOVE: {
            Vector3 Position;
            m_pMeshScript->GetPosition(m_localTime, Position);
            Vector3 LastPosition;
            m_pMeshScript->GetPosition(m_localTime - DX::StepTimer::Instance().GetElapsedSeconds(), LastPosition);
            Position -= LastPosition;
            if (Position.LengthSquared() > 0.001f)
            {
                Position.Normalize();
                Quaternion q = SafeRotationNormalizedArc(Vector3(0.0f, -1.0f, 0.0f), Position);
                m_matWorld = Matrix::CreateFromQuaternion(q);
            }
        }
        break;
        }

        if (!m_pMeshScript->isBlendingEnable(i))
        {
            STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        }
        else
        {
            int iBlendingSrcType = m_pMeshScript->GetBlendingSrcType(i);
            int iBlendingDestType = m_pMeshScript->GetBlendingDestType(i);
            STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, true);
            STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, iBlendingSrcType);
            STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, iBlendingDestType);
        }

        Vector3 Position;
        m_pMeshScript->GetPosition(m_localTime, Position);

        m_matWorld._41 = Position.x;
        m_matWorld._42 = Position.y;
        m_matWorld._43 = Position.z;
        m_matWorld = m_matWorld * *m_localMat;

        STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

        uint8_t byType;
        DirectX::SimpleMath::Color color(1.0f, 1.0f, 1.0f, 1.0f);
        if (m_pMeshScript->GetColorOperationType(i, &byType))
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, byType);

        m_pMeshScript->GetColorFactor(i, &color);

        TTimeEventTableFloat *TableAlpha;

        float fAlpha = 1.0f;
        if (m_pMeshScript->GetTimeTableAlphaPointer(i, &TableAlpha) && !TableAlpha->empty())
            fAlpha = GetTimeEventBlendValue(m_localTime, *TableAlpha);

        CEffectMesh::TEffectMeshData *pMeshData = m_pEffectMesh->GetMeshDataPointer(i);

        assert(m_MeshFrameController.GetCurrentFrame() < pMeshData->EffectFrameDataVector.size());
        CEffectMesh::TEffectFrameData &rFrameData = pMeshData->EffectFrameDataVector[m_MeshFrameController.
            GetCurrentFrame()];

        uint32_t dwcurTextureFrame = rTextureFrameController.GetCurrentFrame();
        if (dwcurTextureFrame < m_TextureInstanceVector[i].TextureInstanceVector.size())
        {
            CGraphicImageInstance *pImageInstance = m_TextureInstanceVector[i].TextureInstanceVector[dwcurTextureFrame].
                get();
            STATEMANAGER.SetTexture(0, pImageInstance->GetTexturePointer()->GetD3DTexture());
        }

        color.A(fAlpha * rFrameData.fVisibility);
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, BlendColor(color, Color{addColor}).BGRA().c);
        Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_TEX1);
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLELIST,
                                     rFrameData.dwIndexCount / 3,
                                     &rFrameData.PTVertexVector[0],
                                     sizeof(TPTVertex));
    }

    D3DPERF_EndEvent();
}

void CEffectMeshInstance::OnSetDataPointer(CEffectElementBase *pElement)
{
    auto* pMesh = (CEffectMeshScript *)pElement;
    m_pMeshScript = pMesh;

    const char *c_szMeshFileName = pMesh->GetMeshFileName();

    m_pEffectMesh = CResourceManager::Instance().LoadResource<CEffectMesh>(c_szMeshFileName);
    if (!m_pEffectMesh)
    {
        SPDLOG_ERROR("Failed to load {0}", c_szMeshFileName);
        return;
    }

    m_MeshFrameController.Clear();
    m_MeshFrameController.SetMaxFrame(m_pEffectMesh->GetFrameCount());
    m_MeshFrameController.SetFrameTime(pMesh->GetMeshAnimationFrameDelay());
    m_MeshFrameController.SetLoopFlag(pMesh->isMeshAnimationLoop());
    m_MeshFrameController.SetLoopCount(pMesh->GetMeshAnimationLoopCount());
    m_MeshFrameController.SetStartFrame(0);

    m_TextureInstanceVector.clear();
    m_TextureInstanceVector.resize(m_pEffectMesh->GetMeshCount());
    for (uint32_t j = 0; j < m_TextureInstanceVector.size(); ++j)
    {
        CEffectMeshScript::TMeshData *pMeshData;
        if (!m_pMeshScript->GetMeshDataPointer(j, &pMeshData))
            continue;

        auto *pTextureVector = m_pEffectMesh->GetTextureVectorPointer(j);
        if (!pTextureVector)
            continue;

        auto &rTextureVector = *pTextureVector;

        CFrameController &rFrameController = m_TextureInstanceVector[j].TextureFrameController;
        rFrameController.Clear();
        rFrameController.SetMaxFrame(rTextureVector.size());
        rFrameController.SetFrameTime(pMeshData->fTextureAnimationFrameDelay);
        rFrameController.SetLoopFlag(pMeshData->bTextureAnimationLoopEnable);
        rFrameController.SetStartFrame(pMeshData->dwTextureAnimationStartFrame);

        auto &rImageInstanceVector = m_TextureInstanceVector[j].TextureInstanceVector;
        rImageInstanceVector.clear();
        rImageInstanceVector.reserve(rTextureVector.size());

        for (auto itor = rTextureVector.begin(); itor != rTextureVector.end(); ++itor)
        {
            auto pImageInstance = std::make_unique<CGraphicImageInstance>();
            pImageInstance->SetImagePointer(*itor);
            rImageInstanceVector.push_back(std::move(pImageInstance));
        }
    }
}

void CEffectMeshInstance_DeleteTextureInstance(CEffectMeshInstance::TTextureInstance &rkInstance)
{
    auto &rVector = rkInstance.TextureInstanceVector;
    rVector.clear();
}

void CEffectMeshInstance::OnInitialize()
{
}

void CEffectMeshInstance::OnDestroy()
{
    for_each(m_TextureInstanceVector.begin(), m_TextureInstanceVector.end(), CEffectMeshInstance_DeleteTextureInstance);
    m_TextureInstanceVector.clear();
    m_pEffectMesh = nullptr;
}

CEffectMeshInstance::CEffectMeshInstance()
{
    Initialize();
}

CEffectMeshInstance::~CEffectMeshInstance()
{
    Destroy();
}
