// PythonBackground.cpp: implementation of the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#include "../eterlib/Camera.h"
#include "../eterlib/CullingManager.h"
#include "../gamelib/MapOutDoor.h"
#include "../gamelib/PropertyLoader.h"
#include "stdafx.h"
#include <pak/Vfs.hpp>

#include "../EterGrnLib/GrannyState.hpp"
#include "../EterLib/Engine.hpp"
#include "../GameLib/EnvironmentData.hpp"
#include "../eterBase/Timer.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonMiniMap.h"
#include "PythonNetworkStream.h"
#include "../EterGrnLib/ModelShader.h"
#include "base/Remotery.h"

#include <base/GroupTextTree.hpp>
#include <optional>

#include "pak/Util.hpp"

std::string g_strEffectName = "d:/ymir work/effect/etc/direction/direction_land.mse";
std::string g_strEffectShopPos = "d:/ymir work/effect/etc/direction/direction_land2.mse";

uint32_t CPythonBackground::GetRenderShadowTime()
{
    return m_dwRenderShadowTime;
}

bool CPythonBackground::SetVisiblePart(int eMapOutDoorPart, bool isVisible)
{
    if (!m_pkMap)
        return false;

    m_pkMap->SetVisiblePart(eMapOutDoorPart, isVisible);
    return true;
}

bool CPythonBackground::SetSplatLimit(int iSplatNum)
{
    if (!m_pkMap)
        return false;

    m_pkMap->SetSplatLimit(iSplatNum);
    return true;
}

void CPythonBackground::CreateCharacterShadowTexture()
{
    if (!m_pkMap)
        return;

    m_pkMap->CreateCharacterShadowTexture();
}

void CPythonBackground::ReleaseCharacterShadowTexture()
{
    if (!m_pkMap)
        return;

    m_pkMap->ReleaseCharacterShadowTexture();
}

void CPythonBackground::RefreshShadowLevel()
{
    SetShadowLevel(Engine::GetSettings().GetShadowLevel());
}

bool CPythonBackground::SetShadowLevel(int eLevel)
{
    if (!m_pkMap)
        return false;

    // Always refresh - we unfortunately have no way of knowing
    // whether our previous changes to m_pkMap are still in effect.
    m_eShadowLevel = eLevel;

    switch (m_eShadowLevel)
    {
    case SHADOW_NONE:
        m_pkMap->SetDrawShadow(false);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(256);
        break;

    case SHADOW_GROUND:
        m_pkMap->SetDrawShadow(true);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(256);
        break;

    case SHADOW_GROUND_AND_SOLO:
        m_pkMap->SetDrawShadow(true);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(512);
        break;

    case SHADOW_ALL:
        m_pkMap->SetDrawShadow(true);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(2048);
        break;

    case SHADOW_ALL_HIGH:
        m_pkMap->SetDrawShadow(true);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(4096);
        break;

    case SHADOW_ALL_MAX:
        m_pkMap->SetDrawShadow(true);
        m_pkMap->SetDrawCharacterShadow(true);
        m_pkMap->SetShadowTextureSize(8192);
        break;
    }

    return true;
}

void CPythonBackground::SelectViewDistanceNum(int eNum)
{
    if (!m_pkMap)
        return;

    const auto &envInstance = m_pkMap->GetActiveEnvironment();
    if (!envInstance)
    {
        SPDLOG_ERROR("CPythonBackground::SelectViewDistanceNum(int eNum={0}) mc_pcurEnvironmentData is NULL", eNum);
        return;
    }

    m_eViewDistanceNum = eNum;

    if (envInstance.GetData()->bReserve)
    {
        m_pkMap->GetSkyBox().SetSkyBoxScale(m_ViewDistanceSet[m_eViewDistanceNum].m_v3SkyBoxScale);

        envInstance.SetFogDistance(m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart,
                                   m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd);

        CSpeedTreeForestDirectX9::instance().SetFog(m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart,
                                          m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd);
    }
}

