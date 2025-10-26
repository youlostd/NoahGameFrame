#include "StdAfx.h"
#include "SceneMap.h"

#include "../WorldEditor.h"
#include "../WorldEditorDoc.h"
#include "../WorldEditorView.h"
#include "../MainFrm.h"
#include "../DataCtrl/ShadowRenderHelper.h"
#include "../DataCtrl/MapAccessorOutdoor.h"
#include "../DataCtrl/MapAccessorTerrain.h"
#include "../DataCtrl/MiniMapRenderHelper.h"

#include <GameLib/EnvironmentData.hpp>
#include <MilesLib/SoundManager.h>
#include <EterLib/Engine.hpp>
#include <EterLib/Camera.h>

#include <base/Random.hpp>


#include <iterator>
#include <sstream>

METIN2_BEGIN_NS

struct FGetPortalID
{
	float m_fRequestX, m_fRequestY;
	std::set<int> m_kSet_iPortalID;
	FGetPortalID(float fRequestX, float fRequestY)
	{
		m_fRequestX=fRequestX;
		m_fRequestY=fRequestY;
	}
	void operator () (CGraphicObjectInstance * pObject)
	{
		for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			int iID = pObject->GetPortal(i);
			if (0 == iID)
				break;

			m_kSet_iPortalID.insert(iID);
		}
	}
};

void CSceneMap::OnUpdate()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	Vector3 pv3Position = ms_Camera->GetTarget();

	m_CursorRenderer.SetCenterPosition(pv3Position);
	m_CursorRenderer.SetCursorPosition(m_vecMouseMapIntersectPosition);

	// TODO(tim): Is this still true?
	// FIXME : 그림자 맵에 그림자가 안 그려져서 컬링을 하지 않았다. [cronan]
	CCullingManager::Instance().Process();
	CCullingManager::Instance().Update();

	CEffectManager::Instance().Update();

	m_pMapManagerAccessor->UpdateEditing();
 	m_pMapManagerAccessor->UpdateMap(pv3Position.x, -pv3Position.y, pv3Position.z);
	m_pMapManagerAccessor->UpdateAroundAmbience(pv3Position.x, -pv3Position.y, pv3Position.z);

	const TOutdoorMapCoordinate & c_rCurCoordinate = m_pMapAccessor->GetCurCoordinate();
	if (c_rCurCoordinate.m_sTerrainCoordX != m_kPrevCoordinate.m_sTerrainCoordX ||
		c_rCurCoordinate.m_sTerrainCoordY != m_kPrevCoordinate.m_sTerrainCoordY)
	{
		m_pMapAccessor->VisibleMarkedArea();
		m_kPrevCoordinate = c_rCurCoordinate;
	}

	CCamera * pMainCamera = CCameraManager::Instance().GetCurrentCamera();
	const Vector3 & c_rv3CameraDirection = pMainCamera->GetView();
	const Vector3 & c_rv3CameraUp = pMainCamera->GetUp();
	CSoundManager::Instance().SetPosition(pv3Position.x, pv3Position.y, pv3Position.z);
	CSoundManager::Instance().SetDirection(c_rv3CameraDirection.x, c_rv3CameraDirection.y, c_rv3CameraDirection.z, c_rv3CameraUp.x, c_rv3CameraUp.y, c_rv3CameraUp.z);
	CSoundManager::Instance().Update();

/*
	{
		CCullingManager & rkCullingMgr = CCullingManager::Instance();
		FGetPortalID kGetPortalID(pv3Position.x, pv3Position.y);

		Vector3d aVector3d;
		aVector3d.Set(pv3Position.x, pv3Position.y, 0.0f);

		Vector3d toTop;
		toTop.Set(0, 0, 25000.0f);

		rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

		std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
		for (; itor != kGetPortalID.m_kSet_iPortalID.end(); ++itor)
		{
			int iID = *itor;
		}
	}

*/
}

void CSceneMap::OnRender(BOOL bClear)
{
	//위의 주석된 내용을 함수로 따로 뺐다.
	//shadowmap, minimap 생성기능임.
	TerrainShadowMapAndMiniMapUpdate();

	if (bClear)
	{
		CScreen::SetClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b);
		CScreen::Clear();
	}

	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (m_bShadowRenderingOn)
	{
		m_pMapAccessor->BeginRenderCharacterShadowToTexture();

		if (m_bCharacterRenderingOn)
			OnRenderCharacter();

		m_pMapAccessor->EndRenderCharacterShadowToTexture();
	}

	static bool s_bSnow = FALSE;
	if (!s_bSnow)
	{
		m_kSnowEnvironment.Create();
		m_kSnowEnvironment.Enable();
		s_bSnow = TRUE;
	}

	m_kSnowEnvironment.Update(ms_Camera->GetTarget());

	///////////////////////////////////////////////////////////////
	// Render Map
	m_pMapAccessor->RenderSky();
	m_pMapAccessor->RenderBeforeLensFlare();
	m_pMapAccessor->RenderCloud();

	// Environment 적용 부분
	m_pMapAccessor->GetActiveEnvironment().BeginPass();

	{
		m_pMapAccessor->SetInverseViewAndDynamicShaodwMatrices();

		SetDiffuseOperation();

		if (m_bObjectRenderingOn)
		{
			m_pMapAccessor->RenderArea(true);
			CSpeedTreeForest::Instance().Render();
		}

		if (m_bTerrainRenderingOn)
			m_pMapAccessor->RenderTerrain();

		if (m_bGuildAreaRenderingOn)
			m_pMapAccessor->RenderMarkedArea();

		if (m_bCharacterRenderingOn)
			OnRenderCharacter();

		if (m_bWaterRenderingOn)
			m_pMapAccessor->RenderWater();

		//m_kSnowEnvironment.Deform();
		//m_kSnowEnvironment.Render();

		m_pMapAccessor->RenderEffect();
		m_pMapAccessor->RenderPCBlocker();
	}

	m_pMapAccessor->GetActiveEnvironment().EndPass();

	if (EDITING_MODE_ATTRIBUTE == m_iEditingMode || m_bAttrRenderingOn)
		OnRenderSceneAttribute();

	if (m_bObjectCollisionRenderingOn)
		OnRenderObjectCollision();

	m_pMapAccessor->RenderAfterLensFlare();

	CEffectManager::Instance().Render();

	m_pMapAccessor->RenderScreenFiltering();

	// RenderScreenFiltering() just draws a big quad on the entire screen
	// which renders depth testing unreliable from now on...
	STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);

	//////////////////////////////////////
	switch (m_iEditingMode)
	{
		case EDITING_MODE_TERRAIN:
		case EDITING_MODE_ATTRIBUTE:
			OnRenderTerrainEditingArea();
			break;

		case EDITING_MODE_OBJECT:
			OnRenderSelectedObject();
			OnRenderObjectSettingArea();
			break;

		case EDITING_MODE_ENVIRONMENT: {
			if (m_bLightPositionEditingOn)
				OnRenderLightDirection();
			break;
		}
	}

	SetColorOperation();
	OnRenderCenterCursor();

	if (m_bCompassOn)
		OnRenderCompass();

	if (m_bMeterGridOn)
		OnRenderMeterGrid();

	if (m_bMapBoundGridOn)
		OnRenderMapBoundGrid();

	if (m_bPatchGridOn)
		OnRenderPatchGrid();

	if (m_bGizmoOn)
		m_cPickingArrows.Render();

	//////////////////////////////////////

	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);

	CEffectInstance::ResetRenderingEffectCount();
}

void CSceneMap::OnRenderUI(float fx, float fy)
{
	int iRenderedPatchNum, iRenderedSplatNum;
	float fSplatRatio;
	std::vector<int> & aVector = m_pMapManagerAccessor->GetRenderedSplatNum(&iRenderedPatchNum, &iRenderedSplatNum, &fSplatRatio);

	std::ostringstream ostr;
	std::copy(aVector.begin(), aVector.end(), std::ostream_iterator<int>(ostr," "));

	char szMsg[128+1];

	_snprintf(szMsg, 128, "Rendered Splat Count: %d", iRenderedSplatNum);
	m_textInstanceSplatTextureCount.SetValue(szMsg);
	_snprintf(szMsg, 128, "Rendered Patch Count: %d", iRenderedPatchNum);
	m_textInstanceSplatMeshCount.SetValue(szMsg);
	_snprintf(szMsg, 128, "Splat ratio: %f", fSplatRatio);
	m_textInstanceSplatMeshPercentage.SetValue(szMsg);

	Vector3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	uint32_t dwCoordX, dwCoordY, wTerrainCoordX, wTerrainCoordY;;
	PR_FLOAT_TO_INT(v3Target.x, dwCoordX);
	PR_FLOAT_TO_INT(-v3Target.y, dwCoordY);

	wTerrainCoordX = dwCoordX / CTerrainImpl::TERRAIN_XSIZE;
	wTerrainCoordY = dwCoordY / CTerrainImpl::TERRAIN_YSIZE;

	BYTE byTerrainNum;
	m_pMapAccessor->GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum);
	CTerrain * pTerrain;
	m_pMapAccessor->GetTerrainPointer(byTerrainNum, &pTerrain);

	if (!pTerrain)
		return;

	TTerrainSplatPatch & rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	dwCoordX -= wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE;
	dwCoordY -= wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE;

	dwCoordX = dwCoordX/(CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE);
	dwCoordY = dwCoordY/(CTerrainImpl::PATCH_YSIZE * CTerrainImpl::CELLSCALE);

	WORD wPatchNum = dwCoordY * CTerrainImpl::PATCH_XCOUNT + dwCoordX;

	std::ostringstream aStream;
	std::vector<std::pair<uint32_t, int> > aTilecountVector;

	for (BYTE byTextureIndex = 0; byTextureIndex <= pTerrain->GetTextureSet()->GetTextureCount(); ++byTextureIndex)
	{
		uint32_t dwTileCount = rTerrainSplatPatch.PatchTileCount[wPatchNum][byTextureIndex];
		if (dwTileCount > 0)
			aTilecountVector.push_back(std::vector<std::pair<uint32_t, BYTE> >::value_type(dwTileCount, (int)byTextureIndex));
	}
	std::sort(aTilecountVector.begin(), aTilecountVector.end());

	aStream << "( 전체 " << aTilecountVector.size() << " 장 ) : ";
	std::vector<std::pair<uint32_t, int> >::reverse_iterator aIterator = aTilecountVector.rbegin();
	while(aTilecountVector.rend() != aIterator)
	{
		std::pair<uint32_t, int> aPair = *aIterator;

		aStream << aPair.second <<"("<< aPair.first << "), ";
		++aIterator;
	}

	_snprintf(szMsg, 128, "Patch splat tile count %s", aStream.str().c_str());
	m_textInstancePatchSplatTileCount.SetValue(szMsg);

	std::ostringstream aStream2, aStream3;

	for (uint32_t dwPatchCoordY = 0; dwPatchCoordY < CTerrainImpl::PATCH_YCOUNT; ++dwPatchCoordY)
	{
		for (uint32_t dwPatchCoordX = 0; dwPatchCoordX < CTerrainImpl::PATCH_XCOUNT; ++dwPatchCoordX)
		{
			WORD wPatchIndex = dwPatchCoordY * CTerrainImpl::PATCH_XCOUNT + dwPatchCoordX;
			if (rTerrainSplatPatch.PatchTileCount[wPatchIndex][0] > 0)
				aStream2 << " : ( " <<
				dwPatchCoordX * CTerrainImpl::PATCH_XSIZE * 2 + wTerrainCoordX * CTerrainImpl::XSIZE * 2<< ", " <<
				dwPatchCoordY * CTerrainImpl::PATCH_YSIZE * 2 + wTerrainCoordY * CTerrainImpl::YSIZE * 2<< " )";
			for (uint32_t dwi = 1; dwi < pTerrain->GetTextureSet()->GetTextureCount(); ++dwi)
			{
				uint32_t dwCount = rTerrainSplatPatch.PatchTileCount[wPatchIndex][dwi];
				if ( dwCount > 0 && dwCount < 51)
					aStream3 << " : ( " <<
					dwPatchCoordX * CTerrainImpl::PATCH_XSIZE * 2 + wTerrainCoordX * CTerrainImpl::XSIZE * 2<< ", " <<
					dwPatchCoordY * CTerrainImpl::PATCH_YSIZE * 2 + wTerrainCoordY * CTerrainImpl::YSIZE * 2<< " )";
			}
		}
	}


	CScreen::SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
	CScreen::RenderBar2d(fx, fy, fx + 500.0f, fy + 140.0f);

	m_textInstanceSplatTextureCount.Update();
	m_textInstanceSplatTextureCount.Render(fx + 10, fy + 10);
	m_textInstanceSplatMeshCount.Update();
	m_textInstanceSplatMeshCount.Render(fx + 10, fy + 30);
	m_textInstanceSplatMeshPercentage.Update();
	m_textInstanceSplatMeshPercentage.Render(fx + 10, fy + 50);
	m_textInstancePatchSplatTileCount.Update();
	m_textInstancePatchSplatTileCount.Render(fx + 10, fy + 70);

	_snprintf(szMsg, 128, "0 Patch locations %s", aStream2.str().c_str());
	m_textInstanceTexture0Count.SetValue(szMsg);
	m_textInstanceTexture0Count.Update();
	m_textInstanceTexture0Count.Render(fx + 10, fy + 90);

	_snprintf(szMsg, 128, "Less than 51 textures patch locations %s", aStream3.str().c_str());
	m_textInstanceTexture0Count.SetValue(szMsg);
	m_textInstanceTexture0Count.Update();
	m_textInstanceTexture0Count.Render(fx + 10, fy + 110);
}

