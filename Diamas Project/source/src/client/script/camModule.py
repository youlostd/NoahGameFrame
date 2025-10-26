# -*- coding: utf-8 -*-

##################################################
## NAME: Camy
## TYPE: Metin2 Module
## AUTHOR: Yiv
##
## COPYRIGHT (C) 2015 Yiv
##################################################

import logging
import math
import os
from _weakref import proxy

import app
import grp

import colorInfo
import ui
from ui_event import Event


class CamyListBox(ui.Window):
    class Item(ui.Window):
        def __init__(self):
            ui.Window.__init__(self)


        def SetParent(self, parent):
            ui.Window.SetParent(self, parent)
            self.parent = proxy(parent)

        def OnMouseLeftButtonDown(self):
            self.parent.SelectItem(self)
            return True

        def OnRender(self):
            if self.parent.GetSelectedItem() == self:
                self.OnSelectedRender()

        def OnSelectedRender(self):
            x, y = self.GetGlobalPosition()
            grp.SetColor(colorInfo.UI_COLOR_SELECTED)
            grp.RenderBar(x, y, self.GetWidth(), self.GetHeight())

    def __init__(self):
        ui.Window.__init__(self)

        self.viewItemCount = 10
        self.basePos = 0
        self.itemHeight = 16
        self.itemStep = 20
        self.selItem = 0
        self.selItemIdx = 0
        self.itemList = []
        self.onSelectItemEvent = None

        self.itemWidth = 100

        self.scrollBar = None
        self.__UpdateSize()

    def __UpdateSize(self):
        height = self.itemStep * self.__GetViewItemCount()

        self.SetSize(self.itemWidth, height)

    def IsEmpty(self):
        if len(self.itemList) == 0:
            return 1
        return 0

    def SetItemStep(self, itemStep):
        self.itemStep = itemStep
        self.__UpdateSize()

    def SetItemSize(self, itemWidth, itemHeight):
        self.itemWidth = itemWidth
        self.itemHeight = itemHeight
        self.__UpdateSize()

    def SetViewItemCount(self, viewItemCount):
        self.viewItemCount = viewItemCount

    def SetSelectEvent(self, event):
        self.onSelectItemEvent = event

    def SetBasePos(self, basePos):
        for oldItem in self.itemList[self.basePos:self.basePos + self.viewItemCount]:
            oldItem.Hide()

        self.basePos = basePos

        pos = basePos
        for newItem in self.itemList[self.basePos:self.basePos + self.viewItemCount]:
            (x, y) = self.GetItemViewCoord(pos, newItem.GetWidth())
            newItem.SetPosition(x, y)
            newItem.Show()
            pos += 1

    def GetItemIndex(self, argItem):
        return self.itemList.index(argItem)

    def GetSelectedItem(self):
        return self.selItem

    def GetSelectedItemIndex(self):
        return self.selItemIdx

    def SelectIndex(self, index):

        if index >= len(self.itemList) or index < 0:
            self.selItem = None
            self.selItemIdx = None
            return

        try:
            self.selItem = self.itemList[index]
            self.selItemIdx = index
            self.onSelectItemEvent(self.selItem)
        except:
            pass

    def ReplaceItemAtIndex(self, index, item):
        item.SetParent(self)
        item.SetSize(self.itemWidth, self.itemHeight)

        if self.__IsInViewRange(index):
            (x, y) = self.GetItemViewCoord(index, item.GetWidth())
            item.SetPosition(x, y)
            item.Show()
        else:
            item.Hide()

        self.itemList[index] = item

    def GetItemAtIndex(self, index):
        if index > (len(self.itemList) - 1):
            return None
        return self.itemList[index]

    def SelectItem(self, selItem):
        self.selItem = selItem
        self.selItemIdx = self.GetItemIndex(selItem)
        self.onSelectItemEvent(selItem)

    def RemoveAllItems(self):
        self.selItem = None
        self.selItemIdx = None
        self.itemList = []

        if self.scrollBar:
            self.scrollBar.SetPos(0)

    def RemoveItem(self, delItem):
        if delItem == self.selItem:
            self.selItem = None
            self.selItemIdx = None

        self.itemList.remove(delItem)

    def AppendItem(self, newItem):
        newItem.SetParent(self)
        newItem.SetSize(self.itemWidth, self.itemHeight)

        pos = len(self.itemList)
        if self.__IsInViewRange(pos):
            (x, y) = self.GetItemViewCoord(pos, newItem.GetWidth())
            newItem.SetPosition(x, y)
            newItem.Show()
        else:
            newItem.Hide()

        self.itemList.append(newItem)

    def SetScrollBar(self, scrollBar):
        scrollBar.SetScrollEvent(self.__OnScroll)
        self.scrollBar = scrollBar

    def __OnScroll(self):
        self.SetBasePos(int(self.scrollBar.GetPos() * self.__GetScrollLen()))

    def __GetScrollLen(self):
        scrollLen = self.__GetItemCount() - self.__GetViewItemCount()
        if scrollLen < 0:
            return 0

        return scrollLen

    def __GetViewItemCount(self):
        return self.viewItemCount

    def __GetItemCount(self):
        return len(self.itemList)

    def GetItemCount(self):
        return len(self.itemList)

    def GetItemViewCoord(self, pos, itemWidth):
        return (0, (pos - self.basePos) * self.itemStep)

    def __IsInViewRange(self, pos):
        if pos < self.basePos:
            return 0
        if pos >= self.basePos + self.viewItemCount:
            return 0
        return 1


