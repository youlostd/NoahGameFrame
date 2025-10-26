#include "../EterLib/Camera.h"
#include "PythonApplication.h"
#include "PythonBindings.h"
#include "StdAfx.h"
#include <ImageHlp.h>
#include <game/AffectConstants.hpp>
#include <utf8.h>
#include "GameData.hpp"
#include "pak/Util.hpp"

#include <tea.h>

extern bool PERF_CHECKER_RENDER_GAME;
extern DirectX::SimpleMath::Color g_fSpecularColor;
BOOL bTestServerFlag = FALSE;

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = 0;

#ifdef USE_OPENID
extern int openid_test;
#endif

PyObject* appLoadLocaleAddr(PyObject* poSelf, PyObject* poArgs)
{
	std::string addrPath;
	if (!PyTuple_GetString(poArgs, 0, addrPath))
		return Py_BuildException();

	FILE* fp = fopen(addrPath.c_str(), "rb");
	if (!fp)
		return Py_BuildException();

	fseek(fp, 0, SEEK_END);

	int size = ftell(fp);
	char* enc = (char*)_alloca(size);
	fseek(fp, 0, SEEK_SET);
	fread(enc, size, 1, fp);
	fclose(fp);

	static const unsigned char key[16] = {
		0x82, 0x1b, 0x34, 0xae,
		0x12, 0x3b, 0xfb, 0x17,
		0xd7, 0x2c, 0x39, 0xae,
		0x41, 0x98, 0xf1, 0x63
	};

	char* buf = (char*)_alloca(size);
	//int decSize = 
	tea_decrypt((uint32_t*)buf, (const uint32_t*)enc, (const uint32_t*)key, size);
	unsigned int retSize = *(unsigned int*)buf;
	char* ret = buf + sizeof(unsigned int);
	return Py_BuildValue("s#", ret, retSize);
}


PyObject *appShowWebPage(PyObject *poSelf, PyObject *poArgs)
{
    std::string szWebPage;
    if (!PyTuple_GetString(poArgs, 0, szWebPage))
        return Py_BuildException();

    PyObject *poRect = PyTuple_GetItem(poArgs, 1);
    if (!PyTuple_Check(poRect))
        return Py_BuildException();

    RECT rcWebPage;
    rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
    rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
    rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
    rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

    CPythonApplication::AppInst().ShowWebPage(szWebPage.c_str(), rcWebPage);
    Py_RETURN_NONE;
}

PyObject *appMoveWebPage(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poRect = PyTuple_GetItem(poArgs, 0);
    if (!PyTuple_Check(poRect))
        return Py_BuildException();

    RECT rcWebPage;
    rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
    rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
    rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
    rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

    CPythonApplication::AppInst().MoveWebPage(rcWebPage);
    Py_RETURN_NONE;
}

PyObject *appHideWebPage(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().HideWebPage();
    Py_RETURN_NONE;
}

PyObject *appIsWebPageMode(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().IsWebPageMode());
}

PyObject *appEnablePerformanceTime(PyObject *poSelf, PyObject *poArgs)
{
    std::string szMode;
    if (!PyTuple_GetString(poArgs, 0, szMode))
        return Py_BuildException();

    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 1, &nEnable))
        return Py_BuildException();

    bool isEnable = nEnable ? true : false;

    if (szMode == "RENDER_GAME")
        PERF_CHECKER_RENDER_GAME = isEnable;

    Py_RETURN_NONE;
}

/////////////////////////////////////////////////////

extern float g_specularSpd;

// TEXTTAIL_LIVINGTIME_CONTROL
extern void TextTail_SetLivingTime(long livingTime);

PyObject *appSetTextTailLivingTime(PyObject *poSelf, PyObject *poArgs)
{
    float livingTime;
    if (!PyTuple_GetFloat(poArgs, 0, &livingTime))
        return Py_BuildException();

    TextTail_SetLivingTime(livingTime * 1000);

    Py_RETURN_NONE;
}

// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

PyObject *appSetHairColorEnable(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BuildException();

    Py_RETURN_NONE;
}

PyObject *appSetArmorSpecularEnable(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BuildException();

    Py_RETURN_NONE;
}

PyObject *appSetWeaponSpecularEnable(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BuildException();

    Py_RETURN_NONE;
}

PyObject *appSetSkillEffectUpgradeEnable(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BuildException();

    Py_RETURN_NONE;
}

PyObject *SetTwoHandedWeaponAttSpeedDecreaseValue(PyObject *poSelf, PyObject *poArgs)
{
    int iValue;
    if (!PyTuple_GetInteger(poArgs, 0, &iValue))
        return Py_BuildException();

    TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = iValue;

    Py_RETURN_NONE;
}

PyObject *appSetRideHorseEnable(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BuildException();

    Py_RETURN_NONE;
}

PyObject *appSetCameraMaxDistance(PyObject *poSelf, PyObject *poArgs)
{
    float fMax;
    if (!PyTuple_GetFloat(poArgs, 0, &fMax))
        return Py_BuildException();

    CCamera::SetCameraMaxDistance(fMax);
    Py_RETURN_NONE;
}

PyObject *appSetSpecularSpeed(PyObject *poSelf, PyObject *poArgs)
{
    float fSpeed;
    if (!PyTuple_GetFloat(poArgs, 0, &fSpeed))
        return Py_BuildException();

    g_specularSpd = fSpeed;

    Py_RETURN_NONE;
}

PyObject *appSetMinFog(PyObject *poSelf, PyObject *poArgs)
{
    float fMinFog;
    if (!PyTuple_GetFloat(poArgs, 0, &fMinFog))
        return Py_BuildException();

    CPythonApplication::AppInst().SetMinFog(fMinFog);
    Py_RETURN_NONE;
}

PyObject *appSetFrameSkip(PyObject *poSelf, PyObject *poArgs)
{
    int nFrameSkip;
    if (!PyTuple_GetInteger(poArgs, 0, &nFrameSkip))
        return Py_BuildException();


    CPythonApplication::AppInst().SetFrameSkip(nFrameSkip ? true : false);
    Py_RETURN_NONE;
}

// LOCALE

PyObject *appForceSetLocale(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    std::string szLocalePath;
    if (!PyTuple_GetString(poArgs, 1, szLocalePath))
        return Py_BuildException();

    LocaleService_ForceSetLocale(szName.c_str(), szLocalePath.c_str());

    Py_RETURN_NONE;
}

PyObject *appGetLocaleServiceName(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("s", LocaleService_GetName());
}

//
bool LoadLocaleData(const char *localePath);

PyObject *appSetCHEONMA(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *appIsCHEONMA(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", false);
}

PyObject *appLoadLocaleData(PyObject *poSelf, PyObject *poArgs)
{
    std::string localePath;
    if (!PyTuple_GetString(poArgs, 0, localePath))
        return Py_BuildException();

    return Py_BuildValue("i", LoadLocaleData(localePath.c_str()));
}

PyObject *appGetLocaleName(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("s", LocaleService_GetLocaleName());
}

PyObject *appGetLocalePath(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("s", LocaleService_GetLocalePath());
}
PyObject *appGetLocaleID(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", LocaleService_GetLocaleID());
}
// END_OF_LOCALE

PyObject *appGetDefaultCodePage(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", 0);
}

#ifdef __VTUNE__

PyObject *appGetImageInfo(PyObject *poSelf, PyObject *poArgs)
{
    char *szFileName;
    if (!PyTuple_GetString(poArgs, 0, &szFileName))
        return Py_BuildException();

    return Py_BuildValue("iii", 0, 0, 0);
}

#else

#include <base/stb_image.h>

PyObject *appGetImageInfo(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
      return Py_BuildException();

    bool canLoad = FALSE;
    int32_t uWidth = 0;
    int32_t uHeight = 0;
    int comp;
    if (stbi_info(szFileName.c_str(), &uWidth, &uHeight, &comp))
    {
        canLoad = TRUE;
    }

    return Py_BuildValue("iii", canLoad, uWidth, uHeight);
}
#endif

#include <pak/Vfs.hpp>

PyObject *appIsExistFile(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    bool isExist = GetVfs().Exists(szFileName);

    return Py_BuildValue("i", isExist);
}

PyObject *appGetFileList(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFilter;
    if (!PyTuple_GetString(poArgs, 0, szFilter))
        return Py_BuildException();

    PyObject *poList = PyList_New(0);

    WIN32_FIND_DATAA wfd;
    memset(&wfd, 0, sizeof(wfd));

    HANDLE hFind = FindFirstFileA(szFilter.c_str(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            PyObject *poFileName = PyString_FromString(wfd.cFileName);
            PyList_Append(poList, poFileName);
        } while (FindNextFileA(hFind, &wfd));

        FindClose(hFind);
    }

    return poList;
}

PyObject *appUpdateGame(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().UpdateGame();
    Py_RETURN_NONE;
}

PyObject *appRenderGame(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().RenderGame();
    Py_RETURN_NONE;
}

PyObject *appSetMouseHandler(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poHandler))
        return Py_BuildException();

    CPythonApplication::AppInst().SetMouseHandler(poHandler);
    Py_RETURN_NONE;
}

PyObject *appCreate(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    int width;
    if (!PyTuple_GetInteger(poArgs, 1, &width))
        return Py_BuildException();

    int height;
    if (!PyTuple_GetInteger(poArgs, 2, &height))
        return Py_BuildException();

    int Windowed;
    if (!PyTuple_GetInteger(poArgs, 3, &Windowed))
        return Py_BuildException();

    CPythonApplication &rkApp = CPythonApplication::AppInst();
    if (!rkApp.Create(poSelf, szName.c_str(), width, height, Windowed))
    {
        return Py_BuildException();
    }

    Py_RETURN_NONE;
}

