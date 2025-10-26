#include "StdAfx.h"
#include "MapAccessorOutdoor.h"
#include "MapAccessorTerrain.h"
#include "../Dialog/MapArrangeHeightProgressDialog.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../WorldEditorView.h"

#include <GameLib/TerrainPatch.h>
#include <GameLib/TerrainQuadtree.h>
#include <GameLib/EnvironmentData.hpp>
#include <EterLib/Camera.h>
#include <EterLib/CullingManager.h>
#include <EterBase/Utils.h>

#include <lzo/LzoCompressor.hpp>
#include <lzo/LzoConfig.hpp>

#include <SpdLog.hpp>

#include <vstl/utility/round.hpp>

#include <sys/stat.h>

// TODO(tim): remove once timeGetTime calls are gone
#include "EterLib/Engine.hpp"

#include <mmsystem.h>
#include <storm/io/StreamUtil.hpp>

METIN2_BEGIN_NS

CMapOutdoorAccessor *			CMapOutdoorAccessor::ms_pThis = NULL;
CMapArrangeHeightProgress *		CMapOutdoorAccessor::ms_pkProgressDialog = NULL;

//////////////////////////////////////////////////////////////////////////
// CMapOutdoorAccessor
//////////////////////////////////////////////////////////////////////////

CMapOutdoorAccessor::CMapOutdoorAccessor()
{
	m_iMapID = 0;
	m_fSplatValue = 4.6f;
	m_selectedEnvironment = 0;
	ms_pkProgressDialog = NULL;
	ms_pThis = this;

	//m_environments.emplace(0, EnvironmentData());
}

CMapOutdoorAccessor::~CMapOutdoorAccessor()
{
	Destroy();
}

float CMapOutdoorAccessor::GetSplatValue()
{
	return m_fSplatValue;
}

void CMapOutdoorAccessor::SetSplatValue(float fValue)
{
	m_fSplatValue = fValue;

	m_matSplatAlpha._41 = m_fTerrainTexCoordBase * m_fSplatValue;
	m_matSplatAlpha._42 = m_fTerrainTexCoordBase * m_fSplatValue;
}

void CMapOutdoorAccessor::OnPreAssignTerrainPtr()
{
	if (!m_bReady)
		return;
	SaveTerrains();
	SaveAreas();
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	pFrame->UpdateMapControlBar();
}

bool CMapOutdoorAccessor::SaveProperty(storm::String strfileName)
{
	strfileName += "\\MapProperty.txt";
	FILE * File = fopen(strfileName.c_str(), "w");

	if (!File)
		return false;

	fprintf(File, "ScriptType MapProperty\n");
	fprintf(File, "\n");

	if (CMapBase::MAPTYPE_INDOOR == GetType())
		fprintf(File, "MapType \"Indoor\"\n");
	else if (CMapBase::MAPTYPE_OUTDOOR == GetType())
		fprintf(File, "MapType \"Outdoor\"\n");
	else
		fprintf(File, "MapType \"Invalid\"\n");
	fprintf(File, "\n");

	fclose(File);
	return true;
}

bool CMapOutdoorAccessor::SaveSetting(storm::String strfileName)
{
	strfileName += "\\Setting.txt";

	FILE * File = fopen(strfileName.c_str(), "w");

	if (!File)
		return false;

	fprintf(File, "ScriptType\tMapSetting\n");
	fprintf(File, "\n");

	fprintf(File, "CellScale\t%ld\n", CTerrainImpl::CELLSCALE);

	if (0.0f != GetHeightScale())
		fprintf(File, "HeightScale\t%f\n", GetHeightScale());
	else
		fprintf(File, "HeightScale\t%f\n", 0.5f);

	fprintf(File, "\n");

	if (0L != GetViewRadius())
		fprintf(File, "ViewRadius\t%ld\n", GetViewRadius() / 4);
	else
		fprintf(File, "ViewRadius\t%ld\n", 128);

	fprintf(File, "\n");

	short sTerrainCountX, sTerrainCountY;
	GetTerrainCount(&sTerrainCountX, &sTerrainCountY);

	if (0 != sTerrainCountX && 0 != sTerrainCountY)
		fprintf(File, "MapSize\t%u\t%u\n", sTerrainCountX, sTerrainCountY);
	else
		fprintf(File, "MapSize\t%u\t%u\n", 1, 1);

	fprintf(File, "TextureSet\t%s\n", m_TextureSet.GetFileName());

	fprintf(File, "Environment\t\"%s\"\n",
	        m_environmentPaths[m_selectedEnvironment].c_str());

	fprintf(File, "\n");

	fclose(File);
	return true;
}

bool CMapOutdoorAccessor::SaveTerrains()
{
	for (auto& terrain : m_TerrainVector) {
		auto terrainAccessor = static_cast<CTerrainAccessor*>(terrain.get());
		if (!terrainAccessor->Save(m_strName)) {
			WORD usCoordX, usCoordY;
			terrain->GetCoordinate(&usCoordX, &usCoordY);

			spdlog::error("Failed to save {0} {1}:{2}",
			          m_strName, usCoordX, usCoordY);
		}
	}

	return true;
}

