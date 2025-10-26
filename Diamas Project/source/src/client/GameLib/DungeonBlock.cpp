#include "DungeonBlock.h"
#include "../EterGrnLib/GrannyState.hpp"
#include "../EterGrnLib/ModelShader.h"
#include "StdAfx.h"

#include "../eterlib/Camera.h"
#include "../eterlib/StateManager.h"

class CDungeonModelInstance : public CGrannyModelInstance
{
  public:
    CDungeonModelInstance()
    {
    }

    virtual ~CDungeonModelInstance()
    {
    }

    void RenderDungeonBlock()
    {
        if (IsEmpty())
            return;

        RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
    }

    void RenderDungeonBlockShadow()
    {
        if (IsEmpty())
            return;

        RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
    }
};

struct FUpdate
{
    float fElapsedTime;
    float allowedError;
    Matrix *pmatWorld;

    void operator()(CGrannyModelInstance *pInstance) const
    {
        pInstance->AdvanceTime(fElapsedTime);
        pInstance->Deform(pmatWorld, allowedError);
    }
};

void CDungeonBlock::Update()
{
    CCamera *pcurCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pcurCamera)
    {
        SPDLOG_ERROR("CDungeonBlock::Update - GetCurrentCamera() == NULL");
        return;
    }

    const Vector3 &c_rv3TargetPosition = pcurCamera->GetTarget();
    const Vector3 &c_rv3CameraPosition = pcurCamera->GetEye();
    const Vector3 &c_v3Position = GetPosition();

    Transform();

    auto dist = Vector3::Distance(c_rv3CameraPosition, c_v3Position);

    FUpdate Update;
    Update.fElapsedTime = 0.0f;
    Update.pmatWorld = &m_worldMatrix;
    Update.allowedError = GrannyFindAllowedLODError(1.0f,                   // error in pixels
                                                    ms_iHeight,             // screen height
                                                    GetFOV(),               // fovy
                                                    dist); // distance
    for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(), Update);
}

void CDungeonBlock::Render()
{
    if (!isShow())
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CDungeonBlock::Render **");

    for (const auto &mi : m_ModelInstanceContainer)
        mi->RenderDungeonBlock();

    D3DPERF_EndEvent();
}

struct FRenderShadow
{
    void operator()(CDungeonModelInstance *pInstance)
    {
        pInstance->RenderDungeonBlockShadow();
    }
};

void CDungeonBlock::OnRenderShadow()
{
    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

    for (const auto &mi : m_ModelInstanceContainer)
        mi->RenderDungeonBlockShadow();

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}

struct FBoundBox
{
    Vector3 *m_pv3Min;
    Vector3 *m_pv3Max;

    FBoundBox(Vector3 *pv3Min, Vector3 *pv3Max)
    {
        m_pv3Min = pv3Min;
        m_pv3Max = pv3Max;
    }

    void operator()(CGrannyModelInstance *pInstance) const
    {
        Vector3 v3Min;
        Vector3 v3Max;
        pInstance->GetBoundBox(&v3Min, &v3Max);

        if (v3Min.x < m_pv3Min->x)
            m_pv3Min->x = v3Min.x;
        if (v3Min.y < m_pv3Min->y)
            m_pv3Min->y = v3Min.y;
        if (v3Min.z < m_pv3Min->z)
            m_pv3Min->z = v3Min.z;

        if (v3Max.x > m_pv3Max->x)
            m_pv3Max->x = v3Max.x;
        if (v3Max.y > m_pv3Max->y)
            m_pv3Max->y = v3Max.y;
        if (v3Max.z > m_pv3Max->z)
            m_pv3Max->z = v3Max.z;
    }
};

bool CDungeonBlock::GetBoundingSphere(Vector3 &v3Center, float &fRadius)
{
    v3Center = m_v3Center;
    fRadius = m_fRadius;
    v3Center = Vector3::Transform(v3Center, GetTransform());
    return true;
}

void CDungeonBlock::OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector)
{
    assert(pscdVector);
    CStaticCollisionDataVector::const_iterator it;
    for (it = pscdVector->begin(); it != pscdVector->end(); ++it)
    {
        AddCollision(&(*it), &GetTransform());
    }
}

void CDungeonBlock::OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance)
{
    assert(pAttributeInstance);
    SetHeightInstance(pAttributeInstance);
}

bool CDungeonBlock::OnGetObjectHeight(float fX, float fY, float *pfHeight)
{
    return m_pHeightAttributeInstance && m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight);
}

void CDungeonBlock::BuildBoundingSphere()
{
    Vector3 v3Min, v3Max;
    GetBoundBox(&v3Min, &v3Max);

    m_v3Center = (v3Min + v3Max) * 0.5f;

    Vector3 vDelta = (v3Max - v3Min);
    m_fRadius = vDelta.Length() * 0.5f + 50.0f; // extra length for attached objects
}

bool CDungeonBlock::Intersect(float *pfu, float *pfv, float *pft)
{
    TModelInstanceContainer::iterator itor = m_ModelInstanceContainer.begin();
    for (; itor != m_ModelInstanceContainer.end(); ++itor)
    {
        CDungeonModelInstance *pInstance = *itor;
        if (pInstance->Intersect(&CGraphicObjectInstance::GetTransform(), pfu, pfv, pft))
            return true;
    }

    return false;
}

void CDungeonBlock::GetBoundBox(Vector3 *pv3Min, Vector3 *pv3Max)
{
    pv3Min->x = pv3Min->y = pv3Min->z = +100000.0f;
    pv3Max->x = pv3Max->y = pv3Max->z = -100000.0f;

    std::for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(), FBoundBox(pv3Min, pv3Max));
}

bool CDungeonBlock::Load(const char *c_szFileName)
{
    Destroy();

    m_pThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);
    if (!m_pThing || m_pThing->GetModelCount() == 0)
    {
        SPDLOG_ERROR("Failed to load {0} (no models?)", c_szFileName);
        return false;
    }

    m_ModelInstanceContainer.reserve(m_pThing->GetModelCount());

    for (int i = 0; i < m_pThing->GetModelCount(); ++i)
    {
        CDungeonModelInstance *pModelInstance = new CDungeonModelInstance;
        pModelInstance->SetLinkedModelPointer(m_pThing->GetModelPointer(i), nullptr);

        m_ModelInstanceContainer.push_back(pModelInstance);
    }

    return true;
}

void CDungeonBlock::ReloadTexture()
{
    for (TModelInstanceContainer::iterator iter = m_ModelInstanceContainer.begin();
         iter != m_ModelInstanceContainer.end(); ++iter)
    {
        (*iter)->ReloadTexture();
    }
}

void CDungeonBlock::__Initialize()
{
    m_v3Center = Vector3(0.0f, 0.0f, 0.0f);
    m_fRadius = 0.0f;

    m_pThing = NULL;
}

void CDungeonBlock::Destroy()
{
    if (m_pThing)
        m_pThing = NULL;

    stl_wipe(m_ModelInstanceContainer);

    __Initialize();
}

CDungeonBlock::CDungeonBlock()
{
    __Initialize();
}

CDungeonBlock::~CDungeonBlock()
{
    Destroy();
}
