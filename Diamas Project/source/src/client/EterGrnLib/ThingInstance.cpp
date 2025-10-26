#include "ThingInstance.h"

#include "GrannyState.hpp"
#include "ModelInstance.h"
#include "ModelShader.h"
#include "StdAfx.h"
#include "Thing.h"

#include "../EterBase/Timer.h"
#include "../Eterlib/Camera.h"
#include <utility>

#include "../EterBase/StepTimer.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/StateManager.h"
#include "base/Remotery.h"
#include "game/Constants.hpp"

CDynamicPool<CGraphicThingInstance> CGraphicThingInstance::ms_kPool;
std::unordered_map<uint32_t, std::unordered_map<uint32_t, CGraphicThing::Ptr>>
    CGraphicThingInstance::m_roMotionThingMap;

CGraphicThing::Ptr CGraphicThingInstance::GetBaseThingPtr()
{
    if (m_modelThings.empty())
    {
        return nullptr;
    }

    return m_modelThings[0];
}

bool CGraphicThingInstance::LessRenderOrder(CGraphicThingInstance *pkThingInst)
{
    return GetBaseThingPtr() < pkThingInst->GetBaseThingPtr();
}

void CGraphicThingInstance::CreateSystem(UINT uCapacity)
{
    ms_kPool.Create(uCapacity);
}

void CGraphicThingInstance::DestroySystem()
{
    m_roMotionThingMap.clear();
    ms_kPool.Destroy();
}

CGraphicThingInstance *CGraphicThingInstance::New()
{
    return ms_kPool.Alloc();
}

void CGraphicThingInstance::Delete(CGraphicThingInstance *pkThingInst)
{
    pkThingInst->Clear();
    ms_kPool.Free(pkThingInst);
}

bool CGraphicThingInstance::Picking(const Vector3 &v, const Vector3 &dir, float &out_x, float &out_y)
{
    if (!m_pHeightAttributeInstance)
    {
        return false;
    }

    return m_pHeightAttributeInstance->Picking(Vector3(v.x, v.y, v.z), Vector3(dir.x, dir.y, dir.z), out_x,
                                               out_y);
}

void CGraphicThingInstance::OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector)
{
    assert(pscdVector);

    for (auto &v : *pscdVector)
    {
        AddCollision(&v, &GetTransform());
    }
}

void CGraphicThingInstance::OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance)
{
    assert(pAttributeInstance);
    SetHeightInstance(pAttributeInstance);
}

bool CGraphicThingInstance::OnGetObjectHeight(float fX, float fY, float *pfHeight)
{
    if (m_pHeightAttributeInstance && m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight))
    {
        return true;
    }

    return false;
}

void CGraphicThingInstance::BuildBoundingSphere()
{
    Vector3 v3Min, v3Max;
    GetBoundBox(0, &v3Min, &v3Max);
    m_v3Center = (v3Min + v3Max) * 0.5f;
    Vector3 vDelta = (v3Max - v3Min);

    m_fRadius = vDelta.Length() * 0.5f + 50.0f; // extra length for attached objects
}

bool CGraphicThingInstance::GetBoundingSphere(Vector3 &v3Center, float &fRadius)
{
    if (m_fRadius <= 0)
    {
        BuildBoundingSphere();

        fRadius = m_fRadius;
        v3Center = m_v3Center;
    }
    else
    {
        fRadius = m_fRadius;
        v3Center = m_v3Center;
    }

    v3Center = Vector3::Transform(v3Center, GetTransform());
    return true;
}

void CGraphicThingInstance::BuildBoundingAABB()
{
    Vector3 v3Min, v3Max;
    GetBoundBox(0, &v3Min, &v3Max);
    m_v3Center = (v3Min + v3Max) * 0.5f;
    m_v3Min = v3Min;
    m_v3Max = v3Max;
}

