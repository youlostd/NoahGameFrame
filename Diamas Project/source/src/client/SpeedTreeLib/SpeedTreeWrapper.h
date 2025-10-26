///////////////////////////////////////////////////////////////////////
//  SpeedTreeRTExample Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//	  Copyright (c) 2001-2003 IDV, Inc.
//	  All Rights Reserved.
//
//	  IDV, Inc.
//	  1233 Washington St. Suite 610
//	  Columbia, SC 29201
//	  Voice: (803) 799-1699
//	  Fax:   (803) 931-0320
//	  Web:   http://www.idvinc.com
//

#ifndef METIN2_CLIENT_SPEEDTREELIB_SPEEDTREEWRAPPER_HPP
#define METIN2_CLIENT_SPEEDTREELIB_SPEEDTREEWRAPPER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#pragma warning(disable : 4786)

///////////////////////////////////////////////////////////////////////
//  Include files

#include "SpeedTreeMaterial.h"

#include <SpeedTreeRT.h>

#include <vector>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)                                                                                                 \
    {                                                                                                                  \
        if (p)                                                                                                         \
        {                                                                                                              \
            delete (p);                                                                                                \
            (p) = NULL;                                                                                                \
        }                                                                                                              \
    }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)                                                                                           \
    {                                                                                                                  \
        if (p)                                                                                                         \
        {                                                                                                              \
            delete[](p);                                                                                               \
            (p) = NULL;                                                                                                \
        }                                                                                                              \
    }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)                                                                                                \
    {                                                                                                                  \
        if (p)                                                                                                         \
        {                                                                                                              \
            (p)->Release();                                                                                            \
            (p) = NULL;                                                                                                \
        }                                                                                                              \
    }
#endif

#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpObjectInstance.h"

class CSpeedTreeRT;

///////////////////////////////////////////////////////////////////////
//  class CSpeedTreeWrapper declaration

METIN2_BEGIN_NS

class CSpeedTreeWrapper : public CGraphicObjectInstance
{
  public:
    enum
    {
        ID = TREE_OBJECT
    };

    int GetType() const
    {
        return ID;
    }

    // Collision Data
    UINT GetCollisionObjectCount();
    void GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType &eType, float *pPosition,
                            float *pDimensions);

    virtual void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector);
    virtual void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance)
    {
    }
    virtual bool OnGetObjectHeight(float fX, float fY, float *pfHeight)
    {
        return false;
    }
    // Bounding Sphere
    virtual bool GetBoundingSphere(Vector3 &v3Center, float &fRadius);

    static bool ms_bSelfShadowOn;

    // methods from CGraphicObjectInstance
    virtual void SetPosition(float x, float y, float z);
    virtual void CalculateBBox();

    // Render 시에 메소드, 그러나 프리뷰나 특수한 경우에만 직접 Render 콜을 부르며
    // 그 이외에는 RenderBranches, RenderFronds 등의 메소드를 CSpeedTreeForest에서 호출한다.
    virtual void OnRender();
    virtual void OnRenderPCBlocker();
    virtual void OnBlendRender()
    {
    }
    virtual void OnRenderToShadowMap(bool showWeapon)
    {
    }
    virtual void OnRenderShadow()
    {
    }

    CSpeedTreeWrapper();
    virtual ~CSpeedTreeWrapper();

    // geometry
    bool LoadTree(const char *pszSptFile, const BYTE *c_pbBlock, unsigned int uiBlockSize, unsigned int nSeed = 1,
                  float fSize = -1.0f, float fSizeVariance = -1.0f);
    const float *GetBoundingBox(void) const
    {
        return m_afBoundingBox;
    }

    // rendering
    void SetupBranchForTreeType(void);
    void SetupFrondForTreeType(void);
    void SetupLeafForTreeType(void);
    void EndLeafForTreeType(void);

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
    void UploadLeafTables(unsigned int uiLocation) const;
