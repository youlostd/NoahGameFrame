#include "ActorInstance.h"
#include "AreaTerrain.h"
#include "MapOutdoor.h"
#include "StdAfx.h"
#include "TerrainPatch.h"
#include "TerrainQuadtree.h"

#include "../SpeedTreeLib/SpeedTreeForest.h"

#include "../EterLib/Camera.h"
#include "../EterLib/CullingManager.h"

#include "../EterBase/Timer.h"
#include "../EterBase/Utils.h"
#include "EnvironmentData.hpp"
#include <execution>

#include "../EterBase/StepTimer.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"

bool CMapOutdoor::Update(float fX, float fY, float fZ)
{
    Vector3 v3Player(fX, fY, fZ);

    m_v3Player = v3Player;

    uint32_t t1 = ELTimer_GetMSec();

    int ix = fX;
    if (fY < 0)
        fY = -fY;
    int iy = fY;

    short sCoordX = std::clamp<short>(ix / CTerrainImpl::TERRAIN_XSIZE, 0, m_sTerrainCountX - 1);
    short sCoordY = std::clamp<short>(iy / CTerrainImpl::TERRAIN_YSIZE, 0, m_sTerrainCountY - 1);

    bool bNeedInit = (m_PrevCoordinate.m_sTerrainCoordX == -1 || m_PrevCoordinate.m_sTerrainCoordY == -1);

    if (bNeedInit || (m_CurCoordinate.m_sTerrainCoordX / LOAD_SIZE_WIDTH) != (sCoordX / LOAD_SIZE_WIDTH) ||
        (m_CurCoordinate.m_sTerrainCoordY / LOAD_SIZE_WIDTH) != (sCoordY / LOAD_SIZE_WIDTH))
    {
        if (bNeedInit)
        {
            m_PrevCoordinate.m_sTerrainCoordX = sCoordX;
            m_PrevCoordinate.m_sTerrainCoordY = sCoordY;
        }
        else
        {
            m_PrevCoordinate.m_sTerrainCoordX = m_CurCoordinate.m_sTerrainCoordX;
            m_PrevCoordinate.m_sTerrainCoordY = m_CurCoordinate.m_sTerrainCoordY;
        }

        m_CurCoordinate.m_sTerrainCoordX = sCoordX;
        m_CurCoordinate.m_sTerrainCoordY = sCoordY;
        m_lCurCoordStartX = sCoordX * CTerrainImpl::TERRAIN_XSIZE;
        m_lCurCoordStartY = sCoordY * CTerrainImpl::TERRAIN_YSIZE;

        short sReferenceCoordMinX, sReferenceCoordMaxX, sReferenceCoordMinY, sReferenceCoordMaxY;
        sReferenceCoordMinX = std::max<short>(m_CurCoordinate.m_sTerrainCoordX - LOAD_SIZE_WIDTH, 0);
        sReferenceCoordMaxX = std::min<short>(m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH, m_sTerrainCountX - 1);
        sReferenceCoordMinY = std::max<short>(m_CurCoordinate.m_sTerrainCoordY - LOAD_SIZE_WIDTH, 0);
        sReferenceCoordMaxY = std::min<short>(m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH, m_sTerrainCountY - 1);

        for (uint16_t usY = sReferenceCoordMinY; usY <= sReferenceCoordMaxY; ++usY)
        {
            for (uint16_t usX = sReferenceCoordMinX; usX <= sReferenceCoordMaxX; ++usX)
            {
                LoadTerrain(usX, usY);
                LoadArea(usX, usY);
            }
        }

        AssignTerrainPtr();
        m_lOldReadX = -1;

        SPDLOG_DEBUG("Update::Load spent {0} ms\n", ELTimer_GetMSec() - t1);
    }
    const float now = DX::StepTimer::instance().GetTotalSeconds();

    // m_Forest CSpeedTreeForest::Instance().UpdateSystem(now);
    /*
     *
    CArea *area;
    if (m_areaLoader.Fetch(&area))
    {

        area->EnablePortal(m_bEnablePortal);
        std::unique_ptr<CArea> areaPtr(area);
        m_AreaVector.push_back(std::move(areaPtr));
        AssignTerrainPtr();
        m_lOldReadX = -1;
    }     */

    CSpeedTreeForestDirectX9::Instance().UpdateSystem(now);
    __UpdateGarbage();
    UpdateTerrain(fX, fY);
    UpdateArea(v3Player);
    UpdateSky();
    return true;
}

