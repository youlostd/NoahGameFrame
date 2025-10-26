# -*- coding: utf-8 -*-


import logging
import string
from _weakref import proxy, ref
from math import floor, ceil

import app
from chr import PLAYER_NAME_MAX_LEN
import grp
import guild
import item
import player
import skill
import snd
import wiki
import wndMgr

import localeInfo
from ui_event import MakeEvent, Event, MakeCallback
from utils import lighten_color

PATCH_COMMON = "d:/ymir work/ui/gui/common"

BACKGROUND_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 1.0)
DARK_COLOR = grp.GenerateColor(0.2, 0.2, 0.2, 1.0)
BRIGHT_COLOR = grp.GenerateColor(0.7, 0.7, 0.7, 1.0)
SELECT_COLOR = grp.GenerateColor(0.0, 0.0, 0.5, 0.3)
# BACKGROUND_COLOR_ALPHA = grp.GenerateColor(30.0 / 255.0, 9.0 / 255.0, 8.0 / 255.0, 0.7)

WHITE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.5)
HALF_WHITE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.2)

BUTTON_DEFAULT_TEXT_COLOR = 0xFFE2E2AF

createToolTipWindowDict = {}


def RenderRoundBox(x, y, width, height, color):
    grp.SetColor(color)
    grp.RenderLine(x + 2, y, width - 3, 0)
    grp.RenderLine(x + 2, y + height, width - 3, 0)
    grp.RenderLine(x, y + 2, 0, height - 4)
    grp.RenderLine(x + width, y + 1, 0, height - 3)
    grp.RenderLine(x, y + 2, 2, -2)
    grp.RenderLine(x, y + height - 2, 2, 2)
    grp.RenderLine(x + width - 2, y, 2, 2)
    grp.RenderLine(x + width - 2, y + height, 2, -2)


def RegisterToolTipWindow(type, createToolTipWindow):
    createToolTipWindowDict[type] = createToolTipWindow


# Window Manager Event List##
#############################
# "OnMouseLeftButtonDown"
# "OnMouseLeftButtonUp"
# "OnMouseLeftButtonDoubleClick"
# "OnMouseRightButtonDown"
# "OnMouseRightButtonUp"
# "OnMouseRightButtonDoubleClick"
# "OnMouseDrag"
# "OnSetFocus"
# "OnKillFocus"
# "OnMouseOverIn"
# "OnMouseOverOut"
# "OnRender"
# "OnUpdate"
# "OnKeyDown"
# "OnKeyUp"
# "OnTop"
# "OnIMEUpdate" ## IME Only
# "OnIMETab"    ## IME Only
# "OnIMEReturn" ## IME Only
##############################
# Window Manager Event List##


def PackColor(rf, gf, bf, af):
    a = int(af * 255)
    r = int(rf * 255)
    g = int(gf * 255)
    b = int(bf * 255)
    return (a << 24) + (r << 16) + (g << 8) + b


def UnpackColor(color):
    color = int(color)
    a = ((color & 0xFF000000) >> 24) / 255.0
    r = ((color & 0x00FF0000) >> 16) / 255.0
    g = ((color & 0x0000FF00) >> 8) / 255.0
    b = ((color & 0x000000FF) >> 0) / 255.0
    return r, g, b, a


class Window(object):
    def NoneMethod(cls):
        pass

    NoneMethod = classmethod(NoneMethod)

    def __init__(self, layer="UI"):
        self.hWnd = None
        self.parentWindow = 0
        self.onRunMouseWheelEvent = None
        self.propertyList = {}
        self.RegisterWindow(layer)
        self.SetWindowName(type(self).__name__)
        self.baseWindowSizeChangeEvent = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.Register(self, layer)

    def Destroy(self):
        pass

    def GetWindowHandle(self):
        return self.hWnd

    def AddFlag(self, style):
        wndMgr.AddFlag(self.hWnd, style)

    def RemoveFlag(self, flag):
        wndMgr.RemoveFlag(self.hWnd, flag)

    def IsRTL(self):
        return wndMgr.IsRTL(self.hWnd)

    def GetAlpha(self):
        return wndMgr.GetAlpha(self.hWnd)

    def SetAlpha(self, alpha):
        return wndMgr.SetAlpha(self.hWnd, alpha)

    def SetAllAlpha(self, alpha):
        return wndMgr.SetAllAlpha(self.hWnd, alpha)

    def SetWindowName(self, Name):
        wndMgr.SetName(self.hWnd, Name)

    def GetWindowName(self):
        return wndMgr.GetName(self.hWnd)

    def SetParent(self, parent):
        try:
            wndMgr.SetParent(self.hWnd, parent.hWnd)
        except Exception as e:
            logging.exception(e)

    def SetAttachedTo(self, attachedTo):
        wndMgr.SetAttachedTo(self.hWnd, attachedTo.hWnd)

    def SetParentProxy(self, parent):
        self.parentWindow = proxy(parent)
        wndMgr.SetParent(self.hWnd, parent.hWnd)

    def GetParentProxy(self):
        return self.parentWindow

    def GetParent(self):
        return wndMgr.GetParent(self.hWnd)

    def SetPickAlways(self):
        wndMgr.SetPickAlways(self.hWnd)

    def SetClipRect(self, left, top, right, bottom):
        return wndMgr.SetClipRect(self.hWnd, left, top, right, bottom)

    def SetHorizontalAlignLeft(self):
        wndMgr.SetWindowHorizontalAlign(self.hWnd, wndMgr.HORIZONTAL_ALIGN_LEFT)

    def SetHorizontalAlignCenter(self):
        wndMgr.SetWindowHorizontalAlign(self.hWnd, wndMgr.HORIZONTAL_ALIGN_CENTER)

    def SetHorizontalAlignRight(self):
        wndMgr.SetWindowHorizontalAlign(self.hWnd, wndMgr.HORIZONTAL_ALIGN_RIGHT)

    def SetVerticalAlignTop(self):
        wndMgr.SetWindowVerticalAlign(self.hWnd, wndMgr.VERTICAL_ALIGN_TOP)

    def SetVerticalAlignCenter(self):
        wndMgr.SetWindowVerticalAlign(self.hWnd, wndMgr.VERTICAL_ALIGN_CENTER)

    def SetVerticalAlignBottom(self):
        wndMgr.SetWindowVerticalAlign(self.hWnd, wndMgr.VERTICAL_ALIGN_BOTTOM)

    def EnableClipping(self):
        wndMgr.EnableClipping(self.hWnd)

    def DisableClipping(self):
        wndMgr.DisableClipping(self.hWnd)

    def SetTop(self):
        wndMgr.SetTop(self.hWnd)

    def Show(self):
        wndMgr.Show(self.hWnd)

    def Hide(self):
        wndMgr.Hide(self.hWnd)

    def SetVisible(self, is_show):
        if is_show == True:
            self.Show()
        else:
            self.Hide()

    def Lock(self):
        wndMgr.Lock(self.hWnd)

    def Unlock(self):
        wndMgr.Unlock(self.hWnd)

    def IsShow(self):
        return wndMgr.IsShow(self.hWnd)

    def IsRendering(self):
        return wndMgr.IsRendering(self.hWnd)

    def UpdateRect(self):
        wndMgr.UpdateRect(self.hWnd)

    def SetSize(self, width, height):
        wndMgr.SetWindowSize(self.hWnd, int(width), int(height))
        if self.baseWindowSizeChangeEvent:
            self.baseWindowSizeChangeEvent()

    def GetWidth(self):
        return wndMgr.GetWindowWidth(self.hWnd)

    def GetHeight(self):
        return wndMgr.GetWindowHeight(self.hWnd)

    def GetLeft(self):
        x, y = self.GetLocalPosition()
        return x

    def GetGlobalLeft(self):
        x, y = self.GetGlobalPosition()
        return x

    def GetTop(self):
        x, y = self.GetLocalPosition()
        return y

    def GetGlobalTop(self):
        x, y = self.GetGlobalPosition()
        return y

    def GetRight(self):
        return self.GetLeft() + self.GetWidth()

    def GetBottom(self):
        return self.GetTop() + self.GetHeight()

    def GetLocalPosition(self):
        return wndMgr.GetWindowLocalPosition(self.hWnd)

    def GetGlobalPosition(self):
        return wndMgr.GetWindowGlobalPosition(self.hWnd)

    def GetMouseLocalPosition(self):
        return wndMgr.GetMouseLocalPosition(self.hWnd)

    def GetRect(self):
        return wndMgr.GetWindowRect(self.hWnd)

    def GetBaseRect(self):
        return wndMgr.GetWindowBaseRect(self.hWnd)

    def SetPosition(self, x, y):
        wndMgr.SetWindowPosition(self.hWnd, int(x), int(y))

    def SetVerticalPosition(self, y):
        x, _ = self.GetLocalPosition()
        self.SetPosition(x, y)

    def SetHorizontalPosition(self, x):
        _, y = self.GetLocalPosition()
        self.SetPosition(x, y)

    def SetCenterPosition(self, x=0, y=0):
        self.SetPosition(
            (wndMgr.GetScreenWidth() - self.GetWidth()) / 2 + x,
            (wndMgr.GetScreenHeight() - self.GetHeight()) / 2 + y,
        )

    def IsFocus(self):
        return wndMgr.IsFocus(self.hWnd)

    def SetFocus(self):
        wndMgr.SetFocus(self.hWnd)

    def KillFocus(self):
        wndMgr.KillFocus(self.hWnd)

    def GetChildCount(self):
        return wndMgr.GetChildCount(self.hWnd)

    def CaptureMouse(self):
        wndMgr.CaptureMouse(self.hWnd)

    def ReleaseMouse(self):
        wndMgr.ReleaseMouse(self.hWnd)

    def IsIn(self):
        return wndMgr.IsIn(self.hWnd)

    def IsInPosition(self):
        xMouse, yMouse = wndMgr.GetMousePosition()
        x, y = self.GetGlobalPosition()
        return (
            xMouse >= x
            and xMouse < x + self.GetWidth()
            and yMouse >= y
            and yMouse < y + self.GetHeight()
        )

    def SetProperty(self, propName, propValue):
        self.propertyList[propName] = propValue

    def GetProperty(self, propName):
        if propName in self.propertyList:
            return self.propertyList[propName]

        return None

    def OnRunMouseWheel(self, nLen):
        if self.onRunMouseWheelEvent:
            return self.onRunMouseWheelEvent(nLen)

        return False

    def SetOnRunMouseWheelEvent(self, event):
        self.onRunMouseWheelEvent = MakeEvent(event)

    def SetSizeChangeEvent(self, event):
        self.baseWindowSizeChangeEvent = MakeEvent(event)


class BaseEvents(object):

    def __init__(self):
        super(BaseEvents, self).__init__()
        self.eventOnMouseOverIn = None
        self.eventOnMouseOverOut = None
        self.eventOnMouseClick = None

    def SetOverInEvent(self, event):
        self.eventOnMouseOverIn = MakeEvent(event)

    def SetOverOutEvent(self, event):
        self.eventOnMouseOverOut = MakeEvent(event)

    def SetMouseLeftButtonDownEvent(self, event):
        self.eventOnMouseClick = MakeEvent(event)

    def OnMouseOverIn(self):
        if self.eventOnMouseOverIn:
            return self.eventOnMouseOverIn()

        return False

    def OnMouseOverOut(self):
        if self.eventOnMouseOverOut:
            return self.eventOnMouseOverOut()

        return False

    def OnMouseLeftButtonDown(self):
        if self.eventOnMouseClick:
            return self.eventOnMouseClick()

        return False


class WindowWithBaseEvents(Window, BaseEvents):

    def __init__(self):
        Window.__init__(self)
        BaseEvents.__init__(self)


