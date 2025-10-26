#include "Model.h"
#include "Mesh.h"
#include "StdAfx.h"

const CGrannyMaterialPalette &CGrannyModel::GetMaterialPalette() const
{
    return m_kMtrlPal;
}

const CGrannyModel::TMeshNode *CGrannyModel::GetMeshNodeList(CGrannyMesh::EType eMeshType,
                                                             CGrannyMaterial::EType eMtrlType) const
{
    return m_meshNodeLists[eMeshType][eMtrlType];
}

CGrannyMesh *CGrannyModel::GetMeshPointer(int iMesh)
{
    assert(m_meshs != NULL);
    assert(CheckMeshIndex(iMesh));
    return m_meshs + iMesh;
}

const CGrannyMesh *CGrannyModel::GetMeshPointer(int iMesh) const
{
    assert(m_meshs != NULL);
    assert(CheckMeshIndex(iMesh));
    return m_meshs + iMesh;
}

int CGrannyModel::GetMeshCount() const
{
    return m_pgrnModel ? m_pgrnModel->MeshBindingCount : 0;
}

granny_model *CGrannyModel::GetGrannyModelPointer()
{
    return m_pgrnModel;
}

bool CGrannyModel::LoadMeshs()
{
    assert(m_meshs == NULL);
    assert(m_pgrnModel != NULL);

    if (m_pgrnModel->MeshBindingCount <= 0) // 메쉬가 없는 모델
        return true;

    const granny_data_type_definition *rigidType;
    bool isMultiTexCoord = false;
    if (!FindBestRigidVertexFormat(m_pgrnModel, rigidType, isMultiTexCoord)) {
        SPDLOG_ERROR("Failed to find vertex format for model {}", m_pgrnModel->Name);
        return false;
    }

    int vtxRigidPos = 0;
    int vtxDeformPos = 0;
    int idxPos = 0;

    int diffusePNTMeshNodeCount = 0;
    int blendPNTMeshNodeCount = 0;

    m_meshs = new CGrannyMesh[m_pgrnModel->MeshBindingCount];

    for (int m = 0; m < m_pgrnModel->MeshBindingCount; ++m)
    {
        CGrannyMesh &rMesh = m_meshs[m];
        granny_mesh *pgrnMesh = m_pgrnModel->MeshBindings[m].Mesh;

        if (!rMesh.Create(pgrnMesh, m_kMtrlPal, rigidType, isMultiTexCoord))
            return false;

        m_bHaveBlendThing |= rMesh.HaveBlendThing();

        idxPos += GrannyGetMeshIndexCount(pgrnMesh);

        if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_DIFFUSE_PNT))
            ++diffusePNTMeshNodeCount;

        if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_BLEND_PNT))
            ++blendPNTMeshNodeCount;
    }

    m_meshNodeCapacity = diffusePNTMeshNodeCount + blendPNTMeshNodeCount;
    m_meshNodes = new TMeshNode[m_meshNodeCapacity];

    for (int n = 0; n < m_pgrnModel->MeshBindingCount; ++n)
    {
        CGrannyMesh &rMesh = m_meshs[n];
        granny_mesh *pgrnMesh = m_pgrnModel->MeshBindings[n].Mesh;

        CGrannyMesh::EType eMeshType = GrannyMeshIsRigid(pgrnMesh) ? CGrannyMesh::TYPE_RIGID : CGrannyMesh::TYPE_DEFORM;

        if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_DIFFUSE_PNT))
            AppendMeshNode(eMeshType, CGrannyMaterial::TYPE_DIFFUSE_PNT, n);

        if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_BLEND_PNT))
            AppendMeshNode(eMeshType, CGrannyMaterial::TYPE_BLEND_PNT, n);
    }

    return true;
}

bool CGrannyModel::CheckMeshIndex(int iIndex) const
{
    if (iIndex >= 0 && iIndex < m_pgrnModel->MeshBindingCount)
        return true;

    SPDLOG_ERROR("CGrannyModel::CheckMeshIndex> index {0} MeshBindingCount {1} name {2}", iIndex,
                 m_pgrnModel->MeshBindingCount, m_pgrnModel->Name);

    return false;
}

void CGrannyModel::AppendMeshNode(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType, int iMesh)
{
    assert(m_meshNodeSize < m_meshNodeCapacity);

    TMeshNode &rMeshNode = m_meshNodes[m_meshNodeSize++];
    rMeshNode.iMesh = iMesh;
    rMeshNode.pMesh = m_meshs + iMesh;
    rMeshNode.pNextMeshNode = m_meshNodeLists[eMeshType][eMtrlType];

    m_meshNodeLists[eMeshType][eMtrlType] = &rMeshNode;
}

bool CGrannyModel::CreateFromGrannyModelPointer(granny_model *pgrnModel)
{
    assert(IsEmpty());

    m_pgrnModel = pgrnModel;

    if (!LoadMeshs())
        return false;

    return true;
}

bool CGrannyModel::CreateDeviceObjects()
{

    int meshCount = GetMeshCount();

    for (int i = 0; i < meshCount; ++i)
    {
        CGrannyMesh &rMesh = m_meshs[i];
        rMesh.RebuildTriGroupNodeList();
    }

    return true;
}

void CGrannyModel::DestroyDeviceObjects()
{
}

bool CGrannyModel::IsEmpty() const
{
    if (m_pgrnModel)
        return false;

    return true;
}

void CGrannyModel::Destroy()
{
    m_kMtrlPal.Clear();

    delete[] m_meshNodes;
    delete[] m_meshs;

    Initialize();
}

void CGrannyModel::Initialize()
{
    memset(m_meshNodeLists, 0, sizeof(m_meshNodeLists));

    m_pgrnModel = NULL;
    m_meshs = NULL;
    m_meshNodes = NULL;

    m_meshNodeSize = 0;
    m_meshNodeCapacity = 0;

    m_bHaveBlendThing = false;
}

CGrannyModel::CGrannyModel()
{
    Initialize();
}

CGrannyModel::~CGrannyModel()
{
    Destroy();
}