bool CMapOutdoorAccessor::SaveAreas()
{
	for (auto& area : m_AreaVector) {
		auto areaAccessor = static_cast<CAreaAccessor*>(area.get());
		if (!areaAccessor->Save(m_strName)) {
			WORD usCoordX, usCoordY;
			area->GetCoordinate(&usCoordX, &usCoordY);

			spdlog::error("Failed to save {0} {1}:{2}",
			          m_strName, usCoordX, usCoordY);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMapOutdoorAccessor::DrawHeightBrush(uint32_t dwBrushShape,
					 uint32_t dwBrushType,
					 WORD wTerrainNumX,
					 WORD wTerrainNumY,
					 long lCellX,
					 long lCellY,
					 BYTE byBrushSize,
					 BYTE byBrushStrength)
{
	BYTE ucTerrainNum;
	if ( !GetTerrainNumFromCoord(wTerrainNumX, wTerrainNumY, &ucTerrainNum) )
		return;

	CTerrainAccessor * pTerrainAccessor = NULL;

	// Phase1 : 하이트 필드 조정

	// 최대 4개로 나뉜다... 그리고 그리는 순서는 항상 왼쪽 위, 오른쪽 위, 왼쪽 아래, 오른쪽 아래
	if (lCellY < byBrushSize)
	{
		if (lCellX < byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum - 4, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX + CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum - 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY + CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum - 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX + CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
		}
		else if (lCellX > CTerrainImpl::XSIZE - 1 - byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum - 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY + CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum - 2, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX - CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX - CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
		}
		else
		{
			if ( GetTerrainPointer(ucTerrainNum - 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY + CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
		}
	}
	else if (lCellY > (CTerrainImpl::XSIZE - 1) - byBrushSize)
	{
		if (lCellX < byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum - 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX + CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 2, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX + CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY - CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX - CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY - CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 4, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX - CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
		}
		else
		{
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 3, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY - CTerrainImpl::YSIZE, byBrushSize, byBrushStrength);
		}
	}
	else
	{
		if (lCellX < byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum - 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX + CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
			if ( GetTerrainPointer(ucTerrainNum + 1, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX - CTerrainImpl::XSIZE, lCellY, byBrushSize, byBrushStrength);
		}
		else
		{
			if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
				pTerrainAccessor->DrawHeightBrush(dwBrushShape, dwBrushType, lCellX, lCellY, byBrushSize, byBrushStrength);
		}
	}

	SetTerrainModified();
}

void CMapOutdoorAccessor::DrawTextureBrush(uint32_t dwBrushShape,
										   std::vector<BYTE> & rVectorTextureNum,
										   WORD wTerrainNumX,
										   WORD wTerrainNumY,
										   long lCellX,
										   long lCellY,
										   BYTE bySubCellX,
										   BYTE bySubCellY,
										   BYTE byBrushSize,
										   bool bErase,
										   bool bDrawOnlyOnBlankTile)
{
	CTerrainAccessor * pTerrainAccessor = NULL;
	BYTE byTerrainNum;
	if(!GetTerrainNumFromCoord(wTerrainNumX, wTerrainNumY, &byTerrainNum))
		return;

	// 여러개로 나눈다...
	if (GetTerrainPointer(byTerrainNum, (CTerrain **) &pTerrainAccessor))
		pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);

	if (lCellX < byBrushSize + 1)
	{
		if (GetTerrainPointer(byTerrainNum - 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX + CTerrainImpl::XSIZE, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
	}
	else if (lCellX > (CTerrainImpl::XSIZE - 2) - byBrushSize)
	{
		if (GetTerrainPointer(byTerrainNum + 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX - CTerrainImpl::XSIZE, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
	}

	if (lCellY < byBrushSize + 1)
	{
		if (GetTerrainPointer(byTerrainNum - 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);

		if (lCellX < byBrushSize + 1)
		{
			if (GetTerrainPointer(byTerrainNum - 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX + CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 2) - byBrushSize)
		{
			if (GetTerrainPointer(byTerrainNum - 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX - CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
		}
	}
	else if (lCellY > (CTerrainImpl::YSIZE - 2) - byBrushSize)
	{
		if (GetTerrainPointer(byTerrainNum + 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);

		if (lCellX < byBrushSize + 1)
		{
			if (GetTerrainPointer(byTerrainNum + 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX + CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 2) - byBrushSize)
		{
			if (GetTerrainPointer(byTerrainNum + 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawTextureBrush(dwBrushShape, rVectorTextureNum, lCellX - CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase, bDrawOnlyOnBlankTile);
		}
	}

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		if (!GetTerrainPointer(i, (CTerrain **) &pTerrainAccessor))
			continue;

		pTerrainAccessor->RAW_GenerateSplat();
	}
}

void CMapOutdoorAccessor::ResetAttrSplats()
{
	CTerrainAccessor * pTerrainAccessor = NULL;

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		if (!GetTerrainPointer(i, (CTerrain **) &pTerrainAccessor))
			continue;

		pTerrainAccessor->RAW_ResetAttrSplat();
	}
}

void CMapOutdoorAccessor::DrawAttrBrush(uint32_t dwBrushShape,
										BYTE byAttrFlag,
										WORD wTerrainNumX,
										WORD wTerrainNumY,
										long lCellX,
										long lCellY,
										BYTE bySubCellX,
										BYTE bySubCellY,
										BYTE byBrushSize,
										bool bErase)
{
	CTerrainAccessor * pTerrainAccessor = NULL;
	BYTE ucTerrainNum = (wTerrainNumY - m_CurCoordinate.m_sTerrainCoordY + 1) * 3 + (wTerrainNumX - m_CurCoordinate.m_sTerrainCoordX + 1);

	// 여러개로 나눈다...
	if ( GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor) )
		pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase);

	if (lCellY < byBrushSize)
	{
		if (wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		if (lCellX < byBrushSize)
		{
			if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX + CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX - CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		}
	}
	else if (lCellY > (CTerrainImpl::YSIZE - 1) - byBrushSize)
	{
		if (wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		if (lCellX < byBrushSize)
		{
			if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX + CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX - CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, bySubCellX, bySubCellY, byBrushSize, bErase);
		}
	}

	if (lCellX < byBrushSize)
	{
		if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && GetTerrainPointer(ucTerrainNum - 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX + CTerrainImpl::XSIZE, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase);
	}
	else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
	{
		if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && GetTerrainPointer(ucTerrainNum + 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawAttrBrush(dwBrushShape, byAttrFlag, lCellX - CTerrainImpl::XSIZE, lCellY, bySubCellX, bySubCellY, byBrushSize, bErase);
	}

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		if (!GetTerrainPointer(i, (CTerrain **) &pTerrainAccessor))
			continue;
		pTerrainAccessor->RAW_UpdateAttrSplat();
	}
}

/* brush 기능 정상화
*  brush가 의도치 않은 위치에 표시되었었다.
*  world 행렬을 셋팅 안해줘서 그런듯.
*  water brush 만 다르게 표시하고 싶어서 그런지 terrain brush 와 다르게
*  따로 함수로 빼서 구현되어있다.
*/
void CMapOutdoorAccessor::PreviewWaterBrush(uint32_t dwBrushShape,
											WORD wTerrainNumX,
											WORD wTerrainNumY,
											long lCellX,
											long lCellY,
											BYTE byBrushSize,
											WORD wWaterHeight,
											bool bErase)
{
	float fx, fy, fz, fLeft, fTop, fRight, fBottom;

	fx = (float)(lCellX * CTerrainImpl::CELLSCALE + wTerrainNumX * CTerrainImpl::TERRAIN_XSIZE);
	fy = (float)(lCellY * CTerrainImpl::CELLSCALE + wTerrainNumY * CTerrainImpl::TERRAIN_YSIZE);
	fz = wWaterHeight * m_fHeightScale;

	fLeft	= (float)((lCellX - byBrushSize) * CTerrainImpl::CELLSCALE + wTerrainNumX * CTerrainImpl::TERRAIN_XSIZE);
	fTop	= -(float)((lCellY - byBrushSize) * CTerrainImpl::CELLSCALE + wTerrainNumY * CTerrainImpl::TERRAIN_YSIZE);
	fRight	= (float)((lCellX + byBrushSize) * CTerrainImpl::CELLSCALE + wTerrainNumX * CTerrainImpl::TERRAIN_XSIZE);
	fBottom	= -(float)((lCellY + byBrushSize) * CTerrainImpl::CELLSCALE + wTerrainNumY * CTerrainImpl::TERRAIN_YSIZE);

	SetDiffuseOperation();
	SetDiffuseColor(0.0f, 0.0f, 1.0f);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());

	/*SetDiffuseColor(1.0f, 1.0f, 1.0f);
	RenderLine3d( fx, -fy, fz - 30.0f, fx, -fy, fz + 30.0f);
	RenderLine3d( fx - 30.0f, -fy, fz, fx + 30.0f, -fy, fz);
	RenderLine3d( fx, -fy - 30.0f, fz, fx, -fy + 30.0f, fz);*/

	RenderLine3d(fLeft, fTop, fz, fLeft, fBottom, fz);
	RenderLine3d(fRight, fTop, fz, fRight, fBottom, fz);
	RenderLine3d(fLeft, fTop, fz, fRight, fTop, fz);
	RenderLine3d(fLeft, fBottom, fz, fRight, fBottom, fz);

	int count, iStep;
	float theta, delta;
	float x, y, z, fRadius;
	std::vector<Vector3> pts;

	fRadius = float(byBrushSize * CTerrainImpl::CELLSCALE);
	iStep = 50;

	pts.clear();
	pts.resize(iStep);

	theta = 0.0;
	delta = 2 * DirectX::XM_PI / float(iStep);

	for (count = 0; count < iStep; ++count)
	{
		x = fx + fRadius * cosf(theta);
		y = fy + fRadius * sinf(theta);
		z = fz;

		pts[count] = Vector3(x, -y, z);

		theta += delta;
	}

	for (count = 0; count < iStep - 1; ++count)
	{
		RenderLine3d(pts[count].x, pts[count].y, pts[count].z, pts[count + 1].x, pts[count + 1].y, pts[count + 1].z);
	}

	RenderLine3d(pts[iStep - 1].x, pts[iStep - 1].y, pts[iStep - 1].z, pts[0].x, pts[0].y, pts[0].z);
}

void CMapOutdoorAccessor::DrawWaterBrush(uint32_t dwBrushShape,
										 WORD wTerrainNumX,
										 WORD wTerrainNumY,
										 long lCellX,
										 long lCellY,
										 BYTE byBrushSize,
										 WORD wWaterHeight,
										 bool bErase)
{
	CTerrainAccessor * pTerrainAccessor = NULL;
	BYTE ucTerrainNum = (wTerrainNumY - m_CurCoordinate.m_sTerrainCoordY + 1) * 3 + (wTerrainNumX - m_CurCoordinate.m_sTerrainCoordX + 1);

	// 여러개로 나눈다...

	if (GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor))
		pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX, lCellY, byBrushSize, wWaterHeight, bErase);

	if (lCellY < byBrushSize)
	{
		if (wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX, lCellY + CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		if (lCellX < byBrushSize)
		{
			if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX + CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY >= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum - 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX - CTerrainImpl::XSIZE, lCellY + CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		}
	}
	else if (lCellY > (CTerrainImpl::YSIZE - 1) - byBrushSize)
	{
		if (wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 3, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX, lCellY - CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		if (lCellX < byBrushSize)
		{
			if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 2, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX + CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		}
		else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
		{
			if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && wTerrainNumY <= m_CurCoordinate.m_sTerrainCoordY && GetTerrainPointer(ucTerrainNum + 4, (CTerrain **) &pTerrainAccessor))
				pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX - CTerrainImpl::XSIZE, lCellY - CTerrainImpl::YSIZE, byBrushSize, wWaterHeight, bErase);
		}
	}

	if (lCellX < byBrushSize)
	{
		if (wTerrainNumX >= m_CurCoordinate.m_sTerrainCoordX && GetTerrainPointer(ucTerrainNum - 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX + CTerrainImpl::XSIZE, lCellY, byBrushSize, wWaterHeight, bErase);
	}
	else if (lCellX > (CTerrainImpl::XSIZE - 1) - byBrushSize)
	{
		if (wTerrainNumX <= m_CurCoordinate.m_sTerrainCoordX && GetTerrainPointer(ucTerrainNum + 1, (CTerrain **) &pTerrainAccessor))
			pTerrainAccessor->DrawWaterBrush(dwBrushShape, lCellX - CTerrainImpl::XSIZE, lCellY, byBrushSize, wWaterHeight, bErase);
	}

	SetTerrainModified();
}

/* 현재 cell 의 water height 값을 알수 있도록 하기 위해 추가
*/
void CMapOutdoorAccessor::CurCellEditWater(WORD wTerrainNumX,
										 WORD wTerrainNumY,
										 long lCellX,
										 long lCellY)
{
	CTerrainAccessor * pTerrainAccessor = NULL;
	BYTE ucTerrainNum = (wTerrainNumY - m_CurCoordinate.m_sTerrainCoordY + 1) * 3 + (wTerrainNumX - m_CurCoordinate.m_sTerrainCoordX + 1);
	long lWaterHeight = 0;
	bool bSuccessed = false;

	if (GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor))
		bSuccessed = pTerrainAccessor->CurCellEditWater(lCellX, lCellY, lWaterHeight);

	//현재 cell 의 water height 값 출력
	//미터 단위로 표기하기 위한 연산이 추가되어있다.
	char szTextLabel[16];
	memset(szTextLabel, 0, sizeof(szTextLabel));
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
	if(bSuccessed)
	{
		lWaterHeight /= 2;
		float fHeight = (float) (lWaterHeight - 16384) * 0.01f;
		sprintf(szTextLabel, "%.2f", fHeight);
	}
	else
		sprintf(szTextLabel, "없음");

	pFrame->m_wndMapCtrlBar.m_pageTerrain.m_CurCellWaterHeight.SetWindowText(szTextLabel);
}

//////////////////////////////////////////////////////////////////////////

int CMapOutdoorAccessor::GetMapID()
{
	return m_iMapID;
}

void CMapOutdoorAccessor::SetMapID(int iID)
{
	m_iMapID = iID;
}

void CMapOutdoorAccessor::SetTerrainModified()
{
	ResetTerrainPatchVertexBuffer();
	m_lOldReadX = m_lOldReadY = -1;
}

bool CMapOutdoorAccessor::CreateNewTerrainSector(WORD wCoordX, WORD wCoordY)
{
	auto pTerrainAccessor = std::make_unique<CTerrainAccessor>();

	pTerrainAccessor->SetMapOutDoor(this);
	pTerrainAccessor->SetCoordinate(wCoordX, wCoordY);

	char szTerrainFolder[256];
	uint32_t ulID = (uint32_t)(wCoordX) * 1000L + (uint32_t)(wCoordY);
	sprintf(szTerrainFolder, "%s\\%06u", m_strName.c_str(), ulID);

	storm::String path = szTerrainFolder;

	struct _stat sb;
	if (0 != _stat(path.c_str(), &sb))
	{
		// Make The Directory
		if (!CreateDirectory(path.c_str(), NULL))
		{
			spdlog::error("Failed to create directory {0}", szTerrainFolder);
			return false;
		}
	}

	if (0 != _stat(path.c_str(), &sb))
	{
		spdlog::error("{0} doesn't exist", szTerrainFolder);
		return false;
	}

	if (!(sb.st_mode & _S_IFDIR))
	{
		spdlog::error("{0} is not a directory", szTerrainFolder);
		return false;
	}

	// Save Property
	if (!pTerrainAccessor->SaveProperty(m_strName))
	{
		spdlog::error(
		          "Failed to save property for {0} {1}",
		          wCoordX,
		          wCoordY);
		return false;
	}

	// Save HeightFieldMap
	if (!pTerrainAccessor->NewHeightMap(m_strName))
	{
		spdlog::error(
		          "Failed to save height for {0} {1}",
		          wCoordX,
		          wCoordY);
		return false;
	}

	// Save TileMap
 	if (!pTerrainAccessor->NewTileMap(m_strName))
	{
		spdlog::error(
		          "Failed to save tile for {0} {1}",
		          wCoordX,
		          wCoordY);
		return false;
	}

	// Save AttrMap
 	if (!pTerrainAccessor->NewAttrMap(m_strName))
	{
		spdlog::error(
		          "Failed to save attr for {0} {1}",
		          wCoordX,
		          wCoordY);
		return false;
	}
	return true;
}

BOOL CMapOutdoorAccessor::GetAreaAccessor(uint32_t dwIndex, CAreaAccessor ** ppAreaAccessor)
{
	if (dwIndex >= AROUND_AREA_NUM)
	{
		*ppAreaAccessor = NULL;
		return FALSE;
	}

	CArea * pArea;

	if (!GetAreaPointer(dwIndex, &pArea))
	{
		*ppAreaAccessor = NULL;
		return FALSE;
	}

	*ppAreaAccessor = (CAreaAccessor *) pArea;
	return TRUE;
}

bool CMapOutdoorAccessor::GetTerrain(int32_t x, int32_t y, CTerrainAccessor*& terrainAccessor)
{
	for (auto& terrain : m_TerrainVector) {
		WORD usCoordX, usCoordY;
		terrain->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordY == y && usCoordX == x) {
			terrainAccessor = static_cast<CTerrainAccessor*>(terrain.get());
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Utility
bool CMapOutdoorAccessor::RAW_InitBaseTexture(const std::vector<BYTE> & c_rVectorBaseTexture)
{
	auto pTerrainAccessor = std::make_unique<CTerrainAccessor>();
	pTerrainAccessor->SetMapOutDoor(this);

	for (WORD usY = 0; usY < m_sTerrainCountY; ++usY)
	{
		for (WORD usX = 0; usX < m_sTerrainCountX; ++usX)
		{
			pTerrainAccessor->SetCoordinate(usX, usY);

			char szTerrainFolder[256], szRAWTileMapName[256];
			uint32_t ulID = (uint32_t)(usX) * 1000L + (uint32_t)(usY);
			sprintf(szTerrainFolder, "%s\\%06u", m_strName.c_str(), ulID);

			storm::String path = szTerrainFolder;

			struct _stat sb;
			if (0 != _stat(path.c_str(), &sb))
			{
				spdlog::error("{0} doesn't exist", path);
				return false;
			}

			if (!(sb.st_mode & _S_IFDIR))
			{
				spdlog::error("{0} isn't a directory", path);
				return false;
			}

			sprintf(szRAWTileMapName, "%s\\tile.raw", szTerrainFolder);

			if (!pTerrainAccessor->RAW_LoadAndSaveTileMap(szRAWTileMapName, m_strName, c_rVectorBaseTexture))
			{
				spdlog::error(
				          "Failed to save property for {0} {1}",
				          usX,
				          usY);
				return false;
			}
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////

void CMapOutdoorAccessor::RenderAccessorTerrain(BYTE byRenderMode, BYTE byAttrFlag)
{
	if (!m_pTerrainPatchProxyList)
		return;

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE eType;
	SelectIndexBuffer(0, &wPrimitiveCount, &eType);

	switch (byRenderMode)
	{
		case RENDER_SHADOW:
			{
				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

				RecurseRenderAccessorTerrain(m_pRootNode, byRenderMode, byAttrFlag, false);
			}
			break;

		case RENDER_ATTR:
			{
				Matrix matTexTransform, matTexTransformTemp;

				DirectX::SimpleMath::MatrixScaling(&matTexTransform, m_fTerrainTexCoordBase * 32.0f, -m_fTerrainTexCoordBase * 32.0f, 0.0f);
				DirectX::SimpleMath::MatrixMultiply(&matTexTransform, &m_matViewInverse, &matTexTransform);
				STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &matTexTransform);
				STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &matTexTransform);

				STATEMANAGER.SetTexture(0, m_attrImageInstance.GetTexturePointer()->GetD3DTexture());

				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
				STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

				STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
				STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
				STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
				STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER,	D3DTEXF_POINT);
				STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER,	D3DTEXF_POINT);
				STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER,	D3DTEXF_POINT);
				STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU,	D3DTADDRESS_CLAMP);
				STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV,	D3DTADDRESS_CLAMP);

				RecurseRenderAccessorTerrain(m_pRootNode, byRenderMode, byAttrFlag);

				STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
				STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
				STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
				STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
				STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
				STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
				STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
				STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
				STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);

				STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
				STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);
			}
			break;
	}
}

void CMapOutdoorAccessor::RecurseRenderAccessorTerrain(CTerrainQuadtreeNode *Node, BYTE byRenderMode, BYTE byAttrFlag, bool bCullEnable)
{
	if (bCullEnable)
	{
		if (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius)==VIEW_NONE)
			return;
	}
	{
		if (Node->Size == 1)
		{
			switch(byRenderMode)
			{
				case RENDER_SHADOW:
					if (4 == m_pTerrainPatchProxyList[Node->PatchNum].GetTerrainNum())
						DrawMeshOnly(Node->PatchNum);
					break;

				case RENDER_ATTR:
					DrawPatchAttr(Node->PatchNum, byAttrFlag);
					if (m_bDrawWireFrame)
	 					DrawWireFrame(Node->PatchNum, m_wNumIndices[0] - 2, D3DPT_TRIANGLESTRIP);
					break;
			}
		}
		else
		{
			if (Node->NW_Node != NULL)
				RecurseRenderAccessorTerrain(Node->NW_Node, byRenderMode, byAttrFlag, bCullEnable);
			if (Node->NE_Node != NULL)
				RecurseRenderAccessorTerrain(Node->NE_Node, byRenderMode, byAttrFlag, bCullEnable);
			if (Node->SW_Node != NULL)
				RecurseRenderAccessorTerrain(Node->SW_Node, byRenderMode, byAttrFlag, bCullEnable);
			if (Node->SE_Node != NULL)
				RecurseRenderAccessorTerrain(Node->SE_Node, byRenderMode, byAttrFlag, bCullEnable);
		}
 	}
}

void CMapOutdoorAccessor::DrawMeshOnly(long patchnum)
{
	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;
	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_NORMAL);
	STATEMANAGER.SetStreamSource(0, pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr()->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);
	Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices[0] - 2);
}

void CMapOutdoorAccessor::RenderShadow()
{
	RenderAccessorTerrain(RENDER_SHADOW, 0);
}

void CMapOutdoorAccessor::DrawPatchAttr(long patchnum, BYTE byAttrFlag)
{
	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	// Deal with this material buffer
	CTerrainAccessor * pTerrainAccessor;
	if (!GetTerrainPointer(ucTerrainNum, (CTerrain **) &pTerrainAccessor))
		return;

	WORD wCoordX, wCoordY;
	pTerrainAccessor->GetCoordinate(&wCoordX, &wCoordY);

	m_matWorldForCommonUse._41 = -(float) (wCoordX * CTerrainImpl::XSIZE * CTerrainImpl::CELLSCALE);
	m_matWorldForCommonUse._42 = (float) (wCoordY * CTerrainImpl::YSIZE * CTerrainImpl::CELLSCALE);

	Matrix matTexTransform, matTexTransformTemp;
	DirectX::SimpleMath::MatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	DirectX::SimpleMath::MatrixMultiply(&matTexTransform, &matTexTransform, &m_matStaticShadow);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matTexTransform);

	TTerrainSplatPatch & rAttrSplatPatch = pTerrainAccessor->RAW_GetAttrSplatPatch();
	STATEMANAGER.SetTexture(1, rAttrSplatPatch.Splats[0].pd3dTexture);

	Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_NORMAL);
	STATEMANAGER.SetStreamSource(0, pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr()->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);
	Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices[0] - 2);
}

