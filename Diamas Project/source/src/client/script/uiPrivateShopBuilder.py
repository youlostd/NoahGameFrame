# -*- coding: utf-8 -*-


import app
import chat
import chr
import item
import player
import shop
import snd
from pygame.app import appInst
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
from uiofflineshop import OfflineShopManager

g_isBuildingPrivateShop = False
g_itemPriceDict = {}
g_privateShopAdvertisementBoardDict = {}


def Clear():
    global g_itemPriceDict
    global g_isBuildingPrivateShop
    g_itemPriceDict = {}
    g_isBuildingPrivateShop = False


def IsPrivateShopItemPriceList():
    return g_itemPriceDict


def IsBuildingPrivateShop():
    global g_isBuildingPrivateShop
    return player.IsOpenPrivateShop() or g_isBuildingPrivateShop


def SetPrivateShopItemPrice(itemVNum, itemPrice):
    global g_itemPriceDict
    g_itemPriceDict[int(itemVNum)] = itemPrice


def GetPrivateShopItemPrice(itemVNum):
    try:
        return g_itemPriceDict[itemVNum]
    except KeyError:
        return 0


def DeleteADBoard(vid):
    if vid not in g_privateShopAdvertisementBoardDict:
        return

    del g_privateShopAdvertisementBoardDict[vid]

def UpdateADBoard():
    for key in g_privateShopAdvertisementBoardDict.keys():
        g_privateShopAdvertisementBoardDict[key].Show()

NAME_BOARD_MARKS = {}


class UIOfflineShopNameBoard(ui.ThinBoardRed):
    def __init__(self, vid, pid, shopName):
        ui.ThinBoardRed.__init__(self, "UI_BOTTOM")

        self.__shopKeeperVID = vid
        self.__ownerPID = pid

        self.__txtShopName = ui.TextLine()
        self.__txtShopName.SetParent(self)
        self.__txtShopName.SetHorizontalAlignCenter()
        self.__txtShopName.SetVerticalAlignCenter()
        self.__txtShopName.SetText(shopName)
        self.__txtShopName.UpdateRect()
        self.__txtShopName.Show()

        self.SetSize(len(shopName) * 6 + 10 * 2, 20)

        self.__nameBoardMark = NAME_BOARD_MARKS.get(vid, {'PID': pid, 'NAME': shopName, 'CLICKED_TIME': 0})

        g_privateShopAdvertisementBoardDict[vid] = self

    def GetShopKeeperVID(self):
        return self.__shopKeeperVID

    def UpdateInformation(self, vid, pid, shopName):
        self.__shopKeeperVID = vid
        self.__ownerPID = pid
        self.__txtShopName.SetText(shopName)

        self.SetSize(len(shopName) * 6 + 10 * 2, 20)

    def OnMouseLeftButtonUp(self):
        if OfflineShopManager.GetInstance().GetViewerInstance().IsShow() or OfflineShopManager.GetInstance().GetEditorInstance().IsShow():
            return

        self.__OnClick()

    def OnMouseLeftButtonDoubleClick(self):
        if OfflineShopManager.GetInstance().GetEditorInstance().IsShow():
            if OfflineShopManager.GetInstance().GetOwnerShopKeeperVID() == self.GetShopKeeperVID():
                return
        elif not OfflineShopManager.GetInstance().GetViewerInstance().IsShow():
            return

        if OfflineShopManager.GetInstance().GetEditorInstance().IsShow():
            OfflineShopManager.GetInstance().GetEditorInstance().Hide()

        if OfflineShopManager.GetInstance().GetViewerInstance().IsShow():
            if OfflineShopManager.GetInstance().GetOwnerShopKeeperVID() != self.GetShopKeeperVID():
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_USE_YET)
                return

            OfflineShopManager.GetInstance().GetViewerInstance().Hide()

        self.__OnClick()

    def __OnClick(self):
        if OfflineShopManager.GetInstance().GetOwnerShopKeeperVID() != self.GetShopKeeperVID():
            if player.GetCharacterDistance(self.GetShopKeeperVID()) > 8000:
                return

            self.__txtShopName.SetTop()

        OfflineShopManager.GetInstance().GetViewerInstance().SetTitle(self.__txtShopName.GetText())
        appInst.instance().GetNet().SendOnClickPacket(self.GetShopKeeperVID())
        self.UpdateShopsignWhenClicked()

    def UpdateShopsignWhenClicked(self):
        self.__txtShopName.SetPackedFontColor(0xffffc600)

    def Destroy(self):
        DeleteADBoard(self.__shopKeeperVID)

    def OnUpdate(self):
        if not self.__shopKeeperVID:
            return

        if appInst.instance().GetSettings().IsShowSalesText():
            self.Show()
            x, y = chr.GetProjectPosition(self.__shopKeeperVID, 220)
            self.SetPosition(x - self.GetWidth() / 2, y - self.GetHeight() / 2)

        else:
            for key in g_privateShopAdvertisementBoardDict.keys():
                if playerInst().GetMainCharacterIndex() == key:  # 상점풍선을 안보이게 감추는 경우에도, 플레이어 자신의 상점 풍선은 보이도록 함. by 김준호
                    title_bar = g_privateShopAdvertisementBoardDict[key]

                    title_bar.Show()

                    x, y = chr.GetProjectPosition(playerInst().GetMainCharacterIndex(), 220)

                    width = x - title_bar.GetWidth() / 2
                    height = y - title_bar.GetHeight() / 2

                    title_bar.SetPosition(width, height)
                else:
                    title_bar = g_privateShopAdvertisementBoardDict[key]
                    title_bar.Hide()

