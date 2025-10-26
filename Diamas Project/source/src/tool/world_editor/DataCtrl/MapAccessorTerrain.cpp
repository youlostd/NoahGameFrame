#include "StdAfx.h"
#include "MapAccessorTerrain.h"
#include "MapAccessorOutdoor.h"
#include "../WorldEditor.h"

#include <pak/Vfs.hpp>

#include <base/Random.hpp>

#include <storm/io/File.hpp>
#include <SpdLog.hpp>

#include <fcntl.h>
#include <il/il.h>
#include <il/ilu.h>
#include <storm/io/Format.hpp>
#include <storm/io/StreamUtil.hpp>

METIN2_BEGIN_NS

void CTerrainAccessor::RAW_ResetTextures(const BYTE * pbyTileMap)
{
	if (pbyTileMap)
	{
		for (long ly = 0; ly < TILEMAP_RAW_YSIZE; ++ly)
		{
			for (long lx = 0; lx < TILEMAP_RAW_XSIZE; ++lx)
			{
				long lOffset = ly * TILEMAP_RAW_XSIZE + lx;
				BYTE byOldRawtileNum = m_abyTileMap[lOffset];
				BYTE byNewRawtileNum = pbyTileMap[lOffset];
				if (byOldRawtileNum != byNewRawtileNum)
					RecalculateTile(lx, ly, byNewRawtileNum);
			}
		}
	}
	else
	{
		for (long y = 0; y < TILEMAP_RAW_YSIZE; ++y)
			for (long x = 0; x < TILEMAP_RAW_XSIZE; ++x)
				RecalculateTile(x, y, m_abyTileMap[y * TILEMAP_RAW_XSIZE + x]);
	}
}

void CTerrainAccessor::DrawHeightBrush(uint32_t dwBrushShape,
									uint32_t dwBrushType,
									long lCellX,
									long lCellY,
									BYTE byBrushSize,
									BYTE byBrushStrength)
{
	switch (dwBrushType)
	{
		case BRUSH_TYPE_UP:
			UpTerrain(dwBrushShape, lCellX, lCellY, byBrushSize, byBrushStrength);
			break;

		case BRUSH_TYPE_DOWN:
			DownTerrain(dwBrushShape, lCellX, lCellY, byBrushSize, byBrushStrength);
			break;

		case BRUSH_TYPE_PLATEAU:
			FlatTerrain(dwBrushShape, lCellX, lCellY, byBrushSize, byBrushStrength);
			break;

		case BRUSH_TYPE_NOISE:
			NoiseTerrain(dwBrushShape, lCellX, lCellY, byBrushSize, byBrushStrength);
			break;

		case BRUSH_TYPE_SMOOTH:
			SmoothTerrain(dwBrushShape, lCellX, lCellY, byBrushSize, byBrushStrength);
			break;

		default:
			return;
	}
	RAW_UpdateAttrSplat();
}

void CTerrainAccessor::DrawTextureBrush(uint32_t dwBrushShape,
										const std::vector<BYTE> & rVectorTextureNum,
										long lCellX,
										long lCellY,
										BYTE bySubCellX,
										BYTE bySubCellY,
										BYTE byBrushSize,
										bool bErase,
										bool bDrawOnlyOnBlankTile)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;
	BYTE origtilenum, newtilenum;

	long Left, Top;

	/* Center location */
	cx = lCellX * HEIGHT_TILE_XRATIO + bySubCellX;
	cy = lCellY * HEIGHT_TILE_YRATIO + bySubCellY;

	/* Move to upper left */
	Left = cx - byBrushSize * HEIGHT_TILE_XRATIO;
	Top = cy - byBrushSize * HEIGHT_TILE_YRATIO;

	BYTE byTextureMax = rVectorTextureNum.size();

	if (0 == byTextureMax)
		return;

	if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
	{
		for (j = 0; j < std::max(2 * byBrushSize * HEIGHT_TILE_YRATIO, 1); j++)
		{
			for (i = 0; i < std::max(2 * byBrushSize * HEIGHT_TILE_XRATIO, 1); i++)
			{
				x2 = Left + i;
				y2 = Top + j;
				if (x2 < -1 || x2 >= TILEMAP_RAW_XSIZE-1 || y2 < -1 || y2 >= TILEMAP_RAW_YSIZE-1)
					continue;
				float xf = (float) (x2) + 0.5f;
				float yf = (float) (y2) + 0.5f;

				dist = sqrtf( ( xf - (float)cx) * ( xf - (float)cx) + ( yf - (float)cy) * ( yf - (float)cy));

				if (dist < std::max(byBrushSize * HEIGHT_TILE_XRATIO, 1))
				{
					origtilenum = m_abyTileMap[(y2+1) * TILEMAP_RAW_XSIZE + (x2+1)];

					if (bDrawOnlyOnBlankTile)
					{
						if (0 == origtilenum)
						{
							newtilenum = rVectorTextureNum[GetRandom(0, byTextureMax - 1)];
							RecalculateTile(x2+1, y2+1, newtilenum);
						}
					}
					else
					{
						if (bErase)
						{
							bool bFoundNErased = false;
							for (uint32_t dwi = 0; dwi < rVectorTextureNum.size(); ++dwi)
							{
								if (origtilenum == rVectorTextureNum[dwi])
								{
									bFoundNErased = true;
									newtilenum = 0;
								}
							}
							if (!bFoundNErased)
								newtilenum = origtilenum;
						}
						else
							newtilenum = rVectorTextureNum[GetRandom(0, byTextureMax - 1)];
						RecalculateTile(x2+1, y2+1, newtilenum);
					}
				}
			}
		}
	}
	else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
	{
		for (j = 0; j < std::max(2 * byBrushSize * HEIGHT_TILE_YRATIO, 1); j++)
		{
			for (i = 0; i < std::max(2 * byBrushSize * HEIGHT_TILE_XRATIO, 1); i++)
			{
				x2 = Left + i;
				y2 = Top + j;
				if (x2 < -1 || x2 >= TILEMAP_RAW_XSIZE-1 || y2 < -1 || y2 >= TILEMAP_RAW_YSIZE-1)
					continue;

				origtilenum = m_abyTileMap[(y2+1) * TILEMAP_RAW_XSIZE + (x2+1)];

				if (bDrawOnlyOnBlankTile)
				{
					if (0 == origtilenum)
					{
						newtilenum = rVectorTextureNum[GetRandom(0, byTextureMax - 1)];
						RecalculateTile(x2+1, y2+1, newtilenum);
					}
				}
				else
				{
					if (bErase)
					{
						bool bFoundNErased = false;
						for (uint32_t dwi = 0; dwi < rVectorTextureNum.size(); ++dwi)
						{
							if (origtilenum == rVectorTextureNum[dwi])
							{
								bFoundNErased = true;
								newtilenum = 0;
							}
						}
						if (!bFoundNErased)
							newtilenum = origtilenum;
					}
					else
						newtilenum = rVectorTextureNum[GetRandom(0, byTextureMax - 1)];
					RecalculateTile(x2+1, y2+1, newtilenum);
				}
			}
		}
	}
}

