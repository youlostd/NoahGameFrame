#ifndef METIN2_CLIENT_GAMELIB_MAPMANAGER_H
#define METIN2_CLIENT_GAMELIB_MAPMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"

class CMapBase;
#include "MapOutdoor.h"
#include "PropertyManager.h"

// VICTIM_COLLISION_TEST
#include "PhysicsObject.h"
// VICTIM_COLLISION_TEST_END
#include <unordered_map>
// Map Manager
class CMapManager : public CScreen, public IPhysicsWorld
{
  public:
    CMapManager();
    virtual ~CMapManager();

    bool IsMapOutdoor();
    CMapOutdoor &GetMapOutdoorRef();

    //////////////////////////////////////////////////////////////////////////
    // Contructor / Destructor
    //////////////////////////////////////////////////////////////////////////
    void Initialize();
    void Destroy();

    void Create(); // AllocMap ȣ�� �ؼ� m_pMap �� ����

    virtual void Clear();
    virtual CMapOutdoor *AllocMap();

    //////////////////////////////////////////////////////////////////////////
    // Map ���� �Լ�
    //////////////////////////////////////////////////////////////////////////
    bool IsMapReady();

    virtual bool LoadMap(const int mapIndex, const std::string &c_rstrMapName, float x, float y, float z);
    bool UnloadMap(const std::string c_strMapName);

    bool UpdateMap(float fx, float fy, float fz);
    void UpdateAroundAmbience(float fx, float fy, float fz);
    float GetHeight(float fx, float fy);
    float GetTerrainHeight(float fx, float fy);
    bool GetWaterHeight(int iX, int iY, long *plWaterHeight);

    bool GetNormal(int ix, int iy, Vector3 *pv3Normal);

    // Portal
    void RefreshPortal();
    void ClearPortal();
    void AddShowingPortalID(int iID);

    // External interface
    void LoadProperty();

    uint32_t GetShadowMapColor(float fx, float fy);

    // VICITM_COLLISION_TEST
    virtual bool isPhysicalCollision(const Vector3 &c_rvCheckPosition);
    // VICITM_COLLISION_TEST_END

    bool isAttrOn(float fX, float fY, uint8_t byAttr);
    bool GetAttr(float fX, float fY, uint8_t *pbyAttr);
    bool isAttrOn(int iX, int iY, uint8_t byAttr);
    bool GetAttr(int iX, int iY, uint8_t *pbyAttr);

    std::vector<int> &GetRenderedSplatNum(int *piPatch, int *piSplat, float *pfSplatRatio);

  protected:
    CPropertyManager m_PropertyManager;

    //////////////////////////////////////////////////////////////////////////
    // Map
    //////////////////////////////////////////////////////////////////////////
    CMapOutdoor *m_pkMap;
    CSpeedTreeForestDirectX9 m_Forest;
  public:
    void SetTerrainRenderSort(CMapOutdoor::ETerrainRenderSort eTerrainRenderSort);
    CMapOutdoor::ETerrainRenderSort GetTerrainRenderSort();

  public:
    void SetTransparentTree(bool bTransparenTree);

    struct SMapInfo
    {
        std::string mapName;
        std::string mapTranslation;
        std::string fieldMusicFile;
        int32_t sizeX{};
        int32_t sizeY{};
        float fieldMusicVolume = 1.0f;
    };
    using TMapInfo = SMapInfo;

  protected:
    std::unordered_map<uint32_t, TMapInfo> m_mapInfo;
    std::unordered_map<std::string, TMapInfo*> m_mapInfoByName;

    std::vector<std::string> m_mapNames;
    std::vector<std::string> m_noBoomMap;
    std::vector<std::string> m_noSnowMap;
    std::vector<std::string> m_noHorseSkillMap;
    std::vector<uint32_t> m_duelMapIndices;

  protected:
    void __LoadMapInfoVector();

  public:
    void SetXMasShowEvent(int iFlag);
    bool IsXmasShow();
    bool IsSnowModeOption();
    void SetSnowModeOption(bool enable);
    bool IsSnowTextureModeOption();
    void SetSnowTextureModeOption(bool enable);
    void SetNightOption(int iFlag);
    bool GetNightOption();
    void SetFogEnable(bool iFlag);
    bool IsFogEnabled();

  private:
    bool m_isFogEnabled;
    bool m_isXmasShow;
    bool m_isSnowModeOption;
    bool m_isSnowTextureModeOption;
    bool m_isNightModeOption;
};
#endif /* METIN2_CLIENT_GAMELIB_MAPMANAGER_H */