class AutoGrowingVerticalContainer(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.containerItems = []
        self.width = 0
        self.height = 0
        self.elementOffsetY = 0
        self.elementStartOffsetY = 0

    def SetWidth(self, width):
        self.width = width

    def SetElementOffset(self, offsetY):
        self.elementOffsetY = offsetY

    def AppendItem(self, item):
        self.height += item.GetHeight() + self.elementOffsetY
        self.containerItems.append(item)
        self.Update()

    def Update(self):
        self.SetSize(self.width, self.height)
        self.UpdateRect()

    def RecalculateHeight(self):
        self.height = 0
        for item in self.containerItems:
            self.height = self.height + item.GetHeight() + self.elementOffsetY
        self.Update()

    def GetElementByIndex(self, index):
        if index >= len(self.containerItems):
            return None

        return self.containerItems[index]

    def ClearItems(self):
        self.containerItems[:] = []
        self.RecalculateHeight()

    # def RecalculateHeight(self):
    #     self.height = 0
    #     shownItems = [i for i in self.containerItems if i.IsShow()]
    #     for item in shownItems:
    #         self.height = self.height + item.GetHeight() + self.elementOffsetY
    #     self.Update()


class ListBoxEx(Window):
    class Item(Window):
        def __init__(self, layer="UI"):
            Window.__init__(self, layer)
            self.parent = None

        def SetParent(self, parent):
            Window.SetParent(self, parent)
            self.parent = proxy(parent)

        def OnMouseLeftButtonDown(self):
            self.parent.SelectItem(self)
            return True

        def OnRender(self):
            if self.parent.GetSelectedItem() == self:
                self.OnSelectedRender()

        def OnSelectedRender(self):
            x, y = self.GetGlobalPosition()
            grp.SetColor(SELECT_COLOR)
            grp.RenderBar(x, y, self.parent.itemWidth, self.parent.itemHeight)

        def LoadImage(self, imageName):
            self.name = imageName
            wndMgr.LoadImage(self.hWnd, imageName)

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.viewItemCount = 10
        self.basePos = 0
        self.itemWidth = 100
        self.itemHeight = 16
        self.itemStep = 20
        self.selItem = 0
        self.itemList = []
        self.onSelectItemEvent = None
        self.SetOnRunMouseWheelEvent(self.OnRunMouseWheel)

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
        self.onSelectItemEvent = MakeEvent(event)

    def SetBasePos(self, basePos):
        for oldItem in self.itemList[self.basePos : self.basePos + self.viewItemCount]:
            oldItem.Hide()

        self.basePos = basePos

        pos = basePos

        for newItem in self.itemList[self.basePos : self.basePos + self.viewItemCount]:
            (x, y) = self.GetItemViewCoord(pos, newItem.GetWidth())

            newItem.SetPosition(x, y)
            newItem.Show()
            pos += 1

    def GetItemIndex(self, argItem):
        return self.itemList.index(argItem)

    def GetSelectedItem(self):
        return self.selItem

    def GetSelectedItemText(self):
        return self.textDict.get(self.selectedLine, "")

    def SelectIndex(self, index):
        if index >= len(self.itemList) or index < 0:
            self.selItem = None
            return

        try:
            self.selItem = self.itemList[index]
        except:
            pass

    def GetItems(self):
        return self.itemList

    def SelectItem(self, selItem):
        self.selItem = selItem
        if self.onSelectItemEvent:
            self.onSelectItemEvent(selItem)

    def RemoveAllItems(self):
        self.selItem = None
        self.itemList = []

        o = self.scrollBar
        if o:
            o.SetPos(0)

    def RemoveItem(self, delItem):
        if delItem == self.selItem:
            self.selItem = None

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
        self.scrollBar = proxy(scrollBar)

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition():
            o = self.scrollBar
            if o:
                if nLen > 0:
                    o.OnUp()
                else:
                    o.OnDown()
            return True
        return False

    def __OnScroll(self):
        o = self.scrollBar
        if o:
            self.SetBasePos(int(o.GetPos() * self.__GetScrollLen()))

    def __GetScrollLen(self):
        scrollLen = self.__GetItemCount() - self.__GetViewItemCount()
        if scrollLen < 0:
            return 0

        return scrollLen

    def __GetViewItemCount(self):
        return self.viewItemCount

    def __GetItemCount(self):
        return len(self.itemList)

    def GetScrollLen(self):
        return self.__GetScrollLen()

    def GetViewItemCount(self):
        return self.viewItemCount

    def GetItemCount(self):
        return len(self.itemList)

    def GetItemStep(self):
        return self.itemStep

    def GetItemViewCoord(self, pos, itemWidth):
        if localeInfo.IsARABIC():
            return (
                self.GetWidth() - itemWidth - 10,
                (pos - self.basePos) * self.itemStep,
            )
        else:
            return (0, (pos - self.basePos) * self.itemStep)

    def __IsInViewRange(self, pos):
        if pos < self.basePos:
            return 0
        if pos >= self.basePos + self.viewItemCount:
            return 0
        return 1


class ListBoxExNew(Window):
    class Item(Window):
        def __init__(self):
            Window.__init__(self)

            self.realWidth = 0
            self.realHeight = 0

            self.removeTop = 0
            self.removeBottom = 0
            self.TEMPORARY_PLACE = 3

            self.SetWindowName("NONAME_ListBoxExNew_Item")

        def SetParent(self, parent):
            Window.SetParent(self, parent)
            self.parent = proxy(parent)

        def SetSize(self, width, height):
            self.realWidth = width
            self.realHeight = height
            Window.SetSize(self, width, height)

        def SetRemoveTop(self, height):
            self.removeTop = height
            self.RefreshHeight()

        def SetRemoveBottom(self, height):
            self.removeBottom = height
            self.RefreshHeight()

        def SetCurrentHeight(self, height):
            Window.SetSize(self, self.GetWidth(), height)

        def GetCurrentHeight(self):
            return Window.GetHeight(self)

        def ResetCurrentHeight(self):
            self.removeTop = 0
            self.removeBottom = 0
            self.RefreshHeight()

        def RefreshHeight(self):
            self.SetCurrentHeight(self.GetHeight() - self.removeTop - self.removeBottom)

        def GetHeight(self):
            return self.realHeight

    def __init__(self, stepSize, viewSteps):
        Window.__init__(self)

        self.viewItemCount = 10
        self.basePos = 0
        self.baseIndex = 0
        self.maxSteps = 0
        self.viewSteps = viewSteps
        self.stepSize = stepSize
        self.itemList = []

        self.scrollBar = None
        self.SetOnRunMouseWheelEvent(self.OnRunMouseWheel)

        self.SetWindowName("NONAME_ListBoxEx")

    def IsEmpty(self):
        if len(self.itemList) == 0:
            return 1
        return 0

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition() and self.scrollBar:
            if nLen > 0:
                self.scrollBar.OnUp()
            else:
                self.scrollBar.OnDown()
            return True
        return False

    def __CheckBasePos(self, pos):
        self.viewItemCount = 0

        start_pos = pos

        height = 0
        while height < self.GetHeight():
            if pos >= len(self.itemList):
                return start_pos == 0
            height += self.itemList[pos].GetHeight()
            pos += 1
            self.viewItemCount += 1
        return height == self.GetHeight()

    def SetBasePos(self, basePos, forceRefresh=True):
        if forceRefresh == False and self.basePos == basePos:
            return

        for oldItem in self.itemList[
            self.baseIndex : self.baseIndex + self.viewItemCount
        ]:
            oldItem.ResetCurrentHeight()
            oldItem.Hide()

        self.basePos = basePos

        baseIndex = 0
        while basePos > 0:
            basePos -= self.itemList[baseIndex].GetHeight() / self.stepSize
            if basePos < 0:
                self.itemList[baseIndex].SetRemoveTop(self.stepSize * abs(basePos))
                break
            baseIndex += 1
        self.baseIndex = baseIndex

        stepCount = 0
        self.viewItemCount = 0
        while baseIndex < len(self.itemList):
            stepCount += self.itemList[baseIndex].GetCurrentHeight() / self.stepSize
            self.viewItemCount += 1
            if stepCount > self.viewSteps:
                self.itemList[baseIndex].SetRemoveBottom(
                    self.stepSize * (stepCount - self.viewSteps)
                )
                break
            elif stepCount == self.viewSteps:
                break
            baseIndex += 1

        y = 0
        for newItem in self.itemList[
            self.baseIndex : self.baseIndex + self.viewItemCount
        ]:
            newItem.SetPosition(0, y)
            newItem.Show()
            y += newItem.GetCurrentHeight()

    def GetItemIndex(self, argItem):
        return self.itemList.index(argItem)

    def GetSelectedItem(self):
        return self.selItem

    def GetSelectedItemIndex(self):
        return self.selItemIdx

    def RemoveAllItems(self):
        self.itemList = []
        self.maxSteps = 0

        if self.scrollBar:
            self.scrollBar.SetPos(0)

    def RemoveItem(self, delItem):
        self.maxSteps -= delItem.GetHeight() / self.stepSize
        self.itemList.remove(delItem)

    def AppendItem(self, newItem):
        if newItem.GetHeight() % self.stepSize != 0:
            logging.debug(
                "Invalid AppendItem height %d stepSize %d"
                % (newItem.GetHeight(), self.stepSize)
            )
            return

        self.maxSteps += newItem.GetHeight() / self.stepSize
        newItem.SetParent(self)
        self.itemList.append(newItem)

    def SetScrollBar(self, scrollBar):
        scrollBar.SetScrollEvent(self.__OnScroll)
        self.scrollBar = scrollBar

    def __OnScroll(self):
        self.SetBasePos(int(self.scrollBar.GetPos() * self.__GetScrollLen()), False)
        return True

    def __GetScrollLen(self):
        scrollLen = self.maxSteps - self.viewSteps
        if scrollLen < 0:
            return 0

        return scrollLen

    def __GetViewItemCount(self):
        return self.viewItemCount

    def __GetItemCount(self):
        return len(self.itemList)

    def GetViewItemCount(self):
        return self.viewItemCount

    def GetItemCount(self):
        return len(self.itemList)


class TextLine(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.max = 0
        self.SetFontName(localeInfo.UI_DEF_FONT)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterTextLine(self, layer)
        wndMgr.AddFlag(self.hWnd, "not_pick")

    def SetMax(self, max):
        wndMgr.SetMax(self.hWnd, max)

    def SetLimitWidth(self, width):
        wndMgr.SetLimitWidth(self.hWnd, width)

    def SetMultiLine(self):
        wndMgr.SetMultiLine(self.hWnd, 1)

    def SetSecret(self, Value=True):
        wndMgr.SetSecret(self.hWnd, Value)

    def SetOutline(self, Value=True):
        wndMgr.SetOutline(self.hWnd, Value)

    def SetFontName(self, fontName):
        wndMgr.SetFontName(self.hWnd, fontName)

    def SetDefaultFontName(self):
        wndMgr.SetFontName(self.hWnd, localeInfo.UI_DEF_FONT)

    def SetFontColor(self, red, green, blue, alpha=1.0):
        wndMgr.SetFontColor(self.hWnd, red, green, blue, alpha)

    def SetPackedFontColor(self, color):
        wndMgr.SetFontColor(self.hWnd, color)

    def SetPackedFontGradient(self, color, color2):
        wndMgr.SetFontColor(self.hWnd, color, color2)

    def SetText(self, text):
        wndMgr.SetText(self.hWnd, str(text))

    def GetText(self):
        return wndMgr.GetText(self.hWnd)

    def GetTextLineCount(self):
        return wndMgr.GetTextLineCount(self.hWnd)

    def GetHyperlinkAtPos(self, x, y):
        return wndMgr.GetHyperlinkAtPos(self.hWnd, x, y)


class LimitedTextLine(TextLine):
    def __init__(self):
        TextLine.__init__(self)
        self.max = 0
        self.eventLimitedTextOverOut = None
        self.eventLimitedTextOverIn = None
        self.textToolTip = TextToolTip()
        self.SetFontName(localeInfo.UI_DEF_FONT)

    def SetText(self, text):
        fullText = text
        if len(text) > self.max:
            text = text[: self.max]
            self.RemoveFlag("not_pick")
            TextLine.SetText(self, text)

        else:
            self.AddFlag("not_pick")
            TextLine.SetText(self, text)

        self.textToolTip.SetText(fullText)

    def OnMouseOverIn(self):
        self.textToolTip.Show()

        if self.eventLimitedTextOverIn:
            self.eventLimitedTextOverIn()
            return True
        return False

    def OnMouseOverOut(self):
        self.textToolTip.Hide()

        if self.eventLimitedTextOverOut:
            self.eventLimitedTextOverOut()
            return True
        return False

    def SetMouseOverInEvent(self, event):
        self.eventLimitedTextOverIn = MakeEvent(event)

    def SetMouseOverOutEvent(self, event):
        self.eventLimitedTextOverOut = MakeEvent(event)

    def GetMax(self):
        return self.max

    def SetMax(self, max):
        self.max = max


class ExtendedTextLine(Window):
    OBJECT_TYPE_IMAGE = 0
    OBJECT_TYPE_TEXT = 1
    OBJECT_TYPE_HEIGHT = 2

    OBJECT_TAGS = {
        OBJECT_TYPE_IMAGE: "IMAGE",
        OBJECT_TYPE_TEXT: "TEXT",
        OBJECT_TYPE_HEIGHT: "HEIGHT",
    }

    def __init__(self):
        Window.__init__(self)

        self.inputText = ""
        self.childrenList = []

        self.limitWidth = 0
        self.x = 0
        self.maxHeight = 0
        self.extraHeight = 0
        self.big = 0

    def Big(self):
        self.big = 1

    def SetLimitWidth(self, width):
        self.limitWidth = width
        if self.inputText != "":
            self.SetText(self.inputText)

    def SetText(self, text):
        self.childrenList = []
        self.x = 0
        self.maxHeight = 0
        self.extraHeight = 0

        charIndex = 0
        currentWord = ""

        textLine = None

        while charIndex < len(text):
            c = text[charIndex : charIndex + 1]

            # tags
            if c == "<":
                if textLine:
                    self.childrenList.append(textLine)
                    self.x += textLine.GetWidth()
                    self.maxHeight = max(self.maxHeight, textLine.GetHeight() + 2)
                    textLine = None

                tagStart = charIndex
                tagEnd = text[tagStart:].find(">")
                if tagEnd == -1:
                    tagEnd = len(text)
                else:
                    tagEnd += tagStart

                tagNameStart = charIndex + 1
                tagNameEnd = text[tagNameStart:].find(" ")
                if tagNameEnd == -1 or tagNameEnd > tagEnd:
                    tagNameEnd = tagEnd
                else:
                    tagNameEnd += tagNameStart
                tag = text[tagNameStart:tagNameEnd]

                content = {}
                tagContentPos = tagNameEnd + 1
                while tagContentPos < tagEnd:
                    tagContentStart = -1
                    for i in xrange(tagContentPos, tagEnd):
                        if text[i : i + 1] != " " and text[i : i + 1] != "\t":
                            tagContentStart = i
                            break
                    if tagContentStart == -1:
                        break

                    tagContentPos = text[tagContentStart:].find("=") + tagContentStart
                    tagKey = text[tagContentStart:tagContentPos]

                    tagContentPos += 1

                    tagContentEnd = -1
                    isBreakAtSpace = True
                    for i in xrange(tagContentPos, tagEnd + 1):
                        if isBreakAtSpace == True and (
                            text[i : i + 1] == " "
                            or text[i : i + 1] == "\t"
                            or text[i : i + 1] == ">"
                        ):
                            tagContentEnd = i
                            break
                        elif text[i : i + 1] == '"':
                            if isBreakAtSpace == True:
                                isBreakAtSpace = False
                                tagContentPos = i + 1
                            else:
                                tagContentEnd = i
                                break
                    if tagContentEnd == -1:
                        break

                    tagValue = text[tagContentPos:tagContentEnd]
                    content[tagKey] = tagValue

                    tagContentPos = text[tagContentEnd:].find(" ")
                    if tagContentPos == -1:
                        tagContentPos = tagContentEnd
                    else:
                        tagContentPos += tagContentEnd

                bRet = True
                for key in self.OBJECT_TAGS:
                    if self.OBJECT_TAGS[key] == tag.upper():
                        bRet = self.__ComputeTag(key, content)
                        break

                if bRet == False:
                    break

                charIndex = tagEnd + 1
                continue

            # text
            if not textLine:
                textLine = TextLine()
                textLine.SetParent(self)
                textLine.SetPosition(self.x, 0)
                textLine.SetVerticalAlignCenter()
                if self.big == 1:
                    textLine.SetFontName(localeInfo.UI_DEF_FONT_LARGE)
                textLine.Show()
            subtext = textLine.GetText()
            textLine.SetText(subtext + c)
            if textLine.GetWidth() + self.x >= self.limitWidth and self.limitWidth != 0:
                if subtext != "":
                    textLine.SetText(subtext)
                    self.childrenList.append(textLine)
                    self.x += textLine.GetWidth()
                    self.maxHeight = max(self.maxHeight, textLine.GetHeight() + 2)
                    textLine = None
                else:
                    textLine = None
                break

            # increase char index
            charIndex += 1

        if textLine:
            self.childrenList.append(textLine)
            self.x += textLine.GetWidth()
            self.maxHeight = max(self.maxHeight, textLine.GetHeight() + 2)
            textLine = None

        self.inputText = text[:charIndex]
        self.SetSize(self.x, self.maxHeight + self.extraHeight)
        self.UpdateRect()

        return charIndex

    def __ComputeTag(self, index, content):
        # tag <IMAGE []>
        if index == self.OBJECT_TYPE_IMAGE:
            if not "path" in content:
                logging.error("Cannot read image tag : no path given")
                return False

            image = ImageBox()
            image.SetParent(self)
            image.SetPosition(self.x, 0)
            image.SetVerticalAlignCenter()
            image.LoadImage(content["path"])
            image.Show()

            if "align" in content and content["align"].lower() == "center":
                image.SetPosition(self.limitWidth / 2 - image.GetWidth() / 2, 0)
            else:
                if (
                    self.x + image.GetWidth() >= self.limitWidth
                    and self.limitWidth != 0
                ):
                    return False
                self.x += image.GetWidth()

            self.childrenList.append(image)
            self.maxHeight = max(self.maxHeight, image.GetHeight())

            return True

        # tag <TEXT []>
        elif index == self.OBJECT_TYPE_TEXT:
            if "text" not in content:
                logging.error("Cannot read text tag : no text given")
                return False

            textLine = TextLine()
            textLine.SetParent(self)
            textLine.SetPosition(self.x, 0)
            textLine.SetVerticalAlignCenter()
            if self.big == 1:
                textLine.SetFontName(localeInfo.UI_DEF_FONT_LARGE)
            textLine.SetText(content["text"])
            if "r" in content and "g" in content and "b" in content:
                textLine.SetFontColor(
                    int(content["r"]) / 255.0,
                    int(content["g"]) / 255.0,
                    int(content["b"]) / 255.0,
                )
            if "outline" in content and content["outline"] == "1":
                textLine.SetOutline()
            textLine.Show()

            if self.x + textLine.GetWidth() >= self.limitWidth and self.limitWidth != 0:
                return False

            self.childrenList.append(textLine)
            self.x += textLine.GetWidth()
            self.maxHeight = max(self.maxHeight, textLine.GetHeight() + 2)

            return True

        # tag <HEIGHT []>
        elif index == self.OBJECT_TYPE_HEIGHT:
            if "size" not in content:
                logging.error("Cannot read height tag : no size given")
                return False

            self.extraHeight += int(content["size"])

            return True

        return False


class MultiTextLine(Window):
    RETURN_STRING = "[ENTER]"
    LINE_HEIGHT = 12

    def __init__(self):
        Window.__init__(self)

        self.lines = []
        self.hAlignCenter = False
        self.vAlignCenter = False
        self.text = ""
        self.textFont = ""
        self.textcolor = [255, 255, 255]
        self.basePos = 0
        self.maxTextWidth = 0

    def SetColor(self, color):
        if type(color) == list:
            self.textcolor = color

    def SetTextFont(self, font):
        self.textFont = font
        self.SetText(self.GetText())

    def SetWidth(self, width):
        self.SetSize(width, self.GetHeight())
        self.SetText(self.GetText())

    def SetHeight(self, height):
        self.SetSize(self.GetWidth(), height)
        self.SetText(self.GetText())

    def NewTextLine(self):
        line = TextLine()
        line.SetParent(self)
        if self.textcolor != [255, 255, 255]:
            try:
                line.SetFontColor(
                    self.textcolor[0] * 255,
                    self.textcolor[1] * 255,
                    self.textcolor[2] * 255,
                )
            except:
                pass
        if self.textFont:
            line.SetFontName(self.textFont)
        if self.hAlignCenter == True:
            line.SetHorizontalAlignCenter()
        if self.vAlignCenter == True:
            line.SetVerticalAlignCenter()
        line.Show()
        self.lines.append(line)

        return self.lines[len(self.lines) - 1]

    def Clear(self):
        self.text = ""
        self.lines = []

    def SetTextHorizontalAlignCenter(self):
        self.hAlignCenter = True
        self.SetText(self.GetText())

    def SetTextVerticalAlignCenter(self):
        self.vAlignCenter = True
        self.SetText(self.GetText())

    def SetText(self, text):
        self.Clear()

        self.text = text
        self.maxTextWidth = 0

        line = self.NewTextLine()
        pos = 0
        newStartPos = 0
        while pos < len(text):
            line.SetText(text[: pos + 1])

            newLine = False
            if len(text) >= pos + len(self.RETURN_STRING):
                if text[pos : pos + len(self.RETURN_STRING)] == self.RETURN_STRING:
                    newLine = True
                    newStartPos = pos + len(self.RETURN_STRING)
            if newLine == False and pos > 0:
                if line.GetWidth() > self.GetWidth() and self.GetWidth() > 0:
                    newLine = True

                    curText = text[: pos + 1]
                    breakPos = curText.rfind(" ")
                    if breakPos == -1:
                        breakPos = curText.rfind(".")
                        if breakPos == -1:
                            breakPos = curText.rfind(",")
                            if breakPos == -1:
                                breakPos = curText.rfind(";")
                                if breakPos == -1:
                                    breakPos = curText.rfind(":")
                    if breakPos != -1:
                        pos = breakPos
                        newStartPos = pos + 1
                    else:
                        newStartPos = pos

            if newLine == True:
                line.SetText(text[:pos])

                line = self.NewTextLine()
                text = text[newStartPos:]
                if text[:1] == " ":
                    text = text[1:]
                pos = 0
            else:
                pos += 1

            self.maxTextWidth = max(self.maxTextWidth, line.GetWidth())

        self.ShowBetween(
            self.GetLineCount() - self.GetViewLineCount(), self.GetLineCount() - 1
        )
        # self.SetSize(self.GetWidth(), len(self.lines) * self.LINE_HEIGHT)

    def GetMaxTextWidth(self):
        return self.maxTextWidth

    def GetLine(self, index):
        if index < 0 or index >= len(self.lines):
            return None
        return self.lines[index]

    def GetLastLine(self):
        return self.GetLine(len(self.lines) - 1)

    def GetLineCount(self):
        return len(self.lines)

    def GetViewLineCount(self):
        if self.GetHeight() == 0:
            return self.GetLineCount()
        return self.GetHeight() / self.LINE_HEIGHT

    def SetBasePos(self, basePos):
        self.basePos = basePos

        self.ShowBetween(self.basePos, self.basePos + self.GetViewLineCount() - 1)

    def GetBasePos(self):
        return self.basePos

    def ShowBetween(self, start, end):
        start = max(0, start)
        end = min(self.GetLineCount() - 1, end)

        height = self.GetHeight()
        if height == 0:
            height = self.GetLineCount() * self.LINE_HEIGHT

        for i in xrange(len(self.lines)):
            if i < start or i > end:
                self.lines[i].Hide()
            else:
                self.lines[i].SetPosition(0, (i - start) * self.LINE_HEIGHT)
                self.lines[i].Show()

    def GetRealHeight(self):
        if self.GetHeight() > 0:
            return self.GetHeight()

        return self.GetLineCount() * self.LINE_HEIGHT

    def GetBottom(self):
        return self.GetTop() + self.GetRealHeight()

    def GetText(self):
        return self.text


class EditLine(TextLine):
    def __init__(self):
        TextLine.__init__(self)
        self.eventReturn = None
        self.eventEscape = None
        self.eventTab = None
        self.eventOnChangeEvent = None
        self.eventFocus = None
        self.eventKillFocus = None
        self.eventOnChar = None
        self.eventClick = None
        self.numberMode = False
        self.canEdit = True
        self.hexMode = False
        self.ToolTipText = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterEditLine(self, layer)

    def SetReturnEvent(self, event):
        self.eventReturn = MakeEvent(event)

    def SetEscapeEvent(self, event):
        self.eventEscape = MakeEvent(event)

    def SetCharEvent(self, event):
        self.eventOnChar = event

    def SetTabEvent(self, event):
        self.eventTab = MakeEvent(event)

    def SetOnFocusEvent(self, event):
        self.eventFocus = MakeEvent(event)

    def SetKillFocusEvent(self, event):
        self.eventKillFocus = MakeEvent(event)

    def SetOnChangeEvent(self, event):
        self.eventOnChangeEvent = MakeEvent(event)

    def SetOnMouseLeftButtonDownEvent(self, event):
        self.eventClick = MakeEvent(event)

    def SetToolTipWindow(self, toolTip):
        self.ToolTipText = toolTip
        self.ToolTipText.SetParent(self)

    def SetToolTipText(self, text, x=0, y=-19):
        if not self.ToolTipText:
            toolTip = TextLine()
            toolTip.SetParent(self)
            toolTip.SetHorizontalAlignCenter()
            toolTip.SetOutline()
            toolTip.Hide()
            toolTip.SetPosition(x, y)
            self.ToolTipText = toolTip

        self.ToolTipText.SetText(text)

    def CanEdit(self, val):
        self.canEdit = val

    def SetMax(self, max):
        wndMgr.SetMax(self.hWnd, max)

    def SetMaxVisible(self, max):
        wndMgr.SetMaxVisible(self.hWnd, max)

    def SetNormalMode(self):
        self.numberMode = False
        self.hexMode = False

    def SetNumberMode(self):
        self.numberMode = True

    def SetHexMode(self):
        self.hexMode = True

    def SetEndPosition(self):
        wndMgr.MoveToEnd(self.hWnd)

    def SetTextColor(self, color):
        TextLine.SetPackedFontColor(self, color)

    def SetPlaceHolderText(self, text):
        wndMgr.SetPlaceholderText(self.hWnd, text)

    def SetPlaceHolderTextColor(self, color):
        wndMgr.SetPlaceholderColor(self.hWnd, color)

    def Insert(self, str, pos=None):
        if None != pos:
            wndMgr.Insert(self.hWnd, str, pos)
        else:
            wndMgr.Insert(self.hWnd, str)

    def Erase(self, count, pos=None):
        if None != pos:
            wndMgr.Erase(self.hWnd, count, pos)
        else:
            wndMgr.Erase(self.hWnd, count)

    def GetCursorPosition(self):
        return wndMgr.GetCursorPosition(self.hWnd)

    def GetHyperlinkAtPos(self, x, y):
        return wndMgr.GetHyperlinkAtPos(self.hWnd, x, y)

    def Disable(self):
        return wndMgr.DisableEditLine(self.hWnd)

    def Enable(self):
        return wndMgr.EnableEditLine(self.hWnd)

    # called by EditControl::OnKeyDown if key is unhandled
    def OnKeyDown(self, key):
        if not self.canEdit:
            return False

        if self.eventOnChangeEvent:
            self.eventOnChangeEvent()

        if key == app.VK_ESCAPE and self.eventEscape:
            self.eventEscape()
            return True

        if key == app.VK_RETURN and self.eventReturn:
            snd.PlaySound("sound/ui/click.wav")
            self.eventReturn()
            return True

        # Prevent other windows from processing OnKeyDown events
        # that might get translated to characters by pretending
        # that we handled them.
        return True

    def OnKeyUp(self):
        return False

    def OnChar(self, ch):
        if not self.canEdit:
            return False

        snd.PlaySound("sound/ui/type.wav")
        if self.numberMode and (ch < ord("0") or ch > ord("9")):
            return False

        if self.hexMode and (chr(ch) not in string.hexdigits and chr(ch) != "#"):
            return False

        if self.eventOnChar:
            return self.eventOnChar()

        return True

    def OnPaste(self, st):
        if not self.canEdit:
            return False

        if self.numberMode and not st.isdigit():
            return False

        if self.hexMode and (not all(c in string.hexdigits for c in st)):
            return False

        return True

    def OnTab(self):
        if self.eventTab:
            self.eventTab()
            return True

        return False

    def OnSetFocus(self):
        if self.eventFocus:
            self.eventFocus()
            return True

        return False

    def OnKillFocus(self):
        if self.eventKillFocus:
            self.eventKillFocus()
            return True

        return False

    def OnMouseLeftButtonDown(self):
        if not self.canEdit:
            return False

        if self.eventClick:
            return self.eventClick()

        return False


class MarkBox(Window):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterMarkBox(self, layer)

    def Load(self):
        wndMgr.MarkBox_Load(self.hWnd)

    def SetScale(self, scale):
        wndMgr.MarkBox_SetScale(self.hWnd, scale)

    def SetIndex(self, guildID):
        MarkID = guild.GuildIDToMarkID(guildID)
        wndMgr.MarkBox_SetImageFilename(
            self.hWnd, guild.GetMarkImageFilenameByMarkID(MarkID)
        )
        wndMgr.MarkBox_SetIndex(self.hWnd, guild.GetMarkIndexByMarkID(MarkID))

    def SetAlpha(self, alpha):
        wndMgr.MarkBox_SetDiffuseColor(self.hWnd, 1.0, 1.0, 1.0, alpha)


class ImageBox(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.eventImageMouseOverIn = None
        self.eventImageMouseOverOut = None
        self.eventImageClick = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterImageBox(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def LoadImage(self, imageName):
        self.name = imageName
        wndMgr.LoadImage(self.hWnd, imageName)

    def SetAlpha(self, alpha):
        wndMgr.SetAlpha(self.hWnd, alpha)

    def SetDiffuseColor(self, r, g, b, a=1.0):
        wndMgr.SetDiffuseColor(self.hWnd, r, g, b, a)

    def GetWidth(self):
        return wndMgr.GetWidth(self.hWnd)

    def GetHeight(self):
        return wndMgr.GetHeight(self.hWnd)

    def SetClickEvent(self, event):
        self.eventImageClick = MakeEvent(event)

    def SetOnMouseOverInEvent(self, event):
        self.eventImageMouseOverIn = event

    def SetOnMouseOverOutEvent(self, event):
        self.eventImageMouseOverOut = event

    def OnMouseLeftButtonUp(self):
        if self.eventImageClick:
            self.eventImageClick()
            return True

        return False

    def OnMouseOverIn(self):
        if self.eventImageMouseOverIn:
            self.eventImageMouseOverIn()
            return True

        return False

    def OnMouseOverOut(self):
        if self.eventImageMouseOverOut:
            self.eventImageMouseOverOut()
            return True

        return False

    def LeftRightReverse(self):
        wndMgr.LeftRightReverseImageBox(self.hWnd)

    def SetCoolTime(self, time):
        wndMgr.SetCoolTimeImageBox(self.hWnd, time)

    def SetStartCoolTime(self, time):
        wndMgr.SetStartCoolTimeImageBox(self.hWnd, time)

    def SetPercentage(self, curValue, maxValue):
        if maxValue:
            self.SetRenderingRect(
                0.0, 0.0, -1.0 + float(curValue) / float(maxValue), 0.0
            )
        else:
            self.SetRenderingRect(0.0, 0.0, 0.0, 0.0)

    def SetRenderingRect(self, left, top, right, bottom):
        wndMgr.SetRenderingRect(self.hWnd, left, top, right, bottom)


class ExpandedImageBox(ImageBox):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterExpandedImageBox(self, layer)

    def SetScale(self, xScale, yScale):
        wndMgr.SetScale(self.hWnd, xScale, yScale)

    def SetOrigin(self, x, y):
        wndMgr.SetOrigin(self.hWnd, x, y)

    def SetRotation(self, rotation):
        wndMgr.SetRotation(self.hWnd, rotation)

    def SetRenderingMode(self, mode):
        wndMgr.SetRenderingMode(self.hWnd, mode)

    def SetRenderingRect(self, left, top, right, bottom):
        wndMgr.SetRenderingRect(self.hWnd, left, top, right, bottom)

    def SetPercentage(self, curValue, maxValue):
        if maxValue:
            self.SetRenderingRect(
                0.0, 0.0, -1.0 + float(curValue) / float(maxValue), 0.0
            )
        else:
            self.SetRenderingRect(0.0, 0.0, 0.0, 0.0)

    def GetWidth(self):
        return wndMgr.GetWindowWidth(self.hWnd)

    def GetHeight(self):
        return wndMgr.GetWindowHeight(self.hWnd)

    def SetMouseLeftButtonDownEvent(self, event):
        self.renderTargetMouseEvent = MakeEvent(event)


class AniImageBox(Window):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterAniImageBox(self, layer)
        self.end_frame_event = None
        self.key_frame_event = None
        self.aniBoxClickEvent = None
        self.aniBoxOverInEvent = None
        self.aniBoxOverOutEvent = None

    def SetDelay(self, delay):
        wndMgr.SetDelay(self.hWnd, delay)

    def SetRenderingRect(self, left, top, right, bottom):
        wndMgr.SetRenderingRect(self.hWnd, left, top, right, bottom)

    def AppendImage(self, filename):
        wndMgr.AppendImage(self.hWnd, filename)

    def Start(self):
        wndMgr.StartAnimation(self.hWnd)

    def Stop(self):
        wndMgr.StopAnimation(self.hWnd)

    def SetPercentage(self, curValue, maxValue):
        wndMgr.SetRenderingRect(
            self.hWnd, 0.0, 0.0, -1.0 + float(curValue) / float(maxValue), 0.0
        )

    def OnEndFrame(self):
        if self.end_frame_event:
            self.end_frame_event()

    def SetPercentageWithScale(self, curValue, maxValue):
        wndMgr.SetRenderingRectWithScale(
            self.hWnd, 0.0, 0.0, -1.0 + float(curValue) / float(maxValue), 0.0
        )

    def SetScale(self, xScale, yScale):
        wndMgr.SetAniImgScale(self.hWnd, xScale, yScale)

    def SetDiffuseColor(self, r, g, b, a=1.0):
        wndMgr.SetAniImgDiffuseColor(self.hWnd, r, g, b, a)

    def SetEndFrameEvent(self, event):
        self.end_frame_event = event

    def ResetFrame(self):
        wndMgr.ResetFrame(self.hWnd)

    def OnKeyFrame(self, cur_frame):
        if self.key_frame_event:
            self.key_frame_event(cur_frame)

    def SetKeyFrameEvent(self, event):
        self.key_frame_event = MakeEvent(event)

    def SetMouseLeftButtonUpEvent(self, event):
        self.aniBoxClickEvent = MakeEvent(event)

    def SetMouseOverInEvent(self, event):
        self.aniBoxOverInEvent = MakeEvent(event)

    def SetMouseOverOutEvent(self, event):
        self.aniBoxOverOutEvent = MakeEvent(event)

    def OnMouseLeftButtonUp(self):
        if self.aniBoxClickEvent:
            self.aniBoxClickEvent()

    def OnMouseOverIn(self):
        if self.aniBoxOverInEvent:
            self.aniBoxOverInEvent()

    def OnMouseOverOut(self):
        if self.aniBoxOverOutEvent:
            self.aniBoxOverOutEvent()


class MoveTextLine(TextLine):
    def __init__(self):
        TextLine.__init__(self)
        self.end_move_event = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterMoveTextLine(self, layer)

    def SetMovePosition(self, dst_x, dst_y):
        wndMgr.SetMovePosition(self.hWnd, dst_x, dst_y)

    def SetMoveSpeed(self, speed):
        wndMgr.SetMoveSpeed(self.hWnd, speed)

    def MoveStart(self):
        wndMgr.MoveStart(self.hWnd)

    def MoveStop(self):
        wndMgr.MoveStop(self.hWnd)

    def GetMove(self):
        return wndMgr.GetMove(self.hWnd)

    def OnEndMove(self):
        if self.end_move_event:
            self.end_move_event()

    def SetEndMoveEvent(self, event):
        self.end_move_event = MakeEvent(event)


class MoveImageBox(ImageBox):
    def __init__(self, layer="UI"):
        ImageBox.__init__(self, layer)
        self.end_move_event = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterMoveImageBox(self, layer)

    def MoveStart(self):
        wndMgr.MoveStart(self.hWnd)

    def MoveStop(self):
        wndMgr.MoveStop(self.hWnd)

    def GetMove(self):
        return wndMgr.GetMove(self.hWnd)

    def SetMovePosition(self, dst_x, dst_y):
        wndMgr.SetMovePosition(self.hWnd, dst_x, dst_y)

    def SetMoveSpeed(self, speed):
        wndMgr.SetMoveSpeed(self.hWnd, speed)

    def OnEndMove(self):
        if self.end_move_event:
            self.end_move_event()

    def SetEndMoveEvent(self, event):
        self.end_move_event = event


class MoveScaleImageBox(MoveImageBox):
    def __init__(self, layer="UI"):
        MoveImageBox.__init__(self, layer)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterMoveScaleImageBox(self, layer)

    def SetMaxScale(self, scale):
        wndMgr.SetMaxScale(self.hWnd, scale)

    def SetMaxScaleRate(self, pivot):
        wndMgr.SetMaxScaleRate(self.hWnd, pivot)

    def SetScalePivotCenter(self, flag):
        wndMgr.SetScalePivotCenter(self.hWnd, flag)


class Button(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.buttonEvent = None
        self.buttonDownEvent = None
        self.buttonOverEvent = None
        self.buttonOverOutEvent = None
        self.eventShowToolTipEvent = None
        self.eventHideToolTipEvent = None
        self.extendedData = None
        self.buttonFontName = ""
        self.tooltipX = 0
        self.tooltipY = -12

        self.texts = []
        self.ToolTipText = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterButton(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetUpVisual(self, filename):
        wndMgr.SetUpVisual(self.hWnd, filename)

    def SetOverVisual(self, filename):
        wndMgr.SetOverVisual(self.hWnd, filename)

    def SetDownVisual(self, filename):
        wndMgr.SetDownVisual(self.hWnd, filename)

    def SetDisableVisual(self, filename):
        wndMgr.SetDisableVisual(self.hWnd, filename)

    def GetUpVisualFileName(self):
        return wndMgr.GetUpVisualFileName(self.hWnd)

    def GetOverVisualFileName(self):
        return wndMgr.GetOverVisualFileName(self.hWnd)

    def GetDownVisualFileName(self):
        return wndMgr.GetDownVisualFileName(self.hWnd)

    def Flash(self):
        wndMgr.Flash(self.hWnd)

    def EnableFlash(self):
        wndMgr.EnableFlash(self.hWnd)

    def DisableFlash(self):
        wndMgr.DisableFlash(self.hWnd)

    def IsEnabled(self):
        return wndMgr.IsEnabled(self.hWnd)

    def Enable(self):
        wndMgr.Enable(self.hWnd)

    def Disable(self):
        wndMgr.Disable(self.hWnd)

    def Down(self):
        wndMgr.Down(self.hWnd)

    def SetUp(self):
        wndMgr.SetUp(self.hWnd)

    def SetEvent(self, event):
        self.buttonEvent = MakeEvent(event)

    def SetDownEvent(self, event):
        self.buttonDownEvent = MakeEvent(event)

    def Over(self):
        wndMgr.Over(self.hWnd)

    def LeftRightReverse(self):
        wndMgr.LeftRightReverse(self.hWnd)

    if app.ENABLE_PRIVATESHOP_SEARCH_SYSTEM:

        def SetAlpha(self, alpha):
            wndMgr.SetButtonDiffuseColor(self.hWnd, 1.0, 1.0, 1.0, alpha)

        def GetText(self):
            try:
                return self.texts[0].GetText()
            except KeyError:
                return ""

        def IsDisable(self):
            return wndMgr.IsDisable(self.hWnd)

    def ClearText(self):
        self.texts = []

    def SetTextColor(self, color):
        if not self.texts:
            return
        self.texts[0].SetPackedFontColor(color)

    def SetText(self, text, height=0):
        if not self.texts or len(self.texts) > 1:
            textLine = TextLine()
            textLine.SetWindowName("ButtonText")
            textLine.SetParent(self)
            textLine.SetVerticalAlignCenter()
            textLine.SetHorizontalAlignCenter()
            if len(self.buttonFontName) > 0:
                textLine.SetFontName(self.buttonFontName)
            textLine.SetPosition(0, height)
            textLine.Show()
            self.texts = [textLine]

        self.texts[0].SetText(text)

    def AppendTextWindow(self, textLine):
        if not self.texts:
            self.texts = [textLine]
        else:
            self.texts.append(textLine)

    def SetAutoSizeText(self, text):
        if not self.texts or len(self.texts) > 1:
            textLine = TextLine()
            textLine.SetWindowName("ButtonText")
            textLine.SetParent(self)
            textLine.SetVerticalAlignCenter()
            textLine.SetHorizontalAlignCenter()
            if len(self.buttonFontName) > 0:
                textLine.SetFontName(self.buttonFontName)
            textLine.Show()
            self.texts = [textLine]

        self.texts[0].SetText(text)

        imageWidth = self.GetButtonImageWidth()
        imageHeight = self.GetButtonImageHeight()

        if imageWidth == 0 or imageHeight == 0:
            return

        (textWidth, textHeight) = (self.texts[0].GetWidth(), self.texts[0].GetHeight())
        textWidth += imageWidth / 5
        textHeight += imageHeight / 2

        curWidth = textWidth
        curHeight = textHeight

        self.SetSize(curWidth, curHeight)
        self.SetButtonScale(curWidth * 1.0 / imageWidth, curHeight * 1.0 / imageHeight)
        self.texts[0].SetPosition(0, 0)

    def SetTextPosition(self, x, y):
        self.texts[0].SetPosition(x, y)

    def SetTextAddPos(self, text, x_add=0, y_add=0, height=4):
        if not self.texts[0]:
            textLine = TextLine()
            textLine.SetParent(self)
            textLine.SetPosition(
                self.GetWidth() / 2 + x_add, self.GetHeight() / 2 + y_add
            )
            textLine.SetVerticalAlignCenter()
            textLine.SetHorizontalAlignCenter()
            textLine.Show()
            self.texts[0] = textLine
        self.texts[0].SetText(text)

    def SetTextAlignLeft(self, text, height=4):
        if not self.texts or len(self.texts) > 1:
            textLine = TextLine()
            textLine.SetWindowName("ButtonText")
            textLine.SetParent(self)
            textLine.SetVerticalAlignCenter()
            textLine.SetHorizontalAlignLeft()
            textLine.SetPosition(0, height)
            textLine.Show()
            self.texts = [textLine]

        self.texts[0].SetText(text)

    def SetAppendTextChangeText(self, idx, text):
        if not len(self.texts):
            return

        self.texts[idx].SetText(text)

    def SetAppendTextColor(self, idx, color):
        if not len(self.texts):
            return

        self.texts[idx].SetPackedFontColor(color)

    def AppendTextLine(
        self,
        text,
        font_size=localeInfo.UI_DEF_FONT,
        font_color=grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0),
        text_sort="center",
        pos_x=None,
        pos_y=None,
    ):
        textLine = TextLine()
        textLine.SetParent(self)
        textLine.SetFontName(font_size)
        textLine.SetPackedFontColor(font_color)
        textLine.SetText(text)
        textLine.Show()

        if not pos_x and not pos_y:
            textLine.SetPosition(self.GetWidth() / 2, self.GetHeight() / 2)
        else:
            textLine.SetPosition(pos_x, pos_y)

        textLine.SetVerticalAlignCenter()
        if "center" == text_sort:
            textLine.SetHorizontalAlignCenter()
        elif "right" == text_sort:
            textLine.SetHorizontalAlignRight()
        elif "left" == text_sort:
            textLine.SetHorizontalAlignLeft()

        self.AppendTextWindow(textLine)

    def SetToolTipWindow(self, toolTip):
        self.ToolTipText = toolTip
        self.ToolTipText.SetParent(self)

    def SetToolTipText(self, text, x=0, y=-19):
        if not self.ToolTipText:
            toolTip = TextLine("TOP_MOST")
            toolTip.SetOutline()
            toolTip.Hide()
            toolTip.SetPosition(
                self.GetGlobalPosition()[0] + x, self.GetGlobalPosition()[1] + y
            )
            self.ToolTipText = toolTip

        self.tooltipX = x
        self.tooltipY = y

        self.ToolTipText.SetText(text)

    def CallEvent(self):
        snd.PlaySound("sound/ui/click.wav")

        if self.buttonEvent:
            self.buttonEvent()

    def DownEvent(self):
        if self.buttonDownEvent:
            self.buttonDownEvent()

    def ShowToolTip(self):
        if self.ToolTipText:
            self.ToolTipText.SetPosition(
                self.GetGlobalPosition()[0] + self.tooltipX,
                self.GetGlobalPosition()[1] + self.tooltipY,
            )
            self.ToolTipText.Show()

        if self.eventShowToolTipEvent:
            self.eventHideToolTipEvent()

    def HideToolTip(self):
        if self.ToolTipText:
            self.ToolTipText.Hide()

        if self.eventHideToolTipEvent:
            self.eventHideToolTipEvent()

    def SetShowToolTipEvent(self, event):
        self.eventHideToolTipEvent = MakeEvent(event)

    def SetHideToolTipEvent(self, event):
        self.eventHideToolTipEvent = MakeEvent(event)

    def IsDown(self):
        return wndMgr.IsDown(self.hWnd)

    def SetAlwaysToolTip(self, bFlag):
        wndMgr.SetAlwaysToolTip(self.hWnd, bFlag)

    def OnMouseOverIn(self):
        if self.buttonOverEvent:
            self.buttonOverEvent()
            return True
        return False

    def OnMouseOverOut(self):
        if self.buttonOverOutEvent:
            self.buttonOverOutEvent()
            return True
        return False

    def SetOverEvent(self, event):
        self.buttonOverEvent = MakeEvent(event)

    def SetOverOutEvent(self, event):
        self.buttonOverOutEvent = MakeEvent(event)

    def SetButtonScale(self, xScale, yScale):
        wndMgr.SetButtonScale(self.hWnd, xScale, yScale)

    def GetButtonImageWidth(self):
        return wndMgr.GetButtonImageWidth(self.hWnd)

    def GetButtonImageHeight(self):
        return wndMgr.GetButtonImageHeight(self.hWnd)

    def SetExtendedData(self, data):
        self.extendedData = data

    def GetExtendedData(self):
        return self.extendedData

    def HasExtendedData(self):
        return self.extendedData is not None

    def SetFontName(self, fontname):
        self.buttonFontName = fontname
        for text in self.texts:
            text.SetFontName(self.buttonFontName)

    def SetPackedFontColor(self, color):
        for text in self.texts:
            text.SetPackedFontColor(color)

    def SetPackedFontGradient(self, color, color2):
        for text in self.texts:
            text.SetPackedFontGradient(color, color2)

    def SetFontOutline(self, val=True):
        for text in self.texts:
            text.SetOutline(val)


class RadioButton(Button):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterRadioButton(self, layer)


class RadioButtonGroup(object):
    def __init__(self):
        self.buttonGroup = []
        self.selectedBtnIdx = -1

    def Show(self):
        for button, selectEvent, unselectEvent in self.buttonGroup:
            button.Show()

    def Hide(self):
        for button, selectEvent, unselectEvent in self.buttonGroup:
            button.Hide()

    def SetText(self, idx, text):
        if idx >= len(self.buttonGroup):
            return

        (button, selectEvent, unselectEvent) = self.buttonGroup[idx]
        button.SetText(text)

    def OnClick(self, btnIdx):
        if btnIdx == self.selectedBtnIdx:
            return

        (button, selectEvent, unselectEvent) = self.buttonGroup[self.selectedBtnIdx]
        if unselectEvent:
            unselectEvent()

        button.SetUp()

        self.selectedBtnIdx = btnIdx
        (button, selectEvent, unselectEvent) = self.buttonGroup[btnIdx]
        if selectEvent:
            selectEvent()

        button.Down()

    def AddButton(self, button, selectEvent, unselectEvent):
        i = len(self.buttonGroup)
        button.SetEvent(Event(self.OnClick, i))
        self.buttonGroup.append(
            [button, MakeEvent(selectEvent), MakeEvent(unselectEvent)]
        )
        button.SetUp()

    @staticmethod
    def Create(rawButtonGroup):
        radioGroup = RadioButtonGroup()
        for button, selectEvent, unselectEvent in rawButtonGroup:
            radioGroup.AddButton(button, selectEvent, unselectEvent)

        radioGroup.OnClick(0)
        return radioGroup

    @staticmethod
    def CreateSelectDefault(rawButtonGroup, default):
        radioGroup = RadioButtonGroup()
        for button, selectEvent, unselectEvent in rawButtonGroup:
            radioGroup.AddButton(button, selectEvent, unselectEvent)

        radioGroup.OnClick(default)
        return radioGroup


class ToggleButton(Button):
    def __init__(self):
        Button.__init__(self)

        self.eventUp = None
        self.eventDown = None

    def SetToggleUpEvent(self, event):
        self.eventUp = MakeEvent(event)

    def SetToggleDownEvent(self, event):
        self.eventDown = MakeEvent(event)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterToggleButton(self, layer)

    def OnToggleUp(self):
        if self.eventUp:
            self.eventUp()

    def OnToggleDown(self):
        if self.eventDown:
            self.eventDown()


class DragButton(Button):
    def __init__(self):
        Button.__init__(self)
        self.AddFlag("moveable")

        self.callbackEnable = True
        self.eventMove = None
        self.onMouseLeftButtonUpEvent = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterDragButton(self, layer)

    def SetMoveEvent(self, event):
        self.eventMove = MakeEvent(event)

    def SetOnMouseLeftButtonUpEvent(self, event):
        self.onMouseLeftButtonUpEvent = MakeEvent(event)

    def SetRestrictMovementArea(self, x, y, width, height):
        wndMgr.SetRestrictMovementArea(self.hWnd, x, y, width, height)

    def OnMouseLeftButtonUp(self):
        if self.onMouseLeftButtonUpEvent:
            self.onMouseLeftButtonUpEvent()

    def OnMove(self):
        if not self.eventMove or not self.callbackEnable:
            return
        # Prevent recursion
        # TODO(tim): This isn't really necessary, our callers are just broken.
        # Fix them (use SetRestrictMovementArea())
        self.callbackEnable = False
        self.eventMove()
        self.callbackEnable = True


class NumberLine(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterNumberLine(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetHorizontalAlignCenter(self):
        wndMgr.SetNumberHorizontalAlignCenter(self.hWnd)

    def SetHorizontalAlignRight(self):
        wndMgr.SetNumberHorizontalAlignRight(self.hWnd)

    def SetVerticalAlignCenter(self):
        wndMgr.SetNumberVerticalAlignCenter(self.hWnd)

    def SetPath(self, path):
        wndMgr.SetPath(self.hWnd, path)

    def SetNumber(self, number):
        wndMgr.SetNumber(self.hWnd, number)


###################################################################################################
## PythonScript Element
###################################################################################################


class Box(Window):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterBox(self, layer)
        self.SetWindowName(self.__class__.__name__)
        self.eventBoxClick = None

    def SetColor(self, color):
        wndMgr.SetColor(self.hWnd, color)

    def SetClickEvent(self, event):
        self.eventBoxClick = MakeEvent(event)

    def OnMouseLeftButtonDown(self):
        if self.eventBoxClick:
            return self.eventBoxClick()
        return False


class Bar(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.eventBarMouseIn = None
        self.eventBarMouseOut = None
        self.eventBarMouseDown = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterBar(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetColor(self, color):
        wndMgr.SetColor(self.hWnd, color)

    def SetMouseOverIn(self, event):
        self.eventBarMouseIn = MakeEvent(event)

    def SetMouseOverOut(self, event):
        self.eventBarMouseOut = MakeEvent(event)

    def SetMouseLeftButtonDown(self, event):
        self.eventBarMouseDown = MakeEvent(event)

    def OnMouseOverIn(self):
        if self.eventBarMouseIn:
            self.eventBarMouseIn()

    def OnMouseOverOut(self):
        if self.eventBarMouseOut:
            self.eventBarMouseOut()

    def OnMouseLeftButtonDown(self):
        if self.eventBarMouseDown:
            return self.eventBarMouseDown()
        return False


class DragBar(Bar):
    def __init__(self):
        Bar.__init__(self)
        self.AddFlag("moveable")

        self.callbackEnable = True
        self.eventMove = None
        self.onMouseLeftButtonUpEvent = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterDragBar(self, layer)

    def SetMoveEvent(self, event):
        self.eventMove = MakeEvent(event)

    def SetOnMouseLeftButtonUpEvent(self, event):
        self.onMouseLeftButtonUpEvent = MakeEvent(event)

    def SetRestrictMovementArea(self, x, y, width, height):
        wndMgr.SetRestrictMovementArea(self.hWnd, x, y, width, height)

    def OnMouseLeftButtonUp(self):
        if self.onMouseLeftButtonUpEvent:
            self.onMouseLeftButtonUpEvent()

    def OnMove(self):
        if not self.eventMove or not self.callbackEnable:
            return
        # Prevent recursion
        # TODO(tim): This isn't really necessary, our callers are just broken.
        # Fix them (use SetRestrictMovementArea())
        self.callbackEnable = False
        self.eventMove()
        self.callbackEnable = True


class Line(Window):
    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterLine(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetColor(self, color):
        wndMgr.SetColor(self.hWnd, color)


class SlotBar(Window):
    def __init__(self):
        Window.__init__(self)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterBar3D(self, layer)
        self.SetWindowName(self.__class__.__name__)


## Same with SlotBar
class Bar3D(Window):
    def __init__(self):
        Window.__init__(self)

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterBar3D(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetColor(self, left, right, center):
        wndMgr.SetColor(self.hWnd, left, right, center)


class SlotWindow(Window):
    def __init__(self):
        Window.__init__(self)

        self.StartIndex = 0

        self.eventSelectEmptySlot = None
        self.eventSelectItemSlot = None
        self.eventUnselectEmptySlot = None
        self.eventUnselectItemSlot = None
        self.eventUseSlot = None
        self.eventOverInItem = None
        self.eventOverOutItem = None
        self.eventPressedSlotButton = None
        self.useMode = False
        self.slotList = []

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterSlotWindow(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def SetWindowType(self, wtype):
        wndMgr.SetWindowType(self.hWnd, wtype)

    def SetSlotStyle(self, style):
        wndMgr.SetSlotStyle(self.hWnd, style)

    def HasSlot(self, slotIndex):
        return wndMgr.HasSlot(self.hWnd, slotIndex)

    def SetSlotScale(self, xScale, yScale):
        wndMgr.SetSlotScale(self.hWnd, xScale, yScale)

    def SetSlotBaseImage(self, imageFileName, r, g, b, a):
        wndMgr.SetSlotBaseImage(self.hWnd, imageFileName, r, g, b, a)

    def SetBaseImageScale(self, xScale, yScale):
        wndMgr.SetBaseImageScale(self.hWnd, xScale, yScale)

    def SetCoverButton(
        self,
        slotIndex,
        upName="d:/ymir work/ui/public/slot_cover_button_01.sub",
        overName="d:/ymir work/ui/public/slot_cover_button_02.sub",
        downName="d:/ymir work/ui/public/slot_cover_button_03.sub",
        disableName="d:/ymir work/ui/public/slot_cover_button_04.sub",
        LeftButtonEnable=False,
        RightButtonEnable=True,
    ):
        wndMgr.SetCoverButton(
            self.hWnd,
            slotIndex,
            upName,
            overName,
            downName,
            disableName,
            LeftButtonEnable,
            RightButtonEnable,
        )

    def SetCorverButtonScale(self, slotIndex, xScale, yScale):
        wndMgr.SetCorverButtonScale(self.hWnd, slotIndex, xScale, yScale)

    def EnableCoverButton(self, slotIndex):
        wndMgr.EnableCoverButton(self.hWnd, slotIndex)

    def DisableCoverButton(self, slotIndex):
        wndMgr.DisableCoverButton(self.hWnd, slotIndex)

    def SetAlwaysRenderCoverButton(self, slotIndex, bAlwaysRender=True):
        wndMgr.SetAlwaysRenderCoverButton(self.hWnd, slotIndex, bAlwaysRender)

    def AppendSlotButton(self, upName, overName, downName):
        wndMgr.AppendSlotButton(self.hWnd, upName, overName, downName)

    def SetSlotButton(self, slotIndex, upName, overName, downName):
        wndMgr.SetSlotButton(self.hWnd, slotIndex, upName, overName, downName)

    def ShowSlotButton(self, slotNumber):
        wndMgr.ShowSlotButton(self.hWnd, slotNumber)

    def SetSlotButtonPosition(self, slotNumber, x, y):
        wndMgr.SetSlotButtonPosition(self.hWnd, slotNumber, x, y)

    def HideAllSlotButton(self):
        wndMgr.HideAllSlotButton(self.hWnd)

    def AppendRequirementSignImage(self, filename):
        wndMgr.AppendRequirementSignImage(self.hWnd, filename)

    def ShowRequirementSign(self, slotNumber):
        wndMgr.ShowRequirementSign(self.hWnd, slotNumber)

    def HideRequirementSign(self, slotNumber):
        wndMgr.HideRequirementSign(self.hWnd, slotNumber)

    def ActivateSlot(self, slotNumber):
        wndMgr.ActivateSlot(self.hWnd, slotNumber)

    if app.ENABLE_ACCE_COSTUME_SYSTEM:

        def SetSlotDiffuseColor(self, slotindex, colortype):
            wndMgr.SetSlotDiffuseColor(self.hWnd, slotindex, colortype)

    def DeactivateSlot(self, slotNumber):
        wndMgr.DeactivateSlot(self.hWnd, slotNumber)

    def ShowSlotBaseImage(self, slotNumber):
        wndMgr.ShowSlotBaseImage(self.hWnd, slotNumber)

    def HideSlotBaseImage(self, slotNumber):
        wndMgr.HideSlotBaseImage(self.hWnd, slotNumber)

    def SetButtonEvent(self, button, state, event):
        if "LEFT" == button:
            if "EMPTY" == state:
                self.eventSelectEmptySlot = MakeEvent(event)
            elif "EXIST" == state:
                self.eventSelectItemSlot = MakeEvent(event)
            elif "ALWAYS" == state:
                self.eventSelectEmptySlot = MakeEvent(event)
                self.eventSelectItemSlot = MakeEvent(event)
        elif "RIGHT" == button:
            if "EMPTY" == state:
                self.eventUnselectEmptySlot = MakeEvent(event)
            elif "EXIST" == state:
                self.eventUnselectItemSlot = MakeEvent(event)
            elif "ALWAYS" == state:
                self.eventUnselectEmptySlot = MakeEvent(event)
                self.eventUnselectItemSlot = MakeEvent(event)

    def SetSelectEmptySlotEvent(self, empty):
        self.eventSelectEmptySlot = empty

    def SetSelectItemSlotEvent(self, item):
        self.eventSelectItemSlot = item

    def SetUnselectEmptySlotEvent(self, empty):
        self.eventUnselectEmptySlot = empty

    def SetUnselectItemSlotEvent(self, item):
        self.eventUnselectItemSlot = item

    def SetUseSlotEvent(self, use):
        self.eventUseSlot = use

    def SetOverInItemEvent(self, event):
        self.eventOverInItem = MakeEvent(event)

    def SetOverOutItemEvent(self, event):
        self.eventOverOutItem = MakeEvent(event)

    def SetPressedSlotButtonEvent(self, event):
        self.eventPressedSlotButton = MakeEvent(event)

    def GetSlotCount(self):
        return wndMgr.GetSlotCount(self.hWnd)

    def SetUseMode(self, flag):
        "TrueÀÏ¶§¸¸ ItemToItem ÀÌ °¡´ÉÇÑÁö º¸¿©ÁØ´Ù"
        wndMgr.SetUseMode(self.hWnd, flag)
        self.useMode = flag

    def SetUsableItem(self, flag):
        "True¸é ÇöÀç °¡¸®Å² ¾ÆÀÌÅÛÀÌ ItemToItem Àû¿ë °¡´ÉÇÏ´Ù"
        wndMgr.SetUsableItem(self.hWnd, flag)

    def GetUseMode(self):
        return self.useMode

    ## Slot
    def SetSlotCoolTime(self, slotIndex, coolTime, elapsedTime=0.0):
        wndMgr.SetSlotCoolTime(self.hWnd, slotIndex, coolTime, elapsedTime)

    def SetSlotCoolTimeColor(self, slotIndex, r, g, b, a):
        wndMgr.SetSlotCoolTimeColor(self.hWnd, slotIndex, r, g, b, a)

    def StoreSlotCoolTime(self, key, slotIndex, coolTime, elapsedTime=0.0):
        wndMgr.StoreSlotCoolTime(self.hWnd, key, slotIndex, coolTime, elapsedTime)

    def RestoreSlotCoolTime(self, key):
        wndMgr.RestoreSlotCoolTime(self.hWnd, key)

    def SelectSlot(self, slotIndex):
        wndMgr.SelectSlot(self.hWnd, slotIndex)

    def ClearSelected(self):
        wndMgr.ClearSelected(self.hWnd)

    def GetSelectedSlotCount(self):
        return wndMgr.GetSelectedSlotCount(self.hWnd)

    def IsSelectedSlot(self, slotIndex):
        return wndMgr.IsSelectedSlot(self.hWnd, slotIndex)

    def GetSelectedSlotNumber(self, slotIndex):
        return wndMgr.GetSelectedSlotNumber(self.hWnd, slotIndex)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def SetSlotCoolTimeInverse(self, slotIndex, coolTime, elapsedTime=0.0):
            wndMgr.SetSlotCoolTimeInverse(self.hWnd, slotIndex, coolTime, elapsedTime)

    if app.ENABLE_GROWTH_PET_SYSTEM or app.ENABLE_MOVE_COSTUME_ATTR:

        def SetCantMouseEventSlot(self, slotIndex):
            wndMgr.SetCantMouseEventSlot(self.hWnd, slotIndex)

    if app.ENABLE_GROWTH_PET_SYSTEM or app.ENABLE_MOVE_COSTUME_ATTR:

        def SetCanMouseEventSlot(self, slotIndex):
            wndMgr.SetCanMouseEventSlot(self.hWnd, slotIndex)

    if app.WJ_ENABLE_TRADABLE_ICON or app.ENABLE_MOVE_COSTUME_ATTR:

        def SetUnusableSlotOnTopWnd(self, slotIndex):
            wndMgr.SetUnusableSlotOnTopWnd(self.hWnd, slotIndex)

        def SetUsableSlotOnTopWnd(self, slotIndex):
            wndMgr.SetUsableSlotOnTopWnd(self.hWnd, slotIndex)

    def DisableSlot(self, slotIndex):
        wndMgr.DisableSlot(self.hWnd, slotIndex)

    def EnableSlot(self, slotIndex):
        wndMgr.EnableSlot(self.hWnd, slotIndex)

    def LockSlot(self, slotIndex):
        wndMgr.LockSlot(self.hWnd, slotIndex)

    def UnlockSlot(self, slotIndex):
        wndMgr.UnlockSlot(self.hWnd, slotIndex)

    def RefreshSlot(self):
        wndMgr.RefreshSlot(self.hWnd)

    def ClearSlot(self, slotNumber):
        wndMgr.ClearSlot(self.hWnd, slotNumber)

    def ClearAllSlot(self):
        wndMgr.ClearAllSlot(self.hWnd)

    def AppendSlot(self, index, x, y, width, height, placementX=0, placementY=0):
        wndMgr.AppendSlot(self.hWnd, index, x, y, width, height, placementX, placementY)

    def SetSlot(
        self,
        slotIndex,
        itemIndex,
        width,
        height,
        icon,
        diffuseColor=(1.0, 1.0, 1.0, 1.0),
    ):
        wndMgr.SetSlot(
            self.hWnd, slotIndex, itemIndex, width, height, icon, diffuseColor
        )

    def SetSlotCount(self, slotNumber, count):
        wndMgr.SetSlotCount(self.hWnd, slotNumber, count)

    def SetRealSlotNumber(self, slotNumber, realSlotNumber):
        wndMgr.SetSlotRealNumber(self.hWnd, slotNumber, realSlotNumber)

    def SetSlotCountNew(self, slotNumber, grade, count):
        wndMgr.SetSlotCountNew(self.hWnd, slotNumber, grade, count)

    def SetSlotSlotNumber(self, slotNumber, grade, count):
        wndMgr.SetSlotSlotNumber(self.hWnd, slotNumber, grade, count)

    def SetSlotSlotText(self, slotNumber, text):
        wndMgr.SetSlotSlotText(self.hWnd, slotNumber, text)

    def SetSlotTextPositon(self, slotNumber, x, y):
        wndMgr.SetSlotTextPositon(self.hWnd, slotNumber, x, y)

    def SetSlotBackground(self, slotIndex, backgroundFilename):
        wndMgr.SetSlotBackground(self.hWnd, slotIndex, backgroundFilename)

    def SetItemSlot(
        self,
        renderingSlotNumber,
        ItemIndex,
        ItemCount=0,
        diffuseColor=(1.0, 1.0, 1.0, 1.0),
    ):
        if 0 == ItemIndex or None == ItemIndex:
            wndMgr.ClearSlot(self.hWnd, renderingSlotNumber)
            return

        item.SelectItem(ItemIndex)
        itemIcon = item.GetIconImageFileName()

        item.SelectItem(ItemIndex)
        (width, height) = item.GetItemSize()

        wndMgr.SetSlot(
            self.hWnd,
            renderingSlotNumber,
            ItemIndex,
            width,
            height,
            itemIcon,
            diffuseColor,
        )
        wndMgr.SetSlotCount(self.hWnd, renderingSlotNumber, ItemCount)

    def SetSkillSlot(self, renderingSlotNumber, skillIndex, skillLevel):

        skillIcon = skill.GetIconImage(skillIndex)

        if 0 == skillIcon:
            wndMgr.ClearSlot(self.hWnd, renderingSlotNumber)
            return

        wndMgr.SetSlot(self.hWnd, renderingSlotNumber, skillIndex, 1, 1, skillIcon)
        wndMgr.SetSlotCount(self.hWnd, renderingSlotNumber, skillLevel)

    def SetSkillSlotNew(
        self, renderingSlotNumber, skillIndex, skillGrade, skillLevel, diffuseColor=None
    ):

        skillIcon = skill.GetIconImageNew(skillIndex, skillGrade)

        if 0 == skillIcon:
            wndMgr.ClearSlot(self.hWnd, renderingSlotNumber)
            return

        if diffuseColor is None:
            wndMgr.SetSlot(self.hWnd, renderingSlotNumber, skillIndex, 1, 1, skillIcon)
        else:
            wndMgr.SetSlot(
                self.hWnd,
                renderingSlotNumber,
                skillIndex,
                1,
                1,
                skillIcon,
                diffuseColor,
            )

    def SetItemDiffuseColor(self, renderingSlotNumber, r, g, b, a):
        wndMgr.SetItemDiffuseColor(self.hWnd, renderingSlotNumber, r, g, b, a)

    def SetEmotionSlot(self, renderingSlotNumber, emotionIndex):
        import emotion

        icon = ""
        try:
            icon = emotion.ICON_DICT[emotionIndex]
        except KeyError:
            pass

        if not icon:
            wndMgr.ClearSlot(self.hWnd, renderingSlotNumber)
            return

        wndMgr.SetSlot(self.hWnd, renderingSlotNumber, emotionIndex, 1, 1, icon)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def SetPetSkillSlotNew(self, renderingSlotNumber, petSkillIndex):

            petSkillIcon = skill.GetPetSkillIconImage(petSkillIndex)

            if 0 == petSkillIcon:
                wndMgr.ClearSlot(self.hWnd, renderingSlotNumber)
                return

            wndMgr.SetSlot(
                self.hWnd, renderingSlotNumber, petSkillIndex, 1, 1, petSkillIcon
            )

    ## Event
    def OnSelectEmptySlot(self, slotNumber):
        if self.eventSelectEmptySlot:
            self.eventSelectEmptySlot(slotNumber)

    def OnSelectItemSlot(self, slotNumber):
        if self.eventSelectItemSlot:
            self.eventSelectItemSlot(slotNumber)

    def OnUnselectEmptySlot(self, slotNumber):
        if self.eventUnselectEmptySlot:
            self.eventUnselectEmptySlot(slotNumber)

    def OnUnselectItemSlot(self, slotNumber):
        if self.eventUnselectItemSlot:
            self.eventUnselectItemSlot(slotNumber)

    def OnUseSlot(self, slotNumber):
        if self.eventUseSlot:
            self.eventUseSlot(slotNumber)

    def OnOverInItem(self, slotNumber):
        if self.eventOverInItem:
            self.eventOverInItem(slotNumber)

    def OnOverOutItem(self):
        if self.eventOverOutItem:
            self.eventOverOutItem()

    def OnPressedSlotButton(self, slotNumber):
        if self.eventPressedSlotButton:
            self.eventPressedSlotButton(slotNumber)

    def GetStartIndex(self):
        return 0

    def SetSlotHighlightedGreeen(self, slotNumber):
        wndMgr.SetSlotHighlightedGreeen(self.hWnd, slotNumber)

    def DisableSlotHighlightedGreen(self, slotNumber):
        wndMgr.DisableSlotHighlightedGreen(self.hWnd, slotNumber)

    def SetSlotCoverImage(self, slotindex, filename):
        wndMgr.SetSlotCoverImage(self.hWnd, slotindex, filename)

    def EnableSlotCoverImage(self, slotindex, onoff):
        wndMgr.EnableSlotCoverImage(self.hWnd, slotindex, onoff)


class GridSlotWindow(SlotWindow):
    def __init__(self):
        SlotWindow.__init__(self)

        self.startIndex = 0

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterGridSlotWindow(self, layer)
        self.SetWindowName(self.__class__.__name__)

    def ArrangeSlot(self, StartIndex, xCount, yCount, xSize, ySize, xBlank, yBlank):
        self.startIndex = StartIndex

        wndMgr.ArrangeSlot(
            self.hWnd, StartIndex, xCount, yCount, xSize, ySize, xBlank, yBlank
        )
        self.startIndex = StartIndex

    def GetStartIndex(self):
        return self.startIndex


class SingleImageTitleBar(ImageBox):
    def __init__(self, layer="UI"):
        ImageBox.__init__(self, layer)
        self.AddFlag("attach")

    def MakeTitleBar(self):
        btnClose = Button()
        btnClose.SetParent(self)
        btnClose.SetUpVisual("d:/ymir work/ui/public/close_button_01.sub")
        btnClose.SetOverVisual("d:/ymir work/ui/public/close_button_02.sub")
        btnClose.SetDownVisual("d:/ymir work/ui/public/close_button_03.sub")
        btnClose.SetToolTipText(localeInfo.UI_CLOSE, 0, -23)
        btnClose.Show()

        self.btnClose = btnClose

    def SetWidth(self, width):
        pass

    def SetCloseEvent(self, event):
        self.btnClose.SetEvent(event)


class TitleBar(Window):
    BLOCK_WIDTH = 32
    BLOCK_HEIGHT = 23

    def __init__(self):
        Window.__init__(self)
        self.AddFlag("attach")

    def MakeTitleBar(self, width, color):

        width = max(64, width - 2)

        imgLeft = ImageBox()
        imgCenter = ExpandedImageBox()
        imgRight = ImageBox()
        imgLeft.AddFlag("not_pick")
        imgCenter.AddFlag("not_pick")
        imgRight.AddFlag("not_pick")
        imgLeft.SetParent(self)
        imgCenter.SetParent(self)
        imgRight.SetParent(self)
        imgLeft.LoadImage("d:/ymir work/ui/pattern/titlebar_left.tga")
        imgCenter.LoadImage("d:/ymir work/ui/pattern/titlebar_center.tga")
        imgRight.LoadImage("d:/ymir work/ui/pattern/titlebar_right.tga")

        imgLeft.Show()
        imgCenter.Show()
        imgRight.Show()

        titleName = TextLine()
        titleName.SetParent(self)
        titleName.SetPosition(0, 10)
        titleName.SetFontColor(255.0 / 255.0, 255.0 / 255.0, 255.0 / 150.0)
        titleName.SetOutline()
        titleName.SetFontName("Roboto:14s")
        titleName.SetHorizontalAlignCenter()
        titleName.Show()

        btnClose = Button()
        btnClose.SetParent(self)
        btnClose.SetUpVisual("d:/ymir work/ui/public/close_button_01.sub")
        btnClose.SetOverVisual("d:/ymir work/ui/public/close_button_02.sub")
        btnClose.SetDownVisual("d:/ymir work/ui/public/close_button_03.sub")
        btnClose.SetToolTipText(localeInfo.UI_CLOSE, 0, -23)
        btnClose.Show()

        self.imgLeft = imgLeft
        self.imgCenter = imgCenter
        self.imgRight = imgRight
        self.btnClose = btnClose
        self.titleName = titleName

        self.btnClose.SetTop()

        self.titleName.SetTop()
        self.titleName.UpdateRect()

        self.SetWidth(width)

    def SetTitleColor(self, color):
        self.titleName.SetPackedFontColor(color)

    def SetTitleName(self, name):
        self.titleName.SetText(name)
        self.titleName.UpdateRect()

    def GetTitleName(self):
        return self.titleName

    def SetWidth(self, width):

        self.imgCenter.SetRenderingRect(
            0.0,
            0.0,
            float((width - self.BLOCK_WIDTH * 2) - self.BLOCK_WIDTH) / self.BLOCK_WIDTH,
            0.0,
        )

        self.imgLeft.SetPosition(0, 7)
        self.imgCenter.SetPosition(self.BLOCK_WIDTH, 7)
        self.imgRight.SetPosition(width - self.BLOCK_WIDTH, 7)

        self.btnClose.SetPosition(width - self.btnClose.GetWidth() - 3, 10)
        self.SetSize(width, self.BLOCK_HEIGHT)

    def DeactivateCloseButton(self):
        self.btnClose.Hide()
        self.SetWidth(self.GetWidth())

    def SetCloseEvent(self, event):
        self.btnClose.SetEvent(event)

    def GetCloseEvent(self):
        return self.btnClose.GetEvent()

    if app.ENABLE_CHANGED_ATTR:

        def CloseButtonHide(self):
            self.btnClose.Hide()

    def OnTop(self):
        self.btnClose.SetTop()

        self.titleName.SetTop()
        self.titleName.UpdateRect()


class SubTitleBar(Button):
    STATUS_IMAGE = [
        "d:/ymir work/ui/quest_re/quest_down.tga",
        "d:/ymir work/ui/quest_re/quest_up.tga",
    ]

    def __init__(self):
        Button.__init__(self)

    def MakeSubTitleBar(self, width, color):
        ## ÇöÀç Color´Â »ç¿ëÇÏ°í ÀÖÁö ¾ÊÀ½

        width = max(64, width)

        self.SetUpVisual("d:/ymir work/ui/quest_re/quest_tab_01.tga")
        self.SetOverVisual("d:/ymir work/ui/quest_re/quest_tab_01.tga")
        self.SetDownVisual("d:/ymir work/ui/quest_re/quest_tab_01.tga")
        self.Show()

        # ³×ÀÌ¹Ö ¤¸¤µ
        isOpenImage = ImageBox()
        isOpenImage.SetParent(self)
        isOpenImage.LoadImage("d:/ymir work/ui/quest_re/quest_down.tga")
        isOpenImage.SetPosition(1, 3)
        isOpenImage.SetHorizontalAlignLeft()
        isOpenImage.AddFlag("not_pick")
        isOpenImage.Show()

        self.isOpenImage = isOpenImage

        self.SetWidth(width)
        self.isOpen = False

    def SetWidth(self, width):
        self.SetPosition(32, 0)

        # if localeInfo.IsARABIC():
        #    self.btnImage.SetPosition(width - self.btnClose.GetWidth() - 3, 3)
        # else:
        #    self.btnImage.SetPosition(0, 3)

        self.SetSize(width, 23)

    def CallEvent(self):
        super(SubTitleBar, self).CallEvent()
        self.OnClickEvent()

    def OnClickEvent(self):
        # self.ToggleSubImage()
        pass

    def CloseImage(self):
        self.isOpen = False
        self.isOpenImage.LoadImage(self.STATUS_IMAGE[self.isOpen])

    def OpenImage(self):
        self.isOpen = True
        self.isOpenImage.LoadImage(self.STATUS_IMAGE[self.isOpen])


class ListBar(Button):
    def __init__(self):
        Button.__init__(self)

    def MakeListBar(self, width, color):

        width = max(64, width)

        self.Show()

        checkbox = ImageBox()
        checkbox.SetParent(self)
        checkbox.LoadImage("d:/ymir work/ui/quest_re/quest_new.tga")
        checkbox.SetPosition(10, 7)
        checkbox.AddFlag("not_pick")
        checkbox.Show()

        self.checkbox = checkbox

        self.SetWidth(width)
        self.isChecked = False

    def SetWidth(self, width):
        self.SetPosition(32, 0)
        self.SetSize(width, 23)

    def CallEvent(self):
        self.OnClickEvent()
        super(ListBar, self).CallEvent()

    def OnClickEvent(self):
        self.checkbox.Hide()
        self.isChecked = True

    def SetSlot(
        self,
        slotIndex,
        itemIndex,
        width,
        height,
        icon,
        diffuseColor=(1.0, 1.0, 1.0, 1.0),
    ):
        wndMgr.SetSlot(
            self.hWnd, slotIndex, itemIndex, width, height, icon, diffuseColor
        )


class VerticalBar(Window):
    BLOCK_WIDTH = 32
    BLOCK_HEIGHT = 17

    def __init__(self):
        Window.__init__(self)
        self.AddFlag("attach")

    def Create(self, height):
        height = max(96, height)

        imgTop = ImageBox()
        imgTop.SetParent(self)
        imgTop.AddFlag("not_pick")
        imgTop.LoadImage("d:/ymir work/ui/test/vertical_bar_top.dds")
        imgTop.Show()

        imgMiddle = ExpandedImageBox()
        imgMiddle.SetParent(self)
        imgMiddle.AddFlag("not_pick")
        imgMiddle.LoadImage("d:/ymir work/ui/test/vertical_bar_middle.dds")
        imgMiddle.Show()

        imgBottom = ImageBox()
        imgBottom.SetParent(self)
        imgBottom.AddFlag("not_pick")
        imgBottom.LoadImage("d:/ymir work/ui/test/vertical_bar_bottom.dds")
        imgBottom.Show()

        self.imgTop = imgTop
        self.imgMiddle = imgMiddle
        self.imgBottom = imgBottom
        self.SetHeight(height)

    def SetHeight(self, height):
        verticalShowingPercentage = (
            float((height - self.BLOCK_HEIGHT * 2) - self.BLOCK_HEIGHT)
            / self.BLOCK_HEIGHT
        )

        self.imgMiddle.SetRenderingRect(0.0, 0.0, 0.0, height - 3)
        self.imgMiddle.SetPosition(0, 2)
        self.imgBottom.SetPosition(0, height)
        self.SetSize(self.BLOCK_WIDTH, height)


class HorizontalBar(Window):
    BLOCK_WIDTH = 32
    BLOCK_HEIGHT = 17

    def __init__(self):
        Window.__init__(self)
        self.AddFlag("attach")

    def Create(self, width):
        width = max(96, width)

        imgLeft = ImageBox()
        imgLeft.SetParent(self)
        imgLeft.AddFlag("not_pick")
        imgLeft.LoadImage("d:/ymir work/ui/pattern/horizontalbar_left.tga")
        imgLeft.Show()

        imgCenter = ExpandedImageBox()
        imgCenter.SetParent(self)
        imgCenter.AddFlag("not_pick")
        imgCenter.LoadImage("d:/ymir work/ui/pattern/horizontalbar_center.tga")
        imgCenter.Show()

        imgRight = ImageBox()
        imgRight.SetParent(self)
        imgRight.AddFlag("not_pick")
        imgRight.LoadImage("d:/ymir work/ui/pattern/horizontalbar_right.tga")
        imgRight.Show()

        self.imgLeft = imgLeft
        self.imgCenter = imgCenter
        self.imgRight = imgRight
        self.SetWidth(width)

    def SetWidth(self, width):
        self.imgCenter.SetRenderingRect(
            0.0,
            0.0,
            float((width - self.BLOCK_WIDTH * 2) - self.BLOCK_WIDTH) / self.BLOCK_WIDTH,
            0.0,
        )
        self.imgCenter.SetPosition(self.BLOCK_WIDTH, 0)
        self.imgRight.SetPosition(width - self.BLOCK_WIDTH, 0)
        self.SetSize(width, self.BLOCK_HEIGHT)


class Gauge(Window):
    SLOT_WIDTH = 16
    SLOT_HEIGHT = 7

    GAUGE_TEMPORARY_PLACE = 12
    GAUGE_WIDTH = 16

    def __init__(self):
        Window.__init__(self)
        self.width = 0
        self.percentage = 100

    def MakeGauge(self, width, color):

        self.width = max(48, width)

        imgSlotLeft = ImageBox()
        imgSlotLeft.SetParent(self)
        imgSlotLeft.LoadImage("d:/ymir work/ui/pattern/gauge_slot_left.tga")
        imgSlotLeft.Show()

        imgSlotRight = ImageBox()
        imgSlotRight.SetParent(self)
        imgSlotRight.LoadImage("d:/ymir work/ui/pattern/gauge_slot_right.tga")
        imgSlotRight.Show()
        imgSlotRight.SetPosition(width - self.SLOT_WIDTH, 0)

        imgSlotCenter = ExpandedImageBox()
        imgSlotCenter.SetParent(self)
        imgSlotCenter.LoadImage("d:/ymir work/ui/pattern/gauge_slot_center.tga")
        imgSlotCenter.Show()
        imgSlotCenter.SetRenderingRect(
            0.0,
            0.0,
            float((width - self.SLOT_WIDTH * 2) - self.SLOT_WIDTH) / self.SLOT_WIDTH,
            0.0,
        )
        imgSlotCenter.SetPosition(self.SLOT_WIDTH, 0)

        imgGauge = ExpandedImageBox()
        imgGauge.SetParent(self)
        imgGauge.LoadImage("d:/ymir work/ui/pattern/gauge_" + color + ".tga")
        imgGauge.Show()
        imgGauge.SetRenderingRect(0.0, 0.0, 0.0, 0.0)
        imgGauge.SetPosition(self.GAUGE_TEMPORARY_PLACE, 0)

        imgSlotLeft.AddFlag("attach")
        imgSlotCenter.AddFlag("attach")
        imgSlotRight.AddFlag("attach")

        self.imgLeft = imgSlotLeft
        self.imgCenter = imgSlotCenter
        self.imgRight = imgSlotRight
        self.imgGauge = imgGauge

        if app.WJ_SHOW_PARTY_ON_MINIMAP:
            imgGauge.AddFlag("not_pick")

        self.SetSize(width, self.SLOT_HEIGHT)

        self.gaugeColor = color

    def SetScale(self, scaleX, scaleY):
        self.imgGauge.SetScale(scaleX, scaleY)

    def SetPercentage(self, curValue, maxValue):

        # PERCENTAGE_MAX_VALUE_ZERO_DIVISION_ERROR
        if maxValue > 0.0:
            percentage = min(1.0, float(curValue) / float(maxValue))
        else:
            percentage = 0.0
        # END_OF_PERCENTAGE_MAX_VALUE_ZERO_DIVISION_ERROR

        self.percentage = int(percentage * 100)
        gaugeSize = (
            -1.0
            + float(self.width - self.GAUGE_TEMPORARY_PLACE * 2)
            * percentage
            / self.GAUGE_WIDTH
        )
        self.imgGauge.SetRenderingRect(0.0, 0.0, gaugeSize, 0.0)

    def GetPercentage(self):
        return self.percentage

    def SetGaugeColor(self, color):
        self.gaugeColor = color
        self.imgGauge.LoadImage("d:/ymir work/ui/pattern/gauge_" + color + ".tga")

    def GetGaugeColor(self):
        return self.gaugeColor

    if app.WJ_SHOW_PARTY_ON_MINIMAP:

        def GaugeImgBoxAddFlag(self, flag):
            # self.imgLeft.AddFlag(flag)
            # self.imgCenter.AddFlag(flag)
            # self.imgRight.AddFlag(flag)
            self.imgGauge.AddFlag(flag)


class NewGauge(Window):
    SLOT_WIDTH = 22
    SLOT_HEIGHT = 7

    GAUGE_TEMPORARY_PLACE = 12
    GAUGE_WIDTH = 22

    def __init__(self):
        Window.__init__(self)
        self.width = 0

    def MakeGauge(self, width, color):

        self.width = max(48, width)

        imgSlotLeft = ImageBox()
        imgSlotLeft.SetParent(self)
        imgSlotLeft.LoadImage("%s/gauge/gauge_slot_left.png" % PATCH_COMMON)
        imgSlotLeft.Show()

        imgSlotRight = ImageBox()
        imgSlotRight.SetParent(self)
        imgSlotRight.LoadImage("%s/gauge/gauge_slot_right.png" % PATCH_COMMON)
        imgSlotRight.Show()
        imgSlotRight.SetPosition(width - self.SLOT_WIDTH, 0)

        imgSlotCenter = ExpandedImageBox()
        imgSlotCenter.SetParent(self)
        imgSlotCenter.LoadImage("%s/gauge/gauge_slot_center.png" % PATCH_COMMON)
        imgSlotCenter.Show()
        imgSlotCenter.SetRenderingRect(
            0.0,
            0.0,
            float((width - self.SLOT_WIDTH * 2) - self.SLOT_WIDTH) / self.SLOT_WIDTH,
            0.0,
        )
        imgSlotCenter.SetPosition(self.SLOT_WIDTH, 0)

        imgGauge = ExpandedImageBox()
        imgGauge.SetParent(self)
        imgGauge.LoadImage("d:/ymir work/ui/gui/common/gauge/gauge_" + color + ".png")
        imgGauge.Show()
        imgGauge.SetRenderingRect(0.0, 0.0, 0.0, 0.0)
        imgGauge.SetPosition(self.GAUGE_TEMPORARY_PLACE, 0)

        imgSlotLeft.AddFlag("attach")
        imgSlotCenter.AddFlag("attach")
        imgSlotRight.AddFlag("attach")

        self.imgLeft = imgSlotLeft
        self.imgCenter = imgSlotCenter
        self.imgRight = imgSlotRight
        self.imgGauge = imgGauge

        self.SetSize(width, self.SLOT_HEIGHT)

    def SetPercentage(self, curValue, maxValue):

        # PERCENTAGE_MAX_VALUE_ZERO_DIVISION_ERROR
        if maxValue > 0.0:
            percentage = min(1.0, float(curValue) / float(maxValue))
        else:
            percentage = 0.0
        # END_OF_PERCENTAGE_MAX_VALUE_ZERO_DIVISION_ERROR

        gaugeSize = (
            -1.0
            + float(self.width - self.GAUGE_TEMPORARY_PLACE * 2)
            * percentage
            / self.GAUGE_WIDTH
        )
        self.imgGauge.SetRenderingRect(0.0, 0.0, gaugeSize, 0.0)


class Board(Window):

    CORNER_WIDTH = 32
    CORNER_HEIGHT = 32
    LINE_WIDTH = 128
    LINE_HEIGHT = 128

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self):
        Window.__init__(self)

        self.MakeBoard(
            "d:/ymir work/ui/pattern/Board_Corner_",
            "d:/ymir work/ui/pattern/Board_Line_",
        )
        self.MakeBase()

    def MakeBoard(self, cornerPath, linePath):

        CornerFileNames = [
            cornerPath + dir + ".tga"
            for dir in (
                "LeftTop",
                "LeftBottom",
                "RightTop",
                "RightBottom",
            )
        ]
        LineFileNames = [
            linePath + dir + ".tga"
            for dir in (
                "Left",
                "Right",
                "Top",
                "Bottom",
            )
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    def MakeBase(self):
        self.Base = ExpandedImageBox()
        self.Base.AddFlag("not_pick")
        self.Base.LoadImage("d:/ymir work/ui/pattern/Board_Base.tga")
        self.Base.SetParent(self)
        self.Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Base.Show()

    def SetSize(self, width, height):

        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )
        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)

        if self.Base:
            self.Base.SetRenderingRect(
                0, 0, horizontalShowingPercentage, verticalShowingPercentage
            )


class Board2(Window):  # anadir separator (common/separator)

    CORNER_WIDTH = 32
    CORNER_HEIGHT = 32
    LINE_WIDTH = 128
    LINE_HEIGHT = 128

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self):
        Window.__init__(self)

        self.MakeBoard(
            "d:/ymir work/ui/gui/board2/Board_Corner_",
            "d:/ymir work/ui/gui/board2/Board_Line_",
        )
        self.MakeBase()

    def MakeBoard(self, cornerPath, linePath):

        CornerFileNames = [
            cornerPath + dir + ".tga"
            for dir in (
                "LeftTop",
                "LeftBottom",
                "RightTop",
                "RightBottom",
            )
        ]
        LineFileNames = [
            linePath + dir + ".tga"
            for dir in (
                "left",
                "right",
                "top",
                "bottom",
            )
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    def MakeBase(self):
        self.Base = ExpandedImageBox()
        self.Base.AddFlag("not_pick")
        self.Base.LoadImage("d:/ymir work/ui/gui/Board2/Board_Base.tga")
        self.Base.SetParent(self)
        self.Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Base.Show()

        self.decoration_leftbottom = ExpandedImageBox()
        self.decoration_leftbottom.AddFlag("not_pick")
        self.decoration_leftbottom.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_leftbottom.tga"
        )
        self.decoration_leftbottom.SetParent(self)
        self.decoration_leftbottom.Show()

        self.decoration_rightbottom = ExpandedImageBox()
        self.decoration_rightbottom.AddFlag("not_pick")
        self.decoration_rightbottom.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_rightbottom.tga"
        )
        self.decoration_rightbottom.SetParent(self)
        self.decoration_rightbottom.Show()

        self.decoration_lefttop = ExpandedImageBox()
        self.decoration_lefttop.AddFlag("not_pick")
        self.decoration_lefttop.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_lefttop.tga"
        )
        self.decoration_lefttop.SetParent(self)
        self.decoration_lefttop.Show()

        self.decoration_righttop = ExpandedImageBox()
        self.decoration_righttop.AddFlag("not_pick")
        self.decoration_righttop.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_righttop.tga"
        )
        self.decoration_righttop.SetParent(self)
        self.decoration_righttop.Show()

    def SetSize(self, width, height):

        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )
        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)

        if self.Base:
            self.Base.SetRenderingRect(
                0, 0, horizontalShowingPercentage, verticalShowingPercentage
            )

        self.decoration_leftbottom.SetPosition(-20, height - 58)
        self.decoration_rightbottom.SetPosition(width - 59, height - 58)
        self.decoration_lefttop.SetPosition(-20, -29)
        self.decoration_righttop.SetPosition(width - 59, -29)


class BoardAlt(Window):
    CORNER_WIDTH = 32
    CORNER_HEIGHT = 32
    LINE_WIDTH = 128
    LINE_HEIGHT = 128

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    BASE_PATH = "d:/ymir work/ui/pattern"
    IMAGES = {
        "CORNER": {
            0: "Board_Corner_LeftTop",
            1: "Board_Corner_LeftBottom",
            2: "Board_Corner_RightTop",
            3: "Board_Corner_RightBottom",
        },
        "BAR": {
            0: "Board_Line_Left",
            1: "Board_Line_Right",
            2: "Board_Line_Top",
            3: "Board_Line_Bottom",
        },
        "FILL": "Board_Base",
    }

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.skipMaxCheck = False

        self.MakeBoard()

    def MakeBoard(self):
        CornerFileNames = []
        LineFileNames = []

        for imageDictKey in ["CORNER", "BAR"]:
            for x in xrange(len(self.IMAGES[imageDictKey])):
                if imageDictKey == "CORNER":
                    CornerFileNames.append(
                        "%s/%s.tga" % (self.BASE_PATH, self.IMAGES[imageDictKey][x])
                    )
                elif imageDictKey == "BAR":
                    LineFileNames.append(
                        "%s/%s.tga" % (self.BASE_PATH, self.IMAGES[imageDictKey][x])
                    )

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

        self.Base = ExpandedImageBox()
        self.Base.AddFlag("not_pick")
        self.Base.LoadImage("%s/%s.tga" % (self.BASE_PATH, self.IMAGES["FILL"]))
        self.Base.SetParent(self)
        self.Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Base.Show()

    def SetSize(self, width, height):
        if not self.skipMaxCheck:
            width = max(self.CORNER_WIDTH * 2, width)
            height = max(self.CORNER_HEIGHT * 2, height)

        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )
        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)

        if self.Base:
            self.Base.SetRenderingRect(
                0, 0, horizontalShowingPercentage, verticalShowingPercentage
            )


class BorderB(BoardAlt):
    CORNER_WIDTH = 16
    CORNER_HEIGHT = 16
    LINE_WIDTH = 16
    LINE_HEIGHT = 16

    BASE_PATH = "d:/ymir work/ui/pattern"

    IMAGES = {
        "CORNER": {
            0: "border_b_left_top",
            1: "border_b_left_bottom",
            2: "border_b_right_top",
            3: "border_b_right_bottom",
        },
        "BAR": {
            0: "border_b_left",
            1: "border_b_right",
            2: "border_b_top",
            3: "border_b_bottom",
        },
        "FILL": "border_b_center",
    }

    def __init__(self):
        BoardAlt.__init__(self)

    def SetSize(self, width, height):
        BoardAlt.SetSize(self, width, height)


class AlphaBoard(Board):
    def __init__(self):
        Board.__init__(self)
        titleBar = TitleBar()
        titleBar.SetParent(self)
        titleBar.MakeTitleBar(0, "red")
        titleBar.SetPosition(8, 7)
        titleBar.Show()

        self.titleBar = titleBar
        self.titleName = proxy(self.titleBar.GetTitleName())

        self.SetCloseEvent(self.Hide)

    def SetSize(self, width, height):
        self.titleBar.SetWidth(width - 15)
        # self.pickRestrictWindow.SetSize(width, height - 30)
        Board.SetSize(self, width, height)
        self.titleName.UpdateRect()

    def SetTitleColor(self, color):
        self.titleName.SetPackedFontColor(color)

    def SetTitleName(self, name):
        self.titleName.SetText(name)

    def SetCloseEvent(self, event):
        self.titleBar.SetCloseEvent(event)


class BorderA(BoardAlt):
    CORNER_WIDTH = 16
    CORNER_HEIGHT = 16
    LINE_WIDTH = 16
    LINE_HEIGHT = 16

    BASE_PATH = "d:/ymir work/ui/pattern"
    IMAGES = {
        "CORNER": {
            0: "border_a_left_top",
            1: "border_a_left_bottom",
            2: "border_a_right_top",
            3: "border_a_right_bottom",
        },
        "BAR": {
            0: "border_a_left",
            1: "border_a_right",
            2: "border_a_top",
            3: "border_a_bottom",
        },
        "FILL": "border_a_center",
    }

    def __init__(self, layer="UI"):
        BoardAlt.__init__(self)

    def SetSize(self, width, height):
        BoardAlt.SetSize(self, width, height)


class BorderC(BoardAlt):
    CORNER_WIDTH = 16
    CORNER_HEIGHT = 16
    LINE_WIDTH = 16
    LINE_HEIGHT = 16

    BASE_PATH = "d:/ymir work/ui/pattern/borderc"
    IMAGES = {
        "CORNER": {
            0: "border_c_left_top",
            1: "border_c_left_bottom",
            2: "border_c_right_top",
            3: "border_c_right_bottom",
        },
        "BAR": {
            0: "border_c_left",
            1: "border_c_right",
            2: "border_c_top",
            3: "border_c_bottom",
        },
        "FILL": "border_c_center",
    }

    def __init__(self, layer="UI"):
        BoardAlt.__init__(self)

    def SetSize(self, width, height):
        BoardAlt.SetSize(self, width, height)


class BoardWithTitleBar(Board):
    def __init__(self):
        Board.__init__(self)
        titleBar = TitleBar()
        titleBar.SetParent(self)
        titleBar.MakeTitleBar(0, "red")
        titleBar.SetPosition(8, 0)
        titleBar.Show()

        self.titleBar = titleBar
        self.titleName = proxy(self.titleBar.GetTitleName())

        self.SetCloseEvent(self.Hide)

    def SetSize(self, width, height):
        self.titleBar.SetWidth(width - 15)
        # self.pickRestrictWindow.SetSize(width, height - 30)
        Board.SetSize(self, width, height)
        self.titleName.UpdateRect()

    def SetTitleColor(self, color):
        self.titleName.SetPackedFontColor(color)

    def SetTitleName(self, name):
        self.titleName.SetText(name)

    def GetTitleName(self):
        return self.titleName.GetText()

    def DeactivateCloseButton(self):
        self.titleBar.DeactivateCloseButton()

    def SetCloseEvent(self, event):
        self.titleBar.SetCloseEvent(event)

    def OnTop(self):
        self.titleBar.SetTop()


class DropDownImage(Window):
    dropped = 0
    dropstat = 0
    last = 0
    lastS = 0
    maxh = 95
    OnChange = None

    class Item(ListBoxEx.Item):
        def __init__(self, text, value=0):
            ListBoxEx.Item.__init__(self, "TOP_MOST")

            self.textBox = TextLine()
            self.textBox.SetParent(self)

            self.textBox.SetText(text)

            # self.textBox.SetLimitWidth(parent.GetWidth()-132)
            self.textBox.Show()
            self.value = value

        def GetValue(self):
            return self.value

    class DropBG(Bar):
        def __init__(self, semiParent):
            Bar.__init__(self, "TOP_MOST")
            self.semiParent = ref(semiParent)

        def OnUpdate(self):
            # TODO: IsShow only reflects the status of the direct parent.
            # so the parent of said parent can be hidden and it'll still return true.
            # it's pretty shit.
            o = self.semiParent()
            if o:
                if not o.IsShow():
                    self.Hide()
                p = o.GetParent()
                if p and not p.IsShow():
                    self.Hide()
                p2 = p.GetParent()
                if p2 and not p2.IsShow():
                    self.Hide()

    def __init__(self, parent, tt="", down=1):
        Window.__init__(self, "UI")
        self.down = down
        self.SetParent(parent)
        self.bg = Bar()
        self.bg.SetParent(self)
        self.bg.SetPosition(0, 0)
        self.bg.SetColor(0xC0000000)
        self.bg.SetMouseOverIn(self.bgMouseIn)
        self.bg.SetMouseOverOut(self.bgMouseOut)
        self.bg.SetMouseLeftButtonDown(self.ExpandMe)
        self.bg.Show()
        self.act = TextLine()
        self.act.SetParent(self.bg)
        self.act.SetPosition(4, 2)
        self.act.SetText(tt)
        self.act.Show()
        self.GetText = self.act.GetText

        self.Drop = self.DropBG(self)
        # self.Drop.SetParent(parent)
        self.Drop.SetPosition(0, 21)
        # self.Drop.SetSize(150,95)
        self.Drop.SetSize(150, 0)
        # self.Drop.SetColor(0xc00a0a0a)
        self.Drop.SetColor(0xFF0A0A0A)

        self.ScrollBar = ThinScrollBar()
        self.ScrollBar.SetParent(self.Drop)
        self.ScrollBar.SetPosition(132, 0)
        # self.ScrollBar.SetScrollBarSize(95)
        self.ScrollBar.SetScrollBarSize(0)
        # self.ScrollBar.Show()

        self.DropList = ListBoxEx()
        self.DropList.SetParent(self.Drop)
        self.DropList.itemHeight = 12
        self.DropList.itemStep = 13
        self.DropList.SetSize(132, 13)
        self.DropList.SetScrollBar(self.ScrollBar)
        self.DropList.SetSelectEvent(self.SetTitle)
        self.DropList.SetViewItemCount(0)
        self.DropList.SetOnRunMouseWheelEvent(self.OnRunMouseWheel)
        self.DropList.Show()

        if tt != "":
            self.AppendItemAndSelect(tt)
        self.selected = self.DropList.GetSelectedItem()

        self.SetSize(120, 20)

    def OnRunMouseWheel(self, nLen):
        if nLen > 0:
            self.ScrollBar.OnUp()
        else:
            self.ScrollBar.OnDown()
        return True

    def Hide(self):
        Window.Hide(self)
        try:  # hide is called before init finishes, droplist/drop wont be set
            self.Drop.Hide()
        except AttributeError:
            pass

    c = 1

    def AppendItem(self, text, value=0):
        self.c += 1
        self.DropList.AppendItem(self.Item(text, value))
        self.maxh = min(95, 13 * self.c)
        if self.c > 7:
            self.ScrollBar.Show()

    def AppendItemAndSelect(self, text, value=0):
        self.DropList.AppendItem(self.Item(text, value))
        self.DropList.SelectIndex(len(self.DropList.itemList) - 1)

    def SelectByAffectId(self, id):
        for x in self.DropList.itemList:
            if x.value == id:
                self.DropList.SelectItem(x)
                break

    def SetTitle(self, item):
        self.act.SetText(str(item.textBox.GetText()))
        self.last = self.DropList.basePos
        self.lastS = self.ScrollBar.GetPos()
        self.dropped = 0
        self.selected = item
        if self.OnChange:
            self.OnChange()

    def SetOnChangeEvent(self, event):
        self.OnChange = MakeEvent(event)

    # self.Drop.Hide()
    def FixSelectPosition(self):
        x, y = self.GetGlobalPosition()
        if self.down == 1:
            self.Drop.SetPosition(x, y + 21)
        else:
            self.Drop.SetPosition(x, y - self.Drop.GetHeight())

    def SetPosition(self, w, h):
        Window.SetPosition(self, w, h)
        self.FixSelectPosition()

    def SetSize(self, w, h):
        Window.SetSize(self, w, h)
        self.bg.SetSize(w, h)
        self.Drop.SetSize(w, 0)
        self.DropList.SetSize(w - 18, self.maxh)
        for x in self.DropList.itemList:
            x.SetSize(w - 18, 12)
        self.ScrollBar.SetPosition(w - 18, 0)

    def ExpandMe(self):
        if self.dropped == 1:
            # self.Drop.Hide()
            self.dropped = 0
        else:
            # self.Drop.Show()
            self.dropped = 1

    def OnUpdate(self):
        iter = 6
        if self.Drop.GetHeight() < 50:
            self.ScrollBar.Hide()
        else:
            self.ScrollBar.Show()

        if self.IsShow():
            self.FixSelectPosition()
        else:
            self.Drop.Hide()
        if self.dropped == 0 and self.dropstat == 1:
            if self.Drop.GetHeight() <= 0:
                self.dropstat = 0
                self.Drop.SetSize(self.Drop.GetWidth(), 0)
                self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight())
                self.Drop.Hide()
            else:
                if self.Drop.GetHeight() - iter < 0:
                    self.Drop.SetSize(self.Drop.GetWidth(), 0)
                else:
                    self.Drop.SetSize(
                        self.Drop.GetWidth(), self.Drop.GetHeight() - iter
                    )
                    (w, h) = self.GetLocalPosition()
                    self.SetPosition(w, h)

                self.DropList.SetViewItemCount(int(self.Drop.GetHeight() / 13))
                self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight())
            self.DropList.SetViewItemCount(int(self.Drop.GetHeight() / 13))
            self.DropList.SetBasePos(self.last + 1)
            self.DropList.SetBasePos(self.last)
        elif self.dropped == 1 and self.dropstat == 0:
            self.Drop.Show()
            if self.Drop.GetHeight() >= self.maxh:
                self.Drop.SetSize(self.Drop.GetWidth(), self.maxh)
                self.ScrollBar.SetScrollBarSize(self.maxh)
                self.dropstat = 1
                self.DropList.SetViewItemCount(7)
                self.ScrollBar.SetPos(self.lastS)
            else:
                self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight() + iter)
                self.Drop.SetSize(self.Drop.GetWidth(), self.Drop.GetHeight() + iter)
                (w, h) = self.GetLocalPosition()
                self.SetPosition(w, h)
            self.DropList.SetViewItemCount(int(self.Drop.GetHeight() / 13))
            self.DropList.SetBasePos(self.last + 1)
            self.DropList.SetBasePos(self.last)

    ## BG Hover
    def bgMouseIn(self):
        self.bg.SetColor(0xC00A0A0A)
        return True

    def bgMouseOut(self):
        self.bg.SetColor(0xC0000000)
        return True