void CTerrainAccessor::DrawAttrBrush(uint32_t dwBrushShape,
									 BYTE byAttrFlag,
									 long lCellX,
									 long lCellY,
									 BYTE bySubCellX,
									 BYTE bySubCellY,
									 BYTE byBrushSize,
									 bool bErase)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;

	long Left, Top;

	float fAttrHeightRatio = ((float)ATTRMAP_XSIZE) / ((float)XSIZE);
	float fAttrTileRatio = ((float)ATTRMAP_XSIZE) / ((float)TILEMAP_XSIZE);

	/* Center location */
	cx = lCellX * fAttrHeightRatio + bySubCellX * fAttrTileRatio;
	cy = lCellY * fAttrHeightRatio + bySubCellY * fAttrTileRatio;

	/* Move to upper left */
	Left = cx - byBrushSize * fAttrHeightRatio;
	Top = cy - byBrushSize * fAttrHeightRatio;

	if (bErase)
	{
		if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
		{
			for (j = 0; j < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++j)
			{
				for (i = 0; i < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= ATTRMAP_XSIZE || y2 < 0 || y2 >= ATTRMAP_YSIZE)
						continue;
					float xf = (float) (Left + i) + 0.5f;
					float yf= (float) (Top + j) + 0.5f;

					dist = sqrtf( ( xf - (float)cx) * ( xf - (float)cx) + ( yf - (float)cy) * ( yf - (float)cy));

					if (dist < std::max<float>(byBrushSize * fAttrHeightRatio, 1))
						m_abyAttrMap[y2 * ATTRMAP_XSIZE + x2] &= ~(byAttrFlag);
				}
			}
		}
		else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
		{
			for (j = 0; j < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++j)
			{
				for (i = 0; i < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= ATTRMAP_XSIZE || y2 < 0 || y2 >= ATTRMAP_XSIZE)
						continue;
					m_abyAttrMap[y2 * ATTRMAP_XSIZE + x2] &= ~(byAttrFlag);
				}
			}
		}
	}
	else
	{
		if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
		{
			for (j = 0; j < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++j)
			{
				for (i = 0; i < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= ATTRMAP_XSIZE || y2 < 0 || y2 >= ATTRMAP_XSIZE)
						continue;
					float xf = (float) (Left + i) + 0.5f;
					float yf= (float) (Top + j) + 0.5f;

					dist = sqrtf((xf - (float)cx) * (xf - (float)cx) + ( yf - (float)cy) * ( yf - (float)cy));

					if (dist < std::max<float>(byBrushSize * fAttrHeightRatio, 1))
						m_abyAttrMap[y2 * ATTRMAP_XSIZE + x2] |= (byAttrFlag);
				}
			}
		}
		else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
		{
			for (j = 0; j < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++j)
			{
				for (i = 0; i < std::max<long>(2 * byBrushSize * fAttrHeightRatio, 1); ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= ATTRMAP_XSIZE || y2 < 0 || y2 >= ATTRMAP_XSIZE)
						continue;
					m_abyAttrMap[y2 * ATTRMAP_XSIZE + x2] |= (byAttrFlag);
				}
			}
		}
	}
	RAW_NotifyAttrModified();
}

/* Fri) Modify DrawWaterBrush
* Continued Crush & abnormal operation
* The existing code at the bottom of the function is commented out and stored.
* Crush reason: The byWaterID value has not been initialized. In certain circumstances, the byWaterID value is calculated as a garbage value.
* Reason for abnormal operation of DrawWaterBrush function: The increase or decrease of m_byNumWater value is abnormal.
* Clean up abnormal values by calling RecalculateWaterMap().
* Later, in case of problems in this section, new calculations for the increase or decrease of the m_byNumWater value should be written.
*
* RAW_NotifyAttrModified(); Add
* Added to update when there is a change in properties
*/
void CTerrainAccessor::DrawWaterBrush(uint32_t dwBrushShape,
										long lCellX,
										long lCellY,
										BYTE byBrushSize,
										WORD wWaterHeight,
										bool bErase)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;
	long offset;

	long Left, Top;

	/* Center location */
	cx = lCellX;
	cy = lCellY;

	/* Move to upper left */
	Left = lCellX - byBrushSize;
	Top = lCellY - byBrushSize;

	BYTE byWaterID = MAX_WATER_NUM;

	if (MAX_WATER_NUM <= m_byNumWater)
	{
		RecalculateWaterMap();

		if(MAX_WATER_NUM <= m_byNumWater)
		{
			spdlog::error("You can not add any more water");
			return;
		}
	}

	BYTE byi;
	for (byi = 0; byi < MAX_WATER_NUM; byi++)
	{
		if (-1 == m_lWaterHeight[byi])
			break;
	}

	byWaterID = byi;
	m_lWaterHeight[byi] = wWaterHeight;
	++m_byNumWater;

	if (bErase)
	{
		if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
		{
			for (j = 0; j < 2 * byBrushSize; ++j)
			{
				for (i = 0; i < 2 * byBrushSize; ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= WATERMAP_XSIZE || y2 < 0 || y2 >= WATERMAP_YSIZE)
						continue;
					float xf = (float) (Left + i) + 0.5f;
					float yf= (float) (Top + j) + 0.5f;

					dist = sqrtf( ( xf - (float)cx) * ( xf - (float)cx) + ( yf - (float)cy) * ( yf - (float)cy));

					if (dist < byBrushSize)
					{
						offset = y2 * WATERMAP_XSIZE + x2;
						if (0xFF != m_abyWaterMap[offset])
							m_abyWaterMap[offset] = 0xFF;

						BYTE byPatchNumX = x2 / PATCH_XSIZE;
						BYTE byPatchNumY = y2 / PATCH_YSIZE;
						m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX].NeedUpdate(true);

						uint32_t dwRatio = ATTRMAP_XSIZE / WATERMAP_XSIZE;
						m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio] &= ~(ATTRIBUTE_WATER);
						m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio + 1] &= ~(ATTRIBUTE_WATER);
						m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio] &= ~(ATTRIBUTE_WATER);
						m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio + 1] &= ~(ATTRIBUTE_WATER);

						RAW_NotifyAttrModified();
					}
				}
			}
		}
		else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
		{
			for (j = 0; j < 2 * byBrushSize; ++j)
			{
				for (i = 0; i < 2 * byBrushSize; ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= WATERMAP_XSIZE || y2 < 0 || y2 >= WATERMAP_YSIZE)
						continue;
					offset = y2 * WATERMAP_XSIZE + x2;
					if (0xFF != m_abyWaterMap[offset])
						m_abyWaterMap[offset] = 0xFF;

					BYTE byPatchNumX = x2 / PATCH_XSIZE;
					BYTE byPatchNumY = y2 / PATCH_YSIZE;
					m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX].NeedUpdate(true);

					uint32_t dwRatio = ATTRMAP_XSIZE / WATERMAP_XSIZE;
					m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio] &= ~(ATTRIBUTE_WATER);
					m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio + 1] &= ~(ATTRIBUTE_WATER);
					m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio] &= ~(ATTRIBUTE_WATER);
					m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio + 1] &= ~(ATTRIBUTE_WATER);

					RAW_NotifyAttrModified();
				}
			}
		}
	}
	else
	{
		if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
		{
			for (j = 0; j < 2 * byBrushSize; ++j)
			{
				for (i = 0; i < 2 * byBrushSize; ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= WATERMAP_XSIZE || y2 < 0 || y2 >= WATERMAP_YSIZE)
						continue;
					float xf = (float) (Left + i) + 0.5f;
					float yf= (float) (Top + j) + 0.5f;

					dist = sqrtf( ( xf - (float)cx) * ( xf - (float)cx) + ( yf - (float)cy) * ( yf - (float)cy));

					if (dist < byBrushSize)
					{
						m_abyWaterMap[y2 * WATERMAP_XSIZE + x2] = byWaterID;

						BYTE byPatchNumX = x2 / PATCH_XSIZE;
						BYTE byPatchNumY = y2 / PATCH_YSIZE;
						m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX].NeedUpdate(true);

						uint32_t dwRatio = ATTRMAP_XSIZE / WATERMAP_XSIZE;
						m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio] |= (ATTRIBUTE_WATER);
						m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio + 1] |= (ATTRIBUTE_WATER);
						m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio] |= (ATTRIBUTE_WATER);
						m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio + 1] |= (ATTRIBUTE_WATER);

						RAW_NotifyAttrModified();
					}
				}
			}
		}
		else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
		{
			for (j = 0; j < 2 * byBrushSize; ++j)
			{
				for (i = 0; i < 2 * byBrushSize; ++i)
				{
					x2 = Left + i;
					y2 = Top + j;
					if (x2 < 0 || x2 >= WATERMAP_XSIZE || y2 < 0 || y2 >= WATERMAP_YSIZE)
						continue;
					m_abyWaterMap[y2 * WATERMAP_XSIZE + x2] = byWaterID;

					BYTE byPatchNumX = x2 / PATCH_XSIZE;
					BYTE byPatchNumY = y2 / PATCH_YSIZE;
					m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX].NeedUpdate(true);

					uint32_t dwRatio = ATTRMAP_XSIZE / WATERMAP_XSIZE;
					m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio] |= (ATTRIBUTE_WATER);
					m_abyAttrMap[y2 * dwRatio * ATTRMAP_XSIZE + x2 * dwRatio + 1] |= (ATTRIBUTE_WATER);
					m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio] |= (ATTRIBUTE_WATER);
					m_abyAttrMap[(y2 * dwRatio + 1) * ATTRMAP_XSIZE + x2 * dwRatio + 1] |= (ATTRIBUTE_WATER);

					RAW_NotifyAttrModified();
				}
			}
		}
	}

	RAW_UpdateAttrSplat();
}

