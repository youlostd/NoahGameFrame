#include "stdafx.h"
#include "PythonExchange.h"
#include <pybind11/stl.h>
PyObject *exchangeInitTrading(PyObject *poSelf, PyObject *poArgs)
{
    CPythonExchange::Instance().End();
    Py_RETURN_NONE;
}

PyObject *exchangeisTrading(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonExchange::Instance().isTrading());
}

PyObject *exchangeGetElkFromSelf(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("L", CPythonExchange::Instance().GetElkFromSelf());
}

PyObject *exchangeGetElkFromTarget(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("L", CPythonExchange::Instance().GetElkFromTarget());
}

PyObject *exchangeGetAcceptFromSelf(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonExchange::Instance().GetAcceptFromSelf());
}

PyObject *exchangeGetAcceptFromTarget(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonExchange::Instance().GetAcceptFromTarget());
}

PyObject *exchangeGetItemVnumFromSelf(PyObject *poSelf, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetItemVnumFromSelf((char)pos));
}

PyObject *exchangeGetItemVnumFromTarget(PyObject *poTarget, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetItemVnumFromTarget((char)pos));
}

PyObject *exchangeGetChangeLookVnumFromSelf(PyObject *poSelf, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetChangeLookVnumFromSelf((char)pos));
}

PyObject *exchangeGetChangeLookVnumFromTarget(PyObject *poTarget, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetChangeLookVnumFromTarget((char)pos));
}

PyObject *exchangeGetItemCountFromSelf(PyObject *poSelf, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetItemCountFromSelf((char)pos));
}

PyObject *exchangeGetItemCountFromTarget(PyObject *poTarget, PyObject *poArgs)
{
    int pos;

    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonExchange::Instance().GetItemCountFromTarget((char)pos));
}

PyObject *exchangeGetTargetVID(PyObject *poTarget, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonExchange::Instance().GetTargetVID());
}

PyObject *exchangeGetItemMetinSocketFromTarget(PyObject *poTarget, PyObject *poArgs)
{
    int pos;
    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();
    int iMetinSocketPos;
    if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketPos))
        return Py_BuildException();
    return Py_BuildValue("L", CPythonExchange::Instance().GetItemMetinSocketFromTarget(pos, iMetinSocketPos));
}

PyObject *exchangeGetItemMetinSocketFromSelf(PyObject *poTarget, PyObject *poArgs)
{
    int pos;
    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();
    int iMetinSocketPos;
    if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketPos))
        return Py_BuildException();
    return Py_BuildValue("L", CPythonExchange::Instance().GetItemMetinSocketFromSelf(pos, iMetinSocketPos));
}

PyObject *exchangeGetItemAttributeFromTarget(PyObject *poTarget, PyObject *poArgs)
{
    int pos;
    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();
    int iAttrSlotPos;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotPos))
        return Py_BuildException();

    ApplyType byType;
    ApplyValue sValue;
    CPythonExchange::Instance().GetItemAttributeFromTarget(pos, iAttrSlotPos, &byType, &sValue);

    return Py_BuildValue("id", byType, sValue);
}

PyObject *exchangeGetItemAttributeFromSelf(PyObject *poTarget, PyObject *poArgs)
{
    int pos;
    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BuildException();
    int iAttrSlotPos;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotPos))
        return Py_BuildException();

    ApplyType byType;
    ApplyValue sValue;
    CPythonExchange::Instance().GetItemAttributeFromSelf(pos, iAttrSlotPos, &byType, &sValue);

    return Py_BuildValue("id", byType, sValue);
}

PyObject *exchangeGetElkMode(PyObject *poTarget, PyObject *poArgs)
{
    return Py_BuildValue("b", CPythonExchange::Instance().GetElkMode());
}

PyObject *exchangeSetElkMode(PyObject *poTarget, PyObject *poArgs)
{
    int elk_mode;

    if (!PyTuple_GetInteger(poArgs, 0, &elk_mode))
        return Py_BuildException();

    CPythonExchange::Instance().SetElkMode(elk_mode ? true : false);
    Py_RETURN_NONE;
}

extern "C" void initexchange()
{
    static PyMethodDef s_methods[] =
    {
        {"InitTrading", exchangeInitTrading, METH_VARARGS},
        {"isTrading", exchangeisTrading, METH_VARARGS},

        {"GetElkFromSelf", exchangeGetElkFromSelf, METH_VARARGS},
        {"GetElkFromTarget", exchangeGetElkFromTarget, METH_VARARGS},

        {"GetItemVnumFromSelf", exchangeGetItemVnumFromSelf, METH_VARARGS},
        {"GetItemVnumFromTarget", exchangeGetItemVnumFromTarget, METH_VARARGS},

        {"GetChangeLookVnumFromSelf", exchangeGetChangeLookVnumFromSelf, METH_VARARGS},
        {"GetChangeLookVnumFromTarget", exchangeGetChangeLookVnumFromTarget, METH_VARARGS},

        {"GetItemCountFromSelf", exchangeGetItemCountFromSelf, METH_VARARGS},
        {"GetItemCountFromTarget", exchangeGetItemCountFromTarget, METH_VARARGS},

        {"GetAcceptFromSelf", exchangeGetAcceptFromSelf, METH_VARARGS},
        {"GetAcceptFromTarget", exchangeGetAcceptFromTarget, METH_VARARGS},

        {"GetTargetVID", exchangeGetTargetVID, METH_VARARGS},

        {"GetItemMetinSocketFromTarget", exchangeGetItemMetinSocketFromTarget, METH_VARARGS},
        {"GetItemMetinSocketFromSelf", exchangeGetItemMetinSocketFromSelf, METH_VARARGS},

        {"GetItemAttributeFromTarget", exchangeGetItemAttributeFromTarget, METH_VARARGS},
        {"GetItemAttributeFromSelf", exchangeGetItemAttributeFromSelf, METH_VARARGS},

        {"GetElkMode", exchangeGetElkMode, METH_VARARGS},
        {"SetElkMode", exchangeSetElkMode, METH_VARARGS},

        {NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("exchange", s_methods);
    PyModule_AddIntConstant(poModule, "EXCHANGE_ITEM_MAX_NUM", CPythonExchange::EXCHANGE_ITEM_MAX_NUM);
}


void init_exchange(py::module &m)
{
    py::module exchange = m.def_submodule("exchange", "");

    auto cps = py::class_<CPythonExchange, std::unique_ptr<CPythonExchange, py::nodelete>>(exchange, "exchangeInst")
                   .def(py::init([]() {
                            return std::unique_ptr<CPythonExchange, py::nodelete>(CPythonExchange::InstancePtr());
                        }),
                        pybind11::return_value_policy::reference_internal);

    cps.def("GetItemDataFromSelf", &CPythonExchange::GetItemDataFromSelf, pybind11::return_value_policy::reference);
    cps.def("GetItemDataFromTarget", &CPythonExchange::GetItemDataFromTarget, pybind11::return_value_policy::reference);
    cps.def("InitTrading", &CPythonExchange::End);
    cps.def("IsTrading", &CPythonExchange::isTrading);
    cps.def("GetTargetVID", &CPythonExchange::GetTargetVID);
    cps.def("GetElkMode", &CPythonExchange::GetElkMode);
    cps.def("SetElkMode", &CPythonExchange::SetElkMode);
}
