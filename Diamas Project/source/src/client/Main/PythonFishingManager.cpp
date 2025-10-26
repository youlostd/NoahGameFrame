#include "PythonFishingManager.h"

#include "PythonApplication.h"
#include "PythonNetworkStream.h"
#include "base/Remotery.h"


#include <game/GamePacket.hpp>

float Distance(float x1, float y1, float x2, float y2)
{
    float v6; // [esp+4h] [ebp-8h]
    float v7; // [esp+8h] [ebp-4h]

    v6 = x1 - x2;
    v7 = y1 - y2;
    return (v6 * v6 + v7 * v7);
}

void FishingInfo::Initialize()
{
    wnd = nullptr;
    clientFish.x = 0.0f;
    clientFish.y = 0.0f;
    pos.x = 0.0f;
    pos.y = 0.0f;
    randPos.x = 0.0f;
    randPos.y = 0.0f;
    swimSpeed = 0.0f;
    fishLength = 0.0f;
    rotation = 0.0f;
    maxRotation = 45.0f;
    elapsedTime = 0.0f;
    state = 0;
    waveTime = 0.6f;
}

void FishingInfo::Animate(float x, float y)
{
    Vector2 v5;
    this->randPos.x = x;
    this->randPos.y = y;
    v5.x = this->randPos.x - this->clientFish.x;
    v5.y = this->randPos.y - this->clientFish.y;
    this->pos = v5;
    this->pos.Normalize();
    auto v6 = this->pos.x * 0.0 + this->pos.y * -1.0;
    auto v3 = acos(v6);
    this->rotation = v3 * 57.295776;
    if (this->pos.x < 0.0)
        this->rotation = 360.0 - this->rotation;

    wnd->SetOrigin(15.0, 15.0);
    wnd->SetRotation(rotation);
    wnd->SetScale((100 + fishLength) / 100.0f);
}

void FishingInfo::Update(float elapsed)
{
    if (state <= 0 || !wnd)
        return;
    elapsedTime += elapsed;
    switch (state)
    {
    case 1: {
        float halfWidth = wnd->GetWidth() * 0.5f;
        float halfHeight = wnd->GetHeight() * 0.5f;

        wnd->SetPosition(clientFish.x - halfWidth, clientFish.y - halfHeight);
        if (elapsedTime >= 0.5f)
        {
            auto rPos = CPythonFishingManager::instance().GetRandomPointInNavArea();
            Animate(rPos.x, rPos.y);
            state = 2;
            elapsedTime = 0.0f;
        }
        break;
    }
    case 2: {
        this->waveTime = this->waveTime - elapsed;

        this->clientFish.x = this->clientFish.x + elapsed * (this->swimSpeed) * this->pos.x;
        this->clientFish.y = this->clientFish.y + elapsed * (this->swimSpeed) * this->pos.y;

        auto newX = this->clientFish.x - this->wnd->GetWidth() * 0.5f;
        auto newY = this->clientFish.y - this->wnd->GetHeight() * 0.5f;

        wnd->SetPosition(newX, newY);
        if (MaxDistCheck())
        {
            state = 1;
            elapsedTime = 0.0f;
        }
        else if (waveTime <= 0.0f)
        {
            Wave();
            waveTime = 0.6f;
        }
        break;
    }
    case 3: {
        rotation = maxRotation * elapsedTime + rotation;
        rotation = static_cast<int>(rotation) % 360;
        wnd->SetRotation(rotation);
        break;
    }
    default:
        break;
    }
}

bool FishingInfo::MaxDistCheck() const
{
    return 3.5f * 3.5f >= Distance(clientFish.x, clientFish.y, randPos.x, randPos.y);
}

void FishingInfo::Wave()
{
    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "FishingWaveEffect",
                          static_cast<int>(clientFish.x - pos.x * 3.0f - 15.0f),
                          static_cast<int>(clientFish.y - pos.y * 3.0f - 15.0f), 5);

    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "FishingWaveEffect",
                          static_cast<int>(clientFish.x - pos.x * 10.0f - 15.0f),
                          static_cast<int>(clientFish.y - pos.y * 10.0f - 15.0f), 9);
}

CPythonFishingManager::CPythonFishingManager()
    : backgroundWater{nullptr}, navigationArea{nullptr}, goalCircle{nullptr}, timerGauge{nullptr}, hitCount{nullptr},
      hitCountText{nullptr}, debugText1{nullptr}, debugText2{nullptr}, debugText3{nullptr}, m_halfGoalCircleWidth{0},
      m_currentLimitTime{15.0f}, m_turnTime{0.5f}, m_maxLimitTime{15.0f}, m_permittedDeviation{11.0f},
      m_currentHitCount{0}, m_maxHitCount{0}, m_hitTimeLimit{1.0f}, m_hitFish{0}, m_fishingInfo{}, m_hitEffectTime{0},
      m_missEffectTime{0}, m_elapsedTime{0}, m_lastTime{0}, m_time{0}, m_state{0}
{
}

