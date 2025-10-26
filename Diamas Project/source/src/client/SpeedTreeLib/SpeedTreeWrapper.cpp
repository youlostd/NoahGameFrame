///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper Class
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
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//

///////////////////////////////////////////////////////////////////////
//  Include Files

#include "SpeedTreeConfig.h"
#include "SpeedTreeForestDirectX9.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"
#include "VertexShaders.h"
#include "SpeedTreeWrapper.h"

#include "../EterLib/StateManager.h"
#include "../EterLib/ResourceManager.h"
#include "../EterBase/Filename.h"
#include "../EterLib/Engine.hpp"


#include <cstdlib>
#include <cstdio>
#include <SpeedTreeRT.h>

METIN2_BEGIN_NS

///////////////////////////////////////////////////////////////////////
//  Static member variable declarations

unsigned int            CSpeedTreeWrapper::m_unNumWrappersActive   = 0;
bool                    CSpeedTreeWrapper::ms_bSelfShadowOn        = true;
LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_lpBranchVertexShader = nullptr;
LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_lpLeafVertexShader   = nullptr;

///////////////////////////////////////////////////////////////////////
//  Color conversion macro

#define AGBR2ARGB(dwColor) (dwColor & 0xff00ff00) + ((dwColor & 0x00ff0000) >> 16) + ((dwColor & 0x000000ff) << 16)


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper( ) :
    m_pSpeedTree(new CSpeedTreeRT),
    m_pTextureInfo(nullptr),
    m_bIsInstance(false),
    m_pInstanceOf(nullptr),
    m_pGeometryCache(nullptr),
	m_pBranchVertexBuffer{ENGINE_INVALID_HANDLE},
    m_unBranchVertexCount(0),
    m_pBranchIndexBuffer(ENGINE_INVALID_HANDLE),
    m_pBranchIndexCounts(nullptr),
    m_pFrondVertexBuffer{ENGINE_INVALID_HANDLE},
    m_unFrondVertexCount(0),
    m_unNumFrondLods(0),
    m_pFrondIndexBuffers(NULL),
    m_pFrondIndexCounts(nullptr),
	m_usNumLeafLods(0),
	m_pLeafVertexBuffers(NULL),
	m_pLeavesUpdatedByCpu(nullptr)
{
    m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    m_unNumWrappersActive++;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
	// 하나만 렌더링 할 때는 LOD 사용하지 않음
	m_pSpeedTree->SetLodLevel(1.0f);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	DWORD dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	DWORD dwAlphaBlendEnable = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);
 	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, true);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
 	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, false);

	// choose fixed function pipeline or custom shader for fronds and branches
	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
	STATEMANAGER.SetVertexShader(ms_lpBranchVertexShader);

	//SetupBranchForTreeType();
	{
		// update the branch geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

		if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
		{
			// update the vertex array
			SFVFBranchVertex* pVertexBuffer = NULL;
			m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unBranchVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pBranchVertexBuffer->Unlock();
		}
#endif

		CGraphicTexture* texture;

		// set texture map
		if ((texture = m_BranchImageInstance.GetTexturePointer()))
			STATEMANAGER.SetTexture(0, texture->GetD3DTexture());

		if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
		{
            // activate the branch vertex buffer
            Engine::GetDevice().SetVertexBuffer(m_pBranchVertexBuffer, 0, 0);
			// set the index buffer
            Engine::GetDevice().SetIndexBuffer(m_pBranchIndexBuffer);
			RenderBranches();
		}
	}

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//SetupFrondForTreeType();
	{
		// update the frond geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
		if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
		{
			// update the vertex array
			SFVFBranchVertex * pVertexBuffer = NULL;
			m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unFrondVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pFrondVertexBuffer->Unlock();
		}
#endif
		if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0 &&
			m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1 &&
			m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
		{
			if (!m_CompositeImageInstance.IsEmpty())
				STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());


		    Engine::GetDevice().SetVertexBuffer(m_pFrondVertexBuffer, 0, 0);
            Engine::GetDevice().SetIndexBuffer(m_pFrondIndexBuffers[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel]);
			RenderFronds();
		}
	}

	//SetupLeafForTreeType();
	{
		Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(ms_lpLeafVertexShader);

		// pass leaf tables to shader
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		UploadLeafTables(c_nVertexShader_LeafTables);
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		RenderLeaves();
		EndLeafForTreeType();
	}

	STATEMANAGER.SetVertexShader(nullptr);
	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, false);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);
 	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}

