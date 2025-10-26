#include "PythonCharacterManager.h"
#include "PythonExchange.h"
#include "PythonItem.h"
#include "PythonNetworkStream.h"
#include "PythonPlayer.h"
#include "PythonSafeBox.h"
#include "PythonShop.h"
#include "StdAfx.h"

#include "../eterBase/Timer.h"
#include "PythonBackground.h"

#include <game/Constants.hpp>

//////////////////////////////////////////////////////////////////////////
// SafeBox

bool CPythonNetworkStream::SendSafeBoxMoneyPacket(uint8_t byState,
                                                  uint32_t dwMoney)
{
    assert(!"CPythonNetworkStream::SendSafeBoxMoneyPacket - 사용하지 않는 "
            "함수");
    return false;

    //	TPacketCGSafeboxMoney kSafeboxMoney;
    //	kSafeboxMoney.bHeader = HEADER_CG_SAFEBOX_MONEY;
    //	kSafeboxMoney.bState = byState;
    //	kSafeboxMoney.dwMoney = dwMoney;
    //	if (!Send(sizeof(kSafeboxMoney), &kSafeboxMoney))
    //		return false;
    //
    //	return true;
}

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos,
                                                    uint16_t bySafeBoxPos)
{
    __PlayInventoryItemDropSound(InventoryPos);

    TPacketCGSafeboxCheckin kSafeboxCheckin;
    kSafeboxCheckin.ItemPos = InventoryPos;
    kSafeboxCheckin.bSafePos = bySafeBoxPos;
    Send(HEADER_CG_SAFEBOX_CHECKIN, kSafeboxCheckin);

    return true;
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(uint16_t bySafeBoxPos,
                                                     TItemPos InventoryPos)
{
    __PlaySafeBoxItemDropSound(bySafeBoxPos);

    TPacketCGSafeboxCheckout kSafeboxCheckout;
    kSafeboxCheckout.bSafePos = bySafeBoxPos;
    kSafeboxCheckout.ItemPos = InventoryPos;
    Send(HEADER_CG_SAFEBOX_CHECKOUT, kSafeboxCheckout);

    return true;
}

bool CPythonNetworkStream::SendSafeBoxItemMovePacket(uint16_t bySourcePos,
                                                     uint16_t byTargetPos,
                                                     CountType byCount)
{
    __PlaySafeBoxItemDropSound(bySourcePos);

    TPacketCGItemMove kItemMove;
    kItemMove.Cell = TItemPos(SAFEBOX, bySourcePos);
    kItemMove.count = byCount;
    kItemMove.CellTo = TItemPos(SAFEBOX, byTargetPos);
    Send(HEADER_CG_SAFEBOX_ITEM_MOVE, kItemMove);
    return true;
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket(const GcItemSetPacket& kItemSet)
{
    CPythonSafeBox::Instance().SetItemData(kItemSet.pos.cell, kItemSet.data);

    __RefreshSafeboxWindow();

    return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket(const TPacketGCItemDel& p)
{
    CPythonSafeBox::Instance().DelItemData(p.pos.cell);

    __RefreshSafeboxWindow();

    return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket(
    const TPacketCGSafeboxWrongPassword& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError");
    return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket(const TPacketCGSafeboxSize& p)
{
    CPythonSafeBox::Instance().OpenSafeBox(p.bSize);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow",
                          p.bSize);

    return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(uint8_t byMallPos,
                                                  TItemPos InventoryPos)
{
    __PlayMallItemDropSound(byMallPos);

    TPacketCGSafeboxCheckout kMallCheckoutPacket;
    kMallCheckoutPacket.bSafePos = byMallPos;
    kMallCheckoutPacket.ItemPos = InventoryPos;
    Send(HEADER_CG_MALL_CHECKOUT, kMallCheckoutPacket);

    return true;
}

bool CPythonNetworkStream::RecvMallOpenPacket(const TPacketCGSafeboxSize& p)
{
    CPythonSafeBox::Instance().OpenMall(p.bSize);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow",
                          p.bSize);

    return true;
}

bool CPythonNetworkStream::RecvMallItemSetPacket(const GcItemSetPacket& p)
{
    CPythonSafeBox::Instance().SetMallItemData(p.pos.cell, p.data);

    __RefreshMallWindow();

    return true;
}

bool CPythonNetworkStream::RecvMallItemDelPacket(const TPacketGCItemDel& p)
{
    CPythonSafeBox::Instance().DelMallItemData(p.pos.cell);

    __RefreshMallWindow();
    SPDLOG_DEBUG(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

    return true;
}

// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemSetPacket(const GcItemSetPacket& p)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

    rkPlayer.SetItemData(p.pos, p.data);

    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::RecvItemDelPacket(const TPacketGCItemDel& p)
{
    ClientItemData itemData = {};

    auto& player = CPythonPlayer::Instance();
    player.SetItemData(p.pos, itemData);

    __RefreshInventoryWindow();
    return true;
}

bool CPythonNetworkStream::RecvItemPickupInfoPacket(
    const GcItemPickupInfoPacket& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickItem",
                          p.vnum, p.count);

    return true;
}

bool CPythonNetworkStream::RecvItemUsePacket(const TPacketGCItemUse& p)
{
    __RefreshInventoryWindow();
    return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket(
    const GcPacketItemGroundAdd& p)
{
    CPythonItem::Instance().CreateItem(p.dwVID, p.dwVnum, p.x, p.y, p.z);
    return true;
}

bool CPythonNetworkStream::RecvItemOwnership(const TPacketGCItemOwnership& p)
{
    CPythonItem::Instance().SetOwnership(p.dwVID, p.szName.c_str());
    return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket(
    const TPacketGCItemGroundDel& p)
{
    CPythonItem::Instance().DeleteItem(p.dwVID);
    return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket(const packet_quickslot_add& p)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    rkPlayer.AddQuickSlot(p.pos, p.slot.type, p.slot.pos);

    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket(const packet_quickslot_del& p)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    rkPlayer.DeleteQuickSlot(p.pos);

    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket(
    const packet_quickslot_swap& p)
{

    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    rkPlayer.MoveQuickSlot(p.pos, p.pos_to);
    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::SendShopEndPacket()
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGShop packet_shop;
    packet_shop.subheader = SHOP_SUBHEADER_CG_END;
    Send(HEADER_CG_SHOP, packet_shop);

    return true;
}

bool CPythonNetworkStream::SendShopBuyPacket(uint16_t pos, uint8_t amount)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGShop p;
    p.subheader = SHOP_SUBHEADER_CG_BUY;

    CgShopActionBuy a;
    a.amount = amount;
    a.pos = pos;
    p.buyAction = a;

    Send(HEADER_CG_SHOP, p);

    return true;
}

bool CPythonNetworkStream::SendShopSellPacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGShop PacketShop;
    PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

    CgShopActionSell a;
    a.amount = 1;
    a.pos = pos;
    PacketShop.sellAction = a;
    Send(HEADER_CG_SHOP, PacketShop);

    return true;
}

bool CPythonNetworkStream::SendShopSellPacketNew(TItemPos pos,
                                                 CountType byCount)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGShop PacketShop;
    PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

    CgShopActionSell a;
    a.amount = byCount;
    a.pos = pos;
    PacketShop.sellAction = a;
    Send(HEADER_CG_SHOP, PacketShop);

    return true;
}

// Send
bool CPythonNetworkStream::SendItemUseMultiplePacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;

    if (__IsEquipItemInSlot(pos)) {
        if (CPythonExchange::Instance().isTrading()) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_AppendNotifyMessage",
                                  "CANNOT_EQUIP_EXCHANGE");
            return true;
        }

        if (CPythonShop::Instance().IsOpen()) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_AppendNotifyMessage",
                                  "CANNOT_EQUIP_SHOP");
            return true;
        }

        if (__IsPlayerAttacking())
            return true;
    }

    __PlayInventoryItemUseSound(pos);

    TPacketCGItemUse itemUsePacket;
    itemUsePacket.Cell = pos;
    Send(HEADER_CG_ITEM_USE_MULTIPLE, itemUsePacket);

    return true;
}

bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;

    if (__IsEquipItemInSlot(pos)) {
        if (CPythonExchange::Instance().isTrading()) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_AppendNotifyMessage",
                                  "CANNOT_EQUIP_EXCHANGE");
            return true;
        }

        if (CPythonShop::Instance().IsOpen()) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_AppendNotifyMessage",
                                  "CANNOT_EQUIP_SHOP");
            return true;
        }

        if (__IsPlayerAttacking())
            return true;
    }

    __PlayInventoryItemUseSound(pos);

    TPacketCGItemUse itemUsePacket;
    itemUsePacket.Cell = pos;

    Send(HEADER_CG_ITEM_USE, itemUsePacket);

    return true;
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos,
                                                   TItemPos target_pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemUseToItem itemUseToItemPacket;
    itemUseToItemPacket.Cell = source_pos;
    itemUseToItemPacket.TargetCell = target_pos;

    Send(HEADER_CG_ITEM_USE_TO_ITEM, itemUseToItemPacket);

#ifdef _DEBUG
    SPDLOG_DEBUG(" << SendItemUseToItemPacket(src=%d, dst=%d)\n",
                 source_pos.cell, target_pos.cell);
#endif

    return true;
}

bool CPythonNetworkStream::SendRemoveMetinPacket(TItemPos pos, uint8_t slot)
{
    if (!__CanActMainInstance())
        return true;

    CgRemoveMetinPacket p;
    p.targetItem = pos;
    p.slot = slot;

    Send(HEADER_GC_REMOVE_METIN, p);

    return true;
}

bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, uint32_t elk)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemDrop itemDropPacket;
    itemDropPacket.Cell = pos;
    itemDropPacket.gold = elk;
    Send(HEADER_CG_ITEM_DROP, itemDropPacket);

    return true;
}

