#include "../EterLib/GrpScreen.h"
#include "../eterlib/Camera.h"
#include "GrannyState.hpp"
#include "Model.h"
#include "ModelInstance.h"
#include "ModelShader.h"
#include "StdAfx.h"
#include "base/Remotery.h"

namespace
{
class SharedLocalPose
{
  public:
    SharedLocalPose(uint32_t boneCount) : m_localPose(GrannyNewLocalPose(boneCount))
    {
        // ctor
    }

    ~SharedLocalPose()
    {
        GrannyFreeLocalPose(m_localPose);
    }

    granny_local_pose *Get(uint32_t boneCount)
    {
        if (boneCount > GrannyGetLocalPoseBoneCount(m_localPose))
        {
            GrannyFreeLocalPose(m_localPose);
            m_localPose = GrannyNewLocalPose(boneCount);
        }

        return m_localPose;
    }

  private:
    granny_local_pose *m_localPose;
};
} // namespace

void CGrannyModelInstance::AdvanceTime(double elapsed)
{
    m_fLocalTime += elapsed;

    // Sample and blend the poses.
    GrannySetModelClock(m_modelInstance, m_fLocalTime);

    // Good a time as any to free up any animation controls that have finished their existence.
    GrannyFreeCompletedModelControls(m_modelInstance);
}

void CGrannyModelInstance::UpdateTransform(Matrix *pMatrix, double fSecondsElapsed)
{
    // TODO(tim): Remove this check?
    if (!m_modelInstance)
    {
        SPDLOG_ERROR("CGrannyModelIstance::UpdateTransform - m_modelInstance = NULL");
        return;
    }

    GrannyUpdateModelMatrix(m_modelInstance, fSecondsElapsed, (const float *)pMatrix, (float *)pMatrix, false);
}

void CGrannyModelInstance::SetWorldTransform(const Matrix &transform)
{
    m_worldTransform = transform;
}

void CGrannyModelInstance::Deform(const Matrix *c_pWorldMatrix, float allowedError)
{
    if (IsEmpty())
        return;

    rmt_ScopedCPUSample(ModelInstanceDeform, RMTSF_Aggregate);
    UpdateWorldPose(allowedError, c_pWorldMatrix);
    UpdateWorldMatrices(c_pWorldMatrix);
}

void CGrannyModelInstance::UpdateSkeleton(const Matrix *c_pWorldMatrix, float allowedError)
{
    UpdateWorldPose(allowedError, c_pWorldMatrix);
    UpdateWorldMatrices(c_pWorldMatrix);
}

void CGrannyModelInstance::UpdateWorldPose(float allowedError, const Matrix *c_pWorldMatrix)
{
    // Our meshes are "worn" on |m_skeletonInstance|.
    // Thus we don't have an own world-pose (see __GetWorldPosePtr)
    if (m_skeletonInstance)
        return;

    rmt_ScopedCPUSample(ModelInstanceUpdatePose, RMTSF_Aggregate);

    assert(m_worldPose && "No world-pose?");

    static SharedLocalPose sharedLocal(90);

    granny_skeleton *pgrnSkeleton = GrannyGetSourceSkeleton(m_modelInstance);
    granny_local_pose *pgrnLocalPose = sharedLocal.Get(pgrnSkeleton->BoneCount);

    const float *pAttachBoneMatrix = nullptr;
    if (m_attachedTo)
        pAttachBoneMatrix = m_attachedTo->GetBoneMatrixPointer(m_attachedToBone);

    GrannySampleModelAnimationsAcceleratedLOD(m_modelInstance, pgrnSkeleton->BoneCount, pAttachBoneMatrix,
                                              pgrnLocalPose, m_worldPose, allowedError);
}

void CGrannyModelInstance::UpdateWorldMatrices(const Matrix *c_pWorldMatrix)
{
    // NO_MESH_BUG_FIX
    if (!m_meshMatrices)
        return;
    // END_OF_NO_MESH_BUG_FIX
    rmt_ScopedCPUSample(ModelInstanceUpdateWorldMatrices, RMTSF_Aggregate);

    assert(m_model != NULL);
    assert(ms_lpd3dMatStack != NULL);

    Matrix tmp = m_worldTransform * *c_pWorldMatrix;

    int meshCount = m_model->GetMeshCount();

    granny_matrix_4x4 *pgrnMatCompositeBuffer = GrannyGetWorldPoseComposite4x4Array(__GetWorldPosePtr());
    Matrix *boneMatrices = (Matrix *)pgrnMatCompositeBuffer;

    for (int i = 0; i < meshCount; ++i)
    {
        const CGrannyMesh *pMesh = m_model->GetMeshPointer(i);

        if (pMesh->IsRigidMesh())
        {
            Matrix &rWorldMatrix = m_meshMatrices[i];
            const granny_int32x *boneIndices = __GetMeshBoneIndices(i);
            rWorldMatrix = boneMatrices[*boneIndices] * tmp;
            rWorldMatrix = rWorldMatrix.Transpose();
        }
        else
        {
            auto &vec = m_meshToMatrices[i];
            vec.clear();
            const auto *meshBinding = m_vct_pgrnMeshBinding[i];
            const granny_int32x *boneIndices = __GetMeshBoneIndices(i);

            int const NumMeshBones = GrannyGetMeshBindingBoneCount(meshBinding);
            for (int Matrix = 0; Matrix < NumMeshBones; Matrix++)
            {
                vec.emplace_back(boneMatrices[boneIndices[Matrix]] * tmp);
            }
        }
    }

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
      const Matrix transViewProj = XMMatrixMultiplyTranspose(ms_matView, ms_matProj);

        ms->SetTransformConstant(transViewProj);
    }
}
