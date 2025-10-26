#ifndef METIN2_CLIENT_GAMELIB_MAPOUTDOOR_H
#define METIN2_CLIENT_GAMELIB_MAPOUTDOOR_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/LensFlare.h"
#include "../EterLib/ScreenFilter.h"
#include "../EterLib/SkyBox.h"

#include "../PRTerrainLib/TerrainType.h"
#include "../PRTerrainLib/TextureSet.h"

#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "Area.h"
#include "AreaLoaderThread.h"
#include "AreaTerrain.h"
#include "EnvironmentInstance.hpp"
#include "EnvironmentShader.h"
#include "MapBase.h"

#include <list>
#include <memory>
#include <optional>
#include <unordered_map>
#include <wrl/client.h>

#undef LoadString

#define LOAD_SIZE_WIDTH 1

#define AROUND_AREA_NUM 1 + (LOAD_SIZE_WIDTH * 2) * (LOAD_SIZE_WIDTH * 2) * 2
#define MAX_PREPARE_SIZE 9
#define MAX_MAPSIZE 256 // 0 ~ 255, cellsize 200 = 64km

#define TERRAINPATCH_LODMAX 3

typedef struct SOutdoorMapCoordinate
{
    short m_sTerrainCoordX; // Terrain ÁÂÇ¥
    short m_sTerrainCoordY;
} TOutdoorMapCoordinate;

class CTerrainPatchProxy;
class CTerrainQuadtreeNode;
class CSpeedTreeWrapper;

struct EnvironmentData;

class CMapOutdoor : public CMapBase
{
  public:
    typedef std::unordered_map<uint32_t, EnvironmentData> EnvironmentMap;

    enum
    {
        VIEW_NONE = 0,
        VIEW_PART,
        VIEW_ALL,
    };

    enum EPart
    {
        PART_TERRAIN,
        PART_OBJECT,
        PART_CLOUD,
        PART_WATER,
        PART_TREE,
        PART_SKY,
        PART_AREA_EFFECT,
        PART_NUM,
    };

    enum ETerrainRenderSort
    {
        DISTANCE_SORT,
        TEXTURE_SORT,
    };

  public:
    CMapOutdoor();
    virtual ~CMapOutdoor();
    void InitBaseTrees();

protected:
    bool Initialize();

    virtual bool Destroy();

    virtual void OnRender();

  public:
    void SetInverseViewAndDynamicShaodwMatrices();
    virtual bool Load(float x, float y, float z);
    virtual float GetHeight(float x, float y);

    virtual bool Update(float fX, float fY, float fZ);
    virtual void UpdateAroundAmbience(float fX, float fY, float fZ);

  public:
    void Clear();

    void SetVisiblePart(int ePart, bool isVisible);
    void SetSplatLimit(int iSplatNum);
    std::vector<int> &GetRenderedSplatNum(int *piPatch, int *piSplat, float *pfSplatRatio);

    bool LoadSetting(const char *c_szFileName);
    bool ReloadSetting(bool useSnowTexture);

    void SetActiveEnvironment(const EnvironmentData *env);

    const EnvironmentInstance &GetActiveEnvironment() const;

    EnvironmentInstance *GetActiveEnvironmentPtr();

    virtual bool RegisterEnvironment(uint32_t id, const char *filename);
    const EnvironmentData *GetEnvironment(uint32_t id) const;

    CSkyBox &GetSkyBox()
    {
        return m_SkyBox;
    }
    CLensFlare &GetLensFlare()
    {
        return m_LensFlare;
    }
    CScreenFilter &GetScreenFilter()
    {
        return m_ScreenFilter;
    }

    void CreateCharacterShadowTexture();
    void ReleaseCharacterShadowTexture();
    void SetShadowTextureSize(uint16_t size);

    bool BeginRenderCharacterShadowToTexture();
    void EndRenderCharacterShadowToTexture();
    void RenderWater();
    void RenderMarkedArea();
    void RecurseRenderAttr(CTerrainQuadtreeNode *Node, bool bCullEnable = TRUE);
    void DrawPatchAttr(long patchnum);
    void ClearGuildArea();
    void RegisterGuildArea(int isx, int isy, int iex, int iey);

    void VisibleMarkedArea();
    void DisableMarkedArea();

    void UpdateSky();
    void RenderCollision();
    void RenderSky();
    void RenderCloud();
    void RenderBeforeLensFlare();
    void RenderAfterLensFlare();
    void RenderScreenFiltering();

    void SetWireframe(bool bWireFrame);
    bool IsWireframe();

