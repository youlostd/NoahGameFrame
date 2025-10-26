#ifndef METIN2_CLIENT_ETERLIB_GRPCOLLISIONOBJECT_HPP
#define METIN2_CLIENT_ETERLIB_GRPCOLLISIONOBJECT_HPP

#pragma once

#include "GrpBase.h"

class CGraphicCollisionObject : public CGraphicBase
{
public:
    CGraphicCollisionObject();
    virtual ~CGraphicCollisionObject();

    bool IntersectTriangle(const Vector3 &c_orig, const Vector3 &c_dir, const Vector3 &c_v0,
                           const Vector3 &c_v1, const Vector3 &c_v2, float *pu, float *pv, float *pt);
    bool IntersectBoundBox(const Matrix *c_pmatWorld, const TBoundBox &c_rboundBox, float *pu, float *pv,
                           float *pt);
    bool IntersectCube(const Matrix *c_pmatWorld, float sx, float sy, float sz, float ex, float ey, float ez,
                       Vector3 &RayOriginal, Vector3 &RayDirection, float *pu, float *pv, float *pt);
    bool IntersectIndexedMesh(const Matrix *c_pmatWorld, const void *vertices, int step, int vtxCount,
                              const void *indices, int idxCount, Vector3 &RayOriginal, Vector3 &RayDirection,
                              float *pu, float *pv, float *pt);
    bool IntersectMesh(const Matrix *c_pmatWorld, const void *vertices, uint32_t dwStep, uint32_t dwvtxCount,
                       Vector3 &RayOriginal, Vector3 &RayDirection, float *pu, float *pv, float *pt);

    bool IntersectSphere(const Vector3 &c_rv3Position, float fRadius, const Vector3 &c_rv3RayOriginal,
                         const Vector3 &c_rv3RayDirection);
    bool IntersectCylinder(const Vector3 &c_rv3Position, float fRadius, float fHeight,
                           const Vector3 &c_rv3RayOriginal, const Vector3 &c_rv3RayDirection);

    // NOTE : ms_vtPickRayOrig와 ms_vtPickRayDir를 CGraphicBGase가 가지고 있는데
    //        굳이 인자로 넣어줘야 하는 이유가 있는가? Customize를 위해서? - [levites]
    bool IntersectSphere(const Vector3 &c_rv3Position, float fRadius);
    bool IntersectCylinder(const Vector3 &c_rv3Position, float fRadius, float fHeight);
};

#endif