void CMapOutdoor::UpdateSky()
{
    m_SkyBox.Update();
}

struct FGetShadowReceiverFromCollisionData
{
    bool m_bCollide;
    std::vector<CGraphicObjectInstance *> *m_pkVct_pkShadowReceiver;
    CDynamicSphereInstance *m_pdsi;

    FGetShadowReceiverFromCollisionData(CDynamicSphereInstance *pdsi,
                                        std::vector<CGraphicObjectInstance *> *pkVct_pkShadowReceiver)
        : m_bCollide(false), m_pdsi(pdsi)
    {
        m_pkVct_pkShadowReceiver = pkVct_pkShadowReceiver;
        m_pkVct_pkShadowReceiver->clear();
    }

    void operator()(CGraphicObjectInstance *pInstance)
    {
        if (!pInstance)
            return;

        if (TREE_OBJECT == pInstance->GetType() || ACTOR_OBJECT == pInstance->GetType() ||
            EFFECT_OBJECT == pInstance->GetType())
            return;
        if (pInstance->CollisionDynamicSphere(*m_pdsi))
        {
            m_pkVct_pkShadowReceiver->push_back(pInstance);
            m_bCollide = true;
        }
    }
};

struct FPCBlockerDistanceSort
{
    Vector3 m_v3Eye;

    FPCBlockerDistanceSort(const Vector3 &v3Eye) : m_v3Eye(v3Eye)
    {
    }

    bool operator()(CGraphicObjectInstance *plhs, CGraphicObjectInstance *prhs) const
    {
        return Vector3::DistanceSquared(plhs->GetPosition(), m_v3Eye) > Vector3::DistanceSquared(prhs->GetPosition(), m_v3Eye);
    }
};

void CMapOutdoor::UpdateAroundAmbience(float fX, float fY, float fZ)
{
    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->UpdateAroundAmbience(fX, fY, fZ);
    }
}

/*virtual*/
void CMapOutdoor::UpdateArea(const Vector3 &v3Player)
{

    m_PCBlockerVector.clear();
    m_ShadowReceiverVector.clear();
    CCameraManager &rCmrMgr = CCameraManager::Instance();
    CCamera *pCamera = rCmrMgr.GetCurrentCamera();
    if (!pCamera)
        return;

    float fDistance = pCamera->GetDistance();

    Vector3 v3View = pCamera->GetView();
    Vector3 v3Target = pCamera->GetTarget();
    Vector3 v3Eye = pCamera->GetEye();

    Vector3 v3Light = Vector3(1.732f, 1.0f, -3.464f); // 빛의 방향
    v3Light *= 50.0f / v3Light.Length();

    __CollectShadowReceiver(v3Player, v3Light);
    __CollectCollisionPCBlocker(v3Eye, v3Player, fDistance);
    __CollectCollisionShadowReceiver(v3Player, v3Light);
    __UpdateAroundAreaList();
}

void CMapOutdoor::__UpdateAroundAreaList()
{
    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->Update();
    }
}

struct FGetShadowReceiverFromHeightData
{
    enum
    {
        COLLECT_MAX = 100,
    };

    uint32_t m_dwCollectOverCount;
    uint32_t m_dwCollectCount;
    uint32_t m_dwCheckCount;
    bool m_bReceiverFound;
    float m_fFromX, m_fFromY, m_fToX, m_fToY;
    float m_fReturnHeight;

    CGraphicObjectInstance *m_apkShadowReceiver[COLLECT_MAX];

    FGetShadowReceiverFromHeightData(float fFromX, float fFromY, float fToX, float fToY)
        : m_bReceiverFound(false), m_fFromX(fFromX), m_fFromY(fFromY), m_fToX(fToX), m_fToY(fToY)
    {
        m_dwCheckCount = 0;
        m_dwCollectOverCount = 0;
        m_dwCollectCount = 0;
    }

    CGraphicObjectInstance *GetCollectItem(uint32_t uIndex)
    {
        if (uIndex >= m_dwCollectCount)
            return nullptr;

        return m_apkShadowReceiver[uIndex];
    }

