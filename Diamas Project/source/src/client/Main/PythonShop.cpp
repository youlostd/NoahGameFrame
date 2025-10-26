#include "stdafx.h"
#include "PythonShop.h"

#include "PythonNetworkStream.h"
#include <game/GamePacket.hpp>
#include "PythonApplication.h"
#include <pybind11/stl.h>
//bool CPythonShop::GetSlotItemID(uint32_t dwSlotPos, uint32_t* pdwItemID)
//{
//	if (!CheckSlotIndex(dwSlotPos))
//		return FALSE;
//	const TShopItemTable * itemData;
//	if (!GetItemData(dwSlotPos, &itemData))
//		return FALSE;
//	*pdwItemID=itemData->vnum;
//	return TRUE;
//}
void CPythonShop::SetTabCoinType(uint8_t tabIdx, uint8_t coinType)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return;
    }
    m_aShoptabs[tabIdx].coinType = coinType;
}

void CPythonShop::SetTabCoinVnum(uint8_t tabIdx, uint32_t coinType)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return;
    }
    m_aShoptabs[tabIdx].coinVnum = coinType;
}

uint8_t CPythonShop::GetTabCoinType(uint8_t tabIdx)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return 0xff;
    }
    return m_aShoptabs[tabIdx].coinType;
}

uint32_t CPythonShop::GetTabCoinVnum(uint8_t tabIdx)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return 0xff;
    }
    return m_aShoptabs[tabIdx].coinVnum;
}

void CPythonShop::SetTabName(uint8_t tabIdx, const char *name)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return;
    }
    m_aShoptabs[tabIdx].name = name;
}

const char *CPythonShop::GetTabName(uint8_t tabIdx)
{
    if (tabIdx >= m_tabCount)
    {
        SPDLOG_ERROR("Out of Index. tabIdx({0}) must be less than {1}.", tabIdx, SHOP_TAB_COUNT_MAX);
        return NULL;
    }

    return m_aShoptabs[tabIdx].name.c_str();
}

const CPythonShop::ShopTab* CPythonShop::GetShopTab(uint8_t tabIdx) {
    if (tabIdx >= m_tabCount) {
        return nullptr;
    }

    return &m_aShoptabs[tabIdx];
}

void CPythonShop::SetItemData(uint32_t slot, const packet_shop_item &item)
{
    SetItemData(slot / SHOP_HOST_ITEM_MAX_NUM,
                slot % SHOP_HOST_ITEM_MAX_NUM, item);
}

const packet_shop_item *CPythonShop::GetItemData(uint32_t slot) const
{
    return GetItemData(slot / SHOP_HOST_ITEM_MAX_NUM,
                       slot % SHOP_HOST_ITEM_MAX_NUM);
}

void CPythonShop::SetItemData(uint8_t tab, uint32_t slot, const packet_shop_item &item)
{
    if (tab >= SHOP_TAB_COUNT_MAX || slot >= SHOP_HOST_ITEM_MAX_NUM)
    {
        SPDLOG_ERROR(
            "Out of Index. tab({0}) must be less than {1}. "
            "slot({2}) must be less than {3}",
            tab, SHOP_TAB_COUNT_MAX, slot, SHOP_HOST_ITEM_MAX_NUM);
        return;
    }

    m_aShoptabs[tab].items[slot] = item;
}

const packet_shop_item *CPythonShop::GetItemData(uint8_t tab, uint32_t slot) const
{
    if (tab >= SHOP_TAB_COUNT_MAX ||
        slot >= SHOP_HOST_ITEM_MAX_NUM)
    {
        SPDLOG_ERROR(
            "Out of Index. tab({0}) must be less than {1}. "
            "slot({2}) must be less than {3}",
            tab, SHOP_TAB_COUNT_MAX, slot,
            SHOP_HOST_ITEM_MAX_NUM);
        return nullptr;
    }

    return &m_aShoptabs[tab].items[slot];
}

void CPythonShop::ClearPrivateShopStock()
{
    m_PrivateShopItemStock.clear();
}

