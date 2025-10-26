# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging
import re

import app
import chat
import item
import nonplayer
import player
import shop
import skill
import wndMgr
from pygame.item import MakeItemPosition
from pygame.player import playerInst

import constInfo
import localeInfo
import ui
import uiToolTip
from ui_event import MakeEvent, MakeCallback, Event


class OnlinePopup(ui.BorderB, ui.BaseEvents):
    def __init__(self):
        ui.BorderB.__init__(self)
        ui.BaseEvents.__init__(self)

        self.isActiveSlide = False
        self.isActiveSlideOut = False
        self.endTime = 0
        self.wndWidth = 0
        self.userType = 0

        self.textLine = ui.TextLine()
        self.textLine.SetParent(self)
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetPosition(13, 0)
        self.textLine.Show()

        self.onlineImage = ui.ImageBox()
        self.onlineImage.SetParent(self)
        self.onlineImage.SetPosition(8, 8)
        self.onlineImage.LoadImage("d:/ymir work/ui/gui/status_online.sub")
        self.onlineImage.Show()

    def SetEvent(self, event):
        self.SetMouseLeftButtonDownEvent(event)

    def SlideIn(self):
        self.SetTop()
        self.Show()

        self.isActiveSlide = True
        self.endTime = app.GetGlobalTimeStamp() + 5

    def Close(self):
        self.Hide()

    def Destroy(self):
        self.Close()

    def SetUserType(self, val):
        self.userType = val

    def GetEndTime(self):
        return self.endTime

    def SetUserName(self, name):
        if self.userType == 0:
            self.textLine.SetText(
                localeInfo.Get("POPUP_PLAYER_ONLINE").format(str(name))
            )
        elif self.userType == 1:
            self.textLine.SetText(
                localeInfo.Get("POPUP_GUILD_MEMBER_ONLINE").format(str(name))
            )
        elif self.userType == 2:
            self.textLine.SetText(localeInfo.Get("POPUP_GM_ONLINE").format(str(name)))

        self.wndWidth = self.textLine.GetWidth() + 40
        self.SetSize(self.wndWidth, 25)
        self.SetPosition(-self.wndWidth, wndMgr.GetScreenHeight() - 200)

    def OnUpdate(self):
        if self.isActiveSlide and self.isActiveSlide == True:
            x, y = self.GetLocalPosition()
            if x < 0:
                self.SetPosition(x + 4, y)

        if (
            self.endTime - app.GetGlobalTimeStamp() <= 0
            and self.isActiveSlideOut == False
            and self.isActiveSlide == True
        ):
            self.isActiveSlide = False
            self.isActiveSlideOut = True

        if self.isActiveSlideOut and self.isActiveSlideOut == True:
            x, y = self.GetLocalPosition()
            if x > -(self.wndWidth):
                self.SetPosition(x - 4, y)

            if x <= -(self.wndWidth):
                self.isActiveSlideOut = False
                self.Close()


class PopupDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__LoadDialog()
        self.acceptEvent = None

    def __LoadDialog(self):
        try:
            PythonScriptLoader = ui.PythonScriptLoader()
            PythonScriptLoader.LoadScriptFile(self, "UIScript/PopupDialog.py")

            self.board = self.GetChild("board")
            self.message = self.GetChild("message")
            self.accceptButton = self.GetChild("accept")
            self.accceptButton.SetEvent(self.Close)
        except Exception as e:
            logging.exception(e)

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Close(self):
        self.Hide()
        if self.acceptEvent:
            self.acceptEvent()

    def Destroy(self):
        self.Close()
        self.ClearDictionary()

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def SetText(self, text):
        self.message.SetText(text)

    def SetAcceptEvent(self, event):
        self.acceptEvent = MakeEvent(event)

    def SetButtonName(self, name):
        self.accceptButton.SetText(name)

    def OnKeyDown(self, key):
        if key == app.VK_RETURN:
            return False

        self.Close()
        return True


class InputDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__CreateDialog()

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/inputdialog.py")

        getObject = self.GetChild
        self.board = getObject("Board")
        self.acceptButton = getObject("AcceptButton")
        self.cancelButton = getObject("CancelButton")
        self.inputSlot = getObject("InputSlot")
        self.inputValue = getObject("InputValue")

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()
        self.SetFocus()

    def Close(self):
        self.ClearDictionary()
        self.board = None
        self.acceptButton = None
        self.cancelButton = None
        self.inputSlot = None
        self.inputValue = None
        self.Hide()

    def SetTitle(self, name):
        self.board.SetTitleName(name)

    def SetNumberMode(self):
        self.inputValue.SetNumberMode()

    def SetSecretMode(self):
        self.inputValue.SetSecret()

    def SetFocus(self):
        self.inputValue.SetFocus()

    def SetMaxLength(self, length):
        width = length * 7 + 10
        self.SetBoardWidth(width + 50)
        self.SetSlotWidth(width)
        self.inputValue.SetMax(length)

    def SetSlotWidth(self, width):
        self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
        self.inputValue.SetSize(width, self.inputValue.GetHeight())
        if self.IsRTL():
            self.inputValue.SetPosition(self.inputValue.GetWidth(), 0)

    def SetBoardWidth(self, width):
        self.SetSize(max(width + 50, 160), self.GetHeight())
        self.board.SetSize(max(width + 50, 160), self.GetHeight())
        if self.IsRTL():
            self.board.SetPosition(self.board.GetWidth(), 0)
        self.UpdateRect()

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)
        self.inputValue.SetReturnEvent(event)

    def SetCancelEvent(self, event):
        self.board.SetCloseEvent(event)
        self.cancelButton.SetEvent(event)
        self.inputValue.SetEscapeEvent(event)

    def GetText(self):
        return self.inputValue.GetText()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False


class InputDialogWithDescription(InputDialog):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__CreateDialog()

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/inputdialogwithdescription.py")

        try:
            getObject = self.GetChild
            self.board = getObject("Board")
            self.acceptButton = getObject("AcceptButton")
            self.cancelButton = getObject("CancelButton")
            self.inputSlot = getObject("InputSlot")
            self.inputValue = getObject("InputValue")
            self.description = getObject("Description")

        except:
            logging.exception("InputDialogWithDescription.LoadBoardDialog.BindObject")

    def SetDescription(self, text):
        self.description.SetText(text)


class InputDialogWithDescription2(InputDialog):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__CreateDialog()

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/inputdialogwithdescription2.py")

        try:
            getObject = self.GetChild
            self.board = getObject("Board")
            self.acceptButton = getObject("AcceptButton")
            self.cancelButton = getObject("CancelButton")
            self.inputSlot = getObject("InputSlot")
            self.inputValue = getObject("InputValue")
            self.description1 = getObject("Description1")
            self.description2 = getObject("Description2")

        except:
            logging.exception("InputDialogWithDescription.LoadBoardDialog.BindObject")

    def SetDescription1(self, text):
        self.description1.SetText(text)

    def SetDescription2(self, text):
        self.description2.SetText(text)


class QuestionDialog(ui.ScriptWindow):
    def __init__(self, layer="TOP_MOST"):
        ui.ScriptWindow.__init__(self, layer)
        self.AddFlag("animated_board")

        self.closeEvent = None
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog.py")

        self.board = self.GetChild("board")
        self.textLine = self.GetChild("message")
        self.acceptButton = self.GetChild("accept")
        self.cancelButton = self.GetChild("cancel")
        self.board.SetCloseEvent(self.Close)

        self.closeEvent = None
        self.acceptEvent = None

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.SetAutoWidth()
        self.Show()
        self.SetFocus()

    def Close(self):
        self.Hide()
        if self.closeEvent:
            self.closeEvent()

    def SetAutoWidth(self):
        self.SetWidth(max(245, self.textLine.GetWidth()))
        self.SetCenterPosition()

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)
        self.acceptEvent = MakeEvent(event)

    def SetCancelEvent(self, event):
        self.cancelButton.SetEvent(event)

    def SetCloseEvent(self, event):
        self.closeEvent = MakeEvent(event)

    def SetText(self, text):
        self.textLine.SetText(text)

    def SetAcceptText(self, text):
        self.acceptButton.SetText(text)

    def SetCancelText(self, text):
        self.cancelButton.SetText(text)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        elif key == app.VK_RETURN:
            self.acceptEvent()
            return True


