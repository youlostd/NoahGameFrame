# -*- coding: utf-8 -*-
from _weakref import proxy

import app
from pygame.app import appInst

import ui
import uiOptionWindow
import uiSwitchChannel
from ui_event import MakeEvent


class SystemDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.gameOptionDlg = None
        self.SwitchChannelDlg = None
        self.interface = None
        self.stream = None

    def BindInterface(self, interface):
        self.interface = proxy(interface)

    def BindStream(self, stream):
        self.stream = stream

    def LoadDialog(self):
        self.__LoadSystemMenu_Default()

    def __LoadSystemMenu_Default(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/SystemDialog.py")

        self.GetChild("switch_channel_button").SetEvent(self.__ClickSwitchChannelButton)

        self.GetChild("game_option_button").SetEvent(self.__ClickGameOptionButton)
        self.GetChild("change_button").SetEvent(self.__ClickChangeCharacterButton)
        self.GetChild("logout_button").SetEvent(self.__ClickLogOutButton)
        self.GetChild("exit_button").SetEvent(self.__ClickExitButton)
        self.GetChild("cancel_button").SetEvent(self.Close)

        self.GetChild("mall_button").SetEvent(self.__ClickInGameShopButton)
        # self.GetChild("switchbot_button").SetEvent(self.__ClickSwitchbotButton)
        self.GetChild("unstuck_button").SetEvent(self.unstuck_button)

    def unstuck_button(self):
        appInst.instance().GetNet().SendChatPacket("/unstuck")

    def Destroy(self):
        self.ClearDictionary()

        if self.gameOptionDlg:
            self.gameOptionDlg.Destroy()

        if self.SwitchChannelDlg:
            self.SwitchChannelDlg.Destroy()

        self.gameOptionDlg = None
        self.SwitchChannelDlg = None
        self.interface = None

    def OpenDialog(self):
        self.Show()

    def __ClickChangeCharacterButton(self):
        self.Close()

        appInst.instance().GetNet().ExitGame()

    def __ClickLogOutButton(self):
        self.Close()
        appInst.instance().GetNet().LogOutGame()

    def __ClickExitButton(self):
        self.Close()
        appInst.instance().GetNet().ExitApp()

    def __ClickGameOptionButton(self):
        self.Close()

        if not self.gameOptionDlg:
            self.gameOptionDlg = uiOptionWindow.OptionsWindow()
            self.gameOptionDlg.BindInterface(self.interface)
            self.gameOptionDlg.OnChangePKMode()

        self.gameOptionDlg.Show()

    def __ClickSwitchChannelButton(self):
        self.Close()

        if not self.SwitchChannelDlg:
            self.SwitchChannelDlg = uiSwitchChannel.MoveChannelDialog(self.stream)

        self.SwitchChannelDlg.Show()

    def __ClickInGameShopButton(self):
        self.Close()

        appInst.instance().GetNet().SendChatPacket("/in_game_mall")

    def Close(self):
        self.Hide()
        return True

    def OnBlockMode(self, mode):
        if self.gameOptionDlg:
            self.gameOptionDlg.OnBlockMode(mode)
        # self.optionDialog.OnBlockMode(mode)

    def OnChangePKMode(self):
        if self.gameOptionDlg:
            self.gameOptionDlg.OnChangePKMode()
        # self.optionDialog.OnChangePKMode()

    def OnPressExitKey(self):
        self.Close()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
