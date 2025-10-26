#include "StdAfx.h"
#include "resource.h"

#include "../EterGrnLib/Material.h"
#include "../EterLib/Engine.hpp"
#include "../GameLib/FlyTrace.h"
#include "../GameLib/FlyingData.h"
#include "../GameLib/FlyingInstance.h"
#include "../GameLib/WeaponTrace.h"
#include "../eterlib/Camera.h"
#include <game/AffectConstants.hpp>

#include "PythonApplication.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include "PythonCharacterManager.h"
#include <utf8.h>
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include "../EterBase/StepTimer.h"
#include "../EterLib/CharacterEffectRegistry.hpp"
#include "base/Remotery.h"
float MIN_FOG = 2400.0f;
float g_specularSpd = 0.007f;
int isInputEnabled = 0;

CPythonApplication *CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

GlobalAppState::~GlobalAppState()
{
    DestroyCollisionInstanceSystem();

    CTextFileLoader::DestroySystem();
    CFlyingInstance::DestroySystem();
    CActorInstance::DestroySystem();

    CGraphicMarkInstance::DestroySystem();
    CGraphicThingInstance::DestroySystem();
    CGrannyModelInstance::DestroySystem();
    CEffectInstance::DestroySystem();
    CWeaponTrace::DestroySystem();
    CFlyTrace::DestroySystem();

    CEffectData::DestroySystem();
    CEffectMesh::SEffectMeshData::DestroySystem();
    NRaceData::DestroySystem();
}

