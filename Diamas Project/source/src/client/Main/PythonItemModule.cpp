#include "PythonItem.h"
#include "StdAfx.h"

#include "../gamelib/ItemManager.h"
#include "InstanceBase.h"
#include "PythonApplication.h"
#include "PythonPlayer.h"
#include <pybind11/stl.h>
using namespace pybind11::literals;

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

PyObject* itemSetUseSoundFileName(PyObject* poSelf, PyObject* poArgs)
{
    int iUseSound;
    if (!PyTuple_GetInteger(poArgs, 0, &iUseSound))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.SetUseSoundFileName(iUseSound, szFileName);
    Py_RETURN_NONE;
}

PyObject* itemSetDropSoundFileName(PyObject* poSelf, PyObject* poArgs)
{
    int iDropSound;
    if (!PyTuple_GetInteger(poArgs, 0, &iDropSound))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.SetDropSoundFileName(iDropSound, szFileName);
    Py_RETURN_NONE;
}

PyObject* itemSelectItem(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    if (iIndex == 0)
        return Py_BuildValue("O", Py_False);

    if (!CItemManager::Instance().SelectItemData(iIndex)) {
        SPDLOG_WARN("Cannot find item by {0}", iIndex);
        CItemManager::Instance().SelectItemData(60001);
        return Py_BuildValue("O", Py_False);
    }

    return Py_BuildValue("O", Py_True);
}

#ifdef ENABLE_DS_SET
PyObject* itemGetDSSetWeight(PyObject* poSelf, PyObject* poArgs)
{
    uint8_t iSetGrade;
    if (!PyTuple_GetInteger(poArgs, 0, &iSetGrade)) {
        return Py_BadArgument();
    }
    uint8_t dsType;
    if (!PyTuple_GetInteger(poArgs, 1, &dsType)) {
        return Py_BadArgument();
    }

    DragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
    if (!dsTable) {
        return Py_BuildException("DragonSoulTable not initalized");
    }

    return Py_BuildValue("f", dsTable->GetDSSetWeight(dsType, iSetGrade));
}

PyObject* itemGetDSSetValue(PyObject* poSelf, PyObject* poArgs)
{

    uint8_t dsType;
    if (!PyTuple_GetInteger(poArgs, 0, &dsType)) {
        return Py_BadArgument();
    }

    uint8_t iAttributeIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttributeIndex)) {
        return Py_BadArgument();
    }

    uint8_t iApplyType;
    if (!PyTuple_GetInteger(poArgs, 2, &iApplyType)) {
        return Py_BadArgument();
    }

    uint8_t iSetGrade;
    if (!PyTuple_GetInteger(poArgs, 3, &iSetGrade)) {
        return Py_BadArgument();
    }

    DragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
    if (!dsTable) {
        return Py_BuildException("DragonSoulTable not initalized");
    }

    return Py_BuildValue(
        "f",
        dsTable->GetDSSetValue(iAttributeIndex, iApplyType, dsType, iSetGrade));
}

PyObject* itemGetDSBasicApplyCount(PyObject* poSelf, PyObject* poArgs)
{
    uint8_t iDSType;
    if (!PyTuple_GetInteger(poArgs, 0, &iDSType)) {
        return Py_BadArgument();
    }

    DragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
    if (!dsTable) {
        return Py_BuildException("DragonSoulTable not initalized");
    }

    return Py_BuildValue("i", dsTable->GetDSBasicApplyCount(iDSType));
}

PyObject* itemGetDSBasicApplyValue(PyObject* poSelf, PyObject* poArgs)
{
    uint8_t iDSType;
    if (!PyTuple_GetInteger(poArgs, 0, &iDSType)) {
        return Py_BadArgument();
    }

    uint16_t iApplyType;
    if (!PyTuple_GetInteger(poArgs, 1, &iApplyType)) {
        return Py_BadArgument();
    }

    DragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
    if (!dsTable) {
        return Py_BuildException("DragonSoulTable not initalized");
    }

    return Py_BuildValue("i",
                         dsTable->GetDSBasicApplyValue(iDSType, iApplyType));
}

PyObject* itemGetDSAdditionalApplyValue(PyObject* poSelf, PyObject* poArgs)
{
    uint8_t iDSType;
    if (!PyTuple_GetInteger(poArgs, 0, &iDSType)) {
        return Py_BadArgument();
    }

    uint16_t iApplyType;
    if (!PyTuple_GetInteger(poArgs, 1, &iApplyType)) {
        return Py_BadArgument();
    }

    DragonSoulTable* dsTable = CItemManager::Instance().GetDragonSoulTable();
    if (!dsTable) {
        return Py_BuildException("DragonSoulTable not initalized");
    }

    return Py_BuildValue(
        "i", dsTable->GetDSAdditionalApplyValue(iDSType, iApplyType));
}
#endif

PyObject* itemGetItemNameByVnum(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();
    CItemData* pItemData = CItemManager::Instance().GetProto(iIndex);
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("s", pItemData->GetName());
}

PyObject* itemGetItemName(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("s", pItemData->GetName());
}

PyObject* itemGetCurrentItemVnum(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetIndex());
}

PyObject* itemGetRefineSet(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetRefineSet());
}

PyObject* itemGetRefinedVnum(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetRefinedVnum());
}

PyObject* itemGetAutoCompleteEntries(PyObject* poSelf, PyObject* poArgs)
{
    std::string name;
    if (!PyTuple_GetString(poArgs, 0, name))
        return Py_BadArgument();

    auto const vec = CItemManager::instance().GetProtoEntriesByName(name, 10);
    if (!vec) {
        py::list list;
        return Py_BuildValue("O", list.ptr());
    }

    py::list list;

    for (const auto& item : vec.value()) {
        std::string itemName = item->GetName();
        py::tuple tup =
            py::make_tuple(item->GetIndex(), py::bytes(itemName), 0);
        list.append(tup);
    }

    return Py_BuildValue("O", list.ptr());
}

PyObject* itemGetItemDescription(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("s", pItemData->GetDescription());
}

PyObject* itemGetItemSummary(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("s", pItemData->GetSummary());
}

PyObject* itemDataGetIconImageFileName(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    CGraphicImage::Ptr pImage = pItemData->GetIconImage();
    if (!pImage)
        return Py_BuildValue("s", "Noname");

    return Py_BuildValue("s", pImage->GetFileName());
}

PyObject* itemGetIconImageFileName(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    CGraphicImage::Ptr pImage = pItemData->GetIconImage();
    if (!pImage)
        return Py_BuildValue("s", "Noname");

    return Py_BuildValue("s", pImage->GetFileName());
}

PyObject* itemGetItemSize(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("(ii)", 1, pItemData->GetSize());
}

PyObject* itemGetItemType(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetType());
}

PyObject* itemGetItemSubType(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetSubType());
}

PyObject* itemGetAttributeSetIndex(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetAttributeSetIndex());
}

PyObject* itemGetIBuyItemPrice(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();

    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetIBuyItemPrice());
}

PyObject* itemGetISellItemPrice(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();

    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->GetISellItemPrice());
}

PyObject* itemIsAntiFlag(PyObject* poSelf, PyObject* poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->IsAntiFlag(iFlag));
}

PyObject* itemIsFlag(PyObject* poSelf, PyObject* poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->IsFlag(iFlag));
}

PyObject* itemIsWearableFlag(PyObject* poSelf, PyObject* poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->IsWearableFlag(iFlag));
}

PyObject* itemIs1GoldItem(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("no selected item data");

    return Py_BuildValue("i", pItemData->IsFlag(ITEM_FLAG_COUNT_PER_1GOLD));
}

PyObject* itemGetLimit(PyObject* poSelf, PyObject* poArgs)
{
    int iValueIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Not yet select item data");

    auto limit = pItemData->GetLimit(iValueIndex);
    ;
    if (!limit)
        return Py_BuildException("ItemLimit index not found");

    return Py_BuildValue("ii", limit->bType, limit->value);
}

PyObject* itemGetAffect(PyObject* poSelf, PyObject* poArgs)
{
    int iValueIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Not yet select item data");

    auto apply = pItemData->GetApply(iValueIndex);
    if (!apply)
        return Py_BuildException();

    TItemApply ItemApply = apply.value();

    return Py_BuildValue("id", ItemApply.bType, ItemApply.lValue);
}

PyObject* itemGetValue(PyObject* poSelf, PyObject* poArgs)
{
    int iValueIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
        return Py_BadArgument();

    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Not yet select item data");

    return Py_BuildValue("i", pItemData->GetValue(iValueIndex));
}

PyObject* itemGetIconInstance(PyObject* poSelf, PyObject* poArgs)
{
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Not yet select item data");

    CGraphicImage::Ptr pImage = pItemData->GetIconImage();
    if (!pImage)
        return Py_BuildException("Cannot get icon image by %d",
                                 pItemData->GetIndex());

    auto pImageInstance = new CGraphicImageInstance;
    pImageInstance->SetImagePointer(pImage);

    const auto capsule =
        PyCapsule_New(pImageInstance, nullptr, ImageCapsuleDestroyer);

    return capsule;
}

PyObject* itemIsEquipmentVID(PyObject* poSelf, PyObject* poArgs)
{
    int iItemVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemVID);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Not yet select item data");

    return Py_BuildValue("i", pItemData->IsEquipment());
}

// 2005.05.20.myevan.통합 USE_TYPE 체크
PyObject* itemGetUseType(PyObject* poSelf, PyObject* poArgs)
{
    int iItemVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemVID);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    return Py_BuildValue("s", pItemData->GetUseTypeString());
}

