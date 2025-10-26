#include "StdAfx.h"

#include "PythonApplication.h"

#ifdef INGAME_WIKI
#include "../gamelib/ItemManager.h"
#include "../EterBase/Stl.h"
#include "PythonNetworkStream.h"
#include "../GameLib/NpcManager.h"
#include "PythonWikiModelViewManager.h"

PyObject *wikiModule = NULL;

PyObject *wikiGetBaseClass(PyObject *poSelf, PyObject *poArgs)
{
    if (!wikiModule)
        return Py_BuildNone();

    return Py_BuildValue("O", wikiModule);
}

PyObject *wikiGetChestInfo(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();
    CItemData *table;
    if (CItemManager::instance().GetItemDataPointer(itemVnum, &table) && !table->GetWikiTable()->pChestInfo.empty())
    {
        size_t dataCount = table->GetWikiTable()->pChestInfo.size();
        PyObject *ret = PyTuple_New(dataCount);
        for (int i = 0; i < dataCount; ++i)
            PyTuple_SetItem(ret, i, Py_BuildValue("i", table->GetWikiTable()->pChestInfo[i]));
        return Py_BuildValue("iiO", table->GetWikiTable()->dwOrigin, table->GetWikiTable()->bIsCommon ? 1 : 0, ret);
    }

    return Py_BuildValue("ii", table->GetWikiTable()->dwOrigin, table->GetWikiTable()->bIsCommon ? 1 : 0);
    ;
}

PyObject *wikiGetMobInfo(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();

    const NpcManager::TWikiInfoTable *mobData = NpcManager::instance().GetWikiTable(itemVnum);
    if (!mobData->dropList.empty())
    {
        size_t dataCount = mobData->dropList.size();
        PyObject *ret = PyTuple_New(dataCount);
        for (int i = 0; i < dataCount; ++i)
            PyTuple_SetItem(ret, i, Py_BuildValue("iiiid", mobData->dropList[i].dwVNum, mobData->dropList[i].iMinLevel, mobData->dropList[i].iMaxLevel, mobData->dropList[i].count, mobData->dropList[i].rarity));
        return Py_BuildValue("O", ret);
    }

    return Py_BuildNone();
}

PyObject *wikiLoadClassItems(PyObject *poSelf, PyObject *poArgs)
{
    BYTE classType;
    int raceFilter;

    if (!PyTuple_GetByte(poArgs, 0, &classType))
        return Py_BadArgument();

    if (!PyTuple_GetInteger(poArgs, 1, &raceFilter))
        return Py_BadArgument();

    return Py_BuildValue("i", CItemManager::Instance().WikiLoadClassItems(classType, raceFilter));
}

PyObject *wikiLoadClassMobs(PyObject *poSelf, PyObject *poArgs)
{
    BYTE classType;
    if (!PyTuple_GetByte(poArgs, 0, &classType))
        return Py_BadArgument();

    WORD fromLvl, toLvl;
    if (!PyTuple_GetInteger(poArgs, 1, &fromLvl))
        return Py_BadArgument();

    if (!PyTuple_GetInteger(poArgs, 2, &toLvl))
        return Py_BadArgument();

    return Py_BuildValue("i", NpcManager::instance().WikiLoadClassMobs(classType, fromLvl, toLvl));
}

PyObject *GetWikiModule() { return wikiModule; }

PyObject *wikiChangePage(PyObject *poSelf, PyObject *poArgs)
{
    unsigned short from;
    unsigned short to;
    int isMob = 0;

    if (!PyTuple_GetInteger(poArgs, 0, &from))
        return Py_BadArgument();

    if (!PyTuple_GetInteger(poArgs, 1, &to))
        return Py_BadArgument();

    if (PyTuple_Size(poArgs) > 2 && !PyTuple_GetInteger(poArgs, 2, &isMob))
        return Py_BadArgument();

    if (from > to)
        return Py_BuildException("to must be bigger than from");

    std::vector<DWORD> *loadVec;
    if (!isMob)
        loadVec = CItemManager::Instance().WikiGetLastItems();
    else
        loadVec = NpcManager::instance().WikiGetLastMobs();

    PyObject *tuple;
    if (loadVec->size() > 0)
    {
        if (loadVec->size() < to)
            to = loadVec->size();

        if (from > to)
            return Py_BuildException("to must be bigger than from");

        tuple = PyTuple_New(to - from);
        for (unsigned short i = from; i < to; ++i)
            PyTuple_SetItem(tuple, i - from, Py_BuildValue("i", (*loadVec)[i]));

        return tuple;
    }

    return Py_BuildNone();
}