/* 현재 cell 의 water height 값을 알수 있게 해달라는 요청에 의하여
*  CurCellEditWater() 함수를 추가함.
*/
bool CTerrainAccessor::CurCellEditWater(long lCellX, long lCellY, long& lResultHeight)
{
	int iWaterMapIndex = lCellY * WATERMAP_XSIZE + lCellX;

	if(iWaterMapIndex >= WATERMAP_YSIZE*WATERMAP_XSIZE
			|| iWaterMapIndex < 0)
		return false;

	BYTE byWaterID	= m_abyWaterMap[iWaterMapIndex];

	if(byWaterID != MAX_WATER_NUM)
	{
		lResultHeight = m_lWaterHeight[byWaterID];
		if(-1 == lResultHeight)
			return false;

		return true;
	}
	return false;
}

void CTerrainAccessor::UpTerrain(uint32_t dwBrushShape, long x, long y, BYTE byBrushSize, BYTE byBrushStrength)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	long hgt, delta;
	long Left, Top;
	float dist;

	/* Center location */
	cx = x;
	cy = y;

	/* Move to upper left */
	Left = x - byBrushSize;
	Top = y - byBrushSize;

	for (j = 0; j < 2 * byBrushSize; ++j)
	{
		for (i = 0; i < 2 * byBrushSize; ++i)
		{
			x2 = Left + i;
			y2 = Top + j;

			if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
				continue;

			dist = sqrtf( ( (float)x2 - (float)cx) * ( (float)x2 - (float)cx) + ( (float)y2 - (float)cy) * ( (float)y2 - (float)cy));

			if (dist < byBrushSize)
			{
				delta = (long) (((float) byBrushSize * (float) byBrushSize - dist * dist) * (float) byBrushStrength / 16.0f);
				if (delta <= 0)
					delta = 0;

				hgt = (long)GetHeightMapValue(x2, y2);
				hgt += delta;

				if (hgt < 0)
					hgt = 0;
				if (hgt > 65535)
					hgt = 65535;
				TerrainPutHeightmap(x2, y2, hgt, false);
			}
		}
	}
}

void CTerrainAccessor::DownTerrain(uint32_t dwBrushShape, long x, long y, BYTE byBrushSize, BYTE byBrushStrength)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	long hgt, delta;
	long Left, Top;
	float dist;

	/* Center location */
	cx = x;
	cy = y;

	/* Move to upper left */
	Left = x - byBrushSize;
	Top = y - byBrushSize;

	for (j = 0; j < 2 * byBrushSize; j++)
	{
		for (i = 0; i < 2 * byBrushSize; i++)
		{
			x2 = Left + i;
			y2 = Top + j;

			if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
				continue;

			dist = sqrtf( ( (float)x2 - (float)cx) * ( (float)x2 - (float)cx) + ( (float)y2 - (float)cy) * ( (float)y2 - (float)cy));

			if (dist < byBrushSize)
			{
				delta = (long) (((float) byBrushSize * (float) byBrushSize - dist * dist) * (float) byBrushStrength / 16.0f);
				if (delta <= 0)
					delta = 0;

				hgt = (long)GetHeightMapValue(x2, y2);
				hgt -= delta;

				if (hgt < 0)
					hgt = 0;
				if (hgt > 65535)
					hgt = 65535;
				TerrainPutHeightmap(x2, y2, hgt, false);
//				UpdateAttrMapFromHeightMap(x2, y2);
			}
		}
	}
}