class QuestionDialog2(QuestionDialog):
    def __init__(self, layer="TOP_MOST"):
        QuestionDialog.__init__(self, layer)
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog2.py")

        self.board = self.GetChild("board")
        self.textLine1 = self.GetChild("message1")
        self.textLine2 = self.GetChild("message2")
        self.acceptButton = self.GetChild("accept")
        self.cancelButton = self.GetChild("cancel")

    def SetText1(self, text):
        self.textLine1.SetText(text)

    def SetText2(self, text):
        self.textLine2.SetText(text)

    def SetAutoWidth(self):
        widthList = [self.textLine1.GetWidth(), self.textLine2.GetWidth()]
        width = max(widthList) + 30

        self.SetWidth(max(width, 245))
        self.SetCenterPosition()


class QuestionDialog3(QuestionDialog):
    def __init__(self, layer="TOP_MOST"):
        QuestionDialog.__init__(self, layer)
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog3.py")

        self.board = self.GetChild("board")
        self.textLine1 = self.GetChild("message1")
        self.textLine2 = self.GetChild("message2")
        self.textLine3 = self.GetChild("message3")
        self.acceptButton = self.GetChild("accept")
        self.cancelButton = self.GetChild("cancel")

    def SetText1(self, text):
        self.textLine1.SetText(text)

    def SetText2(self, text):
        self.textLine2.SetText(text)

    def SetText3(self, text):
        self.textLine3.SetText(text)

    def SetAutoWidth(self):
        widthList = [
            self.textLine1.GetWidth(),
            self.textLine2.GetWidth(),
            self.textLine3.GetWidth(),
        ]
        width = max(widthList) + 30

        self.SetWidth(max(245, width))
        self.SetCenterPosition()


class QuestionDialog4(ui.ScriptWindow):

    def __init__(self, acceptOnEnter=True):
        ui.ScriptWindow.__init__(self, "TOP_MOST")
        self.AddFlag("animated_board")

        self.acceptOnEnter = acceptOnEnter
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog4.py")

        self.board = self.GetChild("board")
        self.textLine = self.GetChild("message")
        self.accept1Button = self.GetChild("accept1")
        self.accept2Button = self.GetChild("accept2")
        self.cancelButton = self.GetChild("cancel")
        self.board.SetCloseEvent(self.Close)

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()
        self.SetFocus()

    def Close(self):
        self.Hide()

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def SetAccept1Event(self, event):
        self.accept1Button.SetEvent(event)

    def SetAccept2Event(self, event):
        self.accept2Button.SetEvent(event)

    def SetCancelEvent(self, event):
        self.cancelButton.SetEvent(event)

    def SetText(self, text):
        self.textLine.SetText(text)

    def SetAccept1Text(self, text):
        self.accept1Button.SetText(text)

    def SetAccept2Text(self, text):
        self.accept2Button.SetText(text)

    def SetCancelText(self, text):
        self.cancelButton.SetText(text)

    def Accept(self):
        self.accept1Button.SimulClick()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True


class QuestionDialogWithTimeLimit(QuestionDialog2):
    def __init__(self):
        QuestionDialog2.__init__(self)
        self.__CreateDialog()

        self.endTime = 0
        self.timeoverMsg = None
        self.isCancelOnTimeover = False

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog2.py")

        self.board = self.GetChild("board")
        self.textLine1 = self.GetChild("message1")
        self.textLine2 = self.GetChild("message2")
        self.acceptButton = self.GetChild("accept")
        self.cancelButton = self.GetChild("cancel")

    def Open(self, msg, timeout):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

        self.SetText1(msg)
        self.endTime = long(app.GetTime() + timeout)

        self.SetAutoWidth()
        self.SetFocus()

    def SetAutoWidth(self):
        self.SetWidth(max(self.textLine1.GetWidth() + 30, 245))
        self.SetCenterPosition()

    def OnUpdate(self):
        leftTime = float(max(0, self.endTime - app.GetTime()))
        self.SetText2(localeInfo.UI_LEFT_TIME.format(localeInfo.DottedNumber(leftTime)))
        if leftTime < 0.5:
            if self.timeoverMsg:
                chat.AppendChat(CHAT_TYPE_INFO, self.timeoverMsg)
            if self.isCancelOnTimeover:
                self.cancelButton.CallEvent()

    def SetTimeOverMsg(self, msg):
        self.timeoverMsg = msg

    def SetCancelOnTimeOver(self):
        self.isCancelOnTimeover = True


