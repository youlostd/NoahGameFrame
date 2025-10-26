# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import app
import chat
import player
import snd
import item
from pygame.app import appInst
from pygame.item import IsPossibleChangeLookLeft
from pygame.item import IsPossibleChangeLookRight
from pygame.item import MakeItemPosition
from pygame.item import itemManager
from pygame.player import SetItemSlot
from pygame.player import playerInst

import localeInfo
import mouseModule
import ui
import uiCommon
import uiToolTip


class ChangeLookWindow(ui.ScriptWindow):
    USE_CHANGELOOKWINDOW_LIMIT_RANGE = 500
    CHANGELOOK_SLOT_LEFT = 0
    CHANGELOOK_SLOT_RIGHT = 1
    CHANGELOOK_SLOT_MAX = 2
    CHANGELOOK_COST = 50000000

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.isloded = 0
        self.tooltipitem = None
        self.wnditem = None
        self.wndpassitem = None
        self.xChangeLookWindowStart = 0
        self.yChangeLookWindowStart = 0
        self.ChangeLookToolTIpButton = None
        self.ChangeLookToolTip = None
        self.pop = None
        self.ChangeLookCost = None
        self.ChangeLookToolTipList = [localeInfo.CHANGE_TOOLTIP_LINE1,
                                      localeInfo.CHANGE_TOOLTIP_LINE2,
                                      localeInfo.CHANGE_TOOLTIP_LINE3,
                                      localeInfo.CHANGE_TOOLTIP_LINE4,
                                      localeInfo.CHANGE_TOOLTIP_LINE5,
                                      localeInfo.CHANGE_TOOLTIP_LINE6]

    def __LoadWindow(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/ChangeLookWindow.py")
        except Exception as e:
            logging.exception(e)

        try:
            wnditem = self.GetChild("ChangeLookSlot")
            wndpassitem = self.GetChild("ChangeLookSlot_PassYangItem")
            self.GetChild("CancelButton").SetEvent(self.Close)
            self.GetChild("AcceptButton").SetEvent(self.Accept)
            self.GetChild("TitleBar").SetCloseEvent(self.Close)
            self.ChangeLookCost = self.GetChild("Cost")
            self.ChangeLookCost.SetText(
                localeInfo.CHANGE_LOOK_COST.format(localeInfo.MoneyFormat(self.CHANGELOOK_COST)))

            self.GetChild("TitleBar").SetTitleName(localeInfo.CHANGE_LOOK_TITLE)
        except Exception as e:
            logging.exception(e)

        wnditem.SetOverInItemEvent(self.OverInItem)
        wnditem.SetOverOutItemEvent(self.OverOutItem)
        wnditem.SetUnselectItemSlotEvent(self.UseItemSlot)
        wnditem.SetUseSlotEvent(self.UseItemSlot)
        wnditem.SetSelectEmptySlotEvent(self.SelectEmptySlot)
        wnditem.SetSelectItemSlotEvent(self.SelectItemSlot)
        wnditem.Show()

        wndpassitem.SetOverInItemEvent(self.OverInItemFreeYang)
        wndpassitem.SetOverOutItemEvent(self.OverOutItem)
        wndpassitem.SetUnselectItemSlotEvent(self.UseItemSlotFreepass)
        wndpassitem.SetUseSlotEvent(self.UseItemSlotFreepass)
        wndpassitem.SetSelectEmptySlotEvent(self.SelectEmptySlotFreepass)
        wndpassitem.Show()
        self.wndpassitem = wndpassitem

        self.wnditem = wnditem

        self.ChangeLookToolTIpButton = self.GetChild("ChangeLookToolTIpButton")
        self.ChangeLookToolTip = self.__CreateGameTypeToolTip(localeInfo.CHANGE_TOOLTIP_TITLE,
                                                              self.ChangeLookToolTipList)
        self.ChangeLookToolTip.SetTop()
        self.ChangeLookToolTIpButton.SetToolTipWindow(self.ChangeLookToolTip)

    def __CreateGameTypeToolTip(self, title, descList):
        toolTip = uiToolTip.ToolTip()
        toolTip.SetTitle(title)
        toolTip.AppendSpace(7)

        for desc in descList:
            toolTip.AppendTextLine(desc)

        toolTip.SetTop()
        return toolTip

    def OnUpdate(self):
        (x, y, z) = playerInst().GetMainCharacterPosition()
        if abs(x - self.xChangeLookWindowStart) > self.USE_CHANGELOOKWINDOW_LIMIT_RANGE or abs(
                        y - self.yChangeLookWindowStart) > self.USE_CHANGELOOKWINDOW_LIMIT_RANGE:
            self.RemoveFlag("animated_board")
            self.Close()
            self.AddFlag("animated_board")

    def Open(self):
        if self.isloded == 0:
            self.isloded = 1
            self.__LoadWindow()

        self.SetCenterPosition()
        self.SetTop()
        self.SetFocus()
        ui.ScriptWindow.Show(self)
        (self.xChangeLookWindowStart, self.yChangeLookWindowStart, z) = playerInst().GetMainCharacterPosition()
        playerInst().SetChangeLookWindowOpen(True)
        self.RefreshChangeLookWindow()

    def Close(self):
        self.Hide()
        appInst.instance().GetNet().SendChangeLookCancelPacket()
        playerInst().SetChangeLookWindowOpen(False)

    def Accept(self):
        leftvnum = playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_LEFT).vnum
        rightvnum = playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_RIGHT).vnum
        if leftvnum == 0 or rightvnum == 0:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_INSERT_ITEM)
        else:
            popup = uiCommon.QuestionDialog()
            popup.SetText(localeInfo.CHANGE_LOOK_CHANGE_ITEM)
            popup.SetAcceptEvent(self.SendAccept)
            popup.SetCancelEvent(self.OnCloseEvent)
            popup.Open()
            self.pop = popup

    def SendAccept(self):
        self.pop.Close()
        self.pop = None
        appInst.instance().GetNet().SendChangeLookAcceptPacket()

    def SetItemToolTip(self, tooltip):
        self.tooltipitem = tooltip

    def __ShowToolTip(self, slotIndex):
        if self.tooltipitem:
            self.tooltipitem.SetChangeLookWindowItem(slotIndex)

    # ¾ÆÀÌÅÛ ÅøÆÁ º¸¿©ÁÖ±â
    def OverInItem(self, slotIndex):
        self.wnditem.SetUsableItem(False)
        self.__ShowToolTip(slotIndex)

    #
    ## ¾ÆÀÌÅÛ ÅøÆÁ °¨Ãß±â
    def OverOutItem(self):
        self.wnditem.SetUsableItem(False)
        if self.tooltipitem:
            self.tooltipitem.HideToolTip()

    def OverInItemFreeYang(self, slotIndex):
        self.wnditem.SetUsableItem(False)
        self.__ShowToolTip_FreeItem(player.GetChangeLookFreeYangInvenSlotPos())

    def __ShowToolTip_FreeItem(self, slotIndex):
        if self.tooltipitem:
            self.tooltipitem.SetInventoryItem(slotIndex, player.INVENTORY)

    ## ¿ÜÇüº¯°æ -> ÀÎº¥ (¾ç ÆÐ½º ¾ÆÀÌÅÛ)
    def UseItemSlotFreepass(self, slotIndex):
        mouseModule.mouseController.DeattachObject()
        appInst.instance().GetNet().SendChangeLookCheckoutPacket(2)
        self.ChangeLookCost.SetText(
            localeInfo.CHANGE_LOOK_COST.format(localeInfo.MoneyFormat(self.CHANGELOOK_COST)))

    ## ÀÎº¥ -> ¿ÜÇüº¯°æ (¾ç ÆÐ½º ¾ÆÀÌÅÛ)
    def SelectEmptySlotFreepass(self, selectedSlotPos):
        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            if attachedSlotType != player.SLOT_TYPE_INVENTORY:
                return

            attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
            ItemVNum = playerInst().GetItemIndex(MakeItemPosition(attachedSlotPos))

            ## ¾ÆÀÌÅÛ Ã¼Å©.
            if ItemVNum not in (72326, 72344, 72341):
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_REGISTER_ITEM)
                return

            appInst.instance().GetNet().SendChangeLookCheckinPacket(MakeItemPosition(attachedSlotPos), 2)
            mouseModule.mouseController.DeattachObject()
            self.ChangeLookCost.SetText(localeInfo.CHANGE_LOOK_COST.format(localeInfo.MoneyFormat(0)))

    ## ÀÎº¥ -> ¿ÜÇüº¯°æ Ã¢.
    def SelectEmptySlot(self, selectedSlotPos):

        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            if attachedSlotType != player.SLOT_TYPE_INVENTORY:
                return

            ItemVNum = playerInst().GetItemIndex(MakeItemPosition(attachedSlotPos))
            attachedSlot = MakeItemPosition(attachedSlotPos)
            attachedProto = itemManager().GetProto(ItemVNum)

            if attachedProto:
                if attachedProto.IsAntiFlag(item.ITEM_ANTIFLAG_CHANGELOOK):
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_CHANGE_LOOK_ITEM)
                    return

            ## ¾ÆÀÌÅÛ Ã¼Å©.
            if selectedSlotPos == self.CHANGELOOK_SLOT_LEFT:
                if IsPossibleChangeLookLeft(1, ItemVNum) == 0:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_CHANGE_LOOK_ITEM)
                    return

                itemData = playerInst().GetItemData(attachedSlot)
                if itemData and itemData.transVnum != 0:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_CHANGE_LOOK_ITEM)
                    return


            else:
                if not playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_LEFT):
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_INSERT_CHANGE_LOOK_ITEM)
                    return
                if IsPossibleChangeLookRight(1, playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_LEFT).vnum, ItemVNum) == 0:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_REGISTER_ITEM)
                    return
                if player.GetChangeLookVnum(*attachedSlotPos) != 0:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_CHANGE_LOOK_ITEM)
                    return

            ## ¾ÆÀÌÅÛ Ã¼Å©.
            proto = itemManager().GetProto(ItemVNum)
            if not proto:
                return

            if attachedSlot.windowType == player.EQUIPMENT:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_EQUIP_ITEM)
                return

            if player.SLOT_TYPE_CHANGE_LOOK != attachedSlotType:
                attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
                if player.RESERVED_WINDOW == attachedInvenType:
                    return

                if selectedSlotPos == self.CHANGELOOK_SLOT_LEFT:
                    appInst.instance().GetNet().SendChangeLookCheckinPacket(MakeItemPosition(attachedSlotPos), selectedSlotPos)
                else:
                    if player.GetItemSealDate(*attachedSlotPos) == -1:
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_DO_NOT_SEAL_ITEM)
                        return

                    if playerInst().GetChangeLookItemInvenSlot(self.CHANGELOOK_SLOT_LEFT) == attachedSlotPos:
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_ALREADY_REGISTER)
                        return

                    appInst.instance().GetNet().SendChangeLookCheckinPacket(MakeItemPosition(attachedSlotPos), selectedSlotPos)
                    popup = uiCommon.PopupDialog()
                    popup.SetText(localeInfo.CHANGE_LOOK_DEL_ITEM)
                    popup.SetAcceptEvent(self.__OnClosePopupDialog)
                    popup.Open()
                    self.pop = popup

            mouseModule.mouseController.DeattachObject()

    ## ¾ÆÀÌÅÛ »ç¿ë ½Ã ¾ø¾Ö±â
    def UseItemSlot(self, slotIndex):
        if slotIndex == self.CHANGELOOK_SLOT_LEFT:
            if playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_RIGHT) and playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_RIGHT).vnum == 0:
                mouseModule.mouseController.DeattachObject()
                appInst.instance().GetNet().SendChangeLookCheckoutPacket(slotIndex)
            else:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_CHECK_OUT_REGISTER_ITEM)
        else:
            mouseModule.mouseController.DeattachObject()
            appInst.instance().GetNet().SendChangeLookCheckoutPacket(slotIndex)

    ## ¾ÆÀÌÅÛ Å¬¸¯
    def SelectItemSlot(self, selectedSlotPos):

        if mouseModule.mouseController.isAttached():
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
                snd.PlaySound("sound/ui/drop.wav")
            mouseModule.mouseController.DeattachObject()
        else:
            curCursorNum = app.GetCursor()
            if app.SELL == curCursorNum:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SAFEBOX_SELL_DISABLE_SAFEITEM)
            elif app.BUY == curCursorNum:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)
            else:
                if selectedSlotPos == self.CHANGELOOK_SLOT_LEFT:
                    if playerInst().GetChangeLookData(self.CHANGELOOK_SLOT_RIGHT).vnum == 0:
                        selectedItemID = playerInst().GetChangeLookData(selectedSlotPos).vnum
                        mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_CHANGE_LOOK, selectedSlotPos,
                                                                 selectedItemID)
                        snd.PlaySound("sound/ui/pick.wav")
                    else:
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_CHECK_OUT_REGISTER_ITEM)
                else:
                    selectedItemID = playerInst().GetChangeLookData(selectedSlotPos).vnum
                    mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_CHANGE_LOOK, selectedSlotPos,
                                                             selectedItemID)
                    snd.PlaySound("sound/ui/pick.wav")

    ## °»½Å
    def RefreshChangeLookWindow(self):
        if not self.wnditem:
            return

        if app.ENABLE_CHANGE_LOOK_ITEM_SYSTEM:
            if not playerInst().GetChangeLookWindowOpen():
                return

        for i in xrange(self.CHANGELOOK_SLOT_MAX):
            data = playerInst().GetChangeLookData(i)
            if data:
                SetItemSlot(self.wnditem.hWnd, i, data)

        self.wnditem.RefreshSlot()

    def OnCloseEvent(self):
        self.pop.Close()
        self.pop = None

    def __OnClosePopupDialog(self):
        self.popup = None