void CTerrainAccessor::FlatTerrain(uint32_t dwBrushShape, long x, long y, BYTE byBrushSize, BYTE byBrushStrength)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;
	long TargetHeight, hgt, delta;
	long Left, Top;

	/* Center location */
	cx = x;
	cy = y;

	/* Move to upper left */
	Left = x - byBrushSize;
	Top = y - byBrushSize;

	if ((GetAsyncKeyState(VK_LSHIFT) & 0x80) == 0x80)
	{
		TargetHeight = 32767;
	}
	else
	{
		BYTE byMyTerrainNum;
		if (!m_pOwnerOutdoorMap->GetTerrainNumFromCoord(m_wX, m_wY, &byMyTerrainNum))
			return;

		CTerrain * pTerrain;
		if (y < 0)
		{
			if (x < 0)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum - 4, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x + XSIZE , y + YSIZE);
			}
			else if (x > XSIZE)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum - 2, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x - XSIZE , y + YSIZE);
			}
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum - 3, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x, y + YSIZE);
			}
		}
		else if (y > YSIZE)
		{
			if (x < 0)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum + 2, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x + XSIZE , y - YSIZE);
			}
			else if (x > XSIZE)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum + 4, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x - XSIZE , y - YSIZE);
			}
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum + 3, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x, y - YSIZE);
			}
		}
		else
		{
			if (x < 0)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum - 1, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x + XSIZE , y);
			}
			else if (x > XSIZE)
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byMyTerrainNum + 1, &pTerrain))
					return;
				TargetHeight = (long) pTerrain->WE_GetHeightMapValue(x - XSIZE , y);
			}
			else
			{
				TargetHeight = (long)WE_GetHeightMapValue(x, y);
			}
		}

		if (TargetHeight < 0)
			TargetHeight = 0;
		if (TargetHeight > 65535)
			TargetHeight = 65535;
	}

	if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
	{
		for (j = 0; j < 2 * byBrushSize; j++)
		{
			for (i = 0; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				dist = sqrtf( ( (float)x2 - (float)cx) * ( (float)x2 - (float)cx) + ( (float)y2 - (float)cy) * ( (float)y2 - (float)cy));

				if (dist < byBrushSize)
				{
					hgt = (long)GetHeightMapValue(x2, y2);

					delta = ( TargetHeight - hgt) * byBrushStrength / static_cast<CMapOutdoorAccessor *>(m_pOwnerOutdoorMap)->GetMaxBrushStrength();
					hgt += delta;

					if (hgt < 0)
						hgt = 0;
					if (hgt > 65535)
						hgt = 65535;
					TerrainPutHeightmap(x2, y2, hgt);
//					UpdateAttrMapFromHeightMap(x2, y2);
 				}
			}
		}
	}
	else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
	{
		for (j = 1; j < 2 * byBrushSize; j++)
		{
			for (i = 1; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				hgt = (long)GetHeightMapValue(x2, y2);

				delta = ( TargetHeight - hgt) * byBrushStrength / static_cast<CMapOutdoorAccessor *>(m_pOwnerOutdoorMap)->GetMaxBrushStrength();
				hgt += delta;

				if (hgt < 0)
					hgt = 0;
				if (hgt > 65535)
					hgt = 65535;
				TerrainPutHeightmap(x2, y2, hgt);
//				UpdateAttrMapFromHeightMap(x2, y2);
			}
		}
	}
}

void CTerrainAccessor::NoiseTerrain(uint32_t dwBrushShape, long x, long y, BYTE byBrushSize, BYTE byBrushStrength)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;
	long hgt, delta;
	long Left, Top;

	/* Center location */
	cx = x;
	cy = y;

	/* Move to upper left */
	Left = x - byBrushSize;
	Top = y - byBrushSize;

	BYTE myTerrainNum;
	m_pOwnerOutdoorMap->GetTerrainNumFromCoord(m_wX, m_wY, &myTerrainNum);
	CTerrainAccessor * pTerrainAccessor = NULL;

	if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
	{
		for (j = 0; j < 2 * byBrushSize; j++)
		{
			for (i = 0; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				/* Find distance from center of brush */
				dist = sqrtf(((float) x2 - (float) cx) * ((float)x2 - (float)cx) + ((float)y2 - (float)cy) * ((float)y2 - (float)cy));

				if (dist < byBrushSize)
				{
					delta = GetRandom(0, byBrushStrength - (byBrushStrength / 2));

					hgt = (long)GetHeightMapValue(x2, y2);
					hgt += delta;

					if (hgt < 0)
						hgt = 0;
					if (hgt > 65535)
						hgt = 65535;
					TerrainPutHeightmap(x2, y2, hgt);
				}
			}
		}
	}
	else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
	{
		for (j = 1; j < 2 * byBrushSize; j++)
		{
			for (i = 1; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				delta = GetRandom(0, byBrushStrength - byBrushStrength / 2);

				hgt = (long)GetHeightMapValue(x2, y2);
				hgt += delta;

				if (hgt < 0)
					hgt = 0;
				if (hgt > 65535)
					hgt = 65535;

				TerrainPutHeightmap(x2, y2, hgt);
			}
		}
	}
}

void CTerrainAccessor::SmoothTerrain(uint32_t dwBrushShape, long x, long y, BYTE byBrushSize, BYTE byBrushStrength)
{
	long cx, cy;
	long i, j;
	long x2, y2;
	float dist;
	long hgt, delta;
	long Left, Top;
	long xt, xb, xl, xr, yt, yb, yl, yr, zt, zb, zl, zr;

	/* Center location */
	cx = x;
	cy = y;

	/* Move to upper left */
	Left = x - byBrushSize;
	Top = y - byBrushSize;

	BYTE myTerrainNum;
	m_pOwnerOutdoorMap->GetTerrainNumFromCoord(m_wX, m_wY, &myTerrainNum);
	CTerrainAccessor * pTerrainAccessor = NULL;

	if (BRUSH_SHAPE_CIRCLE == dwBrushShape)
	{
		for (j = 0; j < 2 * byBrushSize; j++)
		{
			for (i = 0; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				xt = xb = x2;
				yl = yr = y2;
				xl = x2 - 1;
				xr = x2 + 1;
				yt = y2 - 1;
				yb = y2 + 1;

				dist = sqrtf( ( (float)x2 - (float)cx) * ( (float)x2 - (float)cx) + ( (float)y2 - (float)cy) * ( (float)y2 - (float)cy));

				if (dist < byBrushSize)
				{
					/* Find distance from center of brush */
					zt = (long)WE_GetHeightMapValue(xt, yt);
					zb = (long)WE_GetHeightMapValue(xb, yb);
					zl = (long)WE_GetHeightMapValue(xl, yl);
					zr = (long)WE_GetHeightMapValue(xr, yr);

					hgt = (long)GetHeightMapValue(x2, y2);

					delta = (zt + zb + zl + zr - 4 * hgt) / 4 * byBrushStrength / static_cast<CMapOutdoorAccessor *>(m_pOwnerOutdoorMap)->GetMaxBrushStrength();

					hgt += delta;

					if (hgt < 0)
						hgt = 0;
					if (hgt > 65535)
						hgt = 65535;
					TerrainPutHeightmap(x2, y2, hgt);
				}
			}
		}
	}
	else if (BRUSH_SHAPE_SQUARE == dwBrushShape)
	{
		for (j = 1; j < 2 * byBrushSize; j++)
		{
			for (i = 1; i < 2 * byBrushSize; i++)
			{
				x2 = Left + i;
				y2 = Top + j;

				if (x2 < -1 || x2 >= HEIGHTMAP_RAW_XSIZE - 1 || y2 < -1 || y2 >= HEIGHTMAP_RAW_YSIZE -1 )
					continue;

				xt = xb = x2;
				yl = yr = y2;
				xl = x2 - 1;
				xr = x2 + 1;
				yt = y2 - 1;
				yb = y2 + 1;

				zt = (long)WE_GetHeightMapValue(xt, yt);
				zb = (long)WE_GetHeightMapValue(xb, yb);
				zl = (long)WE_GetHeightMapValue(xl, yl);
				zr = (long)WE_GetHeightMapValue(xr, yr);

				hgt = (long)GetHeightMapValue(x2, y2);

				delta = (zt + zb + zl + zr - 4 * hgt) / 4 * byBrushStrength / static_cast<CMapOutdoorAccessor *>(m_pOwnerOutdoorMap)->GetMaxBrushStrength();

				hgt += delta;

				if (hgt < 0)
					hgt = 0;
				if (hgt > 65535)
					hgt = 65535;
				TerrainPutHeightmap(x2, y2, hgt);
			}
		}
	}
}