class PrivateShopAdvertisementDecoBoard(ui.ShopDecoTitle):
    viewedPersistMinutes = 30

    def __init__(self, titleType):
        ui.ShopDecoTitle.__init__(self, titleType - 1, "UI_BOTTOM")
        self.vid = None
        self.titleType = titleType
        self.title = ""
        self.bg = "new"
        self.vid = None

        self.lastSeen = 0
        self.__MakeTextLine()
        self.__CalcFontColor()

    def __CalcFontColor(self):
        if self.titleType != 0:
            import uiMyShopDecoration
            COLOR = int(uiMyShopDecoration.DECO_SHOP_TITLE_LIST[self.titleType - 1][2], 16)

            RED = (COLOR & 0xFF0000) >> 16
            GREEN = (COLOR & 0x00FF00) >> 8
            BLUE = COLOR & 0x0000FF

            self.textLine.SetFontColor(float(RED) / 255.0, float(GREEN) / 255.0, float(BLUE) / 255.0)

    def __MakeTextLine(self):
        self.textLine = ui.TextLine()
        self.textLine.SetParent(self)
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.Show()

    def Open(self, vid, text):
        self.vid = vid
        self.title = text
        if len(text) > 25:
            self.title = text[0:22] + "..."

        self.textLine.SetText(text)
        self.textLine.UpdateRect()
        width = max(32, len(text) * 6 + 20 * 2)
        self.SetSize(width, 32)
        # Load the last viewed time from our cache (if it exists)
        #if vid in g_privateShopAdvertisementLastViewed:
        #    self.lastSeen = g_privateShopAdvertisementLastViewed[vid]
        #    self.UpdatePattern("purple")

        g_privateShopAdvertisementBoardDict[vid] = self

    def UpdateTitle(self, text):
        self.title = text

    def OnMouseLeftButtonUp(self):
        if not self.vid:
            return

        if shop.IsOpen():
            shop.Close()
            appInst.instance().GetNet().SendShopEndPacket()

            appInst.instance().GetNet().SendOnClickPacket(self.vid)

        return True

    def UpdatePattern(self, name):
        if self.bg == name:
            return

        self.bg = name
        #self.SetPattern(name)
        #self.SetSize(len(self.title) * 6 + 10 * 2,
        #            10)  # This is required because load image screws up the resulting image

    def MarkUnseen(self):
        pass
        #self.UpdatePattern("new")

    def MarkSeen(self):
        self.lastSeen = app.GetTime()
        #self.UpdatePattern("purple")

    def Destroy(self):
        DeleteADBoard(self.vid)


