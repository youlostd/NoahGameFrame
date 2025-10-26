#ifndef METIN2_CLIENT_GAMELIB_AREA_H
#define METIN2_CLIENT_GAMELIB_AREA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../SpeedTreeLib/SpeedTreeWrapper.h"
#include "../eterGrnLib/ThingInstance.h"
#include "MapType.h"
#include "DungeonBlock.h"

#include "../SpeedTreeLib/SpeedTreeWrapper.h"
#include "../EterGrnLib/ThingInstance.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <memory>
class CMapOutdoor;
class CEffectInstance;
class SoundSample;

class CArea
{
public:
    enum
    {
        ROTATION_STEP_COUNT = 24,
        ROTATION_STEP_AMOUNT = 360 / ROTATION_STEP_COUNT,
        YAW_STEP_COUNT = 24,
        YAW_STEP_AMOUNT = 360 / YAW_STEP_COUNT,
        PITCH_STEP_COUNT = YAW_STEP_COUNT,
        PITCH_STEP_AMOUNT = YAW_STEP_AMOUNT,
    };

    typedef Vector3 TObjectPosition;

    ///////////////////////////////////////////////////////////////////////////////////
    // Object Static Data
    typedef struct SObjectData
    {
        TObjectPosition Position;
        uint32_t dwCRC;
        uint8_t abyPortalID[PORTAL_ID_MAX_NUM];

        // For Tree
        //  - Nothing

        // For Granny Thing
        float m_fYaw;
        float m_fPitch;
        float m_fRoll;
        float m_fHeightBias;

        // For Effect
        //  - Nothing

        // For Ambience
        uint32_t dwRange;
        float fMaxVolumeAreaPercentage;

        void InitializeRotation();
    } TObjectData;

    struct ObjectDataComp
    {
        bool operator()(const TObjectData &lhs, const TObjectData &rhs) const
        {
            return lhs.dwCRC < rhs.dwCRC;
        }
    };

    typedef std::vector<TObjectData> TObjectDataVector;
    typedef TObjectDataVector::iterator TObjectDataIterator;

    ///////////////////////////////////////////////////////////////////////////////////
    // Object Dynamic Data
    typedef struct SAmbienceInstance : public CScreen
    {
        float fx, fy, fz;
        uint32_t dwRange;
        float fMaxVolumeAreaPercentage;
        std::unique_ptr<SoundSample> sample;
        float fNextPlayTime;
        TPropertyAmbience AmbienceData;

        SAmbienceInstance();

        virtual ~SAmbienceInstance()
        {
        }

        void __Update(float fxCenter, float fyCenter, float fzCenter);
        void (SAmbienceInstance::*Update)(float fxCenter, float fyCenter, float fzCenter);
        void UpdateOnceSound(float fxCenter, float fyCenter, float fzCenter);
        void UpdateStepSound(float fxCenter, float fyCenter, float fzCenter);
        void UpdateLoopSound(float fxCenter, float fyCenter, float fzCenter);
        void Render();
        bool Picking();
        float __GetVolumeFromDistance(float fDistance);
    } TAmbienceInstance;

    typedef struct SObjectInstance
    {
        uint32_t dwType;

        CMapOutdoor* map;

        // Data For Tree
        CSpeedTreeWrapper *pTree;

        // Data For Normal Object or Building
        bool isShadowFlag;
        CGraphicThingInstance *pThingInstance;
        std::unique_ptr<CAttributeInstance> pAttributeInstance;

        // Data for Effect
        CEffectInstance *effect;

        // Data For Ambience
        std::unique_ptr<TAmbienceInstance> pAmbienceInstance;

        // Dungeon
        std::unique_ptr<CDungeonBlock> pDungeonBlock;

        void SetTree(const Vector3 &pos, const Quaternion &rot, uint32_t dwTreeCRC, const
                     char *c_szTreeName);

        SObjectInstance()
            : dwType(PROPERTY_TYPE_NONE)
              , pTree(nullptr)
              , isShadowFlag(false)
              , pThingInstance(nullptr)
              , effect(nullptr)
        {
            // ctor
        }

        ~SObjectInstance();

        SObjectInstance(const SObjectInstance &) = delete;
        SObjectInstance(SObjectInstance &&other);

        SObjectInstance &operator=(const SObjectInstance &) = delete;
        SObjectInstance &operator=(SObjectInstance &&other);
    } TObjectInstance;

