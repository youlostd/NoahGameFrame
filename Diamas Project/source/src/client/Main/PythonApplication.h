#ifndef METIN2_CLIENT_MAIN_PYTHONAPPLICATION_H
#define METIN2_CLIENT_MAIN_PYTHONAPPLICATION_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EffectLib/EffectManager.h"
#include "../EterBase/StepTimer.h"
#include "../EterLib/FontManager.hpp"
#include "../eterLib/Engine.hpp"
#include "../eterLib/GrpDevice.h"
#include "../eterLib/GrpLightManager.h"
#include "../eterLib/MSApplication.h"
#include "../eterLib/Profiler.h"
#include "../gamelib/FlyingObjectManager.h"
#include "../gamelib/GameEventManager.h"
#include "../gamelib/ItemManager.h"
#include "../gamelib/RaceManager.h"
#include "../milesLib/SoundManager.h"
#include "../EterGrnLib/GrannyState.hpp"
#include "../gamelib/WikiManager.hpp"

#include "../EterLib/CharacterEffectRegistry.hpp"
#include "PythonChat.h"
#include "PythonEventManager.h"
#include "PythonExchange.h"
#include "PythonItem.h"
#include "PythonMiniMap.h"
#include "../GameLib/NpcManager.h"
#include "PythonPlayer.h"
#include "PythonShop.h"
#include "PythonSkill.h"
#include "../EterLib/Engine.hpp"
#include "PythonTextTail.h"

#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif

#include <boost/asio/io_service.hpp>
#include "../CWebBrowser/WebManager.h"
#include "../EterLib/RenderTargetManager.h"
#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "PythonCharacterManager.h"
#include "../GameLib/EmojiManager.h"
#include "PythonGuild.h"
#include "PythonItemRenderTargetManager.h"
#include "PythonMessenger.h"
#include "PythonMyShopDecoManager.h"
#include "PythonNetworkStream.h"
#include "PythonPrivateShopSearch.h"
#include "PythonQuest.h"
#include "PythonSafeBox.h"
#include "ServerStateChecker.h"
#ifdef ENABLE_ADMIN_MANAGER
#include "PythonAdmin.h"
#endif
#ifdef ENABLE_GUILD_STORAGE
#include "PythonGuildStorage.h"
#endif

#include "PythonWikiModelViewManager.h"

#include "AuthSocket.hpp"
#include "PythonCube.hpp"
#include "PythonDungeonInfo.h"
#include "PythonFishingManager.h"
#include "game/HuntingManager.hpp"

extern int isInputEnabled;

struct GlobalAppState
{
    ~GlobalAppState();
};

#ifdef __OFFLINE_SHOP__
#include "PythonOfflineShop.h"
#endif

class CPythonApplication : public CMSApplication
{
public:
    enum EDeviceState
    {
        DEVICE_STATE_FALSE,
        DEVICE_STATE_SKIP,
        DEVICE_STATE_OK,
    };

    enum ECursorMode
    {
        CURSOR_MODE_HARDWARE,
        CURSOR_MODE_SOFTWARE,
    };

    enum ECursorShape
    {
        CURSOR_SHAPE_NORMAL,
        CURSOR_SHAPE_ATTACK,
        CURSOR_SHAPE_TARGET,
        CURSOR_SHAPE_TALK,
        CURSOR_SHAPE_CANT_GO,
        CURSOR_SHAPE_PICK,

        CURSOR_SHAPE_DOOR,
        CURSOR_SHAPE_CHAIR,
        CURSOR_SHAPE_MAGIC,
        // Magic
        CURSOR_SHAPE_BUY,
        // Buy
        CURSOR_SHAPE_SELL,
        // Sell

        CURSOR_SHAPE_CAMERA_ROTATE,
        // Camera Rotate
        CURSOR_SHAPE_HSIZE,
        // Horizontal Size
        CURSOR_SHAPE_VSIZE,
        // Vertical Size
        CURSOR_SHAPE_HVSIZE,
        // Horizontal & Vertical Size
        CURSOR_SHAPE_FISHING,

        CURSOR_SHAPE_COUNT,

        // 안정적인 네이밍 변환을 위한 임시 enumerate
        NORMAL = CURSOR_SHAPE_NORMAL,
        ATTACK = CURSOR_SHAPE_ATTACK,
        TARGET = CURSOR_SHAPE_TARGET,
        CAMERA_ROTATE = CURSOR_SHAPE_CAMERA_ROTATE,
        CURSOR_COUNT = CURSOR_SHAPE_COUNT,
    };