void CSceneMap::OnRenderLightDirection()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	const auto data = m_pMapAccessor->GetActiveEnvironment().GetData();
	if (!data)
		return;

	SetDiffuseColor(1.0f, 0.0f, 0.0f);
	Vector3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	RenderLine3d(v3Target.x, v3Target.y, v3Target.z,
		v3Target.x - data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x * 100000.0f,
		v3Target.y - data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y * 100000.0f,
		v3Target.z - data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z * 100000.0f);
}

void CSceneMap::OnRenderSceneAttribute()
{
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pMapManagerAccessor->RenderAttr();
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}

void CSceneMap::OnRenderEnvironmentMap()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	// Save transformation matrices of the device.
	Matrix matProj, matView;
	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matView);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProj);

	// Store the current back buffer and z-buffer.
	LPDIRECT3DSURFACE9 pBackBuffer, pZBuffer;
	ms_lpd3dDevice->GetRenderTarget(0, &pBackBuffer);
	ms_lpd3dDevice->GetDepthStencilSurface(&pZBuffer);

	// Use 90-degree field of view in the projection.
	DirectX::SimpleMath::MatrixPerspectiveFovRH(&matProj, DirectX::XM_PI/2, 1.0f, 0.0f, 10000.0f );
	STATEMANAGER.SetTransform(D3DTS_PROJECTION, &matProj);

	// Loop through the six faces of the cube map.
	for (uint32_t i = 0; i < 6; i++)
	{
		// Standard view that will be overridden below.
		Vector3 vEnvEyePt = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
		Vector3 vLookatPt, vUpVec;

		// 우리는 오른손 좌표계이므로 y, z를 바꾼다..
		switch (i)
		{
			case D3DCUBEMAP_FACE_POSITIVE_X:
				vLookatPt = Vector3( 1.0f, 0.0f, 0.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, 0.0f, 1.0f );
				break;
			case D3DCUBEMAP_FACE_NEGATIVE_X:
				vLookatPt = Vector3( -1.0f, 0.0f, 0.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, 0.0f, 1.0f );
				break;
			case D3DCUBEMAP_FACE_POSITIVE_Y:
				vLookatPt = Vector3( 0.0f, 0.0f, 1.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, -1.0f, 0.0f );
				break;
			case D3DCUBEMAP_FACE_NEGATIVE_Y:
				vLookatPt = Vector3( 0.0f, 0.0f, -1.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, 1.0f, 0.0f );
				break;
			case D3DCUBEMAP_FACE_POSITIVE_Z:
				vLookatPt = Vector3( 0.0f, 1.0f, 0.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, 0.0f, 1.0f );
				break;
			case D3DCUBEMAP_FACE_NEGATIVE_Z:
				vLookatPt = Vector3( 0.0f, -1.0f, 0.0f) + vEnvEyePt;
				vUpVec    = Vector3( 0.0f, 0.0f, 1.0f );
				break;
		}

		DirectX::SimpleMath::MatrixLookAtRH(&matView, &vEnvEyePt, &vLookatPt, &vUpVec);
		STATEMANAGER.SetTransform(D3DTS_VIEW, &matView);

		if (FAILED(ms_lpd3dDevice->Clear(0L,
										 NULL,
										 D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0)))
		{
			spdlog::error("Failed to clear region");
			break;
		}

		ms_lpd3dDevice->BeginScene();

		//////////////////////////////////////////////////////////////////////////

		m_pMapAccessor->RenderSky();
		m_pMapAccessor->RenderBeforeLensFlare();
		m_pMapAccessor->RenderArea();
		m_pMapAccessor->RenderTerrain();

		if (m_bCharacterRenderingOn)
			OnRenderCharacter();

		m_pMapAccessor->RenderAfterLensFlare();
		//////////////////////////////////////////////////////////////////////////

		ms_lpd3dDevice->EndScene();
	}

	ms_lpd3dDevice->SetRenderTarget(0, pBackBuffer);
	ms_lpd3dDevice->SetDepthStencilSurface(pZBuffer);

	pBackBuffer->Release();
	pZBuffer->Release();

	// Restore the original transformation matrices.
	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
}

/* (목)
*  컴파스 출력을 활성화 시킨 후 10미터 grid를 활성화 시키면
*  컴파스 출력 이미지에 알파가 적용되서 Render 되는 버그가 있음.
*  그래서 RenderState Alpha 옵션을 SaveRenderState() 를 통해 호출하고
*  RestoreRenderState() 를 통해 복원함.
*/
void CSceneMap::OnRenderMeterGrid()
{
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
	SetDiffuseOperation();
	SetDiffuseColor(1.0f, 1.0f, 1.0f);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	//STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

	long viewradius;
	float fHeightScale;

	viewradius			= m_pMapAccessor->GetViewRadius();
	fHeightScale		= m_pMapAccessor->GetHeightScale();

	Vector3 pv3Position = ms_Camera->GetTarget();

	float GridInterval = 1000;

	long GridMinX = (((long)(pv3Position.x) / CTerrainImpl::CELLSCALE) - viewradius / 2) * CTerrainImpl::CELLSCALE / GridInterval;
	long GridMinY = (((long)(-pv3Position.y) / CTerrainImpl::CELLSCALE) - viewradius / 2) * CTerrainImpl::CELLSCALE / GridInterval;
	long GridMaxX = (((long)(pv3Position.x) / CTerrainImpl::CELLSCALE) + viewradius / 2) * CTerrainImpl::CELLSCALE / GridInterval;
	long GridMaxY = (((long)(-pv3Position.y) / CTerrainImpl::CELLSCALE) + viewradius / 2) * CTerrainImpl::CELLSCALE / GridInterval;

	float fx, fy, fcurz, fnextz;
	long i, j;
	for (i = GridMinX; i <= GridMaxX; ++i)
	{
		fx = (float)i * GridInterval;
		for (j = GridMinY; j < GridMaxY; ++j)
		{
			fy = (float)j * GridInterval;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx, fy + GridInterval) + 10.0f;
			if (0 == i%10)
			{
				SetDiffuseColor(1.0f, 1.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx, -fy - GridInterval, fnextz);
				SetDiffuseColor(1.0f, 1.0f, 1.0f);
			}
			else if (0 == i%5)
			{
				SetDiffuseColor(1.0f, 0.0f, 1.0f);
				RenderLine3d(fx, -fy, fcurz, fx, -fy - GridInterval, fnextz);
				SetDiffuseColor(1.0f, 1.0f, 1.0f);
			}
			else
			{
				RenderLine3d(fx, -fy, fcurz, fx, -fy - GridInterval, fnextz);
			}
		}
	}

	for (j = GridMinY; j <= GridMaxY; ++j)
	{
		fy = (float) j * GridInterval;

		for (i = GridMinX; i < GridMaxX; ++i)
		{
			fx = (float)i * GridInterval;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx + GridInterval, fy) + 10.0f;
			if (0 == j%10)
			{
				SetDiffuseColor(1.0f, 1.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx + GridInterval, -fy, fnextz);
				SetDiffuseColor(1.0f, 1.0f, 1.0f);
			}
			else if (0 == j%5)
			{
				SetDiffuseColor(1.0f, 0.0f, 1.0f);
				RenderLine3d(fx, -fy, fcurz, fx + GridInterval, -fy, fnextz);
				SetDiffuseColor(1.0f, 1.0f, 1.0f);
			}
			else
			{
				RenderLine3d(fx, -fy, fcurz, fx + GridInterval, -fy, fnextz);
			}
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

/* (목)
*  오브젝트 충돌 출력 활성화시 patch grid 를 활성화 시키면 색상이 변하는 버그 수정.
*/
void CSceneMap::OnRenderPatchGrid()
{
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
	SetDiffuseOperation();
	SetDiffuseColor(1.0f, 0.0f, 1.0f);

	uint32_t dwCurRenderStateLight = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	long viewradius;
	float fHeightScale;
	viewradius			= m_pMapAccessor->GetViewRadius();
	fHeightScale		= m_pMapAccessor->GetHeightScale();

	Vector3 pv3Position = ms_Camera->GetTarget();

	float fx, fy, fcurz, fnextz;
	long i, j;

	WORD wGridWidth = CTerrainImpl::PATCH_XSIZE;
	float fGridWidth = (float)(wGridWidth * CTerrainImpl::CELLSCALE);

	long lMapGridMinX = (((long)pv3Position.x) / CTerrainImpl::CELLSCALE - viewradius) / wGridWidth * wGridWidth;
	long lMapGridMaxX = (((long)pv3Position.x) / CTerrainImpl::CELLSCALE + viewradius) / wGridWidth * wGridWidth;
	long lMapGridMinY = (-((long)pv3Position.y) / CTerrainImpl::CELLSCALE - viewradius) / wGridWidth * wGridWidth;
	long lMapGridMaxY = (-((long)pv3Position.y) / CTerrainImpl::CELLSCALE + viewradius) / wGridWidth * wGridWidth;

	for (i = lMapGridMinX; i <= lMapGridMaxX; i += wGridWidth)
	{
		fx = (float)i * CTerrainImpl::CELLSCALE;
		for (j = lMapGridMinY; j <= lMapGridMaxY; j += wGridWidth)
		{
			fy = (float)j * CTerrainImpl::CELLSCALE;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx, fy + fGridWidth) + 10.0f;
			RenderLine3d(fx, -fy, fcurz, fx, -fy - fGridWidth, fnextz);
		}
	}
	for (j = lMapGridMinY; j <= lMapGridMaxY; j += wGridWidth)
	{
		fy = (float)j * CTerrainImpl::CELLSCALE;
		for (i = lMapGridMinX; i <= lMapGridMaxX; i += wGridWidth)
		{
			fx = (float)i * CTerrainImpl::CELLSCALE;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx + fGridWidth, fy) + 10.0f;
			RenderLine3d(fx, -fy, fcurz, fx + fGridWidth, -fy, fnextz);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwCurRenderStateLight);
}

