#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"
#include "../EterLib/ResourceManager.h"

CGrannyModel *CGrannyModelInstance::GetModel()
{
    return m_model;
}

void CGrannyModelInstance::SetMaterialImagePointer(const char *c_szImageName, CGraphicImage::Ptr pImage)
{
    m_kMtrlPal.SetMaterialImage(c_szImageName, pImage);
}

void CGrannyModelInstance::SetMaterialData(const char *c_szImageName, const SMaterialData &c_rkMaterialData)
{
    m_kMtrlPal.SetMaterialData(c_szImageName, c_rkMaterialData);
}

void CGrannyModelInstance::SetSpecularInfo(const char *c_szMtrlName, bool bEnable, float fPower)
{
    m_kMtrlPal.SetSpecularInfo(c_szMtrlName, bEnable, fPower);
}

void CGrannyModelInstance::SetLocalTime(double fLocalTime)
{
    m_fLocalTime = fLocalTime;
}

void CGrannyModelInstance::ResetLocalTime()
{
    m_fLocalTime = 0.0;
}

double CGrannyModelInstance::GetLocalTime() const
{
    return m_fLocalTime;
}

void CGrannyModelInstance::SetParentModelInstance(const CGrannyModelInstance *parent, int iBone)
{
    m_attachedTo = parent;
    m_attachedToBone = iBone;
}

bool CGrannyModelInstance::IsEmpty()
{
    if (m_model)
    {
        // NO_MESH_BUG_FIX
        if (!m_meshMatrices)
            return true;
        // END_OF_NO_MESH_BUG_FIX
        return false;
    }

    return true;
}

bool CGrannyModelInstance::CreateDeviceObjects()
{
    return true;
}

void CGrannyModelInstance::DestroyDeviceObjects()
{
}

void CGrannyModelInstance::__Initialize()
{
    m_model = nullptr;
    m_attachedTo = nullptr;
    m_attachedToBone = 0;
    m_skeletonInstance = nullptr;

    m_modelInstance = nullptr;

    m_worldPose = nullptr;

    m_worldTransform = Matrix::Identity;

    m_meshMatrices = nullptr;
    m_pgrnCtrl = nullptr;
}

CGrannyModelInstance::CGrannyModelInstance()
{
    m_fLocalTime = 0.0;
    m_model = nullptr;
    m_attachedTo = nullptr;
    m_attachedToBone = 0;
    m_skeletonInstance = nullptr;

    m_modelInstance = nullptr;

    m_worldPose = nullptr;

    m_worldTransform = Matrix::Identity;
    m_meshMatrices = nullptr;
    m_pgrnCtrl = nullptr;
}

CGrannyModelInstance::~CGrannyModelInstance()
{
    Clear();
}

CDynamicPool<CGrannyModelInstance> CGrannyModelInstance::ms_kPool;

CGrannyModelInstance *CGrannyModelInstance::New()
{
    return ms_kPool.Alloc();
}

void CGrannyModelInstance::Delete(CGrannyModelInstance *pkInst)
{
    pkInst->Clear();
    ms_kPool.Free(pkInst);
}

void CGrannyModelInstance::DestroySystem()
{
    ms_kPool.Destroy();
}