bool CGraphicThingInstance::GetBoundingAABB(Vector3 &v3Min, Vector3 &v3Max)
{
    BuildBoundingAABB();

    v3Min = m_v3Min;
    v3Max = m_v3Max;

    m_v3Center = Vector3::Transform(m_v3Center, GetTransform());
    return true;
}

void CGraphicThingInstance::CalculateBBox()
{
    GetBoundBox(&m_v3BBoxMin, &m_v3BBoxMax);

    m_v4TBBox[0] = Vector4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[1] = Vector4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[2] = Vector4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[3] = Vector4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[4] = Vector4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[5] = Vector4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[6] = Vector4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[7] = Vector4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

    const Matrix &c_rmatTransform = GetTransform();

    for (uint32_t i = 0; i < 8; ++i)
    {
        m_v4TBBox[i] = Vector4::Transform(m_v4TBBox[i], c_rmatTransform);
        if (0 == i)
        {
            m_v3TBBoxMin.x = m_v4TBBox[i].x;
            m_v3TBBoxMin.y = m_v4TBBox[i].y;
            m_v3TBBoxMin.z = m_v4TBBox[i].z;
            m_v3TBBoxMax.x = m_v4TBBox[i].x;
            m_v3TBBoxMax.y = m_v4TBBox[i].y;
            m_v3TBBoxMax.z = m_v4TBBox[i].z;
        }
        else
        {
            if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
            {
                m_v3TBBoxMin.x = m_v4TBBox[i].x;
            }
            if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
            {
                m_v3TBBoxMax.x = m_v4TBBox[i].x;
            }
            if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
            {
                m_v3TBBoxMin.y = m_v4TBBox[i].y;
            }
            if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
            {
                m_v3TBBoxMax.y = m_v4TBBox[i].y;
            }
            if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
            {
                m_v3TBBoxMin.z = m_v4TBBox[i].z;
            }
            if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
            {
                m_v3TBBoxMax.z = m_v4TBBox[i].z;
            }
        }
    }
}

bool CGraphicThingInstance::CreateDeviceObjects()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->CreateDeviceObjects();
    }

    return true;
}

void CGraphicThingInstance::DestroyDeviceObjects()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->DestroyDeviceObjects();
    }
}

bool CGraphicThingInstance::CheckModelInstanceIndex(int iModelInstance)
{
    if (iModelInstance < 0)
    {
        return false;
    }

    int max = m_modelInstances.size();
    if (iModelInstance >= max)
    {
        return false;
    }

    return true;
}

bool CGraphicThingInstance::CheckModelThingIndex(int iModelThing)
{
    if (iModelThing < 0)
    {
        return false;
    }

    int max = m_modelThings.size();
    if (iModelThing >= max)
    {
        return false;
    }

    return true;
}

bool CGraphicThingInstance::CheckMotionThingIndex(uint32_t dwMotionKey, uint32_t motionId)
{
    const auto it = m_roMotionThingMap.find(motionId);
    if (it == m_roMotionThingMap.end())
        return false;

    if (const auto itor = it->second.find(dwMotionKey); it->second.end() == itor)
        return false;

    return true;
}

bool CGraphicThingInstance::GetMotionThingPointer(uint32_t motionId, uint32_t dwMotionKey, CGraphicThing::Ptr *ppMotion)
{
    if (!CheckMotionThingIndex(dwMotionKey, motionId))
        return false;

    *ppMotion = m_roMotionThingMap[motionId][dwMotionKey];
    return true;
}

bool CGraphicThingInstance::IsMotionThing()
{
    return !m_roMotionThingMap[m_dwMotionID].empty();
}

void CGraphicThingInstance::ReserveModelInstance(int iCount)
{
    for (const auto &mi : m_modelInstances)
    {
        // Apparently Delete() doesn't handle NULLs
        if (!mi)
        {
            continue;
        }

        CGrannyModelInstance::Delete(mi);
    }

    m_modelInstances.clear();
    m_modelInstances.resize(iCount);
}

void CGraphicThingInstance::ReserveModelThing(int iCount)
{
    m_modelThings.resize(iCount);
}

