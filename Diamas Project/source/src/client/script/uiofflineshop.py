# -*- coding: utf-8 -*-

import logging
from _weakref import proxy

import app
import background
import chat
import chr
import offshop
import player
from pygame.app import appInst
from pygame.item import MakeItemPosition
from pygame.item import itemManager
from pygame.player import playerInst

import localeInfo
import mouseModule
import ui
import uiCommon
import uiPrivateShopBuilder
import uiScriptLocale
from ui_event import Event

OFFLINE_SHOP_CHR_VID = 1000000

BOARD_MARKED_TIME = 30 * 60  # seconds

import item
import constInfo


class OfflineShopManager:
    ## SINGLETON
    __instance = None

    @staticmethod
    def GetInstance():
        if not isinstance(OfflineShopManager.__instance, OfflineShopManager):
            OfflineShopManager.__instance = OfflineShopManager()

        return OfflineShopManager.__instance

    @staticmethod
    def Destroy():
        if not isinstance(OfflineShopManager.__instance, OfflineShopManager):
            return

        if OfflineShopManager.__instance:
            OfflineShopManager.__instance.CleanUp()

        del OfflineShopManager.__instance
        OfflineShopManager.__instance = None

    ## /SINGLETON

    __openingTime = (0, 0)

    __wndInfoIcon = None
    __wndEditor = None

    def __init__(self):
        self.__dictShopNameBoards = {}
        self.__wndBuilder = None
        self.__wndEditor = None
        self.__wndInfoIcon = None
        self.__wndViewer = None
        self.__ownerShopKeeperVID = 0
        self.__shopNameBoardRange = 2000.0
        self.__instGameWindow = None
        self.__interfaceModule = None
        self.dictItemStock = {}

    def CleanUp(self):
        for board in self.__dictShopNameBoards.itervalues():
            board.Destroy()
        self.__dictShopNameBoards = {}

    def Initialize(self, instGameWindow):
        self.__ownerShopKeeperVID = 0
        self.__shopNameBoardRange = 2000.0
        self.__instGameWindow = proxy(instGameWindow)
        self.dictItemStock = {}

        self.__wndInfoIcon = _UIOfflineShopInfoIcon()
        self.__wndInfoIcon.UpdateTime(*self.__openingTime)
        self.__wndInfoIcon.Hide()

        self.__wndBuilder = _UIOfflineShopBuilder()
        self.__wndBuilder.SetCenterPosition()
        self.__wndBuilder.Hide()

        self.__wndEditor = _UIOfflineShopEditor(self)
        self.__wndEditor.SetCenterPosition()
        self.__wndEditor.SetLeftOpeningTime(*self.__openingTime)
        self.__wndEditor.Hide()

        self.__wndViewer = _UIOfflineShopViewer(self)
        self.__wndViewer.SetCenterPosition()
        self.__wndViewer.Hide()

        offshop.SetPythonHandler(self)

    def SetInterface(self, interface):
        self.__interfaceModule = proxy(interface)

        if self.__wndBuilder:
            self.__wndBuilder.SetInterface(self.__interfaceModule)

        if self.__wndEditor:
            self.__wndEditor.SetInterface(self.__interfaceModule)

        if self.__wndViewer:
            self.__wndViewer.SetInterface(self.__interfaceModule)

    def GetBuilderInstance(self):
        if not isinstance(self.__wndBuilder, _UIOfflineShopBuilder):
            self.__wndBuilder = _UIOfflineShopBuilder()
            self.__wndBuilder.SetCenterPosition()
            self.__wndBuilder.Hide()

        return self.__wndBuilder

    def GetEditorInstance(self):
        if not isinstance(self.__wndEditor, _UIOfflineShopEditor):
            self.__wndEditor = _UIOfflineShopBuilder()
            self.__wndEditor.SetCenterPosition()
            self.__wndEditor.Hide()
        return self.__wndEditor

    def GetViewerInstance(self):
        if not isinstance(self.__wndViewer, _UIOfflineShopViewer):
            self.__wndViewer = _UIOfflineShopBuilder()
            self.__wndViewer.SetCenterPosition()
            self.__wndViewer.Hide()

        return self.__wndViewer

    def HideAllWindows(self):
        self.GetBuilderInstance().SetCenterPosition()
        self.GetBuilderInstance().Hide()
        self.GetEditorInstance().SetCenterPosition()
        self.GetEditorInstance().Hide()
        self.GetViewerInstance().SetCenterPosition()
        self.GetViewerInstance().Hide()

    def RefreshWindows(self):
        if self.GetEditorInstance().IsShow():
            self.GetEditorInstance().RefreshItemGrid()

        if self.GetViewerInstance().IsShow():
            self.GetViewerInstance().RefreshItemGrid()

    def ClearItemSlots(self):
        self.dictItemStock.clear()

    def GetItemCount(self):
        return len(self.dictItemStock)

    def CreateShop(self):
        if not self.GetEditorInstance().IsShow():
            offshop.SendCreateMyShopPacket()

    def BINARY_ReceiveOfflineShopPacket(self, subHeader):
        if subHeader == offshop.CLEAR_SHOP:
            self.ClearItemSlots()
        elif subHeader == offshop.CREATE_SHOP:
            self.GetBuilderInstance().Open()

            if self.GetViewerInstance().IsShow():
                self.GetViewerInstance().Hide()
        elif subHeader == offshop.OPEN_SHOP:
            self.GetBuilderInstance().Hide()

            if self.GetEditorInstance().IsShow():
                self.GetEditorInstance().Hide()

            self.GetViewerInstance().Show()
        elif subHeader == offshop.OPEN_MY_SHOP:
            self.GetEditorInstance().SetCenterPosition()
            self.GetEditorInstance().Show()
        elif subHeader == offshop.CLOSE_SHOP:
            self.GetBuilderInstance().Hide()

            if self.GetViewerInstance().IsShow():
                self.GetViewerInstance().Hide()
        elif subHeader == offshop.OPEN_MY_SHOP_FOR_OTHERS:
            self.GetEditorInstance().SetShopOpeningState(True)
        elif subHeader == offshop.CLOSE_MY_SHOP_FOR_OTHERS:
            self.GetEditorInstance().SetShopOpeningState(False)

        self.RefreshWindows()

    def BINARY_ReceiveAdditionalInfoPacket(self, vid, shopName, pid):

        if vid in self.__dictShopNameBoards:
           self.__dictShopNameBoards[vid].UpdateInformation(vid, pid, shopName)
           return

        self.__dictShopNameBoards[vid] = uiPrivateShopBuilder.UIOfflineShopNameBoard(vid, pid, shopName)
        self.__dictShopNameBoards[vid].Show()

    def BINARY_ReceiveAddItemPacket(
        self, vnum, transVnum, count, sockets, attributes, displayPosition, price
    ):
        if displayPosition in self.dictItemStock:
            logging.exception(
                "OfflineShopManager.BINARY_ReceiveAddItemPacket :: There's already an item at position %d (%d)!"
                % (displayPosition, vnum)
            )
            return

        item = _OfflineShopItem()
        item.VNUM = vnum
        item.TRANS_VNUM = transVnum
        item.COUNT = count
        item.SOCKETS = sockets
        item.ATTRIBUTES = attributes
        item.DISPLAY_POSITION = displayPosition
        item.PRICE = price

        self.dictItemStock[displayPosition] = item
        self.RefreshWindows()

    def BINARY_ReceiveMoveItemPacket(self, oldDisplayPosition, newDisplayPosition):
        if oldDisplayPosition not in self.dictItemStock:
            logging.exception(
                "OfflineShopManager.BINARY_ReceiveMoveItemPacket :: There's no item at position %d!"
                % (oldDisplayPosition,)
            )
            return

        if newDisplayPosition in self.dictItemStock:
            logging.exception(
                "OfflineShopManager.BINARY_ReceiveMoveItemPacket :: There's already an item at position %d!"
                % (newDisplayPosition,)
            )
            return

        self.dictItemStock[newDisplayPosition] = self.dictItemStock[oldDisplayPosition]
        del self.dictItemStock[oldDisplayPosition]
        self.RefreshWindows()

    def BINARY_ReceiveRemoveItemPacket(self, displayPosition):
        try:
            if not self.dictItemStock[displayPosition]:
                logging.exception(
                    "OfflineShopManager.BINARY_ReceiveRemoveItemPacket :: There's no item at position %d!"
                    % (displayPosition,)
                )

            del self.dictItemStock[displayPosition]
        except:
            logging.exception(
                "OfflineShopManager.BINARY_ReceiveRemoveItemPacket :: There's no item at position %d!"
                % (displayPosition,)
            )

        self.RefreshWindows()

    def BINARY_ReceiveGoldPacket(self, gold):
        self.GetEditorInstance().SetGold(gold)

    def BINARY_ReceiveNamePacket(self, name):
        self.GetEditorInstance().SetShopName(name)

    def BINARY_ReceiveLeftOpeningTimePacket(
        self, farmedOpeningTime, specialOpeningTime
    ):
        self.__openingTime = (farmedOpeningTime, specialOpeningTime)

        if self.GetInfoIcon():
            self.GetInfoIcon().UpdateTime(*self.__openingTime)

        if self.GetEditorInstance():
            self.GetEditorInstance().SetLeftOpeningTime(*self.__openingTime)

    def BINARY_ReceiveEditorPositionInfoPacket(self, mapIndex, localX, localY, channel):
        self.GetEditorInstance().SetShopPositionInfo(mapIndex, localX, localY, channel)

    def POSITIONING_OnMouseLeftButtonUp(self):
        return self.GetBuilderInstance().POSITIONING_OnMouseLeftButtonUp()

    def SetAffectShowerInstance(self, inst):
        self.__affectShowerInst = inst

    def GetInfoIcon(self):
        return self.__wndInfoIcon

    def SetShopNameBoardRange(self, range):
        self.__shopNameBoardRange = range

    def GetShopNameBoardRange(self):
        return self.__shopNameBoardRange

    def SetOwnerShopKeeperVID(self, shopKeeperVID):
        self.__ownerShopKeeperVID = shopKeeperVID

    def GetOwnerShopKeeperVID(self):
        return self.__ownerShopKeeperVID

    def GetGameWindowInstance(self):
        return self.__instGameWindow


