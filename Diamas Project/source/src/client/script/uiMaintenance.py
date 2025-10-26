# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import wndMgr

import localeInfo
import ui
import uiScriptLocale


class MaintenanceInfoBoard(ui.ThinBoardGold):
    BOARD_WIDTH = 580
    BOARD_HEIGHT = 45

    def __init__(self):
        ui.ThinBoardGold.__init__(self)
        self.AddFlag("float")
        self.timeLimitEnd = 0
        self.duration = 0

        timeTextLine = ui.ExtendedTextLine()
        timeTextLine.SetParent(self)
        timeTextLine.Big()
        timeTextLine.SetPosition(0, -5)
        timeTextLine.SetVerticalAlignCenter()
        timeTextLine.SetHorizontalAlignCenter()
        timeTextLine.Show()
        self.timeTextLine = timeTextLine

        self.SetPosition(wndMgr.GetScreenWidth() / 2 - self.BOARD_WIDTH / 2, 2)
        self.SetSize(self.BOARD_WIDTH, self.BOARD_HEIGHT)

    def SetTimeLimit(self, timeLimit, duration=0):
        if timeLimit > 0:
            self.timeLimitEnd = timeLimit
            self.duration = duration
            self.__Refresh()
        else:
            self.timeLimitEnd = 0
            self.duration = 0
            self.Hide()

    def UpdatePos(self, y):
        self.SetPosition(wndMgr.GetScreenWidth() / 2 - self.BOARD_WIDTH / 2, y)

    def __Refresh(self):
        self.timeTextLine.SetText(uiScriptLocale.MAINTENANCE_TITLE + uiScriptLocale.MAINTENANCE_LEFT_TIME.format(
            localeInfo.SecondToDHMS(self.timeLimitEnd), localeInfo.SecondToDHMS(self.duration)))

    def Close(self):
        self.Hide()