class ThinBoardInner(Window):
    CORNER_WIDTH = 30
    CORNER_HEIGHT = 30
    LINE_WIDTH = 30
    LINE_HEIGHT = 31
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.3)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/whisper_board/sub_thinboard/Corner_" + dir + ".png"
            for dir in ["Left_Top", "Left_Bottom", "Right_Top", "Right_Bottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/whisper_board/sub_thinboard/line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()
        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

        self.AddFlag("not_pick")

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2
        )

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class ThinBoard(Window):
    CORNER_WIDTH = 35
    CORNER_HEIGHT = 20
    LINE_WIDTH = 20
    LINE_HEIGHT = 20
    BOARD_COLOR = grp.GenerateColor(0.03137254, 0.09019607, 0.15686274, 0.85)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        super(ThinBoard, self).__init__(layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/whisper_board/main_thinboard/Corner_" + dir + ".png"
            for dir in ["Left_Top", "Left_Bottom", "Right_Top", "Right_Bottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/whisper_board/main_thinboard/line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = ExpandedImageBox()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(9, 10)
        Base.LoadImage("d:/ymir work/ui/gui/whisper_board/main_thinboard/bg.png")
        Base.Show()
        self.Base = Base

        self.innerThinBoard = ThinBoardInner()
        self.innerThinBoard.AddFlag("attach")
        self.innerThinBoard.AddFlag("not_pick")
        self.innerThinBoard.SetParent(self)
        self.innerThinBoard.SetPosition(10, 12)
        self.innerThinBoard.Show()

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)
        self.decoration_leftbottom = ExpandedImageBox()
        self.decoration_leftbottom.AddFlag("not_pick")
        self.decoration_leftbottom.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_leftbottom.tga"
        )
        self.decoration_leftbottom.SetParent(self)
        self.decoration_leftbottom.Show()

        self.decoration_rightbottom = ExpandedImageBox()
        self.decoration_rightbottom.AddFlag("not_pick")
        self.decoration_rightbottom.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_rightbottom.tga"
        )
        self.decoration_rightbottom.SetParent(self)
        self.decoration_rightbottom.Show()

        self.decoration_lefttop = ExpandedImageBox("TOP_MOST")
        self.decoration_lefttop.AddFlag("not_pick")
        self.decoration_lefttop.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_lefttop.tga"
        )
        self.decoration_lefttop.SetParent(self)
        self.decoration_lefttop.Hide()

        self.decoration_righttop = ExpandedImageBox("TOP_MOST")
        self.decoration_righttop.AddFlag("not_pick")
        self.decoration_righttop.LoadImage(
            "d:/ymir work/ui/gui/Board/decoration_corner_righttop.tga"
        )
        self.decoration_righttop.SetParent(self)
        self.decoration_righttop.Hide()

        self.decoration_fire = ExpandedImageBox()
        self.decoration_fire.AddFlag("not_pick")
        self.decoration_fire.LoadImage("d:/ymir work/ui/fire.png")
        self.decoration_fire.SetParent(self)
        self.decoration_fire.Show()

        self.decoration_leftbottom.SetTop()
        self.decoration_rightbottom.SetTop()
        self.decoration_fire.SetTop()

        self.AddFlag("not_pick")

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):

        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )

        ##Shadows thinboard
        # self.Corners[self.SLT].SetPosition(-11,-11)
        # self.Corners[self.SLB].SetPosition(-11, height-51)
        # self.Corners[self.SRT].SetPosition(width - 51, -11)
        # self.Corners[self.SRB].SetPosition(width - 51, height - 51)

        self.Lines[self.R].SetPosition(width - self.LINE_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )
        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        if self.Base:
            self.Base.SetScale(
                float(width - self.LINE_WIDTH) / 319.0,
                float(height - self.CORNER_HEIGHT) / 350.0,
            )

        self.innerThinBoard.SetSize(width - 20, height - 22)
        self.decoration_fire.SetVerticalAlignBottom()
        self.decoration_fire.SetPosition(20, -15)
        self.decoration_fire.SetRenderingRect(
            0.0, 0.0, float((width - 239.0) - 45.0) / 239.0, 0.0
        )

        self.decoration_rightbottom.SetPosition(width - 65, height - 65)
        self.decoration_leftbottom.SetPosition(-15, height - 65)

        self.decoration_lefttop.SetPosition(-29, -28)
        self.decoration_righttop.SetHorizontalAlignRight()
        self.decoration_righttop.SetPosition(-28, -28)

        Window.SetSize(self, width, height)

    def SetInnerThinBoardSize(self, width, height):
        if width is None:
            width = self.innerThinBoard.GetWidth()

        if height is None:
            height = self.innerThinBoard.GetHeight()

        self.innerThinBoard.SetSize(width, height)

    def SetInnerThinBoardPosition(self, x, y):
        if x is None:
            x = self.innerThinBoard.GetLocalPosition()[0]

        if y is None:
            y = self.innerThinBoard.GetLocalPosition()[1]

        self.innerThinBoard.SetPosition(x, y)

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideDecoFire(self):
        self.decoration_fire.Hide()
        self.decoration_fire_2.Hide()

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


