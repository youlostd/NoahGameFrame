#include "StdAfx.h"

#include <pak/Vfs.hpp>

#include "../EterBase/MappedFile.h"

#include "PythonLauncher.h"
#include <boost/algorithm/string.hpp>

#define METIN2_MODULE(name) extern "C" void init ## name();
#include "ModuleList.inl"
#undef METIN2_MODULE

static PyObject *WriteToStderr(PyObject *self, PyObject *args)
{
    const char *what;
    int len;
    if (!PyArg_ParseTuple(args, "s#", &what, &len))
        return Py_BuildException("Invalid eprint");

    std::string msg(what, len);
    boost::trim(msg);

    if (spdlog::default_logger_raw())
        SPDLOG_ERROR("{}", msg);
    Py_RETURN_NONE;
}

static PyObject *WriteToStdout(PyObject *self, PyObject *args)
{
    const char *what;
    int len;
    if (!PyArg_ParseTuple(args, "s#", &what, &len))
        return Py_BuildException("Invalid print");

    std::string msg(what, len);
    boost::trim(msg);
    if (spdlog::default_logger_raw())
        SPDLOG_INFO("{}", msg);
    Py_RETURN_NONE;
}

namespace
{
static _inittab kModules[] = {

#define METIN2_MODULE(name) { #name, init ## name },
#include "ModuleList.inl"
#undef METIN2_MODULE

    {nullptr, nullptr}
};
}

CPythonLauncher::CPythonLauncher()
{
#ifdef  _DEBUG
    Py_VerboseFlag = 1;
#else
	Py_VerboseFlag = 0;
#endif

    Py_OptimizeFlag = !Py_VerboseFlag;
    Py_NoSiteFlag = 1;
    Py_DontWriteBytecodeFlag = 1;
    Py_IgnoreEnvironmentFlag = 1;
    Py_FrozenFlag = 1; // Suppress errors from getpath.c

    Py_SetProgramName(const_cast<char *>("ADALET"));
    if(PyImport_ExtendInittab(kModules) == -1)
        OutputDebugString("FAILED TO EXTEND INIT TAB");

    Py_Initialize();
    PySys_SetArgvEx(__argc, __argv, 0);

    InitializeLogging();

#ifndef __USE_CYTHON__
    PyObject *sysPath = PySys_GetObject((char *)"path");
    PyObject *curDir = PyString_FromString(".");
    PyList_Append(sysPath, curDir);
    Py_DECREF(curDir);
#endif
}

CPythonLauncher::~CPythonLauncher()
{
    Py_Finalize();
}

void CPythonLauncher::InitializeLogging()
{
    static PyMethodDef err_methods[] = {
        {"write", WriteToStderr, METH_VARARGS, "stderr"},
        {NULL, NULL, 0, NULL}
    };

    static PyMethodDef out_methods[] = {
        {"write", WriteToStdout, METH_VARARGS, "stdout"},
        {NULL, NULL, 0, NULL}
    };
    auto m1 = Py_InitModule("__logerr", err_methods);
    auto m2 = Py_InitModule("__logout", out_methods);

    PySys_SetObject("stderr", m1);
    PySys_SetObject("stdout", m2);
}

bool CPythonLauncher::Run()
{
    try {
        py::module_ m_mainModule = py::module_::import("system");
    } catch(py::error_already_set& e) {
        SPDLOG_CRITICAL(e.what());
    }


    return true;
}
