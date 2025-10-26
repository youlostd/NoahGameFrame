#include "StdAfx.h"
#include "GrpObjectInstance.h"
#include "CullingManager.h"

#include "../EterBase/Timer.h"
#include "base/Remotery.h"

void CGraphicObjectInstance::OnInitialize()
{
    ZeroMemory(m_abyPortalID, sizeof(m_abyPortalID));
}

void CGraphicObjectInstance::Clear()
{
    if (m_CullingHandle)
    {
        CCullingManager::Instance().Unregister(this);
        m_CullingHandle = nullptr;
    }

    ClearHeightInstance();

    m_isVisible = true;
    m_alwaysRender = false;
    m_isForceHide = false;
    m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;
    m_v3Scale.x = m_v3Scale.y = m_v3Scale.z = 1.0f;
    //m_fRotation = 0.0f;
    m_fYaw = m_fPitch = m_fRoll = 0.0f;

    m_v3ScaleAttachment.x = m_v3ScaleAttachment.y = m_v3ScaleAttachment.z = 1.0f;

    m_worldMatrix = Matrix::Identity;
    m_mRotation = Matrix::Identity;
    m_matAttachedWorldTransform = Matrix::Identity;
    m_matScaleTransformation = Matrix::Identity;
    m_matScale = Matrix::Identity;

    ZeroMemory(m_abyPortalID, sizeof(m_abyPortalID));

    OnClear();
}

bool CGraphicObjectInstance::Render()
{
    if (!isShow())
        return false;

    rmt_ScopedCPUSample(ObjectInstanceRender, RMTSF_Aggregate);

    OnRender();
    return true;
}

void CGraphicObjectInstance::BlendRender()
{
    if (!isShow())
        return;

    OnBlendRender();
}

void CGraphicObjectInstance::RenderToShadowMap(bool showWeapon)
{
    if (!isShow())
        return;

    OnRenderToShadowMap(showWeapon);
}

void CGraphicObjectInstance::RenderShadow()
{
    if (!isShow())
        return;

    OnRenderShadow();
}

void CGraphicObjectInstance::RenderPCBlocker()
{
    if (!isShow())
        return;

    OnRenderPCBlocker();
}

void CGraphicObjectInstance::Update()
{
    OnUpdate();

    UpdateBoundingSphere();
}

void CGraphicObjectInstance::Deform()
{
    if (!isShow())
        return;

    rmt_ScopedCPUSample(ObjectInstanceDeform, RMTSF_Aggregate);

    OnDeform();
}

void CGraphicObjectInstance::Transform()
{
    Matrix mat = m_matScale * m_mRotation;
    m_worldMatrix = mat;

    m_worldMatrix._41 += m_v3Position.x;
    m_worldMatrix._42 += m_v3Position.y;
    m_worldMatrix._43 += m_v3Position.z;
}

const Vector3 &CGraphicObjectInstance::GetPosition() const
{
    return m_v3Position;
}

const Vector3 &CGraphicObjectInstance::GetScale() const
{
    return m_v3Scale;
}

const Vector3 &CGraphicObjectInstance::GetAttachmentScale() const
{
    return m_v3ScaleAttachment;
}

float CGraphicObjectInstance::GetRotation() const
{
    return GetRoll();
}

float CGraphicObjectInstance::GetYaw() const
{
    return m_fYaw;
}

float CGraphicObjectInstance::GetPitch() const
{
    return m_fPitch;
}

float CGraphicObjectInstance::GetRoll() const
{
    return m_fRoll;
}

Matrix &CGraphicObjectInstance::GetTransform()
{
    return m_worldMatrix;
}

void CGraphicObjectInstance::SetRotationQuaternion(const Quaternion &q)
{
    m_mRotation = Matrix::CreateFromQuaternion(q);
}

void CGraphicObjectInstance::SetRotationMatrix(const Matrix &m)
{
    m_mRotation = m;
}

void CGraphicObjectInstance::SetRotation(float fRotation)
{
    m_fYaw = 0;
    m_fPitch = 0;
    m_fRoll = fRotation;

    m_mRotation = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(fRotation));
}

void CGraphicObjectInstance::SetRotation(float fYaw, float fPitch, float fRoll)
{
    //m_fRotation = fRotation;
    m_fYaw = fYaw;
    m_fPitch = fPitch;
    m_fRoll = fRoll;

    m_mRotation = Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(fYaw), DirectX::XMConvertToRadians(fPitch), DirectX::XMConvertToRadians(fRoll));
}

void CGraphicObjectInstance::SetPosition(float x, float y, float z)
{
    m_v3Position.x = x;
    m_v3Position.y = y;
    m_v3Position.z = z;
}

