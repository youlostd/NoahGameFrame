///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9 Class
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


///////////////////////////////////////////////////////////////////////
//  includes

#include "StdAfx.h"
#include "SpeedTreeForestDirectX9.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

#include "../EterBase/Timer.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/Engine.hpp"


#include <Direct3D.hpp>
#include <Stl.h>

METIN2_BEGIN_NS

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::CSpeedTreeForestDirectX9

CSpeedTreeForestDirectX9::CSpeedTreeForestDirectX9( )
	: m_dwBranchVertexShader(NULL)
	, m_dwLeafVertexShader(NULL)
{
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9

CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9( )
{
	// release our texture and shaders
	M2_SAFE_RELEASE_CHECK(m_dwBranchVertexShader);
	M2_SAFE_RELEASE_CHECK(m_dwLeafVertexShader);
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::Initialize

bool CSpeedTreeForestDirectX9::Initialize()
{
	if (!InitVertexShaders())
		return false;

	const float c_afLightPosition[4] = { -0.707f, -0.300f, 0.707f, 0.0f };
	const float	c_afLightAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const float	c_afLightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float	c_afLightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float afLight1[] =
	{
		c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2],	// pos
		c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2],		// diffuse
		c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2],		// ambient
		c_afLightSpecular[0], c_afLightSpecular[1], c_afLightSpecular[2],	// specular
		c_afLightPosition[3],												// directional flag
		1.0f, 0.0f, 0.0f													// attenuation (constant, linear, quadratic)
	};

	CSpeedTreeRT::SetLightAttributes(0, afLight1);
	CSpeedTreeRT::SetLightState(0, true);
	return true;
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::UploadWindMatrix

void CSpeedTreeForestDirectX9::UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const
{
	// set wind matrices to vertex shader
	STATEMANAGER.SetVertexShaderConstant(uiLocation, pMatrix, 4);
}

void CSpeedTreeForestDirectX9::UpdateCompoundMatrix(const Vector3 & c_rEyeVec, const Matrix & c_rmatView, const Matrix & c_rmatProj)
{
	// setup composite matrix for shader
	Matrix matBlend = Matrix::Identity;
	Matrix matBlendShader = c_rmatView * c_rmatProj;

	float afDirection[3];
	afDirection[0] = matBlendShader.m[0][2];
	afDirection[1] = matBlendShader.m[1][2];
	afDirection[2] = matBlendShader.m[2][2];
	CSpeedTreeRT::SetCamera((float*)&c_rEyeVec, afDirection);

	matBlendShader = matBlendShader.Transpose();
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, (const float*)&matBlendShader, 4);
}

///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::Render

void CSpeedTreeForestDirectX9::Render(unsigned long ulRenderBitVector)
{
	if (m_pMainTreeMap.empty())
		return;

	if (!(ulRenderBitVector & Forest_RenderToShadow) && !(ulRenderBitVector & Forest_RenderToMiniMap))
		UpdateCompoundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	DWORD dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwColorVertexState = STATEMANAGER.GetRenderState(D3DRS_COLORVERTEX);
	DWORD dwFogVertexMode = STATEMANAGER.GetRenderState(D3DRS_FOGVERTEXMODE);

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, true);
#else
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, true);
#endif

	UINT uiCount;
	TTreeMap::const_iterator itor = m_pMainTreeMap.begin();

	while (itor != m_pMainTreeMap.end())
	{
		CSpeedTreeWrapper * pMainTree = (itor++)->second;
		CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

		for (UINT i = 0; i < uiCount; ++i)
		{
			ppInstances[i]->Advance();
		}
	}

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Light,	m_afLighting, 3);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, m_afFog, 1);

	if (ulRenderBitVector & Forest_RenderToShadow)
	{
		//STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER,	D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER,	D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, true);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// set up fog if it is enabled
	if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
	{
		#ifdef WRAPPER_USE_GPU_WIND
		STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); // GPU needs to work on all cards
		#endif
	}

	// choose fixed function pipeline or custom shader for fronds and branches
	Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);

	// render branches
	if (ulRenderBitVector & Forest_RenderBranches)
	{
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			CSpeedTreeWrapper * pMainTree = (itor++)->second;
			CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupBranchForTreeType();

			for (UINT i = 0; i < uiCount; ++i)
				if (ppInstances[i]->isShow())
					ppInstances[i]->RenderBranches();
		}
	}

	// set render states
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// render fronds
	if (ulRenderBitVector & Forest_RenderFronds)
	{
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			CSpeedTreeWrapper * pMainTree = (itor++)->second;
			CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupFrondForTreeType();

			for (UINT i = 0; i < uiCount; ++i)
				if (ppInstances[i]->isShow())
					ppInstances[i]->RenderFronds();
		}
	}

	// render leaves
	if (ulRenderBitVector & Forest_RenderLeaves)
	{
		Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_LEAF_VERTEX);

		if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
		{
			#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
				STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
			#endif
		}

		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
			STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
		}

		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			CSpeedTreeWrapper * pMainTree = (itor++)->second;
			CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupLeafForTreeType();

			for (UINT i = 0; i < uiCount; ++i)
				if (ppInstances[i]->isShow())
					ppInstances[i]->RenderLeaves();
		}

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
			(itor++)->second->EndLeafForTreeType();
