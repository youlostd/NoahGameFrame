#include "StdAfx.h"
#include "PythonMiniMap.h"

PyObject *miniMapSetScale(PyObject *poSelf, PyObject *poArgs)
{
    float fScale;
    if (!PyTuple_GetFloat(poArgs, 0, &fScale))
        return Py_BuildException();

    CPythonMiniMap::Instance().SetScale(fScale);
    Py_RETURN_NONE;
}

PyObject *miniMapSetCenterPosition(PyObject *poSelf, PyObject *poArgs)
{
    float fCenterX;
    if (!PyTuple_GetFloat(poArgs, 0, &fCenterX))
        return Py_BuildException();

    float fCenterY;
    if (!PyTuple_GetFloat(poArgs, 1, &fCenterY))
        return Py_BuildException();

    CPythonMiniMap::Instance().SetCenterPosition(fCenterX, fCenterY);
    Py_RETURN_NONE;
}

PyObject *miniMapSetMiniMapSize(PyObject *poSelf, PyObject *poArgs)
{
    float fWidth;
    if (!PyTuple_GetFloat(poArgs, 0, &fWidth))
        return Py_BuildException();

    float fHeight;
    if (!PyTuple_GetFloat(poArgs, 1, &fHeight))
        return Py_BuildException();

    CPythonMiniMap::Instance().SetMiniMapSize(fWidth, fHeight);
    Py_RETURN_NONE;
}

PyObject *miniMapDestroy(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().Destroy();
    Py_RETURN_NONE;
}

PyObject *miniMapCreate(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().Create();
    Py_RETURN_NONE;
}

PyObject *miniMapUpdate(PyObject *poSelf, PyObject *poArgs)
{
    float fCenterX;
    if (!PyTuple_GetFloat(poArgs, 0, &fCenterX))
        return Py_BuildException();

    float fCenterY;
    if (!PyTuple_GetFloat(poArgs, 1, &fCenterY))
        return Py_BuildException();

    CPythonMiniMap::Instance().Update(fCenterX, fCenterY);
    Py_RETURN_NONE;
}

PyObject *miniMapRender(PyObject *poSelf, PyObject *poArgs)
{
    float fScrrenX;
    if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
        return Py_BuildException();

    float fScrrenY;
    if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
        return Py_BuildException();

    CPythonMiniMap::Instance().Render(fScrrenX, fScrrenY);
    Py_RETURN_NONE;
}

PyObject *miniMapShow(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().Show();
    Py_RETURN_NONE;
}

PyObject *miniMapHide(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().Hide();
    Py_RETURN_NONE;
}

PyObject *miniMapisShow(PyObject *poSelf, PyObject *poArgs)
{
    bool bShow = CPythonMiniMap::Instance().CanShow();
    return Py_BuildValue("b", bShow);
}

PyObject *miniMapToggleAtlasMarker(PyObject *poSelf, PyObject *poArgs)
{
    int type;
    if (!PyTuple_GetInteger(poArgs, 0, &type))
        return Py_BuildException();

    bool returnValue = CPythonMiniMap::Instance().ToggleAtlasMarker(type);
    return Py_BuildValue("i", (int)returnValue);
}

PyObject *miniMapScaleUp(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().ScaleUp();
    Py_RETURN_NONE;
}

PyObject *miniMapScaleDown(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().ScaleDown();
    Py_RETURN_NONE;
}

PyObject *miniMapGetInfo(PyObject *poSelf, PyObject *poArgs)
{
    float fScrrenX;
    if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
        return Py_BuildException();

    float fScrrenY;
    if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
        return Py_BuildException();

    std::string aString;
    float fPosX, fPosY;
    uint32_t dwTextColor;
    bool bFind = CPythonMiniMap::Instance().GetPickedInstanceInfo(fScrrenX, fScrrenY, aString, &fPosX, &fPosY,
                                                                  &dwTextColor);
    int iPosX, iPosY;
    PR_FLOAT_TO_INT(fPosX, iPosX);
    PR_FLOAT_TO_INT(fPosY, iPosY);
    iPosX /= 100;
    iPosY /= 100;
    return Py_BuildValue("isiil", (int)bFind, aString.c_str(), iPosX, iPosY, (signed)dwTextColor);
}

//////////////////////////////////////////////////////////////////////////
// Atlas
PyObject *miniMapLoadAtlas(PyObject *poSelf, PyObject *poArgs)
{
    if (!CPythonMiniMap::Instance().LoadAtlas())
        SPDLOG_ERROR("CPythonMiniMap::Instance().LoadAtlas() Failed");
    Py_RETURN_NONE;
}

PyObject *miniMapUpdateAtlas(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().UpdateAtlas();
    Py_RETURN_NONE;
}