class _OfflineShopItem:
    VNUM = 0
    TRANS_VNUM = 0
    COUNT = 0
    SOCKETS = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
    ATTRIBUTES = [(0, 1) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
    DISPLAY_POSITION = 0
    PRICE = 0


import grp
import uiToolTip


class _UIOfflineShopInfoIcon(ui.ExpandedImageBox):
    NEGATIVE_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)
    POSITIVE_COLOR = grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)

    TEXT_COLORS = {
        (0): NEGATIVE_COLOR,
        (1): POSITIVE_COLOR,
        (8 * 60 * 60): POSITIVE_COLOR,
    }

    __toolTip = None

    def __init__(self):
        ui.ExpandedImageBox.__init__(self)

        self.LoadImage("D:/ymir work/ui/skill/common/support/language3.sub")
        self.SetScale(0.7, 0.7)

        self.__closeTimestamp = 0
        self.__lastRefreshTimestamp = 0

        self.__toolTip = uiToolTip.ToolTip(250)
        self.__toolTip.HideToolTip()

        self.UpdateTime(0, 0)

    def __GetTextColor(self, time):
        color = uiToolTip.ToolTip.NEGATIVE_COLOR
        for minTime, newColor in self.TEXT_COLORS.iteritems():
            if time < minTime:
                break

            color = newColor

        return color

    def UpdateTime(self, farmedOpeningTime, specialOpeningTime):
        if not self.__toolTip:
            return

        self.__toolTip.ClearToolTip()
        self.__toolTip.SetTitle("Çevrimdışı Pazar")
        self.__toolTip.AppendTextLine(
            uiScriptLocale.PRIVATE_SHOP_LEFT_TIME.format(
                localeInfo.GetFormattedTimeString(farmedOpeningTime)
            ),
            self.__GetTextColor(farmedOpeningTime),
        )
        self.__toolTip.AppendTextLine(
            uiScriptLocale.PRIVATE_SHOP_LEFT_PREMIUM_TIME.format(
                localeInfo.GetFormattedTimeString(specialOpeningTime)
            ),
            self.__GetTextColor(specialOpeningTime),
        )

    def OnMouseOverIn(self):
        self.__toolTip.ShowToolTip()

    def OnMouseOverOut(self):
        self.__toolTip.HideToolTip()


from pygame.item import PagedBoolGrid