#ifdef INGAME_WIKI
bool CPythonNetworkStream::SendWikiRequestInfo(unsigned long long retID,
                                               DWORD vnum, bool isMob)
{
    CgRecvWikiPacket pack;
    pack.ret_id = retID;
    pack.vnum = vnum;
    pack.is_mob = isMob;
    Send(HEADER_CG_WIKI_REQUEST, pack);

    return true;
}
#endif

bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, uint32_t elk,
                                                 CountType count)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemDrop2 itemDropPacket;
    itemDropPacket.Cell = pos;
    itemDropPacket.gold = elk;
    itemDropPacket.count = count;
    Send(HEADER_CG_ITEM_DROP2, itemDropPacket);

    return true;
}

bool CPythonNetworkStream::SendItemDestroyPacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemDestroy itemDestroyPacket;
    itemDestroyPacket.Cell = pos;
    Send(HEADER_CG_ITEM_DESTROY, itemDestroyPacket);

    return true;
}

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    uint32_t dwItemID = rkPlayer.GetItemIndex(uSlotPos);

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    uint32_t dwItemID = rkPlayer.GetItemIndex(uSlotPos);

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.PlayDropSound(dwItemID);
}

// void CPythonNetworkStream::__PlayShopItemDropSound(UINT uSlotPos)
//{
//	uint32_t dwItemID;
//	CPythonShop& rkShop=CPythonShop::Instance();
//	if (!rkShop.GetSlotItemID(uSlotPos, &dwItemID))
//		return;
//
//	CPythonItem& rkItem=CPythonItem::Instance();
//	rkItem.PlayDropSound(dwItemID);
//}

void CPythonNetworkStream::__PlayAcceItemDropSound(UINT uSlotPos)
{
    uint32_t dwItemID;
    CPythonPlayer& rkSafeBox = CPythonPlayer::Instance();
    if (!rkSafeBox.GetAcceSlotItemID(uSlotPos, &dwItemID))
        return;

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayChangeLookItemDropSound(UINT uSlotPos)
{
    uint32_t dwItemID;
    CPythonPlayer& rkSafeBox = CPythonPlayer::Instance();
    if (auto v = rkSafeBox.GetChangeLookData(uSlotPos); v) {

        CPythonItem& rkItem = CPythonItem::Instance();
        rkItem.PlayDropSound(v.value().vnum);
    }
}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos)
{
    uint32_t dwItemID;
    CPythonSafeBox& rkSafeBox = CPythonSafeBox::Instance();
    if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
        return;

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos)
{
    uint32_t dwItemID;
    CPythonSafeBox& rkSafeBox = CPythonSafeBox::Instance();
    if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
        return;

    CPythonItem& rkItem = CPythonItem::Instance();
    rkItem.PlayDropSound(dwItemID);
}

bool CPythonNetworkStream::SendItemSplitPacket(TItemPos pos, CountType num)
{
    if (!__CanActMainInstance())
        return true;

    if (__IsEquipItemInSlot(pos))
        return true;

    if (__IsPlayerAttacking())
        return true;
    __PlayInventoryItemDropSound(pos);

    TPacketCGItemSplit p;
    p.Cell = pos;
    p.count = num;

    Send(HEADER_CG_ITEM_SPLIT, p);

    return true;
}

bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos,
                                              CountType num)
{
    if (!__CanActMainInstance())
        return true;

    if (__IsEquipItemInSlot(pos)) {
        if (CPythonExchange::Instance().isTrading()) {
            if (pos.IsEquipPosition() || change_pos.IsEquipPosition()) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "BINARY_AppendNotifyMessage",
                                      "CANNOT_EQUIP_EXCHANGE");
                return true;
            }
        }

        if (CPythonShop::Instance().IsOpen()) {
            if (pos.IsEquipPosition() || change_pos.IsEquipPosition()) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "BINARY_AppendNotifyMessage",
                                      "CANNOT_EQUIP_SHOP");
                return true;
            }
        }

        if (__IsPlayerAttacking())
            return true;
    }

    __PlayInventoryItemDropSound(pos);

    TPacketCGItemMove itemMovePacket;
    itemMovePacket.Cell = pos;
    itemMovePacket.CellTo = change_pos;
    itemMovePacket.count = num;
    Send(HEADER_CG_ITEM_MOVE, itemMovePacket);
    return true;
}

bool CPythonNetworkStream::SendItemPickUpPacket(uint32_t vid)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemPickup itemPickUpPacket;
    itemPickUpPacket.vid = vid;
    itemPickUpPacket.time = ELTimer_GetServerFrameMSec();
    Send(HEADER_CG_ITEM_PICKUP, itemPickUpPacket);

    return true;
}

