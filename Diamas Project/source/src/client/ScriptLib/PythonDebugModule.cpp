#include "StdAfx.h"

PyObject *dbgLogBox(PyObject *poSelf, PyObject *poArgs)
{
    std::string szMsg;
    if (!PyTuple_GetString(poArgs, 0, szMsg))
        return Py_BadArgument();

    std::string szCaption;
    if (!PyTuple_GetString(poArgs, 1, szCaption))
        MessageBoxA(NULL, szMsg.c_str(), "Debug", MB_ICONERROR);
    else
        MessageBoxA(NULL, szMsg.c_str(), szCaption.c_str(), MB_ICONERROR);

    Py_RETURN_NONE;
}

PyObject *dbgLog(PyObject *self, PyObject *args)
{
    const char *what;
    int whatLen;
    int severity;
    const char *file;
    int line;
    const char *fn;
    if (!PyArg_ParseTuple(args, "s#isis", &what, &whatLen,
                          &severity, &file, &line, &fn))
        return NULL;

    fmt::string_view message(what, whatLen);

    spdlog::source_loc loc;
    loc.filename = file;
    loc.line = line;
    loc.funcname = fn;

    if (spdlog::get("client"))
        spdlog::log(loc, (spdlog::level::level_enum)severity, message);

    Py_RETURN_NONE;
}

extern "C" void initdbg()
{
    static PyMethodDef s_methods[] =
    {
        {"LogBox", dbgLogBox, METH_VARARGS},
        {"Log", dbgLog, METH_VARARGS},
        {NULL, NULL},
    };

    const auto m = Py_InitModule("dbg", s_methods);

    PyModule_AddIntConstant(m, "SEVERITY_INFO", spdlog::level::info);
    PyModule_AddIntConstant(m, "SEVERITY_WARNING", spdlog::level::warn);
    PyModule_AddIntConstant(m, "SEVERITY_ERROR", spdlog::level::err);
}