bool CMapOutdoorAccessor::ResetToDefaultAttr()
{
	for (auto& terrain : m_TerrainVector) {
		auto terrainAccessor = static_cast<CTerrainAccessor*>(terrain.get());

		terrainAccessor->ResetAttr();

		if (!terrainAccessor->SaveAttrMap(m_strName)) {
			WORD usCoordX, usCoordY;
			terrain->GetCoordinate(&usCoordX, &usCoordY);

			spdlog::error(
			          "ResetToDefaultAttr: Failed to save {0} {1}:{2}",
			          m_strName, usCoordX, usCoordY);
		}
	}

	return true;
}

struct ClientAttrMap
{
	static const uint32_t kSizeX = CTerrainImpl::ATTRMAP_XSIZE;
	static const uint32_t kSizeY = CTerrainImpl::ATTRMAP_YSIZE;
	static const uint32_t kSize = kSizeX * kSizeY;
	static const uint32_t kScale = 100;

	ClientAttrMap(uint32_t w, uint32_t h)
		: width(w / (kSizeX * kScale))
		, height(h / (kSizeY * kScale))
		, attr(kSize * width * height)
	{
		// ctor
	}

	uint8_t* GetSector(uint32_t x, uint32_t y)
	{
		STORM_ASSERT(x < width && y < height, "Sanity");
		return &attr[(y * width + x) * kSize];
	}

