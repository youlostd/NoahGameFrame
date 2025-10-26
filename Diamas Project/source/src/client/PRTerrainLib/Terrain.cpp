#include "Stdafx.h"
#include "Terrain.h"
#include <pak/Vfs.hpp>
#include "../EterBase/MappedFile.h"

#include "../EterBase/Timer.h"

#include <cmath>
#include <storm/io/View.hpp>

//////////////////////////////////////////////////////////////////////////
// Texture Set
//////////////////////////////////////////////////////////////////////////
CTextureSet *CTerrainImpl::ms_pTextureSet = NULL;

void CTerrainImpl::SetTextureSet(CTextureSet *pTextureSet)
{
    static CTextureSet s_EmptyTextureSet;

    if (!pTextureSet)
        ms_pTextureSet = &s_EmptyTextureSet;
    else
        ms_pTextureSet = pTextureSet;
}

CTextureSet *CTerrainImpl::GetTextureSet()
{
    if (!ms_pTextureSet)
        SetTextureSet(NULL);

    return ms_pTextureSet;
}

CTerrainImpl::CTerrainImpl()
{
    Initialize();
}

CTerrainImpl::~CTerrainImpl()
{
    Clear();
}

void CTerrainImpl::Initialize()
{
    std::memset(m_lpAlphaTexture, 0, sizeof(m_lpAlphaTexture));
    std::memset(m_awRawHeightMap, 0, sizeof(m_awRawHeightMap));
    std::memset(m_abyTileMap, 0, sizeof(m_abyTileMap));
    std::memset(m_abyAttrMap, 0, sizeof(m_abyAttrMap));
    std::memset(m_abyWaterMap, 0, sizeof(m_abyWaterMap));
    std::memset(m_acNormalMap, 0, sizeof(m_acNormalMap));

    m_wTileMapVersion = 8976;
    m_lViewRadius = 0;
    m_fHeightScale = 0.0f;

    std::memset(&m_TerrainSplatPatch, 0, sizeof(m_TerrainSplatPatch));

    m_byNumWater = 0;
    std::memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));

    m_lpShadowTexture = NULL;
    std::memset(m_awShadowMap, 0xFF, sizeof(m_awShadowMap));

    m_lSplatTilesX = 0;
    m_lSplatTilesY = 0;
}

void CTerrainImpl::Clear()
{
    for (uint32_t i = 0; i < GetTextureSet()->GetTextureCount(); ++i)
    {
        if (m_lpAlphaTexture[i])
            m_lpAlphaTexture[i]->Release();
    }

    Initialize();
}

bool CTerrainImpl::LoadHeightMap(const char *c_szFileName)
{
    SPDLOG_DEBUG("LoadRawHeightMapFile {0}", c_szFileName);

    auto fp = GetVfs().Open(c_szFileName);
    if (!fp)
    {
        SPDLOG_ERROR("HeightMap {0} OPEN ERROR", c_szFileName);
        return false;
    }

    const auto size = fp->GetSize();
    if (size != sizeof(m_awRawHeightMap))
    {
        SPDLOG_ERROR("HeightMap {0} SIZE ERROR", c_szFileName);
        return false;
    }

    fp->Read(0, m_awRawHeightMap, sizeof(m_awRawHeightMap));
    return true;
}

#pragma pack(push)
#pragma pack(1)
static const uint16_t c_wAttrMapMagic = 2634;

struct SAttrMapHeader
{
    uint16_t m_wMagic;
    uint16_t m_wWidth;
    uint16_t m_wHeight;
};
#pragma pack(pop)

