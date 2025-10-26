#include "StdAfx.h"
#include "../EterLib/Engine.hpp"
#include "PythonBackground.h"
#include "../eterlib/StateManager.h"
#include "../gamelib/MapOutDoor.h"
#include <pybind11/stl.h>

PyObject *backgroundIsSoftwareTiling(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    return Py_BuildValue("i", 0);
}

PyObject *backgroundEnableSoftwareTiling(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *backgroundEnableSnow(PyObject *poSelf, PyObject *poArgs)
{
    int nIsEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nIsEnable))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();

    if (rkBG.IsNoSnowMap(rkBG.GetWarpMapName()))
    {
        rkBG.DisableSnowEnvironment();
        rkBG.RefreshTexture(false);
        Py_RETURN_NONE;
    }

    rkBG.SetXMasShowEvent(nIsEnable);
    if (nIsEnable)
    {
        if (rkBG.IsSnowModeOption())
            rkBG.EnableSnowEnvironment();
        if (rkBG.IsSnowTextureModeOption())
            rkBG.RefreshTexture(nIsEnable);
    }
    else
    {
        rkBG.DisableSnowEnvironment();
        rkBG.RefreshTexture(nIsEnable);
    }

    Py_RETURN_NONE;
}

PyObject *backgroundLoadMap(PyObject *poSelf, PyObject *poArgs)
{
    std::string pszMapPathName;

    if (!PyTuple_GetString(poArgs, 0, pszMapPathName))
        return Py_BadArgument();

    float x, y, z;

    if (!PyTuple_GetFloat(poArgs, 1, &x))
        return Py_BadArgument();

    if (!PyTuple_GetFloat(poArgs, 2, &y))
        return Py_BadArgument();

    if (!PyTuple_GetFloat(poArgs, 3, &z))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.LoadMap(0, pszMapPathName, x, y, z);

    //#ifdef _DEBUG
    //	CMapOutdoor& rkMap=rkBG.GetMapOutdoorRef();
    //	rkMap.EnablePortal(TRUE);
    //	rkBG.EnableTerrainOnlyForHeight();
    //#endif

    Py_RETURN_NONE;
}

PyObject *backgroundDestroy(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetShadowLevel(CPythonBackground::SHADOW_NONE);
    rkBG.Destroy();
    Py_RETURN_NONE;
}

PyObject *backgroundRegisterEnvironmentData(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    std::string pszEnvironmentFileName;
    if (!PyTuple_GetString(poArgs, 1, pszEnvironmentFileName))
        return Py_BadArgument();

    auto &bg = CPythonBackground::Instance();
    if (!bg.IsMapReady())
        Py_RETURN_NONE;

    auto &map = bg.GetMapOutdoorRef();
    if (!map.RegisterEnvironment(iIndex, pszEnvironmentFileName.c_str()))
    {
        SPDLOG_ERROR(
            "RegisterEnvironmentData(iIndex={0}, szEnvironmentFileName={1})",
            iIndex, pszEnvironmentFileName);
    }

    Py_RETURN_NONE;
}

PyObject *backgroundSetEnvironmentData(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;

    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    auto &bg = CPythonBackground::Instance();
    if (!bg.IsMapReady())
        Py_RETURN_NONE;

    if (iIndex == CPythonBackground::DAY_MODE_DARK)
        bg.ChangeToNight();
    else
        bg.ChangeToDay();

    if (!bg.GetNightOption() && iIndex == CPythonBackground::DAY_MODE_DARK)
        Py_RETURN_NONE;

    if (bg.IsNoBoomMap(bg.GetWarpMapName()))
        Py_RETURN_NONE;

    auto &map = bg.GetMapOutdoorRef();

    auto env = map.GetEnvironment(iIndex);
    if (env)
    {
        map.SetActiveEnvironment(env);
    }

    Py_RETURN_NONE;
}

PyObject *backgroundGetCurrentMapName(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    return Py_BuildValue("s", rkBG.GetWarpMapName());
}

PyObject *backgroundGetPickingPoint(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    TPixelPosition kPPosPicked(0.0f, 0.0f, 0.0f);
    if (rkBG.GetPickingPoint(&kPPosPicked))
    {
        kPPosPicked.y = -kPPosPicked.y;
    }
    return Py_BuildValue("fff", kPPosPicked.x, kPPosPicked.y, kPPosPicked.z);
}