PyObject* itemIsRefineScroll(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    if (pItemData->GetType() != ITEM_USE)
        return Py_BuildValue("i", FALSE);

    switch (pItemData->GetSubType()) {
        case USE_TUNING:
            return Py_BuildValue("i", TRUE);
            break;
    }

    return Py_BuildValue("i", FALSE);
}

PyObject* itemIsDetachScroll(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    int iType = pItemData->GetType();
    int iSubType = pItemData->GetSubType();
    if (iType == ITEM_USE)
        if (iSubType == USE_DETACHMENT) {
            return Py_BuildValue("i", TRUE);
        }

    return Py_BuildValue("i", FALSE);
}

PyObject* itemCanAddToQuickSlotItem(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex)) {
        return Py_BadArgument();
    }

    const auto proto = CItemManager::Instance().GetProto(iItemIndex);
    if (!proto)
        return Py_BuildException("Can't find select item data");

    if (proto->GetIndex() == UNIQUE_ITEM_CAPE_OF_COURAGE ||
        proto->GetIndex() == UNIQUE_ITEM_CAPE_OF_COURAGE_PREMIUM ||
        proto->GetIndex() == UNIQUE_ITEM_CAPE_OF_COURAGE_PREMIUM2)
        Py_RETURN_TRUE;

    switch (proto->GetType()) {
        case ITEM_USE:
        case ITEM_QUEST:
        case ITEM_TOGGLE:
            Py_RETURN_TRUE;
        default:
            Py_RETURN_FALSE;
    }
}

PyObject* itemIsKey(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    if (ITEM_TREASURE_KEY == pItemData->GetType()) {
        return Py_BuildValue("i", TRUE);
    }

    return Py_BuildValue("i", FALSE);
}

PyObject* itemIsMetin(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    if (ITEM_METIN == pItemData->GetType()) {
        return Py_BuildValue("i", TRUE);
    }

    return Py_BuildValue("i", FALSE);
}

PyObject* itemRender(PyObject* poSelf, PyObject* poArgs)
{
    CPythonItem::Instance().Render();
    Py_RETURN_NONE;
}

PyObject* itemUpdate(PyObject* poSelf, PyObject* poArgs)
{
    CPythonApplication& rkApp = CPythonApplication::AppInst();

    POINT ptMouse;
    rkApp.GetMousePosition(&ptMouse);

    CPythonItem::Instance().Update(ptMouse);
    Py_RETURN_NONE;
}

PyObject* itemCreateItem(PyObject* poSelf, PyObject* poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();
    int iVirtualNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iVirtualNumber))
        return Py_BadArgument();

    float x;
    if (!PyTuple_GetFloat(poArgs, 2, &x))
        return Py_BadArgument();
    float y;
    if (!PyTuple_GetFloat(poArgs, 3, &y))
        return Py_BadArgument();
    float z;
    if (!PyTuple_GetFloat(poArgs, 4, &z))
        return Py_BadArgument();

    bool bDrop = true;
    PyTuple_GetBoolean(poArgs, 5, &bDrop);

    CPythonItem::Instance().CreateItem(iVirtualID, iVirtualNumber, x, y, z,
                                       bDrop);

    Py_RETURN_NONE;
}

PyObject* itemDeleteItem(PyObject* poSelf, PyObject* poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CPythonItem::Instance().DeleteItem(iVirtualID);
    Py_RETURN_NONE;
}

PyObject* itemPick(PyObject* poSelf, PyObject* poArgs)
{
    uint32_t dwItemID;
    if (CPythonItem::Instance().GetPickedItemID(&dwItemID))
        return Py_BuildValue("i", dwItemID);
    else
        return Py_BuildValue("i", -1);
}

PyObject* itemIsSealScroll(PyObject* poSelf, PyObject* poArgs)
{
    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData* pItemData =
        CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
        return Py_BuildException("Can't find select item data");

    if (ITEM_USE == pItemData->GetType() &&
        USE_SPECIAL == pItemData->GetSubType() &&
        (50263 == pItemData->GetIndex() || 50264 == pItemData->GetIndex()))
        return Py_BuildValue("i", TRUE);
    return Py_BuildValue("i", FALSE);
}

PyObject* itemGetDefaultSealDate(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", 0);
}

PyObject* itemGetUnlimitedSealDate(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", -1);
}

PyObject* itemGetApplyPoint(PyObject* poSelf, PyObject* poArgs)
{
    int applyType;
    if (!PyTuple_GetInteger(poArgs, 0, &applyType))
        return Py_BadArgument();

    return Py_BuildValue("i", GetApplyPoint(applyType));
}

ItemPosition MakeItemPosition(uint8_t window, uint16_t cell)
{
    return ItemPosition(window, cell);
}

ItemPosition MakeItemPosition(std::tuple<uint8_t, uint16_t> tuple)
{
    return std::make_from_tuple<ItemPosition>(tuple);
}

bool itemIsItemUsedForDragonSoul(ItemVnum iItemIndex, uint8_t byWndType)
{
    CItemData* pItemData = CItemManager::Instance().GetProto(iItemIndex);
    if (!pItemData)
        return false;

    if ((pItemData->GetType() == ITEM_USE) &&
        (pItemData->GetSubType() == USE_TIME_CHARGE_PER ||
         pItemData->GetSubType() == USE_TIME_CHARGE_FIX ||
         (pItemData->GetSubType() == 33 && byWndType == DRAGON_SOUL_INVENTORY)))
        return true;

    if (pItemData->GetType() == ITEM_EXTRACT) {
        if (byWndType == INVENTORY) {
            if (pItemData->GetSubType() == EXTRACT_DRAGON_SOUL)
                return true;
        }

        if (byWndType == DRAGON_SOUL_INVENTORY) {
            if (pItemData->GetSubType() == EXTRACT_DRAGON_HEART)
                return true;

            return true;
        }
    }

    return false;
}

extern "C" void inititem()
{
    static PyMethodDef s_methods[] = {
        {"SetUseSoundFileName", itemSetUseSoundFileName, METH_VARARGS},
        {"SetDropSoundFileName", itemSetDropSoundFileName, METH_VARARGS},
        {"SelectItem", itemSelectItem, METH_VARARGS},

        {"GetItemName", itemGetItemName, METH_VARARGS},
        {"GetItemNameByVnum", itemGetItemNameByVnum, METH_VARARGS},
        {"GetItemDescription", itemGetItemDescription, METH_VARARGS},
        {"GetItemSummary", itemGetItemSummary, METH_VARARGS},
        {"GetIconImageFileName", itemGetIconImageFileName, METH_VARARGS},
        {"GetItemSize", itemGetItemSize, METH_VARARGS},
        {"GetItemType", itemGetItemType, METH_VARARGS},
        {"GetItemSubType", itemGetItemSubType, METH_VARARGS},
        {"GetIBuyItemPrice", itemGetIBuyItemPrice, METH_VARARGS},
        {"GetISellItemPrice", itemGetISellItemPrice, METH_VARARGS},
        {"IsAntiFlag", itemIsAntiFlag, METH_VARARGS},
        {"IsFlag", itemIsFlag, METH_VARARGS},
        {"IsWearableFlag", itemIsWearableFlag, METH_VARARGS},
        {"Is1GoldItem", itemIs1GoldItem, METH_VARARGS},
        {"GetLimit", itemGetLimit, METH_VARARGS},
        {"GetAffect", itemGetAffect, METH_VARARGS},
        {"GetValue", itemGetValue, METH_VARARGS},
        {"GetIconInstance", itemGetIconInstance, METH_VARARGS},
        {"GetUseType", itemGetUseType, METH_VARARGS},
        {"IsEquipmentVID", itemIsEquipmentVID, METH_VARARGS},
        {"IsRefineScroll", itemIsRefineScroll, METH_VARARGS},
        {"IsDetachScroll", itemIsDetachScroll, METH_VARARGS},
        {"IsKey", itemIsKey, METH_VARARGS},
        {"IsMetin", itemIsMetin, METH_VARARGS},
        {"CanAddToQuickSlotItem", itemCanAddToQuickSlotItem, METH_VARARGS},

        {"Update", itemUpdate, METH_VARARGS},
        {"Render", itemRender, METH_VARARGS},
        {"CreateItem", itemCreateItem, METH_VARARGS},
        {"DeleteItem", itemDeleteItem, METH_VARARGS},
        {"Pick", itemPick, METH_VARARGS},

        {"IsSealScroll", itemIsSealScroll, METH_VARARGS},
        {"GetDefaultSealDate", itemGetDefaultSealDate, METH_VARARGS},
        {"GetUnlimitedSealDate", itemGetUnlimitedSealDate, METH_VARARGS},

        {"GetCurrenItemVnum", itemGetCurrentItemVnum, METH_VARARGS},
        {"GetRefinedVnum", itemGetRefinedVnum, METH_VARARGS},
        {"GetRefineSet", itemGetRefineSet, METH_VARARGS},

        {"GetApplyPoint", itemGetApplyPoint, METH_VARARGS},

        {"GetAutoCompleEntries", itemGetAutoCompleteEntries, METH_VARARGS},
        {"GetAttributeSetIndex", itemGetAttributeSetIndex, METH_VARARGS},

#ifdef ENABLE_DS_SET
        {"GetDSSetWeight", itemGetDSSetWeight, METH_VARARGS},
        {"GetDSBasicApplyCount", itemGetDSBasicApplyCount, METH_VARARGS},
        {"GetDSBasicApplyValue", itemGetDSBasicApplyValue, METH_VARARGS},
        {"GetDSAdditionalApplyValue", itemGetDSAdditionalApplyValue,
         METH_VARARGS},

        {"GetDSSetValue", itemGetDSSetValue, METH_VARARGS},

#endif
        {NULL, NULL, NULL},
    };

    PyObject* poModule = Py_InitModule("item", s_methods);

    PyModule_AddIntConstant(poModule, "USESOUND_ACCESSORY",
                            CPythonItem::USESOUND_ACCESSORY);
    PyModule_AddIntConstant(poModule, "USESOUND_ARMOR",
                            CPythonItem::USESOUND_ARMOR);
    PyModule_AddIntConstant(poModule, "USESOUND_BOW",
                            CPythonItem::USESOUND_BOW);
    PyModule_AddIntConstant(poModule, "USESOUND_DEFAULT",
                            CPythonItem::USESOUND_DEFAULT);
    PyModule_AddIntConstant(poModule, "USESOUND_WEAPON",
                            CPythonItem::USESOUND_WEAPON);
    PyModule_AddIntConstant(poModule, "USESOUND_POTION",
                            CPythonItem::USESOUND_POTION);
    PyModule_AddIntConstant(poModule, "USESOUND_PORTAL",
                            CPythonItem::USESOUND_PORTAL);

    PyModule_AddIntConstant(poModule, "DROPSOUND_ACCESSORY",
                            CPythonItem::DROPSOUND_ACCESSORY);
    PyModule_AddIntConstant(poModule, "DROPSOUND_ARMOR",
                            CPythonItem::DROPSOUND_ARMOR);
    PyModule_AddIntConstant(poModule, "DROPSOUND_BOW",
                            CPythonItem::DROPSOUND_BOW);
    PyModule_AddIntConstant(poModule, "DROPSOUND_DEFAULT",
                            CPythonItem::DROPSOUND_DEFAULT);
    PyModule_AddIntConstant(poModule, "DROPSOUND_WEAPON",
                            CPythonItem::DROPSOUND_WEAPON);

    PyModule_AddIntConstant(poModule, "EQUIPMENT_COUNT", WEAR_SHIELD);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_HEAD", WEAR_HEAD);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_BODY", WEAR_BODY);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_WEAPON", WEAR_WEAPON);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_WRIST", WEAR_WRIST);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_SHOES", WEAR_FOOTS);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_NECK", WEAR_NECK);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_EAR", WEAR_EAR);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE1", WEAR_UNIQUE1);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE2", WEAR_UNIQUE2);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_ARROW", WEAR_ARROW);

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING1", WEAR_RING1);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING2", WEAR_RING2);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING3", WEAR_RING3);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING4", WEAR_RING4);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING5", WEAR_RING5);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING6", WEAR_RING6);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_RING7", WEAR_RING7);

    PyModule_AddIntConstant(poModule, "EQUIPMENT_BELT", WEAR_BELT);