class PrivateShopAdvertisementBoard(ui.ThinBoardOld):
    # These are the minutes for which the shop will display its different "viewed" color.
    viewedPersistMinutes = 30

    def __init__(self):
        ui.ThinBoard2.__init__(self, "UI_BOTTOM")
        self.vid = None
        self.title = ""
        self.bg = "new"
        self.lastSeen = 0
        self.__MakeTextLine()


    def __MakeTextLine(self):
        self.textLine = ui.TextLine()
        self.textLine.SetParent(self)
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.Show()

    def Open(self, vid, text):
        self.vid = vid
        self.title = text
        if len(text) > 25:
            self.title = text[0:22] + "..."


        self.textLine.SetText(text)
        self.textLine.UpdateRect()
        self.SetSize(len(text) * 6 + 10 * 2, 10)

        g_privateShopAdvertisementBoardDict[vid] = self

    def UpdateTitle(self, text):
        self.title = text

    def OnMouseLeftButtonUp(self):
        if not self.vid:
            return

        if shop.IsOpen():
            shop.Close()
            appInst.instance().GetNet().SendShopEndPacket()

        appInst.instance().GetNet().SendOnClickPacket(self.vid)

        return True

    def UpdatePattern(self, name):
        if self.bg == name:
            return

        self.bg = name
        self.SetSize(len(self.title) * 6 + 10 * 2,
                     10)  # This is required because load image screws up the resulting image

    def MarkUnseen(self):
        self.UpdatePattern("new")

    def MarkSeen(self):
        self.lastSeen = app.GetTime()
        self.UpdatePattern("purple")

    def Destroy(self):
        DeleteADBoard(self.vid)


