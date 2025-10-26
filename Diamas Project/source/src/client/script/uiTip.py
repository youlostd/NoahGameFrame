# -*- coding: utf-8 -*-


import app
import grp
import wndMgr

import localeInfo
import ui


class Lines(ui.Window):
    STATE_NONE = 0
    STATE_SCROLLING = 1

    def __init__(self, width, lines, fontSize=14, scrollTime=3.0, textColor=(1, 1, 1), big=True):
        ui.Window.__init__(self)
        font = localeInfo.UI_DEF_FONT_BOLD if big else localeInfo.UI_DEF_FONT
        self.padding = 5.0
        self.SetSize(width, lines * (fontSize + self.padding))
        self.lineCount = lines
        self.shown = []
        self.lines = []
        for i in range(lines * 2):
            line = ui.TextLine()
            line.SetParent(self)
            line.SetFontName(font)
            line.SetFontColor(*textColor)
            if big:
                line.SetHorizontalAlignCenter()
            self.lines.append(line)
        self.fontSize = fontSize
        self.scrollTime = scrollTime
        self.nextScroll = 0
        self.state = self.STATE_NONE
        self.scrollDif = 0
        self.scrollRate = 1  # *60 px per second
        self.curL = -lines
        self.text = []

    def OnUpdate(self):
        if self.state == self.STATE_SCROLLING and len(self.shown) != 0:
            scroll = min(self.scrollRate, self.scrollDif)
            for l in self.shown:
                x, y = l.GetLocalPosition()
                l.SetPosition(int(x), int(y - scroll))
                l.SetClipRect(-x, -y, self.GetWidth() - x, self.GetHeight() - y)
            self.scrollDif -= scroll
            if self.scrollDif <= 0:
                move = []
                for l in self.shown:
                    x, y = l.GetLocalPosition()
                    if y < -self.fontSize:  # should be out of screen by now
                        move.append(l)
                for l in move:
                    self.shown.remove(l)
                    l.Hide()
                    self.lines.append(l)
                    self.curL -= 1

                self._TryAdd()
                self.state = self.STATE_NONE
                self.nextScroll = app.GetTime() + self.scrollTime

        elif app.GetTime() > self.nextScroll:
            self.scrollDif = self.GetHeight()
            self.state = self.STATE_SCROLLING

    def GetShownCount(self):
        return len(self.shown) + len(self.text)

    def _TryAdd(self):
        added = 0
        for text in self.text:
            if len(self.lines) == 0:
                break

            l = self.lines.pop()
            l.SetText(text)
            l.Show()
            y = (self.lineCount + self.curL) * (self.fontSize + self.padding) + (
                self.scrollDif if self.scrollDif == self.GetHeight() else 0)
            l.SetPosition(0, int(y))
            l.SetClipRect(0, 0, self.GetWidth(), self.GetHeight() - y)
            self.shown.append(l)
            self.curL += 1
            added += 1

        del self.text[:added]

    def AddText(self, text):
        self.text.append(text)
        if len(self.shown) == 0:
            self.state = self.STATE_NONE
            self.nextScroll = app.GetTime() + self.scrollTime
        if self.state != self.STATE_SCROLLING:
            self._TryAdd()

    def SetPosition(self, x, y):
        ui.Window.SetPosition(self, int(x), int(y))
        for l in self.shown + self.lines:
            l.SetClipRect(x, y, x + self.GetWidth(), y + self.GetHeight())

