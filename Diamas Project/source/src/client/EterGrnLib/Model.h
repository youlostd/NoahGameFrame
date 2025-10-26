#ifndef METIN2_CLIENT_ETERGRNLIB_MODEL_HPP
#define METIN2_CLIENT_ETERGRNLIB_MODEL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Mesh.h"

#include "../Eterlib/GrpVertexBuffer.h"
#include "../Eterlib/GrpIndexBuffer.h"

class CGrannyModel
{
public:
    typedef struct SMeshNode
    {
        int iMesh;
        const CGrannyMesh *pMesh;
        SMeshNode *pNextMeshNode;
    } TMeshNode;

    CGrannyModel();
    ~CGrannyModel();

    bool IsEmpty() const;
    bool CreateFromGrannyModelPointer(granny_model *pgrnModel);
    bool CreateDeviceObjects();
    void DestroyDeviceObjects();
    void Destroy();

    int GetMeshCount() const;
    CGrannyMesh *GetMeshPointer(int iMesh);
    granny_model *GetGrannyModelPointer();
    const CGrannyMesh *GetMeshPointer(int iMesh) const;


    const TMeshNode *GetMeshNodeList(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType) const;


    const CGrannyMaterialPalette &GetMaterialPalette() const;

    void SetFromFilename(const std::string &fromFilename)
    {
        m_fromFilename = fromFilename;
    }

    const std::string &GetFromFilename() const
    {
        return m_fromFilename;
    }

protected:
    bool LoadMeshs();
    void Initialize();

    bool CheckMeshIndex(int iIndex) const;
    void AppendMeshNode(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType, int iMesh);

    std::string m_fromFilename;
    // Granny Data
    granny_model *m_pgrnModel;
    CGrannyMesh *m_meshs;

    TMeshNode *m_meshNodes;
    TMeshNode *m_meshNodeLists[CGrannyMesh::TYPE_MAX_NUM][CGrannyMaterial::TYPE_MAX_NUM];

    int m_meshNodeSize;
    int m_meshNodeCapacity;

    CGrannyMaterialPalette m_kMtrlPal;
    bool m_bHaveBlendThing;
public:
    bool HaveBlendThing()
    {
        return m_bHaveBlendThing;
    }

};

#endif