bool CPythonNetworkStream::SendQuickSlotAddPacket(uint8_t wpos, uint8_t type,
                                                  uint8_t pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGQuickslotAdd quickSlotAddPacket;
    quickSlotAddPacket.pos = wpos;
    quickSlotAddPacket.slot.type = type;
    quickSlotAddPacket.slot.pos = pos;

    Send(HEADER_CG_QUICKSLOT_ADD, quickSlotAddPacket);
    return true;
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(uint8_t pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGQuickslotDel quickSlotDelPacket;
    quickSlotDelPacket.pos = pos;
    Send(HEADER_CG_QUICKSLOT_DEL, quickSlotDelPacket);

    return true;
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(uint8_t pos,
                                                   uint8_t change_pos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGQuickslotSwap quickSlotSwapPacket;
    quickSlotSwapPacket.pos = pos;
    quickSlotSwapPacket.change_pos = change_pos;
    Send(HEADER_CG_QUICKSLOT_SWAP, quickSlotSwapPacket);

    return true;
}

bool CPythonNetworkStream::RecvSpecialEffect(
    const TPacketGCSpecialEffect& kSpecialEffect)
{

    float scale = 1.0f;
    uint32_t effect = -1;
    bool bPlayPotionSound =
        false; //포션을 먹을 경우는 포션 사운드를 출력하자.!!
    bool bAttachEffect =
        true; //캐리터에 붙는 어태치 이펙트와 일반 이펙트 구분.!!
    uint32_t effectKind = EFFECT_KIND_SPECIAL_EFFECT;
    switch (kSpecialEffect.effectType) {
        case SE_HPUP_RED:
            effect = EFFECT_HPUP_RED;
            bPlayPotionSound = true;
            break;
        case SE_SPUP_BLUE:
            effect = EFFECT_SPUP_BLUE;
            bPlayPotionSound = true;
            break;
        case SE_SPEEDUP_GREEN:
            effect = EFFECT_SPEEDUP_GREEN;
            bPlayPotionSound = true;
            break;
        case SE_DXUP_PURPLE:
            effect = EFFECT_DXUP_PURPLE;
            bPlayPotionSound = true;
            break;
        case SE_CRITICAL:
            effect = EFFECT_CRITICAL;
            break;
        case SE_PENETRATE:
            effect = EFFECT_PENETRATE;
            break;
        case SE_BLOCK:
            effect = EFFECT_BLOCK;
            break;
        case SE_DODGE:
            effect = EFFECT_DODGE;
            break;
        case SE_CHINA_FIREWORK:
            effect = EFFECT_FIRECRACKER;
            bAttachEffect = 0;
            break;
        case SE_SPIN_TOP:
            effect = EFFECT_SPIN_TOP;
            bAttachEffect = 0;
            break;
        case SE_SUCCESS:
            effect = EFFECT_SUCCESS;
            bAttachEffect = 0;
            break;
        case SE_FAIL:
            effect = EFFECT_FAIL;
            break;
        case SE_FR_SUCCESS:
            effect = EFFECT_FR_SUCCESS;
            bAttachEffect = 0;
            break;
        case SE_LEVELUP_ON_14_FOR_GERMANY:
            effect = EFFECT_LEVELUP_ON_14_FOR_GERMANY;
            bAttachEffect = 0;
            break;
        case SE_LEVELUP_UNDER_15_FOR_GERMANY:
            effect = EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
            bAttachEffect = 0;
            break;
        case SE_PERCENT_DAMAGE1:
            effect = EFFECT_PERCENT_DAMAGE1;
            break;
        case SE_PERCENT_DAMAGE2:
            effect = EFFECT_PERCENT_DAMAGE2;
            break;
        case SE_PERCENT_DAMAGE3:
            effect = EFFECT_PERCENT_DAMAGE3;
            break;
        case SE_AUTO_HPUP:
            effect = EFFECT_AUTO_HPUP;
            scale = m_autoPotionScale;
            break;
        case SE_AUTO_SPUP:
            effect = EFFECT_AUTO_SPUP;
            scale = m_autoPotionScale;
            break;
        case SE_EQUIP_RAMADAN_RING:
            effect = EFFECT_RAMADAN_RING_EQUIP;
            break;
        case SE_EQUIP_HALLOWEEN_CANDY:
            effect = EFFECT_HALLOWEEN_CANDY_EQUIP;
            break;
        case SE_EQUIP_HAPPINESS_RING:
            effect = EFFECT_HAPPINESS_RING_EQUIP;
            break;
        case SE_EQUIP_LOVE_PENDANT:
            effect = EFFECT_LOVE_PENDANT_EQUIP;
            break;
        case SE_ACCE_ABSORB:
            effect = EFFECT_ACCE_SUCESS_ABSORB;
            break;
        case SE_ACCE_EQUIP:
            effect = EFFECT_ACCE_EQUIP;
            break;
        case SE_ACCE_BACK:
            effect = EFFECT_ACCE_BACK;
            break;
        case SE_EASTER_CANDY_EQUIP:
            effect = EFFECT_EASTER_CANDY_EQIP;
            break;
        case SE_THUNDER_AREA:
            effect = EFFECT_THUNDER_AREA;
            break;
        case SE_THUNDER:
            effect = EFFECT_THUNDER;
            break;
        case SE_HEAL:
            effect = EFFECT_HEAL;
            break;
        case SE_CAPE_OF_COURAGE:
            effect = EFFECT_CAPE_OF_COURAGE;
            break;
        case SE_CHOCOLATE_PENDANT:
            effect = EFFECT_CHOCOLATE_PENDANT;
            break;
        case SE_FEATHER_WALK:
            effect = EFFECT_FEATHER_WALK;
            break;
        case SE_PEPSI_EVENT:
            effect = EFFECT_PEPSI_EVENT;
            break;
        case SE_DRAGONLAIR_STONE_UNBEATABLE_1:
            effect = EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1;
            break;
        case SE_DRAGONLAIR_STONE_UNBEATABLE_2:
            effect = EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2;
            break;
        case SE_DRAGONLAIR_STONE_UNBEATABLE_3:
            effect = EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3;
            break;
        case SE_BATTLE_POTION:
            effect = EFFECT_BATTLE_POTION;
            break;
        case SE_REFLECT:
            effect = EFFECT_REFLECT;
            break;
        case SE_SKILL_DAMAGE_ZONE:
            effect = EFFECT_SKILL_DAMAGE_ZONE;
            scale = kSpecialEffect.scale / 1036.0;
            break;
        case SE_SKILL_SAFE_ZONE:
            effect = EFFECT_SKILL_SAFE_ZONE;
            scale = kSpecialEffect.scale / 1036.0;
            break;
        case SE_METEOR:
            effect = EFFECT_METEOR;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_BEAD_RAIN:
            effect = EFFECT_BEAD_RAIN;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_ARROW_RAIN:
            effect = EFFECT_ARROW_RAIN;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_FALL_ROCK:
            effect = EFFECT_FALL_ROCK;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_HORSE_DROP:
            effect = EFFECT_HORSE_DROP;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_EGG_DROP:
            effect = EFFECT_EGG_DROP;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_DEAPO_BOOM:
            effect = EFFECT_DEAPO_BOOM;
            scale = kSpecialEffect.scale / 300.0;
            break;
        case SE_FLOWER_EVENT:
            effect = EFFECT_FLOWER_EVENT;
            break;
        case SE_GEM_PENDANT:
            effect = EFFECT_GEM_PENDANT;
            break;
        case SE_DEFENSE_WAVE_LASER:
            effect = EFFECT_DEFENSE_WAVE_LASER;
            break;
        case SE_EFFECT_PET_ATTR_CHANGE_NEW_TYPE:
            effect = EFFECT_PET_ATTR_CHANGE_NEW_TYPE;
            break;
        case SE_PET_PAY_SUMMON1:
            effect = EFFECT_PET_PAY_SUMMON1;
            break;
        case SE_SPECAIL_ROULETTE:
            effect = EFFECT_SPECAIL_ROULETTE;
            break;

        default:
            SPDLOG_ERROR("{} Unknown special effect",
                         kSpecialEffect.effectType);
            break;
    }

    switch (kSpecialEffect.type) {
        case 0:
            if (bPlayPotionSound) {
                if (CPythonPlayer::Instance().IsMainCharacterIndex(
                        kSpecialEffect.vid))
                    CPythonItem::Instance().PlayUsePotionSound();
            }

            if (-1 != effect) {
                auto pInstance =
                    CPythonCharacterManager::Instance().GetInstancePtr(
                        kSpecialEffect.vid);
                if (pInstance) {
                    if (bAttachEffect) {
                        if (effect != EFFECT_ACCE_BACK) {
                            pInstance->AttachEffect(effect, 0, scale,
                                                    effectKind);
                        } else {
                            pInstance->AttacAcceEffect();
                        }
                    } else
                        pInstance->CreateEffect(effect, 0, scale, effectKind);
                }
            }

            break;

        case 1:
            if (-1 != effect) {
                auto pInstance =
                    CPythonCharacterManager::Instance().GetInstancePtr(
                        kSpecialEffect.vid);
                if (pInstance) {
                    float z = CPythonBackground::Instance().GetHeight(
                                  kSpecialEffect.x, -kSpecialEffect.y) +
                              60.0f;

                    Vector3 pos;
                    pos.x = kSpecialEffect.x;
                    pos.y = kSpecialEffect.y;
                    pos.z = z;

                    Vector3 rot = {0.0f, 0.0f, 0.0f};
                    pInstance->CreateEffectWithPositionAndRotation(
                        effect, pos, rot, 0, scale, effectKind);
                }
            }
            break;

        case 2:
            break;
        case 3:
            break;

        default:
            return true;
    }

    return true;
}

bool CPythonNetworkStream::RecvSpecificEffect(const TPacketGCSpecificEffect& p)
{
    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
    if (pInstance) {
        CEffectManager::Instance().RegisterEffect(p.effect_file.c_str());
        pInstance->GetGraphicThingInstanceRef().AttachEffectByName(
            0, nullptr, p.effect_file.c_str());
    }

    return true;
}

bool CPythonNetworkStream::RecvSwitchbotSlotDataPacket(
    const GcSwitchbotSlotDataPacket& p)
{
    CPythonPlayer::instance().SetSwitchbotSlotData(p.data);
    return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine(
    const TPacketGCDragonSoulRefine& p)
{

    switch (p.bSubType) {
        case DS_SUB_HEADER_OPEN:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                                    "DragonSoul"
                                                                    "RefineWind"
                                                                    "ow_Open");
            break;
        case DS_SUB_HEADER_REFINE_FAIL:
        case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
        case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
        case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
        case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
        case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_DragonSoulRefineWindow_RefineFail",
                                  p.bSubType, p.Pos.window_type, p.Pos.cell);
            break;
        case DS_SUB_HEADER_REFINE_SUCCEED:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_DragonSoulRefineWindow_RefineSucceed",
                                  p.Pos.window_type, p.Pos.cell);
            break;
    }

    return true;
}

