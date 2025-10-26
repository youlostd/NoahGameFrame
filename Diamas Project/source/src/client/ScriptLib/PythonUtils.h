#pragma once

#define SET_EXCEPTION(x) PyErr_SetString(PyExc_RuntimeError, #x)

#include "PyStr.hpp"
#include <pybind11/pybind11.h>

bool PyTuple_GetString(PyObject* poArgs, int pos, std::string& ret);
bool PyTuple_GetStringAndSize(PyObject* poArgs, int pos, char** ret, Py_ssize_t* size);

bool PyTuple_GetInteger(PyObject* poArgs, int pos, unsigned char* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, int* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, WORD* ret);
bool PyTuple_GetByte(PyObject* poArgs, int pos, unsigned char* ret);
bool PyTuple_GetUnsignedInteger(PyObject* poArgs, int pos, unsigned int* ret);
bool PyTuple_GetLong(PyObject* poArgs, int pos, long* ret);
bool PyTuple_GetUnsignedLong(PyObject* poArgs, int pos, unsigned long* ret);
bool PyTuple_GetUnsignedLongLong(PyObject* poArgs, int pos, unsigned long long* ret);
bool PyTuple_GetLongLong(PyObject* poArgs, int pos, long long* ret);
bool PyTuple_GetFloat(PyObject* poArgs, int pos, float* ret);
bool PyTuple_GetDouble(PyObject* poArgs, int pos, double* ret);
bool PyTuple_GetObject(PyObject* poArgs, int pos, PyObject** ret);
bool PyTuple_GetBoolean(PyObject* poArgs, int pos, bool* ret);


bool PyCallClassMemberFunc(PyObject* o, const PyStr& member, PyObject* args,
	bool* ret = nullptr);

template <typename... Args> bool PyCallClassMemberFunc(py::handle o, const std::string &attr, Args &&... arg)
{
    if (o && !py::isinstance<py::none>(o) && py::hasattr(o, attr.c_str()))
    {

        auto func = o.attr(attr.c_str());
        if (func.is_none())
            return false;
        try
        {
            auto retObj = func(std::forward<Args>(arg)...);
            if (py::isinstance<py::bool_>(retObj))
                return py::cast<bool>(retObj);

            return true;
        }
        catch (py::error_already_set &e)
        {
            SPDLOG_ERROR("{}", e.what());
            return false;
        }
    }

    return false;
}


PyObject * Py_BuildException(const char * c_pszErr = NULL, ...);
PyObject * Py_BadArgument();
PyObject * Py_BuildNone();
PyObject * Py_BuildEmptyTuple();

//Think - 27/04/14 - More intuitive reference to follow the DWORD usage
static auto &&PyTuple_GetDWORD = PyTuple_GetUnsignedLong;