rgb_scale = 255
cmyk_scale = 100


def rgb_to_cmyk(r, g, b):
    if (r == 0) and (g == 0) and (b == 0):
        # black
        return 0, 0, 0, cmyk_scale

    # rgb [0,255] -> cmy [0,1]
    c = 1 - r / float(rgb_scale)
    m = 1 - g / float(rgb_scale)
    y = 1 - b / float(rgb_scale)

    # extract out k [0,1]
    min_cmy = min(c, m, y)
    c = c - min_cmy
    m = m - min_cmy
    y = y - min_cmy
    k = min_cmy

    # rescale to the range [0,cmyk_scale]
    return c * cmyk_scale, m * cmyk_scale, y * cmyk_scale, k * cmyk_scale


def cmyk_to_rgb(c, m, y, k):
    """ """
    r = rgb_scale * (1.0 - (c + k) / float(cmyk_scale))
    g = rgb_scale * (1.0 - (m + k) / float(cmyk_scale))
    b = rgb_scale * (1.0 - (y + k) / float(cmyk_scale))
    return r, g, b


def ink_add_for_rgb(list_of_colours):
    """input: list of rgb, opacity (r,g,b,o) colours to be added, o acts as weights.
    output (r,g,b)
    """
    C = 0
    M = 0
    Y = 0
    K = 0

    for r, g, b, o in list_of_colours:
        c, m, y, k = rgb_to_cmyk(r, g, b)
        C += o * c
        M += o * m
        Y += o * y
        K += o * k

    return cmyk_to_rgb(C, M, Y, K)


class SmallDarkThinBoard(Window, BaseEvents):
    CORNER_WIDTH = 24
    CORNER_HEIGHT = 13
    LINE_WIDTH = 15
    LINE_HEIGHT = 6
    BOARD_COLOR = 0xE01F0C0C

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        BaseEvents.__init__(self)
        self.AddFlag("component")

        CornerFileNames = [
            "d:/ymir work/ui/gui/dark_thin_board/Corner_" + dir + ".png"
            for dir in ["Left_Top", "Left_Bottom", "Right_Top", "Right_Bottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/dark_thin_board/line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        self.Base = ExpandedImageBox()
        self.Base.AddFlag("attach")
        self.Base.AddFlag("not_pick")
        self.Base.LoadImage("d:/ymir work/ui/gui/dark_thin_board/bg.png")
        self.Base.SetParent(self)
        self.Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Base.Show()

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)
        self.AddFlag("not_pick")

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )

        ##Shadows thinboard
        # self.Corners[self.SLT].SetPosition(-11,-11)
        # self.Corners[self.SLB].SetPosition(-11, height-51)
        # self.Corners[self.SRT].SetPosition(width - 51, -11)
        # self.Corners[self.SRB].SetPosition(width - 51, height - 51)

        self.Lines[self.R].SetPosition(width - 24, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )
        verticalShowingPercentageBoard = (
            float((height - self.CORNER_HEIGHT * 2) - 2) / 2
        )
        horizontalShowingPercentageBoard = (
            float((width - self.CORNER_WIDTH * 2) - 2) / 2
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        if self.Base:
            self.Base.SetRenderingRect(
                0, 0, horizontalShowingPercentageBoard, verticalShowingPercentageBoard
            )

        Window.SetSize(self, width, height)

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()

    def SetColor(self, r, g, b):
        for line in self.Lines:
            line.SetDiffuseColor(r, g, b, 1.0)

        for corner in self.Corners:
            corner.SetDiffuseColor(r, g, b, 1.0)

        self.Base.SetDiffuseColor(r, g, b, 1.0)


# class ThinBoard(Window):
#    CORNER_WIDTH = 16
#    CORNER_HEIGHT = 16
#    LINE_WIDTH = 16
#    LINE_HEIGHT = 16
#    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.51)
#
#    LT = 0
#    LB = 1
#    RT = 2
#    RB = 3
#    L = 0
#    R = 1
#    T = 2
#    B = 3
#
#    def __init__(self, layer="UI"):
#        Window.__init__(self, layer)
#
#        CornerFileNames = ["d:/ymir work/ui/pattern/ThinBoard_Corner_" + dir + ".tga" for dir in
#                           ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]]
#        LineFileNames = ["d:/ymir work/ui/pattern/ThinBoard_Line_" + dir + ".tga" for dir in
#                         ["Left", "Right", "Top", "Bottom"]]
#
#        self.Corners = []
#        for fileName in CornerFileNames:
#            Corner = ExpandedImageBox()
#            Corner.AddFlag("attach")
#            Corner.AddFlag("not_pick")
#            Corner.LoadImage(fileName)
#            Corner.SetParent(self)
#            Corner.SetPosition(0, 0)
#            Corner.Show()
#
#            self.Corners.append(Corner)
#
#        self.Lines = []
#        for fileName in LineFileNames:
#            Line = ExpandedImageBox()
#            Line.AddFlag("attach")
#            Line.AddFlag("not_pick")
#            Line.LoadImage(fileName)
#            Line.SetParent(self)
#            Line.SetPosition(0, 0)
#            Line.Show()
#
#            self.Lines.append(Line)
#
#        Base = Bar()
#        Base.SetParent(self)
#        Base.AddFlag("attach")
#        Base.AddFlag("not_pick")
#        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
#        Base.SetColor(self.BOARD_COLOR)
#        Base.Show()
#
#        self.Base = Base
#
#        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
#        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)
#
#    def SetSize(self, width, height):
#        width = max(self.CORNER_WIDTH * 2, width)
#        height = max(self.CORNER_HEIGHT * 2, height)
#        Window.SetSize(self, width, height)
#
#        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
#        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
#        self.Corners[self.RB].SetPosition(width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT)
#        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
#        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)
#
#        verticalShowingPercentage = float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT) / self.LINE_HEIGHT
#        horizontalShowingPercentage = float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
#
#        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
#        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
#        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
#        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
#        self.Base.SetSize(width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2)
#
#    def ShowCorner(self, corner):
#        self.Corners[corner].Show()
#        self.SetSize(self.GetWidth(), self.GetHeight())
#
#    def HideCorners(self, corner):
#        self.Corners[corner].Hide()
#        self.SetSize(self.GetWidth(), self.GetHeight())
#
#    def ShowLine(self, line):
#        self.Lines[line].Show()
#        self.SetSize(self.GetWidth(), self.GetHeight())
#
#    def HideLine(self, line):
#        self.Lines[line].Hide()
#        self.SetSize(self.GetWidth(), self.GetHeight())
#
#    def ShowInternal(self):
#        self.Base.Show()
#
#        for wnd in self.Lines:
#            wnd.Show()
#        for wnd in self.Corners:
#            wnd.Show()
#
#    def HideInternal(self):
#        self.Base.Hide()
#
#        for wnd in self.Lines:
#            wnd.Hide()
#        for wnd in self.Corners:
#            wnd.Hide()


class ThinBoard2(Window):
    CORNER_WIDTH = 16
    CORNER_HEIGHT = 16
    LINE_WIDTH = 16
    LINE_HEIGHT = 16
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.51)

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Corner_" + dir + "_new.tga"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Line_" + dir + "_new.tga"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()

            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()

            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()

        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    def SetPattern(self, fileName):
        CornerFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Corner_" + dir + "_" + fileName + ".tga"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Line_" + dir + "_" + fileName + ".tga"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        for i in xrange(len(self.Corners)):
            self.Corners[i].LoadImage(CornerFileNames[i])

        for i in xrange(len(self.Lines)):
            self.Lines[i].LoadImage(LineFileNames[i])

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2
        )

    def ShowInternal(self):
        self.Base.Show()

        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()

        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class ThinBoardOld(Window, BaseEvents):
    CORNER_WIDTH = 16
    CORNER_HEIGHT = 16
    LINE_WIDTH = 16
    LINE_HEIGHT = 16
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.51)

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        BaseEvents.__init__(self)

        CornerFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Corner_" + dir + ".tga"
            for dir in [
                "LeftTop_Old",
                "LeftBottom_Old",
                "RightTop_Old",
                "RightBottom_Old",
            ]
        ]
        LineFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Line_" + dir + ".tga"
            for dir in ["Left_Old", "Right_Old", "Top_Old", "Bottom_Old"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()

            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()

            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()

        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    def SetPattern(self, fileName):
        CornerFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Corner_" + dir + "_" + fileName + ".tga"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/pattern/ThinBoard_Line_" + dir + "_" + fileName + ".tga"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        for i in xrange(len(self.Corners)):
            self.Corners[i].LoadImage(CornerFileNames[i])

        for i in xrange(len(self.Lines)):
            self.Lines[i].LoadImage(LineFileNames[i])

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_HEIGHT, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2
        )

    def ShowInternal(self):
        self.Base.Show()

        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()

        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()

    def SetColor(self, r, g, b):

        for line in self.Lines:
            line.SetDiffuseColor(r, g, b, 1.0)

        for corner in self.Corners:
            corner.SetDiffuseColor(r, g, b, 1.0)

        self.Base.SetDiffuseColor(r, g, b, 1.0)


if app.NEW_SELECT_CHARACTER:

    class ThinBoardGold(Window):
        CORNER_WIDTH = 16
        CORNER_HEIGHT = 16
        LINE_WIDTH = 16
        LINE_HEIGHT = 16

        LT = 0
        LB = 1
        RT = 2
        RB = 3
        L = 0
        R = 1
        T = 2
        B = 3

        def __init__(self, layer="UI"):
            Window.__init__(self, layer)

            CornerFileNames = [
                "d:/ymir work/ui/pattern/thinboardgold/ThinBoard_Corner_"
                + dir
                + "_Gold.tga"
                for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
            ]
            LineFileNames = [
                "d:/ymir work/ui/pattern/thinboardgold/ThinBoard_Line_"
                + dir
                + "_Gold.tga"
                for dir in ["Left", "Right", "Top", "Bottom"]
            ]

            self.MakeBase()

            self.Corners = []
            for fileName in CornerFileNames:
                Corner = ExpandedImageBox()
                Corner.AddFlag("attach")
                Corner.AddFlag("not_pick")
                Corner.LoadImage(fileName)
                Corner.SetParent(self)
                Corner.SetPosition(0, 0)
                Corner.Show()
                self.Corners.append(Corner)

            self.Lines = []
            for fileName in LineFileNames:
                Line = ExpandedImageBox()
                Line.AddFlag("attach")
                Line.AddFlag("not_pick")
                Line.LoadImage(fileName)
                Line.SetParent(self)
                Line.SetPosition(0, 0)
                Line.Show()
                self.Lines.append(Line)

            self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
            self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

        def SetSize(self, width, height):

            width = max(self.CORNER_WIDTH * 2, width)
            height = max(self.CORNER_HEIGHT * 2, height)
            Window.SetSize(self, width, height)

            self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
            self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
            self.Corners[self.RB].SetPosition(
                width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
            )
            self.Lines[self.R].SetPosition(
                width - self.CORNER_WIDTH, self.CORNER_HEIGHT
            )
            self.Lines[self.B].SetPosition(
                self.CORNER_HEIGHT, height - self.CORNER_HEIGHT
            )

            verticalShowingPercentage = (
                float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
                / self.LINE_HEIGHT
            )
            horizontalShowingPercentage = (
                float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH)
                / self.LINE_WIDTH
            )

            self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
            self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
            self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
            self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)

            # self.Base.GetWidth()
            # self.Base.GetHeight()
            """
                Defalt Width : 128, Height : 128
                0.0 > 128, 1.0 > 256
            """
            if self.Base:
                self.Base.SetRenderingRect(
                    0,
                    0,
                    (float(width) - 32) / float(self.Base.GetWidth()) - 1.0,
                    (float(height) - 32) / float(self.Base.GetHeight()) - 1.0,
                )

        def MakeBase(self):
            self.Base = ExpandedImageBox()
            self.Base.AddFlag("not_pick")
            self.Base.LoadImage("d:/ymir work/ui/pattern/Board_Base.tga")
            self.Base.SetParent(self)
            self.Base.SetPosition(16, 16)
            self.Base.SetAlpha(0.8)
            self.Base.Show()

        def ShowInternal(self):
            self.Base.Show()
            for wnd in self.Lines:
                wnd.Show()
            for wnd in self.Corners:
                wnd.Show()

        def HideInternal(self):
            self.Base.Hide()
            for wnd in self.Lines:
                wnd.Hide()
            for wnd in self.Corners:
                wnd.Hide()

    class ThinBoardCircle(Window):
        CORNER_WIDTH = 4
        CORNER_HEIGHT = 4
        LINE_WIDTH = 4
        LINE_HEIGHT = 4
        BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.50)

        LT = 0
        LB = 1
        RT = 2
        RB = 3
        L = 0
        R = 1
        T = 2
        B = 3

        def __init__(self, layer="UI"):
            Window.__init__(self, layer)

            CornerFileNames = [
                "d:/ymir work/ui/pattern/thinboardcircle/ThinBoard_Corner_"
                + dir
                + "_Circle.tga"
                for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
            ]
            LineFileNames = [
                "d:/ymir work/ui/pattern/thinboardcircle/ThinBoard_Line_"
                + dir
                + "_Circle.tga"
                for dir in ["Left", "Right", "Top", "Bottom"]
            ]

            self.Corners = []
            for fileName in CornerFileNames:
                Corner = ExpandedImageBox()
                Corner.AddFlag("attach")
                Corner.AddFlag("not_pick")
                Corner.LoadImage(fileName)
                Corner.SetParent(self)
                Corner.SetPosition(0, 0)
                Corner.Show()
                self.Corners.append(Corner)

            self.Lines = []
            for fileName in LineFileNames:
                Line = ExpandedImageBox()
                Line.AddFlag("attach")
                Line.AddFlag("not_pick")
                Line.LoadImage(fileName)
                Line.SetParent(self)
                Line.SetPosition(0, 0)
                Line.Show()
                self.Lines.append(Line)

            Base = Bar()
            Base.SetParent(self)
            Base.AddFlag("attach")
            Base.AddFlag("not_pick")
            Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
            Base.SetColor(self.BOARD_COLOR)
            Base.Show()
            self.Base = Base

            self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
            self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

        def SetAlpha(self, alpha):
            for line in self.Lines:
                line.SetAlpha(alpha)
            self.Base.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, alpha))

        def SetSize(self, width, height):

            width = max(self.CORNER_WIDTH * 1, width)
            height = max(self.CORNER_HEIGHT * 1, height)
            Window.SetSize(self, width, height)

            self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
            self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
            self.Corners[self.RB].SetPosition(
                width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
            )
            self.Lines[self.R].SetPosition(
                width - self.CORNER_WIDTH, self.CORNER_HEIGHT
            )
            self.Lines[self.B].SetPosition(
                self.CORNER_HEIGHT, height - self.CORNER_HEIGHT
            )

            verticalShowingPercentage = (
                float((height - self.CORNER_HEIGHT * 1) - self.LINE_HEIGHT)
                / self.LINE_HEIGHT
            )
            horizontalShowingPercentage = (
                float((width - self.CORNER_WIDTH * 1) - self.LINE_WIDTH)
                / self.LINE_WIDTH
            )
            self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
            self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
            self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
            self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
            self.Base.SetSize(
                width - self.CORNER_WIDTH * 1, height - self.CORNER_HEIGHT * 1
            )

        def ShowInternal(self):
            self.Base.Show()
            for wnd in self.Lines:
                wnd.Show()
            for wnd in self.Corners:
                wnd.Show()

        def HideInternal(self):
            self.Base.Hide()
            for wnd in self.Lines:
                wnd.Hide()
            for wnd in self.Corners:
                wnd.Hide()


class ThinBoardRed(Window):
    CORNER_WIDTH = 37
    CORNER_HEIGHT = 37
    LINE_WIDTH = 37
    LINE_HEIGHT = 37
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.0)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Corner_" + dir + ".png"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()
        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 1, width)
        height = max(self.CORNER_HEIGHT * 1, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 1) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 1) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 1, height - self.CORNER_HEIGHT * 1
        )

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class ThinBoardDarkRed(Window):
    CORNER_WIDTH = 34
    CORNER_HEIGHT = 40
    LINE_WIDTH = 28
    LINE_HEIGHT = 20
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.25)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/whisper_board/dark_thinboard/Corner_" + dir + ".png"
            for dir in ["Left_Top", "Left_Bottom", "Right_Top", "Right_Bottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/whisper_board/dark_thinboard/Line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()
        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 1, width)
        height = max(self.CORNER_HEIGHT * 1, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 1) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 1) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 1, height - self.CORNER_HEIGHT * 1
        )

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class ThinBoardRedBg(Window):
    CORNER_WIDTH = 37
    CORNER_HEIGHT = 34
    LINE_WIDTH = 37
    LINE_HEIGHT = 37
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.3)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Corner_" + dir + ".png"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()
        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2
        )

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class SmallThinBoardDark(Window):
    CORNER_WIDTH = 37
    CORNER_HEIGHT = 34
    LINE_WIDTH = 37
    LINE_HEIGHT = 37
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.3)

    # SLT = 0
    # SLB = 1
    # SRT = 2
    # SRB = 3
    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Corner_" + dir + ".png"
            for dir in ["LeftTop", "LeftBottom", "RightTop", "RightBottom"]
        ]
        LineFileNames = [
            "d:/ymir work/ui/gui/thinboard_red/Line_" + dir + ".png"
            for dir in ["Left", "Right", "Top", "Bottom"]
        ]

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

        Base = Bar()
        Base.SetParent(self)
        Base.AddFlag("attach")
        Base.AddFlag("not_pick")
        Base.SetPosition(self.CORNER_WIDTH, self.CORNER_HEIGHT)
        Base.SetColor(self.BOARD_COLOR)
        Base.Show()
        self.Base = Base

        self.Lines[self.L].SetPosition(0, self.CORNER_HEIGHT)
        self.Lines[self.T].SetPosition(self.CORNER_WIDTH, 0)

    # def __del__(self):
    #    Window.__del__(self)

    def SetSize(self, width, height):
        width = max(self.CORNER_WIDTH * 2, width)
        height = max(self.CORNER_HEIGHT * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LB].SetPosition(0, height - self.CORNER_HEIGHT)
        self.Corners[self.RT].SetPosition(width - self.CORNER_WIDTH, 0)
        self.Corners[self.RB].SetPosition(
            width - self.CORNER_WIDTH, height - self.CORNER_HEIGHT
        )
        self.Lines[self.R].SetPosition(width - self.CORNER_WIDTH, self.CORNER_HEIGHT)
        self.Lines[self.B].SetPosition(self.CORNER_WIDTH, height - self.CORNER_HEIGHT)

        verticalShowingPercentage = (
            float((height - self.CORNER_HEIGHT * 2) - self.LINE_HEIGHT)
            / self.LINE_HEIGHT
        )
        horizontalShowingPercentage = (
            float((width - self.CORNER_WIDTH * 2) - self.LINE_WIDTH) / self.LINE_WIDTH
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Base.SetSize(
            width - self.CORNER_WIDTH * 2, height - self.CORNER_HEIGHT * 2
        )

    def ShowCorner(self, corner):
        self.Corners[corner].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideCorners(self, corner):
        self.Corners[corner].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowLine(self, line):
        self.Lines[line].Show()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def HideLine(self, line):
        self.Lines[line].Hide()
        self.SetSize(self.GetWidth(), self.GetHeight())

    def ShowInternal(self):
        self.Base.Show()
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        self.Base.Hide()
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class Input(Window):
    HEIGHT = 1
    MIN_WIDTH = 1

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.__dictImages = {
            "LEFT": ImageBox(),
            "CENTER": ExpandedImageBox(),
            "RIGHT": ImageBox(),
        }

        for image in self.__dictImages.itervalues():
            image.SetParent(self)
            image.AddFlag("not_pick")
            image.Show()

        self.__dictImages["LEFT"].LoadImage(
            "%s/input/board_left_01_normal.tga" % PATCH_COMMON
        )
        self.__dictImages["CENTER"].LoadImage(
            "%s/input/board_center_01_normal.tga" % PATCH_COMMON
        )
        self.__dictImages["RIGHT"].LoadImage(
            "%s/input/board_right_01_normal.tga" % PATCH_COMMON
        )

    def SetHeight(self, height):
        self.HEIGHT = height

    def SetWidth(self, width):
        width = max(self.MIN_WIDTH, width)
        self.SetSize(width, self.HEIGHT)

        self.__dictImages["LEFT"].SetPosition(
            0, self.HEIGHT - self.__dictImages["LEFT"].GetHeight()
        )
        self.__dictImages["CENTER"].SetPosition(
            self.__dictImages["LEFT"].GetWidth(),
            self.HEIGHT - self.__dictImages["CENTER"].GetHeight(),
        )
        self.__dictImages["RIGHT"].SetPosition(
            self.GetWidth() - self.__dictImages["RIGHT"].GetWidth() + 12,
            self.HEIGHT - self.__dictImages["RIGHT"].GetHeight(),
        )
        self.__dictImages["CENTER"].SetScale(self.GetWidth(), 1.0)


class ShopDecoTitle(Window):
    DEFAULT_VALUE = 16
    CORNER_WIDTH = 48
    CORNER_HEIGHT = 32
    LINE_WIDTH = 16
    LINE_HEIGHT = 32

    LT = 0
    LB = 1
    RT = 2
    RB = 3
    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self, type, layer="UI"):
        Window.__init__(self, layer)

        CornerFileNames, LineFileNames = self.__GetFilePath(type)

        if CornerFileNames == None or LineFileNames == None:
            return

        self.Corners = []
        for fileName in CornerFileNames:
            Corner = ExpandedImageBox()
            Corner.AddFlag("attach")
            Corner.AddFlag("not_pick")
            Corner.LoadImage(fileName)
            Corner.SetParent(self)
            Corner.SetPosition(0, 0)
            Corner.Show()
            self.Corners.append(Corner)

        self.Lines = []
        for fileName in LineFileNames:
            Line = ExpandedImageBox()
            Line.AddFlag("attach")
            Line.AddFlag("not_pick")
            Line.LoadImage(fileName)
            Line.SetParent(self)
            Line.SetPosition(0, 0)
            Line.Show()
            self.Lines.append(Line)

    def __GetFilePath(self, type):
        import uiMyShopDecoration

        CornerFileNames = []
        for direction in ["left_top", "left_bottom", "right_top", "right_bottom"]:
            CornerFileNames.append(
                uiMyShopDecoration.DECO_SHOP_TITLE_LIST[int(type)][1]
                + "_"
                + direction
                + ".tga"
            )

        LineFileNames = []
        for direction in ["left", "right", "top", "bottom"]:
            LineFileNames.append(
                uiMyShopDecoration.DECO_SHOP_TITLE_LIST[int(type)][1]
                + "_"
                + direction
                + ".tga"
            )

        return CornerFileNames, LineFileNames

    def SetSize(self, width, height):
        width = max(self.DEFAULT_VALUE * 2, width)
        height = max(self.DEFAULT_VALUE * 2, height)
        Window.SetSize(self, width, height)

        self.Corners[self.LT].SetPosition(
            -self.CORNER_WIDTH + self.DEFAULT_VALUE,
            -self.CORNER_HEIGHT + self.DEFAULT_VALUE,
        )
        self.Corners[self.LB].SetPosition(
            -self.CORNER_WIDTH + self.DEFAULT_VALUE,
            height - self.CORNER_HEIGHT + self.DEFAULT_VALUE,
        )

        self.Corners[self.RT].SetPosition(
            width - self.DEFAULT_VALUE, -self.CORNER_HEIGHT + self.DEFAULT_VALUE
        )
        self.Corners[self.RB].SetPosition(
            width - self.DEFAULT_VALUE, height - self.CORNER_HEIGHT + self.DEFAULT_VALUE
        )

        self.Lines[self.L].SetPosition(0, self.DEFAULT_VALUE)
        self.Lines[self.R].SetPosition(width - self.DEFAULT_VALUE, self.DEFAULT_VALUE)
        self.Lines[self.B].SetPosition(
            self.DEFAULT_VALUE, height - self.LINE_HEIGHT + self.DEFAULT_VALUE
        )
        self.Lines[self.T].SetPosition(
            self.DEFAULT_VALUE, -self.LINE_HEIGHT + self.DEFAULT_VALUE
        )

        verticalShowingPercentage = (
            float((height - self.DEFAULT_VALUE * 2) - self.DEFAULT_VALUE)
            / self.DEFAULT_VALUE
        )
        horizontalShowingPercentage = (
            float((width - self.DEFAULT_VALUE * 2) - self.DEFAULT_VALUE)
            / self.DEFAULT_VALUE
        )

        self.Lines[self.L].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, verticalShowingPercentage)
        self.Lines[self.T].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, horizontalShowingPercentage, 0)

    def ShowInternal(self):
        for wnd in self.Lines:
            wnd.Show()
        for wnd in self.Corners:
            wnd.Show()

    def HideInternal(self):
        for wnd in self.Lines:
            wnd.Hide()
        for wnd in self.Corners:
            wnd.Hide()


class ScrollBarBackgroundBar(Window):
    def __init__(self):
        Window.__init__(self)
        self.bgColor = 0

    def SetBackgroundColor(self, color):
        self.bgColor = long(color)

    def OnRender(self):
        x, y, w, h = self.GetRect()

        if self.bgColor:
            grp.SetColor(self.bgColor)
        else:
            grp.SetColor(BACKGROUND_COLOR)

        grp.RenderBar(x, y, w, h)

        grp.SetColor(grp.GenerateColor(0.6, 0.6, 0.6, 0.5))
        grp.RenderLine(x, y, w, 0)
        grp.RenderLine(x, y, 0, h)

        grp.SetColor(grp.GenerateColor(0.6, 0.6, 0.6, 0.5))
        grp.RenderLine(x, y + h, w, 0)
        grp.RenderLine(x + w, y, 0, h)


