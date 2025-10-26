#include "Stdafx.h"
#include "CollisionData.h"
#include "Pool.h"
#include "GrpScreen.h"
#include "GrpMath.h"
#include "lineintersect_utils.h"
#include "StateManager.h"

const float gc_fReduceMove = 0.5f;

//const float gc_fSlideMoveSpeed = 5.0f;
/*inline Vector3 FitAtSpecifiedLength(const Vector3 & v3Vector, float length)
{
	Vector3 v;
	D3DXVec3Normalize(&v,&v3Vector);
	return v*length;
}
*/
CDynamicPool<CSphereCollisionInstance> gs_sci;
CDynamicPool<CCylinderCollisionInstance> gs_cci;
CDynamicPool<CPlaneCollisionInstance> gs_pci;
CDynamicPool<CAABBCollisionInstance> gs_aci;
CDynamicPool<COBBCollisionInstance> gs_oci;

void DestroyCollisionInstanceSystem()
{
    gs_sci.Destroy();
    gs_cci.Destroy();
    gs_pci.Destroy();
    gs_aci.Destroy();
    gs_oci.Destroy();
}

/////////////////////////////////////////////
// Base
CBaseCollisionInstance *CBaseCollisionInstance::BuildCollisionInstance(const CStaticCollisionData *c_pCollisionData,
                                                                       const Matrix *pMat)
{
    switch (c_pCollisionData->dwType)
    {
    case COLLISION_TYPE_PLANE: {
        CPlaneCollisionInstance *ppci = gs_pci.Alloc();
        Matrix matRotation = Matrix::CreateFromQuaternion(c_pCollisionData->quatRotation);
        Matrix matTranslationLocal = Matrix::CreateTranslation(c_pCollisionData->v3Position);
        Matrix matTransform = matRotation * matTranslationLocal * *pMat;

        TPlaneData &PlaneData = ppci->GetAttribute();
        PlaneData.v3Position = Vector3::Transform(c_pCollisionData->v3Position, *pMat);
        float fHalfWidth = c_pCollisionData->fDimensions[0] / 2.0f;
        float fHalfLength = c_pCollisionData->fDimensions[1] / 2.0f;

        PlaneData.v3QuadPosition[0].x = -fHalfWidth;
        PlaneData.v3QuadPosition[0].y = -fHalfLength;
        PlaneData.v3QuadPosition[0].z = 0.0f;
        PlaneData.v3QuadPosition[1].x = +fHalfWidth;
        PlaneData.v3QuadPosition[1].y = -fHalfLength;
        PlaneData.v3QuadPosition[1].z = 0.0f;
        PlaneData.v3QuadPosition[2].x = -fHalfWidth;
        PlaneData.v3QuadPosition[2].y = +fHalfLength;
        PlaneData.v3QuadPosition[2].z = 0.0f;
        PlaneData.v3QuadPosition[3].x = +fHalfWidth;
        PlaneData.v3QuadPosition[3].y = +fHalfLength;
        PlaneData.v3QuadPosition[3].z = 0.0f;
        for (uint32_t i = 0; i < 4; ++i)
            PlaneData.v3QuadPosition[i] = Vector3::Transform(PlaneData.v3QuadPosition[i], matTransform);
        Vector3 v3Line0 = PlaneData.v3QuadPosition[1] - PlaneData.v3QuadPosition[0];
        Vector3 v3Line1 = PlaneData.v3QuadPosition[2] - PlaneData.v3QuadPosition[0];
        Vector3 v3Line2 = PlaneData.v3QuadPosition[1] - PlaneData.v3QuadPosition[3];
        Vector3 v3Line3 = PlaneData.v3QuadPosition[2] - PlaneData.v3QuadPosition[3];
        v3Line0.Normalize();
        v3Line1.Normalize();
        v3Line2.Normalize();
        v3Line3.Normalize();
        PlaneData.v3Normal = v3Line0.Cross(v3Line1);
        PlaneData.v3Normal.Normalize();


        PlaneData.v3InsideVector[0] = PlaneData.v3Normal.Cross(v3Line0);
        PlaneData.v3InsideVector[1] = v3Line1.Cross(PlaneData.v3Normal);
        PlaneData.v3InsideVector[2] = v3Line2.Cross(PlaneData.v3Normal);
        PlaneData.v3InsideVector[3] = PlaneData.v3Normal.Cross(v3Line3);

        return ppci;
    }
    break;
    case COLLISION_TYPE_BOX:
        assert(false && "COLLISION_TYPE_BOX not implemented");
        break;
    case COLLISION_TYPE_AABB: {
        CAABBCollisionInstance *paci = gs_aci.Alloc();

        Matrix matTranslationLocal = Matrix::CreateTranslation(c_pCollisionData->v3Position);
        Matrix matTransform = *pMat;

        Vector3 v3Pos;
        v3Pos.x = matTranslationLocal._41;
        v3Pos.y = matTranslationLocal._42;
        v3Pos.z = matTranslationLocal._43;

        TAABBData &AABBData = paci->GetAttribute();
        AABBData.v3Min.x = v3Pos.x - c_pCollisionData->fDimensions[0];
        AABBData.v3Min.y = v3Pos.y - c_pCollisionData->fDimensions[1];
        AABBData.v3Min.z = v3Pos.z - c_pCollisionData->fDimensions[2];
        AABBData.v3Max.x = v3Pos.x + c_pCollisionData->fDimensions[0];
        AABBData.v3Max.y = v3Pos.y + c_pCollisionData->fDimensions[1];
        AABBData.v3Max.z = v3Pos.z + c_pCollisionData->fDimensions[2];

        AABBData.v3Min = Vector3::Transform(AABBData.v3Min, matTransform);
        AABBData.v3Max = Vector3::Transform(AABBData.v3Max, matTransform);

        return paci;
    }
    break;
    case COLLISION_TYPE_OBB: {
        COBBCollisionInstance *poci = gs_oci.Alloc();

        Matrix matTranslationLocal = Matrix::CreateTranslation(c_pCollisionData->v3Position);
        Matrix matRotation = Matrix::CreateFromQuaternion(c_pCollisionData->quatRotation);

        Matrix matTranslationWorld = Matrix::Identity;

        matTranslationWorld._41 = pMat->_41;
        matTranslationWorld._42 = pMat->_42;
        matTranslationWorld._43 = pMat->_43;
        matTranslationWorld._44 = pMat->_44;

        Vector3 v3Min, v3Max;
        v3Min.x = c_pCollisionData->v3Position.x - c_pCollisionData->fDimensions[0];
        v3Min.y = c_pCollisionData->v3Position.y - c_pCollisionData->fDimensions[1];
        v3Min.z = c_pCollisionData->v3Position.z - c_pCollisionData->fDimensions[2];
        v3Max.x = c_pCollisionData->v3Position.x + c_pCollisionData->fDimensions[0];
        v3Max.y = c_pCollisionData->v3Position.y + c_pCollisionData->fDimensions[1];
        v3Max.z = c_pCollisionData->v3Position.z + c_pCollisionData->fDimensions[2];

        v3Min = Vector3::Transform(v3Min, *pMat);
        v3Max = Vector3::Transform(v3Max, *pMat);
        Vector3 v3Position = (v3Min + v3Max) * 0.5f;

        TOBBData &OBBData = poci->GetAttribute();
        OBBData.v3Min.x = v3Position.x - c_pCollisionData->fDimensions[0];
        OBBData.v3Min.y = v3Position.y - c_pCollisionData->fDimensions[1];
        OBBData.v3Min.z = v3Position.z - c_pCollisionData->fDimensions[2];
        OBBData.v3Max.x = v3Position.x + c_pCollisionData->fDimensions[0];
        OBBData.v3Max.y = v3Position.y + c_pCollisionData->fDimensions[1];
        OBBData.v3Max.z = v3Position.z + c_pCollisionData->fDimensions[2];

        Matrix matTransform = *pMat;

        OBBData.matRot = *pMat;
        OBBData.matRot._41 = 0;
        OBBData.matRot._42 = 0;
        OBBData.matRot._43 = 0;
        OBBData.matRot._44 = 1;

        return poci;
    }
    break;
    case COLLISION_TYPE_SPHERE: {
        CSphereCollisionInstance *psci = gs_sci.Alloc();

        Matrix matTranslationLocal = Matrix::CreateTranslation(c_pCollisionData->v3Position);
        matTranslationLocal = matTranslationLocal * *pMat;

        TSphereData &SphereData = psci->GetAttribute();
        SphereData.v3Position.x = matTranslationLocal._41;
        SphereData.v3Position.y = matTranslationLocal._42;
        SphereData.v3Position.z = matTranslationLocal._43;
        SphereData.fRadius = c_pCollisionData->fDimensions[0];

        return psci;
    }
    break;
    case COLLISION_TYPE_CYLINDER: {
        CCylinderCollisionInstance *pcci = gs_cci.Alloc();
        Matrix matTranslationLocal = Matrix::CreateTranslation(c_pCollisionData->v3Position);
        matTranslationLocal = matTranslationLocal * *pMat;

        TCylinderData &CylinderData = pcci->GetAttribute();
        CylinderData.fRadius = c_pCollisionData->fDimensions[0];
        CylinderData.fHeight = c_pCollisionData->fDimensions[1];
        CylinderData.v3Position.x = matTranslationLocal._41;
        CylinderData.v3Position.y = matTranslationLocal._42;
        CylinderData.v3Position.z = matTranslationLocal._43 /*+ CylinderData.fHeight/2.0f*/;

        return pcci;
    }
    break;
    }
    assert(false && "NOT_REACHED");
    return 0;
}