CPythonApplication::CPythonApplication(boost::asio::io_service &ioService)
    : m_ioService(ioService), m_charEffectRegistry(gCharacterEffectRegistry),
      m_authSocket(std::make_shared<AuthSocket>(m_ioService)),
      m_kGuildMarkDownloader(std::make_shared<CGuildMarkDownloader>(m_ioService)),
      m_kGuildMarkUploader(std::make_shared<CGuildMarkUploader>(m_ioService)),
      m_pyNetworkStream(std::make_shared<CPythonNetworkStream>(m_ioService)), m_poMouseHandler(nullptr),
      m_fAveRenderTime(0.0f), m_fGlobalTime(0.0f), m_fGlobalElapsedTime(0.0f), m_dwUpdateFPS(0), m_dwRenderFPS(0),
      m_dwFaceCount(0), m_dwLButtonDownTime(0), m_bCursorVisible(true), m_bLiarCursorOn(false), m_isWindowed(false),
      m_skipRendering(false), m_iCursorMode(CURSOR_MODE_HARDWARE)
{
    Engine::GetSettings().LoadConfig();
    m_SoundManager.SetMusicVolume(Engine::Engine::GetSettings().GetConfig()->music_volume);
    m_pyBackground.SetNightOption(Engine::Engine::GetSettings().GetConfig()->bEnableNight);
    m_pyBackground.SetSnowModeOption(Engine::Engine::GetSettings().GetConfig()->bEnableSnow);
    m_pyBackground.SetSnowTextureModeOption(Engine::Engine::GetSettings().GetConfig()->bEnableSnowTexture);
    m_pyBackground.SetFogEnable(Engine::Engine::GetSettings().GetConfig()->bFogMode);

    SetKeyboardInput(&m_keyboard);

    gAuthSocket = m_authSocket;
    gPythonNetworkStream = m_pyNetworkStream;
    gGuildMarkUploader = m_kGuildMarkUploader;
    gGuildMarkDownloader = m_kGuildMarkDownloader;

    m_timer2.SetFixedTimeStep(true);
    m_timer2.SetTargetElapsedSeconds(1.0 / 60.0);

    CTimer::Instance().UseCustomTime();

    GrannyTimer::InitiateClocks();

    m_dwWidth = Engine::Engine::GetSettings().GetWidth();
    m_dwHeight = Engine::Engine::GetSettings().GetHeight();

    ms_pInstance = this;
    m_isWindowFullScreenEnable = FALSE;

    m_v3CenterPosition = Vector3(0.0f, 0.0f, 0.0f);
    m_dwStartLocalTime = ELTimer_GetMSec();
    m_tServerTime = 0;
    m_tLocalStartTime = 0;

    m_iPort = 0;
    m_iFPS = 60;

    m_isActivateWnd = false;
    m_isMinimizedWnd = true;

    m_fRotationSpeed = 0.0f;
    m_fPitchSpeed = 0.0f;
    m_fZoomSpeed = 0.0f;

    m_fFaceSpd = 0.0f;

    m_dwFaceAccCount = 0;
    m_dwFaceAccTime = 0;

    m_dwFaceSpdSum = 0;
    m_dwFaceSpdCount = 0;

    m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

    m_iCursorNum = CURSOR_SHAPE_NORMAL;
    m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

    m_isSpecialCameraMode = FALSE;
    m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
    m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
    m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

    m_iCameraMode = CAMERA_MODE_NORMAL;
    m_fBlendCameraStartTime = 0.0f;
    m_fBlendCameraBlendTime = 0.0f;

    m_iForceSightRange = -1;

    CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication() { Destroy(); }

void CPythonApplication::GetMousePosition(POINT *ppt) { CMSApplication::GetMousePosition(ppt); }

void CPythonApplication::SetMinFog(float fMinFog) { MIN_FOG = fMinFog; }

void CPythonApplication::SetFrameSkip(bool isEnable) { m_skipRendering = isEnable; }

bool CPythonApplication::IsSkipRendering() const { return m_skipRendering; }

void CPythonApplication::NotifyHack(const char *c_szFormat, ...)
{
    char szBuf[1024];

    va_list args;
    va_start(args, c_szFormat);
    _vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
    va_end(args);
    m_pyNetworkStream->NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string *pstInfo)
{
    switch (eInfo)
    {
    case INFO_ACTOR:
        m_kChrMgr.GetInfo(pstInfo);
        break;
    case INFO_EFFECT:
        m_kEftMgr.GetInfo(pstInfo);
        break;
    case INFO_ITEM:
        m_pyItem.GetInfo(pstInfo);
        break;
    case INFO_TEXTTAIL:
        m_pyTextTail.GetInfo(pstInfo);
        break;
    }
}

void CPythonApplication::Abort()
{
    SPDLOG_ERROR(
        "============================================================================================================");
    SPDLOG_ERROR("Abort!!!!\n\n");
    if (PyErr_Occurred())
        PyErr_Print();

    PyThreadState *ts = PyThreadState_Get();
    PyFrameObject *frame = ts->frame;
    while (frame != 0)
    {
        char const *filename = PyString_AsString(frame->f_code->co_filename);
        char const *name = PyString_AsString(frame->f_code->co_name);
        SPDLOG_ERROR("filename={0}, name={1}", filename, name);
        frame = frame->f_back;
    }

    PostQuitMessage(0);
}

void CPythonApplication::Exit() {
    SPDLOG_INFO(" CPythonApplication::Exit");
    PostQuitMessage(0);
}

void CPythonApplication::SkipRenderBuffering(uint32_t dwSleepMSec)
{
    m_dwBufSleepSkipTime = ELTimer_GetMSec() + dwSleepMSec;
}

void CPythonApplication::RenderImgUiWindows()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (Engine::GetSettings().IsShowFPS()) {
        ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(
            0.5f); // Set transparency level (0.0f to 1.0f)

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

        float fps = ImGui::GetIO().Framerate;
        ImVec2 textSize = ImGui::CalcTextSize("%.1f FPS", NULL);
        float windowWidth = textSize.x + 25.0f;

        ImGui::SetNextWindowSize(ImVec2(windowWidth, 20), ImGuiCond_Always);

        ImGui::Begin("FPS", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "%.1f FPS", fps);

        ImGui::End();

        ImGui::PopStyleVar(2);
    }

    ImGui::EndFrame();
}

