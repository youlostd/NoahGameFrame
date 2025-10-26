#include "Stdafx.h"
#include "ModelInstance.h"
#include "Model.h"

#include "../EterBase/Timer.h"

void CGrannyModelInstance::MakeBoundBox(TBoundBox *pBoundBox,
                                        const float *mat,
                                        const float *OBBMin,
                                        const float *OBBMax,
                                        Vector3 *vtMin,
                                        Vector3 *vtMax)
{
    pBoundBox->sx = OBBMin[0] * mat[0] + OBBMin[1] * mat[4] + OBBMin[2] * mat[8] + mat[12];
    pBoundBox->sy = OBBMin[0] * mat[1] + OBBMin[1] * mat[5] + OBBMin[2] * mat[9] + mat[13];
    pBoundBox->sz = OBBMin[0] * mat[2] + OBBMin[1] * mat[6] + OBBMin[2] * mat[10] + mat[14];

    pBoundBox->ex = OBBMax[0] * mat[0] + OBBMax[1] * mat[4] + OBBMax[2] * mat[8] + mat[12];
    pBoundBox->ey = OBBMax[0] * mat[1] + OBBMax[1] * mat[5] + OBBMax[2] * mat[9] + mat[13];
    pBoundBox->ez = OBBMax[0] * mat[2] + OBBMax[1] * mat[6] + OBBMax[2] * mat[10] + mat[14];

    vtMin->x = std::min(vtMin->x, pBoundBox->sx);
    vtMin->x = std::min(vtMin->x, pBoundBox->ex);
    vtMin->y = std::min(vtMin->y, pBoundBox->sy);
    vtMin->y = std::min(vtMin->y, pBoundBox->ey);
    vtMin->z = std::min(vtMin->z, pBoundBox->sz);
    vtMin->z = std::min(vtMin->z, pBoundBox->ez);

    vtMax->x = std::max(vtMax->x, pBoundBox->sx);
    vtMax->x = std::max(vtMax->x, pBoundBox->ex);
    vtMax->y = std::max(vtMax->y, pBoundBox->sy);
    vtMax->y = std::max(vtMax->y, pBoundBox->ey);
    vtMax->z = std::max(vtMax->z, pBoundBox->sz);
    vtMax->z = std::max(vtMax->z, pBoundBox->ez);
}

bool CGrannyModelInstance::Intersect(const Matrix *c_pMatrix,
                                     float *pu, float *pv, float *pt, Vector3 *scale)
{
    if (!m_modelInstance)
        return false;

    Vector3 vtMin, vtMax;
    GetBoundBox(&vtMin, &vtMax, scale);

    if (GrannyRayIntersectsBox((granny_real32 const *)c_pMatrix, (granny_real32 *)&vtMin, (granny_real32 *)&vtMax,
                               (float*)&ms_vtPickRayOrig, (float*)&ms_vtPickRayDir) == 1)
        return true;

    return false;
}

void CGrannyModelInstance::GetBoundBox(Vector3 *vtMin, Vector3 *vtMax, Vector3 *scale)
{
    if (!m_modelInstance)
        return;

    TBoundBox BoundBox;

    vtMin->x = vtMin->y = vtMin->z = +100000.0f;
    vtMax->x = vtMax->y = vtMax->z = -100000.0f;

    int meshCount = m_model->GetMeshCount();
    for (int m = 0; m < meshCount; ++m)
    {
        const granny_mesh *pgrnMesh = m_model->GetGrannyModelPointer()->MeshBindings[m].Mesh;
        const granny_int32x *boneIndices = __GetMeshBoneIndices(m);

        for (int b = 0; b < pgrnMesh->BoneBindingCount; ++b)
        {
            const granny_bone_binding &rgrnBoneBinding = pgrnMesh->BoneBindings[b];

            Matrix mat(GrannyGetWorldPose4x4(__GetWorldPosePtr(), boneIndices[b]));
            if (scale)
            {
                Matrix matScale = Matrix::CreateScale(*scale);
                mat *= matScale;
            }

            MakeBoundBox(&BoundBox,
                         (float *)&mat,
                         rgrnBoneBinding.OBBMin, rgrnBoneBinding.OBBMax,
                         vtMin, vtMax);
        }
    }
}

bool CGrannyModelInstance::GetMeshMatrixPointer(int iMesh, const Matrix **c_ppMatrix) const
{
    if (!m_modelInstance)
        return false;

    int meshCount = m_model->GetMeshCount();

    if (meshCount <= 0)
        return false;

    *c_ppMatrix = (Matrix *)GrannyGetWorldPose4x4(__GetWorldPosePtr(), __GetMeshBoneIndices(iMesh)[0]);
    return true;
}
