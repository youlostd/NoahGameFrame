#include "MapOutdoor.h"
#include "AreaTerrain.h"
#include "EnvironmentData.hpp"
#include "Property.h"
#include "PropertyManager.h"
#include "StdAfx.h"
#include "TerrainPatch.h"

#include "../EffectLib/EffectManager.h"

#include "../SpeedTreeLib/SpeedTreeForest.h"

#include "../EterLib/Camera.h"
#include "../EterLib/CullingManager.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/StateManager.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "../EterGrnLib/GrannyState.hpp"
#include "../EterGrnLib/ModelShader.h"
#include "../EterLib/RenderTargetManager.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"

struct FGetObjectHeight
{
    bool m_bHeightFound;
    float m_fReturnHeight;
    float m_fRequestX, m_fRequestY;

    FGetObjectHeight(float fRequestX, float fRequestY)
    {
        m_fRequestX = fRequestX;
        m_fRequestY = fRequestY;
        m_bHeightFound = false;
        m_fReturnHeight = 0.0f;
    }

    void operator()(CGraphicObjectInstance *pObject)
    {
        if (pObject->GetObjectHeight(m_fRequestX, m_fRequestY, &m_fReturnHeight))
        {
            m_bHeightFound = true;
        }
    }
};

struct FGetPickingPoint
{
    Vector3 m_v3Start;
    Vector3 m_v3Dir;
    Vector3 m_v3PickingPoint;
    bool m_bPicked;

    FGetPickingPoint(Vector3 &v3Start, Vector3 &v3Dir) : m_v3Start(v3Start), m_v3Dir(v3Dir), m_bPicked(false)
    {
    }

    void operator()(CGraphicObjectInstance *pInstance)
    {
        if (pInstance && pInstance->GetType() == CGraphicThingInstance::ID)
        {
            CGraphicThingInstance *pThing = (CGraphicThingInstance *)pInstance;
            if (!pThing->IsObjectHeight())
                return;

            float fX, fY, fZ;
            if (pThing->Picking(m_v3Start, m_v3Dir, fX, fY))
            {
                if (pThing->GetObjectHeight(fX, -fY, &fZ))
                {
                    m_v3PickingPoint.x = fX;
                    m_v3PickingPoint.y = fY;
                    m_v3PickingPoint.z = fZ;
                    m_bPicked = true;
                }
            }
        }
    }
};

CMapOutdoor::CMapOutdoor()
{

    auto &resMgr = CResourceManager::Instance();
    auto white = resMgr.LoadResource<CGraphicImage>("d:/ymir work/special/white.dds");
    if (white)
    {
        m_attrImageInstance.SetImagePointer(white);
    }
    else
    {
        SPDLOG_ERROR("Could not load attr image white.dds");
    }
    m_BuildingTransparentImageInstance.SetImagePointer(
        resMgr.LoadResource<CGraphicImage>("d:/ymir Work/special/PCBlockerAlpha.dds"));

    //m_areaLoader.Create(0);
    CMapOutdoor::Initialize();
}

CMapOutdoor::~CMapOutdoor()
{
    CMapOutdoor::Destroy();
    //m_areaLoader.Shutdown();
}

bool CMapOutdoor::Initialize()
{

    uint8_t i;
    for (i = 0; i < AROUND_AREA_NUM; ++i)
    {
        m_pArea[i] = nullptr;
        m_pTerrain[i] = nullptr;
    }

    m_pTerrainPatchProxyList = nullptr;

    m_lViewRadius = 0L;
    m_fHeightScale = 0.0f;

    m_sTerrainCountX = m_sTerrainCountY = 0;

    m_CurCoordinate.m_sTerrainCoordX = -1;
    m_CurCoordinate.m_sTerrainCoordY = -1;
    m_PrevCoordinate.m_sTerrainCoordX = -1;
    m_PrevCoordinate.m_sTerrainCoordY = -1;

    m_lCenterX = m_lCenterY = 0;
    m_lOldReadX = m_lOldReadY = -1;

    std::memset(m_pwaIndices, 0, sizeof(m_pwaIndices));
    for (i = 0; i < TERRAINPATCH_LODMAX; ++i) {
        //Engine::GetDevice().DeleteIndexBuffer(m_IndexBuffer[i]);
        m_IndexBuffer[i] = ENGINE_INVALID_HANDLE;
    }

    m_bDrawWireFrame = false;
    m_bDrawShadow = false;
    m_bDrawChrShadow = false;

    m_iSplatLimit = 50000;

    m_wPatchCount = 0;

    m_pRootNode = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // Character Shadow
    m_lpCharacterShadowMapTexture = nullptr;
    m_lpCharacterShadowMapColorSurface = nullptr;
    m_wShadowMapSize = 0;

    m_lpBackupRenderTargetSurface = nullptr;
    m_lpBackupDepthSurface = nullptr;
    // Character Shadow
    //////////////////////////////////////////////////////////////////////////

    m_iRenderedPatchNum = 0;
    m_iRenderedSplatNum = 0;

    //////////////////////////////////////////////////////////////////////////
    m_fOpaqueWaterDepth = 400.0f;

    //////////////////////////////////////////////////////////////////////////
    m_TerrainVector.clear();
    m_TerrainDeleteVector.clear();

    m_AreaVector.clear();
    m_AreaDeleteVector.clear();
    //////////////////////////////////////////////////////////////////////////

    m_PatchVector.clear();

    m_activeEnvironment = nullptr;

    m_eTerrainRenderSort = DISTANCE_SORT;

    m_matWorldForCommonUse = Matrix::Identity;

    InitializeVisibleParts();

    m_setting_environmentFiles.clear();
    m_bShowEntirePatchTextureCount = false;
    m_bTransparentTree = true;

    CMapBase::Clear();

    __XMasTree_Initialize();
    SpecialEffect_Destroy();

    m_bEnablePortal = false;

    m_fTerrainTexCoordBase = 1.0f / (float)(CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE);
    m_fWaterTexCoordBase = 1.0f / (float)(CTerrainImpl::CELLSCALE * 4);

    m_matSplatAlpha = Matrix::CreateScale(
                      +m_fTerrainTexCoordBase * 2.0f * (float)(CTerrainImpl::PATCH_XSIZE) /
                          (float)(CTerrainImpl::SPLATALPHA_RAW_XSIZE - 2),
                      -m_fTerrainTexCoordBase * 2.0f * (float)(CTerrainImpl::PATCH_YSIZE) /
                          (float)(CTerrainImpl::SPLATALPHA_RAW_XSIZE - 2),
                      0.0f);
    m_matSplatAlpha._41 = m_fTerrainTexCoordBase * 4.6f;
    m_matSplatAlpha._42 = m_fTerrainTexCoordBase * 4.6f;

    m_matStaticShadow = Matrix::CreateScale(
                      +m_fTerrainTexCoordBase * ((float)CTerrainImpl::PATCH_XSIZE / CTerrainImpl::XSIZE),
                      -m_fTerrainTexCoordBase * ((float)CTerrainImpl::PATCH_YSIZE / CTerrainImpl::XSIZE), 0.0f);
    m_matStaticShadow._41 = 0.0f;
    m_matStaticShadow._42 = 0.0f;

    m_matDynamicShadowScale = Matrix::CreateScale(1.0f / 2550.0f, -1.0f / 2550.0f, 1.0f);
    m_matDynamicShadowScale._41 = 0.5f;
    m_matDynamicShadowScale._42 = 0.5f;


    return true;
}

