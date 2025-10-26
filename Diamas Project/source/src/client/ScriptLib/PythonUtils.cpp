#include "StdAfx.h"
#include "PythonUtils.h"
#include "PyStr.hpp"

bool __PyCallClassMemberFunc_ByCString(PyObject *poClass, const char *c_szFunc, PyObject *poArgs, PyObject **poRet);
bool __PyCallClassMemberFunc_ByPyString(PyObject *poClass, PyObject *poFuncName, PyObject *poArgs, PyObject **poRet);
bool __PyCallClassMemberFunc(PyObject *poClass, PyObject *poFunc, PyObject *poArgs, PyObject **poRet);

PyObject *Py_BadArgument()
{
    return (PyObject*)PyErr_BadArgument();
}

PyObject *Py_BuildException(const char *c_pszErr, ...)
{
    if (!c_pszErr)
        PyErr_Clear();
    else
    {
        char szErrBuf[512 + 1];
        va_list args;
        va_start(args, c_pszErr);
        vsnprintf(szErrBuf, sizeof(szErrBuf), c_pszErr, args);
        va_end(args);

        PyErr_SetString(PyExc_RuntimeError, szErrBuf);
        return NULL;

    }

    Py_RETURN_NONE;
}

PyObject *Py_BuildNone()
{
    Py_INCREF(Py_None);
    return Py_None;
}

void Py_ReleaseNone()
{
    Py_DECREF(Py_None);
}

bool PyTuple_GetObject(PyObject *poArgs, int pos, PyObject **ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = poItem;
    return true;
}

bool PyTuple_GetLong(PyObject *poArgs, int pos, long *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsLong(poItem);
    return true;
}

bool PyTuple_GetDouble(PyObject *poArgs, int pos, double *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyFloat_AsDouble(poItem);
    return true;
}

bool PyTuple_GetFloat(PyObject *poArgs, int pos, float *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = float(PyFloat_AsDouble(poItem));
    return true;
}

bool PyTuple_GetByte(PyObject *poArgs, int pos, unsigned char *ret)
{
    int val;
    bool result = PyTuple_GetInteger(poArgs, pos, &val);
    *ret = unsigned char(val);
    return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, unsigned char *ret)
{
    int val;
    bool result = PyTuple_GetInteger(poArgs, pos, &val);
    *ret = unsigned char(val);
    return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, WORD *ret)
{
    int val;
    bool result = PyTuple_GetInteger(poArgs, pos, &val);
    *ret = WORD(val);
    return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, int *ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);
	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem);
	return true;
}

bool PyTuple_GetUnsignedLongLong(PyObject *poArgs, int pos, unsigned long long *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsUnsignedLongLongMask(poItem);
    return true;
}

bool PyTuple_GetLongLong(PyObject *poArgs, int pos, long long *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsLongLong(poItem);
    return true;
}

bool PyTuple_GetUnsignedLong(PyObject *poArgs, int pos, unsigned long *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsUnsignedLongMask(poItem);
    return true;
}

bool PyTuple_GetUnsignedInteger(PyObject *poArgs, int pos, unsigned int *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsUnsignedLongMask(poItem);
    return true;
}

bool PyTuple_GetString(PyObject *poArgs, int pos, std::string &ret)
{
    auto m = py::reinterpret_borrow<py::tuple>(poArgs);
    if (!m)
        return false;

    if (pos >= m.size())
        return false;

    const auto obj = m[pos];

    ret = py::cast<std::string>(obj);

    return true;
}

bool PyTuple_GetStringAndSize(PyObject *poArgs, int pos, char **ret, Py_ssize_t *size)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);
    if (!poItem)
        return false;

    if (!PyString_Check(poItem))
        return false;

    return PyString_AsStringAndSize(poItem, ret, size) >= 0;
}

bool PyTuple_GetBoolean(PyObject *poArgs, int pos, bool *ret)
{
    if (pos >= PyTuple_Size(poArgs))
        return false;

    PyObject *poItem = PyTuple_GetItem(poArgs, pos);

    if (!poItem)
        return false;

    *ret = PyLong_AsLong(poItem) ? true : false;
    return true;
}

bool PyCallClassMemberFunc(PyObject *o, const PyStr &member, PyObject *args, bool *ret)
{
    if (!o)
    {
        Py_XDECREF(args);
        return false;
    }

    if (o->ob_refcnt == 0)
    {
        Py_XDECREF(args);
        return false;
    }

    auto fn = PyObject_GetAttr(o, member.Get());
    if (!fn)
    {
        PyErr_Clear();
        Py_XDECREF(args);
        return false;
    }

    if (!PyCallable_Check(fn))
    {
        Py_DECREF(fn);
        Py_XDECREF(args);
        return false;
    }

    auto retObj = PyObject_CallObject(fn, args);

    Py_DECREF(fn);
    Py_XDECREF(args);

    if (!retObj)
    {
        if(PyErr_Occurred())
            PyErr_Print();
        return false;
    }

    if (ret)
        *ret = PyObject_IsTrue(retObj);

    Py_DECREF(retObj);
    return true;
}