#endif

    PyModule_AddIntConstant(poModule, "ITEM_TYPE_NONE", ITEM_NONE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_WEAPON", ITEM_WEAPON);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_ARMOR", ITEM_ARMOR);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_USE", ITEM_USE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_AUTOUSE", ITEM_AUTOUSE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_MATERIAL", ITEM_MATERIAL);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL", ITEM_SPECIAL);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOOL", ITEM_TOOL);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_LOTTERY", ITEM_LOTTERY);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_ELK", ITEM_ELK);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_METIN", ITEM_METIN);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_CONTAINER", ITEM_CONTAINER);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_FISH", ITEM_FISH);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_ROD", ITEM_ROD);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_RESOURCE", ITEM_RESOURCE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_CAMPFIRE", ITEM_CAMPFIRE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_UNIQUE", ITEM_UNIQUE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLBOOK", ITEM_SKILLBOOK);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_QUEST", ITEM_QUEST);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_POLYMORPH", ITEM_POLYMORPH);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_BOX",
                            ITEM_TREASURE_BOX);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_KEY",
                            ITEM_TREASURE_KEY);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_PICK", ITEM_PICK);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_BLEND", ITEM_BLEND);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_DS", ITEM_DS);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL_DS", ITEM_SPECIAL_DS);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_RING", ITEM_RING);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_BELT", ITEM_BELT);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_GIFTBOX", ITEM_GIFTBOX);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_PET", ITEM_PET);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_GACHA", ITEM_GACHA);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLFORGET",
                            ITEM_SKILLFORGET);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOGGLE", ITEM_TOGGLE);
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_TALISMAN", ITEM_TALISMAN);

#ifdef ENABLE_COSTUME_SYSTEM
    PyModule_AddIntConstant(poModule, "ITEM_TYPE_COSTUME", ITEM_COSTUME);

    PyModule_AddIntConstant(poModule, "USE_POTION", USE_POTION);
    PyModule_AddIntConstant(poModule, "USE_TALISMAN", USE_TALISMAN);
    PyModule_AddIntConstant(poModule, "USE_TUNING", USE_TUNING);
    PyModule_AddIntConstant(poModule, "USE_MOVE", USE_MOVE);
    PyModule_AddIntConstant(poModule, "USE_TREASURE_BOX", USE_TREASURE_BOX);
    PyModule_AddIntConstant(poModule, "USE_MONEYBAG", USE_MONEYBAG);
    PyModule_AddIntConstant(poModule, "USE_BAIT", USE_BAIT);
    PyModule_AddIntConstant(poModule, "USE_ABILITY_UP", USE_ABILITY_UP);
    PyModule_AddIntConstant(poModule, "USE_AFFECT", USE_AFFECT);
    PyModule_AddIntConstant(poModule, "USE_CREATE_STONE", USE_CREATE_STONE);
    PyModule_AddIntConstant(poModule, "USE_SPECIAL", USE_SPECIAL);
    PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY", USE_POTION_NODELAY);
    PyModule_AddIntConstant(poModule, "USE_CLEAR", USE_CLEAR);
    PyModule_AddIntConstant(poModule, "USE_INVISIBILITY", USE_INVISIBILITY);
    PyModule_AddIntConstant(poModule, "USE_DETACHMENT", USE_DETACHMENT);
    PyModule_AddIntConstant(poModule, "USE_BUCKET", USE_BUCKET);
    PyModule_AddIntConstant(poModule, "USE_POTION_CONTINUE",
                            USE_POTION_CONTINUE);
    PyModule_AddIntConstant(poModule, "USE_CLEAN_SOCKET", USE_CLEAN_SOCKET);
    PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE",
                            USE_CHANGE_ATTRIBUTE);
    PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE", USE_ADD_ATTRIBUTE);
    PyModule_AddIntConstant(poModule, "USE_ADD_ACCESSORY_SOCKET",
                            USE_ADD_ACCESSORY_SOCKET);
    PyModule_AddIntConstant(poModule, "USE_PUT_INTO_ACCESSORY_SOCKET",
                            USE_PUT_INTO_ACCESSORY_SOCKET);
    PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE2", USE_ADD_ATTRIBUTE2);
    PyModule_AddIntConstant(poModule, "USE_RECIPE", USE_RECIPE);
    PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE2",
                            USE_CHANGE_ATTRIBUTE2);
    PyModule_AddIntConstant(poModule, "USE_BIND", USE_BIND);
    PyModule_AddIntConstant(poModule, "USE_UNBIND", USE_UNBIND);
    PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_PER",
                            USE_TIME_CHARGE_PER);
    PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_FIX",
                            USE_TIME_CHARGE_FIX);
    PyModule_AddIntConstant(poModule, "USE_PUT_INTO_BELT_SOCKET",
                            USE_PUT_INTO_BELT_SOCKET);
    PyModule_AddIntConstant(poModule, "USE_PUT_INTO_RING_SOCKET",
                            USE_PUT_INTO_RING_SOCKET);
    PyModule_AddIntConstant(poModule, "USE_CHEST", USE_CHEST);
    PyModule_AddIntConstant(poModule, "USE_CHANGE_COSTUME_ATTR",
                            USE_CHANGE_COSTUME_ATTR);
    PyModule_AddIntConstant(poModule, "USE_RESET_COSTUME_ATTR",
                            USE_RESET_COSTUME_ATTR);
    PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE_RARE",
                            USE_ADD_ATTRIBUTE_RARE);
    PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE_RARE",
                            USE_CHANGE_ATTRIBUTE_RARE);
    PyModule_AddIntConstant(poModule, "USE_ENHANCE_TIME", USE_ENHANCE_TIME);
    PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE_PERM",
                            USE_CHANGE_ATTRIBUTE_PERM);
    PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE_PERM",
                            USE_ADD_ATTRIBUTE_PERM);
    PyModule_AddIntConstant(poModule, "USE_MAKE_ACCESSORY_SOCKET_PERM",
                            USE_MAKE_ACCESSORY_SOCKET_PERM);
    PyModule_AddIntMacro(poModule, USE_LEVEL_PET_FOOD);
    PyModule_AddIntMacro(poModule, USE_LEVEL_PET_CHANGE_ATTR);
    PyModule_AddIntMacro(poModule, USE_BATTLEPASS);
    PyModule_AddIntMacro(poModule, USE_ADD_SOCKETS);
    PyModule_AddIntMacro(poModule, USE_MAX_NUM);

    // Item Sub Type
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY", COSTUME_BODY);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_HAIR", COSTUME_HAIR);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_MOUNT", COSTUME_MOUNT);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE", COSTUME_ACCE);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON", COSTUME_WEAPON);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY_EFFECT",
                            COSTUME_BODY_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON_EFFECT",
                            COSTUME_WEAPON_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WING_EFFECT",
                            COSTUME_WING_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_RANK", COSTUME_RANK);

    // 인벤토리 및 장비창에서의 슬롯 번호
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_START", WEAR_COSTUME_BODY);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_COUNT", 9);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY", WEAR_COSTUME_BODY);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_HAIR", WEAR_COSTUME_HAIR);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_END",
                            DRAGON_SOUL_EQUIP_SLOT_START);

    PyModule_AddIntMacro(poModule, WEAR_BODY);
    PyModule_AddIntMacro(poModule, WEAR_HEAD);
    PyModule_AddIntMacro(poModule, WEAR_FOOTS);
    PyModule_AddIntMacro(poModule, WEAR_WRIST);
    PyModule_AddIntMacro(poModule, WEAR_WEAPON);
    PyModule_AddIntMacro(poModule, WEAR_NECK);
    PyModule_AddIntMacro(poModule, WEAR_EAR);
    PyModule_AddIntMacro(poModule, WEAR_UNIQUE1);
    PyModule_AddIntMacro(poModule, WEAR_UNIQUE2);
    PyModule_AddIntMacro(poModule, WEAR_ARROW);
    PyModule_AddIntMacro(poModule, WEAR_SHIELD);

    PyModule_AddIntMacro(poModule, WEAR_COSTUME_BODY);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_HAIR);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_MOUNT);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_ACCE);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_WEAPON);
    PyModule_AddIntMacro(poModule, WEAR_RING1);
    PyModule_AddIntMacro(poModule, WEAR_RING2);
    PyModule_AddIntMacro(poModule, WEAR_RING3);
    PyModule_AddIntMacro(poModule, WEAR_RING4);
    PyModule_AddIntMacro(poModule, WEAR_RING5);
    PyModule_AddIntMacro(poModule, WEAR_RING6);
    PyModule_AddIntMacro(poModule, WEAR_RING7);

    PyModule_AddIntMacro(poModule, WEAR_BELT);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_BODY_EFFECT);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_WEAPON_EFFECT);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_WING_EFFECT);
    PyModule_AddIntMacro(poModule, WEAR_COSTUME_RANK);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_1);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_2);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_3);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_4);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_5);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_6);
    PyModule_AddIntMacro(poModule, WEAR_TALISMAN_7);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE", COSTUME_ACCE);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_ACCE", WEAR_COSTUME_ACCE);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON", COSTUME_WEAPON);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WEAPON",
                            WEAR_COSTUME_WEAPON);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_MOUNT", COSTUME_MOUNT);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_MOUNT", WEAR_COSTUME_MOUNT);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_HAIR", COSTUME_HAIR);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_HAIR", WEAR_COSTUME_HAIR);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY", COSTUME_BODY);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY", WEAR_COSTUME_BODY);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY_EFFECT",
                            COSTUME_BODY_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY_EFFECT",
                            WEAR_COSTUME_BODY_EFFECT);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WING_EFFECT",
                            COSTUME_WING_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WING_EFFECT",
                            WEAR_COSTUME_WING_EFFECT);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON_EFFECT",
                            COSTUME_WEAPON_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WEAPON_EFFECT",
                            WEAR_COSTUME_WEAPON_EFFECT);

    PyModule_AddIntConstant(poModule, "COSTUME_TYPE_RANK",
                            COSTUME_WEAPON_EFFECT);
    PyModule_AddIntConstant(poModule, "COSTUME_SLOT_RANK", WEAR_COSTUME_RANK);