	uint32_t width, height;
	std::vector<uint8_t> attr;
};

struct ServerAttrFileHeader
{
	uint32_t width;
	uint32_t height;
};

struct ServerAttrMap
{
	static const uint32_t kSizeX = 128;
	static const uint32_t kSizeY = 128;
	static const uint32_t kSize = kSizeX * kSizeY;
	static const uint32_t kScale = 50;

	ServerAttrMap(uint32_t w, uint32_t h)
		: width(w / (kSizeX * kScale))
		, height(h / (kSizeY * kScale))
		, attr(kSize * width * height)
		, compressor(storm::GetDefaultAllocator())
	{
		// ctor
	}

	uint32_t* GetSector(uint32_t x, uint32_t y)
	{
		STORM_ASSERT(x < width && y < height, "Sanity");
		return &attr[(y * width + x) * kSize];
	}

	bool Save(const std::string& filename)
	{
		storm::File file;

		bsys::error_code ec;
		file.Open(filename, ec,
		          storm::AccessMode::kWrite,
		          storm::CreationDisposition::kCreateAlways,
		          storm::ShareMode::kNone,
		          storm::UsageHint::kSequential);

		if (ec) {
			spdlog::error("Failed to open {0} for writing with {1}",
			          filename, ec);
			return false;
		}

		ServerAttrFileHeader header;
		header.width = width;
		header.height = height;
		storm::WriteExact(file, &header, sizeof(header), ec);

		const auto maxSize = GetMaxLzoCompressedSize(kSize * sizeof(uint32_t));
		std::vector<uint8_t> temp(maxSize);

		for (uint32_t y = 0; y != height; ++y) {
			for (uint32_t x = 0; x != width; ++x) {
				const auto sector = GetSector(x, y);

				std::size_t compressedSize = maxSize;

				if (!compressor.Compress(sector, kSize * sizeof(uint32_t),
				                         temp.data(), compressedSize, 1))
					return false;

				uint32_t size = static_cast<uint32_t>(compressedSize);
				storm::WriteExact(file, &size, sizeof(size), ec);
				storm::WriteExact(file, temp.data(), size, ec);
			}
		}

		return true;
	}

