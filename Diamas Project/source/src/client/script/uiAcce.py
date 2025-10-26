# -*- coding: utf-8 -*-
import logging

import app
import chat
import item
import player
import snd
from pygame.app import appInst
from pygame.item import MakeItemPosition

import localeInfo
import mouseModule
import ui
import uiCommon
from ui_event import Event

if app.ENABLE_ACCE_COSTUME_SYSTEM:
    class AcceWindow(ui.ScriptWindow):
        def __init__(self):
            ui.ScriptWindow.__init__(self)
            self.AddFlag("animated_board")

            self.type = 0
            self.isloded = 0
            self.tooltipitem = None
            self.xAcceWindowStart = 0
            self.yAcceWindowStart = 0

        def __LoadWindow(self, type):
            try:
                pyScrLoader = ui.PythonScriptLoader()
                if type == player.ACCE_SLOT_TYPE_COMBINE:
                    pyScrLoader.LoadScriptFile(self, "UIScript/Acce_CombineWindow.py")
                    self.cost = self.GetChild("Cost")
                else:
                    pyScrLoader.LoadScriptFile(self, "UIScript/Acce_AbsorbWindow.py")
            except:
                logging.exception("AcceWindow.__LoadWindow.UIScript/Acce_CombineWindow.py")
            try:
                wnditem = self.GetChild("AcceSlot")
                self.GetChild("CancelButton").SetEvent(self.Close)
                self.GetChild("AcceptButton").SetEvent(self.Accept)
                self.GetChild("TitleBar").SetCloseEvent(self.Close)
            except:
                logging.exception("AcceWindow.__LoadWindow.AcceSlot")

            wnditem.SetOverInItemEvent(self.OverInItem)
            wnditem.SetOverOutItemEvent(self.OverOutItem)
            wnditem.SetUnselectItemSlotEvent(self.UseItemSlot)
            wnditem.SetUseSlotEvent(self.UseItemSlot)
            wnditem.SetSelectEmptySlotEvent(self.SelectEmptySlot)
            wnditem.SetSelectItemSlotEvent(self.SelectItemSlot)
            wnditem.Show()

            self.wnditem = wnditem

        def Accept(self):
            if player.GetCurrentItemCount() == 3:
                appInst.instance().GetNet().SendAcceRefineAccept(self.type)
            else:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_INITEM)

        def Open(self, type):

            if self.isloded == 0:
                self.isloded = 1
                self.__LoadWindow(type)
                self.type = type
                self.SetCenterPosition()
                self.SetTop()
                ui.ScriptWindow.Show(self)
                player.SetAcceRefineWindowOpen(type)
                (self.xAcceWindowStart, self.yAcceWindowStart, z) = player.GetMainCharacterPosition()

        def Close(self):
            if player.IsAcceWindowEmpty() == 1:
                self.Hide()
                self.isloded = 0
                player.SetAcceRefineWindowOpen(self.type)
            else:
                self.Hide()
                self.isloded = 0
                player.SetAcceRefineWindowOpen(self.type)
                appInst.instance().GetNet().SendAcceRefineCheckOut(0)
                appInst.instance().GetNet().SendAcceRefineCheckOut(1)
            appInst.instance().GetNet().SendAcceRefineCancel()

        def SetItemToolTip(self, tooltip):
            self.tooltipitem = tooltip

        def __ShowToolTip(self, slotIndex):
            if self.tooltipitem:
                self.tooltipitem.SetAcceWindowItem(slotIndex)

        # ¾ÆÀÌÅÛ ÅøÆÁ º¸¿©ÁÖ±â
        def OverInItem(self, slotIndex):
            self.wnditem.SetUsableItem(False)
            self.__ShowToolTip(slotIndex)

        # ¾ÆÀÌÅÛ ÅøÆÁ °¨Ãß±â
        def OverOutItem(self):
            self.wnditem.SetUsableItem(False)
            if self.tooltipitem:
                self.tooltipitem.HideToolTip()

        # ÀÎº¥ -> ¾Ç»ç¼¼¸® Ã¢.
        def SelectEmptySlot(self, selectedSlotPos):
            if selectedSlotPos == (player.ACCE_SLOT_MAX - 1):
                return
            if mouseModule.mouseController.isAttached():

                attachedSlotType = mouseModule.mouseController.GetAttachedType()
                attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
                ItemVNum = player.GetItemIndex(attachedSlotPos)
                item.SelectItem(ItemVNum)

                if attachedSlotPos > player.EQUIPMENT_SLOT_START - 1:  ## ÀÎº¥Ã¢ ¾È¿¡ ÀÖ´Â °Í¸¸.
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_USINGITEM)
                    return

                if player.SLOT_TYPE_ACCE != attachedSlotType:
                    attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
                    if player.RESERVED_WINDOW == attachedInvenType:
                        return
                    possablecheckin = 0
                    # Á¶ÇÕÃ¢ÀÏ¶§
                    if player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_COMBINE:
                        if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                            if item.GetItemSubType() == item.COSTUME_TYPE_ACCE:

                                if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                                    socketInDrainValue = player.GetItemMetinSocket(attachedSlotPos, 0)
                                    if socketInDrainValue >= app.ACCE_MAX_DRAINRATE:
                                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_MAX_DRAINRATE)
                                        return

                                    usingSlot = player.FindActivedAcceSlot(attachedSlotPos)
                                    if player.FindUsingAcceSlot(usingSlot) == attachedSlotPos:
                                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_ALREADY_REGISTER)
                                        return
                                    possablecheckin = 1
                                else:
                                    if item.GetRefinedVnum() == 0:  ## Àü¼±µî±Þ ¾ÆÀÌÅÛÀº °É·¯³¿.
                                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_MAXGRADE)
                                        return
                                    else:
                                        usingSlot = player.FindActivedAcceSlot(attachedSlotPos)
                                        if player.FindUsingAcceSlot(usingSlot) == attachedSlotPos:
                                            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_ALREADY_REGISTER)
                                            return
                                        possablecheckin = 1
                            else:
                                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                                return
                        else:
                            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                            return

                    # Èí¼öÃ¢ÀÏ¶§ ¾Ç¼¼¼­¸®, ¾ÆÀÌÅÛ ±¸ºÐ
                    if player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_ABSORB:
                        if selectedSlotPos == player.ACCE_SLOT_LEFT:
                            if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                                if item.GetItemSubType() == item.COSTUME_TYPE_ACCE:
                                    if player.GetItemMetinSocket(attachedSlotPos, 1) == 0:
                                        possablecheckin = 1
                                else:
                                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                                    return
                            else:
                                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                                return

                        elif selectedSlotPos == player.ACCE_SLOT_RIGHT:
                            if item.GetItemType() == item.ITEM_TYPE_WEAPON:
                                possablecheckin = 1
                            elif item.GetItemType() == item.ITEM_TYPE_ARMOR:
                                if item.GetItemSubType() == item.ARMOR_BODY:
                                    possablecheckin = 1
                                else:
                                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCEITEM)
                                    return
                            else:
                                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCEITEM)
                                return

                            if localeInfo.IsBRAZIL():
                                ## ºê¶óÁú¿¡¼­ ¾Æ·¡ ³ª¿­µÈ ¾ÆÀÌÅÛÀº ´É·ÂÄ¡°¡ »ó´çÈ÷ °­ÇØ¼­ Èí¼ö ¾ÈµÇ°Ô ÇØ´Þ¶ó°í Á¶¸§.
                                ## ¾îÂ¿¼ö ¾øÀÌ ÇÏµåÄÚµùÇÔ.
                                itemvnum = item.GetVnum()
                                if itemvnum == 11979 or itemvnum == 11980 or itemvnum == 11981 or itemvnum == 11982 or itemvnum == 11971 or itemvnum == 11972 or itemvnum == 11973 or itemvnum == 11974:
                                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_DONOT_ABSORDITEM)
                                    return

                    if possablecheckin:
                        ## ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿
                        if player.GetItemSealDate(player.INVENTORY, attachedSlotPos) == -1:
                            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_SEALITEM)
                            return

                        if player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_COMBINE:
                            player.SetAcceActivedItemSlot(selectedSlotPos, attachedSlotPos)
                            appInst.instance().GetNet().SendAcceRefineCheckIn(MakeItemPosition(attachedInvenType, attachedSlotPos), selectedSlotPos, self.type)
                        elif player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_ABSORB:

                            if selectedSlotPos == player.ACCE_SLOT_RIGHT:
                                popup = uiCommon.QuestionDialog()
                                popup.SetText(localeInfo.ACCE_DEL_ABSORDITEM)

                                popup.SetAcceptEvent(Event(self.OnAcceAcceptEvent, attachedInvenType, attachedSlotPos, selectedSlotPos))
                                popup.SetCancelEvent(self.OnAcceCloseEvent)
                                popup.SetTop()
                                popup.Open()
                                self.pop = popup
                            else:
                                player.SetAcceActivedItemSlot(selectedSlotPos, attachedSlotPos)
                                appInst.instance().GetNet().SendAcceRefineCheckIn(MakeItemPosition(attachedInvenType, attachedSlotPos), selectedSlotPos,
                                                          self.type)

                        snd.PlaySound("sound/ui/drop.wav")

                ## °æ°í ¸Þ½ÃÁö ¶ç¿ì±â.
                if not player.FindUsingAcceSlot(
                        player.ACCE_SLOT_RIGHT) == player.ITEM_SLOT_COUNT and not player.FindUsingAcceSlot(
                        player.ACCE_SLOT_LEFT) == player.ITEM_SLOT_COUNT:
                    if selectedSlotPos != player.ACCE_SLOT_MAX:
                        popup = uiCommon.PopupDialog()
                        if player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_COMBINE:

                            if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                                socketInDrainValue = player.GetAcceItemMetinSocket(0, 0)
                                socketInDrainValue2 = player.GetAcceItemMetinSocket(1, 0)
                                socketInDrainValue3 = player.GetItemMetinSocket(attachedSlotPos, 0)
                                ## ¸ÞÀÎ ¼­¹ö Áß. µî·ÏµÈ ¾ÆÀÌÅÛÀÌ Àü¼³ÀÏ¶§ °æ°í ¸Þ½ÃÁö º¯°æ.
                                if socketInDrainValue > 0 or socketInDrainValue2 > 0 or socketInDrainValue3 > 0:
                                    popup.SetText(localeInfo.ACCE_DEL_SERVEITEM2)
                                else:
                                    popup.SetText(localeInfo.ACCE_DEL_SERVEITEM)
                            else:
                                popup.SetText(localeInfo.ACCE_DEL_SERVEITEM)

                            popup.SetAcceptEvent(self.__OnClosePopupDialog)
                            popup.SetTop()
                            popup.Open()
                            self.popup = popup

                mouseModule.mouseController.DeattachObject()

        ## ¾ÆÀÌÅÛ Èí¼ö½Ã Èí¼öµÉ ¾ÆÀÌÅÛ ÇÒÁö ¾ÈÇÒÁö ¼±ÅÃ ÆË¾÷
        if app.ENABLE_ACCE_COSTUME_SYSTEM:
            def OnAcceAcceptEvent(self, attachedInvenType, attachedSlotPos, selectedSlotPos):
                self.pop.Close()
                self.pop = None
                player.SetAcceActivedItemSlot(selectedSlotPos, attachedSlotPos)
                appInst.instance().GetNet().SendAcceRefineCheckIn(MakeItemPosition(attachedInvenType, attachedSlotPos), selectedSlotPos, self.type)

            def OnAcceCloseEvent(self):
                self.pop.Close()
                self.pop = None

        def UseItemSlot(self, slotIndex):

            if slotIndex == (player.ACCE_SLOT_MAX - 1):
                return

            mouseModule.mouseController.DeattachObject()
            appInst.instance().GetNet().SendAcceRefineCheckOut(slotIndex)

        def SelectItemSlot(self, selectedSlotPos):
            if selectedSlotPos == (player.ACCE_SLOT_MAX - 1):
                return

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
                    selectedItemID = player.GetAcceItemID(selectedSlotPos)
                    mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_ACCE, selectedSlotPos,
                                                             selectedItemID)
                    snd.PlaySound("sound/ui/pick.wav")

        def RefreshAcceWindow(self):
            getAcceItem = player.GetAcceItemID
            setAcceItem = self.wnditem.SetItemSlot
            AcceItemSize = player.GetAcceItemSize()

            for i in xrange(AcceItemSize):
                setAcceItem(i, getAcceItem(i), 1)
                if self.type == player.ACCE_SLOT_TYPE_COMBINE:

                    if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                        if i == player.ACCE_SLOT_LEFT:
                            if getAcceItem(i) != 0:
                                item.SelectItem(getAcceItem(i))
                                self.cost.SetText(localeInfo.ACCE_ABSORB_COST.format(item.GetIBuyItemPrice()))
                            else:
                                self.cost.SetText("")
                    else:
                        if i == player.ACCE_SLOT_MAX - player.ACCE_SLOT_MAX:
                            if getAcceItem(i) != 0:
                                item.SelectItem(getAcceItem(i))
                                self.cost.SetText(localeInfo.ACCE_ABSORB_COST.format(item.GetIBuyItemPrice()))
                            else:
                                self.cost.SetText("")

                    if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                        if i == player.ACCE_SLOT_RIGHT:
                            if getAcceItem(i) != 0:
                                item.SelectItem(getAcceItem(i))
                                if item.GetRefinedVnum() == 0:
                                    self.cost.SetText(localeInfo.ACCE_ABSORB_COST.format(item.GetIBuyItemPrice()))

                if app.ENABLE_CHANGE_LOOK_SYSTEM:
                    if self.type == player.ACCE_SLOT_TYPE_ABSORB:
                        changelookvnum = player.GetAcceItemTransmutationID(i)
                        if not changelookvnum == 0:
                            self.wnditem.SetSlotCoverImage(i, "icon/item/ingame_convert_Mark.tga")
                        else:
                            self.wnditem.EnableSlotCoverImage(i, False)

            self.wnditem.RefreshSlot()

        def __OnClosePopupDialog(self):
            self.popup = None

        def OnUpdate(self):
            USE_ACCEWINDOW_LIMIT_RANGE = 500
            (x, y, z) = player.GetMainCharacterPosition()
            if abs(x - self.xAcceWindowStart) > USE_ACCEWINDOW_LIMIT_RANGE or abs(
                            y - self.yAcceWindowStart) > USE_ACCEWINDOW_LIMIT_RANGE:
                self.RemoveFlag("animated_board")
                self.Close()
                self.AddFlag("animated_board")

        def OnKeyDown(self, key):
            if key == app.VK_ESCAPE:
                self.Close()
                return True
            elif key == app.VK_RETURN:
                self.Accept()
                return True