    uint32_t GetCollectCount()
    {
        return m_dwCollectCount;
    }

    void operator()(CGraphicObjectInstance *pInstance)
    {
        m_dwCheckCount++;

        if (!pInstance)
            return;

        if (m_fFromY < 0)
            m_fFromY = -m_fFromY;
        if (m_fToY < 0)
            m_fToY = -m_fToY;
        if (pInstance->GetObjectHeight(m_fFromX, m_fFromY, &m_fReturnHeight) ||
            pInstance->GetObjectHeight(m_fToX, m_fToY, &m_fReturnHeight))
        {
            if (m_dwCollectCount < COLLECT_MAX)
                m_apkShadowReceiver[m_dwCollectCount++] = pInstance;
            else
                m_dwCollectOverCount++;

            m_bReceiverFound = true;
        }
    }
};

void CMapOutdoor::__CollectShadowReceiver(const Vector3 &v3Target, const Vector3 &v3Light)
{
    CDynamicSphereInstance s;
    s.v3LastPosition = v3Target + v3Light;
    s.v3Position = s.v3LastPosition + v3Light;
    s.fRadius = 50.0f;

    Vector3d aVector3d;
    aVector3d = v3Target;

    CCullingManager &rkCullingMgr = CCullingManager::Instance();

    FGetShadowReceiverFromHeightData f(v3Target.x, v3Target.y, s.v3Position.x, s.v3Position.y);
    rkCullingMgr.ForInRange(aVector3d, 10.0f, &f);

    if (f.m_bReceiverFound)
    {
        for (uint32_t i = 0; i < f.GetCollectCount(); ++i)
        {
            CGraphicObjectInstance *pObjInstEach = f.GetCollectItem(i);
            if (!__IsInShadowReceiverList(pObjInstEach))
                m_ShadowReceiverVector.push_back(pObjInstEach);
        }
    }
}

struct PCBlocker_SInstanceList
{
    typedef CGraphicObjectInstance *Item;
    typedef Item *Iterator;

    enum
    {
        CAPACITY = 512,
    };

    uint32_t m_dwInstCount;
    uint32_t m_dwBlockerCount;
    uint32_t m_dwBlockerOverCount;

    Item m_apkPCBlocker[CAPACITY];

    CDynamicSphereInstance *m_dynSpheres;
    uint32_t m_dynSphereCount;

    CCamera *m_pCamera;
    Vector2 m_v2View;
    Vector2 m_v2Target;

    PCBlocker_SInstanceList(CDynamicSphereInstance *dsi, uint32_t dsiCount)
        : m_dwInstCount(0), m_dwBlockerCount(0), m_dwBlockerOverCount(0), m_dynSpheres(dsi), m_dynSphereCount(dsiCount),
          m_pCamera(CCameraManager::Instance().GetCurrentCamera())
    {
        if (!m_pCamera)
            return;

        Vector3 m_v3View = m_pCamera->GetView();
        Vector3 m_v3Target = m_pCamera->GetTarget();

        m_v2View.x = m_v3View.x;
        m_v2View.y = m_v3View.y;

        m_v2Target.x = m_v3Target.x;
        m_v2Target.y = m_v3Target.y;
    }

    ~PCBlocker_SInstanceList()
    {
#ifdef _DEBUG
        __DEBUG_ShowInstanceMaxCount();
#endif
    }

    void __DEBUG_ShowInstanceMaxCount()
    {
        static uint32_t s_dwInstMaxCount = 0;
        if (s_dwInstMaxCount < m_dwInstCount)
            s_dwInstMaxCount = m_dwInstCount;
    }

    Iterator Begin()
    {
        return m_apkPCBlocker;
    }

    Iterator End()
    {
        return m_apkPCBlocker + m_dwBlockerCount;
    }

    uint32_t Size()
    {
        return m_dwBlockerCount;
    }

    bool IsEmpty()
    {
        if (m_dwBlockerCount > 0)
            return false;

        return true;
    }

    void __AppendPCBlocker(CGraphicObjectInstance *pInstance)
    {
        if (m_dwBlockerCount < CAPACITY)
            m_apkPCBlocker[m_dwBlockerCount++] = pInstance;
        else
            m_dwBlockerOverCount++;
    }

