#ifndef METIN2_CLIENT_ETERLIB_GRPMATH_HPP
#define METIN2_CLIENT_ETERLIB_GRPMATH_HPP

#pragma once

float CrossProduct2D(float x1, float y1, float x2, float y2);

bool IsInTriangle2D(float ax, float ay, float bx, float by, float cx, float cy, float tx, float ty);

Vector3 *D3DXVec3Rotation(Vector3 *pvtOut, const Vector3 *c_pvtSrc, const Quaternion *c_pqtRot);
Vector3 *D3DXVec3Translation(Vector3 *pvtOut, const Vector3 *c_pvtSrc, const Vector3 *c_pvtTrans);

void GetRotationFromMatrix(Vector3 *pRotation, const Matrix *c_pMatrix);
void GetPivotAndRotationFromMatrix(Matrix *pMatrix, Vector3 *pPivot, Vector3 *pRotation);

inline Vector3 *D3DXVec3Blend(Vector3 *pvtOut, const Vector3 *c_pvtSrc1, const Vector3 *c_pvtSrc2,
                                  float d)
{
    pvtOut->x = c_pvtSrc1->x + d * (c_pvtSrc2->x - c_pvtSrc1->x);
    pvtOut->y = c_pvtSrc1->y + d * (c_pvtSrc2->y - c_pvtSrc1->y);
    pvtOut->z = c_pvtSrc1->z + d * (c_pvtSrc2->z - c_pvtSrc1->z);

    return pvtOut;
}

inline Quaternion *D3DXQuaternionBlend(Quaternion *pqtOut, const Quaternion *c_pqtSrc1,
                                           const Quaternion *c_pqtSrc2, float d)
{
    pqtOut->x = c_pqtSrc1->x + d * (c_pqtSrc2->x - c_pqtSrc1->x);
    pqtOut->y = c_pqtSrc1->y + d * (c_pqtSrc2->y - c_pqtSrc1->y);
    pqtOut->z = c_pqtSrc1->z + d * (c_pqtSrc2->z - c_pqtSrc1->z);
    pqtOut->w = c_pqtSrc1->w + d * (c_pqtSrc2->w - c_pqtSrc1->w);
    return pqtOut;
}

// Makes sure that |rot| lies between [0, 360)
inline float ClampDegree(float rot)
{
    if (rot >= 360.0f)
        return std::fmod(rot, 360.0f);
    else if (rot < 0.0f)
        return 360.0f + std::fmod(rot, 360.0f);

    return rot;
}

inline float GetVector3Distance(const Vector3 &c_rv3Source, const Vector3 &c_rv3Target)
{
    return (c_rv3Source.x - c_rv3Target.x) * (c_rv3Source.x - c_rv3Target.x) + (c_rv3Source.y - c_rv3Target.y) * (
               c_rv3Source.y - c_rv3Target.y);
}

inline Quaternion SafeRotationNormalizedArc(const Vector3 &vFrom, const Vector3 &vTo)
{
    if (vFrom == vTo)
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    if (vFrom == -vTo)
        return Quaternion(0.0f, 0.0f, 1.0f, 0.0f);
    Vector3 c = vFrom.Cross(vTo);
    c = vFrom.Cross(vTo);
    float d = vFrom.Dot(vTo);
    float s = sqrtf((1 + d) * 2);

    return Quaternion(c.x / s, c.y / s, c.z / s, s * 0.5f);
}

inline Quaternion RotationNormalizedArc(const Vector3 &vFrom, const Vector3 &vTo)

{
    Vector3 c = vFrom.Cross(vTo);
    float d = vFrom.Dot(vTo);
    float s = sqrtf((1 + d) * 2);

    return Quaternion(c.x / s, c.y / s, c.z / s, s * 0.5f);
}

inline Quaternion RotationArc(const Vector3 &vFrom, const Vector3 &vTo)
{
    Vector3 vnFrom, vnTo;
    vFrom.Normalize(vnFrom);
    vTo.Normalize(vnTo);

    return RotationNormalizedArc(vnFrom, vnTo);
}

inline float square_distance_between_linesegment_and_point(const Vector3 &p1, const Vector3 &p2,
                                                           const Vector3 &x)
{
    float l = Vector3::DistanceSquared(p2, p1);
    float d = (x - p1).Dot(p2 - p1);
    if (d <= 0.0f)
    {
        return (x - p1).LengthSquared();
    }
    else if (d >= l)
    {
        return (x - p2).LengthSquared();
    }
    else
    {
        Vector3 c = (x - p1).Cross(p2 - p1);
        return c.LengthSquared() / l;
    }
}




#endif
