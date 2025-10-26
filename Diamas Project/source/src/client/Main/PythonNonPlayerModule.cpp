#include "StdAfx.h"
#include "../GameLib/NpcManager.h"

#include "InstanceBase.h"
#include "PythonCharacterManager.h"

#include <pybind11/stl.h>

PyObject *nonplayerGetEventType(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualNumber;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualNumber))
        return Py_BuildException();

    uint8_t iType = NpcManager::Instance().GetEventType(iVirtualNumber);

    return Py_BuildValue("i", iType);
}

PyObject *nonplayerGetEventTypeByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (NULL == pInstance)
    {
        return Py_BuildValue("i", 0);
    }

    uint16_t dwVnum = pInstance->GetVirtualNumber();

    uint8_t iType = NpcManager::Instance().GetEventType(dwVnum);

    return Py_BuildValue("i", iType);
}

PyObject *nonplayerGetLevelByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", -1);

    const TMobTable *pMobTable = NpcManager::Instance().GetTable(pInstance->GetVirtualNumber());

    if (!pMobTable)
        return Py_BuildValue("i", -1);

    float fAverageLevel = pMobTable->bLevel;
    //(float(pMobTable->abLevelRange[0]) + float(pMobTable->abLevelRange[1])) / 2.0f;
    fAverageLevel = floor(fAverageLevel + 0.5f);
    return Py_BuildValue("i", int(fAverageLevel));
}

PyObject *nonplayerGetGradeByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", -1);

    const TMobTable *pMobTable = NpcManager::Instance().GetTable(pInstance->GetVirtualNumber());

    if (!pMobTable)
        return Py_BuildValue("i", -1);

    return Py_BuildValue("i", pMobTable->bRank);
}

PyObject *nonplayerGetMonsterName(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();
    return Py_BuildValue("s", rkNonPlayer.GetMonsterName(iVNum));
}

PyObject *nonplayerLoadNonPlayerData(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    NpcManager::Instance().LoadNonPlayerData(szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *nonplayerGetMonsterLevel(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("i", -1);
    }

    return Py_BuildValue("i", pMobTable->bLevel);
}

PyObject *nonplayerGetMonsterImmuneFlag(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("i", -1);
    }

    return Py_BuildValue("i", pMobTable->dwImmuneFlag);
}

PyObject *nonplayerGetMonsterRaceFlag(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("i", -1);
    }

    return Py_BuildValue("i", pMobTable->dwRaceFlag);
}

PyObject *nonplayerGetMonsterEXP(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("i", -1);
    }

    return Py_BuildValue("i", pMobTable->dwExp);
}

PyObject *nonplayerGetMonsterMaxHP(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("L", -1);
    }

    return Py_BuildValue("L", pMobTable->dwMaxHP);
}

PyObject *nonplayerGetMonsterGold(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("ii", -1, -1);
    }

    return Py_BuildValue("ii", pMobTable->dwGoldMin, pMobTable->dwGoldMax);
}

PyObject *nonplayerGetMonsterDamage(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("ii", -1, -1);
    }

    return Py_BuildValue("ii", pMobTable->dwDamageRange[0], pMobTable->dwDamageRange[1]);
}

PyObject *nonplayerIsMonsterStone(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    {
        return Py_BuildException();
    }

    const TMobTable *pMobTable = NpcManager::Instance().GetTable((uint32_t)iVNum);

    if (!pMobTable)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pMobTable->bType == 2);
}

PyObject *nonplayerGetRaceNumByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", -1);

    const TMobTable *pMobTable = NpcManager::Instance().GetTable(pInstance->GetVirtualNumber());

    if (!pMobTable)
        return Py_BuildValue("i", -1);

    return Py_BuildValue("i", pMobTable->dwVnum);
}

PyObject *nonplayerGetMonsterExp(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMonsterExp(race));
}

PyObject *nonplayerGetMonsterDamageMultiply(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("f", rkNonPlayer.GetMonsterDamageMultiply(race));
}

PyObject *nonplayerGetMonsterST(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMonsterST(race));
}