#endif

    PyModule_AddIntMacro(poModule, BELT_INVENTORY_SLOT_COUNT);

    PyModule_AddIntConstant(poModule, "WEAPON_SWORD", WEAPON_SWORD);
    PyModule_AddIntConstant(poModule, "WEAPON_DAGGER", WEAPON_DAGGER);
    PyModule_AddIntConstant(poModule, "WEAPON_BOW", WEAPON_BOW);
    PyModule_AddIntConstant(poModule, "WEAPON_TWO_HANDED", WEAPON_TWO_HANDED);
    PyModule_AddIntConstant(poModule, "WEAPON_BELL", WEAPON_BELL);
    PyModule_AddIntConstant(poModule, "WEAPON_FAN", WEAPON_FAN);
    PyModule_AddIntConstant(poModule, "WEAPON_ARROW", WEAPON_ARROW);
    PyModule_AddIntConstant(poModule, "WEAPON_QUIVER", WEAPON_QUIVER);

    PyModule_AddIntConstant(poModule, "WEAPON_NUM_TYPES", WEAPON_NUM_TYPES);

    PyModule_AddIntConstant(poModule, "TOGGLE_AUTO_RECOVERY_HP",
                            TOGGLE_AUTO_RECOVERY_HP);
    PyModule_AddIntConstant(poModule, "TOGGLE_AUTO_RECOVERY_SP",
                            TOGGLE_AUTO_RECOVERY_SP);
    PyModule_AddIntConstant(poModule, "TOGGLE_PET", TOGGLE_PET);
    PyModule_AddIntConstant(poModule, "TOGGLE_MOUNT", TOGGLE_MOUNT);
    PyModule_AddIntConstant(poModule, "TOGGLE_ANTI_EXP", TOGGLE_ANTI_EXP);
    PyModule_AddIntConstant(poModule, "TOGGLE_AFFECT", TOGGLE_AFFECT);
    PyModule_AddIntConstant(poModule, "TOGGLE_LEVEL_PET", TOGGLE_LEVEL_PET);

    PyModule_AddIntConstant(poModule, "USE_POTION", USE_POTION);
    PyModule_AddIntConstant(poModule, "USE_TALISMAN", USE_TALISMAN);
    PyModule_AddIntConstant(poModule, "USE_TUNING", USE_TUNING);
    PyModule_AddIntConstant(poModule, "USE_MOVE", USE_MOVE);
    PyModule_AddIntConstant(poModule, "USE_TREASURE_BOX", USE_TREASURE_BOX);
    PyModule_AddIntConstant(poModule, "USE_MONEYBAG", USE_MONEYBAG);
    PyModule_AddIntConstant(poModule, "USE_BAIT", USE_BAIT);
    PyModule_AddIntConstant(poModule, "USE_ABILITY_UP", USE_ABILITY_UP);
    PyModule_AddIntConstant(poModule, "USE_AFFECT", USE_AFFECT);
    PyModule_AddIntConstant(poModule, "USE_CREATE_STONE", USE_CREATE_STONE);
    PyModule_AddIntConstant(poModule, "USE_SPECIAL", USE_SPECIAL);
    PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY", USE_POTION_NODELAY);
    PyModule_AddIntConstant(poModule, "USE_CLEAR", USE_CLEAR);
    PyModule_AddIntConstant(poModule, "USE_INVISIBILITY", USE_INVISIBILITY);
    PyModule_AddIntConstant(poModule, "USE_DETACHMENT", USE_DETACHMENT);
    PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_PER",
                            USE_TIME_CHARGE_PER);
    PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_FIX",
                            USE_TIME_CHARGE_FIX);
    PyModule_AddIntConstant(poModule, "USE_ADD_ACCESSORY_SOCKET",
                            USE_ADD_ACCESSORY_SOCKET);

    PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_NORMAL",
                            MATERIAL_DS_REFINE_NORMAL);
    PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_BLESSED",
                            MATERIAL_DS_REFINE_BLESSED);
    PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_HOLLY",
                            MATERIAL_DS_REFINE_HOLLY);

    PyModule_AddIntConstant(poModule, "DS_WHITE", DS_SLOT1);
    PyModule_AddIntConstant(poModule, "DS_RED", DS_SLOT2);
    PyModule_AddIntConstant(poModule, "DS_GREEN", DS_SLOT3);
    PyModule_AddIntConstant(poModule, "DS_BLUE", DS_SLOT4);
    PyModule_AddIntConstant(poModule, "DS_YELLOW", DS_SLOT5);
    PyModule_AddIntConstant(poModule, "DS_BLACK", DS_SLOT6);

    PyModule_AddIntConstant(poModule, "FISH_ALIVE", FISH_ALIVE);
    PyModule_AddIntConstant(poModule, "FISH_DEAD", FISH_DEAD);

    PyModule_AddIntConstant(poModule, "RESOURCE_FISHBONE", RESOURCE_FISHBONE);
    PyModule_AddIntConstant(poModule, "RESOURCE_WATERSTONEPIECE",
                            RESOURCE_WATERSTONEPIECE);
    PyModule_AddIntConstant(poModule, "RESOURCE_WATERSTONE",
                            RESOURCE_WATERSTONE);
    PyModule_AddIntConstant(poModule, "RESOURCE_BLOOD_PEARL",
                            RESOURCE_BLOOD_PEARL);
    PyModule_AddIntConstant(poModule, "RESOURCE_BLUE_PEARL",
                            RESOURCE_BLUE_PEARL);
    PyModule_AddIntConstant(poModule, "RESOURCE_WHITE_PEARL",
                            RESOURCE_WHITE_PEARL);
    PyModule_AddIntConstant(poModule, "RESOURCE_BUCKET", RESOURCE_BUCKET);
    PyModule_AddIntConstant(poModule, "RESOURCE_CRYSTAL", RESOURCE_CRYSTAL);
    PyModule_AddIntConstant(poModule, "RESOURCE_GEM", RESOURCE_GEM);
    PyModule_AddIntConstant(poModule, "RESOURCE_STONE", RESOURCE_STONE);
    PyModule_AddIntConstant(poModule, "RESOURCE_METIN", RESOURCE_METIN);
    PyModule_AddIntConstant(poModule, "RESOURCE_ORE", RESOURCE_ORE);

    PyModule_AddIntConstant(poModule, "METIN_NORMAL", METIN_NORMAL);
    PyModule_AddIntConstant(poModule, "METIN_GOLD", METIN_GOLD);

    PyModule_AddIntConstant(poModule, "METIN_NORMAL", METIN_NORMAL);
    PyModule_AddIntConstant(poModule, "METIN_GOLD", METIN_GOLD);

    PyModule_AddIntConstant(poModule, "LIMIT_NONE", LIMIT_NONE);
    PyModule_AddIntConstant(poModule, "LIMIT_LEVEL", LIMIT_LEVEL);
    PyModule_AddIntConstant(poModule, "LIMIT_STR", LIMIT_STR);
    PyModule_AddIntConstant(poModule, "LIMIT_DEX", LIMIT_DEX);
    PyModule_AddIntConstant(poModule, "LIMIT_INT", LIMIT_INT);
    PyModule_AddIntConstant(poModule, "LIMIT_CON", LIMIT_CON);

    PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME", LIMIT_REAL_TIME);
    PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME_START_FIRST_USE",
                            LIMIT_REAL_TIME_START_FIRST_USE);
    PyModule_AddIntConstant(poModule, "LIMIT_TIMER_BASED_ON_WEAR",
                            LIMIT_TIMER_BASED_ON_WEAR);
    PyModule_AddIntConstant(poModule, "LIMIT_TYPE_MAX_NUM", LIMIT_MAX_NUM);
    PyModule_AddIntConstant(poModule, "LIMIT_MAX_LEVEL", LIMIT_MAX_LEVEL);
    PyModule_AddIntConstant(poModule, "LIMIT_MAP", LIMIT_MAP);

    PyModule_AddIntConstant(poModule, "LIMIT_MAX_NUM", ITEM_LIMIT_MAX_NUM);

    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_FEMALE",
                            ITEM_ANTIFLAG_FEMALE);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MALE", ITEM_ANTIFLAG_MALE);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WARRIOR",
                            ITEM_ANTIFLAG_WARRIOR);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ASSASSIN",
                            ITEM_ANTIFLAG_ASSASSIN);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SURA", ITEM_ANTIFLAG_SURA);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SHAMAN",
                            ITEM_ANTIFLAG_SHAMAN);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GET", ITEM_ANTIFLAG_GET);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_DROP", ITEM_ANTIFLAG_DROP);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SELL", ITEM_ANTIFLAG_SELL);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_A",
                            ITEM_ANTIFLAG_EMPIRE_A);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_B",
                            ITEM_ANTIFLAG_EMPIRE_B);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_C",
                            ITEM_ANTIFLAG_EMPIRE_C);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAVE", ITEM_ANTIFLAG_SAVE);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GIVE", ITEM_ANTIFLAG_GIVE);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PKDROP",
                            ITEM_ANTIFLAG_PKDROP);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_STACK",
                            ITEM_ANTIFLAG_STACK);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MYSHOP",
                            ITEM_ANTIFLAG_MYSHOP);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAFEBOX",
                            ITEM_ANTIFLAG_SAFEBOX);

