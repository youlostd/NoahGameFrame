#include "StdAfx.h"
#include "PythonSafeBox.h"
#include <game/GamePacket.hpp>
#include "../GameLib/ItemManager.h"

void CPythonSafeBox::OpenSafeBox(int iSize)
{
    m_dwMoney = 0;
    m_ItemInstanceVector.clear();
    m_ItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

    for (uint32_t i = 0; i < m_ItemInstanceVector.size(); ++i)
    {
        ClientItemData &rInstance = m_ItemInstanceVector[i];
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
}

void CPythonSafeBox::SetItemData(uint32_t dwSlotIndex, const ClientItemData &rItemInstance)
{
    if (dwSlotIndex >= m_ItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::SetItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    m_ItemInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonSafeBox::DelItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::DelItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    ClientItemData &rInstance = m_ItemInstanceVector[dwSlotIndex];
    ZeroMemory(&rInstance, sizeof(rInstance));
}

void CPythonSafeBox::SetMoney(uint32_t dwMoney)
{
    m_dwMoney = dwMoney;
}

uint32_t CPythonSafeBox::GetMoney()
{
    return m_dwMoney;
}

int CPythonSafeBox::GetCurrentSafeBoxSize()
{
    return m_ItemInstanceVector.size();
}

bool CPythonSafeBox::GetSlotItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID)
{
    if (dwSlotIndex >= m_ItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *pdwItemID = m_ItemInstanceVector[dwSlotIndex].vnum;

    return TRUE;
}

bool CPythonSafeBox::GetItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance)
{
    if (dwSlotIndex >= m_ItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *ppInstance = &m_ItemInstanceVector[dwSlotIndex];

    return TRUE;
}

ClientItemData *CPythonSafeBox::GetItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return nullptr;
    }

    return &m_ItemInstanceVector[dwSlotIndex];
}

void CPythonSafeBox::OpenMall(int iSize)
{
    m_MallItemInstanceVector.clear();
    m_MallItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

    for (uint32_t i = 0; i < m_MallItemInstanceVector.size(); ++i)
    {
        ClientItemData &rInstance = m_MallItemInstanceVector[i];
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
}

void CPythonSafeBox::SetMallItemData(uint32_t dwSlotIndex, const ClientItemData &rItemData)
{
    if (dwSlotIndex >= m_MallItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::SetMallItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    m_MallItemInstanceVector[dwSlotIndex] = rItemData;
}

void CPythonSafeBox::DelMallItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_MallItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::DelMallItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    ClientItemData &rInstance = m_MallItemInstanceVector[dwSlotIndex];
    ZeroMemory(&rInstance, sizeof(rInstance));
}

bool CPythonSafeBox::GetMallItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance)
{
    if (dwSlotIndex >= m_MallItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *ppInstance = &m_MallItemInstanceVector[dwSlotIndex];

    return TRUE;
}

ClientItemData *CPythonSafeBox::GetMallItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_MallItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return nullptr;
    }

    return &m_MallItemInstanceVector[dwSlotIndex];
}

bool CPythonSafeBox::GetSlotMallItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID)
{
    if (dwSlotIndex >= m_MallItemInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *pdwItemID = m_MallItemInstanceVector[dwSlotIndex].vnum;

    return TRUE;
}

uint32_t CPythonSafeBox::GetMallSize()
{
    return m_MallItemInstanceVector.size();
}

CPythonSafeBox::CPythonSafeBox()
{
    m_dwMoney = 0;
}

CPythonSafeBox::~CPythonSafeBox()
{
}

PyObject *safeboxGetCurrentSafeboxSize(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonSafeBox::Instance().GetCurrentSafeBoxSize());
}

PyObject *safeboxGetItemID(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->vnum);
}

PyObject *safeboxGetItemTransmutationVnum(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->transVnum);
}

PyObject *safeboxGetItemCount(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->count);
}

PyObject *safeboxGetItemFlags(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    auto proto = CItemManager::instance().GetProto(pInstance->vnum);
    if (proto)
    {
        return Py_BuildValue("i", proto->GetFlags());
    }

    return Py_BuildValue("i", 0);
}

PyObject *safeboxGetItemMetinSocket(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BadArgument();
    int iSocketIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
        return Py_BadArgument();

    if (iSocketIndex >= ITEM_SOCKET_MAX_NUM)
        return Py_BuildException();

    ClientItemData *pItemData;
    if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
        return Py_BuildException();

    return Py_BuildValue("L", pItemData->sockets[iSocketIndex]);
}

