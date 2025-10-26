#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"
#include "PythonCharacterManager.h"
#include "../GameLib/FlyingObjectManager.h"

PyObject *effectRegisterEffect(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BuildException();

    CEffectManager::Instance().RegisterEffect(szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *effectUpdate(PyObject *poSelf, PyObject *poArgs)
{
    CEffectManager::Instance().Update();
    Py_RETURN_NONE;
}

PyObject *effectRender(PyObject *poSelf, PyObject *poArgs)
{
    CEffectManager::Instance().Render();
    Py_RETURN_NONE;
}

PyObject *effectRegisterIndexedFlyData(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();
    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
        return Py_BuildException();
    std::string szFlyDataName;
    if (!PyTuple_GetString(poArgs, 2, szFlyDataName))
        return Py_BuildException();

    CFlyingManager::Instance().RegisterIndexedFlyData(iIndex, iType, szFlyDataName.c_str());

    Py_RETURN_NONE;
}

extern "C" void initeffect()
{
    static PyMethodDef s_methods[] =
    {
        {"RegisterEffect", effectRegisterEffect, METH_VARARGS},

        {"RegisterIndexedFlyData", effectRegisterIndexedFlyData, METH_VARARGS},

        {"Update", effectUpdate, METH_VARARGS},
        {"Render", effectRender, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("effect", s_methods);
    PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_NORMAL", CFlyingManager::INDEX_FLY_TYPE_NORMAL);
    PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_FIRE_CRACKER", CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER);
    PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_AUTO_FIRE", CFlyingManager::INDEX_FLY_TYPE_AUTO_FIRE);

    PyModule_AddIntConstant(poModule, "FLY_NONE", FLY_NONE);
    PyModule_AddIntConstant(poModule, "FLY_EXP", FLY_EXP);
    PyModule_AddIntConstant(poModule, "FLY_HP_MEDIUM", FLY_HP_MEDIUM);
    PyModule_AddIntConstant(poModule, "FLY_HP_BIG", FLY_HP_BIG);
    PyModule_AddIntConstant(poModule, "FLY_SP_SMALL", FLY_SP_SMALL);
    PyModule_AddIntConstant(poModule, "FLY_SP_MEDIUM", FLY_SP_MEDIUM);
    PyModule_AddIntConstant(poModule, "FLY_SP_BIG", FLY_SP_BIG);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK1", FLY_FIREWORK1);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK2", FLY_FIREWORK2);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK3", FLY_FIREWORK3);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK4", FLY_FIREWORK4);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK5", FLY_FIREWORK5);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK6", FLY_FIREWORK6);
    PyModule_AddIntConstant(poModule, "FLY_FIREWORK_XMAS", FLY_FIREWORK_CHRISTMAS);
    PyModule_AddIntConstant(poModule, "FLY_CHAIN_LIGHTNING", FLY_CHAIN_LIGHTNING);
    PyModule_AddIntConstant(poModule, "FLY_HP_SMALL", FLY_HP_SMALL);
    PyModule_AddIntConstant(poModule, "FLY_SKILL_MUYEONG", FLY_SKILL_MUYEONG);
    PyModule_AddIntConstant(poModule, "FLY_QUIVER_ATTACK_NORMAL", FLY_QUIVER_ATTACK_NORMAL);
}