bool CPythonNetworkStream::RecvItemSealPacket(const TPacketGCSeal& p)
{

    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

    if (p.action == 0) {
        rkPlayer.SealItem(p.pos);
    } else {
        rkPlayer.UnSealItem(p.pos);
    }

    return true;
}

bool CPythonNetworkStream::RecvChangeLookPacket(const GcChangeLookPacket& p)
{
    switch (p.subheader) {
        case CHANGELOOK_SUBHEADER_GC_SET_ITEM: {
            auto kItemSet = p.itemSet.value();
            CPythonPlayer::instance().SetChangeLookItemData(kItemSet.pos.cell,
                                                            kItemSet.data);

            const auto origPos = p.pos.value();
            CPythonPlayer::instance().SetChangeLookItemInvenSlot(
                kItemSet.pos.cell, origPos);
            break;
        }
        case CHANGELOOK_SUBHEADER_GC_CLEAR_SLOT: {

            auto pos = p.pos.value();
            CPythonPlayer::instance().DelChangeLookItemData(pos.cell);
            break;
        }
        case CHANGELOOK_SUBHEADER_GC_CLEAR_ALL: {
            CPythonPlayer::instance().DelChangeLookItemData(0);
            CPythonPlayer::instance().DelChangeLookItemData(1);
            CPythonPlayer::instance().DelChangeLookItemData(2);

            break;
        }
    }

    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::RecvAccePacket(const TPacketGCAcce& p)
{

    switch (p.subheader) {
        case ACCE_SUBHEADER_GC_SET_ITEM: {

            CPythonPlayer::instance().SetAcceItemData(
                p.itemSet.value().pos.cell, p.itemSet.value().data);

            break;
        }
        case ACCE_SUBHEADER_GC_CLEAR_SLOT: {
            CPythonPlayer::instance().DelAcceItemData(p.cell.value());

            break;
        }
        case ACCE_SUBHEADER_GC_CLEAR_ALL: {

            CPythonPlayer::instance().DelAcceItemData(0);
            CPythonPlayer::instance().DelAcceItemData(1);
            CPythonPlayer::instance().DelAcceItemData(2);

            break;
        }
        case ACCE_SUBHEADER_GC_CLEAR_ONE: {

            CPythonPlayer::instance().DelAcceItemData(1);
            break;
        }
    }

    __RefreshInventoryWindow();

    return true;
}

bool CPythonNetworkStream::SendSwitchbotSlotActivatePacket(uint8_t slotIndex)
{
    CgSwitchbotPacket p;
    p.subheader = SWITCHBOT_SUBHEADER_CG_START_SLOT;

    CgSwitchbotStatusPacket p2;
    p2.slotIndex = slotIndex;
    p.status = p2;

    Send(HEADER_CG_SWITCHBOT, p);

    return true;
}

bool CPythonNetworkStream::SendSwitchbotSlotDeactivatePacket(uint8_t slotIndex)
{
    CgSwitchbotPacket p;
    p.subheader = SWITCHBOT_SUBHEADER_CG_STOP_SLOT;

    CgSwitchbotStatusPacket p2;
    p2.slotIndex = slotIndex;
    p.status = p2;

    Send(HEADER_CG_SWITCHBOT, p);

    return true;
}

bool CPythonNetworkStream::SendUpdateSwitchbotAttributePacket(uint8_t slotIndex,
                                                              uint8_t altIndex,
                                                              uint8_t attrIndex,
                                                              TItemApply attr)
{
    CgSwitchbotPacket p;
    p.subheader = SWITCHBOT_SUBHEADER_CG_SET_ATTRIBUTE;

    CgSwitchbotAttributeUpdatePacket p2;
    p2.altIndex = altIndex;
    p2.slotIndex = slotIndex;
    p2.attrIndex = attrIndex;
    p2.attribute = attr;
    p.updateAttr = p2;
    Send(HEADER_CG_SWITCHBOT, p);

    return true;
}

bool CPythonNetworkStream::SendAcceRefineCheckinPacket(TItemPos InventoryPos,
                                                       uint8_t byAccePos,
                                                       uint8_t byWindowType)
{
    __PlayInventoryItemDropSound(InventoryPos);

    TPacketCGAcce kPackAcce;
    kPackAcce.subheader = ACCE_SUBHEADER_CG_REFINE_CHECKIN;

    TPacketCGAcceCheckin kAcceCheckin;
    kAcceCheckin.ItemPos = InventoryPos;
    kAcceCheckin.bAccePos = byAccePos;
    kAcceCheckin.windowType = byWindowType;

    kPackAcce.checkin = kAcceCheckin;

    Send(HEADER_CG_ACCE, kPackAcce);

    return true;
}

bool CPythonNetworkStream::SendAcceRefineCheckoutPacket(uint8_t byAccePos)
{
    __PlayAcceItemDropSound(byAccePos);

    TPacketCGAcce kPackAcce;
    kPackAcce.subheader = ACCE_SUBHEADER_CG_REFINE_CHECKOUT;

    TPacketCGAcceCheckout acceCheckout;
    acceCheckout.bAccePos = byAccePos;
    kPackAcce.checkout = acceCheckout;
    Send(HEADER_CG_ACCE, kPackAcce);

    return true;
}

bool CPythonNetworkStream::SendAcceRefineAcceptPacket(uint8_t windowType)
{
    TPacketCGAcce kPackAcce;
    kPackAcce.subheader = ACCE_SUBHEADER_CG_REFINE_ACCEPT;

    TPacketCGAcceRefineAccept acceAccept;
    acceAccept.windowType = windowType;
    kPackAcce.accept = acceAccept;

    Send(HEADER_CG_ACCE, kPackAcce);

    return true;
}

bool CPythonNetworkStream::SendAcceRefineCancelPacket()
{
    TPacketGCAcce kPackAcce;
    kPackAcce.subheader = ACCE_SUBHEADER_CG_REFINE_CANCEL;

    Send(HEADER_CG_ACCE, kPackAcce);
    return true;
}

bool CPythonNetworkStream::SendChangeLookCheckinPacket(TItemPos InventoryPos,
                                                       uint8_t byPos)
{
    __PlayInventoryItemDropSound(InventoryPos);

    CgChangeLookPacket p;
    p.subheader = CHANGELOOK_SUBHEADER_CG_REFINE_CHECKIN;
    CgChangeLookCheckinPacket p2;
    p2.targetPos = byPos;
    p2.invenPos = InventoryPos;
    p.checkin = p2;

    Send(HEADER_CG_CHANGELOOK, p);

    return true;
}

bool CPythonNetworkStream::SendChangeLookCheckoutPacket(uint8_t byPos)
{
    __PlayChangeLookItemDropSound(byPos);

    CgChangeLookPacket p;
    p.subheader = CHANGELOOK_SUBHEADER_CG_REFINE_CHECKOUT;
    p.checkout = byPos;

    Send(HEADER_CG_CHANGELOOK, p);

    return true;
}

bool CPythonNetworkStream::SendChangeLookAcceptPacket()
{

    CgChangeLookPacket p;
    p.subheader = CHANGELOOK_SUBHEADER_CG_REFINE_ACCEPT;
    Send(HEADER_CG_CHANGELOOK, p);

    return true;
}
bool CPythonNetworkStream::SendPrivateShopSearchOpenFilter(int32_t filter)
{
    TPacketCGShopSearch p;
    p.openFilter = filter;
    Send(HEADER_CG_SHOP_SEARCH, p);

    return true;
}
bool CPythonNetworkStream::SendPrivateShopSearchOpenCategory(py::tuple category)
{
    ShopSearchOpenCategory ssoc;
    ssoc.itemType = category[0].cast<int32_t>();
    ssoc.itemSubType = category[1].cast<int32_t>();

    TPacketCGShopSearch p;
    p.openCategory = ssoc;
    Send(HEADER_CG_SHOP_SEARCH, p);

    return true;
}

bool CPythonNetworkStream::SendPrivateShopSearchInfoSearch(py::tuple vnum)
{
    ShopSearchItemSearch ssis;
    ssis.vnum = vnum[0].cast<int32_t>();
    ssis.extra = vnum[1].cast<int32_t>();

    TPacketCGShopSearch p;
    p.itemSearch = ssis;
    Send(HEADER_CG_SHOP_SEARCH, p);

    return true;
}

bool CPythonNetworkStream::SendPrivateShopSearchOpenPage(uint16_t page)
{
    ShopSearchSetPage sssp;
    sssp.page = page;

    TPacketCGShopSearch p;
    p.setPage = sssp;
    Send(HEADER_CG_SHOP_SEARCH, p);

    return true;
}
bool CPythonNetworkStream::SendPrivateShopSearchBuyItem(uint32_t itemId)
{
    TPacketCGShopSearch p;
    p.buyItem = itemId;
    Send(HEADER_CG_SHOP_SEARCH, p);
    return true;
}

bool CPythonNetworkStream::SendChangeLookCancelPacket()
{
    CgChangeLookPacket p;
    p.subheader = CHANGELOOK_SUBHEADER_CG_REFINE_CANCEL;
    Send(HEADER_CG_CHANGELOOK, p);
    return true;
}

#ifdef ENABLE_MOVE_COSTUME_ATTR
bool CPythonNetworkStream::SendItemCombinationPacket(
    const TItemPos& itemPos, const TItemPos& basePos,
    const TItemPos& materialPos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGItemCombiation p;
    p.mediumPos = itemPos;
    p.basePos = basePos;
    p.materialPos = materialPos;

    Send(HEADER_CG_ITEM_COMBINATION, p);
    return true;
}
#endif

bool CPythonNetworkStream::SendSetTitlePacket(std::string title, uint32_t color)
{
    CgSetTitlePacket p;
    p.szTitle = title;
    p.color = color;
    Send(HEADER_CG_SET_TITLE, p);
    return true;
}

bool CPythonNetworkStream::SendAttrPlusLevelPet(uint8_t index)
{
    if (!__CanActMainInstance())
        return true;

    if (CPythonExchange::Instance().isTrading()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              "CANNOT_EQUIP_EXCHANGE");
        return true;
    }

    if (CPythonShop::Instance().IsOpen()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              "CANNOT_EQUIP_SHOP");
        return true;
    }

    if (__IsPlayerAttacking())
        return true;

    CgLevelPetPacket p;
    p.subheader = LEVELPET_SUBHEADER_CG_PLUS_ATTR;
    p.index = index;
    Send(HEADER_CG_LEVEL_PET, p);
    return true;
}