class QuestionDialogItem(ui.ScriptWindow):
    def __init__(self, layer="TOP_MOST"):
        ui.ScriptWindow.__init__(self, layer)
        self.AddFlag("animated_board")

        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/QuestionDialogItem.py")

        self.board = self.GetChild("board")
        self.textLine = self.GetChild("message")
        self.acceptButton = self.GetChild("accept")
        self.destroyButton = self.GetChild("destroy")
        self.cancelButton = self.GetChild("cancel")

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Close(self):
        self.Hide()

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)

    def SetDestroyEvent(self, event):
        self.destroyButton.SetEvent(event)

    def SetCancelEvent(self, event):
        self.cancelButton.SetEvent(event)

    def SetText(self, text):
        self.textLine.SetText(text)

    def SetAcceptText(self, text):
        self.acceptButton.SetText(text)

    def SetCancelText(self, text):
        self.cancelButton.SetText(text)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True


class SliderDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.closeEvent = None
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/SliderDialog.py")

        self.board = self.GetChild("board")
        self.textLine = self.GetChild("message")
        self.slider = self.GetChild("slider")
        self.closeEvent = None
        self.acceptEvent = None

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Close(self):
        self.Hide()
        if self.closeEvent:
            self.closeEvent()

    def SetAcceptEvent(self, event):
        self.acceptEvent = MakeEvent(event)

    def SetCancelEvent(self, event):
        # self.cancelButton.SetEvent(event)
        pass

    def SetCloseEvent(self, event):
        self.closeEvent = MakeEvent(event)

    def SetText(self, text):
        self.textLine.SetText(text)

    def SetAcceptText(self, text):
        # self.acceptButton.SetText(text)
        pass

    def SetCancelText(self, text):
        # self.cancelButton.SetText(text)
        pass

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        elif key == app.VK_RETURN:
            self.acceptEvent()
            return True


class MoneyInputDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.moneyHeaderText = localeInfo.MONEY_INPUT_DIALOG_SELLPRICE
        self.__CreateDialog()
        self.SetMaxLength(13)

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/moneyinputdialog.py")

        getObject = self.GetChild
        self.board = self.GetChild("board")
        self.acceptButton = getObject("AcceptButton")
        self.cancelButton = getObject("CancelButton")
        self.inputValue = getObject("InputValue")
        self.inputValue.OnChange = MakeCallback(self.__OnValueUpdate)
        self.realValue = 0
        self.moneyText = getObject("MoneyValue")

    def Open(self):
        self.inputValue.SetText("")
        self.inputValue.SetFocus()
        self.__OnValueUpdate()
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def SetNumberMode(self):
        self.inputValue.SetNumberMode()

    def Close(self):
        self.ClearDictionary()
        self.board = None
        self.acceptButton = None
        self.cancelButton = None
        self.inputValue = None
        self.Hide()

    def SetTitle(self, name):
        self.board.SetTitleName(name)

    def SetFocus(self):
        self.inputValue.SetFocus()

    def SetMaxLength(self, length):
        length = min(13, length)
        self.inputValue.SetMax(length)

    def SetMoneyHeaderText(self, text):
        self.moneyHeaderText = text

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)
        self.inputValue.SetReturnEvent(event)

    def SetCancelEvent(self, event):
        self.board.SetCloseEvent(event)
        self.cancelButton.SetEvent(event)
        self.inputValue.SetEscapeEvent(event)

    def SetValue(self, value):
        value = str(value)
        self.inputValue.SetText(value)
        self.inputValue.SetEndPosition()
        self.__OnValueUpdate()

    def GetText(self):
        return self.inputValue.GetText()

    def GetValue(self):
        return self.realValue

    def __OnChar(self, ch):
        ui.EditLine.OnChar(self.inputValue, ch)
        self.__OnValueUpdate()

    def __OnValueUpdate(self):
        text = self.inputValue.GetText()

        def multByThousand(val):
            val = val * 1000
            return val

        multCount = text.count("k")

        text = re.sub("[^0-9|\.|\,]", "", text)
        text.replace(",", ".")
        money = 0
        try:
            money = float(text)
            for i in range(multCount):
                money = long(multByThousand(money))
        except ValueError:
            try:
                text.replace(".", ",")
                money = float(text)
                for i in range(multCount):
                    money = long(multByThousand(money))
            except ValueError:
                money = 0

        money = long(money)
        self.realValue = money
        self.moneyText.SetText(
            self.moneyHeaderText + localeInfo.NumberToMoneyString(money)
        )

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False


