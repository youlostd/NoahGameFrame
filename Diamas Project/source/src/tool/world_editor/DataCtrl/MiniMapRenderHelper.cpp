#include "stdafx.h"
#include "../WorldEditor.h"
#include "MiniMapRenderHelper.h"
#include "MapAccessorOutdoor.h"

#include <EterLib/Camera.h>

#include <SpdLog.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

METIN2_BEGIN_NS

CMiniMapRenderHelper aMiniMapRenderHelper;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMiniMapRenderHelper::CMiniMapRenderHelper() :
m_lpMiniMapRenderTargetSurface(NULL),
m_lpMiniMapDepthSurface(NULL),
m_lpMiniMapRenderTargetTexture(NULL),
m_lpBackupRenderTargetSurface(NULL),
m_lpBackupDepthSurface(NULL),
m_pMapOutdoorAccessor(NULL)
{
	m_wCurCoordX = m_wCurCoordY = 0;
	SetMiniMapPower(8);
	m_eMiniMapTextureFilter = D3DTEXF_LINEAR;
}

CMiniMapRenderHelper::~CMiniMapRenderHelper()
{
	ReleaseTextures();
}

void CMiniMapRenderHelper::SetMiniMapPower(BYTE byMiniMapPower)
{
	m_byMiniMapPower = byMiniMapPower;

	m_dwMiniMapSize = 1 << m_byMiniMapPower;

	m_MiniMapViewport.X = 0;
	m_MiniMapViewport.Y = 0;
	m_MiniMapViewport.Width = m_dwMiniMapSize;
	m_MiniMapViewport.Height = m_dwMiniMapSize;
	m_MiniMapViewport.MinZ = 0.0f;
	m_MiniMapViewport.MaxZ = 1.0f;
}

bool CMiniMapRenderHelper::StartRendering()
{
	CTerrain * pTerrain;
	const TOutdoorMapCoordinate & rOutdoorMapCoordinate = m_pMapOutdoorAccessor->GetCurCoordinate();

	BYTE byTerrainNum;
	if (!m_pMapOutdoorAccessor->GetTerrainNumFromCoord(m_wCurCoordX, m_wCurCoordY, &byTerrainNum))
		return false;

	if (!m_pMapOutdoorAccessor->GetTerrainPointer(byTerrainNum, &pTerrain))
		return false;

	bool bSuccess = true;

	Vector3 v3Target = Vector3(
					((float) m_wCurCoordX * 2.0f + 1.0f) * 12800.0f,
					-((float) m_wCurCoordY * 2.0f + 1.0f) * 12800.0f,
					pTerrain->GetHeight((m_wCurCoordX * 2 + 1) * 12800, (m_wCurCoordY * 2 + 1) * 12800));

	Vector3 v3Eye(v3Target.x, v3Target.y, v3Target.z + 32767.5f);

	Matrix matTopProj;
	DirectX::SimpleMath::MatrixOrthoRH(&matTopProj, 25600.0f, 25600.0f, 0.0f, 65535.0f);

	m_matBackupProj = ms_matProj;

	CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_ORTHO_CAMERA);
	CCameraManager::Instance().GetCurrentCamera()->SetViewParams(v3Eye, v3Target, Vector3(0.0f, 1.0f, 0.0f));
	ms_matProj = matTopProj;
	UpdatePipeLineMatrix();

	CSpeedTreeForest::Instance().UpdateCompoundMatrix(v3Eye, ms_matView, ms_matProj);

	SaveRenderTarget();

	if (FAILED(ms_lpd3dDevice->SetRenderTarget(0, m_lpMiniMapRenderTargetSurface)))
	{
		spdlog::error("CMiniMapRenderHelper::StartRenderingPhase Unable to Set Mini Map Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetDepthStencilSurface(m_lpMiniMapDepthSurface)))
	{
		spdlog::error("CMiniMapRenderHelper::StartRenderingPhase Unable to Set Mini Map DepthStencilSurface");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0f, 0)))
	{
		spdlog::error("CMiniMapRenderHelper::StartRenderingPhase Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		spdlog::error("Unable to Save Window Viewport");
		return false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_MiniMapViewport)))
	{
		spdlog::error("SetViewport Error : Phase 2");
		bSuccess = false;
	}

	return bSuccess;
}

void CMiniMapRenderHelper::EndRendering()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);
	RestoreRenderTarget();

	CCameraManager::Instance().ResetToPreviousCamera();
	ms_matProj = m_matBackupProj;
	UpdatePipeLineMatrix();

	CSpeedTreeForest::Instance().UpdateCompoundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);
}

bool CMiniMapRenderHelper::SaveRenderTarget()
{
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
	{
		spdlog::error("Unable to Save Window Render Target");
		return false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		spdlog::error("Unable to Save Window Depth Surface");
		return false;
	}

	return true;
}

void CMiniMapRenderHelper::RestoreRenderTarget()
{
	if (m_lpBackupRenderTargetSurface == NULL && m_lpBackupDepthSurface == NULL)
		return;

	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);

	SAFE_RELEASE(m_lpBackupDepthSurface);
	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
}

bool CMiniMapRenderHelper::CreateTextures()
{
	// 처음 한번만 만든다.
	if (m_lpMiniMapRenderTargetTexture)
		return true;

	// Release any old textures
	ReleaseTextures();

	// Mini Map
	if (FAILED(ms_lpd3dDevice->CreateTexture(m_dwMiniMapSize,
	                                         m_dwMiniMapSize,
	                                         1,
	                                         D3DUSAGE_RENDERTARGET,
	                                         D3DFMT_X8R8G8B8,
	                                         D3DPOOL_DEFAULT,
	                                         &m_lpMiniMapRenderTargetTexture,
	                                         NULL)))
	{
		spdlog::error("Unable to create MiniMap render target texture");
		return false;
	}

	if (FAILED(m_lpMiniMapRenderTargetTexture->GetSurfaceLevel(0, &m_lpMiniMapRenderTargetSurface)))
	{
		spdlog::error("Unable to GetSurfaceLevel MiniMap render target texture");
		return false;
	}

	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_dwMiniMapSize,
	                                                     m_dwMiniMapSize,
	                                                     D3DFMT_D16,
	                                                     D3DMULTISAMPLE_NONE,
	                                                     0,
	                                                     FALSE,
	                                                     &m_lpMiniMapDepthSurface,
	                                                     NULL)))
	{
		spdlog::error("Unable to create Output MiniMap depth Surface");
		return false;
	}

	return true;
}

void CMiniMapRenderHelper::ReleaseTextures()
{
	SAFE_RELEASE(m_lpMiniMapDepthSurface);
	SAFE_RELEASE(m_lpMiniMapRenderTargetTexture);
	SAFE_RELEASE(m_lpMiniMapRenderTargetSurface);
}

METIN2_END_NS
