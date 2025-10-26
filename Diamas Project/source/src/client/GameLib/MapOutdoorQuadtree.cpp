#include "StdAfx.h"
#include "MapOutdoor.h"
#include "TerrainQuadtree.h"

//////////////////////////////////////////////////////////////////////////
// QuadTree
//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::BuildQuadTree()
{
    FreeQuadTree();

    // m_wPatchCount�� ConvertPatchSplat������ ���Ѵ�, ������ ���� ���⼭ üũ�Ѵ�.
    if (0 == m_wPatchCount)
    {
        SPDLOG_ERROR(
            "MapOutdoor::BuildQuadTree : m_wPatchCount is zero, you must call ConvertPatchSplat before call this method.");
        return;
    }

    m_pRootNode = AllocQuadTreeNode(0, 0, m_wPatchCount - 1, m_wPatchCount - 1);
    if (!m_pRootNode)
        SPDLOG_ERROR("CMapOutdoor::BuildQuadTree() RootNode is NULL");

    if (m_pRootNode->Size > 1)
        SubDivideNode(m_pRootNode);
}

CTerrainQuadtreeNode *CMapOutdoor::AllocQuadTreeNode(long x0, long y0, long x1, long y1)
{
    CTerrainQuadtreeNode *Node;
    long xsize, ysize;

    xsize = x1 - x0 + 1;
    ysize = y1 - y0 + 1;
    if ((xsize == 0) || (ysize == 0))
        return NULL;

    Node = new CTerrainQuadtreeNode;
    Node->x0 = x0;
    Node->y0 = y0;
    Node->x1 = x1;
    Node->y1 = y1;

    if (ysize > xsize)
        Node->Size = ysize;
    else
        Node->Size = xsize;

    Node->PatchNum = y0 * m_wPatchCount + x0;

    /*
        const float fTerrainMin = -(float) (m_lViewRadius * m_lCellScale);
        
        minx = fTerrainMin + x0 * c_byPatchSize * m_lCellScale;
        maxx = fTerrainMin + (x1 + 1) * c_byPatchSize * m_lCellScale;
        miny = fTerrainMin + y0 * c_byPatchSize * m_lCellScale;
        maxy = fTerrainMin + (y1 + 1) * c_byPatchSize * m_lCellScale;
        minz = 0.0f;
        maxz = 0.0f;
    
        / * Set up 8 vertices that belong to the bounding box * /
        Node->center.x = minx + (maxx - minx) * 0.5f;
        Node->center.y = miny + (maxy - miny) * 0.5f;
        Node->center.z = minz + (maxz - minz) * 0.5f;
    
        Node->radius = sqrtf(
            (maxx-minx)*(maxx-minx)+
            (maxy-miny)*(maxy-miny)+
            (maxz-minz)*(maxz-minz)
            )/2.0f;
    */

    Node->center.x = 0.0f;
    Node->center.y = 0.0f;
    Node->center.z = 0.0f;

    Node->radius = 0.0f;

    return Node;
}

void CMapOutdoor::SubDivideNode(CTerrainQuadtreeNode *Node)
{
    long nw_size;
    CTerrainQuadtreeNode *tempnode;

    nw_size = Node->Size / 2;

    Node->NW_Node = AllocQuadTreeNode(Node->x0, Node->y0, Node->x0 + nw_size - 1, Node->y0 + nw_size - 1);
    Node->NE_Node = AllocQuadTreeNode(Node->x0 + nw_size, Node->y0, Node->x1, Node->y0 + nw_size - 1);
    Node->SW_Node = AllocQuadTreeNode(Node->x0, Node->y0 + nw_size, Node->x0 + nw_size - 1, Node->y1);
    Node->SE_Node = AllocQuadTreeNode(Node->x0 + nw_size, Node->y0 + nw_size, Node->x1, Node->y1);

    tempnode = (CTerrainQuadtreeNode *)Node->NW_Node;
    if ((tempnode != NULL) && (tempnode->Size > 1))
        SubDivideNode(tempnode);
    tempnode = (CTerrainQuadtreeNode *)Node->NE_Node;
    if ((tempnode != NULL) && (tempnode->Size > 1))
        SubDivideNode(tempnode);
    tempnode = (CTerrainQuadtreeNode *)Node->SW_Node;
    if ((tempnode != NULL) && (tempnode->Size > 1))
        SubDivideNode(tempnode);
    tempnode = (CTerrainQuadtreeNode *)Node->SE_Node;
    if ((tempnode != NULL) && (tempnode->Size > 1))
        SubDivideNode(tempnode);
}

void CMapOutdoor::FreeQuadTree()
{
    delete m_pRootNode;
    m_pRootNode = NULL;
}