	uint32_t width, height;
	std::vector<uint32_t> attr;
	LzoCompressor compressor;
};

void ConvertAttribute(ClientAttrMap& src, ServerAttrMap& dst)
{
	const uint32_t w1 = dst.width * ServerAttrMap::kSizeX;
	const uint32_t h1 = dst.height * ServerAttrMap::kSizeY;

	const uint32_t w2 = src.width * ClientAttrMap::kSizeX;
	const uint32_t h2 = src.height * ClientAttrMap::kSizeY;

	static const uint32_t kValidMask = 0x00000007;

	SPDLOG_DEBUG("Warning: {0} {1} -> {2} {3}",
	           w2, h2, w1, h1);

	/*for (uint32_t y1 = 0; y1 != h1; ++y1) {
		for (uint32_t x1 = 0; x1 != w1; ++x1) {
			const uint32_t y2 = y1 * h2 / h1;
			const uint32_t x2 = x1 * w2 / w1;

			//dst.attr[y1 * w1 + x1] = src.attr[y2 * w2 + x2] & kValidMask;
		}
	}*/
	for (uint32_t y1 = 0; y1 != h1; ++y1) {
		for (uint32_t x1 = 0; x1 != w1; ++x1) {
			const uint32_t y2 = y1 * h2 / h1;
			const uint32_t x2 = x1 * w2 / w1;

			/*const auto srcS = src.GetSector(x2 / ClientAttrMap::kSizeX,
			                                y2 / ClientAttrMap::kSizeY);

			const auto dstS = dst.GetSector(x1 / ServerAttrMap::kSizeX,
			                                y1 / ServerAttrMap::kSizeY);*/

			auto dstS = dst.attr.data();
			dstS += ((y1 / ServerAttrMap::kSizeY) * dst.width + (x1 / ServerAttrMap::kSizeX)) * ServerAttrMap::kSize;
			dstS += (y1 % ServerAttrMap::kSizeY) * ServerAttrMap::kSizeX + (x1 % ServerAttrMap::kSizeX);

			auto srcS = src.attr.data();
			srcS += ((y2 / ClientAttrMap::kSizeY) * src.width + (x2 / ClientAttrMap::kSizeX)) * ClientAttrMap::kSize;
			srcS += (y2 % ClientAttrMap::kSizeY) * ClientAttrMap::kSizeX + (x2 % ClientAttrMap::kSizeX);

			*dstS = *srcS;
		}
	}
}

void CMapOutdoorAccessor::SaveServerAttr(const std::string& filename)
{
	// in pixels
	const auto w = m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;
	const auto h = m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;

	// Copy all attributes to temp. memory
	ClientAttrMap clientAttr(w, h);

	for (auto& terrain : m_TerrainVector) {
		auto terrainAccessor = static_cast<CTerrainAccessor*>(terrain.get());

		WORD x, y;
		terrain->GetCoordinate(&x, &y);

		terrainAccessor->CopyAttr(clientAttr.GetSector(x, y));
	}

	// Add AreaData object collisions
	for (auto& area : m_AreaVector) {
		auto areaAccessor = static_cast<CAreaAccessor*>(area.get());

		WORD x, y;
		area->GetCoordinate(&x, &y);
		areaAccessor->CopyObjectAttr(clientAttr.GetSector(x, y));
	}

	ServerAttrMap serverAttr(w, h);
	ConvertAttribute(clientAttr, serverAttr);

	serverAttr.Save(filename);
}

bool CMapOutdoorAccessor::Destroy()
{
	__DestroyProgressDialog();
	CMapOutdoor::Destroy();
	m_environmentPaths.clear();
	m_selectedEnvironment = 0;
	return true;
}

bool CMapOutdoorAccessor::Load(float x, float y, float z)
{
	Destroy();

	std::string strFileName = GetName() + "\\Setting.txt";
	if (!LoadSetting(strFileName.c_str()))
	{
		spdlog::error("Failed to load {0}", strFileName);
		return false;
	}

	m_selectedEnvironment = 0;

	m_lViewRadius *= 4;

	CreateTerrainPatchProxyList();
	BuildQuadTree();
	LoadWaterTexture(); // FIXME: 임시로 물 텍스춰를 강제로 로드하게 하였다.
	//SetShadowTextureSize(2048);

	LoadGuildAreaList("GuildAreaList.txt");

	m_lOldReadX = -1;

	// TODO: SetRenderingDevice에서 Environment로 부터 라이트 속성을 넘겨줘야 스태틱 라이트가 제대로 작동한다.
	CSpeedTreeForest::Instance().Initialize();

	for (uint16_t uY = 0; uY != m_sTerrainCountY; ++uY) {
		for (uint16_t uX = 0; uX != m_sTerrainCountX; ++uX) {
			LoadTerrain(uX, uY);
			LoadArea(uX, uY);
		}
	}

	Update(x, y, z);
	return true;
}

bool CMapOutdoorAccessor::NewMap(const std::string& name,
                                 uint16_t w, uint16_t h,
                                 const std::string& textureSet)
{
	SetType(CMapBase::MAPTYPE_OUTDOOR);
	SetTerrainCount(w, h);
	SetName(name);

	if (!m_TextureSet.Save(textureSet.c_str())) {
		spdlog::error("Failed to save texture set {0}", textureSet);
		return false;
	}

	return true;
}

bool CMapOutdoorAccessor::LoadGuildAreaList(const char * c_szFileName)
{
	FILE * file = fopen(c_szFileName, "r");
	if (!file)
		return false;

	char szLine[1024+1];
	while (fgets(szLine, 1024, file))
	{
		int iID;
		int iMapIndex;
		uint32_t dwsx;
		uint32_t dwsy;
		uint32_t dwWidth;
		uint32_t dwHeight;
		uint32_t dwGuildID;
		uint32_t dwGuildLevelLimit;
		uint32_t dwPrice;
		sscanf(szLine, "%d %d %d %d %d %d %d %d %d", &iID, &iMapIndex, &dwsx, &dwsy, &dwWidth, &dwHeight, &dwGuildID, &dwGuildLevelLimit, &dwPrice);

		if (iMapIndex == GetMapID())
		{
			RegisterGuildArea(dwsx, dwsy, dwsx+dwWidth, dwsy+dwHeight);
		}
	}

	fclose(file);

	VisibleMarkedArea();
	return true;
}