bool CGraphicThingInstance::FindBoneIndex(int iModelInstance, const char *c_szBoneName, int *iRetBone)
{
    assert(CheckModelInstanceIndex(iModelInstance));

    auto pModelInstance = m_modelInstances[iModelInstance];
    if (!pModelInstance)
    {
        return false;
    }

    return pModelInstance->GetBoneIndexByName(c_szBoneName, iRetBone);
}

bool CGraphicThingInstance::GetBoneMatrixPointer(int iModelInstance, const char *c_szBoneName, float **boneMatrix)
{
    assert(CheckModelInstanceIndex(iModelInstance));

    auto pModelInstance = m_modelInstances[iModelInstance];
    if (!pModelInstance)
    {
        return false;
    }

    int retBone;
    if (!pModelInstance->GetBoneIndexByName(c_szBoneName, &retBone))
    {
        *boneMatrix = nullptr;
        return false;
    }

    *boneMatrix = (float *)pModelInstance->GetBoneMatrixPointer(retBone);
    return *boneMatrix != nullptr;
}

bool CGraphicThingInstance::GetBoneMatrixPointer(int iModelInstance, const char *c_szBoneName, Matrix **boneMatrix)
{
    assert(CheckModelInstanceIndex(iModelInstance));

    auto pModelInstance = m_modelInstances[iModelInstance];
    if (!pModelInstance)
    {
        return false;
    }

    int retBone;
    if (!pModelInstance->GetBoneIndexByName(c_szBoneName, &retBone))
    {
        *boneMatrix = nullptr;
        return false;
    }

    *boneMatrix = (Matrix *)pModelInstance->GetBoneMatrixPointer(retBone);
    return *boneMatrix != nullptr;
}

bool CGraphicThingInstance::AttachModelInstance(int iDstModelInstance, CGraphicThingInstance &rSrcInstance,
                                                int iSrcModelInstance, int boneIndex)
{
    if (!CheckModelInstanceIndex(iDstModelInstance))
    {
        SPDLOG_ERROR("iDstModelInstance {0} out-of-range", iSrcModelInstance);
        return false;
    }

    if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
    {
        SPDLOG_ERROR("iSrcModelInstance {0} out-of-range", iSrcModelInstance);
        return false;
    }

    auto srcMi = rSrcInstance.m_modelInstances[iSrcModelInstance];
    auto dstMi = m_modelInstances[iDstModelInstance];

    assert(srcMi && "No model bound");

    // Apparently these are NULL in some situations...
    // Log and fail in a release version
    if (!srcMi || !dstMi)
    {
        SPDLOG_ERROR("source or dest. model instance is NULL");
        return false;
    }

    srcMi->SetParentModelInstance(dstMi, boneIndex);
    return true;
}

void CGraphicThingInstance::DetachModelInstance(int iDstModelInstance, CGraphicThingInstance &rSrcInstance,
                                                int iSrcModelInstance)
{
    if (!CheckModelInstanceIndex(iDstModelInstance))
    {
        SPDLOG_ERROR("iDstModelInstance {0} out-of-range", iSrcModelInstance);
        return;
    }

    if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
    {
        SPDLOG_ERROR("iSrcModelInstance {0} out-of-range", iSrcModelInstance);
        return;
    }

    auto srcMi = rSrcInstance.m_modelInstances[iSrcModelInstance];
    assert(srcMi && "No model bound");

    // Apparently these are NULL in some situations...
    // Log and fail in a release version
    if (!srcMi)
    {
        SPDLOG_ERROR("source model instance is NULL");
        return;
    }

    srcMi->SetParentModelInstance(nullptr, 0);
}

bool CGraphicThingInstance::GetBonePosition(int iModelIndex, int iBoneIndex, float *pfx, float *pfy, float *pfz)
{
    assert(CheckModelInstanceIndex(iModelIndex));

    auto pModelInstance = m_modelInstances[iModelIndex];
    if (!pModelInstance)
    {
        return false;
    }

    const float *pfMatrix = pModelInstance->GetBoneMatrixPointer(iBoneIndex);

    *pfx = pfMatrix[12];
    *pfy = pfMatrix[13];
    *pfz = pfMatrix[14];
    return true;
}