bool CMapOutdoor::Destroy()
{
    m_bEnablePortal = false;

    XMasTree_Destroy();

    DestroyTerrain();
    DestroyArea();
    DestroyTerrainPatchProxyList();

    FreeQuadTree();
    ReleaseCharacterShadowTexture();
    //CRenderTargetManager::instance().ReleaseRenderTargetTextures();

    m_activeEnvironment = nullptr;
    m_envInstance.SetData(nullptr);
    m_environments.clear();

    m_rkList_kGuildArea.clear();
    CSpeedTreeForestDirectX9::Instance().Clear();
    return true;
}

void CMapOutdoor::Clear()
{
    UnloadWaterTexture();
    Destroy();    // 해제
    Initialize(); // 초기화
}

bool CMapOutdoor::SetTerrainCount(short sTerrainCountX, short sTerrainCountY)
{
    if (0 == sTerrainCountX || MAX_MAPSIZE < sTerrainCountX)
        return false;

    if (0 == sTerrainCountY || MAX_MAPSIZE < sTerrainCountY)
        return false;

    m_sTerrainCountX = sTerrainCountX;
    m_sTerrainCountY = sTerrainCountY;
    return true;
}

void CMapOutdoor::SetActiveEnvironment(const EnvironmentData *env)
{
    m_activeEnvironment = env;

    if (!env)
        return;

    m_envInstance.SetData(env);
    m_envInstance.ApplyTo(m_ScreenFilter);
    m_envInstance.ApplyTo(m_SkyBox);
    m_envInstance.ApplyTo(m_LensFlare);

    auto &forest = CSpeedTreeForestDirectX9::Instance();
    forest.SetFog(m_envInstance.GetFogNearDistance(),
                  m_envInstance.GetFogFarDistance());

    const auto &light = env->DirLights[ENV_DIRLIGHT_CHARACTER];
    forest.SetLight((const float *)&light.Direction,
                    (const float *)&light.Ambient,
                    (const float *)&light.Diffuse);
    forest.SetWindStrength(env->fWindStrength);

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetLightDirection(*(Vector3*)&light.Direction);
        ms->SetLightColor(Vector4(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b, light.Diffuse.a));
        ms->SetFogColor(m_envInstance.GetData()->FogColor);
        ms->SetFogParams(m_envInstance.GetFogNearDistance(), m_envInstance.GetFogFarDistance());
        ms->SetMaterial(env->Material);
        STATEMANAGER.SetMaterial(&env->Material);


    }
    // forest.SetSpeedWindFile(env->fWindStrength);
}

const EnvironmentInstance &CMapOutdoor::GetActiveEnvironment() const
{
    return m_envInstance;
}

EnvironmentInstance *CMapOutdoor::GetActiveEnvironmentPtr()
{
    return &m_envInstance;
}

/*virtual*/
bool CMapOutdoor::RegisterEnvironment(uint32_t id, const char *filename)
{
    if (!m_environments[id].Load(filename))
    {
        m_environments.erase(id);
        return false;
    }

    return true;
}

const EnvironmentData *CMapOutdoor::GetEnvironment(uint32_t id) const
{
    const auto it = m_environments.find(id);
    if (it != m_environments.end())
        return &it->second;

    return nullptr;
}

void CMapOutdoor::SetWireframe(bool bWireFrame)
{
    m_bDrawWireFrame = bWireFrame;
}

bool CMapOutdoor::IsWireframe()
{
    return m_bDrawWireFrame;
}

//////////////////////////////////////////////////////////////////////////
// TerrainPatchList
//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::CreateTerrainPatchProxyList()
{
    m_wPatchCount = ((m_lViewRadius * 2) / TERRAIN_PATCHSIZE) + 2;

    m_pTerrainPatchProxyList = new CTerrainPatchProxy[m_wPatchCount * m_wPatchCount];

    m_iPatchTerrainVertexCount = (TERRAIN_PATCHSIZE + 1) * (TERRAIN_PATCHSIZE + 1);
    m_iPatchWaterVertexCount = TERRAIN_PATCHSIZE * TERRAIN_PATCHSIZE * 6;
    m_iPatchTerrainVertexSize = 24;
    m_iPatchWaterVertexSize = 16;

    SetIndexBuffer();
}