class _UIOfflineShopBuilder(ui.ScriptWindow):
    class _UIOfflineShopBuilderGeneralSettings(ui.ScriptWindow):
        def __init__(self, submitEvent, cancelEvent):
            self.__txtShopName = None

            ui.ScriptWindow.__init__(self)
            self.AddFlag("animated_board")

            self.__isPositioningMode = False
            self.__x, self.__y, self.__z = player.GetMainCharacterPosition()

            self.__submitEvent = submitEvent
            self.__cancelEvent = cancelEvent

            self.__LoadInterface()
            self.Reset()

        def __LoadInterface(self):
            try:
                ui.PythonScriptLoader().LoadScriptFile(
                    self, "uiscript/offlineshopbuilder-general-settings.py"
                )
            except:
                import logging

                logging.exception(
                    "_UIOfflineShopBuilder._UIOfflineShopBuilderGeneralSettings.__LoadInterface :: Failed to load UIScript file."
                )

            try:
                self.GetChild("bBoard").SetCloseEvent(self.__Cancel)

                self.__txtShopName = self.GetChild("txtShopName")
                self.__txtShopName.SetReturnEvent(self.__Submit)

                if offshop.ALLOW_MANUAL_POSITIONING:
                    self.GetChild("btnPosition").SetEvent(self.SetPositioningMode, True)
                self.GetChild("btnSubmit").SetEvent(self.__Submit)
                self.GetChild("btnCancel").SetEvent(self.__Cancel)
            except:
                import logging

                logging.exception(
                    "_UIOfflineShopBuilder._UIOfflineShopBuilderGeneralSettings.__LoadInterface :: Failed to bind objects."
                )

        def __Submit(self):
            self.__submitEvent()

        def __Cancel(self):
            self.__cancelEvent()

        def SetPositioningMode(self, positioningMode):
            self.__isPositioningMode = positioningMode

        def IsPositioningMode(self):
            return self.__isPositioningMode

        def SpawnPseudoShopKeeper(self):
            chr.CreateInstance(OFFLINE_SHOP_CHR_VID)
            chr.SelectInstance(OFFLINE_SHOP_CHR_VID)
            chr.SetVirtualID(OFFLINE_SHOP_CHR_VID)
            chr.SetInstanceType(chr.INSTANCE_TYPE_NPC)

            chr.SetRace(30000)
            chr.SetArmor(0)
            chr.SetLoopMotion(chr.MOTION_WAIT)

            chr.SetPixelPosition(self.__x, self.__y, self.__z)

        def IsPseudoShopKeeperSpawned(self):
            return chr.HasInstance(OFFLINE_SHOP_CHR_VID)

        def PurgePseudoShopKeeper(self):
            if self.IsPseudoShopKeeperSpawned():
                chr.DeleteInstanceByFade(OFFLINE_SHOP_CHR_VID)

        def Reset(self):
            self.__txtShopName.SetText("")
            self.__isPositioningMode = False
            self.__x, self.__y, self.__z = player.GetMainCharacterPosition()

        def GetShopName(self):
            return self.__txtShopName.GetText()

        def GetSelectedXYPosition(self):
            return (self.__x, self.__y)

        def Open(self):
            self.SetCenterPosition()

            if offshop.ALLOW_MANUAL_POSITIONING:
                self.SpawnPseudoShopKeeper()

            self.__txtShopName.SetFocus()

            ui.Window.Show(self)

        def Hide(self):
            self.PurgePseudoShopKeeper()
            ui.Window.Hide(self)

        def OnKeyDown(self, key):
            if key == app.VK_ESCAPE:
                self.__Cancel()
                return True

        def OnUpdate(self):
            if self.IsPositioningMode():
                if not self.IsPseudoShopKeeperSpawned():
                    self.SpawnPseudoShopKeeper()

                self.__x, self.__y, self.__z = background.GetPickingPoint()
                chr.SelectInstance(OFFLINE_SHOP_CHR_VID)
                chr.SetPixelPosition(self.__x, self.__y, self.__z)

    def __init__(self):
        self.__isUILoaded = False

        self.__wndGeneralSettings = None
        self.__interface = None

        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__dictItems = {}
        self.__dictLastItemPrices = {}

        self.__LoadInterface()
        self.__grid = PagedBoolGrid(10, 9)
        self.Hide()

    def Destroy(self):
        del self.__dictItems
        del self.__dictLastItemPrices
        del self.__wndGeneralSettings
        del self.__inpDlgPrice
        del self.__toolTipItem
        del self.__txtShopName
        del self.__gtItems

    def SetInterface(self, interface):
        self.__interface = interface

        self.__toolTipItem.BindInterface(self.__interface)

    def __LoadInterface(self):
        ui.PythonScriptLoader().LoadScriptFile(self, "uiscript/offlineshopbuilder.py")

        self.__wndGeneralSettings = self._UIOfflineShopBuilderGeneralSettings(
            self.__OnSubmitGeneralSettings, self.Hide
        )
        self.__wndGeneralSettings.Hide()

        self.__inpDlgPrice = uiCommon.MoneyInputDialog()
        self.__inpDlgPrice.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_PRICE_DIALOG_TITLE)
        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.SetAcceptEvent(self.__OnAcceptInputPrice)
        self.__inpDlgPrice.SetCancelEvent(self.__OnCancelInputPrice)
        self.__inpDlgPrice.ITEM_INFO = {
            "VNUM": 0,
            "SLOT_TYPE": 0,
            "SLOT_INDEX": 0,
            "TARGET_SLOT_INDEX": 0,
        }
        self.__inpDlgPrice.Hide()

        self.__toolTipItem = uiToolTip.ItemToolTip()
        self.__toolTipItem.HideToolTip()

        self.GetChild("bBoard").SetCloseEvent(self.Hide)

        # self.GetChild("slotBarShopName").SetClickEvent(self.__wndGeneralSettings.Open)
        self.__txtShopName = self.GetChild("txtShopName")

        self.__gtItems = self.GetChild("gtItems")
        self.__gtItems.SetSelectEmptySlotEvent(self.__OnSelectEmptySlot)
        self.__gtItems.SetSelectItemSlotEvent(self.__OnSelectItemSlot)
        self.__gtItems.SetOverInItemEvent(self.__OnOverInItem)
        self.__gtItems.SetOverOutItemEvent(self.__OnOverOutItem)

        self.GetChild("btnSubmit").SetEvent(self.__SendCreatePacket)
        self.GetChild("btnCancel").SetEvent(self.Hide)

        self.__isUILoaded = True

    def __RefreshItemGrid(self):
        i = self.__gtItems.GetStartIndex()
        while self.__gtItems.HasSlot(i):
            if i in self.__dictItems:

                itemData = playerInst().GetItemData(
                    MakeItemPosition(
                        self.__dictItems[i]["SLOT_TYPE"],
                        self.__dictItems[i]["SLOT_INDEX"],
                    )
                )
                if itemData:
                    itemCount = itemData.count
                    self.__gtItems.SetItemSlot(
                        i, itemData.vnum, itemCount > 1 and itemCount or 0
                    )

                    if itemData.transVnum == 0:
                        self.__gtItems.EnableSlotCoverImage(i, False)
                    else:
                        self.__gtItems.SetSlotCoverImage(
                            i, "icon/item/ingame_convert_Mark.tga"
                        )
                else:
                    self.__gtItems.ClearSlot(i)

            else:
                self.__gtItems.ClearSlot(i)

            i += 1

        self.__gtItems.RefreshSlot()

    def __OnSubmitGeneralSettings(self):
        shopName = self.__wndGeneralSettings.GetShopName()
        if len(shopName) < 3:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_NAME_LENGTH_MIN)
            return
        elif len(shopName) > 32:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_NAME_LENGTH_MAX)
            return

        self.__wndGeneralSettings.Hide()

        # Reset focus to game
        OfflineShopManager.GetInstance().GetGameWindowInstance().CheckFocus()

        self.__txtShopName.SetText(shopName)
        self.Show()

    def __OnAcceptInputPrice(self):
        if not self.__inpDlgPrice.GetText():
            return True

        slotIndexToRemove = -1
        for slotIndex, itemInfo in self.__dictItems.items():
            if (
                itemInfo["SLOT_TYPE"] == self.__inpDlgPrice.ITEM_INFO["SLOT_TYPE"]
                and itemInfo["SLOT_INDEX"] == self.__inpDlgPrice.ITEM_INFO["SLOT_INDEX"]
            ):
                slotIndexToRemove = slotIndex
                break
        if slotIndexToRemove != -1:
            self.__RemoveItem(slotIndexToRemove)

        price = long(self.__inpDlgPrice.GetValue())
        if price < 1:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_ENTER_PRICE)
            return True

        self.__SetLastItemPrice(
            player.GetItemIndex(
                self.__inpDlgPrice.ITEM_INFO["SLOT_TYPE"],
                self.__inpDlgPrice.ITEM_INFO["SLOT_INDEX"],
            ),
            price,
        )

        self.__dictItems[self.__inpDlgPrice.ITEM_INFO["TARGET_SLOT_INDEX"]] = {
            "SLOT_TYPE": self.__inpDlgPrice.ITEM_INFO["SLOT_TYPE"],
            "SLOT_INDEX": self.__inpDlgPrice.ITEM_INFO["SLOT_INDEX"],
            "PRICE": price,
        }

        item.SelectItem(
            player.GetItemIndex(
                self.__inpDlgPrice.ITEM_INFO["SLOT_TYPE"],
                self.__inpDlgPrice.ITEM_INFO["SLOT_INDEX"],
            )
        )

        width, height = item.GetItemSize()

        if not self.__grid.PutPos(
            True, self.__inpDlgPrice.ITEM_INFO["TARGET_SLOT_INDEX"], height
        ):
            logging.debug(
                "Slot at target positon %d is not empty",
                self.__inpDlgPrice.ITEM_INFO["TARGET_SLOT_INDEX"],
            )

        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.Hide()

        self.SetFocus()

        self.__RefreshItemGrid()
        return True

    def __OnCancelInputPrice(self):
        self.__inpDlgPrice.ITEM_INFO = {
            "VNUM": 0,
            "SLOT_TYPE": 0,
            "SLOT_INDEX": 0,
            "TARGET_SLOT_INDEX": 0,
        }
        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.Hide()

    def AddItemByClick(self, itemPos):
        itemVnum = playerInst().GetItemIndex(itemPos)
        itemData = itemManager().GetProto(itemVnum)
        if not itemData:
            return

        sizeY = itemData.GetSize()

        if itemData.IsAntiFlag(item.ANTIFLAG_GIVE) or itemData.IsAntiFlag(
            item.ANTIFLAG_MYSHOP
        ):
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_SELL_ITEM)
            return

        freeCell = self.__grid.FindBlank(sizeY)
        if freeCell is None:
            return

        self.__inpDlgPrice.ITEM_INFO = {
            "VNUM": itemVnum,
            "SLOT_TYPE": itemPos.windowType,
            "SLOT_INDEX": itemPos.cell,
            "TARGET_SLOT_INDEX": freeCell,
        }
        self.__inpDlgPrice.SetValue(0)

        self.__inpDlgPrice.Open()
        self.__inpDlgPrice.SetTop()
        self.__inpDlgPrice.SetFocus()

    def __OnSelectEmptySlot(self, slotIndex):
        if mouseModule.mouseController.isAttached():
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotIndex = mouseModule.mouseController.GetAttachedSlotPosition()
            mouseModule.mouseController.DeattachObject()

            if attachedSlotType not in [
                player.SLOT_TYPE_INVENTORY,
                player.SLOT_TYPE_DRAGON_SOUL_INVENTORY,
            ]:
                return

            itemPos = MakeItemPosition(attachedSlotIndex)
            itemVnum = playerInst().GetItemIndex(itemPos)
            itemData = itemManager().GetProto(itemVnum)
            if not itemData:
                return

            if itemData.IsAntiFlag(item.ANTIFLAG_GIVE) or itemData.IsAntiFlag(
                item.ANTIFLAG_MYSHOP
            ):
                chat.AppendChat(
                    CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_SELL_ITEM
                )
                return

            self.__inpDlgPrice.ITEM_INFO = {
                "VNUM": itemVnum,
                "SLOT_TYPE": attachedSlotIndex[0],
                "SLOT_INDEX": attachedSlotIndex[1],
                "TARGET_SLOT_INDEX": slotIndex,
            }

            self.__inpDlgPrice.Open()
            self.__inpDlgPrice.SetTop()
            self.__inpDlgPrice.SetFocus()

            lastPrice = self.__GetLastItemPrice(itemVnum)
            if lastPrice != 0:
                self.__inpDlgPrice.SetValue(lastPrice)

    def __OnSelectItemSlot(self, slotIndex):
        self.__RemoveItem(slotIndex)

    def __OnOverInItem(self, slotIndex):
        if slotIndex not in self.__dictItems:
            return

        self.__toolTipItem.SetInventoryItem(
            self.__dictItems[slotIndex]["SLOT_INDEX"],
            self.__dictItems[slotIndex]["SLOT_TYPE"],
        )
        self.__toolTipItem.AppendPrice(self.__dictItems[slotIndex]["PRICE"])
        self.__toolTipItem.ShowToolTip()

    def __OnOverOutItem(self):
        self.__toolTipItem.HideToolTip()

    def __RemoveItem(self, slotIndex):
        if slotIndex not in self.__dictItems:
            return

        itemPos = MakeItemPosition(
            (
                self.__dictItems[slotIndex]["SLOT_TYPE"],
                self.__dictItems[slotIndex]["SLOT_INDEX"],
            )
        )
        itemVnum = playerInst().GetItemIndex(itemPos)
        itemData = itemManager().GetProto(itemVnum)
        if not itemData:
            return

        self.__grid.ClearPos(slotIndex, itemData.GetSize())

        del self.__dictItems[slotIndex]
        self.__RefreshItemGrid()

    def __SetLastItemPrice(self, vnum, price):
        self.__dictLastItemPrices[vnum] = price

    def __GetLastItemPrice(self, vnum):
        if vnum not in self.__dictLastItemPrices:
            return 0

        return self.__dictLastItemPrices[vnum]

    def __SendCreatePacket(self):
        if len(self.__dictItems) < 1:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_IS_EMPTY)
            return

        (x, y) = self.__wndGeneralSettings.GetSelectedXYPosition()
        isShowEditor = (
            OfflineShopManager.GetInstance().GetEditorInstance().IsShow() != 0
        )
        OfflineShopManager.GetInstance().GetEditorInstance().JumpNextCentering(
            isShowEditor
        )
        offshop.SendCreatePacket(
            self.__wndGeneralSettings.GetShopName(),
            x,
            y,
            self.__dictItems,
            isShowEditor,
        )
        self.Hide()

    def __Cancel(self):
        if self.IsShow() or (
            self.__wndGeneralSettings and self.__wndGeneralSettings.IsShow()
        ):
            offshop.SendCancelCreateShopPacket()
            self.Reset()

    def Open(self):
        self.Reset()
        self.__wndGeneralSettings.Open()
        self.SetFocus()
        self.SetCenterPosition()

    def Hide(self):
        if not self.IsShow():
            if self.__wndGeneralSettings:
                if not self.__wndGeneralSettings.IsShow():
                    return
            else:
                return

        self.__Cancel()

        self.__grid.Clear()

        if self.__isUILoaded:
            self.__wndGeneralSettings.Hide()
            self.__wndGeneralSettings.PurgePseudoShopKeeper()

        if self.__interface:
            self.__interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
            self.__interface.RefreshMarkInventoryBag()

        # Reset focus to game
        OfflineShopManager.GetInstance().GetGameWindowInstance().CheckFocus()

        ui.Window.Hide(self)

    def Reset(self):
        if not self.__isUILoaded:
            return

        self.__wndGeneralSettings.Reset()
        self.__grid.Clear()

        del self.__dictItems
        self.__dictItems = {}
        del self.__dictLastItemPrices
        self.__dictLastItemPrices = {}

        self.__RefreshItemGrid()

    def POSITIONING_OnMouseLeftButtonUp(self):
        if self.__wndGeneralSettings.IsPositioningMode():
            self.__wndGeneralSettings.SetPositioningMode(False)
            return True

        return False

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.__Cancel()
            return True
        return False

    def OnSetFocus(self):
        if not self.__interface:
            return

        interface = self.__interface
        interface.SetOnTopWindow(player.ON_TOP_WND_SHOP_BUILD)
        interface.RefreshMarkInventoryBag()

    def IsBlockedItem(self, slotIndex):
        itemIndex = player.GetItemIndex(player.INVENTORY, slotIndex)

        for index, itemInfo in self.__dictItems.items():
            if itemInfo["SLOT_INDEX"] == slotIndex:
                return True

        if itemIndex:
            if (
                player.GetItemSealDate(player.INVENTORY, slotIndex) == -1
            ):  # ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿.
                return True
            elif player.IsAntiFlagBySlot(
                slotIndex, item.ANTIFLAG_GIVE
            ) or player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_MYSHOP):
                return True
        return False


