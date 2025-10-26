#include "StdAfx.h"
#include "../eterLib/Profiler.h"

PyObject *profilerPush(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *profilerPop(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

extern "C" void initprofiler()
{
    static PyMethodDef s_methods[] =
    {
        {"Push", profilerPush, METH_VARARGS},
        {"Pop", profilerPop, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    Py_InitModule("profiler", s_methods);
}
