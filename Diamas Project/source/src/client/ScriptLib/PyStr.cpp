#include "StdAfx.h"
#include "PyStr.hpp"
#include <Python.h>

PyStr::PyStr()
    : m_o(nullptr)
{
    // ctor
}

PyStr::PyStr(const char *str)
    : m_o(PyString_FromString(str))
{
    // ctor
}

PyStr::PyStr(std::string_view str)
    : m_o(PyString_FromStringAndSize(str.data(), str.length()))
{
    // ctor
}

PyStr::PyStr(const std::string &str)
    : m_o(PyString_FromStringAndSize(str.data(), str.length()))
{
    // ctor
}

PyStr::PyStr(const PyStr &str)
    : m_o(str.m_o)
{
    Py_XINCREF(m_o);
}

PyStr::~PyStr()
{
    Py_XDECREF(m_o);
}

PyStr &PyStr::operator=(const PyStr &str)
{
    Py_XDECREF(m_o);
    m_o = str.m_o;
    Py_XINCREF(m_o);
    return *this;
}

PyObject *PyStr::Get() const
{
    return m_o;
}

std::string_view PyStr::ToString() const
{
    if (!m_o)
        return std::string_view();

    return std::string_view(PyString_AsString(m_o),
                            PyString_Size(m_o));
}
