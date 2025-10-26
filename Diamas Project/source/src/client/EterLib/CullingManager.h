#ifndef METIN2_CLIENT_ETERLIB_CULLINGMANAGER_HPP
#define METIN2_CLIENT_ETERLIB_CULLINGMANAGER_HPP

#pragma once

#include "GrpScreen.h"

#include <base/Singleton.hpp>

#include "../SphereLib/spherepack.h"

#include <unordered_set>

class CGraphicObjectInstance;

template <class T>
struct RangeTester : public SpherePackCallback
{
    T *f;
    float dist;

    RangeTester(T *fn, float distance = -1)
        : f(fn)
          , dist(distance)
    {
    }

    virtual ~RangeTester()
    {
    }

    virtual void RayTraceCallback(const Vector3d &p1,  // source pos of ray
                                  const Vector3d &dir, // dest pos of ray
                                  float distance,
                                  const Vector3d &sect,
                                  SpherePack *sphere)
    {
        if (dist <= 0.0f || dist >= distance)
            (*f)((CGraphicObjectInstance *)sphere->GetUserData());
    }

    virtual void VisibilityCallback(CGraphicObjectInstance *pInstance, ContainmentType state)
    {
    };
    virtual void VisibilityCallback(const Frustum &f, SpherePack *sphere, ViewState state)
    {
    };
    virtual void RangeTestCallback(const Vector3d &p, float distance, SpherePack *sphere, ViewState state)
    {
        if (state != VS_OUTSIDE)
            (*f)((CGraphicObjectInstance *)sphere->GetUserData());
    }

    virtual void PointTest2dCallback(const Vector3d &p, SpherePack *sphere, ViewState state)
    {
        if (state != VS_OUTSIDE)
            (*f)((CGraphicObjectInstance *)sphere->GetUserData());
    }
};

class CCullingManager
    : public CSingleton<CCullingManager>
      , public SpherePackCallback
      , private CScreen
{
public:
    typedef SpherePack *CullingHandle;
    typedef std::vector<CGraphicObjectInstance *> TRangeList;

    CCullingManager();
    virtual ~CCullingManager();

    virtual void RayTraceCallback(const Vector3d &p1,  // source pos of ray
                                  const Vector3d &dir, // dest pos of ray
                                  float distance,
                                  const Vector3d &sect,
                                  SpherePack *sphere);

    virtual void VisibilityCallback(
        CGraphicObjectInstance *pInstance, ContainmentType state);
    virtual void VisibilityCallback(const Frustum &f,
                                    SpherePack *sphere,
                                    ViewState state);
    void RangeTestCallback(const Vector3d &p,
                           float distance,
                           SpherePack *sphere,
                           ViewState state);

    void Reset();
    void Update();
    void Process();

    void FindRange(const Vector3d &p, float radius);
    void FindRay(const Vector3d &p1, const Vector3d &dir);
    void FindRayDistance(const Vector3d &p1, const Vector3d &dir, float distance);

    void RangeTest(const Vector3d &p, float radius, SpherePackCallback *callback)
    {
        m_factory.RangeTest(p, radius, callback);
    }

    void PointTest2d(const Vector3d &p, SpherePackCallback *callback)
    {
        m_factory.PointTest2d(p, callback);
    }

    template <class T>
    void ForInRange2d(const Vector3d &p, T *pFunc)
    {
        RangeTester<T> r(pFunc);
        m_factory.PointTest2d(p, &r);
    }

    template <class T>
    void ForInRange(const Vector3d &p, float radius, T *pFunc)
    {
        RangeTester<T> r(pFunc);
        m_factory.RangeTest(p, radius, &r/*this*/);
    }

    template <class T>
    void ForInRay(const Vector3d &p1, const Vector3d &dir, T *pFunc)
    {
        RangeTester<T> r(pFunc);
        m_factory.RayTrace(p1, dir, &r/*this*/);
    }

    template <class T>
    void ForInRayDistance(const Vector3d &p, const Vector3d &dir, float distance, T *pFunc)
    {
        RangeTester<T> r(pFunc, distance);
        m_factory.RayTrace(p, dir, &r/*this*/);
    }

    CullingHandle Register(CGraphicObjectInstance *ob);
    void Unregister(CGraphicObjectInstance *obj);

    TRangeList::iterator begin()
    {
        return m_list.begin();
    }

    TRangeList::iterator end()
    {
        return m_list.end();
    }

protected:
    TRangeList m_list;

    float m_RayFarDistance;
    SpherePackFactory m_factory;
    std::unordered_set<CGraphicObjectInstance *> m_instances;
};

#endif