PyObject *wikiRegisterModelViewWindow(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    UI::CWindow *hwnd;
    if (!PyTuple_GetWindow(poArgs, 1, &hwnd))
    {
        CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
        if (mod)
            mod->RegisterWindow(nullptr);
        return Py_BuildNone();
    }

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->RegisterWindow(hwnd);
    return Py_BuildNone();
}

PyObject *wikiLoadInfo(PyObject *poSelf, PyObject *poArgs)
{
    unsigned long long retID;
    if (!PyTuple_GetUnsignedLongLong(poArgs, 0, &retID))
        return Py_BadArgument();

    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 1, &itemVnum))
        return Py_BadArgument();

    int isMob = 0;
    if (PyTuple_Size(poArgs) > 2 && !PyTuple_GetInteger(poArgs, 2, &isMob))
        return Py_BadArgument();

    if (!isMob)
    {
        CItemData *itemData;
        if (CItemManager::Instance().GetItemDataPointer(itemVnum, &itemData))
        {
            if (!itemData->GetWikiTable()->isSet)
                gPythonNetworkStream->SendWikiRequestInfo(retID, itemVnum, false);
            else
                PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo", Py_BuildValue("(Ki)", retID, itemVnum));
        }
    }
    else
    {
        const NpcManager::TWikiInfoTable *mobData = NpcManager::instance().GetWikiTable(itemVnum);
        if (mobData)
        {
            if (!mobData->isSet)
                gPythonNetworkStream->SendWikiRequestInfo(retID, itemVnum, true);
            else
                PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo", Py_BuildValue("(Ki)", retID, itemVnum));
        }
    }

    return Py_BuildNone();
}

PyObject *wikiGetFreeModelViewID(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonWikiModelViewManager::instance().GetFreeID());
}

PyObject *wikiAddModelView(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    CPythonWikiModelViewManager::instance().AddView(moduleID);
    return Py_BuildNone();
}

PyObject *wikiRemoveModelView(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    CPythonWikiModelViewManager::instance().RemoveView(moduleID);
    return Py_BuildNone();
}

PyObject *wikiShowModelView(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int isShow;
    if (!PyTuple_GetInteger(poArgs, 1, &isShow))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetShow(isShow != 0);
    return Py_BuildNone();
}

PyObject *wikiShowModelViewManager(PyObject *poSelf, PyObject *poArgs)
{
    int isShow;
    if (!PyTuple_GetInteger(poArgs, 0, &isShow))
        return Py_BadArgument();

    CPythonWikiModelViewManager::instance().SetShow(isShow != 0);
    return Py_BuildNone();
}

PyObject *wikiSetModelViewModel(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int vnum;
    if (!PyTuple_GetInteger(poArgs, 1, &vnum))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetModel(vnum);
    return Py_BuildNone();
}

PyObject *wikiSetModelViewRotation(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int vnum;
    if (!PyTuple_GetInteger(poArgs, 1, &vnum))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetRotationStatus(vnum);
    return Py_BuildNone();
}

PyObject *wikiSetModelViewZoom(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int zoom;
    if (!PyTuple_GetInteger(poArgs, 1, &zoom))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetModelZoom(zoom);

    return Py_BuildNone();
}


PyObject *wikiSetModelViewDrag(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int x;
    if (!PyTuple_GetInteger(poArgs, 1, &x))
        return Py_BadArgument();

    int y;
    if (!PyTuple_GetInteger(poArgs, 2, &y))
        return Py_BadArgument();
    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->Drag(x, y);

    return Py_BuildNone();
}



