# -*- coding: utf-8 -*-
import logging

from app import VK_ESCAPE

from ui import ScriptWindow, PythonScriptLoader


class PreviewWindow(ScriptWindow):
    def __init__(self):
        super(PreviewWindow, self).__init__()
        self.AddFlag("animated_board")

        self.loaded = False
        self.renderTarget = None

    def LoadWindow(self):
        if self.loaded:
            return

        logging.debug("Load ItemPreview")

        loader = PythonScriptLoader()
        loader.LoadScriptFile(self, "UIScript/ItemPreview.py")

        self.renderTarget = self.GetChild("item_renderer")
        self.renderTarget.AddFlag("dragable")
        self.renderTarget.SetFov(15.0)
        self.renderTarget.SetTargetZ(0.0)
        self.renderTarget.EnableRotation(False)
        self.renderTarget.Hide()

        self.GetChild("board").SetCloseEvent(self.Close)

        self.loaded = True

    def Open(self, data):
        self.LoadWindow()
        self.SetTop()
        self.SetFocus()

        self.renderTarget.SetData(data)
        self.renderTarget.Show()

        self.Show()

    def Close(self):
        if self.renderTarget:
            self.renderTarget.Hide()
        self.Hide()

    def Destroy(self):
        self.ClearDictionary()

    def OnKeyDown(self, key):
        if key == VK_ESCAPE:
            self.Close()
            return True
        return False
