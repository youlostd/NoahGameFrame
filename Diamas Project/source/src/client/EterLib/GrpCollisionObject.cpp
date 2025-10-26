#include "StdAfx.h"
#include "GrpCollisionObject.h"

bool CGraphicCollisionObject::IntersectBoundBox(const Matrix *c_pmatWorld, const TBoundBox &c_rboundBox, float *pu,
                                                float *pv, float *pt)
{
    return IntersectCube(c_pmatWorld, c_rboundBox.sx, c_rboundBox.sy, c_rboundBox.sz, c_rboundBox.ex, c_rboundBox.ey,
                         c_rboundBox.ez, ms_vtPickRayOrig, ms_vtPickRayDir, pu, pv, pt);
}

bool CGraphicCollisionObject::IntersectCube(const Matrix *c_pmatWorld, float sx, float sy, float sz, float ex,
                                            float ey, float ez,
                                            Vector3 &RayOriginal, Vector3 &RayDirection, float *pu, float *pv,
                                            float *pt)
{
    TPosition posVertices[8];

    posVertices[0] = TPosition(sx, sy, sz);
    posVertices[1] = TPosition(ex, sy, sz);
    posVertices[2] = TPosition(sx, ey, sz);
    posVertices[3] = TPosition(ex, ey, sz);
    posVertices[4] = TPosition(sx, sy, ez);
    posVertices[5] = TPosition(ex, sy, ez);
    posVertices[6] = TPosition(sx, ey, ez);
    posVertices[7] = TPosition(ex, ey, ez);

    static const uint16_t c_awFillCubeIndices[36] = {
        0, 1, 2, 1, 3, 2,
        2, 0, 6, 0, 4, 6,
        0, 1, 4, 1, 5, 4,
        1, 3, 5, 3, 7, 5,
        3, 2, 7, 2, 6, 7,
        4, 5, 6, 5, 7, 6,
    };

    return IntersectIndexedMesh(
        c_pmatWorld,
        posVertices,
        sizeof(TPosition),
        8,
        c_awFillCubeIndices,
        36,
        RayOriginal,
        RayDirection,
        pu,
        pv,
        pt
        );
}

const int c_iLimitVertexCount = 1024;

bool CGraphicCollisionObject::IntersectIndexedMesh(const Matrix *c_pmatWorld, const void *vertices, int step,
                                                   int vtxCount, const void *indices, int idxCount,
                                                   Vector3 &RayOriginal, Vector3 &RayDirection, float *pu,
                                                   float *pv, float *pt)
{
    static Vector3 s_v3PositionArray[c_iLimitVertexCount];
    static uint32_t s_dwPositionCount;

    if (vtxCount > c_iLimitVertexCount)
    {
        SPDLOG_ERROR("The vertex count of mesh which is used in collision detection is too much : {0} / {1}", vtxCount,
                      c_iLimitVertexCount);
        return false;
    }

    s_dwPositionCount = 0;

    char *pcurVtx = (char *)vertices;

    while (vtxCount--)
    {
        float *pos = (float *)pcurVtx;

        s_v3PositionArray[s_dwPositionCount++] = Vector3::Transform(*(Vector3*)pos, *c_pmatWorld);

        pcurVtx += step;
    }

    uint16_t *pcurIdx = (uint16_t *)indices;

    int triCount = idxCount / 3;
    while (triCount--)
    {
        if (IntersectTriangle(RayOriginal, RayDirection,
                              s_v3PositionArray[pcurIdx[0]],
                              s_v3PositionArray[pcurIdx[1]],
                              s_v3PositionArray[pcurIdx[2]],
                              pu, pv, pt))
        {
            return true;
        }

        pcurIdx += 3;
    }

    return false;
}

bool CGraphicCollisionObject::IntersectMesh(const Matrix *c_pmatWorld, const void *vertices, uint32_t dwStep,
                                            uint32_t dwvtxCount, Vector3 &RayOriginal, Vector3 &RayDirection,
                                            float *pu, float *pv, float *pt)
{
    char *pcurVtx = (char *)vertices;

    Vector3 v3Vertex[3];

    for (uint32_t i = 0; i < dwvtxCount; i += 3)
    {
        v3Vertex[0] = Vector3::Transform(*(Vector3 *)pcurVtx, *c_pmatWorld);
        pcurVtx += dwStep;

        v3Vertex[1] = Vector3::Transform(*(Vector3 *)pcurVtx, *c_pmatWorld);
        pcurVtx += dwStep;

        v3Vertex[2] = Vector3::Transform(*(Vector3 *)pcurVtx, *c_pmatWorld);
        pcurVtx += dwStep;

        if (IntersectTriangle(RayOriginal, RayDirection,
                              v3Vertex[0], v3Vertex[1], v3Vertex[2],
                              pu, pv, pt))
        {
            return true;
        }
    }

    return false;
}