class MissionLines(ui.Window):
    STATE_NONE = 0
    STATE_SCROLLING = 1

    def __init__(self, width, fontSize=12, textColor=(1, 1, 1), big=False):
        ui.Window.__init__(self)
        font = localeInfo.UI_DEF_FONT_BOLD if big else localeInfo.UI_DEF_FONT
        self.padding = 5.0
        self.SetSize(width, 2 * (fontSize + self.padding))

        line = ui.TextLine()
        line.SetParent(self)
        line.SetFontName(font)
        line.SetFontColor(*textColor)
        line.SetHorizontalAlignCenter()
        self.missionMessage = line
        self.missionMessage.Show()

        line = ui.TextLine()
        line.SetParent(self)
        line.SetFontName(font)
        line.SetFontColor(*textColor)
        self.subMissionMessage = line
        self.subMissionMessage.Show()

        self.fontSize = fontSize


    def SetMissionMessage(self, text):
        self.missionMessage.SetText(text)

    def SetSubMissionMessage(self, text):
        self.subMissionMessage.SetText(text)
        self.subMissionMessage.SetPosition(self.missionMessage.GetGlobalPosition()[0] + self.missionMessage.GetWidth(), 0)

    def SetPosition(self, x, y):
        ui.Window.SetPosition(self, int(x), int(y))
        #self.missionMessage.SetClipRect(x, y, x + self.GetWidth(), y + self.GetHeight())
        #self.subMissionMessage.SetClipRect(x, y, x + self.GetWidth(), y + self.GetHeight())



class TipBoard(ui.Bar):
    def __init__(self):
        ui.Bar.__init__(self)

        self.AddFlag("not_pick")
        self.tipList = []
        self.curPos = 0
        self.dstPos = 0
        self.nextScrollTime = 0

        self.width = 370

        self.SetPosition(0, 70)
        self.SetSize(370, 20)
        self.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, 0.5))
        self.SetHorizontalAlignCenter()

        self.lines = Lines(370, 1)
        self.lines.SetParent(self)
        self.lines.SetPosition(3, 2)
        self.lines.Show()

    def SetTip(self, text):
        self.lines.AddText(text)

        if not self.IsShow():
            self.Show()

    def OnUpdate(self):
        if self.lines.GetShownCount() == 0:
            self.Hide()
            return


class BigBoard(ui.Bar):
    SCROLL_WAIT_TIME = 5.0
    TIP_DURATION = 10.0
    FONT_WIDTH = 18
    FONT_HEIGHT = 18
    LINE_WIDTH = 500
    LINE_HEIGHT = FONT_HEIGHT + 5
    STEP_HEIGHT = LINE_HEIGHT * 2
    LINE_CHANGE_LIMIT_WIDTH = 350

    FRAME_IMAGE_FILE_NAME_LIST = [
        "d:/ymir work/ui/game/oxevent/frame_0.sub",
        "d:/ymir work/ui/game/oxevent/frame_1.sub",
        "d:/ymir work/ui/game/oxevent/frame_2.sub",
    ]

    FRAME_IMAGE_STEP = 256

    FRAME_BASE_X = -20
    FRAME_BASE_Y = -12

    def __init__(self):
        ui.Bar.__init__(self)

        self.AddFlag("not_pick")
        self.tipList = []
        self.curPos = 0
        self.dstPos = 0
        self.nextScrollTime = 0

        self.SetPosition(0, 150)
        self.SetSize(512, 55)
        self.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, 0.5))
        self.SetHorizontalAlignCenter()

        self.lines = Lines(510, 2, 18, 5, (0.94901, 0.90588, 0.75686), True)
        self.lines.SetParent(self)
        self.lines.SetPosition(6, 11)
        self.lines.Show()

        self.__LoadFrameImages()

    def __LoadFrameImages(self):
        x = self.FRAME_BASE_X
        y = self.FRAME_BASE_Y
        self.imgList = []
        for imgFileName in self.FRAME_IMAGE_FILE_NAME_LIST:
            self.imgList.append(self.__LoadImage(x, y, imgFileName))
            x += self.FRAME_IMAGE_STEP

    def __LoadImage(self, x, y, fileName):
        img = ui.ImageBox()
        img.SetParent(self)
        img.AddFlag("not_pick")
        img.LoadImage(fileName)
        img.SetPosition(x, y)
        img.Show()
        return img

    def SetTip(self, text):
        if not app.IsVisibleNotice():
            return
        self.lines.AddText(text)

        if not self.IsShow():
            self.Show()

    def OnUpdate(self):
        if self.lines.GetShownCount() == 0:
            self.Hide()
            return