PyObject *appLoop(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().Loop();

    Py_RETURN_NONE;
}

PyObject *appGetInfo(PyObject *poSelf, PyObject *poArgs)
{
    int nInfo;
    if (!PyTuple_GetInteger(poArgs, 0, &nInfo))
        return Py_BuildException();

    std::string stInfo;
    CPythonApplication::AppInst().GetInfo(nInfo, &stInfo);
    return Py_BuildValue("s", stInfo.c_str());
}

PyObject *appProcess(PyObject *poSelf, PyObject *poArgs)
{
    if (CPythonApplication::AppInst().Process())
        return Py_BuildValue("i", 1);

    return Py_BuildValue("i", 0);
}

PyObject *appAbort(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().Abort();
    Py_RETURN_NONE;
}

PyObject *appExit(PyObject *poSelf, PyObject *poArgs)
{
    Engine::GetSettings().SaveConfig();
    CPythonApplication::AppInst().Exit();
    Py_RETURN_NONE;
}

PyObject *appSetCamera(PyObject *poSelf, PyObject *poArgs)
{
    float Distance;
    if (!PyTuple_GetFloat(poArgs, 0, &Distance))
        return Py_BuildException();

    float Pitch;
    if (!PyTuple_GetFloat(poArgs, 1, &Pitch))
        return Py_BuildException();

    float Rotation;
    if (!PyTuple_GetFloat(poArgs, 2, &Rotation))
        return Py_BuildException();

    float fDestinationHeight;
    if (!PyTuple_GetFloat(poArgs, 3, &fDestinationHeight))
        return Py_BuildException();

    CPythonApplication::AppInst().SetCamera(Distance, Pitch, Rotation, fDestinationHeight);
    Py_RETURN_NONE;
}

PyObject *appGetCamera(PyObject *poSelf, PyObject *poArgs)
{
    float Distance, Pitch, Rotation, DestinationHeight;
    CPythonApplication::AppInst().GetCamera(&Distance, &Pitch, &Rotation, &DestinationHeight);

    return Py_BuildValue("ffff", Distance, Pitch, Rotation, DestinationHeight);
}

PyObject *appGetCameraPitch(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("f", CPythonApplication::AppInst().GetPitch());
}

PyObject *appGetCameraRotation(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("f", CPythonApplication::AppInst().GetRotation());
}

PyObject *appGetTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("f", CPythonApplication::AppInst().GetGlobalTime());
}

PyObject *appGetGlobalTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetServerTime());
}

PyObject *appGetGlobalTimeStamp(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetServerTimeStamp());
}

PyObject *appGetUpdateFPS(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetUpdateFPS());
}

PyObject *appGetRenderFPS(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetRenderFPS());
}



PyObject *appRotateCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().RotateCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appPitchCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().PitchCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appZoomCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().ZoomCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appMovieRotateCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().MovieRotateCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appMoviePitchCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().MoviePitchCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appMovieZoomCamera(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();
    CPythonApplication::AppInst().MovieZoomCamera(iDirection);
    Py_RETURN_NONE;
}

PyObject *appMovieResetCamera(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().MovieResetCamera();
    Py_RETURN_NONE;
}

PyObject *appGetFaceSpeed(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("f", CPythonApplication::AppInst().GetFaceSpeed());
}

PyObject *appGetRenderTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("fi", CPythonApplication::AppInst().GetAveRenderTime(),
                         CPythonApplication::AppInst().GetCurRenderTime());
}

PyObject *appGetUpdateTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetCurUpdateTime());
}

PyObject *appGetLoad(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetLoad());
}

PyObject *appGetFaceCount(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetFaceCount());
}

PyObject *appGetAvaiableTextureMememory(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CGraphicBase::GetAvailableTextureMemory());
}

PyObject *appSetFPS(PyObject *poSelf, PyObject *poArgs)
{
    int iFPS;
    if (!PyTuple_GetInteger(poArgs, 0, &iFPS))
        return Py_BuildException();

    CPythonApplication::AppInst().SetFPS(iFPS);

    Py_RETURN_NONE;
}

PyObject *appSetGlobalCenterPosition(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
        return Py_BuildException();

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
        return Py_BuildException();

    CPythonApplication::AppInst().SetCenterPosition(x, y);
    Py_RETURN_NONE;
}

PyObject *appSetCenterPosition(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
        return Py_BadArgument();

    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
        return Py_BadArgument();

    float fz;
    if (!PyTuple_GetFloat(poArgs, 2, &fz))
    {
        CPythonApplication::AppInst().SetCenterPosition(fx, fy);
    }
    else
    {
        CPythonApplication::AppInst().SetCenterPosition(fx, -fy, fz);
    }
    Py_RETURN_NONE;
}

PyObject *appGetCursorPosition(PyObject *poSelf, PyObject *poArgs)
{
    auto [lx, ly] = UI::CWindowManager::Instance().GetMousePosition();
    return Py_BuildValue("ii", lx, ly);
}

PyObject *appIsPressed(PyObject *poSelf, PyObject *poArgs)
{
    int iKey;
    if (!PyTuple_GetInteger(poArgs, 0, &iKey))
        return Py_BuildException();

    return Py_BuildValue("i", GetKeyboardInput().IsKeyPressed(iKey));
}

PyObject *appSetCursor(PyObject *poSelf, PyObject *poArgs)
{
    /*
        char * szName;
        if (!PyTuple_GetString(poArgs, 0, &szName))
            return Py_BuildException();

        if (!CPythonApplication::Instance().SetHardwareCursor(szName))
            return Py_BuildException("Wrong Cursor Name [%s]", szName);
    */
    int iCursorNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iCursorNum))
        return Py_BuildException();

    if (!CPythonApplication::AppInst().SetCursorNum(iCursorNum))
        return Py_BuildException("Wrong Cursor Name [%d]", iCursorNum);

    Py_RETURN_NONE;
}

PyObject *appGetCursor(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonApplication::AppInst().GetCursorNum());
}

PyObject *appShowCursor(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().SetCursorVisible(TRUE);

    Py_RETURN_NONE;
}

PyObject *appHideCursor(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().SetCursorVisible(FALSE);

    Py_RETURN_NONE;
}

PyObject *appIsShowCursor(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", TRUE == CPythonApplication::AppInst().GetCursorVisible());
}

PyObject *appIsLiarCursorOn(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", TRUE == CPythonApplication::AppInst().GetLiarCursorOn());
}

PyObject *appSetSoftwareCursor(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *appSetHardwareCursor(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
    Py_RETURN_NONE;
}

PyObject *appSetConnectData(PyObject *poSelf, PyObject *poArgs)
{
    std::string szIP;
    if (!PyTuple_GetString(poArgs, 0, szIP))
        return Py_BuildException();

    int iPort;
    if (!PyTuple_GetInteger(poArgs, 1, &iPort))
        return Py_BuildException();

    CPythonApplication::AppInst().SetConnectData(szIP.c_str(), iPort);

    Py_RETURN_NONE;
}

PyObject *appGetConnectData(PyObject *poSelf, PyObject *poArgs)
{
    std::string strIP;
    int iPort;

    CPythonApplication::AppInst().GetConnectData(strIP, iPort);

    return Py_BuildValue("si", strIP.c_str(), iPort);
}

PyObject *appGetRandom(PyObject *poSelf, PyObject *poArgs)
{
    int from;
    if (!PyTuple_GetInteger(poArgs, 0, &from))
        return Py_BuildException();

    int to;
    if (!PyTuple_GetInteger(poArgs, 1, &to))
        return Py_BuildException();

    if (from > to)
    {
        int tmp = from;
        from = to;
        to = tmp;
    }

    return Py_BuildValue("i", GetRandom(from, to));
}

PyObject *appGetRotatingDirection(PyObject *poSelf, PyObject *poArgs)
{
    float fSource;
    if (!PyTuple_GetFloat(poArgs, 0, &fSource))
        return Py_BuildException();
    float fTarget;
    if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
        return Py_BuildException();

    return Py_BuildValue("i", GetRotatingDirection(fSource, fTarget));
}

PyObject *appGetDegreeDifference(PyObject *poSelf, PyObject *poArgs)
{
    float fSource;
    if (!PyTuple_GetFloat(poArgs, 0, &fSource))
        return Py_BuildException();
    float fTarget;
    if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
        return Py_BuildException();

    return Py_BuildValue("f", GetDegreeDifference(fSource, fTarget));
}

PyObject *appSleep(PyObject *poSelf, PyObject *poArgs)
{
    int iTime;
    if (!PyTuple_GetInteger(poArgs, 0, &iTime))
        return Py_BuildException();

    Sleep(iTime);

    Py_RETURN_NONE;
}

PyObject *appSetDefaultFontName(PyObject *poSelf, PyObject *poArgs)
{
    std::string normal;
    if (!PyTuple_GetString(poArgs, 0, normal))
        return Py_BuildException("missing font name");

    std::string italic;
    if (!PyTuple_GetString(poArgs, 0, italic))
        return Py_BuildException("missing italic font name");
    std::string bold;
    if (!PyTuple_GetString(poArgs, 0, bold))
        return Py_BuildException("missing bold font name");

    Engine::GetFontManager().SetDefaultFont(normal, italic, bold);
    Py_RETURN_NONE;
}

PyObject *appSetGuildSymbolPath(PyObject *poSelf, PyObject *poArgs)
{
    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 0, szPathName))
        return Py_BuildException();

    SetGuildSymbolPath(szPathName.c_str());

    Py_RETURN_NONE;
}

