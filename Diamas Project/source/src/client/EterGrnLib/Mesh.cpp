#include "Mesh.h"
#include "StdAfx.h"

#include "../EterLib/Engine.hpp"
#include "GrannyState.hpp"
#include "Material.h"
#include "Model.h"

CGrannyMesh::CGrannyMesh()
{
    Initialize();
}

CGrannyMesh::~CGrannyMesh()
{
    Destroy();
}

void CGrannyMesh::Initialize()
{
    for (int r = 0; r < CGrannyMaterial::TYPE_MAX_NUM; ++r)
        m_triGroupNodeLists[r] = NULL;

    m_outputVertexType = GrannyPNGBT33332VertexType;
    m_pgrnMesh = NULL;

    m_triGroupNodes = NULL;
    m_hVertexBuffer = ENGINE_INVALID_HANDLE;
    m_hIndexBuffer = ENGINE_INVALID_HANDLE;

    m_isTwoSide = false;
    m_bHaveBlendThing = false;
}

void CGrannyMesh::Destroy()
{
    if (m_triGroupNodes)
        delete[] m_triGroupNodes;

    m_mtrlIndexVector.clear();

    Engine::GetDevice().DeleteVertexBuffer(m_hVertexBuffer);
    Engine::GetDevice().DeleteIndexBuffer(m_hIndexBuffer);

    Initialize();
}

bool CGrannyMesh::IsRigidMesh() const
{
    return GrannyMeshIsRigid(m_pgrnMesh);
}

const granny_mesh* CGrannyMesh::GetGrannyMeshPointer() const
{
    return m_pgrnMesh;
}

VertexBufferHandle CGrannyMesh::GetVertexBuffer() const
{
    return m_hVertexBuffer;
}
IndexBufferHandle CGrannyMesh::GetIndexBuffer() const
{
    return m_hIndexBuffer;
}

const CGrannyMesh::TTriGroupNode*
CGrannyMesh::GetTriGroupNodeList(CGrannyMaterial::EType eMtrlType) const
{
    return m_triGroupNodeLists[eMtrlType];
}

int CGrannyMesh::GetVertexCount() const
{
    assert(m_pgrnMesh != NULL);
    return GrannyGetMeshVertexCount(m_pgrnMesh);
}

bool CGrannyMesh::IsEmpty() const
{
    if (m_pgrnMesh)
        return false;

    return true;
}

bool CGrannyMesh::Create(granny_mesh* pgrnMesh, CGrannyMaterialPalette& matPal,
                         const granny_data_type_definition* outputVertexType,
                         bool isMultiTexCoord)
{
    assert(IsEmpty());

    m_pgrnMesh = pgrnMesh;
    m_outputVertexType = outputVertexType;

    if (m_pgrnMesh->BoneBindingCount < 0)
        return true;

    D3DFORMAT IndexFormat;
    int IndexCount = GrannyGetMeshIndexCount(pgrnMesh);
    if (GrannyGetMeshBytesPerIndex(pgrnMesh) == 2) {
        IndexFormat = D3DFMT_INDEX16;
    } else {
        assert(GrannyGetMeshBytesPerIndex(pgrnMesh) == 4);
        IndexFormat = D3DFMT_INDEX32;
    }

    m_hIndexBuffer = Engine::GetDevice().CreateIndexBuffer(
        IndexCount, IndexFormat, GrannyGetMeshIndices(pgrnMesh),
        (D3DUSAGE_WRITEONLY));
    if (!isValid(m_hIndexBuffer)) {
        SPDLOG_ERROR("Failed to create index buffer for mesh {}",
                     pgrnMesh->Name);
        return false;
    }

    granny_data_type_definition* VertexFormat;
    int VertexBufferSize;
    int VertexSize;

    if (!isMultiTexCoord) {
        if (GrannyMeshIsRigid(pgrnMesh)) {
            VertexFormat = GrannyPNGBT33332VertexType;
            VertexBufferSize = (sizeof(granny_pngbt33332_vertex) *
                                GrannyGetMeshVertexCount(pgrnMesh));
            VertexSize = sizeof(granny_pngbt33332_vertex);
        } else {
            VertexFormat = GrannyPWNGBT343332VertexType;
            VertexBufferSize = (sizeof(granny_pwngbt343332_vertex) *
                                GrannyGetMeshVertexCount(pgrnMesh));
            VertexSize = sizeof(granny_pwngbt343332_vertex);
        }
    } else {
        m_bHasMultiTexture = true;
        if (GrannyMeshIsRigid(pgrnMesh)) {
            VertexFormat = GrannyPNGBT333322VertexType;
            VertexBufferSize = (sizeof(granny_pngbt333322_vertex) *
                                GrannyGetMeshVertexCount(pgrnMesh));
            VertexSize = sizeof(granny_pngbt333322_vertex);
        } else {
            VertexFormat = GrannyPWNGBT3433322VertexType;
            VertexBufferSize = (sizeof(granny_pwngbt3433322_vertex) *
                                GrannyGetMeshVertexCount(pgrnMesh));
            VertexSize = sizeof(granny_pwngbt3433322_vertex);
        }
    }

    m_hVertexBuffer = Engine::GetDevice().CreateVertexBuffer(
        VertexBufferSize, VertexSize, NULL, (D3DUSAGE_WRITEONLY));
    if (!isValid(m_hVertexBuffer)){
        SPDLOG_ERROR("Failed to create vertex buffer for mesh {}",
                     pgrnMesh->Name);
        return false;
    }

    void* dstVertices = Engine::GetDevice().LockVertexBuffer(m_hVertexBuffer);
    GrannyCopyMeshVertices(pgrnMesh, VertexFormat, dstVertices);
    Engine::GetDevice().UnlockVertexBuffer(m_hVertexBuffer);

    /*void *pVertex = Engine::GetDevice().LockVertexBuffer(m_hVertexBuffer,
    D3DLOCK_NOOVERWRITE, 0, VertexBufferSize); if (pVertex)
    {
        GrannyGetMeshVertices(pgrnMesh, VertexFormat, pVertex);
    }
    Engine::GetDevice().UnlockVertexBuffer(m_hVertexBuffer);
    */
    // Two Side Mesh

    if (!strncmp(m_pgrnMesh->Name, "2x", 2))
        m_isTwoSide = true;

    if (!LoadMaterials(matPal))
        return false;

    if (!LoadTriGroupNodeList(matPal))
        return false;

    return true;
}