void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, uint8_t byDisplayPos, Gold price)
{
    DelPrivateShopItemStock(ItemPos);

    TShopItemTable SellingItem{};
    SellingItem.vnum = 0;
    SellingItem.transVnum = 0;
    SellingItem.count = 0;
    SellingItem.pos = ItemPos;
    SellingItem.price = price;
    SellingItem.display_pos = byDisplayPos;
    m_PrivateShopItemStock.emplace(ItemPos, SellingItem);
}

void CPythonShop::DelPrivateShopItemStock(TItemPos ItemPos)
{
    if (m_PrivateShopItemStock.end() == m_PrivateShopItemStock.find(ItemPos))
        return;

    m_PrivateShopItemStock.erase(ItemPos);
}

Gold CPythonShop::GetPrivateShopItemPrice(TItemPos ItemPos)
{
    const auto itor = m_PrivateShopItemStock.find(ItemPos);
    if (m_PrivateShopItemStock.end() == itor)
        return 0;

    return itor->second.price;
}

struct ItemStockSortFunc
{
    bool operator()(TShopItemTable &rkLeft, TShopItemTable &rkRight)
    {
        return rkLeft.display_pos < rkRight.display_pos;
    }
};

void CPythonShop::BuildPrivateShop(const char *sign, const TItemPos &bundleItem)
{
    std::vector<TShopItemTable> ItemStock;
    ItemStock.reserve(m_PrivateShopItemStock.size());

    for (const auto &p : m_PrivateShopItemStock)
        ItemStock.push_back(p.second);

    std::sort(ItemStock.begin(), ItemStock.end(), ItemStockSortFunc());

    gPythonNetworkStream->SendMyShopOpenPacket(sign, bundleItem, ItemStock);
}

void CPythonShop::Open(bool isPrivateShop, bool isMainPrivateShop)
{
    m_isShoping = true;
    m_isPrivateShop = isPrivateShop;
    m_isMainPlayerPrivateShop = isMainPrivateShop;
}

void CPythonShop::Close()
{
    m_isShoping = false;
    m_isPrivateShop = false;
    m_isMainPlayerPrivateShop = false;
}

bool CPythonShop::IsOpen()
{
    return m_isShoping;
}

bool CPythonShop::IsPrivateShop()
{
    return m_isPrivateShop;
}

bool CPythonShop::IsMainPlayerPrivateShop()
{
    return m_isMainPlayerPrivateShop;
}

void CPythonShop::Clear()
{
    m_isShoping = false;
    m_isPrivateShop = false;
    m_isMainPlayerPrivateShop = false;
    ClearPrivateShopStock();
    m_tabCount = 1;

    for (int i = 0; i < SHOP_TAB_COUNT_MAX; i++)
    {
        m_aShoptabs[i].coinType = SHOP_COIN_TYPE_GOLD;
        m_aShoptabs[i].coinVnum = 0;
        m_aShoptabs[i].name.clear();
        memset(m_aShoptabs[i].items, 0,
               sizeof(packet_shop_item) * SHOP_HOST_ITEM_MAX_NUM);
    }
}

CPythonShop::CPythonShop()
    : m_isShoping(false)
      , m_isPrivateShop(false)
      , m_isMainPlayerPrivateShop(false)
      , m_tabCount(1)
{
    // ctor
}

CPythonShop::~CPythonShop()
{
}

CPythonShop::ShopTab::ShopTab()
    : coinType(SHOP_COIN_TYPE_GOLD), coinVnum(0)
{
    std::memset(items, 0, sizeof(items));
}

PyObject *shopOpen(PyObject *poSelf, PyObject *poArgs)
{
    int isPrivateShop = false;
    PyTuple_GetInteger(poArgs, 0, &isPrivateShop);
    int isMainPrivateShop = false;
    PyTuple_GetInteger(poArgs, 1, &isMainPrivateShop);

    CPythonShop &rkShop = CPythonShop::Instance();
    rkShop.Open(isPrivateShop, isMainPrivateShop);
    Py_RETURN_NONE;
}

PyObject *shopClose(PyObject *poSelf, PyObject *poArgs)
{
    CPythonShop &rkShop = CPythonShop::Instance();
    rkShop.Close();
    Py_RETURN_NONE;
}

