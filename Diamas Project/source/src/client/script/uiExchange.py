# -*- coding: utf-8 -*-


from datetime import datetime

import app
import chat
import chr
import exchange
import item
import player
from pygame.app import appInst
from pygame.exchange import exchangeInst
from pygame.item import MakeItemPosition
from pygame.player import SetItemSlot
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
import uiCommon
import uiPickMoney
from playerSettingModule import RACE_WARRIOR_M, RACE_WARRIOR_W, RACE_ASSASSIN_M, RACE_ASSASSIN_W, RACE_SURA_M, \
    RACE_SURA_W, RACE_SHAMAN_M, RACE_SHAMAN_W, RACE_WOLFMAN_M

FACE_IMAGE_DICT = {
    RACE_WARRIOR_M: "icon/face/warrior_m.tga",
    RACE_WARRIOR_W: "icon/face/warrior_w.tga",
    RACE_ASSASSIN_M: "icon/face/assassin_m.tga",
    RACE_ASSASSIN_W: "icon/face/assassin_w.tga",
    RACE_SURA_M: "icon/face/sura_m.tga",
    RACE_SURA_W: "icon/face/sura_w.tga",
    RACE_SHAMAN_M: "icon/face/shaman_m.tga",
    RACE_SHAMAN_W: "icon/face/shaman_w.tga",
    RACE_WOLFMAN_M: "icon/face/wolfman_m.tga",
}

HIGHLIGHT_SLOT = "d:/ymir work/ui/public/slot_highlight.sub"
BASE_VISUALS_ROUTE = "d:/ymir work/ui/game/exchange/"