bool compareByMaterial(const CGrannyMesh::TTriGroupNode* a,
                       const CGrannyMesh::TTriGroupNode* b)
{
    return (a && b) && a->mtrlIndex < b->mtrlIndex;
}

bool CGrannyMesh::LoadTriGroupNodeList(CGrannyMaterialPalette& matPal)
{
    assert(m_pgrnMesh != NULL);
    assert(m_triGroupNodes == NULL);

    int mtrlCount = m_pgrnMesh->MaterialBindingCount;
    if (mtrlCount <= 0) // 천의 동굴 2층 크래쉬 발생
        return true;

    int GroupNodeCount = GrannyGetMeshTriangleGroupCount(m_pgrnMesh);
    if (GroupNodeCount <= 0)
        return true;

    m_triGroupNodes = new TTriGroupNode[GroupNodeCount];

    const granny_tri_material_group* grnTriGroups =
        GrannyGetMeshTriangleGroups(m_pgrnMesh);

    for (int g = 0; g < GroupNodeCount; ++g) {
        const granny_tri_material_group& grnTriGroup = grnTriGroups[g];
        TTriGroupNode& triGroupNode = m_triGroupNodes[g];

        triGroupNode.idxPos = grnTriGroup.TriFirst * 3;
        triGroupNode.triCount = grnTriGroup.TriCount;

        int iMtrl = grnTriGroup.MaterialIndex;
        if (iMtrl < 0 || iMtrl >= mtrlCount)
            triGroupNode.mtrlIndex = 0;
        else
            triGroupNode.mtrlIndex = m_mtrlIndexVector[iMtrl];

        const CGrannyMaterial& mat =
            matPal.GetMaterialRef(triGroupNode.mtrlIndex);

        triGroupNode.pNextTriGroupNode = m_triGroupNodeLists[mat.GetType()];
        m_triGroupNodeLists[mat.GetType()] = &triGroupNode;
    }

    std::sort(std::begin(m_triGroupNodeLists), std::end(m_triGroupNodeLists),
              compareByMaterial);
    return true;
}

void CGrannyMesh::RebuildTriGroupNodeList()
{
    assert(!"CGrannyMesh::RebuildTriGroupNodeList() - 왜 리빌드를 하는가- -?");
    /*
    int mtrlCount = m_pgrnMesh->MaterialBindingCount;
    int GroupNodeCount = GrannyGetMeshTriangleGroupCount(m_pgrnMesh);

    if (GroupNodeCount <= 0)
        return;

    const granny_tri_material_group * c_pgrnTriGroups =
    GrannyGetMeshTriangleGroups(m_pgrnMesh);

    for (int g = 0; g < GroupNodeCount; ++g)
    {
        const granny_tri_material_group& c_rgrnTriGroup = c_pgrnTriGroups[g];
        TTriGroupNode * pTriGroupNode = m_triGroupNodes + g;

        int iMtrl = c_rgrnTriGroup.MaterialIndex;

        if (iMtrl >= 0 && iMtrl < mtrlCount)
        {
            CGrannyMaterial & rMtrl = m_mtrls[iMtrl];

            pTriGroupNode->lpd3dTextures[0] = rMtrl.GetD3DTexture(0);
            pTriGroupNode->lpd3dTextures[1] = rMtrl.GetD3DTexture(1);

        }
    }
    */
}

bool CGrannyMesh::LoadMaterials(CGrannyMaterialPalette& matPal)
{
    assert(m_pgrnMesh != NULL);

    if (m_pgrnMesh->MaterialBindingCount <= 0)
        return true;

    bool bHaveBlendThing = false;

    int mtrlCount = m_pgrnMesh->MaterialBindingCount;
    for (int m = 0; m < mtrlCount; ++m) {
        granny_material* pgrnMaterial =
            m_pgrnMesh->MaterialBindings[m].Material;

        uint32_t index = matPal.RegisterMaterial(pgrnMaterial);
        m_mtrlIndexVector.push_back(index);

        if (matPal.GetMaterialRef(index).GetType() ==
            CGrannyMaterial::TYPE_BLEND_PNT)
            bHaveBlendThing = true;
    }

    m_bHaveBlendThing = bHaveBlendThing;
    return true;
}

bool CGrannyMesh::IsTwoSide() const
{
    return m_isTwoSide;
}
