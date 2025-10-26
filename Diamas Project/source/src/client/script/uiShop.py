# -*- coding: utf-8 -*-


import app
import chat
import chr
import item
import player
import shop
import snd
import wndMgr
from pygame.app import appInst
from pygame.item import MakeItemPosition
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
import uiCommon
from ui_event import Event
from pygame.item import itemManager


def RoundUp(num, factor):
    rest = num % factor
    if 0 == rest:
        return num

    return num + factor - rest


class ShopDialog(ui.ScriptWindow):
    TAB_BUTTON_WIDTH = 73
    TAB_BUTTON_HEIGHT = 21

    TAB_ROW_HEIGHT = 24
    TAB_ROW_BTN_COUNT = 4

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.tooltipItem = 0

        self.xShopStart = 0
        self.yShopStart = 0

        self.questionDialog = None
        self.popup = None
        self.itemBuyQuestionDialog = None
        self.dlgPickItem = None
        self.interface = None

        self.tab_index = 0
        self.tab_row_count = 0
        self.tab_rows = None
        self.tab_buttons = None

    def __GetRealIndex(self, i):
        return self.tab_index * shop.SHOP_SLOT_COUNT + i

    def Refresh(self):
        for i in xrange(shop.SHOP_SLOT_COUNT):
            idx = self.__GetRealIndex(i)
            itemCount = shop.GetItemCount(idx)
            if itemCount <= 1:
                itemCount = 0

            self.itemSlotWindow.SetItemSlot(i, shop.GetItemID(idx), itemCount)

        wndMgr.RefreshSlot(self.itemSlotWindow.GetWindowHandle())

    def BindInterface(self, interface):
        from _weakref import proxy

        self.interface = proxy(interface)

    def SetItemData(self, pos, itemID, itemCount, itemPrice):
        shop.SetItemData(pos, itemID, itemCount, itemPrice)

    def LoadDialog(self):
        PythonScriptLoader = ui.PythonScriptLoader()
        PythonScriptLoader.LoadScriptFile(self, "UIScript/ShopDialog.py")

        self.itemSlotWindow = self.GetChild("ItemSlot")
        self.btnBuy = self.GetChild("BuyButton")
        self.btnSell = self.GetChild("SellButton")
        self.btnClose = self.GetChild("CloseButton")
        self.titleBar = self.GetChild("TitleBar")

        self.itemSlotWindow.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
        self.itemSlotWindow.SetButtonEvent("LEFT", "EMPTY", self.SelectEmptySlot)
        self.itemSlotWindow.SetButtonEvent("LEFT", "EXIST", self.SelectItemSlot)
        self.itemSlotWindow.SetButtonEvent("RIGHT", "EXIST", self.UnselectItemSlot)

        self.itemSlotWindow.SetOverInItemEvent(self.OverInItem)
        self.itemSlotWindow.SetOverOutItemEvent(self.OverOutItem)

        self.btnBuy.SetToggleUpEvent(self.CancelShopping)
        self.btnBuy.SetToggleDownEvent(self.OnBuy)

        self.btnSell.SetToggleUpEvent(self.CancelShopping)
        self.btnSell.SetToggleDownEvent(self.OnSell)

        self.btnClose.SetEvent(self.AskClosePrivateShop)

        self.titleBar.SetCloseEvent(self.Close)
        import uiPickMoney

        dlgPickItem = uiPickMoney.PickMoneyDialog("TOP_MOST")
        dlgPickItem.LoadDialog()
        dlgPickItem.Hide()
        self.dlgPickItem = dlgPickItem

        self.Refresh()

    def Destroy(self):
        self.Close()
        self.ClearDictionary()

        self.tooltipItem = 0
        self.itemSlotWindow = 0
        self.btnBuy = 0
        self.btnSell = 0
        self.btnClose = 0
        self.titleBar = 0
        self.questionDialog = None
        self.popup = None

    def Open(self, vid):
        self.AddFlag("animated_board")

        isPrivateShop = False
        isMainPlayerPrivateShop = False

        if chr.IsNPC(vid):
            isPrivateShop = False
        else:
            isPrivateShop = True

        if playerInst().IsMainCharacterIndex(vid):
            isMainPlayerPrivateShop = True

            self.btnBuy.Hide()
            self.btnSell.Hide()
            self.btnClose.Show()
        else:
            isMainPlayerPrivateShop = False

            self.btnBuy.Hide()
            self.btnSell.Hide()
            self.btnClose.Hide()

        shop.Open(isPrivateShop, isMainPlayerPrivateShop)

        self.tab_index = 0

        if not isPrivateShop:
            self.__InitializeTabs()

        self.Refresh()
        self.Show()
        self.SetTop()
        self.SetFocus()

        (self.xShopStart, self.yShopStart, z) = player.GetMainCharacterPosition()

    def Close(self):
        if self.interface:
            self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
            self.interface.RefreshMarkInventoryBag()

        if self.itemBuyQuestionDialog:
            self.itemBuyQuestionDialog.Close()
            self.itemBuyQuestionDialog = None
            constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

        if self.questionDialog:
            self.OnCloseQuestionDialog()

        self.__DestroyTabs()

        shop.Close()
        appInst.instance().GetNet().SendShopEndPacket()
        self.CancelShopping()
        self.tooltipItem.HideToolTip()
        self.Hide()

    def GetIndexFromSlotPos(self, slotPos):
        return self.tab_index * shop.SHOP_SLOT_COUNT + slotPos

    def AskClosePrivateShop(self):
        questionDialog = uiCommon.QuestionDialog()
        questionDialog.SetText(localeInfo.PRIVATE_SHOP_CLOSE_QUESTION)
        questionDialog.SetAcceptEvent(self.OnClosePrivateShop)
        questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
        questionDialog.Open()
        self.questionDialog = questionDialog

        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
        return True

    def OnClosePrivateShop(self):
        appInst.instance().GetNet().SendChatPacket("/close_shop")
        self.OnCloseQuestionDialog()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def OnPressExitKey(self):
        self.Close()
        return True

    def OnBuy(self):
        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)
        app.SetCursor(app.BUY)
        self.btnSell.SetUp()

    def OnSell(self):
        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_SELL_INFO)
        app.SetCursor(app.SELL)
        self.btnBuy.SetUp()

    def CancelShopping(self):
        self.btnBuy.SetUp()
        self.btnSell.SetUp()
        app.SetCursor(app.NORMAL)

    def __OnClosePopupDialog(self):
        self.pop = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    ## 용혼석 팔리는 기능 추가.
    def SellAttachedItem(self):
        if shop.IsPrivateShop():
            mouseModule.mouseController.DeattachObject()
            return

        attachedSlotType = mouseModule.mouseController.GetAttachedType()
        attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
        attachedCount = mouseModule.mouseController.GetAttachedItemCount()
        attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

        if player.SLOT_TYPE_INVENTORY == attachedSlotType:
            item.SelectItem(attachedItemIndex)

            if item.IsAntiFlag(item.ANTIFLAG_SELL):
                popup = uiCommon.PopupDialog()
                popup.SetText(localeInfo.SHOP_CANNOT_SELL_ITEM)
                popup.SetAcceptEvent(self.__OnClosePopupDialog)
                popup.Open()
                self.popup = popup
                return

            if player.IsValuableItem(*attachedSlotPos):
                itemPrice = item.GetISellItemPrice()

                if item.Is1GoldItem():
                    itemPrice = attachedCount / itemPrice / 5
                else:
                    itemPrice = itemPrice * max(1, attachedCount) / 5

                itemName = item.GetItemName()

                questionDialog = uiCommon.QuestionDialog()
                questionDialog.SetText(
                    localeInfo.DO_YOU_SELL_ITEM(itemName, attachedCount, itemPrice)
                )
                questionDialog.SetAcceptEvent(
                    Event(self.OnSellItem, attachedSlotPos, attachedCount)
                )
                questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
                questionDialog.Open()
                self.questionDialog = questionDialog

                constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
            else:
                self.OnSellItem(attachedSlotPos, attachedCount)
        else:
            snd.PlaySound("sound/ui/loginfail.wav")

        mouseModule.mouseController.DeattachObject()

    def OnSellItem(self, slotPos, count):
        appInst.instance().GetNet().SendShopSellPacketNew(
            MakeItemPosition(slotPos), count
        )
        snd.PlaySound("sound/ui/money.wav")
        self.OnCloseQuestionDialog()

    def OnCloseQuestionDialog(self):
        if not self.questionDialog:
            return

        self.questionDialog.Close()
        self.questionDialog = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    def SelectEmptySlot(self, selectedSlotPos):
        isAttached = mouseModule.mouseController.isAttached()
        if isAttached:
            self.SellAttachedItem()

    def UnselectItemSlot(self, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        coinType = shop.GetTabCoinType(self.tab_index)
        if coinType == shop.SHOP_COIN_TYPE_CASH:
            if app.IsPressed(app.VK_SHIFT):
                itemIndex = shop.GetItemID(self.__GetRealIndex(selectedSlotPos))
                proto = itemManager().GetProto(itemIndex)
                if proto:
                    itemName = proto.GetName()
                else:
                    itemName = ""
                self.dlgPickItem.SetTitleName(itemName)
                self.dlgPickItem.SetAcceptEvent(self.OnPickItem)
                self.dlgPickItem.Open(200)
                self.dlgPickItem.SetMax(200)
                self.dlgPickItem.itemGlobalSlotIndex = selectedSlotPos
            else:
                self.AskBuyItem(selectedSlotPos)
            return

        if shop.IsPrivateShop():
            self.AskBuyItem(selectedSlotPos)
        else:
            if app.IsPressed(app.VK_SHIFT):
                itemIndex = shop.GetItemID(self.__GetRealIndex(selectedSlotPos))
                proto = itemManager().GetProto(itemIndex)
                if proto:
                    itemName = proto.GetName()
                else:
                    itemName = ""
                self.dlgPickItem.SetTitleName(itemName)
                self.dlgPickItem.SetAcceptEvent(self.OnPickItem)
                self.dlgPickItem.Open(200)
                self.dlgPickItem.SetMax(200)
                self.dlgPickItem.itemGlobalSlotIndex = selectedSlotPos
            else:
                appInst.instance().GetNet().SendShopBuyPacket(
                    self.__GetRealIndex(selectedSlotPos)
                )

    def OnPickItem(self, count):
        itemSlotIndex = self.dlgPickItem.itemGlobalSlotIndex

        itemIndex = shop.GetItemID(self.__GetRealIndex(itemSlotIndex))
        proto = itemManager().GetProto(itemIndex)
        if proto:
            if not proto.IsFlag(item.ITEM_FLAG_STACKABLE) and count > 3:
                self.AskBuyItem(itemSlotIndex, count)
            else:
                appInst.instance().GetNet().SendShopBuyPacket(
                    self.__GetRealIndex(itemSlotIndex), int(count)
                )

    def SelectItemSlot(self, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        isAttached = mouseModule.mouseController.isAttached()
        if isAttached:
            self.SellAttachedItem()
        else:
            if shop.IsMainPlayerPrivateShop():
                return

            coinType = shop.GetTabCoinType(self.tab_index)
            if coinType == shop.SHOP_COIN_TYPE_CASH:
                self.AskBuyItem(selectedSlotPos)
                return

            curCursorNum = app.GetCursor()
            if app.BUY == curCursorNum:
                self.AskBuyItem(selectedSlotPos)
            elif app.SELL == curCursorNum:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_SELL_INFO)
            else:
                selectedSlotPos = self.__GetRealIndex(selectedSlotPos)

                selectedItemID = shop.GetItemID(selectedSlotPos)
                itemCount = shop.GetItemCount(selectedSlotPos)

                shopType = player.SLOT_TYPE_SHOP
                if shop.IsPrivateShop():
                    shopType = player.SLOT_TYPE_PRIVATE_SHOP

                mouseModule.mouseController.AttachObject(
                    self, shopType, selectedSlotPos, selectedItemID, itemCount
                )
                mouseModule.mouseController.SetCallBack(
                    "INVENTORY", self.DropToInventory
                )
                snd.PlaySound("sound/ui/pick.wav")

    def DropToInventory(self):
        attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
        self.AskBuyItem(attachedSlotPos)

    def AskBuyItem(self, slotPos, count=1):
        slotPos = self.__GetRealIndex(slotPos)

        itemIndex = shop.GetItemID(slotPos)
        itemPrice = shop.GetItemPrice(slotPos) * count
        itemCount = shop.GetItemCount(slotPos) * count

        item.SelectItem(itemIndex)
        itemName = item.GetItemName()

        itemBuyQuestionDialog = uiCommon.QuestionDialog()
        coinType = shop.GetTabCoinType(self.tab_index)
        coinVnum = shop.GetTabCoinVnum(self.tab_index)
        if shop.SHOP_COIN_TYPE_GOLD == coinType:
            itemBuyQuestionDialog.SetText(
                localeInfo.DO_YOU_BUY_ITEM(
                    itemName, itemCount, localeInfo.NumberToMoneyString(itemPrice)
                )
            )
        elif shop.SHOP_COIN_TYPE_CASH == coinType:
            itemBuyQuestionDialog.SetText(
                localeInfo.DO_YOU_BUY_ITEM(
                    itemName, itemCount, localeInfo.NumberToCoinsString(itemPrice)
                )
            )
        else:
            itemBuyQuestionDialog.SetText(
                localeInfo.DO_YOU_BUY_ITEM(
                    itemName,
                    itemCount,
                    localeInfo.NumberToSecondaryCoinString(
                        itemPrice,
                        "|Eitem:{}|e {}".format(
                            coinVnum, item.GetItemNameByVnum(coinVnum)
                        ),
                    ),
                )
            )

        itemBuyQuestionDialog.SetAcceptEvent(Event(self.AnswerBuyItem, True, count))
        itemBuyQuestionDialog.SetCancelEvent(Event(self.AnswerBuyItem, False, count))
        itemBuyQuestionDialog.Open()
        itemBuyQuestionDialog.pos = slotPos
        self.itemBuyQuestionDialog = itemBuyQuestionDialog

        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

    def AnswerBuyItem(self, flag, count):
        if flag:
            pos = self.itemBuyQuestionDialog.pos
            appInst.instance().GetNet().SendShopBuyPacket(pos, count)

        self.itemBuyQuestionDialog.Close()
        self.itemBuyQuestionDialog = None

        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def OverInItem(self, slotIndex):
        slotIndex = self.__GetRealIndex(slotIndex)
        if mouseModule.mouseController.isAttached():
            return

        if 0 != self.tooltipItem:
            if shop.SHOP_COIN_TYPE_GOLD == shop.GetTabCoinType(self.tab_index):
                self.tooltipItem.SetShopItem(slotIndex)
            else:
                self.tooltipItem.SetShopItemBySecondaryCoin(
                    slotIndex, shop.GetTabCoinVnum(self.tab_index)
                )

    def OverOutItem(self):
        if 0 != self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OnUpdate(self):
        USE_SHOP_LIMIT_RANGE = 1000

        (x, y, z) = player.GetMainCharacterPosition()
        if (
            abs(x - self.xShopStart) > USE_SHOP_LIMIT_RANGE
            or abs(y - self.yShopStart) > USE_SHOP_LIMIT_RANGE
        ):
            self.RemoveFlag("animated_board")
            self.Close()

    if app.WJ_ENABLE_TRADABLE_ICON:

        def CantSellInvenItem(self, slotIndex):
            itemIndex = player.GetItemIndex(slotIndex)
            if itemIndex:
                if (
                    player.GetItemSealDate(player.INVENTORY, slotIndex) == -1
                ):  # ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿.
                    return True
                return player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_SELL)
            return False

        def BindInterface(self, interface):
            from _weakref import proxy

            self.interface = proxy(interface)

        def OnSetFocus(self):
            if not self.interface:
                return

            if shop.IsPrivateShop():
                self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
                self.interface.RefreshMarkInventoryBag()
            else:
                self.interface.SetOnTopWindow(player.ON_TOP_WND_SHOP)
                self.interface.RefreshMarkInventoryBag()

    def __InitializeTabs(self):
        tab_count = shop.GetTabCount()
        if tab_count == 1:
            # Tek seçenek varsa seçim paneline ihtiyacımız yok.
            return
    
        self.tab_row_count = (tab_count + self.TAB_ROW_BTN_COUNT - 1) // self.TAB_ROW_BTN_COUNT
    
        board = self.GetChild("board")
        board_h = board.GetHeight()
    
        additional_h = self.tab_row_count * self.TAB_ROW_HEIGHT
    
        board.SetSize(board.GetWidth(), board_h + additional_h)
        self.SetSize(self.GetWidth(), self.GetHeight() + additional_h)
    
        self.tab_rows = []
    
        for i in range(self.tab_row_count):
            w = 5 + self.TAB_BUTTON_WIDTH
            w *= min(self.TAB_ROW_BTN_COUNT, tab_count - i * self.TAB_ROW_BTN_COUNT)
    
            row = ui.Window()
            row.SetParent(board)
            row.SetPosition(0, board_h - 10 + i * self.TAB_ROW_HEIGHT - 11)
            row.SetSize(w, self.TAB_ROW_HEIGHT)
            row.SetHorizontalAlignCenter()
            row.Show()
            self.tab_rows.append(row)
    
        self.tab_buttons = ui.RadioButtonGroup()
    
        for i in range(tab_count):
            row_index = i // self.TAB_ROW_BTN_COUNT
            x = 3 + (self.TAB_BUTTON_WIDTH + 5) * (i % self.TAB_ROW_BTN_COUNT)  # 5 yerine (self.TAB_BUTTON_WIDTH + 5)
    
            btn = ui.RadioButton()
            btn.SetParent(self.tab_rows[row_index])
            btn.SetPosition(x, 0)
    
            btn.SetUpVisual("d:/ymir work/ui/dragonsoul/button_01.tga")
            btn.SetOverVisual("d:/ymir work/ui/dragonsoul/button_02.tga")
            btn.SetDownVisual("d:/ymir work/ui/dragonsoul/button_03.tga")
            # btn.SetSize(btn.GetWidth())
            # btn.SetButtonScale(1.0, 1.0)
    
            btn.SetText(shop.GetTabName(i))
    
            self.tab_buttons.AddButton(btn, Event(self.__OnClickTabButton, i), None)
    
        self.tab_buttons.Show()
        self.tab_buttons.OnClick(self.tab_index)


    def __DestroyTabs(self):
        self.tab_buttons = None
        self.tab_rows = None

        additional_h = self.tab_row_count * self.TAB_ROW_HEIGHT

        board = self.GetChild("board")
        board.SetSize(board.GetWidth(), board.GetHeight() - additional_h)
        self.SetSize(self.GetWidth(), self.GetHeight() - additional_h)

        self.tab_row_count = 0

    def __OnClickTabButton(self, index):
        self.tab_index = index
        self.Refresh()


class MallPageDialog(ui.ScriptWindow):
    def Destroy(self):
        self.ClearDictionary()

    def Open(self):
        scriptLoader = ui.PythonScriptLoader()
        scriptLoader.LoadScriptFile(self, "UIScript/MallpageDialog.py")

        self.GetChild("titlebar").SetCloseEvent(self.Close)

        (x, y) = self.GetGlobalPosition()
        x += 10
        y += 30

        MALL_PAGE_WIDTH = 600
        MALL_PAGE_HEIGHT = 480

        app.ShowWebPage(
            "http://metin2.co.kr/08_mall/game_mall/login_fail.htm",
            (x, y, x + MALL_PAGE_WIDTH, y + MALL_PAGE_HEIGHT),
        )

        self.Lock()
        self.Show()

    def Close(self):
        app.HideWebPage()
        self.Unlock()
        self.Hide()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