PyObject *backgroundSetCharacterDirLight(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetCharacterDirLight();
    Py_RETURN_NONE;
}

PyObject *backgroundSetBackgroundDirLight(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetBackgroundDirLight();
    Py_RETURN_NONE;
}

PyObject *backgroundInitialize(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.Create();
    Py_RETURN_NONE;
}

PyObject *backgroundUpdate(PyObject *poSelf, PyObject *poArgs)
{
    float fCameraX;
    if (!PyTuple_GetFloat(poArgs, 0, &fCameraX))
        return Py_BadArgument();

    float fCameraY;
    if (!PyTuple_GetFloat(poArgs, 1, &fCameraY))
        return Py_BadArgument();

    float fCameraZ;
    if (!PyTuple_GetFloat(poArgs, 2, &fCameraZ))
        return Py_BadArgument();

    CPythonBackground::Instance().Update(fCameraX, fCameraY, fCameraZ);
    Py_RETURN_NONE;
}

PyObject *backgroundRender(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().Render();
    Py_RETURN_NONE;
}

PyObject *backgroundRenderCollision(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().RenderCollision();
    Py_RETURN_NONE;
}

PyObject *backgroundRenderCharacterShadowToTexture(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().RenderCharacterShadowToTexture();
    Py_RETURN_NONE;
}

PyObject *backgroundRenderDungeon(PyObject *poSelf, PyObject *poArgs)
{
    assert(!"background.RenderDungeon() - 사용하지 않는 함수입니다 - [levites]");
    Py_RETURN_NONE;
}

PyObject *backgroundGetHeight(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
        return Py_BadArgument();

    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
        return Py_BadArgument();

    float fz = CPythonBackground::Instance().GetHeight(fx, fy);
    return Py_BuildValue("f", fz);
}

PyObject *backgroundGetRenderedSplatNum(PyObject *poSelf, PyObject *poArgs)
{
    int iPatch;
    int iSplat;
    float fSplatRatio;

    std::vector<int> &aTextureNumVector = CPythonBackground::Instance().GetRenderedSplatNum(
        &iPatch, &iSplat, &fSplatRatio);

    char szOutput[MAX_PATH] = "";
    int iOutput = 0;
    for (std::vector<int>::iterator it = aTextureNumVector.begin(); it != aTextureNumVector.end(); it++)
    {
        iOutput += _snprintf(szOutput + iOutput, sizeof(szOutput) - iOutput, "%d ", *it);
    }
    //std::copy(aTextureNumVector.begin(),aTextureNumVector.end(),std::ostream_iterator<int>(ostr," "));

    return Py_BuildValue("iifs", iPatch, iSplat, fSplatRatio, szOutput);
}

PyObject *backgroundGetRenderShadowTime(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    return Py_BuildValue("i", rkBG.GetRenderShadowTime());
}

PyObject *backgroundGetShadowMapcolor(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
        return Py_BadArgument();

    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
        return Py_BadArgument();

    uint32_t dwColor = CPythonBackground::Instance().GetShadowMapColor(fx, fy);
    return Py_BuildValue("i", dwColor);
}

PyObject *backgroundSetShadowLevel(PyObject *poSelf, PyObject *poArgs)
{
    int iLevel;

    if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetShadowLevel(iLevel);
    Py_RETURN_NONE;
}

PyObject *backgroundSetVisiblePart(PyObject *poSelf, PyObject *poArgs)
{
    int ePart;
    if (!PyTuple_GetInteger(poArgs, 0, &ePart))
        return Py_BadArgument();

    int isVisible;
    if (!PyTuple_GetInteger(poArgs, 1, &isVisible))
        return Py_BadArgument();

    if (ePart >= CMapOutdoor::PART_NUM)
        return Py_BuildException("ePart(%d)<background.PART_NUM(%d)", ePart, CMapOutdoor::PART_NUM);

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetVisiblePart(ePart, isVisible ? true : false);

    Py_RETURN_NONE;
}

PyObject *backgroundSetSpaltLimit(PyObject *poSelf, PyObject *poArgs)
{
    int iSplatNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iSplatNum))
        return Py_BadArgument();

    if (iSplatNum < 0)
        return Py_BuildException("background.SetSplatLimit(iSplatNum(%d)>=0)", iSplatNum);

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetSplatLimit(iSplatNum);

    Py_RETURN_NONE;
}