bool CPythonApplication::Process()
{

    ELTimer_SetFrameMSec();




    // 	m_Profiler.Clear();
    rmt_BeginCPUSample(Render, 0);

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    m_dwUpdateFPS = m_timer2.GetFramesPerSecond();
    m_dwRenderFPS = m_timer2.GetFramesPerSecond();
    m_dwLoad = 0;

    CTimer &rkTimer = CTimer::Instance();
    rkTimer.Advance();

    m_fGlobalTime = m_timer2.GetTotalSeconds();
    m_fGlobalElapsedTime = m_timer2.GetElapsedSeconds();

    rmt_BeginCPUSample(Network, 0);
    m_ioService.poll();
    m_ioService.restart();
    rmt_EndCPUSample();
       // Mouse
        POINT Point;
        if (GetCursorPos(&Point))
        {
            ScreenToClient(m_hWnd, &Point);
            OnMouseMove(Point.x, Point.y);
        }

    GrannyTimer::UpdateClock();


    m_timer2.Tick([&]() {
 

        gPythonNetworkStream->Update();
        //!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
        // if (m_isActivateWnd
        rmt_BeginCPUSample(CameraUpdate, 0);
        __UpdateCamera();
        rmt_EndCPUSample();
        // Update Game Playing
        rmt_BeginCPUSample(ResourceUpdate, 0);
        CResourceManager::Instance().Update();
        rmt_EndCPUSample();

        rmt_BeginCPUSample(OnUpdateCamera, 0);
        OnCameraUpdate();
        rmt_EndCPUSample();

        OnMouseUpdate();
        if (m_pyNetworkStream->IsGamePhase())
        {
            rmt_BeginCPUSample(UpdateGame, 0);
            UpdateGame();
            rmt_EndCPUSample();
        }

        rmt_BeginCPUSample(UpdateUI, 0);
        OnUIUpdate();
        rmt_EndCPUSample();

        rmt_BeginCPUSample(Cull, 0);
        CCullingManager::Instance().Update();
        rmt_EndCPUSample();

        CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);
    });

    static UINT s_uiNextFrameTime = ELTimer_GetMSec();

    UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;	//17 - 1초당 60fps기준.
    DWORD dwCurrentTime = ELTimer_GetMSec();

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime;

		if (dt >= 500)
		{
			s_uiNextFrameTime += nAdjustTime;

		
			CTimer::Instance().Adjust(nAdjustTime);
		}

	}
    {
        bool canRender = m_timer2.GetFrameCount() != 0;

        if (m_isMinimizedWnd)
        {
            canRender = false;
        }
        else
        {
            if (DEVICE_STATE_OK != CheckDeviceState())
                canRender = false;
        }

        // 리스토어 처리때를 고려해 일정 시간동안은 버퍼링을 하지 않는다
        if (canRender)
        {
            Engine::GetDevice().Clear(true, true, false);
            if (m_pyGraphic.Begin())
            {
                // m_pyGraphic.ClearDepthBuffer();

                if (!m_skipRendering)
                {

#ifdef _DEBUG
                    m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
                    m_pyGraphic.Clear();
#endif

                    /////////////////////
                    // Interface
                    m_pyGraphic.SetInterfaceRenderState();

                    OnUIRender();
                    OnMouseRender();
                    /////////////////////
                }
                else
                {
                    m_pyGraphic.Clear();
                }

                RenderImgUiWindows();
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                m_pyGraphic.End();

                m_pyGraphic.Show();

                m_pyGraphic.ResetFaceCount();
            }
        }
    }
    rmt_EndCPUSample();

    // m_Profiler.ProfileByScreen();
    return true;
}

// Taken from: https://blogs.msdn.microsoft.com/oldnewthing/20060126-00/?p=32513/
bool SleepMsg(uint32_t timeout)
{
    static const uint32_t MSGF_SLEEPMSG = 0x5300;
    static const uint32_t MWFMO_WAITANY = 0;

    uint32_t start = ELTimer_GetMSec();
    uint32_t elapsed = 0;
    do
    {
        const auto status =
            MsgWaitForMultipleObjectsEx(0, NULL, timeout - elapsed, QS_ALLINPUT, MWFMO_WAITANY | MWMO_INPUTAVAILABLE);
        if (status != WAIT_OBJECT_0)
            continue;

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {

                PostQuitMessage((int)msg.wParam);
                return false; // abandoned due to WM_QUIT
            }

            if (!CallMsgFilter(&msg, MSGF_SLEEPMSG))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    } while ((elapsed = ELTimer_GetMSec() - start) < timeout);
    return true; // timed out
}

void CPythonApplication::Loop()
{
    MSG msg = {};
    PeekMessageW(&msg, nullptr, 0U, 0U, PM_NOREMOVE);

    while (WM_QUIT != msg.message)
    {
        // Use PeekMessage() so we can use idle time to render the scene.
        bool messageReceived = (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0);

        if (messageReceived)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
        {
            rmt_BeginCPUSample(ApplicationProcess, 0);
            Process();
            rmt_EndCPUSample();
        }
    }
}

bool PERF_CHECKER_RENDER_GAME = false;