PyObject *appEnableSpecialCameraMode(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().EnableSpecialCameraMode();
    Py_RETURN_NONE;
}

PyObject *appSetCameraSpeed(PyObject *poSelf, PyObject *poArgs)
{
    int iPercentage;
    if (!PyTuple_GetInteger(poArgs, 0, &iPercentage))
        return Py_BuildException();

    CPythonApplication::AppInst().SetCameraSpeed(iPercentage);

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (pCamera)
        pCamera->SetResistance(float(iPercentage) / 100.0f);
    Py_RETURN_NONE;
}

PyObject *appIsFileExist(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    return Py_BuildValue("i", -1 != _access(szFileName.c_str(), 0));
}

PyObject *appSetCameraSetting(PyObject *poSelf, PyObject *poArgs)
{
    int ix;
    if (!PyTuple_GetInteger(poArgs, 0, &ix))
        return Py_BuildException();
    int iy;
    if (!PyTuple_GetInteger(poArgs, 1, &iy))
        return Py_BuildException();
    int iz;
    if (!PyTuple_GetInteger(poArgs, 2, &iz))
        return Py_BuildException();

    int iZoom;
    if (!PyTuple_GetInteger(poArgs, 3, &iZoom))
        return Py_BuildException();
    int iRotation;
    if (!PyTuple_GetInteger(poArgs, 4, &iRotation))
        return Py_BuildException();
    int iPitch;
    if (!PyTuple_GetInteger(poArgs, 5, &iPitch))
        return Py_BuildException();

    CPythonApplication::SCameraSetting CameraSetting;
    ZeroMemory(&CameraSetting, sizeof(CameraSetting));
    CameraSetting.v3CenterPosition.x = float(ix);
    CameraSetting.v3CenterPosition.y = float(iy);
    CameraSetting.v3CenterPosition.z = float(iz);
    CameraSetting.fZoom = float(iZoom);
    CameraSetting.fRotation = float(iRotation);
    CameraSetting.fPitch = float(iPitch);
    CPythonApplication::AppInst().SetEventCamera(CameraSetting);
    Py_RETURN_NONE;
}

