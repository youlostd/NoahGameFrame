# -*- coding: utf-8 -*-

import app
from ui import ScriptWindow, PythonScriptLoader


class WebWindow(ScriptWindow):
    def __init__(self):
        ScriptWindow.__init__(self, "TOP_MOST")
        self.SetWindowName("WebWindow")
        self.oldPos = None
        self.isLoaded = False
        self.AddFlag("not_pick")
        self.LoadWindow()

    def LoadWindow(self):
        if self.isLoaded:
            return

        pyScrLoader = PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/WebWindow.py")
        GetObject = self.GetChild
        self.board = GetObject("board")
        self.webView = GetObject("WebView")
        self.board.SetCloseEvent(self.Hide)

        self.isLoaded = True

    def Destroy(self):
        self.Hide()
        self.ClearDictionary()
        self.board = None

    def Open(self, url):
        self.Show()
        self.SetCenterPosition()
        self.webView.LoadUrl(url)
        self.webView.SetTop()
        self.webView.Show()
        self.board.SetTop()
        self.SetFocus()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True
        return False