PyObject *backgroundSelectViewDistanceNum(PyObject *poSelf, PyObject *poArgs)
{
    int iNum;

    if (!PyTuple_GetInteger(poArgs, 0, &iNum))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SelectViewDistanceNum(iNum);

    Py_RETURN_NONE;
}

PyObject *backgroundSetViewDistanceSet(PyObject *poSelf, PyObject *poArgs)
{
    int iNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iNum))
        return Py_BadArgument();

    float fFarClip;
    if (!PyTuple_GetFloat(poArgs, 1, &fFarClip))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.SetViewDistanceSet(iNum, fFarClip);
    Py_RETURN_NONE;
}

PyObject *backgroundGetFarClip(PyObject *poSelf, PyObject *poArgs)
{
    float fFarClip = CPythonBackground::Instance().GetFarClip();
    return Py_BuildValue("f", fFarClip);
}

PyObject *backgroundGetDistanceSetInfo(PyObject *poSelf, PyObject *poArgs)
{
    int iNum;
    float fStart, fEnd, fFarClip;
    CPythonBackground::Instance().GetDistanceSetInfo(&iNum, &fStart, &fEnd, &fFarClip);
    return Py_BuildValue("ifff", iNum, fStart, fEnd, fFarClip);
}

PyObject *backgroundSetBGLoading(PyObject *poSelf, PyObject *poArgs)
{
    bool bBGLoading;
    if (!PyTuple_GetBoolean(poArgs, 0, &bBGLoading))
        return Py_BadArgument();

    //CPythonBackground::Instance().BGLoadingEnable(bBGLoading);
    Py_RETURN_NONE;
}

PyObject *backgroundSetRenderSort(PyObject *poSelf, PyObject *poArgs)
{
    int eSort;
    if (!PyTuple_GetInteger(poArgs, 0, &eSort))
        return Py_BadArgument();

    CPythonBackground::Instance().SetTerrainRenderSort((CMapOutdoor::ETerrainRenderSort)eSort);
    Py_RETURN_NONE;
}

PyObject *backgroundSetTransparentTree(PyObject *poSelf, PyObject *poArgs)
{
    int bTransparent;
    if (!PyTuple_GetInteger(poArgs, 0, &bTransparent))
        return Py_BadArgument();

    CPythonBackground::Instance().SetTransparentTree(bTransparent ? true : false);
    Py_RETURN_NONE;
}

PyObject *backgroundGetMapInfo(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    auto info = rkBG.GetMapInfo(index);

    if (!info)
        return Py_BuildValue("s", "");
    else
        return Py_BuildValue("s", info->mapName.c_str());
}

PyObject *backgroundGetCurrentMapMusic(PyObject *poSelf, PyObject *poArgs)
{
    auto &rkBG = CPythonBackground::Instance();

    const auto mapInfo = rkBG.GetCurrentMapInfo();
    if (!mapInfo)
    {
        return Py_BuildValue("s", "");
    }

    return Py_BuildValue("s", mapInfo->fieldMusicFile.c_str());
}

PyObject *backgroundGetCurrentMapMusicVolume(PyObject *poSelf, PyObject *poArgs)
{
    auto &rkBG = CPythonBackground::Instance();

    const auto mapInfo = rkBG.GetCurrentMapInfo();
    if (!mapInfo)
    {
        return Py_BuildValue("f", 1.0f);
    }

    return Py_BuildValue("f", mapInfo->fieldMusicVolume);
}

PyObject *backgroundGetCurrentMapIndex(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonBackground::Instance().GetMapIndex());
}

PyObject *backgroundGetMapInfoCount(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    return Py_BuildValue("i", rkBG.GetMapInfoCount());
}

PyObject *backgroundIsMapInfoByMapName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BadArgument();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    return Py_BuildValue("i", rkBG.IsMapInfoByName(szName.c_str()));
}

PyObject *backgroundWarpTest(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
        return Py_BadArgument();

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
        return Py_BadArgument();

    CPythonBackground::Instance().Warp(index, x * 100, y * 100);
    Py_RETURN_NONE;
}

PyObject *backgroundSetXMasTree(PyObject *poSelf, PyObject *poArgs)
{
    int iGrade;
    if (!PyTuple_GetInteger(poArgs, 0, &iGrade))
        return Py_BadArgument();

    CPythonBackground::Instance().SetXMaxTree(iGrade);
    Py_RETURN_NONE;
}

PyObject *backgroundRegisterDungeonMapName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BadArgument();

    CPythonBackground::Instance().RegisterDungeonMapName(szName.c_str());
    Py_RETURN_NONE;
}