    enum EInfo
    {
        INFO_ACTOR,
        INFO_EFFECT,
        INFO_ITEM,
        INFO_TEXTTAIL,
    };

    enum ECameraControlDirection
    {
        CAMERA_TO_POSITIVE = 1,
        CAMERA_TO_NEGITIVE = -1,
        CAMERA_STOP = 0,
    };

    enum
    {
        CAMERA_MODE_NORMAL = 0,
        CAMERA_MODE_STAND = 1,
        CAMERA_MODE_BLEND = 2,
        CAMERA_MODE_FREE = 3,
        EVENT_CAMERA_NUMBER = 101,
    };

    struct SCameraPos
    {
        float m_fUpDir;
        float m_fViewDir;
        float m_fCrossDir;

        SCameraPos()
            : m_fUpDir(0.0f)
              , m_fViewDir(0.0f)
              , m_fCrossDir(0.0f)
        {
        }
    };

    struct SCameraSetting
    {
        Vector3 v3CenterPosition;
        SCameraPos kCmrPos;
        float fRotation;
        float fPitch;
        float fZoom;

        SCameraSetting()
            : v3CenterPosition(0.0f, 0.0f, 0.0f)
              , fRotation(0.0f)
              , fPitch(0.0f)
              , fZoom(0.0f)
        {
        }
    };

    struct SCameraSpeed
    {
        float m_fUpDir;
        float m_fViewDir;
        float m_fCrossDir;

        SCameraSpeed()
            : m_fUpDir(0.0f)
              , m_fViewDir(0.0f)
              , m_fCrossDir(0.0f)
        {
        }
    };

public:
    CPythonApplication(boost::asio::io_service &ioService);
    virtual ~CPythonApplication();

public:
    void ShowWebPage(const char *c_szURL, const RECT &c_rcWebPage);
    void MoveWebPage(const RECT &c_rcWebPage);
    void HideWebPage();

    bool IsWebPageMode();

public:
    void NotifyHack(const char *c_szFormat, ...);
    void GetInfo(UINT eInfo, std::string *pstInfo);
    void GetMousePosition(POINT *ppt);

    static CPythonApplication &AppInst()
    {
        assert(ms_pInstance != NULL);
        return *ms_pInstance;
    }

    static CPythonApplication *InstancePtr()
    {
        assert(ms_pInstance != NULL);
        return ms_pInstance;
    }

    void Loop();
    void Destroy();
    void Clear();
    void FlashApplication();
    void Exit();
    void Abort();

    void SetMinFog(float fMinFog);
    void SetFrameSkip(bool isEnable);
    bool IsSkipRendering() const;
    void SkipRenderBuffering(uint32_t dwSleepMSec);
    void RenderImgUiWindows();

    bool Create(PyObject *poSelf, const char *c_szName, int width, int height, int Windowed);
    bool CreateDevice(int width, int height, int Windowed, int bit = 32, int frequency = 0);

    void UpdateGame();
    void RenderGame();

    bool Process();

    void UpdateClientRect();

    bool CreateCursors();
    void DestroyCursors();

    void SafeSetCapture();
    void SafeReleaseCapture();

    bool SetCursorNum(int iCursorNum);
    void SetCursorVisible(bool bFlag, bool bLiarCursorOn = false);
    bool GetCursorVisible();
    bool GetLiarCursorOn();
    void SetCursorMode(int iMode);
    int GetCursorMode();

    int GetCursorNum()
    {
        return m_iCursorNum;
    }

    void SetMouseHandler(PyObject *poMouseHandler);

    int GetWidth();
    int GetHeight();

    void SetCenterPosition(LONG x, LONG y);
    void SetCenterPosition(float fx, float fy, float fz);
    void GetCenterPosition(TPixelPosition *pPixelPosition);
    void SetCamera(float Distance, float Pitch, float Rotation, float fDestinationHeight);
    void GetCamera(float *Distance, float *Pitch, float *Rotation, float *DestinationHeight);
    void RotateCamera(int iDirection);
    void PitchCamera(int iDirection);
    void ZoomCamera(int iDirection);
    void MovieRotateCamera(int iDirection);
    void MoviePitchCamera(int iDirection);
    void MovieZoomCamera(int iDirection);
    void MovieResetCamera();
    void SetViewDirCameraSpeed(float fSpeed);
    void SetCrossDirCameraSpeed(float fSpeed);
    void SetUpDirCameraSpeed(float fSpeed);
    void TargetModelCamera();

    float GetRotation();
    float GetPitch();

    void SetFPS(int iFPS);
    void SetServerTime(time_t tTime);
    time_t GetServerTime();
    time_t GetServerTimeStamp();
    double GetGlobalTime();
    double GetGlobalElapsedTime();