void CPythonFishingManager::Initialize()
{
    m_goalCircleCenterPos.x = 0.0f;
    m_goalCircleCenterPos.y = 0.0f;
    m_hitFish = 0;
    m_currentLimitTime = 15.0f;
    m_maxLimitTime = 15.0f;
    m_turnTime = 0.5f;
    m_fishingInfo.Initialize();
    m_elapsedTime = 0.0f;
    m_lastTime = 0.0f;
    m_time = 0.0f;
    m_state = 0;
    m_currentHitCount = 0;
    m_maxHitCount = 0;
    backgroundWater = nullptr;
    navigationArea = nullptr;
    goalCircle = nullptr;
    timerGauge = nullptr;
    hitCount = nullptr;
    hitCountText = nullptr;
    m_permittedDeviation = 11.0f;
    m_missEffectTime = 0.0f;
    m_hitEffectTime = 0.0f;
    m_hitTimeLimit = 1.0f;
}

void CPythonFishingManager::SetFishingState(int i)
{
    m_state = i;
    if (i)
    {
        if (i == 1)
        {
            m_fishingInfo.state = 1;
        }
        else if (i == 2)
        {
            m_fishingInfo.state = 3;
        }
        else
        {
            m_fishingInfo.state = 0;
            m_fishingInfo.Initialize();
        }
    }
    else
    {
        m_fishingInfo.state = 0;
    }
}

void CPythonFishingManager::Quit()
{
    SPDLOG_ERROR("Closing fishing window Quit");

    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "CloseFishingGameWindow");
    if (m_state == 1)
    {
        gPythonNetworkStream->SendFishingGamePacket(0, m_currentLimitTime);
    }
    else if (m_state == 2)
    {
        gPythonNetworkStream->SendFishingGamePacket(m_currentHitCount, m_currentLimitTime);
    }
    SetFishingState(0);
    Initialize();
}

void CPythonFishingManager::SetFishingInfoWindow(UI::CAniImageBox *wnd) { m_fishingInfo.wnd = wnd; }

bool CPythonFishingManager::HasHitFish() const
{
    auto v2 = m_fishingInfo.fishLength + m_halfGoalCircleWidth;
    return v2 * v2 >= Distance(m_fishingInfo.clientFish.x, m_fishingInfo.clientFish.y, m_goalCircleCenterPos.x,
                               m_goalCircleCenterPos.y);
}

bool CPythonFishingManager::FishingIsOnGoing()
{
    if (!m_state)
        return false;
    if (m_state == 1)
        return true;
    return m_state == 2;
}

void CPythonFishingManager::OnTimeOut()
{
    SPDLOG_ERROR("Timeout");
    SetFishingState(2);
    m_fishingInfo.state = 0;
    hitCountText->SetText(fmt::format("{} / {}", m_currentHitCount, m_maxHitCount));
    m_turnTime = 0.5f;
}

Vector2 CPythonFishingManager::GetRandomFishPos()
{
    Vector2 positions[8];
    Vector2 a2;

    if (navigationArea)
    {
        positions[0].x = navigationArea->GetPositionX();
        positions[0].y = navigationArea->GetPositionY();

        positions[1].x = navigationArea->GetPositionX();
        positions[1].y = navigationArea->GetHeight() * 0.5f + navigationArea->GetPositionY();

        positions[2].x = navigationArea->GetPositionX();
        positions[2].y = navigationArea->GetHeight() + navigationArea->GetPositionY();

        positions[3].x = navigationArea->GetWidth() * 0.5f + navigationArea->GetPositionX();
        positions[3].y = navigationArea->GetPositionY();

        positions[4].x = navigationArea->GetWidth() * 0.5f + navigationArea->GetPositionX();
        positions[4].y = (navigationArea->GetHeight() + navigationArea->GetPositionY());

        positions[5].x = (navigationArea->GetWidth() + navigationArea->GetPositionX());
        positions[5].y = navigationArea->GetPositionY();

        positions[6].x = (navigationArea->GetWidth() + navigationArea->GetPositionX());
        positions[6].y = navigationArea->GetHeight() * 0.5f + navigationArea->GetPositionY();

        positions[7].x = (navigationArea->GetWidth() + navigationArea->GetPositionX());
        positions[7].y = (navigationArea->GetHeight() + navigationArea->GetPositionY());
        const auto randIndex = Random::get(0, 7);
        return positions[randIndex];
    }

    return Vector2(0.0f, 0.0f);
}

