#include "AreaTerrain.h"
#include "MapOutdoor.h"
#include "StdAfx.h"
#include "TerrainPatch.h"
#include "TerrainQuadtree.h"

#include "../SpeedTreeLib/SpeedTreeForest.h"

#include "../EterLib/Camera.h"
#include "../EterLib/StateManager.h"

#include "../EterGrnLib/GrannyState.hpp"
#include "../EterLib/Engine.hpp"
#include "EnvironmentData.hpp"
#include "../EterGrnLib/ModelShader.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"
#include "base/Remotery.h"

#include <execution>
#include <mmsystem.h>

void CMapOutdoor::RenderTerrain()
{
    if (!IsVisiblePart(PART_TERRAIN))
        return;

    // Inserted by levites
    if (!m_pTerrainPatchProxyList)
        return;

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCamera)
        return;

    rmt_ScopedCPUSample(RenderTerrain, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderTerrain **");

    BuildViewFrustum(ms_matView * ms_matProj);

    Vector3 v3Eye = pCamera->GetEye();
    m_fXforDistanceCaculation = -v3Eye.x;
    m_fYforDistanceCaculation = -v3Eye.y;

    //////////////////////////////////////////////////////////////////////////
    // Push
    m_PatchVector.clear();

    __RenderTerrain_RecurseRenderQuadTree(m_pRootNode);

    // °Å¸®¼ø Á¤·Ä
    std::sort(std::execution::par, m_PatchVector.begin(), m_PatchVector.end());

    // ±×¸®±â À§ÇÑ º¤ÅÍ ¼¼ÆÃ
    __RenderTerrain_RenderHardwareTransformPatch();
    D3DPERF_EndEvent();
}

void CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree(CTerrainQuadtreeNode *Node, bool bCullCheckNeed)
{
    if (bCullCheckNeed)
    {
        switch (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius))
        {
        case VIEW_ALL:
            // all child nodes need not cull check
            bCullCheckNeed = false;
            break;
        case VIEW_PART:
            break;
        case VIEW_NONE:
            // no need to render
            return;
        }
        // if no need cull check more
        // -> bCullCheckNeed = false;
    }

    if (Node->Size == 1)
    {
        Vector3 v3Center = Node->center;
        float fDistance =
            std::max(fabs(v3Center.x + m_fXforDistanceCaculation), fabs(-v3Center.y + m_fYforDistanceCaculation));
        __RenderTerrain_AppendPatch(v3Center, fDistance, Node->PatchNum);
    }
    else
    {
        if (Node->NW_Node != nullptr)
            __RenderTerrain_RecurseRenderQuadTree(Node->NW_Node, bCullCheckNeed);
        if (Node->NE_Node != nullptr)
            __RenderTerrain_RecurseRenderQuadTree(Node->NE_Node, bCullCheckNeed);
        if (Node->SW_Node != nullptr)
            __RenderTerrain_RecurseRenderQuadTree(Node->SW_Node, bCullCheckNeed);
        if (Node->SE_Node != nullptr)
            __RenderTerrain_RecurseRenderQuadTree(Node->SE_Node, bCullCheckNeed);
    }
}

int CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(const Vector3 &c_v3Center,
                                                                           const float &c_fRadius)
{
    const int count = 6;

    Vector3 center = c_v3Center;
    center.y = -center.y;

    int i;

    float distance[count];
    for (i = 0; i < count; ++i)
    {
        distance[i] = m_plane[i].DotCoordinate(center);
        if (distance[i] <= -c_fRadius)
            return VIEW_NONE;
    }

    for (i = 0; i < count; ++i)
    {
        if (distance[i] <= c_fRadius)
            return VIEW_PART;
    }

    return VIEW_ALL;
}

void CMapOutdoor::__RenderTerrain_AppendPatch(const Vector3 &c_rv3Center, float fDistance, long lPatchNum)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::__RenderTerrain_AppendPatch");
    if (!m_pTerrainPatchProxyList[lPatchNum].isUsed())
        return;

    m_pTerrainPatchProxyList[lPatchNum].SetCenterPosition(c_rv3Center);
    m_PatchVector.emplace_back(fDistance, lPatchNum);
}

// 2004. 2. 17. myevan. 모든 부분을 보이게 초기화 한다
void CMapOutdoor::InitializeVisibleParts()
{
    m_dwVisiblePartFlags = 0xffffffff;
}