void CTerrainAccessor::RAW_RestoreMaps(const WORD * pHeightMap, const BYTE * pbyTileMap, const char * pNormalMap)
{
	memcpy(m_awRawHeightMap, pHeightMap, sizeof(WORD) * HEIGHTMAP_RAW_YSIZE * HEIGHTMAP_RAW_XSIZE);
	memcpy(m_acNormalMap, pNormalMap, sizeof(char) * NORMALMAP_YSIZE * NORMALMAP_XSIZE * 3);

	for (BYTE byPatchNumY = 0; byPatchNumY < PATCH_YCOUNT; ++byPatchNumY)
		for (BYTE byPatchNumX = 0; byPatchNumX < PATCH_XCOUNT; ++byPatchNumX)
			m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX].NeedUpdate(true);

	RAW_ResetTextures(pbyTileMap);
}

/* RecalculateWaterMap() 함수호출 추가.
*  RecalculateWaterMap() 함수 호출을 통하여 m_byNumWater 수치를 정상적으로 계산한다.
*/
bool CTerrainAccessor::SaveProperty(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\AreaProperty.txt", c_rstrMapName.c_str(), ulID);

	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	storm::WriteString(fp, "ScriptType AreaProperty\n");
	storm::WriteString(fp, "\n");

	storm::WriteString(fp, "AreaName \"{0}\"\n", m_strName);
	storm::WriteString(fp, "\n");

	storm::WriteString(fp, "NumWater {0}\n", m_byNumWater);
	storm::WriteString(fp, "\n");
	return !ec;
}

bool CTerrainAccessor::NewHeightMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\height.raw", c_rstrMapName.c_str(), ulID);


	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	WORD map[HEIGHTMAP_RAW_YSIZE*HEIGHTMAP_RAW_XSIZE];
	std::fill(std::begin(map), std::end(map), 0x7fff);

	storm::WriteExact(fp, map, sizeof(map), ec);
	return !ec;
}

bool CTerrainAccessor::Save(const std::string& mapName)
{
	if (!SaveProperty(mapName)) {
		spdlog::error("Failed to save property for {0} {1} {2}", mapName, m_wX, m_wY);
		return false;
	}

	if (!SaveHeightMap(mapName)) {
		spdlog::error("Failed to save height for {0} {1}", m_wX, m_wY);
		return false;
	}

	if (!RAW_SaveTileMap(mapName)) {
		spdlog::error("Failed to save tile for {0} {1}", m_wX, m_wY);
		return false;
	}

	if (!SaveWaterMap(mapName)) {
		spdlog::error("Failed to save water for {0} {1}", m_wX, m_wY);
		return false;
	}

	if (!SaveAttrMap(mapName)) {
		spdlog::error("Failed to save attr for {0} {1}", m_wX, m_wY);
		return false;
	}

	return true;
}

bool CTerrainAccessor::SaveHeightMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\height.raw", c_rstrMapName.c_str(), ulID);


	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	storm::WriteExact(fp, m_awRawHeightMap, sizeof(m_awRawHeightMap), ec);
	return !ec;
}

bool CTerrainAccessor::NewTileMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\tile.raw", c_rstrMapName.c_str(), ulID);

	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	BYTE tileMap[TILEMAP_RAW_XSIZE * TILEMAP_RAW_YSIZE] = {};
	storm::WriteExact(fp, tileMap, sizeof(tileMap), ec);
	return !ec;
}

bool CTerrainAccessor::RAW_SaveTileMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\tile.raw", c_rstrMapName.c_str(), ulID);


	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	storm::WriteExact(fp, m_abyTileMap, sizeof(m_abyTileMap), ec);
	return !ec;
}

bool CTerrainAccessor::SaveAttrMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\attr.atr", c_rstrMapName.c_str(), ulID);


	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	const WORD mapver = 2634;
	storm::WriteExact(fp, &mapver, sizeof(WORD), ec); /* write the magic number */

	const WORD wWidth = ATTRMAP_XSIZE;
	const WORD wHeight = ATTRMAP_YSIZE;
	storm::WriteExact(fp, &wWidth, sizeof(WORD), ec); /* write dimensions */
	storm::WriteExact(fp, &wHeight, sizeof(WORD), ec);

	storm::WriteExact(fp, m_abyAttrMap, sizeof(m_abyAttrMap), ec);
	return !ec;
}