PyObject *wikiSetModelViewWeapon(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int vnum;
    if (!PyTuple_GetInteger(poArgs, 1, &vnum))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetWeaponModel(vnum);
    return Py_BuildNone();
}

PyObject *wikiSetModelViewHair(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int vnum;
    if (!PyTuple_GetInteger(poArgs, 1, &vnum))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetModelHair(vnum);
    return Py_BuildNone();
}

PyObject *wikiSetModelViewForm(PyObject *poSelf, PyObject *poArgs)
{
    int moduleID;
    if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
        return Py_BadArgument();

    int vnum;
    if (!PyTuple_GetInteger(poArgs, 1, &vnum))
        return Py_BadArgument();

    CPythonWikiModelView *mod = CPythonWikiModelViewManager::instance().GetModule(moduleID);
    if (mod)
        mod->SetModelForm(vnum);
    return Py_BuildNone();
}

PyObject *wikiSearchItem(PyObject *poSelf, PyObject *poArgs)
{
    std::string itemName;

    if (!PyTuple_GetString(poArgs, 0, itemName))
        return Py_BadArgument();

    if (itemName.length() < 2)
    {
        return Py_BuildException("Lenght must be at least 2!");
    }

    for (int i = 0; i < itemName.length(); i++)
        itemName[i] = tolower(itemName[i]);

    return Py_BuildValue("i", CItemManager::instance().WikiSearchItem(itemName));
}

PyObject *wikiGetRefineInfo(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();
    CItemData *table = CItemManager::instance().GetProto(itemVnum);
    if (table && !table->GetWikiTable()->pRefineData.empty())
    {
        PyObject *ret = PyTuple_New(10);
        for (int i = 0; i < 10; ++i)
        {
            const auto &refine_data = table->GetWikiTable()->pRefineData[i];
            PyObject *ret2 = PyTuple_New(refine_data.material_count);
            for (int j = 0; j < table->GetWikiTable()->pRefineData[i].material_count; ++j)
                PyTuple_SetItem(ret2, j,
                                Py_BuildValue("ii", table->GetWikiTable()->pRefineData[i].materials[j].vnum,
                                              table->GetWikiTable()->pRefineData[i].materials[j].count));

            PyTuple_SetItem(ret, i, Py_BuildValue("LO", refine_data.price, ret2));
        }
        return Py_BuildValue("O", ret);
    }

    return Py_BuildNone();
}

PyObject *wikiGetOriginInfo(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();
    CItemData *table = CItemManager::instance().GetProto(itemVnum);
    if (table && !table->GetWikiTable()->pOriginInfo.empty())
    {
        TWikiItemInfo *wikiTbl = table->GetWikiTable();
        PyObject *ret = PyTuple_New(wikiTbl->pOriginInfo.size());
        for (int i = 0; i < wikiTbl->pOriginInfo.size(); ++i)
            PyTuple_SetItem(ret, i,
                            Py_BuildValue("ii", wikiTbl->pOriginInfo[i].vnum, wikiTbl->pOriginInfo[i].is_mob ? 1 : 0));

        return Py_BuildValue("O", ret);
    }

    return Py_BuildNone();
}

PyObject *wikiIsSet(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();

    bool isMob = false;
    if (PyTuple_Size(poArgs) > 1)
        if (!PyTuple_GetBoolean(poArgs, 1, &isMob))
            return Py_BadArgument();

    if (!isMob)
    {
        CItemData *table;
        if (CItemManager::instance().GetItemDataPointer(itemVnum, &table))
            return Py_BuildValue("i", table->GetWikiTable()->isSet ? 1 : 0);
    }
    else
    {
        NpcManager::TWikiInfoTable *ptr = NpcManager::instance().GetWikiTable(itemVnum);
        if (ptr)
            return Py_BuildValue("i", ptr->isSet ? 1 : 0);
    }

    return Py_BuildValue("i", 0);
}

PyObject *wikiHasData(PyObject *poSelf, PyObject *poArgs)
{
    int itemVnum;
    if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
        return Py_BadArgument();
    CItemData *table;
    if (CItemManager::instance().GetItemDataPointer(itemVnum, &table))
    {
        return Py_BuildValue("i", table->GetWikiTable()->hasData ? 1 : 0);
    }
    return Py_BuildValue("i", 0);
}