PyObject *miniMapRenderAtlas(PyObject *poSelf, PyObject *poArgs)
{
    float fScrrenX;
    if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
        return Py_BuildException();

    float fScrrenY;
    if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
        return Py_BuildException();

    CPythonMiniMap::Instance().RenderAtlas(fScrrenX, fScrrenY);
    Py_RETURN_NONE;
}

PyObject *miniMapShowAtlas(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().ShowAtlas();
    Py_RETURN_NONE;
}

PyObject *miniMapHideAtlas(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().HideAtlas();
    Py_RETURN_NONE;
}

PyObject *miniMapisShowAtlas(PyObject *poSelf, PyObject *poArgs)
{
    bool bShow = CPythonMiniMap::Instance().CanShowAtlas();
    return Py_BuildValue("b", bShow);
}

PyObject *miniMapIsAtlas(PyObject *poSelf, PyObject *poArgs)
{
    bool isData = CPythonMiniMap::Instance().IsAtlas();
    return Py_BuildValue("b", isData);
}

PyObject *miniMapGetAtlasInfo(PyObject *poSelf, PyObject *poArgs)
{
    float fScrrenX;
    if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
        return Py_BuildException();
    float fScrrenY;
    if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
        return Py_BuildException();

    std::string aString = "";
    float fPosX = 0.0f;
    float fPosY = 0.0f;
    uint32_t dwTextColor = 0;
    uint32_t dwGuildID = 0;
    bool bFind = CPythonMiniMap::Instance().GetAtlasInfo(fScrrenX, fScrrenY, aString, &fPosX, &fPosY, &dwTextColor,
                                                         &dwGuildID);
    int iPosX, iPosY;
    PR_FLOAT_TO_INT(fPosX, iPosX);
    PR_FLOAT_TO_INT(fPosY, iPosY);
    iPosX /= 100;
    iPosY /= 100;
    return Py_BuildValue("isiili", (int)bFind, aString.c_str(), iPosX, iPosY, (signed)dwTextColor, dwGuildID);
}

PyObject *miniMapGetAtlasSize(PyObject *poSelf, PyObject *poArgs)
{
    float fSizeX, fSizeY;
    bool bGet = CPythonMiniMap::Instance().GetAtlasSize(&fSizeX, &fSizeY);

    /*
        float fSizeXoo256 = fSizeX / 256.0f;
        float fSizeYoo256 = fSizeY / 256.0f;
    
        if (fSizeXoo256 >= fSizeYoo256)
        {
            fSizeX /= fSizeYoo256;
            fSizeY = 256.0f;
        }
        else
        {
            fSizeX = 256.0f;
            fSizeY /= fSizeXoo256;
        }
    */

    int iSizeX, iSizeY;
    PR_FLOAT_TO_INT(fSizeX, iSizeX);
    PR_FLOAT_TO_INT(fSizeY, iSizeY);

    return Py_BuildValue("bii", (int)bGet, iSizeX, iSizeY);
}

PyObject *miniMapAddWayPoint(PyObject *poSelf, PyObject *poArgs)
{
    int iID;
    if (!PyTuple_GetInteger(poArgs, 0, &iID))
        return Py_BuildException();

    float fX;
    if (!PyTuple_GetFloat(poArgs, 1, &fX))
        return Py_BuildException();

    float fY;
    if (!PyTuple_GetFloat(poArgs, 2, &fY))
        return Py_BuildException();

    std::string buf;
    if (!PyTuple_GetString(poArgs, 3, buf))
        return Py_BuildException();

    CPythonMiniMap::Instance().AddWayPoint(CPythonMiniMap::TYPE_WAYPOINT, (uint32_t)iID, fX, fY, buf);

    Py_RETURN_NONE;
}

PyObject *miniMapRemoveWayPoint(PyObject *poSelf, PyObject *poArgs)
{
    int iID;
    if (!PyTuple_GetInteger(poArgs, 0, &iID))
        return Py_BuildException();

    CPythonMiniMap::Instance().RemoveWayPoint((uint32_t)iID);

    Py_RETURN_NONE;
}

PyObject *miniMapRegisterAtlasWindow(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poHandler))
        return Py_BuildException();
    CPythonMiniMap::Instance().RegisterAtlasWindow(poHandler);
    Py_RETURN_NONE;
}

PyObject *miniMapUnregisterAtlasWindow(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMiniMap::Instance().UnregisterAtlasWindow();
    Py_RETURN_NONE;
}

PyObject *miniMapGetGuildAreaID(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
        return Py_BuildException();
    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
        return Py_BuildException();

    uint32_t dwGuildID = CPythonMiniMap::Instance().GetGuildAreaID(fx, fy);
    return Py_BuildValue("i", dwGuildID);
}