void CBaseCollisionInstance::Destroy()
{
    OnDestroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*------------------------------------------------------Sphere---------------------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSphereData &CSphereCollisionInstance::GetAttribute()
{
    return m_attribute;
}

const TSphereData &CSphereCollisionInstance::GetAttribute() const
{
    return m_attribute;
}

void CSphereCollisionInstance::Render(D3DFILLMODE d3dFillMode)
{
    static CScreen s;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    s.RenderSphere(NULL, m_attribute.v3Position.x, m_attribute.v3Position.y, m_attribute.v3Position.z,
                   m_attribute.fRadius, d3dFillMode);
}

void CSphereCollisionInstance::OnDestroy()
{
    gs_sci.Free(this);
}

bool CSphereCollisionInstance::OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    if (square_distance_between_linesegment_and_point(s.v3LastPosition, s.v3Position, m_attribute.v3Position) < (
            m_attribute.fRadius + s.fRadius) * (m_attribute.fRadius + s.fRadius))
    {
        // NOTE : 거리가 가까워 졌을때만.. - [levites]
        if (GetVector3Distance(s.v3Position, m_attribute.v3Position) <
            GetVector3Distance(s.v3LastPosition, m_attribute.v3Position))
            return true;
    }

    return false;
}

bool CSphereCollisionInstance::OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    if (square_distance_between_linesegment_and_point(s.v3LastPosition, s.v3Position, m_attribute.v3Position) < (
            m_attribute.fRadius + s.fRadius) * (m_attribute.fRadius + s.fRadius))
    {
        return true;
    }

    return false;
}