PyObject *shopIsOpen(PyObject *poSelf, PyObject *poArgs)
{
    CPythonShop &rkShop = CPythonShop::Instance();
    return Py_BuildValue("i", rkShop.IsOpen());
}

PyObject *shopIsPrviateShop(PyObject *poSelf, PyObject *poArgs)
{
    CPythonShop &rkShop = CPythonShop::Instance();
    return Py_BuildValue("i", rkShop.IsPrivateShop());
}

PyObject *shopIsMainPlayerPrivateShop(PyObject *poSelf, PyObject *poArgs)
{
    CPythonShop &rkShop = CPythonShop::Instance();
    return Py_BuildValue("i", rkShop.IsMainPlayerPrivateShop());
}

PyObject *shopGetItemID(PyObject *poSelf, PyObject *poArgs)
{
    int nPos;
    if (!PyTuple_GetInteger(poArgs, 0, &nPos))
        return Py_BuildException();

    const auto itemData = CPythonShop::Instance().GetItemData(nPos);
    if (itemData)
        return Py_BuildValue("i", itemData->vnum);

    return Py_BuildValue("i", 0);
}

PyObject *shopGetItemData(PyObject *poSelf, PyObject *poArgs)
{
    int nPos;
    if (!PyTuple_GetInteger(poArgs, 0, &nPos))
        return Py_BuildException();

    auto itemData = CPythonShop::Instance().GetItemData(nPos);
    if (itemData)
    {
        try
        {
            py::dict data;
            data["vnum"] = py::cast(itemData->vnum);
            data["transVnum"] = py::cast(itemData->transVnum);
            data["price"] = py::cast(itemData->price);
            data["count"] = py::cast(itemData->count);
            data["display_pos"] = py::cast(itemData->display_pos);
            //data["sockets"] = c_pItemData->alSockets;
            //data["attrs"] = c_pItemData->aAttr;

            return Py_BuildValue("O", data.ptr());
        }
        catch (pybind11::error_already_set &e)
        {
            SPDLOG_ERROR("Python exception:", e.what());
            return Py_BuildValue("i", 0);
        }
    }

    return Py_BuildValue("i", 0);
}

PyObject *shopGetItemChangeLookVnum(PyObject *poSelf, PyObject *poArgs)
{
    int nPos;
    if (!PyTuple_GetInteger(poArgs, 0, &nPos))
        return Py_BuildException();

    const auto itemData = CPythonShop::Instance().GetItemData(nPos);
    if (itemData)
        return Py_BuildValue("i", itemData->transVnum);

    return Py_BuildValue("i", 0);
}

PyObject *shopGetItemCount(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    const auto itemData = CPythonShop::Instance().GetItemData(iIndex);
    if (itemData)
        return Py_BuildValue("i", itemData->count);

    return Py_BuildValue("i", 0);
}

PyObject *shopGetItemPrice(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    const auto itemData = CPythonShop::Instance().GetItemData(iIndex);
    if (itemData)
        return Py_BuildValue("L", itemData->price);

    return Py_BuildValue("L", 0);
}

PyObject *shopGetItemMetinSocket(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();
    int iMetinSocketIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
        return Py_BuildException();

    const auto itemData = CPythonShop::Instance().GetItemData(iIndex);
    if (itemData)
        return Py_BuildValue("L", itemData->GetSocket(iMetinSocketIndex));

    return Py_BuildValue("L", 0);
}

PyObject *shopGetItemAttribute(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();
    int iAttrSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
        return Py_BuildException();

    if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_MAX_NUM)
    {
        const auto* itemData = CPythonShop::Instance().GetItemData(iIndex);
        if (itemData)
            return Py_BuildValue("id", itemData->GetAttr(iAttrSlotIndex).bType,
                                 itemData->GetAttr(iAttrSlotIndex).sValue);
    }

    return Py_BuildValue("id", 0, 0);
}

PyObject *shopClearPrivateShopStock(PyObject *poSelf, PyObject *poArgs)
{
    CPythonShop::Instance().ClearPrivateShopStock();
    Py_RETURN_NONE;
}