class ExchangeDialog(ui.ScriptWindow):
    class TextRenderer(ui.Window):
        def OnRender(self):
            (x, y) = self.GetGlobalPosition()
            #chat.RenderBox("$EXCHANGE_CHAT$", x, y - 3)

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.TitleName = None
        self.tooltipItem = None
        self.xStart = 0
        self.yStart = 0
        self.questionDialog = None
        self.interface = None
        self.board = None
        self.Character_Name = None
        self.Character_Level = None
        self.TargetName = None
        self.TargetLevel = None
        self.TargetFriendIndicator = None
        self.TargetSlot = None
        self.TargetMoney = None
        self.TargetOverlay = None
        self.OwnerMoney = None
        self.OwnerMoneyButton = None
        self.OwnerOverlay = None
        self.OwnerSlot = None
        self.dlgPickMoney = None
        self.ReadyButton = None
        self.ReadyButton2 = None

    def LoadDialog(self):
        PythonScriptLoader = ui.PythonScriptLoader()
        PythonScriptLoader.LoadScriptFile(self, "UIScript/exchangedialog.py")

        ## Owner
        self.OwnerSlot = self.GetChild("Owner_Slot")
        self.OwnerSlot.SetSelectEmptySlotEvent(self.SelectOwnerEmptySlot)
        self.OwnerSlot.SetSelectItemSlotEvent(self.SelectOwnerItemSlot)
        self.OwnerSlot.SetOverInItemEvent(self.OverInOwnerItem)
        self.OwnerSlot.SetOverOutItemEvent(self.OverOutItem)
        self.OwnerSlot.SetUnselectItemSlotEvent(self.RightClickItemSlot)

        self.OwnerMoney = self.GetChild("Owner_Money_Value")
        self.OwnerMoneyButton = self.GetChild("Owner_Money")
        self.OwnerMoneyButton.SetEvent(self.OpenPickMoneyDialog)
        self.OwnerOverlay = self.GetChild("Owner_Overlay")
        self.OwnerOverlay.Hide()

        ## Target
        self.TargetSlot = self.GetChild("Target_Slot")
        self.TargetSlot.SetOverInItemEvent(self.OverInTargetItem)
        self.TargetSlot.SetOverOutItemEvent(self.OverOutItem)
        self.TargetMoney = self.GetChild("Target_Money_Value")
        self.Character_Name = self.GetChild("Character_Name")
        self.Character_Level = self.GetChild("Character_Level")
        self.TargetName = self.GetChild("Target_Name")
        self.TargetLevel = self.GetChild("Target_Level")
        self.TargetOverlay = self.GetChild("Target_Overlay")
        self.TargetOverlay.Hide()

        ## PickMoneyDialog
        dlgPickMoney = uiPickMoney.PickMoneyDialog()
        dlgPickMoney.LoadDialog()
        dlgPickMoney.SetAcceptEvent(self.OnPickMoney)
        dlgPickMoney.SetTitleName(localeInfo.EXCHANGE_MONEY)
        dlgPickMoney.SetMax(14)
        dlgPickMoney.SetZeroAllowed(True)
        dlgPickMoney.Hide()
        self.dlgPickMoney = dlgPickMoney

        ## Button
        self.ReadyButton = self.GetChild("Middle_Exchange_Button")
        self.ReadyButton.SetEvent(self.ReadyExchange)
        self.ReadyButton2 = self.GetChild("Middle_Exchange_Button2")
        self.ReadyButton2.SetEvent(self.ReadyExchange)

        self.board = self.GetChild("board")
        self.board.SetCloseEvent(appInst.instance().GetNet().SendExchangeExitPacket)

    def Destroy(self):
        self.ClearDictionary()
        self.dlgPickMoney.Destroy()
        self.TitleName = None
        self.tooltipItem = None
        self.xStart = 0
        self.yStart = 0
        self.questionDialog = None
        self.interface = None
        self.board = None
        self.Character_Name = None
        self.Character_Level = None
        self.TargetName = None
        self.TargetLevel = None
        self.TargetFriendIndicator = None
        self.TargetSlot = None
        self.TargetMoney = None
        self.TargetOverlay = None
        self.OwnerMoney = None
        self.OwnerMoneyButton = None
        self.OwnerOverlay = None
        self.OwnerSlot = None
        self.dlgPickMoney = None
        self.ReadyButton = None
        self.ReadyButton2 = None

    def OpenDialog(self):
        self.AddFlag("animated_board")

        targetVID = exchange.GetTargetVID()

        self.TargetName.SetText(chr.GetNameByVID(targetVID))
        self.Character_Name.SetText(player.GetName())
        self.TargetLevel.SetText("Lv. {:d}".format(chr.GetLevelByVID(targetVID)))
        self.Character_Level.SetText("Lv. {:G}".format(playerInst().GetPoint(player.LEVEL)))

        self.SetTop()
        self.board.SetTop()
        self.Show()
        self.SetFocus()

        (self.xStart, self.yStart, z) = player.GetMainCharacterPosition()

    def CloseDialog(self):
        if app.WJ_ENABLE_TRADABLE_ICON:
            self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
            self.interface.RefreshMarkInventoryBag()

        if 0 != self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.dlgPickMoney.Close()
        self.Hide()

    def AppendInformation(self, unixTime, info, error=False):
        if not error:
            chatType = WHISPER_TYPE_NORMAL
        else:
            chatType = WHISPER_TYPE_SYSTEM

        time = datetime.fromtimestamp(unixTime).timetuple()

        #chat.AppendToBox(chatType, "$EXCHANGE_CHAT$", "[" + strftime("%H:%M:%S", time) + "] " + info)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def OpenPickMoneyDialog(self):
        if exchange.GetElkFromSelf() > 0:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.EXCHANGE_CANT_EDIT_MONEY)
            return
        self.dlgPickMoney.Open(long(player.GetElk()))

    def OnPickMoney(self, money):
        appInst.instance().GetNet().SendExchangeElkAddPacket(long(money))

    def ReadyExchange(self):
        emptyslots = 0;
        emptyslots_self = 0;
        for i in xrange(exchange.EXCHANGE_ITEM_MAX_NUM):
            itemVnum_self = exchange.GetItemVnumFromSelf(i)
            itemVnum = exchange.GetItemVnumFromTarget(i)
            if itemVnum == 0:
                emptyslots += 1
            if itemVnum_self == 0:
                emptyslots_self += 1

        if emptyslots_self != exchange.EXCHANGE_ITEM_MAX_NUM:
            if emptyslots == exchange.EXCHANGE_ITEM_MAX_NUM and exchange.GetElkFromTarget() <= 10000:
                self.questionDialog = uiCommon.QuestionDialog3()
                self.questionDialog.SetText1(localeInfo.TRADE_FOR_NOTHING1)
                self.questionDialog.SetText2(localeInfo.TRADE_FOR_NOTHING2)
                self.questionDialog.SetText3(localeInfo.TRADE_FOR_NOTHING3)
                self.questionDialog.SetAcceptEvent(self.DoExchange)
                self.questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
                self.questionDialog.Open()
            else:
                self.DoExchange()
        else:
            self.DoExchange()

    def DoExchange(self):
        appInst.instance().GetNet().SendExchangeAcceptPacket()
        self.OnCloseQuestionDialog()

    def OnCloseQuestionDialog(self):
        if not self.questionDialog:
            return

        self.questionDialog.Close()
        self.questionDialog = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    def SelectOwnerEmptySlot(self, SlotIndex):

        if False == mouseModule.mouseController.isAttached():
            return

        if player.GetAcceRefineWindowOpen() == 1:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_WINDOWOPEN)
            return

        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            if player.GetChangeLookWindowOpen() == 1:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_OPEN_OTHER_WINDOW)
                return

        if mouseModule.mouseController.IsAttachedMoney():
            appInst.instance().GetNet().SendExchangeElkAddPacket(long(mouseModule.mouseController.GetAttachedMoneyAmount()))
        else:
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            if (player.SLOT_TYPE_INVENTORY == attachedSlotType
                or player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedSlotType):

                attachedInvenType = mouseModule.mouseController.GetAttachedSlotPosition()[0]
                SrcSlotNumber = mouseModule.mouseController.GetRealAttachedSlotNumber()
                DstSlotNumber = SlotIndex

                itemID = player.GetItemIndex(attachedInvenType, SrcSlotNumber)
                item.SelectItem(itemID)

                if item.IsAntiFlag(item.ANTIFLAG_GIVE):
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.EXCHANGE_CANNOT_GIVE)
                    mouseModule.mouseController.DeattachObject()
                    return

                appInst.instance().GetNet().SendExchangeItemAddPacket(MakeItemPosition(attachedInvenType, SrcSlotNumber), DstSlotNumber)

        mouseModule.mouseController.DeattachObject()

    def RightClickItemSlot(self, SlotIndex):
        appInst.instance().GetNet().SendExchangeItemDelPacket(SlotIndex)

    def SelectOwnerItemSlot(self, SlotIndex):
        if player.ITEM_MONEY == mouseModule.mouseController.GetAttachedItemIndex():
            money = mouseModule.mouseController.GetAttachedItemCount()
            appInst.instance().GetNet().SendExchangeElkAddPacket(long(money))

    def HighlightSlots(self, isSelf):
        if isSelf:
            slotWindow = self.OwnerSlot
            getItemVnumFunc = exchange.GetItemVnumFromSelf
        else:
            slotWindow = self.TargetSlot
            getItemVnumFunc = exchange.GetItemVnumFromTarget

        for slotIndex in xrange(exchange.EXCHANGE_ITEM_MAX_NUM):
            itemVnum = getItemVnumFunc(slotIndex)
            if itemVnum < 1:
                continue

            item.SelectItem(itemVnum)
            itemType = item.GetItemType()
            if itemType != item.ITEM_TYPE_WEAPON and itemType != item.ITEM_TYPE_ARMOR:
                continue

            _, itemHeight = item.GetItemSize()

    def RefreshOwnerSlot(self):
        for i in xrange(exchange.EXCHANGE_ITEM_MAX_NUM):
            itemData = exchangeInst().GetItemDataFromSelf(i)
            SetItemSlot(self.OwnerSlot.hWnd, i, itemData)

        # See if any item needs highlighting
        self.HighlightSlots(True)
        self.OwnerSlot.RefreshSlot()

    def RefreshTargetSlot(self):
        for i in xrange(exchange.EXCHANGE_ITEM_MAX_NUM):
            itemData = exchangeInst().GetItemDataFromTarget(i)
            SetItemSlot(self.TargetSlot.hWnd, i, itemData)

        # See if the any item needs highlighting
        self.HighlightSlots(False)
        self.TargetSlot.RefreshSlot()

    def Refresh(self):
        self.RefreshOwnerSlot()
        self.RefreshTargetSlot()

        self.OwnerMoney.SetText(localeInfo.MoneyFormat(exchange.GetElkFromSelf()))
        self.TargetMoney.SetText(localeInfo.MoneyFormat(exchange.GetElkFromTarget()))

        if exchange.GetAcceptFromSelf():
            self.OwnerOverlay.Show()

            self.ReadyButton.SetUpVisual("d:/ymir work/ui/public/middle_button_03.sub");
            self.ReadyButton.SetDownVisual("d:/ymir work/ui/public/middle_button_03.sub");
            self.ReadyButton.SetOverVisual("d:/ymir work/ui/public/middle_button_03.sub");
        else:
            self.OwnerOverlay.Hide()

        if exchange.GetAcceptFromTarget():
            self.TargetOverlay.Show()

            self.ReadyButton2.SetUpVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton2.SetDownVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton2.SetOverVisual("d:/ymir work/ui/public/middle_button_01.sub");
        else:
            self.TargetOverlay.Hide()

        if not exchange.GetAcceptFromTarget():
            self.ReadyButton2.SetUpVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton2.SetDownVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton2.SetOverVisual("d:/ymir work/ui/public/middle_button_01.sub");

        if not exchange.GetAcceptFromSelf():
            self.ReadyButton.SetUpVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton.SetDownVisual("d:/ymir work/ui/public/middle_button_01.sub");
            self.ReadyButton.SetOverVisual("d:/ymir work/ui/public/middle_button_01.sub");

    def OverInOwnerItem(self, slotIndex):

        if 0 != self.tooltipItem:
            self.tooltipItem.SetExchangeOwnerItem(slotIndex)

    def OverInTargetItem(self, slotIndex):

        if 0 != self.tooltipItem:
            self.tooltipItem.SetExchangeTargetItem(slotIndex)

    def OverOutItem(self):

        if 0 != self.tooltipItem:
            self.tooltipItem.HideToolTip()

    if not app.WJ_ENABLE_TRADABLE_ICON:
        def OnTop(self):
            self.tooltipItem.SetTop()

    def OnMouseLeftButtonDown(self):
        hyperlink = ui.GetHyperlink()
        if hyperlink:
            self.interface.MakeHyperlinkTooltip(hyperlink)

    def OnUpdate(self):
        USE_EXCHANGE_LIMIT_RANGE = 1000

        (x, y, z) = player.GetMainCharacterPosition()
        if abs(x - self.xStart) > USE_EXCHANGE_LIMIT_RANGE or abs(y - self.yStart) > USE_EXCHANGE_LIMIT_RANGE:
            (self.xStart, self.yStart, z) = player.GetMainCharacterPosition()
            self.RemoveFlag("animated_board")
            appInst.instance().GetNet().SendExchangeExitPacket()

    if app.WJ_ENABLE_TRADABLE_ICON:
        def CantTradableItem(self, slotIndex):
            itemIndex = player.GetItemIndex(slotIndex)
    
            if itemIndex:
                if player.GetItemSealDate(player.INVENTORY, slotIndex) == -1:  # ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿.
                    return True
                return player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_GIVE)
            return False

        def BindInterface(self, interface):
            from _weakref import proxy
            self.interface = proxy(interface)

        def OnSetFocus(self):
            self.tooltipItem.SetTop()
            if not self.interface:
                return
    
            self.interface.SetOnTopWindow(player.ON_TOP_WND_EXCHANGE)
            self.interface.RefreshMarkInventoryBag()
