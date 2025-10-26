# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
from pygame.app import appInst

import serverInfo
import ui
from ui_event import Event


class MoveChannelDialog(ui.ScriptWindow):
    def __init__(self, stream):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.stream = stream

        self.moveChannelBoard = None
        self.blackBoard = None
        self.acceptButton = None
        self.closeButton = None

        self.channelIndex = self.stream.channelIndex
        if self.channelIndex > 6:
            self.channelIndex = 1

        self.moveChannelIndex = 0

        self.channelButtons = []
        self.channelNames = []
        self.channelStates = []

        self.channelData = serverInfo.Get()[self.stream.GetServerName()]["channel"]

        self.__LoadWindow()
        self.Open()

    def Destroy(self):
        self.Close()
        self.ClearDictionary()

    def __LoadWindow(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/MoveChannelDialog.py")

    def Open(self):
        self.__LoadWindow()
        GetObject = self.GetChild
        GetObject("MoveChannelBoard").SetCloseEvent(self.Close)

        self.moveChannelBoard = GetObject("MoveChannelBoard")
        self.blackBoard = GetObject("BlackBoard")

        for i, channel in enumerate(self.channelData):
            btn = ui.MakeButton(
                self.blackBoard,
                6,
                6 + (28 * i),
                "",
                "d:/ymir work/ui/game/myshop_deco/",
                "select_btn_01.sub",
                "select_btn_02.sub",
                "select_btn_03.sub",
            )
            channelName = "CH" + str(i + 1)
            btn.SetText(channelName)
            btn.SetEvent(Event(self.AcceptButton, i))
            self.channelButtons.append(btn)
            self.channelButtons[i].Show()

            self.channelNames.append(channelName)

        self.SetSize(200, len(self.channelButtons) * 28 + 94 + 9)
        self.moveChannelBoard.SetSize(195, len(self.channelButtons) * 28 + 74 + 9)
        self.blackBoard.SetSize(161, len(self.channelButtons) * 28 + 8)
        ui.ScriptWindow.Show(self)

    def AcceptButton(self, channelIndex):
        self.SelectChannel(channelIndex)
        self.Close()
        appInst.instance().GetNet().SendChatPacket(
            "/change_channel %d" % int(channelIndex + 1)
        )

    def Close(self):
        self.Hide()
        return True

    def SelectChannel(self, idx):
        if idx == 99:
            for btn in self.channelButtons:
                btn.Down()
                btn.Disable()
            return

        for btn in self.channelButtons:
            btn.SetUp()
            btn.Enable()

        self.channelButtons[idx - 1].Down()
        self.channelButtons[idx - 1].Disable()

    def Show(self):
        self.SelectChannel(self.stream.channelIndex + 1)
        ui.ScriptWindow.Show(self)
        self.SetCenterPosition()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