// 2004. 2. 17. myevan. Æ¯Á¤ ºÎºÐÀ» º¸ÀÌ°Ô ÇÏ°Å³ª °¨Ãß´Â ÇÔ¼ö
void CMapOutdoor::SetVisiblePart(int ePart, bool isVisible)
{
    uint32_t dwMask = (1 << ePart);
    if (isVisible)
    {
        m_dwVisiblePartFlags |= dwMask;
    }
    else
    {
        uint32_t dwReverseMask = ~dwMask;
        m_dwVisiblePartFlags &= dwReverseMask;
    }
}

// 2004. 2. 17. myevan. Æ¯Á¤ ºÎºÐÀÌ º¸ÀÌ´ÂÁö ¾Ë¾Æ³»´Â ÇÔ¼ö
bool CMapOutdoor::IsVisiblePart(int ePart)
{
    uint32_t dwMask = (1 << ePart);
    if (dwMask & m_dwVisiblePartFlags)
        return true;

    return false;
}

// Splat °³¼ö Á¦ÇÑ
void CMapOutdoor::SetSplatLimit(int iSplatNum)
{
    m_iSplatLimit = iSplatNum;
}

std::vector<int> &CMapOutdoor::GetRenderedSplatNum(int *piPatch, int *piSplat, float *pfSplatRatio)
{
    *piPatch = m_iRenderedPatchNum;
    *piSplat = m_iRenderedSplatNum;
    *pfSplatRatio = m_iRenderedSplatNumSqSum / float(m_iRenderedPatchNum);

    return m_RenderedTextureNumVector;
}

