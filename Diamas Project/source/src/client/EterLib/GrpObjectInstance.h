#ifndef METIN2_CLIENT_ETERLIB_GRPOBJECTINSTANCE_HPP
#define METIN2_CLIENT_ETERLIB_GRPOBJECTINSTANCE_HPP

#pragma once

#include "GrpScreen.h"
#include "CollisionData.h"
#include "AttributeInstance.h"

class SpherePack;

enum
{
    THING_OBJECT = 0xadf21f13,
    TREE_OBJECT = 0x8ac9f7a6,
    ACTOR_OBJECT = 0x29a76c24,
    EFFECT_OBJECT = 0x1cfa97c6,
    DUNGEON_OBJECT = 0x18326035,
};

enum
{
    PORTAL_ID_MAX_NUM = 8,
};

class CGraphicObjectInstance : public CGraphicCollisionObject
{
public:
    CGraphicObjectInstance();
    virtual ~CGraphicObjectInstance();
    virtual int GetType() const = 0;

public:
    const Vector3 &GetPosition() const;
    const Vector3 &GetScale() const;
    const Vector3 &GetAttachmentScale() const;
    float GetRotation() const;
    float GetYaw() const;
    float GetPitch() const;
    float GetRoll() const;

    void SetPosition(float x, float y, float z);
    void SetPosition(const Vector3 &newposition);
    void SetScale(float x, float y, float z);
    void SetRotation(float fRotation);
    void SetRotation(float fYaw, float fPitch, float fRoll);
    void SetRotationQuaternion(const Quaternion &q);
    void SetRotationMatrix(const Matrix &m);
    void SetAttachmentScale(float x, float y, float z);
    void SetAttachmentScale(const Vector3 &newscale);

    void Clear();
    void Update();
    bool Render();
    void BlendRender();
    void RenderToShadowMap(bool showWeapon = true);
    void RenderShadow();
    void RenderPCBlocker();
    void Deform();
    void Transform();

    void Show();
    void Hide();
    bool isShow() const;
    virtual void SetAlwaysRender(bool val);
    bool IsAlwaysRender() const;
    void EnableForceHide();
    void DisableForceHide();
    bool isForceHide() const;

    // Camera Block
    void BlockCamera(bool bBlock)
    {
        m_BlockCamera = bBlock;
    }

    bool BlockCamera() const
    {
        return m_BlockCamera;
    }

    // Ray Test
    bool isIntersect(const CRay &c_rRay, float *pu, float *pv, float *pt);

    // Bounding Box
    Vector4 &GetWTBBoxVertex(const unsigned char &c_rucNumTBBoxVertex);

    Vector3 &GetTBBoxMin()
    {
        return m_v3TBBoxMin;
    }

    Vector3 &GetTBBoxMax()
    {
        return m_v3TBBoxMax;
    }

    Vector3 &GetBBoxMin()
    {
        return m_v3BBoxMin;
    }

    Vector3 &GetBBoxMax()
    {
        return m_v3BBoxMax;
    }

    // Matrix
    Matrix &GetTransform();

    const Matrix &GetWorldMatrix() const
    {
        return m_worldMatrix;
    }

    // Portal
    void SetPortal(uint32_t dwIndex, int iID);
    int GetPortal(uint32_t dwIndex);

    // Initialize
    void Initialize();
    virtual void OnInitialize();

    // Bounding Sphere
public:
    void UpdateBoundingSphere();
    void RegisterBoundingSphere();
    SpherePack * GetCullingHandle() { return m_CullingHandle; }
    virtual bool GetBoundingSphere(Vector3 &v3Center, float &fRadius) = 0;

    virtual void OnRender() = 0;
    virtual void OnBlendRender() = 0;
    virtual void OnRenderToShadowMap(bool showWeapon) = 0;
    virtual void OnRenderShadow() = 0;
    virtual void OnRenderPCBlocker() = 0;

    virtual void OnClear()
    {
    }

    virtual void OnUpdate()
    {
    }

    virtual void OnDeform()
    {
    }

protected:
    Vector3 m_v3Position;
    Vector3 m_v3Scale;

    float m_fYaw;
    float m_fPitch;
    float m_fRoll;

    Matrix m_mRotation;

    bool m_isVisible = true;
    bool m_alwaysRender = false;
    bool m_isForceHide = false;
    Matrix m_worldMatrix;

    // Camera Block
    bool m_BlockCamera;

    // Bounding Box
    Vector4 m_v4TBBox[8];
    Vector3 m_v3TBBoxMin, m_v3TBBoxMax;
    Vector3 m_v3BBoxMin, m_v3BBoxMax;

    // Portal
    uint8_t m_abyPortalID[PORTAL_ID_MAX_NUM];
    Vector3 m_v3ScaleAttachment = {1.0f, 1.0f, 1.0f};
    Matrix m_matAttachedWorldTransform;
    Matrix m_matScaleTransformation;
    Matrix m_matScale;

    // Culling
    SpherePack *m_CullingHandle;

    // Static Collision Data
public:
    void AddCollision(const CStaticCollisionData *pscd, const Matrix *pMat);
    void ClearCollision();
    bool CollisionDynamicSphere(const CDynamicSphereInstance &s) const;
    bool MovementCollisionDynamicSphere(const CDynamicSphereInstance &s) const;
    Vector3 GetCollisionMovementAdjust(const CDynamicSphereInstance &s) const;

    void UpdateCollisionData(const CStaticCollisionDataVector *pscdVector = 0);

protected:
    CCollisionInstanceVector m_StaticCollisionInstanceVector;
    virtual void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector) = 0;

    // using in WorldEditor
public:
    uint32_t GetCollisionInstanceCount();
    CBaseCollisionInstance *GetCollisionInstanceData(uint32_t dwIndex);

    // Height Data
public:
    void SetHeightInstance(CAttributeInstance *pAttributeInstance);
    void ClearHeightInstance();

    void UpdateHeightInstance(CAttributeInstance *pAttributeInstance = 0);

    bool IsObjectHeight() const;
    bool GetObjectHeight(float fX, float fY, float *pfHeight);

protected:
    CAttributeInstance *m_pHeightAttributeInstance;
    virtual void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance) = 0;
    virtual bool OnGetObjectHeight(float fX, float fY, float *pfHeight) = 0;
};

#endif