void CPythonApplication::RenderGame()
{
    rmt_ScopedCPUSample(RenderGame, 0);

    rmt_BeginCPUSample(ItemRenderTargetBackground, 0);
    m_itemRenderTarget.RenderBackground();
    rmt_EndCPUSample();

    float fAspect = m_kWndMgr.GetAspect();
    float fFarClip = m_pyBackground.GetFarClip();

#ifdef ENABLE_PERSPECTIVE_VIEW
    m_pyGraphic.SetPerspective(
        ((Engine::GetSettings().GetFieldPerspective() / 100.0f) * 55.0f) +
            30.0f,
        fAspect,
        100.0f, fFarClip);
#else
    m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0f, fFarClip);
#endif

    CCullingManager::Instance().Process();

    m_kChrMgr.Deform();
    m_itemRenderTarget.Deform();
    m_pyWikiModelViewManager.DeformModel();

    CMapOutdoor *map = nullptr;
    if (m_pyBackground.IsMapReady())
        map = &m_pyBackground.GetMapOutdoorRef();

    if (map)
        m_pyBackground.RenderCharacterShadowToTexture();

    m_pyGraphic.SetGameRenderState();
    m_pyGraphic.PushState();

    {
        auto [lx, ly] = m_kWndMgr.GetMousePosition();
        m_pyGraphic.SetCursorPosition(lx, ly);
    }

    if (map)
    {
        map->RenderSky();
        map->RenderBeforeLensFlare();
        map->RenderCloud();

        map->GetActiveEnvironment().BeginPass();
        m_pyBackground.Render();

        m_pyBackground.SetCharacterDirLight();
    }

    m_kChrMgr.Render();
    m_pyWikiModelViewManager.RenderModel();

    if (map)
    {
        m_pyBackground.SetBackgroundDirLight();
        map->RenderWater();
        m_pyBackground.RenderSnow();
        map->RenderEffect();
        map->GetActiveEnvironment().EndPass();
    }
    m_itemRenderTarget.RenderModel();
    m_kEftMgr.Render();
    m_pyItem.Render();
    m_FlyingManager.Render();

    if (map)
    {
        rmt_ScopedCPUSample(RenderBlockersAndPostEffects, 0);

        map->GetActiveEnvironment().BeginPass();
        map->RenderPCBlocker();
        map->GetActiveEnvironment().EndPass();

        map->RenderAfterLensFlare();
        map->RenderScreenFiltering();
    }
}

void CPythonApplication::UpdateGame()
{
    rmt_ScopedCPUSample(UpdateGame, 0);

    POINT ptMouse;
    GetMousePosition(&ptMouse);

    //!@# Alt+Tab Áß SetTransfor ¿¡¼­ Æ¨±è Çö»ó ÇØ°áÀ» À§ÇØ - [levites]
    // if (m_isActivateWnd)
    {
        rmt_ScopedCPUSample(BuildViewFrustum, 0);
        CScreen s;
        float fAspect = UI::CWindowManager::Instance().GetAspect();
        float fFarClip = CPythonBackground::Instance().GetFarClip();



#ifdef ENABLE_PERSPECTIVE_VIEW
        s.SetPerspective(
            ((Engine::GetSettings().GetFieldPerspective() / 100.0f) * 55.0f) +
                30.0f,
            fAspect, 100.0f, fFarClip);
#else
        s.SetPerspective(30.0f, fAspect, 100.0f, fFarClip);
#endif
        s.BuildViewFrustum();
    }

    TPixelPosition kPPosMainActor;
    m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

    // This needs to happen before the call to CMapManager::Update(),
    // because it deletes the garbage actors from the last frame,
    // which could otherwise end up in the PC blocker/shadow receiver lists.
    rmt_BeginCPUSample(UpdateCharacterManager, 0);
    m_kChrMgr.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateShopDecoManager, 0);
    m_shopDecoManager.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateItemRenderTargets, 0);
    m_itemRenderTarget.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateWikiModels, 0);
    m_pyWikiModelViewManager.UpdateModel();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateBackground, 0);
    m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateGameEventManager, 0);
    m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
    m_GameEventManager.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateEffects, 0);
    m_kEftMgr.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateFlyManager, 0);
    m_FlyingManager.Update();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateItem, 0);
    m_pyItem.Update(ptMouse);
    rmt_EndCPUSample();

    rmt_BeginCPUSample(UpdateMainPlayer, 0);
    m_pyPlayer.Update();
    rmt_EndCPUSample();

 
    m_fishingManager.Update();

    // NOTE : Update µ¿¾È À§Ä¡ °ªÀÌ ¹Ù²î¹Ç·Î ´Ù½Ã ¾ò¾î ¿É´Ï´Ù - [levites]
    //        ÀÌ ºÎºÐ ¶§¹®¿¡ ¸ÞÀÎ ÄÉ¸¯ÅÍÀÇ Sound°¡ ÀÌÀü À§Ä¡¿¡¼­ ÇÃ·¹ÀÌ µÇ´Â Çö»óÀÌ ÀÖ¾úÀ½.
    m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
    SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