bool CPythonNetworkStream::SendOpenLevelPet(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;

    if (CPythonExchange::Instance().isTrading()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              "CANNOT_EQUIP_EXCHANGE");
        return true;
    }

    if (CPythonShop::Instance().IsOpen()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              "CANNOT_EQUIP_SHOP");
        return true;
    }

    if (__IsPlayerAttacking())
        return true;

    CgLevelPetPacket p;
    p.subheader = LEVELPET_SUBHEADER_CG_OPEN;
    p.itemPos = pos;
    Send(HEADER_CG_LEVEL_PET, p);

    return true;
}

bool CPythonNetworkStream::SendCloseLevelPet()
{
    if (!__CanActMainInstance())
        return true;

    CgLevelPetPacket p;
    p.subheader = LEVELPET_SUBHEADER_CG_CLOSE;
    Send(HEADER_CG_LEVEL_PET, p);
    return true;
}

bool CPythonNetworkStream::Update()
{

    static uint32_t s_nextRefreshTime = ELTimer_GetMSec();

    uint32_t curTime = ELTimer_GetMSec();
    if (s_nextRefreshTime > curTime)
        return true;

    if (m_isRefreshCharacterWnd) {
        m_isRefreshCharacterWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharact"
                                                                "er");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshEquipmentWnd) {
        m_isRefreshEquipmentWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipme"
                                                                "nt");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshInventoryWnd) {
        m_isRefreshInventoryWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInvento"
                                                                "ry");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshExchangeWnd) {
        m_isRefreshExchangeWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchang"
                                                                "e");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshSkillWnd) {
        m_isRefreshSkillWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshSafeboxWnd) {
        m_isRefreshSafeboxWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebo"
                                                                "x");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshMallWnd) {
        m_isRefreshMallWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshStatus) {
        m_isRefreshStatus = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatu"
                                                                "s");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshMessengerWnd) {
        m_isRefreshMessengerWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMesseng"
                                                                "er");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndInfoPage) {
        m_isRefreshGuildWndInfoPage = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildIn"
                                                                "foPage");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndBoardPage) {
        m_isRefreshGuildWndBoardPage = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBo"
                                                                "ardPage");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndMemberPage) {
        m_isRefreshGuildWndMemberPage = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMe"
                                                                "mberPage");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndMemberPageGradeComboBox) {
        m_isRefreshGuildWndMemberPageGradeComboBox = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMe"
                                                                "mberPageGradeC"
                                                                "omboBox");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndSkillPage) {
        m_isRefreshGuildWndSkillPage = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSk"
                                                                "illPage");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshGuildWndGradePage) {
        m_isRefreshGuildWndGradePage = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGr"
                                                                "adePage");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshAcceWnd) {
        m_isRefreshAcceWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAcceWin"
                                                                "dow");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshShopInfoWnd) {
        m_isRefreshShopInfoWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShopInf"
                                                                "o");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshShopSearchWnd) {
        m_isRefreshShopSearchWnd = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshPrivate"
                                                                "ShopSearch");
        s_nextRefreshTime = curTime + 300;
    }

    if (m_isRefreshHuntingMissions) {
        m_isRefreshHuntingMissions = false;
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshHunting"
                                                                "Missions");
        s_nextRefreshTime = curTime + 300;
    }

    return true;
}
