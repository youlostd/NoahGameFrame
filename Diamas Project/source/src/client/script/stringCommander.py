# -*- coding: utf-8 -*-
from ui_event import MakeCallback


class Analyzer:
    def __init__(self):
        self.cmdDict = {}

    def SAFE_RegisterCallBack(self, cmd, callBackFunc):
        self.cmdDict[cmd] = MakeCallback(callBackFunc)

    def Run(self, line):
        tokens = line.split()
        if not tokens:
            return True

        cmd = tokens.pop(0)

        try:
            callBackFunc = self.cmdDict[cmd]
        except KeyError:
            return False

        callBackFunc(*tokens)
        return True