if app.ENABLE_OX_RENEWAL:
    class BigBoardControl(ui.Bar):

        FONT_WIDTH = 18
        FONT_HEIGHT = 18
        LINE_WIDTH = 500
        LINE_HEIGHT = FONT_HEIGHT + 5
        STEP_HEIGHT = LINE_HEIGHT * 2
        LINE_CHANGE_LIMIT_WIDTH = 450
        BIG_TEXTBAR_MAX_HIGHT = 1000

        FRAME_IMAGE_FILE_NAME_LIST = [
            "season1/interface/oxevent/frame_new_0.sub",
            "season1/interface/oxevent/frame_new_1.sub",
        ]

        FRAME_IMAGE_STEP = 256

        FRAME_BASE_X = -20
        FRAME_BASE_Y = -12

        def __init__(self):
            ui.Bar.__init__(self)

            self.AddFlag("not_pick")
            self.tipList = []
            self.curPos = 0
            self.dstPos = 0
            self.nextScrollTime = 0
            self.scrollstop = 0
            self.pretexsize = 0
            self.nexttextsize = 0
            self.changeline = 0
            self.addtipListcount = 0

            self.SetPosition(0, 150)
            self.SetSize(512, 55)
            self.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, 0.5))
            self.SetHorizontalAlignCenter()

            self.__CreateTextBar()
            self.__LoadFrameImages()

        def __LoadFrameImages(self):
            x = self.FRAME_BASE_X
            y = self.FRAME_BASE_Y
            self.imgList = []
            for imgFileName in self.FRAME_IMAGE_FILE_NAME_LIST:
                self.imgList.append(self.__LoadImage(x, y, imgFileName))

        def __LoadImage(self, x, y, fileName):
            img = ui.ImageBox()
            img.SetParent(self)
            img.AddFlag("not_pick")
            img.LoadImage(fileName)
            img.SetPosition(x, y)
            img.Show()
            return img

        def __CreateTextBar(self):

            x, y = self.GetGlobalPosition()

            self.textBar = BigTextBar(self.LINE_WIDTH, self.BIG_TEXTBAR_MAX_HIGHT, self.FONT_HEIGHT)
            self.textBar.SetParent(self)
            self.textBar.SetPosition(6, 8)
            self.textBar.SetTextColor(242, 231, 193)
            self.textBar.SetClipRect(0, y + 8, wndMgr.GetScreenWidth(), y + 8 + self.STEP_HEIGHT)
            self.textBar.Show()

        def __CleanOldTip(self):
            self.tipList = []
            self.textBar.ClearBar()
            self.Hide()

        def __RefreshBoard(self):

            self.textBar.ClearBar()
            preaddtipListcount = self.addtipListcount
            self.addtipListcount = 0

            index = 0
            for text in self.tipList:
                (text_width, text_height) = self.textBar.GetTextExtent(text)

                if index == 0:
                    self.textBar.TextOut((500 - text_width) / 2, index * self.LINE_HEIGHT, " ")
                    index += 1
                    self.addtipListcount += 1

                if index == (
                        len(self.tipList) - (len(self.tipList) - self.pretexsize)) + 1 and not self.pretexsize == 0:
                    self.textBar.TextOut((500 - text_width) / 2, index * self.LINE_HEIGHT, " ")
                    index += 1
                    self.textBar.TextOut((500 - text_width) / 2, index * self.LINE_HEIGHT, " ")
                    index += 1
                    self.addtipListcount += 2

                self.textBar.TextOut((500 - text_width) / 2, index * self.LINE_HEIGHT, text)
                index += 1

            if self.tipList:
                index += 1
                self.addtipListcount += 1
                self.textBar.TextOut((500 - text_width) / 2, index * self.LINE_HEIGHT, " ")

        def SetTip(self, text):

            if not app.IsVisibleNotice():
                return

            if text == "#start" or text == "#end" or text == "#send":
                pass
            else:
                self.__AppendText(text)

            if text == "#start" or text == "#send":

                if text == "#send" and self.pretexsize == 0:
                    text = "#end"

                self.__RefreshBoard()

                if text == "#start":
                    self.STEP_HEIGHT = (self.LINE_HEIGHT * (len(self.tipList) + self.addtipListcount))
                    self.pretexsize = len(self.tipList)

                    x, y = self.GetGlobalPosition()
                    self.textBar.SetClipRect(0, y + 8, wndMgr.GetScreenWidth(), y + 8 + self.STEP_HEIGHT)
                    self.SetSize(512, 10 + (self.LINE_HEIGHT * (len(self.tipList) + self.addtipListcount)) + 10)
                    self.imgList[1].SetPosition(self.FRAME_BASE_X, self.STEP_HEIGHT + 10)

                    self.scrollstop = 0
                    self.changeline = 1

                if text == "#send":
                    self.scrollstop = 1
                    self.nexttextsize = len(self.tipList) - self.pretexsize

                if not self.IsShow():
                    self.curPos = -self.STEP_HEIGHT
                    self.dstPos = -self.STEP_HEIGHT
                    self.textBar.SetPosition(3, 8 - self.curPos)
                    self.Show()

            if text == "#end":
                self.curPos = -self.STEP_HEIGHT
                self.dstPos = -self.STEP_HEIGHT
                self.textBar.SetPosition(3, 8 - self.curPos)
                self.textcount = 0
                self.changeline = 0
                self.pretexsize = 0
                self.nexttextsize = 0
                self.addtipListcount = 0
                self.__CleanOldTip()

        def __AppendText(self, text):
            prevPos = 0
            while 1:
                curPos = text.find(" ", prevPos)
                if curPos < 0:
                    break

                (text_width, text_height) = self.textBar.GetTextExtent(text[:curPos])
                if text_width > self.LINE_CHANGE_LIMIT_WIDTH:
                    self.tipList.append(text[:prevPos])

                    (text_width, text_height) = self.textBar.GetTextExtent(text[prevPos:])
                    if text_width > self.LINE_CHANGE_LIMIT_WIDTH:
                        text = text[prevPos:]
                        if text[:prevPos] == "":
                            prevPos = curPos + 1
                        else:
                            prevPos = 0
                    else:
                        text = text[prevPos:]
                        break
                else:
                    prevPos = curPos + 1

            self.tipList.append(text)

        def OnUpdate(self):

            if not self.tipList:
                self.Hide()
                return

            if self.changeline:
                self.changeline = 0
                self.dstPos = self.curPos + self.STEP_HEIGHT

            if self.dstPos > self.curPos and self.curPos <= -1 and not self.scrollstop:
                self.curPos += 1
                self.textBar.SetPosition(3, 8 - self.curPos)

            if self.scrollstop:
                self.curPos += 1
                self.textBar.SetPosition(3, 8 - self.curPos)

            if self.curPos > (self.pretexsize + 2) * self.LINE_HEIGHT:
                self.pretexsize = (self.pretexsize + self.nexttextsize)
                self.scrollstop = 0