void CMapOutdoorAccessor::RenderObjectCollision()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pAreaAccessor;
		if (!GetAreaPointer(i, (CArea **) &pAreaAccessor) )
			continue;
		pAreaAccessor->RenderCollision();
	}
}

//////////////////////////////////////////////////////////////////////////
// Shadow Map
void CMapOutdoorAccessor::RenderToShadowMap()
{
	for (BYTE i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pAreaAccessor;

		if (GetAreaPointer(i, (CArea **) &pAreaAccessor))
		{
			pAreaAccessor->RenderToShadowMap();
		}
	}
}

bool CMapOutdoorAccessor::LoadArea(WORD wAreaCoordX, WORD wAreaCoordY)
{
	if (isAreaLoaded(wAreaCoordX, wAreaCoordY))
		return true;

	uint32_t dwStartTime = timeGetTime();

	unsigned long ulID = (unsigned long) (wAreaCoordX) * 1000L + (unsigned long) (wAreaCoordY);
	char szAreaDataName[64];
	_snprintf(szAreaDataName,	64, "%s\\%06u\\", m_strName.c_str(), ulID);

	CAreaAccessor* pAreaAccessor = new CAreaAccessor;

	pAreaAccessor->SetMapOutDoor(this);
	pAreaAccessor->SetCoordinate(wAreaCoordX, wAreaCoordY);
	pAreaAccessor->Load(szAreaDataName);

	m_AreaVector.push_back(std::unique_ptr<CArea>(pAreaAccessor));

	SPDLOG_INFO("CMapOutdoor::LoadArea {0}", timeGetTime() - dwStartTime);

	return true;
}

bool CMapOutdoorAccessor::LoadTerrain(WORD wX, WORD wY)
{
	if (isTerrainLoaded(wX, wY))
		return true;

// 	InitializeFog();	// FIXME: 임시 위치, 알파 포그를 위한 파일을 로드 한다.

	unsigned long ulID = (unsigned long) (wX) * 1000L + (unsigned long) (wY);
	char filename[256];
	sprintf(filename, "%s\\%06u\\AreaProperty.txt", m_strName.c_str(), ulID);

	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(filename, stTokenVectorMap))
		return false;

	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
		return false;

	if (stTokenVectorMap.end() == stTokenVectorMap.find("areaname"))
		return false;

	const auto& c_rstrType = stTokenVectorMap["scripttype"][0];
	const auto& c_rstrAreaName = stTokenVectorMap["areaname"][0];

	if (c_rstrType != "AreaProperty")
		return false;

	CTerrainAccessor* pTerrainAccessor = new CTerrainAccessor();

	char szTileMapName[64];
	char szRAWTileMapName[64];
	char szWaterMapName[64];
	char szAttrMapName[64];
	char szShadowTexName[64];
	char szShadowMapName[64];
	char szRawHeightFilename[64];

	_snprintf(szRawHeightFilename, 64, "%s\\%06u\\height.raw", m_strName.c_str(), ulID);
	_snprintf(szRAWTileMapName,	64, "%s\\%06u\\tile.raw", m_strName.c_str(), ulID);
	_snprintf(szTileMapName,	64, "%s\\%06u\\color.cmp", m_strName.c_str(), ulID);
	_snprintf(szAttrMapName,	64, "%s\\%06u\\attr.atr", m_strName.c_str(), ulID);
	_snprintf(szWaterMapName,	64, "%s\\%06u\\water.wtr", m_strName.c_str(), ulID);
	_snprintf(szShadowTexName,	64, "%s\\%06u\\shadowmap.dds", m_strName.c_str(), ulID);
	_snprintf(szShadowMapName,	64, "%s\\%06u\\shadowmap.raw", m_strName.c_str(), ulID);

	pTerrainAccessor->SetMapOutDoor(this);
	pTerrainAccessor->SetCoordinate(wX, wY);

	pTerrainAccessor->CopySettingFromGlobalSetting();

	pTerrainAccessor->LoadWaterMap(szWaterMapName);
	pTerrainAccessor->LoadHeightMap(szRawHeightFilename);
	pTerrainAccessor->LoadAttrMap(szAttrMapName);
	pTerrainAccessor->RAW_LoadTileMap(szRAWTileMapName);
	pTerrainAccessor->LoadShadowTexture(szShadowTexName);
	pTerrainAccessor->LoadShadowMap(szShadowMapName);
	pTerrainAccessor->SetName(c_rstrAreaName.c_str());
	pTerrainAccessor->CalculateTerrainPatch();

	// Attribute Map
	pTerrainAccessor->RAW_AllocateAttrSplats();

	pTerrainAccessor->SetReady();

	m_TerrainVector.push_back(std::unique_ptr<CTerrain>(pTerrainAccessor));
	return true;
}

/*virtual*/ void CMapOutdoorAccessor::UpdateAreaList(long lCenterX, long lCenterY)
{
	// do nothing
	// we never delete unseen terrains/areas
}

/*virtual*/ void CMapOutdoorAccessor::UpdateArea(const Vector3& v3Player)
{
	m_PCBlockerVector.clear();
	m_ShadowReceiverVector.clear();
	__UpdateAroundAreaList();
}

void CMapOutdoorAccessor::ResetTerrainPatchVertexBuffer()
{
	for (int i = 0; i < m_wPatchCount * m_wPatchCount; ++i)
		m_pTerrainPatchProxyList[i].SetTerrainPatch(NULL);
	for (BYTE byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		CTerrain * pTerrain;
		if (GetTerrainPointer(byTerrainNum, &pTerrain))
			pTerrain->CalculateTerrainPatch();
	}
}

void CMapOutdoorAccessor::ResetTextures()
{
	m_TextureSet.Reload(m_fTerrainTexCoordBase);
}

void CMapOutdoorAccessor::RenderMiniMap()
{
	SetInverseViewAndDynamicShaodwMatrices();
	SetDiffuseOperation();
	RenderArea(false);

	if (IsVisiblePart(PART_TREE))
		CSpeedTreeForest::Instance().Render(Forest_RenderAll | Forest_RenderToMiniMap);

	RenderTerrain();
	RenderWater();
}

bool CMapOutdoorAccessor::GetPickingCoordinate(Vector3 * v3IntersectPt, int * piCellX, int * piCellY, BYTE * pbySubCellX, BYTE * pbySubCellY, WORD * pwTerrainNumX, WORD * pwTerrainNumY)
{
	return GetPickingCoordinateWithRay(ms_Ray, v3IntersectPt, piCellX, piCellY, pbySubCellX, pbySubCellY, pwTerrainNumX, pwTerrainNumY);
}

bool CMapOutdoorAccessor::GetPickingCoordinateWithRay(const CRay & rRay, Vector3 * v3IntersectPt, int * piCellX, int * piCellY, BYTE * pbySubCellX, BYTE * pbySubCellY, WORD * pwTerrainNumX, WORD * pwTerrainNumY)
{
	CTerrain * pTerrain;

	Vector3 v3Start, v3End, v3CurPos;

	rRay.GetStartPoint(&v3Start);
	rRay.GetEndPoint(&v3End);

	float fAdd = 1.0f / (float)(CTerrainImpl::TERRAIN_XSIZE);

	float ft = 0.0f;
	while (ft < 1.0f)
	{
		D3DXVec3Lerp(&v3CurPos, &v3Start, &v3End, ft);
		BYTE byTerrainNum;
		float fMultiplier = 1.0f;
		if (GetTerrainNum(v3CurPos.x, v3CurPos.y, &byTerrainNum))
		{
			if (GetTerrainPointer(byTerrainNum, &pTerrain))
			{
				int ix, iy;
				PR_FLOAT_TO_INT(v3CurPos.x, ix);
				PR_FLOAT_TO_INT(fabs(v3CurPos.y), iy);
				float fMapHeight = pTerrain->GetHeight(ix, iy);
				if (fMapHeight >= v3CurPos.z) {
					__ACCESS_ConvertToMapCoords(v3CurPos.x, v3CurPos.y, piCellX, piCellY, pbySubCellX, pbySubCellY, pwTerrainNumX, pwTerrainNumY);
					*v3IntersectPt = v3CurPos;
					return true;
				} else {
					fMultiplier = fMAX(1.0f, 0.01f * (v3CurPos.z - fMapHeight));
				}
			}
		}

		ft += fAdd * fMultiplier;
	}

	return false;
}

