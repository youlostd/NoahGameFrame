# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import player
from pygame.app import appInst
from pygame.player import playerInst

import localeInfo
import ui
import uiCommon


class GameButtonWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.popup = None
        self.gameButtonDict = {}
        self.__LoadWindow("UIScript/gamewindow.py")

    def __LoadWindow(self, filename):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, filename)
        except Exception as msg:
            logging.exception("GameButtonWindow.LoadScript")
            return False

        try:
            self.gameButtonDict = {
                "STATUS": self.GetChild("StatusPlusButton"),
                "SKILL": self.GetChild("SkillPlusButton"),
                "QUEST": self.GetChild("QuestButton"),
                "BUILD": self.GetChild("BuildGuildBuilding"),
                "EXIT_OBSERVER": self.GetChild("ExitObserver"),
                "GUILDWAR": self.GetChild("GuildWarButton"),
                "DUNGEON": self.GetChild("DungeonRejoinButton"),
            }

            self.gameButtonDict["DUNGEON"].SetEvent(self.__RequestDungeonRejoin)
            self.gameButtonDict["GUILDWAR"].SetEvent(self.__RequestGuildWarEnter)
            self.gameButtonDict["EXIT_OBSERVER"].SetEvent(self.__OnClickExitObserver)

        except Exception as msg:
            logging.exception("GameButtonWindow.RegisterChildren")
            return False

        self.__HideAllGameButton()
        self.SetObserverMode(player.IsObserverMode(), True)


        return True

    def __RequestGuildWarEnter(self):
        questionDialog = uiCommon.QuestionDialog()
        questionDialog.SetText(localeInfo.GAME_GUILDWAR_NOW_JOIN)
        questionDialog.SetAcceptEvent(self.AcceptGuildWarEnter)
        questionDialog.SetCancelEvent(self.ClosePopUpDialog)
        questionDialog.Open()
        self.popup = questionDialog

    def __RequestDungeonRejoin(self):
        questionDialog = uiCommon.QuestionDialog()
        questionDialog.SetText(localeInfo.Get("GAME_DUNGEON_REJOIN"))
        questionDialog.SetAcceptEvent(self.AcceptDungeonRejoin)
        questionDialog.SetCancelEvent(self.CancelDungeonRejoin)
        questionDialog.Open()
        self.popup = questionDialog

    def AcceptGuildWarEnter(self):
        appInst.instance().GetNet().SendChatPacket("/guildwar_request_enter")
        if self.popup:
            self.popup.Close()
        self.popup = None

    def AcceptDungeonRejoin(self):
        appInst.instance().GetNet().SendChatPacket("/dungeon_rejoin 1")
        if self.popup:
            self.popup.Close()
        self.popup = None
        self.HideDungeonRejoinButton()

    def CancelDungeonRejoin(self):
        appInst.instance().GetNet().SendChatPacket("/dungeon_rejoin 0")
        if self.popup:
            self.popup.Close()
        self.popup = None
        self.HideDungeonRejoinButton()

    def ClosePopUpDialog(self):
        if self.popup:
            self.popup.Close()
        self.popup = None

    def ShowGuildWarButton(self):
        self.gameButtonDict["GUILDWAR"].Show()
        logging.debug("GUILDWAR SHOW")

    def HideGuildWarButton(self):
        self.gameButtonDict["GUILDWAR"].Hide()

    def ShowDungeonRejoinButton(self):
        self.gameButtonDict["DUNGEON"].Show()

    def HideDungeonRejoinButton(self):
        self.gameButtonDict["DUNGEON"].Hide()

    def Destroy(self):
        for key in self.gameButtonDict:
            self.gameButtonDict[key].SetEvent(0)

        self.gameButtonDict = {}
        self.popup = None

    def SetButtonEvent(self, name, event):
        try:
            self.gameButtonDict[name].SetEvent(event)
        except Exception as msg:
            logging.exception("GameButtonWindow.SetButtonEvent")
            return

    def ShowBuildButton(self):
        self.gameButtonDict["BUILD"].Show()

    def HideBuildButton(self):
        self.gameButtonDict["BUILD"].Hide()

    def CheckGameButton(self):

        if not self.IsShow():
            return

        statusPlusButton = self.gameButtonDict["STATUS"]
        skillPlusButton = self.gameButtonDict["SKILL"]

        if playerInst().GetPoint(player.STAT) > 0:
            statusPlusButton.Show()
        else:
            statusPlusButton.Hide()

        if self.__IsSkillStat():
            skillPlusButton.Show()
        else:
            skillPlusButton.Hide()


    # Quest Button
    def ShowQuestButton(self):
        self.gameButtonDict["QUEST"].Show()

    def HideQuestButton(self):
        self.gameButtonDict["QUEST"].Hide()

    def ToggleQuestButton(self):
        if self.gameButtonDict["QUEST"].IsShow():
            self.HideQuestButton()
        else:
            self.ShowQuestButton()

    def __IsSkillStat(self):
        if playerInst().GetPoint(player.SKILL_ACTIVE) > 0:
            return True

        return False

    def __OnClickExitObserver(self):
        appInst.instance().GetNet().SendChatPacket("/observer_exit")

    def __HideAllGameButton(self):
        for btn in self.gameButtonDict.values():
            btn.Hide()

    def SetObserverMode(self, isEnable, isButtonShow):
        if isButtonShow:
            if isEnable:
                self.gameButtonDict["EXIT_OBSERVER"].Show()
            else:
                self.gameButtonDict["EXIT_OBSERVER"].Hide()