bool CTerrainImpl::LoadAttrMap(const char *c_szFileName)
{
	DWORD dwStart = ELTimer_GetMSec();

	auto fp = GetVfs().Open(c_szFileName);
	if (!fp) {
		SPDLOG_ERROR("AttrMap {0} OPEN ERROR", c_szFileName);
		return false;
	}

	DWORD dwFileSize = fp->GetSize();
	if (dwFileSize < sizeof(SAttrMapHeader)) {
		SPDLOG_ERROR(" AttrMap {0} FILE SIZE ERROR", c_szFileName);
		return false;
	}

	storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, sizeof(SAttrMapHeader));

	auto& header = *reinterpret_cast<const SAttrMapHeader*>(data.GetData());

	if (c_wAttrMapMagic != header.m_wMagic) {
		SPDLOG_ERROR("AttrMap {0} MAGIC NUMBER({1}!=MAGIC[{2}])",
		          c_szFileName, header.m_wMagic, c_wAttrMapMagic);
		return false;
	}

	if (ATTRMAP_XSIZE != header.m_wWidth) {
		SPDLOG_ERROR("header({0}).m_width({1})!=ATTRMAP_XSIZE({2})",
		          c_szFileName, header.m_wWidth, ATTRMAP_XSIZE);
		return false;
	}

	if (ATTRMAP_YSIZE != header.m_wHeight) {
		SPDLOG_ERROR("header({0}).m_height({1})!=ATTRMAP_YSIZE({2})",
		          c_szFileName, header.m_wHeight, ATTRMAP_YSIZE);
		return false;
	}

	DWORD dwFileRestSize = dwFileSize - sizeof(header);
	DWORD dwFileNeedSize = sizeof(m_abyAttrMap);

	if (dwFileRestSize != dwFileNeedSize) {
		SPDLOG_ERROR("AttrMap {0} FILE DATA SIZE(rest {1} != need {2})",
		          c_szFileName, dwFileRestSize, dwFileNeedSize);
		return false;
	}

	fp->Read(sizeof(header), m_abyAttrMap, sizeof(m_abyAttrMap));

	SPDLOG_DEBUG("LoadAttrMapFile {0} took {1} ms",
	           c_szFileName, ELTimer_GetMSec() - dwStart);
	return true;
}

bool CTerrainImpl::RAW_LoadTileMap(const char *c_szFileName)
{
    SPDLOG_DEBUG("LoadSplatFile {0}", c_szFileName);

    auto fp = GetVfs().Open(c_szFileName);

    if (!fp)
    {
        SPDLOG_ERROR("RawTileMap {0} OPEN ERROR", c_szFileName);
        return false;
    }

    const auto size = fp->GetSize();
    if (size != sizeof(m_abyTileMap))
    {
        SPDLOG_ERROR("RawTileMap {0} SIZE ERROR", c_szFileName);
        return false;
    }

    fp->Read(0, m_abyTileMap, sizeof(m_abyTileMap));
    return true;
}

bool CTerrainImpl::LoadWaterMap(const char *c_szFileName)
{
    uint32_t dwStart = ELTimer_GetMSec();

    if (!LoadWaterMapFile(c_szFileName))
    {
        memset(m_abyWaterMap, 0xFF, sizeof(m_abyWaterMap));

        m_byNumWater = 0;
        memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));

        SPDLOG_ERROR("LoadWaterMapFile({0}) Failed", c_szFileName);
        return false;
    }

    SPDLOG_DEBUG("LoadWaterMapFile {0} took {1}ms",
                  c_szFileName, ELTimer_GetMSec() - dwStart);
    return true;
}

// TODO: What is this doing here
#pragma pack(push)
#pragma pack(1)
static const uint16_t c_wWaterMapMagic = 5426;

struct SWaterMapHeader
{
    uint16_t m_wMagic;
    uint16_t m_wWidth;
    uint16_t m_wHeight;
    uint8_t m_byLayerCount;
};
#pragma pack(pop)