void CSpeedTreeWrapper::OnRender()
{
	// 하나만 렌더링 할 때는 LOD 사용하지 않음
	m_pSpeedTree->SetLodLevel(1.0f);
	//Advance();

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, false);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, true);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, true);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, false);

	// choose fixed function pipeline or custom shader for fronds and branches
	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
	STATEMANAGER.SetVertexShader(ms_lpBranchVertexShader);

	SetupBranchForTreeType();
	RenderBranches();

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	SetupFrondForTreeType();
	RenderFronds();

	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
	STATEMANAGER.SetVertexShader(ms_lpLeafVertexShader);

	SetupLeafForTreeType();
	RenderLeaves();
	EndLeafForTreeType();

	STATEMANAGER.SetVertexShader(nullptr);
	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, false);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

// collision detection routines
UINT CSpeedTreeWrapper::GetCollisionObjectCount()
{
	assert(m_pSpeedTree);
	return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	assert(m_pSpeedTree);
	m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}

// pscdVector may be null
void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector * /*pscdVector*/)
{
	Matrix mat = Matrix::CreateTranslation(m_afPos[0], m_afPos[1], m_afPos[2]);

	/////
	for (UINT i = 0; i < GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType ObjectType;
		CStaticCollisionData CollisionData;

		GetCollisionObject(i, ObjectType, (float * )&CollisionData.v3Position, CollisionData.fDimensions);

		if (ObjectType == CSpeedTreeRT::CO_BOX)
			continue;

		switch(ObjectType)
		{
		case CSpeedTreeRT::CO_SPHERE:
			CollisionData.dwType = COLLISION_TYPE_SPHERE;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

		case CSpeedTreeRT::CO_CYLINDER:
			CollisionData.dwType = COLLISION_TYPE_CYLINDER;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			CollisionData.fDimensions[1] = CollisionData.fDimensions[1] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

			/*case CSpeedTreeRT::CO_BOX:
			break;*/
		}
		AddCollision(&CollisionData, &mat);
	}
}