PyObject *nonplayerGetMonsterDX(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMonsterDX(race));
}

PyObject *nonplayerGetMobRegenCycle(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMobRegenCycle(race));
}

PyObject *nonplayerGetMobRegenPercent(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMobRegenPercent(race));
}

PyObject *nonplayerGetMobGoldMin(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMobGoldMin(race));
}

PyObject *nonplayerGetMobGoldMax(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMobGoldMax(race));
}

PyObject *nonplayerGetAttElementFlagByVID(PyObject *poSelf, PyObject *poArgs)
{
    int vid;
    if (!PyTuple_GetInteger(poArgs, 0, &vid))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();
    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(vid);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    auto dwVnum = pInstance->GetRace();

    return Py_BuildValue("i", rkNonPlayer.GetAttElementFlag(dwVnum));
}

PyObject *nonplayerGetMobResist(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BuildException();

    uint8_t resistNum;
    if (!PyTuple_GetInteger(poArgs, 1, &resistNum))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();

    return Py_BuildValue("i", rkNonPlayer.GetMobResist(race, resistNum));
}

PyObject *nonplayerGetVnumByNamePart(PyObject *poSelf, PyObject *poArgs)
{
    std::string szNamePart;
    if (!PyTuple_GetString(poArgs, 0, szNamePart))
        return Py_BadArgument();

    return Py_BuildValue("i", NpcManager::Instance().GetVnumByNamePart(szNamePart.c_str()));
}

PyObject *nonplayerBuildWikiSearchList(PyObject *poSelf, PyObject *poArgs)
{
    NpcManager::Instance().BuildWikiSearchList();
    return Py_BuildNone();
}

PyObject *nonplayerGetAutoCompleteEntries(PyObject *poSelf, PyObject *poArgs)
{
    std::string name;
    if (!PyTuple_GetString(poArgs, 0, name))
        return Py_BadArgument();

    auto const vec = NpcManager::instance().GetEntriesByName(name, 10);
    if (!vec)
    {
        py::list list;
        return Py_BuildValue("O", list.ptr());
    }

    py::list list;
    for (const auto &item : vec.value())
    {
        py::tuple tup = py::make_tuple(item.first, py::bytes(item.second), 1);
        list.append(tup);
    }

    return Py_BuildValue("O", list.ptr());
}

PyObject *nonplayerGetMonsterResistValue(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
        return Py_BuildException();

    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
        return Py_BuildException();

    if (iType < 0 || iType > MOB_RESISTS_MAX_NUM)
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();
    const TMobTable *pkTab = rkNonPlayer.GetTable(iVNum);
    if (!pkTab)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pkTab->cResists[iType]);
}

PyObject *nonplayerGetMonsterDefense(PyObject *poSelf, PyObject *poArgs)
{
    int iVNum;
    if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
        return Py_BuildException();

    NpcManager &rkNonPlayer = NpcManager::Instance();
    const TMobTable *pkTab = rkNonPlayer.GetTable(iVNum);
    if (!pkTab)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pkTab->wDef);
}

