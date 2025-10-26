# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import grp
import wndMgr

import ui
from ui_event import MakeEvent


class PhaseCurtain(ui.Bar):
    def __init__(self):
        ui.Bar.__init__(self, "CURTAIN")
        self.speed = 0.1
        self.curAlpha = 0.0
        self.event = None
        self.eventFadeIn = 0
        self.FadeInFlag = False
        self.SetWindowName("PhaseCurtain")
        self.AddFlag("float")

    def FadeOut(self, event):
        self.curAlpha = 0.0
        self.SetAlpha(self.curAlpha)
        # self.SetTop()
        self.Show()
        self.event = MakeEvent(event)

    def FadeIn(self):
        self.event = 0
        self.FadeInFlag = True

    def SetAlpha(self, alpha):
        self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())

        color = grp.GenerateColor(0.0, 0.0, 0.0, alpha)
        self.SetColor(color)

    def OnUpdate(self):
        if 0 != self.event:
            self.curAlpha += self.speed
            if self.curAlpha >= 1.0:
                self.curAlpha = 1.0

                self.event()

        elif self.FadeInFlag:

            self.curAlpha -= self.speed
            if self.curAlpha <= 0.0:
                self.curAlpha = 0.0
                self.eventFadeIn = 0
                self.FadeInFlag = False
                self.Hide()

        self.SetAlpha(self.curAlpha)