#ifdef ENABLE_ADMIN_MANAGER
PyObject * miniMapUpdateAdminManagerAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().UpdateAdminManagerAtlas();
	Py_RETURN_NONE;
}

PyObject * miniMapRenderAdminManagerAtlas(PyObject * poSelf, PyObject * poArgs)
{
	float fScreenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScreenX))
		return Py_BuildException();

	float fScreenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScreenY))
		return Py_BuildException();

	float fMaxWidth;
	if (!PyTuple_GetFloat(poArgs, 2, &fMaxWidth))
		return Py_BuildException();

	float fMaxHeight;
	if (!PyTuple_GetFloat(poArgs, 3, &fMaxHeight))
		return Py_BuildException();

	CPythonMiniMap::Instance().RenderAdminManagerAtlas(fScreenX, fScreenY, fMaxWidth, fMaxHeight);
	Py_RETURN_NONE;
}

PyObject * miniMapGetAdminManagerAtlasInfo(PyObject * poSelf, PyObject * poArgs)
{
	float fScreenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScreenX))
		return Py_BadArgument();
	float fScreenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScreenY))
		return Py_BadArgument();

	std::string stName;
	float fPosX, fPosY;
	uint32_t dwTextColor;
	uint32_t dwStoneDropVnum;
	if (CPythonMiniMap::Instance().GetAdminManagerAtlasInfo(fScreenX, fScreenY, stName, &fPosX, &fPosY, &dwTextColor, &dwStoneDropVnum))
		return Py_BuildValue("bsffii", true, stName.c_str(), fPosX / 100.0f, fPosY / 100.0f, dwTextColor, dwStoneDropVnum);
	else
		return Py_BuildValue("bsffii", false, "", 0.0f, 0.0f, 0, 0);
}

PyObject * miniMapGetAdminManagerAtlasInfoNew(PyObject * poSelf, PyObject * poArgs)
{
	float fScreenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScreenX))
		return Py_BadArgument();
	float fScreenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScreenY))
		return Py_BadArgument();

	float fPosX, fPosY;
	if (CPythonMiniMap::Instance().GetAdminManagerAtlasInfoNew(fScreenX, fScreenY, &fPosX, &fPosY))
		return Py_BuildValue("bff", true, fPosX / 100.0f, fPosY / 100.0f);
	else
		return Py_BuildValue("bff", false, 0.0f, 0.0f);
}

PyObject * miniMapShowAdminManagerAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().ShowAdminManagerAtlas();
	Py_RETURN_NONE;
}

PyObject * miniMapHideAdminManagerAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().HideAdminManagerAtlas();
	Py_RETURN_NONE;
}

PyObject * miniMapisShowAdminManagerAtlas(PyObject * poSelf, PyObject * poArgs)
{
	bool bShow = CPythonMiniMap::Instance().CanShowAdminManagerAtlas();
	return Py_BuildValue("b", bShow);
}

PyObject * miniMapShowAdminManagerAtlasFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CPythonMiniMap::Instance().ShowAdminManagerFlag(iFlag);
	Py_RETURN_NONE;
}

PyObject * miniMapHideAdminManagerAtlasFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CPythonMiniMap::Instance().HideAdminManagerFlag(iFlag);
	Py_RETURN_NONE;
}

PyObject * miniMapIsAdminManagerAtlasFlagShown(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	return Py_BuildValue("b", CPythonMiniMap::Instance().IsAdminManagerFlagShown(iFlag));
}
#endif

