#ifndef METIN2_CLIENT_MAIN_PYTHONFISHINGMANAGER_H
#define METIN2_CLIENT_MAIN_PYTHONFISHINGMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterPythonLib/PythonWindow.h"


struct FishingInfo
{
    void Initialize();

    void Animate(float x, float y);

    void Update(float elapsed);

    bool MaxDistCheck() const;

    void Wave();

    UI::CAniImageBox *wnd;
    Vector2 clientFish;
    Vector2 pos;
    Vector2 randPos;
    float swimSpeed;
    float fishLength;
    float rotation;
    float maxRotation;
    float elapsedTime;
    uint8_t state;
    float waveTime;
};

class CPythonFishingManager : public CSingleton<CPythonFishingManager>
{

public:
  CPythonFishingManager();

  void Initialize();
  void SetFishingState(int i);
  void Quit();
  void SetFishingInfoWindow(UI::CAniImageBox *wnd);

  bool HasHitFish() const;
  bool FishingIsOnGoing();
  void OnTimeOut();
  Vector2 GetRandomFishPos();
  Vector2 GetRandomPointInNavArea() const;
  bool PointIsInNavigationArea(float x, float y) const;
  void ShowMissEffect();

  void ShowHitEffect();
  void MaxHitsReached();
  void ReactFishing(int maxHits, int x, int y);
  void OnClickEvent();
  void Update();

  void SetBackgroundWaterWindow(UI::CImageBox * bw) { backgroundWater = bw; }
  void SetNavigationAreaWindow(UI::CBox * bw) { navigationArea = bw; }
  void SetGoalCircleWindow(UI::CImageBox * bw) { goalCircle = bw; }

  void SetTimeGaugeWindow(UI::CExpandedImageBox * bw) { timerGauge = bw; }
  void SetTouchCountWindow(UI::CExpandedImageBox * bw) { hitCount = bw; }
  void SetTouchCountTextWindow(UI::CTextLine * bw) { hitCountText = bw; }

  void SetDebugText1(UI::CTextLine * bw) { debugText1 = bw; }
  void SetDebugText2(UI::CTextLine * bw) { debugText2 = bw; }
  void SetDebugText3(UI::CTextLine * bw) { debugText3 = bw; }


private:
    UI::CImageBox *backgroundWater;
    UI::CBox *navigationArea;
    UI::CImageBox *goalCircle;
    UI::CExpandedImageBox *timerGauge;
    UI::CExpandedImageBox *hitCount;
    UI::CTextLine *hitCountText;
    UI::CTextLine *debugText1;
    UI::CTextLine *debugText2;
    UI::CTextLine *debugText3;
    Vector2 m_goalCircleCenterPos;
    float m_halfGoalCircleWidth;
    float m_currentLimitTime;
    float m_turnTime;
    float m_maxLimitTime;
    Vector2 m_mouse;
    float m_permittedDeviation;
    uint32_t m_currentHitCount;
    uint32_t m_maxHitCount;
    float m_hitTimeLimit;
    uint8_t m_hitFish;
    FishingInfo m_fishingInfo;
    float m_hitEffectTime;
    float m_missEffectTime;
    float m_elapsedTime;
    float m_lastTime;
    float m_time;
    uint8_t m_state;
};

#endif