class ScrollBar(Window):
    SCROLLBAR_WIDTH = 17
    SCROLLBAR_MIDDLE_HEIGHT = 9
    SCROLLBAR_BUTTON_WIDTH = 17
    SCROLLBAR_BUTTON_HEIGHT = 17
    MIDDLE_BAR_POS = 5
    MIDDLE_BAR_UPPER_PLACE = 3
    MIDDLE_BAR_DOWNER_PLACE = 3
    TEMP_SPACE = MIDDLE_BAR_UPPER_PLACE + MIDDLE_BAR_DOWNER_PLACE

    upButton = None
    downButton = None

    class MiddleBar(DragButton):
        top = None
        bottom = None
        middle = None

        def __init__(self):
            DragButton.__init__(self)
            self.AddFlag("moveable")

        def MakeImage(self):
            top = ImageBox()
            top.SetParent(self)
            top.LoadImage("d:/ymir work/ui/pattern/ScrollBar_Top.tga")
            top.SetPosition(0, 0)
            top.AddFlag("not_pick")
            top.Show()

            bottom = ImageBox()
            bottom.SetParent(self)
            bottom.LoadImage("d:/ymir work/ui/pattern/ScrollBar_Bottom.tga")
            bottom.AddFlag("not_pick")
            bottom.Show()

            middle = ExpandedImageBox()
            middle.SetParent(self)
            middle.LoadImage("d:/ymir work/ui/pattern/ScrollBar_Middle.tga")
            middle.SetPosition(0, 4)
            middle.AddFlag("not_pick")
            middle.Show()

            self.top = top
            self.bottom = bottom
            self.middle = middle

        def SetSize(self, height):
            height = max(12, height)
            DragButton.SetSize(self, 10, height)
            if self.bottom:
                self.bottom.SetPosition(0, height - 4)

            height -= 4 * 3
            if self.middle:
                self.middle.SetRenderingRect(0, 0, 0, float(height) / 4.0)

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.pageSize = 1
        self.curPos = 0.0
        self.eventScroll = None
        self.middleButtonDragEndEvent = None
        self.lockFlag = False
        self.scrollStep = 0.20

        self.CreateScrollBar()

    def CreateScrollBar(self):
        barSlot = ScrollBarBackgroundBar()
        barSlot.SetParent(self)
        barSlot.AddFlag("not_pick")
        barSlot.Show()

        middleBar = self.MiddleBar()
        middleBar.SetParent(self)
        middleBar.SetMoveEvent(self.OnMove)
        middleBar.SetOnMouseLeftButtonUpEvent(self.MiddleButtonDragEnd)
        middleBar.Show()
        middleBar.MakeImage()
        middleBar.SetSize(12)

        upButton = Button()
        upButton.SetParent(self)
        upButton.SetEvent(self.OnUp)
        upButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_up_button_01.sub")
        upButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_up_button_02.sub")
        upButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_up_button_03.sub")
        upButton.Show()

        downButton = Button()
        downButton.SetParent(self)
        downButton.SetEvent(self.OnDown)
        downButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_down_button_01.sub")
        downButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_down_button_02.sub")
        downButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_down_button_03.sub")
        downButton.Show()

        self.upButton = upButton
        self.downButton = downButton
        self.middleBar = middleBar
        self.barSlot = barSlot

        self.SCROLLBAR_WIDTH = self.upButton.GetWidth()
        self.SCROLLBAR_MIDDLE_HEIGHT = self.middleBar.GetHeight()
        self.SCROLLBAR_BUTTON_WIDTH = self.upButton.GetWidth()
        self.SCROLLBAR_BUTTON_HEIGHT = self.upButton.GetHeight()

    def Destroy(self):
        self.middleBar = None
        self.upButton = None
        self.downButton = None
        self.eventScroll = None

    def SetScrollEvent(self, event):
        self.eventScroll = MakeEvent(event)

    def SetMiddleBarSize(self, pageScale):
        realHeight = self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2
        self.SCROLLBAR_MIDDLE_HEIGHT = int(pageScale * float(realHeight))
        self.middleBar.SetSize(self.SCROLLBAR_MIDDLE_HEIGHT)
        self.pageSize = (
            (self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2)
            - self.SCROLLBAR_MIDDLE_HEIGHT
            - (self.TEMP_SPACE)
        )

    def SetScrollBarSize(self, height):
        self.pageSize = (
            (height - self.SCROLLBAR_BUTTON_HEIGHT * 2)
            - self.SCROLLBAR_MIDDLE_HEIGHT
            - (self.TEMP_SPACE)
        )
        self.SetSize(self.SCROLLBAR_WIDTH, height)
        if self.upButton:
            self.upButton.SetPosition(0, 0)
        if self.downButton:
            self.downButton.SetPosition(0, height - self.SCROLLBAR_BUTTON_HEIGHT)

        self.middleBar.SetRestrictMovementArea(
            self.MIDDLE_BAR_POS,
            self.SCROLLBAR_BUTTON_HEIGHT + self.MIDDLE_BAR_UPPER_PLACE,
            self.MIDDLE_BAR_POS + 2,
            height - self.SCROLLBAR_BUTTON_HEIGHT * 2 - self.TEMP_SPACE,
        )
        self.middleBar.SetPosition(self.MIDDLE_BAR_POS, 0)

        self.UpdateBarSlot()

    def UpdateBarSlot(self):
        self.barSlot.SetPosition(0, self.SCROLLBAR_BUTTON_HEIGHT)
        self.barSlot.SetSize(
            self.GetWidth(), self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2 - 2
        )

    def SetMiddleButtonDragEndEvent(self, event):
        self.middleButtonDragEndEvent = MakeEvent(event)

    def MiddleButtonDragEnd(self):
        if self.middleButtonDragEndEvent:
            self.middleButtonDragEndEvent()

    def GetPos(self):
        return self.curPos

    def SetPos(self, pos):
        pos = max(0.0, pos)
        pos = min(1.0, pos)

        newPos = float(self.pageSize) * pos
        self.middleBar.SetPosition(
            self.MIDDLE_BAR_POS,
            int(newPos) + self.SCROLLBAR_BUTTON_HEIGHT + self.MIDDLE_BAR_UPPER_PLACE,
        )
        self.OnMove()

    def SetScrollStep(self, step):
        self.scrollStep = step

    def GetScrollStep(self):
        return self.scrollStep

    def OnUp(self):
        self.SetPos(self.curPos - self.scrollStep)

    def OnDown(self):
        self.SetPos(self.curPos + self.scrollStep)

    def OnMove(self):
        if self.lockFlag:
            return

        if 0 == self.pageSize:
            return

        (xLocal, yLocal) = self.middleBar.GetLocalPosition()
        self.curPos = float(
            yLocal - self.SCROLLBAR_BUTTON_HEIGHT - self.MIDDLE_BAR_UPPER_PLACE
        ) / float(self.pageSize)

        if self.eventScroll:
            self.eventScroll()

    def OnMouseLeftButtonDown(self):
        (xMouseLocalPosition, yMouseLocalPosition) = self.GetMouseLocalPosition()
        pickedPos = (
            yMouseLocalPosition
            - self.SCROLLBAR_BUTTON_HEIGHT
            - self.SCROLLBAR_MIDDLE_HEIGHT / 2
        )
        newPos = float(pickedPos) / float(self.pageSize)
        self.SetPos(newPos)
        return True

    def LockScroll(self):
        self.lockFlag = True

    def UnlockScroll(self):
        self.lockFlag = False


# »ç¿ë ¾ÈÇÔ
class NewScrollBar(ScrollBar):
    def CreateScrollBar(self):
        upButton = Button()
        upButton.SetParent(self)
        upButton.SetEvent(self.OnUp)
        upButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_up_button_01.sub")
        upButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_up_button_02.sub")
        upButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_up_button_03.sub")
        upButton.Show()

        downButton = Button()
        downButton.SetParent(self)
        downButton.SetEvent(self.OnDown)
        downButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_down_button_01.sub")
        downButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_down_button_02.sub")
        downButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_down_button_03.sub")
        downButton.Show()

        middle = ExpandedImageBox()
        middle.SetParent(self)
        middle.LoadImage("d:/ymir work/ui/game/quest/quest_scrollbar.tga")
        middle.SetPosition(0, 4)
        middle.AddFlag("not_pick")
        middle.Show()

    def SetScrollBarSize(self, height):
        self.pageSize = (
            (height - self.SCROLLBAR_BUTTON_HEIGHT * 2)
            - self.SCROLLBAR_MIDDLE_HEIGHT
            - self.TEMP_SPACE
        )
        self.SetSize(self.SCROLLBAR_WIDTH, height)
        self.middleBar.SetRestrictMovementArea(
            self.MIDDLE_BAR_POS,
            self.SCROLLBAR_BUTTON_HEIGHT + self.MIDDLE_BAR_UPPER_PLACE,
            self.MIDDLE_BAR_POS,
            height - self.SCROLLBAR_BUTTON_HEIGHT * 2 - self.TEMP_SPACE,
        )
        self.SetPosition(self.MIDDLE_BAR_POS, 0)

        self.UpdateBarSlot()

    def UpdateBarSlot(self):
        pass


class ThinScrollBar(ScrollBar):
    SCROLLBAR_WIDTH = 11
    SCROLLBAR_MIDDLE_HEIGHT = 31
    SCROLLBAR_BUTTON_WIDTH = 0
    SCROLLBAR_BUTTON_HEIGHT = 0
    MIDDLE_BAR_POS = 6
    MIDDLE_BAR_UPPER_PLACE = 4
    MIDDLE_BAR_DOWNER_PLACE = 4
    TEMP_SPACE = MIDDLE_BAR_UPPER_PLACE + MIDDLE_BAR_DOWNER_PLACE

    upButton = None
    downButton = None

    class MiddleBar(DragButton):
        top = None
        bottom = None
        middle = None

        def __init__(self):
            DragButton.__init__(self)
            self.AddFlag("moveable")

        def MakeImage(self):
            top = ImageBox()
            top.SetParent(self)
            top.LoadImage("d:/ymir work/ui/gui/thinmiddlebar/top.png")
            top.SetPosition(0, 0)
            top.AddFlag("not_pick")
            top.Show()

            bottom = ImageBox()
            bottom.SetParent(self)
            bottom.LoadImage("d:/ymir work/ui/gui/thinmiddlebar/bottom.png")
            bottom.AddFlag("not_pick")
            bottom.Show()

            middle = ExpandedImageBox()
            middle.SetParent(self)
            middle.LoadImage("d:/ymir work/ui/gui/thinmiddlebar/middle.png")
            middle.SetPosition(0, 14)
            middle.AddFlag("not_pick")
            middle.Show()

            self.top = top
            self.bottom = bottom
            self.middle = middle

        def SetSize(self, height):
            height = max(31, height)
            DragButton.SetSize(self, 8, height)
            if self.bottom:
                self.bottom.SetPosition(0, height - 14)

            height -= 4 * 3
            if self.middle:
                self.middle.SetRenderingRect(0, 0, 0, float(height) - 14)

    def CreateScrollBar(self):
        barSlot = ScrollBarBackgroundBar()
        barSlot.SetParent(self)
        barSlot.AddFlag("not_pick")
        barSlot.SetBackgroundColor(0xFF190505)
        barSlot.Show()

        middleBar = self.MiddleBar()
        middleBar.SetParent(self)
        middleBar.SetMoveEvent(self.OnMove)
        middleBar.SetOnMouseLeftButtonUpEvent(self.MiddleButtonDragEnd)
        middleBar.Show()
        middleBar.MakeImage()
        middleBar.SetSize(31)

        upButton = Button()
        upButton.SetParent(self)
        upButton.SetEvent(self.OnUp)
        upButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_up_button_01.sub")
        upButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_up_button_02.sub")
        upButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_up_button_03.sub")
        upButton.Show()

        downButton = Button()
        downButton.SetParent(self)
        downButton.SetEvent(self.OnDown)
        downButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_down_button_01.sub")
        downButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_down_button_02.sub")
        downButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_down_button_03.sub")
        downButton.Show()

        self.upButton = upButton
        self.downButton = downButton
        self.middleBar = middleBar
        self.barSlot = barSlot

        self.upButton.Hide()
        self.downButton.Hide()

        self.SCROLLBAR_WIDTH = self.upButton.GetWidth()
        self.SCROLLBAR_MIDDLE_HEIGHT = self.middleBar.GetHeight()
        self.SCROLLBAR_BUTTON_WIDTH = self.upButton.GetWidth()
        self.SCROLLBAR_BUTTON_HEIGHT = self.upButton.GetHeight()

    def SetMiddleBarSize(self, pageScale):
        realHeight = self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2
        self.SCROLLBAR_MIDDLE_HEIGHT = int(pageScale * float(realHeight))
        self.middleBar.SetSize(self.SCROLLBAR_MIDDLE_HEIGHT)
        self.pageSize = (
            (self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2)
            - self.SCROLLBAR_MIDDLE_HEIGHT
            - (self.TEMP_SPACE)
        )

    def SetScrollBarSize(self, height):
        self.pageSize = (
            (height - self.SCROLLBAR_BUTTON_HEIGHT * 2)
            - self.SCROLLBAR_MIDDLE_HEIGHT
            - (self.TEMP_SPACE)
        )
        self.SetSize(self.SCROLLBAR_WIDTH, height)
        if self.upButton:
            self.upButton.SetPosition(0, 0)
        if self.downButton:
            self.downButton.SetPosition(0, height - self.SCROLLBAR_BUTTON_HEIGHT)

        self.middleBar.SetRestrictMovementArea(
            self.MIDDLE_BAR_POS,
            self.SCROLLBAR_BUTTON_HEIGHT + self.MIDDLE_BAR_UPPER_PLACE,
            self.MIDDLE_BAR_POS,
            height - self.SCROLLBAR_BUTTON_HEIGHT * 2 - self.TEMP_SPACE,
        )
        self.middleBar.SetPosition(self.MIDDLE_BAR_POS, 0)

        self.UpdateBarSlot()

    def UpdateBarSlot(self):
        self.barSlot.SetPosition(0, self.SCROLLBAR_BUTTON_HEIGHT)
        self.barSlot.SetSize(
            self.SCROLLBAR_WIDTH,
            self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2 - 2,
        )


class SmallThinScrollBar(ScrollBar):
    class MiddleBar(DragBar):
        top = None
        bottom = None
        middle = None

        def __init__(self):
            DragBar.__init__(self)
            self.AddFlag("moveable")
            self.AddFlag("restrict_x")

        def SetSize(self, height):
            height = max(15, height)
            DragBar.SetSize(self, 6, height)

        def SetWidth(self, width):
            DragBar.SetSize(self, width, self.GetHeight())

        def OnMouseLeftButtonDown(self):
            self.CaptureMouse()
            return True

        def OnMouseLeftButtonUp(self):
            self.ReleaseMouse()
            return True

    def __init__(self, layer="UI"):
        super(SmallThinScrollBar, self).__init__(layer)
        self.scrollSpeed = 1
        self.sizeScale = 1.0
        self.documentSize = 100.0

    def CreateScrollBar(self):
        barSlot = ScrollBarBackgroundBar()
        barSlot.SetParent(self)
        barSlot.AddFlag("not_pick")
        barSlot.Show()

        middleBar = self.MiddleBar()
        middleBar.SetParent(self)
        middleBar.SetColor(grp.GenerateColor(1, 1, 1, 0.6))
        middleBar.SetMoveEvent(self.OnMove)
        middleBar.Show()

        self.middleBar = middleBar
        self.barSlot = barSlot

        self.SCROLLBAR_WIDTH = 3
        self.SCROLLBAR_WIDTH_EXPANDED = 9
        self.SCROLLBAR_MIDDLE_HEIGHT = self.middleBar.GetHeight()
        self.SCROLLBAR_BUTTON_WIDTH = 3
        self.SCROLLBAR_BUTTON_HEIGHT = 0
        self.MIDDLE_BAR_POS = 0
        self.MIDDLE_BAR_UPPER_PLACE = 0
        self.MIDDLE_BAR_DOWNER_PLACE = 0
        self.TEMP_SPACE = 0
        self.barSlot.SetTop()

    def SetPos(self, pos, add=False):
        pos = max(0.0, pos)
        pos = min(1.0, pos)

        newPos = float(self.pageSize) * pos
        if add:
            newPos = ceil(newPos)
        self.middleBar.SetPosition(self.MIDDLE_BAR_POS, newPos)
        self.OnMove()

    def SetScrollStep(self, step):
        self.scrollStep = step

    def GetScrollStep(self):
        return self.scrollStep * 2

    def OnUp(self):
        self.SetPos(self.curPos - self.scrollStep)

    def OnDown(self):
        self.SetPos(self.curPos + self.scrollStep, True)

    def OnMove(self):
        if self.lockFlag:
            return

        if 0 == self.pageSize:
            return

        (xLocal, yLocal) = self.middleBar.GetLocalPosition()
        self.curPos = float(yLocal) / float(self.pageSize)

        if self.eventScroll:
            self.eventScroll()

    def UpdateBarSlot(self):
        self.barSlot.SetPosition(0, self.SCROLLBAR_BUTTON_HEIGHT)
        self.barSlot.SetSize(
            self.GetWidth(), self.GetHeight() - self.SCROLLBAR_BUTTON_HEIGHT * 2 - 2
        )
        self.middleBar.SetTop()

    def SetScale(self, fScale):
        self.sizeScale = fScale
        self.middleBar.SetSize(floor((self.GetHeight() - 2) * fScale))

    def SetScrollSpeed(self, speed):
        self.scrollSpeed = speed

    def SetPosScale(self, fScale):
        self.middleBar.SetPosition(
            1, ceil((self.GetHeight() - 2 - self.middleBar.GetHeight()) * fScale) + 1
        )

    def SetMiddleBarSize(self, pageScale):
        self.SCROLLBAR_MIDDLE_HEIGHT = max(15, int(pageScale * float(self.GetHeight())))
        self.middleBar.SetSize(self.SCROLLBAR_MIDDLE_HEIGHT)
        self.pageSize = (self.GetHeight()) - self.SCROLLBAR_MIDDLE_HEIGHT

    def SetScrollBarSize(self, height):
        self.SetSize(self.SCROLLBAR_WIDTH, height)
        self.middleBar.SetRestrictMovementArea(0, 0, 0, height)
        self.middleBar.SetPosition(0, 0)
        self.UpdateBarSlot()

    def OnMouseOverIn(self):
        self.SetSize(self.SCROLLBAR_WIDTH_EXPANDED, self.GetHeight())
        self.middleBar.SetWidth(8)
        self.UpdateBarSlot()
        self.SetPosition(self.GetLocalPosition()[0] - 5, self.GetLocalPosition()[1])

        return True

    def OnMouseOverOut(self):
        self.SetSize(self.SCROLLBAR_WIDTH, self.GetHeight())
        self.UpdateBarSlot()
        self.middleBar.SetWidth(6)
        self.SetPosition(self.GetLocalPosition()[0] + 5, self.GetLocalPosition()[1])

        return True


class SliderBar(Window):
    def __init__(self):
        Window.__init__(self)

        self.curPos = 1.0
        self.pageSize = 1.0
        self.eventChange = None

        self.__CreateBackGroundImage()
        self.__CreateCursor()

    def __CreateBackGroundImage(self):
        img = ImageBox()
        img.SetParent(self)
        img.LoadImage("d:/ymir work/ui/game/windows/sliderbar.sub")
        img.Show()
        self.backGroundImage = img

        ##
        self.SetSize(self.backGroundImage.GetWidth(), self.backGroundImage.GetHeight())

    def __CreateCursor(self):
        cursor = DragButton()
        cursor.AddFlag("moveable")
        cursor.AddFlag("restrict_y")
        cursor.SetParent(self)
        cursor.SetMoveEvent(self.__OnMove)
        cursor.SetUpVisual("d:/ymir work/ui/game/windows/sliderbar_cursor.sub")
        cursor.SetOverVisual("d:/ymir work/ui/game/windows/sliderbar_cursor.sub")
        cursor.SetDownVisual("d:/ymir work/ui/game/windows/sliderbar_cursor.sub")
        cursor.Show()
        self.cursor = cursor

        ##
        self.cursor.SetRestrictMovementArea(0, 0, self.backGroundImage.GetWidth(), 0)
        self.pageSize = self.backGroundImage.GetWidth() - self.cursor.GetWidth()

    def __OnMove(self):
        (xLocal, yLocal) = self.cursor.GetLocalPosition()
        self.curPos = float(xLocal) / float(self.pageSize)

        if self.eventChange:
            self.eventChange()

    def SetSliderPos(self, pos):
        self.curPos = pos
        self.cursor.SetPosition(int(self.pageSize * pos), 0)

    def GetSliderPos(self):
        return self.curPos

    def SetEvent(self, event):
        self.eventChange = MakeEvent(event)

    def Enable(self):
        self.cursor.Show()

    def Disable(self):
        self.cursor.Hide()


class ListBox(Window):
    TEMPORARY_PLACE = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.layer = layer
        self.overLine = -1
        self.selectedLine = -1
        self.width = 0
        self.height = 0
        self.stepSize = 17
        self.basePos = 0
        self.showLineCount = 0
        self.drawBorder = False
        self.itemCenterAlign = True
        self.itemFontColor = None
        self.itemFontName = None
        self.itemList = []
        self.keyDict = {}
        self.textDict = {}
        self.event = None
        self.doubleClickEvent = None

    def GetStepSize(self):
        return self.stepSize

    def SetDrawBorder(self, flag):
        self.drawBorder = flag

    def SetWidth(self, width):
        self.SetSize(width, self.height)

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)
        self.width = width
        self.height = height

    def SetTextCenterAlign(self, flag):
        self.itemCenterAlign = flag

    def SetItemFontName(self, fontName):
        self.itemFontName = fontName

    def SetItemFontColor(self, fontColor):
        self.itemFontColor = fontColor

    def SetBasePos(self, pos):
        self.basePos = pos
        self._LocateItem()

    def GetBasePos(self):
        return self.basePos

    def ClearItem(self):
        self.keyDict = {}
        self.textDict = {}
        self.itemList = []
        self.overLine = -1
        self.selectedLine = -1

    def HasItem(self, number):
        for i in self.keyDict:
            if self.keyDict[i] == number:
                return True

        return False

    def EraseItem(self, number):
        for i in self.keyDict:
            if self.keyDict[i] == number:
                for j in xrange(number, len(self.itemList) - 1):
                    self.keyDict[j] = self.keyDict[j + 1]
                    self.textDict[j] = self.textDict[j + 1]

                del self.keyDict[len(self.itemList) - 1]
                del self.textDict[len(self.itemList) - 1]
                del self.itemList[i]

                maxBasePos = max(self.GetItemCount() - self.GetViewItemCount(), 0)
                if self.basePos > maxBasePos:
                    self.SetBasePos(maxBasePos)
                else:
                    self.LocateItem()

                return True

        return False

    def InsertItem(self, number, text):
        self.keyDict[len(self.itemList)] = number
        self.textDict[len(self.itemList)] = text

        textLine = TextLine()
        textLine.SetParent(self)

        if self.itemFontName:
            textLine.SetFontName(self.itemFontName)

        if self.itemFontColor:
            textLine.SetPackedFontColor(self.itemFontColor)

        textLine.SetText(text)
        textLine.Show()

        if self.itemCenterAlign:
            textLine.SetHorizontalAlignCenter()
            textLine.SetHorizontalAlignCenter()

        self.itemList.append(textLine)

        self._LocateItem()

    def ChangeItem(self, number, text):
        for key, value in self.keyDict.items():
            if value == number:
                self.textDict[key] = text

                if number < len(self.itemList):
                    self.itemList[key].SetText(text)

                return

    def LocateItem(self):
        self._LocateItem()

    def _LocateItem(self):

        skipCount = self.basePos
        yPos = 0
        self.showLineCount = 0

        for textLine in self.itemList:
            textLine.Hide()

            if skipCount > 0:
                skipCount -= 1
                continue

            textLine.SetPosition(0, yPos + 3)

            yPos += self.stepSize

            if yPos <= self.GetHeight():
                self.showLineCount += 1
                textLine.Show()

    def ArrangeItem(self):
        self.SetSize(self.width, len(self.itemList) * self.stepSize)
        self._LocateItem()

    def GetViewItemCount(self):
        return int(self.GetHeight() / self.stepSize)

    def GetItemCount(self):
        return len(self.itemList)

    def SetEvent(self, event):
        self.event = MakeEvent(event)

    def SetDoubleClickEvent(self, event):
        self.doubleClickEvent = MakeEvent(event)

    def SelectItem(self, line):

        if line not in self.keyDict:
            return

        if line == self.selectedLine:
            return

        self.selectedLine = line
        if self.event:
            self.event(self.keyDict.get(line, 0), self.textDict.get(line, "None"))

    def GetSelectedItem(self):
        return self.keyDict.get(self.selectedLine, 0)

    def GetSelectedItemText(self):
        return self.textDict.get(self.selectedLine, "")

    def GetSelectedItem(self, defaultVal=0):
        return self.keyDict.get(self.selectedLine, defaultVal)

    def GetSelectedLine(self):
        return self.selectedLine

    def GetItemText(self, index):
        return self.textDict.get(index, "")

    def OnMouseLeftButtonDown(self):
        if self.overLine < 0:
            return False
        return True

    def OnMouseLeftButtonUp(self):
        if self.overLine >= 0:
            self.SelectItem(self.overLine + self.basePos)
            return True
        return False

    def OnMouseLeftButtonDoubleClick(self):
        if self.overLine < 0:
            return False

        if self.doubleClickEvent:
            try:
                self.doubleClickEvent(self.keyDict.get(self.overLine, 0))
            except TypeError:
                self.doubleClickEvent(
                    self.keyDict.get(self.overLine, 0),
                    self.textDict.get(self.overLine, "None"),
                )

    def IsOverLine(self):
        return self.overLine != -1

    def GetOverLine(self, defaultVal=0):
        return self.keyDict.get(self.overLine + self.basePos, defaultVal)

    def OnUpdate(self):

        self.overLine = -1

        if self.IsIn():
            x, y = self.GetGlobalPosition()
            height = self.GetHeight()
            xMouse, yMouse = wndMgr.GetMousePosition()

            if yMouse - y < height - 1:
                self.overLine = (yMouse - y) / self.stepSize

                if self.overLine < 0:
                    self.overLine = -1
                if self.overLine >= len(self.itemList):
                    self.overLine = -1

    def OnRender(self):
        xRender, yRender = self.GetGlobalPosition()
        yRender -= self.TEMPORARY_PLACE
        widthRender = self.width
        heightRender = self.height + self.TEMPORARY_PLACE * 2

        if self.drawBorder == True:
            grp.SetColor(DARK_COLOR)
            grp.RenderBar(xRender - 1, yRender + 1, 1, self.GetHeight() + 2)
            grp.RenderBar(xRender - 1, yRender + 1, self.GetWidth() + 2, 1)
            grp.RenderBar(
                xRender + self.GetWidth(), yRender + 1, 1, self.GetHeight() + 2
            )
            grp.RenderBar(
                xRender - 1, yRender + self.GetHeight() + 2, self.GetWidth() + 2, 1
            )

        if True:
            if -1 != self.overLine and self.keyDict[self.overLine] != -1:
                grp.SetColor(HALF_WHITE_COLOR)
                grp.RenderBar(
                    xRender + 2,
                    yRender + self.overLine * self.stepSize + 4,
                    self.width - 3,
                    self.stepSize,
                )

            if -1 != self.selectedLine and self.keyDict[self.selectedLine] != -1:
                if self.selectedLine >= self.basePos:
                    if self.selectedLine - self.basePos < self.showLineCount:
                        grp.SetColor(SELECT_COLOR)
                        grp.RenderBar(
                            xRender + 2,
                            yRender
                            + int(self.selectedLine - self.basePos) * self.stepSize
                            + 4,
                            self.width - 3,
                            self.stepSize,
                        )


class ApplyListBox(Window):
    TEMPORARY_PLACE = 3

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.overLine = -1
        self.selectedLine = -1
        self.width = 0
        self.height = 0
        self.stepSize = 17
        self.basePos = 0
        self.showLineCount = 0
        self.itemCenterAlign = False
        self.itemList = []
        self.keyDict = {}
        self.textDict = {}
        self.valueDict = {}

        self.event = None

    def SetWidth(self, width):
        self.SetSize(width, self.height)

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)
        self.width = width
        self.height = height

    def SetTextCenterAlign(self, flag):
        self.itemCenterAlign = flag

    def SetBasePos(self, pos):
        self.basePos = pos
        self._LocateItem()

    def ClearItem(self):
        self.keyDict = {}
        self.textDict = {}
        self.valueDict = {}
        self.itemList = []
        self.overLine = -1
        self.selectedLine = -1

    def InsertItem(self, number, text, value):
        self.keyDict[len(self.itemList)] = number
        self.textDict[len(self.itemList)] = text
        self.valueDict[len(self.itemList)] = value

        textLine = TextLine()
        textLine.SetParent(self)
        textLine.SetText(text)
        textLine.Show()

        if self.itemCenterAlign:
            textLine.SetHorizontalAlignCenter()

        self.itemList.append(textLine)

        self._LocateItem()

    def ChangeItem(self, number, text, setValue):
        for key, value in self.keyDict.items():
            if value == number:
                self.textDict[key] = text
                self.valueDict[key] = setValue

                if number < len(self.itemList):
                    self.itemList[key].SetText(text)

                return

    def LocateItem(self):
        self._LocateItem()

    def _LocateItem(self):

        skipCount = self.basePos
        yPos = 0
        self.showLineCount = 0

        for textLine in self.itemList:
            textLine.Hide()

            if skipCount > 0:
                skipCount -= 1
                continue

            textLine.SetPosition(0, yPos + 3)

            yPos += self.stepSize

            if yPos <= self.GetHeight():
                self.showLineCount += 1
                textLine.Show()

    def ArrangeItem(self):
        self.SetSize(self.width, len(self.itemList) * self.stepSize)
        self._LocateItem()

    def GetViewItemCount(self):
        return int(self.GetHeight() / self.stepSize)

    def GetItemCount(self):
        return len(self.itemList)

    def SetEvent(self, event):
        self.event = event

    def SelectItem(self, line):

        if line not in self.keyDict:
            return

        if line == self.selectedLine:
            return

        self.selectedLine = line
        self.event(self.keyDict.get(line, 0), self.textDict.get(line, "None"))

    def GetSelectedItem(self):
        return self.keyDict.get(self.selectedLine, 0)

    def GetSelectedItemText(self):
        return self.textDict.get(self.selectedLine, "")

    def GetSelectedItemValue(self):
        return self.valueDict.get(self.selectedLine, 0)

    def GetItemText(self, index):
        return self.textDict.get(index, "")

    def OnMouseLeftButtonDown(self):
        if self.overLine < 0:
            return True

    def OnMouseLeftButtonUp(self):
        if self.overLine >= 0:
            self.SelectItem(self.overLine + self.basePos)

    def OnUpdate(self):

        self.overLine = -1

        if self.IsIn():
            x, y = self.GetGlobalPosition()
            height = self.GetHeight()
            xMouse, yMouse = wndMgr.GetMousePosition()

            if yMouse - y < height - 1:
                self.overLine = (yMouse - y) / self.stepSize

                if self.overLine < 0:
                    self.overLine = -1
                if self.overLine >= len(self.itemList):
                    self.overLine = -1

    def OnRender(self):
        xRender, yRender = self.GetGlobalPosition()
        yRender -= self.TEMPORARY_PLACE
        widthRender = self.width
        heightRender = self.height + self.TEMPORARY_PLACE * 2

        if True:
            if -1 != self.overLine and self.keyDict[self.overLine] != -1:
                grp.SetColor(HALF_WHITE_COLOR)
                grp.RenderBar(
                    xRender + 2,
                    yRender + self.overLine * self.stepSize + 4,
                    self.width - 3,
                    self.stepSize,
                )

            if -1 != self.selectedLine and self.keyDict[self.selectedLine] != -1:
                if self.selectedLine >= self.basePos:
                    if self.selectedLine - self.basePos < self.showLineCount:
                        grp.SetColor(SELECT_COLOR)
                        grp.RenderBar(
                            xRender + 2,
                            yRender
                            + int(self.selectedLine - self.basePos) * self.stepSize
                            + 4,
                            self.width - 3,
                            self.stepSize,
                        )


class ListBox2(ListBox):
    def __init__(self, *args, **kwargs):
        ListBox.__init__(self, *args, **kwargs)
        self.rowCount = 10
        self.barWidth = 0
        self.colCount = 0

    def SetRowCount(self, rowCount):
        self.rowCount = rowCount

    def SetSize(self, width, height):
        ListBox.SetSize(self, width, height)
        self._RefreshForm()

    def ClearItem(self):
        ListBox.ClearItem(self)
        self._RefreshForm()

    def InsertItem(self, *args, **kwargs):
        ListBox.InsertItem(self, *args, **kwargs)
        self._RefreshForm()

    def OnUpdate(self):
        mpos = wndMgr.GetMousePosition()
        self.overLine = self._CalcPointIndex(mpos)

    def OnRender(self):
        x, y = self.GetGlobalPosition()
        pos = (x + 2, y)

        if -1 != self.overLine:
            grp.SetColor(HALF_WHITE_COLOR)
            self._RenderBar(pos, self.overLine)

        if -1 != self.selectedLine:
            if self.selectedLine >= self.basePos:
                if self.selectedLine - self.basePos < self.showLineCount:
                    grp.SetColor(SELECT_COLOR)
                    self._RenderBar(pos, self.selectedLine - self.basePos)

    def _CalcPointIndex(self, mpos):
        if self.IsIn():
            px, py = mpos
            gx, gy = self.GetGlobalPosition()
            lx, ly = px - gx, py - gy

            col = lx / self.barWidth
            row = ly / self.stepSize
            idx = col * self.rowCount + row
            if col >= 0 and col < self.colCount:
                if row >= 0 and row < self.rowCount:
                    if idx >= 0 and idx < len(self.itemList):
                        return idx

        return -1

    def _CalcRenderPos(self, pos, idx):
        x, y = pos
        row = idx % self.rowCount
        col = idx / self.rowCount
        return x + col * self.barWidth, y + row * self.stepSize

    def _RenderBar(self, basePos, idx):
        x, y = self._CalcRenderPos(basePos, idx)
        grp.RenderBar(x, y, self.barWidth - 3, self.stepSize)

    def _LocateItem(self):
        pos = (0, self.TEMPORARY_PLACE)

        self.showLineCount = 0
        for textLine in self.itemList:
            x, y = self._CalcRenderPos(pos, self.showLineCount)
            textLine.SetPosition(x, y)
            textLine.Show()

            self.showLineCount += 1

    def _RefreshForm(self):
        if len(self.itemList) % self.rowCount:
            self.colCount = len(self.itemList) / self.rowCount + 1
        else:
            self.colCount = len(self.itemList) / self.rowCount

        if self.colCount:
            self.barWidth = self.width / self.colCount
        else:
            self.barWidth = self.width