    bool GetPickingPointWithRay(const CRay &rRay, Vector3 *v3IntersectPt);
    bool GetPickingPointWithRayOnlyTerrain(const CRay &rRay, Vector3 *v3IntersectPt);
    bool GetPickingPoint(Vector3 *v3IntersectPt);
    void GetTerrainCount(short *psTerrainCountX, short *psTerrainCountY)
    {
        *psTerrainCountX = m_sTerrainCountX;
        *psTerrainCountY = m_sTerrainCountY;
    }

    bool SetTerrainCount(short sTerrainCountX, short sTerrainCountY);

    // Shadow
    void SetDrawShadow(bool bDrawShadow);
    void SetDrawCharacterShadow(bool bDrawChrShadow);

    uint32_t GetShadowMapColor(float fx, float fy);

  protected:
    bool __PickTerrainHeight(float &fPos, const Vector3 &v3Start, const Vector3 &v3End, float fStep,
                             float fRayRange, float fLimitRange, Vector3 *pv3Pick);

    void __UpdateGarbage();

    virtual bool LoadTerrain(uint16_t wTerrainCoordX, uint16_t wTerrainCoordY);
    virtual bool LoadArea(uint16_t wAreaCoordX, uint16_t wAreaCoordY);
    virtual void UpdateAreaList(long lCenterX, long lCenterY);
    bool isTerrainLoaded(uint16_t wX, uint16_t wY);
    bool isAreaLoaded(uint16_t wX, uint16_t wY);

    void AssignTerrainPtr(); // 현재 좌표에서 주위(ex. 3x3)에 있는 것들의 포인터를 연결한다. (업데이트 시 불려짐)

    //////////////////////////////////////////////////////////////////////////
    // New
    //////////////////////////////////////////////////////////////////////////
    // 여러가지 맵들을 얻는다.
    void GetHeightMap(const uint8_t &c_rucTerrainNum, uint16_t **pwHeightMap);
    void GetNormalMap(const uint8_t &c_rucTerrainNum, char **pucNormalMap);

    // Water
    void GetWaterMap(const uint8_t &c_rucTerrainNum, uint8_t **pucWaterMap);
    void GetWaterHeight(uint8_t byTerrainNum, uint8_t byWaterNum, long *plWaterHeight);

    //////////////////////////////////////////////////////////////////////////
    // Terrain
    //////////////////////////////////////////////////////////////////////////
  protected:
    // 데이터
    CTerrain *m_pTerrain[AROUND_AREA_NUM]{}; // Terrain
    CTerrainPatchProxy *m_pTerrainPatchProxyList{}; // Polygon patches that actually render when rendering terrain...
                                                    // Independent from CTerrain for Seamless Map...

    long m_lViewRadius{};   // Viewing distance.. Cell unit..
    float m_fHeightScale{}; // Height scale... When it is 1.0, it can be expressed from 0 to 655.35 meters.

    short m_sTerrainCountX{}, m_sTerrainCountY{}; // seamless map 안에 들어가는 Terrain개수

    TOutdoorMapCoordinate m_CurCoordinate{}; // 현재의 좌표

    long m_lCurCoordStartX{}, m_lCurCoordStartY{};
    TOutdoorMapCoordinate m_PrevCoordinate{}; // 현재의 좌표

    uint16_t m_wPatchCount{};

    //////////////////////////////////////////////////////////////////////////
    // Index Buffer
    uint16_t *m_pwaIndices[TERRAINPATCH_LODMAX]{};

    IndexBufferHandle m_IndexBuffer[TERRAINPATCH_LODMAX]{};
    uint16_t m_wNumIndices[TERRAINPATCH_LODMAX]{};

    virtual void DestroyTerrain();

    void CreateTerrainPatchProxyList();
    void DestroyTerrainPatchProxyList();

    void UpdateTerrain(float fX, float fY);

    void ConvertTerrainToTnL(long lx, long ly);

    void AssignPatch(long lPatchNum, long lx0, long ly0, long lx1, long ly1);

