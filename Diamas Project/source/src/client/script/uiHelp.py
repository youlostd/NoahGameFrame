# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app

import ui
from ui_event import MakeEvent

ENABLE_HELP_MULTIPAGE = 1


class HelpWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self, "TOP_MOST")
        self.eventClose = 0

    def LoadDialog(self):
        if ENABLE_HELP_MULTIPAGE:
            self.LoadDialogMultiPage()
        else:
            self.LoadDialogSinglePage()

    def LoadDialogSinglePage(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/HelpWindow.py")

        GetObject = self.GetChild
        self.btnClose = GetObject("close_button")


    def LoadDialogMultiPage(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/HelpWindow2.py")

        GetObject = self.GetChild
        self.btnClose = GetObject("close_button")
        self.pages = {}
        self.btnPages = {}
        self.pages[0] = GetObject("page_1")
        self.pages[1] = GetObject("page_2")
        self.btnPages[0] = GetObject("page_1_button")
        self.btnPages[1] = GetObject("page_2_button")
        self.btnPages[0].SetEvent(self.__OnClickPage1)
        self.btnPages[1].SetEvent(self.__OnClickPage2)

        self.__SelectPage(0)


    def __OnClickPage1(self):
        self.__SelectPage(0)

    def __OnClickPage2(self):
        self.__SelectPage(1)

    def Destroy(self):
        self.eventClose = 0
        self.closeButton = 0
        self.pages = {}
        self.btnPages = {}

    def SetCloseEvent(self, event):
        self.eventClose = MakeEvent(event)
        self.btnClose.SetEvent(event)

    def Open(self):
        self.Lock()
        self.Show()

    def Close(self):
        self.Unlock()
        self.Hide()

    def OnKeyDown(self, key):
        if (app.VK_H == key or key == app.VK_ESCAPE) and 0 != self.eventClose:
            self.eventClose()

        return True

    def OnPressExitKey(self):
        if 0 != self.eventClose:
            self.eventClose()
        return True

    def __SelectPage(self, pageIndex):
        for page in self.pages.values():
            page.Hide()
        for btn in self.btnPages.values():
            btn.SetUp()

        self.pages[pageIndex].Show()
        self.btnPages[pageIndex].Down()