bool CGraphicThingInstance::SetModelInstance(int iDstModelInstance, int iSrcModelThing, int iSrcModel,
                                             int skeletonModelInstance)
{
    if (!CheckModelInstanceIndex(iDstModelInstance))
    {
        SPDLOG_ERROR("iDstModelInstance={0} pModelThing={1} iSrcModel={2}", iDstModelInstance, iSrcModelThing,
                     iSrcModel);
        return false;
    }

    if (!CheckModelThingIndex(iSrcModelThing))
    {
        SPDLOG_ERROR("iDstModelInstance={0}, pModelThing={1}, iSrcModel={2}", iDstModelInstance, iSrcModelThing,
                     iSrcModel);
        return false;
    }

    if (m_modelInstances[iDstModelInstance])
    {
        CGrannyModelInstance::Delete(m_modelInstances[iDstModelInstance]);
        m_modelInstances[iDstModelInstance] = nullptr;
    }

    auto &modelThing = m_modelThings[iSrcModelThing];
    if (!modelThing)
    {
        return false;
    }

    if (modelThing->GetModelCount() <= iSrcModel)
    {
        SPDLOG_ERROR("iSrcModel {0} >= model count {1}", iSrcModel, modelThing->GetModelCount());
        return false;
    }

    CGrannyModelInstance *skeletonMi = nullptr;
    if (skeletonModelInstance != -1)
    {
        if (!CheckModelInstanceIndex(skeletonModelInstance))
        {
            SPDLOG_ERROR("skeletonModelInstance={0} out of range", skeletonModelInstance);
            return false;
        }

        skeletonMi = m_modelInstances[skeletonModelInstance];
    }

    auto mi = CGrannyModelInstance::New();
    mi->SetLinkedModelPointer(modelThing->GetModelPointer(iSrcModel), skeletonMi);

    // TODO(Nover): We shoudln't need to do this!
    // FIXME: I use CModelInstance :: m_worldPose in Update,
    // NULL if not deformed. Maybe the structure needs to change a bit. - [levites]
    mi->DeformNoSkin(&ms_matIdentity, 0.0f);

    m_modelInstances[iDstModelInstance] = mi;
    return true;
}

void CGraphicThingInstance::SetMaterialImagePointer(UINT ePart, const char *c_szImageName, CGraphicImage::Ptr pImage)
{
    if (ePart >= m_modelInstances.size())
    {
        SPDLOG_ERROR("ePart {0} >= model instance count {1} image {2} {3}", ePart, m_modelInstances.size(),
                     c_szImageName, pImage->GetFileName());
        return;
    }

    if (!m_modelInstances[ePart])
    {
        SPDLOG_ERROR("ePart {0} NULL - model instance count {1} image {2} {3}", ePart, m_modelInstances.size(),
                     c_szImageName, pImage->GetFileName());
        return;
    }

    m_modelInstances[ePart]->SetMaterialImagePointer(c_szImageName, pImage);
}

void CGraphicThingInstance::SetMaterialData(UINT ePart, const char *c_szImageName, const SMaterialData &kMaterialData)
{
    if (ePart >= m_modelInstances.size())
    {
        SPDLOG_ERROR("ePart {0} >= model instance count {1} image {2}", ePart, m_modelInstances.size(), c_szImageName);
        return;
    }

    if (!m_modelInstances[ePart])
    {
        SPDLOG_ERROR("ePart {0} NULL - model instance count {1} image {2}", ePart, m_modelInstances.size(),
                     c_szImageName);
        return;
    }

    m_modelInstances[ePart]->SetMaterialData(c_szImageName, kMaterialData);
}

