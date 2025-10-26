# -*- coding: utf-8 -*-

###################################################################################################
# Network

import app
from pygame.app import appInst

import emoji_config
import game
import introLogin
import localeInfo
import ui
import uiPhaseCurtain
from ui_event import MakeEvent


class PopupDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self, "TOP_MOST")
        self.eventClose = None

    def LoadDialog(self):
        PythonScriptLoader = ui.PythonScriptLoader()
        PythonScriptLoader.LoadScriptFile(self, "UIScript/PopupDialog.py")

    def Open(self, Message, event=None, ButtonName=localeInfo.UI_CANCEL):
        if self.IsShow():
            self.Close()

        self.Lock()
        self.SetTop()

        self.eventClose = MakeEvent(event)

        AcceptButton = self.GetChild("accept")
        # AcceptButton.SetAutoSizeText(ButtonName)
        AcceptButton.SetEvent(self.Close)

        self.GetChild("message").SetText(Message)
        self.GetChild("board").SetSize(
            max(self.GetChild("message").GetWidth() + 40, 280),
            self.GetChild("board").GetHeight(),
        )
        self.GetChild("content").SetSize(
            max(self.GetChild("message").GetWidth() + 40, 280),
            self.GetChild("content").GetHeight(),
        )

        self.Show()

    def Close(self):
        if not self.IsShow():
            self.eventClose = None
            return

        self.Hide()
        self.Unlock()

        if self.eventClose:
            self.eventClose()
            self.eventClose = None

    def Destroy(self):
        self.Close()
        self.ClearDictionary()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

        if key == app.VK_RETURN:
            self.Close()
            return True

        return False


class MainStream(object):
    isChrData = 0
    isChangingChannel = False

    def __init__(self):
        appInst.instance().GetNet().SetHandler(self)

        emoji_config.RegisterEmoticon()

        self.channelIndex = 0
        self.selectedEmpire = 0
        self.serverName = ""

        self.slot = 0

        self.curtain = uiPhaseCurtain.PhaseCurtain()
        self.curtain.speed = 0.09

        self.curPhaseWindow = None
        self.newPhaseWindow = None
        self.persistentObjects = {}

        self.isChangingChannel = False
        self.CreatePopupDialog()

    def Destroy(self):
        if self.curPhaseWindow:
            self.curPhaseWindow.Close()
            self.curPhaseWindow = None

        if self.newPhaseWindow:
            self.newPhaseWindow.Close()
            self.newPhaseWindow = None

        self.popupWindow.Destroy()
        self.popupWindow = None

        self.curtain = None
        appInst.instance().GetNet().SetHandler(None)

    def SetPhaseWindow(self, newPhaseWindow):
        if self.newPhaseWindow:
            self.__ChangePhaseWindow()

        self.newPhaseWindow = newPhaseWindow

        if self.curPhaseWindow:
            self.curtain.FadeOut(self.__ChangePhaseWindow)
        else:
            self.__ChangePhaseWindow()

    def __ChangePhaseWindow(self):
        oldPhaseWindow = self.curPhaseWindow
        newPhaseWindow = self.newPhaseWindow
        self.curPhaseWindow = None
        self.newPhaseWindow = None

        if oldPhaseWindow:
            oldPhaseWindow.Close()

        if newPhaseWindow:
            newPhaseWindow.Open()

        self.curPhaseWindow = newPhaseWindow

        if self.curPhaseWindow:
            self.curtain.FadeIn()
            self.curPhaseWindow.SetFocus()
        else:
            appInst.instance().Exit()

    def CreatePopupDialog(self):
        self.popupWindow = PopupDialog()
        self.popupWindow.LoadDialog()
        self.popupWindow.SetCenterPosition()
        self.popupWindow.Hide()

    ## SelectPhase
    ##########################################################################################
    def SetLoginPhase(self):
        appInst.instance().GetNet().SetOffLinePhase()
        appInst.instance().GetNet().Disconnect()
        self.SetPhaseWindow(introLogin.LoginWindow(self))
        self.OnPhaseChange("login")

    def SameLogin_SetLoginPhase(self):
        appInst.instance().GetNet().SetOffLinePhase()
        appInst.instance().GetNet().Disconnect()
        loginPhase = introLogin.LoginWindow(self)
        self.SetPhaseWindow(loginPhase)
        self.OnPhaseChange("login-same")
        self.popupWindow.Close()

        loginPhase.SameLogin_OpenUI()

    def SetSelectEmpirePhase(self):
        import introEmpire

        self.SetPhaseWindow(introEmpire.SelectEmpireWindow(self))
        self.OnPhaseChange("select-empire")

    def SetReselectEmpirePhase(self):
        import introEmpire

        self.SetPhaseWindow(introEmpire.ReselectEmpireWindow(self))
        self.OnPhaseChange("select-empire")

    def SetSelectCharacterPhase(self):
        import introSelect

        self.popupWindow.Close()
        self.SetPhaseWindow(introSelect.SelectCharacterWindow(self))
        self.OnPhaseChange("select-char")

    def SetCreateCharacterPhase(self):
        import introCreate

        self.SetPhaseWindow(introCreate.CreateCharacterWindow(self))
        self.OnPhaseChange("create-char")

    def SetLoadingPhase(self):
        # self.SetPhaseWindow(None)
        self.OnPhaseChange("loading")

    def SetGamePhase(self):
        self.popupWindow.Close()
        self.SetPhaseWindow(game.GameWindow.instance(self))
        self.OnPhaseChange("game")

    ################################
    # Functions used in python
    def SetServerName(self, name):
        self.serverName = name

    def GetServerName(self):
        return self.serverName

    def SetChannelIndex(self, channelIndex):
        self.channelIndex = channelIndex

    def IsChangingChannel(self):
        return self.isChangingChannel

    def ChangeChannel(self, channelIndex):
        self.channelIndex = channelIndex
        self.isChangingChannel = True

    def CancelEnterGame(self):
        pass

    ## Select
    def SetCharacterSlot(self, slot):
        self.slot = slot

    def GetCharacterSlot(self):
        return self.slot

    def SetSelectedEmpire(self, empire):
        self.selectedEmpire = empire

    def GetSelectedEmpire(self):
        return self.selectedEmpire

    ## Empty
    def EmptyFunction(self):
        pass

    # Persistence
    def GetPersistentObject(self, key):
        try:
            return self.persistentObjects[key]
        except KeyError:
            return None

    def RegisterPersistentObject(self, key, obj):
        self.persistentObjects[key] = obj

    def RemovePersistentObject(self, key):
        del self.persistentObjects[key]

    def OnPhaseChange(self, phase):
        c = dict(self.persistentObjects)
        for i in c.itervalues():
            try:
                i.OnPhaseChange(self, phase)
            except AttributeError:
                pass