void CMapOutdoor::RenderBeforeLensFlare()
{
            rmt_ScopedCPUSample(RenderBeforeLensFlare, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderBeforeLensFlare **");

    m_LensFlare.DrawBeforeFlare();

    if (!m_activeEnvironment)
    {
        SPDLOG_ERROR("CMapOutdoor::RenderBeforeLensFlare mc_pEnvironmentData is NULL");
        return;
    }

    m_LensFlare.Compute(*(Vector3*)&m_activeEnvironment->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction);
    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderAfterLensFlare()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderAfterLensFlare **");
            rmt_ScopedCPUSample(RenderAfterLensFlare, 0);

    m_LensFlare.AdjustBrightness();
    m_LensFlare.DrawFlare();
    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderCollision()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderCollision **");

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->RenderCollision();
    }
    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderScreenFiltering()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderScreenFiltering **");
            rmt_ScopedCPUSample(RenderScreenFiltering, 0);

    m_ScreenFilter.Render();
    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderSky()
{
          rmt_ScopedCPUSample(RenderSky, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderSky **");

    if (IsVisiblePart(PART_SKY))
        m_SkyBox.Render();

    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderCloud()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderCloud **");
            rmt_ScopedCPUSample(RenderCloud, 0);

    if (IsVisiblePart(PART_CLOUD))
        m_SkyBox.RenderCloud();

    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderTree()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** RenderTree **");
    rmt_ScopedCPUSample(RenderTrees, 0);

    if (IsVisiblePart(PART_TREE))
        CSpeedTreeForestDirectX9::Instance().Render();

    D3DPERF_EndEvent();
}

void CMapOutdoor::SetInverseViewAndDynamicShaodwMatrices()
{
    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();

    if (!pCamera)
        return;

    m_matViewInverse = pCamera->GetInverseViewMatrix();

    Vector3 v3Target = pCamera->GetTarget();
    Vector3 v3Eye = pCamera->GetEye();

    Vector3 v3LightEye(v3Target.x - 1.732f * 1250.0f, v3Target.y - 1250.0f, v3Target.z + 2.0f * 1.732f * 1250.0f);
    m_matLightView = Matrix::CreateLookAt(v3LightEye, v3Target, Vector3::Backward);
    m_matDynamicShadow = m_matViewInverse * m_matLightView * m_matDynamicShadowScale;
}

void CMapOutdoor::OnRender()
{
    rmt_ScopedCPUSample(RenderOutdoor, 0);

    SetInverseViewAndDynamicShaodwMatrices();

    SetBlendOperation();
    RenderArea();

    RenderTree();
    RenderTerrain();
    RenderBlendArea();
}

struct FAreaRenderShadow
{
    void operator()(CGraphicObjectInstance *pInstance)
    {
        pInstance->RenderShadow();
    }
};

struct FPCBlockerHide
{
    void operator()(CGraphicObjectInstance *pInstance)
    {
        pInstance->Hide();
    }
};

struct FRenderPCBlocker
{
    void operator()(CGraphicObjectInstance *pInstance)
    {
        pInstance->Show();
        pInstance->RenderPCBlocker();
    }
};

void CMapOutdoor::RenderEffect()
{
    if (GetMapIndex() == 403)
        return;

    if (!IsVisiblePart(PART_AREA_EFFECT))
        return;
#ifndef WORLD_EDITOR
    if (!Engine::GetSettings().IsShowGeneralEffects())
        return;
#endif

    rmt_ScopedCPUSample(RenderMapEffects, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CMapOutdoor::RenderEffect **");

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = GetAreaPointer(i);
        if (pArea)
            pArea.value()->RenderEffect();
    }

    D3DPERF_EndEvent();
}

struct CMapOutdoor_LessThingInstancePtrRenderOrder
{
    bool operator()(CGraphicThingInstance *pkLeft, CGraphicThingInstance *pkRight)
    {
        // TODO : CameraÀ§Ä¡±â¹ÝÀ¸·Î ¼ÒÆÃ
        CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
        const Vector3 &c_rv3CameraPos = pCurrentCamera->GetEye();
        const Vector3 &c_v3LeftPos = pkLeft->GetPosition();
        const Vector3 &c_v3RightPos = pkRight->GetPosition();

        return Vector3::DistanceSquared(c_rv3CameraPos ,c_v3LeftPos) < 
               Vector3::DistanceSquared(c_rv3CameraPos, c_v3RightPos);
    }
};

struct CMapOutdoor_FOpaqueThingInstanceRender
{
    inline void operator()(CGraphicThingInstance *pkThingInst)
    {

        pkThingInst->Render();
    }
};

struct CMapOutdoor_FBlendThingInstanceRender
{
    inline void operator()(CGraphicThingInstance *pkThingInst)
    {
        pkThingInst->Render();
        pkThingInst->BlendRender();

    }
};

void CMapOutdoor::RenderArea(bool bRenderAmbience)
{
    if (!IsVisiblePart(PART_OBJECT))
        return;
    rmt_ScopedCPUSample(RenderArea, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** CMapOutdoor::RenderArea **");
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (m_activeEnvironment != NULL)
        {
            auto &light = m_activeEnvironment->DirLights[ENV_DIRLIGHT_BACKGROUND];
            ms->SetLightDirection(*(XMFLOAT3*)&light.Direction);
            ms->SetLightColor(Vector4(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b, light.Diffuse.a));
            ms->SetFogColor(m_activeEnvironment->FogColor);
            ms->SetFogParams(m_envInstance.GetFogNearDistance(), m_envInstance.GetFogFarDistance());
            ms->SetMaterial(m_activeEnvironment->Material);
        }
    }

    for (int j = 0; j < AROUND_AREA_NUM; ++j)
    {
        auto pArea = GetAreaPointer(j);
        if (!pArea)
            continue;

        pArea.value()->RenderDungeon();
    }


    // PCBlocker
    std::for_each(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), FPCBlockerHide());

    if (m_bDrawShadow && m_bDrawChrShadow)
    {
        if (ms)
        {
            ms->SetPassIndex(MODEL_SHADER_PASS_SHADOW);
        }


        std::for_each(m_ShadowReceiverVector.begin(), m_ShadowReceiverVector.end(), FAreaRenderShadow());


    }

    {
        if (ms)
        {
            ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE_OBJECT);
        }
        static std::vector<CGraphicThingInstance *> s_kVct_pkOpaqueThingInstSort;
        s_kVct_pkOpaqueThingInstSort.clear();

        for (int i = 0; i < AROUND_AREA_NUM; ++i)
        {
            CArea *pArea;
            if (GetAreaPointer(i, &pArea))
            {
                pArea->CollectRenderingObject(s_kVct_pkOpaqueThingInstSort);
#ifdef WORLD_EDITOR
                if (bRenderAmbience)
                    pArea->RenderAmbience();
#endif
            }
        }

        std::sort(s_kVct_pkOpaqueThingInstSort.begin(), s_kVct_pkOpaqueThingInstSort.end(),
                  CMapOutdoor_LessThingInstancePtrRenderOrder());
        std::for_each(s_kVct_pkOpaqueThingInstSort.begin(), s_kVct_pkOpaqueThingInstSort.end(),
                      CMapOutdoor_FOpaqueThingInstanceRender());
    }

    // Shadow Receiver
    if (m_bDrawShadow && m_bDrawChrShadow)
    {
        for (const auto &obj : m_ShadowReceiverVector)
            obj->Show();
    }

    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderBlendArea()
{
    if (!IsVisiblePart(PART_OBJECT))
        return;
    rmt_ScopedCPUSample(RenderBlendArea, 0);

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** CMapOutdoor::RenderBlendArea **");

    static std::vector<CGraphicThingInstance *> s_kVct_pkBlendThingInstSort;
    s_kVct_pkBlendThingInstSort.clear();

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        CArea *pArea;
        if (GetAreaPointer(i, &pArea))
        {
            pArea->CollectBlendRenderingObject(s_kVct_pkBlendThingInstSort);
        }
    }

    if (!s_kVct_pkBlendThingInstSort.empty())
    {
        std::sort(std::execution::par, s_kVct_pkBlendThingInstSort.begin(), s_kVct_pkBlendThingInstSort.end(),
                  CMapOutdoor_LessThingInstancePtrRenderOrder());

        std::for_each(std::execution::par, s_kVct_pkBlendThingInstSort.begin(), s_kVct_pkBlendThingInstSort.end(),
                      CMapOutdoor_FBlendThingInstanceRender());
    }

    D3DPERF_EndEvent();
}