PyObject *appSaveCameraSetting(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    CPythonApplication::AppInst().SaveCameraSetting(szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *appLoadCameraSetting(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    bool bResult = CPythonApplication::AppInst().LoadCameraSetting(szFileName.c_str());
    return Py_BuildValue("i", bResult);
}

PyObject *appSetDefaultCamera(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().SetDefaultCamera();
    Py_RETURN_NONE;
}

PyObject *appSetFreeCamera(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().SetFreeCamera();
    Py_RETURN_NONE;
}

PyObject *appSetFreeCameraSpeed(PyObject *poSelf, PyObject *poArgs)
{
    int iSpeed;
    if (!PyTuple_GetInteger(poArgs, 0, &iSpeed))
        return Py_BuildException();

    CPythonApplication::AppInst().SetFreeCameraSpeed(iSpeed);
    Py_RETURN_NONE;
}

PyObject *appSetSightRange(PyObject *poSelf, PyObject *poArgs)
{
    int iRange;
    if (!PyTuple_GetInteger(poArgs, 0, &iRange))
        return Py_BuildException();

    CPythonApplication::AppInst().SetForceSightRange(iRange);
    Py_RETURN_NONE;
}

PyObject *apptestGetAccumulationTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", 0);
}

PyObject *apptestResetAccumulationTime(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *apptestSetSpecularColor(PyObject *poSelf, PyObject *poArgs)
{
    float fr;
    if (!PyTuple_GetFloat(poArgs, 0, &fr))
        return Py_BuildException();
    float fg;
    if (!PyTuple_GetFloat(poArgs, 1, &fg))
        return Py_BuildException();
    float fb;
    if (!PyTuple_GetFloat(poArgs, 2, &fb))
        return Py_BuildException();
    g_fSpecularColor = DirectX::SimpleMath::Color(fr, fg, fb, 1.0f);
    Py_RETURN_NONE;
}

PyObject *appSetVisibleNotice(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BuildException();
    Py_RETURN_NONE;
}

PyObject *appIsVisibleNotice(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", 1);
}

PyObject *appEnableTestServerFlag(PyObject *poSelf, PyObject *poArgs)
{
    bTestServerFlag = TRUE;
    Py_RETURN_NONE;
}

PyObject *appIsEnableTestServerFlag(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", bTestServerFlag);
}

class CTextLineLoader
{
  public:
    CTextLineLoader(const char *c_szFileName)
    {
        auto data = LoadFileToString(GetVfs(), c_szFileName);
        if (!data)
            return;

        m_kTextFileLoader.Bind(data.value());
    }

    uint32_t GetLineCount()
    {
        return m_kTextFileLoader.GetLineCount();
    }

    const char *GetLine(uint32_t dwIndex)
    {
        if (dwIndex >= GetLineCount())
            return "";

        return m_kTextFileLoader.GetLineString(dwIndex).c_str();
    }

  protected:
    CMemoryTextFileLoader m_kTextFileLoader;
};

PyObject *appSetGuildMarkPath(PyObject *poSelf, PyObject *poArgs)
{
    std::string path;
    if (!PyTuple_GetString(poArgs, 0, path))
        return Py_BuildException();

    char newPath[256];
    auto ext = strstr(path.c_str(), ".tga");

    if (ext)
    {
        int extPos = ext - path.c_str();
        strncpy_s(newPath, path.c_str(), extPos);
        newPath[extPos] = '\0';
    }
    else
        strncpy_s(newPath, path.c_str(), sizeof(newPath) - 1);

    CGuildMarkManager::Instance().SetMarkPathPrefix(newPath);
    Py_RETURN_NONE;
}

PyObject *appIsDevStage(PyObject *poSelf, PyObject *poArgs)
{
    int nIsDevelopmentStage = 0;
#if defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
    nIsDevelopmentStage = 1;
#endif
    return Py_BuildValue("i", nIsDevelopmentStage);
}

PyObject *appIsTestStage(PyObject *poSelf, PyObject *poArgs)
{
    int nIsTestStage = 0;
#if defined(LOCALE_SERVICE_STAGE_TEST)
    nIsTestStage = 1;
#endif
    return Py_BuildValue("i", nIsTestStage);
}

PyObject *appIsLiveStage(PyObject *poSelf, PyObject *poArgs)
{
    int nIsLiveStage = 0;
#if !defined(LOCALE_SERVICE_STAGE_TEST) && !defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
    nIsLiveStage = 1;
#endif
    return Py_BuildValue("i", nIsLiveStage);
}

PyObject *appLogoOpen(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    int nIsSuccess = 1; // CPythonApplication::Instance().OnLogoOpen(szName);

    return Py_BuildValue("i", nIsSuccess);
}

PyObject *appLogoUpdate(PyObject *poSelf, PyObject *poArgs)
{
    int nIsRun = 0; // CPythonApplication::Instance().OnLogoUpdate();
    return Py_BuildValue("i", nIsRun);
}

PyObject *appLogoRender(PyObject *poSelf, PyObject *poArgs)
{
    // CPythonApplication::Instance().OnLogoRender();
    Py_RETURN_NONE;
}

PyObject *appLogoClose(PyObject *poSelf, PyObject *poArgs)
{
    // CPythonApplication::Instance().OnLogoClose();
    Py_RETURN_NONE;
}

PyObject *appMyShopDecoBGCreate(PyObject *poSelf, PyObject *poArgs)
{
    // CPythonApplication::Instance().OnLogoClose();
    return Py_BuildValue("b", CPythonApplication::AppInst().MyShopDecoBGCreate());
}

PyObject *appEncryptLocal(PyObject * /*self*/, PyObject *args)
{
    char *data;
    Py_ssize_t size;
    if (!PyTuple_GetStringAndSize(args, 0, &data, &size))
        return Py_BadArgument();

    if (data)
    {
        DATA_BLOB input, output;
        input.pbData = (uint8_t *)data;
        input.cbData = size;

        PyObject *resultStr = nullptr;

        if (CryptProtectData(&input, L"METIN2_LOCAL", nullptr, nullptr, nullptr,
                             CRYPTPROTECT_LOCAL_MACHINE | CRYPTPROTECT_UI_FORBIDDEN, &output))
        {
            resultStr = PyString_FromStringAndSize(reinterpret_cast<char *>(output.pbData), output.cbData);

            LocalFree(output.pbData);
        }
        else
        {
            PyErr_SetFromWindowsErr(GetLastError());
        }

        return resultStr;
    }
    Py_RETURN_NONE;
}

PyObject *appDecryptLocal(PyObject * /*self*/, PyObject *args)
{
    char *data;
    Py_ssize_t size;
    if (!PyTuple_GetStringAndSize(args, 0, &data, &size))
        return Py_BadArgument();

    if (data)
    {
        DATA_BLOB input, output;
        input.pbData = (uint8_t *)data;
        input.cbData = size;

        PyObject *resultStr = nullptr;
        if (CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output))
        {
            resultStr = PyString_FromStringAndSize(reinterpret_cast<char *>(output.pbData), output.cbData);

            LocalFree(output.pbData);
        }
        else
        {
            PyErr_SetFromWindowsErr(GetLastError());
        }

        return resultStr;
    }
    Py_RETURN_NONE;
}

PyObject *appSetDefaultCodePage(PyObject *poSelf, PyObject *poArgs)
{
    int codePage;
    if (!PyTuple_GetInteger(poArgs, 0, &codePage))
    {
        return Py_BuildException();
    }

    Py_RETURN_NONE;
}

#ifdef CAMY_MODULE
PyObject *appSetCamyActivityState(PyObject *poSelf, PyObject *poArgs)
{
    int iActive;
    if (!PyTuple_GetInteger(poArgs, 0, &iActive))
        return Py_BuildException();

    if (iActive == 1)
    {

        CPythonApplication::Instance().m_bCamyIsActive = true;
    }
    else
    {
        CPythonApplication::Instance().m_bCamyIsActive = false;
    }

    Py_RETURN_NONE;
}
#endif

PyObject *appFlashApplication(PyObject *poSelf, PyObject *poArgs)
{
    CPythonApplication::AppInst().FlashApplication();
    Py_RETURN_NONE;
}

extern "C" void initapp()
{
    static PyMethodDef s_methods[] = {
        {"IsDevStage", appIsDevStage, METH_VARARGS},
        {"IsTestStage", appIsTestStage, METH_VARARGS},
        {"IsLiveStage", appIsLiveStage, METH_VARARGS},
#ifdef CAMY_MODULE
        {"SetCamyActivityState", appSetCamyActivityState, METH_VARARGS},
#endif

        // TEXTTAIL_LIVINGTIME_CONTROL
        {"SetTextTailLivingTime", appSetTextTailLivingTime, METH_VARARGS},
        // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

        {"EnablePerformanceTime", appEnablePerformanceTime, METH_VARARGS},
        {"SetHairColorEnable", appSetHairColorEnable, METH_VARARGS},

        {"SetArmorSpecularEnable", appSetArmorSpecularEnable, METH_VARARGS},
        {"SetWeaponSpecularEnable", appSetWeaponSpecularEnable, METH_VARARGS},
        {"SetSkillEffectUpgradeEnable", appSetSkillEffectUpgradeEnable, METH_VARARGS},
        {"SetTwoHandedWeaponAttSpeedDecreaseValue", SetTwoHandedWeaponAttSpeedDecreaseValue, METH_VARARGS},
        {"SetRideHorseEnable", appSetRideHorseEnable, METH_VARARGS},

        {"SetCameraMaxDistance", appSetCameraMaxDistance, METH_VARARGS},
        {"SetMinFog", appSetMinFog, METH_VARARGS},
        {"SetFrameSkip", appSetFrameSkip, METH_VARARGS},
        {"GetImageInfo", appGetImageInfo, METH_VARARGS},
        {"GetInfo", appGetInfo, METH_VARARGS},
        {"Create", appCreate, METH_VARARGS},
        {"SetMouseHandler", appSetMouseHandler, METH_VARARGS},
        {"IsExistFile", appIsExistFile, METH_VARARGS},
        {"GetFileList", appGetFileList, METH_VARARGS},

        {"SetCamera", appSetCamera, METH_VARARGS},
        {"GetCamera", appGetCamera, METH_VARARGS},
        {"GetCameraPitch", appGetCameraPitch, METH_VARARGS},
        {"GetCameraRotation", appGetCameraRotation, METH_VARARGS},
        {"GetTime", appGetTime, METH_VARARGS},
        {"GetGlobalTime", appGetGlobalTime, METH_VARARGS},
        {"GetGlobalTimeStamp", appGetGlobalTimeStamp, METH_VARARGS},
        {"GetUpdateFPS", appGetUpdateFPS, METH_VARARGS},
        {"GetRenderFPS", appGetRenderFPS, METH_VARARGS},
        {"RotateCamera", appRotateCamera, METH_VARARGS},
        {"PitchCamera", appPitchCamera, METH_VARARGS},
        {"ZoomCamera", appZoomCamera, METH_VARARGS},
        {"MovieRotateCamera", appMovieRotateCamera, METH_VARARGS},
        {"MoviePitchCamera", appMoviePitchCamera, METH_VARARGS},
        {"MovieZoomCamera", appMovieZoomCamera, METH_VARARGS},
        {"MovieResetCamera", appMovieResetCamera, METH_VARARGS},

        {"GetAvailableTextureMemory", appGetAvaiableTextureMememory, METH_VARARGS},
        {"GetRenderTime", appGetRenderTime, METH_VARARGS},
        {"GetUpdateTime", appGetUpdateTime, METH_VARARGS},
        {"GetLoad", appGetLoad, METH_VARARGS},
        {"GetFaceSpeed", appGetFaceSpeed, METH_VARARGS},
        {"GetFaceCount", appGetFaceCount, METH_VARARGS},
        {"SetFPS", appSetFPS, METH_VARARGS},
        {"SetCenterPosition", appSetCenterPosition, METH_VARARGS},
        {"GetCursorPosition", appGetCursorPosition, METH_VARARGS},

        {"GetRandom", appGetRandom, METH_VARARGS},
        {"IsWebPageMode", appIsWebPageMode, METH_VARARGS},
        {"ShowWebPage", appShowWebPage, METH_VARARGS},
        {"MoveWebPage", appMoveWebPage, METH_VARARGS},
        {"HideWebPage", appHideWebPage, METH_VARARGS},
        {"IsPressed", appIsPressed, METH_VARARGS},
        {"SetCursor", appSetCursor, METH_VARARGS},
        {"GetCursor", appGetCursor, METH_VARARGS},
        {"ShowCursor", appShowCursor, METH_VARARGS},
        {"HideCursor", appHideCursor, METH_VARARGS},
        {"IsShowCursor", appIsShowCursor, METH_VARARGS},
        {"IsLiarCursorOn", appIsLiarCursorOn, METH_VARARGS},
        {"SetSoftwareCursor", appSetSoftwareCursor, METH_VARARGS},
        {"SetHardwareCursor", appSetHardwareCursor, METH_VARARGS},

        {"SetConnectData", appSetConnectData, METH_VARARGS},
        {"GetConnectData", appGetConnectData, METH_VARARGS},

        {"GetRotatingDirection", appGetRotatingDirection, METH_VARARGS},
        {"GetDegreeDifference", appGetDegreeDifference, METH_VARARGS},
        {"Sleep", appSleep, METH_VARARGS},
        {"SetDefaultFontName", appSetDefaultFontName, METH_VARARGS},
        {"SetGuildSymbolPath", appSetGuildSymbolPath, METH_VARARGS},

        {"EnableSpecialCameraMode", appEnableSpecialCameraMode, METH_VARARGS},
        {"SetCameraSpeed", appSetCameraSpeed, METH_VARARGS},

        {"SaveCameraSetting", appSaveCameraSetting, METH_VARARGS},
        {"LoadCameraSetting", appLoadCameraSetting, METH_VARARGS},
        {"SetDefaultCamera", appSetDefaultCamera, METH_VARARGS},
        {"SetCameraSetting", appSetCameraSetting, METH_VARARGS},
        {"SetFreeCamera", appSetFreeCamera, METH_VARARGS},
        {"SetFreeCameraSpeed", appSetFreeCameraSpeed, METH_VARARGS},

        {"SetSightRange", appSetSightRange, METH_VARARGS},

        {"IsFileExist", appIsFileExist, METH_VARARGS},

        // LOCALE
        {"GetLocaleServiceName", appGetLocaleServiceName, METH_VARARGS},
        {"GetLocaleName", appGetLocaleName, METH_VARARGS},
        {"GetLocalePath", appGetLocalePath, METH_VARARGS},
        {"ForceSetLocale", appForceSetLocale, METH_VARARGS},
        {"GetLocaleID", appGetLocaleID, METH_VARARGS},

        
        // END_OF_LOCALE

        {"SetCHEONMA", appSetCHEONMA, METH_VARARGS},
        {"IsCHEONMA", appIsCHEONMA, METH_VARARGS},

        {"GetDefaultCodePage", appGetDefaultCodePage, METH_VARARGS},
        {"SetSpecularSpeed", appSetSpecularSpeed, METH_VARARGS},

        {"testGetAccumulationTime", apptestGetAccumulationTime, METH_VARARGS},
        {"testResetAccumulationTime", apptestResetAccumulationTime, METH_VARARGS},
        {"testSetSpecularColor", apptestSetSpecularColor, METH_VARARGS},

        {"SetVisibleNotice", appSetVisibleNotice, METH_VARARGS},
        {"IsVisibleNotice", appIsVisibleNotice, METH_VARARGS},
        {"EnableTestServerFlag", appEnableTestServerFlag, METH_VARARGS},
        {"IsEnableTestServerFlag", appIsEnableTestServerFlag, METH_VARARGS},

        {"SetGuildMarkPath", appSetGuildMarkPath, METH_VARARGS},

        {"OnLogoUpdate", appLogoUpdate, METH_VARARGS},
        {"OnLogoRender", appLogoRender, METH_VARARGS},
        {"OnLogoOpen", appLogoOpen, METH_VARARGS},
        {"OnLogoClose", appLogoClose, METH_VARARGS},

        {"MyShopDecoBGCreate", appMyShopDecoBGCreate, METH_VARARGS},

        {"EncryptByComputer", appEncryptLocal, METH_VARARGS},
        {"DecryptByComputer", appDecryptLocal, METH_VARARGS},
        {"SetDefaultCodePage", appSetDefaultCodePage, METH_VARARGS},

        {"FlashApplication", appFlashApplication, METH_VARARGS},
		{"LoadLocaleAddr",  appLoadLocaleAddr,  METH_VARARGS },

        {NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("app", s_methods);
#ifdef CAMY_MODULE
    PyModule_AddIntConstant(poModule, "CAMY_COMPATIBLE_CLIENT", TRUE);
#endif
    PyModule_AddIntConstant(poModule, "INFO_ITEM", CPythonApplication::INFO_ITEM);
    PyModule_AddIntConstant(poModule, "INFO_ACTOR", CPythonApplication::INFO_ACTOR);
    PyModule_AddIntConstant(poModule, "INFO_EFFECT", CPythonApplication::INFO_EFFECT);
    PyModule_AddIntConstant(poModule, "INFO_TEXTTAIL", CPythonApplication::INFO_TEXTTAIL);

    PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_SAME", DEGREE_DIRECTION_SAME);
    PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_RIGHT", DEGREE_DIRECTION_RIGHT);
    PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_LEFT", DEGREE_DIRECTION_LEFT);

    PyModule_AddIntConstant(poModule, "VK_BACK", kVirtualKeyBack);
    PyModule_AddIntConstant(poModule, "VK_TAB", kVirtualKeyTab);

    PyModule_AddIntConstant(poModule, "VK_CLEAR", kVirtualKeyClear);
    PyModule_AddIntConstant(poModule, "VK_RETURN", kVirtualKeyReturn);

    PyModule_AddIntConstant(poModule, "VK_SHIFT", kVirtualKeyShift);
    PyModule_AddIntConstant(poModule, "VK_CONTROL", kVirtualKeyControl);
    PyModule_AddIntConstant(poModule, "VK_MENU", kVirtualKeyMenu);
    PyModule_AddIntConstant(poModule, "VK_PAUSE", kVirtualKeyPause);
    PyModule_AddIntConstant(poModule, "VK_CAPITAL", kVirtualKeyCapital);

    PyModule_AddIntConstant(poModule, "VK_ESCAPE", kVirtualKeyEscape);
    PyModule_AddIntConstant(poModule, "VK_SPACE", kVirtualKeySpace);

    PyModule_AddIntConstant(poModule, "VK_PRIOR", kVirtualKeyPrior);
    PyModule_AddIntConstant(poModule, "VK_NEXT", kVirtualKeyNext);

    PyModule_AddIntConstant(poModule, "VK_END", kVirtualKeyEnd);
    PyModule_AddIntConstant(poModule, "VK_HOME", kVirtualKeyHome);

    PyModule_AddIntConstant(poModule, "VK_LEFT", kVirtualKeyLeft);
    PyModule_AddIntConstant(poModule, "VK_UP", kVirtualKeyUp);
    PyModule_AddIntConstant(poModule, "VK_RIGHT", kVirtualKeyRight);
    PyModule_AddIntConstant(poModule, "VK_DOWN", kVirtualKeyDown);

    PyModule_AddIntConstant(poModule, "VK_SELECT", kVirtualKeySelect);
    PyModule_AddIntConstant(poModule, "VK_PRINT", kVirtualKeyPrint);
    PyModule_AddIntConstant(poModule, "VK_EXECUTE", kVirtualKeyExecute);
    PyModule_AddIntConstant(poModule, "VK_SNAPSHOT", kVirtualKeySnapshot);

    PyModule_AddIntConstant(poModule, "VK_INSERT", kVirtualKeyInsert);
    PyModule_AddIntConstant(poModule, "VK_DELETE", kVirtualKeyDelete);

    PyModule_AddIntConstant(poModule, "VK_0", kVirtualKey0);
    PyModule_AddIntConstant(poModule, "VK_1", kVirtualKey1);
    PyModule_AddIntConstant(poModule, "VK_2", kVirtualKey2);
    PyModule_AddIntConstant(poModule, "VK_3", kVirtualKey3);
    PyModule_AddIntConstant(poModule, "VK_4", kVirtualKey4);
    PyModule_AddIntConstant(poModule, "VK_5", kVirtualKey5);
    PyModule_AddIntConstant(poModule, "VK_6", kVirtualKey6);
    PyModule_AddIntConstant(poModule, "VK_7", kVirtualKey7);
    PyModule_AddIntConstant(poModule, "VK_8", kVirtualKey8);
    PyModule_AddIntConstant(poModule, "VK_9", kVirtualKey9);

    PyModule_AddIntConstant(poModule, "VK_A", kVirtualKeyA);
    PyModule_AddIntConstant(poModule, "VK_B", kVirtualKeyB);
    PyModule_AddIntConstant(poModule, "VK_C", kVirtualKeyC);
    PyModule_AddIntConstant(poModule, "VK_D", kVirtualKeyD);
    PyModule_AddIntConstant(poModule, "VK_E", kVirtualKeyE);
    PyModule_AddIntConstant(poModule, "VK_F", kVirtualKeyF);
    PyModule_AddIntConstant(poModule, "VK_G", kVirtualKeyG);
    PyModule_AddIntConstant(poModule, "VK_H", kVirtualKeyH);
    PyModule_AddIntConstant(poModule, "VK_I", kVirtualKeyI);
    PyModule_AddIntConstant(poModule, "VK_J", kVirtualKeyJ);
    PyModule_AddIntConstant(poModule, "VK_K", kVirtualKeyK);
    PyModule_AddIntConstant(poModule, "VK_L", kVirtualKeyL);
    PyModule_AddIntConstant(poModule, "VK_M", kVirtualKeyM);
    PyModule_AddIntConstant(poModule, "VK_N", kVirtualKeyN);
    PyModule_AddIntConstant(poModule, "VK_O", kVirtualKeyO);
    PyModule_AddIntConstant(poModule, "VK_P", kVirtualKeyP);
    PyModule_AddIntConstant(poModule, "VK_Q", kVirtualKeyQ);
    PyModule_AddIntConstant(poModule, "VK_R", kVirtualKeyR);
    PyModule_AddIntConstant(poModule, "VK_S", kVirtualKeyS);
    PyModule_AddIntConstant(poModule, "VK_T", kVirtualKeyT);
    PyModule_AddIntConstant(poModule, "VK_U", kVirtualKeyU);
    PyModule_AddIntConstant(poModule, "VK_V", kVirtualKeyV);
    PyModule_AddIntConstant(poModule, "VK_W", kVirtualKeyW);
    PyModule_AddIntConstant(poModule, "VK_X", kVirtualKeyX);
    PyModule_AddIntConstant(poModule, "VK_Y", kVirtualKeyY);
    PyModule_AddIntConstant(poModule, "VK_Z", kVirtualKeyZ);

    PyModule_AddIntConstant(poModule, "VK_NUMPAD0", kVirtualKeyNumpad0);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD1", kVirtualKeyNumpad1);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD2", kVirtualKeyNumpad2);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD3", kVirtualKeyNumpad3);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD4", kVirtualKeyNumpad4);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD5", kVirtualKeyNumpad5);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD6", kVirtualKeyNumpad6);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD7", kVirtualKeyNumpad7);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD8", kVirtualKeyNumpad8);
    PyModule_AddIntConstant(poModule, "VK_NUMPAD9", kVirtualKeyNumpad9);

    PyModule_AddIntConstant(poModule, "VK_MULTIPLY", kVirtualKeyMultiply);
    PyModule_AddIntConstant(poModule, "VK_ADD", kVirtualKeyAdd);
    PyModule_AddIntConstant(poModule, "VK_SEPARATOR", kVirtualKeySeparator);
    PyModule_AddIntConstant(poModule, "VK_SUBTRACT", kVirtualKeySubtract);

    PyModule_AddIntConstant(poModule, "VK_F1", kVirtualKeyF1);
    PyModule_AddIntConstant(poModule, "VK_F2", kVirtualKeyF2);
    PyModule_AddIntConstant(poModule, "VK_F3", kVirtualKeyF3);
    PyModule_AddIntConstant(poModule, "VK_F4", kVirtualKeyF4);
    PyModule_AddIntConstant(poModule, "VK_F5", kVirtualKeyF5);
    PyModule_AddIntConstant(poModule, "VK_F6", kVirtualKeyF6);
    PyModule_AddIntConstant(poModule, "VK_F7", kVirtualKeyF7);
    PyModule_AddIntConstant(poModule, "VK_F8", kVirtualKeyF8);
    PyModule_AddIntConstant(poModule, "VK_F9", kVirtualKeyF9);
    PyModule_AddIntConstant(poModule, "VK_F10", kVirtualKeyF10);
    PyModule_AddIntConstant(poModule, "VK_F11", kVirtualKeyF11);
    PyModule_AddIntConstant(poModule, "VK_F12", kVirtualKeyF12);

    PyModule_AddIntConstant(poModule, "VK_COMMA", kVirtualKeyComma);
    PyModule_AddIntConstant(poModule, "VK_OEM3", kVirtualKeyOEM3);
    PyModule_AddIntConstant(poModule, "VK_OEM5", kVirtualKeyOEM5);

    PyModule_AddIntMacro(poModule, LANGUAGE_GLOBAL);
    PyModule_AddIntMacro(poModule, LANGUAGE_HU);
    PyModule_AddIntMacro(poModule, LANGUAGE_PL);
    PyModule_AddIntMacro(poModule, LANGUAGE_DE);
    PyModule_AddIntMacro(poModule, LANGUAGE_RO);
    PyModule_AddIntMacro(poModule, LANGUAGE_TR);
    PyModule_AddIntMacro(poModule, LANGUAGE_EN);
    PyModule_AddIntMacro(poModule, LANGUAGE_PT);
    PyModule_AddIntMacro(poModule, LANGUAGE_IT);
    PyModule_AddIntMacro(poModule, LANGUAGE_MAX_NUM);

    // Cursor
    PyModule_AddIntConstant(poModule, "NORMAL", CPythonApplication::CURSOR_SHAPE_NORMAL);
    PyModule_AddIntConstant(poModule, "ATTACK", CPythonApplication::CURSOR_SHAPE_ATTACK);
    PyModule_AddIntConstant(poModule, "TARGET", CPythonApplication::CURSOR_SHAPE_TARGET);
    PyModule_AddIntConstant(poModule, "TALK", CPythonApplication::CURSOR_SHAPE_TALK);
    PyModule_AddIntConstant(poModule, "CANT_GO", CPythonApplication::CURSOR_SHAPE_CANT_GO);
    PyModule_AddIntConstant(poModule, "PICK", CPythonApplication::CURSOR_SHAPE_PICK);

    PyModule_AddIntConstant(poModule, "DOOR", CPythonApplication::CURSOR_SHAPE_DOOR);
    PyModule_AddIntConstant(poModule, "CHAIR", CPythonApplication::CURSOR_SHAPE_CHAIR);
    PyModule_AddIntConstant(poModule, "MAGIC", CPythonApplication::CURSOR_SHAPE_MAGIC);
    PyModule_AddIntConstant(poModule, "BUY", CPythonApplication::CURSOR_SHAPE_BUY);
    PyModule_AddIntConstant(poModule, "SELL", CPythonApplication::CURSOR_SHAPE_SELL);

    PyModule_AddIntConstant(poModule, "CAMERA_ROTATE", CPythonApplication::CURSOR_SHAPE_CAMERA_ROTATE);
    PyModule_AddIntConstant(poModule, "HSIZE", CPythonApplication::CURSOR_SHAPE_HSIZE);
    PyModule_AddIntConstant(poModule, "VSIZE", CPythonApplication::CURSOR_SHAPE_VSIZE);
    PyModule_AddIntConstant(poModule, "HVSIZE", CPythonApplication::CURSOR_SHAPE_HVSIZE);
    PyModule_AddIntConstant(poModule, "FISHING", CPythonApplication::CURSOR_SHAPE_FISHING);

  

    PyModule_AddIntConstant(poModule, "CAMERA_TO_POSITIVE", CPythonApplication::CAMERA_TO_POSITIVE);
    PyModule_AddIntConstant(poModule, "CAMERA_TO_NEGATIVE", CPythonApplication::CAMERA_TO_NEGITIVE);
    PyModule_AddIntConstant(poModule, "CAMERA_STOP", CPythonApplication::CAMERA_STOP);