void CPythonApplication::UpdateClientRect()
{
    RECT rcApp;
    GetClientRect(&rcApp);
    OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject *poMouseHandler) { m_poMouseHandler = poMouseHandler; }

int CPythonApplication::CheckDeviceState()
{
    CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

    switch (e_deviceState)
    {
        // µð¹ÙÀÌ½º°¡ ¾øÀ¸¸é ÇÁ·Î±×·¥ÀÌ Á¾·á µÇ¾î¾ß ÇÑ´Ù.
    case CGraphicDevice::DEVICESTATE_NULL:
        return DEVICE_STATE_FALSE;

        // DEVICESTATE_BROKENÀÏ ¶§´Â ´ÙÀ½ ·çÇÁ¿¡¼­ º¹±¸ µÉ ¼ö ÀÖµµ·Ï ¸®ÅÏ ÇÑ´Ù.
        // ±×³É ÁøÇàÇÒ °æ¿ì DrawPrimitive °°Àº °ÍÀ» ÇÏ¸é ÇÁ·Î±×·¥ÀÌ ÅÍÁø´Ù.
    case CGraphicDevice::DEVICESTATE_BROKEN:
        return DEVICE_STATE_SKIP;

    case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
        m_pyBackground.ReleaseCharacterShadowTexture();

        SPDLOG_INFO("DEVICESTATE_NEEDS_RESET - attempting");

        if (!m_grpDevice.Reset())
            return DEVICE_STATE_SKIP;
        m_pyBackground.CreateCharacterShadowTexture();
        break;
    case CGraphicDevice::DEVICESTATE_OK:
        break;
    default:;
    }

    return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
    m_grpDevice.InitBackBufferCount(Windowed ? 2 : 1);

    auto iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit, frequency);


    switch (iRet)
    {
    case CGraphicDevice::CREATE_OK:
        Engine::RegisterDevice(&m_grpDevice);
        GrannyState::instance().InitGrannyShader();
        return true;

    case CGraphicDevice::CREATE_REFRESHRATE:
        return true;

    case CGraphicDevice::CREATE_ENUM:
    case CGraphicDevice::CREATE_DETECT:
        SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
        SPDLOG_ERROR("CreateDevice: Enum & Detect failed");
        return false;

    case CGraphicDevice::CREATE_NO_DIRECTX:
        // PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
        SET_EXCEPTION(CREATE_NO_DIRECTX);
        SPDLOG_ERROR("CreateDevice: DirectX 9 or greater required to run game");
        return false;

    case CGraphicDevice::CREATE_DEVICE:
        // PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
        SET_EXCEPTION(CREATE_DEVICE);
        SPDLOG_ERROR("CreateDevice: GraphicDevice create failed");
        return false;

    case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
        PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
        SPDLOG_ERROR("CreateDevice: GetDevCaps failed");
        return false;

    case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
        PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
        SPDLOG_ERROR("CreateDevice: GetDevCaps2 failed");
        return false;

    default:
        if (iRet & CGraphicDevice::CREATE_OK) {
            Engine::RegisterDevice(&m_grpDevice);
            GrannyState::instance().InitGrannyShader();
            return true;
        }

        SET_EXCEPTION(UNKNOWN_ERROR);
        SPDLOG_ERROR("CreateDevice: Unknown Error!");
        return false;
    }
}