PyObject *wikiRegisterClass(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *mClass;
    if (!PyTuple_GetObject(poArgs, 0, &mClass))
        return Py_BadArgument();

    wikiModule = mClass;
    return Py_BuildNone();
}

PyObject *wikiUnregisterClass(PyObject *poSelf, PyObject *poArgs)
{
    wikiModule = NULL;
    return Py_BuildNone();
}

PyObject *wikiRegisterItemBlacklist(PyObject *poSelf, PyObject *poArgs)
{
    int vnum;
    if (!PyTuple_GetInteger(poArgs, 0, &vnum))
        return Py_BadArgument();

    CItemManager::instance().WikiAddVnumToBlacklist(vnum);
    return Py_BuildNone();
}

PyObject *wikiRegisterMonsterBlacklist(PyObject *poSelf, PyObject *poArgs)
{
    int vnum;
    if (!PyTuple_GetInteger(poArgs, 0, &vnum))
        return Py_BadArgument();

    NpcManager::instance().WikiSetBlacklisted(vnum);
    return Py_BuildNone();
}


extern "C" void initwiki()
{
    static PyMethodDef s_methods[] = {{"GetBaseClass", wikiGetBaseClass, METH_VARARGS},
                                      {"GetChestInfo", wikiGetChestInfo, METH_VARARGS},
                                      {"LoadClassItems", wikiLoadClassItems, METH_VARARGS},
                                      {"SearchItem", wikiSearchItem, METH_VARARGS},
                                      {"ChangePage", wikiChangePage, METH_VARARGS},
                                      {"LoadInfo", wikiLoadInfo, METH_VARARGS},
                                      {"RegisterClass", wikiRegisterClass, METH_VARARGS},
                                      {"UnregisterClass", wikiUnregisterClass, METH_VARARGS},
                                      {"HasData", wikiHasData, METH_VARARGS},
                                      {"IsSet", wikiIsSet, METH_VARARGS},
                                      {"GetRefineInfo", wikiGetRefineInfo, METH_VARARGS},
                                      {"GetOriginInfo", wikiGetOriginInfo, METH_VARARGS},

                                      {"RegisterItemBlacklist", wikiRegisterItemBlacklist, METH_VARARGS},
                                      {"RegisterMonsterBlacklist", wikiRegisterMonsterBlacklist, METH_VARARGS},

                                      {"GetFreeModelViewID", wikiGetFreeModelViewID, METH_VARARGS},
                                      {"AddModelView", wikiAddModelView, METH_VARARGS},
                                      {"RemoveModelView", wikiRemoveModelView, METH_VARARGS},
                                      {"ShowModelView", wikiShowModelView, METH_VARARGS},
                                      {"ShowModelViewManager", wikiShowModelViewManager, METH_VARARGS},
                                      {"SetModelViewModel", wikiSetModelViewModel, METH_VARARGS},
                                      {"SetModelViewWeapon", wikiSetModelViewWeapon, METH_VARARGS},
                                      {"SetModelViewRotationStatus", wikiSetModelViewRotation, METH_VARARGS},

                                      {"SetModelViewZoom", wikiSetModelViewZoom, METH_VARARGS},
                                      {"SetModelViewDrag", wikiSetModelViewDrag, METH_VARARGS},


                                      {"SetModelViewHair", wikiSetModelViewHair, METH_VARARGS},
                                      {"SetModelViewForm", wikiSetModelViewForm, METH_VARARGS},
                                      {"RegisterModelViewWindow", wikiRegisterModelViewWindow, METH_VARARGS},

                                      {"LoadClassMobs", wikiLoadClassMobs, METH_VARARGS},
                                      {"GetMobInfo", wikiGetMobInfo, METH_VARARGS},
                                      {NULL, NULL, NULL}};

    PyObject *module = Py_InitModule("wiki", s_methods);
    PyModule_AddIntConstant(module, "REFINE_MATERIAL_MAX_NUM", REFINE_MATERIAL_MAX_NUM);
}
#endif