    void __AppendObject(CGraphicObjectInstance *pInstance)
    {
        Vector3 v3Center;
        float fRadius;
        pInstance->GetBoundingSphere(v3Center, fRadius);

        Vector2 v2TargetToCenter;
        v2TargetToCenter.x = v3Center.x - m_v2Target.x;
        v2TargetToCenter.y = v3Center.y - m_v2Target.y;

        if (m_v2View.Dot(v2TargetToCenter) <= 0)
        {
            __AppendPCBlocker(pInstance);
            return;
        }
    }

    void operator()(CGraphicObjectInstance *pInstance)
    {
        if (!m_pCamera || !pInstance)
            return;

        ++m_dwInstCount;

        for (uint32_t i = 0; i < m_dynSphereCount; ++i)
        {
            CDynamicSphereInstance &rkDSI = m_dynSpheres[i];
            if (pInstance->CollisionDynamicSphere(rkDSI))
            {
                if (TREE_OBJECT == pInstance->GetType())
                {
                    __AppendPCBlocker(pInstance);
                    return;
                }
                else if (THING_OBJECT == pInstance->GetType())
                {
                    __AppendObject(pInstance);
                }
                else if (ACTOR_OBJECT == pInstance->GetType())
                {
                    if (((CActorInstance *)pInstance)->IsBuilding())
                    {
                        __AppendObject(pInstance);
                    }
                }
            }
        }
    }
};

void CMapOutdoor::__CollectCollisionPCBlocker(const Vector3 &v3Eye, const Vector3 &v3Target, float fDistance)
{
    Vector3d v3dRayStart;
    v3dRayStart = v3Eye;

    static const uint32_t kCount = 4;
    CDynamicSphereInstance dynSpheres[kCount];

    {
        CDynamicSphereInstance *pkDSI = dynSpheres;
        pkDSI->fRadius = fDistance * 0.5f;
        pkDSI->v3LastPosition = v3Eye;
        pkDSI->v3Position = v3Eye + (v3Target - v3Eye) * 0.5f;
        ++pkDSI;

        pkDSI->fRadius = fDistance * 0.5f;
        pkDSI->v3LastPosition = v3Eye + (v3Target - v3Eye) * 0.5f;
        pkDSI->v3Position = v3Target;
        ++pkDSI;

        pkDSI->fRadius = fDistance * 0.5f;
        pkDSI->v3LastPosition = v3Target;
        pkDSI->v3Position = v3Eye + (v3Target - v3Eye) * 0.5f;
        ++pkDSI;

        pkDSI->fRadius = fDistance * 0.5f;
        pkDSI->v3LastPosition = v3Eye + (v3Target - v3Eye) * 0.5f;
        pkDSI->v3Position = v3Eye;
        ++pkDSI;
    }

    CCullingManager &rkCullingMgr = CCullingManager::Instance();

    PCBlocker_SInstanceList kPCBlockerList(dynSpheres, kCount);
    RangeTester<PCBlocker_SInstanceList> kPCBlockerRangeTester(&kPCBlockerList);
    rkCullingMgr.RangeTest(v3dRayStart, fDistance, &kPCBlockerRangeTester);

    if (!kPCBlockerList.IsEmpty())
    {
        for (auto i = kPCBlockerList.Begin(); i != kPCBlockerList.End(); ++i)
        {
            CGraphicObjectInstance *pObjInstEach = *i;

            if (!pObjInstEach)
                continue;

            if (TREE_OBJECT == pObjInstEach->GetType() && !m_bTransparentTree)
                continue;

            if (!__IsInShadowReceiverList(pObjInstEach))
                if (!__IsInPCBlockerList(pObjInstEach))
                    m_PCBlockerVector.push_back(pObjInstEach);
        }
    }

    std::sort(std::execution::par, m_PCBlockerVector.begin(), m_PCBlockerVector.end(), FPCBlockerDistanceSort(v3Eye));
}