bool CSpeedTreeWrapper::GetBoundingSphere(Vector3 & v3Center, float & fRadius)
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	v3Center.x = 0.0f;
	v3Center.y = 0.0f;
	v3Center.z = fZ * 0.5f;

	fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f; // 0.9f for reduce size

	Vector3 vec{m_pSpeedTree->GetTreePosition()};

	v3Center+=vec;

	return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	m_v3BBoxMin.x = -fX / 2.0f;
	m_v3BBoxMin.y = -fY / 2.0f;
	m_v3BBoxMin.z = 0.0f;
	m_v3BBoxMax.x = fX / 2.0f;
	m_v3BBoxMax.y = fY / 2.0f;
	m_v3BBoxMax.z = fZ;

	m_v4TBBox[0] = Vector4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = Vector4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = Vector4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = Vector4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = Vector4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = Vector4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = Vector4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = Vector4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const Matrix & c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
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
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper( )
{
    // if this is not an instance, then clean up
    if (!m_bIsInstance)
    {
        if (m_unBranchVertexCount > 0)
        {
            Engine::GetDevice().DeleteVertexBuffer(m_pBranchVertexBuffer);
            Engine::GetDevice().DeleteIndexBuffer(m_pBranchIndexBuffer);
            SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
        }

        if (m_unFrondVertexCount > 0)
        {
            Engine::GetDevice().DeleteVertexBuffer(m_pFrondVertexBuffer);
			for (unsigned int i = 0; i < m_unNumFrondLods; ++i)
				if (m_pFrondIndexCounts[i] > 0)
					Engine::GetDevice().DeleteIndexBuffer(m_pFrondIndexBuffers[i]);

            m_pFrondIndexBuffers.clear();
            SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
        }

        for (unsigned short i = 0; i < m_usNumLeafLods; ++i)
        {
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

            if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
                Engine::GetDevice().DeleteVertexBuffer(m_pLeafVertexBuffers[i]);
        }

		SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
        m_pLeafVertexBuffers.clear();

        SAFE_DELETE(m_pTextureInfo);
		SAFE_DELETE(m_pGeometryCache);
    }

    // always delete the SpeedTree
    SAFE_DELETE(m_pSpeedTree);

    --m_unNumWrappersActive;
    //#ifdef WRAPPER_RENDER_SELF_SHADOWS
    //    if (m_unNumWrappersActive == 0)
    //        SAFE_RELEASE(m_texShadow);
    //#endif

    Clear();
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, const BYTE * c_pbBlock = nullptr, unsigned int uiBlockSize = 0, unsigned int nSeed, float fSize, float fSizeVariance)
{
    // directx, so allow for flipping of the texture coordinate
    #ifdef WRAPPER_FLIP_T_TEXCOORD
        m_pSpeedTree->SetTextureFlip(true);
    #endif

    // load the tree file
    if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
	{
		if (!m_pSpeedTree->LoadTree(pszSptFile))
		{
			SPDLOG_ERROR("SpeedTreeRT Error: {0}", CSpeedTreeRT::GetCurrentError());
			return false;
		}
	}

    // override the lighting method stored in the spt file
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
    #else
        m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
    #endif

    // set the wind method
    #ifdef WRAPPER_USE_GPU_WIND
        m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
        m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
        m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
    #endif
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_CPU);
        m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_CPU);
        m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_CPU);
    #endif
    #ifdef WRAPPER_USE_NO_WIND
        m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
        m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
        m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
    #endif

    m_pSpeedTree->SetNumLeafRockingGroups(1);

    // override the size, if necessary
    if (fSize >= 0.0f && fSizeVariance >= 0.0f)
        m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

    // generate tree geometry
    if (m_pSpeedTree->Compute(nullptr, nSeed))
    {
        // get the dimensions
        m_pSpeedTree->GetBoundingBox(m_afBoundingBox);

        // make the leaves rock in the wind
        m_pSpeedTree->SetLeafRockingState(true);

        // billboard setup
        #ifdef WRAPPER_BILLBOARD_MODE
            CSpeedTreeRT::SetDropToBillboard(true);
        #else
            CSpeedTreeRT::SetDropToBillboard(false);
        #endif

        // query & set materials
        m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial( ));
        m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial( ));
        m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial( ));

        // adjust lod distances
        float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
        m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);

        // query textures
        m_pTextureInfo = new CSpeedTreeRT::STextures;
        m_pSpeedTree->GetTextures(*m_pTextureInfo);

		// load branch textures
		std::string filename = pszSptFile;

		if (!LoadTexture(filename, m_pTextureInfo->m_pBranchTextureFilename, m_BranchImageInstance))
			return false;

#ifdef WRAPPER_RENDER_SELF_SHADOWS
		if (!LoadTexture(filename, m_pTextureInfo->m_pSelfShadowFilename, m_ShadowImageInstance))
			return false;
#endif

		if (!LoadTexture(filename, m_pTextureInfo->m_pCompositeFilename, m_CompositeImageInstance))
			return false;

        // setup the vertex and index buffers
        SetupBuffers( );
    } else {
        SPDLOG_ERROR("Failed to compute tree {0} with {1}",
                  pszSptFile,
                  CSpeedTreeRT::GetCurrentError());
    }

    return true;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
    // read all the geometry for highest LOD into the geometry cache
    m_pSpeedTree->SetLodLevel(1.0f);
    if (m_pGeometryCache == nullptr)
        m_pGeometryCache = new CSpeedTreeRT::SGeometry;
    m_pSpeedTree->GetGeometry(*m_pGeometryCache);

    // setup the buffers for each tree part
    SetupBranchBuffers( );
    SetupFrondBuffers( );
    SetupLeafBuffers( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
    // reference to the branch structure
    CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
    m_unBranchVertexCount = pBranches->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has branches
    if (m_unBranchVertexCount > 1)
    {
		// create the vertex buffer for storing branch vertices
        SFVFBranchVertex * pVertexBuffer = nullptr;

#ifndef WRAPPER_USE_CPU_WIND
        m_pBranchVertexBuffer = Engine::GetDevice().CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), sizeof(SFVFBranchVertex), nullptr, D3DUSAGE_WRITEONLY);
        // fill the vertex buffer by interleaving SpeedTree data
        pVertexBuffer = (SFVFBranchVertex*)Engine::GetDevice().LockVertexBuffer(m_pBranchVertexBuffer);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pBranchVertexBuffer, NULL);
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
        for (unsigned int i = 0; i < m_unBranchVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

			// normal or color
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
			memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));