#ifdef ENABLE_ANTI_CHANGE_ATTR
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_CHANGE_ATTRIBUTE",
                            ITEM_ANTIFLAG_CHANGE_ATTRIBUTE);
#endif
#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_DESTROY",
                            ITEM_ANTIFLAG_DESTROY);
#endif

    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_CHANGELOOK",
                            ITEM_ANTIFLAG_CHANGELOOK);
    
    PyModule_AddIntConstant(poModule, "ITEM_FLAG_UNIQUE", ITEM_FLAG_UNIQUE);
    PyModule_AddIntConstant(poModule, "ITEM_FLAG_IRREMOVABLE",
                            ITEM_FLAG_IRREMOVABLE);
    PyModule_AddIntConstant(poModule, "ITEM_FLAG_CONFIRM_WHEN_USE",
                            ITEM_FLAG_CONFIRM_WHEN_USE);
    PyModule_AddIntConstant(poModule, "ITEM_FLAG_STACKABLE",
                            ITEM_FLAG_STACKABLE);


    PyModule_AddIntConstant(poModule, "ANTIFLAG_FEMALE", ITEM_ANTIFLAG_FEMALE);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_MALE", ITEM_ANTIFLAG_MALE);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_WARRIOR",
                            ITEM_ANTIFLAG_WARRIOR);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_ASSASSIN",
                            ITEM_ANTIFLAG_ASSASSIN);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_SURA", ITEM_ANTIFLAG_SURA);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_SHAMAN", ITEM_ANTIFLAG_SHAMAN);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_GET", ITEM_ANTIFLAG_GET);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_DROP", ITEM_ANTIFLAG_DROP);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_SELL", ITEM_ANTIFLAG_SELL);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_A",
                            ITEM_ANTIFLAG_EMPIRE_A);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_B",
                            ITEM_ANTIFLAG_EMPIRE_B);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_C",
                            ITEM_ANTIFLAG_EMPIRE_C);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_SAVE", ITEM_ANTIFLAG_SAVE);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_GIVE", ITEM_ANTIFLAG_GIVE);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_PKDROP", ITEM_ANTIFLAG_PKDROP);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_STACK", ITEM_ANTIFLAG_STACK);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_MYSHOP", ITEM_ANTIFLAG_MYSHOP);
    PyModule_AddIntConstant(poModule, "ANTIFLAG_SAFEBOX",
                            ITEM_ANTIFLAG_SAFEBOX);

    PyModule_AddIntConstant(poModule, "WEARABLE_BODY", WEARABLE_BODY);
    PyModule_AddIntConstant(poModule, "WEARABLE_HEAD", WEARABLE_HEAD);
    PyModule_AddIntConstant(poModule, "WEARABLE_FOOTS", WEARABLE_FOOTS);
    PyModule_AddIntConstant(poModule, "WEARABLE_WRIST", WEARABLE_WRIST);
    PyModule_AddIntConstant(poModule, "WEARABLE_WEAPON", WEARABLE_WEAPON);
    PyModule_AddIntConstant(poModule, "WEARABLE_NECK", WEARABLE_NECK);
    PyModule_AddIntConstant(poModule, "WEARABLE_EAR", WEARABLE_EAR);
    PyModule_AddIntConstant(poModule, "WEARABLE_UNIQUE", WEARABLE_UNIQUE);
    PyModule_AddIntConstant(poModule, "WEARABLE_SHIELD", WEARABLE_SHIELD);
    PyModule_AddIntConstant(poModule, "WEARABLE_ARROW", WEARABLE_ARROW);

    PyModule_AddIntConstant(poModule, "ARMOR_BODY", ARMOR_BODY);
    PyModule_AddIntConstant(poModule, "ARMOR_HEAD", ARMOR_HEAD);
    PyModule_AddIntConstant(poModule, "ARMOR_SHIELD", ARMOR_SHIELD);
    PyModule_AddIntConstant(poModule, "ARMOR_WRIST", ARMOR_WRIST);
    PyModule_AddIntConstant(poModule, "ARMOR_FOOTS", ARMOR_FOOTS);
    PyModule_AddIntConstant(poModule, "ARMOR_NECK", ARMOR_NECK);
    PyModule_AddIntConstant(poModule, "ARMOR_EAR", ARMOR_EAR);

    PyModule_AddIntConstant(poModule, "ITEM_APPLY_MAX_NUM", ITEM_APPLY_MAX_NUM);
    PyModule_AddIntConstant(poModule, "ITEM_SOCKET_MAX_NUM",
                            ITEM_SOCKET_MAX_NUM);

    PyModule_AddIntConstant(poModule, "APPLY_NONE", APPLY_NONE);
    PyModule_AddIntConstant(poModule, "APPLY_STR", APPLY_STR);
    PyModule_AddIntConstant(poModule, "APPLY_DEX", APPLY_DEX);
    PyModule_AddIntConstant(poModule, "APPLY_CON", APPLY_CON);
    PyModule_AddIntConstant(poModule, "APPLY_INT", APPLY_INT);
    PyModule_AddIntConstant(poModule, "APPLY_MAX_HP", APPLY_MAX_HP);
    PyModule_AddIntConstant(poModule, "APPLY_MAX_SP", APPLY_MAX_SP);
    PyModule_AddIntConstant(poModule, "APPLY_HP_REGEN", APPLY_HP_REGEN);
    PyModule_AddIntConstant(poModule, "APPLY_SP_REGEN", APPLY_SP_REGEN);
    PyModule_AddIntConstant(poModule, "APPLY_DEF_GRADE_BONUS",
                            APPLY_DEF_GRADE_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_ATT_GRADE_BONUS",
                            APPLY_ATT_GRADE_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_ATT_SPEED", APPLY_ATT_SPEED);
    PyModule_AddIntConstant(poModule, "APPLY_MOV_SPEED", APPLY_MOV_SPEED);
    PyModule_AddIntConstant(poModule, "APPLY_CAST_SPEED", APPLY_CAST_SPEED);
    PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATT_GRADE",
                            APPLY_MAGIC_ATT_GRADE);
    PyModule_AddIntConstant(poModule, "APPLY_MAGIC_DEF_GRADE",
                            APPLY_MAGIC_DEF_GRADE);
    PyModule_AddIntConstant(poModule, "APPLY_SKILL", APPLY_SKILL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ANIMAL",
                            APPLY_ATTBONUS_ANIMAL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD",
                            APPLY_ATTBONUS_UNDEAD);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL",
                            APPLY_ATTBONUS_DEVIL);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_HUMAN",
                            APPLY_ATTBONUS_HUMAN);
    PyModule_AddIntConstant(poModule, "APPLY_BOW_DISTANCE", APPLY_BOW_DISTANCE);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_BOW", APPLY_RESIST_BOW);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_FIRE", APPLY_RESIST_FIRE);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_ELEC", APPLY_RESIST_ELEC);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC", APPLY_RESIST_MAGIC);
    PyModule_AddIntConstant(poModule, "APPLY_POISON_PCT", APPLY_POISON_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_SLOW_PCT", APPLY_SLOW_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_STUN_PCT", APPLY_STUN_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_CRITICAL_PCT",
                            APPLY_CRITICAL_PCT); // n% 확률로 두배 타격
    PyModule_AddIntConstant(poModule, "APPLY_PENETRATE_PCT",
                            APPLY_PENETRATE_PCT); // n% 확률로 적의 방어력 무시
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ORC",
                            APPLY_ATTBONUS_ORC); // 웅귀에게 n% 추가 데미지
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MILGYO",
                            APPLY_ATTBONUS_MILGYO); // 밀교에게 n% 추가 데미지
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_INSECT",
                            APPLY_ATTBONUS_INSECT); // 밀교에게 n% 추가 데미지
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD",
                            APPLY_ATTBONUS_UNDEAD); // 시체에게 n% 추가 데미지
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL",
                            APPLY_ATTBONUS_DEVIL); // 악마에게 n% 추가 데미지
    PyModule_AddIntConstant(
        poModule, "APPLY_STEAL_HP",
        APPLY_STEAL_HP); // n% 확률로 타격의 10% 를 생명력으로 흡수
    PyModule_AddIntConstant(
        poModule, "APPLY_STEAL_SP",
        APPLY_STEAL_SP); // n% 확률로 타격의 10% 를 정신력으로 흡수
    PyModule_AddIntConstant(
        poModule, "APPLY_MANA_BURN_PCT",
        APPLY_MANA_BURN_PCT); // n% 확률로 상대의 마나를 깎는다
    PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_SP_RECOVER",
                            APPLY_DAMAGE_SP_RECOVER); // n% 확률로 정신력 2 회복
    PyModule_AddIntConstant(poModule, "APPLY_BLOCK",
                            APPLY_BLOCK); // n% 확률로 물리공격 완벽 방어
    PyModule_AddIntConstant(poModule, "APPLY_DODGE",
                            APPLY_DODGE); // n% 확률로 물리공격 완벽 회피
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_SWORD",
                            APPLY_RESIST_SWORD); // 한손검에 의한 피해를 n% 감소
    PyModule_AddIntConstant(
        poModule, "APPLY_RESIST_TWOHAND",
        APPLY_RESIST_TWOHAND); // 양손검에 의한 피해를 n% 감소
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_DAGGER",
                            APPLY_RESIST_DAGGER); // 단도에 의한 피해를 n% 감소
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_BELL",
                            APPLY_RESIST_BELL); // 방울에 의한 피해를 n% 감소
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_FAN",
                            APPLY_RESIST_FAN); // 부채에 의한 피해를 n% 감소
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_WIND",
                            APPLY_RESIST_WIND); // 바람에 의한 피해를 n% 감소
    PyModule_AddIntConstant(
        poModule, "APPLY_REFLECT_MELEE",
        APPLY_REFLECT_MELEE); // 근접 타격 n% 를 적에게 되돌린다
    PyModule_AddIntConstant(poModule, "APPLY_REFLECT_CURSE",
                            APPLY_REFLECT_CURSE);
    // 적이 나에게 저주 사용시 n% 확률로 되돌린다
    PyModule_AddIntConstant(poModule, "APPLY_POISON_REDUCE",
                            APPLY_POISON_REDUCE); // 독에 의한 데미지 감소
    PyModule_AddIntConstant(poModule, "APPLY_KILL_SP_RECOVER",
                            APPLY_KILL_SP_RECOVER);
    // 적을 죽였을때 n% 확률로 정신력 10 회복
    PyModule_AddIntConstant(
        poModule, "APPLY_EXP_DOUBLE_BONUS",
        APPLY_EXP_DOUBLE_BONUS); // n% 확률로 경험치 획득량 2배
    PyModule_AddIntConstant(poModule, "APPLY_GOLD_DOUBLE_BONUS",
                            APPLY_GOLD_DOUBLE_BONUS); // n% 확률로 돈 획득량 2배
    PyModule_AddIntConstant(
        poModule, "APPLY_ITEM_DROP_BONUS",
        APPLY_ITEM_DROP_BONUS); // n% 확률로 아이템 획득량 2배
    PyModule_AddIntConstant(
        poModule, "APPLY_POTION_BONUS",
        APPLY_POTION_BONUS); // 물약 복용시 n% 만큼 성능 증대
    PyModule_AddIntConstant(poModule, "APPLY_KILL_HP_RECOVER",
                            APPLY_KILL_HP_RECOVER); // 죽일때마다 생명력 회복
    PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_STUN",
                            APPLY_IMMUNE_STUN); // 기절 하지 않는다
    PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_SLOW",
                            APPLY_IMMUNE_SLOW); // 느려지지 않는다
    PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_FALL",
                            APPLY_IMMUNE_FALL); // 넘어지지 않는다
    PyModule_AddIntConstant(poModule, "APPLY_MAX_STAMINA",
                            APPLY_MAX_STAMINA); // 최대 스테미너 증가
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WARRIOR",
                            APPLY_ATTBONUS_WARRIOR);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ASSASSIN",
                            APPLY_ATTBONUS_ASSASSIN);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SURA",
                            APPLY_ATTBONUS_SURA);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SHAMAN",
                            APPLY_ATTBONUS_SHAMAN);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MONSTER",
                            APPLY_ATTBONUS_MONSTER);
    PyModule_AddIntConstant(poModule, "APPLY_MALL_ATTBONUS",
                            APPLY_MALL_ATTBONUS);
    PyModule_AddIntConstant(poModule, "APPLY_MALL_DEFBONUS",
                            APPLY_MALL_DEFBONUS);
    PyModule_AddIntConstant(poModule, "APPLY_MALL_EXPBONUS",
                            APPLY_MALL_EXPBONUS);
    PyModule_AddIntConstant(poModule, "APPLY_MALL_ITEMBONUS",
                            APPLY_MALL_ITEMBONUS);
    PyModule_AddIntConstant(poModule, "APPLY_MALL_GOLDBONUS",
                            APPLY_MALL_GOLDBONUS);
    PyModule_AddIntConstant(poModule, "APPLY_MAX_HP_PCT", APPLY_MAX_HP_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_MAX_SP_PCT", APPLY_MAX_SP_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BONUS",
                            APPLY_SKILL_DAMAGE_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DAMAGE_BONUS",
                            APPLY_NORMAL_HIT_DAMAGE_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_SKILL_DEFEND_BONUS",
                            APPLY_SKILL_DEFEND_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DEFEND_BONUS",
                            APPLY_NORMAL_HIT_DEFEND_BONUS);

    PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_EXP_BONUS",
                            APPLY_PC_BANG_EXP_BONUS);
    PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_DROP_BONUS",
                            APPLY_PC_BANG_DROP_BONUS);

    PyModule_AddIntConstant(poModule, "APPLY_RESIST_WARRIOR",
                            APPLY_RESIST_WARRIOR);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_ASSASSIN",
                            APPLY_RESIST_ASSASSIN);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_SURA", APPLY_RESIST_SURA);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_SHAMAN",
                            APPLY_RESIST_SHAMAN);
    PyModule_AddIntConstant(poModule, "APPLY_ENERGY", APPLY_ENERGY); // 기력
    PyModule_AddIntConstant(poModule, "APPLY_COSTUME_ATTR_BONUS",
                            APPLY_COSTUME_ATTR_BONUS);

    PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATTBONUS_PER",
                            APPLY_MAGIC_ATTBONUS_PER);
    PyModule_AddIntConstant(poModule, "APPLY_MELEE_MAGIC_ATTBONUS_PER",
                            APPLY_MELEE_MAGIC_ATTBONUS_PER);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_ICE", APPLY_RESIST_ICE);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_EARTH", APPLY_RESIST_EARTH);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_DARK", APPLY_RESIST_DARK);
    PyModule_AddIntConstant(poModule, "APPLY_ANTI_CRITICAL_PCT",
                            APPLY_ANTI_CRITICAL_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_ANTI_PENETRATE_PCT",
                            APPLY_ANTI_PENETRATE_PCT);

    PyModule_AddIntConstant(poModule, "APPLY_BOOST_CRITICAL_DMG",
                            APPLY_BOOST_CRITICAL_DMG);
    PyModule_AddIntConstant(poModule, "APPLY_BOOST_PENETRATE_DMG",
                            APPLY_BOOST_PENETRATE_DMG);

    PyModule_AddIntConstant(poModule, "E_SEAL_DATE_DEFAULT_TIMESTAMP",
                            E_SEAL_DATE_UNLIMITED_TIMESTAMP);
    PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_PCT", APPLY_BLEEDING_PCT);
    PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_REDUCE",
                            APPLY_BLEEDING_REDUCE);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WOLFMAN",
                            APPLY_ATTBONUS_WOLFMAN);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_WOLFMAN",
                            APPLY_RESIST_WOLFMAN);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_CLAW", APPLY_RESIST_CLAW);
    PyModule_AddIntConstant(poModule, "WEAPON_CLAW", WEAPON_CLAW);
    PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WOLFMAN",
                            ITEM_ANTIFLAG_WOLFMAN);
    PyModule_AddIntConstant(poModule, "APPLY_ACCEDRAIN_RATE",
                            APPLY_ACCEDRAIN_RATE);
    PyModule_AddIntConstant(poModule, "MEDIUM_MOVE_COSTUME_ATTR",
                            MEDIUM_MOVE_COSTUME_ATTR);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_METIN",
                            APPLY_ATTBONUS_METIN);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BOSS",
                            APPLY_ATTBONUS_BOSS);
    PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_TRENT",
                            APPLY_ATTBONUS_TRENT);
    PyModule_AddIntConstant(poModule, "APPLY_RESIST_HUMAN", APPLY_RESIST_HUMAN);

    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_ELECT);
    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_FIRE);
    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_ICE);
    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_WIND);
    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_EARTH);
    PyModule_AddIntMacro(poModule, APPLY_ENCHANT_DARK);
    PyModule_AddIntMacro(poModule, APPLY_ATTBONUS_DESERT);
    PyModule_AddIntMacro(poModule, APPLY_ATTBONUS_CZ);
    PyModule_AddIntMacro(poModule, APPLY_ATTBONUS_SHADOW);
}