Vector3 CSphereCollisionInstance::OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    if (Vector3::DistanceSquared(s.v3Position, m_attribute.v3Position) >=
        (s.fRadius + m_attribute.fRadius) * (m_attribute.fRadius + s.fRadius))
        return Vector3(0.0f, 0.0f, 0.0f);
    Vector3 c;
    c = (s.v3Position - s.v3LastPosition).Cross(Vector3::Backward);

    float sum = -c.Dot((s.v3Position - m_attribute.v3Position));
    float mul = (s.fRadius + m_attribute.fRadius) * (s.fRadius + m_attribute.fRadius) -
                Vector3::DistanceSquared(s.v3Position, m_attribute.v3Position);

    if (sum * sum - 4 * mul <= 0)
        return Vector3(0.0f, 0.0f, 0.0f);
    float sq = sqrt(sum * sum - 4 * mul);
    float t1 = -sum - sq, t2 = -sum + sq;
    t1 *= 0.5f;
    t2 *= 0.5f;

    if (fabs(t1) <= fabs(t2))
    {
        return c * (gc_fReduceMove * t1);
    }
    else
        return c * (gc_fReduceMove * t2);

    /*
    Vector3 p1 = s.v3Position+t1*c;
    Vector3 p2 = s.v3Position+t2*c;

    if (D3DXVec3LengthSq(&(p2-s.v3Position))>D3DXVec3LengthSq(&(p1-s.v3Position)))
    {
        return p1-s.v3Position;
    }
    else
    {
        return p2-s.v3Position;
    }
    */
}

/////////////////////////////////////////////
// Plane
TPlaneData &CPlaneCollisionInstance::GetAttribute()
{
    return m_attribute;
}

const TPlaneData &CPlaneCollisionInstance::GetAttribute() const
{
    return m_attribute;
}

bool CPlaneCollisionInstance::OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v3SpherePosition = s.v3Position - m_attribute.v3Position;
    Vector3 v3SphereLastPosition = s.v3LastPosition - m_attribute.v3Position;

    float fPosition1 = m_attribute.v3Normal.Dot(v3SpherePosition);
    float fPosition2 = m_attribute.v3Normal.Dot(v3SphereLastPosition);

    if (fPosition1 > 0.0f && fPosition2 < 0.0f || fPosition1 < 0.0f && fPosition2 > 0.0f
        || (fPosition1) <= s.fRadius && fPosition1 >= -s.fRadius)
    {
        Vector3 v3QuadPosition1 = s.v3Position - m_attribute.v3QuadPosition[0];
        Vector3 v3QuadPosition2 = s.v3Position - m_attribute.v3QuadPosition[3];

        if (v3QuadPosition1.Dot(m_attribute.v3InsideVector[0]) > - s.fRadius/*0.0f*/)
            if (v3QuadPosition1.Dot(m_attribute.v3InsideVector[1]) > -s.fRadius/*0.0f*/)
                if (v3QuadPosition2.Dot(m_attribute.v3InsideVector[2]) > - s.fRadius/*0.0f*/)
                    if (v3QuadPosition2.Dot(m_attribute.v3InsideVector[3]) > - s.fRadius/*0.0f*/)
                    {
                        // NOTE : 거리가 가까워 졌을때만.. - [levites]
                        if (fabs((s.v3Position - m_attribute.v3Position).Dot(m_attribute.v3Normal)) <
                            fabs((s.v3LastPosition - m_attribute.v3Position).Dot(m_attribute.v3Normal)))
                            return true;
                    }
    }

    return false;
}