void CMapOutdoorAccessor::__ACCESS_ConvertToMapCoords(float fx, float fy, int *iCellX, int *iCellY, BYTE * pucSubCellX, BYTE * pucSubCellY, WORD * pwTerrainNumX, WORD * pwTerrainNumY)
{
	if ( fy < 0 )
		fy = -fy;

	int ix, iy;
	PR_FLOAT_TO_INT(fx, ix);
	PR_FLOAT_TO_INT(fy, iy);

	*pwTerrainNumX = ix / (CTerrainImpl::TERRAIN_XSIZE);
	*pwTerrainNumY = iy / (CTerrainImpl::TERRAIN_YSIZE);

	float maxx = (float) CTerrainImpl::TERRAIN_XSIZE;
	float maxy = (float) CTerrainImpl::TERRAIN_YSIZE;

	while (fx < 0)
		fx += maxx;

	while (fy < 0)
		fy += maxy;

	while (fx >= maxx)
		fx -= maxx;

	while (fy >= maxy)
		fy -= maxy;

	float fooscale = 1.0f / (float)(CTerrainImpl::CELLSCALE);

	float fCellX, fCellY;

	fCellX = fx * fooscale;
	fCellY = fy * fooscale;

	PR_FLOAT_TO_INT(fCellX, *iCellX);
	PR_FLOAT_TO_INT(fCellY, *iCellY);

	float fRatioooscale = ((float)CTerrainImpl::HEIGHT_TILE_XRATIO) * fooscale;

	float fSubcellX, fSubcellY;
	fSubcellX = fx * fRatioooscale;
	fSubcellY = fy * fRatioooscale;

	PR_FLOAT_TO_INT(fSubcellX, *pucSubCellX);
	PR_FLOAT_TO_INT(fSubcellY, *pucSubCellY);
	*pucSubCellX = (*pucSubCellX) % CTerrainImpl::HEIGHT_TILE_XRATIO;
	*pucSubCellY = (*pucSubCellY) % CTerrainImpl::HEIGHT_TILE_YRATIO;
}

void CMapOutdoorAccessor::ReloadBuildingTexture()
{
	for (BYTE i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pAreaAccessor;
		if (GetAreaPointer(i, (CArea **) &pAreaAccessor))
			pAreaAccessor->ReloadBuildingTexture();
	}
}

/* 추가
*  DungeonBlock 도 Texture를 Reload 할수 있게 추가.
*/
void CMapOutdoorAccessor::ReloadDungeonBlockTexture()
{
	for (BYTE i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pAreaAccessor;
		if (GetAreaPointer(i, (CArea **) &pAreaAccessor))
			pAreaAccessor->ReloadDungeonBlockTexture();
	}
}

void CMapOutdoorAccessor::ExportHeightmaps()
{
	for (uint32_t y = 0; y != m_sTerrainCountY; ++y) {
		for (uint32_t x = 0; x != m_sTerrainCountX; ++x) {
			CTerrainAccessor* terrain;
			if (!GetTerrain(x, y, terrain))
				continue;

			ExportSingleHeightmap(terrain);
		}
	}
}

void CMapOutdoorAccessor::ExportSingleHeightmap(CTerrainAccessor* terrain)
{
	static const uint32_t kSizeX = TERRAIN_SIZE;
	static const uint32_t kSizeY = TERRAIN_SIZE;

	std::vector<uint16_t> heightmap;
	heightmap.reserve(kSizeX * kSizeY);

	// 1) Copy source heightmap
	auto srcHeightMap = terrain->GetHeightMap();

	// First row is always ignored.
	srcHeightMap += CTerrainImpl::HEIGHTMAP_RAW_XSIZE;

	for (uint32_t y = 0; y != kSizeY; ++y) {
		// First column is always ignored
		heightmap.insert(heightmap.end(),
		                 srcHeightMap + 1,
		                 srcHeightMap + 1 + kSizeX);

		srcHeightMap += CTerrainImpl::HEIGHTMAP_RAW_XSIZE;
	}

	// 2) Fix edge values
	WORD terrainX, terrainY;
	terrain->GetCoordinate(&terrainX, &terrainY);

	CTerrainAccessor* neighbor;

	// Copy heightmap values from top sector if possible
	if (terrainY != 0) {
		if (!GetTerrain(terrainX, terrainY - 1, neighbor)) {
			spdlog::error("Failed to find sector {0}:{1}",
			          terrainX, terrainY - 1);
			return;
		}

		for (uint32_t x = 0; x != kSizeX; ++x)
			heightmap[x] =
				neighbor->GetHeightMapValue(x, 128);
	}

	for (uint32_t y = 1; y != kSizeY; ++y) {
		// Copy heightmap value from left sector if possible
		if (terrainX != 0) {
			if (!GetTerrain(terrainX - 1, terrainY, neighbor))
				continue;

			heightmap[y * kSizeX] =
				neighbor->GetHeightMapValue(128, y);
		}

		// Copy heightmap value from right sector if possible
		if (terrainX + 1 != m_sTerrainCountX) {
			if (!GetTerrain(terrainX + 1, terrainY, neighbor))
				continue;

			heightmap[y * kSizeX + kSizeX - 1] =
				neighbor->GetHeightMapValue(0, y);
		}
	}

	// Copy heightmap values from bottom sector if possible
	if (terrainY + 1 != m_sTerrainCountY) {
		if (!GetTerrain(terrainX, terrainY + 1, neighbor)) {
			spdlog::error("Failed to find sector {0}:{1}",
			          terrainX, terrainY - 1);
			return;
		}

		for (uint32_t x = 0; x != kSizeX; ++x)
			heightmap[kSizeX * (kSizeY - 1)+ x] =
				neighbor->GetHeightMapValue(x, 0);
	}

	char szFileName[256];
	snprintf(szFileName, sizeof(szFileName),
	         "%s\\%06u\\exportedheight.raw", m_strName.c_str(),
	         (uint32_t)(terrainX) * 1000L + (uint32_t)(terrainY));

	storm::String realFilename = szFileName;

	storm::File fp;

	bsys::error_code ec;
	fp.Open(realFilename, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          realFilename, ec);
		return;
	}

	storm::WriteExact(fp, heightmap.data(),
	                  heightmap.size() * sizeof(uint16_t), ec);
}

void CMapOutdoorAccessor::SetSelectedObjectName(const char * szName)
{
	m_strSelectedObjectName.assign(szName);
}

void CMapOutdoorAccessor::ClearSelectedPortalNumber()
{
	m_kVec_iPortalNumber.clear();
}

void CMapOutdoorAccessor::AddSelectedObjectPortalNumber(int iNum)
{
	m_kVec_iPortalNumber.push_back(iNum);
}

const CMapOutdoorAccessor::TPortalNumberVector & CMapOutdoorAccessor::GetSelectedObjectPortalVectorRef()
{
	return m_kVec_iPortalNumber;
}

const char * CMapOutdoorAccessor::GetSelectedObjectName()
{
	return m_strSelectedObjectName.c_str();
}

