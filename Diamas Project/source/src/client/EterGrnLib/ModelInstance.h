#ifndef METIN2_CLIENT_ETERGRNLIB_MODELINSTANCE_HPP
#define METIN2_CLIENT_ETERGRNLIB_MODELINSTANCE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterlib/GrpImage.h"
#include "../eterlib/GrpCollisionObject.h"

#include "Model.h"
#include "Motion.h"

class CGrannyModelInstance : public CGraphicCollisionObject
{
public:
    static void DestroySystem();

    static CGrannyModelInstance *New();
    static void Delete(CGrannyModelInstance *pkInst);

    static CDynamicPool<CGrannyModelInstance> ms_kPool;

public:
    CGrannyModelInstance();
    virtual ~CGrannyModelInstance();

    bool IsEmpty();
    void Clear();

    bool CreateDeviceObjects();
    void DestroyDeviceObjects();

    // Update & Render
    void AdvanceTime(double elapsed);
    void UpdateTransform(Matrix *pMatrix, double fSecondsElapsed);

    void SetWorldTransform(const Matrix &transform);

    void UpdateSkeleton(const Matrix *c_pWorldMatrix, float allowedError);
    void DeformNoSkin(const Matrix *c_pWorldMatrix, float allowedError);
    void Deform(const Matrix *c_pWorldMatrix, float allowedError);

    // FIXME : 현재는 하드웨어의 한계로 2장의 텍스춰로 제한이 되어있는 상태이기에 이런
    //         불안정한 아키텍춰가 가능하지만, 궁극적인 방향은 (모델 텍스춰 전부) + (효과용 텍스춰)
    //         이런식의 자동 셋팅이 이뤄져야 되지 않나 생각합니다. - [levites]
    // NOTE : 내부에 if문을 포함 시키기 보다는 조금은 번거롭지만 이렇게 함수 콜 자체를 분리
    //        시키는 것이 퍼포먼스 적인 측면에서는 더 나은 것 같습니다. - [levites]
    // NOTE : 건물은 무조건 OneTexture. 캐릭터는 경우에 따라 TwoTexture.
    void RenderWithOneTexture();
    void RenderWithTwoTexture();
    void BlendRenderWithOneTexture();
    void BlendRenderWithTwoTexture();
    void RenderWithoutTexture();

    // Model
    CGrannyModel *GetModel();

    void SetMaterialImagePointer(const char *c_szImageName,
                                 CGraphicImage::Ptr pImage);

    void SetMaterialData(const char *c_szImageName,
                         const SMaterialData &c_rkMaterialData);

    void SetSpecularInfo(const char *c_szMtrlName,
                         bool bEnable, float fPower);

    void SetLinkedModelPointer(CGrannyModel *model,
                               CGrannyModelInstance *skeletonInstance);


    // Motion
    void SetMotionPointer(const CGrannyMotion *pMotion,
                          float blendTime = 0.0f,
                          int loopCount = 0,
                          float speedRatio = 1.0f);

    void ChangeMotionPointer(const CGrannyMotion *pMotion,
                             int loopCount = 0,
                             float speedRatio = 1.0f);

    void SetMotionAtEnd();
    bool IsMotionPlaying();
    uint32_t GetLoopIndex();

    void PrintControls();

    // Time
    void SetLocalTime(double fLocalTime);
    void ResetLocalTime();
    double GetLocalTime() const;
    double GetNextTime() const;

    // Bone & Attaching
    const float *GetBoneMatrixPointer(int iBone) const;
    const float *GetCompositeBoneMatrixPointer(int iBone) const;
    bool GetMeshMatrixPointer(int iMesh, const Matrix **c_ppMatrix) const;
    bool GetBoneIndexByName(const char *c_szBoneName, int *pBoneIndex) const;

    void SetParentModelInstance(const CGrannyModelInstance *parent, int iBone);

    // Collision Detection
    bool Intersect(const Matrix *c_pMatrix,
                   float *pu, float *pv, float *pt, Vector3 *scale = nullptr);

    void MakeBoundBox(TBoundBox *pBoundBox, const float *mat,
                      const float *OBBMin, const float *OBBMax,
                      Vector3 *vtMin, Vector3 *vtMax);

    void GetBoundBox(Vector3 *vtMin, Vector3 *vtMax, Vector3 *scale = nullptr);

    // Reload Texture
    void ReloadTexture();

    bool HaveBlendThing()
    {
        return m_model->HaveBlendThing();
    }

    const granny_model_instance *GetModelInstance() const
    {
        return m_modelInstance;
    }

protected:
    void __Initialize();

    void __DestroyModelInstance();
    void __DestroyMeshMatrices();

    void __CreateModelInstance();
    void __CreateMeshMatrices();

    void __DestroyWorldPose();
    void __CreateWorldPose();

    bool __CreateMeshBindingVector(CGrannyModel *skeletonModel);
    void __DestroyMeshBindingVector();

    const granny_int32x *__GetMeshBoneIndices(unsigned int iMeshBinding) const;
    granny_world_pose *__GetWorldPosePtr() const;

    // Update & Render
    void UpdateWorldPose(float allowedError,const Matrix *c_pWorldMatrix);
    void UpdateWorldMatrices(const Matrix *c_pWorldMatrix);

    void RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType,
                                          CGrannyMaterial::EType eMtrlType);
    void RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType,
                                          CGrannyMaterial::EType eMtrlType);

    void RenderMeshNodeListWithThreeTexture(CGrannyMesh::EType eMeshType,
                                            CGrannyMaterial::EType eMtrlType);
    void RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType,
                                          CGrannyMaterial::EType eMtrlType);

protected:
    std::vector<granny_mesh_binding *> m_vct_pgrnMeshBinding;
    CGrannyMaterialPalette m_kMtrlPal;

    Matrix m_worldTransform;
    Matrix *m_meshMatrices;
    std::unordered_map<uint32_t, std::vector<Matrix>> m_meshToMatrices;

    CGrannyModel *m_model;
    granny_model_instance *m_modelInstance;
    granny_control *m_pgrnCtrl;
    CGrannyModelInstance *m_skeletonInstance;

    const CGrannyModelInstance *m_attachedTo;
    granny_world_pose *m_worldPose; // Physical memory allocated here
    uint64_t m_attachedToBone;
    float m_fLocalTime;
};

#endif