    double GetFaceSpeed()
    {
        return m_fFaceSpd;
    }

    double GetAveRenderTime()
    {
        return m_fAveRenderTime;
    }

    uint32_t GetCurRenderTime()
    {
        return m_dwCurRenderTime;
    }

    uint32_t GetCurUpdateTime()
    {
        return m_dwCurUpdateTime;
    }

    uint32_t GetUpdateFPS()
    {
        return m_dwUpdateFPS;
    }

    uint32_t GetRenderFPS()
    {
        return m_dwRenderFPS;
    }

    uint32_t GetLoad()
    {
        return m_dwLoad;
    }

    uint32_t GetFaceCount()
    {
        return m_dwFaceCount;
    }

    void SetConnectData(const char *c_szIP, int iPort);
    void GetConnectData(std::string &rstIP, int &riPort);

    void RunPressExitKey();

    void EnableSpecialCameraMode();
    void SetCameraSpeed(int iPercentage);

    bool IsLockCurrentCamera();
    void SetEventCamera(const SCameraSetting &c_rCameraSetting);

    // Free camera [Think]
    void SetFreeCamera();
    void MoveFreeCamera(float fDirRot, bool isBackwards);
    void SetFreeCameraSpeed(int pct);
    // End

    void BlendEventCamera(const SCameraSetting &c_rCameraSetting, float fBlendTime);
    void SetDefaultCamera();

    void SetCameraSetting(const SCameraSetting &c_rCameraSetting);
    void GetCameraSetting(SCameraSetting *pCameraSetting);
    void SaveCameraSetting(const char *c_szFileName);
    bool LoadCameraSetting(const char *c_szFileName);

    void SetForceSightRange(int iRange);

    void UpdateEventCameraX(float fdx);
    void UpdateEventCameraY(float fdy);
    void UpdateEventCameraZ(float fdz);

    LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

    void OnCameraUpdate();

    void OnUIUpdate();
    void OnUIRender();

    void OnMouseUpdate();
    void OnMouseRender();

    void OnMouseWheel(int nLen);
    void OnMouseMove(int x, int y);
    void OnSuspending();
    void OnWindowMoved();
    void OnActivated();
    void OnDeactivated();
    void OnMouseMiddleButtonDown(int x, int y);
    void OnMouseMiddleButtonUp(int x, int y);
    void OnMouseLeftButtonDown(int x, int y);
    void OnMouseLeftButtonUp(int x, int y);
    void OnMouseLeftButtonDoubleClick(int x, int y);
    void OnMouseRightButtonDown(int x, int y);
    void OnMouseRightButtonUp(int x, int y);
    void OnSizeChange(int width, int height);
    void OnResuming();
    void OnChar(uint32_t ch);
    void OnKeyDown(KeyCode code);
    void OnKeyUp(KeyCode code);

    int CheckDeviceState();

    bool __IsContinuousChangeTypeCursor(int iCursorNum);

    void __UpdateCamera();

    void __SetFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP);
    void __MinimizeFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight);

    std::shared_ptr<CPythonNetworkStream> GetNetworkStream() const
    {
        return m_pyNetworkStream;
    }


protected:
        DX::StepTimer m_timer2;
    GlobalAppState m_globalState;
    boost::asio::io_service &m_ioService;

    CGraphicDevice m_grpDevice;
    CTimer m_timer;
    CResourceManager m_resManager;

    GrannyState m_grannyState;

    CPythonGraphic m_pyGraphic;

    CRenderTargetManager m_renderTargetManager;
    WebManager m_webManager;

    CLightManager m_LightManager;
    CSoundManager m_SoundManager;
    CFlyingManager m_FlyingManager;
    CRaceManager m_RaceManager;
    CGameEventManager m_GameEventManager;
    CItemManager m_kItemMgr;
#ifdef ENABLE_GUILD_STORAGE
    CPythonGuildStorage m_pyGuildStorage;