class ItemQuestionDialog(ui.ScriptWindow):
    def __init__(self, layer="TOP_MOST"):
        ui.ScriptWindow.__init__(self, layer)
        self.AddFlag("animated_board")

        self.tooltipItem = uiToolTip.ItemToolTip()
        self.toolTip = uiToolTip.ToolTip()

        self.window_type = 0
        self.count = 0

        self.dropType = 0
        self.dropCount = 0
        self.dropNumber = 0

        self.slotList = []
        self.itemPrice = None
        self.slotGrid = None
        self.closeEvent = None
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/QuestionDialog.py")

        self.board = self.GetChild("board")
        self.textLine = self.GetChild("message")
        self.acceptButton = self.GetChild("accept")
        self.cancelButton = self.GetChild("cancel")

        self.slotList = []
        for i in xrange(3):
            slot = ui.ImageBox()
            slot.LoadImage("d:/ymir work/ui/public/slot_base.sub")
            slot.SetParent(self)
            slot.SetHorizontalAlignCenter()
            self.slotList.append(slot)

    def Open(self, vnum, slot=None, price=None, sockets=None):
        item.SelectItem(vnum)
        xSlotCount, ySlotCount = item.GetItemSize()

        if slot:
            slot = MakeItemPosition(slot[0], slot[1])

        metinSlot = []

        try:
            if self.window_type == player.INVENTORY:
                metinSlot = [
                    playerInst().GetItemMetinSocket(slot, i)
                    for i in xrange(player.METIN_SOCKET_MAX_NUM)
                ]
            elif self.window_type == player.SHOP:
                metinSlot = [
                    shop.GetItemMetinSocket(slot.cell, i)
                    for i in xrange(player.METIN_SOCKET_MAX_NUM)
                ]
            if sockets:
                metinSlot = sockets

            if vnum in (50300, 70037):
                self.board.SetTitleName(
                    "%s %s" % (skill.GetSkillName(metinSlot[0]), item.GetItemName())
                )
            elif vnum == 70104:
                self.board.SetTitleName(
                    "%s %s"
                    % (nonplayer.GetMonsterName(metinSlot[0]), item.GetItemName())
                )
            else:
                self.board.SetTitleName(item.GetItemName())
        except:
            pass

        newHeight = 0

        if price:
            newHeight = 20

            itemPrice = ui.TextLine()
            itemPrice.SetPosition(0, 77 + 32 * ySlotCount)
            itemPrice.SetHorizontalAlignCenter()
            itemPrice.SetHorizontalAlignCenter()
            itemPrice.SetVerticalAlignCenter()
            itemPrice.SetParent(self.board)
            if str(price).isdigit():
                itemPrice.SetText(localeInfo.NumberToMoneyString(price))
            else:
                itemPrice.SetText(price)
            itemPrice.Show()
            self.itemPrice = itemPrice

        slotGrid = ui.SlotWindow()
        slotGrid.SetParent(self)
        slotGrid.SetPosition(-32, 42)
        slotGrid.SetHorizontalAlignCenter()
        slotGrid.AppendSlot(0, 0, 0, 32 * xSlotCount, 32 * ySlotCount)
        slotGrid.AddFlag("not_pick")
        slotGrid.Show()
        self.slotGrid = slotGrid

        if self.count > 1 and vnum != 1:
            self.slotGrid.SetItemSlot(0, vnum, self.count)
        else:
            self.slotGrid.SetItemSlot(0, vnum)

        self.SetSize(
            60 + self.textLine.GetWidth() + 30, 110 + 32 * ySlotCount + newHeight
        )
        self.board.SetSize(
            60 + self.textLine.GetWidth() + 30, 110 + 32 * ySlotCount + newHeight
        )
        self.textLine.SetVerticalAlignBottom()
        self.textLine.SetPosition(0, 44)

        for i in xrange(min(3, ySlotCount)):
            self.slotList[i].SetPosition(0, 10 + ySlotCount * 32 - i * 32)
            if vnum != 1:
                self.slotList[i].SetOnMouseOverInEvent(Event(self.OverInItem, slot))
                self.slotList[i].SetOnMouseOverOutEvent(
                    Event(self.OverOutItem, self.tooltipItem)
                )
            else:
                self.slotList[i].SetOnMouseOverInEvent(
                    Event(self.OverInToolTip, localeInfo.MONETARY_UNIT0)
                )
                self.slotList[i].SetOnMouseOverOutEvent(self.OverOutItem)
            self.slotList[i].Show()

        self.GetChild("accept").SetPosition(-80, 74 + 32 * ySlotCount + newHeight)
        self.GetChild("cancel").SetPosition(90, 74 + 32 * ySlotCount + newHeight)

        self.board.SetCloseEvent(self.Close)

        self.SetCenterPosition()
        self.SetTop()
        self.board.SetTop()
        for slot in self.slotList:
            slot.SetTop()
        self.slotGrid.SetTop()

        self.Show()

    def SetMessage(self, text):
        self.textLine.SetText(text)

    def OverInToolTip(self, arg):
        self.toolTip.ClearToolTip()
        self.toolTip.AppendTextLine(arg, 0xFFFFFF00)
        self.toolTip.Show()

    def OverOutToolTip(self):
        self.toolTip.Hide()

    def OverInItem(self, slot):
        if self.tooltipItem:
            if self.window_type == player.NPC_SHOP:
                self.tooltipItem.SetShopItem(slot.cell)
            elif self.window_type == player.INVENTORY:
                self.tooltipItem.SetInventoryItem(slot.cell, slot.windowType)
        return True

    def OverOutItem(self, tooltipItem):
        if None != tooltipItem:
            self.tooltipItem.HideToolTip()
            self.tooltipItem.ClearToolTip()
        return True

    def Close(self):
        if self.closeEvent:
            self.closeEvent()

        self.ClearDictionary()
        self.slotList = []
        self.itemPrice = None
        self.slotGrid = None

        self.toolTip = None
        self.tooltipItem = None
        self.Hide()

        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)
        self.acceptEvent = MakeEvent(event)

    def SetCancelEvent(self, event):
        self.cancelButton.SetEvent(event)

    def SetCloseEvent(self, event):
        self.closeEvent = MakeEvent(event)

    def SetText(self, text):
        self.textLine.SetText(text)
        self.SetAutoWidth()

    def SetAutoWidth(self):
        width = self.textLine.GetWidth() + 30
        self.SetWidth(width)
        self.SetCenterPosition()

    def SetAcceptText(self, text):
        self.acceptButton.SetText(text)

    def SetCancelText(self, text):
        self.cancelButton.SetText(text)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        elif key == app.VK_RETURN:
            self.acceptEvent()
            return True


