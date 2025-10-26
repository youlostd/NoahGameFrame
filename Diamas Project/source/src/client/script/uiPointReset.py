# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
from pygame.app import appInst

import ui


###################################################################################################
## PointReset
class PointResetDialog(ui.ScriptWindow):

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.ConfirmDialog = ui.ScriptWindow()

    def LoadDialog(self):
        PythonScriptLoader = ui.PythonScriptLoader()
        PythonScriptLoader.LoadScriptFile(self, "uiscript/questiondialog2.py")
        PythonScriptLoader.LoadScriptFile(self.ConfirmDialog, "uiscript/questiondialog2.py")

        GetObject = self.ConfirmDialog.GetChild
        self.ConfirmText = GetObject("message1")
        self.ConfirmText2 = GetObject("message2")
        self.ConfirmAcceptButton = GetObject("accept")
        self.ConfirmCancelButton = GetObject("cancel")


        self.GetChild("message1").SetText("½ºÅÈ/½ºÅ³ Æ÷ÀÎÆ®¸¦ ÃÊ±âÈ­ÇØÁÖ°Ú³×.")
        self.GetChild("message2").SetText("°¡°ÝÀº 500¿øÀÌ¾ß. ¾î¶§, ÃÊ±âÈ­ÇÒÅÙ°¡?")
        self.GetChild("accept").SetEvent(self.OpenConfirmDialog)
        self.GetChild("cancel").SetEvent(self.Close)

        ## Confirm Dialog
        self.ConfirmText.SetText("ÇöÀç ·¹º§ÀÇ °æÇèÄ¡°¡ ¸ðµÎ ¾ø¾îÁø´Ù³×.")
        self.ConfirmText.SetFontColor(1.0, 0.3, 0.3)
        self.ConfirmText2.SetText("Á¤¸» ÃÊ±âÈ­ÇÏ°í ½ÍÀº°¡?")
        self.ConfirmAcceptButton.SetEvent(self.ResetPoint)
        self.ConfirmCancelButton.SetEvent(self.Close)

    def Destroy(self):
        self.ClearDictionary()
        self.ConfirmDialog.ClearDictionary()
        self.ConfirmAcceptButton.SetEvent(None)
        self.ConfirmCancelButton.SetEvent(None)

        self.ConfirmDialog = 0
        self.ConfirmText = 0
        self.ConfirmAcceptButton = 0
        self.ConfirmCancelButton = 0

    def OpenDialog(self):
        self.Show()

    def OpenConfirmDialog(self):
        self.ConfirmDialog.Show()
        self.ConfirmDialog.SetTop()

    def ResetPoint(self):
        appInst.instance().GetNet().SendChatPacket("/pointreset")
        self.Close()

    def Close(self):
        self.ConfirmDialog.Hide()
        self.Hide()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