void CMapOutdoor::RenderPCBlocker()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CMapOutdoor::RenderPCBlocker **");
        rmt_ScopedCPUSample(RenderPCBlocker, 0);

    // PCBlocker
    if (!m_PCBlockerVector.empty())
    {
        STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

		STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
		STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU,	D3DTADDRESS_CLAMP);
		STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV,	D3DTADDRESS_CLAMP);	

#ifndef CAMY_MODULE
		STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matBuildingTransparent);
		STATEMANAGER.SetTexture(1, m_BuildingTransparentImageInstance.GetTexturePointer()->GetD3DTexture());
#endif

        std::for_each(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), FRenderPCBlocker());
#ifndef CAMY_MODULE
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);
#endif

        STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
		STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
		STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);
		STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    }

    D3DPERF_EndEvent();
}

void CMapOutdoor::SelectIndexBuffer(uint8_t byLODLevel, uint16_t *pwPrimitiveCount, D3DPRIMITIVETYPE *pePrimitiveType)
{
    if (0 == byLODLevel)
    {
        *pwPrimitiveCount = m_wNumIndices[0] - 2;
        *pePrimitiveType = D3DPT_TRIANGLESTRIP;
    }
    else
    {
        *pwPrimitiveCount = m_wNumIndices[byLODLevel] / 3;
        *pePrimitiveType = D3DPT_TRIANGLELIST;
    }

    Engine::GetDevice().SetIndexBuffer(m_IndexBuffer[byLODLevel]);
}

float CMapOutdoor::__GetNoFogDistance()
{
    return (float)(CTerrainImpl::CELLSCALE * m_lViewRadius) * 0.5f;
}

float CMapOutdoor::__GetFogDistance()
{
    return (float)(CTerrainImpl::CELLSCALE * m_lViewRadius) * 0.75f;
}

struct FPatchNumMatch
{
    long m_lPatchNumToCheck;

    FPatchNumMatch(long lPatchNum)
    {
        m_lPatchNumToCheck = lPatchNum;
    }

    bool operator()(std::pair<long, uint8_t> aPair)
    {
        return m_lPatchNumToCheck == aPair.first;
    }
};

