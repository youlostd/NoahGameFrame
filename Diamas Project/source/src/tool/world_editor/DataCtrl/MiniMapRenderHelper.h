#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_MINIMAPRENDERHELPER_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_MINIMAPRENDERHELPER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMiniMapRenderHelper : public CScreen, public CSingleton<CMiniMapRenderHelper>
{
public:
	CMiniMapRenderHelper();
	~CMiniMapRenderHelper();

	bool CreateTextures();
	void ReleaseTextures();

	// Assign
	void SetMapOutdoorAccssorPointer(CMapOutdoorAccessor * pMapOutdoorAccessor) { m_pMapOutdoorAccessor = pMapOutdoorAccessor;	}
	void SetTargetTerrainCoord(WORD wCoordX, WORD wCoordY) { m_wCurCoordX = wCoordX; m_wCurCoordY = wCoordY; }

	void SetMiniMapPower(BYTE byMiniMapPower);
	void SetMiniMapFilter(D3DTEXTUREFILTERTYPE eTextureFilter);

	LPDIRECT3DTEXTURE9 GetMiniMapTexture() { return m_lpMiniMapRenderTargetTexture; }

	bool StartRendering();
	void EndRendering();

private:
	// Backup
	bool SaveRenderTarget();
	void RestoreRenderTarget();

	// CMapOutdoorAccessor Pointer
	CMapOutdoorAccessor *	m_pMapOutdoorAccessor;

	// 랜더링할 땅 좌표
	WORD					m_wCurCoordX;
	WORD					m_wCurCoordY;

	// Size
	BYTE					m_byMiniMapPower;
	uint32_t					m_dwMiniMapSize;

	// Shadow Map
	LPDIRECT3DSURFACE9		m_lpMiniMapRenderTargetSurface;
	LPDIRECT3DSURFACE9		m_lpMiniMapDepthSurface;
	LPDIRECT3DTEXTURE9		m_lpMiniMapRenderTargetTexture;
	D3DVIEWPORT9			m_MiniMapViewport;

	D3DTEXTUREFILTERTYPE	m_eMiniMapTextureFilter;

	// Backup
	LPDIRECT3DSURFACE9		m_lpBackupRenderTargetSurface;
	LPDIRECT3DSURFACE9		m_lpBackupDepthSurface;
	D3DVIEWPORT9			m_BackupViewport;
	D3DXMATRIX				m_matBackupProj;

};

METIN2_END_NS

#endif