void CGraphicThingInstance::SetSpecularInfo(UINT ePart, const char *c_szMtrlName, bool bEnable, float fPower)
{
    if (ePart >= m_modelInstances.size())
    {
        SPDLOG_ERROR("ePart {0} >= model instance count {1} image {2}", ePart, m_modelInstances.size(), c_szMtrlName);
        return;
    }

    if (!m_modelInstances[ePart])
    {
        SPDLOG_ERROR("ePart {0} NULL - model instance count {1} image {2}", ePart, m_modelInstances.size(),
                     c_szMtrlName);
        return;
    }

    m_modelInstances[ePart]->SetSpecularInfo(c_szMtrlName, bEnable, fPower);
}

bool CGraphicThingInstance::SetMotion(uint32_t dwMotionKey, float blendTime, int loopCount, float speedRatio)
{
    const auto itor = m_roMotionThingMap[m_dwMotionID].find(dwMotionKey);
    if (itor == m_roMotionThingMap[m_dwMotionID].end())
    {
        return false;
    }

    CGraphicThing::Ptr pMotionThing = itor->second;
    assert(pMotionThing && "NULL check");

    if (!pMotionThing->CheckMotionIndex(0))
    {
        return false;
    }

    const auto *motion = pMotionThing->GetMotionPointer(0);
    assert(motion && "NULL check");

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->SetMotionPointer(motion, blendTime, loopCount, speedRatio);
    }

    return true;
}

bool CGraphicThingInstance::ChangeMotion(uint32_t dwMotionKey, int loopCount, float speedRatio)
{
    const auto itor = m_roMotionThingMap[m_dwMotionID].find(dwMotionKey);
    if (itor == m_roMotionThingMap[m_dwMotionID].end())
    {
        return false;
    }

    auto pMotionThing = itor->second;
    assert(pMotionThing && "NULL check");

    if (!pMotionThing->CheckMotionIndex(0))
    {
        return false;
    }

    const auto *motion = pMotionThing->GetMotionPointer(0);
    assert(motion && "NULL check");

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->ChangeMotionPointer(motion, loopCount, speedRatio);
    }

    return true;
}

void CGraphicThingInstance::SetMotionAtEnd()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->SetMotionAtEnd();
    }
}

uint32_t CGraphicThingInstance::GetLoopIndex(uint32_t modelInstance)
{
    if (m_modelInstances.size() <= modelInstance || !m_modelInstances[modelInstance])
    {
        return 0;
    }

    return m_modelInstances[modelInstance]->GetLoopIndex();
}

void CGraphicThingInstance::PrintControls()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        SPDLOG_DEBUG("ModelInstance: {0}", mi->GetModel()->GetFromFilename());
    }
}

void CGraphicThingInstance::RegisterModelThing(int iModelThing, CGraphicThing::Ptr pModelThing)
{
    if (!CheckModelThingIndex(iModelThing))
    {
        SPDLOG_ERROR("CGraphicThingInstance::RegisterModelThing(iModelThing={0})", iModelThing);
        return;
    }

    m_modelThings[iModelThing] = std::move(pModelThing);
}

void CGraphicThingInstance::RegisterMotionThing(uint32_t motionId, uint32_t dwMotionKey,
                                                CGraphicThing::Ptr pMotionThing)
{
    assert(pMotionThing && "null motions not allowed");

    if (CheckMotionThingIndex(dwMotionKey, motionId))
    {
        return;
    }

    SPDLOG_INFO("Registered motion ({}, {})", motionId, dwMotionKey);
    m_roMotionThingMap[motionId][dwMotionKey] = pMotionThing;
}

void CGraphicThingInstance::ResetLocalTime()
{
    m_fLastLocalTime = 0.0f;
    m_fLocalTime = 0.0f;

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->ResetLocalTime();
    }
}

void CGraphicThingInstance::InsertDelay(float fDelay)
{
    m_fDelay = fDelay;
}

float CGraphicThingInstance::GetLastLocalTime() const
{
    return m_fLastLocalTime;
}

float CGraphicThingInstance::GetLocalTime() const
{
    return m_fLocalTime;
}

