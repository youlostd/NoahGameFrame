# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-

from pygame.app import appInst

import ui


###################################################################################################
## Restart
class RestartDialog(ui.ScriptWindow):

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

    def LoadDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/restartdialog.py")

        self.GetChild("board").DeactivateCloseButton()
        self.restartHereButton=self.GetChild("restart_here_button")
        self.restartTownButton=self.GetChild("restart_town_button")

        self.restartHereButton.SetEvent(self.RestartHere)
        self.restartTownButton.SetEvent(self.RestartTown)

        return 1

    def Destroy(self):
        self.restartHereButton=0
        self.restartTownButton=0
        self.ClearDictionary()

    def OpenDialog(self):
        self.Show()

    def Close(self):
        self.Hide()
        return True

    def RestartHere(self):
        appInst.instance().GetNet().SendChatPacket("/restart_here")

    def RestartTown(self):
        appInst.instance().GetNet().SendChatPacket("/restart_town")

    def OnPressExitKey(self):
        return True