bool CGraphicCollisionObject::IntersectTriangle(const Vector3 &c_orig,
                                                const Vector3 &c_dir,
                                                const Vector3 &c_v0,
                                                const Vector3 &c_v1,
                                                const Vector3 &c_v2,
                                                float *pu,
                                                float *pv,
                                                float *pt)
{
    Vector3 edge1 = c_v1 - c_v0;
    Vector3 edge2 = c_v2 - c_v0;
    Vector3 pvec;
    pvec = c_dir.Cross(edge2);

    FLOAT det = edge1.Dot(pvec);
    Vector3 tvec;

    if (det > 0)
    {
        tvec = c_orig - c_v0;
    }
    else
    {
        tvec = c_v0 - c_orig;
        det = -det;
    }

    if (det < 0.0001f)
        return false;

    float u, v, t;
    u = tvec.Dot(pvec);
    if (u < 0.0f || u > det)
        return false;

    Vector3 qvec = tvec.Cross(edge1);

    v = c_dir.Dot(qvec);
    if (v < 0.0f || u + v > det)
        return false;

    t = edge2.Dot(qvec);
    FLOAT fInvDet = 1.0f / det;
    t *= fInvDet;
    u *= fInvDet;
    v *= fInvDet;

    Vector3 spot = edge1 * u + edge2 * v;
    spot += c_v0;

    *pu = spot.x;
    *pv = spot.y;
    *pt = t;

    return true;
}

bool CGraphicCollisionObject::IntersectSphere(const Vector3 &c_rv3Position, float fRadius,
                                              const Vector3 &c_rv3RayOriginal, const Vector3 &c_rv3RayDirection)
{
    Vector3 v3RayOriginal = c_rv3RayOriginal - c_rv3Position;

    float a = c_rv3RayDirection.Dot(c_rv3RayDirection);
    float b = 2 * v3RayOriginal.Dot(c_rv3RayDirection);
    float c = v3RayOriginal.Dot(v3RayOriginal) - fRadius * fRadius;

    float D = b * b - 4 * a * c;

    if (D >= 0)
        return true;

    return false;
}

bool CGraphicCollisionObject::IntersectCylinder(const Vector3 &c_rv3Position, float fRadius, float fHeight,
                                                const Vector3 &c_rv3RayOriginal,
                                                const Vector3 &c_rv3RayDirection)
{
    Vector3 v3RayOriginal = c_rv3RayOriginal - c_rv3Position;

    float a = c_rv3RayDirection.x * c_rv3RayDirection.x + c_rv3RayDirection.y * c_rv3RayDirection.y;
    float b = 2 * (v3RayOriginal.x * c_rv3RayDirection.x + v3RayOriginal.y * c_rv3RayDirection.y);
    float c = v3RayOriginal.x * v3RayOriginal.x + v3RayOriginal.y * v3RayOriginal.y - fRadius * fRadius;

    float D = b * b - 4 * a * c;
    if (D > 0)
        if (0.0f != a)
        {
            float tPlus = (-b + sqrtf(D)) / (2 * a);
            float tMinus = (-b - sqrtf(D)) / (2 * a);
            float fzPlus = v3RayOriginal.z + tPlus * c_rv3RayDirection.z;
            float fzMinus = v3RayOriginal.z + tMinus * c_rv3RayDirection.z;

            if (fzPlus > 0.0f && fzPlus <= fHeight)
                return true;
            if (fzMinus > 0.0f && fzMinus <= fHeight)
                return true;
            if (fzMinus * fzPlus < 0.0f)
                return true;
        }

    return false;
}

bool CGraphicCollisionObject::IntersectSphere(const Vector3 &c_rv3Position, float fRadius)
{
    return CGraphicCollisionObject::IntersectSphere(c_rv3Position, fRadius, ms_vtPickRayOrig, ms_vtPickRayDir);
}

bool CGraphicCollisionObject::IntersectCylinder(const Vector3 &c_rv3Position, float fRadius, float fHeight)
{
    return CGraphicCollisionObject::IntersectCylinder(c_rv3Position, fRadius, fHeight, ms_vtPickRayOrig,
                                                      ms_vtPickRayDir);
}

CGraphicCollisionObject::CGraphicCollisionObject()
{
}

CGraphicCollisionObject::~CGraphicCollisionObject()
{
}