bool CPlaneCollisionInstance::OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v3SpherePosition = s.v3Position - m_attribute.v3Position;
    Vector3 v3SphereLastPosition = s.v3LastPosition - m_attribute.v3Position;

    float fPosition1 = m_attribute.v3Normal.Dot(v3SpherePosition);
    float fPosition2 = m_attribute.v3Normal.Dot(v3SphereLastPosition);

    if (fPosition1 > 0.0f && fPosition2 < 0.0f || fPosition1 < 0.0f && fPosition2 > 0.0f
        || (fPosition1) <= s.fRadius && fPosition1 >= -s.fRadius)
    {
        Vector3 v3QuadPosition1 = s.v3Position - m_attribute.v3QuadPosition[0];
        Vector3 v3QuadPosition2 = s.v3Position - m_attribute.v3QuadPosition[3];

        if (v3QuadPosition1.Dot(m_attribute.v3InsideVector[0]) > - s.fRadius/*0.0f*/)
            if (v3QuadPosition1.Dot(m_attribute.v3InsideVector[1]) > -s.fRadius/*0.0f*/)
                if (v3QuadPosition2.Dot(m_attribute.v3InsideVector[2]) > - s.fRadius/*0.0f*/)
                    if (v3QuadPosition2.Dot(m_attribute.v3InsideVector[3]) > - s.fRadius/*0.0f*/)
                    {
                        return true;
                    }
    }

    return false;
}

Vector3 CPlaneCollisionInstance::OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    Vector3 advance = s.v3Position - s.v3LastPosition;

    float d = m_attribute.v3Normal.Dot(advance);
    if (d >= -0.0001 && d <= 0.0001)
        return Vector3(0.0f, 0.0f, 0.0f);
    float t = - m_attribute.v3Normal.Dot(s.v3Position - m_attribute.v3Position) / d;

    //Vector3 onplane = s.v3Position+t*advance;

    if (m_attribute.v3Normal.Dot(advance) >= 0)
    {
        //return m_attribute.v3Normal*((-s.fRadius+D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position)))*gc_fReduceMove);
        return advance * t - m_attribute.v3Normal * s.fRadius;
    }
    else
    {
        //return m_attribute.v3Normal*((s.fRadius+D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position)))*gc_fReduceMove);
        return advance * t + m_attribute.v3Normal * s.fRadius;
    }

    /*if (D3DXVec3Dot(&m_attribute.v3Normal, &advance)>=0)
    {
        Tracef("%f %f\n",s.fRadius,-(D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position))));
        return m_attribute.v3Normal*((-s.fRadius+D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position)))*gc_fReduceMove);
    }
    else
    {
        Tracef("%f %f\n",(s.fRadius),(D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position))));
        return m_attribute.v3Normal*((s.fRadius+D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position-m_attribute.v3Position)))*gc_fReduceMove);
    }*/

    /*
    Vector3 advance = s.v3Position-s.v3LastPosition;
    Vector3 slide(-advance.y,advance.x,advance.z);
    slide = m_attribute.v3Normal;

    Vector3 radius_adjust = advance;
    D3DXVec3Normalize(&radius_adjust,&radius_adjust);
    radius_adjust*=s.fRadius;

    float d = D3DXVec3Dot(&m_attribute.v3Normal, &slide);
    if (d>=-0.0001 && d<=0.0001)
        return Vector3(0.0f,0.0f,0.0f);

    float t= - D3DXVec3Dot(&m_attribute.v3Normal, &(s.v3Position+radius_adjust-m_attribute.v3Position))
                / d;*/

    //Vector3 nextposition;
    //nextposition = s.v3Position + t*slide;
    //Tracef("$T %f",t);
    //if (D3DXVec3Dot(&m_attribute.v3Normal, &advance)>=0)
    //	return (t*slide - m_attribute.v3Normal * s.fRadius)/**gc_fReduceMove*/;
    //else
    //	return (t*slide + m_attribute.v3Normal * s.fRadius)/*gc_fReduceMove*/;
    //if (D3DXVec3Dot(&m_attribute.v3Normal, &advance)>=0)
    //	return (t*slide + m_attribute.v3Normal * D3DXVec3Dot(&m_attribute.v3Normal,&(s.v3LastPosition-m_attribute.v3Position))/** s.fRadius*/)*gc_fReduceMove;
    //else
    //	return (t*slide + m_attribute.v3Normal * D3DXVec3Dot(&m_attribute.v3Normal,&(s.v3LastPosition-m_attribute.v3Position))/*s.fRadius*/)*gc_fReduceMove;
    //
}