PyObject *shopAddPrivateShopItemStock(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bItemWindowType;
    if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
        return Py_BuildException();
    uint16_t wItemSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
        return Py_BuildException();
    int iDisplaySlotIndex;
    if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
        return Py_BuildException();
    Gold iPrice;
    if (!PyTuple_GetLongLong(poArgs, 3, &iPrice))
        return Py_BuildException();

    CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex,
                                                    iPrice);
    Py_RETURN_NONE;
}

PyObject *shopDelPrivateShopItemStock(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bItemWindowType;
    if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
        return Py_BuildException();
    uint16_t wItemSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
        return Py_BuildException();

    CPythonShop::Instance().DelPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex));
    Py_RETURN_NONE;
}

PyObject *shopGetPrivateShopItemPrice(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bItemWindowType;
    if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
        return Py_BuildException();
    uint16_t wItemSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
        return Py_BuildException();

    Gold iValue = CPythonShop::Instance().GetPrivateShopItemPrice(TItemPos(bItemWindowType, wItemSlotIndex));
    return Py_BuildValue("L", iValue);
}

PyObject *shopBuildPrivateShop(PyObject *poSelf, PyObject *poArgs)
{
    std::string sign;
    if (!PyTuple_GetString(poArgs, 0, sign))
        return Py_BadArgument();

    TItemPos bundleItem;
    if (!PyTuple_GetInteger(poArgs, 1, &bundleItem.window_type) ||
        !PyTuple_GetInteger(poArgs, 2, &bundleItem.cell))
        return Py_BadArgument();

    CPythonShop::Instance().BuildPrivateShop(sign.c_str(), bundleItem);
    Py_RETURN_NONE;
}

PyObject *shopGetTabCount(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonShop::instance().GetTabCount());
}

PyObject *shopGetTabName(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bTabIdx;
    if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
        return Py_BuildException();

    return Py_BuildValue("s", CPythonShop::instance().GetTabName(bTabIdx));
}

PyObject *shopGetTabCoinType(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bTabIdx;
    if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonShop::instance().GetTabCoinType(bTabIdx));
}

PyObject *shopGetTabCoinVnum(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bTabIdx;
    if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonShop::instance().GetTabCoinVnum(bTabIdx));
}

extern "C" void initshop()
{
    static PyMethodDef s_methods[] =
    {
        // Shop
        {"Open", shopOpen, METH_VARARGS},
        {"Close", shopClose, METH_VARARGS},
        {"IsOpen", shopIsOpen, METH_VARARGS},
        {"IsPrivateShop", shopIsPrviateShop, METH_VARARGS},
        {"IsMainPlayerPrivateShop", shopIsMainPlayerPrivateShop, METH_VARARGS},
        {"GetItemData", shopGetItemData, METH_VARARGS},
        {"GetItemID", shopGetItemID, METH_VARARGS},
        {"GetItemCount", shopGetItemCount, METH_VARARGS},
        {"GetItemPrice", shopGetItemPrice, METH_VARARGS},
        {"GetItemMetinSocket", shopGetItemMetinSocket, METH_VARARGS},
        {"GetItemAttribute", shopGetItemAttribute, METH_VARARGS},
        {"GetTabCount", shopGetTabCount, METH_VARARGS},
        {"GetTabName", shopGetTabName, METH_VARARGS},
        {"GetTabCoinType", shopGetTabCoinType, METH_VARARGS},
        {"GetTabCoinVnum", shopGetTabCoinVnum, METH_VARARGS},

        {"GetItemChangeLookVnum", shopGetItemChangeLookVnum, METH_VARARGS},

        // Private Shop
        {"ClearPrivateShopStock", shopClearPrivateShopStock, METH_VARARGS},
        {"AddPrivateShopItemStock", shopAddPrivateShopItemStock, METH_VARARGS},
        {"DelPrivateShopItemStock", shopDelPrivateShopItemStock, METH_VARARGS},
        {"GetPrivateShopItemPrice", shopGetPrivateShopItemPrice, METH_VARARGS},
        {"BuildPrivateShop", shopBuildPrivateShop, METH_VARARGS},

        {NULL, NULL, NULL},
    };
    PyObject *poModule = Py_InitModule("shop", s_methods);

    PyModule_AddIntConstant(poModule, "SHOP_SLOT_COUNT", SHOP_HOST_ITEM_MAX_NUM);
    PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_GOLD", SHOP_COIN_TYPE_GOLD);
    PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_SECONDARY_COIN", SHOP_COIN_TYPE_SECONDARY_COIN);
    PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_CASH", SHOP_COIN_TYPE_CASH);
}

