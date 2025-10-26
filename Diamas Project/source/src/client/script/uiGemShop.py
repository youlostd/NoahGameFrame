# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import chat
import item
import player

import localeInfo
import ui
import uiCommon
from ui_event import Event


class GemShopWindow(ui.ScriptWindow):
    GEM_SHOP_SLOT_MAX = 9
    GEM_SHOP_REFRESH_ITEM_VNUM = 39063
    GEM_SHOP_WINDOW_LIMIT_RANGE = 500
    GEM_SHOP_ADD_ITEM_VNUM = 39064

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.tooltipitem = None
        self.wndSellItemSlot = None
        self.pop = None
        self.RefreshTimeValue = None
        self.itempricelist = []
        self.itemslottovnum = []
        self.itemslottoprice = []

        self.xGemWindowStart = 0
        self.yGemWindowStart = 0
        self.gemshoprefreshtime = 0

        self.IsOpen = 0
        self.isLoaded = False

    def __LoadWindow(self):
        if self.isLoaded:
            return

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/GemShopWindow.py")

        getChild = self.GetChild
        getChild("TitleBar").SetCloseEvent(self.Close)
        self.wndSellItemSlot = getChild("SellItemSlot")
        self.wndSellItemSlot.SetOverInItemEvent(self.OverInItem)
        self.wndSellItemSlot.SetOverOutItemEvent(self.OverOutItem)
        self.wndSellItemSlot.SetSelectItemSlotEvent(self.UseItemSlot)
        self.wndSellItemSlot.Show()

        self.itempricelist.append(getChild("slot_1_price"))
        self.itempricelist.append(getChild("slot_2_price"))
        self.itempricelist.append(getChild("slot_3_price"))
        self.itempricelist.append(getChild("slot_4_price"))
        self.itempricelist.append(getChild("slot_5_price"))
        self.itempricelist.append(getChild("slot_6_price"))
        self.itempricelist.append(getChild("slot_7_price"))
        self.itempricelist.append(getChild("slot_8_price"))
        self.itempricelist.append(getChild("slot_9_price"))

        getChild("RefreshButton").SetEvent(self.RefreshItemSlot)
        self.RefreshTimeValue = getChild("BuyRefreshTime")

        # TODO could not properly see how this functions inspecting later
        if localeInfo.IsARABIC():
            getChild("gemshopbackimg").LeftRightReverse()
            (x, y) = self.GetLocalPosition()

            # Code for left right reversing the items
            for itemPriceElement in self.itempricelist:
                itemPriceElement.SetPosition(x, y)

        self.isLoaded = True

    def Open(self):
        self.__LoadWindow()
        self.SetCenterPosition()
        self.SetTop()
        ui.ScriptWindow.Show(self)
        (self.xGemWindowStart, self.yGemWindowStart, z) = player.GetMainCharacterPosition()
        self.IsOpen = True
        player.SetGemShopWindowOpen(True)

    def Close(self):
        self.Hide()
        #appInst.instance().GetNet().SendGemShopClose()
        self.IsOpen = False
        if self.pop:
            self.pop.Close()
            self.pop = None

        player.SetGemShopWindowOpen(False)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def Destroy(self):
        if self.pop:
            self.pop.Close()
            self.pop = None

    def RefreshGemShopWindow(self):
        setGemShopSlot = self.wndSellItemSlot.SetItemSlot
        getGemShopItemId = player.GetGemShopItemID

        self.itemslottovnum = []
        self.itemslottoprice = []

        for i in xrange(self.GEM_SHOP_SLOT_MAX):
            (vnum, price, enabled, count) = getGemShopItemId(i)
            setGemShopSlot(i, vnum, count)
            self.itempricelist[i].SetText(str(price))

            self.itemslottovnum.append(vnum)
            self.itemslottoprice.append(price)
            if enabled == 0:
                openSlotCount = player.GetGemShopOpenSlotCount()
                if i >= openSlotCount:
                    self.wndSellItemSlot.DisableSlot(i)
                else:
                    self.wndSellItemSlot.LockSlot(i)

        self.wndSellItemSlot.RefreshSlot()
        self.gemshoprefreshtime = player.GetGemShopRefreshTime()

    def GemShopSlotAdd(self, slotindex):
        self.wndSellItemSlot.EnableSlot(slotindex)
        self.wndSellItemSlot.RefreshSlot()

    def GemShopSlotBuy(self, slotindex, enable):
        if not enable:
            self.wndSellItemSlot.LockSlot(slotindex)
            self.wndSellItemSlot.RefreshSlot()

    def SetItemToolTip(self, tooltip):
        self.tooltipitem = tooltip

    def __ShowToolTip(self, slotindex):
        if self.tooltipitem:
            self.tooltipitem.ClearToolTip()

            metinSlot = []
            for i in xrange(player.METIN_SOCKET_MAX_NUM):
                metinSlot.append(0)

            attrSlot = []
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                attrSlot.append([0, 0])

            self.tooltipitem.AddItemData(self.itemslottovnum[slotindex], metinSlot, attrSlot)

    def OverInItem(self, slotindex):
        self.wndSellItemSlot.SetUsableItem(False)
        self.__ShowToolTip(slotindex)

    def OverOutItem(self):
        self.wndSellItemSlot.SetUsableItem(False)
        if self.tooltipitem:
            self.tooltipitem.HideToolTip()

    def SlotAddQuestion(self, slotindex):
        if player.IsGemShopWindowOpen():
            slotsLeftToUnlock = player.GetGemShopOpenSlotItemCount(player.GetGemShopOpenSlotCount())
            if slotsLeftToUnlock == 0:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.GEM_SYSTEM_NO_MORE_ADDSLOT)
                return
            item.SelectItem(self.GEM_SHOP_ADD_ITEM_VNUM)
            itemName = item.GetItemName()
            self.pop = uiCommon.QuestionDialog()
            self.pop.SetText(localeInfo.GEM_SYSTEM_ADD_SLOT.format(itemName))
            self.pop.SetAcceptEvent(self.SlotAddQuestionAccept)
            self.pop.SetCancelEvent(self.SlotAddQuestionCancel)
            self.pop.Open()
            return

        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.GEM_SYSTEM_OPEN_GEMSHOP)

    def SlotAddQuestionAccept(self):
        self.pop.Close()
        self.pop = None
        #appInst.instance().GetNet().SendSlotAdd()

    def SlotAddQuestionCancel(self):
        if self.pop:
            self.pop.Close()
            self.pop = None

    def UseItemSlot(self, slotindex):
        (vnum, _, disabled, count) = player.GetGemShopItemID(slotindex)
        if not disabled:
            if player.GetGem() < self.itemslottoprice[slotindex]:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.GEM_SYSTEM_NOT_ENOUGH_HP_GEM)
                return

            self.pop = uiCommon.QuestionDialog()
            self.pop.SetText(localeInfo.GEM_SYSTEM_BUY_ITEM)
            self.pop.SetAcceptEvent(Event(self.SendBuyAccept, slotindex))
            self.pop.SetCancelEvent(self.OnCloseEvent)
            self.pop.Open()
            return

        if slotindex >= player.GetGemShopOpenSlotCount():
            self.SlotAddQuestion(slotindex)

    def SendBuyAccept(self, slotindex):
        self.pop.Close()
        self.pop = None
        #appInst.instance().GetNet().SendGemShopBuy(slotindex)

    def OnCloseEvent(self):
        self.pop.Close()
        self.pop = None

    def RefreshItemSlot(self):
        if player.GetItemCountByVnum(self.GEM_SHOP_REFRESH_ITEM_VNUM) == 0:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.GEM_SYSTEM_NOT_ENOUGHT_REFRESHITEM)
            return

        self.pop = uiCommon.QuestionDialog()
        self.pop.SetText(localeInfo.GEM_SYSTEM_REFRESH_SHOP_ITEMS)
        self.pop.SetAcceptEvent(self.SendRequestRefreshAccept)
        self.pop.SetCancelEvent(self.OnCloseEvent)
        self.pop.Open()

    def SendRequestRefreshAccept(self):
        self.pop.Close()
        self.pop = None
        #appInst.instance().GetNet().SendRequestRefresh()

    def OnUpdate(self):
        if self.IsOpen:
            (x, y, z) = player.GetMainCharacterPosition()
            if abs(x - self.xGemWindowStart) > self.GEM_SHOP_WINDOW_LIMIT_RANGE or abs(
                            y - self.yGemWindowStart) > self.GEM_SHOP_WINDOW_LIMIT_RANGE:
                self.RemoveFlag("animated_board")
                self.Close()
                self.AddFlag("animated_board")

            if self.gemshoprefreshtime - app.GetGlobalTimeStamp() > 0 and self.gemshoprefreshtime > 0:
                self.RefreshTimeValue.SetText(localeInfo.SecondToHMGlobal(self.gemshoprefreshtime - app.GetGlobalTimeStamp()))