// SUPPORT_NEW_KOREA_SERVER
bool LoadLocaleData(const char *localePath)
{
    NpcManager &rkNPCMgr = NpcManager::Instance();
    CPythonSkill &rkSkillMgr = CPythonSkill::Instance();
    auto rkNetStream = gPythonNetworkStream;
    auto &itemMgr = CItemManager::Instance();

    char path[256];

    if (!itemMgr.LoadItemDesc("locale/de/itemdesc.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - Failed to Load locale/de/itemdesc.txt");
    }

    if (!itemMgr.LoadItemDesc(fmt::format("{}/itemdesc.txt", localePath).c_str()))
    {
        SPDLOG_DEBUG("LoadLocaleData - Failed to load {}/itemdesc.txt", localePath);
    }

    if (!rkNPCMgr.LoadNames("locale/de/mob_names.txt"))
    {
        SPDLOG_ERROR("LoadLocaleData - Failed to load locale/de/mob_names.txt");
        return false;
    }

    if (!rkNPCMgr.LoadNames(fmt::format("{}/mob_names.txt", localePath).c_str()))
    {
    }

    if (!rkSkillMgr.RegisterSkillDesc(fmt::format("{}/SkillDesc.txt", localePath).c_str()))
    {
        SPDLOG_ERROR("RegisterSkillDesc({}/SkillDesc.txt) Error", localePath);
        return false;
    }

    if (!rkSkillMgr.RegisterSkillTable("data/skill_proto"))
    {
        return false;
    }

    if (!rkNetStream->LoadInsultList(fmt::format("{}/insult.txt", localePath).c_str()))
    {
        SPDLOG_DEBUG("LoadInsultList({}/insult.txt) failed", localePath);
    }

    snprintf(path, sizeof(path), "%s/guild_object_names.txt", localePath);
    CPythonGuild::Instance().LoadObjectNames(path);

    return true;
}

// END_OF_SUPPORT_NEW_KOREA_SERVER

unsigned __GetWindowMode(bool windowed)
{
    if (windowed)
        return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    return WS_POPUP;
}

float GetGamma(uint32_t grade)
{
    switch (grade)
    {
    case 0:
        return 0.4f;
    case 1:
        return 0.7f;
    case 2:
        return 1.0f;
    case 3:
        return 1.2f;
    case 4:
        return 1.4f;
    }

    return 1.0f;
}