extern "C" void initnonplayer()
{
    static PyMethodDef s_methods[] =
    {
        {"GetEventType", nonplayerGetEventType, METH_VARARGS},
        {"GetEventTypeByVID", nonplayerGetEventTypeByVID, METH_VARARGS},
        {"GetLevelByVID", nonplayerGetLevelByVID, METH_VARARGS},
        {"GetGradeByVID", nonplayerGetGradeByVID, METH_VARARGS},
        {"GetMonsterName", nonplayerGetMonsterName, METH_VARARGS},
        {"GetMonsterLevel", nonplayerGetMonsterLevel, METH_VARARGS},
        {"GetMonsterRaceFlag", nonplayerGetMonsterRaceFlag, METH_VARARGS},
        {"GetMonsterImmuneFlag", nonplayerGetMonsterImmuneFlag, METH_VARARGS},
        {"GetMonsterMaxHP", nonplayerGetMonsterMaxHP, METH_VARARGS},
        {"GetMonsterGold", nonplayerGetMonsterGold, METH_VARARGS},
        {"GetMonsterDamage", nonplayerGetMonsterDamage, METH_VARARGS},
        {"GetMonsterMaxHP", nonplayerGetMonsterMaxHP, METH_VARARGS},
        {"GetMonsterLevel", nonplayerGetMonsterLevel, METH_VARARGS},
        {"GetMonsterExp", nonplayerGetMonsterExp, METH_VARARGS},
        {"GetMonsterDamageMultiply", nonplayerGetMonsterDamageMultiply, METH_VARARGS},
        {"GetMonsterST", nonplayerGetMonsterST, METH_VARARGS},
        {"GetMonsterDX", nonplayerGetMonsterDX, METH_VARARGS},
        {"GetMonsterResistValue", nonplayerGetMonsterResistValue, METH_VARARGS},
        {"GetMonsterDefense", nonplayerGetMonsterDefense, METH_VARARGS},

        {"IsMonsterStone", nonplayerIsMonsterStone, METH_VARARGS},
        {"IsMonsterStone", nonplayerIsMonsterStone, METH_VARARGS},

        {"GetMobRegenCycle", nonplayerGetMobRegenCycle, METH_VARARGS},
        {"GetMobRegenPercent", nonplayerGetMobRegenPercent, METH_VARARGS},
        {"GetMobGoldMin", nonplayerGetMobGoldMin, METH_VARARGS},
        {"GetMobGoldMax", nonplayerGetMobGoldMax, METH_VARARGS},
        {"GetResist", nonplayerGetMobResist, METH_VARARGS},
        {"GetAttElementFlagByVID", nonplayerGetAttElementFlagByVID, METH_VARARGS},
        {"BuildWikiSearchList", nonplayerBuildWikiSearchList, METH_VARARGS},
        {"GetVnumByNamePart", nonplayerGetVnumByNamePart, METH_VARARGS},
        {"GetRaceNumByVID", nonplayerGetRaceNumByVID, METH_VARARGS},

        {"LoadNonPlayerData", nonplayerLoadNonPlayerData, METH_VARARGS},
        {"GetAutocompleteEntries", nonplayerGetAutoCompleteEntries, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("nonplayer", s_methods);

    PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_NONE", ON_CLICK_NONE);
    PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_SHOP", ON_CLICK_SHOP);
    PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_TALK", ON_CLICK_TALK);

    PyModule_AddIntConstant(poModule, "PAWN", MOB_RANK_PAWN);
    PyModule_AddIntConstant(poModule, "S_PAWN", MOB_RANK_S_PAWN);
    PyModule_AddIntConstant(poModule, "KNIGHT", MOB_RANK_KNIGHT);
    PyModule_AddIntConstant(poModule, "S_KNIGHT", MOB_RANK_S_KNIGHT);
    PyModule_AddIntConstant(poModule, "BOSS", MOB_RANK_BOSS);
    PyModule_AddIntConstant(poModule, "KING", MOB_RANK_KING);
    PyModule_AddIntConstant(poModule, "LEGENDARY", MOB_RANK_LEGENDARY);

    PyModule_AddIntConstant(poModule, "MOB_RESIST_SWORD", MOB_RESIST_SWORD);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_TWOHAND", MOB_RESIST_TWOHAND);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_DAGGER", MOB_RESIST_DAGGER);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_BELL", MOB_RESIST_BELL);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_FAN", MOB_RESIST_FAN);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_BOW", MOB_RESIST_BOW);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_FIRE", MOB_RESIST_FIRE);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_ELECT", MOB_RESIST_ELECT);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_MAGIC", MOB_RESIST_MAGIC);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_WIND", MOB_RESIST_WIND);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_POISON", MOB_RESIST_POISON);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_BLEEDING", MOB_RESIST_BLEEDING);
    PyModule_AddIntConstant(poModule, "MOB_RESIST_MAX_NUM", MOB_RESISTS_MAX_NUM);

    PyModule_AddIntMacro(poModule, RACE_FLAG_ANIMAL);
    PyModule_AddIntMacro(poModule, RACE_FLAG_UNDEAD);
    PyModule_AddIntMacro(poModule, RACE_FLAG_DEVIL);
    PyModule_AddIntMacro(poModule, RACE_FLAG_HUMAN);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ORC);
    PyModule_AddIntMacro(poModule, RACE_FLAG_MILGYO);
    PyModule_AddIntMacro(poModule, RACE_FLAG_INSECT);
    PyModule_AddIntMacro(poModule, RACE_FLAG_FIRE);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ICE);
    PyModule_AddIntMacro(poModule, RACE_FLAG_DESERT);
    PyModule_AddIntMacro(poModule, RACE_FLAG_TREE);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_ELEC);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_FIRE);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_ICE);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_WIND);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_EARTH);
    PyModule_AddIntMacro(poModule, RACE_FLAG_ATT_DARK);
    PyModule_AddIntMacro(poModule, RACE_FLAG_CZ);
    PyModule_AddIntMacro(poModule, RACE_FLAG_SHADOW);
    PyModule_AddIntMacro(poModule, RACE_FLAG_MAX_NUM);

    PyModule_AddIntMacro(poModule, IMMUNE_STUN);
    PyModule_AddIntMacro(poModule, IMMUNE_SLOW);
    PyModule_AddIntMacro(poModule, IMMUNE_FALL);
    PyModule_AddIntMacro(poModule, IMMUNE_CURSE);
    PyModule_AddIntMacro(poModule, IMMUNE_POISON);
    PyModule_AddIntMacro(poModule, IMMUNE_TERROR);
    PyModule_AddIntMacro(poModule, IMMUNE_REFLECT);
    PyModule_AddIntMacro(poModule, IMMUNE_FLAG_MAX_NUM);
}

