# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
from pygame.app import appInst

import localeInfo
import ui
from ui_event import Event


class SelectStorageDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.AddFlag("animated_board")
        self.selectStorageBoard = None
        self.blackBoard = None
        self.acceptButton = None
        self.closeButton = None

        self.storageButtons = []
        self.storageNames = []
        self.storageIndex = 0
        self.storageData = [[localeInfo.SELECT_STORAGE_SAFEBOX, "/click_safebox"],
                            [localeInfo.SELECT_STORAGE_MALL, "/click_mall"]]

        self.__LoadWindow()
        self.Open()

    def Destroy(self):
        self.Close()
        self.ClearDictionary()

    def __LoadWindow(self):
        pyScrLoader = ui.PythonScriptLoader()
        try:
            pyScrLoader.LoadScriptFile(self, "UIScript/SelectStorageDialog.py")
        except IOError:
            import logging
            logging.exception("MoveChannelDialog.__LoadDialog.LoadObject")

    def Open(self):
        try:
            GetObject = self.GetChild
            GetObject("SelectStorageBoard").SetCloseEvent(self.Close)

            self.selectStorageBoard = GetObject("SelectStorageBoard")
            self.blackBoard = GetObject("BlackBoard")

            for i, channel in enumerate(self.storageData):
                btn = ui.MakeButton(self.blackBoard, 6, 6 + (28 * i), "", "d:/ymir work/ui/game/myshop_deco/", "select_btn_01.sub", "select_btn_02.sub", "select_btn_03.sub")
                btn.SetText(channel[0])
                btn.SetEvent(Event(self.AcceptButton, i))
                # btn.SetButtonScale(0.80, 0.85)
                # btn.SetPackedFontColor(0xffffc539)

                self.storageButtons.append(btn)
                self.storageButtons[i].Show()
                self.storageNames.append(channel[0])

            self.SetSize(200, len(self.storageButtons) * 28 + 94 + 9)
            self.selectStorageBoard.SetSize(200, len(self.storageButtons) * 28 + 74 + 9)
            self.blackBoard.SetSize(161, len(self.storageButtons) * 28 + 8)

        except:
            import logging
            logging.exception("MoveChannelDialog.Open.BindObject")

        ui.ScriptWindow.Show(self)

    def AcceptButton(self, storageIndex):
        self.Close()
        appInst.instance().GetNet().SendChatPacket(self.storageData[storageIndex][1])

    def Close(self):
        self.Hide()
        return True

    def Show(self):
        ui.ScriptWindow.Show(self)
        self.SetCenterPosition()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.OnPressEscapeKey()
            return True
        return False

    def OnPressEscapeKey(self):
        self.Close()
        return True
