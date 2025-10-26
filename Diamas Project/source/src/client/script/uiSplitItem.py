# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import wndMgr

import localeInfo
import ui
from ui_event import MakeEvent


class SplitItemDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.unitValue = 1
        self.maxValue = 0
        self.eventAccept = 0
        self.loaded = False

    def LoadDialog(self):
        if self.loaded:
            return

        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/SplitItemDialog.py")
        except Exception as e:
            logging.exception(e)

        try:
            self.board = self.GetChild("board")
            self.maxValueTextLine = self.GetChild("max_value")
            self.pickValueEditLine = self.GetChild("money_value")
            self.splitCheckbox = self.GetChild("split_checkbox")
            self.acceptButton = self.GetChild("accept_button")
            self.cancelButton = self.GetChild("cancel_button")
        except Exception as e:
            logging.exception(e)


        self.pickValueEditLine.SetReturnEvent(self.OnAccept)
        self.pickValueEditLine.SetEscapeEvent(self.Close)
        self.splitCheckbox.SetEvent(self.OnFullSplitCheckbox)
        self.acceptButton.SetEvent(self.OnAccept)
        self.cancelButton.SetEvent(self.Close)
        self.board.SetCloseEvent(self.Close)

    def Destroy(self):
        self.ClearDictionary()
        self.eventAccept = 0
        self.maxValue = 0
        self.pickValueEditLine = None
        self.acceptButton = None
        self.cancelButton = None
        self.board = None
        ui.ScriptWindow.Destroy(self)

    def SetTitleName(self, text):
        self.board.SetTitleName(text)

    def SetAcceptEvent(self, event):
        self.eventAccept = MakeEvent(event)

    def SetMax(self, max):
        self.pickValueEditLine.SetMax(max)

    def OnFullSplitCheckbox(self):
        logging.debug("Set %i" % self.splitCheckbox.IsChecked())

    # cfg.Set(cfg.SAVE_OPTION, "FullSplitItem", self.splitCheckbox.IsChecked())

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

        if localeInfo.IsARABIC():
            self.maxValueTextLine.SetText("/" + str(maxValue))
        else:
            self.maxValueTextLine.SetText(" / " + str(maxValue))


        self.pickValueEditLine.SetText(str(unitValue))
        self.pickValueEditLine.SetFocus()

        self.pickValueEditLine.SetEndPosition()

        self.unitValue = unitValue
        self.maxValue = maxValue

        # self.splitCheckbox.SetChecked(int(cfg.Get(cfg.SAVE_OPTION, "FullSplitItem", "0")))

        self.Show()
        self.SetTop()

    def Close(self):
        self.pickValueEditLine.KillFocus()
        self.Hide()

    def __ConvertMoneyText(self, text):
        resultText = text

        while resultText.find("k") != -1:
            pos = resultText.find("k")
            resultText = resultText[:pos] + "000" + resultText[pos + 1:]

        return int(resultText)

    def OnAccept(self):

        text = self.pickValueEditLine.GetText()

        if len(text) > 0:

            money = self.__ConvertMoneyText(text)
            money = min(money, self.maxValue)

            if money > 0:
                if self.eventAccept:
                    self.eventAccept(money, self.splitCheckbox.IsChecked())

        self.Close()