void CSceneMap::OnRenderMapBoundGrid()
{
	SetDiffuseOperation();
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
	SetDiffuseColor(0.0f, 0.0f, 0.0f);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	long viewradius;
	float fHeightScale;
	viewradius			= m_pMapAccessor->GetViewRadius();
	fHeightScale		= m_pMapAccessor->GetHeightScale();

	Vector3 pv3Position = ms_Camera->GetTarget();

	float fx, fy, fcurz, fnextz;
	long i, j;

	WORD wGridWidth = CTerrainImpl::XSIZE / 4;
	float fGridWidth = (float)(wGridWidth * CTerrainImpl::CELLSCALE);

	long lMapGridMinX = (((long)pv3Position.x) / CTerrainImpl::CELLSCALE - viewradius) / wGridWidth * wGridWidth;
	long lMapGridMaxX = (((long)pv3Position.x) / CTerrainImpl::CELLSCALE + viewradius) / wGridWidth * wGridWidth;
	long lMapGridMinY = (-((long)pv3Position.y) / CTerrainImpl::CELLSCALE - viewradius) / wGridWidth * wGridWidth;
	long lMapGridMaxY = (-((long)pv3Position.y) / CTerrainImpl::CELLSCALE + viewradius) / wGridWidth * wGridWidth;

	for (i = lMapGridMinX; i <= lMapGridMaxX; i += wGridWidth)
	{
		fx = (float)i * CTerrainImpl::CELLSCALE;
		for (j = lMapGridMinY; j <= lMapGridMaxY; j += wGridWidth)
		{
			fy = (float)j * CTerrainImpl::CELLSCALE;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx, fy + fGridWidth) + 10.0f;
			if (0 == i%CTerrainImpl::XSIZE)
			{
				RenderLine3d(fx, -fy, fcurz, fx, -fy - fGridWidth, fnextz);
			}
			else if (0 == i%(CTerrainImpl::XSIZE / 2))
			{
				SetDiffuseColor(1.0f, 1.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx, -fy - fGridWidth, fnextz);
				SetDiffuseColor(0.0f, 0.0f, 0.0f);
			}
			else if (0 == i%(CTerrainImpl::XSIZE / 4))
			{
				SetDiffuseColor(1.0f, 0.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx, -fy - fGridWidth, fnextz);
				SetDiffuseColor(0.0f, 0.0f, 0.0f);
			}
		}
	}
	for (j = lMapGridMinY; j <= lMapGridMaxY; j += wGridWidth)
	{
		fy = (float)j * CTerrainImpl::CELLSCALE;
		for (i = lMapGridMinX; i <= lMapGridMaxX; i += wGridWidth)
		{
			fx = (float)i * CTerrainImpl::CELLSCALE;
			fcurz = m_pMapAccessor->GetHeight(fx, fy) + 10.0f;
			fnextz = m_pMapAccessor->GetHeight(fx + fGridWidth, fy) + 10.0f;
			if (0 == j%CTerrainImpl::YSIZE)
			{
				RenderLine3d(fx, -fy, fcurz, fx + fGridWidth, -fy, fnextz);
			}
			else if (0 == j%(CTerrainImpl::YSIZE / 2))
			{
				SetDiffuseColor(1.0f, 1.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx + fGridWidth, -fy, fnextz);
				SetDiffuseColor(0.0f, 0.0f, 0.0f);
			}
			else if (0 == j%(CTerrainImpl::YSIZE / 4))
			{
				SetDiffuseColor(1.0f, 0.0f, 0.0f);
				RenderLine3d(fx, -fy, fcurz, fx + fGridWidth, -fy, fnextz);
				SetDiffuseColor(0.0f, 0.0f, 0.0f);
			}
		}
	}
}

void CSceneMap::OnRenderCharacter()
{
	Vector3 pv3Position = ms_Camera->GetTarget();

	SetDiffuseOperation();
	RenderBackGroundCharacter(pv3Position.x, pv3Position.y, pv3Position.z - 100.0f);
}



/* (월)
*  특정 상황에서 의도치 않은 색깔로 Render 되던 버그 수정.
*  색상지정 코드가 없어 SetDiffuseColor 로 색깔을 지정함.
*/
void CSceneMap::OnRenderObjectCollision()
{
	SetDiffuseColor(0.0f, 1.0f, 1.0f);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
	m_pMapManagerAccessor->RenderObjectCollision();
}

/* (월)
*  특정상황에서 의도치 않은 색깔로 Render 되던 버그 수정.
*  Light 영향으로 인한 버그라 무조건 light 를 끄고 Render 시킴.
*/
void CSceneMap::OnRenderCenterCursor()
{
	SetDiffuseColor(m_PickingPointColor.r, m_PickingPointColor.g, m_PickingPointColor.b);

	uint32_t dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	RenderLine3d(
		m_vecMouseMapIntersectPosition.x, m_vecMouseMapIntersectPosition.y, m_vecMouseMapIntersectPosition.z - 100.0f,
		m_vecMouseMapIntersectPosition.x, m_vecMouseMapIntersectPosition.y, m_vecMouseMapIntersectPosition.z + 100.0f);
	RenderLine3d(
		m_vecMouseMapIntersectPosition.x - 100.0f, m_vecMouseMapIntersectPosition.y, m_vecMouseMapIntersectPosition.z + 1.0f,
		m_vecMouseMapIntersectPosition.x + 100.0f, m_vecMouseMapIntersectPosition.y, m_vecMouseMapIntersectPosition.z + 1.0f);
	RenderLine3d(
		m_vecMouseMapIntersectPosition.x, m_vecMouseMapIntersectPosition.y - 100.0f, m_vecMouseMapIntersectPosition.z + 1.0f,
		m_vecMouseMapIntersectPosition.x, m_vecMouseMapIntersectPosition.y + 100.0f, m_vecMouseMapIntersectPosition.z + 1.0);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
}

/* (목)
*  x,y,z 축을 표시해 주는 선분의 색깔이 특정 상황에서 변하는 버그가 있었음.
*  RenderState Lighting 옵션 관련 하여 코드 추가.
*/
void CSceneMap::OnRenderCompass()
{
	Vector3 pv3Position = ms_Camera->GetTarget();

	SetDiffuseOperation();
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());

	uint32_t dwBackUpRenderStateLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	Vector3 arrowpartx1(-10.0f, 0.0f, -10.0f);
	Vector3 arrowpartx2(-10.0f, 0.0f, 10.0f);
	Matrix matPitch, matRoll;

	DirectX::SimpleMath::MatrixRotationAxis(&matPitch, &Vector3(1.0f, 0.0f, 0.0f), -DirectX::XMConvertToRadians(ms_Camera->GetPitch()));
	D3DXVec3TransformCoord(&arrowpartx1, &arrowpartx1, &matPitch);
	D3DXVec3TransformCoord(&arrowpartx2, &arrowpartx2, &matPitch);
	SetDiffuseColor(1.0f, 0.0f, 0.0f);
	RenderLine3d(pv3Position.x, pv3Position.y, pv3Position.z - 100.0f,
				 pv3Position.x+100.0f, pv3Position.y, pv3Position.z - 100.0f);
	RenderLine3d(pv3Position.x+100.0f, pv3Position.y, pv3Position.z - 100.0f,
				 pv3Position.x+100.0f+arrowpartx1.x, pv3Position.y+arrowpartx1.y, pv3Position.z - 100.0f+arrowpartx1.z);
	RenderLine3d(pv3Position.x+100.0f, pv3Position.y, pv3Position.z - 100.0f,
				 pv3Position.x+100.0f+arrowpartx2.x, pv3Position.y+arrowpartx2.y, pv3Position.z - 100.0f+arrowpartx2.z);

	Vector3 arrowparty1(0.0f, -10.0f, -10.0f);
	Vector3 arrowparty2(0.0f, -10.0f, 10.0f);
	DirectX::SimpleMath::MatrixRotationAxis(&matPitch, &Vector3(0.0f, 1.0f, 0.0f), -DirectX::XMConvertToRadians(ms_Camera->GetPitch()));
	D3DXVec3TransformCoord(&arrowparty1, &arrowparty1, &matPitch);
	D3DXVec3TransformCoord(&arrowparty2, &arrowparty2, &matPitch);
	SetDiffuseColor(0.0f, 0.5f, 0.0f);
	RenderLine3d(pv3Position.x, pv3Position.y, pv3Position.z - 100.0f,
				 pv3Position.x, pv3Position.y+100.0f, pv3Position.z - 100.0f);
	RenderLine3d(pv3Position.x, pv3Position.y+100.0f, pv3Position.z - 100.0f,
				 pv3Position.x+arrowparty1.x, pv3Position.y+100.0f+arrowparty1.y, pv3Position.z - 100.0f+arrowparty1.z);
	RenderLine3d(pv3Position.x, pv3Position.y+100.0f, pv3Position.z - 100.0f,
				 pv3Position.x+arrowparty2.x, pv3Position.y+100.0f+arrowparty2.y, pv3Position.z - 100.0f+arrowparty2.z);

	Vector3 arrowpartz1(-10.0f, 0.0f, -10.0f);
	Vector3 arrowpartz2(10.0f, 0.0f, -10.0f);
	DirectX::SimpleMath::MatrixRotationZ(&matRoll, -DirectX::XMConvertToRadians(ms_Camera->GetRoll()));
	D3DXVec3TransformCoord(&arrowpartz1, &arrowpartz1, &matRoll);
	D3DXVec3TransformCoord(&arrowpartz2, &arrowpartz2, &matRoll);
	SetDiffuseColor(0.0f, 0.0f, 1.0f);
	RenderLine3d(pv3Position.x, pv3Position.y, pv3Position.z - 100.0f,
				 pv3Position.x, pv3Position.y, pv3Position.z);
	RenderLine3d(pv3Position.x, pv3Position.y, pv3Position.z,
				 pv3Position.x+arrowpartz1.x, pv3Position.y+arrowpartz1.y, pv3Position.z+arrowpartz1.z);
	RenderLine3d(pv3Position.x, pv3Position.y, pv3Position.z,
				 pv3Position.x+arrowpartz2.x, pv3Position.y+arrowpartz2.y, pv3Position.z+arrowpartz2.z);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwBackUpRenderStateLighting);

	float fx, fy[3];
	ProjectPosition(pv3Position.x + 100.0f,
	                pv3Position.y,
	                pv3Position.z - 100.0f,
	                &fx, &fy[0]);

	ProjectPosition(pv3Position.x,
	                pv3Position.y + 100.0f,
	                pv3Position.z - 100.0f,
	                &fx, &fy[1]);

	ProjectPosition(pv3Position.x,
	                pv3Position.y,
	                pv3Position.z,
	                &fx, &fy[2]);

	Matrix matIdentity;
	DirectX::SimpleMath::MatrixIdentity(&matIdentity);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matIdentity);
	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matIdentity);
	SetOrtho2D(1024.0f, 768.0f, 400.0f);
	SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < 3; ++i)
	{
		m_TextInstance[i].Update();
		m_TextInstance[i].Render(fx, fy[i]);
	}

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetTexture(0, m_pCompasGraphicImageInstance.GetTexturePointer()->GetD3DTexture());
}