void CPlaneCollisionInstance::Render(D3DFILLMODE /*d3dFillMode*/)
{
    static CScreen s;
    s.RenderBar3d(m_attribute.v3QuadPosition);
}

void CPlaneCollisionInstance::OnDestroy()
{
    gs_pci.Free(this);
}

/////////////////////////////////////////////
// Cylinder
TCylinderData &CCylinderCollisionInstance::GetAttribute()
{
    return m_attribute;
}

const TCylinderData &CCylinderCollisionInstance::GetAttribute() const
{
    return m_attribute;
}

bool CCylinderCollisionInstance::CollideCylinderVSDynamicSphere(const TCylinderData &c_rattribute,
                                                                const CDynamicSphereInstance &s) const
{
    if (s.v3Position.z + s.fRadius < c_rattribute.v3Position.z)
        return false;

    if (s.v3Position.z - s.fRadius > c_rattribute.v3Position.z + c_rattribute.fHeight)
        return false;

    /*Vector2 v2curDistance(s.v3Position.x - c_rattribute.v3Position.x, s.v3Position.y - c_rattribute.v3Position.y);
    float fDistance = D3DXVec2Length(&v2curDistance);
    if (fDistance <= s.fRadius + c_rattribute.fRadius)
        return true;
        */

    Vector3 oa, ob;
    IntersectLineSegments(c_rattribute.v3Position,
                          Vector3(c_rattribute.v3Position.x, c_rattribute.v3Position.y,
                                      c_rattribute.v3Position.z + c_rattribute.fHeight), s.v3LastPosition, s.v3Position,
                          oa, ob);
    return (Vector3::DistanceSquared(oa, ob)) <= (c_rattribute.fRadius + s.fRadius) * (c_rattribute.fRadius + s.fRadius);
}

bool CCylinderCollisionInstance::OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    if (CollideCylinderVSDynamicSphere(m_attribute, s))
    {
        // NOTE : 거리가 가까워 졌을때만.. - [levites]
        if (GetVector3Distance(s.v3Position, m_attribute.v3Position) <
            GetVector3Distance(s.v3LastPosition, m_attribute.v3Position))
            return true;
    }

    // NOTE : 이동 거리가 클 경우 빈틈없이 (원 크기 단위로) 이동하면서 전부 체크 해 본다 - [levites]
    Vector3 v3Distance = s.v3Position - s.v3LastPosition;
    float fDistance = v3Distance.Length();
    if (s.fRadius <= 0.0001f)
        return false;
    if (fDistance >= s.fRadius * 2.0f)
    {
        TCylinderData cylinder;
        cylinder = m_attribute;
        cylinder.v3Position = s.v3LastPosition;

        int iStep = fDistance / s.fRadius * 2.0f;
        Vector3 v3Step = v3Distance / float(iStep);

        for (int i = 0; i < iStep; ++i)
        {
            cylinder.v3Position += v3Step;
            if (CollideCylinderVSDynamicSphere(cylinder, s))
                return true;
        }
    }

    return false;
}

bool CCylinderCollisionInstance::OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    return (CollideCylinderVSDynamicSphere(m_attribute, s));
}

Vector3 CCylinderCollisionInstance::OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    Vector3 v3Position = m_attribute.v3Position;
    v3Position.z = s.v3Position.z;
    if (Vector3::DistanceSquared(s.v3Position, v3Position) >= (s.fRadius + m_attribute.fRadius) * (
            m_attribute.fRadius + s.fRadius))
        return Vector3(0.0f, 0.0f, 0.0f);
    Vector3 c;
    Vector3 advance = s.v3Position - s.v3LastPosition;
    advance.z = 0;
    c = advance.Cross(Vector3::Backward);

    float sum = - c.Dot(s.v3Position - v3Position);
    float mul = (s.fRadius + m_attribute.fRadius) * (s.fRadius + m_attribute.fRadius) - Vector3::DistanceSquared(s.v3Position, v3Position);

    if (sum * sum - 4 * mul <= 0)
        return Vector3(0.0f, 0.0f, 0.0f);
    float sq = sqrt(sum * sum - 4 * mul);
    float t1 = -sum - sq, t2 = -sum + sq;
    t1 *= 0.5f;
    t2 *= 0.5f;

    if (fabs(t1) <= fabs(t2))
    {
        return c * (gc_fReduceMove * t1);
    }
    else
        return c * (gc_fReduceMove * t2);

    /*Vector3 p1 = s.v3Position+t1*c;
    Vector3 p2 = s.v3Position+t2*c;

    if (D3DXVec3LengthSq(&(p2-s.v3Position))>D3DXVec3LengthSq(&(p1-s.v3Position)))
    {
        return p1-s.v3Position;
    }
    else
    {
        return p2-s.v3Position;
    }*/
}