PyObject *backgroundVisibleGuildArea(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().VisibleGuildArea();
    Py_RETURN_NONE;
}

PyObject *backgroundDisableGuildArea(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().DisableGuildArea();
    Py_RETURN_NONE;
}

PyObject *backgroundCreatePrivateShopPos(PyObject *poSelf, PyObject *poArgs)
{
    int chr_vid;
    if (!PyTuple_GetInteger(poArgs, 0, &chr_vid))
    {
        return Py_BadArgument();
    }

    CPythonBackground::Instance().CreatePrivateShopPos(0, chr_vid);
    Py_RETURN_NONE;
}

PyObject *backgroundDeletePrivateShopPos(PyObject *poSelf, PyObject *poArgs)
{
    CPythonBackground::Instance().DeletePrivateShopPos(0);
    Py_RETURN_NONE;
}

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
PyObject *backgroundSetXMasShowEvent(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CPythonBackground::instance().SetXMasShowEvent(iFlag);

    Py_RETURN_NONE;
}

PyObject *backgroundSetSnowTextureModeOption(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CPythonBackground::instance().SetSnowTextureModeOption(iFlag);

    Py_RETURN_NONE;
}

PyObject *backgroundSetSnowModeOption(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CPythonBackground::instance().SetSnowModeOption(iFlag);

    Py_RETURN_NONE;
}

PyObject *backgroundEnableSnowMode(PyObject *poSelf, PyObject *poArgs)
{
    int nEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
        return Py_BadArgument();

    if (nEnable && CPythonBackground::instance().IsXmasShow() && !CPythonBackground::instance().IsNoSnowMap(
            CPythonBackground::instance().GetWarpMapName()))
        CPythonBackground::instance().EnableSnowEnvironment();
    else
        CPythonBackground::instance().DisableSnowEnvironment();

    Py_RETURN_NONE;
}

PyObject *backgroundSetNightOption(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CPythonBackground::instance().SetNightOption(iFlag);

    Py_RETURN_NONE;
}

PyObject *backgroundChangeEnvironmentData(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    auto &bg = CPythonBackground::Instance();
    if (!bg.IsMapReady())
        Py_RETURN_NONE;

    auto &map = bg.GetMapOutdoorRef();

    auto env = map.GetEnvironment(iFlag);
    if (env)
        map.SetActiveEnvironment(env);

    Py_RETURN_NONE;
}

PyObject *backgroundEnableSnowTextureMode(PyObject *poSelf, PyObject *poArgs)
{
    if (CPythonBackground::instance().IsXmasShow() && CPythonBackground::instance().IsSnowTextureModeOption() && !
        CPythonBackground::instance().IsNoSnowMap(CPythonBackground::instance().GetWarpMapName()))
        CPythonBackground::instance().RefreshTexture(CPythonBackground::instance().IsSnowTextureModeOption() != 0);
    else
        CPythonBackground::instance().RefreshTexture(false);
    Py_RETURN_NONE;
}

PyObject *backgroundIsBoomMap(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue(
        "i", !CPythonBackground::instance().IsNoBoomMap(CPythonBackground::instance().GetWarpMapName()));
}

PyObject *backgroundGetDayMode(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonBackground::instance().GetDayMode());
}

#endif

