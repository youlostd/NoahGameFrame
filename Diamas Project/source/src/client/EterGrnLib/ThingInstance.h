#ifndef METIN2_CLIENT_ETERGRNLIB_THINGINSTANCE_HPP
#define METIN2_CLIENT_ETERGRNLIB_THINGINSTANCE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../Eterlib/GrpObjectInstance.h"
#include "../EterLib/GrpImage.h"

#include "Thing.h"

#include <unordered_map>
#include <vector>

class CGrannyModelInstance;
struct SMaterialData;

class CGraphicThingInstance : public CGraphicObjectInstance
{
public:
    enum
    {
        ID = THING_OBJECT
    };

    int GetType() const override
    {
        return ID;
    }

    CGraphicThingInstance();
    ~CGraphicThingInstance() override;

    void DeformNoSkin();
    void TransformAttachment();

    void UpdateLODLevel();
    void UpdateTime();

    bool LessRenderOrder(CGraphicThingInstance *pkThingInst);

    bool Picking(const Vector3 &v, const Vector3 &dir, float &out_x, float &out_y);

    void OnInitialize() override;

    bool CreateDeviceObjects();
    void DestroyDeviceObjects();

    void ReserveModelInstance(int iCount);
    void ReserveModelThing(int iCount);

    bool CheckModelInstanceIndex(int iModelInstance);
    bool CheckModelThingIndex(int iModelThing);
    bool IsMotionThing();

    void RegisterModelThing(int iModelThing, CGraphicThing::Ptr pModelThing);

    static void RegisterMotionThing(uint32_t motionId, uint32_t dwMotionKey, CGraphicThing::Ptr pMotionThing);
    static bool CheckMotionThingIndex(uint32_t dwMotionKey, uint32_t motionId);
    static bool GetMotionThingPointer(uint32_t motionId, uint32_t dwKey, CGraphicThing::Ptr *ppMotion);

    bool SetModelInstance(int iDstModelInstance,
                          int iSrcModelThing, int iSrcModel,
                          int skeletonModelInstance = -1);

    bool AttachModelInstance(int iDstModelInstance,
                             CGraphicThingInstance &rSrcInstance,
                             int iSrcModelInstance,
                             int boneIndex);

    void DetachModelInstance(int iDstModelInstance,
                             CGraphicThingInstance &rSrcInstance,
                             int iSrcModelInstance);

    bool FindBoneIndex(int iModelInstance, const char *c_szBoneName, int *iRetBone);
    bool GetBoneMatrixPointer(int iModelInstance, const char *c_szBoneName, float **boneMatrix);
    bool GetBoneMatrixPointer(int iModelInstance, const char *c_szBoneName, Matrix **boneMatrix);
    bool GetBonePosition(int iModelIndex, int iBoneIndex, float *pfx, float *pfy, float *pfz);

    void ResetLocalTime();
    void InsertDelay(float fDelay);

    void SetMaterialImagePointer(UINT ePart, const char *c_szImageName, CGraphicImage::Ptr pImage);
    void SetMaterialData(UINT ePart, const char *c_szImageName, const SMaterialData &kMaterialData);
    void SetSpecularInfo(UINT ePart, const char *c_szMtrlName, bool bEnable, float fPower);

    void __SetLocalTime(float fLocalTime); // Only Used by Tools
    float GetLastLocalTime() const;
    float GetLocalTime() const;
    float GetSecondElapsed() const;
    float GetAverageSecondElapsed() const;

    float GetHeight();

    void RenderWithOneTexture(bool showWeapon);
    void RenderWithTwoTexture();
    void BlendRenderWithOneTexture();
    void BlendRenderWithTwoTexture();

    uint32_t GetModelInstanceCount() const;
    CGrannyModelInstance *GetModelInstancePointer(uint32_t dwModelIndex) const;

    void ReloadTexture();

    CGraphicThing::Ptr GetBaseThingPtr();

    bool SetMotion(uint32_t dwMotionKey, float blendTime = 0.0f, int loopCount = 0, float speedRatio = 1.0f);
    bool ChangeMotion(uint32_t dwMotionKey, int loopCount = 0, float speedRatio = 1.0f);

    void SetMotionAtEnd();

    void SetEndStopMotion()
    {
        SetMotionAtEnd();
    }

    uint32_t GetLoopIndex(uint32_t modelInstance);
    void PrintControls();

    bool Intersect(float *pu, float *pv, float *pt);
    void GetBoundBox(Vector3 *vtMin, Vector3 *vtMax);
    bool GetBoundBox(uint32_t dwModelInstanceIndex, Vector3 *vtMin, Vector3 *vtMax);
    bool GetBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, Matrix **ppMatrix);
    bool GetCompositeBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, Matrix **ppMatrix);
    void UpdateTransform(Matrix *pMatrix, double fSecondsElapsed = 0.0, uint64_t iModelInstanceIndex = 0);

    void BuildBoundingSphere();
    void BuildBoundingAABB();
    virtual void CalculateBBox();
    bool GetBoundingSphere(Vector3 &v3Center, float &fRadius) override;
    virtual bool GetBoundingAABB(Vector3 &v3Min, Vector3 &v3Max);

    bool HaveBlendThing();

    void SetMotionID(DWORD motionID)
    {
        m_dwMotionID = motionID;
    }

    DWORD GetMotionID()
    {
        return m_dwMotionID;
    }

protected:
    void OnClear() override;
    void AcceTransform();

    void OnDeform() override;
    void OnUpdate() override;
    void OnRender() override;
    void OnBlendRender() override;
    void OnRenderToShadowMap(bool showWeapon) override;
    void OnRenderShadow() override;
    void OnRenderPCBlocker() override;

    void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector) override;
    void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance) override;
    bool OnGetObjectHeight(float fX, float fY, float *pfHeight) override;

    bool m_bUpdated;

    float m_fLastLocalTime;
    float m_fLocalTime;
    float m_fDelay;
    float m_fSecondElapsed;
    float m_fAverageSecondElapsed;
    float m_fRadius;
    float m_allowedError;

    Vector3 m_v3Center;
    Vector3 m_v3Min, m_v3Max;
    DWORD m_dwMotionID;

    std::vector<CGrannyModelInstance *> m_modelInstances;
    std::vector<CGraphicThing::Ptr> m_modelThings;
    static std::unordered_map<uint32_t, std::unordered_map<uint32_t, CGraphicThing::Ptr>> m_roMotionThingMap;

public:
    static void CreateSystem(UINT uCapacity);
    static void DestroySystem();

    static CGraphicThingInstance *New();
    static void Delete(CGraphicThingInstance *pkThingInst);

    static CDynamicPool<CGraphicThingInstance> ms_kPool;
};

#endif