void CMapOutdoor::__CollectCollisionShadowReceiver(const Vector3 &v3Target, const Vector3 &v3Light)
{
    CDynamicSphereInstance s;
    s.fRadius = 50.0f;
    s.v3LastPosition = v3Target + v3Light;
    s.v3Position = s.v3LastPosition + v3Light;

    Vector3d aVector3d = v3Target;

    CCullingManager &rkCullingMgr = CCullingManager::Instance();

    std::vector<CGraphicObjectInstance *> kVct_pkShadowReceiver;
    FGetShadowReceiverFromCollisionData kGetShadowReceiverFromCollisionData(&s, &kVct_pkShadowReceiver);
    rkCullingMgr.ForInRange(aVector3d, 100.0f, &kGetShadowReceiverFromCollisionData);
    if (!kGetShadowReceiverFromCollisionData.m_bCollide)
        return;

    std::vector<CGraphicObjectInstance *>::iterator i;
    for (i = kVct_pkShadowReceiver.begin(); i != kVct_pkShadowReceiver.end(); ++i)
    {
        CGraphicObjectInstance *pObjInstEach = *i;
        if (!__IsInPCBlockerList(pObjInstEach))
            if (!__IsInShadowReceiverList(pObjInstEach))
                m_ShadowReceiverVector.push_back(pObjInstEach);
    }
}

bool CMapOutdoor::__IsInShadowReceiverList(CGraphicObjectInstance *pkObjInstTest)
{
    if (m_ShadowReceiverVector.end() ==
        std::find(m_ShadowReceiverVector.begin(), m_ShadowReceiverVector.end(), pkObjInstTest))
        return false;

    return true;
}

bool CMapOutdoor::__IsInPCBlockerList(CGraphicObjectInstance *pkObjInstTest)
{
    if (m_PCBlockerVector.end() == std::find(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), pkObjInstTest))
        return false;

    return true;
}

// Updates the position of the terrain
void CMapOutdoor::UpdateTerrain(float fX, float fY)
{
    if (fY < 0)
        fY = -fY;

    const int sx = fX;
    const int sy = fY;

    long lDivider = (CTerrainImpl::CELLSCALE * TERRAIN_PATCHSIZE);

    m_lCenterX = (sx - m_lCurCoordStartX) / lDivider;
    m_lCenterY = (sy - m_lCurCoordStartY) / lDivider;

    if ((m_lCenterX != m_lOldReadX) || (m_lCenterY != m_lOldReadY))
    {
        long lRealCenterX = m_lCenterX * TERRAIN_PATCHSIZE;
        long lRealCenterY = m_lCenterY * TERRAIN_PATCHSIZE;
        m_lOldReadX = m_lCenterX;
        m_lOldReadY = m_lCenterY;

        ConvertTerrainToTnL(lRealCenterX, lRealCenterY);
        UpdateAreaList(lRealCenterX, lRealCenterY);
        // SPDLOG_DEBUG("»ç¿ëÇÏ´Â Area, Terrain : (%d, %d), Áö¿ï Area, Terrain : (%d, %d)\n",
        //	m_AreaVector.size(), m_TerrainVector.size(), m_AreaDeleteVector.size(), m_TerrainDeleteVector.size());
    }
}

void CMapOutdoor::__UpdateGarbage()
{
    const uint32_t dwTerrainEraseInterval = 1000 * 60;
    static uint64_t dwEraseTime = ELTimer_GetMSec();

    if (!m_TerrainDeleteVector.empty())
    {
        if (ELTimer_GetMSec() - dwEraseTime <= dwTerrainEraseInterval)
            return;

        m_TerrainDeleteVector.pop_back();

        dwEraseTime = ELTimer_GetMSec();
        return;
    }

    if (!m_AreaDeleteVector.empty())
    {
        if (ELTimer_GetMSec() - dwEraseTime <= dwTerrainEraseInterval)
            return;

        m_AreaDeleteVector.pop_back();

        dwEraseTime = ELTimer_GetMSec();
        return;
    }
}

void CMapOutdoor::UpdateAreaList(long lCenterX, long lCenterY)
{
    if (m_TerrainVector.size() <= AROUND_AREA_NUM && m_AreaVector.size() <= AROUND_AREA_NUM)
        return;

    m_TerrainDeleteVector.clear();
    m_AreaDeleteVector.clear();

    EDeleteDir lr, tb;

    if (lCenterX > CTerrainImpl::XSIZE / 2)
        lr = DELETE_LEFT;
    else
        lr = DELETE_RIGHT;

    if (lCenterY > CTerrainImpl::YSIZE / 2)
        tb = DELETE_TOP;
    else
        tb = DELETE_BOTTOM;

    PruneSectorObjectList(m_TerrainVector, m_TerrainDeleteVector, lr, tb);
    PruneSectorObjectList(m_AreaVector, m_AreaDeleteVector, lr, tb);
}