void CGraphicObjectInstance::SetPosition(const Vector3 &newposition)
{
    m_v3Position = newposition;
}

void CGraphicObjectInstance::SetScale(float x, float y, float z)
{
    m_v3Scale.x = x;
    m_v3Scale.y = y;
    m_v3Scale.z = z;
    m_matScale = Matrix::CreateScale(m_v3Scale);
}

void CGraphicObjectInstance::SetAttachmentScale(float x, float y, float z)
{
    m_v3ScaleAttachment.x = x;
    m_v3ScaleAttachment.y = y;
    m_v3ScaleAttachment.z = z;
}

void CGraphicObjectInstance::SetAttachmentScale(const Vector3 &scale)
{
    m_v3ScaleAttachment = scale;
}

void CGraphicObjectInstance::Show()
{
    m_isVisible = true;
}

void CGraphicObjectInstance::Hide()
{
    m_isVisible = false;
}

bool CGraphicObjectInstance::isShow() const
{
    if (m_alwaysRender)
        return true;

    if (m_isForceHide)
        return false;

    return m_isVisible;
}

void CGraphicObjectInstance::SetAlwaysRender(bool val)
{
    m_alwaysRender = val;
}

bool CGraphicObjectInstance::IsAlwaysRender() const
{
    return m_alwaysRender;
}

void CGraphicObjectInstance::EnableForceHide()
{
    m_isForceHide = true;
}

void CGraphicObjectInstance::DisableForceHide()
{
    m_isForceHide = false;
}

bool CGraphicObjectInstance::isForceHide() const
{
    return m_isForceHide;
}

// 

//////////////////////////////////////////////////////////////////////////

Vector4 &CGraphicObjectInstance::GetWTBBoxVertex(const unsigned char &c_rucNumTBBoxVertex)
{
    return m_v4TBBox[c_rucNumTBBoxVertex];
}

bool CGraphicObjectInstance::isIntersect(const CRay &c_rRay, float *pu, float *pv, float *pt)
{
    Vector3 v3Start, v3Dir;
    float fRayRange;
    c_rRay.GetStartPoint(&v3Start);
    c_rRay.GetDirection(&v3Dir, &fRayRange);

    TPosition posVertices[8];

    posVertices[0] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMin.y, m_v3TBBoxMin.z);
    posVertices[1] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMin.y, m_v3TBBoxMin.z);
    posVertices[2] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMax.y, m_v3TBBoxMin.z);
    posVertices[3] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMax.y, m_v3TBBoxMin.z);
    posVertices[4] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMin.y, m_v3TBBoxMax.z);
    posVertices[5] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMin.y, m_v3TBBoxMax.z);
    posVertices[6] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMax.y, m_v3TBBoxMax.z);
    posVertices[7] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMax.y, m_v3TBBoxMax.z);

    TIndex Indices[36] = {0, 1, 2, 1, 3, 2,
                          2, 0, 6, 0, 4, 6,
                          0, 1, 4, 1, 5, 4,
                          1, 3, 5, 3, 7, 5,
                          3, 2, 7, 2, 6, 7,
                          4, 5, 6, 5, 7, 6};

    int triCount = 12;
    uint16_t *pcurIdx = (uint16_t *)Indices;

    while (triCount--)
    {
        if (IntersectTriangle(v3Start, v3Dir,
                              posVertices[pcurIdx[0]],
                              posVertices[pcurIdx[1]],
                              posVertices[pcurIdx[2]],
                              pu, pv, pt))
        {
            return true;
        }

        pcurIdx += 3;
    }

    return false;
}

CGraphicObjectInstance::CGraphicObjectInstance()
{
    m_CullingHandle = 0;
    Initialize();
}

void CGraphicObjectInstance::Initialize()
{
    if (m_CullingHandle)
        CCullingManager::Instance().Unregister(this);
    m_CullingHandle = 0;

    m_pHeightAttributeInstance = NULL;

    m_isVisible = true;

    m_BlockCamera = false;

    m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;
    m_v3Scale.x = m_v3Scale.y = m_v3Scale.z = 1.0f;
    m_fYaw = m_fPitch = m_fRoll = 0.0f;
    m_v3ScaleAttachment.x = m_v3ScaleAttachment.y = m_v3ScaleAttachment.z = 1.0f;

    m_worldMatrix = Matrix::Identity;
    m_mRotation = Matrix::Identity;
    m_matAttachedWorldTransform = Matrix::Identity;
    m_matScaleTransformation = Matrix::Identity;
    m_matScale = Matrix::Identity;

    OnInitialize();
}