    typedef std::vector<CGraphicThingInstance *> TThingInstanceVector;
    typedef std::vector<CDungeonBlock *> TDungeonBlockInstanceVector;
    typedef std::vector<TAmbienceInstance *> TAmbienceInstanceVector;
    typedef std::unordered_set<int> TShowingPortalIDSet;

public:
    CArea();
    virtual ~CArea();

    void CollectRenderingObject(std::vector<CGraphicThingInstance *> &rkVct_pkOpaqueThingInst);
    void CollectBlendRenderingObject(std::vector<CGraphicThingInstance *> &rkVct_pkBlendThingInst);

    void SetMapOutDoor(CMapOutdoor *pOwnerOutdoorMap);
    void Clear();

    bool Load(const char *c_szPathName);

    uint32_t GetObjectDataCount();
    bool GetObjectDataPointer(uint32_t dwIndex, const TObjectData **ppObjectData) const;

    uint32_t GetObjectInstanceCount() const;
    const TObjectInstance *GetObjectInstancePointer(const uint32_t &dwIndex) const;

    void EnablePortal(bool bFlag = true);
    void ClearPortal();
    void AddShowingPortalID(int iNum);
    void RefreshPortal();

    //////////////////////////////////////////////////////////////////////////

    void Update();
    void UpdateAroundAmbience(float fX, float fY, float fZ);

    void Render();
    void RenderEffect();
    void RenderCollision();
    void RenderAmbience();
    void RenderDungeon();
    void RenderBoundingSpheres();

    void Refresh();

    //////////////////////////////////////////////////////////////////////////

    void SetCoordinate(const uint16_t &c_usCoordX, const uint16_t &c_usCoordY);
    void GetCoordinate(uint16_t *usCoordX, uint16_t *usCoordY);

    //////////////////////////////////////////////////////////////////////////

    CMapOutdoor *GetOwner()
    {
        return m_pOwnerOutdoorMap;
    }

protected:

    bool CheckObjectIndex(uint32_t dwIndex) const;

    bool __Load_LoadObject(const char *c_szFileName);
    bool __Load_LoadAmbience(const char *c_szFileName);
    void __Load_BuildObjectInstances();

    void __UpdateAniThingList();
    void __UpdateEffectList();

    void __LoadAttribute(TObjectInstance *pObjectInstance, const char *c_szAttributeFileName);

protected:
    void __SetObjectInstance(TObjectInstance *pObjectInstance, const TObjectData *c_pData);
    void __SetObjectInstance_SetTree(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                     CProperty *pProperty);
    void __SetObjectInstance_SetBuilding(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                         CProperty *pProperty);
    void __SetObjectInstance_SetEffect(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                       CProperty *pProperty);
    void __SetObjectInstance_SetAmbience(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                         CProperty *pProperty);
    void __SetObjectInstance_SetDungeonBlock(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                             CProperty *pProperty);

protected:
    // Static Data
    TObjectDataVector m_ObjectDataVector; // Area 상에 있는 오브젝트들

    // Rendering Instances
    std::vector<TObjectInstance> m_ObjectInstanceVector;

    // Clone Instance Vector
    TThingInstanceVector m_ThingCloneInstaceVector;
    TDungeonBlockInstanceVector m_DungeonBlockCloneInstanceVector;
    TThingInstanceVector m_AniThingCloneInstanceVector;
    TThingInstanceVector m_ShadowThingCloneInstaceVector;
    TAmbienceInstanceVector m_AmbienceCloneInstanceVector;
    std::unordered_map<TObjectInstance *, CEffectInstance *> m_effectInstances;

    // 좌표 : 심리스 맵 전체 중에서의 좌표... 둘다 0~999
    uint16_t m_wX;
    uint16_t m_wY;

    // Owner COutdoorMap poineter
    CMapOutdoor *m_pOwnerOutdoorMap;

protected:
    bool m_bPortalEnable;
    TShowingPortalIDSet m_kSet_ShowingPortalID;
};
#endif /* METIN2_CLIENT_GAMELIB_AREA_H */
