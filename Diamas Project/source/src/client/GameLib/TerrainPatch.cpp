// TerrainPatch.cpp: implementation of the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "TerrainPatch.h"
#include "stdafx.h"

#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CTerrainPatch::Clear()
{
    Engine::GetDevice().DeleteVertexBuffer(m_kVB);
    Engine::GetDevice().DeleteVertexBuffer(m_WaterVertexBuffer);
    
    ClearID();
    SetUse(false);

    m_bWaterExist = false;
    m_bNeedUpdate = true;

    m_dwWaterPriCount = 0;
    m_byType = PATCH_TYPE_PLAIN;

    m_fMinX = m_fMaxX = m_fMinY = m_fMaxY = m_fMinZ = m_fMaxZ = 0.0f;

    m_dwVersion = 0;
}

void CTerrainPatch::BuildWaterVertexBuffer(SWaterVertex *akSrcVertex, UINT uWaterVertexCount)
{

    UINT uVBSize = sizeof(SWaterVertex) * uWaterVertexCount;
    m_WaterVertexBuffer = Engine::GetDevice().CreateVertexBuffer(uVBSize, sizeof(SWaterVertex), akSrcVertex);
    m_dwWaterPriCount = uWaterVertexCount / 3;
}

void CTerrainPatch::BuildTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex *akSrcVertex)
{
    __BuildHardwareTerrainVertexBuffer(akSrcVertex);
}

void CTerrainPatch::__BuildHardwareTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex *akSrcVertex)
{
    UINT uVBSize = sizeof(HardwareTransformPatch_SSourceVertex) * TERRAIN_VERTEX_COUNT;
    m_kVB =
        Engine::GetDevice().CreateVertexBuffer(uVBSize, sizeof(HardwareTransformPatch_SSourceVertex), akSrcVertex);
}

UINT CTerrainPatch::GetWaterFaceCount()
{
    return m_dwWaterPriCount;
}

CTerrainPatchProxy::CTerrainPatchProxy()
{
    Clear();
}

CTerrainPatchProxy::~CTerrainPatchProxy()
{
    Clear();
}

void CTerrainPatchProxy::SetCenterPosition(const Vector3 &c_rv3Center)
{
    m_v3Center = c_rv3Center;
}

bool CTerrainPatchProxy::IsIn(const Vector3 &c_rv3Target, float fRadius)
{
    float dx = m_v3Center.x - c_rv3Target.x;
    float dy = m_v3Center.y - c_rv3Target.y;
    float fDist = dx * dx + dy * dy;
    float fCheck = fRadius * fRadius;

    if (fDist < fCheck)
        return true;

    return false;
}

VertexBufferHandle CTerrainPatchProxy::HardwareTransformPatch_GetVertexBufferPtr()
{
    if (m_pTerrainPatch)
        return m_pTerrainPatch->HardwareTransformPatch_GetVertexBufferPtr();

    return ENGINE_INVALID_HANDLE;
}

UINT CTerrainPatchProxy::GetWaterFaceCount()
{
    if (m_pTerrainPatch)
        return m_pTerrainPatch->GetWaterFaceCount();

    return 0;
}

void CTerrainPatchProxy::Clear()
{
    m_bUsed = false;
    m_sPatchNum = 0;
    m_byTerrainNum = 0xFF;

    m_pTerrainPatch = NULL;
}