CGraphicObjectInstance::~CGraphicObjectInstance()
{
    Initialize();
}

void CGraphicObjectInstance::UpdateBoundingSphere()
{
    if (m_CullingHandle)
    {
        Vector3d center;
        float radius;
        GetBoundingSphere(center, radius);
        if (radius != m_CullingHandle->GetRadius())
            m_CullingHandle->NewPosRadius(center, radius);
        else
            m_CullingHandle->NewPos(center);
    }
}

void CGraphicObjectInstance::RegisterBoundingSphere()
{
    if (m_CullingHandle)
        CCullingManager::Instance().Unregister(this);

    m_CullingHandle = CCullingManager::Instance().Register(this);
}

void CGraphicObjectInstance::AddCollision(const CStaticCollisionData *pscd, const Matrix *pMat)
{
    m_StaticCollisionInstanceVector.push_back(CBaseCollisionInstance::BuildCollisionInstance(pscd, pMat));
}

void CGraphicObjectInstance::ClearCollision()
{
    CCollisionInstanceVector::iterator it;
    for (it = m_StaticCollisionInstanceVector.begin(); it != m_StaticCollisionInstanceVector.end(); ++it)
    {
        (*it)->Destroy();
    }
    m_StaticCollisionInstanceVector.clear();
}

bool CGraphicObjectInstance::CollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    CCollisionInstanceVector::const_iterator it;
    for (it = m_StaticCollisionInstanceVector.begin(); it != m_StaticCollisionInstanceVector.end(); ++it)
    {
        if ((*it)->CollisionDynamicSphere(s))
            return true;
    }
    return false;
}

bool CGraphicObjectInstance::MovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const
{
    CCollisionInstanceVector::const_iterator it;
    for (it = m_StaticCollisionInstanceVector.begin(); it != m_StaticCollisionInstanceVector.end(); ++it)
    {
        if ((*it)->MovementCollisionDynamicSphere(s))
            return true;
    }
    return false;
}

Vector3 CGraphicObjectInstance::GetCollisionMovementAdjust(const CDynamicSphereInstance &s) const
{
    CCollisionInstanceVector::const_iterator it;
    for (it = m_StaticCollisionInstanceVector.begin(); it != m_StaticCollisionInstanceVector.end(); ++it)
    {
        if ((*it)->MovementCollisionDynamicSphere(s))
            return (*it)->GetCollisionMovementAdjust(s);
    }

    return Vector3(0.0f, 0.0f, 0.0f);
}

void CGraphicObjectInstance::UpdateCollisionData(const CStaticCollisionDataVector *pscdVector)
{
    ClearCollision();
    OnUpdateCollisionData(pscdVector);
}

uint32_t CGraphicObjectInstance::GetCollisionInstanceCount()
{
    return m_StaticCollisionInstanceVector.size();
}

CBaseCollisionInstance *CGraphicObjectInstance::GetCollisionInstanceData(uint32_t dwIndex)
{
    if (dwIndex > m_StaticCollisionInstanceVector.size())
    {
        return 0;
    }
    return m_StaticCollisionInstanceVector[dwIndex];
}

//////////////////////////////////////////////////////////////////////////
// Height

void CGraphicObjectInstance::SetHeightInstance(CAttributeInstance *pAttributeInstance)
{
    m_pHeightAttributeInstance = pAttributeInstance;
}

void CGraphicObjectInstance::ClearHeightInstance()
{
    m_pHeightAttributeInstance = NULL;
}

void CGraphicObjectInstance::UpdateHeightInstance(CAttributeInstance *pAttributeInstance)
{
    ClearHeightInstance();
    OnUpdateHeighInstance(pAttributeInstance);
}

bool CGraphicObjectInstance::IsObjectHeight() const
{
    if (m_pHeightAttributeInstance)
        return true;

    return false;
}

bool CGraphicObjectInstance::GetObjectHeight(float fX, float fY, float *pfHeight)
{
    if (!m_pHeightAttributeInstance)
        return false;

    return OnGetObjectHeight(fX, fY, pfHeight);
}

void CGraphicObjectInstance::SetPortal(uint32_t dwIndex, int iID)
{
    if (dwIndex >= PORTAL_ID_MAX_NUM)
    {
        assert(dwIndex < PORTAL_ID_MAX_NUM);
        return;
    }

    m_abyPortalID[dwIndex] = iID;
}

int CGraphicObjectInstance::GetPortal(uint32_t dwIndex)
{
    if (dwIndex >= PORTAL_ID_MAX_NUM)
    {
        assert(dwIndex < PORTAL_ID_MAX_NUM);
        return 0;
    }

    return m_abyPortalID[dwIndex];
}