#else
			pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pBranches->m_pColors[i]);
#endif

			// texcoords for layer 0
			pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
			pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

			// texcoords for layer 1 (if enabled)
#ifdef WRAPPER_RENDER_SELF_SHADOWS
			pVertexBuffer->m_fShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
			pVertexBuffer->m_fShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];
#endif

			// gpu wind data
#ifdef WRAPPER_USE_GPU_WIND
			pVertexBuffer->m_fWindIndex = 4.0f * pBranches->m_pWindMatrixIndices[i];
			pVertexBuffer->m_fWindWeight = (float)pBranches->m_pWindWeights[i];
#endif

            ++pVertexBuffer;
        }
        Engine::GetDevice().UnlockVertexBuffer(m_pBranchVertexBuffer);

        // create and fill the index counts for each LOD
        unsigned int unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels( );
        m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
        for (unsigned int i = 0; i < unNumLodLevels; ++i)
        {
            // force geometry update for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, (short)i);

            // check if this LOD has branches
            if (pBranches->m_usNumStrips > 0)
                m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
            else
                m_pBranchIndexCounts[i] = 0;
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);

        // the first LOD level contains the most indices of all the levels, so
        // we use its size to allocate the index buffer
        m_pBranchIndexBuffer = Engine::GetDevice().CreateIndexBuffer(m_pBranchIndexCounts[0], D3DFMT_INDEX16, pBranches->m_pStrips[0], D3DUSAGE_WRITEONLY);

    }
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
    // reference to frond structure
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
    m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has fronds
    if (m_unFrondVertexCount > 1)
    {
        // fill the vertex buffer by interleaving SpeedTree data
        SFVFBranchVertex* pVertexBuffer = nullptr;
        // create the vertex buffer for storing frond vertices
        #ifndef WRAPPER_USE_CPU_WIND
            m_pFrondVertexBuffer = Engine::GetDevice().CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex),  sizeof(SFVFBranchVertex), nullptr,  D3DUSAGE_WRITEONLY);
            pVertexBuffer = (SFVFBranchVertex*)Engine::GetDevice().LockVertexBuffer(m_pFrondVertexBuffer,  D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
        #else
            ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pFrondVertexBuffer, NULL);
	        m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        #endif

        for (unsigned short i = 0; i < m_unFrondVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

            // normal or color
            #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));
            #else
                pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pFronds->m_pColors[i]);
            #endif

            // texcoords for layer 0
            pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
            pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

            // texcoords for layer 1 (if enabled)
            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                pVertexBuffer->m_fShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
                pVertexBuffer->m_fShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];
            #endif

            // gpu wind data
            #ifdef WRAPPER_USE_GPU_WIND
                pVertexBuffer->m_fWindIndex = 4.0f * pFronds->m_pWindMatrixIndices[i];
                pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];
            #endif

            ++pVertexBuffer;
        }
        Engine::GetDevice().UnlockVertexBuffer(m_pFrondVertexBuffer);

        // create and fill the index counts for each LOD
        m_unNumFrondLods = m_pSpeedTree->GetNumFrondLodLevels( );
        m_pFrondIndexCounts = new unsigned short[m_unNumFrondLods];
		m_pFrondIndexBuffers.clear();
        m_pFrondIndexBuffers.resize(m_unNumFrondLods);

        for (unsigned short i = 0; i < m_unNumFrondLods; ++i)
        {
            // force update of geometry for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, i);

            // check if this LOD has fronds
            if (pFronds->m_usNumStrips > 0)
                m_pFrondIndexCounts[i] = pFronds->m_pStripLengths[0];
            else
                m_pFrondIndexCounts[i] = 0;

			if (m_pFrondIndexCounts[i] > 0)
			{
				m_pFrondIndexBuffers[i] = Engine::GetDevice().CreateIndexBuffer(m_pFrondIndexCounts[i], D3DFMT_INDEX16, pFronds->m_pStrips[0], D3DUSAGE_WRITEONLY);

			}
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
    // set up constants
    const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };

    // set up the leaf counts for each LOD
    m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels( );

    // create arrays for the number of LOD levels
  //  m_pLeafVertexBuffers = new LPDIRECT3DVERTEXBUFFER9[m_usNumLeafLods];
    m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];
    m_pLeafVertexBuffers.resize(m_usNumLeafLods);

    for (unsigned int unLod = 0; unLod < m_usNumLeafLods; ++unLod)
    {
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, unLod);

        m_pLeavesUpdatedByCpu[unLod] = false;

        // if this level has no leaves, then skip it
        unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;
        if (usLeafCount < 1)
            continue;

        SFVFLeafVertex* pVertexBuffer = nullptr;
        // create the vertex buffer for storing leaf vertices
		#ifndef WRAPPER_USE_CPU_LEAF_PLACEMENT
			ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_MANAGED, &m_pLeafVertexBuffer[unLod], NULL);
	        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        #else
            m_pLeafVertexBuffers[unLod] = Engine::GetDevice().CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), sizeof(SFVFLeafVertex));
            pVertexBuffer = (SFVFLeafVertex*)Engine::GetDevice().LockVertexBuffer( m_pLeafVertexBuffers[unLod], D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
        #endif

        // fill the vertex buffer by interleaving SpeedTree data
        SFVFLeafVertex* pVertex = pVertexBuffer;
        for (unsigned int unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
        {
            const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
            for (unsigned int unVert = 0; unVert < 6; ++unVert)  // 6 verts == 2 triangles
            {
                // position
                memcpy(&pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

                #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                    // normal
                    memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
                #else
                    // color
                    pVertex->m_dwDiffuseColor = AGBR2ARGB(pLeaf->m_pColors[unLeaf]);
                #endif

                // tex coord
                memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));

                // wind weights
                #ifdef WRAPPER_USE_GPU_WIND
                    pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
                    pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
                #endif

                // gpu leaf placement data
                #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                    pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
                    pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments( )[unLod];
                #endif

                ++pVertex;
            }

        }
        Engine::GetDevice().UnlockVertexBuffer(m_pLeafVertexBuffers[unLod]);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
    // compute LOD level (based on distance from camera)
    m_pSpeedTree->ComputeLodLevel( );
	//m_pSpeedTree->SetLodLevel(1.0f); // Metin2 : LOD 계산하지도 않고 사용하지 않음

    // compute wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->ComputeWindEffects(true, true, true);
    #endif
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    // create a new object
    CSpeedTreeWrapper* pInstance = new CSpeedTreeWrapper;
    SAFE_DELETE(pInstance->m_pSpeedTree);

    // make an instance of this object's SpeedTree
    pInstance->m_bIsInstance = true;
    pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance( );
    if (pInstance->m_pSpeedTree)
    {
        // use the same materials
        pInstance->m_cBranchMaterial = m_cBranchMaterial;
        pInstance->m_cLeafMaterial = m_cLeafMaterial;
        pInstance->m_cFrondMaterial = m_cFrondMaterial;
		pInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
		pInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());

		if (!m_ShadowImageInstance.IsEmpty())
			pInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());
        pInstance->m_pTextureInfo = m_pTextureInfo;

        // use the same geometry cache
        pInstance->m_pGeometryCache = m_pGeometryCache;

        // use the same buffers
        pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
        pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
        pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
        pInstance->m_unBranchVertexCount = m_unBranchVertexCount;

        pInstance->m_pFrondIndexBuffers = m_pFrondIndexBuffers;
		pInstance->m_unNumFrondLods = m_unNumFrondLods;
        pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
        pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
        pInstance->m_unFrondVertexCount = m_unFrondVertexCount;

        pInstance->m_pLeafVertexBuffers = m_pLeafVertexBuffers;
        pInstance->m_usNumLeafLods = m_usNumLeafLods;
        pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;

        // new stuff
        memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
        memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
        pInstance->m_pInstanceOf = this;
        m_vInstances.push_back(pInstance);
    }
    else
    {
        fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError( ));
        delete pInstance;
        pInstance = nullptr;
    }

    return pInstance;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::GetInstances