#ifdef ENABLE_CYTHON
    PyModule_AddIntConstant(poModule, "ENABLE_CYTHON", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CYTHON", 0);
#endif

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM", 0);
#endif

#ifdef ENABLE_COSTUME_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", 0);
#endif

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    PyModule_AddIntConstant(poModule, "ENABLE_MELEY_LAIR_DUNGEON", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MELEY_LAIR_DUNGEON", 0);
#endif

#ifdef ENABLE_ENERGY_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", 0);
#endif

#ifdef ENABLE_DRAGON_SOUL_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", 0);
#endif

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", 0);
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER
    PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER", 0);
#endif

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
    PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5", 0);
#endif

    PyModule_AddIntConstant(poModule, "PLAYER_PER_ACCOUNT", PLAYER_PER_ACCOUNT);

#ifdef ENABLE_678TH_SKILL
    PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL", 0);
#endif

#ifdef ENABLE_FOV_OPTION
    PyModule_AddIntConstant(poModule, "ENABLE_FOV_OPTION", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_FOV_OPTION", 0);
#endif

#ifdef USE_OPENID
    PyModule_AddIntConstant(poModule, "USE_OPENID", 1);
    if (openid_test)
        PyModule_AddIntConstant(poModule, "OPENID_TEST", 1);
    else
        PyModule_AddIntConstant(poModule, "OPENID_TEST", 0);
#else
    PyModule_AddIntConstant(poModule, "USE_OPENID", 0);
    PyModule_AddIntConstant(poModule, "OPENID_TEST", 0);
#endif /* USE_OPENID */

    PyModule_AddIntConstant(poModule, "SEARCH_FILE", 0);
    PyModule_AddIntConstant(poModule, "SEARCH_PACK", 1);

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_GUILDRENEWAL_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_GUILDRENEWAL_SYSTEM", 0);
#endif
#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_SECOND_GUILDRENEWAL_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_SECOND_GUILDRENEWAL_SYSTEM", 0);
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM", 0);
#endif

#ifdef NON_CHECK_DRAGON_SOUL_QUALIFIED
    PyModule_AddIntConstant(poModule, "NON_CHECK_DRAGON_SOUL_QUALIFIED", 1);
#else
    PyModule_AddIntConstant(poModule, "NON_CHECK_DRAGON_SOUL_QUALIFIED", 0);
#endif

#ifdef ENABLE_THREEWAY_RENEWAL
    PyModule_AddIntConstant(poModule, "ENABLE_THREEWAY_RENEWAL", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_THREEWAY_RENEWAL", 0);
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
    PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", 0);
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 0);
#endif

#ifdef POPUPDIALOG_MODIFY
    PyModule_AddIntConstant(poModule, "POPUPDIALOG_MODIFY", 1);
#else
    PyModule_AddIntConstant(poModule, "POPUPDIALOG_MODIFY", 0);
#endif

#ifdef WJ_NEW_USER_CARE
    PyModule_AddIntConstant(poModule, "WJ_NEW_USER_CARE", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_NEW_USER_CARE", 0);
#endif

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", 0);
#endif

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
    PyModule_AddIntConstant(poModule, "WJ_SHOW_PARTY_ON_MINIMAP", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_SHOW_PARTY_ON_MINIMAP", 0);
#endif

#ifdef WJ_SHOW_NPC_QUEST_NAME
    PyModule_AddIntConstant(poModule, "WJ_SHOW_NPC_QUEST_NAME", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_SHOW_NPC_QUEST_NAME", 0);
#endif

#ifdef WJ_SHOW_MOB_INFO
    PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO", 0);
#endif

#ifdef WJ_SHOW_ACCE_ONOFF
    PyModule_AddIntConstant(poModule, "WJ_SHOW_ACCE_ONOFF", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_SHOW_ACCE_ONOFF", 0);
#endif

#ifdef ENABLE_SPECULAR_ONOFF
    PyModule_AddIntConstant(poModule, "ENABLE_SPECULAR_ONOFF", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_SPECULAR_ONOFF", 0);
#endif

#ifdef WJ_SHOW_SHOP_CONTROLLER
    PyModule_AddIntConstant(poModule, "WJ_SHOW_SHOP_CONTROLLER", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_SHOW_SHOP_CONTROLLER", 0);
#endif

#ifdef WJ_MULTI_TEXTLINE
    PyModule_AddIntConstant(poModule, "WJ_MULTI_TEXTLINE", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_MULTI_TEXTLINE", 0);
#endif

#ifdef ENABLE_DS_PASSWORD
    PyModule_AddIntConstant(poModule, "ENABLE_DS_PASSWORD", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_DS_PASSWORD", 0);
#endif

#ifdef ENABLE_QUIVER_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM", 0);
#endif

#ifdef ENABLE_OX_RENEWAL
    PyModule_AddIntConstant(poModule, "ENABLE_OX_RENEWAL", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_OX_RENEWAL", 0);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM", 0);
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
    PyModule_AddIntConstant(poModule, "ENABLE_MOVE_COSTUME_ATTR", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MOVE_COSTUME_ATTR", 0);
#endif
#ifdef ENABLE_MYSHOP_DECO
    PyModule_AddIntConstant(poModule, "ENABLE_MYSHOP_DECO", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MYSHOP_DECO", 0);
#endif

#ifdef ENABLE_DICE_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM", 0);
#endif

#ifdef ENABLE_MONSTER_CARD
    PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_CARD", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_CARD", 0);
#endif
#ifdef ENABLE_CHANGED_ATTR
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", 0);
#endif

#ifdef ENABLE_SKILLBOOK_COMBINATION
    PyModule_AddIntConstant(poModule, "ENABLE_SKILLBOOK_COMBINATION", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_SKILLBOOK_COMBINATION", 0);
#endif

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    PyModule_AddIntConstant(poModule, "ENABLE_MELEY_LAIR_DUNGEON", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MELEY_LAIR_DUNGEON", 0);
#endif

#ifdef ENABLE_SAFEBOX_IMPROVING
    PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_IMPROVING", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_IMPROVING", 0);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", 0);
#endif
    PyModule_AddIntConstant(poModule, "ACCE_MAX_DRAINRATE", 25);

#ifdef ENABLE_MESSENGER_BLOCK
    PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK", 0);
#endif

#ifdef NEW_SELECT_CHARACTER
    PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER", 1);
#else
    PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER", 0);
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_PRIVATESHOP_SEARCH_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PRIVATESHOP_SEARCH_SYSTEM", 0);
#endif

#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
    PyModule_AddIntConstant(poModule, "ENABLE_DESTROY_ITEM_SYSTTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_DESTROY_ITEM_SYSTTEM", 0);
#endif

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", 0);
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 1);
#else
    PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", 0);
#endif
#ifdef LOGIN_COUNT_DOWN_UI_MODIFY
    PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY", 1);
#else
    PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY", 0);
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
    PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", 0);
#endif

#if defined(ENABLE_AFFECT_POLYMORPH_REMOVE)
    PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_POLYMORPH_REMOVE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_POLYMORPH_REMOVE", 0);
#endif

#ifdef ENABLE_GEM_SYSTEM
    PyModule_AddIntConstant(poModule, "ENABLE_GEM_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_GEM_SYSTEM", 0);
#endif

#if defined(ENABLE_ACCE_SECOND_COSTUME_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SECOND_COSTUME_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SECOND_COSTUME_SYSTEM", 0);
#endif

#if defined(ENABLE_WHISPER_ADMIN_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_WHISPER_ADMIN_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_WHISPER_ADMIN_SYSTEM", 0);
#endif
#if defined(ENABLE_ERROR_MEMBER_BLOCK_CHAT)
    PyModule_AddIntConstant(poModule, "ENABLE_ERROR_MEMBER_BLOCK_CHAT", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ERROR_MEMBER_BLOCK_CHAT", 0);
#endif

#if defined(ENABLE_CHEQUE_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM", 0);
#endif

#if defined(ENABLE_DETAILS_UI)
    PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_UI", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_UI", 0);
#endif

#if defined(ENABLE_12ZI)
    PyModule_AddIntConstant(poModule, "ENABLE_12ZI", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_12ZI", 0);
#endif

#if defined(ENABLE_CHANGE_LOOK_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", 0);
#endif

#if defined(ENABLE_CHANGE_LOOK_ITEM_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_ITEM_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_ITEM_SYSTEM", 0);
#endif

#if defined(ENABLE_PVP_BALANCE)
    PyModule_AddIntConstant(poModule, "ENABLE_PVP_BALANCE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PVP_BALANCE", 0);
#endif

#if defined(ENABLE_NEW_USER_CARE)
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_USER_CARE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_USER_CARE", 0);
#endif

#if defined(ENABLE_BATTLE_FIELD)
    PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_FIELD", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_FIELD", 0);
#endif
#if defined(ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM)
    PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM", 0);
#endif
#if defined(ENABLE_PARTY_MATCH)
    PyModule_AddIntConstant(poModule, "ENABLE_PARTY_MATCH", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PARTY_MATCH", 0);
#endif

#if defined(ENABLE_PENDANT)
    PyModule_AddIntConstant(poModule, "ENABLE_PENDANT", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PENDANT", 0);
#endif

#if defined(ENABLE_ELEMENT_ADD)
    PyModule_AddIntConstant(poModule, "ENABLE_ELEMENT_ADD", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ELEMENT_ADD", 0);
#endif
#if defined(ENABLE_ACCUMULATE_DAMAGE_DISPLAY)
    PyModule_AddIntConstant(poModule, "ENABLE_ACCUMULATE_DAMAGE_DISPLAY", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_ACCUMULATE_DAMAGE_DISPLAY", 0);
#endif

#if defined(ENABLE_CHANGED_ATTR)
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", 0);
#endif

#if defined(ENABLE_WEB_LINKED_BANNER)
    PyModule_AddIntConstant(poModule, "ENABLE_WEB_LINKED_BANNER", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_WEB_LINKED_BANNER", 0);
#endif

#if defined(ENABLE_WEB_LINKED_BANNER_LIMIT_REMOVE)
    PyModule_AddIntConstant(poModule, "ENABLE_WEB_LINKED_BANNER_LIMIT_REMOVE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_WEB_LINKED_BANNER_LIMIT_REMOVE", 0);
#endif

#if defined(ENABLE_USER_SITUATION_NOTICE)
    PyModule_AddIntConstant(poModule, "ENABLE_USER_SITUATION_NOTICE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_USER_SITUATION_NOTICE", 0);
#endif
#if defined(ENABLE_PARTY_CHANNEL_FIX)
    PyModule_AddIntConstant(poModule, "ENABLE_PARTY_CHANNEL_FIX", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PARTY_CHANNEL_FIX", 0);
#endif

#if defined(ENABLE_EXP_EVENT)
    PyModule_AddIntConstant(poModule, "ENABLE_EXP_EVENT", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_EXP_EVENT", 0);
#endif
#if defined(ENABLE_INPUT_CANCEL)
    PyModule_AddIntConstant(poModule, "ENABLE_INPUT_CANCEL", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_INPUT_CANCEL", 0);
#endif

#if defined(INGAME_WIKI)
    PyModule_AddIntConstant(poModule, "ENABLE_INGAME_WIKI", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_INGAME_WIKI", 0);
#endif

#if defined(ENABLE_MONSTER_BACK)
    PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", 0);
#endif

#if defined(ENABLE_CARNIVAL2016)
    PyModule_AddIntConstant(poModule, "ENABLE_CARNIVAL2016", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_CARNIVAL2016", 0);
#endif

#if defined(ENABLE_MINI_GAME_OKEY_NORMAL)
    PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_OKEY_NORMAL", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_OKEY_NORMAL", 0);
#endif

#if defined(ENABLE_FISH_EVENT)
    PyModule_AddIntConstant(poModule, "ENABLE_FISH_EVENT", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_FISH_EVENT", 0);
#endif

#if defined(ENABLE_MINI_GAME_YUTNORI)
    PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI", 0);
#endif

#if defined(ENABLE_SUMMER_EVENT_ROULETTE)
    PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT_ROULETTE", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT_ROULETTE", 0);
#endif

#ifdef ENABLE_BATTLE_PASS
    PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS", 0);
#endif
#ifdef ENABLE_PERSPECTIVE_VIEW
    PyModule_AddIntConstant(poModule, "ENABLE_PERSPECTIVE_VIEW", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PERSPECTIVE_VIEW", 0);
#endif
#ifdef __COOL_REFRESH_EDITLINE__
    PyModule_AddIntConstant(poModule, "__COOL_REFRESH_EDITLINE__", 1);
#else
    PyModule_AddIntConstant(poModule, "__COOL_REFRESH_EDITLINE__", 0);
#endif
#ifdef ENABLE_NEW_GUILD_WAR
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_GUILD_WAR", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_NEW_GUILD_WAR", 0);
#endif
#ifdef ENABLE_PLAYTIME_ICON
    PyModule_AddIntConstant(poModule, "ENABLE_PLAYTIME_ICON", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_PLAYTIME_ICON", 0);
#endif

#ifdef ENABLE_FLY_MOUNT
    PyModule_AddIntConstant(poModule, "ENABLE_FLY_MOUNT", 1);
#else
    PyModule_AddIntConstant(poModule, "ENABLE_FLY_MOUNT", 0);
#endif

}

void SetShowSpecular(CClientConfig &sysSetting, bool iFlag)
{
    sysSetting.SetShowSpecular(iFlag);

    CPythonCharacterManager::CharacterIterator itor = CPythonCharacterManager::instance().CharacterInstanceBegin();
    CPythonCharacterManager::CharacterIterator itorEnd = CPythonCharacterManager::instance().CharacterInstanceEnd();
    for (; itor != itorEnd; ++itor)
    {
        CInstanceBase *pInstance = *itor;
        if (pInstance->IsPC())
        {
            continue;
        }

        float fSpecular = 0.0f;
        if (Engine::GetSettings().IsShowSpecular())
        {
            if (NpcManager::instance().HasSpecularConfig(pInstance->GetRace()))
            {
                fSpecular = NpcManager::instance().GetSpecularValue(pInstance->GetRace());
            }
            else
            {
                fSpecular = 1.0;
            }
        }

        if (fSpecular == 0.0f)
            pInstance->GetGraphicThingInstanceRef().SetSpecularInfoForce(FALSE, PART_MAIN, fSpecular);
        else
            pInstance->GetGraphicThingInstanceRef().SetSpecularInfoForce(TRUE, PART_MAIN, fSpecular);
    }
}

CClientConfig &GetClientConfig(CPythonApplication &app)
{
    return Engine::GetSettings();
}

void init_app(py::module &m)
{
    py::module app = m.def_submodule("app", "Provides access to the app instance");

    app.def("GetLangDisplayName", &GetLangDisplayName);
    app.def("GetLanguageNameByID", &GetLanguageNameByID);
    app.def("GetLanguageIDByName", &GetLanguageIDByName);


    auto settings = py::class_<CClientConfig>(app, "settings");
    settings.def("SaveConfig", &CClientConfig::SaveConfig);
    settings.def("GetGamma", &CClientConfig::GetGamma);
    settings.def("GetWidth", &CClientConfig::GetWidth);
    settings.def("GetHeight", &CClientConfig::GetHeight);
    settings.def("GetBPP", &CClientConfig::GetBPP);
    settings.def("GetFrequency", &CClientConfig::GetFrequency);
    settings.def("IsSoftwareCursor", &CClientConfig::IsSoftwareCursor);
    settings.def("IsWindowed", &CClientConfig::IsWindowed);
    settings.def("SetWindowed", &CClientConfig::SetWindowed);
    settings.def("IsViewChat", &CClientConfig::IsViewChat);
    settings.def("IsAlwaysShowPlayerName", &CClientConfig::IsAlwaysShowPlayerName);
    settings.def("IsAlwaysShowMonsterName", &CClientConfig::IsAlwaysShowMonsterName);
    settings.def("IsAlwaysShowNPCName", &CClientConfig::IsAlwaysShowNPCName);
    settings.def("IsAlwaysShowItemName", &CClientConfig::IsAlwaysShowItemName);
    settings.def("IsShowDamage", &CClientConfig::IsShowDamage);
    settings.def("IsShowSalesText", &CClientConfig::IsShowSalesText);
    settings.def("IsShowCountryFlags", &CClientConfig::IsShowCountryFlags);
    settings.def("IsCollectEquipment", &CClientConfig::IsCollectEquipment);
    settings.def("IsCollectUseableEquipment", &CClientConfig::IsCollectUseableEquipment);
    settings.def("IsHideShops", &CClientConfig::IsHideShops);
    settings.def("IsHidePets", &CClientConfig::IsHidePets);
    settings.def("IsHideGrowthPets", &CClientConfig::IsHideGrowthPets);
    settings.def("IsHideMounts", &CClientConfig::IsHideMounts);
    settings.def("IsFogMode", &CClientConfig::IsFogMode);
    settings.def("IsEnableNightMode", &CClientConfig::IsEnableNightMode);
    settings.def("IsEnableSnowTexture", &CClientConfig::IsEnableSnowTexture);
    settings.def("IsEnableEmojiSystem", &CClientConfig::IsEnableEmojiSystem);
    settings.def("IsShowMoneyLog", &CClientConfig::IsShowMoneyLog);
    settings.def("SetViewChatFlag", &CClientConfig::SetViewChatFlag);
    settings.def("SetAlwaysShowPlayerNameFlag", &CClientConfig::SetAlwaysShowPlayerNameFlag);
    settings.def("SetAlwaysShowNPCNameFlag", &CClientConfig::SetAlwaysShowNPCNameFlag);
    settings.def("SetAlwaysShowMonsterNameFlag", &CClientConfig::SetAlwaysShowMonsterNameFlag);
    settings.def("SetAlwaysShowItemNameFlag", &CClientConfig::SetAlwaysShowItemNameFlag);
    settings.def("SetShowDamageFlag", &CClientConfig::SetShowDamageFlag);
    settings.def("SetShowSalesTextFlag", &CClientConfig::SetShowSalesTextFlag);
    settings.def("SetFogMode", &CClientConfig::SetFogMode);
    settings.def("SetEnableNightMode", &CClientConfig::SetEnableNightMode);
    settings.def("SetEnableeSnowTexture", &CClientConfig::SetEnableeSnowTexture);
    settings.def("SetEnableEmojiSystem", &CClientConfig::SetEnableEmojiSystem);
    settings.def("SetShowMoneyLog", &CClientConfig::SetShowMoneyLog);
    settings.def("IsShowItemLog", &CClientConfig::IsShowItemLog);
    settings.def("SetShowItemLog", &CClientConfig::SetShowItemLog);
    settings.def("IsPickupInfoWindowActive", &CClientConfig::IsPickupInfoWindowActive);
    settings.def("SetPickupInfoWindowActive", &CClientConfig::SetPickupInfoWindowActive);
    settings.def("SetShowCountryFlags", &CClientConfig::SetShowCountryFlags);
    settings.def("SetCollectEquipment", &CClientConfig::SetCollectEquipment);
    settings.def("SetCollectUseableEquipment", &CClientConfig::SetCollectUseableEquipment);
    settings.def("SetHideShops", &CClientConfig::SetHideShops);
    settings.def("SetHidePets", &CClientConfig::SetHidePets);
    settings.def("SetHideGrowthPets", &CClientConfig::SetHideGrowthPets);
    settings.def("SetHideMounts", &CClientConfig::SetHideMounts);
    settings.def("SetShowMobAIFlag", &CClientConfig::SetShowMobAIFlag);
    settings.def("IsShowMobAIFlag", &CClientConfig::IsShowMobAIFlag);
    settings.def("SetShowMobLevel", &CClientConfig::SetShowMobLevel);
    settings.def("IsEnableSnowFall", &CClientConfig::IsEnableSnowFall);
    settings.def("SetEnableSnowFall", &CClientConfig::SetEnableSnowFall);
    settings.def("IsShowMobLevel", &CClientConfig::IsShowMobLevel);
    settings.def("IsShowAcce", &CClientConfig::IsShowAcce);
    settings.def("SetShowAcce", &CClientConfig::SetShowAcce);
    settings.def("IsShowCostume", &CClientConfig::IsShowCostume);
    settings.def("SetShowCostume", &CClientConfig::SetShowCostume);
    settings.def("IsShowSpecular", &CClientConfig::IsShowSpecular);
    settings.def("GetResolution", &CClientConfig::GetResolution);
    settings.def("IsShowGeneralEffects", &CClientConfig::IsShowGeneralEffects);
    settings.def("SetShowGeneralEffects", &CClientConfig::SetShowGeneralEffects);
    settings.def("IsShowBuffEffects", &CClientConfig::IsShowBuffEffects);
    settings.def("SetShowBuffEffects", &CClientConfig::SetShowBuffEffects);
    settings.def("IsShowSkillEffects", &CClientConfig::IsShowSkillEffects);
    settings.def("SetShowSkillEffects", &CClientConfig::SetShowSkillEffects);
    settings.def("IsUsingItemHighlight", &CClientConfig::IsUsingItemHighlight);
    settings.def("SetUsingItemHighlight", &CClientConfig::SetUsingItemHighlight);
    settings.def("GetResolutionCount", &CClientConfig::GetResolutionCount);
    settings.def("SetResolution", &CClientConfig::SetResolution);
    settings.def("IsResolutionByDescription", &CClientConfig::IsResolutionByDescription);
    settings.def("GetMusicVolume", &CClientConfig::GetMusicVolume);
    settings.def("GetSoundVolume", &CClientConfig::GetSoundVolume);
    settings.def("SetMusicVolume", &CClientConfig::SetMusicVolume);
    settings.def("SetSoundVolumef", &CClientConfig::SetSoundVolumef);
    settings.def("GetViewDistance", &CClientConfig::GetViewDistance);
    settings.def("SetViewDistance", &CClientConfig::SetViewDistance);
    settings.def("GetShadowLevel", &CClientConfig::GetShadowLevel);
    settings.def("SetShadowLevel", &CClientConfig::SetShadowLevel);
    settings.def("GetCameraDistanceMode", &CClientConfig::GetCameraDistanceMode);
    settings.def("SetCameraDistanceMode", &CClientConfig::SetCameraDistanceMode);
    settings.def("GetNearbyShopsDisplayed", &CClientConfig::GetNearbyShopsDisplayed);
    settings.def("SetNearbyShopsDisplayed", &CClientConfig::SetNearbyShopsDisplayed);
    settings.def("SetShowSpecular", &SetShowSpecular);
    settings.def("IsShowFPS", &CClientConfig::IsShowFPS);
    settings.def("SetShowFPS", &CClientConfig::SetShowFPS);
    settings.def("GetPickupIgnore", &CClientConfig::GetPickupIgnore);
    settings.def("SetPickupIgnore", &CClientConfig::SetPickupIgnore);
    settings.def("IsShowGMNotifications", &CClientConfig::IsShowGMNotifications);
    settings.def("SetShowGMNotifications", &CClientConfig::SetShowGMNotifications);
    settings.def("IsShowFriendNotifications", &CClientConfig::IsShowFriendNotifications);
    settings.def("SetShowFriendNotifications", &CClientConfig::SetShowFriendNotifications);
    settings.def("IsShowGuildNotifications", &CClientConfig::IsShowGuildNotifications);
    settings.def("SetShowGuildNotifications", &CClientConfig::SetShowGuildNotifications);
#ifdef ENABLE_PERSPECTIVE_VIEW
    settings.def("SetFieldPerspective", &CClientConfig::SetFieldPerspective);
    settings.def("GetFieldPerspective", &CClientConfig::GetFieldPerspective);
#endif

    py::enum_<CClientConfig::EPickupIgnores>(app, "PickupIgnore")
        .value("Sword", CClientConfig::PICKUP_IGNORE_SWORD)
        .value("Dagger", CClientConfig::PICKUP_IGNORE_DAGGER)
        .value("Bow", CClientConfig::PICKUP_IGNORE_BOW)
        .value("TwoHand", CClientConfig::PICKUP_IGNORE_TWO_HAND)
        .value("Bell", CClientConfig::PICKUP_IGNORE_BELL)
        .value("Fan", CClientConfig::PICKUP_IGNORE_FAN)
        .value("Armor", CClientConfig::PICKUP_IGNORE_ARMOR)
        .value("Head", CClientConfig::PICKUP_IGNORE_HEAD)
        .value("Shield", CClientConfig::PICKUP_IGNORE_SHIELD)
        .value("Wrist", CClientConfig::PICKUP_IGNORE_WRIST)
        .value("Foots", CClientConfig::PICKUP_IGNORE_FOOTS)
        .value("Necklace", CClientConfig::PICKUP_IGNORE_NECK)
        .value("Ear", CClientConfig::PICKUP_IGNORE_EAR)
        .value("Etc", CClientConfig::PICKUP_IGNORE_ETC)
        .value("Ring", CClientConfig::PICKUP_IGNORE_RING)
        .value("Talisman", CClientConfig::PICKUP_IGNORE_TALISMAN)
        .value("Elixir", CClientConfig::PICKUP_IGNORE_ELIXIR)
        .export_values();


    //py::class_<CPythonApplication::SCameraSetting>(app, "SCameraSetting")
    //.def_readwrite("v3CenterPosition", &CPythonApplication::SCameraSetting::v3CenterPosition)
    //.def_readwrite("kCmrPos", &CPythonApplication::SCameraSetting::kCmrPos)
    //.def_readwrite("fRotation", &CPythonApplication::SCameraSetting::fRotation)
    //.def_readwrite("fPitch", &CPythonApplication::SCameraSetting::fPitch)
    //.def_readwrite("fZoom", &CPythonApplication::SCameraSetting::fZoom)

    //;

    // TODO: We want to completely get rid of the old module
    py::class_<CPythonApplication>(app, "appInst")
        .def_static("instance", &CPythonApplication::AppInst, pybind11::return_value_policy::reference)
        .def("RenderGame", &CPythonApplication::RenderGame)
        .def("UpdateGame", &CPythonApplication::UpdateGame)
        .def("Loop", &CPythonApplication::Loop)
        .def("Process", &CPythonApplication::Process)
        .def("Exit", &CPythonApplication::Exit)
        .def("FlashApplication", &CPythonApplication::FlashApplication)
        .def("Abort", &CPythonApplication::Abort)
        .def("GetNet", &CPythonApplication::GetNetworkStream, py::return_value_policy::reference)
        .def("GetSettings", &GetClientConfig, py::return_value_policy::reference);

    app.def("LoadLocaleData", &LoadLocaleData);
    app.def("LoadGameData", &LoadGameData);
    app.def("RegisterSkill", &RegisterSkill);
}
