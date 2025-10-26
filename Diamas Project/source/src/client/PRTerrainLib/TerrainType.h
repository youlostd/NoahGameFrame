#ifndef METIN2_CLIENT_PRTERRAINLIB_TERRAINTYPE_HPP
#define METIN2_CLIENT_PRTERRAINLIB_TERRAINTYPE_HPP


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include "../Eterlib/GrpVertexBuffer.h"
#include "../Eterlib/GrpIndexBuffer.h"



#define TERRAIN_PATCHSIZE 16
#define TERRAIN_SIZE 128
#define TERRAIN_PATCHCOUNT TERRAIN_SIZE/TERRAIN_PATCHSIZE
#define MAXTERRAINTEXTURES 256

typedef struct
{
	long Active;
	long NeedsUpdate;
	LPDIRECT3DTEXTURE9 pd3dTexture;
} TTerainSplat;

typedef struct
{
	uint32_t TileCount[MAXTERRAINTEXTURES];
	uint32_t PatchTileCount[TERRAIN_PATCHCOUNT*TERRAIN_PATCHCOUNT][MAXTERRAINTEXTURES];
	TTerainSplat Splats[MAXTERRAINTEXTURES];
	bool m_bNeedsUpdate;
} TTerrainSplatPatch;

#define PR_FLOAT_TO_INT(inreal, outint) (outint) = static_cast<int>(inreal);


#endif