bool CTerrainAccessor::NewAttrMap(const std::string & c_rstrMapName)
{
	uint32_t ulID = (uint32_t)(m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\attr.atr", c_rstrMapName.c_str(), ulID);

	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	const WORD mapver = 2634;
	storm::WriteExact(fp, &mapver, sizeof(WORD), ec); /* write the magic number */

	const WORD wWidth = ATTRMAP_XSIZE;
	const WORD wHeight = ATTRMAP_YSIZE;
	storm::WriteExact(fp, &wWidth, sizeof(WORD), ec); /* write dimensions */
	storm::WriteExact(fp, &wHeight, sizeof(WORD), ec);

	BYTE attrMap[ATTRMAP_XSIZE * ATTRMAP_YSIZE] = {};
	storm::WriteExact(fp, attrMap, sizeof(attrMap), ec);
	return !ec;
}

void CTerrainAccessor::RecalculateWaterMap()
{
	BYTE byNumWaterFirst, byNumWaterSecond;
	WORD wWidth, wHeight;
	// Phase 1 : 같은 높이 물끼리 번호 통일한다.
	for (byNumWaterFirst = 0; byNumWaterFirst < m_byNumWater - 1; ++byNumWaterFirst)
	{
		if (-1 == m_lWaterHeight[byNumWaterFirst])
			continue;
		for (byNumWaterSecond = byNumWaterFirst + 1; byNumWaterSecond < m_byNumWater; ++byNumWaterSecond)
		{
			if (-1 == m_lWaterHeight[byNumWaterSecond])
				continue;
			if (m_lWaterHeight[byNumWaterSecond] == m_lWaterHeight[byNumWaterFirst])
			{
				for (wWidth = 0; wWidth < WATERMAP_YSIZE; ++wWidth)
				{
					for (wHeight = 0; wHeight < WATERMAP_XSIZE; ++wHeight)
					{
						if (byNumWaterSecond == m_abyWaterMap[wHeight * WATERMAP_YSIZE + wWidth])
							m_abyWaterMap[wHeight * WATERMAP_XSIZE + wWidth] = byNumWaterFirst;
					}
				}
				m_lWaterHeight[byNumWaterSecond] = -1;
			}
		}
	}

	// Phase 2 : 각 번호의 물 개수를 센다.
	uint32_t dwNumWater[MAX_WATER_NUM];
	memset(dwNumWater, 0, sizeof(dwNumWater));

	for (wWidth = 0; wWidth < WATERMAP_YSIZE; ++wWidth)
	{
		for (wHeight = 0; wHeight < WATERMAP_XSIZE; ++wHeight)
		{
			BYTE byNumWater = m_abyWaterMap[wHeight * WATERMAP_XSIZE + wWidth];
			if (0xFF != byNumWater)
				++dwNumWater[byNumWater];
		}
	}

	// Phase 3 : 번호를 앞으로 밀어 붙이고 물개수 조정한다.
	BYTE byNumWaterAfterRecalculate = 0;

	for (byNumWaterFirst = 0; byNumWaterFirst < MAX_WATER_NUM - 1; ++byNumWaterFirst)
	{
		if (0 == dwNumWater[byNumWaterFirst])
		{
			bool bWaterFound = false;
			for (byNumWaterSecond = byNumWaterFirst + 1; byNumWaterSecond < MAX_WATER_NUM; ++byNumWaterSecond)
			{
				if (0 != dwNumWater[byNumWaterSecond])
				{
					bWaterFound = true;
					break;
				}
				else
					m_lWaterHeight[byNumWaterSecond] = -1;
			}
			if (!bWaterFound)
			{
				m_lWaterHeight[byNumWaterFirst] = -1;
				break;
			}
			for (WORD wWidth = 0; wWidth < WATERMAP_YSIZE; ++wWidth)
			{
				for (WORD wHeight = 0; wHeight < WATERMAP_XSIZE; ++wHeight)
				{
					if (byNumWaterSecond == m_abyWaterMap[wHeight * WATERMAP_XSIZE + wWidth])
						m_abyWaterMap[wHeight * WATERMAP_XSIZE + wWidth] = byNumWaterFirst;
				}
			}
			m_lWaterHeight[byNumWaterFirst] = m_lWaterHeight[byNumWaterSecond];
			m_lWaterHeight[byNumWaterSecond] = -1;
		}
		else
		{
			++byNumWaterAfterRecalculate;
		}
	}

	m_byNumWater = byNumWaterAfterRecalculate;
}

bool CTerrainAccessor::SaveWaterMap(const std::string & c_rstrMapName)
{
	RecalculateWaterMap();

	uint32_t ulID = (uint32_t) (m_wX) * 1000L + (uint32_t)(m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\water.wtr", c_rstrMapName.c_str(), ulID);

	storm::File fp;

	bsys::error_code ec;
	fp.Open(szFileName, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          szFileName, ec);
		return false;
	}

	const WORD mapver = 5426;
	storm::WriteExact(fp, &mapver, sizeof(WORD), ec); /* write the magic number */

	const WORD wWidth = WATERMAP_XSIZE;
	const WORD wHeight = WATERMAP_YSIZE;
	storm::WriteExact(fp, &wWidth, sizeof(wWidth), ec);
	storm::WriteExact(fp, &wHeight, sizeof(wHeight), ec);
	storm::WriteExact(fp, &m_byNumWater, sizeof(m_byNumWater), ec);

	storm::WriteExact(fp, m_abyWaterMap, sizeof(m_abyWaterMap), ec);

	if (m_byNumWater > 0)
		storm::WriteExact(fp, m_lWaterHeight, m_byNumWater * sizeof(long), ec);

	return !ec;
}

bool CTerrainAccessor::SaveShadowMap(const std::string& c_rstrMapName, LPDIRECT3DTEXTURE9 texture)
{
	char szFileName[256];
	uint32_t ulID = (uint32_t) (m_wX) * 1000L + (uint32_t)(m_wY);

	storm::String realFilename;

	_snprintf(szFileName, sizeof(szFileName), "%s\\%06u\\shadowmap.raw",
	          c_rstrMapName.c_str(), ulID);

	DeleteFileA(szFileName);

	_snprintf(szFileName, sizeof(szFileName), "%s\\%06u\\shadowmap.dds",
	          c_rstrMapName.c_str(), ulID);

	D3DXSaveTextureToFile(szFileName, D3DXIFF_DDS, texture, NULL);

	_snprintf(szFileName, sizeof(szFileName), "%s\\%06u\\shadowmap.tga",
	          c_rstrMapName.c_str(), ulID);

	D3DXSaveTextureToFile(szFileName, D3DXIFF_TGA, texture, NULL);

	{
		//////////////////////////////////////////////////////////////////////////
		// 사이즈 줄여 BMP로 변환
		ilInit();
		ilEnable(IL_FILE_OVERWRITE);

		ILuint image;
		ilGenImages(1, &image);
		ilBindImage(image);

		ilLoadImage(realFilename.c_str());
		iluScale(256, 256, 1);

		// Is there a bug.. If you save it as RAW data, the capacity is too large.
		// Save and read again. -_- Do this..
		// TODO(tim): What?
		//ilSaveImage(realFilename.c_str());
		//ilLoadImage(realFilename.c_str());

		ilConvertImage(IL_RGBA, IL_BYTE);

		// Raw Data (Shadow Map) 저장
		ILubyte * pRawData = ilGetData();

		sprintf(szFileName, "%s\\%06u\\shadowmap.raw", c_rstrMapName.c_str(), ulID);

		FILE * fp = fopen(szFileName, "w");
		if (fp)
		{
			BYTE * pbData = (BYTE *) pRawData;

			int w = ilGetInteger(IL_IMAGE_WIDTH);
			int h = ilGetInteger(IL_IMAGE_HEIGHT);

			for (int y = 0; y < h; ++y)
			{
				for (int x = 0; x < w; ++x)
				{
					BYTE r = *(pbData++);
					BYTE g = *(pbData++);
					BYTE b = *(pbData++);
					BYTE a = *(pbData++);

					// Raw565 = R5 G6 B5
					WORD wColor = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
					fwrite(&wColor, sizeof(WORD), 1, fp);
				}
			}

			fclose(fp);
		}

		ilDeleteImages(1, &image);
	}

	return true;
}

/*  minimap.dds 저장되게 끔 수정.
*  D3DXSaveTextureToFile 주석처리
*  SaveDDSDXT1 함수 호출하여 저장.
*/
bool CTerrainAccessor::SaveMiniMap(const std::string& c_rstrMapName, LPDIRECT3DTEXTURE9 texture)
{
	uint32_t ulID = (uint32_t) (m_wX) * 1000L + (uint32_t)(m_wY);


	char szFileName[256];
	_snprintf(szFileName, sizeof(szFileName), "%s\\%06u\\minimap.bmp",
	          c_rstrMapName.c_str(), ulID);

	DeleteFileA(szFileName);
	D3DXSaveTextureToFile(szFileName, D3DXIFF_BMP, texture, NULL);

	_snprintf(szFileName, sizeof(szFileName), "%s\\%06u\\minimap.dds",
	          c_rstrMapName.c_str(), ulID);

	DeleteFileA(szFileName);
	D3DXSaveTextureToFile(szFileName, D3DXIFF_DDS, texture, NULL);
	return true;
}

bool CTerrainAccessor::ReloadShadowTexture(const std::string & c_rstrMapName)
{
	char szFileName[256];
	uint32_t ulID = (uint32_t) (m_wX) * 1000L + (uint32_t)(m_wY);
	sprintf(szFileName, "%s\\%06u\\shadowmap.dds", c_rstrMapName.c_str(), ulID);

	auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(szFileName);
	m_ShadowGraphicImageInstance.ReloadImagePointer(r);

	if (!m_ShadowGraphicImageInstance.GetTexturePointer()->IsEmpty())
		m_lpShadowTexture = m_ShadowGraphicImageInstance.GetTexturePointer()->GetD3DTexture();
	else
		m_lpShadowTexture = NULL;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CTerrainAccessor::RAW_LoadAndSaveTileMap(const char *rawtilename, const std::string & c_rstrMapName, const std::vector<BYTE> & c_rVectorBaseTexture)
{
	if (!RAW_LoadTileMap(rawtilename))
	{
		spdlog::error("Failed to load tilemap {0}", rawtilename);
		return false;
	}

	BYTE byNumTexture = c_rVectorBaseTexture.size();
	for (int ix = 0; ix < TILEMAP_YSIZE; ++ix)
		for (int iy = 0; iy < TILEMAP_XSIZE; ++iy)
			m_abyTileMap[iy * TILEMAP_XSIZE + ix] = c_rVectorBaseTexture[GetRandom(0, byNumTexture - 1)];

	if (!RAW_SaveTileMap(c_rstrMapName))
	{
		spdlog::error("Failed to save tilemap {0}", c_rstrMapName);
		return false;
	}

	return true;
}
// Utility
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Attr

void CTerrainAccessor::RAW_DeallocateAttrSplats()
{
	TTerainSplat & rSplat = m_RAWAttrSplatPatch.Splats[0];
	if (m_lpAttrTexture)
	{
		ULONG ulRef;
		do
		{
			ulRef = m_lpAttrTexture->Release();
		} while(ulRef > 0);
	}
	rSplat.pd3dTexture = m_lpAttrTexture = NULL;
}

void CTerrainAccessor::RAW_AllocateAttrSplats()
{
	RAW_DeallocateAttrSplats();
	m_RAWAttrSplatPatch.m_bNeedsUpdate = true;
	m_RAWAttrSplatPatch.Splats[0].NeedsUpdate = true;
	RAW_GenerateAttrSplat();
}

void CTerrainAccessor::RAW_ResetAttrSplat()
{
	m_RAWAttrSplatPatch.m_bNeedsUpdate = true;
	m_RAWAttrSplatPatch.Splats[0].NeedsUpdate = true;
	RAW_GenerateAttrSplat();
}

void CTerrainAccessor::RAW_UpdateAttrSplat()
{
	RAW_GenerateAttrSplat();
}

void CTerrainAccessor::RAW_GenerateAttrSplat()
{
	if (!m_RAWAttrSplatPatch.m_bNeedsUpdate)
		return;

	m_RAWAttrSplatPatch.m_bNeedsUpdate = false;

	BYTE abyAlphaMap[ATTRMAP_XSIZE * ATTRMAP_YSIZE];

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	BYTE bySelectedAttrFlag = pMapManagerAccessor->GetSelectedAttrFlag();

	TTerainSplat & rAttrSplat = m_RAWAttrSplatPatch.Splats[0];
	if (rAttrSplat.NeedsUpdate)
	{
		rAttrSplat.NeedsUpdate = 0;

		// make alpha texture
		if (m_lpAttrTexture)
		{
			ULONG ulRef;
			do
			{
				ulRef = m_lpAttrTexture->Release();
			} while(ulRef > 0);
		}
		rAttrSplat.pd3dTexture = m_lpAttrTexture = NULL;

		BYTE * aptr = abyAlphaMap;

		for (long y = 0; y < ATTRMAP_YSIZE; ++y)
		{
			for (long x = 0; x < ATTRMAP_XSIZE; ++x)
			{
				if (isAttrOn(x, y, bySelectedAttrFlag))
					*aptr = 0x60;
				else
					*aptr = 0x00;

				++aptr;
			}
		}
		D3DLOCKED_RECT  d3dlr;

		HRESULT hr;
		do
		{
			hr = ms_lpd3dDevice->CreateTexture(ATTRMAP_XSIZE,
			                                   ATTRMAP_YSIZE,
			                                   1, 0,
			                                   D3DFMT_A8R8G8B8,
			                                   D3DPOOL_MANAGED,
			                                   &m_lpAttrTexture,
			                                   NULL);
		} while(FAILED(hr));

		do
		{
			hr = m_lpAttrTexture->LockRect(0, &d3dlr, 0, 0);
		} while(FAILED(hr));

		PutImage32(abyAlphaMap, (BYTE*) d3dlr.pBits, ATTRMAP_XSIZE, d3dlr.Pitch, ATTRMAP_XSIZE, ATTRMAP_YSIZE);

		do
		{
			hr = m_lpAttrTexture->UnlockRect(0);
		} while(FAILED(hr));

		rAttrSplat.pd3dTexture = m_lpAttrTexture;
	}
}

void CTerrainAccessor::RAW_NotifyAttrModified()
{
	m_RAWAttrSplatPatch.m_bNeedsUpdate = true;
	m_RAWAttrSplatPatch.Splats[0].NeedsUpdate = true;
}

void CTerrainAccessor::CopyAttr(uint8_t* dst) const
{
	std::memcpy(dst, m_abyAttrMap, sizeof(m_abyAttrMap));
}

void CTerrainAccessor::ResetAttr()
{
	for (uint32_t y = 0; y != ATTRMAP_YSIZE; ++y)
		for (uint32_t x = 0; x != ATTRMAP_XSIZE; ++x)
			m_abyAttrMap[y * ATTRMAP_XSIZE + x] &= ~0xF0;

	RAW_NotifyAttrModified();
}
// Attr
//////////////////////////////////////////////////////////////////////////

void CTerrainAccessor::RecalculateTile(long lX, long lY, BYTE byNewTileNum)
{
	if (lX < 0 || lY < 0 || lX >= TILEMAP_RAW_XSIZE || lY >= TILEMAP_RAW_YSIZE)
		return;

	uint32_t dwTileOffset = lY * TILEMAP_RAW_XSIZE + lX;
	BYTE byOrigTilelNum = m_abyTileMap[dwTileOffset];
	m_abyTileMap[dwTileOffset] = byNewTileNum;

	m_TerrainSplatPatch.m_bNeedsUpdate = true;

	if (m_TerrainSplatPatch.TileCount[byOrigTilelNum] > 0)
		--m_TerrainSplatPatch.TileCount[byOrigTilelNum];

	++m_TerrainSplatPatch.TileCount[byNewTileNum];

	for (int inum = std::min<int>(byOrigTilelNum, byNewTileNum); inum <= std::max(byOrigTilelNum, byNewTileNum); ++inum)
		m_TerrainSplatPatch.Splats[inum].NeedsUpdate = 1;

	long lPatchIndexX = std::min<long>(std::max<long>((lX - 1) / PATCH_TILE_XSIZE, 0), PATCH_XCOUNT - 1);
	long lPatchIndexY = std::min<long>(std::max<long>((lY - 1) / PATCH_TILE_YSIZE, 0), PATCH_YCOUNT - 1);
	long lPatchNum = lPatchIndexY * PATCH_XCOUNT + lPatchIndexX;

	if (m_TerrainSplatPatch.PatchTileCount[lPatchNum][byOrigTilelNum] > 0)
		--m_TerrainSplatPatch.PatchTileCount[lPatchNum][byOrigTilelNum];

	++m_TerrainSplatPatch.PatchTileCount[lPatchNum][byNewTileNum];

	if (0 == lY % PATCH_TILE_YSIZE && 0 != lY && (TILEMAP_RAW_YSIZE - 2) != lY)
	{
		++m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + lPatchIndexX][byNewTileNum];
		--m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + lPatchIndexX][byOrigTilelNum];
		if (0 == lX % PATCH_TILE_XSIZE && 0 != lX && (TILEMAP_RAW_YSIZE - 2) != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byOrigTilelNum];
			++m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byOrigTilelNum];
		}
		else if (1 == lX % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE -1) != lX && 1 != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byOrigTilelNum];
			++m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[std::min<long>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byOrigTilelNum];
		}
	}
	else if (1 == lY % PATCH_TILE_YSIZE && (TILEMAP_RAW_YSIZE -1) != lY && 1 != lY)
	{
		++m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + lPatchIndexX][byNewTileNum];
		--m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + lPatchIndexX][byOrigTilelNum];
		if (0 == lX % PATCH_TILE_XSIZE && 0 != lX && (TILEMAP_RAW_YSIZE - 2) != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byOrigTilelNum];
			++m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byOrigTilelNum];
		}
		else if (1 == lX % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE -1) !=lX && 1 != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byOrigTilelNum];
			++m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[std::max<long>(0, lPatchIndexY - 1) * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byOrigTilelNum];
		}
	}
	else
	{
		if (0 == lX % PATCH_TILE_XSIZE && 0 != lX && (TILEMAP_RAW_YSIZE - 2) != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<long>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][byOrigTilelNum];
		}
		else if (1 == lX % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE -1) != lX && 1 != lX)
		{
			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byNewTileNum];
			--m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max<long>(0, lPatchIndexX - 1)][byOrigTilelNum];
		}
	}
}