PyObject *backgroundSetFogMode(PyObject *poSelf, PyObject *poArgs)
{
    bool iFlag;
    if (!PyTuple_GetBoolean(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CPythonBackground::instance().SetFogEnable(iFlag);

    Engine::GetSettings().SetFogMode(iFlag);

    Py_RETURN_NONE;
}

extern "C" void initbackground()
{
    static PyMethodDef s_methods[] =
    {
        {"EnableSnow", backgroundEnableSnow, METH_VARARGS},
        {"GetMapInfo", backgroundGetMapInfo, METH_VARARGS},
        {"GetRenderShadowTime", backgroundGetRenderShadowTime, METH_VARARGS},
        {"LoadMap", backgroundLoadMap, METH_VARARGS},
        {"Destroy", backgroundDestroy, METH_VARARGS},
        {"RegisterEnvironmentData", backgroundRegisterEnvironmentData, METH_VARARGS},
        {"SetEnvironmentData", backgroundSetEnvironmentData, METH_VARARGS},
        {"GetCurrentMapName", backgroundGetCurrentMapName, METH_VARARGS},
        {"GetPickingPoint", backgroundGetPickingPoint, METH_VARARGS},

        {"SetCharacterDirLight", backgroundSetCharacterDirLight, METH_VARARGS},
        {"SetBackgroundDirLight", backgroundSetBackgroundDirLight, METH_VARARGS},

        {"Initialize", backgroundInitialize, METH_VARARGS},
        {"Update", backgroundUpdate, METH_VARARGS},
        {"Render", backgroundRender, METH_VARARGS},
        {"RenderCollision", backgroundRenderCollision, METH_VARARGS},
        {"RenderCharacterShadowToTexture", backgroundRenderCharacterShadowToTexture, METH_VARARGS},
        {"GetHeight", backgroundGetHeight, METH_VARARGS},

        {"SetShadowLevel", backgroundSetShadowLevel, METH_VARARGS},

        {"SetVisiblePart", backgroundSetVisiblePart, METH_VARARGS},
        {"GetShadowMapColor", backgroundGetShadowMapcolor, METH_VARARGS},
        {"SetSplatLimit", backgroundSetSpaltLimit, METH_VARARGS},
        {"GetRenderedSplatNum", backgroundGetRenderedSplatNum, METH_VARARGS},
        {"SelectViewDistanceNum", backgroundSelectViewDistanceNum, METH_VARARGS},
        {"SetViewDistanceSet", backgroundSetViewDistanceSet, METH_VARARGS},
        {"GetFarClip", backgroundGetFarClip, METH_VARARGS},
        {"GetDistanceSetInfo", backgroundGetDistanceSetInfo, METH_VARARGS},
        {"SetBGLoading", backgroundSetBGLoading, METH_VARARGS},
        {"SetRenderSort", backgroundSetRenderSort, METH_VARARGS},
        {"SetTransparentTree", backgroundSetTransparentTree, METH_VARARGS},
        {"SetXMasTree", backgroundSetXMasTree, METH_VARARGS},
        {"RegisterDungeonMapName", backgroundRegisterDungeonMapName, METH_VARARGS},

        {"VisibleGuildArea", backgroundVisibleGuildArea, METH_VARARGS},
        {"DisableGuildArea", backgroundDisableGuildArea, METH_VARARGS},
        {"IsMapInfoByMapName", backgroundIsMapInfoByMapName, METH_VARARGS},

        {"WarpTest", backgroundWarpTest, METH_VARARGS},

        {"SetXMasShowEvent", backgroundSetXMasShowEvent, METH_VARARGS},
        {"SetSnowModeOption", backgroundSetSnowModeOption, METH_VARARGS},
        {"EnableSnowMode", backgroundEnableSnowMode, METH_VARARGS},
        {"SetNightModeOption", backgroundSetNightOption, METH_VARARGS},
        {"GetDayMode", backgroundGetDayMode, METH_VARARGS},
        {"ChangeEnvironmentData", backgroundChangeEnvironmentData, METH_VARARGS},
        {"SetSnowTextureModeOption", backgroundSetSnowTextureModeOption, METH_VARARGS},
        {"EnableSnowTextureMode", backgroundEnableSnowTextureMode, METH_VARARGS},
        {"IsBoomMap", backgroundIsBoomMap, METH_VARARGS},
        {"SetFogMode", backgroundSetFogMode, METH_VARARGS},
        {"GetMapInfoCount", backgroundGetMapInfoCount, METH_VARARGS},

        {"CreatePrivateShopPos", backgroundCreatePrivateShopPos, METH_VARARGS},
        {"DeletePrivateShopPos", backgroundDeletePrivateShopPos, METH_VARARGS},

        {"GetCurrentMapMusic", backgroundGetCurrentMapMusic, METH_VARARGS},
        {"GetCurrentMapMusicVolume", backgroundGetCurrentMapMusicVolume, METH_VARARGS},
        {"GetCurrentMapIndex", backgroundGetCurrentMapIndex, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("background", s_methods);

    PyModule_AddIntConstant(poModule, "PART_SKY", CMapOutdoor::PART_SKY);
    PyModule_AddIntConstant(poModule, "PART_TREE", CMapOutdoor::PART_TREE);
    PyModule_AddIntConstant(poModule, "PART_CLOUD", CMapOutdoor::PART_CLOUD);
    PyModule_AddIntConstant(poModule, "PART_WATER", CMapOutdoor::PART_WATER);
    PyModule_AddIntConstant(poModule, "PART_OBJECT", CMapOutdoor::PART_OBJECT);
    PyModule_AddIntConstant(poModule, "PART_TERRAIN", CMapOutdoor::PART_TERRAIN);
    PyModule_AddIntConstant(poModule, "PART_AREA_EFFECT", CMapOutdoor::PART_AREA_EFFECT);

    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DEFAULT", CSkyObject::SKY_RENDER_MODE_DEFAULT);
    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DIFFUSE", CSkyObject::SKY_RENDER_MODE_DIFFUSE);
    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_TEXTURE", CSkyObject::SKY_RENDER_MODE_TEXTURE);
    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE", CSkyObject::SKY_RENDER_MODE_MODULATE);
    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE2X", CSkyObject::SKY_RENDER_MODE_MODULATE2X);
    PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE4X", CSkyObject::SKY_RENDER_MODE_MODULATE4X);

    PyModule_AddIntConstant(poModule, "SHADOW_NONE", CPythonBackground::SHADOW_NONE);
    PyModule_AddIntConstant(poModule, "SHADOW_GROUND", CPythonBackground::SHADOW_GROUND);
    PyModule_AddIntConstant(poModule, "SHADOW_GROUND_AND_SOLO", CPythonBackground::SHADOW_GROUND_AND_SOLO);
    PyModule_AddIntConstant(poModule, "SHADOW_ALL", CPythonBackground::SHADOW_ALL);
    PyModule_AddIntConstant(poModule, "SHADOW_ALL_HIGH", CPythonBackground::SHADOW_ALL_HIGH);
    PyModule_AddIntConstant(poModule, "SHADOW_ALL_MAX", CPythonBackground::SHADOW_ALL_MAX);

    PyModule_AddIntConstant(poModule, "DISTANCE0", CPythonBackground::DISTANCE0);
    PyModule_AddIntConstant(poModule, "DISTANCE1", CPythonBackground::DISTANCE1);
    PyModule_AddIntConstant(poModule, "DISTANCE2", CPythonBackground::DISTANCE2);
    PyModule_AddIntConstant(poModule, "DISTANCE3", CPythonBackground::DISTANCE3);
    PyModule_AddIntConstant(poModule, "DISTANCE4", CPythonBackground::DISTANCE4);

    PyModule_AddIntConstant(poModule, "DISTANCE_SORT", CMapOutdoor::DISTANCE_SORT);
    PyModule_AddIntConstant(poModule, "TEXTURE_SORT", CMapOutdoor::TEXTURE_SORT);

    PyModule_AddIntConstant(poModule, "DAY_MODE_LIGHT", CPythonBackground::DAY_MODE_LIGHT);
    PyModule_AddIntConstant(poModule, "DAY_MODE_DARK", CPythonBackground::DAY_MODE_DARK);
}

void init_background(py::module &m) {

    py::module bg = m.def_submodule("background", "");

    py::class_<CMapManager::SMapInfo>(bg, "MapInfo")
        .def(py::init<>())
        .def_readwrite("mapName", &CMapManager::SMapInfo::mapName)
        .def_readwrite("fieldMusicFile", &CMapManager::SMapInfo::fieldMusicFile)
        .def_readwrite("fieldMusicVolume", &CMapManager::SMapInfo::fieldMusicVolume)
        .def_readwrite("mapTranslation", &CMapManager::SMapInfo::mapTranslation)
        .def_readwrite("sizeX", &CMapManager::SMapInfo::sizeX)
        .def_readwrite("sizeY", &CMapManager::SMapInfo::sizeY);

    auto cpb = py::class_<CPythonBackground, std::unique_ptr<CPythonBackground, py::nodelete>>(bg, "bgInst")
                   .def(py::init([]() {
                            return std::unique_ptr<CPythonBackground, py::nodelete>(CPythonBackground::InstancePtr());
                        }),
                        pybind11::return_value_policy::reference_internal);

    cpb.def("GetMapInfo", &CPythonBackground::GetMapInfo, pybind11::return_value_policy::reference);
    cpb.def("GetMapInfoByName", &CPythonBackground::GetMapInfoByName, pybind11::return_value_policy::reference);
    cpb.def("GetCurrentMapInfo", &CPythonBackground::GetCurrentMapInfo, pybind11::return_value_policy::reference);
    cpb.def("GetMapIndex", &CPythonBackground::GetMapIndex);


}