PyObject *safeboxGetItemAttribute(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();
    int iAttrSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
        return Py_BuildException();

    if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_MAX_NUM)
    {
        ClientItemData *pItemData;
        if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
            return Py_BuildValue("id", pItemData->attrs[iAttrSlotIndex].bType, pItemData->attrs[iAttrSlotIndex].sValue);
    }

    return Py_BuildValue("ii", 0, 0);
}

PyObject *safeboxGetMoney(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonSafeBox::Instance().GetMoney());
}

PyObject *safeboxGetMallItemID(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->vnum);
}

PyObject *safeboxGetMallItemTransmutationVnum(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->transVnum);
}

PyObject *safeboxGetMallItemCount(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
        return Py_BadArgument();

    ClientItemData *pInstance;
    if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
        return Py_BuildException();

    return Py_BuildValue("i", pInstance->count);
}

PyObject *safeboxGetMallItemMetinSocket(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BadArgument();
    int iSocketIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
        return Py_BadArgument();

    if (iSocketIndex >= ITEM_SOCKET_MAX_NUM)
        return Py_BuildException();

    ClientItemData *pItemData;
    if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
        return Py_BuildException();

    return Py_BuildValue("L", pItemData->sockets[iSocketIndex]);
}

PyObject *safeboxGetMallItemAttribute(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();
    int iAttrSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
        return Py_BuildException();

    if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_MAX_NUM)
    {
        ClientItemData *pItemData;
        if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
            return Py_BuildValue("id", pItemData->attrs[iAttrSlotIndex].bType, pItemData->attrs[iAttrSlotIndex].sValue);
    }

    return Py_BuildValue("ii", 0, 0);
}

PyObject *safeboxGetMallSize(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonSafeBox::Instance().GetMallSize());
}

PyObject *safeboxIsOpen(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonSafeBox::Instance().IsOpen());
}

PyObject *safeboxSetOpen(PyObject *poSelf, PyObject *poArgs)
{
    bool iOpen;
    if (!PyTuple_GetBoolean(poArgs, 0, &iOpen))
        return Py_BadArgument();

    CPythonSafeBox::instance().SetOpen(iOpen);

    Py_RETURN_NONE;
}

void init_safebox(py::module &m)
{
    py::module safebox = m.def_submodule("safebox", "Provides safebox data access");

        py::class_<CPythonSafeBox, std::unique_ptr<CPythonSafeBox, py::nodelete>>(safebox, "safeboxInst")
        .def(py::init([]()
        {
            return std::unique_ptr<CPythonSafeBox, py::nodelete>(CPythonSafeBox::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)
        .def("GetMallItemData", &CPythonSafeBox::GetMallItemData, pybind11::return_value_policy::reference)
        .def("GetItemData", &CPythonSafeBox::GetItemData, pybind11::return_value_policy::reference)

    ;
}

extern "C" void initsafebox()
{
    static PyMethodDef s_methods[] =
    {
        // SafeBox
        {"GetCurrentSafeboxSize", safeboxGetCurrentSafeboxSize, METH_VARARGS},
        {"GetItemID", safeboxGetItemID, METH_VARARGS},
        {"GetItemChangeLookVnum", safeboxGetItemTransmutationVnum, METH_VARARGS},
        {"GetItemCount", safeboxGetItemCount, METH_VARARGS},
        {"GetItemFlags", safeboxGetItemFlags, METH_VARARGS},
        {"GetItemMetinSocket", safeboxGetItemMetinSocket, METH_VARARGS},
        {"GetItemAttribute", safeboxGetItemAttribute, METH_VARARGS},
        {"GetMoney", safeboxGetMoney, METH_VARARGS},

        // Mall
        {"GetMallItemID", safeboxGetMallItemID, METH_VARARGS},
        {"GetMallItemChangeLookVnum", safeboxGetMallItemTransmutationVnum, METH_VARARGS},
        {"GetMallItemCount", safeboxGetMallItemCount, METH_VARARGS},
        {"GetMallItemMetinSocket", safeboxGetMallItemMetinSocket, METH_VARARGS},
        {"GetMallItemAttribute", safeboxGetMallItemAttribute, METH_VARARGS},
        {"GetMallSize", safeboxGetMallSize, METH_VARARGS},

        {"IsOpen", safeboxIsOpen, METH_VARARGS},
        {"SetOpen", safeboxSetOpen, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("safebox", s_methods);
    PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_X_COUNT", CPythonSafeBox::SAFEBOX_SLOT_X_COUNT);
    PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_Y_COUNT", CPythonSafeBox::SAFEBOX_SLOT_Y_COUNT);
    PyModule_AddIntConstant(poModule, "SAFEBOX_PAGE_SIZE", CPythonSafeBox::SAFEBOX_PAGE_SIZE);
}
