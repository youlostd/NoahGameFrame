#include "stdafx.h"
#include "../gamelib/FlyingObjectManager.h"

PyObject *flyUpdate(PyObject *poSelf, PyObject *poArgs)
{
    CFlyingManager::Instance().Update();
    Py_RETURN_NONE;
}

PyObject *flyRender(PyObject *poSelf, PyObject *poArgs)
{
    CFlyingManager::Instance().Render();
    Py_RETURN_NONE;
}

extern "C" void initfly()
{
    static PyMethodDef s_methods[] =
    {
        {"Update", flyUpdate, METH_VARARGS},
        {"Render", flyRender, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    Py_InitModule("fly", s_methods);
}