float CGraphicThingInstance::GetSecondElapsed() const
{
    return m_fSecondElapsed;
}

float CGraphicThingInstance::GetAverageSecondElapsed() const
{
    return m_fAverageSecondElapsed;
}

bool CGraphicThingInstance::Intersect(float *pu, float *pv, float *pt)
{
    if (!CGraphicObjectInstance::isShow())
        return false;

    if (!m_bUpdated)
        return false;

    if (m_modelInstances.empty() || !m_modelInstances[0])
        return false;

    return m_modelInstances[0]->Intersect(&GetTransform(), pu, pv, pt);
}

void CGraphicThingInstance::GetBoundBox(Vector3 *vtMin, Vector3 *vtMax)
{
    vtMin->x = vtMin->y = vtMin->z = 100000.0f;
    vtMax->x = vtMax->y = vtMax->z = -100000.0f;

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->GetBoundBox(vtMin, vtMax);
    }
}

bool CGraphicThingInstance::GetBoundBox(uint32_t dwModelInstanceIndex, Vector3 *vtMin, Vector3 *vtMax)
{
    if (!CheckModelInstanceIndex(dwModelInstanceIndex))
    {
        return false;
    }

    vtMin->x = vtMin->y = vtMin->z = 100000.0f;
    vtMax->x = vtMax->y = vtMax->z = -100000.0f;

    auto mi = m_modelInstances[dwModelInstanceIndex];
    if (!mi)
    {
        return false;
    }

    mi->GetBoundBox(vtMin, vtMax);
    return true;
}

bool CGraphicThingInstance::GetBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, Matrix **ppMatrix)
{
    if (!CheckModelInstanceIndex(dwModelInstanceIndex))
    {
        return false;
    }

    const auto *mi = m_modelInstances[dwModelInstanceIndex];
    if (!mi)
    {
        return false;
    }

    *ppMatrix = (Matrix *)mi->GetBoneMatrixPointer(dwBoneIndex);
    if (!*ppMatrix)
    {
        return false;
    }

    return true;
}

bool CGraphicThingInstance::GetCompositeBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex,
                                                   Matrix **ppMatrix)
{
    if (!CheckModelInstanceIndex(dwModelInstanceIndex))
    {
        return false;
    }

    const auto *mi = m_modelInstances[dwModelInstanceIndex];
    if (!mi)
    {
        return false;
    }

    *ppMatrix = (Matrix *)mi->GetCompositeBoneMatrixPointer(dwBoneIndex);
    return true;
}

void CGraphicThingInstance::UpdateTransform(Matrix *pMatrix, double fSecondsElapsed, uint64_t iModelInstanceIndex)
{
    if (iModelInstanceIndex >= m_modelInstances.size())
    {
        return;
    }

    auto &mi = m_modelInstances[iModelInstanceIndex];
    if (!mi)
    {
        return;
    }

    mi->UpdateTransform(pMatrix, fSecondsElapsed);
}

void CGraphicThingInstance::DeformNoSkin()
{
    m_bUpdated = true;

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
            continue;

        mi->DeformNoSkin(&m_worldMatrix, m_allowedError);
    }
}

void CGraphicThingInstance::TransformAttachment()
{
    const auto mainMi = m_modelInstances[0];
    if (mainMi)
    {
        int32_t boneIndex = 0;
        mainMi->GetBoneIndexByName("Bip01 Spine2", &boneIndex);

        const Matrix *matBone = reinterpret_cast<const Matrix *>(mainMi->GetBoneMatrixPointer(boneIndex));
        if (matBone)
        {
            Vector3 scalingCenter(matBone->_41, matBone->_42, matBone->_43);

            Quaternion scalingRotation(0.0f, 0.0f, 0.0f, 0.0f);

            using namespace  DirectX;

            m_matScaleTransformation = XMMatrixTransformation(scalingCenter, scalingRotation, m_v3ScaleAttachment, Vector3::Zero, Vector3::Zero, Vector3::Zero);
        }

        Matrix scaleTransformed;
        scaleTransformed = m_matScale * m_matScaleTransformation;
        m_matAttachedWorldTransform = scaleTransformed * m_mRotation;
        m_matAttachedWorldTransform._41 += m_v3Position.x;
        m_matAttachedWorldTransform._42 += m_v3Position.y;
        m_matAttachedWorldTransform._43 += m_v3Position.z;
    }
}