EnvironmentData* CMapOutdoorAccessor::GetEnvironment(uint32_t id)
{
	const auto it = m_environments.find(id);
	if (it != m_environments.end())
		return &it->second;

	return nullptr;
}

const char* CMapOutdoorAccessor::GetEnvironmentPath(uint32_t id) const
{
	const auto it = m_environmentPaths.find(id);
	if (it != m_environmentPaths.end())
		return it->second.c_str();

	return "";
}

bool CMapOutdoorAccessor::SelectEnvironment(uint32_t id)
{
	const auto data = GetEnvironment(id);
	if (!data)
		return false;

	m_selectedEnvironment = id;
	SetActiveEnvironment(data);
	return true;
}

/*virtual*/ bool CMapOutdoorAccessor::RegisterEnvironment(uint32_t id, const char* filename)
{
	if (!CMapOutdoor::RegisterEnvironment(id, filename))
		return false;

	m_environmentPaths[id] = filename;
	return true;
}

void CMapOutdoorAccessor::RefreshEnvironment()
{
	SetActiveEnvironment(GetActiveEnvironment().GetData());
}

bool CMapOutdoorAccessor::SaveEnvironments()
{
	for (const auto& p : m_environments) {
		const auto& filename = m_environmentPaths[p.first];
		const auto& env = p.second;

		if (filename.empty()) {
			SPDLOG_WARN("{0}: Filename is empty", p.first);
			continue;
		}

		if (!env.Save(filename)) {
			spdlog::error("{0}: Failed to save to {1}",
			          p.first, filename);
			return false;
		}
	}

	return true;
}

struct FGetDungeonObjectHeight
{
	bool m_bHeightFound;
	float m_fReturnHeight;
	float m_fRequestX, m_fRequestY;
	FGetDungeonObjectHeight(float fRequestX, float fRequestY)
	{
		m_fRequestX=fRequestX;
		m_fRequestY=fRequestY;
		m_bHeightFound=false;
		m_fReturnHeight=0.0f;
	}
	void operator () (CGraphicObjectInstance * pObject)
	{
		if (pObject->GetObjectHeight(m_fRequestX, m_fRequestY, &m_fReturnHeight))
		{
			m_bHeightFound = true;
		}
	}
};

void CMapOutdoorAccessor::ArrangeTerrainHeight()
{
	int iRet = ::MessageBox(NULL, "This can not be undone\nDo you want to continue?", "Notification", MB_YESNO);
	if (6 != iRet)
		return;
	//TODO(imer): Unlocking is difficult due to the thread
	// We cant access the main window from the thread
	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();
	pView->Lock();

	__CreateProgressDialog();
	CMapOutdoorAccessor::main_ArrangeTerrainHeight(nullptr);
}

void CMapOutdoorAccessor::__CreateProgressDialog()
{
	__DestroyProgressDialog();

	ms_pkProgressDialog = new CMapArrangeHeightProgress;
	ms_pkProgressDialog->Create(CMapArrangeHeightProgress::IDD);
	ms_pkProgressDialog->Init();
	ms_pkProgressDialog->CenterWindow();
	ms_pkProgressDialog->ShowWindow(TRUE);
}

void CMapOutdoorAccessor::__HideProgressDialog()
{
	if (!ms_pkProgressDialog)
		return;

	ms_pkProgressDialog->ShowWindow(SW_HIDE);
}

void CMapOutdoorAccessor::__DestroyProgressDialog()
{
	if (ms_pkProgressDialog)
	{
		delete ms_pkProgressDialog;
	}
	ms_pkProgressDialog = NULL;
}

void CMapOutdoorAccessor::main_ArrangeTerrainHeight(void* pv)
{
	if (!ms_pkProgressDialog)
		return;

	for (uint8_t i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CTerrainAccessor * pTerrainAccessor;
		if (!ms_pThis->GetTerrainPointer(i, (CTerrain**) &pTerrainAccessor))
		{
			ms_pkProgressDialog->AddLine("%d/%d Skipping\n", i+1, AROUND_AREA_NUM);
			continue;
		}

		WORD wx, wy;
		pTerrainAccessor->GetCoordinate(&wx, &wy);

		int x, y;
		float afHeightMap[CTerrain::HEIGHTMAP_RAW_XSIZE*CTerrain::HEIGHTMAP_RAW_YSIZE];
		BOOL abObjectHeightFound[CTerrain::HEIGHTMAP_RAW_XSIZE*CTerrain::HEIGHTMAP_RAW_YSIZE];

		for (x = 0; x < CTerrain::HEIGHTMAP_RAW_XSIZE; ++x)
		for (y = 0; y < CTerrain::HEIGHTMAP_RAW_YSIZE; ++y)
		{
			float fxLocalPos = float(x * CTerrain::CELLSCALE);
			float fyLocalPos = float(y * CTerrain::CELLSCALE);
			float fxGlobalPos = (wx * CTerrain::TERRAIN_XSIZE) + fxLocalPos;
			float fyGlobalPos = (wy * CTerrain::TERRAIN_YSIZE) + fyLocalPos;

			afHeightMap[x + y * CTerrain::HEIGHTMAP_RAW_XSIZE] = ms_pThis->GetHeight(fxGlobalPos, fyGlobalPos);

			CCullingManager & rkCullingMgr = CCullingManager::Instance();
			Vector3d aVector3d(fxGlobalPos, -fyGlobalPos, 0.0f);
			FGetDungeonObjectHeight kGetObjHeight(fxGlobalPos, fyGlobalPos);
			RangeTester<FGetDungeonObjectHeight> kRangeTester_kGetObjHeight(&kGetObjHeight);
			rkCullingMgr.PointTest2d(aVector3d, &kRangeTester_kGetObjHeight);
			abObjectHeightFound[x + y * CTerrain::HEIGHTMAP_RAW_XSIZE] = kGetObjHeight.m_bHeightFound;
		}

		for (x = 0; x < CTerrain::HEIGHTMAP_RAW_XSIZE; ++x)
		for (y = 0; y < CTerrain::HEIGHTMAP_RAW_YSIZE; ++y)
		{
			float fChangingHeight = 0.0f;
			if (abObjectHeightFound[x + y * CTerrain::HEIGHTMAP_RAW_XSIZE])
			{
				fChangingHeight = afHeightMap[x + y * CTerrain::HEIGHTMAP_RAW_XSIZE];
			}
			else
			{
				for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
				{
					if (x+i-2 <= 0 || y+j-2 <= 0)
						continue;
					if (x+i-2 >= CTerrain::HEIGHTMAP_RAW_XSIZE-1 || y+j-2 >= CTerrain::HEIGHTMAP_RAW_YSIZE-1)
						continue;

					if (abObjectHeightFound[(x+i)+(y+j)*CTerrain::HEIGHTMAP_RAW_XSIZE])
					{
						fChangingHeight = std::max(fChangingHeight, afHeightMap[(x+i)+(y+j)*CTerrain::HEIGHTMAP_RAW_XSIZE]);
					}
				}
			}

			if (0.0f != fChangingHeight)
			{
				float fxLocalPos = float(x * CTerrain::CELLSCALE);
				float fyLocalPos = float(y * CTerrain::CELLSCALE);
				pTerrainAccessor->TerrainPutHeightmap(long(fxLocalPos/200.0f),
													  long(fyLocalPos/200.0f),
													  32767 + (fChangingHeight-16382)*2,
													  false);
			}
		}

		ms_pkProgressDialog->AddLine("%d/%d Completed\n", i+1, AROUND_AREA_NUM);
		ms_pkProgressDialog->SetProgress(float(i)/float(AROUND_AREA_NUM) * 100.0f);
	}

	ms_pkProgressDialog->AddLine("Finishing up\n");

	ms_pThis->SetTerrainModified();
	ms_pThis->__HideProgressDialog();
	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();
	pView->Unlock();
}

METIN2_END_NS
