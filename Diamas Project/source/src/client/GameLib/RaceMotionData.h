#ifndef METIN2_CLIENT_GAMELIB_RACEMOTIONDATA_H
#define METIN2_CLIENT_GAMELIB_RACEMOTIONDATA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "RaceMotionDataEvent.h"
#include <Eigen/Core>
#include <Eigen/src/Core/util/ForwardDeclarations.h>

class CRaceMotionData
{
  public:
    typedef struct SComboInputData
    {
        float fInputStartTime;
        float fNextComboTime;
        float fInputEndTime;
    } TComboInputData;

    typedef NMotionEvent::SMotionEventData TMotionEventData;
    typedef NMotionEvent::SMotionEventDataScreenWaving TScreenWavingEventData;
    typedef NMotionEvent::SMotionEventDataScreenFlashing TScreenFlashingEventData;
    typedef NMotionEvent::SMotionEventDataEffect TMotionEffectEventData;
    typedef NMotionEvent::SMotionEventDataFly TMotionFlyEventData;
    typedef NMotionEvent::SMotionEventDataAttack TMotionAttackingEventData;
    typedef NMotionEvent::SMotionEventDataSound TMotionSoundEventData;
    typedef NMotionEvent::SMotionEventDataCharacterShow TMotionCharacterShowEventData;
    typedef NMotionEvent::SMotionEventDataCharacterHide TMotionCharacterHideEventData;
    typedef NMotionEvent::SMotionEventDataWarp TMotionWarpEventData;
    typedef NMotionEvent::SMotionEventDataEffectToTarget TMotionEffectToTargetEventData;
    typedef NMotionEvent::SMotionEventDataRelativeMoveOn TMotionRelativeMoveOnEventData;
    typedef NMotionEvent::SMotionEventDataRelativeMoveOff TMotionRelativeMoveOffEventData;

    typedef std::vector<std::unique_ptr<TMotionEventData>> TMotionEventDataVector;

  public:
    CRaceMotionData();

    void SetName(UINT eName);

    UINT GetType() const; // 모션 타입 얻기
    bool IsLock() const;  // 락된 모션 인가? (다른 모션에 캔슬되지 않음)

    int GetLoopCount() const;

    const char *GetMotionFileName() const;
    const char *GetSoundScriptFileName() const;

    void SetMotionDuration(float fDur);
    float GetMotionDuration();

    bool IsAccumulationMotion() const
    {
        return m_isAccumulationMotion;
    }
    void SetAccumulationPosition(Eigen::Vector3f c_rPos);
    const Eigen::Vector3f &GetAccumulationPosition() const
    {
        return m_accumulationPosition;
    }

    bool IsComboInputTimeData() const; // 콤보 입력 타이밍 데이터가 있는가?

    float GetComboInputStartTime() const;
    float GetNextComboTime() const;
    float GetComboInputEndTime() const;

    // Attacking
    bool isAttackingMotion() const;
    const NRaceData::TMotionAttackData *GetMotionAttackDataPointer() const;
    const NRaceData::TMotionAttackData &GetMotionAttackDataReference() const;
    bool HasSplashMotionEvent() const;

    // Skill
    bool IsCancelEnableSkill() const;

    // Loop
    bool IsLoopMotion() const;
    float GetLoopStartTime() const;
    float GetLoopEndTime() const;

    // Motion Event Data
    uint32_t GetMotionEventDataCount() const;
    bool GetMotionEventDataPointer(uint8_t byIndex, const CRaceMotionData::TMotionEventData **c_ppData) const;

    bool GetMotionAttackingEventDataPointer(
        uint8_t byIndex, const CRaceMotionData::TMotionAttackingEventData **c_ppMotionEventData) const;

    int GetEventType(uint32_t dwIndex) const;
    float GetEventStartTime(uint32_t dwIndex) const;

    // Sound Data
    const TSoundInstanceVector *GetSoundInstanceVectorPointer() const;

    // File
    bool SaveMotionData(const char *c_szFileName);

    bool LoadMotionData(const char *c_szFileName);
    bool LoadSoundScriptData(const char *c_szFileName);

  protected:
    void SetType(UINT eType);

    UINT m_eType;
    UINT m_eName;
    bool m_isLock = false;
    int m_iLoopCount = 0;

    std::string m_strMotionFileName;
    std::string m_strSoundScriptDataFileName;
    float m_fMotionDuration = 0.0f;

    bool m_isAccumulationMotion = false;
    Eigen::Vector3f m_accumulationPosition{0.0f, 0.0f, 0.0f};

    bool m_isComboMotion = false;
    TComboInputData m_ComboInputData{};

    bool m_isLoopMotion = false;
    float m_fLoopStartTime = 0.0f;
    float m_fLoopEndTime = 0.0f;

    bool m_isAttackingMotion = false;
    NRaceData::TMotionAttackData m_MotionAttackData{};

    bool m_bCancelEnableSkill = false;

    TMotionEventDataVector m_MotionEventDataVector;
    TSoundInstanceVector m_SoundInstanceVector;

    bool m_hasSplashEvent = false;
};
#endif /* METIN2_CLIENT_GAMELIB_RACEMOTIONDATA_H */
