#ifndef METIN2_CLIENT_GAMELIB_RACEDATA_H
#define METIN2_CLIENT_GAMELIB_RACEDATA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GameType.h"
#include "RaceMotionData.h"

#include "../EterGrnLib/Thing.h"
#include "../EterLib/AttributeData.h"

#include <game/MotionTypes.hpp>
#include <memory>
#include <unordered_map>

#define COMBO_KEY uint32_t
#define MAKE_COMBO_KEY(motion_mode, combo_type) ((uint32_t(motion_mode) << 16) | (uint32_t(combo_type)))
#define COMBO_KEY_GET_MOTION_MODE(key) (uint16_t(uint32_t(key) >> 16 & 0xFFFF))
#define COMBO_KEY_GET_COMBO_TYPE(key) (uint16_t(uint32_t(key) & 0xFFFF))

class CRaceData
{
  public:
    enum
    {
        SMOKE_NUM = 4,
    };

    /////////////////////////////////////////////////////////////////////////////////
    // Graphic Resource

    // Model
    using TGraphicThingMap = std::unordered_map<uint16_t, CGraphicThing::Ptr>;
    using TAttachingBoneNameMap = std::unordered_map<uint32_t, std::string>;
    using TRegisteredMotionCacheMap = std::unordered_map<uint32_t, std::string>;
    // Motion
    typedef struct SMotion
    {
        // TODO(tim): Remove this MSVC workaround!
        SMotion() = default;
        SMotion(const SMotion &) = delete;
        SMotion(SMotion &&o)
            : byPercentage(o.byPercentage), pMotion(std::move(o.pMotion)), pMotionData(std::move(o.pMotionData))
        {
            // ctor
        }

        uint8_t byPercentage;
        CGraphicThing::Ptr pMotion;
        std::unique_ptr<CRaceMotionData> pMotionData;
    } TMotion;

    typedef std::vector<TMotion> TMotionVector;
    typedef std::unordered_map<uint16_t, TMotionVector> TMotionVectorMap;

    typedef struct SMotionModeData
    {
        // TODO(tim): Remove this MSVC workaround!
        SMotionModeData() = default;
        SMotionModeData(SMotionModeData &&o)
            : wMotionModeIndex(o.wMotionModeIndex), MotionVectorMap(std::move(o.MotionVectorMap))
        {
            // ctor
        }

        uint16_t wMotionModeIndex;
        TMotionVectorMap MotionVectorMap;
    } TMotionModeData;

    typedef std::unordered_map<uint16_t, TMotionModeData> TMotionModeDataMap;
    typedef TMotionModeDataMap::iterator TMotionModeDataIterator;

    /////////////////////////////////////////////////////////////////////////////////
    // Combo Data
    typedef std::vector<uint32_t> TComboIndexVector;
    typedef struct SComboAttackData
    {
        TComboIndexVector ComboIndexVector;
    } TComboData;

    typedef std::unordered_map<uint32_t, uint32_t> TNormalAttackIndexMap;
    typedef std::unordered_map<COMBO_KEY, TComboData> TComboAttackDataMap;
    typedef TComboAttackDataMap::iterator TComboAttackDataIterator;

    struct SSkin
    {
        SSkin() : m_ePart(0)
        {
            // ctor
        }

        int m_ePart;

        std::string m_stSrcFileName;
        std::string m_stDstFileName;
    };

    struct SHair
    {
        std::string m_stModelFileName;
        std::vector<SSkin> m_kVct_kSkin;
    };

    struct SShape
    {
        SShape() : specular(0)
        {
            // ctor
        }

        float specular;
        std::string m_stModelFileName;
        std::vector<SSkin> m_kVct_kSkin;
        NRaceData::TAttachingDataVector m_attachingData;
    };

  public:
    CRaceData();

    // Codes For Client
    const char *GetBaseModelFileName() const;
    const char *GetAttributeFileName() const;
    const char *GetMotionListFileName() const;

    CGraphicThing::Ptr GetBaseModelThing();
    CAttributeData::Ptr GetAttributeDataPtr();

    bool CreateMotionModeIterator(TMotionModeDataIterator &itor);

    bool NextMotionModeIterator(TMotionModeDataIterator &itor);

    void LoadMotions();

    const TMotionModeDataMap &GetMotionModes() const;