bool CTerrainImpl::LoadWaterMapFile(const char *c_szFileName)
{
   	auto fp = GetVfs().Open(c_szFileName);
	if (!fp) {
		SPDLOG_ERROR("WaterMap {0} OPEN ERROR", c_szFileName);
		return false;
	}

	DWORD dwFileSize = fp->GetSize();
	if (dwFileSize < sizeof(SWaterMapHeader)) {
		SPDLOG_ERROR(" WaterMap {0} FILE SIZE ERROR", c_szFileName);
		return false;
	}

	storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, sizeof(SWaterMapHeader));

	auto& header = *reinterpret_cast<const SWaterMapHeader*>(data.GetData());

	if (c_wWaterMapMagic != header.m_wMagic) {
		SPDLOG_ERROR("WaterMap {0} MAGIC NUMBER({1}!=MAGIC[{2}])",
		          c_szFileName, header.m_wMagic, c_wWaterMapMagic);
		return false;
	}

	if (WATERMAP_XSIZE != header.m_wWidth) {
		SPDLOG_ERROR("header({0}).m_width({1})!=WATERMAP_XSIZE({2})",
		          c_szFileName, header.m_wWidth, WATERMAP_XSIZE);
		return false;
	}

	if (WATERMAP_YSIZE != header.m_wHeight) {
		SPDLOG_ERROR("header({0).m_height({1})!=WATERMAP_YSIZE({2})",
		          c_szFileName, header.m_wHeight, WATERMAP_YSIZE);
		return false;
	}

	m_byNumWater = header.m_byLayerCount;

	DWORD dwFileRestSize = dwFileSize - sizeof(header);
	DWORD dwFileNeedSize = sizeof(m_abyWaterMap) + sizeof(long) * m_byNumWater;
	DWORD dwFileNeedSize2 = sizeof(m_abyWaterMap) + sizeof(WORD) * m_byNumWater;

	if (dwFileRestSize == dwFileNeedSize2) {
		fp->Read(sizeof(header), m_abyWaterMap, sizeof(m_abyWaterMap));

		m_byNumWater = std::min<BYTE>(MAX_WATER_NUM, m_byNumWater);
		if (m_byNumWater) {
			WORD wWaterHeight[MAX_WATER_NUM + 1];
			fp->Read(sizeof(header) + sizeof(m_abyWaterMap),
			         wWaterHeight, sizeof(WORD) * m_byNumWater);

			for (int i = 0; i < m_byNumWater; ++i)
				m_lWaterHeight[i] = wWaterHeight[i];
		}
	} else if (dwFileRestSize != dwFileNeedSize) {
		SPDLOG_ERROR("WaterMap {0} FILE DATA SIZE(rest {1} != need {2})",
		          c_szFileName, dwFileRestSize, dwFileNeedSize);
		return false;
	}

	fp->Read(sizeof(header), m_abyWaterMap, sizeof(m_abyWaterMap));

	if (m_byNumWater)
		fp->Read(sizeof(header) + sizeof(m_abyWaterMap),
		         m_lWaterHeight, sizeof(long) * m_byNumWater);

	return true;
}

uint32_t CTerrainImpl::GetShadowMapColor(float fx, float fy)
{
    float fMapSize = (float)(TERRAIN_XSIZE);
    float fooMapSize = 1.0f / fMapSize;
    if (fx < 0 || fy < 0 || fx >= fMapSize || fy >= fMapSize)
        return 0xFFFFFFFF;

    fx = fx * fooMapSize * (float)(SHADOWMAP_XSIZE - 1);
    fy = fy * fooMapSize * (float)(SHADOWMAP_YSIZE - 1);
    int ix, iy;
    PR_FLOAT_TO_INT(fx, ix);
    PR_FLOAT_TO_INT(fy, iy);

    uint16_t w = *(m_awShadowMap + (iy * SHADOWMAP_XSIZE) + ix);

    int b = w & 0x1f;
    w >>= 5;
    b <<= 3;
    int g = w & 0x1f;
    w >>= 5;
    g <<= 3;
    int r = w & 0x1f;
    r <<= 3;

    return (uint32_t)(0xff << 24) | (g << 16) | (g << 8) | r;
}