#endif
    CPythonCube m_cubeMgr;
    EmojiManager m_pythonEmoji;
    NpcManager m_pyNonPlayer;

    UI::CWindowManager m_kWndMgr;
    CEffectManager m_kEftMgr;
    CPythonCharacterManager m_kChrMgr;
    EngineObject<CharacterEffectRegistry> m_charEffectRegistry;


    // CPythonNetworkDatagram		m_pyNetworkDatagram;
    CPythonPlayer m_pyPlayer;
    CPythonItem m_pyItem;
    CPythonShop m_pyShop;
    CPythonExchange m_pyExchange;
    CPythonChat m_pyChat;
    CPythonTextTail m_pyTextTail;
    CPythonMiniMap m_pyMiniMap;
    CPythonEventManager m_pyEventManager;
    CPythonBackground m_pyBackground;
    CPythonSkill m_pySkill;
    CPythonQuest m_pyQuest;
    CPythonMessenger m_pyManager;
    CPythonSafeBox m_pySafeBox;
    CPythonPrivateShopSearch m_pyShopSearch;
    CPythonGuild m_pyGuild;
    CPythonMyShopDecoManager m_shopDecoManager;
    CPythonItemRenderTargetManager m_itemRenderTarget;
    CPythonWikiModelViewManager m_pyWikiModelViewManager;
    CPythonFishingManager m_fishingManager;
    HuntingManager m_huntingManager;
    CPythonDungeonInfo m_dungeonInfo;
    CGuildMarkManager m_kGuildMarkManager;

    std::shared_ptr<AuthSocket> m_authSocket;
    std::shared_ptr<CGuildMarkDownloader> m_kGuildMarkDownloader;
    std::shared_ptr<CGuildMarkUploader> m_kGuildMarkUploader;
    std::shared_ptr<CPythonNetworkStream> m_pyNetworkStream;

#ifdef ENABLE_ADMIN_MANAGER
    CPythonAdmin m_pyAdmin;
#endif
#ifdef __OFFLINE_SHOP__
    CPythonOfflineShop m_pyOfflineShop;
#endif
    WikiManager m_wikiManager;
    KeyboardInput m_keyboard;

    PyObject *m_poMouseHandler;
    Vector3 m_v3CenterPosition;

    uint32_t m_iFPS;
    double m_fAveRenderTime;
    uint64_t m_dwCurRenderTime;
    uint64_t m_dwCurUpdateTime;
    uint32_t m_dwLoad;
    uint32_t m_dwWidth;
    uint32_t m_dwHeight;

    // Time
    uint64_t m_dwStartLocalTime;
    time_t m_tServerTime;
    time_t m_tLocalStartTime;
    double m_fGlobalTime;
    double m_fGlobalElapsedTime;

    /////////////////////////////////////////////////////////////
    // Camera
    SCameraSetting m_DefaultCameraSetting;
    SCameraSetting m_kEventCameraSetting;

    int m_iCameraMode;
    float m_fBlendCameraStartTime;
    float m_fBlendCameraBlendTime;
    SCameraSetting m_kEndBlendCameraSetting;

    float m_fRotationSpeed;
    float m_fPitchSpeed;
    float m_fZoomSpeed;
    int m_freeCameraSpeedPct;

    float m_fCameraRotateSpeed;
    float m_fCameraPitchSpeed;
    float m_fCameraZoomSpeed;

    SCameraPos m_kCmrPos;
    SCameraSpeed m_kCmrSpd;

    bool m_isSpecialCameraMode;
    // Camera
    /////////////////////////////////////////////////////////////

    double m_fFaceSpd;
    uint32_t m_dwFaceSpdSum;
    uint32_t m_dwFaceSpdCount;

    uint32_t m_dwFaceAccCount;
    uint32_t m_dwFaceAccTime;

    uint32_t m_dwUpdateFPS;
    uint32_t m_dwRenderFPS;
    uint32_t m_dwFaceCount;

    uint32_t m_dwLButtonDownTime;
    uint32_t m_dwLButtonUpTime;

    typedef std::map<int, HANDLE> TCursorHandleMap;
    TCursorHandleMap m_CursorHandleMap;
    HANDLE m_hCurrentCursor;

    bool m_bCursorVisible;
    bool m_bLiarCursorOn;
    bool m_isWindowed;
    bool m_skipRendering;

    int m_iCursorMode;

    // Connect Data
    std::string m_strIP;
    int m_iPort;

    static CPythonApplication *ms_pInstance;

    bool m_isMinimizedWnd;
    bool m_isActivateWnd;
    bool m_isWindowFullScreenEnable;

    uint32_t m_dwStickyKeysFlag;
    uint32_t m_dwBufSleepSkipTime;
    int m_iForceSightRange;
#ifdef CAMY_MODULE
public:
    bool m_bCamyIsActive;
#endif
protected:
    int m_iCursorNum;
    int m_iContinuousCursorNum;

public:
    LPDIRECT3D9 GetDirectx8();
    LPDIRECT3DDEVICE9 GetDevice();
    bool MyShopDecoBGCreate();
};

#endif /* METIN2_CLIENT_MAIN_PYTHONAPPLICATION_H */