class _UIOfflineShopEditor(ui.ScriptWindow):
    def __init__(self, mgrInst):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__dictItemStock = OfflineShopManager.GetInstance().dictItemStock
        self.__buyState = False
        self.__gold = 0
        self.__shopName = ""
        self.__isJumpNextCentering = False
        self.__interface = None

        self.__LoadInterface()

    def SetInterface(self, interface):
        self.__interface = interface

        self.__toolTipItem.BindInterface(interface)

    def __LoadInterface(self):
        ui.PythonScriptLoader().LoadScriptFile(self, "uiscript/offlineshopeditor.py")

        self.__inpDlgPrice = uiCommon.MoneyInputDialog()
        self.__inpDlgPrice.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_PRICE_DIALOG_TITLE)
        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.SetAcceptEvent(self.__OnAcceptInputPrice)
        self.__inpDlgPrice.SetCancelEvent(self.__OnCancelInputPrice)
        self.__inpDlgPrice.ITEM_INFO = {
            "VNUM": 0,
            "SLOT_TYPE": 0,
            "SLOT_INDEX": 0,
            "TARGET_SLOT_INDEX": 0,
        }
        self.__inpDlgPrice.Hide()

        self.__dlgAskRemove = uiCommon.QuestionDialog()
        self.__dlgAskRemove.SetText(localeInfo.PRIVATE_SHOP_REMOVE_ITEM_QUESTION)
        self.__dlgAskRemove.SetCancelEvent(self.__dlgAskRemove.Hide)
        self.__dlgAskRemove.INVENTORY_INDEX = -1
        self.__dlgAskRemove.DISPLAY_SLOT_INDEX = -1
        self.__dlgAskRemove.Hide()

        self.__dlgAskClose = uiCommon.QuestionDialog()
        self.__dlgAskClose.SetText(localeInfo.PRIVATE_SHOP_CLOSE_QUESTION)
        self.__dlgAskClose.SetAcceptEvent(self.__CloseShop)
        self.__dlgAskClose.SetCancelEvent(self.__dlgAskClose.Hide)
        self.__dlgAskClose.Hide()

        self.__toolTipItem = uiToolTip.ItemToolTip()
        self.__toolTipItem.HideToolTip()

        self.__bBoard = self.GetChild("bBoard")
        self.background_border = self.GetChild("background_border")
        self.__bBoard.SetCloseEvent(self.Hide)

        self.__slotBarName = self.GetChild("slotBarName")
        self.__txtShopName = self.GetChild("txtShopName")
        # self.__txtShopName.SetClickEvent(self.__txtShopName.OnSetFocus)
        self.__txtShopName.SetOnFocusEvent(self.__StartEditShopName)
        self.__txtShopName.SetKillFocusEvent(Event(self.__CancelEditShopName, True))
        self.__txtShopName.SetReturnEvent(self.__SubmitEditShopName)
        self.__txtShopName.SetEscapeEvent(self.__CancelEditShopName)

        self.__gtItems = self.GetChild("gtItems")
        self.__gtItems.SetSelectItemSlotEvent(self.__OnSelectItemSlot)
        self.__gtItems.SetSelectEmptySlotEvent(self.__OnSelectEmptySlot)
        self.__gtItems.SetUseSlotEvent(self.OnClickItemSlot)
        self.__gtItems.SetUnselectItemSlotEvent(self.OnClickItemSlot)
        self.__gtItems.SetOverInItemEvent(self.__OnOverInItem)
        self.__gtItems.SetOverOutItemEvent(self.__OnOverOutItem)

        self.__txtLeftTime = self.GetChild("txtLeftTime")
        self.__leftTimeGauge = self.GetChild("OfflineShopTimeGauge")
        self.__leftTimeSlot = self.GetChild("OfflineShopTimeSlot")

        self.__txtGold = self.GetChild("txtGold")
        self.__txtOfflineShopTime = self.GetChild("txtOfflineShopTime")
        self.__slotBarGold = self.GetChild("slotBarGold")
        self.__txtShopGold = self.GetChild("txtShopGold")
        self.__btnWithdrawGold = self.GetChild("btnWithdrawGold")
        self.__btnWithdrawGold.SetEvent(self.__WithdrawGold)

        self.__btnOpenShop = self.GetChild("btnOpenShop")
        self.__btnOpenShop.SetEvent(self.__OpenMyShop)

        self.__btnWarpShop = self.GetChild("btnWarpShop")
        self.__btnWarpShop.SetEvent(self.__WarpToShop)

        self.__btnCloseShop = self.GetChild("btnCloseShop")
        self.__btnCloseShop.SetEvent(self.__AskClose)

    def __AskClose(self):
        self.__dlgAskClose.Open()
        self.__dlgAskClose.SetTop()
        self.__dlgAskClose.SetFocus()

    def __OpenMyShop(self):
        appInst.instance().GetNet().SendOpenMyShop()

    def __WarpToShop(self):
        appInst.instance().GetNet().SendOfflineShopWarpPacket()

    def __OnAcceptInputPrice(self):
        if not self.__inpDlgPrice.GetValue():
            return True

        price = long(self.__inpDlgPrice.GetValue())
        offshop.SendAddItemPacket(
            self.__inpDlgPrice.ITEM_INFO["SLOT_TYPE"],
            self.__inpDlgPrice.ITEM_INFO["SLOT_INDEX"],
            self.__inpDlgPrice.ITEM_INFO["TARGET_SLOT_INDEX"],
            price,
        )

        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.Hide()
        return True

    def __OnCancelInputPrice(self):
        self.__inpDlgPrice.ITEM_INFO = {
            "VNUM": 0,
            "SLOT_TYPE": 0,
            "SLOT_INDEX": 0,
            "TARGET_SLOT_INDEX": 0,
        }
        self.__inpDlgPrice.SetValue(0)
        self.__inpDlgPrice.Hide()

    def __WithdrawGold(self):
        offshop.SendWithdrawGoldPacket(self.__gold)

    def __CloseShop(self):
        offshop.SendCloseMyShopPacket()
        self.__dlgAskClose.Hide()

    def __OnSelectItemSlot(self, slotIndex):
        if mouseModule.mouseController.isAttached():
            mouseModule.mouseController.DeattachObject()
            return

        if slotIndex not in self.__dictItemStock:
            return

        itemInfo = self.__dictItemStock[slotIndex]
        self.__dlgAskRemove.DISPLAY_SLOT_INDEX = slotIndex
        mouseModule.mouseController.AttachObject(
            self,
            player.SLOT_TYPE_OFFLINE_SHOP,
            slotIndex,
            itemInfo.VNUM,
            itemInfo.COUNT,
        )

    def __OnSelectEmptySlot(self, slotIndex):
        if not mouseModule.mouseController.isAttached():
            return

        attachedSlotType = mouseModule.mouseController.GetAttachedType()
        attachedSlotIndex = mouseModule.mouseController.GetAttachedSlotPosition()
        mouseModule.mouseController.DeattachObject()

        if attachedSlotType in [
            player.SLOT_TYPE_INVENTORY,
            player.SLOT_TYPE_DRAGON_SOUL_INVENTORY,
        ]:
            itemVnum = player.GetItemIndex(attachedSlotIndex[0], attachedSlotIndex[1])

            item.SelectItem(itemVnum)
            if item.IsAntiFlag(item.ANTIFLAG_GIVE) or item.IsAntiFlag(
                item.ANTIFLAG_MYSHOP
            ):
                chat.AppendChat(
                    CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_SELL_ITEM
                )
                return

            self.__inpDlgPrice.ITEM_INFO = {
                "VNUM": itemVnum,
                "SLOT_TYPE": attachedSlotIndex[0],
                "SLOT_INDEX": attachedSlotIndex[1],
                "TARGET_SLOT_INDEX": slotIndex,
            }
            self.__inpDlgPrice.SetValue(0)

            self.__inpDlgPrice.Open()
            self.__inpDlgPrice.SetTop()
            self.__inpDlgPrice.SetFocus()
        elif attachedSlotType == player.SLOT_TYPE_OFFLINE_SHOP:
            offshop.SendMoveItemPacket(attachedSlotIndex, slotIndex)

    def OnClickItemSlot(self, slotIndex, inventoryPosition=-1):
        if slotIndex not in self.__dictItemStock:
            return

        self.__dlgAskRemove.SetAcceptEvent(self.__RemoveItem)
        self.__dlgAskRemove.INVENTORY_INDEX = inventoryPosition
        self.__dlgAskRemove.DISPLAY_SLOT_INDEX = slotIndex
        self.__dlgAskRemove.Open()
        self.__dlgAskRemove.SetTop()
        self.__dlgAskRemove.SetFocus()

    def __RemoveItem(self):
        if self.__dlgAskRemove.DISPLAY_SLOT_INDEX == -1:
            return

        if self.__dlgAskRemove.INVENTORY_INDEX != -1:
            offshop.SendRemoveItemPacket(
                self.__dlgAskRemove.DISPLAY_SLOT_INDEX,
                player.INVENTORY,
                self.__dlgAskRemove.INVENTORY_INDEX,
            )
        else:
            offshop.SendRemoveItemPacket(self.__dlgAskRemove.DISPLAY_SLOT_INDEX, 0, 0)
        self.__dlgAskRemove.Hide()

    def __OnOverInItem(self, slotIndex):
        if slotIndex not in self.__dictItemStock:
            return

        self.__toolTipItem.SetOfflineShopItem(self.__dictItemStock[slotIndex])
        self.__toolTipItem.ShowToolTip()

    def __OnOverOutItem(self):
        self.__toolTipItem.HideToolTip()

    def __StartEditShopName(self):
        self.__txtShopName.SetText(self.__shopName)
        self.__txtShopName.SetEndPosition()
        self.__txtShopName.SetPackedFontColor(0xFF3DBDD4)

    def __SubmitEditShopName(self):
        shopName = self.__txtShopName.GetText()
        if shopName != self.__shopName:
            offshop.SendShopNamePacket(shopName)

        self.__CancelEditShopName()

    def __CancelEditShopName(self, onKillFocus=False):
        if not onKillFocus:
            self.__txtShopName.OnKillFocus()
        self.__txtShopName.SetPackedFontColor(0xFFFFFFFF)
        self.__txtShopName.SetText(self.__shopName[:16])
        if len(self.__shopName) > 16:
            self.__txtShopName.SetText(self.__txtShopName.GetText() + "...")

    def Hide(self, destroying=False):
        if not destroying:
            OfflineShopManager.GetInstance().ClearItemSlots()

        if self.IsShow():
            offshop.SendCloseOtherShopPacket()

        ui.Window.Hide(self)

    def Show(self):
        ui.Window.Show(self)

    def SetGold(self, gold):
        self.__gold = gold
        self.__txtShopGold.SetText(localeInfo.NumberToMoneyString(gold))

    def SetShopName(self, shopName):
        self.__shopName = shopName
        self.__txtShopName.SetText(self.__shopName[:16])
        if len(self.__shopName) > 16:
            self.__txtShopName.SetText(self.__txtShopName.GetText() + "...")

    def JumpNextCentering(self, jmp=True):
        self.__isJumpNextCentering = jmp

    def SetCenterPosition(self):
        if self.__isJumpNextCentering:
            self.__isJumpNextCentering = False
            return

        ui.ScriptWindow.SetCenterPosition(self)

    def RefreshItemGrid(self):
        for i in range(
            self.__gtItems.GetStartIndex(),
            self.__gtItems.GetStartIndex() + self.__gtItems.GetSlotCount(),
        ):
            if i in self.__dictItemStock:
                count = self.__dictItemStock[i].COUNT
                if count <= 1:
                    count = 0

                self.__gtItems.SetItemSlot(
                    i,
                    self.__dictItemStock[i].VNUM,
                    self.__dictItemStock[i].COUNT > 1
                    and self.__dictItemStock[i].COUNT
                    or 0,
                )

                if self.__dictItemStock[i].TRANS_VNUM == 0:
                    self.__gtItems.EnableSlotCoverImage(i, False)
                else:
                    self.__gtItems.SetSlotCoverImage(
                        i, "icon/item/ingame_convert_Mark.tga"
                    )

            else:
                self.__gtItems.ClearSlot(i)

        self.__gtItems.RefreshSlot()

        if len(self.__dictItemStock) > 0:
            self.SetSize(200 * 2, 475)
            self.__bBoard.SetSize(200 * 2, 475)
            self.__bBoard.SetTitleName(uiScriptLocale.PRIVATE_SHOP_EDIT_TITLE)
            self.background_border.SetSize(214 * 2 - 47, 450 - 17)

            self.__slotBarName.Show()

            self.__gtItems.Show()

            self.__txtGold.SetPosition(23, 387 - 10)
            self.__slotBarGold.SetPosition(21, 392)
            self.__btnWithdrawGold.SetPosition(205, 393)

            self.__txtOfflineShopTime.SetPosition(23, 415)
            self.__leftTimeGauge.SetPosition(21 + 4, 434)
            self.__leftTimeSlot.SetPosition(22, 438 - 10)
            self.__txtLeftTime.SetPosition(45, 434)
            self.__btnWarpShop.SetPosition(295, 393)

            self.__btnWarpShop.Show()
            self.__btnOpenShop.Hide()
            self.__btnCloseShop.Show()

        else:
            self.SetSize(200 * 2, 146)
            self.__bBoard.SetSize(200 * 2, 146)
            self.background_border.SetSize(214 * 2 - 47, 146 - 38)

            if self.__gold < 1:
                self.__bBoard.SetTitleName(uiScriptLocale.PRIVATE_SHOP_EMPTY)

            self.__slotBarName.Hide()

            self.__gtItems.Hide()

            self.__txtGold.SetPosition(23, 87 - 40)
            self.__slotBarGold.SetPosition(21, 104 - 40)
            self.__btnWithdrawGold.SetPosition(112 + 111, 104 - 40)

            self.__txtOfflineShopTime.SetPosition(23, 104 - 18)
            self.__leftTimeSlot.SetPosition(21, 103)
            self.__leftTimeGauge.SetPosition(22 + 4, 109)
            self.__txtLeftTime.SetPosition(43, 107)
            self.__btnOpenShop.SetPosition(245, 104)
            self.__btnWarpShop.Hide()
            self.__btnOpenShop.Show()
            self.__btnCloseShop.Hide()

    def SetLeftOpeningTime(self, farmedOpeningTime, specialOpeningTime):
        self.__leftTimeGauge.SetPercentage(
            farmedOpeningTime + specialOpeningTime, 2 * 24 * 60 * 60
        )
        self.__txtLeftTime.SetText(
            localeInfo.GetFormattedTimeString(farmedOpeningTime + specialOpeningTime)
        )

    def SetShopOpeningState(self, shopOpened):
        if shopOpened:
            self.__btnOpenShop.Hide()
            self.__btnCloseShop.SetText(uiScriptLocale.PRIVATE_SHOP_CLOSE_BUTTON)
            self.__btnCloseShop.SetEvent(self.__AskClose)
        else:
            self.__btnCloseShop.SetText(uiScriptLocale.PRIVATE_SHOP_OPEN_BUTTON)
            self.__btnCloseShop.SetEvent(offshop.SendReopenShopPacket)

    def SetShopPositionInfo(self, mapIndex, localX, localY, channel):
        mapName = localeInfo.GetMapNameByIndex(mapIndex)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

        return False