extern "C" void initminiMap()
{
    static PyMethodDef s_methods[] =
    {
        {"SetScale", miniMapSetScale, METH_VARARGS},
        {"ScaleUp", miniMapScaleUp, METH_VARARGS},
        {"ScaleDown", miniMapScaleDown, METH_VARARGS},
        {"SetMiniMapSize", miniMapSetMiniMapSize, METH_VARARGS},

        {"SetCenterPosition", miniMapSetCenterPosition, METH_VARARGS},

        {"Destroy", miniMapDestroy, METH_VARARGS},
        {"Create", miniMapCreate, METH_VARARGS},
        {"Update", miniMapUpdate, METH_VARARGS},
        {"Render", miniMapRender, METH_VARARGS},

        {"Show", miniMapShow, METH_VARARGS},
        {"Hide", miniMapHide, METH_VARARGS},

        {"isShow", miniMapisShow, METH_VARARGS},

        {"GetInfo", miniMapGetInfo, METH_VARARGS},

        {"LoadAtlas", miniMapLoadAtlas, METH_VARARGS},
        {"UpdateAtlas", miniMapUpdateAtlas, METH_VARARGS},
        {"RenderAtlas", miniMapRenderAtlas, METH_VARARGS},
        {"ShowAtlas", miniMapShowAtlas, METH_VARARGS},
        {"HideAtlas", miniMapHideAtlas, METH_VARARGS},
        {"isShowAtlas", miniMapisShowAtlas, METH_VARARGS},
        {"IsAtlas", miniMapIsAtlas, METH_VARARGS},
        {"GetAtlasInfo", miniMapGetAtlasInfo, METH_VARARGS},
        {"GetAtlasSize", miniMapGetAtlasSize, METH_VARARGS},
        {"ToggleAtlasMarker", miniMapToggleAtlasMarker, METH_VARARGS},

        {"AddWayPoint", miniMapAddWayPoint, METH_VARARGS},
        {"RemoveWayPoint", miniMapRemoveWayPoint, METH_VARARGS},

        {"RegisterAtlasWindow", miniMapRegisterAtlasWindow, METH_VARARGS},
        {"UnregisterAtlasWindow", miniMapUnregisterAtlasWindow, METH_VARARGS},

        {"GetGuildAreaID", miniMapGetGuildAreaID, METH_VARARGS},
#ifdef ENABLE_ADMIN_MANAGER
	{ "UpdateAdminManagerAtlas",		miniMapUpdateAdminManagerAtlas,					METH_VARARGS },
	{ "RenderAdminManagerAtlas",		miniMapRenderAdminManagerAtlas,					METH_VARARGS },
	{ "GetAdminManagerAtlasInfo",		miniMapGetAdminManagerAtlasInfo,				METH_VARARGS },
	{ "GetAdminManagerAtlasInfoNew",	miniMapGetAdminManagerAtlasInfoNew,				METH_VARARGS },
	{ "ShowAdminManagerAtlas",			miniMapShowAdminManagerAtlas,					METH_VARARGS },
	{ "HideAdminManagerAtlas",			miniMapHideAdminManagerAtlas,					METH_VARARGS },
	{ "isShowAdminManagerAtlas",		miniMapisShowAdminManagerAtlas,					METH_VARARGS },
	{ "ShowAdminManagerAtlasFlag",		miniMapShowAdminManagerAtlasFlag,				METH_VARARGS },
	{ "HideAdminManagerAtlasFlag",		miniMapHideAdminManagerAtlasFlag,				METH_VARARGS },
	{ "IsAdminManagerAtlasFlagShown",	miniMapIsAdminManagerAtlasFlagShown,			METH_VARARGS },
#endif
        {NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("miniMap", s_methods);

    PyModule_AddIntConstant(poModule, "TYPE_OPC", CPythonMiniMap::TYPE_OPC);
    PyModule_AddIntConstant(poModule, "TYPE_OPCPVP", CPythonMiniMap::TYPE_OPCPVP);
    PyModule_AddIntConstant(poModule, "TYPE_OPCPVPSELF", CPythonMiniMap::TYPE_OPCPVPSELF);
    PyModule_AddIntConstant(poModule, "TYPE_NPC", CPythonMiniMap::TYPE_NPC);
    PyModule_AddIntConstant(poModule, "TYPE_MONSTER", CPythonMiniMap::TYPE_MONSTER);
    PyModule_AddIntConstant(poModule, "TYPE_WARP", CPythonMiniMap::TYPE_WARP);
    PyModule_AddIntConstant(poModule, "TYPE_WAYPOINT", CPythonMiniMap::TYPE_WAYPOINT);
    PyModule_AddIntConstant(poModule, "TYPE_PARTY", CPythonMiniMap::TYPE_PARTY);
    PyModule_AddIntConstant(poModule, "TYPE_EMPIRE", CPythonMiniMap::TYPE_EMPIRE);
    PyModule_AddIntConstant(poModule, "TYPE_SHOP", CPythonMiniMap::TYPE_SHOP);

#ifdef ENABLE_ADMIN_MANAGER
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_OBSERVING_PLAYER", CPythonMiniMap::ADMIN_MANAGER_SHOW_OBSERVING_PLAYER);
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_PC", CPythonMiniMap::ADMIN_MANAGER_SHOW_PC);
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_MOB", CPythonMiniMap::ADMIN_MANAGER_SHOW_MOB);
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_STONE", CPythonMiniMap::ADMIN_MANAGER_SHOW_STONE);
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_NPC", CPythonMiniMap::ADMIN_MANAGER_SHOW_NPC);
	PyModule_AddIntConstant(poModule, "ADMIN_SHOW_ORE", CPythonMiniMap::ADMIN_MANAGER_SHOW_ORE);
#endif
}