void CCylinderCollisionInstance::Render(D3DFILLMODE d3dFillMode)
{
    static CScreen s;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    s.RenderCylinder(NULL, m_attribute.v3Position.x, m_attribute.v3Position.y,
                     m_attribute.v3Position.z + m_attribute.fHeight / 2, m_attribute.fRadius, m_attribute.fHeight,
                     d3dFillMode);
}

void CCylinderCollisionInstance::OnDestroy()
{
    gs_cci.Free(this);
}

/////////////////////////////////////////////
// AABB (Aligned Axis Bounding Box)
TAABBData &CAABBCollisionInstance::GetAttribute()
{
    return m_attribute;
}

const TAABBData &CAABBCollisionInstance::GetAttribute() const
{
    return m_attribute;
}

bool CAABBCollisionInstance::OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v;
    Vector3 v3center = (m_attribute.v3Min + m_attribute.v3Max) * 0.5f;

    memcpy(&v, &s.v3Position, sizeof(Vector3));

    if (v.x < m_attribute.v3Min.x)
        v.x = m_attribute.v3Min.x;
    if (v.x > m_attribute.v3Max.x)
        v.x = m_attribute.v3Max.x;
    if (v.y < m_attribute.v3Min.y)
        v.x = m_attribute.v3Min.y;
    if (v.y > m_attribute.v3Max.y)
        v.x = m_attribute.v3Max.y;
    if (v.z < m_attribute.v3Min.z)
        v.z = m_attribute.v3Min.z;
    if (v.z > m_attribute.v3Max.z)
        v.z = m_attribute.v3Max.z;

    if (GetVector3Distance(v, s.v3Position) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    memcpy(&v, &s.v3LastPosition, sizeof(Vector3));

    if (v.x < m_attribute.v3Min.x)
        v.x = m_attribute.v3Min.x;
    if (v.x > m_attribute.v3Max.x)
        v.x = m_attribute.v3Max.x;
    if (v.y < m_attribute.v3Min.y)
        v.x = m_attribute.v3Min.y;
    if (v.y > m_attribute.v3Max.y)
        v.x = m_attribute.v3Max.y;
    if (v.z < m_attribute.v3Min.z)
        v.z = m_attribute.v3Min.z;
    if (v.z > m_attribute.v3Max.z)
        v.z = m_attribute.v3Max.z;

    if (GetVector3Distance(v, s.v3LastPosition) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    return false;
}

bool CAABBCollisionInstance::OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v;
    memcpy(&v, &s.v3Position, sizeof(Vector3));

    if (v.x < m_attribute.v3Min.x)
        v.x = m_attribute.v3Min.x;
    if (v.x > m_attribute.v3Max.x)
        v.x = m_attribute.v3Max.x;
    if (v.y < m_attribute.v3Min.y)
        v.x = m_attribute.v3Min.y;
    if (v.y > m_attribute.v3Max.y)
        v.x = m_attribute.v3Max.y;
    if (v.z < m_attribute.v3Min.z)
        v.z = m_attribute.v3Min.z;
    if (v.z > m_attribute.v3Max.z)
        v.z = m_attribute.v3Max.z;

    if (v.x > m_attribute.v3Min.x && v.x < m_attribute.v3Max.x &&
        v.y > m_attribute.v3Min.y && v.y < m_attribute.v3Max.y &&
        v.z > m_attribute.v3Min.z && v.z < m_attribute.v3Max.z)
    {
        return true;
    }

    if (GetVector3Distance(v, s.v3Position) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    memcpy(&v, &s.v3LastPosition, sizeof(Vector3));

    if (v.x < m_attribute.v3Min.x)
        v.x = m_attribute.v3Min.x;
    if (v.x > m_attribute.v3Max.x)
        v.x = m_attribute.v3Max.x;
    if (v.y < m_attribute.v3Min.y)
        v.x = m_attribute.v3Min.y;
    if (v.y > m_attribute.v3Max.y)
        v.x = m_attribute.v3Max.y;
    if (v.z < m_attribute.v3Min.z)
        v.z = m_attribute.v3Min.z;
    if (v.z > m_attribute.v3Max.z)
        v.z = m_attribute.v3Max.z;

    if (v.x > m_attribute.v3Min.x && v.x < m_attribute.v3Max.x &&
        v.y > m_attribute.v3Min.y && v.y < m_attribute.v3Max.y &&
        v.z > m_attribute.v3Min.z && v.z < m_attribute.v3Max.z)
    {
        return true;
    }

    if (GetVector3Distance(v, s.v3LastPosition) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    return false;
}

Vector3 CAABBCollisionInstance::OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    //Tracef("OnGetCollisionMovementAdjust	v3Min.x = %f, v3Max.x = %f\n", m_attribute.v3Min.x, m_attribute.v3Max.x);
    /*
    float fARadius = D3DXVec3Length(&(m_attribute.v3Min - m_attribute.v3Max));
    if (D3DXVec3LengthSq(&(s.v3Position-(m_attribute.v3Max + m_attribute.v3Min)))>=(s.fRadius+fARadius)*(fARadius+s.fRadius))
        return Vector3(0.0f,0.0f,0.0f);
    Vector3 c;
    D3DXVec3Cross(&c, &(s.v3Position-s.v3LastPosition), &Vector3(0.0f,0.0f,1.0f) );

    float sum = - D3DXVec3Dot(&c,&(s.v3Position-(m_attribute.v3Max + m_attribute.v3Min)));
    float mul = (s.fRadius+fARadius)*(s.fRadius+fARadius)-D3DXVec3LengthSq(&(s.v3Position-(m_attribute.v3Max + m_attribute.v3Min)));

    if (sum*sum-4*mul<=0)
        return Vector3(0.0f,0.0f,0.0f);
    float sq = sqrt(sum*sum-4*mul);
    float t1=-sum-sq, t2=-sum+sq;
    t1*=0.5f;
    t2*=0.5f;

    if (fabs(t1)<=fabs(t2))
    {
        return (gc_fReduceMove*t1)*c;
    }
    else
        return (gc_fReduceMove*t2)*c;
    */

    Vector3 v3Temp;
    if (s.v3Position.x + s.fRadius <= m_attribute.v3Min.x)
    {
        v3Temp.x = m_attribute.v3Min.x;
    }
    else if (s.v3Position.x - s.fRadius >= m_attribute.v3Max.x)
    {
        v3Temp.x = m_attribute.v3Max.x;
    }
    else if (s.v3Position.x + s.fRadius >= m_attribute.v3Min.x && s.v3Position.x + s.fRadius <= m_attribute.v3Max.x)
    {
        v3Temp.x = s.v3Position.x + s.fRadius;
    }
    else
    {
        v3Temp.x = s.v3Position.x - s.fRadius;
    }

    if (s.v3Position.y + s.fRadius <= m_attribute.v3Min.y)
    {
        v3Temp.y = m_attribute.v3Min.y;
    }
    else if (s.v3Position.y - s.fRadius >= m_attribute.v3Max.y)
    {
        v3Temp.y = m_attribute.v3Max.y;
    }
    else if (s.v3Position.y + s.fRadius >= m_attribute.v3Min.y && s.v3Position.y + s.fRadius <= m_attribute.v3Max.y)
    {
        v3Temp.y = s.v3Position.y + s.fRadius;
    }
    else
    {
        v3Temp.y = s.v3Position.y - s.fRadius;
    }

    if (s.v3Position.z + s.fRadius <= m_attribute.v3Min.z)
    {
        v3Temp.z = m_attribute.v3Min.z;
    }
    else if (s.v3Position.z - s.fRadius >= m_attribute.v3Max.z)
    {
        v3Temp.z = m_attribute.v3Max.z;
    }
    else if (s.v3Position.z + s.fRadius >= m_attribute.v3Min.z && s.v3Position.z + s.fRadius <= m_attribute.v3Max.z)
    {
        v3Temp.z = s.v3Position.z + s.fRadius;
    }
    else
    {
        v3Temp.z = s.v3Position.z - s.fRadius;
    }

    if (Vector3::DistanceSquared(v3Temp, s.v3Position) < s.fRadius * s.fRadius)
        return Vector3::Zero;

    return Vector3::Zero;
}

void CAABBCollisionInstance::Render(D3DFILLMODE d3dFillMode)
{
    static CScreen s;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    s.RenderCube(m_attribute.v3Min.x, m_attribute.v3Min.y, m_attribute.v3Min.z, m_attribute.v3Max.x,
                 m_attribute.v3Max.y, m_attribute.v3Max.z);
    return;
}

void CAABBCollisionInstance::OnDestroy()
{
    gs_aci.Free(this);
}

/////////////////////////////////////////////
// OBB

TOBBData &COBBCollisionInstance::GetAttribute()
{
    return m_attribute;
}

const TOBBData &COBBCollisionInstance::GetAttribute() const
{
    return m_attribute;
}

bool COBBCollisionInstance::OnMovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v3Center = (m_attribute.v3Min + m_attribute.v3Max) * 0.5f;
    Vector3 v3Sphere = s.v3Position - v3Center;
    v3Sphere = Vector3::Transform(v3Sphere, m_attribute.matRot);
    v3Sphere = v3Sphere + v3Center;

    Vector3 v3Point = v3Sphere;
    if (v3Point.x < m_attribute.v3Min.x)
    {
        v3Point.x = m_attribute.v3Min.x;
    }
    if (v3Point.x > m_attribute.v3Max.x)
    {
        v3Point.x = m_attribute.v3Max.x;
    }
    if (v3Point.y < m_attribute.v3Min.y)
    {
        v3Point.y = m_attribute.v3Min.y;
    }
    if (v3Point.y > m_attribute.v3Max.y)
    {
        v3Point.y = m_attribute.v3Max.y;
    }
    if (v3Point.z < m_attribute.v3Min.z)
    {
        v3Point.z = m_attribute.v3Min.z;
    }
    if (v3Point.z > m_attribute.v3Max.z)
    {
        v3Point.z = m_attribute.v3Max.z;
    }

    if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    v3Sphere = s.v3LastPosition - v3Center;
    v3Sphere = Vector3::Transform(v3Sphere, m_attribute.matRot);
    v3Sphere = v3Sphere + v3Center;

    v3Point = v3Sphere;
    if (v3Point.x < m_attribute.v3Min.x)
    {
        v3Point.x = m_attribute.v3Min.x;
    }
    if (v3Point.x > m_attribute.v3Max.x)
    {
        v3Point.x = m_attribute.v3Max.x;
    }
    if (v3Point.y < m_attribute.v3Min.y)
    {
        v3Point.y = m_attribute.v3Min.y;
    }
    if (v3Point.y > m_attribute.v3Max.y)
    {
        v3Point.y = m_attribute.v3Max.y;
    }
    if (v3Point.z < m_attribute.v3Min.z)
    {
        v3Point.z = m_attribute.v3Min.z;
    }
    if (v3Point.z > m_attribute.v3Max.z)
    {
        v3Point.z = m_attribute.v3Max.z;
    }

    if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    return false;
}