class _UIOfflineShopViewer(ui.ScriptWindow):
    def __init__(self, mgrInst):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__dictItemStock = OfflineShopManager.GetInstance().dictItemStock
        self.__buyState = False
        self.__interface = None

        self.__LoadInterface()

    def SetInterface(self, interface):
        self.__interface = interface

        self.__toolTipItem.BindInterface(interface)

    def __LoadInterface(self):
        ui.PythonScriptLoader().LoadScriptFile(self, "uiscript/offlineshopviewer.py")

        self.__dlgAskBuy = None
        self.__toolTipItem = uiToolTip.ItemToolTip()
        self.__toolTipItem.HideToolTip()

        self.__bBoard = self.GetChild("bBoard")
        self.__bBoard.SetCloseEvent(self.Hide)

        self.__gtItems = self.GetChild("gtItems")
        self.__gtItems.SetSelectItemSlotEvent(self.__OnSelectItemSlot)
        self.__gtItems.SetUseSlotEvent(self.OnClickItemSlot)
        self.__gtItems.SetUnselectItemSlotEvent(self.OnClickItemSlot)
        self.__gtItems.SetOverInItemEvent(self.__OnOverInItem)
        self.__gtItems.SetOverOutItemEvent(self.__OnOverOutItem)

        btnBuy = self.GetChild("btnBuy")
        btnBuy.SetToggleUpEvent(Event(self.__SetBuyState, False))
        btnBuy.SetToggleDownEvent(Event(self.__SetBuyState, True))

    def __OnSelectItemSlot(self, slotIndex):
        if self.__buyState:
            self.__AskBuy(slotIndex)

    def OnClickItemSlot(self, slotIndex, dstSlot=-1):
        self.__AskBuy(slotIndex, dstSlot)

    def __OnOverInItem(self, slotIndex):
        if slotIndex not in self.__dictItemStock:
            return

        self.__toolTipItem.SetOfflineShopItem(self.__dictItemStock[slotIndex])
        self.__toolTipItem.ShowToolTip()

    def __OnOverOutItem(self):
        self.__toolTipItem.HideToolTip()

    def __SetBuyState(self, buyState):
        self.__buyState = buyState

    def __AskBuy(self, slotIndex, dstSlot=-1):
        if slotIndex not in self.__dictItemStock:
            return

        itemInfo = self.__dictItemStock[slotIndex]

        proto = itemManager().GetProto(itemInfo.VNUM)
        if not proto:
            return

        ## Question Text
        questionText = localeInfo.DO_YOU_BUY_ITEM(
            proto.GetName(),
            itemInfo.COUNT,
            localeInfo.NumberToMoneyString(itemInfo.PRICE),
        )

        dlgAskBuy = uiCommon.ItemQuestionDialog("TOP_MOST")
        dlgAskBuy.SetMessage(questionText)

        dlgAskBuy.SetAcceptEvent(Event(self.RequestBuyItem, True))
        dlgAskBuy.SetCancelEvent(Event(self.RequestBuyItem, False))
        dlgAskBuy.slotIndex = slotIndex
        dlgAskBuy.dstSlot = dstSlot
        dlgAskBuy.Open(itemInfo.VNUM, sockets=itemInfo.SOCKETS)
        self.__dlgAskBuy = dlgAskBuy
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

    def CloseItemBuyDialog(self):
        if not self.__dlgAskBuy:
            return False

        self.__dlgAskBuy.Hide()
        self.__dlgAskBuy = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)
        return True

    def RequestBuyItem(self, answer):
        if not self.__dlgAskBuy:
            return

        if answer:
            slotIndex = self.__dlgAskBuy.slotIndex
            dstSlot = self.__dlgAskBuy.dstSlot
            if dstSlot != -1:
                offshop.SendBuyItemPacket(
                    slotIndex, player.SLOT_TYPE_INVENTORY, dstSlot
                )
            else:
                offshop.SendBuyItemPacket(slotIndex, player.SLOT_TYPE_NONE, dstSlot)

        self.CloseItemBuyDialog()

    def Hide(self, destroying=False):
        if not destroying:
            OfflineShopManager.GetInstance().ClearItemSlots()

        if self.IsShow():
            offshop.SendCloseOtherShopPacket()

        ui.Window.Hide(self)

    def Show(self):
        self.SetCenterPosition()
        ui.Window.Show(self)

    def RefreshItemGrid(self):
        for i in range(
            self.__gtItems.GetStartIndex(),
            self.__gtItems.GetStartIndex() + self.__gtItems.GetSlotCount(),
        ):
            if i in self.__dictItemStock:
                count = self.__dictItemStock[i].COUNT
                if count <= 1:
                    count = 0
                self.__gtItems.SetItemSlot(
                    i,
                    self.__dictItemStock[i].VNUM,
                    self.__dictItemStock[i].COUNT > 1
                    and self.__dictItemStock[i].COUNT
                    or 0,
                )

                if self.__dictItemStock[i].TRANS_VNUM == 0:
                    self.__gtItems.EnableSlotCoverImage(i, False)
                else:
                    self.__gtItems.SetSlotCoverImage(
                        i, "icon/item/ingame_convert_Mark.tga"
                    )

            else:
                self.__gtItems.ClearSlot(i)

        self.__gtItems.RefreshSlot()

    def SetTitle(self, shopName):
        if len(shopName) > 16:
            shopName = "%s..." % shopName[:16]
        self.__bBoard.SetTitleName(shopName)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

        return False