#endif

    void RenderBranches(void) const;
    void RenderFronds(void) const;
    void RenderLeaves(void) const;
    void RenderBillboards(void) const;

    // instancing
    CSpeedTreeWrapper **GetInstances(unsigned int &nCount);
    CSpeedTreeWrapper *InstanceOf(void) const
    {
        return m_pInstanceOf;
    }
    CSpeedTreeWrapper *MakeInstance(void);
    void DeleteInstance(CSpeedTreeWrapper *pInstance);
    CSpeedTreeRT *GetSpeedTree(void) const
    {
        return m_pSpeedTree;
    }

    // lighting
    const CSpeedTreeMaterial &GetBranchMaterial(void) const
    {
        return m_cBranchMaterial;
    }
    const CSpeedTreeMaterial &GetFrondMaterial(void) const
    {
        return m_cFrondMaterial;
    }
    const CSpeedTreeMaterial &GetLeafMaterial(void) const
    {
        return m_cLeafMaterial;
    }
    float GetLeafLightingAdjustment(void) const
    {
        return m_pSpeedTree->GetLeafLightingAdjustment();
    }

    // wind
    void Advance(void);

    // utility
    void CleanUpMemory(void);

  private:
    void SetupBuffers(void);
    void SetupBranchBuffers(void);
    void SetupFrondBuffers(void);
    void SetupLeafBuffers(void);
    void PositionTree(void) const;
    static bool LoadTexture(const std::string &treePath, const char *filename, CGraphicImageInstance &image);
    void SetShaderConstants(const float *pMaterial) const;

    // SpeedTreeRT data
    CSpeedTreeRT *m_pSpeedTree;                    // SpeedTree object
    CSpeedTreeRT::STextures *m_pTextureInfo;       // cached texture info
    bool m_bIsInstance;                            // is this tree an instance?
    std::vector<CSpeedTreeWrapper *> m_vInstances; // list of instances of this tree
    CSpeedTreeWrapper *m_pInstanceOf;              // this tree is an instance of what

    // geometry cache
    CSpeedTreeRT::SGeometry *m_pGeometryCache; // geometry cache

    // branch buffers
    VertexBufferHandle m_pBranchVertexBuffer; // branch vertex buffer
    unsigned int m_unBranchVertexCount;            // number of vertices in branches
    IndexBufferHandle m_pBranchIndexBuffer;   // branch index buffer
    unsigned short *m_pBranchIndexCounts;          // number of indexes per branch LOD level

    // frond buffers
    VertexBufferHandle m_pFrondVertexBuffer; // frond vertex buffer
    unsigned int m_unFrondVertexCount;            // number of vertices in frond
    unsigned int m_unNumFrondLods;                // number of frond LODs
    std::vector<IndexBufferHandle> m_pFrondIndexBuffers; // frond index buffers
    unsigned short *m_pFrondIndexCounts;          // number of indexes per frond LOD level

    // leaf buffers
    unsigned short m_usNumLeafLods;               // the number of leaf LODs
    std::vector<VertexBufferHandle> m_pLeafVertexBuffers; // leaf vertex buffer
    bool *m_pLeavesUpdatedByCpu;                  // stores which LOD's have been updated already per frame

    // tree properties
    float m_afPos[3];         // tree position
    float m_afBoundingBox[6]; // tree's bounding box

    // materials
    CSpeedTreeMaterial m_cBranchMaterial; // branch material
    CSpeedTreeMaterial m_cLeafMaterial;   // leaf material
    CSpeedTreeMaterial m_cFrondMaterial;  // frond material

    // branch texture
    CGraphicImageInstance m_BranchImageInstance; // branch texture object
    CGraphicImageInstance m_ShadowImageInstance; // shadow texture object
    CGraphicImageInstance m_CompositeImageInstance;

    // texture loading
    static unsigned int m_unNumWrappersActive;
    static LPDIRECT3DVERTEXSHADER9 ms_lpBranchVertexShader;
    static LPDIRECT3DVERTEXSHADER9 ms_lpLeafVertexShader;
};

METIN2_END_NS

#endif