template <class T> void CMapOutdoor::PruneSectorObjectList(T &objects, T &delQueue, EDeleteDir lr, EDeleteDir tb)
{
    // std::vector<T>::value_type -> std::unique_ptr<U>::element_type -> U
    typedef typename T::value_type::element_type SectorObject;

    auto f = IsUsedSectorObject<SectorObject>(lr, tb, m_CurCoordinate);
    auto firstDeleted = std::partition(objects.begin(), objects.end(), f);

    if (firstDeleted != objects.end())
    {
        // Reserve space (optimization)
        delQueue.reserve(std::distance(firstDeleted, objects.end()));

        std::move(firstDeleted, objects.end(), std::back_inserter(delQueue));
        objects.erase(firstDeleted, objects.end());
    }
}

void CMapOutdoor::ConvertTerrainToTnL(long lx, long ly)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::ConvertTerrainToTnL");

    for (long i = 0; i < m_wPatchCount * m_wPatchCount; i++)
        m_pTerrainPatchProxyList[i].SetUsed(false);

    lx -= m_lViewRadius; /* Move to the top left corner of the */
    ly -= m_lViewRadius; /* input rectangle */

    long diameter = m_lViewRadius * 2;

    long x0 = lx / TERRAIN_PATCHSIZE;
    long y0 = ly / TERRAIN_PATCHSIZE;
    long x1 = (lx + diameter - 1) / TERRAIN_PATCHSIZE;
    long y1 = (ly + diameter - 1) / TERRAIN_PATCHSIZE;

    long xw = x1 - x0 + 1; /* Figure out how many patches are needed */
    long yw = y1 - y0 + 1;

    long ex = lx + diameter;
    long ey = ly + diameter;

    y0 = ly;
    for (long yp = 0; yp < yw; yp++)
    {
        x0 = lx;
        y1 = (y0 / TERRAIN_PATCHSIZE + 1) * TERRAIN_PATCHSIZE;
        if (y1 > ey)
            y1 = ey;
        for (long xp = 0; xp < xw; xp++)
        {
            x1 = (x0 / TERRAIN_PATCHSIZE + 1) * TERRAIN_PATCHSIZE;
            if (x1 > ex)
                x1 = ex;
            AssignPatch(yp * m_wPatchCount + xp, x0, y0, x1, y1);
            x0 = x1;
        }
        y0 = y1;
    }
    UpdateQuadTreeHeights(m_pRootNode);
}

void CMapOutdoor::AssignPatch(long lPatchNum, long x0, long y0, long x1, long y1)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::AssignPatch");

    CTerrainPatchProxy *pTerrainPatchProxy = &m_pTerrainPatchProxyList[lPatchNum];

    if (y0 < 0 && y1 <= 0)
    {
        if (x0 < 0 && x1 <= 0)
        {
            pTerrainPatchProxy->SetTerrainNum(0);
            x0 += CTerrainImpl::XSIZE;
            x1 += CTerrainImpl::XSIZE;
        }
        else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
        {
            pTerrainPatchProxy->SetTerrainNum(2);
            x0 -= CTerrainImpl::XSIZE;
            x1 -= CTerrainImpl::XSIZE;
        }
        else
            pTerrainPatchProxy->SetTerrainNum(1);

        y0 += CTerrainImpl::YSIZE;
        y1 += CTerrainImpl::YSIZE;
    }
    else if (y0 >= CTerrainImpl::YSIZE && y1 > CTerrainImpl::YSIZE)
    {
        if (x0 < 0 && x1 <= 0)
        {
            pTerrainPatchProxy->SetTerrainNum(6);
            x0 += CTerrainImpl::XSIZE;
            x1 += CTerrainImpl::XSIZE;
        }
        else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
        {
            pTerrainPatchProxy->SetTerrainNum(8);
            x0 -= CTerrainImpl::XSIZE;
            x1 -= CTerrainImpl::XSIZE;
        }
        else
            pTerrainPatchProxy->SetTerrainNum(7);

        y0 -= CTerrainImpl::YSIZE;
        y1 -= CTerrainImpl::YSIZE;
    }
    else
    {
        if (x0 < 0 && x1 <= 0)
        {
            pTerrainPatchProxy->SetTerrainNum(3);
            x0 += CTerrainImpl::XSIZE;
            x1 += CTerrainImpl::XSIZE;
        }
        else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
        {
            pTerrainPatchProxy->SetTerrainNum(5);
            x0 -= CTerrainImpl::XSIZE;
            x1 -= CTerrainImpl::XSIZE;
        }
        else
            pTerrainPatchProxy->SetTerrainNum(4);
    }

    CTerrain *pTerrain;
    if (!GetTerrainPointer(pTerrainPatchProxy->GetTerrainNum(), &pTerrain))
        return;

    uint8_t byPatchNumX, byPatchNumY;
    byPatchNumX = x0 / CTerrainImpl::PATCH_XSIZE;
    byPatchNumY = y0 / CTerrainImpl::PATCH_YSIZE;

    CTerrainPatch *pTerrainPatch = pTerrain->GetTerrainPatchPtr(byPatchNumX, byPatchNumY);
    if (!pTerrainPatch)
        return;

    pTerrainPatchProxy->SetPatchNum(byPatchNumY * CTerrainImpl::PATCH_XCOUNT + byPatchNumX);
    pTerrainPatchProxy->SetTerrainPatch(pTerrainPatch);
    pTerrainPatchProxy->SetUsed(true);
}