void CPythonBackground::SetViewDistanceSet(int eNum, float fFarClip)
{
    if (!m_pkMap)
        return;

    m_ViewDistanceSet[eNum].m_fFogStart = fFarClip * 0.5f; // 0.3333333f;
    m_ViewDistanceSet[eNum].m_fFogEnd = fFarClip * 0.7f;   // 0.6666667f;

    float fSkyBoxScale = fFarClip * 0.6f; // 0.5773502f;
    m_ViewDistanceSet[eNum].m_v3SkyBoxScale = Vector3(fSkyBoxScale, fSkyBoxScale, fSkyBoxScale);
    m_ViewDistanceSet[eNum].m_fFarClip = fFarClip;

    if (eNum == m_eViewDistanceNum)
        SelectViewDistanceNum(eNum);
}

float CPythonBackground::GetFarClip()
{
    if (!m_pkMap)
        return 50000.0f;

    if (m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip <= 0.0f)
    {
 //       SPDLOG_ERROR("CPythonBackground::GetFarClip m_eViewDistanceNum={0}", m_eViewDistanceNum);
        m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip = 25600.0f;
    }

    return m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

void CPythonBackground::GetDistanceSetInfo(int *peNum, float *pfStart, float *pfEnd, float *pfFarClip)
{
    if (!m_pkMap)
    {
        *peNum = 4;
        *pfStart = 10000.0f;
        *pfEnd = 15000.0f;
        *pfFarClip = 50000.0f;
        return;
    }
    *peNum = m_eViewDistanceNum;
    *pfStart = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
    *pfEnd = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
    *pfFarClip = m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPythonBackground::CPythonBackground()
{
    m_dwRenderShadowTime = 0;
    m_eViewDistanceNum = 0;
    m_eViewDistanceNum = 0;
    m_eViewDistanceNum = 0;
    m_eShadowLevel = SHADOW_NONE;

    m_strMapName = "";
    m_iDayMode = DAY_MODE_LIGHT;
    m_iXMasTreeGrade = 0;
    m_bVisibleGuildArea = FALSE;

    static int s_isCreateProperty = false;

    if (!s_isCreateProperty)
    {
        s_isCreateProperty = true;
        __CreateProperty();
    }

    SetViewDistanceSet(4, 25600.0f);
    SetViewDistanceSet(3, 25600.0f);
    SetViewDistanceSet(2, 25600.0f);
    SetViewDistanceSet(1, 25600.0f);
    SetViewDistanceSet(0, 25600.0f);
}

CPythonBackground::~CPythonBackground()
{
}

uint32_t CPythonBackground::GetMapIndex()
{
    if (!IsMapReady())
        return 0;

    return m_pkMap->GetMapIndex();
}

void CPythonBackground::__CreateProperty()
{

    m_PropertyManager.Initialize("pack/property");
    
}

//////////////////////////////////////////////////////////////////////
// Normal Functions
//////////////////////////////////////////////////////////////////////

bool CPythonBackground::GetPickingPoint(Vector3 *v3IntersectPt)
{
    return m_pkMap->GetPickingPoint(v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRay(const CRay &rRay, Vector3 *v3IntersectPt)
{
    return m_pkMap->GetPickingPointWithRay(rRay, v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRayOnlyTerrain(const CRay &rRay, Vector3 *v3IntersectPt)
{
    return m_pkMap->GetPickingPointWithRayOnlyTerrain(rRay, v3IntersectPt);
}

bool CPythonBackground::GetLightDirection(Vector3 &rv3LightDirection)
{
    if (!m_pkMap)
        return false;

    const auto &envInstance = m_pkMap->GetActiveEnvironment();
    if (!envInstance)
        return false;

    const auto data = envInstance.GetData();
    rv3LightDirection.x = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
    rv3LightDirection.y = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
    rv3LightDirection.z = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CPythonBackground::Destroy()
{
    CMapManager::Destroy();
    m_SnowEnvironment.Destroy();
    m_bVisibleGuildArea = FALSE;
}

void CPythonBackground::Create()
{
    CMapManager::Create();

    m_SnowEnvironment.Create();
}

struct FGetPortalID
{
    float m_fRequestX, m_fRequestY;
    std::unordered_set<int> m_kSet_iPortalID;

    FGetPortalID(float fRequestX, float fRequestY)
    {
        m_fRequestX = fRequestX;
        m_fRequestY = fRequestY;
    }

    void operator()(CGraphicObjectInstance *pObject)
    {
        for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
        {
            int iID = pObject->GetPortal(i);
            if (0 == iID)
                break;

            m_kSet_iPortalID.insert(iID);
        }
    }
};

void CPythonBackground::Update(float fCenterX, float fCenterY, float fCenterZ)
{
    if (!IsMapReady())
        return;

    UpdateMap(fCenterX, fCenterY, fCenterZ);
    UpdateAroundAmbience(fCenterX, fCenterY, fCenterZ);
    m_SnowEnvironment.Update(Vector3(fCenterX, -fCenterY, fCenterZ));

    // Portal Process
    if (m_pkMap->IsEnablePortal())
    {
        CCullingManager &rkCullingMgr = CCullingManager::Instance();
        FGetPortalID kGetPortalID(fCenterX, -fCenterY);

        Vector3d aVector3d;
        aVector3d = Vector3(fCenterX, -fCenterY, fCenterZ);

        Vector3d toTop;
        toTop = Vector3(0, 0, 25000.0f);

        rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

        auto itor = kGetPortalID.m_kSet_iPortalID.begin();
        if (!__IsSame(kGetPortalID.m_kSet_iPortalID, m_kSet_iShowingPortalID))
        {
            ClearPortal();
            auto itor = kGetPortalID.m_kSet_iPortalID.begin();
            for (; itor != kGetPortalID.m_kSet_iPortalID.end(); ++itor)
            {
                AddShowingPortalID(*itor);
            }
            RefreshPortal();

            m_kSet_iShowingPortalID = kGetPortalID.m_kSet_iPortalID;
        }
    }

    // Target Effect Process
    {
        auto itor = m_kMap_dwTargetID_dwChrID.begin();
        for (; itor != m_kMap_dwTargetID_dwChrID.end(); ++itor)
        {
            uint32_t dwTargetID = itor->first;
            uint32_t dwChrID = itor->second;

            CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

            if (!pInstance)
                continue;

            TPixelPosition kPixelPosition;
            pInstance->NEW_GetPixelPosition(&kPixelPosition);

            CreateSpecialEffect(dwTargetID, +kPixelPosition.x, -kPixelPosition.y, +kPixelPosition.z,
                                g_strEffectName.c_str());
        }
    }

    // Shop Pos Effect Process
    for (auto &id_dw_chr_id : m_kMapShop_dwTargetID_dwChrID)
    {
        uint32_t dwTargetID = id_dw_chr_id.first;
        uint32_t dwChrID = id_dw_chr_id.second;

        CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

        if (!pInstance)
        {
            continue;
        }
        TPixelPosition kPixelPosition;
        pInstance->NEW_GetPixelPosition(&kPixelPosition);

        CreateSpecialShopPos(dwTargetID, +kPixelPosition.x, -kPixelPosition.y, +kPixelPosition.z,
                             g_strEffectShopPos.c_str());
    }

    // Reserve Target Effect
    {
        auto itor = m_kMap_dwID_kReserveTargetEffect.begin();
        for (; itor != m_kMap_dwID_kReserveTargetEffect.end();)
        {
            uint32_t dwID = itor->first;
            SReserveTargetEffect &rReserveTargetEffect = itor->second;

            float ilx = float(rReserveTargetEffect.ilx);
            float ily = float(rReserveTargetEffect.ily);

            float fHeight = m_pkMap->GetHeight(ilx, ily);
            if (0.0f == fHeight)
            {
                ++itor;
                continue;
            }

            CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());

            itor = m_kMap_dwID_kReserveTargetEffect.erase(itor);
        }
    }
}

bool CPythonBackground::__IsSame(std::unordered_set<int> &rleft, std::unordered_set<int> &rright)
{
    std::unordered_set<int>::iterator itor_l;
    std::unordered_set<int>::iterator itor_r;

    for (itor_l = rleft.begin(); itor_l != rleft.end(); ++itor_l)
    {
        if (rright.end() == rright.find(*itor_l))
            return false;
    }

    for (itor_r = rright.begin(); itor_r != rright.end(); ++itor_r)
    {
        if (rleft.end() == rleft.find(*itor_r))
            return false;
    }

    return true;
}

void CPythonBackground::Render()
{
    if (!IsMapReady())
        return;

    rmt_ScopedCPUSample(RenderMap, 0);

    m_SnowEnvironment.Deform();

    m_pkMap->Render();

    if (m_bVisibleGuildArea)
        m_pkMap->RenderMarkedArea();
}

void CPythonBackground::RenderSnow()
{
  rmt_ScopedCPUSample(RenderSnow, 0);

    m_SnowEnvironment.Render();
}

void CPythonBackground::RenderCollision()
{
    if (!IsMapReady())
        return;

    m_pkMap->RenderCollision();
}

void CPythonBackground::RenderCharacterShadowToTexture()
{
    rmt_ScopedCPUSample(RenderCharacterShadowToTexture, 0);

    if (!IsMapReady())
        return;

    auto t1 = ELTimer_GetMSec();

    Matrix matWorld;
    STATEMANAGER.GetTransform(D3DTS_WORLD, &matWorld);

    bool canRender = m_pkMap->BeginRenderCharacterShadowToTexture();
    if (canRender)
    {
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        if (m_eShadowLevel != SHADOW_NONE && m_eShadowLevel != SHADOW_GROUND)
        {

            if (m_eShadowLevel == SHADOW_GROUND_AND_SOLO)
                rkChrMgr.RenderShadowMainInstance();
            else
                rkChrMgr.RenderShadowAllInstances();
        }
    }
    m_pkMap->EndRenderCharacterShadowToTexture();

    STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorld);

    auto t2 = ELTimer_GetMSec();

    m_dwRenderShadowTime = t2 - t1;
}

struct CollisionChecker
{
    bool isBlocked;
    CInstanceBase *pInstance;

    CollisionChecker(CInstanceBase *pInstance) : pInstance(pInstance), isBlocked(false)
    {
    }

    void operator()(CGraphicObjectInstance *pOpponent)
    {
        if (isBlocked)
            return;

        if (!pOpponent)
            return;

        if (pInstance->IsBlockObject(*pOpponent))
            isBlocked = true;
    }
};

struct CollisionAdjustChecker
{
    bool isBlocked;
    CInstanceBase *pInstance;

    CollisionAdjustChecker(CInstanceBase *pInstance) : pInstance(pInstance), isBlocked(false)
    {
    }

    void operator()(CGraphicObjectInstance *pOpponent)
    {
        if (!pOpponent)
            return;

        if (pInstance->AvoidObject(*pOpponent))
            isBlocked = true;
    }
};

bool CPythonBackground::CheckAdvancing(CInstanceBase *pInstance)
{
    if (!IsMapReady())
        return true;

    if (!pInstance)
        return true;

    Vector3d center;
    float radius;
    pInstance->GetGraphicThingInstanceRef().GetBoundingSphere(center, radius);

    CCullingManager &rkCullingMgr = CCullingManager::Instance();

    CollisionAdjustChecker kCollisionAdjustChecker(pInstance);
    rkCullingMgr.ForInRange(center, radius, &kCollisionAdjustChecker);
    if (kCollisionAdjustChecker.isBlocked)
    {
        CollisionChecker kCollisionChecker(pInstance);
        rkCullingMgr.ForInRange(center, radius, &kCollisionChecker);
        if (kCollisionChecker.isBlocked)
        {
            pInstance->BlockMovement();
            return true;
        }
        else
        {
            pInstance->NEW_MoveToDestPixelPositionDirection(pInstance->NEW_GetDstPixelPositionRef());
        }
        return false;
    }
    return false;
}

void CPythonBackground::ClearGuildArea()
{
    if (!IsMapReady())
        return;

    m_pkMap->ClearGuildArea();
}

void CPythonBackground::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
    if (!IsMapReady())
        return;

    m_pkMap->RegisterGuildArea(isx, isy, iex, iey);
}

void CPythonBackground::SetCharacterDirLight()
{
    if (!IsMapReady())
        return;

    const auto &envInstance = m_pkMap->GetActiveEnvironment();
    if (!envInstance)
        return;

    const auto data = envInstance.GetData();
    STATEMANAGER.SetLight(0, &data->DirLights[ENV_DIRLIGHT_CHARACTER]);
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        auto &light = data->DirLights[ENV_DIRLIGHT_CHARACTER];
        ms->SetLightDirection(Vector3((const float*)&light.Direction ));
        ms->SetLightColor(Vector4(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b, light.Diffuse.a));
        ms->SetFogColor(envInstance.GetData()->FogColor);
        ms->SetFogParams(envInstance.GetFogNearDistance(), envInstance.GetFogFarDistance());
        ms->SetMaterial(data->Material);
        STATEMANAGER.SetMaterial(&data->Material);

    }
}

void CPythonBackground::SetBackgroundDirLight()
{
    if (!IsMapReady())
        return;

    const auto &envInstance = m_pkMap->GetActiveEnvironment();
    if (!envInstance)
        return;

    const auto data = envInstance.GetData();
    STATEMANAGER.SetLight(0, &data->DirLights[ENV_DIRLIGHT_BACKGROUND]);
}

void CPythonBackground::RegisterDungeonMapName(const char *c_szMapName)
{
    m_kSet_strDungeonMapName.insert(c_szMapName);
}

std::optional<CPythonBackground::TMapInfo> CPythonBackground::GetMapInfo(uint32_t mapIndex)
{
    auto it = m_mapInfo.find(mapIndex);
    if (it == m_mapInfo.end())
        return std::nullopt;

    return it->second;
}

std::optional<CPythonBackground::TMapInfo> CPythonBackground::GetMapInfoByName(const std::string& name)
{
    auto it = m_mapInfoByName.find(name);
    if (it == m_mapInfoByName.end())
        return std::nullopt;

    if(!it->second)
        return std::nullopt;

    return *it->second;
}

std::optional<CPythonBackground::TMapInfo> CPythonBackground::GetCurrentMapInfo()
{
    auto it = m_mapInfo.find(GetMapIndex());
    if (it == m_mapInfo.end())
        return std::nullopt;

    return it->second;
}

uint32_t CPythonBackground::GetMapInfoCount()
{
    return m_mapInfo.size();
}

bool CPythonBackground::IsMapInfoByName(const char *mapName)
{
    if (std::find(m_mapNames.begin(), m_mapNames.end(), mapName) != m_mapNames.end())
    {
        return true;
    }
    return false;
}

bool CPythonBackground::Warp(uint32_t mapIndex, uint32_t x, uint32_t y)
{
    auto info = GetMapInfo(mapIndex);
    if (!info)
    {
        SPDLOG_ERROR("Could not find map with index {0}", mapIndex);
        return false;
    }

    if (!LoadMap(mapIndex, info->mapName, float(x), float(y), 0))
        return false;

    RefreshShadowLevel();

    CPythonMiniMap::Instance().LoadAtlas();
    m_strMapName = info->mapName;
    SetXMaxTree(m_iXMasTreeGrade);

    m_kSet_iShowingPortalID.clear();
    m_kMap_dwTargetID_dwChrID.clear();
    m_kMap_dwID_kReserveTargetEffect.clear();

    if (!IsNoSnowMap(m_strMapName.c_str()))
    {
        if (IsXmasShow() && IsSnowModeOption())
            EnableSnowEnvironment();

        if (IsXmasShow())
            RefreshTexture(IsSnowTextureModeOption());
    }
    else
    {
        DisableSnowEnvironment();
    }

    auto &map = GetMapOutdoorRef();

    if (GetNightOption() && !IsNoBoomMap(m_strMapName.c_str()))
    {
        if (const auto *environment = map.GetEnvironment(DAY_MODE_DARK); environment)
        {
            map.SetActiveEnvironment(environment);
        }
    }
    else if (const auto *environment = map.GetEnvironment(DAY_MODE_LIGHT); environment)
    {
        map.SetActiveEnvironment(environment);
    }

    return true;
}

void CPythonBackground::VisibleGuildArea()
{
    if (!IsMapReady())
        return;

    m_pkMap->VisibleMarkedArea();

    m_bVisibleGuildArea = TRUE;
}

void CPythonBackground::DisableGuildArea()
{
    if (!IsMapReady())
        return;

    m_pkMap->DisableMarkedArea();

    m_bVisibleGuildArea = FALSE;
}

const char *CPythonBackground::GetWarpMapName() const
{
    return m_strMapName.c_str();
}

void CPythonBackground::ChangeToDay()
{
    m_iDayMode = DAY_MODE_LIGHT;
}

void CPythonBackground::ChangeToNight()
{
    m_iDayMode = DAY_MODE_DARK;
}

void CPythonBackground::EnableSnowEnvironment()
{
    m_SnowEnvironment.Enable();
}

void CPythonBackground::DisableSnowEnvironment()
{
    m_SnowEnvironment.Disable();
}

const Vector3 c_v3TreePos = Vector3(76500.0f, -60900.0f, 20215.0f);

void CPythonBackground::SetXMaxTree(int iGrade)
{
    if (!m_pkMap)
        return;

    assert(iGrade >= 0 && iGrade <= 3);
    m_iXMasTreeGrade = iGrade;

    if ("map_n_snowm_01" != m_strMapName)
    {
        m_pkMap->XMasTree_Destroy();
        return;
    }

    if (0 == iGrade)
    {
        m_pkMap->XMasTree_Destroy();
        return;
    }

    //////////////////////////////////////////////////////////////////////

    iGrade -= 1;
    iGrade = std::max(iGrade, 0);
    iGrade = std::min(iGrade, 2);

    static std::string s_strTreeName[3] = {"d:/ymir work/tree/christmastree1.spt",
                                           "d:/ymir work/tree/christmastree2.spt",
                                           "d:/ymir work/tree/christmastree3.spt"};
    static std::string s_strEffectName[3] = {
        "d:/ymir work/effect/etc/christmas_tree/tree_1s.mse",
        "d:/ymir work/effect/etc/christmas_tree/tree_2s.mse",
        "d:/ymir work/effect/etc/christmas_tree/tree_3s.mse",
    };
    m_pkMap->XMasTree_Set(c_v3TreePos.x, c_v3TreePos.y, c_v3TreePos.z, s_strTreeName[iGrade].c_str(),
                          s_strEffectName[iGrade].c_str());
}

void CPythonBackground::CreatePrivateShopPos(uint32_t dwID, uint32_t dwChrVID)
{
    m_kMapShop_dwTargetID_dwChrID.insert(std::make_pair(dwID, dwChrVID));
}

void CPythonBackground::CreateTargetEffect(uint32_t dwID, uint32_t dwChrVID)
{
    m_kMap_dwTargetID_dwChrID.insert(std::make_pair(dwID, dwChrVID));
}

void CPythonBackground::CreateTargetEffect(uint32_t dwID, long lx, long ly)
{
    if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
        return;

    float fHeight = m_pkMap->GetHeight(float(lx), float(ly));

    if (0.0f == fHeight)
    {
        SReserveTargetEffect ReserveTargetEffect;
        ReserveTargetEffect.ilx = lx;
        ReserveTargetEffect.ily = ly;
        m_kMap_dwID_kReserveTargetEffect.insert(std::make_pair(dwID, ReserveTargetEffect));
        return;
    }

    CreateSpecialEffect(dwID, lx, ly, fHeight, g_strEffectName.c_str());
}

void CPythonBackground::DeleteTargetEffect(uint32_t dwID)
{
    if (m_kMap_dwID_kReserveTargetEffect.end() != m_kMap_dwID_kReserveTargetEffect.find(dwID))
    {
        m_kMap_dwID_kReserveTargetEffect.erase(dwID);
    }
    if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
    {
        m_kMap_dwTargetID_dwChrID.erase(dwID);
    }

    DeleteSpecialEffect(dwID);
}

void CPythonBackground::DeletePrivateShopPos(uint32_t dwID)
{
    if (m_kMapShop_dwTargetID_dwChrID.end() != m_kMapShop_dwTargetID_dwChrID.find(dwID))
    {
        m_kMapShop_dwTargetID_dwChrID.erase(dwID);
    }

    DeleteSpecialShopPos(dwID);
}

void CPythonBackground::CreateSpecialShopPos(uint32_t dwID, float fx, float fy, float fz, const char *c_szFileName)
{
    if (m_pkMap)
        m_pkMap->SpecialEffectShopPos_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialShopPos(uint32_t dwID)
{
    if (m_pkMap)
        m_pkMap->SpecialEffectShopPos_Delete(dwID);
}

void CPythonBackground::CreateSpecialEffect(uint32_t dwID, float fx, float fy, float fz, const char *c_szFileName)
{
    m_pkMap->SpecialEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialEffect(uint32_t dwID)
{
    m_pkMap->SpecialEffect_Delete(dwID);
}

bool CPythonBackground::LoadMapConfg(const std::string &filename)
{
    auto data = LoadFileToString(GetVfs(), filename.c_str());
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        return false;
    }

    const auto noSnowMaps = reader.GetList("NoSnowMaps");
    if (noSnowMaps != nullptr)
    {
        for (uint32_t i = 0; i < noSnowMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noSnowMaps->GetLine(i));
            boost::trim(line);
            m_noSnowMap.emplace_back(line);
        }
    }

    const auto noBoomMaps = reader.GetList("NoBoomMaps");
    if (noBoomMaps != nullptr)
    {
        for (uint32_t i = 0; i < noBoomMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noBoomMaps->GetLine(i));
            boost::trim(line);
            m_noBoomMap.push_back(line);
        }
    }

    const auto noHorseSkillMaps = reader.GetList("NoHorseSkillMaps");
    if (noHorseSkillMaps != nullptr)
    {
        for (uint32_t i = 0; i < noHorseSkillMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noHorseSkillMaps->GetLine(i));
            boost::trim(line);
            m_noHorseSkillMap.push_back(line);
        }
    }

    const auto duelMaps = reader.GetList("DuelMaps");
    if (duelMaps != nullptr)
    {
        for (uint32_t i = 0; i < duelMaps->GetLineCount(); ++i)
        {
            auto line = std::string(duelMaps->GetLine(i));
            boost::trim(line);
            uint32_t index;
            storm::ParseNumber(line, index);
            m_duelMapIndices.push_back(index);
        }
    }

    return true;
}

bool CPythonBackground::IsNoSnowMap(const char *mapName)
{
    return m_noSnowMap.end() != std::find(m_noSnowMap.begin(), m_noSnowMap.end(), mapName);
}

bool CPythonBackground::IsNoBoomMap(const char *mapName)
{
    return m_noBoomMap.end() != std::find(m_noBoomMap.begin(), m_noBoomMap.end(), mapName);
}

bool CPythonBackground::IsDuelMap(uint32_t index)
{
    return m_duelMapIndices.end() != std::find(m_duelMapIndices.begin(), m_duelMapIndices.end(), index);
}


bool CPythonBackground::IsOnBlockedHorseSkillMap() const
{
    return m_noHorseSkillMap.end() != std::find(m_noHorseSkillMap.begin(), m_noHorseSkillMap.end(), GetWarpMapName());
}

void CPythonBackground::RefreshTexture(bool isSnowTextureModeOption)
{
    if (m_pkMap)
    {
        m_pkMap->ReloadSetting(isSnowTextureModeOption);
    }
}

bool CPythonBackground::IsBlock(int x, int y)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
}