Vector2 CPythonFishingManager::GetRandomPointInNavArea() const
{
    Vector2 result;
    result.x =
        Random::get<float>(navigationArea->GetPositionX(), navigationArea->GetWidth() + navigationArea->GetPositionX());
    result.y = Random::get<float>(navigationArea->GetPositionY(),
                                  navigationArea->GetHeight() + navigationArea->GetPositionY());

    return result;
}

bool CPythonFishingManager::PointIsInNavigationArea(float x, float y) const
{
    return x < navigationArea->GetPositionX() || x > (navigationArea->GetWidth() + navigationArea->GetPositionX()) ||
           y < navigationArea->GetPositionY() || y > (navigationArea->GetHeight() + navigationArea->GetPositionY());
}

void CPythonFishingManager::ShowMissEffect()
{
    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "FishingMissEffect", m_mouse.x, m_mouse.y);
    m_missEffectTime = 0.1f;
}

void CPythonFishingManager::ShowHitEffect()
{
    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "FishingHitEffect");
    m_hitEffectTime = 0.1f;
}

void CPythonFishingManager::MaxHitsReached()
{
    SetFishingState(2);
    m_fishingInfo.state = 0;
    hitCountText->SetText(fmt::format("{} / {}", m_currentHitCount, m_maxHitCount));
    m_turnTime = 0.5f;
}

void CPythonFishingManager::ReactFishing(int maxHits, int fishSpeed, int fishLength)
{
    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "OpenFishingGameWindow");
    m_maxHitCount = maxHits;
    m_fishingInfo.swimSpeed = fishSpeed;
    m_fishingInfo.fishLength = fishLength;
    const auto &pos = GetRandomFishPos();
    m_fishingInfo.clientFish = pos;
    SetFishingState(1);
}

void CPythonFishingManager::OnClickEvent()
{
    if (m_state != 1)
        return;

    const auto pos = (m_fishingInfo.fishLength );
    if ((pos * pos) < Distance(m_mouse.x, m_mouse.y, m_fishingInfo.clientFish.x, m_fishingInfo.clientFish.y))
    {
        m_currentLimitTime = m_currentLimitTime - 1.0f;
        ShowMissEffect();
        return;
    }

    if (m_hitTimeLimit < 1.0f)
        return;

    if (!m_hitFish)
    {
        m_currentLimitTime = m_currentLimitTime - 1.0f;
        ShowMissEffect();
    }
    else
    {
        m_hitTimeLimit = 0.0f;
        ++m_currentHitCount;
        ShowHitEffect();
        if (m_currentHitCount >= m_maxHitCount)
            MaxHitsReached();
    }
}

void CPythonFishingManager::Update()
{
              rmt_ScopedCPUSample(UpdateFishing, 0);

    if (!m_state)
        return;

    if (m_time == 0.0f)
    {
        m_time = DX::StepTimer::Instance().GetTotalSeconds();
        m_lastTime = m_time;
        m_currentLimitTime = 15.0f;
    }
    else
    {
        m_lastTime = m_time;
        m_time = DX::StepTimer::Instance().GetTotalSeconds();
    }

    m_elapsedTime = m_time - m_lastTime;

    switch (m_state)
    {
    case 1: {
        if (!navigationArea || !goalCircle || !timerGauge || !hitCount || !hitCountText)
        {
            Quit();
            return;
        }
        m_fishingInfo.Update(m_elapsedTime);
        if (PointIsInNavigationArea(m_fishingInfo.clientFish.x, m_fishingInfo.clientFish.y))
        {
            const auto &pos = GetRandomFishPos();
            m_fishingInfo.clientFish.x = pos.x;
            m_fishingInfo.clientFish.y = pos.y;
            m_fishingInfo.state = 1;
        }

        m_halfGoalCircleWidth = goalCircle->GetWidth() * 0.5f;

        m_goalCircleCenterPos.x = goalCircle->GetWidth() * 0.5f + goalCircle->GetPositionX();
        m_goalCircleCenterPos.y = goalCircle->GetWidth() * 0.5f + goalCircle->GetPositionY();

        auto [x, y] = UI::CWindowManager::Instance().GetMousePosition();

        backgroundWater->MakeLocalPosition(x, y);

        m_mouse.x = x;
        m_mouse.y = y;
        hitCountText->SetText(fmt::format("{} / {}", m_currentHitCount, m_maxHitCount));
        if (m_currentLimitTime < 0.0f)
        {
            OnTimeOut();
        }
        else
        {
            m_currentLimitTime -= m_elapsedTime;

            auto fRight = -1.0f + m_currentLimitTime / 15.0f;
            this->timerGauge->SetRenderingRect(0.0, 0.0, fRight, 0.0);

            if (m_currentLimitTime / 15.0f > 0.3f)
            {
                timerGauge->SetDiffuseColor(1.0, 1.0, 1.0, 1.0);
            }
            else
            {
                timerGauge->SetDiffuseColor(1.0, 0.0, 0.0, 1.0);
            }

            if (m_missEffectTime > 0.0f)
            {
                m_missEffectTime -= m_elapsedTime;
                timerGauge->SetDiffuseColor(1.0, 0.0, 0.0, 1.0);
            } else if(m_hitEffectTime > 0.0f) {
                m_hitEffectTime -= m_elapsedTime;
                timerGauge->SetDiffuseColor(0.0f, 0.2f, 0.5f, 1.0f);
            }


            m_hitFish = HasHitFish();
            if (m_hitFish && m_hitTimeLimit >= 1.0f)
                goalCircle->SetDiffuseColor(1.0f, 0.68f, 0.78f, 1.0);
            else
                goalCircle->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);

            m_hitTimeLimit += m_elapsedTime;
        }
        break;
    }
    case 2: {
        m_turnTime = m_turnTime - m_elapsedTime;
        if (!hitCount)
            return;
        if (m_hitEffectTime <= 0.0f)
        {
            hitCount->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            m_hitEffectTime = m_hitEffectTime - m_elapsedTime;
            hitCount->SetDiffuseColor(0.0f, 0.2f, 0.5f, 1.0f);
        }
        if (m_turnTime <= 0.0f)
        {
            gPythonNetworkStream->SendFishingGamePacket(m_currentHitCount, m_currentLimitTime);
            SetFishingState(0);
            SPDLOG_ERROR("Closing fishing window m_turnTime");
            PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(PHASE_GAME), "CloseFishingGameWindow");
            Initialize();
        }
        break;
    }
    default:
        break;
    }
}