bool COBBCollisionInstance::OnCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    Vector3 v3Center = (m_attribute.v3Min + m_attribute.v3Max) * 0.5f;
    Vector3 v3Sphere = s.v3Position - v3Center;
    v3Sphere = Vector3::Transform(v3Sphere, m_attribute.matRot);
    v3Sphere = v3Sphere + v3Center;

    Vector3 v3Point = v3Sphere;
    if (v3Point.x < m_attribute.v3Min.x)
    {
        v3Point.x = m_attribute.v3Min.x;
    }
    if (v3Point.x > m_attribute.v3Max.x)
    {
        v3Point.x = m_attribute.v3Max.x;
    }
    if (v3Point.y < m_attribute.v3Min.y)
    {
        v3Point.y = m_attribute.v3Min.y;
    }
    if (v3Point.y > m_attribute.v3Max.y)
    {
        v3Point.y = m_attribute.v3Max.y;
    }
    if (v3Point.z < m_attribute.v3Min.z)
    {
        v3Point.z = m_attribute.v3Min.z;
    }
    if (v3Point.z > m_attribute.v3Max.z)
    {
        v3Point.z = m_attribute.v3Max.z;
    }

    if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    v3Sphere = s.v3LastPosition - v3Center;
    v3Sphere = Vector3::Transform(v3Sphere, m_attribute.matRot);
    v3Sphere = v3Sphere + v3Center;

    v3Point = v3Sphere;
    if (v3Point.x < m_attribute.v3Min.x)
    {
        v3Point.x = m_attribute.v3Min.x;
    }
    if (v3Point.x > m_attribute.v3Max.x)
    {
        v3Point.x = m_attribute.v3Max.x;
    }
    if (v3Point.y < m_attribute.v3Min.y)
    {
        v3Point.y = m_attribute.v3Min.y;
    }
    if (v3Point.y > m_attribute.v3Max.y)
    {
        v3Point.y = m_attribute.v3Max.y;
    }
    if (v3Point.z < m_attribute.v3Min.z)
    {
        v3Point.z = m_attribute.v3Min.z;
    }
    if (v3Point.z > m_attribute.v3Max.z)
    {
        v3Point.z = m_attribute.v3Max.z;
    }

    if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius)
    {
        return true;
    }

    return false;
}

Vector3 COBBCollisionInstance::OnGetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    return Vector3(.0f, .0f, .0f);
}

void COBBCollisionInstance::Render(D3DFILLMODE d3dFillMode)
{
    static CScreen s;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    s.RenderCube(m_attribute.v3Min.x, m_attribute.v3Min.y, m_attribute.v3Min.z, m_attribute.v3Max.x,
                 m_attribute.v3Max.y, m_attribute.v3Max.z, m_attribute.matRot);
    return;
}

void COBBCollisionInstance::OnDestroy()
{
    gs_oci.Free(this);
}