bool CPythonApplication::Create(PyObject *poSelf, const char *c_szName, int width, int height, int Windowed)
{
    Windowed = Engine::Engine::GetSettings().IsWindowed() ? 1 : 0;

    bool bAnotherWindow = false;

    if (FindWindowA(NULL, c_szName))
        bAnotherWindow = true;

    m_dwWidth = width;
    m_dwHeight = height;

    std::string name = c_szName;
    auto end_it = utf8::find_invalid(name.begin(), name.end());

    std::wstring utf16line;
    utf8::utf8to16(name.begin(), end_it, back_inserter(utf16line));

    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    if (Windowed)
    {
        w = Engine::Engine::GetSettings().GetWidth();
        h = Engine::Engine::GetSettings().GetHeight();
    }

    if (!CMSWindow::Create(utf16line.c_str(), 4, CS_DBLCLKS, __GetWindowMode(Windowed),
                           ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2)), IDC_CURSOR_NORMAL))
    {
        // PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Create failed");
        SPDLOG_ERROR("CMSWindow::Create failed");
        SET_EXCEPTION(CREATE_WINDOW);
        return false;
    }

    AdjustSize(w, h);

    if (Windowed)
    {
        m_isWindowed = true;

        if (bAnotherWindow)
        {
            RECT rc;
            GetClientRect(&rc);

            int windowWidth = rc.right - rc.left;
            int windowHeight = (rc.bottom - rc.top);

            CMSApplication::SetPosition(GetScreenWidth() - windowWidth, GetScreenHeight() - 60 - windowHeight);
        }
    }
    else
    {
        m_isWindowed = false;
        SetPosition(0, 0);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Cursor
    if (!CreateCursors())
    {
        // PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Cursors Create Error");
        SPDLOG_ERROR("CMSWindow::Cursors Create Error");
        SET_EXCEPTION("CREATE_CURSOR");
        return false;
    }

    // Sound
    if (!m_SoundManager.Create())
    {
        // NOTE : 중국측의 요청으로 생략
        //		LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
    }

    Engine::GetFontManager().RegisterFont("font/2Dumb.ttf");
    Engine::GetFontManager().RegisterFont("font/3Dumb.ttf");
    Engine::GetFontManager().RegisterFont("font/arial.ttf");
    Engine::GetFontManager().RegisterFont("font/CamingoCode-Bold.ttf");
    Engine::GetFontManager().RegisterFont("font/CamingoCode-BoldItalic.ttf");
    Engine::GetFontManager().RegisterFont("font/CamingoCode-Italic.ttf");
    Engine::GetFontManager().RegisterFont("font/CamingoCode-Regular.ttf");
    Engine::GetFontManager().RegisterFont("font/Deutsch.ttf");
    Engine::GetFontManager().RegisterFont("font/Dosis-Bold.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-ExtraBold.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-ExtraLight.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-Light.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-Medium.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-Regular.otf");
    Engine::GetFontManager().RegisterFont("font/Dosis-SemiBold.otf");
    Engine::GetFontManager().RegisterFont("font/FontAwesome.otf");
    Engine::GetFontManager().RegisterFont("font/Immortal-Regular.ttf");
    Engine::GetFontManager().RegisterFont("font/Louis.ttf");
    Engine::GetFontManager().RegisterFont("font/msyi.ttf");
    Engine::GetFontManager().RegisterFont("font/NunitoSans-Bold.ttf");
    Engine::GetFontManager().RegisterFont("font/NunitoSans-Italic.ttf");
    Engine::GetFontManager().RegisterFont("font/NunitoSans-Regular.ttf");
    Engine::GetFontManager().RegisterFont("font/NunitoSans-SemiBold.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Bold.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Italic.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Light.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Medium.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Regular.ttf");
    Engine::GetFontManager().RegisterFont("font/Roboto-Thin.ttf");
    Engine::GetFontManager().RegisterFont("font/Tahoma-Regular.ttf");
    Engine::GetFontManager().RegisterFont("font/TrajanPro-Bold.otf");
    Engine::GetFontManager().RegisterFont("font/TrajanPro-Regular.otf");
    Engine::GetFontManager().RegisterFont("font/UnZialish.ttf");
    Engine::GetFontManager().RegisterFont("font/Verdana-Bold.ttf");
    Engine::GetFontManager().RegisterFont("font/Verdana-Italic.ttf");
    Engine::GetFontManager().RegisterFont("font/Verdana-Regular.ttf");

    // Device
    if (!CreateDevice(Engine::GetSettings().GetWidth(), Engine::GetSettings().GetHeight(), Engine::GetSettings().IsWindowed(),
                      Engine::GetSettings().GetBPP(), Engine::GetSettings().GetFrequency()))
        return false;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(GetWindowHandle());
    ImGui_ImplDX9_Init(Engine::GetDevice().GetDevice());

    Engine::Engine::GetSettings().CreateVideoModeList();

    SetVisibleMode(true);

    if (m_isWindowFullScreenEnable)
    {
        SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
    }

    m_pyGraphic.SetGamma(GetGamma(Engine::Engine::GetSettings().GetGamma()));

    // Mouse
    SetCursorMode(CURSOR_MODE_HARDWARE);

    m_pyItem.Create();

    // Other Modules
    CPythonTextTail::Instance().Initialize();

    // Light Manager
    m_LightManager.Initialize();

    m_kWndMgr.SetAppWindow(GetWindowHandle());

    if (!m_renderTargetManager.CreateRenderTarget(m_dwWidth, m_dwHeight))
        return false;

    if (!m_renderTargetManager.CreateRenderTargetWithIndex(m_dwWidth, m_dwHeight, 1))
        return false;

    CParticleInstance::Create();

    // 백업
    STICKYKEYS sStickKeys;
    memset(&sStickKeys, 0, sizeof(sStickKeys));
    sStickKeys.cbSize = sizeof(sStickKeys);
    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
    m_dwStickyKeysFlag = sStickKeys.dwFlags;

    // 설정
    sStickKeys.dwFlags &= ~(SKF_AVAILABLE | SKF_HOTKEYACTIVE);
    SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);

    // SphereMap
    CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
    CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
    return true;
}

void CPythonApplication::SetCenterPosition(LONG x, LONG y) { SetCenterPosition(x, y, m_pyBackground.GetHeight(x, y)); }

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
    m_v3CenterPosition.x = +fx;
    m_v3CenterPosition.y = -fy;
    m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition *pPixelPosition)
{
    pPixelPosition->x = +m_v3CenterPosition.x;
    pPixelPosition->y = -m_v3CenterPosition.y;
    pPixelPosition->z = +m_v3CenterPosition.z;
}