if app.ENABLE_12ZI:
    class MissionBoard(ui.Bar):
        FONT_HEIGHT = 15
        LINE_HEIGHT = FONT_HEIGHT + 5
        STEP_HEIGHT = LINE_HEIGHT + 5
        LONG_TEXT_START_X = 300
        SCREEN_WIDTH = wndMgr.GetScreenWidth()

        def __init__(self):
            ui.Bar.__init__(self)

            self.AddFlag("not_pick")
            self.missionText = None
            self.missionFullText = None
            self.curPos = 0
            self.dstPos = -5
            self.nextScrollTime = 0
            self.flowMode = False
            self.ScrollStartTime = 0.0

            self.SetPosition(0, 100)
            self.SetSize(self.SCREEN_WIDTH, 35)
            self.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, 0.5))
            self.SetHorizontalAlignCenter()

            self.lines = MissionLines(self.SCREEN_WIDTH, self.FONT_HEIGHT, (0.94901, 0.90588, 0.75686), True)
            self.lines.SetParent(self)
            self.lines.SetPosition(6, 8)
            self.lines.Show()


        def CleanMission(self):
            self.Hide()

        def SetMission(self, text):
            self.lines.SetMissionMessage(text)

            if not self.IsShow():
                self.Show()

        def SetSubMission(self, text):
            self.lines.SetSubMissionMessage(text)
