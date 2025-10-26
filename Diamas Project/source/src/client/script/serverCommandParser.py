# -*- coding: utf-8 -*-


import logging

from pygame.net import PreserveCommand

import stringCommander


class serverCommandParser(object):
    def __init__(self):
        self.__ServerCommand_Build()

    def __ServerCommand_Build(self):
        serverCommandList = {"DayMode": self.__DayMode_Update, "xmas_snow": self.__XMasSnow_Enable,
                             "xmas_boom": self.__XMasBoom_Enable, "xmas_tree": self.__XMasTree_Enable,
                             "newyear_boom": self.__XMasBoom_Enable, "item_mall": self.__ItemMall_Open,
                             "hide_horse_state": self.__HideHorseState,
                             "guild_war": self.Guild_War_Check,
                             "lover_logout": self.__LoverLogout,
                             "lover_login": self.__LoverLogin,
                             "warboard": self.__WarBoard,
                             }

        self.serverCommander = stringCommander.Analyzer()
        for serverCommandItem in serverCommandList.items():
            self.serverCommander.SAFE_RegisterCallBack(
                serverCommandItem[0], serverCommandItem[1]
            )

    def BINARY_ServerCommand_Run(self, line):
        try:
            return self.serverCommander.Run(line)
        except RuntimeError as msg:
            logging.exception("Failed to run cmd: %s", line)
            return 0

    def __DayMode_Update(self, mode):
        PreserveCommand("PRESERVE_DayMode " + mode)

    def __ItemMall_Open(self):
        PreserveCommand("item_mall")

    def __HideHorseState(self):
        PreserveCommand("hide_horse_state")

    def __LoverLogout(self):
        PreserveCommand("lover_login")

    def __LoverLogin(self):
        PreserveCommand("lover_logout")

    ## �ӽ�
    def __XMasBoom_Enable(self, mode):
        if "1" == mode:
            PreserveCommand("PRESERVE_DayMode dark")
        else:
            PreserveCommand("PRESERVE_DayMode light")

    def __XMasSnow_Enable(self, mode):
        PreserveCommand("xmas_snow " + mode)

    def __XMasTree_Enable(self, grade):
        PreserveCommand("xmas_tree " + grade)

    def Guild_War_Check(self, enable):
        PreserveCommand("guild_war " + enable)

    def __WarBoard(self, arg):
        PreserveCommand("warboard " + arg)

    def PassGuildCommand(self):
        pass