class DoubleInputDialogWithDescription(ui.ScriptWindow):

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__CreateDialog()

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/doubleinputdialogwithdescription.py")

        getObject = self.GetChild
        self.board = getObject("Board")
        self.acceptButton = getObject("AcceptButton")
        self.cancelButton = getObject("CancelButton")
        self.inputSlot = [getObject("InputSlot1"), getObject("InputSlot2")]
        self.inputValue = [getObject("InputValue1"), getObject("InputValue2")]
        self.description = [getObject("Description1"), getObject("Description2")]
        self.maxLen = [0, 0]

        self.inputValue[0].SetReturnEvent(self.inputValue[1].SetFocus)
        self.inputValue[0].SetTabEvent(self.inputValue[1].SetFocus)

    def Open(self):
        self.SetFocus()
        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Close(self):
        self.ClearDictionary()
        self.board = None
        self.acceptButton = None
        self.cancelButton = None
        self.inputSlot = None
        self.inputValue = None
        self.Hide()

    def SoftClose(self):
        for i in xrange(2):
            self.inputValue[i].KillFocus()
        self.Hide()

    def SetTitle(self, name):
        self.board.SetTitleName(name)

    def SetNumberMode(self, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetNumberMode(i)
        else:
            self.inputValue[index].SetNumberMode()

    def SetSecretMode(self, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetSecretMode(i)
        else:
            self.inputValue[index].SetSecret()

    def SetFocus(self, index=0):
        self.inputValue[index].SetFocus()

    def SetMaxLength(self, length, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetMaxLength(length, i)
        else:
            width = length * 6 + 10
            self.maxLen[index] = width
            maxWidth = max(self.maxLen[1 - index], width)
            self.SetBoardWidth(max(maxWidth + 50, 160))
            self.SetSlotWidth(width, index)
            self.inputValue[index].SetMax(length)

    def SetSlotWidth(self, width, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetSlotWidth(width, i)
        else:
            self.inputSlot[index].SetSize(width, self.inputSlot[index].GetHeight())
            self.inputValue[index].SetSize(width, self.inputValue[index].GetHeight())
            if self.IsRTL():
                self.inputValue[index].SetPosition(self.inputValue[index].GetWidth(), 0)

    def GetDisplayWidth(self, index):
        return self.inputSlot[index].GetWidth()

    def SetDisplayWidth(self, width, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetEditWidth(width, i)
        else:
            self.maxLen[index] = width
            maxWidth = max(self.maxLen[1 - index], width)
            self.SetBoardWidth(max(maxWidth + 50, 160))
            self.SetSlotWidth(width, index)

    def SetBoardWidth(self, width):
        self.SetSize(max(width + 50, 160), self.GetHeight())
        self.board.SetSize(max(width + 50, 160), self.GetHeight())
        if self.IsRTL():
            self.board.SetPosition(self.board.GetWidth(), 0)
        self.UpdateRect()

    def SetAcceptEvent(self, event):
        self.acceptButton.SetEvent(event)
        self.inputValue[1].SetReturnEvent(event)

    def SetCancelEvent(self, event):
        self.board.SetCloseEvent(event)
        self.cancelButton.SetEvent(event)
        for i in xrange(2):
            self.inputValue[i].SetEscapeEvent(event)

    def GetText(self, index):
        return self.inputValue[index].GetText()

    def SetDescription(self, text, index=-1):
        if index == -1:
            for i in xrange(2):
                self.SetDescription(text, i)
        else:
            self.description[index].SetText(text)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False


class InputDialogWithColorPicker(InputDialog):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.chosenColor = 0
        self.__CreateDialog()

    def __CreateDialog(self):

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/inputwithcolorpicker.py")

        try:
            getObject = self.GetChild
            self.board = getObject("Board")
            self.acceptButton = getObject("AcceptButton")
            self.cancelButton = getObject("CancelButton")
            self.extraButton = getObject("ExtraButton")
            self.inputSlot = getObject("InputSlot")
            self.inputValue = getObject("InputValue")
            self.description = getObject("Description")
            self.preview = getObject("Preview")
            self.previewButton = getObject("PreviewButton")

        except:
            logging.exception(
                "InputDialogWithDescriptionExtraButton.LoadBoardDialog.BindObject"
            )

    def SetPreviewEvent(self, event):
        self.previewButton.SetEvent(event)

    def GetPreviewButton(self):
        return self.previewButton

    def SetDescription(self, text):
        self.description.SetText(text)

    def SetExtraButtonEvent(self, event):
        self.extraButton.SetEvent(event)

    def SetInputValueColor(self, color):
        self.chosenColor = color
        self.inputValue.SetPackedFontColor(color)
        self.preview.SetColor(color)

    def GetInputValueColor(self):
        return self.chosenColor

    def SetWidth(self, width):
        height = self.GetHeight()
        self.SetSize(width, height)
        self.board.SetSize(width, height)
        self.SetCenterPosition()
        self.UpdateRect()

    def Open(self):
        self.SetCenterPosition()
        self.SetTop()
        self.SetAutoWidth()
        self.Show()
        self.SetFocus()

    def SetAutoWidth(self):
        self.SetWidth(max(245, self.description.GetWidth()))
        self.SetCenterPosition()