void init_npc(py::module &m)
{
    py::module npc = m.def_submodule("npc", "Provides npc support");

    py::class_<TMobTable>(npc, "MobProto")
        .def(py::init<>())
        .def_readwrite("vnum", &TMobTable::dwVnum)
        .def_readwrite("type", &TMobTable::bType)
        .def_readwrite("rank", &TMobTable::bRank)
        .def_readwrite("battleType", &TMobTable::bBattleType)
        .def_readwrite("level", &TMobTable::bLevel)
        .def_readwrite("aiFlag", &TMobTable::dwAIFlag);

    py::class_<WorldBossPosition>(npc, "WorldBossPosition")
        .def(py::init<uint32_t, uint32_t, uint32_t>())
        .def_readwrite("mapIndex", &WorldBossPosition::mapIndex)
        .def_readwrite("x", &WorldBossPosition::x)
        .def_readwrite("y", &WorldBossPosition::y);

    py::class_<TWorldBossInfo>(npc, "WorldBossInfo")
        .def(py::init<>())
        .def_readwrite("day", &TWorldBossInfo::bDay)
        .def_readwrite("hour", &TWorldBossInfo::bHour)
        .def_readwrite("boss", &TWorldBossInfo::dwRaceNum)
        .def_readwrite("iMapCount", &TWorldBossInfo::iMapCount)
        .def_readwrite("mapIndexList", &TWorldBossInfo::piMapIndexList)
        .def_readwrite("nameList", &TWorldBossInfo::pszMapNameList)
        .def_readwrite("bIsDaily", &TWorldBossInfo::bIsDaily);

    py::class_<NpcManager, std::unique_ptr<NpcManager, py::nodelete>>(npc, "npcManager")
        .def(py::init([]()
        {
            return std::unique_ptr<NpcManager, py::nodelete>(NpcManager::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)

        .def("GetMonsterName", [](NpcManager& n, uint32_t vnum)
        {
            return py::bytes(n.GetName(vnum).value_or(""));
        })
        .def("GetProto", &NpcManager::GetTable, pybind11::return_value_policy::reference)
        .def("GetWorldBossInfos", &NpcManager::GetWorldBossInfos, pybind11::return_value_policy::reference);
}