bool IsUsableItemToItem(uint32_t vnum, TItemPos pos)
{
    const auto itemData = CItemManager::instance().GetProto(vnum);
    if (!itemData)
        return false;

    auto itemType = itemData->GetType();
    auto itemSubType = itemData->GetSubType();

    if (itemType == ITEM_USE && itemSubType == USE_TUNING)
        return true;

    if (itemType == ITEM_USE && itemSubType == USE_SPECIAL &&
        (vnum == 50263 || vnum == 50264))
        return true;

    if (itemType == ITEM_METIN)
        return true;

    if (itemType == ITEM_USE && itemSubType == USE_DETACHMENT)
        return true;

    if (itemType == ITEM_TREASURE_KEY)
        return true;

    if ((CPythonPlayer::instance().GetItemFlags(pos) & ITEM_FLAG_APPLICABLE) ==
        ITEM_FLAG_APPLICABLE)
        return true;

    if (itemType == ITEM_USE && itemSubType < USE_MAX_NUM)
        return true;

    return false;
}

bool IsPossibleChangeLookLeft(uint8_t type, ItemVnum vnum)
{
    const auto proto = CItemManager::instance().GetProto(vnum);
    if (!proto)
        return false;

    const auto itemType = proto->GetType();
    const auto subType = proto->GetSubType();

    if (type == 1) {
        switch (proto->GetType()) {
            case ITEM_WEAPON: {
                if (subType < WEAPON_ARROW ||
                    subType > WEAPON_MOUNT_SPEAR && subType != WEAPON_QUIVER)
                    return true;
                break;
            }

            case ITEM_ARMOR: {
                if (subType == ARMOR_BODY && (vnum < 11901 || vnum > 11914))
                    return true;
                break;
            }

            case ITEM_COSTUME: {
                if (subType == COSTUME_BODY || subType == COSTUME_WEAPON ||
                    subType == COSTUME_BODY_EFFECT ||
                    subType == COSTUME_WEAPON_EFFECT ||
                    subType == COSTUME_WING_EFFECT || subType == COSTUME_RANK ||
                    subType == COSTUME_HAIR)
                    return true;
                break;
            }
            case ITEM_TOGGLE: {
                if (subType == TOGGLE_MOUNT || subType == TOGGLE_PET)
                    return true;
                break;
            }
        }
    } else if (type == 2) {
        if (itemType == ITEM_QUEST) {
            if (vnum >= 50051 && vnum <= 50053)
                return true;
        } else if (itemType == ITEM_TOGGLE && subType == TOGGLE_MOUNT)
            return true;
    }

    return false;
}

