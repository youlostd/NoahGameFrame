# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import wndMgr

import localeInfo
import ui


class MessagePopUp(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.Board = None
        self.nextMove = app.GetGlobalTimeStamp()
        self.isSliding = 0
        self.hasEnded = 0
        self.comp = None
        self.bg = None
        self.messageTitle = None
        self.message = None
        self.yCoord = wndMgr.GetScreenHeight() - 50 + 900
        self.BuildWindow()

    def BuildWindow(self):
        self.Board = ui.BoardWithTitleBar()
        self.Board.SetSize(0, 0)
        self.Board.SetPosition(wndMgr.GetScreenWidth() - 258, wndMgr.GetScreenHeight() - 50 + 900)
        self.Board.SetTitleName('Window')
        self.Board.SetCloseEvent(self.Close)
        self.Board.Show()

        self.comp = Component()
        self.bg = self.comp.ExpandedImage(self.Board, 0, -128 - 500,
                                          'd:/ymir work/ui/public/public_intro_btn/emptyslot_btn.sub')
        self.bg.SetRotation(0.0)
        self.bg.SetScale(float(2), float(2.3))

        self.messageTitle = ui.TextLine()
        self.messageTitle.SetParent(self.Board)
        self.messageTitle.SetPosition(110, 13 - 128 - 500)
        self.messageTitle.SetFontColor(self.comp.RGB(255, 215, 0)[0], self.comp.RGB(255, 215, 0)[1],
                                       self.comp.RGB(255, 215, 0)[2])
        self.messageTitle.SetText(localeInfo.MESSAGE_POPUP_MESSAGE)
        self.messageTitle.Show()
        self.message = self.comp.TextLine(self.Board, '', 60, 34 - 128 - 500, self.comp.RGB(255, 255, 255))

    def PopUp(self, message):
        self.message.SetText(message)
        self.nextMove = app.GetGlobalTimeStamp()
        self.isSliding = 1
        self.hasEnded = 0

    def OnUpdate(self):
        if self.hasEnded:
            return

        if self.isSliding:
            if app.GetGlobalTimeStamp() >= self.nextMove and self.yCoord != wndMgr.GetScreenHeight() - 50 + 500:
                self.Board.SetPosition(wndMgr.GetScreenWidth() - 258, self.yCoord - 10)
                self.yCoord -= 5
                self.nextMove = app.GetGlobalTimeStamp()
            if self.yCoord == wndMgr.GetScreenHeight() - 50 + 500:
                self.isSliding = 0
                self.nextMove = app.GetGlobalTimeStamp() + 3
        else:
            if app.GetGlobalTimeStamp() >= self.nextMove and self.yCoord != wndMgr.GetScreenHeight() - 50 + 900:
                self.Board.SetPosition(wndMgr.GetScreenWidth() - 258, self.yCoord - 10)
                self.yCoord += 5
                self.nextMove = app.GetGlobalTimeStamp()
            if self.yCoord == wndMgr.GetScreenHeight() - 50 + 900:
                self.hasEnded = 1
                self.Close()

    def Close(self):
        self.Board.Hide()


class Component:
    def __init__(self):
        pass

    @staticmethod
    def TextLine(parent, textLineText, x, y, color):
        textLine = ui.TextLine()
        if parent is not None:
            textLine.SetParent(parent)
        textLine.SetPosition(x, y)
        if color is not None:
            textLine.SetFontColor(color[0], color[1], color[2])
        textLine.SetText(textLineText)
        textLine.Show()
        return textLine

    @staticmethod
    def RGB(r, g, b):
        return r * 255, g * 255, b * 255

    @staticmethod
    def ExpandedImage(parent, x, y, img):
        image = ui.ExpandedImageBox()
        if parent is not None:
            image.SetParent(parent)
        image.SetPosition(x, y)
        image.LoadImage(img)
        image.Show()
        return image