void CMapOutdoor::UpdateQuadTreeHeights(CTerrainQuadtreeNode *Node)
{
    // Inserted by levites
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::UpdateQuadTreeHeights");
    if (!m_pTerrainPatchProxyList)
        return;

    float minx, maxx, miny, maxy, minz, maxz;
    minx = maxx = miny = maxy = minz = maxz = 0;

    if (m_pTerrainPatchProxyList[Node->PatchNum].isUsed())
    {
        minx = m_pTerrainPatchProxyList[Node->PatchNum].GetMinX();
        maxx = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxX();
        miny = m_pTerrainPatchProxyList[Node->PatchNum].GetMinY();
        maxy = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxY();
        minz = m_pTerrainPatchProxyList[Node->PatchNum].GetMinZ();
        maxz = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxZ();
    }

    for (long y = Node->y0; y <= Node->y1; y++)
    {
        for (long x = Node->x0; x <= Node->x1; x++)
        {
            long patch = y * m_wPatchCount + x;

            if (!m_pTerrainPatchProxyList[patch].isUsed())
                continue;

            if (m_pTerrainPatchProxyList[patch].GetMinX() < minx)
                minx = m_pTerrainPatchProxyList[patch].GetMinX();
            if (m_pTerrainPatchProxyList[patch].GetMaxX() > maxx)
                maxx = m_pTerrainPatchProxyList[patch].GetMaxX();

            if (m_pTerrainPatchProxyList[patch].GetMinY() < miny)
                miny = m_pTerrainPatchProxyList[patch].GetMinY();
            if (m_pTerrainPatchProxyList[patch].GetMaxY() > maxy)
                maxy = m_pTerrainPatchProxyList[patch].GetMaxY();

            if (m_pTerrainPatchProxyList[patch].GetMinZ() < minz)
                minz = m_pTerrainPatchProxyList[patch].GetMinZ();
            if (m_pTerrainPatchProxyList[patch].GetMaxZ() > maxz)
                maxz = m_pTerrainPatchProxyList[patch].GetMaxZ();
        }
    }

    Node->center.x = (maxx + minx) * 0.5f;
    Node->center.y = (maxy + miny) * 0.5f;
    Node->center.z = (maxz + minz) * 0.5f;

    Node->radius =
        sqrtf((maxx - minx) * (maxx - minx) + (maxy - miny) * (maxy - miny) + (maxz - minz) * (maxz - minz)) / 2.0f;

    if (Node->NW_Node != nullptr)
        UpdateQuadTreeHeights(Node->NW_Node);

    if (Node->NE_Node != nullptr)
        UpdateQuadTreeHeights(Node->NE_Node);

    if (Node->SW_Node != nullptr)
        UpdateQuadTreeHeights(Node->SW_Node);

    if (Node->SE_Node != nullptr)
        UpdateQuadTreeHeights(Node->SE_Node);
}