class PrivateShopBuilder(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.loaded = False
        self.nameLine = None
        self.itemSlot = None
        self.btnOk = None
        self.btnClose = None
        self.titleBar = None
        self.itemSlot = None
        self.itemStock = {}
        self.tooltipItem = None
        self.priceInputBoard = None
        self.title = ""
        self.interface = None

    def __LoadWindow(self):
        if self.loaded:
            return
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PrivateShopBuilder.py")

        GetObject = self.GetChild
        self.nameLine = GetObject("NameLine")
        self.itemSlot = GetObject("ItemSlot")
        self.btnOk = GetObject("OkButton")
        self.btnClose = GetObject("CloseButton")
        self.titleBar = GetObject("TitleBar")

        self.btnOk.SetEvent(self.OnOk)
        self.btnClose.SetEvent(self.OnClose)
        self.titleBar.SetCloseEvent(self.OnClose)

        self.itemSlot.SetSelectEmptySlotEvent(self.OnSelectEmptySlot)
        self.itemSlot.SetSelectItemSlotEvent(self.OnSelectItemSlot)
        self.itemSlot.SetOverInItemEvent(self.OnOverInItem)
        self.itemSlot.SetOverOutItemEvent(self.OnOverOutItem)
        self.loaded = True

    def BindInterface(self, interface):
        from _weakref import proxy
        self.interface = proxy(interface)

    def Destroy(self):
        self.ClearDictionary()

        if self.nameLine:
            self.nameLine.KillFocus()

        if self.priceInputBoard:
            self.priceInputBoard.KillFocus()
        self.nameLine = None
        self.itemSlot = None
        self.btnOk = None
        self.btnClose = None
        self.titleBar = None
        self.priceInputBoard = None

    def Open(self, title):
        self.__LoadWindow()

        self.title = title

        if len(title) > 25:
            title = title[:22] + "..."

        self.itemStock = {}
        shop.ClearPrivateShopStock()
        self.nameLine.SetText(title)
        self.SetCenterPosition()
        self.Refresh()
        self.SetTop()
        self.SetFocus()
        self.Show()

        global g_isBuildingPrivateShop
        g_isBuildingPrivateShop = True

    def Close(self):
        global g_isBuildingPrivateShop
        g_isBuildingPrivateShop = False

        self.title = ""
        self.itemStock = {}
        self.nameLine.KillFocus()
        if self.priceInputBoard:
            self.priceInputBoard.KillFocus()
        shop.ClearPrivateShopStock()
        self.Hide()
        if app.WJ_ENABLE_TRADABLE_ICON:
            if self.interface:
                self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
                self.interface.RefreshMarkInventoryBag()

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def Refresh(self):
        getitemVNum = player.GetItemIndex
        getItemCount = player.GetItemCount
        setitemVNum = self.itemSlot.SetItemSlot
        delItem = self.itemSlot.ClearSlot

        for i in xrange(shop.SHOP_SLOT_COUNT):

            if i not in self.itemStock:
                delItem(i)
                continue

            pos = self.itemStock[i]

            itemCount = getItemCount(*pos)
            if itemCount <= 1:
                itemCount = 0
            setitemVNum(i, getitemVNum(*pos), itemCount)

        self.itemSlot.RefreshSlot()

    def OnSelectEmptySlot(self, selectedSlotPos):

        isAttached = mouseModule.mouseController.isAttached()
        if isAttached:
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
            mouseModule.mouseController.DeattachObject()

            if player.SLOT_TYPE_INVENTORY != attachedSlotType and player.SLOT_TYPE_DRAGON_SOUL_INVENTORY != attachedSlotType:
                return
            attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)

            itemVNum = player.GetItemIndex(attachedInvenType, attachedSlotPos)
            item.SelectItem(itemVNum)

            if item.IsAntiFlag(item.ANTIFLAG_GIVE) or item.IsAntiFlag(item.ANTIFLAG_MYSHOP):
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_SELL_ITEM)
                return

            import uiPickMoney
            self.priceInputBoard = uiPickMoney.PickMoneyDialog()
            self.priceInputBoard.LoadDialog()
            self.priceInputBoard.SetTitleName(localeInfo.PRIVATE_SHOP_INPUT_PRICE_DIALOG_TITLE)
            self.priceInputBoard.SetAcceptEvent(self.AcceptInputPrice)
            self.priceInputBoard.SetCancelEvent(self.CancelInputPrice)
            self.priceInputBoard.Open(999999999999999)
            self.priceInputBoard.SetMax(13)

            itemPrice = long(GetPrivateShopItemPrice(itemVNum))

            if itemPrice > 0:
                self.priceInputBoard.SetValue(long(itemPrice))

            self.priceInputBoard.itemVNum = itemVNum
            self.priceInputBoard.sourceWindowType = attachedInvenType
            self.priceInputBoard.sourceSlotPos = attachedSlotPos
            self.priceInputBoard.targetSlotPos = selectedSlotPos

    def OnSelectItemSlot(self, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return
        isAttached = mouseModule.mouseController.isAttached()
        if isAttached:
            snd.PlaySound("sound/ui/loginfail.wav")
            mouseModule.mouseController.DeattachObject()

        else:
            if not selectedSlotPos in self.itemStock:
                return

            invenType, invenPos = self.itemStock[selectedSlotPos]
            shop.DelPrivateShopItemStock(invenType, invenPos)
            snd.PlaySound("sound/ui/drop.wav")

            del self.itemStock[selectedSlotPos]

            self.Refresh()

    def AcceptInputPrice(self, price):

        if not self.priceInputBoard:
            return True

        if not price:
            return True

        if price <= 0:
            return True

        attachedInvenType = self.priceInputBoard.sourceWindowType
        sourceSlotPos = self.priceInputBoard.sourceSlotPos
        targetSlotPos = self.priceInputBoard.targetSlotPos

        for privatePos, (itemWindowType, itemSlotIndex) in self.itemStock.items():
            if itemWindowType == attachedInvenType and itemSlotIndex == sourceSlotPos:
                shop.DelPrivateShopItemStock(itemWindowType, itemSlotIndex)
                del self.itemStock[privatePos]

        if IsPrivateShopItemPriceList():
            SetPrivateShopItemPrice(self.priceInputBoard.itemVNum, long(price))

        shop.AddPrivateShopItemStock(attachedInvenType, sourceSlotPos, targetSlotPos, long(price))
        self.itemStock[targetSlotPos] = (attachedInvenType, sourceSlotPos)
        snd.PlaySound("sound/ui/drop.wav")

        self.Refresh()
        self.SetTop()

        #####
        self.priceInputBoard.KillFocus()
        self.priceInputBoard = None
        return True

    def CancelInputPrice(self):
        self.priceInputBoard.KillFocus()
        self.priceInputBoard = None
        return True

    def SetIsCashItem(self, value):
        self.IsCashItem = value

    def OnOk(self):

        if not self.title:
            return

        if 0 == len(self.itemStock):
            return

        shop.BuildPrivateShop(self.title)
        self.Close()

    def OnClose(self):
        self.Close()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def OnOverInItem(self, slotIndex):

        if self.tooltipItem:
            if slotIndex in self.itemStock:
                self.tooltipItem.SetPrivateShopBuilderItem(*self.itemStock[slotIndex] + (slotIndex,))

    def OnOverOutItem(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    if app.WJ_ENABLE_TRADABLE_ICON:
        def CantTradableItem(self, slotIndex):
            itemIndex = player.GetItemIndex(slotIndex)

            if itemIndex:
                if player.GetItemSealDate(player.INVENTORY, slotIndex) == -1:  # ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿.
                    return True
                elif player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_GIVE) or player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_MYSHOP):
                    return True
            return False

        def BindInterface(self, interface):
            from _weakref import proxy
            self.interface = proxy(interface)

        def OnSetFocus(self):
            if not self.interface:
                return

            self.interface.SetOnTopWindow(player.ON_TOP_WND_PRIVATE_SHOP)
            self.interface.RefreshMarkInventoryBag()