void CPythonApplication::SetServerTime(time_t tTime)
{
    m_dwStartLocalTime = ELTimer_GetMSec();
    m_tServerTime = tTime;
    m_tLocalStartTime = time(0);
}

time_t CPythonApplication::GetServerTime() { return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime; }

// 2005.03.28 - MALL ¾ÆÀÌÅÛ¿¡ µé¾îÀÖ´Â ½Ã°£ÀÇ ´ÜÀ§°¡ ¼­¹ö¿¡¼­ time(0) À¸·Î ¸¸µé¾îÁö´Â
//              °ªÀÌ±â ¶§¹®¿¡ ´ÜÀ§¸¦ ¸ÂÃß±â À§ÇØ ½Ã°£ °ü·Ã Ã³¸®¸¦ º°µµ·Î Ãß°¡
time_t CPythonApplication::GetServerTimeStamp() { return (time(0) - m_tLocalStartTime) + m_tServerTime; }

double CPythonApplication::GetGlobalTime() { return m_fGlobalTime; }

double CPythonApplication::GetGlobalElapsedTime() { return m_fGlobalElapsedTime; }

void CPythonApplication::SetFPS(int iFPS) { m_iFPS = iFPS; }

int CPythonApplication::GetWidth() { return m_dwWidth; }

int CPythonApplication::GetHeight() { return m_dwHeight; }

void CPythonApplication::SetConnectData(const char *c_szIP, int iPort)
{
    m_strIP = c_szIP;
    m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string &rstIP, int &riPort)
{
    rstIP = m_strIP;
    riPort = m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode() { m_isSpecialCameraMode = TRUE; }

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
    m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
    m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
    m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange) { m_iForceSightRange = iRange; }

void CPythonApplication::Clear() {}

void CPythonApplication::FlashApplication()
{
    HWND hWnd = GetWindowHandle();
    FLASHWINFO fi;
    fi.cbSize = sizeof(FLASHWINFO);
    fi.hwnd = hWnd;
    fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    fi.uCount = 0;
    fi.dwTimeout = 0;
    FlashWindowEx(&fi);
}

void CPythonApplication::Destroy()
{
    m_kWndMgr.Destroy();

    Engine::Engine::GetSettings().SaveConfig();

    m_renderTargetManager.Destroy();
    m_shopDecoManager.Destroy();
    m_itemRenderTarget.Destroy();

    m_pyEventManager.Destroy();

    m_pyMiniMap.Destroy();

    m_pyTextTail.Destroy();

    m_kChrMgr.Destroy();
    m_RaceManager.Destroy();

    m_pyItem.Destroy();
    m_kItemMgr.Destroy();

    m_pyBackground.Destroy();

    m_kEftMgr.Destroy();
    m_LightManager.Destroy();

    m_pyGraphic.Destroy();
    // m_pyNetworkDatagram.Destroy();

    CGrannyModelInstance::DestroySystem();
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    m_grpDevice.Destroy();

    // FIXME : ¸¸µé¾îÁ® ÀÖÁö ¾ÊÀ½ - [levites]
    // CSpeedTreeForest::Instance().Clear();

    CTextFileLoader::DestroySystem();
    DestroyCursors();

    CMSApplication::Destroy();

    STICKYKEYS sStickKeys;
    memset(&sStickKeys, 0, sizeof(sStickKeys));
    sStickKeys.cbSize = sizeof(sStickKeys);
    sStickKeys.dwFlags = m_dwStickyKeysFlag;
    SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);

    CGrannyMaterial::DestroySphereMap();

    gAuthSocket = nullptr;
    gPythonNetworkStream = nullptr;
    gGuildMarkUploader = nullptr;
    gGuildMarkDownloader = nullptr;

    SetKeyboardInput(nullptr);
    ms_pInstance = nullptr;

    WebManager::instance().DestroyAllWebViews();
}

//µð¹ÙÀÌ½º ¸®ÅÏ.
LPDIRECT3D9 CPythonApplication::GetDirectx8() { return CGraphicDevice::GetDirectx9(); }

LPDIRECT3DDEVICE9 CPythonApplication::GetDevice() { return CGraphicDevice::GetDevice(); }

bool CPythonApplication::MyShopDecoBGCreate()
{
    return m_shopDecoManager.CreateModelBackground(m_dwWidth, m_dwHeight) &&
           m_itemRenderTarget.CreateModelBackground(m_dwWidth, m_dwHeight);
}