class KeyFrame(CamyListBox.Item):
    def __init__(self, _x, _y, _z, _zoom, _pitch, _rotation, _time, _ease, _name=""):
        CamyListBox.Item.__init__(self)

        self.name = ui.TextLine()
        self.name.SetParent(self)
        self.name.SetPosition(4, 1)
        self.name.SetText(_name)
        self.name.Show()
        self.name.OnMouseLeftButtonDown = self.OnMouseLeftButtonDown

        self.X = _x
        self.Y = _y
        self.Z = _z
        self.ZOOM = _zoom
        self.PITCH = _pitch
        self.ROTATION = _rotation
        self.TIME = _time
        self.EASE = _ease


class ScriptManager(ui.ScriptWindow):
    def __init__(self, camyInstance, iSaveLoad=0):
        ui.ScriptWindow.__init__(self)
        self.camyInstance = camyInstance
        self.InitializeUI(iSaveLoad)


    def InitializeUI(self, iSaveLoad):
        try:
            ui.PythonScriptLoader().LoadScriptFile(self, "uiscript/camModuleScriptManagerInterface.py")
            getObject = self.GetChild

            btnDone = getObject("btnDone")
            if iSaveLoad == 0:
                getObject("board").SetTitleName("Script speichern")
                btnDone.SetText("Speichern")
                btnDone.SetEvent(Event(self.Done, self.camyInstance.SaveScript))
            else:
                getObject("board").SetTitleName("Script laden")
                btnDone.SetText("Laden")
                btnDone.SetEvent(Event(self.Done, self.camyInstance.LoadScript))

            self.fileName = getObject("editlineScriptName")
        except:
            logging.exception("Failed to load and/or bind interface")

    def Done(self, func):
        if self.fileName.GetText() != "":
            func(self.fileName.GetText())
        self.Hide()


