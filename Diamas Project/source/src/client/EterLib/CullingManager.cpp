#include "CullingManager.h"
#include "GrpObjectInstance.h"
#include "StdAfx.h"
#include "base/Remotery.h"

#define COUNT_SHOWING_SPHERE
#ifdef COUNT_SHOWING_SPHERE
int showingcount = 0;
#endif

void CCullingManager::RayTraceCallback(const Vector3d & /*p1*/,  // source pos of ray
                                       const Vector3d & /*dir*/, // dest pos of ray
                                       float distance, const Vector3d & /*sect*/, SpherePack *sphere)
{
    if (m_RayFarDistance <= 0.0f || m_RayFarDistance >= distance)
        m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());
}
void CCullingManager::VisibilityCallback(const Frustum & /*f*/, SpherePack *sphere, ViewState state)
{
    CGraphicObjectInstance *pInstance = (CGraphicObjectInstance *)sphere->GetUserData();
    if (state == VS_OUTSIDE)
    {
#ifdef COUNT_SHOWING_SPHERE
        if (pInstance->isShow())
        {
            SPDLOG_DEBUG("SH : {}  ", (void *)pInstance);
            showingcount--;
            SPDLOG_DEBUG("show size : {}\n", showingcount);
        }
#endif
        if (pInstance->isShow() && !pInstance->IsAlwaysRender())
            pInstance->Hide();
    }
    else
    {
#ifdef COUNT_SHOWING_SPHERE
        if (!pInstance->isShow())
        {
            SPDLOG_DEBUG("HS : {}  ", (void *)pInstance);
            showingcount++;
            SPDLOG_DEBUG("show size : {}d\n", showingcount);
        }
#endif
        if ((!pInstance->isForceHide() && !pInstance->isShow()) || pInstance->IsAlwaysRender())
            pInstance->Show();
    }
}
void CCullingManager::VisibilityCallback( CGraphicObjectInstance *pInstance, ContainmentType state)
{
    if (state == DISJOINT)
    {
        if (pInstance->isShow() && !pInstance->IsAlwaysRender())
            pInstance->Hide();
    }
    else
    {
        if ((!pInstance->isForceHide() && !pInstance->isShow()) || pInstance->IsAlwaysRender())
            pInstance->Show();
    }
}

void CCullingManager::RangeTestCallback(const Vector3d & /*p*/, float /*distance*/, SpherePack *sphere, ViewState state)
{
    if (state != VS_OUTSIDE)
        m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());
}

void CCullingManager::Reset()
{
    m_instances.clear();
    m_factory.Reset();
}

void CCullingManager::Update()
{

    m_factory.Process();
}

void CCullingManager::Process()
{
    rmt_ScopedCPUSample(ProcessCulling, 0);
    UpdateViewMatrix();
    UpdateProjMatrix();
    BuildViewFrustum();

    Vector3 center;
    float radius;

    for(auto& inst : m_instances) {
      inst->GetBoundingSphere(center, radius);
      BoundingSphere boundingSphere(center, radius);

      VisibilityCallback(inst,  ms_frustum.Contains(boundingSphere));
    }
;
}

CCullingManager::CullingHandle CCullingManager::Register(CGraphicObjectInstance *obj)
{
    assert(obj);
#ifdef COUNT_SHOWING_SPHERE
    SPDLOG_DEBUG("CR : {}  ", (void *)obj);
    showingcount++;
    SPDLOG_DEBUG("show size : {}\n", showingcount);
#endif

    m_instances.insert(obj);
    Vector3d center;
    float radius;
    obj->GetBoundingSphere(center, radius);
    return m_factory.AddSphere_(center, radius, obj, false);
}

void CCullingManager::Unregister(CGraphicObjectInstance *obj)
{
    m_instances.erase(obj);
    m_factory.Remove(obj->GetCullingHandle());
}

CCullingManager::CCullingManager()
    : m_factory(10000, // maximum count
                6400,  // root radius
                1600,  // leaf radius
                400    // extra radius
                ),
      m_RayFarDistance(0.0f)
{
}

CCullingManager::~CCullingManager()
{
}
void CCullingManager::FindRange(const Vector3d &p, float radius)
{
    m_list.clear();
    m_factory.RangeTest(p, radius, this);
}

void CCullingManager::FindRay(const Vector3d &p1, const Vector3d &dir)
{
    m_RayFarDistance = 0.0f;
    m_list.clear();
    m_factory.RayTrace(p1, dir, this);
}

void CCullingManager::FindRayDistance(const Vector3d &p1, const Vector3d &dir, float distance)
{
    m_RayFarDistance = distance;
    m_list.clear();
    m_factory.RayTrace(p1, dir, this);
}