#endif
		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
		}
	}

	// render billboards
	#ifndef WRAPPER_NO_BILLBOARD_MODE
		if (ulRenderBitVector & Forest_RenderBillboards)
		{
			STATEMANAGER.SetVertexShader(NULL);
			Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
			STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, false);

			itor = m_pMainTreeMap.begin();

			while (itor != m_pMainTreeMap.end())
			{
				CSpeedTreeWrapper * pMainTree = (itor++)->second;
				CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

				pMainTree->SetupBranchForTreeType();

				for (UINT i = 0; i < uiCount; ++i)
					if (ppInstances[i]->isShow())
						ppInstances[i]->RenderBillboards();
			}
		}
	#endif

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, dwColorVertexState);
	STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, dwFogVertexMode);

	// 셀프섀도우로 쓰는 TextureStage 1의 COLOROP와 ALPHAOP를 꺼줘야 다음 렌더링 할 놈들이
	// 제대로 나온다. (안그러면 검게 나올 가능성이..)
	if (!(ulRenderBitVector & Forest_RenderToShadow))
	{
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	//// advance all trees' LOD and wind
	//while (itor != m_pMainTreeMap.end())
	//{
	//	CSpeedTreeWrapper * pMainTree = (itor++)->second;
	//	CSpeedTreeWrapper ** ppInstances = pMainTree->GetInstances(uiCount);

	//	for (UINT i = 0; i < uiCount; ++i)
	//	{
	//		ppInstances[i]->Advance();
	//	}
	//}

 //   if (!m_vAllTrees.empty( ))
 //   {
 //       // setup general graphics states
 //       STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, true);
 //       STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
 //       STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
 //       STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
 //       STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
 //       STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

 //       // set up fog, if enabled
 //       #ifdef WRAPPER_USE_FOG
 //           STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, true);
 //           STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, 0xFF334C66);
 //           STATEMANAGER.SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
 //           #ifdef WRAPPER_USE_GPU_WIND
 //               STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
 //           #else
 //               STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
 //               STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((DWORD*) (&c_fFogStart)));
 //               STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((DWORD*) (&c_fFogEnd)));
 //           #endif
 //       #else
 //           STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, false);
 //       #endif

 //       // choose fixed function pipeline or custom shader for fronds and branches
 //       Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
 //       STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);

 //       // render branches
 //       if (ulRenderBitVector & Forest_RenderBranches)
 //       {
 //           for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
 //           {
 //               if (!m_vAllTrees[i]->InstanceOf( ))
 //                   m_vAllTrees[i]->SetupBranchForTreeType( );
 //               m_vAllTrees[i]->RenderBranches( );
 //           }
 //       }

 //       // bind composite map & set render states
 //       STATEMANAGER.SetTexture(0, m_texCompositeMap);
 //       STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

 //       // render fronds
 //       if (ulRenderBitVector & Forest_RenderFronds)
 //       {
 //           for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
 //           {
 //               if (!m_vAllTrees[i]->InstanceOf( ))
 //                   m_vAllTrees[i]->SetupFrondForTreeType( );
 //               m_vAllTrees[i]->RenderFronds( );
 //           }
 //       }

 //       // render leaves
 //       if (ulRenderBitVector & Forest_RenderLeaves)
 //       {
 //           Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
 //           STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);
 //           #ifdef WRAPPER_USE_FOG
 //               #if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
 //                   STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
 //               #endif
 //           #endif
 //           for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
 //           {
 //               if (!m_vAllTrees[i]->InstanceOf( ))
 //                   m_vAllTrees[i]->SetupLeafForTreeType( );
 //               m_vAllTrees[i]->RenderLeaves( );
 //           }
 //           for (i = 0; i < m_vAllTrees.size( ); ++i)
 //               if (!m_vAllTrees[i]->InstanceOf( ))
 //                   m_vAllTrees[i]->EndLeafForTreeType( );
 //       }

 //       // render billboards
 //       #ifdef WRAPPER_BILLBOARD_MODE
 //           if (ulRenderBitVector & Forest_RenderBillboards)
 //           {
 //               STATEMANAGER.SetVertexShader(0);
 //				  Engine::GetDevice().SetFvF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
 //               STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
 //               STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, false);
 //               #ifdef WRAPPER_USE_FOG
 //                   STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
 //                   STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((DWORD*) (&c_fFogStart)));
 //                   STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((DWORD*) (&c_fFogEnd)));
 //               #endif

 //               for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
 //                   m_vAllTrees[i]->RenderBillboards( );

 //               #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
 //                   STATEMANAGER.SetRenderState(D3DRS_LIGHTING, true);
 //               #else
 //                   STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, true);
 //               #endif
 //           }
 //       #endif
 //   }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeForestDirectX9::InitVertexShaders

bool CSpeedTreeForestDirectX9::InitVertexShaders()
{
	// load the vertex shaders
	m_dwBranchVertexShader = LoadBranchShader(ms_lpd3dDevice);
	m_dwLeafVertexShader = LoadLeafShader(ms_lpd3dDevice);

	return true;
}

METIN2_END_NS