class Interface(ui.ScriptWindow):
    EASE_TYPES_NAMES = ["Linear", "Quadratisch", "Kubisch", "Quartisch", "Quintisch", "Sinus", "Exponentiell",
                        "Zirkular"]
    EASE_TYPES = ["linear", "quadratic", "cubic", "quartic", "quintic", "circular", "exponential", "sinusoidal"]

    def __init__(self, inst=None):
        ui.ScriptWindow.__init__(self)
        self.currentlySelectedKeyFrameIndex = None
        self.currentlySelectedKeyFrame = None
        self.currentlySelectedKeyFrameHelp = None
        self.inst = inst
        self.downKey = 0
        self.firstKeyDown = 0
        self.nextUpdateTime = 0
        self.lastChange = ""
        self.SetPlayState(0)
        self.CheckStructure()
        self.camyClient = 0
        if hasattr(app, "CAMY_COMPATIBLE_CLIENT"):
            logging.debug("##############################")
            logging.debug("This client is ready for Camy!")
            logging.debug("##############################")
            self.camyClient = 1
        self.InitializeUI()


    def Show(self):
        ui.ScriptWindow.Show(self)
        if self.inst:
            self.inst.interface.HideAllWindows()
            self.inst.interface.wndGameButton.Hide()
            self.inst.interface.HideAllQuestButton()
            self.inst.affectShower.Hide()
        if self.camyClient == 1:
            app.SetCamyActivityState(1)

    def Close(self):
        if self.camyClient == 1:
            app.SetCamyActivityState(0)
        self.Hide()
        app.SetDefaultCamera()
        if self.inst:
            self.inst.interface.ShowAllWindows()
            self.inst.interface.wndGameButton.Show()
            self.inst.interface.ShowAllQuestButton()
            self.inst.affectShower.Show()

    def CheckStructure(self):
        if not os.path.isdir("camy"):
            os.mkdir("camy")
        if not os.path.isdir("camy/spl"):
            os.mkdir("camy/spl")

    def InitializeUI(self):
        try:
            ui.PythonScriptLoader().LoadScriptFile(self, "uiscript/camModuleInterface.py")
            getObject = self.GetChild
            self.board = getObject("board")
            self.board.SetCloseEvent(self.Close)

            getObject("kfDelete").SetEvent(self.DeleteKeyFrame)
            getObject("kfAdd").SetEvent(self.AddNewKeyFrame)

            self.kfList = CamyListBox()
            self.kfList.SetParent(getObject("kfBackgroundBar"))
            self.kfList.SetPosition(0, 0)
            self.kfList.SetSize(129, 400)
            self.kfList.SetViewItemCount(19)
            self.kfList.SetScrollBar(getObject("kfScroll"))
            self.kfList.SetSelectEvent(self.SelectKeyFrame)
            self.kfList.Show()

            self.kfPosXValue = getObject("kfPosXValue")
            self.kfPosXSlider = getObject("kfPosXSlider")
            self.kfPosXSlider.SetEvent(self.SliderChangeX)
            self.kfPosYValue = getObject("kfPosYValue")
            self.kfPosYSlider = getObject("kfPosYSlider")
            self.kfPosYSlider.SetEvent(self.SliderChangeY)
            self.kfPosZValue = getObject("kfPosZValue")
            self.kfPosZSlider = getObject("kfPosZSlider")
            self.kfPosZSlider.SetEvent(self.SliderChangeZ)
            self.kfPosZoomValue = getObject("kfPosZoomValue")
            self.kfPosZoomSlider = getObject("kfPosZoomSlider")
            self.kfPosZoomSlider.SetEvent(self.SliderChangeZoom)
            self.kfPosPitchValue = getObject("kfPosPitchValue")
            self.kfPosPitchSlider = getObject("kfPosPitchSlider")
            self.kfPosPitchSlider.SetEvent(self.SliderChangePitch)
            self.kfPosRotationValue = getObject("kfPosRotationValue")
            self.kfPosRotationSlider = getObject("kfPosRotationSlider")
            self.kfPosRotationSlider.SetEvent(self.SliderChangeRotation)
            self.kfSettingsTimeValue = getObject("kfSettingsTimeValue")
            self.kfSettingsTimeSlider = getObject("kfSettingsTimeSlider")
            self.kfSettingsTimeSlider.SetEvent(self.SliderChangeTime)

            self.rbtngrpEaseTypes = ui.RadioButtonGroup()
            for i in xrange(0, len(self.EASE_TYPES)):
                self.rbtngrpEaseTypes.AddButton(getObject("kfBtnEase_" + str(i)), self.ChangeEaseType, None)
                self.rbtngrpEaseTypes.SetText(i, self.EASE_TYPES_NAMES[i])
            self.rbtngrpEaseTypes.Show()

            self.kfBtnResetChanges = getObject("kfBtnResetChanges")
            self.kfBtnResetChanges.SetEvent(self.ResetChanges)
            self.kfBtnApplyChanges = getObject("kfBtnApplyChanges")
            self.kfBtnApplyChanges.SetEvent(self.ApplyChanges)
            getObject("btnPlay").SetEvent(Event(self.PlaySequence, 0))
            getObject("btnPlaySmooth").SetEvent(Event(self.PlaySequence, 1))
            getObject("btnSaveScript").SetEvent(Event(self.ShowScriptManager, 0))
            getObject("btnLoadScript").SetEvent(Event(self.ShowScriptManager, 1))

            if self.camyClient == 1:
                camyCompatibleInfo = getObject("txtCamyCompatible")
                camyCompatibleInfo.SetText("Camy ist kompatibel!")
                camyCompatibleInfo.SetFontColor(0.0, 1.0, 0.0)
        except:
            logging.exception("Failed to load and/or bind interface")

    def DeleteKeyFrame(self):
        if self.GetCurrentlySelectedKeyFrame() == None:
            return

        self.currentlySelectedKeyFrameIndex = None
        self.currentlySelectedKeyFrame = None
        self.currentlySelectedKeyFrameHelp = None

        idx = self.kfList.GetSelectedItemIndex()
        newList = []
        for i in xrange(0, self.kfList.GetItemCount()):
            if i == idx:
                continue
            it = self.kfList.GetItemAtIndex(i)
            point = KeyFrame(it.X, it.Y, it.Z, it.ZOOM, it.PITCH, it.ROTATION, it.TIME, it.EASE,
                             "KeyFrame " + str(len(newList)))
            point.SetParent(self.kfList)
            newList.append(point)
        self.kfList.RemoveAllItems()
        for i in xrange(0, len(newList)):
            self.kfList.AppendItem(newList[i])

    def AddNewKeyFrame(self):
        if self.kfList.GetItemCount() < 1:
            app.SaveCameraSetting("camy/camSettings.default")
            lines = None
            with open("camy/camSettings.default") as f:
                lines = f.readlines()
            pos = lines[0].split(" ")
            point = KeyFrame(int(round(float(pos[1]))), int(round(float(pos[2]))), int(round(float(pos[3]))), 2494, 31,
                             -7, 1, 0, "KeyFrame " + str(self.kfList.GetItemCount()))
            point.SetParent(self.kfList)
            self.kfList.AppendItem(point)
        else:
            a = self.kfList.GetItemAtIndex(self.kfList.GetItemCount() - 1)
            point = KeyFrame(a.X, a.Y, a.Z, a.ZOOM, a.PITCH, a.ROTATION, a.TIME, a.EASE,
                             "KeyFrame " + str(self.kfList.GetItemCount()))
            point.SetParent(self.kfList)
            self.kfList.AppendItem(point)

    def ShowScriptManager(self, state):
        self.scriptMgr = None
        self.scriptMgr = ScriptManager(self, state)
        self.scriptMgr.Show()

    def SaveScript(self, fileName):
        c = self.kfList.GetItemCount()
        with open("camy/" + fileName + ".script", "wb") as fw:
            for i in xrange(0, c):
                kf = self.kfList.GetItemAtIndex(i)
                fw.write(kf.name.GetText() + "#" + str(kf.X) + "#" + str(kf.Y) + "#" + str(kf.Z) + "#" + str(
                    kf.ZOOM) + "#" + str(kf.PITCH) + "#" + str(kf.ROTATION) + "#" + str(kf.TIME) + "#" + str(
                    kf.EASE) + "\n")

    def LoadScript(self, fileName):
        try:
            fileContent = []
            with open("camy/" + fileName + ".script", "r") as fr:
                fileContent = fr.readlines()
            fileContentSize = len(fileContent)
            if fileContentSize > 0:
                self.kfList.RemoveAllItems()
                for i in xrange(0, fileContentSize):
                    splitted = fileContent[i].split("#")
                    if len(splitted) == 9:
                        point = KeyFrame(int(round(float(splitted[1]))), int(round(float(splitted[2]))),
                                         int(round(float(splitted[3]))), int(round(float(splitted[4]))),
                                         int(round(float(splitted[5]))), int(round(float(splitted[6]))),
                                         int(round(float(splitted[7]))), int(round(float(splitted[8]))), splitted[0])
                        point.SetParent(self.kfList)
                        self.kfList.AppendItem(point)
        except:
            logging.error("Failed to read file " + fileName)

    def GetCurrentlySelectedKeyFrame(self):
        return self.currentlySelectedKeyFrame

    def SetPlayState(self, state):
        self.isPlaySequence = state
        if state == 0:
            app.ShowCursor()
        else:
            app.HideCursor()

    def PlaySequence(self, state):
        self.board.Hide()

        if state == 0:
            playKeyFrames = []
            for i in xrange(0, self.kfList.GetItemCount()):
                playKeyFrames.append(self.kfList.GetItemAtIndex(i))
            f = app.GetUpdateFPS()
            self.xCoords = []
            self.yCoords = []
            self.zCoords = []
            self.zoomCoords = []
            self.pitchCoords = []
            self.rotationCoords = []
            for i in xrange(0, len(playKeyFrames) - 1):
                f = f * playKeyFrames[i].TIME
                self.calcNormalPlay(playKeyFrames[i].X, playKeyFrames[i + 1].X, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.xCoords)
                self.calcNormalPlay(playKeyFrames[i].Y, playKeyFrames[i + 1].Y, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.yCoords)
                self.calcNormalPlay(playKeyFrames[i].Z, playKeyFrames[i + 1].Z, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.zCoords)
                self.calcNormalPlay(playKeyFrames[i].ZOOM, playKeyFrames[i + 1].ZOOM, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.zoomCoords)
                self.calcNormalPlay(playKeyFrames[i].PITCH, playKeyFrames[i + 1].PITCH, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.pitchCoords)
                self.calcNormalPlay(playKeyFrames[i].ROTATION, playKeyFrames[i + 1].ROTATION, f,
                                    self.EASE_TYPES[playKeyFrames[i].EASE], self.rotationCoords)
            self.act = 0
            self.SetPlayState(1)
        elif state == 1:
            with open("camy/spl/tmp.camyin", "wb") as fw:
                for i in xrange(0, self.kfList.GetItemCount()):
                    item = self.kfList.GetItemAtIndex(i)
                    fw.write(str(item.X) + "|" + str(item.Y) + "|" + str(item.Z) + "|" + str(item.ZOOM) + "|" + str(
                        item.PITCH) + "|" + str(item.ROTATION) + "|" + str(item.TIME) + "|" + self.EASE_TYPES[
                                 item.EASE] + "|\n")
            count = self.kfList.GetItemCount() * app.GetUpdateFPS() * self.kfList.GetItemAtIndex(0).TIME
            if os.path.exists("camy/spl/tmp.camyout"):
                os.remove("camy/spl/tmp.camyout")
            os.system("camy.exe camy/spl/tmp " + str(count))

            self.playKeyFrames = []
            lines = []
            if os.path.exists("camy/spl/tmp.camyout"):
                with open("camy/spl/tmp.camyout", "r") as fr:
                    lines = fr.readlines()
            else:
                return

            for i in xrange(0, len(lines)):
                x, y, z, zoom, pitch, rotation = lines[i].split("|")
                self.playKeyFrames.append(
                    KeyFrame(float(x), float(y), float(z), float(zoom), float(pitch), float(rotation), 0.0, 0, ""))

            self.curX = float(self.playKeyFrames[0].X)
            self.curY = float(self.playKeyFrames[0].Y)
            self.curZ = float(self.playKeyFrames[0].Z)
            self.curZOOM = float(self.playKeyFrames[0].ZOOM)
            self.curPITCH = float(self.playKeyFrames[0].PITCH)
            self.curROTATION = float(self.playKeyFrames[0].ROTATION)
            self.act = 0
            self.SetPlayState(2)

    def RedoLast(self):
        if self.lastChange != "":
            if self.lastChange == "X":
                self.currentlySelectedKeyFrame.X = self.currentlySelectedKeyFrameHelp.X
                self.SliderChangeX(1)
            elif self.lastChange == "Y":
                self.currentlySelectedKeyFrame.Y = self.currentlySelectedKeyFrameHelp.Y
                self.SliderChangeY(1)
            elif self.lastChange == "Z":
                self.currentlySelectedKeyFrame.Z = self.currentlySelectedKeyFrameHelp.Z
                self.SliderChangeZ(1)
            elif self.lastChange == "D":
                self.currentlySelectedKeyFrame.ZOOM = self.currentlySelectedKeyFrameHelp.ZOOM
                self.SliderChangeZoom(1)
            elif self.lastChange == "P":
                self.currentlySelectedKeyFrame.PITCH = self.currentlySelectedKeyFrameHelp.PITCH
                self.SliderChangePitch(1)
            elif self.lastChange == "R":
                self.currentlySelectedKeyFrame.ROTATION = self.currentlySelectedKeyFrameHelp.ROTATION
                self.SliderChangeRotation(1)
            elif self.lastChange == "T":
                self.currentlySelectedKeyFrame.TIME = self.currentlySelectedKeyFrameHelp.TIME
                self.SliderChangeTime(1)
        self.lastChange = ""

    def SelectKeyFrame(self, selectedKeyFrame):
        self.lastChange = ""
        self.currentlySelectedKeyFrameIndex = self.kfList.GetSelectedItemIndex()
        self.currentlySelectedKeyFrame = KeyFrame(selectedKeyFrame.X, selectedKeyFrame.Y, selectedKeyFrame.Z,
                                                  selectedKeyFrame.ZOOM, selectedKeyFrame.PITCH,
                                                  selectedKeyFrame.ROTATION, selectedKeyFrame.TIME,
                                                  selectedKeyFrame.EASE, selectedKeyFrame.name.GetText())
        self.currentlySelectedKeyFrameHelp = KeyFrame(selectedKeyFrame.X, selectedKeyFrame.Y, selectedKeyFrame.Z,
                                                      selectedKeyFrame.ZOOM, selectedKeyFrame.PITCH,
                                                      selectedKeyFrame.ROTATION, selectedKeyFrame.TIME,
                                                      selectedKeyFrame.EASE, selectedKeyFrame.name.GetText())
        self.rbtngrpEaseTypes.OnClick(selectedKeyFrame.EASE)
        self.SetPlayState(0)
        self.ResetSliders()

    def ReloadSettingValues(self):
        if self.currentlySelectedKeyFrame == None:
            self.kfPosXValue.SetText("0")
            self.kfPosYValue.SetText("0")
            self.kfPosZValue.SetText("0")
            self.kfPosZoomValue.SetText("0")
            self.kfPosPitchValue.SetText("0")
            self.kfPosRotationValue.SetText("0")
            self.kfSettingsTimeValue.SetText("0")
            self.rbtngrpEaseTypes.OnClick(0)
            return

        self.kfPosXValue.SetText(str(self.currentlySelectedKeyFrame.X))
        self.kfPosYValue.SetText(str(self.currentlySelectedKeyFrame.Y))
        self.kfPosZValue.SetText(str(self.currentlySelectedKeyFrame.Z))
        self.kfPosZoomValue.SetText(str(self.currentlySelectedKeyFrame.ZOOM))
        self.kfPosPitchValue.SetText(str(self.currentlySelectedKeyFrame.PITCH))
        self.kfPosRotationValue.SetText(str(self.currentlySelectedKeyFrame.ROTATION))
        self.kfSettingsTimeValue.SetText(str(self.currentlySelectedKeyFrame.TIME))

    def ResetSliders(self):
        self.kfPosXSlider.SetSliderPos(0.5)
        self.kfPosYSlider.SetSliderPos(0.5)
        self.kfPosZSlider.SetSliderPos(0.5)
        self.kfPosZoomSlider.SetSliderPos(0.5)
        self.kfPosPitchSlider.SetSliderPos(0.5)
        self.kfPosRotationSlider.SetSliderPos(0.5)
        self.kfSettingsTimeSlider.SetSliderPos(0.5)

    def SliderChangeX(self, adjust=0):
        self.lastChange = "X"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.X = self.currentlySelectedKeyFrameHelp.X + int(
                    (self.kfPosXSlider.GetSliderPos() - 0.5) * 10000.0)
        else:
            self.kfPosXSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrameHelp.X) - float(
                self.currentlySelectedKeyFrameHelp.X) + 10000.0) / 10000.0 - 0.5)))

    def SliderChangeY(self, adjust=0):
        self.lastChange = "Y"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.Y = self.currentlySelectedKeyFrameHelp.Y + int(
                    (self.kfPosYSlider.GetSliderPos() - 0.5) * 10000.0)
        else:
            self.kfPosYSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrame.Y) - float(
                self.currentlySelectedKeyFrameHelp.Y) + 10000.0) / 10000.0 - 0.5)))

    def SliderChangeZ(self, adjust=0):
        self.lastChange = "Z"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.Z = self.currentlySelectedKeyFrameHelp.Z + int(
                    (self.kfPosZSlider.GetSliderPos() - 0.5) * 10000.0)
        else:
            self.kfPosZSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrame.Z) - float(
                self.currentlySelectedKeyFrameHelp.Z) + 10000.0) / 10000.0 - 0.5)))

    def SliderChangeZoom(self, adjust=0):
        self.lastChange = "D"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.ZOOM = self.currentlySelectedKeyFrameHelp.ZOOM + int(
                    (self.kfPosZoomSlider.GetSliderPos() - 0.5) * 2000.0)
        else:
            self.kfPosZoomSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrame.ZOOM) - float(
                self.currentlySelectedKeyFrameHelp.ZOOM) + 2000.0) / 2000.0 - 0.5)))

    def SliderChangePitch(self, adjust=0):
        self.lastChange = "P"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.PITCH = self.currentlySelectedKeyFrameHelp.PITCH + int(
                    (self.kfPosPitchSlider.GetSliderPos() - 0.5) * 40.0)
        else:
            self.kfPosPitchSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrame.PITCH) - float(
                self.currentlySelectedKeyFrameHelp.PITCH) + 40.0) / 40.0 - 0.5)))

    def SliderChangeRotation(self, adjust=0):
        self.lastChange = "R"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.ROTATION = self.currentlySelectedKeyFrameHelp.ROTATION + int(
                    (self.kfPosRotationSlider.GetSliderPos() - 0.5) * 180.0)
        else:
            self.kfPosRotationSlider.SetSliderPos(max(0.0, min(1.0, (
                        float(self.currentlySelectedKeyFrame.ROTATION) - float(
                    self.currentlySelectedKeyFrameHelp.ROTATION) + 180.0) / 180.0 - 0.5)))

    def SliderChangeTime(self, adjust=0):
        self.lastChange = "T"
        if adjust == 0:
            if self.currentlySelectedKeyFrame != None and self.currentlySelectedKeyFrameHelp != None:
                self.currentlySelectedKeyFrame.TIME = self.currentlySelectedKeyFrameHelp.TIME + int(
                    (self.kfSettingsTimeSlider.GetSliderPos() - 0.5) * 8.0)
        else:
            self.kfPosTimeSlider.SetSliderPos(max(0.0, min(1.0, (float(self.currentlySelectedKeyFrame.TIME) - float(
                self.currentlySelectedKeyFrameHelp.TIME) + 8.0) / 8.0 - 0.5)))

    def ChangeEaseType(self):
        if self.currentlySelectedKeyFrame != None:
            idx = 0
            for i in xrange(0, len(self.rbtngrpEaseTypes.buttonGroup)):
                if self.rbtngrpEaseTypes.buttonGroup[i][0].IsDown():
                    idx = i
            self.currentlySelectedKeyFrame.EASE = idx

    def DownKey(self, key):
        if self.downKey == key:
            if self.nextUpdateTime < app.GetTime():
                self.PressedKey()
        else:
            self.downKey = key
            self.nextUpdateTime = app.GetTime() + 0.25
            self.PressedKey()

    def PressedKey(self):
        if self.currentlySelectedKeyFrame == None:
            return

        if app.VK_NUMPAD0 == self.downKey:
            self.lastChange = "Z"
            self.currentlySelectedKeyFrame.Z = self.currentlySelectedKeyFrame.Z - 10
            self.SliderChangeZ(1)
        elif app.VK_NUMPAD1 == self.downKey:
            self.lastChange = "Z"
            self.currentlySelectedKeyFrame.Z = self.currentlySelectedKeyFrame.Z + 10
            self.SliderChangeZ(1)
        elif app.VK_NUMPAD2 == self.downKey:
            self.lastChange = "P"
            self.currentlySelectedKeyFrame.PITCH = self.currentlySelectedKeyFrame.PITCH - 1
            self.SliderChangePitch(1)
        elif app.VK_NUMPAD3 == self.downKey:
            self.lastChange = "P"
            self.currentlySelectedKeyFrame.PITCH = self.currentlySelectedKeyFrame.PITCH + 1
            self.SliderChangePitch(1)
        elif app.VK_NUMPAD4 == self.downKey:
            self.lastChange = "X"
            self.currentlySelectedKeyFrame.X = self.currentlySelectedKeyFrame.X - 10
            self.SliderChangeX(1)
        elif app.VK_NUMPAD5 == self.downKey:
            self.lastChange = "Y"
            self.currentlySelectedKeyFrame.Y = self.currentlySelectedKeyFrame.Y - 10
            self.SliderChangeY(1)
        elif app.VK_NUMPAD6 == self.downKey:
            self.lastChange = "X"
            self.currentlySelectedKeyFrame.X = self.currentlySelectedKeyFrame.X + 10
            self.SliderChangeX(1)
        elif app.VK_NUMPAD7 == self.downKey:
            self.lastChange = "R"
            self.currentlySelectedKeyFrame.ROTATION = self.currentlySelectedKeyFrame.ROTATION - 1
            self.SliderChangeRotation(1)
        elif app.VK_NUMPAD8 == self.downKey:
            self.lastChange = "Y"
            self.currentlySelectedKeyFrame.Y = self.currentlySelectedKeyFrame.Y + 10
            self.SliderChangeY(1)
        elif app.VK_NUMPAD9 == self.downKey:
            self.lastChange = "R"
            self.currentlySelectedKeyFrame.ROTATION = self.currentlySelectedKeyFrame.ROTATION + 1
            self.SliderChangeRotation(1)

    def UpKey(self, key):
        self.downKey = 0
        self.nextUpdateTime = 0

    def ApplyChanges(self):
        if self.currentlySelectedKeyFrameIndex != None and self.currentlySelectedKeyFrame != None:
            self.kfList.ReplaceItemAtIndex(self.currentlySelectedKeyFrameIndex,
                                           KeyFrame(self.currentlySelectedKeyFrame.X, self.currentlySelectedKeyFrame.Y,
                                                    self.currentlySelectedKeyFrame.Z,
                                                    self.currentlySelectedKeyFrame.ZOOM,
                                                    self.currentlySelectedKeyFrame.PITCH,
                                                    self.currentlySelectedKeyFrame.ROTATION,
                                                    self.currentlySelectedKeyFrame.TIME,
                                                    self.currentlySelectedKeyFrame.EASE,
                                                    self.currentlySelectedKeyFrame.name.GetText()))
            self.kfList.SelectIndex(self.currentlySelectedKeyFrameIndex)

    def ResetChanges(self):
        self.kfList.SelectIndex(self.currentlySelectedKeyFrameIndex)

    def Cancel(self):
        self.isPlaySequence = 0
        app.ShowCursor()

    def OnUpdate(self):
        if self.isPlaySequence == 0:
            cSKF = self.GetCurrentlySelectedKeyFrame()
            self.ReloadSettingValues()
            if not self.board.IsShow():
                self.board.Show()
            if cSKF != None:
                app.SetCameraSetting(int(cSKF.X), int(cSKF.Y), int(cSKF.Z), int(cSKF.ZOOM), int(cSKF.ROTATION), int(cSKF.PITCH))
                app.SetCamera(cSKF.ZOOM, cSKF.PITCH, cSKF.ROTATION, 0.0)
            else:
                app.SetCamera(1.0, 0.0, 0.0, 0.0)
        elif self.isPlaySequence == 1:
            app.SetCameraSetting(int(self.xCoords[self.act]), int(self.yCoords[self.act]), int(self.zCoords[self.act]),
                                 int(self.zoomCoords[self.act]), int(self.rotationCoords[self.act]), int(self.pitchCoords[self.act]))
            app.SetCamera(self.zoomCoords[self.act], self.pitchCoords[self.act], self.rotationCoords[self.act], 0.0)
            self.act = self.act + 1
            if self.act == len(self.xCoords):
                self.SetPlayState(0)
                self.kfList.SelectIndex(self.kfList.GetItemCount() - 1)
        elif self.isPlaySequence == 2:
            k = self.playKeyFrames[self.act]
            app.SetCameraSetting(int(k.X), int(k.Y), int(k.Z), int(k.ZOOM), int(k.PITCH), int(k.ROTATION))
            app.SetCamera(k.ZOOM, k.PITCH, k.ROTATION, 0.0)
            self.act = self.act + 1

            if (len(self.playKeyFrames) - 1) == self.act:
                self.SetPlayState(0)
                self.kfList.SelectIndex(self.kfList.GetItemCount() - 1)

    def calcNormalPlay(self, a, b, pp, ease, coords):
        delta = b - a
        for i in xrange(0, pp):
            coords.append(self.easeByType(ease, i, a, delta, pp))

    def easeByType(self, type, t, b, c, d):
        t = float(t)
        b = float(b)
        c = float(c)
        d = float(d)
        if type == "linear":
            return float(c * t / d + b)
        elif type == "quadratic":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(c / 2.0 * t * t + b)
            t = t - 1.0
            return float(-c / 2.0 * (t * (t - 2.0) - 1.0) + b)
        elif type == "cubic":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(c / 2.0 * t * t * t + b)
            t = t - 2.0
            return (c / 2.0 * (t * t * t + 2.0) + b)
        elif type == "quartic":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(c / 2.0 * t * t * t * t + b)
            t = t - 2.0
            return float(-c / 2.0 * (t * t * t * t - 2.0) + b)
        elif type == "quintic":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(c / 2.0 * t * t * t * t * t + b)
            t = t - 2.0
            return float(c / 2.0 * (t * t * t * t * t + 2.0) + b)
        elif type == "sinusoidal":
            return float(-c / 2.0 * (math.cos(math.pi * t / d) - 1.0) + b)
        elif type == "exponential":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(c / 2.0 * math.pow(2.0, 10.0 * (t - 1)) + b)
            t = t - 1.0
            return float(c / 2.0 * (-math.pow(2.0, -10.0 * t) + 2.0) + b)
        elif type == "circular":
            t = t / (d / 2.0)
            if t < 1.0:
                return float(-c / 2.0 * (math.sqrt(1.0 - t * t) - 1) + b)
            t = t - 2.0
            return float(c / 2.0 * (math.sqrt(1.0 - t * t) + 1.0) + b)
        else:
            return float(c * t / d + b)