void CMapOutdoor::DestroyTerrainPatchProxyList()
{
    if (m_pTerrainPatchProxyList)
    {
        delete[] m_pTerrainPatchProxyList;
        m_pTerrainPatchProxyList = nullptr;
    }

    for (auto &i : m_IndexBuffer) {
        Engine::GetDevice().DeleteIndexBuffer(i);
        i = ENGINE_INVALID_HANDLE;
    }
}

//////////////////////////////////////////////////////////////////////////
// Area
//////////////////////////////////////////////////////////////////////////

void CMapOutdoor::EnablePortal(bool bFlag)
{
    m_bEnablePortal = bFlag;

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
        if (m_pArea[i])
            m_pArea[i]->EnablePortal(bFlag);
}

void CMapOutdoor::DestroyArea()
{
    m_AreaVector.clear();
    m_AreaDeleteVector.clear();

    for (auto &i : m_pArea)
        i = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Terrain
//////////////////////////////////////////////////////////////////////////

void CMapOutdoor::DestroyTerrain()
{
    m_TerrainVector.clear();
    m_TerrainDeleteVector.clear();

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
        m_pTerrain[i] = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// New
//////////////////////////////////////////////////////////////////////////

bool CMapOutdoor::GetTerrainNum(float fx, float fy, uint8_t *pbyTerrainNum)
{
    if (fy < 0)
        fy = -fy;

    int ix, iy;

    PR_FLOAT_TO_INT(fx, ix);
    PR_FLOAT_TO_INT(fy, iy);

    uint16_t wTerrainNumX = ix / (CTerrainImpl::TERRAIN_XSIZE);
    uint16_t wTerrainNumY = iy / (CTerrainImpl::TERRAIN_YSIZE);

    return GetTerrainNumFromCoord(wTerrainNumX, wTerrainNumY, pbyTerrainNum);
}

bool CMapOutdoor::GetPickingPoint(Vector3 *v3IntersectPt)
{
    return GetPickingPointWithRay(ms_Ray, v3IntersectPt);
}

bool CMapOutdoor::__PickTerrainHeight(float &fPos, const Vector3 &v3Start, const Vector3 &v3End, float fStep,
                                      float fRayRange, float fLimitRange, Vector3 *pv3Pick)
{
    CTerrain *pTerrain;

    float fRayRangeInv = 1.0f / fRayRange;
    while (fPos < fRayRange && fPos < fLimitRange)
    {
        Vector3 v3CurPos = Vector3::Lerp(v3Start, v3End, fPos * fRayRangeInv);
        uint8_t byTerrainNum;
        float fMultiplier = 1.0f;
        if (GetTerrainNum(v3CurPos.x, v3CurPos.y, &byTerrainNum))
        {
            if (GetTerrainPointer(byTerrainNum, &pTerrain))
            {
                int ix, iy;
                PR_FLOAT_TO_INT(v3CurPos.x, ix);
                PR_FLOAT_TO_INT(fabs(v3CurPos.y), iy);
                float fMapHeight = pTerrain->GetHeight(ix, iy);
                if (fMapHeight >= v3CurPos.z)
                {
                    *pv3Pick = v3CurPos;
                    return true;
                }
                else
                {
                    fMultiplier = std::max(1.0f, 0.01f * (v3CurPos.z - fMapHeight));
                }
            }
        }
        fPos += fStep * fMultiplier;
    }
    return false;
}

bool CMapOutdoor::GetPickingPointWithRay(const CRay &rRay, Vector3 *v3IntersectPt)
{
    bool bObjectPick = false;
    bool bTerrainPick = false;
    Vector3 v3ObjectPick, v3TerrainPick;

    Vector3 v3Start, v3End, v3Dir, v3CurPos;
    float fRayRange;
    rRay.GetStartPoint(&v3Start);
    rRay.GetDirection(&v3Dir, &fRayRange);
    rRay.GetEndPoint(&v3End);

    Vector3d v3dStart = {v3Start.x, v3Start.y, v3Start.z};
    Vector3d v3dEnd = {v3End.x - v3Start.x, v3End.y - v3Start.y, v3End.z - v3Start.z};

    // TODO(tim): Is this the part that causes problems with bridges?
    if (IsVisiblePart(PART_TERRAIN))
    {
        // uint32_t baseTime = timeGetTime();
        CCullingManager &rkCullingMgr = CCullingManager::Instance();
        FGetPickingPoint kGetPickingPoint(v3Start, v3Dir);
        rkCullingMgr.ForInRange2d(v3dStart, &kGetPickingPoint);

        if (kGetPickingPoint.m_bPicked)
        {
            bObjectPick = true;
            v3ObjectPick = kGetPickingPoint.m_v3PickingPoint;
        }
    }

    float fPos = 0.0f;
    // float fStep = 1.0f;
    // float fRayRangeInv=1.0f/fRayRange;

    bTerrainPick = true;
    if (!__PickTerrainHeight(fPos, v3Start, v3End, 5.0f, fRayRange, 5000.0f, &v3TerrainPick))
        if (!__PickTerrainHeight(fPos, v3Start, v3End, 10.0f, fRayRange, 10000.0f, &v3TerrainPick))
            if (!__PickTerrainHeight(fPos, v3Start, v3End, 100.0f, fRayRange, 100000.0f, &v3TerrainPick))
                bTerrainPick = false;

    if (bObjectPick && bTerrainPick)
    {
        if (Vector3::Distance(v3ObjectPick, v3Start) >= Vector3::Distance(v3TerrainPick,v3Start))
            *v3IntersectPt = v3TerrainPick;
        else
            *v3IntersectPt = v3ObjectPick;
        return true;
    }
    else if (bObjectPick)
    {
        *v3IntersectPt = v3ObjectPick;
        return true;
    }
    else if (bTerrainPick)
    {
        *v3IntersectPt = v3TerrainPick;
        return true;
    }

    return false;
}

bool CMapOutdoor::GetPickingPointWithRayOnlyTerrain(const CRay &rRay, Vector3 *v3IntersectPt)
{
    bool bTerrainPick = false;
    Vector3 v3TerrainPick;

    Vector3 v3Start, v3End, v3Dir, v3CurPos;
    float fRayRange;
    rRay.GetStartPoint(&v3Start);
    rRay.GetDirection(&v3Dir, &fRayRange);
    rRay.GetEndPoint(&v3End);

    Vector3 v3dStart = Vector3(v3Start.x, v3Start.y, v3Start.z);
    Vector3 v3dEnd = Vector3(v3End.x - v3Start.x, v3End.y - v3Start.y, v3End.z - v3Start.z);

    float fPos = 0.0f;
    bTerrainPick = true;
    if (!__PickTerrainHeight(fPos, v3Start, v3End, 5.0f, fRayRange, 5000.0f, &v3TerrainPick))
        if (!__PickTerrainHeight(fPos, v3Start, v3End, 10.0f, fRayRange, 10000.0f, &v3TerrainPick))
            if (!__PickTerrainHeight(fPos, v3Start, v3End, 100.0f, fRayRange, 100000.0f, &v3TerrainPick))
                bTerrainPick = false;

    if (bTerrainPick)
    {
        *v3IntersectPt = v3TerrainPick;
        return true;
    }

    return false;
}

/*
{
    bool bTerrainPick = false;
    Vector3 v3TerrainPick;

    CTerrain * pTerrain;

    Vector3 v3Start, v3End, v3Dir, v3CurPos;
    float fRayRange;
    rRay.GetStartPoint(&v3Start);
    rRay.GetDirection(&v3Dir, &fRayRange);
    rRay.GetEndPoint(&v3End);

    Vector3d v3dStart, v3dEnd;
    v3dStart.Set(v3Start.x, v3Start.y, v3Start.z);
    v3dEnd.Set(v3End.x - v3Start.x, v3End.y - v3Start.y, v3End.z - v3Start.z);

    float fAdd = 1.0f / fRayRange;

    float ft = 0.0f;
    while (ft < 1.0f)
    {
        D3DXVec3Lerp(&v3CurPos, &v3Start, &v3End, ft);
        uint8_t byTerrainNum;
        float fMultiplier = 1.0f;
        if (GetTerrainNum(v3CurPos.x, v3CurPos.y, &byTerrainNum))
        {
            if (GetTerrainPointer(byTerrainNum, &pTerrain))
            {
                int ix, iy;
                PR_FLOAT_TO_INT(v3CurPos.x, ix);
                PR_FLOAT_TO_INT(fabs(v3CurPos.y), iy);
                float fMapHeight = pTerrain->GetHeight(ix, iy);
                if ( fMapHeight >= v3CurPos.z)
                {
                    bTerrainPick = true;
                    v3TerrainPick = v3CurPos;
                    break;
                }
                else
                    fMultiplier = std::max(1.0f, 0.01f * ( v3CurPos.z - fMapHeight ) );
            }
        }
        ft += fAdd * fMultiplier;
    }

    if (bTerrainPick)
    {
        *v3IntersectPt = v3TerrainPick;
        return true;
    }

    return false;
}
*/

void CMapOutdoor::GetHeightMap(const uint8_t &c_rucTerrainNum, uint16_t **pwHeightMap)
{
    if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
    {
        *pwHeightMap = nullptr;
        return;
    }

    *pwHeightMap = m_pTerrain[c_rucTerrainNum]->GetHeightMap();
}

void CMapOutdoor::GetNormalMap(const uint8_t &c_rucTerrainNum, char **pucNormalMap)
{
    if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
    {
        *pucNormalMap = nullptr;
        return;
    }

    *pucNormalMap = m_pTerrain[c_rucTerrainNum]->GetNormalMap();
}

void CMapOutdoor::GetWaterMap(const uint8_t &c_rucTerrainNum, uint8_t **pucWaterMap)
{
    if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
    {
        *pucWaterMap = nullptr;
        return;
    }

    *pucWaterMap = m_pTerrain[c_rucTerrainNum]->GetWaterMap();
}

void CMapOutdoor::GetWaterHeight(uint8_t byTerrainNum, uint8_t byWaterNum, long *plWaterHeight)
{
    if (byTerrainNum < 0 || byTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[byTerrainNum])
    {
        *plWaterHeight = -1;
        return;
    }

    m_pTerrain[byTerrainNum]->GetWaterHeight(byWaterNum, plWaterHeight);
}

bool CMapOutdoor::GetWaterHeight(int iX, int iY, long *plWaterHeight)
{
    if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE ||
        iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
        return false;

    uint16_t wTerrainCoordX, wTerrainCoordY;
    wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
    wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;

    uint8_t byTerrainNum;
    if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
        return false;
    CTerrain *pTerrain;
    if (!GetTerrainPointer(byTerrainNum, &pTerrain))
        return false;

    uint16_t wLocalX, wLocalY;
    wLocalX = (iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::WATERMAP_XSIZE);
    wLocalY = (iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::WATERMAP_YSIZE);

    return pTerrain->GetWaterHeight(wLocalX, wLocalY, plWaterHeight);
}

//////////////////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////////////////

bool CMapOutdoor::GetTerrainNumFromCoord(uint16_t wCoordX, uint16_t wCoordY, uint8_t *pbyTerrainNum)
{
    *pbyTerrainNum = (wCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
                     (wCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH);

    if (*pbyTerrainNum < 0 || *pbyTerrainNum > AROUND_AREA_NUM)
        return false;
    return true;
}

void CMapOutdoor::BuildViewFrustum(Matrix &mat)
{
    // m_plane[0] = Plane(mat._14 + mat._13, mat._24 + mat._23, mat._34 + mat._33, mat._44 + mat._43);
    m_plane[0] = Plane(mat._13, mat._23, mat._33, mat._43);                                         // Near
    m_plane[1] = Plane(mat._14 - mat._13, mat._24 - mat._23, mat._34 - mat._33, mat._44 - mat._43); // Far
    m_plane[2] = Plane(mat._14 + mat._11, mat._24 + mat._21, mat._34 + mat._31, mat._44 + mat._41); // Left
    m_plane[3] = Plane(mat._14 - mat._11, mat._24 - mat._21, mat._34 - mat._31, mat._44 - mat._41); // Right
    m_plane[4] = Plane(mat._14 + mat._12, mat._24 + mat._22, mat._34 + mat._32, mat._44 + mat._42); // Bottom
    m_plane[5] = Plane(mat._14 - mat._12, mat._24 - mat._22, mat._34 - mat._32, mat._44 - mat._42); // Top

    for (int i = 0; i < 6; ++i)
        m_plane[i].Normalize();
}

float CMapOutdoor::GetHeight(float fx, float fy)
{
    float fTerrainHeight = GetTerrainHeight(fx, fy);

    if (IsVisiblePart(PART_TERRAIN))
    {
        CCullingManager &rkCullingMgr = CCullingManager::Instance();

        float CHECK_HEIGHT = 25000.0f;
        float fObjectHeight = -CHECK_HEIGHT;

        Vector3d aVector3d(fx, -fy, fTerrainHeight);

        FGetObjectHeight kGetObjHeight(fx, fy);

        RangeTester<FGetObjectHeight> kRangeTester_kGetObjHeight(&kGetObjHeight);
        rkCullingMgr.PointTest2d(aVector3d, &kRangeTester_kGetObjHeight);

        if (kGetObjHeight.m_bHeightFound)
            fObjectHeight = kGetObjHeight.m_fReturnHeight;

        return std::max(fObjectHeight, fTerrainHeight);
    }

    return fTerrainHeight;
}

bool CMapOutdoor::GetNormal(int ix, int iy, Vector3 *pv3Normal)
{
    if (ix <= 0)
        ix = 0;
    else if (ix >= m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE)
        ix = m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;

    if (iy <= 0)
        iy = 0;
    else if (iy >= m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
        iy = m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;

    uint16_t usCoordX, usCoordY;

    usCoordX = (uint16_t)(ix / (CTerrainImpl::TERRAIN_XSIZE));
    usCoordY = (uint16_t)(iy / (CTerrainImpl::TERRAIN_YSIZE));

    if (usCoordX >= m_sTerrainCountX - 1)
        usCoordX = m_sTerrainCountX - 1;

    if (usCoordY >= m_sTerrainCountY - 1)
        usCoordY = m_sTerrainCountY - 1;

    uint8_t byTerrainNum;
    if (!GetTerrainNumFromCoord(usCoordX, usCoordY, &byTerrainNum))
        return false;

    CTerrain *pTerrain;

    if (!GetTerrainPointer(byTerrainNum, &pTerrain))
        return false;

    while (ix >= CTerrainImpl::TERRAIN_XSIZE)
        ix -= CTerrainImpl::TERRAIN_XSIZE;

    while (iy >= CTerrainImpl::TERRAIN_YSIZE)
        iy -= CTerrainImpl::TERRAIN_YSIZE;

    return pTerrain->GetNormal(ix, iy, pv3Normal);
}

float CMapOutdoor::GetTerrainHeight(float fx, float fy)
{
    if (fy < 0)
        fy = -fy;
    long lx, ly;
    PR_FLOAT_TO_INT(fx, lx);
    PR_FLOAT_TO_INT(fy, ly);

    uint16_t usCoordX, usCoordY;

    usCoordX = (uint16_t)(lx / CTerrainImpl::TERRAIN_XSIZE);
    usCoordY = (uint16_t)(ly / CTerrainImpl::TERRAIN_YSIZE);

    uint8_t byTerrainNum;
    if (!GetTerrainNumFromCoord(usCoordX, usCoordY, &byTerrainNum))
        return 0.0f;

    CTerrain *pTerrain;

    if (!GetTerrainPointer(byTerrainNum, &pTerrain))
        return 0.0f;

    return pTerrain->GetHeight(lx, ly);
}

//////////////////////////////////////////////////////////////////////////
// For Grass
float CMapOutdoor::GetHeight(float *pPos)
{
    pPos[2] = GetHeight(pPos[0], pPos[1]);
    return pPos[2];
}

bool CMapOutdoor::GetBrushColor(float fX, float fY, float *pLowColor, float *pHighColor)
{
    bool bSuccess = false;

    //	float fU, fV;
    //
    //	GetOneToOneMappingCoordinates(fX, fY, fU, fV);
    //
    //	if (fU >= 0.0f && fU <= 1.0f && fV >= 0.0f && fV <= 1.0f)
    //	{
    //		int nImageCol = (m_cBrushMap.GetWidth() - 1) * fU;
    //		int nImageRow = (m_cBrushMap.GetHeight() - 1) * fV;
    //
    //		// low
    //		uint8_t* pPixel = m_cBrushMap.GetPixel(nImageCol, nImageRow);
    //		pLowColor[0] = (pPixel[0] / 255.0f);
    //		pLowColor[1] = (pPixel[1] / 255.0f);
    //		pLowColor[2] = (pPixel[2] / 255.0f);
    //		pLowColor[3] = (pPixel[3] / 255.0f);
    //
    //		// high
    //		pPixel = m_cBrushMap2.GetPixel(nImageCol, nImageRow);
    //		pHighColor[0] = (pPixel[0] / 255.0f);
    //		pHighColor[1] = (pPixel[1] / 255.0f);
    //		pHighColor[2] = (pPixel[2] / 255.0f);
    //		pHighColor[3] = (pPixel[3] / 255.0f);
    //
    //		bSuccess = true;
    //	}
    pLowColor[0] = (1.0f);
    pLowColor[1] = (1.0f);
    pLowColor[2] = (1.0f);
    pLowColor[3] = (1.0f);
    pHighColor[0] = (1.0f);
    pHighColor[1] = (1.0f);
    pHighColor[2] = (1.0f);
    pHighColor[3] = (1.0f);

    return bSuccess;
}

// End of for grass
//////////////////////////////////////////////////////////////////////////

std::optional<CArea *> CMapOutdoor::GetAreaPointer(const uint8_t c_byAreaNum)
{
    if (c_byAreaNum >= AROUND_AREA_NUM)
        return std::nullopt;

    if (nullptr == m_pArea[c_byAreaNum])
        return std::nullopt;

    return m_pArea[c_byAreaNum];
}

bool CMapOutdoor::GetAreaPointer(const uint8_t c_byAreaNum, CArea **ppArea)
{
    if (c_byAreaNum >= AROUND_AREA_NUM)
    {
        *ppArea = nullptr;
        return false;
    }

    if (nullptr == m_pArea[c_byAreaNum])
    {
        *ppArea = nullptr;
        return false;
    }

    *ppArea = m_pArea[c_byAreaNum];
    return true;
}

bool CMapOutdoor::GetTerrainPointer(const uint8_t c_byTerrainNum, CTerrain **ppTerrain)
{
    if (c_byTerrainNum >= AROUND_AREA_NUM)
    {
        *ppTerrain = nullptr;
        return false;
    }

    if (nullptr == m_pTerrain[c_byTerrainNum])
    {
        *ppTerrain = nullptr;
        return false;
    }

    *ppTerrain = m_pTerrain[c_byTerrainNum];
    return true;
}

void CMapOutdoor::SetDrawShadow(bool bDrawShadow)
{
    m_bDrawShadow = bDrawShadow;
}

void CMapOutdoor::SetDrawCharacterShadow(bool bDrawChrShadow)
{
    m_bDrawChrShadow = bDrawChrShadow;
}

uint32_t CMapOutdoor::GetShadowMapColor(float fx, float fy)
{
    if (fy < 0)
        fy = -fy;

    float fTerrainSize = (float)(CTerrainImpl::TERRAIN_XSIZE);
    float fXRef = fx - (float)(m_lCurCoordStartX);
    float fYRef = fy - (float)(m_lCurCoordStartY);

    CTerrain *pTerrain;

    if (fYRef < -fTerrainSize)
        return 0xFFFFFFFF;
    else if (fYRef >= -fTerrainSize && fYRef < 0.0f)
    {
        if (fXRef < -fTerrainSize)
            return 0xFFFFFFFF;
        else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
        {
            if (GetTerrainPointer(0, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef + fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= 0.0f && fXRef < fTerrainSize)
        {
            if (GetTerrainPointer(1, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef, fYRef + fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
        {
            if (GetTerrainPointer(2, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef + fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else
            return 0xFFFFFFFF;
    }
    else if (fYRef >= 0.0f && fYRef < fTerrainSize)
    {
        if (fXRef < -fTerrainSize)
            return 0xFFFFFFFF;
        else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
        {
            if (GetTerrainPointer(3, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= 0.0f && fXRef < fTerrainSize)
        {
            if (GetTerrainPointer(4, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef, fYRef);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
        {
            if (GetTerrainPointer(5, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef);
            else
                return 0xFFFFFFFF;
        }
        else
            return 0xFFFFFFFF;
    }
    else if (fYRef >= fTerrainSize && fYRef < 2.0f * fTerrainSize)
    {
        if (fXRef < -fTerrainSize)
            return 0xFFFFFFFF;
        else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
        {
            if (GetTerrainPointer(6, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef - fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= 0.0f && fXRef < fTerrainSize)
        {
            if (GetTerrainPointer(7, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef, fYRef - fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
        {
            if (GetTerrainPointer(8, &pTerrain))
                return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef - fTerrainSize);
            else
                return 0xFFFFFFFF;
        }
        else
            return 0xFFFFFFFF;
    }
    else
        return 0xFFFFFFFF;

    return 0xFFFFFFFF;
}

bool CMapOutdoor::isAttrOn(float fX, float fY, uint8_t byAttr)
{
    int iX, iY;
    PR_FLOAT_TO_INT(fX, iX);
    PR_FLOAT_TO_INT(fY, iY);

    return isAttrOn(iX, iY, byAttr);
}

bool CMapOutdoor::GetAttr(float fX, float fY, uint8_t *pbyAttr)
{
    int iX, iY;
    PR_FLOAT_TO_INT(fX, iX);
    PR_FLOAT_TO_INT(fY, iY);

    return GetAttr(iX, iY, pbyAttr);
}

bool CMapOutdoor::isAttrOn(int iX, int iY, uint8_t byAttr)
{
    if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE ||
        iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
        return false;

    uint16_t wTerrainCoordX, wTerrainCoordY;
    wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
    wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;

    uint8_t byTerrainNum;
    if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
        return false;
    CTerrain *pTerrain;
    if (!GetTerrainPointer(byTerrainNum, &pTerrain))
        return false;

    uint16_t wLocalX, wLocalY;
    wLocalX = (iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::HALF_CELLSCALE);
    wLocalY = (iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::HALF_CELLSCALE);

    return pTerrain->isAttrOn(wLocalX, wLocalY, byAttr);
}

bool CMapOutdoor::GetAttr(int iX, int iY, uint8_t *pbyAttr)
{
    if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE ||
        iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
        return false;

    uint16_t wTerrainCoordX, wTerrainCoordY;
    wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
    wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;

    uint8_t byTerrainNum;
    if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
        return false;
    CTerrain *pTerrain;
    if (!GetTerrainPointer(byTerrainNum, &pTerrain))
        return false;

    uint16_t wLocalX, wLocalY;
    wLocalX = (uint16_t)(iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::HALF_CELLSCALE);
    wLocalY = (uint16_t)(iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::HALF_CELLSCALE);

    uint8_t byAttr = pTerrain->GetAttr(wLocalX, wLocalY);

    *pbyAttr = byAttr;

    return true;
}

//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::__XMasTree_Initialize()
{
    m_kXMas.m_pkTree = nullptr;
    m_kXMas.m_iEffectID = -1;
}

void CMapOutdoor::XMasTree_Destroy()
{
    if (m_kXMas.m_pkTree)
    {
        auto &rkForest = CSpeedTreeForestDirectX9::Instance();
        m_kXMas.m_pkTree->Clear();
        rkForest.DeleteInstance(m_kXMas.m_pkTree);
        m_kXMas.m_pkTree = nullptr;
    }
    if (-1 != m_kXMas.m_iEffectID)
    {
        CEffectManager &rkEffMgr = CEffectManager::Instance();
        rkEffMgr.DestroyEffectInstance(m_kXMas.m_iEffectID);
        m_kXMas.m_iEffectID = -1;
    }
}

void CMapOutdoor::__XMasTree_Create(float x, float y, float z, const char *c_szTreeName, const char *c_szEffName)
{
    assert(NULL == m_kXMas.m_pkTree);
    assert(-1 == m_kXMas.m_iEffectID);

    CSpeedTreeForestDirectX9 &rkForest = CSpeedTreeForestDirectX9::Instance();
    uint32_t dwCRC32 = ComputeCrc32(0, c_szTreeName, strlen(c_szTreeName));
    m_kXMas.m_pkTree = rkForest.CreateInstance(x, y, z, dwCRC32, c_szTreeName);

    CEffectManager &rkEffMgr = CEffectManager::Instance();
    rkEffMgr.RegisterEffect(c_szEffName);
    m_kXMas.m_iEffectID = rkEffMgr.CreateEffect(c_szEffName);

    Matrix matWorld = Matrix::CreateTranslation(x, y, z);

    auto* effect = rkEffMgr.GetEffectInstance(m_kXMas.m_iEffectID);
    if (effect)
        effect->SetGlobalMatrix(matWorld);
}

void CMapOutdoor::XMasTree_Set(float x, float y, float z, const char *c_szTreeName, const char *c_szEffName)
{
    XMasTree_Destroy();
    __XMasTree_Create(x, y, z, c_szTreeName, c_szEffName);
}

void CMapOutdoor::SpecialEffect_Create(uint32_t dwID, float x, float y, float z, const char *c_szEffName)
{
    CEffectManager &rkEffMgr = CEffectManager::Instance();

    TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.find(dwID);
    if (m_kMap_dwID_iEffectID.end() != itor)
    {
        auto* effect = rkEffMgr.GetEffectInstance(itor->second);
        if (effect)
        {
            Matrix mat = Matrix::CreateTranslation(x, y, z);
            effect->SetGlobalMatrix(mat);
            return;
        }
    }

    rkEffMgr.RegisterEffect(c_szEffName);
    uint32_t dwEffectID = rkEffMgr.CreateEffect(c_szEffName);

    Matrix matWorld = Matrix::CreateTranslation(x, y, z);

    auto* effect = rkEffMgr.GetEffectInstance(dwEffectID);
    if (effect)
        effect->SetGlobalMatrix(matWorld);

    m_kMap_dwID_iEffectID.insert(std::make_pair(dwID, dwEffectID));
}

void CMapOutdoor::SpecialEffect_Delete(uint32_t dwID)
{
    TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.find(dwID);

    if (m_kMap_dwID_iEffectID.end() == itor)
        return;

    CEffectManager &rkEffMgr = CEffectManager::Instance();
    int iEffectID = itor->second;
    rkEffMgr.DestroyEffectInstance(iEffectID);
}

void CMapOutdoor::SpecialEffect_Destroy()
{
    CEffectManager &rkEffMgr = CEffectManager::Instance();

    TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.begin();
    for (; itor != m_kMap_dwID_iEffectID.end(); ++itor)
    {
        int iEffectID = itor->second;
        rkEffMgr.DestroyEffectInstance(iEffectID);
    }
}

void CMapOutdoor::SpecialEffectShopPos_Create(uint32_t dwID, float x, float y, float z, const char *c_szEffName)
{
    CEffectManager &rkEffMgr = CEffectManager::Instance();

    TSpecialEffectMap::iterator itor = m_kMapShop_dwID_iEffectID.find(dwID);
    if (m_kMapShop_dwID_iEffectID.end() != itor)
    {
        auto* effect = rkEffMgr.GetEffectInstance(itor->second);
        if (effect)
        {
            Matrix mat = Matrix::CreateTranslation(x, y, z);
            effect->SetGlobalMatrix(mat);
            return;
        }
    }

    rkEffMgr.RegisterEffect(c_szEffName);
    uint32_t dwEffectID = rkEffMgr.CreateEffect(c_szEffName);

    const Matrix matWorld = Matrix::CreateTranslation(x, y, z);

    auto* effect = rkEffMgr.GetEffectInstance(dwEffectID);
    if (effect)
        effect->SetGlobalMatrix(matWorld);

    m_kMapShop_dwID_iEffectID.emplace(dwID, dwEffectID);
}

void CMapOutdoor::SpecialEffectShopPos_Delete(uint32_t dwID)
{
    TSpecialEffectMap::iterator itor = m_kMapShop_dwID_iEffectID.find(dwID);

    if (m_kMapShop_dwID_iEffectID.end() == itor)
    {
        return;
    }

    CEffectManager &rkEffMgr = CEffectManager::Instance();
    int iEffectID = itor->second;
    SPDLOG_ERROR("SpecialEffectShopPos_Delete {0} {1}", dwID, iEffectID);
    rkEffMgr.DestroyEffectInstance(iEffectID);
    m_kMapShop_dwID_iEffectID.erase(itor);
}

void CMapOutdoor::SpecialEffectShopPos_Destroy()
{
    CEffectManager &rkEffMgr = CEffectManager::Instance();

    TSpecialEffectMap::iterator itor = m_kMapShop_dwID_iEffectID.begin();
    for (; itor != m_kMapShop_dwID_iEffectID.end(); ++itor)
    {
        int iEffectID = itor->second;
        rkEffMgr.DestroyEffectInstance(iEffectID);
    }
}

void CMapOutdoor::ClearGuildArea()
{
    m_rkList_kGuildArea.clear();
}

void CMapOutdoor::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
    RECT rect;
    rect.left = isx;
    rect.top = isy;
    rect.right = iex;
    rect.bottom = iey;
    m_rkList_kGuildArea.push_back(rect);
}

void CMapOutdoor::VisibleMarkedArea()
{
    static const std::size_t kBufferSize = CTerrain::ATTRMAP_XSIZE * CTerrain::ATTRMAP_YSIZE;

    std::unordered_map<int, std::unique_ptr<uint8_t[]>> attrMaps;

    for (const RECT &rkRect : m_rkList_kGuildArea)
    {
        int ix1Cell;
        int iy1Cell;
        BYTE byx1SubCell;
        BYTE byy1SubCell;
        WORD wx1TerrainNum;
        WORD wy1TerrainNum;

        int ix2Cell;
        int iy2Cell;
        BYTE byx2SubCell;
        BYTE byy2SubCell;
        WORD wx2TerrainNum;
        WORD wy2TerrainNum;

        ConvertToMapCoords(float(rkRect.left), float(rkRect.top), &ix1Cell, &iy1Cell, &byx1SubCell, &byy1SubCell,
                           &wx1TerrainNum, &wy1TerrainNum);
        ConvertToMapCoords(float(rkRect.right), float(rkRect.bottom), &ix2Cell, &iy2Cell, &byx2SubCell, &byy2SubCell,
                           &wx2TerrainNum, &wy2TerrainNum);

        ix1Cell = ix1Cell + wx1TerrainNum * CTerrain::ATTRMAP_XSIZE;
        iy1Cell = iy1Cell + wy1TerrainNum * CTerrain::ATTRMAP_YSIZE;
        ix2Cell = ix2Cell + wx2TerrainNum * CTerrain::ATTRMAP_XSIZE;
        iy2Cell = iy2Cell + wy2TerrainNum * CTerrain::ATTRMAP_YSIZE;

        for (int ixCell = ix1Cell; ixCell <= ix2Cell; ++ixCell)
        {
            for (int iyCell = iy1Cell; iyCell <= iy2Cell; ++iyCell)
            {
                int ixLocalCell = ixCell % CTerrain::ATTRMAP_XSIZE;
                int iyLocalCell = iyCell % CTerrain::ATTRMAP_YSIZE;
                int ixTerrain = ixCell / CTerrain::ATTRMAP_XSIZE;
                int iyTerrain = iyCell / CTerrain::ATTRMAP_YSIZE;
                int iTerrainNum = ixTerrain + iyTerrain * 100;

                BYTE byTerrainNum;
                if (!GetTerrainNumFromCoord(ixTerrain, iyTerrain, &byTerrainNum))
                    continue;

                CTerrain *pTerrain;
                if (!GetTerrainPointer(byTerrainNum, &pTerrain))
                    continue;

                if (attrMaps.end() == attrMaps.find(iTerrainNum))
                {
                    std::unique_ptr<uint8_t[]> buf(new uint8_t[kBufferSize]);
                    std::memset(buf.get(), 0, kBufferSize);
                    attrMaps[iTerrainNum] = std::move(buf);
                }

                auto buf = attrMaps[iTerrainNum].get();
                buf[ixLocalCell + iyLocalCell * CTerrain::ATTRMAP_XSIZE] = 0xff;
            }
        }
    }

    for (const auto &p : attrMaps)
    {
        int ixTerrain = p.first % 100;
        int iyTerrain = p.first / 100;

        BYTE byTerrainNum;
        if (!GetTerrainNumFromCoord(ixTerrain, iyTerrain, &byTerrainNum))
            continue;

        CTerrain *pTerrain;
        if (!GetTerrainPointer(byTerrainNum, &pTerrain))
            continue;

        pTerrain->AllocateMarkedSplats(p.second.get());
    }
}

void CMapOutdoor::DisableMarkedArea()
{
    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        if (!m_pTerrain[i])
            continue;

        m_pTerrain[i]->DeallocateMarkedSplats();
    }
}

void CMapOutdoor::ConvertToMapCoords(float fx, float fy, int *iCellX, int *iCellY, uint8_t *pucSubCellX,
                                     uint8_t *pucSubCellY, uint16_t *pwTerrainNumX, uint16_t *pwTerrainNumY)
{
    if (fy < 0)
        fy = -fy;

    int ix, iy;
    PR_FLOAT_TO_INT(fx, ix);
    PR_FLOAT_TO_INT(fy, iy);

    *pwTerrainNumX = ix / (CTerrainImpl::TERRAIN_XSIZE);
    *pwTerrainNumY = iy / (CTerrainImpl::TERRAIN_YSIZE);

    float maxx = (float)CTerrainImpl::TERRAIN_XSIZE;
    float maxy = (float)CTerrainImpl::TERRAIN_YSIZE;

    while (fx < 0)
        fx += maxx;

    while (fy < 0)
        fy += maxy;

    while (fx >= maxx)
        fx -= maxx;

    while (fy >= maxy)
        fy -= maxy;

    float fooscale = 1.0f / (float)(CTerrainImpl::HALF_CELLSCALE);

    float fCellX, fCellY;

    fCellX = fx * fooscale;
    fCellY = fy * fooscale;

    PR_FLOAT_TO_INT(fCellX, *iCellX);
    PR_FLOAT_TO_INT(fCellY, *iCellY);

    float fRatioooscale = ((float)CTerrainImpl::HEIGHT_TILE_XRATIO) * fooscale;

    float fSubcellX, fSubcellY;
    fSubcellX = fx * fRatioooscale;
    fSubcellY = fy * fRatioooscale;

    PR_FLOAT_TO_INT(fSubcellX, *pucSubCellX);
    PR_FLOAT_TO_INT(fSubcellY, *pucSubCellY);
    *pucSubCellX = (*pucSubCellX) % CTerrainImpl::HEIGHT_TILE_XRATIO;
    *pucSubCellY = (*pucSubCellY) % CTerrainImpl::HEIGHT_TILE_YRATIO;
}