bool IsPossibleChangeLookRight(uint8_t type, ItemVnum itemVnum,
                               ItemVnum itemVnum2)
{
    const auto* item1 = CItemManager::instance().GetProto(itemVnum);
    if (!item1)
        return false;

    const auto* item2 = CItemManager::instance().GetProto(itemVnum2);
    if (!item2)
        return false;

    const auto itemType = item1->GetType();
    const auto itemType2 = item2->GetType();
    const auto subType = item2->GetSubType();
    const auto subType2 = item2->GetSubType();

    bool maleOnly2 = item2->IsAntiFlag(ITEM_ANTIFLAG_FEMALE);
    bool femaleOnly2 = item2->IsAntiFlag(ITEM_ANTIFLAG_MALE);

    bool maleOnly1 = item1->IsAntiFlag(ITEM_ANTIFLAG_FEMALE);
    bool femaleOnly1 = item1->IsAntiFlag(ITEM_ANTIFLAG_MALE);

    if (maleOnly2 || femaleOnly2) {
        if (maleOnly1 && !maleOnly2)
            return false;

        if (femaleOnly1 && !femaleOnly2)
            return false;
    }

    bool raceCheck = (!item1->IsAntiFlag(ITEM_ANTIFLAG_WARRIOR) &&
                      !item2->IsAntiFlag(ITEM_ANTIFLAG_WARRIOR)) ||
                     (!item1->IsAntiFlag(ITEM_ANTIFLAG_ASSASSIN) &&
                      !item2->IsAntiFlag(ITEM_ANTIFLAG_ASSASSIN)) ||
                     (!item1->IsAntiFlag(ITEM_ANTIFLAG_SURA) &&
                      !item2->IsAntiFlag(ITEM_ANTIFLAG_SURA)) ||
                     (!item1->IsAntiFlag(ITEM_ANTIFLAG_SHAMAN) &&
                      !item2->IsAntiFlag(ITEM_ANTIFLAG_SHAMAN));
    if (!raceCheck)
        return false;

    if (type == 1) {
        switch (itemType) {
            case ITEM_WEAPON: {
                if (itemType2 == ITEM_WEAPON) {
                    if (subType2 < WEAPON_ARROW ||
                        subType2 > WEAPON_MOUNT_SPEAR &&
                            subType2 != WEAPON_QUIVER) {
                        if (subType == subType2)
                            return true;
                    }
                } else if (itemType2 == ITEM_COSTUME &&
                           subType2 == COSTUME_WEAPON) {
                    if (subType == item2->GetValue(3))
                        return true;
                }

                return false;
            } break;

            case ITEM_ARMOR: {
                if (itemType2 == ITEM_ARMOR) {
                    if (subType2 == ARMOR_BODY &&
                        (itemVnum2 < 11901 || itemVnum2 > 11914))
                        return true;
                } else if (itemType2 == ITEM_COSTUME &&
                           subType2 == COSTUME_BODY) {
                    return true;
                }
                return false;
            } break;

            case ITEM_COSTUME: {
                if (itemType2 == itemType) {
                    if (subType2 == subType) {

                        if (subType == COSTUME_WEAPON)
                            return item1->GetValue(3) == item2->GetValue(3);

                        return true;
                    }
                    return false;
                }
                return false;
            } break;

            case ITEM_TOGGLE: {
                if (itemType2 == itemType) {
                    if (subType2 == subType)
                        return true;
                }
                return false;
            } break;
        }
    } else if (type == 2) {
        if (itemType == ITEM_QUEST) {
            if (itemVnum >= 50051 && itemVnum <= 50053 &&
                itemType2 == ITEM_TOGGLE && subType2 == TOGGLE_MOUNT) {
                if (itemVnum2 != 56000 &&
                    (itemVnum2 <= 71236 || itemVnum2 > 71241))
                    return true;
            }
        } else if (itemType == ITEM_TOGGLE && subType == TOGGLE_MOUNT &&
                   itemType2 == 28 && subType2 == 2) {
            if (itemVnum2 != 56000 && (itemVnum2 <= 71236 || itemVnum2 > 71241))
                return true;
        }
    }

    return false;
}