void CSceneMap::OnRenderTerrainEditingArea()
{
	int EditX, EditY;
	unsigned char ucSubEditX, ucSubEditY;
	unsigned short usEditTerrainX, usEditTerrainY;
	m_pMapManagerAccessor->GetEditArea(&EditX, &EditY, &ucSubEditX, &ucSubEditY, &usEditTerrainX, &usEditTerrainY);

	unsigned char ucBrushSize = m_pMapManagerAccessor->GetBrushSize();

	float fheightscale, fx, fy, fz, fLeft, fTop, fRight, fBottom;
	fheightscale		= m_pMapAccessor->GetHeightScale();

	uint32_t dwBrushShape = m_pMapManagerAccessor->GetBrushShape();

	if (EDITING_MODE_ATTRIBUTE == m_iEditingMode)
	{
		if (m_pMapManagerAccessor->isAttrEditing())
		{
			fx = (float)(EditX * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fy = (float)(EditY * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fz = m_pMapAccessor->GetHeight((float)fx, (float)fy) + 10.0f;

			fLeft	= (float)((EditX - ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fTop	= (float)((EditY - ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fRight	= (float)((EditX + ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fBottom	= (float)((EditY + ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
		} else {
			return;
		}
	}
	else
	{
		if (m_pMapManagerAccessor->isWaterEditing())
		{
			m_pMapManagerAccessor->PreviewEditWater();
			return;
		}
		else if (m_pMapManagerAccessor->isTextureEditing())
		{
			fx = (float)(EditX * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fy = (float)(EditY * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fz = m_pMapAccessor->GetHeight((float)fx, (float)fy) + 10.0f;

			fLeft	= (float)((EditX - ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fTop	= (float)((EditY - ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fRight	= (float)((EditX + ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditX * CTerrainImpl::CELLSCALE / 2 + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fBottom	= (float)((EditY + ucBrushSize) * CTerrainImpl::CELLSCALE + ucSubEditY * CTerrainImpl::CELLSCALE / 2 + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
		}
		else
		{
			fx = (float)(EditX * CTerrainImpl::CELLSCALE + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fy = (float)(EditY * CTerrainImpl::CELLSCALE + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fz = m_pMapAccessor->GetHeight((float)fx, (float)fy) + 10.0f;

			fLeft	= (float)((EditX - ucBrushSize) * CTerrainImpl::CELLSCALE + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fTop	= (float)((EditY - ucBrushSize) * CTerrainImpl::CELLSCALE + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
			fRight	= (float)((EditX + ucBrushSize) * CTerrainImpl::CELLSCALE + usEditTerrainX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
			fBottom	= (float)((EditY + ucBrushSize) * CTerrainImpl::CELLSCALE + usEditTerrainY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);
		}
	}

	SetDiffuseColor(m_EditCenterColor.r, m_EditCenterColor.g, m_EditCenterColor.b);
	RenderLine3d( fx, -fy, fz - 30.0f, fx, -fy, fz + 30.0f);
	RenderLine3d( fx - 30.0f, -fy, fz, fx + 30.0f, -fy, fz);
	RenderLine3d( fx, -fy - 30.0f, fz, fx, -fy + 30.0f, fz);

	SetDiffuseColor(m_EditablePointColor.r, m_EditablePointColor.g, m_EditablePointColor.b);

	SetDiffuseOperation();
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	SetDiffuseColor(0.0f, 1.0f, 0.0f);

	if (CTerrainAccessor::BRUSH_SHAPE_CIRCLE == dwBrushShape)
		m_CursorRenderer.RenderCursorCircle(fx, fy, 0.0f, float(ucBrushSize * CTerrainImpl::CELLSCALE));
	else if (CTerrainAccessor::BRUSH_SHAPE_SQUARE == dwBrushShape)
		m_CursorRenderer.RenderCursorSquare( fLeft,	fTop, fRight, fBottom, 2 * ucBrushSize);
}

void CSceneMap::OnRenderSelectedObject()
{
	m_pMapManagerAccessor->RenderSelectedObject();
}

void CSceneMap::OnRenderObjectSettingArea()
{
	m_CursorRenderer.Update();
	SetColorOperation();
	m_CursorRenderer.RenderCursorArea();
	SetDiffuseOperation();
	m_CursorRenderer.Render();
}

void CSceneMap::OnLightMove(const long & c_rlx, const long & c_rly)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	const auto data = m_pMapAccessor->GetEnvironment(m_pMapAccessor->GetSelectedEnvironment());
	if (!data)
		return;

	Vector3 v3DirLight;
	v3DirLight.x = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
	v3DirLight.y = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
	v3DirLight.z = data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;

	long xMove = m_loldX - c_rlx;
	long yMove = m_loldY - c_rly;

	float fRoll, fPitch;
	fRoll = (float)xMove * 0.01f;
	fPitch = (float)yMove * 0.01f;

	Matrix matTranform, udRotation, lrRotation;
	DirectX::SimpleMath::MatrixRotationX(&udRotation, fPitch);
	DirectX::SimpleMath::MatrixRotationZ(&lrRotation, fRoll);

	matTranform=lrRotation*udRotation;

	D3DXVec3TransformCoord(&v3DirLight, &v3DirLight, &matTranform);
	D3DXVec3Normalize(&v3DirLight, &v3DirLight);

	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x = v3DirLight.x;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y = v3DirLight.y;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z = v3DirLight.z;

	// TODO(tim)
	m_pMapAccessor->RefreshEnvironment();

	m_loldX = c_rlx;
	m_loldY = c_rly;
}

void CSceneMap::SetObjectShadowRendering(bool bOn)
{
	m_pMapAccessor->SetDrawShadow(bOn);
	m_bShadowRenderingOn = bOn;
}

void CSceneMap::SetGuildAreaRendering(bool bOn)
{
	m_bGuildAreaRenderingOn = bOn;
	if (bOn)
		m_pMapAccessor->VisibleMarkedArea();
	else
		m_pMapAccessor->DisableMarkedArea();
}

void CSceneMap::SetEditingMode(int iMode)
{
	if (m_iEditingMode!=iMode)
	{
		// NOTE : 선택하고 있는게 있더라도 초기화가 되어버린다.
		//        MapObjectPage.cpp에서 모드가 바뀔때 커서를 다시 셋팅하도록 바꿨음
		//m_CursorRenderer.ClearCursor();
		m_iEditingMode = iMode;
	}
}

void CSceneMap::RefreshArea()
{
	m_pMapManagerAccessor->RefreshArea();
}

CCursorRenderer * CSceneMap::GetCursorRenererPointer()
{
	return & m_CursorRenderer;
}

void CSceneMap::SetMapManagerAccessor(CMapManagerAccessor * pMapManagerAccessor)
{
	m_pMapManagerAccessor	= pMapManagerAccessor;
	m_pMapAccessor			= (CMapOutdoorAccessor*) &pMapManagerAccessor->GetMapOutdoorRef();
	m_pHeightObserver		= pMapManagerAccessor->GetHeightObserverPointer();
	m_CursorRenderer.SetHeightObserver(m_pHeightObserver);
}

void CSceneMap::OnKeyDown(int iChar)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();
	switch (iChar)
	{
		case VK_SPACE:
			SwapHoldOn();
			break;

		case VK_Z:
			m_pMapAccessor->SetSplatValue(m_pMapAccessor->GetSplatValue() - 0.1f);
			break;

		case VK_X:
			m_pMapAccessor->SetSplatValue(m_pMapAccessor->GetSplatValue() + 0.1f);
			break;

		/*
		*  Snap 기능 활성, 비활성 단축키 제공
		*
		*  (월)
		*  Control + S 키가 맵 저장 단축키 입니다.
		*  SaveMap 내부에서 MessageBox 호출이 일어나는데
		*  이 경우 KeyUp Event 가 발생하지 않습니다.
		*  단축키를 통해서 맵을 저장하고 나면 버그가 발생했습니다.
		*  SendMessage 를 통해 강제로 KeyUp Event 를 발생시켰습니다.
		*  임시방편이고 하드코딩입니다. 추후 단축키가 변경되거나
		*  key 처리 관련 코드에 대해 리펙토링이 이루어진다면 없어져야 합니다.
		*/
		case VK_S:
			if (pView->IsControlKey())
			{
				m_pMapManagerAccessor->SaveMap();
				pView->SendMessage(WM_KEYUP, VK_CONTROL, 0);	//view ? ??? keyup message ??
			} else {
				SnapEnable();
			}

			break;

		case VK_F3:
			SetMapBoundGrid(!GetMapBoundGrid());
			break;

		case VK_ESCAPE:
			__ClearCursor();
			break;

		case VK_DELETE:
			if (EDITING_MODE_OBJECT == m_iEditingMode)
			{
				if (m_bHoldOn)
					SwapHoldOn();

				m_pMapManagerAccessor->BackupObject();
				m_pMapManagerAccessor->DeleteSelectedObject();
				m_pMapManagerAccessor->BackupObjectCurrent();
			}
			pFrame->UpdateMapControlBar();
			break;

		case VK_R:
			m_pMapManagerAccessor->ReloadTerrainShadowTexture();
			m_pMapAccessor->ResetTextures();
			m_pMapAccessor->ReloadBuildingTexture();
			break;

		case VK_E:
			m_pMapAccessor->RefreshEnvironment();
			break;

		case VK_PRIOR:
			CShadowRenderHelper::Instance().SetShadowMapPower(CShadowRenderHelper::Instance().GetShadowMapPower() + 1);
			break;

		case VK_NEXT:
			CShadowRenderHelper::Instance().SetShadowMapPower(CShadowRenderHelper::Instance().GetShadowMapPower() - 1);
			break;

		case VK_HOME:
			CShadowRenderHelper::Instance().SetIntermediateShadowMapPower(CShadowRenderHelper::Instance().GetIntermediateShadowMapPower()+1);
			break;

		case VK_END:
			CShadowRenderHelper::Instance().SetIntermediateShadowMapPower(CShadowRenderHelper::Instance().GetIntermediateShadowMapPower()-1);
			break;

		/*
		*  에디트모드(오브젝트) 단축키 제공 VK_1, VK_2, VK_3, VK_4
		*  4개의 키는 Focus 만 셋팅해 준다.
		*  Focus 가 셋팅되고 나면 Key 메세지가 MapObjectPage 쪽으로 간다.
		*  그래서 MapObjectPage 에도 KeyDown 을 정의함.
		*
		*
		*  에디트모드(터레인) 단축키 제공 VK_1
		*  water brush check 되는 기능 추가
		*/
		case VK_1:
			if (EDITING_MODE_OBJECT == m_iEditingMode)
				RotationYawFocus();
			else if (EDITING_MODE_TERRAIN == m_iEditingMode)
				WaterBrushCheck();

			break;

		case VK_2:
			if (EDITING_MODE_OBJECT == m_iEditingMode)
				RotationPitchFocus();

			break;

		case VK_3:
			if (EDITING_MODE_OBJECT == m_iEditingMode)
				RotationRollFocus();

			break;

		case VK_4:
			if (EDITING_MODE_OBJECT == m_iEditingMode)
				HeightBiasFocus();

			break;

		/*
		*  Object Render 활성,비활성 단축키 제공
		*/
		case VK_C:
			ObjectRenderEnable();
			break;

		/* (수)
		*  다른탭에서도 속성을 볼수 있도록 단축키 제공
		*/
		case VK_A:
			AttributeRenderEnable();
			break;
	}
}

void CSceneMap::OnKeyUp(int iChar)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	switch (iChar)
	{
		case VK_INSERT:
			m_bTerrainShadowMapAndMiniMapUpdateNeeded = true;
			break;
	}
}

/* 26 Sys key check 를 위해 추가
*/
void CSceneMap::OnSysKeyDown(int iChar)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	switch (iChar)
	{
	case VK_MENU:
		WaterEraserEnable(true);
		AttributeEraserEnable(true);
		break;
	}
}

/*  Sys key check 를 위해 추가
*/
void CSceneMap::OnSysKeyUp(int iChar)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	switch (iChar)
	{
	case VK_MENU:
		WaterEraserEnable(false);
		AttributeEraserEnable(false);
		break;
	}
}


void CSceneMap::OnMouseMove(LONG x, LONG y)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();

	CRect Rect;
	pView->GetWindowRect(&Rect);
 	SetPerspective(40.0f, float(Rect.Width()) / float(Rect.Height()), 100.0f, 50000.0f);
	SetCursorPosition((int) x, (int) y, Rect.Width(), Rect.Height());
	GetCursorPosition(&ms_vecMousePosition.x, &ms_vecMousePosition.y, &ms_vecMousePosition.z);
	m_pMapManagerAccessor->UpdateHeightFieldEditingPt(&m_vecMouseMapIntersectPosition);
	m_pMapManagerAccessor->SetEditingCursorPosition(m_CursorRenderer.GetCursorPosition());

	switch(m_iEditingMode)
	{
		case EDITING_MODE_OBJECT:

			if (m_CursorRenderer.IsPicking())
			{
				const Vector3 & c_rv3PickedPosition = m_CursorRenderer.GetPickedPosition();
				const Vector3 & c_rv3CurrentPosition = m_CursorRenderer.GetCursorPosition();
				Vector3 v3Movement = c_rv3CurrentPosition - c_rv3PickedPosition;
				m_CursorRenderer.UpdatePickedPosition();

				if (m_bGizmoOn)
				{
					switch(m_iGizmoSelectedAxis)
					{
					case -1:	//선택한 축이 없다.
						m_pMapManagerAccessor->MoveSelectedObject(v3Movement.x, v3Movement.y);
						break;

					case 0:		//X 축 선택
						m_pMapManagerAccessor->MoveSelectedObject(v3Movement.x, 0.0f);
						break;

					case 1:		//Y 축 선택
						m_pMapManagerAccessor->MoveSelectedObject(0.0f, v3Movement.y);
						break;

					case 2:		//Z 축 선택
						m_pMapManagerAccessor->MoveSelectedObjectHeight(v3Movement.y);
						break;
					}

					GizmoUpdate(m_pickedObject);
				} else {
					m_pMapManagerAccessor->MoveSelectedObject(v3Movement.x, v3Movement.y);
				}

				m_bObjectIsMoved = TRUE;
			}
			else if (m_CursorRenderer.IsSelecting())
			{
				float fxStart, fyStart;
				float fxEnd, fyEnd;

				m_CursorRenderer.GetSelectArea(&fxStart, &fyStart, &fxEnd, &fyEnd);
				BOOL bNeedChange = m_pMapManagerAccessor->SelectObject(fxStart, fyStart, fxEnd, fyEnd, (GetAsyncKeyState(VK_CONTROL) & 0x8000) );
				if (bNeedChange)
				{
					pFrame->UpdateMapControlBar();
				}
			}
			else if(m_CursorRenderer.IsDeselecting())
			{
				float fxStart, fyStart;
				float fxEnd, fyEnd;

				m_CursorRenderer.GetDeselectArea(&fxStart, &fyStart, &fxEnd, &fyEnd);
				BOOL bNeedChange = m_pMapManagerAccessor->DeselectObject(fxStart, fyStart, fxEnd, fyEnd);
				if (bNeedChange)
				{
					pFrame->UpdateMapControlBar();
				}
			}
			break;
		case EDITING_MODE_ENVIRONMENT:
			if (m_bLightPositionEditingOn && m_bLightPositionEditingInProgress)
				OnLightMove(x, y);
			break;
	}
}

void CSceneMap::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (m_bHoldOn)
		HoldStart();
	else
		ObjectSelectAndDeselectStart((GetAsyncKeyState(VK_LCONTROL) & 0x8000),
		                             (GetAsyncKeyState(VK_MENU) & 0x8000));

	InsertObjectOnTheSceneStart();

	switch (m_iEditingMode)
	{
		case EDITING_MODE_TERRAIN:
			m_pMapManagerAccessor->EditingStart();
			m_pMapManagerAccessor->BackupTerrain();
			m_pMapManagerAccessor->SetEditingCursorPosition(m_CursorRenderer.GetCursorPosition());
			break;
		case EDITING_MODE_ENVIRONMENT:
			if (0 == (GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				m_ptClick = point;
				m_loldX = m_ptClick.x;
				m_loldY = m_ptClick.y;
				LightPositionEditingStart();
			}
			break;
		case EDITING_MODE_ATTRIBUTE:
			if (0 == (GetAsyncKeyState(VK_LCONTROL) & 0x8000))
				m_pMapManagerAccessor->EditingStart();
			break;
	}
}

void CSceneMap::OnLButtonUp()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();

	if (m_bHoldOn)
		HoldEnd();
	else
		ObjectSelectAndDeselectEnd((GetAsyncKeyState(VK_LCONTROL) & 0x8000),
		                           (GetAsyncKeyState(VK_MENU) & 0x8000));

	InsertObjectOnTheSceneEnd();

	m_pMapManagerAccessor->EditingEnd();
	m_pMapManagerAccessor->BackupTerrainCurrent();
	if (EDITING_MODE_ENVIRONMENT == m_iEditingMode)
	{
		LightPositionEditingEnd();
		m_ptClick = CPoint(-1, -1);
	}
	m_CursorRenderer.SetCursor(m_dwCursorObjectCRC);
}

void CSceneMap::OnRButtonDown()
{
	if (EDITING_MODE_OBJECT == m_iEditingMode) {
		// 오브젝트 탭에서
		// 마우스 오른쪽 버튼 클릭시 3가지 요구사항
		// 1. Detach, 2. Non Select, 3. Rotation 초기화
		// 1개 추가
		//4. HeightBias 초기화
		DetachObject();

		if (!m_bHoldOn)
			m_pMapManagerAccessor->CancelSelect();

		ClearRotation();
		ClearHeightBias();
	}
}

void CSceneMap::OnRButtonUp()
{
}

BOOL CSceneMap::OnMouseWheel(short zDelta)
{
	if (m_iEditingMode == EDITING_MODE_TERRAIN)
	{
		CWorldEditorApp* pApplication = (CWorldEditorApp*) AfxGetApp();
		CMapManagerAccessor* pMapManagerAccessor = pApplication->GetMapManagerAccessor();

		if (GetAsyncKeyState(VK_4)&0x8000)
		{
			float fNewHeight = pMapManagerAccessor->GetBrushWaterHeight() + float(zDelta / 120) * 100.0f;
			pMapManagerAccessor->SetBrushWaterHeight(fNewHeight);

			CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
			pFrame->UpdateMapControlBar();
			return TRUE;
		}

		if (GetAsyncKeyState(VK_5)&0x8000)
		{
			float fNewHeight = pMapManagerAccessor->GetBrushWaterHeight() + float(zDelta / 120) * 5.0f;
			pMapManagerAccessor->SetBrushWaterHeight(fNewHeight);

			CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
			pFrame->UpdateMapControlBar();
			return TRUE;
		}
	}

	return FALSE;
}

/* (화)
*  오리지널 버전 맵툴에서는 케릭터 출력 기능을 통해
*  케릭터가 오브젝트의 Height 를 따라 잘 이동하는지 체크를 할수 있음.
*  똑같은 기능을 구현하기 위해 OnMovePosition 함수 기능을 수정함.
*
*  1. mdatr 확장자를 가진 파일 내부에 충돌 정보가 있음.
*  2. 오브젝트의 측면 충돌 정보를 metin2 코드에서는 collision 단어를 사용.
*	  오브젝트의 밑면 충돌 정보에 대해서는 height 라는 단어를 사용함.
*/
void CSceneMap::OnMovePosition(float fx, float fy)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

 	Vector3 pv3Position = ms_Camera->GetTarget();

	short sTerrainCountX, sTerrainCountY;
	m_pMapAccessor->GetTerrainCount(&sTerrainCountX, &sTerrainCountY);

	if (pv3Position.x + fx <= 0)
		fx = -pv3Position.x;
	else if (pv3Position.x + fx >= (float)(sTerrainCountX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE))
		fx = -pv3Position.x + (float)(sTerrainCountX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
	if ( pv3Position.y + fy >= 0)
		fy = -pv3Position.y;
	else if (pv3Position.y + fy <= - (float)(sTerrainCountY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE))
		fy = -pv3Position.y - (float)(sTerrainCountY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);

	//원본 코드 주석 전까지 추가 및 수정됨.
	float fTerrainHeight = m_pMapAccessor->GetHeight(pv3Position.x + fx, - pv3Position.y - fy);
	float fObjectHeight = 0.0f;
	float fResultHeight = 0.0f;

	if(m_bObjectRenderingOn && m_pMapManagerAccessor->GetObjectHeight(pv3Position.x, fabsf(pv3Position.y), &fObjectHeight))
		fResultHeight = (fTerrainHeight < fObjectHeight)? fObjectHeight : fTerrainHeight;
	else
		fResultHeight  = fTerrainHeight;

	CCameraManager::Instance().GetCurrentCamera()->Move(Vector3(fx, fy, fResultHeight - pv3Position.z + 100.0f));
}

Vector3 CSceneMap::GetMouseMapIntersect()
{
	return m_vecMouseMapIntersectPosition;
}

//////////////////////////////////////////////////////////////////////////////
void CSceneMap::SetObjectBrushType(int iType)
{
	m_CursorRenderer.SetObjectBrushType(iType);
}
void CSceneMap::SetObjectBrushScale(int iScale)
{
	m_CursorRenderer.SetObjectAreaSize(iScale);
}

void CSceneMap::SetObjectBrushDensity(int iDensity)
{
	m_CursorRenderer.SetObjectAreaDensity(iDensity);
}

void CSceneMap::SetObjectBrushRandom(int iRandom)
{
	m_CursorRenderer.SetObjectAreaRandom(iRandom);
}

void CSceneMap::SetCheckingNearObject(bool bFlag)
{
	m_isCheckingNearObject = bFlag;
}

void CSceneMap::SetRandomRotation(bool bFlag)
{
	m_isRandomRotation = bFlag;
}

void CSceneMap::SetCursorYaw(float fYaw)
{
	m_CursorRenderer.SetCursorYaw(fYaw);
}

void CSceneMap::SetCursorPitch(float fPitch)
{
	m_CursorRenderer.SetCursorPitch(fPitch);
}

void CSceneMap::SetCursorRoll(float fRoll)
{
	m_CursorRenderer.SetCursorRoll(fRoll);
}

void CSceneMap::SetCursorYawPitchRoll(const float fYaw, const float fPitch, const float fRoll)
{
	m_CursorRenderer.SetCursorYaw(fYaw);
	m_CursorRenderer.SetCursorPitch(fPitch);
	m_CursorRenderer.SetCursorRoll(fRoll);
}

void CSceneMap::SetCursorScale(uint32_t dwScale)
{
	m_CursorRenderer.SetCursorScale(dwScale);
}

void CSceneMap::RefreshCursor()
{
	m_CursorRenderer.RefreshCursor();
}

void CSceneMap::ClearCursor()
{
	m_CursorRenderer.ClearCursor();
}

void CSceneMap::ChangeCursor(uint32_t dwCRC)
{
	m_dwCursorObjectCRC = dwCRC;
	m_CursorRenderer.SetCursor(dwCRC);
}

void CSceneMap::SetGridMode(BYTE byGridMode)
{
	m_CursorRenderer.SetGridMode(byGridMode);
}

void CSceneMap::SetGridDistance(float fDistance)
{
	m_CursorRenderer.SetGridDistance(fDistance);
}

void CSceneMap::SetObjectHeight(float fHeight)
{
	//m_pMapManagerAccessor->MoveSelectedObjectHeight(fHeight - m_CursorRenderer.GetObjectHeight());
	m_CursorRenderer.SetObjectHeight(fHeight);
}

float CSceneMap::GetCursorYaw()
{
	return m_CursorRenderer.GetCursorYaw();
}

float CSceneMap::GetCursorRoll()
{
	return m_CursorRenderer.GetCursorRoll();
}

float CSceneMap::GetCursorPitch()
{
	return m_CursorRenderer.GetCursorPitch();
}

float CSceneMap::GetCursorObjectHeight()
{
	return m_CursorRenderer.GetObjectHeight();
}

uint32_t CSceneMap::GetCursorScale()
{
	return m_CursorRenderer.GetCursorScale();
}

//
// 왜 SetCursorXXX 면 set 만 해야지
// AddSelectedObjectRotation 까지 같이 하는지 모르겠다.
// 기능 분리를 위해 추가
void CSceneMap::AddSelectedObjectRotation(const float fYaw, const float fPitch, const float fRoll)
{
	m_pMapManagerAccessor->AddSelectedObjectRotation(fYaw, fPitch, fRoll);
}

void CSceneMap::MoveSelectedObjectHeight(const float fHeight)
{
	m_pMapManagerAccessor->MoveSelectedObjectHeight(fHeight);
}

void CSceneMap::__ClearCursor()
{
	m_CursorRenderer.SetCursor(0);
}

void CSceneMap::CreateEnvironment()
{
}

void CSceneMap::Initialize()
{
	auto& resMgr = CResourceManager::Instance();

	auto pText = Engine::GetFontManager().LoadFont("font/Tahoma-Regular.ttf:20");

	m_TextInstance[0].SetTextPointer(pText);
	m_TextInstance[1].SetTextPointer(pText);
	m_TextInstance[2].SetTextPointer(pText);

	m_TextInstance[0].SetValue("X");
	m_TextInstance[1].SetValue("Y");
	m_TextInstance[2].SetValue("Z");

	m_TextInstance[0].Update();
	m_TextInstance[1].Update();
	m_TextInstance[2].Update();

	m_TextInstance[0].SetColor(1.0f, 0.0f, 0.0f);
	m_TextInstance[1].SetColor(0.0f, 0.5f, 0.0f);
	m_TextInstance[2].SetColor(0.0f, 0.0f, 1.0f);

	m_pCompasGraphicImageInstance.SetImagePointer(resMgr.LoadResource<CGraphicImage>("D:\\Ymir Work\\special\\compas.dds"));

	///////////////////////////////////////////////////////////////////////////////////////////

	auto pTextOrtho = Engine::GetFontManager().LoadFont("font/Tahoma-Regular.ttf:15");

	m_textInstanceSplatTextureCount.SetTextPointer(pTextOrtho);
	m_textInstanceSplatTextureCount.SetColor(0.8f, 0.8f, 0.8f);
	m_textInstanceSplatMeshCount.SetTextPointer(pTextOrtho);
	m_textInstanceSplatMeshCount.SetColor(0.8f, 0.8f, 0.8f);
	m_textInstanceSplatMeshPercentage.SetTextPointer(pTextOrtho);
	m_textInstanceSplatMeshPercentage.SetColor(0.8f, 0.8f, 0.8f);
	m_textInstancePatchSplatTileCount.SetTextPointer(pTextOrtho);
	m_textInstancePatchSplatTileCount.SetColor(0.8f, 0.8f, 0.8f);
	m_textInstanceTexture0Count.SetTextPointer(pTextOrtho);
	m_textInstanceTexture0Count.SetColor(0.8f, 0.8f, 0.8f);

	m_cPickingArrows.SetCenterPosition( Vector3(0.0f, 0.0f, 0.0f) );
	m_cPickingArrows.SetArrowSets(0);
	m_cPickingArrows.SetScale(20.0f);
}

CSceneMap::CSceneMap()
{
	m_pMapManagerAccessor = NULL;
	m_pMapAccessor = NULL;
	m_pHeightObserver = NULL;
	m_iEditingMode = EDITING_MODE_TERRAIN;

	m_vecMouseMapIntersectPosition = Vector3(0.0f, 0.0f, 0.0f);
	m_ClearColor = DirectX::SimpleMath::Color(0.4882f, 0.4882f, 0.4882f, 1.0f);
	m_EditCenterColor = DirectX::SimpleMath::Color(1.0f, 0.0f, 0.0f, 1.0f);
	m_EditablePointColor = DirectX::SimpleMath::Color(1.0f, 0.25f, 0.25f, 1.0f);
	m_PickingPointColor = DirectX::SimpleMath::Color(0.0f, 1.0f, 1.0f, 1.0f);

	m_isCheckingNearObject = false;
	m_isRandomRotation = false;

	m_bCompassOn = false;
	m_bMeterGridOn = false;
	m_bPatchGridOn = false;
	m_bMapBoundGridOn = false;
	m_bCharacterRenderingOn = false;
	m_bWaterRenderingOn = false;
	m_bObjectRenderingOn = true;
	m_bObjectCollisionRenderingOn = false;
	m_bTerrainRenderingOn = true;
	m_bShadowRenderingOn = true;
	m_bGuildAreaRenderingOn = false;
	m_bAttrRenderingOn = false;

	m_bLightPositionEditingInProgress = false;
	m_bLightPositionEditingOn = false;	//(월) false 로 변경. 빛방향조정 버튼이 비활성화 상태인데 기능이 작동함.

	m_bCursorYawPitchChange = false;

	m_bTerrainShadowMapAndMiniMapUpdateNeeded = false;
	m_bTerrainShadowMapAndMiniMapUpdateAll	= false;

	m_kPrevCoordinate.m_sTerrainCoordX = -1;
	m_kPrevCoordinate.m_sTerrainCoordY = -1;

	m_bObjectIsMoved = FALSE;
	m_dwCursorObjectCRC = 0;

	m_bHoldOn	= false;
	m_bGizmoOn	= false;
	m_pickedObject = {0, -1};
	m_iGizmoSelectedAxis	= -1;
}

CSceneMap::~CSceneMap()
{
	m_kSnowEnvironment.Destroy();
}

//Scene 화면에 배치된 오브젝트를 선택 또는 해제합니다(시작)
void CSceneMap::ObjectSelectAndDeselectStart(SHORT sAddedSelectKey, SHORT sDeselectKey)
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	if (0 != m_CursorRenderer.GetCurrentCRC())
		return;

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	m_pMapManagerAccessor->BackupObject();
	m_bObjectIsMoved = FALSE;

	const auto picked = m_pMapManagerAccessor->GetPickedObjectIndex();
	if (!picked) {
		if (sDeselectKey)
			m_CursorRenderer.DeselectStart();
		else
			m_CursorRenderer.SelectStart();
	} else {
		if (0 == sDeselectKey) { //DeselectKey 키가 안눌렸고
			if (0 == sAddedSelectKey) { //AddedSelectKey 키가 안눌렸다면
				// 선택된 오브젝트가 아니라면..
				if (!m_pMapManagerAccessor->IsSelectedObject(picked))
					// 선택을 취소한다
					m_pMapManagerAccessor->CancelSelect();

				m_CursorRenderer.PickStart();
			}

			m_pMapManagerAccessor->SelectObject(picked);
		}
	}

	pFrame->UpdateMapControlBar();
}


/* (화)
*  오브젝트 찍고나서 attach 되어있는 오브젝트 종류를
*  키보드 방향키로 바꿀수 있게 해달라는 요청에 의해 SetFocus() 추가함
*/
//Scene 화면에 오브젝트를 추가합니다(시작)
void CSceneMap::InsertObjectOnTheSceneStart()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if(EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	if (0 == m_CursorRenderer.GetCurrentCRC())
		return;

	m_pMapManagerAccessor->BackupObject();
	uint32_t dwObjectCRC = m_CursorRenderer.GetCurrentCRC();

	float fRoll = m_CursorRenderer.GetCursorRoll();

	if (m_isRandomRotation)
		fRoll = GetRandom(0.0f, 360.0f);

	int ix, iy;
	for (uint32_t i = 0; i < m_CursorRenderer.GetSettingCursorCount(); ++i)
	{
		const Vector3 & c_rv3CursorPosition = m_CursorRenderer.GetCursorPosition();
		m_CursorRenderer.GetSettingCursorPosition(i, &ix, &iy);
		m_pMapManagerAccessor->InsertObject(c_rv3CursorPosition.x + ix,
			c_rv3CursorPosition.y + iy,
			m_CursorRenderer.GetObjectHeight(),
			m_CursorRenderer.GetCursorYaw(),
			m_CursorRenderer.GetCursorPitch(),
			fRoll,
			m_CursorRenderer.GetCursorScale(),
			dwObjectCRC);

		CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlPropertyTree.SetFocus();
	}

	m_pMapManagerAccessor->BackupObjectCurrent();
}

//Scene 화면에 배치된 오브젝트를 선택 또는 해제합니다(종료)
void CSceneMap::ObjectSelectAndDeselectEnd(SHORT sAddedSelectKey, SHORT sDeselectKey)
{
	m_CursorRenderer.PickEnd();
	m_CursorRenderer.SelectEnd();
	m_CursorRenderer.DeselectEnd();

	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	const auto picked = m_pMapManagerAccessor->GetPickedObjectIndex();

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();

	if (!picked)
	{
		if(0 == sDeselectKey)
		{
			if(0 == sAddedSelectKey)
			{
				if(!m_pMapManagerAccessor->IsAddedSelectObject())
					m_pMapManagerAccessor->CancelSelect();
			}
		}
	}
	else
	{
		if(0 == sDeselectKey)
		{
			if(0 == sAddedSelectKey)
			{
				if(!m_pMapManagerAccessor->IsAddedSelectObject() && !m_bObjectIsMoved)
				{
					m_pMapManagerAccessor->CancelSelect();
					m_pMapManagerAccessor->SelectObject(picked);
				}
			}
		}
		else
		{
			m_pMapManagerAccessor->DeselectObject(picked);
		}
	}

	if (m_bObjectIsMoved)
	{
		m_pMapManagerAccessor->BackupObjectCurrent();
		m_bObjectIsMoved		= FALSE;
	}

	m_pMapManagerAccessor->ClearAddedSelectObject();
	m_pMapManagerAccessor->ClearDeselectObject();
}

//Scene 화면에 오브젝트를 추가합니다(종료)
void CSceneMap::InsertObjectOnTheSceneEnd()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	if (m_bObjectIsMoved)
		m_pMapManagerAccessor->BackupObjectCurrent();

	m_bObjectIsMoved = FALSE;
}

/*
* 오브젝트 탭에서 오브젝트 선택시 마우스커서에 오브젝트가 Attach 된다.
* 반대로 Detach 시켜주는 함수.
*  (목)
* DetachObject 시 왼쪽 탭에 프랍리스트가 닫히지 않게 해달라고 요청하여 수정함.
*/

void CSceneMap::DetachObject()
{
	__ClearCursor();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	HTREEITEM hCurItem = pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlPropertyTree.GetSelectedItem();
	HTREEITEM hParentItem = pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlPropertyTree.GetParentItem(hCurItem);
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlPropertyTree.SelectItem(hParentItem);
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlPropertyTree.Expand(hParentItem, TVE_EXPAND);

	ChangeCursor(0);
}

/*
* 오브젝트 탭의 Rotation 정보 Clear 시킨다.
* 오브젝트 탭의 Rotation 정보는
* CCursorRenderer 의 yaw,pitch,roll 정보를 기반으로 셋팅되기 때문에
* 이를 응용하여 작성.
*/
void CSceneMap::ClearRotation()
{
	SetCursorYawPitchRoll(0.0f, 0.0f, 0.0f);
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->UpdateMapControlBar();
}

/*
* 오브젝트 탭의 Height Bias 정보 Clear 시킨다.
* 오브젝트 탭의 Height Bias 정보는
* CCursorRenderer 의 m_fObjectHeight 정보를 기반으로 셋팅되기 때문에
* 이를 응용하여 작성.
*/
void CSceneMap::ClearHeightBias()
{
	SetObjectHeight(0.0f);
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->UpdateMapControlBar();
}

/*
*  에디트모드(오브젝트) Rotation 탭의 Yaw 항목 Focus
*  단축키 추가하는 과정에서 포커스만 설정해주고 수치는 휠키로 조정
*/
void CSceneMap::RotationYawFocus()
{
	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlCursorYaw.SetFocus();
}

/*
*  에디트모드(오브젝트) Rotation 탭의 Pitch 항목 Focus
*  단축키 추가하는 과정에서 포커스만 설정해주고 수치는 휠키로 조정
*/
void CSceneMap::RotationPitchFocus()
{
	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlCursorPitch.SetFocus();
}

/*
*  에디트모드(오브젝트) Rotation 탭의 Roll 항목 Focus
*  단축키 추가하는 과정에서 포커스만 설정해주고 수치는 휠키로 조정
*/
void CSceneMap::RotationRollFocus()
{
	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlCursorRoll.SetFocus();
}

/*
*  에디트모드(오브젝트) Height Bias 탭의 수치 항목 Focus
*  단축키 추가하는 과정에서 포커스만 설정해주고 수치는 휠키로 조정
*/
void CSceneMap::HeightBiasFocus()
{
	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->m_wndMapCtrlBar.m_pageObject.m_ctrlObjectHeight.SetFocus();
}

/* (화) Object Render 상태를 VK_C 키로 껐다 켰다 할수 있게 추가
*  충돌탭에서 오브젝트 출력을 제어할 필요가 있다고 하여 추가함.
*/
void CSceneMap::ObjectRenderEnable()
{
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hFilePage = pFrame->m_wndMapCtrlBar.m_pageFile.GetSafeHwnd();

	if (NULL == hFilePage)
		return;

	UINT uChecked = IsDlgButtonChecked(hFilePage, IDC_MAP_TERRAIN_OPTION_OBJECT);
	CheckDlgButton(hFilePage, IDC_MAP_TERRAIN_OPTION_OBJECT, !uChecked);
	SetObjectRendering(!uChecked);
}

/* (화) Collision Render 껐다 켰다 할수 있게 추가
*  현재는 사용X
*/
void CSceneMap::ObjectCollisionRenderEnable()
{
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hMapFile = pFrame->m_wndMapCtrlBar.m_pageFile.GetSafeHwnd();

	if (NULL == hMapFile)
		return;

	UINT uChecked = IsDlgButtonChecked(hMapFile, IDC_MAP_TERRAIN_OPTION_OBJECT_COLLISION);
	CheckDlgButton(hMapFile, IDC_MAP_TERRAIN_OPTION_OBJECT_COLLISION, !uChecked);
	SetObjectCollisionRendering(!uChecked);
}

/* (화) 에디트모드(오브젝트) snap 활성,비활성 기능 추가
*/
void CSceneMap::SnapEnable()
{
	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hObjectPage = pFrame->m_wndMapCtrlBar.m_pageObject.GetSafeHwnd();

	UINT uChecked = IsDlgButtonChecked(hObjectPage, IDD_MAP_OBJECT_GRID_MODE);
	CheckDlgButton(hObjectPage, IDD_MAP_OBJECT_GRID_MODE, !uChecked);
	SetGridMode(!uChecked);
}

/* (월) 에디트모드(터레인) water brush check 기능 추가
*  brush check 시 water rendering 도 같이 활성
*/
void CSceneMap::WaterBrushCheck()
{
	if (NULL == m_pMapManagerAccessor || !m_pMapManagerAccessor->IsMapReady())
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hTerrainPage = pFrame->m_wndMapCtrlBar.m_pageTerrain.GetSafeHwnd();
	HWND hFilePage = pFrame->m_wndMapCtrlBar.m_pageFile.GetSafeHwnd();

	if (NULL == hTerrainPage || NULL == hFilePage)
		return;

	CheckDlgButton(hTerrainPage, IDC_CHECK_HEIGHTBRUSH, 0);
	CheckDlgButton(hTerrainPage, IDC_CHECK_TEXTUREBRUSH, 0);
	CheckDlgButton(hTerrainPage, IDC_CHECK_WATERBRUSH, 1);

	m_pMapManagerAccessor->SetHeightEditing(false);
	m_pMapManagerAccessor->SetTextureEditing(false);
	m_pMapManagerAccessor->SetWaterEditing(true);
	m_pMapManagerAccessor->SetAttrEditing(false);

	//water rendering ? ?? ??
	CheckDlgButton(hFilePage, IDC_CHECK_WATER, 1);
	SetWaterRendering(true);
}

/* (월) Water Eraser 기능 단축키 제공을 위해 추가
*/
void CSceneMap::WaterEraserEnable(bool bFlag)
{
	if (NULL == m_pMapManagerAccessor || !m_pMapManagerAccessor->IsMapReady())
		return;

	if (!m_pMapManagerAccessor->isWaterEditing())
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hTerrainPage = pFrame->m_wndMapCtrlBar.m_pageTerrain.GetSafeHwnd();

	if(NULL == hTerrainPage)
		return;

	CheckDlgButton(hTerrainPage, IDC_CHECK_ERASERWATER, bFlag);
	m_pMapManagerAccessor->SetEraseWater(bFlag);
}

/* (수) Attribute Eraser 기능 단축키 제공을 위해 추가
*/
void CSceneMap::AttributeEraserEnable(bool bFlag)
{
	if (NULL == m_pMapManagerAccessor || !m_pMapManagerAccessor->IsMapReady() )
		return;

	if (EDITING_MODE_ATTRIBUTE != m_iEditingMode || !m_pMapManagerAccessor->isAttrEditing())
		return;

	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	HWND hAttributePage = pFrame->m_wndMapCtrlBar.m_pageAttribute.GetSafeHwnd();

	if (NULL == hAttributePage)
		return;

	CheckDlgButton(hAttributePage, IDC_CHECK_ATTRERASER, bFlag);
	m_pMapManagerAccessor->SetEraseAttr(bFlag);
}

/* (수) 추가. shadowmap, minimap 을 갱신하여 dds 파일로 저장해준다.
*  OnRender 부분의 관련 기능을 함수로 뺐다.
*  기존의 코드를 최대한 유지하면서 몇몇개 기능 수정
*  수정 1 : 기능 수행후 카메라 및 구역이동이 일어나는 현상 수정.
*  수정 2 : m_bTerrainShadowMapAndMiniMapUpdateAll 변수를 통하여
            shadowmap, minimap 생성을 전체구역 또는 해당구역만 수행할수 있도록 두가지 방식 제공.
*/
void CSceneMap::TerrainShadowMapAndMiniMapUpdate()
{
	if (!m_bTerrainShadowMapAndMiniMapUpdateNeeded)
		return;

	CTerrainAccessor * pTerrainAccessor;
	if (!m_pMapAccessor->GetTerrainPointer(4, (CTerrain **) &pTerrainAccessor))
		return;

	CMainFrame * pFrame			= (CMainFrame*)AfxGetMainWnd();
	CWorldEditorView * pView	= (CWorldEditorView *)pFrame->GetActiveView();

	//상태 복구용 데이터 저장
	WORD wOx, wOy;
	pTerrainAccessor->GetCoordinate(&wOx, &wOy);	//현재 몇구역에 위치하는지
	const Vector3 vEye		= CCameraManager::Instance().GetCurrentCamera()->GetEye();
	const Vector3 vTarget	= CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	const Vector3 vUp		= CCameraManager::Instance().GetCurrentCamera()->GetUp();


	if (m_bTerrainShadowMapAndMiniMapUpdateAll)
	{
		//현재 map의 size. 몇 by 몇 짜리 맵인지
		short wX, wY;
		m_pMapAccessor->GetTerrainCount(&wX, &wY);

		for (short x = 0; x < wX; x++)
		{
			for (short y = 0; y < wY; y++)
			{
				m_pMapManagerAccessor->SetTerrainModified();
				m_pMapManagerAccessor->UpdateMap((float)(x * CTerrainImpl::TERRAIN_XSIZE), - (float)(y * CTerrainImpl::TERRAIN_YSIZE), 0.0f);
				pView->UpdateTargetPosition((float)(x * CTerrainImpl::TERRAIN_XSIZE) - vTarget.x, - (float)(y * CTerrainImpl::TERRAIN_YSIZE) - vTarget.y);
				CCullingManager::Instance().Process();
				CCullingManager::Instance().Update();
				CEffectManager::Instance().Update();
				m_pMapManagerAccessor->UpdateTerrainShadowMap();
				m_pMapManagerAccessor->ReloadTerrainShadowTexture(false);
				m_pMapManagerAccessor->SaveMiniMap();
			}
		}
	}
	else
	{
		m_pMapManagerAccessor->SetTerrainModified();
		m_pMapManagerAccessor->UpdateMap((float)(wOx * CTerrainImpl::TERRAIN_XSIZE), - (float)(wOy * CTerrainImpl::TERRAIN_YSIZE), 0.0f);
		pView->UpdateTargetPosition((float)(wOx * CTerrainImpl::TERRAIN_XSIZE) - vTarget.x, - (float)(wOy * CTerrainImpl::TERRAIN_YSIZE) - vTarget.y);
		CCullingManager::Instance().Process();
		CCullingManager::Instance().Update();
		CEffectManager::Instance().Update();
		m_pMapManagerAccessor->UpdateTerrainShadowMap();
		m_pMapManagerAccessor->ReloadTerrainShadowTexture(false);
		m_pMapManagerAccessor->SaveMiniMap();
	}

	//상태 복구
	CCameraManager::Instance().GetCurrentCamera()->SetViewParams(vEye, vTarget, vUp);
	pTerrainAccessor->SetCoordinate(wOx, wOy);
	m_pMapManagerAccessor->UpdateMap(vTarget.x, vTarget.y, vTarget.z);
	UpdateStatusBar();

	m_bTerrainShadowMapAndMiniMapUpdateNeeded = false;
	m_bTerrainShadowMapAndMiniMapUpdateAll = false;
}

/* (수)
*  특정 탭에서의 Undo/Redo 기능을 막기위해 검사함수 추가
*/
bool CSceneMap::EditingModeCheckForUndoRedo()
{
	if (EDITING_MODE_ATTRIBUTE == m_iEditingMode || EDITING_MODE_ENVIRONMENT == m_iEditingMode)
		return false;

	return true;
}

/* (수)
*  다른탭에서도 속성을 볼수 있도록 단축키 추가
*/
void CSceneMap::AttributeRenderEnable()
{
	m_bAttrRenderingOn = !m_bAttrRenderingOn;
}

//--------------------------------------------------------------
/* (목)
*  hold 기능을 위해 추가
*/
void CSceneMap::SwapHoldOn()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();

	if (m_bHoldOn) {
		m_bHoldOn = false;
		m_bGizmoOn = false;
		m_iGizmoSelectedAxis = -1;
		m_pickedObject = {0, -1};
		pFrame->UpdateStatusBarHoldIcon();
	} else {
		if (m_pMapManagerAccessor->GetSelectedObjectCount() > 0)
		{
			m_bHoldOn = true;
			m_iGizmoSelectedAxis = -1;
			m_pickedObject = {0, -1};
			pFrame->UpdateStatusBarHoldIcon();
		}
	}
}

/* (목)
*  오브젝트 Hold 기능 추가
*/
void CSceneMap::HoldStart()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	m_pMapManagerAccessor->BackupObject();

	AreaObjectRef ref;
	if (!GizmoChecking(ref))
		return;

	GizmoUpdate(ref);
	pFrame->UpdateMapControlBar();
}

/* (목)
*  오브젝트 Hold 기능 추가
*/
void CSceneMap::HoldEnd()
{
	if (!m_pMapManagerAccessor->IsMapReady())
		return;

	if (EDITING_MODE_OBJECT != m_iEditingMode)
		return;

	m_CursorRenderer.PickEnd();
}
//--------------------------------------------------------------

/* (화)
*  Hold 기능 내부에서만 사용하도록 만들어서 private 로 했음.
*  1. 기즈모 출력 여부 체크
*  2. 클릭한 물체 index 담아줌.
*  3. 선택된 물체를 클릭시 TRUE, 아닐시 FALSE 리턴
*/
bool CSceneMap::GizmoChecking(AreaObjectRef& ref)
{
	//기즈모 축중에서 선택한 축 확인
	m_iGizmoSelectedAxis = m_cPickingArrows.Picking();

	//클릭한 물체 index 확인
	ref = m_pMapManagerAccessor->GetPickedObjectIndex();

	//클릭한 물체가 선택된 물체중에서 클릭한것인지 아닌지 판별
	bool bSelected = m_pMapManagerAccessor->IsSelectedObject(ref);
	if (bSelected) { //선택O
		m_bGizmoOn = true;
		m_pickedObject = ref;
	} else { //선택X
		if (-1 == m_iGizmoSelectedAxis) //기즈모클릭X
			m_bGizmoOn = false;
		else //기즈모클릭O
			m_bGizmoOn = true;
	}

	m_CursorRenderer.PickStart();
	return bSelected;
}

/* (화)
*  Hold 기능 내부에서만 사용하도록 만들어서 private 로 했음.
*  1. 선택한 물체에 index 값을 바탕으로 데이터 구해옴.
*  2. 해당 물체의 위치에 맞게 기즈모가 출력되게 Position 설정.
*/
void CSceneMap::GizmoUpdate(const AreaObjectRef& ref)
{
	if (!ref)
		return;

	const CArea::TObjectData* pObjectData = NULL;
	if (!m_pMapManagerAccessor->GetObjectDataPointer(ref, &pObjectData))
		return;

	Vector3 v3GizmoPosition(0.0f, 0.0f, 0.0f);
	Vector3 v3Max(0.0f, 0.0f, 0.0f);
	Vector3 v3Min(0.0f, 0.0f, 0.0f);

	const CArea::TObjectInstance* pObjectInstanceData = NULL;
	if (m_pMapManagerAccessor->GetObjectInstancePointer(ref, &pObjectInstanceData))
	{
		switch(pObjectInstanceData->dwType)
		{
		case PROPERTY_TYPE_BUILDING:
			pObjectInstanceData->pThingInstance->GetBoundBox( &v3Min, &v3Max );
			v3GizmoPosition = (v3Max + v3Min) / 2.0f;
			v3GizmoPosition += pObjectData->Position;
			v3GizmoPosition.z += pObjectData->m_fHeightBias;
			break;

		case PROPERTY_TYPE_DUNGEON_BLOCK:
			pObjectInstanceData->pDungeonBlock->GetBoundBox( &v3Min, &v3Max );
			v3GizmoPosition = (v3Max + v3Min) / 2.0f;
			v3GizmoPosition += pObjectData->Position;
			v3GizmoPosition.z += pObjectData->m_fHeightBias;
			break;

		case PROPERTY_TYPE_TREE:
		case PROPERTY_TYPE_EFFECT:
			v3GizmoPosition = pObjectData->Position;
			v3GizmoPosition.z += pObjectData->m_fHeightBias + 300.0f;
			break;
		}
	}

	m_cPickingArrows.SetCenterPosition(v3GizmoPosition);
}

void CSceneMap::GizmoOff()
{
	m_bGizmoOn = false;
	m_iGizmoSelectedAxis = -1;
}

METIN2_END_NS
