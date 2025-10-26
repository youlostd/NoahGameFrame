#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include <cStringIO.h>

PyObject *GetVfsFile(PyObject *poSelf, PyObject *poArgs)
{
    std::string filename;
    if (!PyTuple_GetString(poArgs, 0, filename))
        Py_RETURN_NONE;

    auto fp = GetVfs().Open(filename);
    if (!fp)
        Py_RETURN_NONE;

    const auto size = fp->GetSize();
    auto string = PyString_FromStringAndSize(nullptr, size);
    fp->Read(0, PyString_AsString(string), size);
    return string;
}

PyObject *OpenVfsFile(PyObject *poSelf, PyObject *poArgs)
{
    std::string filename;
    if (!PyTuple_GetString(poArgs, 0, filename))
        Py_RETURN_NONE;

    if (!PycStringIO)
        PycString_IMPORT;

    auto string = GetVfsFile(nullptr, Py_BuildValue("(s)", filename.c_str()));
    if (!PyString_Check(string))
        Py_RETURN_NONE;

    return PycStringIO->NewInput(string);
}

extern "C" void initpack()
{
    static PyMethodDef s_methods[] =
    {
        {"OpenVfsFile", OpenVfsFile, METH_VARARGS},
        {"GetVfsFile", GetVfsFile, METH_VARARGS},

        {NULL, NULL},
    };

    Py_InitModule("pack", s_methods);
}
