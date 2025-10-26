# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import re

import app
import wndMgr

import localeInfo
import ui
from ui_event import MakeEvent


class PickMoneyDialog(ui.ScriptWindow):
    def __init__(self, layer="UI"):
        ui.ScriptWindow.__init__(self, layer)
        self.AddFlag("animated_board")

        self.unitValue = 1
        self.maxValue = 0
        self.eventAccept = None
        self.eventClose = None
        self.allowZero = 0
        self.pickValueEditLine = 0
        self.acceptButton = 0
        self.cancelButton = 0
        self.board = None

    def LoadDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PickMoneyDialog.py")

        self.board = self.GetChild("board")
        self.maxValueTextLine = self.GetChild("max_value")
        self.pickValueEditLine = self.GetChild("money_value")
        self.acceptButton = self.GetChild("accept_button")
        self.cancelButton = self.GetChild("cancel_button")

        self.pickValueEditLine.SetReturnEvent(self.OnAccept)
        self.pickValueEditLine.SetEscapeEvent(self.Close)
        self.acceptButton.SetEvent(self.OnAccept)
        self.cancelButton.SetEvent(self.Cancel)
        self.board.SetCloseEvent(self.Close)

    def Cancel(self):
        if self.eventClose:
            self.eventClose()
        self.Close()

    def Destroy(self):
        self.ClearDictionary()
        self.eventAccept = None
        self.eventClose = None
        self.maxValue = 0
        self.pickValueEditLine = None
        self.acceptButton = None
        self.cancelButton = None
        self.board = None

    def SetTitleName(self, text):
        self.board.SetTitleName(text)

    def SetAcceptEvent(self, event):
        self.eventAccept = MakeEvent(event)

    def SetCancelEvent(self, event):
        self.eventClose = MakeEvent(event)

    def SetZeroAllowed(self, value):
        self.allowZero = value

    def SetMax(self, max):
        self.pickValueEditLine.SetMax(max)

    def SetValue(self, val):
        self.pickValueEditLine.SetText(str(val))
        self.pickValueEditLine.SetEndPosition()  # move cursor to the end
        self.pickValueEditLine.SetFocus()  # transfer focus

    def Open(self, maxValue, unitValue=1):
        width = self.GetWidth()
        (mouseX, mouseY) = wndMgr.GetMousePosition()

        if mouseX + width / 2 > wndMgr.GetScreenWidth():
            xPos = wndMgr.GetScreenWidth() - width
        elif mouseX - width / 2 < 0:
            xPos = 0
        else:
            xPos = mouseX - width / 2

        self.SetPosition(xPos, mouseY - self.GetHeight() - 20)

        self.maxValueTextLine.SetText(" / " + localeInfo.MoneyFormat(maxValue))

        self.pickValueEditLine.SetText(str(unitValue))
        self.pickValueEditLine.SetEndPosition()  # move cursor to the end
        self.pickValueEditLine.SetFocus()  # transfer focus

        self.unitValue = unitValue
        self.maxValue = maxValue
        self.Show()
        self.SetTop()

    def Close(self):
        self.pickValueEditLine.KillFocus()

        self.Hide()

    def OnAccept(self):

        text = self.pickValueEditLine.GetText()

        if len(text) > 0 and text.isdigit():

            money = 0
            try:
                money = long(text)
            except ValueError:
                money = 199999999

            if self.allowZero or money > 0:
                if self.eventAccept:
                    self.eventAccept(money)

        elif len(text) > 0:

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
                    money = 1999999999

            if money > self.maxValue:
                money = self.maxValue

            if self.allowZero or money > 0:
                if self.eventAccept:
                    self.eventAccept(money)

        self.Close()

    def OnUpdate(self):
        if len(self.pickValueEditLine.GetText()):
            if int(self.pickValueEditLine.GetText()) > self.maxValue:
                self.pickValueEditLine.SetText(self.maxValue)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False