void init_shop(py::module &m)
{
    py::module shop = m.def_submodule("shop", "");

    py::class_<packet_shop_item>(shop, "packet_shop_item")
        .def(py::init<>())
        .def_readwrite("vnum", &packet_shop_item::vnum)
        .def_readwrite("transVnum", &packet_shop_item::transVnum)
        .def_readwrite("sealDate", &packet_shop_item::nSealDate)
        .def_readwrite("count", &packet_shop_item::count)
        .def_readwrite("price", &packet_shop_item::price)
        .def_property_readonly(
            "sockets", [](const packet_shop_item &c) -> const std::array<SocketValue, ITEM_SOCKET_MAX_NUM>&
            {
                return reinterpret_cast<const std::array<SocketValue, ITEM_SOCKET_MAX_NUM> &>(c.sockets);
            }
            )
        .def_property_readonly(
            "attrs", [](const packet_shop_item &c) -> const std::array<TPlayerItemAttribute, ITEM_ATTRIBUTE_MAX_NUM>&
            {
                return reinterpret_cast<const std::array<TPlayerItemAttribute, ITEM_ATTRIBUTE_MAX_NUM> &>(c.attrs);
            }
            )
        .def_readwrite("display_pos", &packet_shop_item::display_pos);


    py::class_<CPythonShop::ShopTab>(shop, "ShopTab")
        .def(py::init<>())
        .def_readwrite("coinType", &CPythonShop::ShopTab::coinType)
        .def_readwrite("coinVnum", &CPythonShop::ShopTab::coinVnum)
        .def_readwrite("name", &CPythonShop::ShopTab::name)
        .def_property_readonly("items", [](const CPythonShop::ShopTab &c) -> const std::array<packet_shop_item, SHOP_HOST_ITEM_MAX_NUM>&
            {
                return reinterpret_cast<const std::array<packet_shop_item, SHOP_HOST_ITEM_MAX_NUM> &>(c.items);
            });


        auto cps = py::class_<CPythonShop, std::unique_ptr<CPythonShop, py::nodelete>>(shop, "shopInst")
        .def(py::init([]()
        {
            return std::unique_ptr<CPythonShop, py::nodelete>(CPythonShop::InstancePtr());
        }), pybind11::return_value_policy::reference_internal);

        cps.def("GetItemData", py::overload_cast<uint32_t>(&CPythonShop::GetItemData, py::const_), pybind11::return_value_policy::reference);
        cps.def("GetItemData", py::overload_cast<uint8_t, uint32_t>(&CPythonShop::GetItemData, py::const_), pybind11::return_value_policy::reference);
        cps.def("IsMainPlayerPrivateShop", &CPythonShop::IsMainPlayerPrivateShop);
        cps.def("IsPrivateShop", &CPythonShop::IsPrivateShop);
        cps.def("IsOpen", &CPythonShop::IsOpen);
        cps.def("Open", &CPythonShop::Open);
        cps.def("Close", &CPythonShop::Close);
        cps.def("BuildPrivateShop", &CPythonShop::BuildPrivateShop);
        cps.def("GetShopTab", &CPythonShop::GetShopTab);
        cps.def("ClearPrivateShopStock", &CPythonShop::ClearPrivateShopStock);
        cps.def("AddPrivateShopItemStock", &CPythonShop::AddPrivateShopItemStock);
        cps.def("DelPrivateShopItemStock", &CPythonShop::DelPrivateShopItemStock);
        cps.def("GetPrivateShopItemPrice", &CPythonShop::GetPrivateShopItemPrice);
}