void CMapOutdoor::NEW_DrawWireFrame(CTerrainPatchProxy *pTerrainPatchProxy, uint16_t wPrimitiveCount,
                                    D3DPRIMITIVETYPE ePrimitiveType)
{
    uint32_t dwFillMode = STATEMANAGER.GetRenderState(D3DRS_FILLMODE);
    STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    uint32_t dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

    STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetTexture(1, nullptr);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

    Engine::GetDevice().DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

    STATEMANAGER.SetRenderState(D3DRS_FILLMODE, dwFillMode);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

void CMapOutdoor::DrawWireFrame(long patchnum, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::DrawWireFrame");

    CTerrainPatchProxy *pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

    if (!pTerrainPatchProxy->isUsed())
        return;

    long sPatchNum = pTerrainPatchProxy->GetPatchNum();
    if (sPatchNum < 0)
        return;
    uint8_t ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
    if (0xFF == ucTerrainNum)
        return;

    uint32_t dwFillMode = STATEMANAGER.GetRenderState(D3DRS_FILLMODE);
    STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    uint32_t dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, false);

    STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetTexture(1, nullptr);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

    Engine::GetDevice().DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

    STATEMANAGER.SetRenderState(D3DRS_FILLMODE, dwFillMode);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

// Attr
void CMapOutdoor::RenderMarkedArea()
{
    if (!m_pTerrainPatchProxyList)
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CMapOutdoor::RenderMarkedArea **");

    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

    WORD wPrimitiveCount;
    D3DPRIMITIVETYPE eType;
    SelectIndexBuffer(0, &wPrimitiveCount, &eType);

    Matrix matTexTransform, matTexTransformTemp;

    matTexTransform = Matrix::CreateScale(m_fTerrainTexCoordBase * 32.0f, -m_fTerrainTexCoordBase * 32.0f, 0.0f);
    matTexTransform = m_matViewInverse * matTexTransform;
    STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &matTexTransform);
    STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &matTexTransform);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    static long lStartTime = timeGetTime();
    float fTime = float((timeGetTime() - lStartTime) % 3000) / 3000.0f;
    float fAlpha = fabs(fTime - 0.5f) / 2.0f + 0.1f;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, fAlpha).BGRA());
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    STATEMANAGER.SetTexture(0, m_attrImageInstance.GetTexturePointer()->GetD3DTexture());

    RecurseRenderAttr(m_pRootNode);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);

    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

    D3DPERF_EndEvent();
}

void CMapOutdoor::RecurseRenderAttr(CTerrainQuadtreeNode *Node, bool bCullEnable)
{
    if (bCullEnable)
    {
        if (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius) == VIEW_NONE)
            return;
    }

    {
        if (Node->Size == 1)
        {
            DrawPatchAttr(Node->PatchNum);
        }
        else
        {
            if (Node->NW_Node != nullptr)
                RecurseRenderAttr(Node->NW_Node, bCullEnable);
            if (Node->NE_Node != nullptr)
                RecurseRenderAttr(Node->NE_Node, bCullEnable);
            if (Node->SW_Node != nullptr)
                RecurseRenderAttr(Node->SW_Node, bCullEnable);
            if (Node->SE_Node != nullptr)
                RecurseRenderAttr(Node->SE_Node, bCullEnable);
        }
    }
}

void CMapOutdoor::DrawPatchAttr(long patchnum)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(251, 50, 50, 0), L" CMapOutdoor::DrawPatchAttr");

    CTerrainPatchProxy *pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
    if (!pTerrainPatchProxy->isUsed())
        return;

    long sPatchNum = pTerrainPatchProxy->GetPatchNum();
    if (sPatchNum < 0)
        return;

    BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
    if (0xFF == ucTerrainNum)
        return;

    // Deal with this material buffer
    CTerrain *pTerrain;
    if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
        return;

    if (!pTerrain->IsMarked())
        return;

    WORD wCoordX, wCoordY;
    pTerrain->GetCoordinate(&wCoordX, &wCoordY);

    m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
    m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);

    Matrix matTexTransform, matTexTransformTemp;
    matTexTransform = m_matViewInverse * m_matWorldForCommonUse;
    matTexTransform = matTexTransform * m_matStaticShadow;
    STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matTexTransform);

    TTerrainSplatPatch &rAttrSplatPatch = pTerrain->GetMarkedSplatPatch();
    STATEMANAGER.SetTexture(1, rAttrSplatPatch.Splats[0].pd3dTexture);

    VertexBufferHandle pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
    if (!isValid(pkVB))
        return;

    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_NORMAL);
    Engine::GetDevice().SetVertexBuffer(pkVB, 0, 0);
	Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices[0] - 2);

    D3DPERF_EndEvent();
}
