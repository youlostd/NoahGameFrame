﻿#include "StdAfx.h"

#include "../EterBase/StepTimer.h"
#include "../eterLib/ResourceManager.h"
#include "../eterLib/StateManager.h"

#include "../EterLib/Engine.hpp"
#include "../eterBase/Timer.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "base/Remotery.h"

void CMapOutdoor::LoadWaterTexture()
{
    UnloadWaterTexture();

    auto &resMgr = CResourceManager::Instance();

    char buf[256];
    for (int i = 0; i < 30; ++i)
    {
        sprintf(buf, "d:/ymir Work/special/water/%02d.dds", i + 1);
        m_WaterInstances[i].SetImagePointer(resMgr.LoadResource<CGraphicImage>(buf));
    }
}

void CMapOutdoor::UnloadWaterTexture()
{
    for (int i = 0; i < 30; ++i)
        m_WaterInstances[i].Destroy();
}

void CMapOutdoor::RenderWater()
{
    if (m_PatchVector.empty())
        return;

    if (!IsVisiblePart(PART_WATER))
        return;

    rmt_ScopedCPUSample(RenderWater, 0);
    Engine::GetDevice().ShaderOff();
    //////////////////////////////////////////////////////////////////////////
    // RenderState
    Matrix matTexTransformWater;
    Engine::GetDevice().SetDepthEnable(true, false);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    STATEMANAGER.SaveRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, true);

    STATEMANAGER.SetTexture(0, m_WaterInstances[((ELTimer_GetMSec() / 70) % 30)].GetTexturePointer()->GetD3DTexture());

    matTexTransformWater = Matrix::CreateScale(m_fWaterTexCoordBase, -m_fWaterTexCoordBase, 0.0f);
    matTexTransformWater = m_matViewInverse * matTexTransformWater;

    STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &matTexTransformWater);
    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    STATEMANAGER.SetTexture(1, NULL);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    // RenderState
    //////////////////////////////////////////////////////////////////////////

    // 물 위 아래 애니시키기...
    static float s_fWaterHeightCurrent = 0;
    static float s_fWaterHeightBegin = 0;
    static float s_fWaterHeightEnd = 0;
    static uint64_t s_dwLastHeightChangeTime = DX::StepTimer::instance().GetTotalMillieSeconds();
    static uint64_t s_dwBlendtime = 300;

    // 1.5초 마다 변경
    if ((DX::StepTimer::instance().GetTotalMillieSeconds() - s_dwLastHeightChangeTime) > s_dwBlendtime)
    {
        s_dwBlendtime = Random::get(1000, 3000);

        if (s_fWaterHeightEnd == 0)
            s_fWaterHeightEnd = -Random::get(0, 15);
        else
            s_fWaterHeightEnd = 0;

        s_fWaterHeightBegin = s_fWaterHeightCurrent;
        s_dwLastHeightChangeTime = DX::StepTimer::instance().GetTotalMillieSeconds();
    }

    s_fWaterHeightCurrent = s_fWaterHeightBegin +
                            (s_fWaterHeightEnd - s_fWaterHeightBegin) *
                                (float)((DX::StepTimer::instance().GetTotalMillieSeconds() - s_dwLastHeightChangeTime) /
                                        (float)s_dwBlendtime);
    m_matWorldForCommonUse._43 = s_fWaterHeightCurrent;

    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

    float fFogDistance = __GetFogDistance();

    std::vector<std::pair<float, long>>::iterator i;

    for (i = m_PatchVector.begin(); i != m_PatchVector.end(); ++i)
    {
        if (i->first < fFogDistance)
            DrawWater(i->second);
    }

   // STATEMANAGER.SetTexture(0, NULL);
   // STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, false);

    //for (i = m_PatchVector.begin(); i != m_PatchVector.end(); ++i)
    //{
    //    if (i->first >= fFogDistance)
    //        DrawWater(i->second);
    //}

    // 렌더링 한 후에는 물 z 위치를 복구
    m_matWorldForCommonUse._43 = 0.0f;

    //////////////////////////////////////////////////////////////////////////
    // RenderState
    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

    STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
    STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    Engine::GetDevice().SetDepthEnable(true, true);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CMapOutdoor::DrawWater(long patchnum)
{
    assert(NULL != m_pTerrainPatchProxyList);
    if (!m_pTerrainPatchProxyList)
        return;

    CTerrainPatchProxy &rkTerrainPatchProxy = m_pTerrainPatchProxyList[patchnum];

    if (!rkTerrainPatchProxy.isUsed())
        return;

    if (!rkTerrainPatchProxy.isWaterExists())
        return;

    auto pkVB = rkTerrainPatchProxy.GetWaterVertexBufferPointer();
    if (!isValid(pkVB))
        return;

    UINT uPriCount = rkTerrainPatchProxy.GetWaterFaceCount();
    if (!uPriCount)
        return;

    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
    Engine::GetDevice().SetVertexBuffer(pkVB, 0, 0);
    Engine::GetDevice().DrawPrimitive(D3DPT_TRIANGLELIST, 0, uPriCount);

    ms_faceCount += uPriCount;
}
