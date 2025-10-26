#include "StdAfx.h"

#ifdef __OFFLINE_SHOP__
#include "PythonOfflineShop.h"

PyObject *offshopSendCreateMyShopPacket(PyObject *poSelf, PyObject *poArgs)
{
    CPythonOfflineShop::instance().SendCreateMyShopPacket();
    Py_RETURN_NONE;
}

PyObject *offshopSendCloseMyShopPacket(PyObject *poSelf, PyObject *poArgs)
{
    CPythonOfflineShop::instance().SendCloseMyShopPacket();
    Py_RETURN_NONE;
}

PyObject *offshopSendCloseOtherShopPacket(PyObject *poSelf, PyObject *poArgs)
{
    CPythonOfflineShop::instance().SendCloseOtherShopPacket();
    Py_RETURN_NONE;
}

PyObject *offshopSendCreatePacket(PyObject *poSelf, PyObject *poArgs)
{
    std::string szShopName;
    float fX, fY;
    bool bIsShowEditor;
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	int iOpeningTime;
#endif
    if (!PyTuple_GetString(poArgs, 0, szShopName) ||
        !PyTuple_GetFloat(poArgs, 1, &fX) ||
        !PyTuple_GetFloat(poArgs, 2, &fY) ||
        !PyTuple_GetBoolean(poArgs, 4, &bIsShowEditor)
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	        || !PyTuple_GetInteger(poArgs, 5, &iOpeningTime)
#endif
    )
    {
        return Py_BuildException();
    }

    PyObject *poDict = PyTuple_GetItem(poArgs, 3);
    if (!poDict)
    {
        return Py_BuildException();
    }

    std::vector<TOfflineShopCGItemInfo> *pvecItems = new std::vector<
        TOfflineShopCGItemInfo>();

    PyObject *poKeys = PyDict_Keys(poDict);
    PyObject *poValues = PyDict_Values(poDict);

    PyObject *poKey, *poValue;
    for (int i = 0; i < PyList_GET_SIZE(poKeys); ++i)
    {
        poKey = PyList_GET_ITEM(poKeys, i);
        poValue = PyList_GET_ITEM(poValues, i);

        TOfflineShopCGItemInfo kItemInfo;
        kItemInfo.dwDisplayPosition = PyLong_AsUnsignedLong(poKey);

        PyObject *poSubValues = PyDict_Values(poValue);
        kItemInfo.kInventoryPosition.window_type = ((uint8_t)PyLong_AsLong(
            PyDict_GetItem(poValue, Py_BuildValue("s", "SLOT_TYPE"))));
        kItemInfo.kInventoryPosition.cell = ((uint16_t)PyLong_AsLong(
            PyDict_GetItem(poValue, Py_BuildValue("s", "SLOT_INDEX"))));

        kItemInfo.llPrice = PyLong_AsLongLong(PyDict_GetItem(poValue, Py_BuildValue("s", "PRICE")));

        pvecItems->push_back(kItemInfo);
    }

    CPythonOfflineShop::instance().SendCreatePacket(
        szShopName.c_str(),
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
	    fX, fY,
#endif
        bIsShowEditor,
        pvecItems
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	    , iOpeningTime
#endif
        );

    delete(pvecItems);

    Py_RETURN_NONE;
}

PyObject *offshopSendShopNamePacket(PyObject *poSelf, PyObject *poArgs)
{
    std::string szShopName;
    if (!PyTuple_GetString(poArgs, 0, szShopName))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendShopNamePacket(szShopName.c_str());
    Py_RETURN_NONE;
}

#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
PyObject* offshopSendShopPositionPacket(PyObject* poSelf, PyObject* poArgs)
{
	float fX, fY;
	if (!PyTuple_GetFloat(poArgs, 0, &fX) ||
	        !PyTuple_GetFloat(poArgs, 1, &fY))
	{
		return Py_BuildException();
	}

	CPythonOfflineShop::instance().SendShopPositionPacket(fX, fY);
	Py_RETURN_NONE;
}
#endif

PyObject *offshopSendAddItemPacket(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos kInventoryPosition;
    DWORD dwDisplayPosition;
    LONGLONG llPrice;

    if (!PyTuple_GetInteger(poArgs, 0, &kInventoryPosition.window_type) ||
        !PyTuple_GetInteger(poArgs, 1, &kInventoryPosition.cell) ||
        !PyTuple_GetUnsignedLong(poArgs, 2, &dwDisplayPosition) ||
        !PyTuple_GetLongLong(poArgs, 3, &llPrice)
    )
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendAddItemPacket(&kInventoryPosition, dwDisplayPosition, llPrice);
    Py_RETURN_NONE;
}

PyObject *offshopSendMoveItemPacket(PyObject *poSelf, PyObject *poArgs)
{
    DWORD dwOldDisplayPosition, dwNewDisplayPosition;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &dwOldDisplayPosition) ||
        !PyTuple_GetUnsignedLong(poArgs, 1, &dwNewDisplayPosition))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendMoveItemPacket(dwOldDisplayPosition, dwNewDisplayPosition);
    Py_RETURN_NONE;
}