void CGraphicThingInstance::OnDeform()
{

    rmt_ScopedCPUSample(ThingInstanceDeform, RMTSF_Aggregate);
    for (size_t i = 0; i < m_modelInstances.size(); i++)
    {
        auto *mi = m_modelInstances[i];
        if (!mi)
            continue;

#if defined(WJ_SHOW_ACCE_ONOFF)
        if (i == PART_ACCE && Engine::GetSettings().IsShowAcce())
        {
            TransformAttachment();
            mi->Deform(&m_matAttachedWorldTransform, m_allowedError);
            continue;
        }
#endif

        mi->Deform(&m_worldMatrix, m_allowedError);
    }

    m_bUpdated = true;
}

void CGraphicThingInstance::__SetLocalTime(float fLocalTime)
{
    m_fLastLocalTime = m_fLocalTime;
    m_fLocalTime = fLocalTime;

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->SetLocalTime(fLocalTime);
    }
}

void CGraphicThingInstance::UpdateLODLevel()
{
    CCamera *pcurCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pcurCamera)
    {
        SPDLOG_ERROR("CGraphicThingInstance::UpdateLODLevel - GetCurrentCamera() == NULL");
        return;
    }

    const Vector3 &c_rv3CameraPosition = pcurCamera->GetEye();
    const Vector3 &c_v3Position = GetPosition();
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetCameraPosition(c_rv3CameraPosition);
    }
    // NOTE : 중심으로부터의 거리 계산에 z값 차이는 사용하지 않는다. - [levites]
    float distanceFromCamera = Vector3::Distance(c_rv3CameraPosition, c_v3Position);

    m_allowedError = GrannyFindAllowedLODError(1.0f,                // error in pixels
                                               ms_iHeight,          // screen height
                                               GetFOV(),            // fovy
                                               distanceFromCamera); // distance
}

void CGraphicThingInstance::UpdateTime()
{
	m_fSecondElapsed = CTimer::Instance().GetElapsedSecond();

    if (m_fDelay > m_fSecondElapsed)
    {
        m_fDelay -= m_fSecondElapsed;
        m_fSecondElapsed = 0.0f;
    }
    else
    {
        m_fSecondElapsed -= m_fDelay;
        m_fDelay = 0.0f;
    }

    m_fLastLocalTime = m_fLocalTime;
    m_fLocalTime += m_fSecondElapsed;
	m_fAverageSecondElapsed = m_fAverageSecondElapsed + (m_fSecondElapsed - m_fAverageSecondElapsed) / 4.0f;

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
            continue;

        mi->AdvanceTime(m_fSecondElapsed);
    }
}


void CGraphicThingInstance::OnUpdate()
{
    UpdateTime();
}

void CGraphicThingInstance::OnRender()
{
    RenderWithOneTexture(true);
}

void CGraphicThingInstance::OnBlendRender()
{
    BlendRenderWithOneTexture();
}

void CGraphicThingInstance::RenderWithOneTexture(bool showWeapon)
{
    // assert(m_bUpdated);
    if (!m_bUpdated)
    {
        return;
    }
    rmt_ScopedCPUSample(RenderWithOneTexture, RMTSF_Aggregate);

    for (size_t i = 0; i < m_modelInstances.size(); i++)
    {
        auto *mi = m_modelInstances[i];
        if (!mi)
            continue;

#if defined(WJ_SHOW_ACCE_ONOFF)
        if (i == PART_ACCE && !Engine::GetSettings().IsShowAcce())
        {
            continue;
        }
#endif

        if ((i == 3 || i == 1) && !showWeapon)
            continue;
        
        mi->RenderWithOneTexture();
    }

}