# 상하 스크롤이 되는 스크롤 윈도우.
# 좌우 스크롤은 생각하지 않음.
# 	ui에 rotate가 없어서 scroll bar를 회전시킬 수가 엄서...
# 	필요한 사람이 추가작업해... 귀찮아...
class ScrollWindow(Window):
    THIN, MIDDLE, THICK = xrange(3)

    # SCROLLBAR_TOP_IMAGE = "d:/ymir work/ui/pattern/ScrollBar_Top.tga"
    # SCROLLBAR_MIDDLE_IMAGE = "d:/ymir work/ui/pattern/ScrollBar_Middle.tga"
    # SCROLLBAR_BOTTOM_IMAGE = "d:/ymir work/ui/pattern/ScrollBar_Bottom.tga"

    def __init__(self, layer="UI"):
        super(ScrollWindow, self).__init__(layer)

        self.layer = layer
        self.startContentOffset = (0, 0)
        self.scrollBar = None
        self.contentWnd = None
        self.scrollSpeed = 4
        self.__loadScrollBar()

    def SetScrollSpeed(self, speed):
        self.scrollSpeed = speed

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition() and self.scrollBar.IsShow():
            times = abs(int(nLen / 120.0)) * self.scrollSpeed
            if nLen > 0:
                for _ in xrange(times):
                    self.scrollBar.OnUp()
            else:
                for _ in xrange(times):
                    self.scrollBar.OnDown()
            return True
        return False

    def __loadScrollBar(self):
        self.scrollBar = SmallThinScrollBar(self.layer)
        self.scrollBar.SetScrollBarSize(self.GetHeight())
        self.scrollBar.SetParent(self)
        self.scrollBar.SetPosition(self.GetWidth() - self.scrollBar.SCROLLBAR_WIDTH, 0)
        self.scrollBar.SetPos(0)
        self.scrollBar.SetScrollEvent(self.__OnScroll)
        self.scrollBar.SetMiddleBarSize(0.3)
        self.scrollBar.SetMiddleButtonDragEndEvent(self.MiddleButtonDragEnd)
        self.scrollBar.SetTop()

    def Show(self):
        super(ScrollWindow, self).Show()
        if self.scrollBar:
            self.scrollBar.Show()

        if self.contentWnd:
            self.contentWnd.Show()

    def SetSize(self, width, height):
        super(ScrollWindow, self).SetSize(width, height)
        self.EnableClipping()
        self.scrollBar.SetScrollBarSize(self.GetHeight())

    def Hide(self):
        super(ScrollWindow, self).Hide()
        if self.scrollBar:
            self.scrollBar.Hide()

        if self.contentWnd:
            self.contentWnd.Hide()

    def NeedScrollbar(self):
        return self.contentWnd.GetHeight() > self.GetHeight()

    def SetContentWindow(self, contentWnd):
        x, y = contentWnd.GetLocalPosition()
        from _weakref import proxy

        self.contentWnd = proxy(contentWnd)
        self.contentWnd.SetParent(self)
        self.contentWnd.SetSizeChangeEvent(self.OnContentWindowSizeChange)
        self.scrollBar.SetMiddleBarSize(0.1)
        self.scrollBar.SetScrollStep(
            1.0 / max(1, (self.contentWnd.GetHeight() - self.GetHeight()))
        )
        self.startContentOffset = (x, y)
        self.SetContentOffset(x, y)

    def OnContentWindowSizeChange(self):
        if self.scrollBar and self.contentWnd:
            if not self.NeedScrollbar():
                self.scrollBar.Hide()
                self.ScrollToStart()
            else:
                self.scrollBar.SetMiddleBarSize(
                    float(self.GetHeight()) / float(self.contentWnd.GetHeight())
                )
                self.scrollBar.SetScrollStep(
                    1.0 / (self.contentWnd.GetHeight() - self.GetHeight())
                )
                self.scrollBar.Show()

    # width를 contentWnd.width + scrollbar.width로 맞춤.
    def FitWidth(self, contentWidth):
        fittedWidth = contentWidth + self.scrollBar.SCROLLBAR_WIDTH
        self.SetSize(fittedWidth, self.GetHeight())
        self.scrollBar.SetScrollBarSize(self.GetHeight())
        self.scrollBar.SetPosition(contentWidth - 2, 0)
        self.scrollBar.SetTop()

    # width를 contentWnd.width + scrollbar.width로 맞춤.
    def AutoFitWidth(self):
        fittedWidth = self.contentWnd.GetWidth() + self.scrollBar.SCROLLBAR_WIDTH
        self.SetSize(fittedWidth, self.GetHeight())
        self.scrollBar.SetScrollBarSize(self.GetHeight())
        self.scrollBar.SetPosition(self.contentWnd.GetWidth(), 0)
        self.scrollBar.SetTop()

    # AutoFitWidth 와 같으나 스크롤 윈도우 안에 스크롤 넣고 싶을 때 사용
    def QuestWindowFitWidth(self):
        fittedWidth = self.contentWnd.GetWidth() + self.scrollBar.SCROLLBAR_WIDTH
        self.SetSize(fittedWidth, self.GetHeight())
        self.scrollBar.SetScrollBarSize(self.GetHeight())
        self.scrollBar.SetPosition(
            self.contentWnd.GetWidth() - self.scrollBar.SCROLLBAR_WIDTH - 10, 0
        )
        self.scrollBar.SetTop()

    # AutoFitWidth 와 같으나 스크롤 윈도우 안에 스크롤 넣고 싶을 때 사용
    def QuestSlideWindowFitWidth(self):
        fittedWidth = self.contentWnd.GetWidth() + self.scrollBar.SCROLLBAR_WIDTH
        self.SetSize(fittedWidth, self.GetHeight())
        self.scrollBar.SetScrollBarSize(self.GetHeight() - 15)
        self.scrollBar.SetPosition(
            self.contentWnd.GetWidth() - self.scrollBar.SCROLLBAR_WIDTH - 10, 15
        )
        self.scrollBar.SetTop()

    def OnUpdate(self):
        if not self.NeedScrollbar():
            self.scrollBar.Hide()

    def __OnScroll(self):
        x, y = self.GetContentOffset()

        scrollPos = self.scrollBar.GetPos() * max(
            1, self.contentWnd.GetHeight() - self.GetHeight()
        )
        scrollUnit = self.scrollBar.GetScrollStep() * max(
            1, self.contentWnd.GetHeight() - self.GetHeight()
        )
        newY = int(round(scrollPos / scrollUnit)) * scrollUnit

        # scrollPos = self.scrollBar.GetPos() * max(0, self.contentWnd.GetHeight() - self.GetHeight())
        # scrollUnit = self.scrollBar.GetScrollStep() * max(0, self.contentWnd.GetHeight() - self.GetHeight())
        # newY = min(max(0, self.contentWnd.GetHeight() - self.GetHeight()), int(round(scrollPos / scrollUnit)) * scrollUnit)

        self.SetContentOffset(x, -newY)

    def ShowScrollBar(self):
        self.scrollBar.Show()

    def HideScrollBar(self):
        self.scrollBar.Hide()

    def MiddleButtonDragEnd(self):
        pass
        # self.scrollBar.SetPos(
        #    int(round(self.scrollBar.GetPos() / self.scrollBar.GetScrollStep())) * self.scrollBar.GetScrollStep())

    def SetScrollStep(self, step):
        self.scrollBar.SetScrollStep(step)

    def GetContentOffset(self):
        return self.contentWnd.GetLocalPosition()

    def SetContentOffset(self, x, y):
        self.contentWnd.SetPosition(int(x), int(round(y)))

    def SetScrollPos(self, pos):
        self.scrollBar.SetPos(pos)

    def ScrollToStart(self):
        self.SetContentOffset(*self.startContentOffset)
        self.SetScrollPos(0.0)


class ScrollWindowWithBackground(ScrollWindow):

    def OnRender(self):
        xRender, yRender = self.GetGlobalPosition()

        widthRender = self.GetWidth()
        heightRender = self.GetHeight()
        # grp.SetColor(BACKGROUND_COLOR_ALPHA)
        grp.RenderBar(xRender, yRender, widthRender, heightRender)
        grp.SetColor(DARK_COLOR)
        grp.RenderLine(xRender, yRender, widthRender, 0)
        grp.RenderLine(xRender, yRender, 0, heightRender)


class SimpleListBox(Window):
    class Item(Window):
        def __init__(self, index, text):
            Window.__init__(self)
            self.parent = None
            self.text = text
            self.index = index
            self.textLine = TextLine()
            self.textLine.SetParent(self)
            self.textLine.SetText(self.text)
            self.textLine.Show()

        def SetParent(self, parent):
            Window.SetParent(self, parent)
            self.parent = proxy(parent)

        def OnMouseLeftButtonDown(self):
            return self.parent.SelectItem(self)

        def OnMouseLeftButtonUp(self):
            return True

        def OnRender(self):
            if self.parent.GetSelectedItem() == self:
                self.OnSelectedRender()

        def GetText(self):
            return self.text

        def GetIndex(self):
            return self.index

        def OnSelectedRender(self):
            x, y = self.GetGlobalPosition()
            grp.SetColor(SELECT_COLOR)
            grp.RenderBar(x, y, self.parent.itemWidth, self.parent.itemHeight)

    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.itemList = []
        self.width = 0
        self.height = 0
        self.itemStep = 0
        self.elementStartOffsetY = 0
        self.itemWidth = 100
        self.itemHeight = 16
        self.selItem = 0
        self.onSelectItemEvent = None

    def SetWidth(self, width):
        self.width = width
        self.itemWidth = width

    def SetElementOffset(self, offsetY):
        self.itemStep = offsetY

    def AppendItem(self, item):
        item.SetParent(self)
        item.SetSize(self.itemWidth, self.itemHeight)

        pos = len(self.itemList)
        item.SetPosition(0, self.height)
        item.Show()

        self.height += item.GetHeight() + self.itemStep
        self.itemList.append(item)
        self.Update()

    def Update(self):
        self.SetSize(self.width, self.height)
        self.UpdateRect()

    def RecalculateHeight(self):
        self.height = 0
        for item in self.itemList:
            self.height = self.height + item.GetHeight() + self.itemStep
        self.Update()

    def GetElementByIndex(self, index):
        if index >= len(self.itemList):
            return None

        return self.itemList[index]

    def SetSelectEvent(self, event):
        self.onSelectItemEvent = MakeEvent(event)

    def GetSelectedItem(self):
        return self.selItem

    def GetSelectedItemText(self):
        if self.selItem:
            return self.selItem.GetText()
        return ""

    def SelectIndex(self, index, useEvent=True):
        if index >= len(self.itemList) or index < 0:
            self.selItem = None
            return

        try:
            if useEvent:
                self.SelectItem(self.itemList[index])
            else:
                self.selItem = self.itemList[index]
        except:
            pass

    def SelectItem(self, selItem):
        self.selItem = selItem
        if self.onSelectItemEvent:
            return self.onSelectItemEvent(selItem)
        return False

    def RemoveAllItems(self):
        self.selItem = None
        self.itemList = []
        self.RecalculateHeight()

    def RemoveItem(self, delItem):
        if delItem == self.selItem:
            self.selItem = None

        self.itemList.remove(delItem)
        self.RecalculateHeight()

    def GetItemCount(self):
        return len(self.itemList)


class ComboBoxAutoComplete(Window):

    def __init__(self, parent, name, x, y, x_scale=1.0, y_scale=1.0):
        Window.__init__(self, "UI")
        self.AddFlag("focusable")
        self.isSelected = False
        self.isOver = False
        self.isListOpened = False
        self.eventSelectEntry = None
        self.eventOnAutoComplete = None
        self.eventFocus = None
        self.eventKillFocus = None
        self.enable = True
        self.imagebox = None

        self.SetParent(parent)
        ## imagebox
        image = ExpandedImageBox()
        image.SetParent(self)
        image.LoadImage(name)
        image.SetScale(x_scale, y_scale)
        image.SetPosition(0, 0)
        image.EnableClipping()
        image.Show()

        self.imagebox = image

        ## BaseSetting
        self.x = x + 1
        self.y = y + 1
        self.width = self.imagebox.GetWidth() - 3
        self.height = self.imagebox.GetHeight() - 3
        self.SetParent(parent)

        # EditLine
        editLine = EditLine()
        editLine.SetParent(self.imagebox)
        editLine.SetPosition(3, 3)
        editLine.SetOnChangeEvent(self.OnAutoComplete)
        editLine.SetCharEvent(self.IsInputEnabled)
        editLine.SetOnMouseLeftButtonDownEvent(self.OnMouseLeftButtonDown)
        editLine.SetKillFocusEvent(self.OnKillFocus)
        editLine.SetMax(34)
        editLine.SetSize(self.imagebox.GetWidth() - 3, self.imagebox.GetHeight() - 3)
        editLine.UpdateRect()
        editLine.Show()
        self.editLine = editLine

        ## ListBox
        self.listBox = SimpleListBox("TOP_MOST")
        self.listBox.SetSize(self.width, 1)
        self.listBox.SetSelectEvent(self.OnSelectItem)
        self.listBox.SetWidth(self.width)
        # self.listBox.SetTextCenterAlign(False)
        # self.listBox.SetItemFontColor(WHITE_COLOR)
        # self.listBox.SetItemFontName(localeInfo.UI_DEF_FONT_NUNITO)
        self.listBox.Hide()

        self.scrollWindow = ScrollWindowWithBackground("TOP_MOST")
        self.scrollWindow.SetSize(self.width, 160)
        self.scrollWindow.SetParent(self)
        self.scrollWindow.SetContentWindow(self.listBox)
        self.scrollWindow.AutoFitWidth()
        self.scrollWindow.SetPosition(0, self.height + 3)
        self.scrollWindow.Hide()

        Window.SetPosition(self, self.x, self.y)
        Window.SetSize(self, self.width, self.height)
        self.editLine.UpdateRect()
        self.__ArrangeListBox()

    def Destroy(self):
        self.listBox = None
        self.imagebox = None
        self.editLine = None
        self.eventOnAutoComplete = None
        self.eventSelectEntry = None

    def GetImageBox(self):
        return self.imagebox

    def SetAutoCompleteEvent(self, event):
        self.eventOnAutoComplete = MakeEvent(event)

    def SetOnMouseLeftButtonDownEvent(self, event):
        self.eventFocus = MakeEvent(event)

    def SetKillFocusEvent(self, event):
        self.eventKillFocus = MakeEvent(event)

    def SetPosition(self, x, y):
        Window.SetPosition(self, x, y)
        self.imagebox.SetPosition(x, y)
        self.x = x
        self.y = y
        self.editLine.UpdateRect()
        self.__ArrangeListBox()

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)
        self.width = width
        self.height = height
        self.__ArrangeListBox()

    def __ArrangeListBox(self):
        self.listBox.SetPosition(0, self.height + 5)
        self.listBox.SetWidth(self.width)
        self.scrollWindow.ScrollToStart()

    def Show(self):
        Window.Show(self)
        self.imagebox.Show()
        self.editLine.Show()

    def Hide(self):
        Window.Hide(self)
        self.imagebox.Hide()
        self.editLine.Hide()
        self.scrollWindow.ReleaseMouse()

    def Enable(self):
        self.editLine.Enable()
        self.enable = True

    def Disable(self):
        self.enable = False
        self.editLine.Disable()
        self.CloseListBox()

    def SetEvent(self, event):
        self.eventSelectEntry = MakeEvent(event)

    def ClearItem(self):
        self.CloseListBox()
        self.listBox.RemoveAllItems()

    def InsertItem(self, index, name):
        self.listBox.AppendItem(SimpleListBox.Item(index, name))

    def SetCurrentItem(self, text):
        self.editLine.SetText(text)

    def SelectItem(self, key):
        self.listBox.SelectItem(key)

    def OnSelectItem(self, item):
        self.CloseListBox()
        if item:
            self.editLine.SetText(item.GetText())
            return self.eventSelectEntry(item.GetIndex())
        return False

    def CloseListBox(self):
        self.isListOpened = False
        self.scrollWindow.Hide()
        self.scrollWindow.ReleaseMouse()

    def OpenListBox(self):
        if self.listBox.GetItemCount() > 0:
            self.isListOpened = True
            self.scrollWindow.Show()
            self.scrollWindow.CaptureMouse()
            self.__ArrangeListBox()

    def GetInput(self):
        return self.editLine.GetText()

    def ClearInput(self):
        self.editLine.SetText("")

    def IsInputEnabled(self):
        return self.enable

    def OnMouseLeftButtonDown(self):
        if not self.enable:
            return False

        self.isSelected = True

        if self.eventFocus:
            return self.eventFocus()

        return False

    def OnMouseLeftButtonUp(self):
        if not self.enable:
            return False

        self.isSelected = False

        if self.isListOpened:
            self.CloseListBox()
        else:
            self.OpenListBox()

        return True

    def OnAutoComplete(self):
        if not self.enable:
            return False

        if self.eventOnAutoComplete:
            return self.eventOnAutoComplete()
        return True

    def OnKillFocus(self):
        if self.eventKillFocus:
            return self.eventKillFocus()

        return False

    def IsListOpened(self):
        return self.isListOpened

    def GetEditLine(self):
        return self.editLine


class InputBox(Window):
    def __init__(self, parent, name, x, y, max=34, defaultVal="", numberOnly=False):
        Window.__init__(self, "UI")
        self.event = None
        self.enable = True
        self.imagebox = None
        self.defaultVal = defaultVal

        ## imagebox
        image = ImageBox("UI")
        image.SetParent(parent)
        image.LoadImage(name)
        image.SetPosition(x, y)
        image.Show()
        self.imagebox = image

        ## BaseSetting
        self.x = x + 1
        self.y = y + 1
        self.width = self.imagebox.GetWidth() - 3
        self.height = self.imagebox.GetHeight() - 3
        self.SetParent(parent)

        # EditLine
        editLine = EditLine()
        editLine.SetParent(parent)
        editLine.SetPosition(x + 3, y + 3)
        editLine.SetMax(max)
        editLine.SetText(defaultVal)
        if numberOnly:
            editLine.SetNumberMode()
        editLine.SetSize(self.imagebox.GetWidth() - 3, self.imagebox.GetHeight() - 3)
        editLine.UpdateRect()
        editLine.Show()
        self.editLine = editLine

        Window.SetPosition(self, self.x, self.y)
        Window.SetSize(self, self.width, self.height)
        self.editLine.UpdateRect()

    def Destroy(self):
        self.imagebox = None
        self.editLine = None

    def SetPosition(self, x, y):
        Window.SetPosition(self, x, y)
        self.imagebox.SetPosition(x, y)
        self.x = x
        self.y = y
        self.editLine.UpdateRect()

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)
        self.width = width
        self.height = height

    def Show(self):
        Window.Show(self)
        self.imagebox.Show()
        self.editLine.Show()

    def Hide(self):
        Window.Hide(self)
        self.imagebox.Hide()
        self.editLine.Hide()

    def Enable(self):
        self.enable = True

    def Disable(self):
        self.enable = False

    def SetEvent(self, event):
        self.event = MakeEvent(event)

    def GetText(self):
        return self.editLine.GetText()

    def SetText(self, text):
        return self.editLine.SetText(text)

    def ClearInput(self):
        self.editLine.SetText(self.defaultVal)

    def Disable(self):
        self.editLine.Disable()

    def Enable(self):
        self.editLine.Enable()

    def OnMouseLeftButtonDown(self):

        if not self.enable:
            return False

        self.isSelected = True
        return True

    def OnMouseLeftButtonUp(self):
        if not self.enable:
            return False
        return True


class ComboBoxNew(Window):
    class ListBoxWithBoard(ListBox):

        def __init__(self, layer):
            ListBox.__init__(self, layer)

        def OnRender(self):
            xRender, yRender = self.GetGlobalPosition()
            yRender -= self.TEMPORARY_PLACE
            widthRender = self.width
            heightRender = self.height + self.TEMPORARY_PLACE * 2
            # grp.SetColor(BACKGROUND_COLOR)
            grp.RenderBar(xRender, yRender, widthRender, heightRender)
            grp.SetColor(DARK_COLOR)
            grp.RenderLine(xRender, yRender, widthRender, 0)
            grp.RenderLine(xRender, yRender, 0, heightRender)
            ListBox.OnRender(self)

    def __init__(self):
        Window.__init__(self, "UI")
        self.AddFlag("focusable")
        self.isSelected = False
        self.isOver = False
        self.isListOpened = False
        self.event = None
        self.enable = True
        self.imagebox = None
        self.eventOnListBoxOpen = None
        self.eventOnListBoxClose = None
        self.eventFocus = None
        self.textLine = None
        self.listBox = None
        self.viewItemCount = 5

    def Create(self, parent, imageName=None, x=0, y=0):
        if imageName is None:
            imageName = "d:/ymir work/ui/game/windows/combobox_bg.tga"

        self.SetParent(parent)

        ## imagebox
        image = ExpandedImageBox()
        image.SetParent(self)
        image.LoadImage(imageName)
        image.SetClickEvent(self.OnMouseLeftButtonUp)
        image.SetPosition(0, 0)
        image.EnableClipping()
        image.Show()
        self.imagebox = image

        ## BaseSetting
        self.x = x + 1
        self.y = y + 1
        self.width = self.imagebox.GetWidth() - 3
        self.height = self.imagebox.GetHeight() - 3

        ## TextLine
        self.textLine = MakeTextLine(self.imagebox)
        self.textLine.SetText(localeInfo.UI_ITEM)

        ## ListBox
        self.listBox = SimpleListBox("TOP_MOST")
        self.listBox.SetSize(self.width, 1)
        self.listBox.SetSelectEvent(self.OnSelectItem)
        self.listBox.SetWidth(self.width)
        self.listBox.Hide()

        self.scrollWindow = ScrollWindowWithBackground("TOP_MOST")
        self.scrollWindow.SetSize(self.width, 160)
        self.scrollWindow.SetParent(self)
        self.scrollWindow.SetContentWindow(self.listBox)
        self.scrollWindow.FitWidth(self.width - 5)
        self.scrollWindow.SetPosition(0, self.height)
        self.scrollWindow.Hide()

        Window.SetPosition(self, self.x, self.y)
        Window.SetSize(self, self.width, self.height)
        self.textLine.UpdateRect()
        self.__ArrangeListBox()

    def SetListBoxOpenEvent(self, event):
        self.eventOnListBoxOpen = MakeEvent(event)

    def SetListBoxCloseEvent(self, event):
        self.eventOnListBoxClose = MakeEvent(event)

    def SetOnMouseLeftButtonDownEvent(self, event):
        self.eventFocus = MakeEvent(event)

    def Destroy(self):
        self.textLine = None
        self.listBox = None
        self.imagebox = None

    def SetPosition(self, x, y):
        Window.SetPosition(self, x, y)
        self.imagebox.SetPosition(x, y)
        self.x = x
        self.y = y
        self.__ArrangeListBox()

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)
        self.imagebox.SetScale(float(width) / float(self.imagebox.GetWidth()), 1.0)
        self.width = width
        self.height = height
        self.scrollWindow.SetSize(self.width, 160)
        self.scrollWindow.FitWidth(self.width - 5)
        self.listBox.RecalculateHeight()
        self.textLine.UpdateRect()
        self.__ArrangeListBox()

    def __ArrangeListBox(self):
        self.listBox.SetPosition(0, 0)
        self.listBox.SetWidth(self.width)

    def Enable(self):
        self.enable = True

    def Disable(self):
        self.enable = False
        self.textLine.SetText("")
        self.CloseListBox()

    def SetEvent(self, event):
        self.event = MakeEvent(event)

    def ClearItem(self):
        self.CloseListBox()
        self.listBox.RemoveAllItems()

    def InsertItem(self, index, name):
        self.listBox.AppendItem(SimpleListBox.Item(index, name))
        self.scrollWindow.SetSize(self.width, self.viewItemCount * 18)

    def SetViewItemCount(self, count):
        self.viewItemCount = count

    def SetCurrentItem(self, text):
        self.textLine.SetText(text)

    def SelectItem(self, key):
        self.listBox.SelectIndex(key, False)
        self.textLine.SetText(self.listBox.GetElementByIndex(key).GetText())

    def OnSelectItem(self, selItem):
        self.CloseListBox()
        if selItem is not None:
            self.textLine.SetText(selItem.GetText())
            if self.event:
                self.event(selItem.GetIndex())

    def CloseListBox(self):
        self.isListOpened = False
        self.scrollWindow.Hide()
        self.scrollWindow.ReleaseMouse()

    def OpenListBox(self):
        if self.listBox.GetItemCount() > 0:
            self.isListOpened = True
            self.scrollWindow.Show()
            self.scrollWindow.CaptureMouse()
            self.__ArrangeListBox()

    def OnMouseLeftButtonDown(self):
        if not self.enable:
            return False

        self.isSelected = True

        if self.eventFocus:
            self.eventFocus()
        return True

    def OnMouseLeftButtonUp(self):
        if not self.enable:
            return False

        self.isSelected = False

        if self.isListOpened:
            self.CloseListBox()
        else:
            self.OpenListBox()

        return True

    def OnKillFocus(self):
        self.isSelected = False

        if self.isListOpened:
            self.CloseListBox()
        return True


if app.ENABLE_PRIVATESHOP_SEARCH_SYSTEM:

    class ComboBoxImage(Window):
        class ListBoxWithBoard(ListBox):

            def __init__(self, layer):
                ListBox.__init__(self, layer)

            def OnRender(self):
                xRender, yRender = self.GetGlobalPosition()
                yRender -= self.TEMPORARY_PLACE
                widthRender = self.width
                heightRender = self.height + self.TEMPORARY_PLACE * 2
                grp.SetColor(BACKGROUND_COLOR)
                grp.RenderBar(xRender, yRender, widthRender, heightRender)
                grp.SetColor(DARK_COLOR)
                grp.RenderLine(xRender, yRender, widthRender, 0)
                grp.RenderLine(xRender, yRender, 0, heightRender)
                ListBox.OnRender(self)

        def __init__(self, parent, imageName, x, y):
            Window.__init__(self)

            self.isSelected = False
            self.isOver = False
            self.isListOpened = False
            self.event = None
            self.enable = True
            self.imagebox = None
            self.eventOnListBoxOpen = None
            self.eventOnListBoxClose = None

            ## imagebox
            image = ImageBox()
            image.SetParent(self)
            image.LoadImage(imageName)
            image.SetClickEvent(self.OnMouseLeftButtonUp)
            image.SetPosition(0, 0)
            image.Show()
            self.imagebox = image

            ## BaseSetting
            self.x = x + 1
            self.y = y + 1
            self.width = self.imagebox.GetWidth() - 3
            self.height = self.imagebox.GetHeight() - 3

            ## TextLine
            self.textLine = MakeTextLine(self.imagebox)
            self.textLine.SetText(localeInfo.UI_ITEM)

            ## ListBox
            self.listBox = self.ListBoxWithBoard("TOP_MOST")
            self.listBox.SetParent(self)
            self.listBox.SetEvent(self.OnSelectItem)
            self.listBox.Hide()

            self.SetParent(parent)

            Window.SetPosition(self, self.x, self.y)
            Window.SetSize(self, self.width, self.height)
            self.textLine.UpdateRect()
            self.__ArrangeListBox()

        def SetListBoxOpenEvent(self, event):
            self.eventOnListBoxOpen = MakeEvent(event)

        def SetListBoxCloseEvent(self, event):
            self.eventOnListBoxClose = MakeEvent(event)

        def Destroy(self):
            self.textLine = None
            self.listBox = None
            self.imagebox = None

        def SetPosition(self, x, y):
            Window.SetPosition(self, x, y)
            self.imagebox.SetPosition(x, y)
            self.x = x
            self.y = y
            self.__ArrangeListBox()

        def SetSize(self, width, height):
            Window.SetSize(self, width, height)
            self.width = width
            self.height = height
            self.textLine.UpdateRect()
            self.__ArrangeListBox()

        def __ArrangeListBox(self):
            self.listBox.SetPosition(0, self.height + 5)
            self.listBox.SetWidth(self.width)

        def Enable(self):
            self.enable = True

        def Disable(self):
            self.enable = False
            self.textLine.SetText("")
            self.CloseListBox()

        def SetEvent(self, event):
            self.event = MakeEvent(event)

        def ClearItem(self):
            self.CloseListBox()
            self.listBox.ClearItem()

        def InsertItem(self, index, name):
            self.listBox.InsertItem(index, name)
            self.listBox.ArrangeItem()

        def SetCurrentItem(self, text):
            self.textLine.SetText(text)

        def SelectItem(self, key):
            self.listBox.SelectItem(key)

        def OnSelectItem(self, index, name):
            self.CloseListBox()
            return self.event(index)

        def CloseListBox(self):
            if self.eventOnListBoxClose:
                self.eventOnListBoxClose()
            self.isListOpened = False
            self.listBox.Hide()
            self.listBox.ReleaseMouse()

        def OnMouseLeftButtonDown(self):
            if not self.enable:
                return False

            self.isSelected = True
            return True

        def OnMouseLeftButtonUp(self):
            if not self.enable:
                return False

            self.isSelected = False

            if self.isListOpened:
                self.CloseListBox()
            else:
                if self.listBox.GetItemCount() > 0:
                    self.isListOpened = True
                    if self.eventOnListBoxOpen:
                        self.eventOnListBoxOpen()
                    self.listBox.Show()
                    self.listBox.CaptureMouse()
                    self.__ArrangeListBox()

            return True

        def OnUpdate(self):

            if not self.enable:
                return

            if self.IsIn():
                self.isOver = True
            else:
                self.isOver = False

        def OnRender(self):
            self.x, self.y = self.GetGlobalPosition()
            xRender = self.x
            yRender = self.y
            widthRender = self.width
            heightRender = self.height
            if self.isOver:
                grp.SetColor(HALF_WHITE_COLOR)
                grp.RenderBar(
                    xRender + 2, yRender + 3, self.width - 3, heightRender - 5
                )
                if self.isSelected:
                    grp.SetColor(WHITE_COLOR)
                    grp.RenderBar(
                        xRender + 2, yRender + 3, self.width - 3, heightRender - 5
                    )


class ComboBox(Window):
    class ListBoxWithBoard(ListBox):
        def OnRender(self):
            x, y, w, h = self.GetRect()

            y -= self.TEMPORARY_PLACE
            h += self.TEMPORARY_PLACE * 2

            grp.SetColor(BACKGROUND_COLOR)
            grp.RenderBar(x, y, w, h)

            grp.SetColor(DARK_COLOR)
            grp.RenderLine(x, y, w, 0)
            grp.RenderLine(x, y, 0, h)

            grp.SetColor(BRIGHT_COLOR)
            grp.RenderLine(x, y + h, w, 0)
            grp.RenderLine(x + w, y, 0, h)

            ListBox.OnRender(self)

    class ComboBoxButton(Window):
        def OnMouseLeftButtonUp(self):
            self.GetParent().OnMouseLeftButtonUp()
            return True

        def OnRender(self):
            x, y, w, h = self.GetRect()

            grp.SetColor(BACKGROUND_COLOR)
            grp.RenderBar(x, y, w, h)

            grp.SetColor(DARK_COLOR)
            grp.RenderLine(x, y, w, 0)
            grp.RenderLine(x, y, 0, h)

            grp.SetColor(BRIGHT_COLOR)
            grp.RenderLine(x, y + h, w, 0)
            grp.RenderLine(x + w, y, 0, h)

            if self.IsIn():
                grp.SetColor(HALF_WHITE_COLOR)
                grp.RenderBar(x + 2, y + 3, w - 3, h - 5)

                if self.GetParent().isSelected:
                    grp.SetColor(WHITE_COLOR)
                    grp.RenderBar(x + 2, y + 3, w - 3, h - 5)

    def __init__(self):
        Window.__init__(self)

        self.isSelected = False
        self.isListOpened = False
        self.event = None
        self.enable = True

        self.button = self.ComboBoxButton()
        self.button.SetParent(self)
        self.button.SetTop()
        self.button.Show()

        self.textLine = TextLine()
        self.textLine.SetParent(self.button)
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetText(localeInfo.UI_ITEM)
        self.textLine.Show()

        self.listBox = self.ListBoxWithBoard()
        self.listBox.SetParent(self)
        self.listBox.SetEvent(self.OnSelectItem)
        self.listBox.Hide()

        self.AddFlag("not_pick")

    def Destroy(self):
        self.textLine = None
        self.listBox = None

    def OpenListBox(self):
        self.isListOpened = True
        self.listBox.Show()
        self.listBox.CaptureMouse()

    def CloseListBox(self):
        self.isListOpened = False
        self.listBox.Hide()
        self.listBox.ReleaseMouse()

    # This actually sets only the button size
    def SetSize(self, width, height):
        self.button.SetSize(width, height)
        self.__Fit()

    def Enable(self):
        self.enable = True

    def Disable(self):
        self.enable = False
        self.textLine.SetText("")
        self.CloseListBox()

    def SetEvent(self, event):
        self.event = MakeEvent(event)

    def ClearItem(self):
        self.CloseListBox()
        self.listBox.ClearItem()
        self.__Fit()

    def InsertItem(self, index, name):
        self.listBox.InsertItem(index, name)
        self.listBox.ArrangeItem()
        self.__Fit()

    def SetCurrentItem(self, text):
        self.textLine.SetText(text)

    def SelectItem(self, key):
        self.listBox.SelectItem(key)

    def OnSelectItem(self, index, name):
        self.CloseListBox()
        if self.event:
            self.event(index)

    # Due to FLAG_NOT_PICK this is only called when we have captured the mouse
    def OnMouseLeftButtonDown(self):
        if not self.enable:
            return False

        self.isSelected = True
        return True

    # Due to FLAG_NOT_PICK this is called when we have captured the mouse
    # and by ComboBoxButton.OnMouseLeftButtonUp
    def OnMouseLeftButtonUp(self):
        if not self.enable:
            return False

        self.isSelected = False

        if self.isListOpened:
            self.CloseListBox()
        elif self.listBox.GetItemCount() > 0:
            self.OpenListBox()
        return True

    def __Fit(self):
        bw, bh = self.button.GetWidth(), self.button.GetHeight()
        lh = self.listBox.GetHeight()

        self.listBox.SetPosition(0, bh + 5)
        self.listBox.SetSize(bw, lh)
        Window.SetSize(self, bw, bh + 5 + lh)