void init_item(py::module& m)
{
    py::module item = m.def_submodule("item", "Provides item support");

    item.def("MakeItemPosition",
             py::overload_cast<uint8_t, uint16_t>(&MakeItemPosition));
    item.def(
        "MakeItemPosition",
        py::overload_cast<std::tuple<uint8_t, uint16_t>>(&MakeItemPosition));

    item.def("IsUsableItemToItem", &IsUsableItemToItem);
    item.def("IsPossibleChangeLookLeft", &IsPossibleChangeLookLeft);
    item.def("IsPossibleChangeLookRight", &IsPossibleChangeLookRight);
    item.def("IsItemUsedForDragonSoul", &itemIsItemUsedForDragonSoul);

#define DEF_PROPERTY_ARRAY(mainType, name, type, size)                         \
    .def_property_readonly(                                                    \
        #name, [](const mainType& c) -> const std::array<type, size>& {        \
            return reinterpret_cast<const std::array<type, size>&>(c.name);    \
        })
    py::class_<ItemPosition>(item, "ItemPosition")
        .def(py::init<>())
        .def(py::init<uint8_t, uint16_t>())
        .def(py::init<std::tuple<uint8_t, uint16_t>>())
        .def_readwrite("cell", &ItemPosition::cell)
        .def_readwrite("windowType", &ItemPosition::window_type);

    py::class_<TItemLimit>(item, "ItemLimit")
        .def(py::init<>())
        .def_readwrite("type", &TItemLimit::bType)
        .def_readwrite("value", &TItemLimit::value);

    py::class_<TItemApply>(item, "ItemApply")
        .def(py::init<>())
        .def(py::init<ApplyType, ApplyValue>())
        .def_readwrite("type", &TItemApply::bType)
        .def_readwrite("value", &TItemApply::lValue);

    py::class_<PagedGrid<bool>>(item, "PagedBoolGrid")
        .def(py::init<uint32_t, uint32_t, uint32_t, PagedGrid<bool>*>(),
             "width"_a, "height"_a, "pages"_a = 1, "prev"_a = nullptr)
        .def("GetSize", &PagedGrid<bool>::GetSize)
        .def("GetWidth", &PagedGrid<bool>::GetWidth)
        .def("GetHeight", &PagedGrid<bool>::GetHeight)
        .def("GetPos", &PagedGrid<bool>::GetPos)
        .def("Get", &PagedGrid<bool>::Get)
        .def("Put", &PagedGrid<bool>::Put, "item"_a, "x"_a, "y"_a,
             "height"_a = 1)
        .def("PutPos", &PagedGrid<bool>::PutPos, "item"_a, "position"_a,
             "height"_a = 1)
        .def("Clear", &PagedGrid<bool>::Clear)
        .def("ClearPos", &PagedGrid<bool>::ClearPos, "position"_a,
             "height"_a = 1)
        .def("FindBlank", &PagedGrid<bool>::FindBlank)
        .def("FindBlankBetweenPages", &PagedGrid<bool>::FindBlankBetweenPages,
             "height"_a, "startPage"_a, "endPage"_a, "exceptPosition"_a = -1)
        .def("IsEmpty", &PagedGrid<bool>::IsEmpty, "position"_a, "height"_a = 1,
             "except"_a = nullptr)
        .def(
            "__iter__",
            [](const PagedGrid<bool>& s) {
                return py::make_iterator(s.begin(), s.end());
            },
            py::keep_alive<
                0,
                1>() /* Essential: keep object alive while iterator exists */);

    /*py::class_<TItemTable>(item, "ItemTable")
    .def(py::init<>())
    .def("vnum", &TItemTable::dwVnum)
    .def("vnumRange", &TItemTable::dwVnumRange)
    .def("name", &TItemTable::szName)
    .def("localeName", &TItemTable::szLocaleName)
    .def("type", &TItemTable::bType)
    .def("subType", &TItemTable::bSubType)
    .def("weight", &TItemTable::bWeight)
    .def("size", &TItemTable::bSize)
    .def("antiFlags", &TItemTable::dwAntiFlags)
    .def("flags", &TItemTable::dwFlags)
    .def("wearFlags", &TItemTable::dwWearFlags)
    .def("immuneFlags", &TItemTable::dwImmuneFlag)
    .def("buyPrice", &TItemTable::dwIBuyItemPrice)
    .def("sellPrice", &TItemTable::dwISellItemPrice)
    // TODO: .def("limits", &TItemTable::aLimits)
    // TODO: .def("applies", &TItemTable::aApplies)
    // TODO: .def("values", &TItemTable::alValues)
    // TODO: .def("sockets", &TItemTable::alSockets)
    .def("refinedVnum", &TItemTable::dwRefinedVnum)
    .def("refineSet", &TItemTable::wRefineSet)
    .def("alterToMagicItemPct", &TItemTable::bAlterToMagicItemPct)
    .def("specular", &TItemTable::bSpecular)
    .def("gainSocketPct", &TItemTable::bGainSocketPct);
    */
    py::class_<CItemData>(item, "ItemData")
        .def(py::init<>())
        .def("GetTable", &CItemData::GetTable)
        .def("GetIndex", &CItemData::GetIndex)
        .def("GetVnum", &CItemData::GetIndex)
        .def("GetName",
             [](CItemData* itemData) -> py::bytes {
                 if (itemData)
                     return itemData->GetName();
                 return "";
             })
        .def("GetDescription",
             [](CItemData* itemData) -> py::bytes {
                 if (itemData)
                     return itemData->GetDescription();
                 return "";
             })
        .def("GetSummary",
             [](CItemData* itemData) -> py::bytes {
                 if (itemData)
                     return itemData->GetSummary();
                 return "";
             })
        .def("GetType", &CItemData::GetType)
        .def("GetSubType", &CItemData::GetSubType)
        .def("GetAttributeSetIndex", &CItemData::GetAttributeSetIndex)
        .def("IsAttrChangeableCostume", &CItemData::IsAttrChangeableCostume)
        .def("GetRefine", &CItemData::GetRefine)
        .def("GetUseTypeString", &CItemData::GetUseTypeString)
        .def("GetWeaponType", &CItemData::GetWeaponType)
        .def("GetSize", &CItemData::GetSize)
        .def("IsAntiFlag", &CItemData::IsAntiFlag)
        .def("IsFlag", &CItemData::IsFlag)
        .def("IsWearableFlag", &CItemData::IsWearableFlag)
        .def("HasNextGrade", &CItemData::HasNextGrade)
        .def("GetWearFlags", &CItemData::GetWearFlags)
        .def("GetIBuyItemPrice", &CItemData::GetIBuyItemPrice)
        .def("GetISellItemPrice", &CItemData::GetISellItemPrice)
        .def("GetApply", &CItemData::GetApply)
        .def("GetLimit", &CItemData::GetLimit)
        .def("GetAntiFlags", &CItemData::GetAntiFlags)
        .def("GetFlags", &CItemData::GetFlags)
        .def("GetValue", &CItemData::GetValue)
        .def("GetSocketCount", &CItemData::GetSocketCount)
        .def("GetRefinedVnum", &CItemData::GetRefinedVnum)
        .def("GetRefineSet", &CItemData::GetRefineSet)
        .def("GetIconImageFileName", &CItemData::GetIconImageFileName)
        .def("GetAddonType", &CItemData::GetAddonType);

    py::class_<ClientItemData>(item, "ClientItemData")
        .def(py::init<>())
        .def_readwrite("id", &ClientItemData::id)
        .def_readwrite("vnum", &ClientItemData::vnum)
        .def_readwrite("transVnum", &ClientItemData::transVnum)
        .def_readwrite("sealDate", &ClientItemData::nSealDate)
        .def_readwrite("count", &ClientItemData::count) DEF_PROPERTY_ARRAY(
            ClientItemData, attrs, TPlayerItemAttribute, ITEM_ATTRIBUTE_MAX_NUM)
            DEF_PROPERTY_ARRAY(ClientItemData, sockets, SocketValue,
                               ITEM_SOCKET_MAX_NUM)
        .def("GetSocket", &ClientItemData::GetSocket)
        .def("GetAttr", &ClientItemData::GetAttr);

    /*
     *
     * struct ItemAttrProto
{
    static const uint32_t kVersion = 1;

    uint32_t apply;
    uint32_t prob;
    int32_t values[5];
    uint8_t maxBySet[ATTRIBUTE_SET_MAX_NUM];
};
    */

    py::class_<LevelPetDisplayedFood>(item, "LevelPetDisplayedFood")
        .def(py::init<>())
        .def_readwrite("min", &LevelPetDisplayedFood::min)
        .def_readwrite("max", &LevelPetDisplayedFood::max)
        .def_readwrite("vnum", &LevelPetDisplayedFood::vnum)

        ;

    py::class_<ItemAttrProto>(item, "ItemAttrProto")
        .def(py::init<>())
        .def_readwrite("apply", &ItemAttrProto::apply)
        .def_readwrite("prob", &ItemAttrProto::prob)
            DEF_PROPERTY_ARRAY(ItemAttrProto, values, float, 5)
                DEF_PROPERTY_ARRAY(ItemAttrProto, maxBySet, uint8_t,
                                   ATTRIBUTE_SET_MAX_NUM);

    py::class_<CItemManager, std::unique_ptr<CItemManager, py::nodelete>>(
        item, "itemManager")
        .def(py::init([]() {
                 return std::unique_ptr<CItemManager, py::nodelete>(
                     CItemManager::InstancePtr());
             }),
             pybind11::return_value_policy::reference_internal)
        .def("GetProto", &CItemManager::GetProto,
             pybind11::return_value_policy::reference)
        .def("GetRefinedFromItems", &CItemManager::GetRefinedFromItems,
             pybind11::return_value_policy::reference)
        .def("GetRareItemAttr", &CItemManager::GetRareItemAttr,
             pybind11::return_value_policy::reference)
        .def("GetItemAttr", &CItemManager::GetItemAttr,
             pybind11::return_value_policy::reference)
        .def("GetLevelPetDisplayFoods", &CItemManager::GetLevelPetDisplayFoods,
             pybind11::return_value_policy::reference)
        .def("GetHyperlinkItemData", &CItemManager::GetHyperlinkItemData,
             pybind11::return_value_policy::reference)
        .def("GetLevelPetMobs", &CItemManager::GetLevelPetMobs,
             pybind11::return_value_policy::reference)
        .def("SelectItemData", &CItemManager::SelectItemData);
}