void CGraphicThingInstance::BlendRenderWithOneTexture()
{
    if (!m_bUpdated)
    {
        return;
    }

    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->BlendRenderWithOneTexture();
    }
}

void CGraphicThingInstance::OnRenderToShadowMap(bool showWeapon)
{
    if (!m_bUpdated)
    {
        return;
    }
    auto ms = GrannyState::instance().GetModelShader();
    if(ms)
    {
        ms->SetPassIndex(MODEL_SHADER_PASS_SHADOW);
    }
    for (size_t i = 0; i < m_modelInstances.size(); i++)
    {
        auto *mi = m_modelInstances[i];
        if (!mi)
            continue;

#if defined(WJ_SHOW_ACCE_ONOFF)
        if (i == PART_ACCE && !Engine::GetSettings().IsShowAcce())
        {
            continue;
        }
#endif

        if ((i == 3 || i == 1) && !showWeapon)
            continue;
        
        mi->RenderWithoutTexture();
    }
}

void CGraphicThingInstance::OnRenderShadow()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->RenderWithOneTexture();
    }
}

void CGraphicThingInstance::OnRenderPCBlocker()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        auto *ms = GrannyState::instance().GetModelShader();
        if (ms)
        {
            ms->SetAlphaBlendValue(0.5f);
            ms->SetPassIndex(MODEL_SHADER_PASS_BLEND);
        }
        mi->RenderWithOneTexture();
        mi->BlendRenderWithOneTexture();
        if (ms)
        {
            ms->SetAlphaBlendValue(1.0f);
            ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
        }
    }
}

uint32_t CGraphicThingInstance::GetModelInstanceCount() const
{
    return m_modelInstances.size();
}

CGrannyModelInstance *CGraphicThingInstance::GetModelInstancePointer(uint32_t dwModelIndex) const
{
    assert(dwModelIndex < m_modelInstances.size());
    return m_modelInstances[dwModelIndex];
}

float CGraphicThingInstance::GetHeight()
{
    if (m_modelInstances.empty())
    {
        return 0.0f;
    }

    auto pModelInstance = m_modelInstances[0];
    if (!pModelInstance)
    {
        return 0.0f;
    }

    Vector3 vtMin, vtMax;
    pModelInstance->GetBoundBox(&vtMin, &vtMax);

    return fabs(vtMin.z - vtMax.z);
}

void CGraphicThingInstance::ReloadTexture()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        mi->ReloadTexture();
    }
}

bool CGraphicThingInstance::HaveBlendThing()
{
    for (auto &mi : m_modelInstances)
    {
        if (!mi)
        {
            continue;
        }

        if (mi->HaveBlendThing())
        {
            return true;
        }
    }

    return false;
}

void CGraphicThingInstance::OnClear()
{
    for (const auto &mi : m_modelInstances)
    {
        // Apparently Delete() doesn't handle NULLs
        if (!mi)
        {
            continue;
        }

        CGrannyModelInstance::Delete(mi);
    }

    m_modelInstances.clear();
    m_modelThings.clear();
}

void CGraphicThingInstance::AcceTransform()
{
}

void CGraphicThingInstance::OnInitialize()
{
    m_bUpdated = false;
    m_fLastLocalTime = 0.0;
    m_fLocalTime = 0.0;
    m_fDelay = 0.0;
    m_fSecondElapsed = 0.0;
    m_fAverageSecondElapsed = 0.03f;
    m_fRadius = -1.0f;
    m_v3Center = Vector3(0.0f, 0.0f, 0.0f);
    m_allowedError = 0.0f;
    m_dwMotionID = std::numeric_limits<uint32_t>::max();
    ResetLocalTime();
}

CGraphicThingInstance::CGraphicThingInstance()
{
    Initialize();
}

CGraphicThingInstance::~CGraphicThingInstance() = default;