void init_fishing(py::module &m)
{
    py::module fishing = m.def_submodule("fishing", "");

    auto cfm =
        py::class_<CPythonFishingManager, std::unique_ptr<CPythonFishingManager, py::nodelete>>(fishing, "fishingInst")
            .def(py::init([]() {
                     return std::unique_ptr<CPythonFishingManager, py::nodelete>(CPythonFishingManager::InstancePtr());
                 }),
                 pybind11::return_value_policy::reference_internal);
    cfm.def("OnClickEvent", &CPythonFishingManager::OnClickEvent);
    cfm.def("Quit", &CPythonFishingManager::Quit);

    cfm.def("SetBackgroundWaterWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetBackgroundWaterWindow(nullptr);
            return;
        }

        UI::CImageBox *wnd = (UI::CImageBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetBackgroundWaterWindow(wnd);
    });

    cfm.def("SetNavigationAreaWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetNavigationAreaWindow(nullptr);
            return;
        }
        UI::CBox *wnd = (UI::CBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetNavigationAreaWindow(wnd);
    });

    cfm.def("SetGoalCircleWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetGoalCircleWindow(nullptr);
            return;
        }
        UI::CImageBox *wnd = (UI::CImageBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetGoalCircleWindow(wnd);
    });

    cfm.def("SetTimeGaugeWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetTimeGaugeWindow(nullptr);
            return;
        }
        UI::CExpandedImageBox *wnd = (UI::CExpandedImageBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetTimeGaugeWindow(wnd);
    });

    cfm.def("SetTouchCountWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetTouchCountWindow(nullptr);
            return;
        }
        UI::CExpandedImageBox *wnd = (UI::CExpandedImageBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetTouchCountWindow(wnd);
    });

    cfm.def("SetTouchCountTextWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetTouchCountTextWindow(nullptr);
            return;
        }
        UI::CTextLine *wnd = (UI::CTextLine *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetTouchCountTextWindow(wnd);
    });

    cfm.def("SetDebugText1", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetDebugText1(nullptr);
            return;
        }
        UI::CTextLine *wnd = (UI::CTextLine *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetDebugText1(wnd);
    });

    cfm.def("SetDebugText2", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetDebugText2(nullptr);
            return;
        }
        UI::CTextLine *wnd = (UI::CTextLine *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetDebugText2(wnd);
    });

    cfm.def("SetDebugText3", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetDebugText3(nullptr);
            return;
        }
        UI::CTextLine *wnd = (UI::CTextLine *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetDebugText3(wnd);
    });

    cfm.def("SetFishWindow", [](CPythonFishingManager &a, py::object p) {
        if (py::isinstance<py::none>(p))
        {
            a.SetFishingInfoWindow(nullptr);
            return;
        }
        UI::CAniImageBox *wnd = (UI::CAniImageBox *)PyCapsule_GetPointer((PyObject *)p.ptr(), nullptr);
        if (!wnd)
            return;
        a.SetFishingInfoWindow(wnd);
    });
}
