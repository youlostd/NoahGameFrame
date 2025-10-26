#ifndef METIN2_CLIENT_MAIN_PYTHONBACKGROUND_H
#define METIN2_CLIENT_MAIN_PYTHONBACKGROUND_H

#include <Config.hpp>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../gamelib/MapManager.h"
#include "../gamelib/SnowEnvironment.h"
#include <optional>

class CInstanceBase;

class CPythonBackground : public CMapManager, public CSingleton<CPythonBackground>
{
public:
    enum
    {
        SHADOW_NONE,
        SHADOW_GROUND,
        SHADOW_GROUND_AND_SOLO,
        SHADOW_ALL,
        SHADOW_ALL_HIGH,
        SHADOW_ALL_MAX,
    };

    enum
    {
        DISTANCE0,
        DISTANCE1,
        DISTANCE2,
        DISTANCE3,
        DISTANCE4,
        NUM_DISTANCE_SET
    };

    enum
    {
        DAY_MODE_LIGHT = 0,
        DAY_MODE_DARK = 99,
    };

    typedef struct SVIEWDISTANCESET
    {
        float m_fFogStart;
        float m_fFogEnd;
        float m_fFarClip;
        Vector3 m_v3SkyBoxScale;
    } TVIEWDISTANCESET;

public:
    CPythonBackground();
    ~CPythonBackground();

    void Destroy();
    void Create();

    bool SetSplatLimit(int iSplatNum);
    bool SetVisiblePart(int ePart, bool isVisible);
    bool SetShadowLevel(int eLevel);
    void RefreshShadowLevel();
    void SelectViewDistanceNum(int eNum);
    void SetViewDistanceSet(int eNum, float fFarClip);
    float GetFarClip();

    uint32_t GetRenderShadowTime();
    void GetDistanceSetInfo(int *peNum, float *pfStart, float *pfEnd, float *pfFarClip);

    bool GetPickingPoint(Vector3 *v3IntersectPt);
    bool GetPickingPointWithRay(const CRay &rRay, Vector3 *v3IntersectPt);
    bool GetPickingPointWithRayOnlyTerrain(const CRay &rRay, Vector3 *v3IntersectPt);
    bool GetLightDirection(Vector3 &rv3LightDirection);

    void Update(float fCenterX, float fCenterY, float fCenterZ);

    void CreateCharacterShadowTexture();
    void ReleaseCharacterShadowTexture();
    void Render();
    void RenderSnow();
    void RenderPCBlocker();
    void RenderCollision();
    void RenderCharacterShadowToTexture();
    void RenderSky();
    void RenderCloud();
    void RenderWater();
    void RenderEffect();
    void RenderBeforeLensFlare();
    void RenderAfterLensFlare();

    bool CheckAdvancing(CInstanceBase *pInstance);

    void SetCharacterDirLight();
    void SetBackgroundDirLight();

    void ChangeToDay();
    void ChangeToNight();
    void EnableSnowEnvironment();
    void DisableSnowEnvironment();
    void SetXMaxTree(int iGrade);
    void CreatePrivateShopPos(uint32_t dwID, uint32_t dwChrVID);

    void ClearGuildArea();
    void RegisterGuildArea(int isx, int isy, int iex, int iey);

    void CreateTargetEffect(uint32_t dwID, uint32_t dwChrVID);
    void CreateTargetEffect(uint32_t dwID, long lx, long ly);
    void DeleteTargetEffect(uint32_t dwID);
    void DeletePrivateShopPos(uint32_t dwID);
    void CreateSpecialShopPos(uint32_t dwID, float fx, float fy, float fz, const char *c_szFileName);
    void DeleteSpecialShopPos(uint32_t dwID);

    void CreateSpecialEffect(uint32_t dwID, float fx, float fy, float fz, const char *c_szFileName);
    void DeleteSpecialEffect(uint32_t dwID);
    bool LoadMapConfg(const std::string &filename);

    bool IsNoSnowMap(const char *mapName);
    bool IsNoBoomMap(const char *mapName);
    bool IsDuelMap(uint32_t index);
    bool IsOnBlockedHorseSkillMap() const;
    void RefreshTexture(bool isSnowTextureModeOption);
    bool Warp(uint32_t mapIndex, uint32_t x, uint32_t y);

    void VisibleGuildArea();
    void DisableGuildArea();

    bool IsBlock(int x, int y);

    void                                       RegisterDungeonMapName(const char * c_szMapName);
    std::optional<CPythonBackground::TMapInfo> GetMapInfo(uint32_t mapIndex);
    std::optional<CPythonBackground::TMapInfo> GetMapInfoByName(const std::string& name);
    std::optional<CPythonBackground::TMapInfo> GetCurrentMapInfo();
    uint32_t                                   GetMapInfoCount();
    bool                                       IsMapInfoByName(const char * mapName);
    const char *                               GetWarpMapName() const;

    int GetDayMode()
    {
        return m_iDayMode;
    }

    uint32_t GetMapIndex();

protected:
    void __CreateProperty();
    bool __IsSame(std::unordered_set<int> &rleft, std::unordered_set<int> &rright);

protected:
    std::string m_strMapName;

private:
    CSnowEnvironment m_SnowEnvironment;

    int m_iDayMode;
    int m_iXMasTreeGrade;
    int m_eShadowLevel;
    int m_eViewDistanceNum;

    bool m_bVisibleGuildArea;

    uint32_t m_dwRenderShadowTime;

    TVIEWDISTANCESET m_ViewDistanceSet[NUM_DISTANCE_SET];

    std::unordered_set<int> m_kSet_iShowingPortalID;
    std::set<std::string> m_kSet_strDungeonMapName;
    std::map<uint32_t, uint32_t> m_kMap_dwTargetID_dwChrID;
    std::map<uint32_t, uint32_t> m_kMapShop_dwTargetID_dwChrID;

    struct SReserveTargetEffect
    {
        int ilx;
        int ily;
    };

    std::map<uint32_t, SReserveTargetEffect> m_kMap_dwID_kReserveTargetEffect;


};

#endif /* METIN2_CLIENT_MAIN_PYTHONBACKGROUND_H */