CSpeedTreeWrapper** CSpeedTreeWrapper::GetInstances(unsigned int& nCount)
{
    nCount = m_vInstances.size( );
	return nCount ? &(m_vInstances[0]) : nullptr;
}

void CSpeedTreeWrapper::DeleteInstance(CSpeedTreeWrapper * pInstance)
{
	std::vector<CSpeedTreeWrapper *>::iterator itor = m_vInstances.begin();

	while (itor != m_vInstances.end())
	{
		if (*itor == pInstance)
		{
			delete pInstance;
			itor = m_vInstances.erase(itor);
		}
		else
			++itor;
	}
}
///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupBranchForTreeType

void CSpeedTreeWrapper::SetupBranchForTreeType(void)
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cBranchMaterial.Activate(ms_lpd3dDevice);
        SetShaderConstants(m_pSpeedTree->GetBranchMaterial( ));
    #endif

    // update the branch geometry for CPU wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);
        if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
        {
            // update the vertex array
            SFVFBranchVertex* pVertexBuffer = NULL;
            m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
            for (unsigned int i = 0; i < m_unBranchVertexCount; ++i)
            {
                memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
            }
            m_pBranchVertexBuffer->Unlock( );
        }
    #endif

	CGraphicTexture* texture;

    // set texture map
    if ((texture = m_BranchImageInstance.GetTexturePointer()))
        STATEMANAGER.SetTexture(0, texture->GetD3DTexture());

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
		if (ms_bSelfShadowOn &&
		    (texture = m_ShadowImageInstance.GetTexturePointer()))
			STATEMANAGER.SetTexture(1, texture->GetD3DTexture());
		else
			STATEMANAGER.SetTexture(1, nullptr);
    #endif

    // if tree has branches, bind the buffers
    if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
    {
        Engine::GetDevice().SetVertexBuffer(m_pBranchVertexBuffer, 0, 0);
        Engine::GetDevice().SetIndexBuffer(m_pBranchIndexBuffer);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::RenderBranches

void CSpeedTreeWrapper::RenderBranches(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

    if (m_pGeometryCache->m_fBranchAlphaTestValue > 0.0f)
    {
        PositionTree( );

        // set alpha test value
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));

        // if tree has branches at this LOD, draw them
        if (m_pBranchIndexCounts &&
			m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1 &&
			m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
		{
			ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
		}
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupFrondForTreeType

void CSpeedTreeWrapper::SetupFrondForTreeType(void)
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cFrondMaterial.Activate(ms_lpd3dDevice);
        SetShaderConstants(m_pSpeedTree->GetFrondMaterial( ));
    #endif

    // update the frond geometry for CPU wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
        if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
        {
            // update the vertex array
            SFVFBranchVertex* pVertexBuffer = NULL;
            m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
            for (unsigned int i = 0; i < m_unFrondVertexCount; ++i)
            {
                memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
            }
            m_pFrondVertexBuffer->Unlock( );
        }
    #endif

	CGraphicTexture* texture;

	if ((texture = m_CompositeImageInstance.GetTexturePointer()))
		STATEMANAGER.SetTexture(0, texture->GetD3DTexture());

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
		if ((texture = m_ShadowImageInstance.GetTexturePointer()))
			STATEMANAGER.SetTexture(1, texture->GetD3DTexture());
    #endif

    // if tree has fronds, bind the buffers
    if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
    {
        Engine::GetDevice().SetVertexBuffer(m_pFrondVertexBuffer, 0, 0);
        // activate the frond vertex buffer
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::RenderFronds

void CSpeedTreeWrapper::RenderFronds(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

    if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
    {
        PositionTree( );
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

        // if tree has fronds, draw them
        if (m_pFrondIndexCounts &&
			m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1 &&
			m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
		{
            Engine::GetDevice().SetIndexBuffer(m_pFrondIndexBuffers[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel]);
			Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2);
            ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;

		}
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetupLeafForTreeType

void CSpeedTreeWrapper::SetupLeafForTreeType(void)
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cLeafMaterial.Activate(ms_lpd3dDevice);
        SetShaderConstants(m_pSpeedTree->GetLeafMaterial( ));
    #endif

    // send leaf tables to the gpu
    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        UploadLeafTables(c_nVertexShader_LeafTables);
    #endif

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
    STATEMANAGER.SetTexture(1, nullptr);
    #endif
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::UploadLeafTables

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
void CSpeedTreeWrapper::UploadLeafTables(unsigned int uiLocation) const
{
    // query leaf cluster table from RT
    unsigned int uiEntryCount = 0;
    const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);

    // upload for vertex shader use
    STATEMANAGER.SetVertexShaderConstant(uiLocation, pTable, uiEntryCount / 4);
}
#endif


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::RenderLeaves

void CSpeedTreeWrapper::RenderLeaves(void) const
{
	#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, 0);
	#else
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);
	#endif

    // update the LOD level vertex arrays we need
    #if defined(WRAPPER_USE_GPU_LEAF_PLACEMENT) && defined(WRAPPER_USE_GPU_WIND)
        // do nothing
    #else
        #if !defined WRAPPER_USE_NO_WIND || defined WRAPPER_USE_CPU_LEAF_PLACEMENT
            // might need to draw 2 LOD's
            for (unsigned int i = 0; i < 2; ++i)
            {
                // reference to leaves structure
                const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (i == 0) ? &m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;
                int unLod = pLeaf->m_nDiscreteLodLevel;

                #if defined WRAPPER_USE_GPU_LEAF_PLACEMENT
                    if (pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0 && !m_pLeavesUpdatedByCpu[unLod])
                    {
                        // update the centers
                        SFVFLeafVertex* pVertex = NULL;
                        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertex), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
                        for (unsigned int unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
                        {
                            Vector3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 0
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 1
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 2
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 0
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 2
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 3
                        }
                        m_pLeafVertexBuffer[unLod]->Unlock( );
                        m_pLeavesUpdatedByCpu[unLod] = true;
                    }
                #else
                    if (pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
                    {
                        // update the vertices
                        SFVFLeafVertex* pVertex = (SFVFLeafVertex*)Engine::GetDevice().LockVertexBuffer(m_pLeafVertexBuffers[unLod]);
                        
                        for (unsigned int unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
                        {
                            Vector3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
                            Vector3 vec0(&pLeaf->m_pLeafMapCoords[unLeaf][0]);
                            Vector3 vec1(&pLeaf->m_pLeafMapCoords[unLeaf][4]);
                            Vector3 vec2(&pLeaf->m_pLeafMapCoords[unLeaf][8]);
                            Vector3 vec3(&pLeaf->m_pLeafMapCoords[unLeaf][12]);

                            (pVertex++)->m_vPosition = vecCenter + vec0;        // vertex 0
                            (pVertex++)->m_vPosition = vecCenter + vec1;        // vertex 1
                            (pVertex++)->m_vPosition = vecCenter + vec2;        // vertex 2
                            (pVertex++)->m_vPosition = vecCenter + vec0;        // vertex 0
                            (pVertex++)->m_vPosition = vecCenter + vec2;        // vertex 2
                            (pVertex++)->m_vPosition = vecCenter + vec3;        // vertex 3
                        }
                        Engine::GetDevice().UnlockVertexBuffer(m_pLeafVertexBuffers[unLod]);
                    }
                #endif
            }
        #endif
    #endif

    PositionTree( );

    // might need to draw 2 LOD's
    for (unsigned int unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
    {
        const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
            &m_pGeometryCache->m_sLeaves0 : pLeaf = &m_pGeometryCache->m_sLeaves1;

        int unLod = pLeaf->m_nDiscreteLodLevel;

        // if this LOD is active and has leaves, draw it
        if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
        {
            Engine::GetDevice().SetVertexBuffer(m_pLeafVertexBuffers[unLod], 0, 0);
            STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

			ms_faceCount += pLeaf->m_usLeafCount * 2;
            Engine::GetDevice().DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::EndLeafForTreeType

void CSpeedTreeWrapper::EndLeafForTreeType(void)
{
#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT
    // reset flags for CPU data copying
    for (unsigned int i = 0; i < m_usNumLeafLods; ++i)
		m_pLeavesUpdatedByCpu[i] = false;
#endif
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::RenderBillboards

void CSpeedTreeWrapper::RenderBillboards(void) const
{
    // draw billboards in immediate mode (as close as directx gets to immediate mode)
    #ifdef WRAPPER_BILLBOARD_MODE
        PositionTree( );

        struct SBillboardVertex
        {
            float fX, fY, fZ;
            DWORD dColor;
            float fU, fV;
        };

        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

        if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
        {
            const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
            const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
            SBillboardVertex sVertex[4] =
            {
                { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
            };
            STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

			ms_faceCount += 2;
            Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
        }

        // if there is a 360 degree billboard, then we need to draw the second one
        if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
        {
            const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
            const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
            SBillboardVertex sVertex[4] =
            {
                { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
            };
            STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

			ms_faceCount += 2;
            Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
        }

        // if we have a horizontal bilboard and it is enabled, draw it too
        #ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
            if (m_pGeometryCache->m_sHorizontalBillboard.m_bIsActive)
            {
                const float* pCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pCoords;
                const float* pTexCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pTexCoords;
                SBillboardVertex sVertex[4] =
                {
                    { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                    { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                    { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                    { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
                };
                STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sHorizontalBillboard.m_fAlphaTestValue));

				ms_faceCount += 2;
                STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
            }

        #endif
    #endif
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
    if (!m_bIsInstance)
        m_pSpeedTree->DeleteTransientData( );
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::PositionTree

void CSpeedTreeWrapper::PositionTree(void) const
{
    const Vector3 vecPosition = Vector3{m_pSpeedTree->GetTreePosition( )};
    Matrix matTranslation = Matrix::CreateTranslation(vecPosition);

    // store translation for client-side transformation
    STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);

    // store translation for use in vertex shader
    Vector4 vecConstant(vecPosition.x, vecPosition.y, vecPosition.z, 0.0f);
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::LoadTexture

bool CSpeedTreeWrapper::LoadTexture(const std::string& treePath,
                                    const char* filename,
                                    CGraphicImageInstance& image)
{
	// Ignore empty filenames
	if (!filename || filename[0] == '\0')
		return true;

	const auto texturePath = CFileNameHelper::GetPath(treePath) +
	                         CFileNameHelper::NoExtension(filename) +
	                         ".dds";

	auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(texturePath.c_str());
	image.SetImagePointer(r);

	if (image.IsEmpty()) {
		SPDLOG_ERROR("{0}: Failed to load {1}", treePath, texturePath);

		// TODO(tim): Apparently missing textures are common and nobody wants to do
		// something about that. Fine.
		return true;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetShaderConstants

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
    const float afUsefulConstants[] =
    {
        m_pSpeedTree->GetLeafLightingAdjustment( ), 0.0f, 0.0f, 0.0f
    };
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

    const float afMaterial[] =
    {
        pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
        pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
    };
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeWrapper::SetPosition

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
	m_afPos[0] = x;
	m_afPos[1] = y;
	m_afPos[2] = z;

	m_pSpeedTree->SetTreePosition(x, y, z);
	CGraphicObjectInstance::SetPosition(x, y, z);
}

METIN2_END_NS
