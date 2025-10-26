#ifndef METIN2_CLIENT_ETERGRNLIB_MESH_HPP
#define METIN2_CLIENT_ETERGRNLIB_MESH_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Material.h"
#include "../EterLib/GrpDevice.h"
#include "../Eterlib/GrpVertexBuffer.h"

class CGrannyMesh
{
public:
    enum EType
    {
        TYPE_RIGID,
        TYPE_DEFORM,
        TYPE_MAX_NUM
    };

    typedef struct STriGroupNode
    {
        STriGroupNode *pNextTriGroupNode;
        int idxPos;
        int triCount;
        uint32_t mtrlIndex;
    } TTriGroupNode;

public:
    CGrannyMesh();
    ~CGrannyMesh();

    bool IsEmpty() const;

    bool Create(granny_mesh *pgrnMesh,
                CGrannyMaterialPalette &matPal,
                const granny_data_type_definition *outputVertexType, bool isMultiTexCoord);

    void Destroy();

    bool IsRigidMesh() const;


    bool IsTwoSide() const;
    bool IsMultiTexture() const { return  m_bHasMultiTexture; };

    int GetVertexCount() const;

    const granny_mesh *GetGrannyMeshPointer() const;
    VertexBufferHandle GetVertexBuffer() const;
    IndexBufferHandle GetIndexBuffer() const;
    const TTriGroupNode *GetTriGroupNodeList(CGrannyMaterial::EType eMtrlType) const;

    void RebuildTriGroupNodeList();

    bool HaveBlendThing()
    {
        return m_bHaveBlendThing;
    }

protected:
    void Initialize();

    bool LoadMaterials(CGrannyMaterialPalette &matPal);
    bool LoadTriGroupNodeList(CGrannyMaterialPalette &matPal);

    // Source Granny mesh
    granny_mesh *m_pgrnMesh;

    // General output vertex format
    // Used by LoadVertices() and DeformPNTVertices
    const granny_data_type_definition *m_outputVertexType;

    // Granny Material Data
    std::vector<uint32_t> m_mtrlIndexVector;

    // TriGroups Data
    TTriGroupNode *m_triGroupNodes;
    TTriGroupNode *m_triGroupNodeLists[CGrannyMaterial::TYPE_MAX_NUM];

    bool m_isTwoSide;
    bool m_bHaveBlendThing;
    bool m_bHasMultiTexture;

    IndexBufferHandle m_hIndexBuffer;
    VertexBufferHandle m_hVertexBuffer;
};

#endif