class Table(Window):
    ROW_HEIGHT = 18
    HEADER_EXTRA_HEIGHT = 0

    class TableLine(Window):

        def __init__(self, mouseLeftButtonDownEvent, mouseLeftButtonDoubleClickEvent):
            Window.__init__(self)

            self.colSize = []
            self.textLines = []

            self.mouseLeftButtonDownEvent = MakeEvent(mouseLeftButtonDownEvent)
            self.mouseLeftButtonDoubleClickEvent = MakeEvent(
                mouseLeftButtonDoubleClickEvent
            )

            self.SetSize(0, Table.ROW_HEIGHT)

            self.SetWindowName("NONAME_Table_TableLine")

        def __UpdateWidth(self, appendSize):
            self.SetSize(self.GetWidth() + appendSize, self.GetHeight())

        def __CheckLength(self, line, maxWidth):
            if line.GetWidth() <= maxWidth:
                return line.GetText()

            text = line.GetText()
            pos = len(text)
            while pos > 1:
                pos = pos - 1

                line.SetText(text[:pos] + "..")
                if line.GetWidth() <= maxWidth and text[pos - 1] != " ":
                    return text[:pos] + ".."

            return ".."

        def AppendCol(self, wnd, width, checkLength):
            lineWnd = WindowWithBaseEvents()
            lineWnd.SetParent(self)
            lineWnd.SetMouseLeftButtonDownEvent(self.mouseLeftButtonDownEvent)
            # lineWnd.SetMouseLeftButtonDownEvent(self.mouseLeftButtonDoubleClickEvent)
            lineWnd.SetSize(width, self.GetHeight())
            lineWnd.SetPosition(self.GetWidth(), 0)
            lineWnd.Show()

            if (
                isinstance(wnd, basestring)
                or isinstance(wnd, int)
                or isinstance(wnd, float)
            ):
                textWnd = TextLine()
                textWnd.SetParent(lineWnd)
                textWnd.SetHorizontalAlignCenter()
                textWnd.SetVerticalAlignCenter()
                textWnd.SetText(str(wnd))
                if checkLength == True:
                    textWnd.SetText(self.__CheckLength(textWnd, width))
                textWnd.SetPosition(0, 0)
                textWnd.UpdateRect()
                textWnd.Show()

                lineWnd.wnd = textWnd
            else:
                wnd.SetParent(lineWnd)
                wnd.SetPosition(self.GetWidth(), 0)
                wnd.SetHorizontalAlignCenter()
                wnd.SetVerticalAlignCenter()
                wnd.Show()
                lineWnd.wnd = wnd

            self.textLines.append(lineWnd)

            self.__UpdateWidth(width)

        def OnMouseLeftButtonDown(self):
            return self.mouseLeftButtonDownEvent()

        def OnMouseLeftButtonDoubleClick(self):
            self.mouseLeftButtonDoubleClickEvent()

    def __init__(self):
        Window.__init__(self)

        self.cols = 0
        self.rows = 0
        self.basePos = 0
        self.viewLineCount = 0

        self.overLine = -1
        self.overHeader = -1
        self.selectedKey = -1
        self.selectedLine = -1

        self.overRender = None
        self.selectRender = None

        self.colSizePct = []
        self.checkLengthIndexes = []
        self.maxColSizePct = 100
        self.headerLine = None
        self.lines = []
        self.keys = []
        self.keyDict = {}

        self.headerClickEvent = None
        self.doubleClickEvent = None

        self.SetWindowName("NONAME_Table")

    def Destroy(self):
        self.lines = []

    def SetWidth(self, width):
        self.SetSize(width, self.GetHeight())

    def SetColSizePct(self, colSizeList):
        self.colSizePct = []
        self.maxColSizePct = 0
        for size in colSizeList:
            self.colSizePct.append(size)
            self.maxColSizePct += size
        self.LocateLines()

    def AddCheckLengthIndex(self, index):
        self.checkLengthIndexes.append(index)

    def GetColSize(self, index):
        return int(self.GetWidth() * self.colSizePct[index] / self.maxColSizePct)

    def __BuildLine(self, colList):
        line = self.TableLine(
            self.OnMouseLeftButtonDown, self.OnMouseLeftButtonDoubleClick
        )
        line.SetParent(self)
        for i in xrange(len(colList)):
            line.AppendCol(colList[i], self.GetColSize(i), i in self.checkLengthIndexes)
        return line

    def SetHeader(self, colList, extraHeight=0):
        self.headerLine = self.__BuildLine(colList)
        self.HEADER_EXTRA_HEIGHT = extraHeight
        self.LocateLines()

    def ClearHeader(self):
        self.headerLine = None
        self.LocateLines()

    def Clear(self):
        self.lines = []
        self.keys = []
        self.basePos = 0
        self.overLine = -1
        self.overHeader = -1
        self.selectedKey = -1
        self.selectedLine = -1
        self.LocateLines()

    def GetLineCount(self):
        return len(self.lines)

    def GetMaxLineCount(self):
        if self.GetHeight() < self.ROW_HEIGHT:
            return 0

        maxHeight = self.GetHeight()
        if self.headerLine != None:
            maxHeight -= self.ROW_HEIGHT

        return int(maxHeight / self.ROW_HEIGHT)

    def GetViewLineCount(self):
        return self.viewLineCount

    def Append(self, index, colList, refresh=True):
        self.keyDict[index] = len(self.lines)
        self.lines.append(self.__BuildLine(colList))
        self.keys.append(index)
        if refresh == True:
            self.LocateLines()

    def Erase(self, index):
        if index not in self.keyDict:
            return

        listIndex = self.keyDict[index]

        for i in xrange(listIndex + 1, len(self.lines)):
            self.keyDict[self.keys[i]] -= 1

        del self.lines[listIndex]
        del self.keys[listIndex]
        del self.keyDict[index]

        if self.selectedLine != -1:
            if self.selectedKey == index:
                self.selectedKey = -1
                self.selectedLine = -1
            else:
                self.selectedLine = self.keyDict[self.selectedKey]
            self.__RefreshSelectedLineRender()

        if self.basePos <= listIndex < self.basePos + self.viewLineCount:
            if (
                self.basePos > 0
                and self.GetLineCount() < self.basePos + self.viewLineCount
            ):
                self.basePos -= 1
            self.LocateLines()

    def LocateLines(self):
        maxHeight = self.GetHeight()
        if maxHeight < self.ROW_HEIGHT:
            maxHeight = 0

        height = 0

        if self.headerLine is not None:
            self.headerLine.SetPosition(0, height)
            self.headerLine.Show()
            height += self.ROW_HEIGHT + self.HEADER_EXTRA_HEIGHT

        self.viewLineCount = 0

        for i in xrange(len(self.lines)):
            if i < self.basePos or (
                maxHeight != 0 and height + self.ROW_HEIGHT >= maxHeight
            ):
                self.lines[i].Hide()
                continue

            self.lines[i].SetPosition(0, height)
            self.lines[i].Show()
            height += self.ROW_HEIGHT

            self.viewLineCount += 1

    def SetBasePos(self, basePos):
        if basePos < 0:
            basePos = 0
        elif basePos + self.GetMaxLineCount() > self.GetLineCount():
            basePos = max(0, self.GetLineCount() - self.GetMaxLineCount())

        self.basePos = basePos
        self.LocateLines()
        self.__RefreshSelectedLineRender()

    def GetBasePos(self):
        return self.basePos

    def SelectLine(self, line):
        if line < 0 or line >= self.GetViewLineCount():
            line = -1
        else:
            line += self.basePos

        self.selectedKey = self.keys[line]
        self.selectedLine = line
        self.__RefreshSelectedLineRender()

    def __RefreshSelectedLineRender(self):
        self.selectRender = None

        if self.selectedLine == -1:
            return

        if (
            self.selectedLine < self.basePos
            or self.selectedLine >= self.basePos + self.viewLineCount
        ):
            return

        x, y = self.GetGlobalPosition()
        if self.headerLine != None:
            y += self.ROW_HEIGHT + self.HEADER_EXTRA_HEIGHT

        self.selectRender = {
            "x": x,
            "y": y + self.ROW_HEIGHT * (self.selectedLine - self.basePos),
            "width": self.GetWidth(),
            "height": self.ROW_HEIGHT,
        }

    def OnMouseLeftButtonDown(self):
        if self.overLine != -1:
            self.SelectLine(self.overLine)
        elif self.overHeader != -1:
            if self.headerClickEvent:
                self.headerClickEvent(self.overHeader)
        return True

    def SetDoubleClickEvent(self, event):
        self.doubleClickEvent = MakeEvent(event)

    def OnMouseLeftButtonDoubleClick(self):
        if self.selectedLine != -1 and self.overLine == self.selectedLine:
            if self.doubleClickEvent:
                self.doubleClickEvent(self.selectedKey)

    def SetHeaderClickEvent(self, event):
        self.headerClickEvent = MakeEvent(event)

    def OnUpdate(self):
        self.__RefreshSelectedLineRender()

        self.overLine = -1
        self.overHeader = -1
        self.overRender = None

        x, y = self.GetGlobalPosition()
        xMouse, yMouse = wndMgr.GetMousePosition()

        if xMouse < x or xMouse > x + self.GetWidth():
            return

        if self.headerLine != None:
            y += self.ROW_HEIGHT + self.HEADER_EXTRA_HEIGHT

        overLine = int((yMouse - y) / self.ROW_HEIGHT)
        if overLine < 0 or overLine >= self.viewLineCount:
            if (
                yMouse >= y - (self.ROW_HEIGHT + self.HEADER_EXTRA_HEIGHT)
                and yMouse < y - self.HEADER_EXTRA_HEIGHT
                and self.headerLine != None
            ):
                width = 0
                headerColIndex = 0
                for i in xrange(len(self.colSizePct)):
                    width = int(
                        self.GetWidth() * self.colSizePct[i] / self.maxColSizePct
                    )
                    if xMouse <= x + width:
                        break
                    headerColIndex += 1
                    x += width
                    if headerColIndex >= len(self.colSizePct):
                        return

                self.overHeader = headerColIndex
                self.overRender = {
                    "x": x,
                    "y": y - self.ROW_HEIGHT - self.HEADER_EXTRA_HEIGHT,
                    "width": width,
                    "height": self.ROW_HEIGHT,
                }

            return

        self.overLine = overLine
        self.overRender = {
            "x": x,
            "y": y + overLine * self.ROW_HEIGHT,
            "width": self.GetWidth(),
            "height": self.ROW_HEIGHT,
        }

    def __DrawRender(self, color, render):
        grp.SetColor(color)
        grp.RenderBar(render["x"], render["y"], render["width"], render["height"])

    def OnRender(self):
        if self.overRender:
            self.__DrawRender(HALF_WHITE_COLOR, self.overRender)
        if self.selectRender:
            self.__DrawRender(SELECT_COLOR, self.selectRender)


class UpDownButton(Window):

    def __init__(self):
        Window.__init__(self)

        self.cur = 0
        self.min = 0
        self.max = 0

        self.downBtn = None
        self.lastDownTime = 0
        self.isDownUsed = False

        slotImage = ImageBox()
        slotImage.SetParent(self)
        slotImage.LoadImage("d:/ymir work/ui/updownslot.tga")
        slotImage.SetAlpha(0.3)
        slotImage.Show()

        numberLine = TextLine()
        numberLine.SetParent(slotImage)
        numberLine.SetHorizontalAlignCenter()
        numberLine.SetHorizontalAlignCenter()
        numberLine.SetVerticalAlignCenter()
        numberLine.Show()

        upButton = Button()
        upButton.SetParent(self)
        upButton.SetPosition(slotImage.GetRight(), 0)
        upButton.SetUpVisual("d:/ymir work/ui/public/scrollbar_thin_up_button_01.sub")
        upButton.SetOverVisual("d:/ymir work/ui/public/scrollbar_thin_up_button_02.sub")
        upButton.SetDownVisual("d:/ymir work/ui/public/scrollbar_thin_up_button_03.sub")
        upButton.SetEvent(self.OnClickUpButton)
        upButton.SetDownEvent(self.OnDownUpButton)
        upButton.Show()

        downButton = Button()
        downButton.SetParent(self)
        downButton.SetPosition(slotImage.GetRight(), upButton.GetBottom())
        downButton.SetUpVisual(
            "d:/ymir work/ui/public/scrollbar_thin_down_button_01.sub"
        )
        downButton.SetOverVisual(
            "d:/ymir work/ui/public/scrollbar_thin_down_button_02.sub"
        )
        downButton.SetDownVisual(
            "d:/ymir work/ui/public/scrollbar_thin_down_button_03.sub"
        )
        downButton.SetEvent(self.OnClickDownButton)
        downButton.SetDownEvent(self.OnDownDownButton)
        downButton.Show()

        self.slotImage = slotImage
        self.upButton = upButton
        self.downButton = downButton
        self.numberLine = numberLine

        self.SetSize(
            max(upButton.GetRight(), downButton.GetRight()),
            max(slotImage.GetHeight(), downButton.GetBottom()),
        )
        self.Refresh()

    def SetValue(self, val):
        val = int(val)
        val = max(val, self.min)
        if self.max > self.min:
            val = min(val, self.max)
        self.cur = val

        self.Refresh()

    def GetValue(self):
        return self.cur

    def SetMin(self, val):
        self.min = int(val)
        self.SetValue(self.cur)

    def SetMax(self, val):
        self.max = int(val)
        self.SetValue(self.cur)

    def OnClickUpButton(self):
        if self.isDownUsed:
            return
        self.SetValue(self.cur + 1)

    def OnDownUpButton(self):
        self.downBtn = self.upButton
        self.lastDownTime = app.GetTime()
        self.isDownUsed = False

    def OnClickDownButton(self):
        if self.isDownUsed:
            return
        self.SetValue(self.cur - 1)

    def OnDownDownButton(self):
        self.downBtn = self.downButton
        self.lastDownTime = app.GetTime()
        self.isDownUsed = False

    def Refresh(self):
        self.numberLine.SetText(str(self.cur))
        self.numberLine.UpdateRect()

    def OnUpdate(self):
        if self.downBtn != None:
            if not self.downBtn.IsDown():
                self.downBtn = None
                self.isDownUsed = False
                return

            if app.GetTime() - self.lastDownTime >= 0.25 or (
                app.GetTime() - self.lastDownTime >= 0.15 and self.isDownUsed == False
            ):
                self.lastDownTime = app.GetTime()

                self.isDownUsed = False
                self.downBtn.CallEvent()
                self.isDownUsed = True


class InputField(Window):
    PATH = "d:/ymir work/ui/pattern/input_%s.tga"

    BORDER_SIZE = 1
    BASE_SIZE = 1

    L = 0
    R = 1
    T = 2
    B = 3

    def __init__(self):
        Window.__init__(self)
        self.SetWindowName("NONAME_InputField")
        self.onClickEvent = None
        self.Lines = []

    def MakeField(self, basePath=PATH):
        self.Lines = []
        for i in xrange(4):
            line = ExpandedImageBox()
            line.SetParent(self)
            line.LoadImage(basePath % "border")
            line.Show()
            self.Lines.append(line)

        self.Lines[self.T].SetPosition(self.BORDER_SIZE, 0)
        self.Lines[self.B].SetPosition(self.BORDER_SIZE, 0)

        self.Base = ExpandedImageBox()
        self.Base.SetParent(self)
        self.Base.SetPosition(self.BORDER_SIZE, self.BORDER_SIZE)
        self.Base.LoadImage(basePath % "base")
        self.Base.Show()

    def SetSize(self, width, height):
        minSize = self.BORDER_SIZE * 2 + self.BASE_SIZE
        width = max(minSize, width)
        height = max(minSize, height)
        Window.SetSize(self, width, height)

        scaleH = float(width - self.BORDER_SIZE * 2 - self.BORDER_SIZE) / float(
            self.BORDER_SIZE
        )
        scaleV = float(height - self.BORDER_SIZE) / float(self.BORDER_SIZE)
        self.Lines[self.L].SetRenderingRect(0, 0, 0, scaleV)
        self.Lines[self.R].SetRenderingRect(0, 0, 0, scaleV)
        self.Lines[self.T].SetRenderingRect(0, 0, scaleH, 0)
        self.Lines[self.B].SetRenderingRect(0, 0, scaleH, 0)
        self.Lines[self.R].SetPosition(
            width - self.BORDER_SIZE, self.Lines[self.R].GetTop()
        )
        self.Lines[self.B].SetPosition(
            self.Lines[self.B].GetLeft(), height - self.BORDER_SIZE
        )

        scaleH = float(width - self.BORDER_SIZE * 2 - self.BASE_SIZE) / float(
            self.BASE_SIZE
        )
        scaleV = float(height - self.BORDER_SIZE * 2 - self.BASE_SIZE) / float(
            self.BASE_SIZE
        )
        self.Base.SetRenderingRect(0, 0, scaleH, scaleV)

    def SetAlpha(self, alpha):
        for line in self.Lines:
            line.SetAlpha(alpha)
        self.Base.SetAlpha(alpha)

    def SetEvent(self, event):
        self.onClickEvent = MakeEvent(event)

    def OnMouseLeftButtonDown(self):
        if self.onClickEvent:
            return self.onClickEvent()
        return False


class CheckBox(Window):
    IMAGE_NORMAL = "d:/ymir work/ui/game/windows/box_uncheck.dds"
    IMAGE_CHECKED = "d:/ymir work/ui/game/windows/box_checked.dds"

    STATE_UNSELECTED = 0
    STATE_SELECTED = 1

    def __init__(self):
        Window.__init__(self)

        self.state = self.STATE_UNSELECTED
        self.event = None
        self.args = None

        image = ImageBox()
        image.SetParent(self)
        image.LoadImage(self.IMAGE_NORMAL)
        image.OnMouseLeftButtonDown = MakeCallback(self.OnMouseLeftButtonDown)
        image.Show()
        self.image = image

        textLine = TextLine()
        textLine.SetParent(self)
        textLine.SetPosition(image.GetRight() + 5, -3)
        textLine.SetVerticalAlignCenter()
        textLine.Show()
        self.textLine = textLine

        self.SetSize(image.GetWidth(), image.GetHeight())

    def SetText(self, text):
        self.textLine.SetText(text)

        self.SetSize(
            self.textLine.GetLeft() + self.textLine.GetWidth(), self.GetHeight()
        )

    def IsChecked(self):
        return self.state == self.STATE_SELECTED

    def SetChecked(self, val):
        self.state = val

    def __SetState(self, state):
        self.SetState(state)

        if self.event:
            self.event()

    def SetState(self, state):
        self.state = state
        self.RefreshImage()

    def RefreshImage(self):
        if self.IsChecked():
            self.image.LoadImage(self.IMAGE_CHECKED)
        else:
            self.image.LoadImage(self.IMAGE_NORMAL)

    def OnMouseLeftButtonDown(self):
        if self.state == self.STATE_UNSELECTED:
            state = self.STATE_SELECTED
        else:
            state = self.STATE_UNSELECTED
        self.__SetState(state)
        return True

    def SetEvent(self, event):
        self.event = MakeEvent(event)


class ExpandedCheckBox(Window):
    IMAGE_NORMAL = "d:/ymir work/ui/game/windows/box_uncheck.dds"
    IMAGE_CHECKED = "d:/ymir work/ui/game/windows/box_checked.dds"

    STATE_UNSELECTED = 0
    STATE_SELECTED = 1

    def __init__(self):
        Window.__init__(self)

        self.state = self.STATE_UNSELECTED
        self.event = None
        self.args = None
        self.bindStateFuncGet = None
        self.bindStateFuncSet = None
        self.boundStateFunc = False
        self.boundStateNegated = False

        image = ImageBox()
        image.SetParent(self)
        image.LoadImage(self.IMAGE_NORMAL)
        image.OnMouseLeftButtonDown = MakeCallback(self.OnMouseLeftButtonDown)
        image.Show()
        self.image = image

        textLine = TextLine()
        textLine.SetParent(self)
        textLine.SetPosition(image.GetRight() + 5, -3)
        textLine.SetVerticalAlignCenter()
        textLine.Show()
        self.textLine = textLine

        self.SetSize(image.GetWidth(), image.GetHeight())

    def SetText(self, text):
        self.textLine.SetText(text)

        self.SetSize(
            self.textLine.GetLeft() + self.textLine.GetWidth(), self.GetHeight()
        )

    def IsChecked(self):
        return self.state == self.STATE_SELECTED

    def SetChecked(self, val):
        self.state = val
        self.__SetState(val)

    def __SetState(self, state):
        self.SetState(state)

        if self.event:
            self.event(state)

    def SetState(self, state):
        self.state = state
        self.RefreshImage()

    def BindStateFunc(self, funcGet, funcSet, negated=False):
        # No need for a weak reference here as the set and get functions are builtin
        self.bindStateFuncGet = funcGet
        self.bindStateFuncSet = funcSet
        self.boundStateFunc = True
        self.boundStateNegated = negated

        if self.boundStateNegated:
            self.SetState(not self.bindStateFuncGet())
        else:
            self.SetState(self.bindStateFuncGet())

    def RefreshImage(self):
        if self.IsChecked():
            self.image.LoadImage(self.IMAGE_CHECKED)
        else:
            self.image.LoadImage(self.IMAGE_NORMAL)

    def OnMouseLeftButtonDown(self):
        if self.boundStateFunc:
            if self.boundStateNegated:
                self.bindStateFuncSet(not self.bindStateFuncGet())
            else:
                self.bindStateFuncSet(not self.bindStateFuncGet())

            if self.boundStateNegated:
                self.SetState(not self.bindStateFuncGet())
            else:
                self.SetState(self.bindStateFuncGet())

            return True

        if self.state == self.STATE_UNSELECTED:
            state = self.STATE_SELECTED
        else:
            state = self.STATE_UNSELECTED
        self.__SetState(state)
        return True

    def SetEvent(self, event):
        self.event = MakeEvent(event)


# Simple text-only tooltip


class TextToolTip(Window):
    def __init__(self):
        Window.__init__(self, "TOP_MOST")

        self.SetWindowName("TextToolTip")

        self.textLine = TextLine()
        self.textLine.SetParent(self)
        self.textLine.SetOutline()
        self.textLine.Show()

    def SetText(self, text):
        self.textLine.SetText(text)

        # grab updated bounds from CTextLine
        self.SetSize(self.textLine.GetWidth(), self.textLine.GetHeight())

    def SetPackedFontColor(self, packedColor):
        self.textLine.SetPackedFontColor(packedColor)

    def OnUpdate(self):
        x = 0
        y = 0

        width = self.textLine.GetWidth()
        height = self.textLine.GetHeight()

        (mouseX, mouseY) = wndMgr.GetMousePosition()

        if mouseY < height + 5:
            y = mouseY + height + 5
        else:
            y = mouseY - height - 5

        x = mouseX - width / 2

        x = max(x, 0)
        y = max(y, 0)
        x = min(x + width / 2, wndMgr.GetScreenWidth() - width / 2) - width / 2
        y = min(y + self.GetHeight(), wndMgr.GetScreenHeight()) - self.GetHeight()

        self.SetPosition(x, y)


class TabButton(RadioButton):
    def __init__(self):
        super(TabButton, self).__init__()
        self.onTabClickEvent = None
        self.index = 0

    def SetIndex(self, index):
        self.index = index

    def SetOnTabClickEvent(self, event):
        self.onTabClickEvent = MakeEvent(event)

    def CallEvent(self):
        snd.PlaySound("sound/ui/click.wav")

        if self.onTabClickEvent:
            self.onTabClickEvent(self.index)


class TabControl(Window):
    ORIENTATION_HORIZONTAL = 0
    ORIENTATION_VERTICAL = 1

    def __init__(self):
        super(TabControl, self).__init__()
        self.tabButtons = []
        self.buttonGroup = RadioButtonGroup()
        self.orientation = self.ORIENTATION_VERTICAL
        self.currentTabIndex = 0
        self.tabChangeEvent = None
        self.currentX = 0
        self.currentY = 0

    def AddTab(self, index, text):
        newButton = TabButton()
        newButton.SetUpVisual("d:/ymir work/ui/gui/normal_button.sub")
        newButton.SetDownVisual("d:/ymir work/ui/gui/normal_button_down.sub")
        newButton.SetOverVisual("d:/ymir work/ui/gui/normal_button_hover.sub")
        newButton.SetUpVisual()
        newButton.SetText(text)
        newButton.SetIndex(index)
        newButton.SetParent(self)
        self.tabButtons.append(newButton)
        self.buttonGroup.AddButton(newButton, self.OnTabChange, None)

        self.currentX += newButton.GetHeight()

        if self.orientation == self.ORIENTATION_VERTICAL:
            newButton.SetPosition(self.currentX, 0)

    def SetTabIndexChangeEvent(self, event):
        self.tabChangeEvent = MakeEvent(event)

    def OnTabChange(self, index):
        if self.tabChangeEvent:
            self.tabChangeEvent()


class RadioButtonWithText(RadioButton):
    def __init__(self):
        super(RadioButtonWithText, self).__init__()

        self.text = TextLine()
        self.text.SetHorizontalAlignRight()
        self.text.SetParent(self)
        self.text.SetPosition(-6, -3)
        self.text.SetFontColor(1.0, 1.0, 1.0, 1.0)
        self.text.SetFontName(localeInfo.UI_DEF_FONT_NUNITO_BOLD)
        self.text.Show()
        self.text.SetTop()

    def AdjustSize(self):
        self.SetSize(self.GetWidth() + self.text.GetWidth(), self.GetHeight())

    def SetText(self, text):
        self.text.SetText(text)
        self.AdjustSize()


###################################################################################################
## Python Script Loader
###################################################################################################