    //////////////////////////////////////////////////////////////////////////
    // Index Buffer
    void ADDLvl1TL(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl1T(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl1TR(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl1L(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl1R(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl1BL(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl1B(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl1BR(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl1M(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl2TL(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl2T(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl2TR(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl2L(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl2R(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl2BL(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl2B(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);
    void ADDLvl2BR(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount,
                   const uint8_t &c_rucNumLineWarp);
    void ADDLvl2M(uint16_t *pIndices, uint16_t &rwCount, const uint16_t &c_rwCurCount, const uint8_t &c_rucNumLineWarp);

  public:
    bool GetTerrainPointer(uint8_t c_ucTerrainNum, CTerrain **ppTerrain);
    float GetTerrainHeight(float fx, float fy);
    bool GetWaterHeight(int iX, int iY, long *plWaterHeight);
    bool GetNormal(int ix, int iy, Vector3 *pv3Normal);

    void RenderTerrain();

    const long GetViewRadius()
    {
        return m_lViewRadius;
    }
    const float GetHeightScale()
    {
        return m_fHeightScale;
    }

    const TOutdoorMapCoordinate &GetCurCoordinate()
    {
        return m_CurCoordinate;
    }
    const TOutdoorMapCoordinate &GetPrevCoordinate()
    {
        return m_PrevCoordinate;
    }

    //////////////////////////////////////////////////////////////////////////
    // Area
    //////////////////////////////////////////////////////////////////////////
  protected:
    CArea *m_pArea[AROUND_AREA_NUM]{}; // Data

    void DestroyArea();

    virtual void UpdateArea(const Vector3 &v3Player);

    void __BuildDynamicSphereInstanceVector();

    void __CollectShadowReceiver(const Vector3 &v3Target, const Vector3 &v3Light);
    void __CollectCollisionPCBlocker(const Vector3 &v3Eye, const Vector3 &v3Target, float fDistance);
    void __CollectCollisionShadowReceiver(const Vector3 &v3Target, const Vector3 &v3Light);
    void __UpdateAroundAreaList();
    bool __IsInShadowReceiverList(CGraphicObjectInstance *pkObjInstTest);
    bool __IsInPCBlockerList(CGraphicObjectInstance *pkObjInstTest);

    void ConvertToMapCoords(float fx, float fy, int *iCellX, int *iCellY, uint8_t *pucSubCellX, uint8_t *pucSubCellY,
                            uint16_t *pwTerrainNumX, uint16_t *pwTerrainNumY);

  public:
    bool GetAreaPointer(const uint8_t c_ucAreaNum, CArea **ppArea);
    void RenderArea(bool bRenderAmbience = false);
    void RenderBlendArea();
    void RenderEffect();
    void RenderPCBlocker();
    void RenderTree();

  public:
    //////////////////////////////////////////////////////////////////////////
    // For Grass
    //////////////////////////////////////////////////////////////////////////
    float GetHeight(float *pPos);
    bool GetBrushColor(float fX, float fY, float *pLowColor, float *pHighColor);
    std::optional<CArea *> GetAreaPointer(uint8_t c_byAreaNum);
    bool isAttrOn(float fX, float fY, uint8_t byAttr);
    bool GetAttr(float fX, float fY, uint8_t *pbyAttr);
    bool isAttrOn(int iX, int iY, uint8_t byAttr);
    bool GetAttr(int iX, int iY, uint8_t *pbyAttr);

    void SetMaterialDiffuse(float fr, float fg, float fb);
    void SetMaterialAmbient(float fr, float fg, float fb);

    bool GetTerrainNum(float fx, float fy, uint8_t *pbyTerrainNum);
    bool GetTerrainNumFromCoord(uint16_t wCoordX, uint16_t wCoordY, uint8_t *pbyTerrainNum);

  protected:
    //////////////////////////////////////////////////////////////////////////
    // New
    //////////////////////////////////////////////////////////////////////////
    long m_lCenterX, m_lCenterY;   // Terrain ÁÂÇ¥ ³»ÀÇ ¼¿ ÁÂÇ¥...
    long m_lOldReadX, m_lOldReadY; /* Last center */

    //////////////////////////////////////////////////////////////////////////
    // Octree
    //////////////////////////////////////////////////////////////////////////
    CTerrainQuadtreeNode *m_pRootNode;

    void BuildQuadTree();
    CTerrainQuadtreeNode *AllocQuadTreeNode(long x0, long y0, long x1, long y1);
    void SubDivideNode(CTerrainQuadtreeNode *Node);
    void UpdateQuadTreeHeights(CTerrainQuadtreeNode *Node);

    void FreeQuadTree();

  protected:
    std::vector<std::pair<float, long>> m_PatchVector;

    void NEW_DrawWireFrame(CTerrainPatchProxy *pTerrainPatchProxy, uint16_t wPrimitiveCount,
                           D3DPRIMITIVETYPE ePrimitiveType);

    void DrawWireFrame(long patchnum, uint16_t wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType);
    void DrawWater(long patchnum);

    bool m_bDrawWireFrame;
    bool m_bDrawShadow;
    bool m_bDrawChrShadow;

    //////////////////////////////////////////////////////////////////////////
    // Water
    void LoadWaterTexture();
    void UnloadWaterTexture();
    // Water
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Character Shadow
    IDirect3DTexture9* m_lpCharacterShadowMapTexture;
    LPDIRECT3DSURFACE9 m_lpCharacterShadowMapColorSurface;
    D3DVIEWPORT9 m_ShadowMapViewport;
    uint16_t m_wShadowMapSize;

    // Backup Device Context
    LPDIRECT3DSURFACE9 m_lpBackupRenderTargetSurface;
    LPDIRECT3DSURFACE9 m_lpBackupDepthSurface;
    D3DVIEWPORT9 m_BackupViewport;

    // Character Shadow
    //////////////////////////////////////////////////////////////////////////

    // View Frustum Culling
    Plane m_plane[6];

    void BuildViewFrustum(Matrix &mat);

    CTextureSet m_TextureSet;
    CTextureSet m_SnowTextureSet;

  protected:
    EnvironmentMap m_environments;

    const EnvironmentData *m_activeEnvironment;
    EnvironmentInstance m_envInstance;
    CSkyBox m_SkyBox;
    CLensFlare m_LensFlare;
    CScreenFilter m_ScreenFilter;

  protected:
    void SetIndexBuffer();
    void SelectIndexBuffer(uint8_t byLODLevel, uint16_t *pwPrimitiveCount, D3DPRIMITIVETYPE *pePrimitiveType);

    Matrix m_matWorldForCommonUse;
    Matrix m_matViewInverse;

    Matrix m_matSplatAlpha;
    Matrix m_matStaticShadow;
    Matrix m_matDynamicShadow;
    Matrix m_matDynamicShadowScale;
    Matrix m_matLightView;

    float m_fTerrainTexCoordBase;
    float m_fWaterTexCoordBase;

    float m_fXforDistanceCaculation, m_fYforDistanceCaculation;

  protected:
    typedef std::vector<std::unique_ptr<CTerrain>> TTerrainPtrVector;
    typedef std::vector<std::unique_ptr<CArea>> TAreaPtrVector;

    TTerrainPtrVector m_TerrainVector;
    TTerrainPtrVector m_TerrainDeleteVector;

    TAreaPtrVector m_AreaVector;
    TAreaPtrVector m_AreaDeleteVector;

    enum EDeleteDir
    {
        DELETE_LEFT,
        DELETE_RIGHT,
        DELETE_TOP,
        DELETE_BOTTOM,
    };

    template <class T> struct IsUsedSectorObject
    {
        EDeleteDir m_eLRDeleteDir;
        EDeleteDir m_eTBDeleteDir;
        TOutdoorMapCoordinate m_CurCoordinate;

        IsUsedSectorObject(EDeleteDir eLRDeleteDir, EDeleteDir eTBDeleteDir, TOutdoorMapCoordinate CurCoord)
        {
            m_eLRDeleteDir = eLRDeleteDir;
            m_eTBDeleteDir = eTBDeleteDir;
            m_CurCoordinate = CurCoord;
        }

        bool operator()(std::unique_ptr<T> &p);
    };

    template <class T> void PruneSectorObjectList(T &objects, T &delQueue, EDeleteDir lr, EDeleteDir tb);

  protected:
    void InitializeVisibleParts();
    bool IsVisiblePart(int ePart);

    float __GetNoFogDistance();
    float __GetFogDistance();

  protected:
    uint32_t m_dwVisiblePartFlags;

    int m_iRenderedSplatNumSqSum;
    int m_iRenderedSplatNum;
    int m_iRenderedPatchNum;
    std::vector<int> m_RenderedTextureNumVector;
    int m_iSplatLimit;

  protected:
    int m_iPatchTerrainVertexCount;
    int m_iPatchWaterVertexCount;

    int m_iPatchTerrainVertexSize;
    int m_iPatchWaterVertexSize;

    std::list<RECT> m_rkList_kGuildArea;

  protected:
    void __RenderTerrain_RecurseRenderQuadTree(CTerrainQuadtreeNode *Node, bool bCullCheckNeed = true);
    int __RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(const Vector3 &c_v3Center,
                                                                  const float &c_fRadius);

    void __RenderTerrain_AppendPatch(const Vector3 &c_rv3Center, float fDistance, long lPatchNum);

    void __RenderTerrain_RenderHardwareTransformPatch();

  protected:
    void __HardwareTransformPatch_RenderPatchSplat(long patchnum, uint16_t wPrimitiveCount,
                                                   D3DPRIMITIVETYPE ePrimitiveType);
    void __HardwareTransformPatch_RenderPatchNone(long patchnum, uint16_t wPrimitiveCount,
                                                  D3DPRIMITIVETYPE ePrimitiveType);

  protected:
    std::vector<CGraphicObjectInstance *> m_ShadowReceiverVector;
    std::vector<CGraphicObjectInstance *> m_PCBlockerVector;

  protected:
    float m_fOpaqueWaterDepth;
    CGraphicImageInstance m_WaterInstances[30];

  public:
    float GetOpaqueWaterDepth()
    {
        return m_fOpaqueWaterDepth;
    }
    void SetOpaqueWaterDepth(float fOpaqueWaterDepth)
    {
        m_fOpaqueWaterDepth = fOpaqueWaterDepth;
    }
    void SetTerrainRenderSort(ETerrainRenderSort eTerrainRenderSort)
    {
        m_eTerrainRenderSort = eTerrainRenderSort;
    }
    ETerrainRenderSort GetTerrainRenderSort()
    {
        return m_eTerrainRenderSort;
    }

  protected:
    ETerrainRenderSort m_eTerrainRenderSort;

  protected:
    CGraphicImageInstance m_attrImageInstance;
    CGraphicImageInstance m_BuildingTransparentImageInstance;
    Matrix m_matBuildingTransparent;

  public:
    void EnablePortal(bool bFlag);
    bool IsEnablePortal()
    {
        return m_bEnablePortal;
    }

    void SetTransparentTree(bool bTransparentTree)
    {
        m_bTransparentTree = bTransparentTree;
    }

  protected:
    Vector3 m_v3Player;

    bool m_bShowEntirePatchTextureCount;
    bool m_bTransparentTree;
    bool m_bEnablePortal;

    // XMas
  private:
    struct SXMasTree
    {
        CSpeedTreeWrapper *m_pkTree;
        int m_iEffectID;
    } m_kXMas;

    void __XMasTree_Initialize();
    void __XMasTree_Create(float x, float y, float z, const char *c_szTreeName, const char *c_szEffName);

  public:
    void XMasTree_Destroy();
    void XMasTree_Set(float x, float y, float z, const char *c_szTreeName, const char *c_szEffName);

    // Special Effect
  private:
    typedef std::unordered_map<uint32_t, int> TSpecialEffectMap;
    TSpecialEffectMap m_kMap_dwID_iEffectID;
    TSpecialEffectMap m_kMapShop_dwID_iEffectID;

  public:
    void SpecialEffect_Create(uint32_t dwID, float x, float y, float z, const char *c_szEffName);
    void SpecialEffect_Delete(uint32_t dwID);
    void SpecialEffect_Destroy();
    void SpecialEffectShopPos_Create(uint32_t dwID, float x, float y, float z, const char *c_szEffName);
    void SpecialEffectShopPos_Delete(uint32_t dwID);
    void SpecialEffectShopPos_Destroy();

  public:
    std::string &GetEnvironmentDataName(uint32_t idx);
    int32_t GetEnvironmentDataCount() const;

  protected:
    std::vector<std::string> m_setting_environmentFiles;
    std::vector<std::string> m_environmentFiles;


  private:
    TEMP_CAreaLoaderThread m_areaLoader;
};

template <class T> bool CMapOutdoor::IsUsedSectorObject<T>::operator()(std::unique_ptr<T> &p)
{
    uint16_t wReferenceCoordX = m_CurCoordinate.m_sTerrainCoordX;
    uint16_t wReferenceCoordY = m_CurCoordinate.m_sTerrainCoordY;

    uint16_t wCoordX, wCoordY;
    p->GetCoordinate(&wCoordX, &wCoordY);

    switch (m_eLRDeleteDir)
    {
    case DELETE_LEFT:
        if (wCoordX < wReferenceCoordX - LOAD_SIZE_WIDTH)
            return false;
        break;
    case DELETE_RIGHT:
        if (wCoordX > wReferenceCoordX + LOAD_SIZE_WIDTH)
            return false;
        break;
    }

    switch (m_eTBDeleteDir)
    {
    case DELETE_TOP:
        if (wCoordY < wReferenceCoordY - LOAD_SIZE_WIDTH)
            return false;
        break;
    case DELETE_BOTTOM:
        if (wCoordY > wReferenceCoordY + LOAD_SIZE_WIDTH)
            return false;
        break;
    }

    return true;
}
#endif