PyObject *offshopSendRemoveItemPacket(PyObject *poSelf, PyObject *poArgs)
{
    DWORD dwDisplayPosition;
    TItemPos kInventoryPosition = NPOS;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &dwDisplayPosition) ||
        !PyTuple_GetByte(poArgs, 1, &kInventoryPosition.window_type) ||
        !PyTuple_GetInteger(poArgs, 2, &kInventoryPosition.cell))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendRemoveItemPacket(dwDisplayPosition, &kInventoryPosition);
    Py_RETURN_NONE;
}

PyObject *offshopSendBuyItemPacket(PyObject *poSelf, PyObject *poArgs)
{
    DWORD dwDisplayPosition;
    TItemPos kInventoryPosition;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &dwDisplayPosition) ||
        !PyTuple_GetByte(poArgs, 1, &kInventoryPosition.window_type) ||
        !PyTuple_GetInteger(poArgs, 2, &kInventoryPosition.cell))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendBuyItemPacket(dwDisplayPosition, &kInventoryPosition);
    Py_RETURN_NONE;
}

PyObject *offshopSendWithdrawGoldPacket(PyObject *poSelf, PyObject *poArgs)
{
    LONGLONG llGold;
    if (!PyTuple_GetLongLong(poArgs, 0, &llGold))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendWithdrawGoldPacket(llGold);
    Py_RETURN_NONE;
}

PyObject *offshopSendClickShopPacket(PyObject *poSelf, PyObject *poArgs)
{
    DWORD dwShopOwnerPID;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &dwShopOwnerPID))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SendClickShopPacket(dwShopOwnerPID);
    Py_RETURN_NONE;
}

PyObject *offshopSendReopenShopPacket(PyObject *poSelf, PyObject *poArgs)
{
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	int iOpeningTime;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpeningTime))
	{
		return Py_BuildException();
	}

	CPythonOfflineShop::instance().SendReopenShopPacket(iOpeningTime);
#else
    CPythonOfflineShop::instance().SendReopenShopPacket();
#endif
    Py_RETURN_NONE;
}

PyObject *offshopSetPythonHandler(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poHandler))
    {
        return Py_BuildException();
    }

    CPythonOfflineShop::instance().SetPythonHandler(poHandler);
    Py_RETURN_NONE;
}

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
PyObject *offshopSendCancelCreateShopPacket(PyObject *poSelf, PyObject *poArgs)
{
    CPythonOfflineShop::instance().SendCancelCreateShopPacket();
    Py_RETURN_NONE;
}
#endif

extern "C" void initoffshop()
{
    static PyMethodDef s_methods[] =
    {
        {"SendCreateMyShopPacket", offshopSendCreateMyShopPacket, METH_VARARGS},
        {"SendCloseMyShopPacket", offshopSendCloseMyShopPacket, METH_VARARGS},
        {"SendCloseOtherShopPacket", offshopSendCloseOtherShopPacket, METH_VARARGS},

        {"SendCreatePacket", offshopSendCreatePacket, METH_VARARGS},

        {"SendShopNamePacket", offshopSendShopNamePacket, METH_VARARGS},
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
		{ "SendShopPositionPacket",		offshopSendShopPositionPacket,		METH_VARARGS },
#endif
        {"SendAddItemPacket", offshopSendAddItemPacket, METH_VARARGS},
        {"SendMoveItemPacket", offshopSendMoveItemPacket, METH_VARARGS},
        {"SendRemoveItemPacket", offshopSendRemoveItemPacket, METH_VARARGS},
        {"SendBuyItemPacket", offshopSendBuyItemPacket, METH_VARARGS},
        {"SendWithdrawGoldPacket", offshopSendWithdrawGoldPacket, METH_VARARGS},

        {"SendClickShopPacket", offshopSendClickShopPacket, METH_VARARGS},

        {"SendReopenShopPacket", offshopSendReopenShopPacket, METH_VARARGS},

        {"SetPythonHandler", offshopSetPythonHandler, METH_VARARGS},

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
        {"SendCancelCreateShopPacket", offshopSendCancelCreateShopPacket, METH_VARARGS},
#endif

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("offshop", s_methods);

    PyModule_AddIntConstant(poModule, "CLEAR_SHOP", EOfflineShopGCSubHeader::CLEAR_SHOP);
    PyModule_AddIntConstant(poModule, "CREATE_SHOP", EOfflineShopGCSubHeader::CREATE_SHOP);
    PyModule_AddIntConstant(poModule, "OPEN_SHOP", EOfflineShopGCSubHeader::OPEN_SHOP);
    PyModule_AddIntConstant(poModule, "OPEN_MY_SHOP", EOfflineShopGCSubHeader::OPEN_MY_SHOP);
    PyModule_AddIntConstant(poModule, "CLOSE_SHOP", EOfflineShopGCSubHeader::CLOSE_SHOP);
#ifndef __OFFLINE_SHOP_OPENING_TIME__
    PyModule_AddIntConstant(poModule, "OPEN_MY_SHOP_FOR_OTHERS",
                            EOfflineShopGCSubHeader::OPEN_MY_SHOP_FOR_OTHERS);
    PyModule_AddIntConstant(poModule, "CLOSE_MY_SHOP_FOR_OTHERS",
                            EOfflineShopGCSubHeader::CLOSE_MY_SHOP_FOR_OTHERS);
#endif

#ifdef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
    PyModule_AddIntConstant(poModule, "ALLOW_MANUAL_POSITIONING", FALSE);
#else
	PyModule_AddIntConstant(poModule, "ALLOW_MANUAL_POSITIONING",	TRUE);
#endif
}
#endif
