// TerrainQuadtreeNode.h: interface for the CTerrainQuadtreeNode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef METIN2_CLIENT_GAMELIB_TERRAINQUADTREE_HPP
#define METIN2_CLIENT_GAMELIB_TERRAINQUADTREE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CTerrainQuadtreeNode
{
  public:
    CTerrainQuadtreeNode();
    virtual ~CTerrainQuadtreeNode();

  public:
    long x0, y0, x1, y1;
    CTerrainQuadtreeNode *NW_Node;
    CTerrainQuadtreeNode *NE_Node;
    CTerrainQuadtreeNode *SW_Node;
    CTerrainQuadtreeNode *SE_Node;
    long Size;
    long PatchNum;
    Vector3 center;
    float radius;
    uint8_t m_byLODLevel;
};

#endif