class ScriptWindow(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        # Top-Level-Windows are always focusable
        self.AddFlag("focusable")

        self.Children = []
        self.ElementDictionary = {}
        self.TypeDict = {}

    def Show(self):
        Window.Show(self)

        for children in self.GetChildrenByType("titlebar"):
            children.SetTop()

    def ClearDictionary(self):
        self.Children = []
        self.ElementDictionary = {}
        self.TypeDict = {}

    def InsertChild(self, name, child, Type):
        self.ElementDictionary[name] = child
        if Type in self.TypeDict:
            self.TypeDict[Type].append(child)
        else:
            self.TypeDict[Type] = [child]

    def IsChild(self, name):
        return name in self.ElementDictionary

    def GetChild(self, name):
        return self.ElementDictionary[name]

    def GetChildrenByType(self, Type):
        return self.TypeDict.get(Type, [])

    def GetOptionalChild(self, name):
        return self.ElementDictionary.get(name, None)

    def SetSize(self, width, height):
        Window.SetSize(self, width, height)


class ChatFilterCheckBoxLanguage(ImageBox):
    STATE_UNSELECTED = 0
    STATE_SELECTED = 1

    def __init__(self, parent, bLanguage, x, y, event):
        ImageBox.__init__(self)
        self.SetParent(parent)
        self.SetPosition(x, y)
        self.LoadImage("d:/ymir work/ui/public/Parameter_Slot_02.sub")
        self.state = self.STATE_UNSELECTED
        self.event = None

        import uiGuild

        self.mouse = uiGuild.MouseReflector(self)
        self.mouse.SetSize(self.GetWidth(), self.GetHeight())

        self.AttributeLocked = app.GetLocaleID() == bLanguage

        image = MakeImageBox(self, "d:/ymir work/ui/public/check_image.sub", -7, 0)
        image.AddFlag("not_pick")
        image.SetHorizontalAlignCenter()
        image.SetVerticalAlignCenter()
        image.Hide()
        self.event = MakeEvent(event)
        self.image, self.bLanguage, self.state = image, bLanguage, 0

        if self.AttributeLocked:
            self.Select()

        self.Show()
        self.mouse.UpdateRect()

    def GetLanguageIndex(self):
        return self.bLanguage

    def IsChecked(self):
        return self.state == self.STATE_SELECTED

    def __SetState(self, state):
        self.SetState(state)

        if self.event:
            self.event(state)

    def SetState(self, state):
        self.state = state
        if state:
            self.image.Show()
        else:
            self.image.Hide()

    def UnSelect(self):
        if not self.AttributeLocked:
            self.SetState(self.STATE_UNSELECTED)

    def Select(self):
        self.SetState(self.STATE_SELECTED)

    def OnMouseOverIn(self):
        self.mouse.Show()

    def OnMouseOverOut(self):
        self.mouse.Hide()

    def OnMouseLeftButtonDown(self):
        if self.state == self.STATE_UNSELECTED:
            state = self.STATE_SELECTED
        else:
            state = self.STATE_UNSELECTED
        self.__SetState(state)
        self.mouse.Down()
        return True

    def OnMouseLeftButtonUp(self):
        self.mouse.Up()


class PythonScriptLoader(object):
    BODY_KEY_LIST = ("x", "y", "width", "height")

    #####

    DEFAULT_KEY_LIST = (
        "type",
        "x",
        "y",
    )
    WINDOW_KEY_LIST = (
        "width",
        "height",
    )
    IMAGE_KEY_LIST = ("image",)
    EXPANDED_IMAGE_KEY_LIST = ("image",)
    ANI_IMAGE_KEY_LIST = ("images",)
    SLOT_KEY_LIST = (
        "width",
        "height",
        "slot",
    )
    CANDIDATE_LIST_KEY_LIST = (
        "item_step",
        "item_xsize",
        "item_ysize",
    )
    GRID_TABLE_KEY_LIST = (
        "start_index",
        "x_count",
        "y_count",
        "x_step",
        "y_step",
    )
    EDIT_LINE_KEY_LIST = (
        "width",
        "height",
        "input_limit",
    )
    COMBO_BOX_KEY_LIST = (
        "width",
        "height",
        "item",
    )
    TITLE_BAR_KEY_LIST = ("width",)
    SUB_TITLE_BAR_KEY_LIST = ("width",)
    LIST_BAR_KEY_LIST = ("width",)
    HORIZONTAL_BAR_KEY_LIST = ("width",)
    BOARD_KEY_LIST = (
        "width",
        "height",
    )
    BOARD_WITH_TITLEBAR_KEY_LIST = (
        "width",
        "height",
        "title",
    )
    BOX_KEY_LIST = (
        "width",
        "height",
    )
    BAR_KEY_LIST = (
        "width",
        "height",
    )
    LINE_KEY_LIST = (
        "width",
        "height",
    )
    SLOTBAR_KEY_LIST = (
        "width",
        "height",
    )
    GAUGE_KEY_LIST = (
        "width",
        "color",
    )
    SCROLLBAR_KEY_LIST = ("size",)
    LIST_BOX_KEY_LIST = (
        "width",
        "height",
    )
    COMBO_BOX_KEY_LIST = ("width", "height")
    if app.ENABLE_MYSHOP_DECO:
        RENDER_TARGET_KEY_LIST = ("index",)
    TABLE_KEY_LIST = ("width",)
    INPUT_FIELD_KEY_LIST = (
        "width",
        "height",
    )
    INPUT_KEY_LIST = ("width",)
    NEWGAUGE_KEY_LIST = (
        "width",
        "color",
    )
    VIDEO_FRAME_KEY_LIST = ("video_file",)

    def __init__(self):
        self.Clear()

    def Clear(self):
        self.ScriptDictionary = {
            "SCREEN_WIDTH": wndMgr.GetScreenWidth(),
            "SCREEN_HEIGHT": wndMgr.GetScreenHeight(),
        }
        self.InsertFunction = 0

    def LoadScriptFile(self, window, FileName):
        self.Clear()

        logging.debug("Loading UIScript: %s", FileName)

        from utils import Sandbox

        sandbox = Sandbox(
            True, ["uiScriptLocale", "localeInfo", "sys", "item", "app", "grp"]
        )

        self.ScriptDictionary["PLAYER_NAME_MAX_LEN"] = PLAYER_NAME_MAX_LEN
        self.ScriptDictionary["DRAGON_SOUL_EQUIPMENT_SLOT_START"] = (
            player.DRAGON_SOUL_EQUIPMENT_SLOT_START
        )
        self.ScriptDictionary["EQUIPMENT_SLOT_START"] = player.EQUIPMENT_SLOT_START
        self.ScriptDictionary["CHARACTER_NAME_MAX_LEN"] = player.CHARACTER_NAME_MAX_LEN
        self.ScriptDictionary["LOCALE_PATH"] = app.GetLocalePath()

        try:
            sandbox.execfile(FileName, self.ScriptDictionary)
        except:
            logging.exception("Failed to load script file: %s", FileName)
            return

        #####

        Body = self.ScriptDictionary["window"]
        self.CheckKeyList("window", Body, self.BODY_KEY_LIST)

        window.ClearDictionary()
        self.InsertFunction = window.InsertChild

        window.SetPosition(int(Body["x"]), int(Body["y"]))
        window.SetSize(int(Body["width"]), int(Body["height"]))
        if "style" in Body:
            for StyleList in Body["style"]:
                window.AddFlag(StyleList)

        self.LoadChildren(window, Body)

    def LoadChildren(self, parent, dicChildren):

        if localeInfo.IsARABIC():
            parent.AddFlag("rtl")

        if "style" in dicChildren:
            for style in dicChildren["style"]:
                parent.AddFlag(style)

        if "children" not in dicChildren:
            return False

        Index = 0

        ChildrenList = dicChildren["children"]
        parent.Children = range(len(ChildrenList))

        for ElementValue in ChildrenList:
            try:
                Name = ElementValue["name"]
            except KeyError:
                Name = ElementValue["name"] = "NONAME"
            except TypeError:
                logging.error("Unnamed window found! {}".format(ElementValue))

            try:
                Type = ElementValue["type"]
            except KeyError:
                Type = ElementValue["type"] = "window"

            if not self.CheckKeyList(Name, ElementValue, self.DEFAULT_KEY_LIST):
                del parent.Children[Index]
                continue

            if Type == "window":
                parent.Children[Index] = Window()
                parent.Children[Index].SetParent(parent)
                self.LoadElementWindow(parent.Children[Index], ElementValue, parent)

            elif Type == "auto_grow_vertical":
                parent.Children[Index] = AutoGrowingVerticalContainer()
                parent.Children[Index].SetParent(parent)
                self.LoadElementWindow(parent.Children[Index], ElementValue, parent)
            elif Type == "auto_grow_vertical_ex":
                from ui_modern_controls import AutoGrowingVerticalContainerEx

                parent.Children[Index] = AutoGrowingVerticalContainerEx()
                parent.Children[Index].SetParent(parent)
                self.LoadElementWindow(parent.Children[Index], ElementValue, parent)
            elif Type == "scrollwindow":
                parent.Children[Index] = ScrollWindow()
                parent.Children[Index].SetParent(parent)
                self.LoadElementWindow(parent.Children[Index], ElementValue, parent)

            elif Type == "button":
                parent.Children[Index] = Button()
                parent.Children[Index].SetParent(parent)
                self.LoadElementButton(parent.Children[Index], ElementValue, parent)

            elif Type == "radio_button":
                parent.Children[Index] = RadioButton()
                parent.Children[Index].SetParent(parent)
                self.LoadElementButton(parent.Children[Index], ElementValue, parent)

            elif Type == "radio_button_with_text":
                parent.Children[Index] = RadioButtonWithText()
                parent.Children[Index].SetParent(parent)
                self.LoadElementButton(parent.Children[Index], ElementValue, parent)

            elif Type == "toggle_button":
                parent.Children[Index] = ToggleButton()
                parent.Children[Index].SetParent(parent)
                self.LoadElementButton(parent.Children[Index], ElementValue, parent)

            elif Type == "mark":
                parent.Children[Index] = MarkBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementMark(parent.Children[Index], ElementValue, parent)

            elif Type == "image":
                parent.Children[Index] = ImageBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementImage(parent.Children[Index], ElementValue, parent)

            elif Type == "expanded_image":
                parent.Children[Index] = ExpandedImageBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementExpandedImage(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "ani_image":
                parent.Children[Index] = AniImageBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementAniImage(parent.Children[Index], ElementValue, parent)

            elif Type == "slot":
                parent.Children[Index] = SlotWindow()
                parent.Children[Index].SetParent(parent)
                self.LoadElementSlot(parent.Children[Index], ElementValue, parent)

            elif Type == "grid_table":
                parent.Children[Index] = GridSlotWindow()
                parent.Children[Index].SetParent(parent)
                self.LoadElementGridTable(parent.Children[Index], ElementValue, parent)

            elif Type == "text":
                parent.Children[Index] = TextLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementText(parent.Children[Index], ElementValue, parent)

            elif Type == "limit_text":
                parent.Children[Index] = LimitedTextLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementText(parent.Children[Index], ElementValue, parent)

            elif Type == "extended_text":
                parent.Children[Index] = ExtendedTextLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementExtendedText(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "editline":
                parent.Children[Index] = EditLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementEditLine(parent.Children[Index], ElementValue, parent)

            elif Type == "multi_text":
                parent.Children[Index] = MultiTextLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementMultiText(parent.Children[Index], ElementValue, parent)

            elif Type == "titlebar":
                parent.Children[Index] = TitleBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementTitleBar(parent.Children[Index], ElementValue, parent)

            elif Type == "single_titlebar":
                parent.Children[Index] = SingleImageTitleBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementSingleImageTitleBar(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "subtitlebar":
                parent.Children[Index] = SubTitleBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementSubTitleBar(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "listbar":
                parent.Children[Index] = ListBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementListBar(parent.Children[Index], ElementValue, parent)

            elif Type == "horizontalbar":
                parent.Children[Index] = HorizontalBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementHorizontalBar(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "verticalbar":
                parent.Children[Index] = VerticalBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementVerticalBar(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "board":
                parent.Children[Index] = Board()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "board2":
                parent.Children[Index] = Board2()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBoard2(parent.Children[Index], ElementValue, parent)

            elif Type == "board_with_titlebar":
                parent.Children[Index] = BoardWithTitleBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBoardWithTitleBar(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "thinboard":
                parent.Children[Index] = ThinBoard()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "small_dark_thinboard":
                parent.Children[Index] = SmallDarkThinBoard()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "thinboard_red":
                parent.Children[Index] = ThinBoardRed()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "thinboard_darkred":
                parent.Children[Index] = ThinBoardDarkRed()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "small_thinboard_red":
                parent.Children[Index] = SmallThinBoardDark()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "thinboard_redbg":
                parent.Children[Index] = ThinBoardRedBg()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "thinboard_gold":
                parent.Children[Index] = ThinBoardGold()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoardGold(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "thinboard_circle":
                parent.Children[Index] = ThinBoardCircle()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoardCircle(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "thinboard_old":
                parent.Children[Index] = ThinBoardOld()
                parent.Children[Index].SetParent(parent)
                self.LoadElementThinBoard(parent.Children[Index], ElementValue, parent)

            elif Type == "box":
                parent.Children[Index] = Box()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBox(parent.Children[Index], ElementValue, parent)

            elif Type == "bar":
                parent.Children[Index] = Bar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBar(parent.Children[Index], ElementValue, parent)

            elif Type == "line":
                parent.Children[Index] = Line()
                parent.Children[Index].SetParent(parent)
                self.LoadElementLine(parent.Children[Index], ElementValue, parent)

            elif Type == "slotbar":
                parent.Children[Index] = SlotBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementSlotBar(parent.Children[Index], ElementValue, parent)

            elif Type == "gauge":
                parent.Children[Index] = Gauge()
                parent.Children[Index].SetParent(parent)
                self.LoadElementGauge(parent.Children[Index], ElementValue, parent)

            elif Type == "newgauge":
                parent.Children[Index] = NewGauge()
                parent.Children[Index].SetParent(parent)
                self.LoadElementNewGauge(parent.Children[Index], ElementValue, parent)

            elif Type == "scrollbar":
                parent.Children[Index] = ScrollBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementScrollBar(parent.Children[Index], ElementValue, parent)

            elif Type == "thin_scrollbar":
                parent.Children[Index] = ThinScrollBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementScrollBar(parent.Children[Index], ElementValue, parent)

            elif Type == "small_thin_scrollbar":
                parent.Children[Index] = SmallThinScrollBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementScrollBar(parent.Children[Index], ElementValue, parent)

            elif Type == "input":
                parent.Children[Index] = Input()
                parent.Children[Index].SetParent(parent)
                self.LoadElementInput(parent.Children[Index], ElementValue, parent)

            elif Type == "sliderbar":
                parent.Children[Index] = SliderBar()
                parent.Children[Index].SetParent(parent)
                self.LoadElementSliderBar(parent.Children[Index], ElementValue, parent)

            elif Type == "listbox":
                parent.Children[Index] = ListBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementListBox(parent.Children[Index], ElementValue, parent)

            elif Type == "listbox2":
                parent.Children[Index] = ListBox2()
                parent.Children[Index].SetParent(parent)
                self.LoadElementListBox2(parent.Children[Index], ElementValue, parent)
            elif Type == "listboxex":
                parent.Children[Index] = ListBoxEx()
                parent.Children[Index].SetParent(parent)
                self.LoadElementListBoxEx(parent.Children[Index], ElementValue, parent)
            elif Type == "border_a":
                parent.Children[Index] = BorderA()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBoard(parent.Children[Index], ElementValue, parent)
            elif Type == "border_c":
                parent.Children[Index] = BorderC()
                parent.Children[Index].SetParent(parent)
                self.LoadElementBoard(parent.Children[Index], ElementValue, parent)
            elif Type == "combo_box":
                parent.Children[Index] = ComboBoxNew()
                parent.Children[Index].SetParent(parent)
                self.LoadElementComboBox(parent.Children[Index], ElementValue, parent)

            elif Type == "numberline":
                parent.Children[Index] = NumberLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementNumberLine(parent.Children[Index], ElementValue, parent)
            elif Type == "render_target":
                parent.Children[Index] = RenderTarget()
                parent.Children[Index].SetParent(parent)
                self.LoadElementRenderTarget(
                    parent.Children[Index], ElementValue, parent
                )
            elif Type == "item_render_target":
                parent.Children[Index] = ItemRenderTarget()
                parent.Children[Index].SetParent(parent)
                self.LoadElementRenderTarget(
                    parent.Children[Index], ElementValue, parent
                )
            elif Type == "video_frame":
                parent.Children[Index] = VideoFrame()
                parent.Children[Index].SetParent(parent)
                self.LoadElementVideoFrame(parent.Children[Index], ElementValue, parent)
            elif Type == "color_picker":
                parent.Children[Index] = ColorPicker()
                parent.Children[Index].SetParent(parent)
                self.LoadElementColorPicker(
                    parent.Children[Index], ElementValue, parent
                )
            elif Type == "web_view":
                parent.Children[Index] = WebView()
                parent.Children[Index].SetParent(parent)
                self.LoadElementWebView(parent.Children[Index], ElementValue, parent)
            elif Type == "table":
                parent.Children[Index] = Table()
                parent.Children[Index].SetParent(parent)
                self.LoadElementTable(parent.Children[Index], ElementValue, parent)

            elif Type == "updownbutton":
                parent.Children[Index] = UpDownButton()
                parent.Children[Index].SetParent(parent)
                self.LoadElementUpDownButton(
                    parent.Children[Index], ElementValue, parent
                )

            elif Type == "field":
                parent.Children[Index] = InputField()
                parent.Children[Index].SetParent(parent)
                self.LoadElementInputField(parent.Children[Index], ElementValue, parent)

            elif Type == "checkbox":
                parent.Children[Index] = CheckBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementCheckBox(parent.Children[Index], ElementValue, parent)

            elif Type == "checkbox_with_text":
                parent.Children[Index] = ExpandedCheckBox()
                parent.Children[Index].SetParent(parent)
                self.LoadElementCheckBox(parent.Children[Index], ElementValue, parent)

            elif Type == "number":
                parent.Children[Index] = NumberLine()
                parent.Children[Index].SetParent(parent)
                self.LoadElementNumberLine(parent.Children[Index], ElementValue, parent)

            else:
                logging.debug("Unknown Type %s", Type)
                Index += 1
                continue

            parent.Children[Index].SetWindowName(Name)
            if 0 != self.InsertFunction:
                self.InsertFunction(Name, parent.Children[Index], Type)

            self.LoadChildren(parent.Children[Index], ElementValue)
            Index += 1

    def CheckKeyList(self, name, value, key_list):

        for DataKey in key_list:
            if not DataKey in value:
                print("Failed to find data key", "[" + name + "/" + DataKey + "]")
                return False

        return True

    def LoadDefaultData(self, window, value, parentWindow):
        loc_x = int(value["x"])
        loc_y = int(value["y"])
        if "vertical_align" in value:
            if "center" == value["vertical_align"]:
                window.SetVerticalAlignCenter()
            elif "bottom" == value["vertical_align"]:
                window.SetVerticalAlignBottom()

        if parentWindow.IsRTL():
            loc_x = int(value["x"]) + window.GetWidth()
            if "horizontal_align" in value:
                if "center" == value["horizontal_align"]:
                    window.SetHorizontalAlignCenter()
                    loc_x = -int(value["x"])
                elif "right" == value["horizontal_align"]:
                    window.SetHorizontalAlignLeft()
                    loc_x = int(value["x"]) - window.GetWidth()
                    ## loc_x = parentWindow.GetWidth() - int(value["x"]) + window.GetWidth()
            else:
                window.SetHorizontalAlignRight()

            if "all_align" in value:
                window.SetVerticalAlignCenter()
                window.SetHorizontalAlignCenter()
                loc_x = -int(value["x"])
        else:
            if "horizontal_align" in value:
                if "center" == value["horizontal_align"]:
                    window.SetHorizontalAlignCenter()
                elif "right" == value["horizontal_align"]:
                    window.SetHorizontalAlignRight()

        window.SetPosition(loc_x, loc_y)
        isHidden = False
        if "hidden" in value:
            isHidden = value["hidden"]

        if not isHidden:
            window.Show()

        if "content_window" in value:
            # TODO: Maybe we should check the window type (ScrollWindow)
            parentWindow.SetContentWindow(window)
            parentWindow.AutoFitWidth()
            parentWindow.OnContentWindowSizeChange()

    ## Window
    def LoadElementWindow(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.WINDOW_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    def LoadElementInput(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.INPUT_KEY_LIST):
            return False

        if value.has_key("enable_codepage"):
            window.SetIMEFlag(value["enable_codepage"])

        window.SetHeight(int(value["height"]))
        window.SetWidth(int(value["width"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Button
    def LoadElementButton(self, window, value, parentWindow):
        if "width" in value and "height" in value:
            window.SetSize(int(value["width"]), int(value["height"]))

        if "default_image" in value:
            window.SetUpVisual(value["default_image"])
        if "over_image" in value:
            window.SetOverVisual(value["over_image"])
        if "down_image" in value:
            window.SetDownVisual(value["down_image"])
        if "disable_image" in value:
            window.SetDisableVisual(value["disable_image"])

        if "text" in value:
            if "text_height" in value:
                window.SetText(value["text"], value["text_height"])
            else:
                window.SetText(value["text"])

        if "text_left" in value:
            if "text_height" in value:
                window.SetTextAlignLeft(value["text_left"], value["text_height"])
            else:
                window.SetTextAlignLeft(value["text_left"])

        if "text_auto" in value:
            window.SetAutoSizeText(value["text_auto"])

        if "text_color" in value:
            window.SetTextColor(value["text_color"])

        if "tooltip_text" in value:
            if "tooltip_x" in value and "tooltip_y" in value:
                window.SetToolTipText(
                    value["tooltip_text"],
                    int(value["tooltip_x"]),
                    int(value["tooltip_y"]),
                )
            else:
                window.SetToolTipText(value["tooltip_text"])

        if "text_position" in value:
            window.SetTextPosition(*value["text_position"])

        if "extended_data" in value:
            window.SetExtendedData(value["extended_data"])

        if "fontname" in value:
            window.SetFontName(value["fontname"])

        if "outline" in value and value["outline"]:
            window.SetFontOutline()

        if "x_scale" in value and "y_scale" in value:
            window.SetButtonScale(float(value["x_scale"]), float(value["y_scale"]))

        if "r" in value and "g" in value and "b" in value:
            window.SetPackedFontColor(
                grp.GenerateColor(
                    float(value["r"]), float(value["g"]), float(value["b"])
                )
            )
        elif "color" in value:
            if isinstance(value["color"], tuple):
                window.SetPackedFontGradient(value["color"][0], value["color"][1])
            else:
                window.SetPackedFontColor(value["color"])

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Mark
    def LoadElementMark(self, window, value, parentWindow):

        # if not self.CheckKeyList(value["name"], value, self.MARK_KEY_LIST):
        #    return False

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Image
    def LoadElementImage(self, window, value, parentWindow):

        if "image" in value:
            window.LoadImage(value["image"])
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## AniImage
    def LoadElementAniImage(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.ANI_IMAGE_KEY_LIST):
            return False

        if "delay" in value:
            window.SetDelay(value["delay"])

        for image in value["images"]:
            window.AppendImage(image)

        if "width" in value and "height" in value:
            window.SetSize(value["width"], value["height"])

        if "x_scale" in value and "y_scale" in value:
            window.SetScale(float(value["x_scale"]), float(value["y_scale"]))

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Expanded Image
    def LoadElementExpandedImage(self, window, value, parentWindow):

        if "image" in value:
            window.LoadImage(value["image"])

        if "x_origin" in value and "y_origin" in value:
            window.SetOrigin(float(value["x_origin"]), float(value["y_origin"]))

        if "x_scale" in value and "y_scale" in value:
            window.SetScale(float(value["x_scale"]), float(value["y_scale"]))

        if "rect" in value:
            RenderingRect = value["rect"]
            window.SetRenderingRect(
                RenderingRect[0], RenderingRect[1], RenderingRect[2], RenderingRect[3]
            )

        if "mode" in value:
            mode = value["mode"]

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Slot
    def LoadElementSlot(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.SLOT_KEY_LIST):
            return False

        global_x = int(value["x"])
        global_y = int(value["y"])
        global_width = int(value["width"])
        global_height = int(value["height"])

        window.SetPosition(global_x, global_y)
        window.SetSize(global_width, global_height)
        window.Show()

        r = 1.0
        g = 1.0
        b = 1.0
        a = 1.0

        if (
            "image_r" in value
            and "image_g" in value
            and "image_b" in value
            and "image_a" in value
        ):
            r = float(value["image_r"])
            g = float(value["image_g"])
            b = float(value["image_b"])
            a = float(value["image_a"])

        SLOT_ONE_KEY_LIST = ("index", "x", "y", "width", "height")

        for slot in value["slot"]:
            if self.CheckKeyList(value["name"] + " - one", slot, SLOT_ONE_KEY_LIST):
                placementX = 0
                placementY = 0
                if "placement_x" in slot:
                    placementX = int(slot["placement_x"])
                if "placement_y" in slot:
                    placementY = int(slot["placement_y"])

                wndMgr.AppendSlot(
                    window.hWnd,
                    int(slot["index"]),
                    int(slot["x"]),
                    int(slot["y"]),
                    int(slot["width"]),
                    int(slot["height"]),
                    placementX,
                    placementY,
                )
                window.slotList.append(int(slot["index"]))

        if "x_scale" in value and "y_scale" in value:
            window.SetSlotScale(float(value["x_scale"]), float(value["y_scale"]))

        if "image" in value:
            wndMgr.SetSlotBaseImage(window.hWnd, value["image"], r, g, b, a)

        return True

    def LoadElementCandidateList(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.CANDIDATE_LIST_KEY_LIST):
            return False

        window.SetPosition(int(value["x"]), int(value["y"]))
        window.SetItemSize(int(value["item_xsize"]), int(value["item_ysize"]))
        window.SetItemStep(int(value["item_step"]))
        window.Show()

        return True

    ## Table
    def LoadElementGridTable(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.GRID_TABLE_KEY_LIST):
            return False

        xBlank = 0
        yBlank = 0
        if "x_blank" in value:
            xBlank = int(value["x_blank"])
        if "y_blank" in value:
            yBlank = int(value["y_blank"])

        if localeInfo.IsARABIC():
            pass
        else:
            window.SetPosition(int(value["x"]), int(value["y"]))

        window.ArrangeSlot(
            int(value["start_index"]),
            int(value["x_count"]),
            int(value["y_count"]),
            int(value["x_step"]),
            int(value["y_step"]),
            xBlank,
            yBlank,
        )
        if "image" in value:
            r = 1.0
            g = 1.0
            b = 1.0
            a = 1.0
            if (
                "image_r" in value
                and "image_g" in value
                and "image_b" in value
                and "image_a" in value
            ):
                r = float(value["image_r"])
                g = float(value["image_g"])
                b = float(value["image_b"])
                a = float(value["image_a"])
            wndMgr.SetSlotBaseImage(window.hWnd, value["image"], r, g, b, a)

        if "style" in value:
            if "select" == value["style"]:
                wndMgr.SetSlotStyle(window.hWnd, wndMgr.SLOT_STYLE_SELECT)
        if localeInfo.IsARABIC():
            self.LoadDefaultData(window, value, parentWindow)
        else:
            window.Show()

        return True

    ## Text
    def LoadElementText(self, window, value, parentWindow):
        if "fontsize" in value:
            fontSize = value["fontsize"]
            if "LARGE" == fontSize:
                window.SetFontName(localeInfo.UI_DEF_FONT_LARGE)
        elif "fontname" in value:
            fontName = value["fontname"]
            window.SetFontName(fontName)

        if "all_align" in value:
            window.SetHorizontalAlignCenter()
            window.SetVerticalAlignCenter()

        if "r" in value and "g" in value and "b" in value:
            window.SetFontColor(float(value["r"]), float(value["g"]), float(value["b"]))
        elif "color" in value:
            if isinstance(value["color"], tuple):
                window.SetPackedFontGradient(value["color"][0], value["color"][1])
            else:
                window.SetPackedFontColor(value["color"])
        else:
            window.SetPackedFontGradient(0xFFF3CD84, 0xFFE9DB9C)

        if "outline" in value and value["outline"]:
            window.SetOutline()

        if "multi_line" in value and value["multi_line"]:
            window.SetMultiLine()

        if "text" in value:
            window.SetText(value["text"])

        if isinstance(window, LimitedTextLine):
            if "max" in value:
                window.SetMax(value["max"])

        self.LoadDefaultData(window, value, parentWindow)
        return True

    ## ExtendedTextLine
    def LoadElementExtendedText(self, window, value, parentWindow):

        if True == "text" in value:
            window.SetText(value["text"])

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## MultiTextLine
    def LoadElementMultiText(self, window, value, parentWindow):

        if "width" in value and "height" in value:
            window.SetSize(value["width"], value["height"])
        elif "width" in value:
            window.SetWidth(value["width"])

        if "text_horizontal_align" in value:
            if "center" == value["text_horizontal_align"]:
                window.SetTextHorizontalAlignCenter()

        if "text_vertical_align" in value:
            if "center" == value["text_vertical_align"]:
                window.SetTextVerticalAlignCenter()

        if "text" in value:
            window.SetText(value["text"])

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## EditLine
    def LoadElementEditLine(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.EDIT_LINE_KEY_LIST):
            return False

        if "secret_flag" in value:
            window.SetSecret(value["secret_flag"])
        if "with_codepage" in value:
            if value["with_codepage"]:
                window.bCodePage = True
        if "only_number" in value:
            if value["only_number"]:
                window.SetNumberMode()
        if "only_hex" in value:
            if value["only_hex"]:
                window.SetHexMode()
        if "enable_codepage" in value:
            window.SetIMEFlag(value["enable_codepage"])
        if "enable_ime" in value:
            window.SetIMEFlag(value["enable_ime"])
        if "limit_width" in value:
            window.SetLimitWidth(value["limit_width"])
        if "multi_line" in value:
            if value["multi_line"]:
                window.SetMultiLine()

        fontSize = "NORMAL"
        if "fontsize" in value:
            fontSize = value["fontsize"]

        if "LARGE" == fontSize:
            window.SetFontName(localeInfo.UI_DEF_FONT_LARGE)

        if "placeholder_text" in value:
            window.SetPlaceHolderText(value["placeholder_text"])

        if "placeholder_color" in value:
            window.SetPlaceHolderTextColor(value["placeholder_color"])

        window.SetMax(int(value["input_limit"]))
        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadElementText(window, value, parentWindow)

        return True

    ## TitleBar
    def LoadElementTitleBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.TITLE_BAR_KEY_LIST):
            return False

        window.MakeTitleBar(int(value["width"]), value.get("color", "red"))

        if "title" in value:
            window.SetTitleName(value["title"])

        if "title_color" in value:
            window.SetTitleColor(value["title_color"])

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## SingleImageTitleBar
    def LoadElementSingleImageTitleBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.IMAGE_KEY_LIST):
            return False

        window.MakeTitleBar()
        window.LoadImage(value["image"])
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## SubTitleBar
    def LoadElementSubTitleBar(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.SUB_TITLE_BAR_KEY_LIST):
            return False

        window.MakeSubTitleBar(int(value["width"]), value.get("color", "red"))
        # self.LoadDefaultData(window, value, parentWindow)
        self.LoadElementButton(window, value, parentWindow)
        # test
        window.Show()
        return True

    ## ListBar
    def LoadElementListBar(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.LIST_BAR_KEY_LIST):
            return False

        window.MakeListBar(int(value["width"]), value.get("color", "red"))
        # self.LoadDefaultData(window, value, parentWindow)
        self.LoadElementButton(window, value, parentWindow)

        return True

    ## HorizontalBar
    def LoadElementHorizontalBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.HORIZONTAL_BAR_KEY_LIST):
            return False

        window.Create(int(value["width"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## VerticalBar
    def LoadElementVerticalBar(self, window, value, parentWindow):
        window.Create(int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Board
    def LoadElementBoard(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.BOARD_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Board2
    def LoadElementBoard2(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.BOARD_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Board With TitleBar
    def LoadElementBoardWithTitleBar(self, window, value, parentWindow):

        if not self.CheckKeyList(
            value["name"], value, self.BOARD_WITH_TITLEBAR_KEY_LIST
        ):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        window.SetTitleName(value["title"])
        if "close_button" in value:
            if int(value["close_button"]) == 0:
                window.DeactivateCloseButton()

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## ThinBoard
    def LoadElementThinBoard(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.BOARD_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        if isinstance(window, ThinBoard):
            if "inner_width" in value and "inner_height" in value:
                window.SetInnerThinBoardSize(
                    value["inner_height"], value["inner_width"]
                )
            elif "inner_width" in value and "inner_height" not in value:
                window.SetInnerThinBoardSize(value["inner_width"], None)
            elif "inner_width" not in value and "inner_height" in value:
                window.SetInnerThinBoardSize(None, value["inner_height"])

            if "inner_x" in value and "inner_y" in value:
                window.SetInnerThinBoardPosition(value["inner_x"], value["inner_y"])
            elif "inner_x" in value and "inner_y" not in value:
                window.SetInnerThinBoardPosition(value["inner_x"], None)
            elif "inner_x" not in value and "inner_y" in value:
                window.SetInnerThinBoardPosition(None, value["inner_y"])

        return True

    def LoadElementThinBoardGold(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.BOARD_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)
        return True

    def LoadElementThinBoardCircle(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.BOARD_KEY_LIST):
            return False

        if "alpha" in value:
            window.SetAlpha(value["alpha"])

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)
        return True

    ## Box
    def LoadElementBox(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.BOX_KEY_LIST):
            return False

        if "color" in value:
            window.SetColor(value["color"])

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Bar
    def LoadElementBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.BAR_KEY_LIST):
            return False

        if "color" in value:
            window.SetColor(value["color"])

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Line
    def LoadElementLine(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.LINE_KEY_LIST):
            return False

        if "color" in value:
            window.SetColor(value["color"])

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Slot
    def LoadElementSlotBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.SLOTBAR_KEY_LIST):
            return False

        window.SetSize(int(value["width"]), int(value["height"]))
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## Gauge
    def LoadElementGauge(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.GAUGE_KEY_LIST):
            return False

        window.MakeGauge(value["width"], value["color"])
        self.LoadDefaultData(window, value, parentWindow)

        return True

    def LoadElementNewGauge(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.NEWGAUGE_KEY_LIST):
            return False

        self.LoadDefaultData(window, value, parentWindow)
        window.MakeGauge(value["width"], value["color"])

        return True

    ## ScrollBar
    def LoadElementScrollBar(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.SCROLLBAR_KEY_LIST):
            return False

        window.SetScrollBarSize(value["size"])
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## SliderBar
    def LoadElementSliderBar(self, window, value, parentWindow):

        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## ListBox
    def LoadElementListBox(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.LIST_BOX_KEY_LIST):
            return False

        if "item_align" in value:
            window.SetTextCenterAlign(value["item_align"])

        window.SetSize(value["width"], value["height"])
        self.LoadDefaultData(window, value, parentWindow)

        return True

    ## ListBox2
    def LoadElementListBox2(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.LIST_BOX_KEY_LIST):
            return False

        window.SetRowCount(value.get("row_count", 10))  # ±âº» ¼¼·Î °³¼ö 10°³
        window.SetSize(value["width"], value["height"])
        self.LoadDefaultData(window, value, parentWindow)

        if "item_align" in value:
            window.SetTextCenterAlign(value["item_align"])

        return True

    def LoadElementComboBox(self, window, value, parentWindow):
        if not self.CheckKeyList(value["name"], value, self.COMBO_BOX_KEY_LIST):
            return False

        imageName = None

        if "image" in value:
            imageName = value["image"]

        if "viewcount" in value:
            window.SetViewItemCount(value["viewcount"])

        window.Create(parentWindow, imageName, value["x"], value["y"])
        window.SetSize(value["width"], value["height"])

        if "item_list" in value:
            for i, v in enumerate(value["item_list"]):
                window.InsertItem(i, v)
        window.Show()

        return True

    def LoadElementListBoxEx(self, window, value, parentWindow):

        if not self.CheckKeyList(value["name"], value, self.LIST_BOX_KEY_LIST):
            return False

        window.SetSize(value["width"], value["height"])
        self.LoadDefaultData(window, value, parentWindow)

        if "itemsize_x" in value and "itemsize_y" in value:
            window.SetItemSize(int(value["itemsize_x"]), int(value["itemsize_y"]))

        if "itemstep" in value:
            window.SetItemStep(int(value["itemstep"]))

        if "viewcount" in value:
            window.SetViewItemCount(int(value["viewcount"]))

        return True

    def LoadElementNumberLine(self, window, value, parentWindow):
        self.LoadDefaultData(window, value, parentWindow)
        return True

    def LoadElementTable(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.TABLE_KEY_LIST):
            return False

        if "height" in value:
            window.SetSize(value["width"], value["height"])
        else:
            window.SetWidth(value["width"])
        self.LoadDefaultData(window, value, parentWindow)

        if "col_length_check" in value:
            for index in value["col_length_check"]:
                window.AddCheckLengthIndex(index)

        if "col_size" in value:
            window.SetColSizePct(value["col_size"])

            if "header" in value:
                if "header_extra" in value:
                    window.SetHeader(value["header"], int(value["header_extra"]))
                else:
                    window.SetHeader(value["header"])

            if "content" in value:
                i = 0
                for colList in value["content"]:
                    window.Append(i, colList, False)
                    i += 1
                window.LocateLines()

        return True

    def LoadElementUpDownButton(self, window, value, parentWindow):

        if "value" in value:
            window.SetValue(value["value"])
        if "min" in value:
            window.SetMin(value["min"])
        if "max" in value:
            window.SetMax(value["max"])

        self.LoadDefaultData(window, value, parentWindow)

        return True

    def LoadElementInputField(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.INPUT_FIELD_KEY_LIST):
            return False

        if True == "alpha" in value:
            window.SetAlpha(value["alpha"])

        if True == "path" in value:
            window.MakeField(value["path"])
        else:
            window.MakeField()

        window.SetSize(int(value["width"]), int(value["height"]))

        self.LoadDefaultData(window, value, parentWindow)
        return True

    def LoadElementCheckBox(self, window, value, parentWindow):

        if "text" in value:
            window.SetText(value["text"])

        if "checked" in value and value["checked"] == True:
            window.SetChecked(window.STATE_SELECTED)

        self.LoadDefaultData(window, value, parentWindow)

        return True

    def LoadElementColorPicker(self, window, value, parentWindow):
        window.SetSize(value["width"], value["height"])

        if True == "style" in value:
            for style in value["style"]:
                window.AddFlag(style)

        self.LoadDefaultData(window, value, parentWindow)
        window.CreateColorPicker()
        return True

    def LoadElementVideoFrame(self, window, value, parentWindow):

        if False == self.CheckKeyList(value["name"], value, self.VIDEO_FRAME_KEY_LIST):
            return False

        window.SetSize(value["width"], value["height"])

        if True == "style" in value:
            for style in value["style"]:
                window.AddFlag(style)

        self.LoadDefaultData(window, value, parentWindow)

        if "video_file" in value:
            window.OpenVideo(value["video_file"])

        return True

    def LoadElementRenderTarget(self, window, value, parentWindow):

        if False == self.CheckKeyList(
            value["name"], value, self.RENDER_TARGET_KEY_LIST
        ):
            return False

        window.SetSize(value["width"], value["height"])

        if True == "style" in value:
            for style in value["style"]:
                window.AddFlag(style)

        self.LoadDefaultData(window, value, parentWindow)

        if "index" in value:
            window.SetRenderTarget(int(value["index"]))

        return True

    def LoadElementWebView(self, window, value, parentWindow):

        window.SetSize(value["width"], value["height"])

        if "style" in value:
            for style in value["style"]:
                window.AddFlag(style)

        self.LoadDefaultData(window, value, parentWindow)

        window.WebViewOnCreateInstance()
        return True


class ColorPicker(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)
        self.AddFlag("dragable")

        self.pickColorEvent = None
        self.mouseOverEvent = None

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterColorPicker(self, layer)

    def CreateColorPicker(self):
        wndMgr.ColorPickerCreate(self.hWnd)

    def SetPickColorEvent(self, event):
        self.pickColorEvent = MakeEvent(event)

    def SetMouseOveEvent(self, event):
        self.mouseOverEvent = MakeEvent(event)

    def Hide(self):
        self.ReleaseMouse()
        Window.Hide(self)

    def OnMouseOver(self):
        if self.mouseOverEvent:
            self.mouseOverEvent()

    def OnMouseOverIn(self):
        self.CaptureMouse()

    def OnMouseOverOut(self):
        self.ReleaseMouse()

    def OnPickColor(self, color):
        if self.pickColorEvent:
            self.pickColorEvent(color)


class VideoFrame(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.number = -1

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterVideoFrame(self, layer)

    def OpenVideo(self, videoFile):
        wndMgr.VideoOpen(self.hWnd, videoFile)


class RenderTarget(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.number = -1

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterRenderTarget(self, layer)

    def SetRenderTarget(self, number):
        self.number = number
        wndMgr.SetRenderTarget(self.hWnd, self.number)


class ItemRenderTarget(RenderTarget):
    def __init__(self, layer="UI"):
        RenderTarget.__init__(self, layer)
        self.AddFlag("dragable")

    def OnMouseOverIn(self):
        app.SetCursor(app.CAMERA_ROTATE)
        return True

    def OnMouseOverOut(self):
        app.SetCursor(app.NORMAL)
        return True

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition():
            if nLen > 0:
                self.SetModelZoom(1)
            else:
                self.SetModelZoom(0)
            return True
        return False

    def Show(self):
        RenderTarget.Show(self)
        player.ShowItemRenderTarget()

    def Hide(self):
        RenderTarget.Hide(self)
        player.HideItemRenderTarget()

    def SetData(self, data):
        player.SetItemRenderTargetData(data)

    def SetFov(self, fov):
        player.SetItemRenderTargetFov(fov)

    def SetTargetZ(self, z):
        player.SetItemRenderTargetZ(z)

    def EnableRotation(self, val):
        player.SetItemRenderEnableRotation(val)

    def SetRotSpeed(self, s):
        player.SetItemRenderSetRotSpeed(s)

    def SetBaseRot(self, r):
        player.SetItemRenderSetBaseRot(r)

    def SetModelZoom(self, isNear):
        player.SetItemRenderTargetModelZoom(isNear)

    def OnMouseDrag(self, x, y):
        player.ItemRenderTargetDrag(x, y)


class WikiRenderTarget(RenderTarget):
    def __init__(self, width, height):
        self.moduleID = -1
        super(WikiRenderTarget, self).__init__()
        self.AddFlag("dragable")

        self.SetSize(width, height)
        self.moduleID = wiki.GetFreeModelViewID()
        self.renderTargetMouseEvent = None
        wndMgr.SetWikiRenderTarget(self.hWnd, self.moduleID)
        wiki.AddModelView(self.moduleID)
        wiki.RegisterModelViewWindow(self.moduleID, self.hWnd)

    def __del__(self):
        wiki.RegisterModelViewWindow(self.moduleID, 0)
        # wiki.RemoveModelView(self.moduleID) #TODO : nevo -> This is breaking the render target fix this.
        wndMgr.Destroy(self.hWnd)

    def SetModel(self, vnum):
        if self.moduleID >= 0:
            wiki.SetModelViewModel(self.moduleID, vnum)

    def SetWeaponModel(self, vnum):
        if self.moduleID >= 0:
            wiki.SetModelViewWeapon(self.moduleID, vnum)

    def SetModelHair(self, vnum):
        if self.moduleID >= 0:
            wiki.SetModelViewHair(self.moduleID, vnum)

    def SetModelForm(self, vnum):
        if self.moduleID >= 0:
            wiki.SetModelViewForm(self.moduleID, vnum)

    def SetModelZoom(self, isNear):
        if self.moduleID >= 0:
            wiki.SetModelViewZoom(self.moduleID, isNear)

    def Show(self):
        super(WikiRenderTarget, self).Show()
        if self.moduleID >= 0:
            wiki.ShowModelView(self.moduleID, True)

    def Hide(self):
        super(WikiRenderTarget, self).Hide()
        if self.moduleID >= 0:
            wiki.ShowModelView(self.moduleID, False)

    def SetMouseLeftButtonDownEvent(self, event):
        self.renderTargetMouseEvent = MakeEvent(event)

    def OnMouseLeftButtonDown(self):
        if self.renderTargetMouseEvent:
            return self.renderTargetMouseEvent()
        return False

    def OnMouseOverIn(self):
        app.SetCursor(app.CAMERA_ROTATE)
        return True

    def OnMouseOverOut(self):
        app.SetCursor(app.NORMAL)
        return True

    def OnMouseDrag(self, x, y):
        if self.moduleID >= 0:
            wiki.SetModelViewDrag(self.moduleID, x, y)

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition():
            if nLen > 0:
                self.SetModelZoom(1)
            else:
                self.SetModelZoom(0)
            return True
        return False


class WebView(Window):
    def __init__(self, layer="UI"):
        Window.__init__(self, layer)

        self.url = "about:blank"

    def RegisterWindow(self, layer):
        self.hWnd = wndMgr.RegisterWebView(self, layer)

    def LoadUrl(self, url):
        self.url = url
        wndMgr.LoadUrl(self.hWnd, self.url)

    def WebViewOnCreateInstance(self):
        wndMgr.WebViewOnCreateInstance(self.hWnd)


class ScriptColorPicker(ScriptWindow):
    def __init__(self):
        ScriptWindow.__init__(self, "TOP_MOST")

        self.a = self.r = self.g = self.b = 0.0

        self.finishEvent = None
        self.cancelEvent = None
        self.defaultColor = 0
        self.isLoaded = False
        self.color_string = None
        self.preview = None
        self.colorPicker = None

    def SetDefaultColor(self, color):
        self.defaultColor = color

    def Open(self):
        self.__LoadWindow()
        self.SetCenterPosition()
        self.Show()
        self.SetTop()
        self.SetFocus()

    def Close(self):
        self.Hide()

    def GetColor(self):
        if self.r == 0 and self.g == 0 and self.b == 0:
            return self.defaultColor

        return PackColor(self.r, self.g, self.b, self.a)

    def SetColor(self, color):
        self.r, self.g, self.b, self.a = UnpackColor(color)
        if self.r == 0 and self.g == 0 and self.b == 0:
            self.a = 0

        self.__RefreshPreview()

    def SetFinishEvent(self, event):
        self.finishEvent = MakeEvent(event)

    # Window events
    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False

    def __LoadWindow(self):
        if self.isLoaded:
            return
        loader = PythonScriptLoader()
        loader.LoadScriptFile(self, "UIScript/ColorPicker.py")
        self.color_string = self.GetChild("ColorString")
        self.color_string.SetHexMode()
        self.preview = self.GetChild("Preview")
        self.colorPicker = self.GetChild("ColorContainer")
        self.colorPicker.SetPickColorEvent(self.PickColor)

        self.GetChild("Board").SetCloseEvent(self.Close)
        self.GetChild("FinishButton").SetEvent(self.__OnFinish)
        self.GetChild("CancelButton").SetEvent(self.Close)
        self.GetChild("SetFromHexButton").SetEvent(self.__SetFromHexString)
        self.isLoaded = True

    def PickColor(self, color):
        self.SetColor(color)
        self.__RefreshPreview()

    def __OnFinish(self):
        if self.finishEvent:
            self.finishEvent()

    def __RefreshPreview(self):
        if self.color_string:
            self.color_string.SetText(
                "#{:08x}".format(PackColor(1.0, self.r, self.g, self.b))
            )
        if self.preview:
            self.preview.SetColor(self.GetColor())

    def __SetFromHexString(self):
        color = self.color_string.GetText()
        if not color:
            return

        if color[0] == "#":
            color = color[1:]

        try:
            color = long(color[1:], 16)
        except ValueError:
            return

        self.SetColor(color)


class AlphaBox(Window, BaseEvents):
    def __init__(self):
        BaseEvents.__init__(self)
        Window.__init__(self)
        self.eventOnDoubleClick = None
        self.baseColor = (0, 0, 0)
        self.baseAlpha = 1.0
        self.borderColor = lighten_color(*self.baseColor, factor=0.1)

        self.calculatedBaseColor = 0
        self.calculatedBorderColor = 0
        self.RecalculateColors()

    def SetAlpha(self, alpha):
        self.baseAlpha = alpha
        self.RecalculateColors()

    def SetColor(self, r, g, b):
        self.baseColor = (r, g, b)
        self.borderColor = lighten_color(*self.baseColor, factor=0.1)
        self.RecalculateColors()

    def RecalculateColors(self):
        floatBaseColor = [x / 255.0 for x in self.baseColor]
        floatBorderColor = [x / 255.0 for x in self.borderColor]
        self.calculatedBaseColor = grp.GenerateColor(
            floatBaseColor[0], floatBaseColor[1], floatBaseColor[2], self.baseAlpha
        )
        self.calculatedBorderColor = grp.GenerateColor(
            floatBorderColor[0],
            floatBorderColor[1],
            floatBorderColor[2],
            self.baseAlpha,
        )

    def SetOnLeftButtonDoubleClickEvent(self, event):
        self.eventOnDoubleClick = MakeEvent(event)

    def OnMouseLeftButtonDoubleClick(self):
        if self.eventOnDoubleClick:
            return self.eventOnDoubleClick()

        return False

    def OnRender(self):
        # xRender, yRender = self.GetGlobalPosition()
        # widthRender = self.GetWidth()
        # heightRender = self.GetHeight()
        # grp.SetColor(BACKGROUND_COLOR_ALPHA)
        # grp.RenderBar(xRender, yRender, widthRender, heightRender)
        # grp.SetColor(DARK_COLOR)
        # grp.RenderLine(xRender, yRender, widthRender, 0)
        # grp.RenderLine(xRender, yRender, 0, heightRender)

        x, y, w, h = self.GetRect()

        grp.SetColor(self.calculatedBaseColor)
        grp.RenderBar(x, y, w, h)

        grp.SetColor(self.calculatedBorderColor)
        grp.RenderLine(x, y, w, 0)
        grp.RenderLine(x, y, 0, h)

        grp.SetColor(self.calculatedBorderColor)
        grp.RenderLine(x, y + h, w, 0)
        grp.RenderLine(x + w, y, 0, h)


def MakeSlotBar(parent, x, y, width, height):
    slotBar = SlotBar()
    slotBar.SetParent(parent)
    slotBar.SetSize(width, height)
    slotBar.SetPosition(x, y)
    slotBar.Show()
    return slotBar


def MakeImageBox(parent, name, x, y):
    image = ImageBox()
    image.SetParent(parent)
    image.LoadImage(name)
    image.SetPosition(x, y)
    image.Show()
    return image


def MakeExpandedImageBox(parent, name, x, y, flag=""):
    image = ExpandedImageBox()
    image.SetParent(parent)
    image.LoadImage(name)
    image.SetPosition(x, y)
    image.AddFlag(flag)
    image.Show()
    return image


def MakeVideoFrame(parent, filename, x, y):
    video = VideoFrame()
    video.SetParent(parent)
    video.OpenVideo(filename)
    video.SetPosition(x, y)
    video.Show()
    return video


def MakeTextLine(parent):
    textLine = TextLine()
    textLine.SetParent(parent)
    textLine.SetHorizontalAlignCenter()
    textLine.SetVerticalAlignCenter()
    textLine.Show()
    return textLine


def MakeTextLineNew(parent, x, y, text=""):
    textLine = TextLine()
    textLine.SetParent(parent)
    textLine.SetHorizontalAlignLeft()
    textLine.SetVerticalAlignCenter()
    textLine.SetPosition(x, y)
    textLine.SetText(text)
    textLine.Show()
    return textLine


def MakeLimitedTextLine(parent, max=15):
    textLine = LimitedTextLine()
    textLine.SetParent(parent)
    textLine.SetHorizontalAlignCenter()
    textLine.SetVerticalAlignCenter()
    textLine.SetMax(max)
    textLine.Show()
    return textLine


def MakeButton(parent, x, y, tooltipText, path, up, over, down):
    button = Button()
    button.SetParent(parent)
    button.SetPosition(x, y)
    button.SetUpVisual(path + up)
    button.SetOverVisual(path + over)
    button.SetDownVisual(path + down)
    button.SetToolTipText(tooltipText)
    button.Show()
    return button


def MakeButtonWithText(parent, text, x, y, path, up, over, down):
    button = Button()
    button.SetParent(parent)
    button.SetPosition(x, y)
    button.SetUpVisual(path + up)
    button.SetOverVisual(path + over)
    button.SetDownVisual(path + down)
    button.SetText(text)
    button.Show()
    return button


def GenerateColor(r, g, b, a=1.0):
    r = float(r) / 255.0
    g = float(g) / 255.0
    b = float(b) / 255.0
    return grp.GenerateColor(r, g, b, a)


def GetHyperlink():
    active = wndMgr.GetPickedWindow()

    if not active or not isinstance(active, TextLine):
        return None

    return active.GetHyperlinkAtPos(*active.GetMouseLocalPosition())


RegisterToolTipWindow("TEXT", TextLine)