    bool GetAttachingBoneName(uint32_t dwPartIndex, const char **c_pszBoneName);

    bool GetMotionKey(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint32_t *pMotionKey);
    bool GetMotionModeDataPointer(uint16_t wMotionMode, TMotionModeData **ppMotionModeData);
    bool GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, const TMotionVector **c_ppMotionVector);
    bool GetMotionDataPointer(uint16_t wMotionMode, uint16_t wMotionIndex, uint16_t wMotionSubIndex,
                              CRaceMotionData **ppMotionData);
    bool GetMotionDataPointer(uint32_t dwMotionKey, CRaceMotionData **ppMotionData);

    const NRaceData::TAttachingDataVector &GetAttachingData() const;

    bool IsTree();
    const char *GetTreeFileName();

    uint32_t GetRaceIndex() const
    {
        return m_dwRaceIndex;
    }

    ///////////////////////////////////////////////////////////////////
    // Setup by Script
    bool LoadRaceData(const char *c_szFileName);

    CGraphicThing::Ptr RegisterMotionData(uint16_t wMotionMode, uint16_t wMotionIndex, const std::string &c_szFileName,
                                          uint8_t byPercentage = 100);

    ///////////////////////////////////////////////////////////////////
    // Setup by Python
    void SetRace(uint32_t dwRaceIndex);
    void RegisterAttachingBoneName(uint32_t dwPartIndex, const char *c_szBoneName);

    void RegisterMotionMode(uint16_t wMotionModeIndex);
    void SetMotionModeParent(uint16_t wParentMotionModeIndex, uint16_t wMotionModeIndex);

    CGraphicThing::Ptr RegisterMotion(std::unique_ptr<CRaceMotionData> pkMotionData, uint16_t wMotionModeIndex,
                                      uint16_t wMotionIndex, const char *c_szFileName, uint8_t byPercentage = 100);

    bool SetMotionRandomWeight(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint16_t wMotionSubIndex,
                               uint8_t byPercentage);

    void RegisterNormalAttack(uint16_t wMotionModeIndex, uint16_t wMotionIndex);
    bool GetNormalAttackIndex(uint16_t wMotionModeIndex, uint16_t *pwMotionIndex);

    void ReserveComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboCount);
    void RegisterComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboIndex,
                             uint16_t wMotionIndex);
    bool GetComboDataPointer(uint16_t wMotionModeIndex, uint16_t wComboType, TComboData **ppComboData);

    void SetShapeModel(UINT eShape, const char *c_szModelFileName);
    void AppendShapeSkin(UINT eShape, UINT ePart, const char *c_szSrcFileName, const char *c_szDstFileName);

    void SetHairSkin(UINT eHair, UINT ePart, const char *c_szModelFileName, const char *c_szSrcFileName,
                     const char *c_szDstFileName);
  void SetHairModel(UINT eHair, const char* c_szModelFileName);

  /////

    uint32_t GetSmokeEffectID(UINT eSmoke);

    const std::string &GetSmokeBone() const;

    SHair *FindHair(UINT eHair);
    SShape *FindShape(UINT eShape);

  protected:
    bool GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, TMotionVector **ppMotionVector);
    bool m_motionsLoaded = false;
    uint32_t m_dwRaceIndex;
    uint32_t m_adwSmokeEffectID[SMOKE_NUM];

    CGraphicThing::Ptr m_pBaseModelThing;

    std::string m_strBaseModelFileName;
    std::string m_strTreeFileName;
    std::string m_strAttributeFileName;
    std::string m_strMotionListFileName;
    std::string m_strSmokeBoneName;

    TMotionModeDataMap m_pMotionModeDataMap;
    TAttachingBoneNameMap m_AttachingBoneNameMap;
    TComboAttackDataMap m_ComboAttackDataMap;
    TNormalAttackIndexMap m_NormalAttackIndexMap;

    std::unordered_map<uint32_t, SHair> m_kMap_dwHairKey_kHair;
    std::unordered_map<uint32_t, SShape> m_kMap_dwShapeKey_kShape;

    NRaceData::TAttachingDataVector m_AttachingDataVector;
    TRegisteredMotionCacheMap m_registeredMotionCache;
};
#endif /* METIN2_CLIENT_GAMELIB_RACEDATA_H */