void CTerrainAccessor::Clear()
{
	memset(&m_abyAttrMap, 0, sizeof(ATTRMAP_YSIZE*ATTRMAP_XSIZE));
	memset(&m_RAWAttrSplatPatch, 0, sizeof(m_RAWAttrSplatPatch));
	CTerrain::Clear();
}

void CTerrainAccessor::TerrainPutHeightmap(long x, long y, WORD val, bool bRecursive)
{
	int iPos = (y + 1) * HEIGHTMAP_RAW_XSIZE + (x+1);
	if (iPos >= 0)
	if (iPos < HEIGHTMAP_RAW_YSIZE*HEIGHTMAP_RAW_XSIZE)
		m_awRawHeightMap[iPos] = val;

	if ( x >= 0 && y >= 0 && x < NORMALMAP_XSIZE && y < NORMALMAP_YSIZE)
		CalculateNormal(x, y);
	long x2 = x - 1;
	long y2 = y;
	if ( x2 >= 0 && y2 >= 0 && x2 < NORMALMAP_XSIZE && y2 < NORMALMAP_YSIZE)
		CalculateNormal(x2, y2);
	x2 = x + 1;
	y2 = y;
	if ( x2 >= 0 && y2 >= 0 && x2 < NORMALMAP_XSIZE && y2 < NORMALMAP_YSIZE)
		CalculateNormal(x2, y2);
	x2 = x;
	y2 = y - 1;
	if ( x2 >= 0 && y2 >= 0 && x2 < NORMALMAP_XSIZE && y2 < NORMALMAP_YSIZE)
		CalculateNormal(x2, y2);
	x2 = x;
	y2 = y + 1;
	if ( x2 >= 0 && y2 >= 0 && x2 < NORMALMAP_XSIZE && y2 < NORMALMAP_YSIZE)
		CalculateNormal(x2, y2);

	BYTE byPatchNumX, byPatchNumY;
	byPatchNumX = std::max<BYTE>(x, 0) / PATCH_XSIZE;
	byPatchNumY = std::max<BYTE>(y, 0) / PATCH_YSIZE;
	int iPatchPos = byPatchNumY * PATCH_XCOUNT + byPatchNumX;
	if (iPatchPos >= 0)
	if (iPatchPos < PATCH_XCOUNT * PATCH_YCOUNT)
		m_TerrainPatchList[iPatchPos].NeedUpdate(true);
	if ( y % PATCH_YSIZE == 0)
	{
		if ( x % PATCH_YSIZE == 0)
		{
			int iPatchPos = (byPatchNumY - 1) * PATCH_XCOUNT + (byPatchNumX - 1);
			if (iPatchPos >= 0)
			if (iPatchPos < PATCH_XCOUNT * PATCH_YCOUNT)
				m_TerrainPatchList[iPatchPos].NeedUpdate(true);
		}
		else
		{
			int iPatchPos = (byPatchNumY - 1) * PATCH_XCOUNT + byPatchNumX;
			if (iPatchPos >= 0)
			if (iPatchPos < PATCH_XCOUNT * PATCH_YCOUNT)
				m_TerrainPatchList[iPatchPos].NeedUpdate(true);
		}
	}
	else if ( x % PATCH_YSIZE == 0)
	{
		int iPatchPos = byPatchNumY * PATCH_XCOUNT + (byPatchNumX - 1);
		if (iPatchPos >= 0)
		if (iPatchPos < PATCH_XCOUNT * PATCH_YCOUNT)
			m_TerrainPatchList[iPatchPos].NeedUpdate(true);
	}

	if (!bRecursive)
		return;

	bool bWrongPut= false;
	int i,j;

	BYTE byTerrainNum;
	if ( !m_pOwnerOutdoorMap->GetTerrainNumFromCoord(m_wX, m_wY, &byTerrainNum) )
	{
		spdlog::error("CTerrainAccessor::TerrainPutHeightmap: Can't Get TerrainNum from Coord {0} {1}", m_wX, m_wY);
		byTerrainNum = 4;
	}

	if (( x >= 0 && x < HEIGHTMAP_XSIZE && y >= 0 && y < HEIGHTMAP_YSIZE ) && ((x<=1 || x>=HEIGHTMAP_XSIZE-2) || (y<=1 || y>=HEIGHTMAP_YSIZE-2)))
	{
		for(i=-1;i<=1;i++)
		{
			for(j=-1;j<=1;j++)
			{
				if (i==j && i==0)
					continue;
				else
				{
					int nx, ny;
					nx = x-j*YSIZE;
					ny = y-i*XSIZE;
					if ( (nx<=0 || ny<=0 || nx >= HEIGHTMAP_RAW_XSIZE-3 || ny >= HEIGHTMAP_RAW_YSIZE-3)&&
						nx>=-1 && nx<HEIGHTMAP_RAW_XSIZE-1 && ny>=-1 && ny<HEIGHTMAP_RAW_YSIZE-1)
					{
						CTerrain* pTerrain;
						if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum+i*3+j,&pTerrain))
							bWrongPut = true;
						else
							((CTerrainAccessor*)pTerrain)->TerrainPutHeightmap(nx,ny,val,false);
					}
				}
			}
		}
	}

	if (bWrongPut)
		spdlog::error("Wrongly put TerrainNum({1}), x({2}), y({3})\n", byTerrainNum, x, y);
}

CTerrainAccessor::CTerrainAccessor() : m_lpAttrTexture(NULL)
{
	m_isDestroied = FALSE;
}

CTerrainAccessor::~CTerrainAccessor()
{
	assert(!m_isDestroied);
	RAW_DeallocateAttrSplats();
	Clear();
	m_isDestroied = TRUE;
}

METIN2_END_NS
