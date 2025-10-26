# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
from __future__ import division

import logging
import math
from _weakref import proxy, ref

import app
import grp
import item
import nonplayer
import player
import wiki, chat
from pack import OpenVfsFile
from pygame.item import itemManager
from pygame.wiki import wikiInst

import localeInfo
import ui
from switchbot import FormatApplyAsString
from ui import WikiRenderTarget
from ui_event import Event, MakeEvent

enableDebugThings = 0

# if enableDebugThings:
#    WIKI_CATEGORIES[1][1].append(["DEV allchests", ([],)])

categoryPeakWindowSize = [109, 380]
mainBoardPos = [148, 106]
mainBoardSize = [550, 435]
monsterBonusInfoPageSize = [539, 157]
itemOriginPageSize = [357, 222]


def MakeMoneyText(money):
    money = str(money)
    original = money
    sLen = len(original)
    while sLen > 3 and original[sLen - 3 :] == "000":
        money = money[::-1].replace("000"[::-1], "k"[::-1], 1)[::-1]
        original = original[: sLen - 3]
        sLen -= 3

    return money


def HAS_FLAG(value, flag):
    return (value & flag) == flag


class SubCategObject(ui.ExpandedImageBox):
    def __init__(self, text):
        super(SubCategObject, self).__init__()
        self.SetWindowName("SubCategObject_ListBoxEx_Item")

        self.mArgs = ()
        self.bannerFileName = None
        if isinstance(text, list):
            self.mArgs = text[1]
            if len(text) > 2:
                self.bannerFileName = text[2]

            text = text[0]
        self.needAnim = False
        self.originText = text
        self.isMoving = False
        self.parent = None
        self.selected = False

        self.textLine = ui.TextLine()
        self.textLine.AddFlag("attach")
        self.textLine.AddFlag("not_pick")
        self.textLine.SetParent(self)
        self.textLine.SetText(text)
        self.textLine.SetPosition(0, 0)
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.Show()

        self.LoadImage("d:/ymir work/ui/wiki/sub_cat_0.tga")

    def SetParent(self, parent):
        ui.Window.SetParent(self, parent)
        self.parent = proxy(parent)

    def OnMouseLeftButtonDown(self):
        self.parent.SelectItem(self)

    def OnUpdate(self):
        if self.parent.GetSelectedItem() == self and not self.selected:
            self.LoadImage("d:/ymir work/ui/game/myshop_deco/select_btn_03.sub")
            self.selected = True
        elif self.parent.GetSelectedItem() != self and self.selected:
            self.LoadImage("d:/ymir work/ui/game/myshop_deco/select_btn_02.sub")
            self.selected = False


class WikiCategory(ui.Window):
    TICK_COUNT = 6
    MIN_HEIGHT = 20
    ARROW_IMG = ["d:/ymir work/ui/wiki/arrow_2.tga", "d:/ymir work/ui/wiki/arrow.tga"]

    LINES_COLOR = grp.GenerateColor(0.2941, 0.2941, 0.2941, 1.0)
    CORNER_COLOR = grp.GenerateColor(0.2117, 0.2117, 0.2117, 1.0)
    BASE_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 1.0)

    def __init__(self, owner=None):
        super(WikiCategory, self).__init__()
        if owner:
            self.owner = proxy(owner)
        else:
            self.owner = None

        self.clickEvent = None

        self.expectedSize = 0
        self.isAnimating = False
        self.isOpening = False
        self.currHeight = self.MIN_HEIGHT

        self.titleImg = ui.ExpandedImageBox()
        self.titleImg.SetParent(self)
        self.titleImg.AddFlag("attach")
        self.titleImg.LoadImage("d:/ymir work/ui/wiki/category.tga")
        self.titleImg.SetClickEvent(self.ClickExpand)

        self.titleImg.Show()

        self.titleText = ui.TextLine()
        self.titleText.SetParent(self.titleImg)
        self.titleText.AddFlag("attach")
        self.titleText.AddFlag("not_pick")
        self.titleText.SetPosition(5, self.titleImg.GetHeight() / 2)
        self.titleText.SetOutline(True)

        self.titleText.SetHorizontalAlignCenter()
        self.titleText.Show()

        self.arrow = ui.ExpandedImageBox()
        self.arrow.SetParent(self.titleImg)
        self.arrow.AddFlag("attach")
        self.arrow.AddFlag("not_pick")
        self.arrow.LoadImage(self.ARROW_IMG[0])
        self.arrow.SetPosition(
            int(self.titleImg.GetWidth() - self.arrow.GetWidth() - 5),
            int(self.titleImg.GetHeight() / 2 - self.arrow.GetHeight() / 2),
        )
        self.arrow.Show()

        self.expandWnd = ui.Window()
        self.expandWnd.SetParent(self)
        self.expandWnd.AddFlag("attach")
        self.expandWnd.AddFlag("not_pick")
        # self.expandWnd.SetAlpha(0.5)
        self.expandWnd.SetPosition(0, self.titleImg.GetHeight())
        self.expandWnd.SetSize(self.titleImg.GetWidth(), 0)
        self.expandWnd.Hide()

        self.categList = ui.ListBoxEx()
        self.categList.SetParent(self.expandWnd)
        self.categList.SetItemSize(self.titleImg.GetWidth() - 6, 17)
        self.categList.SetItemStep(17)
        self.categList.SetHorizontalAlignCenter()
        self.categList.SetSize(self.titleImg.GetWidth() - 6, self.MIN_HEIGHT - 1)
        self.categList.SetPosition(3, 1)
        self.categList.SetSelectEvent(self.OnSelectSubCategory)
        self.categList.Show()

        ui.Window.SetSize(self, self.titleImg.GetWidth(), self.titleImg.GetHeight())
        self.ArrangeBars(self.MIN_HEIGHT)

    def OnSelectSubCategory(self, elem):
        self.expandWnd.Show()
        self.owner.NotifyCategorySelect(self)

        wikiClass = wiki.GetBaseClass()
        if elem.bannerFileName and wikiClass:
            wikiClass.header.LoadImage(elem.bannerFileName)
            wikiClass.header.Show()
        elif wikiClass:
            wikiClass.header.Hide()

        if self.clickEvent:
            self.clickEvent(*elem.mArgs)

        self.UnselectSubCategory()
        elem.LoadImage("d:/ymir work/ui/wiki/sub_cat_2.tga")

    def UnselectSubCategory(self):
        self.categList.SelectIndex(-1)

        for item in self.categList.GetItems():
            item.LoadImage("d:/ymir work/ui/wiki/sub_cat_0.tga")

    def AddSubCategory(self, key, text):
        self.currHeight = 16 * (self.categList.GetItemCount() + 1) + 10

        if self.expandWnd.GetHeight() > 0:
            self.ArrangeBars(self.currHeight)
            self.isAnimating = True

        self.categList.SetViewItemCount(self.categList.GetItemCount() + 1)
        self.categList.AppendItem(SubCategObject(text))
        self.categList.SetSize(self.categList.GetWidth(), self.currHeight - 1)

    def ClickExpand(self):
        if self.isOpening:
            self.arrow.LoadImage(self.ARROW_IMG[0])
        else:
            self.arrow.LoadImage(self.ARROW_IMG[1])

        self.isOpening = not self.isOpening
        self.isAnimating = True
        self.expandWnd.Show()
        if self.isOpening:
            self.ArrangeBars(self.currHeight)
        else:
            self.ArrangeBars(0)

    def OnUpdate(self):
        if self.isAnimating:
            h = self.expandWnd.GetHeight()
            if h == self.expectedSize:
                self.isAnimating = False
                if h == 0:
                    self.expandWnd.Hide()
                return

            isOpening = True
            if h > self.expectedSize:
                isOpening = False

            newSize = 0
            step = self.currHeight / self.TICK_COUNT
            if isOpening:
                newSize = min(self.currHeight, self.expandWnd.GetHeight() + step)
            else:
                newSize = max(0, self.expandWnd.GetHeight() - step)

            change = newSize - self.expandWnd.GetHeight()

            self.expandWnd.SetSize(self.titleImg.GetWidth(), newSize)
            ui.Window.SetSize(
                self, self.titleImg.GetWidth(), self.titleImg.GetHeight() + newSize
            )
            self.owner.NotifySizeChange(self, change)

    def SetSize(self, width, height):
        import dbg

        dbg.LogBox("WikiCategory -> SetSize - unsupported function")

    def ArrangeBars(self, currHeight):
        self.expectedSize = currHeight

    def SetTitleName(self, text):
        self.titleText.SetText(text)
        self.titleText.SetPosition(
            5, int(self.titleImg.GetHeight() / 2 - self.titleText.GetHeight() / 2 - 1)
        )


class WikiCategories(ui.Window):
    CATEGORY_PADDING = 5
    SCROLL_SPEED = 17

    def __init__(self):
        super(WikiCategories, self).__init__()
        self.elements = []

        self.SetSize(categoryPeakWindowSize[0], categoryPeakWindowSize[1])

        self.scrollWindow = ui.ScrollWindow()
        self.scrollWindow.SetSize(categoryPeakWindowSize[0], categoryPeakWindowSize[1])
        self.scrollWindow.SetWindowName("WikiCategoriesScrollWindow")
        self.scrollWindow.SetPosition(0, 0)

        from ui_modern_controls import AutoGrowingVerticalContainerEx

        self.scrollBoard = AutoGrowingVerticalContainerEx()
        self.scrollBoard.SetWidth(categoryPeakWindowSize[0])
        self.scrollBoard.SetPosition(0, 0)
        self.scrollWindow.SetContentWindow(self.scrollBoard)
        self.scrollWindow.FitWidth(categoryPeakWindowSize[0])
        self.scrollWindow.SetParent(self)
        self.scrollWindow.Show()

        self.hideWindowsEvent = None

    def AddCategory(self, text):
        tmp = WikiCategory(self)
        tmp.SetPosition(0, 0)
        tmp.SetParent(self.scrollBoard)
        tmp.SetTitleName(text)

        self.scrollBoard.AppendItem(tmp)
        self.elements.append(tmp)
        tmp.Show()
        return tmp

    def NotifySizeChange(self, obj, amount):
        self.scrollBoard.RecalculateHeight()

    def NotifyCategorySelect(self, obj):
        if self.hideWindowsEvent:
            self.hideWindowsEvent()
        for i in self.elements:
            if obj != proxy(i) and obj != i:
                i.UnselectSubCategory()


class WikiMainWeaponWindow(ui.Window):
    class WikiItem(ui.Window):
        # this item is fucking dooomed (since I got the image in one piece)
        TABLE_COLS = [
            [131, 21, 40, 134],
            [172, 21, 40, 134],
            [213, 21, 41, 134],
            [255, 21, 40, 134],
            [296, 21, 40, 134],
            [337, 21, 41, 134],
            [379, 21, 40, 134],
            [420, 21, 40, 134],
            [461, 21, 41, 134],
            [503, 21, 36, 134],
        ]
        ROW_HEIGHTS = [21, 44, 51, 17]
        ROW_START_Y = [0, 22, 67, 119]
        ROW_HEIGHTS3 = [21, 44, 47, 42, 22]
        ROW_START_Y3 = [
            0,
            21 + 1,
            21 + 1 + 44 + 1,
            21 + 1 + 44 + 1 + 47 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1,
        ]
        ROW_HEIGHTS4 = [21, 44, 47, 42, 42, 22]
        ROW_START_Y4 = [
            0,
            21 + 1,
            21 + 1 + 44 + 1,
            21 + 1 + 44 + 1 + 47 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1,
        ]
        ROW_HEIGHTS5 = [21, 44, 47, 42, 42, 42, 22]
        ROW_START_Y5 = [
            0,
            21 + 1,
            21 + 1 + 44 + 1,
            21 + 1 + 44 + 1 + 47 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1 + 42 + 1,
        ]
        ROW_HEIGHTS6 = [21, 44, 47, 42, 42, 42, 42, 22]
        ROW_START_Y6 = [
            0,
            21 + 1,
            21 + 1 + 44 + 1,
            21 + 1 + 44 + 1 + 47 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1 + 42 + 1,
            21 + 1 + 44 + 1 + 47 + 1 + 42 + 1 + 42 + 1 + 42 + 1 + 42 + 1,
        ]

        def __init__(self, vnum, parent, enable3Row=False):
            ui.Window.__init__(self)

            wikiBase = wiki.GetBaseClass()
            if vnum not in wikiBase.objList:
                v = wikiBase.objList.setdefault(vnum, {})
                if vnum not in v:
                    v[self.GetWindowName()] = ref(self)

            self.additionalLoaded = False
            self.vnum = vnum
            self.levelLimit = 0
            self.parent = proxy(parent)
            wiki.LoadInfo(long(id(self)), self.vnum)
            proto = itemManager().GetProto(self.vnum)

            self.base = ui.ExpandedImageBox()
            self.base.SetParent(self)
            self.base.AddFlag("attach")
            self.base.LoadImage("d:/ymir work/ui/wiki/slot_2.tga")
            self.base.Show()
            self.enable3Row = enable3Row

            self.cols = []
            cnt = 0
            for i in self.TABLE_COLS:
                tmp = ui.Window()
                tmp.SetParent(self.base)
                tmp.AddFlag("attach")
                tmp.SetPosition(i[0], i[1])
                tmp.SetSize(i[2], i[3])
                tmp.Show()

                titleWnd = ui.WindowWithBaseEvents()
                titleWnd.SetParent(tmp)
                titleWnd.SetPosition(0, self.ROW_START_Y[0])
                titleWnd.SetSize(tmp.GetWidth(), self.ROW_HEIGHTS[0])
                titleWnd.SetOverInEvent(Event(self.parent.OnOverIn, self.vnum + cnt))
                titleWnd.SetOverOutEvent(self.parent.OnOverOut)
                if wiki.GetBaseClass():
                    titleWnd.SetMouseLeftButtonDownEvent(
                        Event(
                            wiki.GetBaseClass().OpenSpecialPage,
                            self.parent,
                            self.vnum,
                            False,
                        )
                    )
                titleWnd.Show()
                tmp.titleWnd = titleWnd

                tx = ui.TextLine()
                tx.SetParent(titleWnd)
                tx.SetText("+{}".format(cnt))
                tx.SetHorizontalAlignCenter()
                tx.SetOutline()
                tx.SetPosition(
                    0, int(titleWnd.GetHeight() / 2 - tx.GetHeight() / 2 - 1)
                )
                tx.Show()

                tmp.refineText = tx

                tmp.refineMat = []
                for _ in xrange(2):
                    img = ui.ExpandedImageBox()
                    img.SetParent(tmp)
                    img.SetHorizontalAlignCenter()
                    img.SetPosition(0, 0)
                    img.Show()
                    tmp.refineMat.append(img)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")

                tx.SetPosition(
                    int(tmp.GetWidth() / 2 + 10 - tx.GetWidth() / 2),
                    int(
                        self.ROW_START_Y[1]
                        + self.ROW_HEIGHTS[1] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )

                tx.Show()

                tmp.refineMatText = [tx]

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y[2]
                        + self.ROW_HEIGHTS[2] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.LimitedTextLine()
                tx.SetMax(6)
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()

                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y[3]
                        + self.ROW_HEIGHTS[3] / 2
                        - tx.GetHeight() / 2
                        + 2
                    ),
                )
                tx.Show()

                tmp.moneyText = tx
                self.cols.append(tmp)
                cnt += 1

            self.itemNameText = ui.TextLine()
            self.itemNameText.SetParent(self.base)

            itemName = proto.GetName()
            fnd = itemName.find("+")
            if fnd >= 0:
                itemName = itemName[:fnd].strip()
            if enableDebugThings:
                self.itemNameText.SetText(itemName + " (%i)" % self.vnum)
            else:
                self.itemNameText.SetText(itemName)

            self.itemNameText.SetHorizontalAlignCenter()
            self.itemNameText.SetPosition(
                0, int(1 + 19 / 2 - self.itemNameText.GetHeight() / 2)
            )
            self.itemNameText.Show()

            self.slotList = []
            for i in xrange(3):
                slot = ui.ImageBox()
                slot.LoadImage("d:/ymir work/ui/public/slot_base.sub")
                slot.SetParent(self.base)
                self.slotList.append(slot)

            self.slotGrid = ui.SlotWindow()
            self.slotGrid.SetParent(self.base)
            self.slotGrid.SetPosition(10, 52)
            self.slotGrid.AppendSlot(0, 0, 0, 32 * 1, 32 * proto.GetSize())
            self.slotGrid.AddFlag("not_pick")
            self.slotGrid.Show()
            self.slotGrid.SetItemSlot(0, self.vnum, 0)

            for i in xrange(min(3, proto.GetSize())):
                self.slotList[i].SetPosition(10, 20 + proto.GetSize() * 32 - i * 32)
                self.slotList[i].SetOnMouseOverInEvent(
                    Event(self.parent.OnOverIn, self.vnum + 9)
                )
                self.slotList[i].SetOnMouseOverOutEvent(self.parent.OnOverOut)
                self.slotList[i].SetClickEvent(
                    Event(
                        wiki.GetBaseClass().OpenSpecialPage,
                        self.parent,
                        self.vnum,
                        False,
                    )
                )
                self.slotList[i].Show()

            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            self.NoticeMe()

        def OnUpdate(self):
            if not self.additionalLoaded:
                if not wiki.IsSet(self.vnum, False):
                    wiki.LoadInfo(long(id(self)), self.vnum, False)
                else:
                    self.NoticeMe()
                    self.additionalLoaded = True

        def NoticeMe(self):
            retInfo = wiki.GetRefineInfo(self.vnum)
            if not retInfo:
                return

            maxMat = 2
            moneyRow = 3
            useHeight = self.ROW_HEIGHTS
            useStart = self.ROW_START_Y
            for i in xrange(10):
                curr = 0
                for j in retInfo[i][1]:
                    if j[0] == 0:
                        continue
                    curr += 1
                maxMat = max(maxMat, curr)

            if maxMat == 3:
                self.Set3Row()
                moneyRow = 4
                useHeight = self.ROW_HEIGHTS3
                useStart = self.ROW_START_Y3

            if maxMat == 4:
                self.Set4Row()
                moneyRow = 5
                useHeight = self.ROW_HEIGHTS4
                useStart = self.ROW_START_Y4

            if maxMat == 5:
                self.Set5Row()
                moneyRow = 6
                useHeight = self.ROW_HEIGHTS5
                useStart = self.ROW_START_Y5

            if maxMat == 6:
                self.Set6Row()
                moneyRow = 7
                useHeight = self.ROW_HEIGHTS6
                useStart = self.ROW_START_Y6

            for i in xrange(10):
                currWindow = self.cols[i]
                money = MakeMoneyText(retInfo[i][0])
                currWindow.moneyText.SetText(money)
                currWindow.moneyText.SetPosition(
                    0,
                    useStart[moneyRow]
                    + useHeight[moneyRow] / 2
                    - currWindow.moneyText.GetHeight() / 2
                    + 2,
                )

                curr = 0
                for j in retInfo[i][1]:
                    if j[0] == 0:
                        continue
                    if curr >= maxMat:
                        break

                    item.SelectItem(j[0])
                    currImage = currWindow.refineMat[curr]
                    currImage.LoadImage(item.GetIconImageFileName())
                    currImage.SetPosition(
                        0,
                        useStart[curr + 1]
                        + useHeight[curr + 1] / 2
                        - currImage.GetHeight() / 2,
                    )
                    currImage.SetOnMouseOverInEvent(
                        Event(self.parent.OnOverIn, j[0], int(j[1]))
                    )
                    currImage.SetOnMouseOverOutEvent(self.parent.OnOverOut)
                    currImage.SetClickEvent(
                        Event(
                            wiki.GetBaseClass().OpenSpecialPage,
                            self.parent,
                            j[0],
                            False,
                        )
                    )
                    currImage.SetScale(0.7, 0.7)

                    currText = currWindow.refineMatText[curr]
                    currText.SetOutline(True)

                    currText.SetFontColor(1.0, 1.0, 1.0)
                    currText.SetText(j[1])
                    currText.SetHorizontalAlignCenter()
                    currText.SetPosition(
                        0,
                        currImage.GetLocalPosition()[1]
                        + currImage.GetHeight()
                        - currText.GetHeight(),
                    )

                    curr += 1

        def Set3Row(self):
            oldSize = self.base.GetHeight()
            self.base.LoadImage("d:/ymir work/ui/wiki/slot_3.tga")
            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            proto = itemManager().GetProto(self.vnum)

            for tmp in self.cols:
                tmp.SetSize(tmp.GetWidth(), 178)
                tmp.refineMat = []
                for j in xrange(3):
                    img = ui.ExpandedImageBox()
                    img.SetParent(tmp)
                    img.SetPosition(0, 0)
                    img.SetHorizontalAlignCenter()
                    img.Show()
                    tmp.refineMat.append(img)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y3[1]
                        + self.ROW_HEIGHTS3[1] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText = [tx]

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y3[2]
                        + self.ROW_HEIGHTS3[2] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y3[3]
                        + self.ROW_HEIGHTS3[3] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.LimitedTextLine()
                tx.SetMax(6)
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y3[4]
                        + self.ROW_HEIGHTS3[4] / 2
                        - tx.GetHeight() / 2
                        + 2
                    ),
                )
                tx.Show()

                tmp.moneyText = tx

            if self.parent:
                self.parent.ChangeElementSize(self, self.base.GetHeight() - oldSize)

        def Set4Row(self):
            oldSize = self.base.GetHeight()
            self.base.LoadImage("d:/ymir work/ui/wiki/slot_4.tga")
            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            proto = itemManager().GetProto(self.vnum)

            for tmp in self.cols:
                tmp.SetSize(tmp.GetWidth(), 218)
                tmp.refineMat = []
                for j in xrange(4):
                    img = ui.ExpandedImageBox()
                    img.SetParent(tmp)
                    img.SetPosition(0, 0)
                    img.SetHorizontalAlignCenter()
                    img.Show()
                    tmp.refineMat.append(img)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y4[1]
                        + self.ROW_HEIGHTS4[1] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText = [tx]

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y4[2]
                        + self.ROW_HEIGHTS4[2] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(197.0 / 255.0, 106.0 / 255.0, 32.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y4[3]
                        + self.ROW_HEIGHTS4[3] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y4[4]
                        + self.ROW_HEIGHTS4[4] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.LimitedTextLine()
                tx.SetMax(6)
                tx.SetParent(tmp)
                tx.SetText("-")
                # tx.SetFontColor(228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0)
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y4[5]
                        + self.ROW_HEIGHTS4[5] / 2
                        - tx.GetHeight() / 2
                        + 2
                    ),
                )
                tx.Show()

                tmp.moneyText = tx

            if self.parent:
                self.parent.ChangeElementSize(self, self.base.GetHeight() - oldSize)

        def Set5Row(self):
            oldSize = self.base.GetHeight()
            self.base.LoadImage("d:/ymir work/ui/wiki/slot_5.tga")
            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            proto = itemManager().GetProto(self.vnum)

            for tmp in self.cols:
                tmp.SetSize(tmp.GetWidth(), 265)
                tmp.refineMat = []
                for j in xrange(5):
                    img = ui.ExpandedImageBox()
                    img.SetParent(tmp)
                    img.SetPosition(0, 0)
                    img.SetHorizontalAlignCenter()
                    img.Show()
                    tmp.refineMat.append(img)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[1]
                        + self.ROW_HEIGHTS5[1] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText = [tx]

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[2]
                        + self.ROW_HEIGHTS5[2] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[3]
                        + self.ROW_HEIGHTS5[3] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[4]
                        + self.ROW_HEIGHTS5[4] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[5]
                        + self.ROW_HEIGHTS5[5] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.LimitedTextLine()
                tx.SetMax(6)
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y5[6]
                        + self.ROW_HEIGHTS5[6] / 2
                        - tx.GetHeight() / 2
                        + 2
                    ),
                )
                tx.Show()

                tmp.moneyText = tx

            if self.parent:
                self.parent.ChangeElementSize(self, self.base.GetHeight() - oldSize)

        def Set6Row(self):
            oldSize = self.base.GetHeight()
            self.base.LoadImage("d:/ymir work/ui/wiki/slot_5.tga")
            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            proto = itemManager().GetProto(self.vnum)

            for tmp in self.cols:
                tmp.SetSize(tmp.GetWidth(), 178)
                tmp.refineMat = []
                for j in xrange(6):
                    img = ui.ExpandedImageBox()
                    img.SetParent(tmp)
                    img.SetPosition(0, 0)
                    img.SetHorizontalAlignCenter()
                    img.Show()
                    tmp.refineMat.append(img)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[1]
                        + self.ROW_HEIGHTS6[1] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText = [tx]

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[2]
                        + self.ROW_HEIGHTS6[2] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[3]
                        + self.ROW_HEIGHTS6[3] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[4]
                        + self.ROW_HEIGHTS6[4] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[5]
                        + self.ROW_HEIGHTS6[5] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.TextLine()
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[6]
                        + self.ROW_HEIGHTS6[6] / 2
                        - tx.GetHeight() / 2
                        - 1
                    ),
                )
                tx.Show()

                tmp.refineMatText.append(tx)

                tx = ui.LimitedTextLine()
                tx.SetMax(6)
                tx.SetParent(tmp)
                tx.SetText("-")
                tx.SetHorizontalAlignCenter()
                tx.SetPosition(
                    0,
                    int(
                        self.ROW_START_Y6[7]
                        + self.ROW_HEIGHTS6[7] / 2
                        - tx.GetHeight() / 2
                        + 2
                    ),
                )
                tx.Show()

                tmp.moneyText = tx

            if self.parent:
                self.parent.ChangeElementSize(self, self.base.GetHeight() - oldSize)

    ELEM_PADDING = 2
    SCROLL_SPEED = 50
    ITEM_LOAD_PER_UPDATE = 1
    CLASS_BUTTONS = [
        [
            item.ITEM_ANTIFLAG_WARRIOR,
            "d:/ymir work/ui/wiki/class_w_normal.tga",
            "d:/ymir work/ui/wiki/class_w_hover.tga",
            "d:/ymir work/ui/wiki/class_w_selected.tga",
        ],
        [
            item.ITEM_ANTIFLAG_ASSASSIN,
            "d:/ymir work/ui/wiki/class_n_normal.tga",
            "d:/ymir work/ui/wiki/class_n_hover.tga",
            "d:/ymir work/ui/wiki/class_n_selected.tga",
        ],
        [
            item.ITEM_ANTIFLAG_SHAMAN,
            "d:/ymir work/ui/wiki/class_s_normal.tga",
            "d:/ymir work/ui/wiki/class_s_hover.tga",
            "d:/ymir work/ui/wiki/class_s_selected.tga",
        ],
        [
            item.ITEM_ANTIFLAG_SURA,
            "d:/ymir work/ui/wiki/class_su_normal.tga",
            "d:/ymir work/ui/wiki/class_su_hover.tga",
            "d:/ymir work/ui/wiki/class_su_selected.tga",
        ],
    ]

    def __init__(self):
        super(WikiMainWeaponWindow, self).__init__()

        self.SetSize(mainBoardSize[0], mainBoardSize[1])

        self.elements = []
        self.isOpened = False
        self.loadFrom = 0
        self.loadTo = 0
        self.currFlag = 0
        self.currCateg = 0
        self.toolTip = None

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(550, self.GetHeight() - 130)
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(550)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 50)
        self.peekWindow.Show()

        self.categWindow = ui.Window()
        self.categWindow.SetParent(self)
        self.categWindow.AddFlag("attach")
        self.categWindow.AddFlag("not_pick")
        self.categWindow.Show()

        self.classBtns = []
        for i in self.CLASS_BUTTONS:
            tmp = ui.RadioButton()
            tmp.SetParent(self.categWindow)
            tmp.SetUpVisual(i[1])
            tmp.SetOverVisual(i[2])
            tmp.SetDownVisual(i[3])
            tmp.SetEvent(Event(self.OnSelectCateg, proxy(tmp), i[0]))
            tmp.Show()

            tmp.SetPosition(
                int(self.categWindow.GetWidth() + 5 * self.CLASS_BUTTONS.index(i)), 0
            )
            self.categWindow.SetSize(
                self.categWindow.GetWidth() + tmp.GetWidth(), tmp.GetHeight()
            )
            self.classBtns.append(tmp)

        self.categWindow.SetSize(
            self.categWindow.GetWidth() + 10 * (len(self.CLASS_BUTTONS) - 1),
            self.categWindow.GetHeight(),
        )
        self.categWindow.SetPosition(
            int(self.GetWidth() / 2 - self.categWindow.GetWidth() / 2), 10
        )

    def SetItemToolTip(self, t):
        self.toolTip = t

    def OpenWindow(self):
        super(WikiMainWeaponWindow, self).Show()

    def Show(self, categID):
        super(WikiMainWeaponWindow, self).Show()
        self.peekWindow.ScrollToStart()

        isChanged = not categID == self.currCateg
        self.currCateg = categID
        if not self.isOpened:
            self.isOpened = True
            if len(self.classBtns):
                self.OnSelectCateg(proxy(self.classBtns[0]), self.CLASS_BUTTONS[0][0])
        else:
            self.loadTo = wiki.LoadClassItems(self.currCateg, self.currFlag)
            if self.loadFrom > self.loadTo or isChanged:
                del self.elements[:]
                self.scrollBoard.SetSize(0, 0)
                self.loadFrom = 0

    def OnOverIn(self, vnum, count=None):
        self.toolTip.ClearToolTip()
        self.toolTip.AddItemData(
            vnum,
            [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)],
            0,
            showRefinedVnum=True,
        )
        if count is not None:
            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_REQUIRED_COUNT").format(count)
            )

    def OnOverOut(self):
        self.toolTip.Hide()

    def OnSelectCateg(self, btn, flag):
        self.currFlag = flag
        for i in self.classBtns:
            if proxy(i) != btn:
                i.SetUp()
            else:
                i.Down()

        del self.elements[:]
        self.scrollBoard.SetSize(0, 0)

        self.loadTo = wiki.LoadClassItems(self.currCateg, self.currFlag)
        self.loadFrom = 0

    def OnUpdate(self):
        if self.loadFrom < self.loadTo:
            for i in wiki.ChangePage(
                self.loadFrom,
                min(self.loadTo, self.loadFrom + self.ITEM_LOAD_PER_UPDATE),
            ):
                self.AddItem(i)
                self.loadFrom += 1

    def ChangeElementSize(self, elem, sizeDiff):
        foundItem = False
        for i in self.elements:
            if elem != i and not foundItem:
                continue
            elif elem == i:
                foundItem = True
                continue

            i.SetPosition(i.GetLocalPosition()[0], i.GetLocalPosition()[1] + sizeDiff)

        if foundItem:
            self.scrollBoard.SetSize(
                elem.GetWidth(), self.scrollBoard.GetHeight() + sizeDiff
            )

    def AddItem(self, vnum):
        if vnum != 94326:
            vnum = int(vnum / 10) * 10
        for i in self.elements:
            if vnum == i.vnum:
                return None
        if not itemManager().GetProto(vnum):
            return None

        tmp = self.WikiItem(vnum, self, True)
        tmp.SetParent(self.scrollBoard)
        tmp.AddFlag("attach")

        totalElem = len(self.elements)
        addPadding = 0
        if totalElem > 0:
            lastIndex = 0
            for i in xrange(totalElem):
                if (
                    self.elements[i].levelLimit < tmp.levelLimit
                    or self.elements[i].levelLimit == tmp.levelLimit
                    and self.elements[i].vnum < tmp.vnum
                ):
                    break
                lastIndex += 1

            self.elements.insert(lastIndex, tmp)
            totalElem += 1

            for i in xrange(lastIndex, totalElem):
                if i == 0:
                    self.elements[i].SetPosition(0, 0)
                else:
                    self.elements[i].SetPosition(
                        0,
                        self.elements[i - 1].GetLocalPosition()[1]
                        + self.elements[i - 1].GetHeight()
                        + self.ELEM_PADDING,
                    )

            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            addPadding = self.ELEM_PADDING

        else:
            self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            tmp.GetWidth(), self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight()
        )
        return tmp


class ChestPeekWindow(ui.Window):
    ELEM_X_PADDING = 0
    ELEM_PADDING = 0
    SCROLL_SPEED = 25
    ELEM_PER_LINE = 11

    def __init__(self, parent, w, h, sendParent=True):
        ui.Window.__init__(self)

        self.SetSize(w, h)

        self.parent = proxy(parent)
        self.posMap = {}
        self.elements = []
        self.mOverInEvent = None
        self.mOverOutEvent = None
        self.sendParent = sendParent

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.AddFlag("attach")
        self.peekWindow.AddFlag("not_pick")

        self.peekWindow.SetSize(w - 6, self.GetHeight())
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(w - 6)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 0)
        self.peekWindow.Show()

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition():
            return self.scrollBoard.OnRunMouseWheel(nLen)

        return False

    def ClearItems(self):
        self.elements = []
        self.posMap = {}

    def AddItem(self, vnum, countTex=None, rarity=None):
        metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]

        proto = itemManager().GetProto(vnum)
        if not proto:
            return None

        tmp = ui.ExpandedImageBox()
        tmp.SetParent(self.scrollBoard)
        tmp.LoadImage(proto.GetIconImageFileName())
        tmp.itemSize = proto.GetSize()
        tmp.SetOnMouseOverInEvent(
            Event(self.mOverInEvent, vnum, metinSlot, False, countTex, rarity)
        )
        tmp.SetOnMouseOverOutEvent(self.mOverOutEvent)
        if wiki.GetBaseClass():
            if not self.sendParent:
                tmp.SetClickEvent(
                    Event(wiki.GetBaseClass().OpenSpecialPage, None, vnum, False)
                )
            else:
                if hasattr(self.parent, "parent"):
                    tmp.SetClickEvent(
                        Event(wiki.GetBaseClass().OpenSpecialPage, None, vnum, False)
                    )
                else:
                    tmp.SetClickEvent(
                        Event(wiki.GetBaseClass().OpenSpecialPage, None, vnum, False)
                    )
        tmp.vnum = vnum

        if countTex:
            tmp.countText = ui.NumberLine()
            # tmp.countText.SetOutline(True)
            tmp.countText.SetNumber(str(countTex))
            # tmp.countText.SetHorizontalAlignRight()
            # tmp.countText.SetVerticalAlignBottom()
            tmp.countText.SetParent(tmp)
            # tmp.countText.SetPosition(
            #     0, tmp.GetLocalPosition()[1] + tmp.GetHeight() - tmp.GetHeight()
            # )
            tmp.countText.SetPosition(
                tmp.GetWidth()
                - tmp.countText.GetWidth()
                - 10
                - (len(str(countTex))) * 3,
                tmp.GetHeight() - tmp.countText.GetHeight() - 10,
            )

            tmp.countText.Show()

        totalElem = len(self.elements)
        if totalElem > 0:
            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            currAdd = 0
            while True:
                if currAdd in self.posMap:
                    currAdd += 1
                    continue
                break

            totalLine = currAdd % self.ELEM_PER_LINE
            currH = math.floor(currAdd / self.ELEM_PER_LINE) * (32 + self.ELEM_PADDING)

            for i in xrange(tmp.itemSize):
                self.posMap[currAdd + i * self.ELEM_PER_LINE] = True

            x = int(1 + totalLine * (36 + self.ELEM_X_PADDING))
            y = int(0 + currH)
            tmp.SetPosition(x, y)

        else:
            for i in xrange(tmp.itemSize):
                self.posMap[i * self.ELEM_PER_LINE] = True

            tmp.SetPosition(1, 0)

        self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            self.peekWindow.GetWidth(),
            max(
                self.scrollBoard.GetHeight(),
                tmp.GetLocalPosition()[1] + tmp.GetHeight(),
            ),
        )
        return tmp


class WikiMainChestWindow(ui.Window):
    class WikiItem(ui.Window):
        # this item is fucking dooomed (since I got the image in one piece)
        def __init__(self, vnum, parent):
            ui.Window.__init__(self)
            self.SetWindowName("MainChestItem")

            wikiBase = wiki.GetBaseClass()
            if vnum not in wikiBase.objList:
                v = wikiBase.objList.setdefault(vnum, {})
                if vnum not in v:
                    v[self.GetWindowName()] = ref(self)

            self.additionalLoaded = False
            self.vnum = vnum
            self.parent = proxy(parent)
            proto = itemManager().GetProto(self.vnum)
            self.base = ui.ExpandedImageBox()
            self.base.SetParent(self)
            self.base.AddFlag("attach")
            self.base.AddFlag("not_pick")
            self.base.LoadImage("d:/ymir work/ui/wiki/detail_chest.tga")
            self.base.Show()

            self.chestImage = ui.ExpandedImageBox()
            self.chestImage.SetParent(self.base)
            self.chestImage.LoadImage(proto.GetIconImageFileName())
            self.chestImage.SetPosition(
                1 + 47 / 2 - self.chestImage.GetWidth() / 2,
                1 + 87 / 2 - self.chestImage.GetHeight() / 2,
            )

            self.chestImage.Show()

            self.chestImage.SetOnMouseOverInEvent(
                Event(self.parent.OnOverIn, self.vnum)
            )
            self.chestImage.SetOnMouseOverOutEvent(self.parent.OnOverOut)

            self.dropList = ChestPeekWindow(self, 401, 66)
            self.dropList.AddFlag("attach")
            self.dropList.SetParent(self.base)
            self.dropList.SetPosition(49, 22)
            self.dropList.Show()
            self.dropList.mOverInEvent = MakeEvent(self.parent.OnOverIn)
            self.dropList.mOverOutEvent = MakeEvent(self.parent.OnOverOut)

            self.originTextHead = ui.TextLine()
            self.originTextHead.SetParent(self.base)
            self.originTextHead.AddFlag("attach")
            self.originTextHead.AddFlag("not_pick")
            self.originTextHead.SetText(localeInfo.WIKI_CHESTINFO_ORIGIN)
            self.originTextHead.SetHorizontalAlignCenter()
            self.originTextHead.SetPosition(
                int(431 + 89 / 2 - self.originTextHead.GetWidth() / 2),
                int(1 + 20 / 2 - self.originTextHead.GetHeight() / 2 - 1),
            )
            self.originTextHead.Show()

            self.contentText = ui.TextLine()
            self.contentText.SetParent(self.base)
            self.contentText.AddFlag("attach")
            # self.contentText.SetFontColor(
            #     228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0
            # )

            self.contentText.AddFlag("not_pick")
            self.contentText.SetText(proto.GetName())
            self.contentText.SetPosition(
                int(49 + 401 / 2 - self.contentText.GetWidth() / 2),
                int(1 + 20 / 2 - self.contentText.GetHeight() / 2 - 1),
            )
            self.contentText.Hide()

            self.originText = ui.TextLine()
            self.originText.SetParent(self.base)
            self.originText.AddFlag("attach")
            self.originText.AddFlag("not_pick")
            self.originText.SetText("-")

            self.originText.SetPosition(
                int(451 + 89 / 2 - self.originText.GetWidth() / 2),
                int(22 + 66 / 2 - self.originText.GetHeight() / 2 - 1),
            )
            self.originText.Show()

            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            self.NoticeMe()

        def __del__(self):
            try:
                if wiki.GetBaseClass():
                    wiki.GetBaseClass().objList[self.vnum].pop(self.GetWindowName())
            except KeyError as e:
                logging.debug("Missing key {}", self.vnum)

        def __GenerateMultiLine(self, text, maxWidth):
            currText = self.__GenerateSingleLine()
            textHolder = []

            tempText = ui.TextLine()
            tempText.Hide()

            splt = text.split(" ")
            currText.SetText(splt[0])
            splt = splt[1:]
            for i in splt:
                tempText.SetText(" " + i)
                if tempText.GetWidth() + currText.GetWidth() > maxWidth:
                    currText.UpdateRect()
                    textHolder.append(currText)
                    currText = self.__GenerateSingleLine()
                    currText.SetText(i)
                else:
                    currText.SetText(currText.GetText() + " " + i)

            textHolder.append(currText)
            return textHolder

        def __GenerateSingleLine(self):
            text = ui.TextLine()
            text.SetParent(self.base)
            text.Show()
            return text

        def NoticeMe(self):
            self.dropList.ClearItems()

            ret = wiki.GetChestInfo(self.vnum)
            if len(ret) == 2:
                (dwOrigin, isCommon) = ret
            else:
                (dwOrigin, isCommon, lst) = ret

            self.originText.Hide()
            self.multiHolder = []
            if self.vnum in wikiInst().GetItemOriginMap():
                self.multiHolder = self.__GenerateMultiLine(
                    wikiInst().GetItemOriginMap()[self.vnum], 66
                )
            elif isCommon:
                self.multiHolder = self.__GenerateMultiLine(
                    localeInfo.WIKI_CHESTINFO_COMMON_DROP, 66
                )
            elif dwOrigin:
                self.multiHolder = self.__GenerateMultiLine(
                    nonplayer.GetMonsterName(dwOrigin), 66
                )
            else:
                self.originText.SetPosition(
                    int(451 + 89 / 2 - self.originText.GetWidth() / 2),
                    int(22 + 66 / 2 - self.originText.GetHeight() / 2 - 1),
                )
                self.originText.Show()

            for i in self.multiHolder:
                totalH = self.multiHolder[0].GetHeight() * len(self.multiHolder) + 3 * (
                    len(self.multiHolder) - 1
                )
                # i.SetFontName("Nunito Sans:10")
                i.SetPosition(
                    int(451 + 89 / 2 - i.GetWidth() / 2),
                    int(
                        22
                        + 66 / 2
                        - totalH / 2
                        + 3 * self.multiHolder.index(i)
                        + i.GetHeight() * self.multiHolder.index(i)
                    ),
                )

            if len(ret) < 3:
                return

            sizeLst = []
            orderedLst = []
            otherStuff = []
            for i in lst:
                if i < 10:
                    otherStuff.append(i)
                    continue

                for j in xrange(i, i + 1):
                    lastPos = 0
                    size = 0
                    itemData = itemManager().GetProto(j)
                    if itemData:
                        size = itemData.GetSize()

                    for k in xrange(len(sizeLst)):
                        if sizeLst[k] < size:
                            break
                        lastPos += 1

                    sizeLst.insert(lastPos, size)
                    orderedLst.insert(lastPos, j)

            for i in orderedLst:
                self.dropList.AddItem(i)

            for i in otherStuff:
                self.dropList.AddItem(i)

        def OnRender(self):
            if not self.additionalLoaded:
                if wiki.IsSet(self.vnum) or self.parent.loadFrom == self.parent.loadTo:
                    self.additionalLoaded = True
                    wiki.LoadInfo(long(id(self)), self.vnum)

    ELEM_PADDING = 5
    SCROLL_SPEED = 50
    ITEM_LOAD_PER_UPDATE = 1

    def __init__(self):
        super(WikiMainChestWindow, self).__init__()

        self.SetSize(mainBoardSize[0], mainBoardSize[1])

        self.elements = []
        self.isOpened = False
        self.loadFrom = 0
        self.loadTo = 0
        self.chestVnums = []
        self.toolTip = None

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(550, self.GetHeight() - 70)
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(550)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 0)
        self.peekWindow.Show()

    def SetItemToolTip(self, t):
        self.toolTip = t

    def OpenWindow(self):
        super(WikiMainChestWindow, self).Show()

    def Show(self, vnums):
        super(WikiMainChestWindow, self).Show()
        self.peekWindow.ScrollToStart()

        isChanged = not len(vnums) == len(self.chestVnums)
        if not isChanged:
            for i in vnums:
                if i not in self.chestVnums:
                    isChanged = True
                    break

        if not isChanged:
            for i in self.chestVnums:
                if i not in vnums:
                    isChanged = True
                    break

        if not len(vnums) and enableDebugThings:
            self.loadTo = wiki.LoadClassItems(8, 0)
            del self.chestVnums[:]
        else:
            self.chestVnums = vnums[:]
            self.loadTo = len(self.chestVnums)

        if not self.isOpened:
            self.isOpened = True
            self.loadFrom = 0

        if self.loadFrom > self.loadTo or isChanged:
            del self.elements[:]
            self.loadFrom = 0
            self.scrollBoard.SetSize(0, 0)

    def OnOverIn(
        self, vnum, metinSlot=None, displayRefinedVnum=False, count=None, rarity=None
    ):
        if metinSlot is None:
            metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
        self.toolTip.ClearToolTip()
        self.toolTip.AddItemData(vnum, metinSlot, 0, showRefinedVnum=displayRefinedVnum)
        if count:
            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_DROP_COUNT").format(count)
            )
        if rarity:
            from uiTarget import FormatRarity

            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_RARITY").format(FormatRarity(rarity))
            )

    def OnOverOut(self):
        self.toolTip.Hide()

    def OnUpdate(self):
        if self.loadFrom < self.loadTo:
            if not len(self.chestVnums) and enableDebugThings:
                for i in wiki.ChangePage(
                    self.loadFrom,
                    min(self.loadTo, self.loadFrom + self.ITEM_LOAD_PER_UPDATE),
                ):
                    self.AddItem(i)
                    self.loadFrom += 1
            else:
                for i in xrange(
                    self.loadFrom,
                    min(self.loadTo, self.loadFrom + self.ITEM_LOAD_PER_UPDATE),
                ):
                    self.AddItem(self.chestVnums[i])
                    self.loadFrom += 1

    def AddItem(self, vnum):
        for i in self.elements:
            if vnum == i.vnum:
                return None
        proto = itemManager().GetProto(vnum)
        if not proto:
            return None

        tmp = self.WikiItem(vnum, self)
        tmp.SetParent(self.scrollBoard)
        tmp.AddFlag("attach")

        totalElem = len(self.elements)
        addPadding = 0
        if totalElem > 0:
            lastIndex = totalElem
            """for i in xrange(totalElem):
                if self.elements[i].vnum < tmp.vnum:
                    break
                lastIndex += 1"""

            self.elements.insert(lastIndex, tmp)
            totalElem += 1

            for i in xrange(lastIndex, totalElem):
                if i == 0:
                    self.elements[i].SetPosition(0, 0)
                else:
                    self.elements[i].SetPosition(
                        0,
                        self.elements[i - 1].GetLocalPosition()[1]
                        + self.elements[i - 1].GetHeight()
                        + self.ELEM_PADDING,
                    )

            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            addPadding = self.ELEM_PADDING

        else:
            self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            tmp.GetWidth(), self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight()
        )
        return tmp


class WikiMainBossWindow(ui.Window):
    class WikiItem(ui.Window):
        # this item is fucking dooomed (since I got the image in one piece)
        def __init__(self, vnum, parent):
            ui.Window.__init__(self)
            self.SetWindowName("MainWikiItem")

            wikiBase = wiki.GetBaseClass()
            if vnum not in wikiBase.objList:
                v = wikiBase.objList.setdefault(vnum, {})
                if vnum not in v:
                    v[self.GetWindowName()] = ref(self)

            self.additionalLoaded = False
            self.vnum = vnum
            self.parent = proxy(parent)

            self.base = ui.ExpandedImageBox()
            self.base.SetParent(self)
            self.base.AddFlag("attach")
            self.base.AddFlag("not_pick")
            self.base.LoadImage("d:/ymir work/ui/wiki/detail_chest.tga")
            self.base.Show()

            self.modelView = WikiRenderTarget(47, 87)
            self.modelView.SetParent(self.base)
            self.modelView.RemoveFlag("attach")
            self.modelView.AddFlag("dragable")
            self.modelView.SetPosition(
                1 + 47 / 2 - self.modelView.GetWidth() / 2,
                1 + 87 / 2 - self.modelView.GetHeight() / 2,
            )

            if wiki.GetBaseClass():
                self.modelView.SetMouseLeftButtonDownEvent(
                    Event(
                        wiki.GetBaseClass().OpenSpecialPage,
                        self.parent,
                        self.vnum,
                        True,
                    )
                )

            self.dropList = ChestPeekWindow(self, 401, 66)
            self.dropList.AddFlag("attach")
            self.dropList.SetParent(self.base)
            self.dropList.SetPosition(49, 22)
            self.dropList.Show()
            self.dropList.mOverInEvent = MakeEvent(self.parent.OnOverIn)
            self.dropList.mOverOutEvent = MakeEvent(self.parent.OnOverOut)

            self.originTextHead = ui.TextLine()
            self.originTextHead.SetParent(self.base)
            self.originTextHead.AddFlag("attach")
            self.originTextHead.AddFlag("not_pick")
            self.originTextHead.SetText(localeInfo.WIKI_CHESTINFO_ORIGIN)
            self.originTextHead.SetHorizontalAlignCenter()
            self.originTextHead.SetPosition(
                int(431 + 89 / 2 - self.originTextHead.GetWidth() / 2),
                int(1 + 20 / 2 - self.originTextHead.GetHeight() / 2 - 1),
            )
            self.originTextHead.Show()

            self.contentText = ui.TextLine()
            self.contentText.SetParent(self.base)
            self.contentText.AddFlag("attach")
            self.contentText.AddFlag("not_pick")
            self.contentText.SetHorizontalAlignCenter()
            self.contentText.SetText(nonplayer.GetMonsterName(self.vnum))
            self.contentText.SetPosition(
                0, int(1 + 20 / 2 - self.contentText.GetHeight() / 2 - 1)
            )
            self.contentText.Show()

            self.originText = ui.TextLine()
            self.originText.SetParent(self.base)
            self.originText.AddFlag("attach")
            self.originText.AddFlag("not_pick")
            self.originText.SetPosition(
                int(451 + 89 / 2 - self.originText.GetWidth() / 2),
                int(22 + 66 / 2 - self.originText.GetHeight() / 2 - 1),
            )
            self.originText.Show()

            self.SetSize(self.base.GetWidth(), self.base.GetHeight())
            self.NoticeMe()

        def __del__(self):
            try:
                if wiki.GetBaseClass():
                    wiki.GetBaseClass().objList[self.vnum].pop(self.GetWindowName())
            except KeyError as e:
                logging.debug("Missing key {}", self.vnum)

        def OnUpdate(self):
            if not self.additionalLoaded:
                if not wiki.IsSet(self.vnum, True):
                    wiki.LoadInfo(long(id(self)), self.vnum, True)
                else:
                    self.modelView.SetModel(self.vnum)
                    self.modelView.Show()
                    self.NoticeMe()
                    self.additionalLoaded = True

        def __GenerateMultiLine(self, text, maxWidth):
            currText = self.__GenerateSingleLine()
            textHolder = []

            tempText = ui.TextLine()
            # tempText.SetPackedFontColor(0xFFA46E28)
            tempText.Hide()

            splt = text.split(" ")
            currText.SetText(localeInfo.Get(splt[0]))
            splt = splt[1:]
            for i in splt:
                tempText.SetText(" " + localeInfo.Get(i))
                if tempText.GetWidth() + currText.GetWidth() > maxWidth:
                    currText.UpdateRect()
                    textHolder.append(currText)
                    currText = self.__GenerateSingleLine()
                    currText.SetText(localeInfo.Get(i))
                else:
                    currText.SetText(currText.GetText() + " " + localeInfo.Get(i))

            textHolder.append(currText)
            return textHolder

        def __GenerateSingleLine(self):
            text = ui.TextLine()
            text.SetParent(self.base)
            text.Show()
            return text

        def NoticeMe(self):
            lst = wiki.GetMobInfo(self.vnum)

            self.originText.Hide()
            self.multiHolder = []
            if self.vnum in wikiInst().GetMobOriginMap():
                self.multiHolder = self.__GenerateMultiLine(
                    wikiInst().GetMobOriginMap()[self.vnum], 66
                )
            else:
                self.originText.SetPosition(
                    int(451 + 89 / 2 - self.originText.GetWidth() / 2),
                    int(22 + 66 / 2 - self.originText.GetHeight() / 2 - 1),
                )
                self.originText.Show()

            for i in self.multiHolder:
                totalH = self.multiHolder[0].GetHeight() * len(self.multiHolder) + 3 * (
                    len(self.multiHolder) - 1
                )
                i.SetPosition(
                    int(451 + 89 / 2 - i.GetWidth() / 2),
                    int(
                        22
                        + 66 / 2
                        - totalH / 2
                        + 3 * self.multiHolder.index(i)
                        + i.GetHeight() * self.multiHolder.index(i)
                    ),
                )

            if not lst:
                return

            self.dropList.ClearItems()

            for i in lst:
                count = i[3] if i[3] > 1 else 0
                self.dropList.AddItem(i[0], count, i[4])

    ELEM_PADDING = 5
    SCROLL_SPEED = 50
    ITEM_LOAD_PER_UPDATE = 2

    def __init__(self):
        super(WikiMainBossWindow, self).__init__()

        self.SetSize(mainBoardSize[0], mainBoardSize[1])

        self.elements = []
        self.isOpened = False
        self.loadFrom = 0
        self.loadTo = 0
        self.mobtypes = 0
        self.fromlvl = 0
        self.tolvl = 0
        self.toolTip = None

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(550, self.GetHeight() - 70)
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(550)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 0)
        self.peekWindow.Show()

    def SetItemToolTip(self, t):
        self.toolTip = t

    def OpenWindow(self):
        super(WikiMainBossWindow, self).Show()

    def Show(self, mobtypes, fromlvl, tolvl):
        super(WikiMainBossWindow, self).Show()
        self.peekWindow.ScrollToStart()

        isChanged = False
        if not (
            mobtypes == self.mobtypes
            and fromlvl == self.fromlvl
            and tolvl == self.tolvl
        ):
            isChanged = True
        self.mobtypes = mobtypes
        self.fromlvl = fromlvl
        self.tolvl = tolvl

        self.loadTo = wiki.LoadClassMobs(mobtypes, fromlvl, tolvl)
        if not self.isOpened:
            self.isOpened = True
            self.loadFrom = 0

        if self.loadFrom > self.loadTo or isChanged:
            del self.elements[:]
            self.loadFrom = 0
            self.scrollBoard.SetSize(0, 0)

    def Hide(self):
        super(WikiMainBossWindow, self).Hide()

    def OnOverIn(
        self, vnum, metinSlot=None, displayRefinedVnum=False, count=None, rarity=None
    ):
        if metinSlot is None:
            metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
        self.toolTip.ClearToolTip()
        self.toolTip.AddItemData(vnum, metinSlot, 0, showRefinedVnum=displayRefinedVnum)
        if count:
            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_DROP_COUNT").format(count)
            )
        if rarity:
            from uiTarget import FormatRarity

            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_RARITY").format(FormatRarity(rarity))
            )

    def OnOverOut(self):
        self.toolTip.Hide()

    def OnUpdate(self):
        if self.loadFrom < self.loadTo:
            for i in wiki.ChangePage(
                self.loadFrom,
                min(self.loadTo, self.loadFrom + self.ITEM_LOAD_PER_UPDATE),
                True,
            ):
                self.AddItem(i)
                self.loadFrom += 1

    def AddItem(self, vnum):
        for i in self.elements:
            if vnum == i.vnum:
                return None

        tmp = self.WikiItem(vnum, self)
        tmp.SetParent(self.scrollBoard)
        tmp.AddFlag("attach")

        totalElem = len(self.elements)
        addPadding = 0
        if totalElem > 0:
            lastIndex = 0
            for i in xrange(totalElem):
                if self.elements[i].vnum < tmp.vnum:
                    break
                lastIndex += 1

            self.elements.insert(lastIndex, tmp)
            totalElem += 1

            for i in xrange(lastIndex, totalElem):
                if i == 0:
                    self.elements[i].SetPosition(0, 0)
                else:
                    self.elements[i].SetPosition(
                        0,
                        self.elements[i - 1].GetLocalPosition()[1]
                        + self.elements[i - 1].GetHeight()
                        + self.ELEM_PADDING,
                    )

            addPadding = self.ELEM_PADDING

        else:
            self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            tmp.GetWidth(), self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight()
        )
        return tmp


class WikiMonsterBonusInfoWindow(ui.Window):
    ELEM_PADDING = 5
    SCROLL_SPEED = 50
    ITEM_LOAD_PER_UPDATE = 2

    RACE_FLAG_TO_NAME = {
        nonplayer.RACE_FLAG_ANIMAL: localeInfo.TARGET_INFO_RACE_ANIMAL,
        nonplayer.RACE_FLAG_UNDEAD: localeInfo.TARGET_INFO_RACE_UNDEAD,
        nonplayer.RACE_FLAG_DEVIL: localeInfo.TARGET_INFO_RACE_DEVIL,
        nonplayer.RACE_FLAG_HUMAN: localeInfo.TARGET_INFO_RACE_HUMAN,
        nonplayer.RACE_FLAG_ORC: localeInfo.TARGET_INFO_RACE_ORC,
        nonplayer.RACE_FLAG_MILGYO: localeInfo.TARGET_INFO_RACE_MILGYO,
        nonplayer.RACE_FLAG_TREE: localeInfo.TARGET_INFO_RACE_TREE,
        nonplayer.RACE_FLAG_CZ: localeInfo.TARGET_INFO_RACE_ZODIAC,
    }
    SUB_RACE_FLAG_TO_NAME = {
        nonplayer.RACE_FLAG_ATT_ELEC: localeInfo.TARGET_INFO_RACE_ELEC,
        nonplayer.RACE_FLAG_ATT_FIRE: localeInfo.TARGET_INFO_RACE_FIRE,
        nonplayer.RACE_FLAG_ATT_ICE: localeInfo.TARGET_INFO_RACE_ICE,
        nonplayer.RACE_FLAG_ATT_WIND: localeInfo.TARGET_INFO_RACE_WIND,
        nonplayer.RACE_FLAG_ATT_EARTH: localeInfo.TARGET_INFO_RACE_EARTH,
        nonplayer.RACE_FLAG_ATT_DARK: localeInfo.TARGET_INFO_RACE_DARK,
    }
    IMMUNE_FLAG_TO_NAME = {
        nonplayer.IMMUNE_STUN: localeInfo.WIKI_MONSTERINFO_IMMUNE_STUN,
        nonplayer.IMMUNE_SLOW: localeInfo.WIKI_MONSTERINFO_IMMUNE_SLOW,
        nonplayer.IMMUNE_FALL: localeInfo.WIKI_MONSTERINFO_IMMUNE_FALL,
        nonplayer.IMMUNE_CURSE: localeInfo.WIKI_MONSTERINFO_IMMUNE_CURSE,
        nonplayer.IMMUNE_POISON: localeInfo.WIKI_MONSTERINFO_IMMUNE_POISON,
        nonplayer.IMMUNE_TERROR: localeInfo.WIKI_MONSTERINFO_IMMUNE_TERROR,
        nonplayer.IMMUNE_REFLECT: localeInfo.WIKI_MONSTERINFO_IMMUNE_REFLECT,
    }

    def __init__(self, vnum):
        super(WikiMonsterBonusInfoWindow, self).__init__()

        self.SetSize(*monsterBonusInfoPageSize)
        self.vnum = vnum

        self.elements = []

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(self.GetWidth() - 8 - 6, self.GetHeight() + 10)
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(self.GetWidth() - 8)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(3, 0)
        self.peekWindow.Show()

        self.AddItem(
            localeInfo.WIKI_MONSTERINFO_LEVEL.format(
                nonplayer.GetMonsterLevel(self.vnum)
            )
        )
        (mainrace, subrace) = self.GetRaceStrings()
        self.AddItem(
            localeInfo.TARGET_INFO_MAINRACE.format(
                mainrace + " | " + localeInfo.TARGET_INFO_SUBRACE.format(subrace)
            )
        )
        self.AddItem(
            localeInfo.WIKI_MONSTERINFO_IMMUNE_TO.format(self.GetImmuneString())
        )
        (damageMin, damageMax) = nonplayer.GetMonsterDamage(self.vnum)
        damageMin = localeInfo.MoneyFormat(damageMin)
        damageMax = localeInfo.MoneyFormat(damageMax)
        self.AddItem(
            localeInfo.WIKI_MONSTERINFO_DMG_HP.format(
                damageMin,
                damageMax,
                localeInfo.MoneyFormat(nonplayer.GetMonsterMaxHP(self.vnum)),
            )
        )
        (goldMin, goldMax) = nonplayer.GetMonsterGold(self.vnum)
        goldMin = localeInfo.MoneyFormat(goldMin)
        goldMax = localeInfo.MoneyFormat(goldMax)
        self.AddItem(
            localeInfo.WIKI_MONSTERINFO_GOLD_EXP.format(
                goldMin,
                goldMax,
                localeInfo.MoneyFormat(nonplayer.GetMonsterExp(self.vnum)),
            )
        )
        self.AddItem(localeInfo.WIKI_MONSTERINFO_RESISTANCES)
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_SWORD,
                nonplayer.GetMonsterResistValue(self.vnum, nonplayer.MOB_RESIST_SWORD),
            ),
            # 15
        )
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_TWOHAND,
                nonplayer.GetMonsterResistValue(
                    self.vnum, nonplayer.MOB_RESIST_TWOHAND
                ),
            ),
            # 15
        )
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_DAGGER,
                nonplayer.GetMonsterResistValue(self.vnum, nonplayer.MOB_RESIST_DAGGER),
            ),
            # 15
        )
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_BELL,
                nonplayer.GetMonsterResistValue(self.vnum, nonplayer.MOB_RESIST_BELL),
            ),
            # 15
        )
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_FAN,
                nonplayer.GetMonsterResistValue(self.vnum, nonplayer.MOB_RESIST_FAN),
            ),
            # 15
        )
        self.AddItem(
            FormatApplyAsString(
                item.APPLY_RESIST_BOW,
                nonplayer.GetMonsterResistValue(self.vnum, nonplayer.MOB_RESIST_BOW),
            ),
            # 15
        )
        self.scrollBoard.SetSize(self.GetWidth(), self.scrollBoard.GetHeight() + 10)

    def GetImmuneString(self):
        dwImmuneFlag = nonplayer.GetMonsterImmuneFlag(self.vnum)
        immuneflags = ""

        for i in xrange(nonplayer.IMMUNE_FLAG_MAX_NUM):
            curFlag = 1 << i
            if HAS_FLAG(dwImmuneFlag, curFlag):
                if curFlag in self.IMMUNE_FLAG_TO_NAME:
                    immuneflags += self.IMMUNE_FLAG_TO_NAME[curFlag] + ", "

        if immuneflags == "":
            immuneflags = localeInfo.WIKI_MONSTERINFO_IMMUNE_NOTHING
        else:
            immuneflags = immuneflags[:-2]

        return immuneflags

    def GetRaceStrings(self):
        raceFlag = nonplayer.GetMonsterRaceFlag(self.vnum)

        mainrace = ""
        subrace = ""
        for i in xrange(nonplayer.RACE_FLAG_MAX_NUM):
            curFlag = 1 << i
            if (raceFlag & curFlag) == curFlag:

                if curFlag in localeInfo.RACE_FLAG_TO_NAME:
                    mainrace += localeInfo.RACE_FLAG_TO_NAME[curFlag] + ", "
                elif curFlag in localeInfo.SUB_RACE_FLAG_TO_NAME:
                    subrace += localeInfo.SUB_RACE_FLAG_TO_NAME[curFlag] + ", "

        if nonplayer.IsMonsterStone(self.vnum):
            mainrace += localeInfo.TARGET_INFO_RACE_METIN + ", "

        if mainrace == "":
            mainrace = localeInfo.TARGET_INFO_NO_RACE
        else:
            mainrace = mainrace[:-2]

        if subrace == "":
            subrace = localeInfo.TARGET_INFO_NO_RACE
        else:
            subrace = subrace[:-2]

        return (mainrace, subrace)

    def AddItem(self, text, padding=0):
        tmp = ui.Window()
        tmp.SetParent(self.scrollBoard)
        tmp.AddFlag("attach")
        tmp.SetSize(self.GetWidth() - 8, 15)

        img = ui.ExpandedImageBox()
        img.SetParent(tmp)
        img.AddFlag("attach")
        img.AddFlag("not_pick")
        img.LoadImage("d:/ymir work/ui/wiki/arrow_2.tga")
        img.SetPosition(int(padding), int(tmp.GetHeight() / 2 - img.GetHeight() / 2))
        img.Show()
        tmp.img = img

        txt = ui.TextLine()
        txt.SetParent(tmp)
        txt.AddFlag("attach")
        txt.AddFlag("not_pick")
        fnt = localeInfo.UI_DEF_FONT_NUNITO.split(":")
        # txt.SetFontName(fnt[0] + ":" + str(int(fnt[1]) + 2))
        txt.SetText(text)
        txt.SetPosition(
            img.GetLocalPosition()[0] + img.GetWidth() + 5,
            tmp.GetHeight() / 2 - txt.GetHeight() / 2 - 1,
        )

        txt.Show()
        tmp.txt = txt

        totalElem = len(self.elements)
        addPadding = 0
        if totalElem > 0:
            lastIndex = totalElem

            self.elements.insert(lastIndex, tmp)
            totalElem += 1

            for i in xrange(lastIndex, totalElem):
                if i == 0:
                    self.elements[i].SetPosition(0, 0)
                else:
                    self.elements[i].SetPosition(
                        0,
                        self.elements[i - 1].GetLocalPosition()[1]
                        + self.elements[i - 1].GetHeight()
                        + self.ELEM_PADDING,
                    )

            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            addPadding = self.ELEM_PADDING

        else:
            self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            tmp.GetWidth(), self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight()
        )
        return tmp


class WikiItemOriginInfo(ui.Window):
    ELEM_PADDING = 5
    SCROLL_SPEED = 50
    ITEM_LOAD_PER_UPDATE = 2

    def __init__(self, vnum):
        super(WikiItemOriginInfo, self).__init__()

        self.SetSize(*itemOriginPageSize)
        self.vnum = vnum
        self.elements = []

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(self.GetWidth() - 8 - 6, self.GetHeight() - 85)
        self.scrollBoard = ui.Window()

        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(self.GetWidth() - 8 - 6)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(3, 3)
        self.peekWindow.Show()
        if self.vnum in wikiInst().GetItemOriginMap():
            self.AddItem(0, False, wikiInst().GetItemOriginMap()[self.vnum])

        for proto in itemManager().GetRefinedFromItems(self.vnum):
            self.AddItem(
                proto.GetVnum(),
                False,
                localeInfo.Get("TOOLTIP_REFINE_FROM").format(proto.GetName()),
            )

        self.ParseTextlines()

    def ParseTextlines(self):
        lst = wiki.GetOriginInfo(self.vnum)
        if not lst:
            return

        self.ClearItems()
        if self.vnum in wikiInst().GetItemOriginMap():
            self.AddItem(0, False, wikiInst().GetItemOriginMap()[self.vnum])
        alreadyParsed = []

        for vnum, isMonster in lst:
            bAlready = False
            for i in alreadyParsed:
                if i[0] == vnum and i[1] == isMonster:
                    bAlready = True
                    break
            if bAlready:
                continue
            if isMonster:
                currName = nonplayer.GetMonsterName(vnum)
            else:
                currName = ""
                proto = itemManager().GetProto(vnum)
                if proto:
                    currName = proto.GetName()

            alreadyParsed.append([vnum, isMonster])
            self.AddItem(vnum, isMonster, currName)

    def AddItem(self, vnum, isMonster, text, padding=0):
        tmp = ui.WindowWithBaseEvents()
        tmp.SetParent(self.scrollBoard)
        tmp.SetSize(self.GetWidth() - 8, 15)
        tmp.SetMouseLeftButtonDownEvent(
            Event(wiki.GetBaseClass().OpenSpecialPage, None, vnum, isMonster)
        )

        img = ui.ExpandedImageBox()
        img.SetParent(tmp)
        img.AddFlag("attach")
        img.AddFlag("not_pick")
        img.LoadImage("d:/ymir work/ui/wiki/arrow.tga")
        img.SetPosition(int(padding), int(tmp.GetHeight() / 2 - img.GetHeight() / 2))
        img.Show()
        tmp.img = img

        txt = ui.TextLine()
        txt.SetParent(tmp)
        txt.AddFlag("attach")
        txt.AddFlag("not_pick")
        fnt = localeInfo.UI_DEF_FONT_NUNITO.split(":")
        txt.SetFontName(fnt[0] + ":" + str(int(fnt[1]) + 2))
        txt.SetText(text)
        txt.SetPosition(
            int(img.GetLocalPosition()[0] + img.GetWidth() + 5),
            int(tmp.GetHeight() / 2 - txt.GetHeight() / 2 - 1),
        )
        txt.Show()
        tmp.txt = txt

        totalElem = len(self.elements)
        addPadding = 0
        if totalElem > 0:
            lastIndex = totalElem

            self.elements.insert(lastIndex, tmp)
            totalElem += 1

            for i in xrange(lastIndex, totalElem):
                if i == 0:
                    self.elements[i].SetPosition(0, 0)
                else:
                    self.elements[i].SetPosition(
                        0,
                        self.elements[i - 1].GetLocalPosition()[1]
                        + self.elements[i - 1].GetHeight()
                        + self.ELEM_PADDING,
                    )

            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            addPadding = self.ELEM_PADDING

        else:
            self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            tmp.GetWidth(), self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight()
        )
        return tmp

    def ClearItems(self):
        self.elements = []


class SpecialPageWindow(ui.Window):
    def __init__(self, vnum, isMonster):
        super(SpecialPageWindow, self).__init__()
        self.SetWindowName("SpecialPageWindow")
        wiki.LoadInfo(long(id(self)), vnum, isMonster)
        self.EnableClipping()

        wikiBase = wiki.GetBaseClass()
        if vnum not in wikiBase.objList:
            v = wikiBase.objList.setdefault(vnum, {})
            if vnum not in v:
                v[self.GetWindowName()] = ref(self)

        self.realVnum = vnum
        if not isMonster:
            itemData = itemManager().GetProto(vnum)
            if (
                itemData.GetType() == item.ITEM_TYPE_WEAPON
                or itemData.GetType() == item.ITEM_TYPE_ARMOR
                or itemData.GetType() == item.ITEM_TYPE_TALISMAN
                or itemData.GetType() == item.ITEM_TYPE_BELT
            ):
                self.vnum = int(vnum / 10) * 10
            else:
                self.vnum = vnum
        else:
            self.vnum = vnum
        itemData = None if isMonster else itemManager().GetProto(self.vnum)

        self.isMonster = isMonster
        self.toolTip = None
        self.EnableClipping()
        self.isChest = (
            True if itemData and itemData.GetType() == item.ITEM_TYPE_GIFTBOX else False
        )
        self.additionalLoaded = False
        self.hasNoBonusInfo = False

        self.bg = ui.ExpandedImageBox()
        self.bg.SetParent(self)
        self.bg.AddFlag("attach")
        self.bg.AddFlag("not_pick")
        self.bg.EnableClipping()
        # if self.isMonster or self.isChest:
        self.bg.LoadImage("d:/ymir work/ui/wiki/detail_monster.tga")
        # else:
        #     self.bg.LoadImage("d:/ymir work/ui/wiki/detail_item_special.png")
        self.bg.Show()

        self.subTitleText1 = ui.TextLine()
        self.subTitleText1.SetParent(self.bg)
        self.subTitleText1.AddFlag("attach")
        self.subTitleText1.AddFlag("not_pick")
        self.subTitleText1.SetHorizontalAlignCenter()
        self.subTitleText1.Show()

        self.subTitleText2 = ui.TextLine()
        self.subTitleText2.SetParent(self.bg)
        self.subTitleText2.AddFlag("attach")
        self.subTitleText2.AddFlag("not_pick")
        self.subTitleText2.Hide()

        if self.isMonster:

            self.subTitleText1.SetText(nonplayer.GetMonsterName(self.vnum))
            self.subTitleText2.SetText(
                localeInfo.WIKI_MONSTERINFO_STATISTICSOF.format(
                    nonplayer.GetMonsterName(self.vnum)
                )
            )

            self.modelView = WikiRenderTarget(163, 163)
            self.modelView.SetParent(self.bg)
            self.modelView.AddFlag("dragable")
            self.modelView.RemoveFlag("attach")
            self.modelView.SetPosition(int(1 + 187 / 2 - 163 / 2), 1)
            self.modelView.SetModel(vnum)
            self.modelView.Show()

            self.itemContainer = ChestPeekWindow(self, 351, 142, True)
            self.itemContainer.ELEM_PER_LINE = 10
            self.itemContainer.ELEM_X_PADDING = -2
            self.itemContainer.AddFlag("attach")
            self.itemContainer.SetParent(self.bg)
            self.itemContainer.SetPosition(189, 22)
            self.itemContainer.Show()
            self.itemContainer.mOverInEvent = MakeEvent(self.OnOverIn)
            self.itemContainer.mOverOutEvent = MakeEvent(self.OnOverOut)
            # .LoadDropData()

            self.bonusInfo = WikiMonsterBonusInfoWindow(self.vnum)
            self.bonusInfo.SetParent(self.bg)
            self.bonusInfo.AddFlag("attach")
            self.bonusInfo.SetPosition(1, 188)
            self.bonusInfo.Show()

        else:

            if item.ITEM_TYPE_GIFTBOX == itemData.GetType():
                self.bonusPeekWindow = ui.ScrollWindow()
                self.bonusPeekWindow.SetSize(540, 230)
                self.bonusPeekWindow.SetPosition(0, 0)

                from ui_modern_controls import AutoGrowingVerticalContainerEx

                self.bonusScrollBoard = AutoGrowingVerticalContainerEx()
                self.bonusScrollBoard.SetWidth(540)
                # self.bonusScrollBoard.SetPosition(3, 200)

                self.bonusInfo = ChestPeekWindow(self, 540, 142, True)
                self.bonusInfo.ELEM_PER_LINE = 15
                self.bonusInfo.ELEM_X_PADDING = -2
                self.bonusInfo.SetParent(self.bonusScrollBoard)
                self.bonusInfo.AddFlag("attach")
                self.bonusInfo.mOverInEvent = MakeEvent(self.OnOverIn)
                self.bonusInfo.mOverOutEvent = MakeEvent(self.OnOverOut)
                self.bonusScrollBoard.AppendItem(self.bonusInfo)

                self.bonusPeekWindow.SetContentWindow(self.bonusScrollBoard)
                self.bonusPeekWindow.AutoFitWidth()
                self.bonusPeekWindow.SetParent(self)
                self.bonusPeekWindow.SetPosition(3, 200)
                self.bonusPeekWindow.Show()
                self.bonusInfo.Show()

                self.isChest = True
                self.LoadChestDropData()
                self.subTitleText2.SetText(
                    localeInfo.WIKI_MONSTERINFO_DROPLISTOF.format(itemData.GetName())
                )
                self.subTitleText2.SetPosition(155, 170)

            elif (
                itemData.GetType()
                in (
                    item.ITEM_TYPE_WEAPON,
                    item.ITEM_TYPE_ARMOR,
                    item.ITEM_TYPE_RING,
                    item.ITEM_TYPE_TALISMAN,
                    item.ITEM_TYPE_BELT,
                )
                or (
                    itemData.GetType() == item.ITEM_TYPE_COSTUME
                    and itemData.GetSubType() == item.COSTUME_TYPE_ACCE
                )
                or (
                    itemData.GetType() == item.ITEM_TYPE_TOGGLE
                    and itemData.GetSubType() == item.TOGGLE_AFFECT
                )
            ):
                self.bonusPeekWindow = ui.ScrollWindow()
                self.bonusPeekWindow.SetSize(540, 185)
                self.bonusPeekWindow.SetPosition(0, 0)

                from ui_modern_controls import AutoGrowingVerticalContainerEx

                self.bonusScrollBoard = AutoGrowingVerticalContainerEx()
                self.bonusScrollBoard.SetWidth(540 + 27)

                self.bonusInfo = WikiMainWeaponWindow.WikiItem(self.vnum, self)
                self.bonusInfo.SetParent(self.bonusScrollBoard)
                self.bonusInfo.AddFlag("attach")
                self.bonusScrollBoard.AppendItem(self.bonusInfo)

                self.bonusPeekWindow.SetContentWindow(self.bonusScrollBoard)
                self.bonusPeekWindow.AutoFitWidth()
                self.bonusPeekWindow.SetPosition(0, 0)

                self.bonusPeekWindow.SetParent(self)
                self.bonusPeekWindow.SetPosition(0, 167)
                self.bonusPeekWindow.Show()
                self.bonusInfo.Show()
                self.subTitleText2.SetPosition(
                    int(1 + 539 / 2 - self.subTitleText2.GetWidth() / 2),
                    int(165 + 11 - self.subTitleText2.GetHeight() / 2),
                )

            else:
                self.hasNoBonusInfo = True
                self.subTitleText2.SetPosition(
                    int(1 + 539 / 2 - self.subTitleText2.GetWidth() / 2),
                    int(165 + 11 - self.subTitleText2.GetHeight() / 2),
                )

            self.subTitleText1.SetText(localeInfo.WIKI_ITEMINFO_OPTAINEDFROM)

            self.modelView = ui.ExpandedImageBox()
            self.modelView.SetParent(self.bg)
            self.modelView.AddFlag("attach")
            self.modelView.LoadImage(itemData.GetIconImageFileName())
            self.modelView.SetPosition(
                int(1 + 167 / 2 - self.modelView.GetWidth() / 2), 50
            )
            self.modelView.SetOnMouseOverInEvent(Event(self.OnOverIn, self.vnum))
            self.modelView.SetOnMouseOverOutEvent(self.OnOverOut)
            self.modelView.Show()

            self.originInfo = WikiItemOriginInfo(self.realVnum)
            self.originInfo.SetParent(self.bg)
            self.originInfo.AddFlag("attach")
            self.originInfo.SetPosition(189, 22)
            self.originInfo.Show()

        self.subTitleText1.SetPosition(
            0, int(1 + 10 - self.subTitleText1.GetHeight() / 2)
        )

        self.NoticeMe()

    def __del__(self):
        try:
            if wiki.GetBaseClass():
                wiki.GetBaseClass().objList[self.vnum].pop(self.GetWindowName())
        except KeyError as e:
            logging.debug("Missing key {}", self.vnum)

    def SetItemToolTip(self, t):
        self.toolTip = t

    def ChangeElementSize(self, elem, sizeDiff):
        self.bonusScrollBoard.SetSize(
            self.bonusScrollBoard.GetWidth(),
            self.bonusScrollBoard.GetHeight() + sizeDiff,
        )

    def OnOverIn(
        self, vnum, metinSlot=None, displayRefinedVnum=False, count=None, rarity=None
    ):
        if not self.toolTip:
            return False
        try:
            if metinSlot is None:
                metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]

            self.toolTip.ClearToolTip()
            self.toolTip.AddItemData(
                vnum, metinSlot, 0, showRefinedVnum=displayRefinedVnum
            )
            if count:
                self.toolTip.AppendTextLine(
                    localeInfo.Get("TOOLTIP_DROP_COUNT").format(count)
                )
            if rarity:
                from uiTarget import FormatRarity

                self.toolTip.AppendTextLine(
                    localeInfo.Get("TOOLTIP_RARITY").format(FormatRarity(rarity))
                )

        except Exception as e:
            pass

    def OnOverOut(self):
        try:
            if self.toolTip:
                self.toolTip.Hide()
        except Exception as e:
            pass

    def OnRender(self):
        if not self.isMonster and not self.additionalLoaded and wiki.IsSet(self.vnum):
            self.additionalLoaded = True
            self.originInfo.ParseTextlines()

    def OpenWindow(self):
        super(SpecialPageWindow, self).Show()

    def NoticeMe(self):
        if self.isMonster:
            self.LoadMonsterDropData()
        elif self.isChest:
            self.LoadChestDropData()
        if not self.isMonster and not self.isChest and not self.hasNoBonusInfo:
            self.bonusInfo.NoticeMe()

    def LoadChestDropData(self):
        self.bonusInfo.ClearItems()
        ret = wiki.GetChestInfo(self.vnum)
        if len(ret) == 2:
            (dwOrigin, isCommon) = ret
        else:
            (dwOrigin, isCommon, lst) = ret

        if len(ret) < 3:
            return

        sizeLst = []
        orderedLst = []
        otherStuff = []
        for i in lst:
            if i < 10:
                otherStuff.append(i)
                continue

            for j in xrange(i, i + 1):
                lastPos = 0
                size = 0
                itemData = itemManager().GetProto(j)
                if itemData:
                    size = itemData.GetSize()

                for k in xrange(len(sizeLst)):
                    if sizeLst[k] < size:
                        break
                    lastPos += 1

                sizeLst.insert(lastPos, size)
                orderedLst.insert(lastPos, j)

        for i in orderedLst:
            self.bonusInfo.AddItem(i)

        for i in otherStuff:
            self.bonusInfo.AddItem(i)

    def LoadMonsterDropData(self):
        if not self.isMonster:
            return

        if not wiki.IsSet(self.vnum, True):
            wiki.LoadInfo(long(id(self)), self.vnum, True)
            return

        lst = wiki.GetMobInfo(self.vnum)
        if not lst:
            return

        self.itemContainer.ClearItems()

        for i in lst:
            self.itemContainer.AddItem(i[0], i[3], i[4])

    def Show(self):
        super(SpecialPageWindow, self).Show()

    def Hide(self):
        super(SpecialPageWindow, self).Hide()


def GetColorFromString(strCol):
    hCol = long(strCol, 16)
    retData = []
    dNum = 4
    if hCol <= 0xFFFFFF:
        retData.append(1.0)
        dNum = 3

    for i in xrange(dNum):
        retData.append(float((hCol >> (8 * i)) & 0xFF) / 255.0)

    retData.reverse()
    return retData


class SimpleTextLoader(ui.Window):
    ELEM_PADDING = 0
    SCROLL_SPEED = 50

    def __init__(self):
        super(SimpleTextLoader, self).__init__()

        self.SetSize(mainBoardSize[0], mainBoardSize[1])

        self.elements = []
        self.images = []

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(self.GetWidth() - 8, self.GetHeight() - 85)

        self.scrollBoard = ui.Window()
        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(self.GetWidth() - 8)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 5)
        self.peekWindow.Show()
        self.scrollBoard.Show()

    def ParseToken(self, data):
        data = data.replace(chr(10), "").replace(chr(13), "")
        if not (len(data) and data[0] == "["):
            return False, {}, data

        fnd = data.find("]")
        if fnd <= 0:
            return False, {}, data

        content = data[1:fnd]
        data = data[fnd + 1 :]

        content = content.split(";")
        container = {}
        for i in content:
            i = i.strip()
            splt = i.split("=")
            if len(splt) == 1:
                container[splt[0].lower().strip()] = True
            else:
                container[splt[0].lower().strip()] = splt[1].lower().strip()

        return True, container, data

    def LoadFile(self, filename):

        del self.elements[:]
        del self.images[:]

        self.scrollBoard.SetSize(0, 0)

        open_filename = app.GetLocalePath() + "/wiki/" + filename

        if enableDebugThings:
            open_filename = "wiki\\\\" + filename
            loadF = open(open_filename)
        else:
            loadF = OpenVfsFile(open_filename)

        for i in loadF.readlines()[1:]:
            (ret, tokenMap, i) = self.ParseToken(i)
            if ret:
                if tokenMap.has_key("banner_img"):
                    if wiki.GetBaseClass():
                        wiki.GetBaseClass().header.LoadImage(tokenMap["banner_img"])
                        wiki.GetBaseClass().header.Show()

                    tokenMap.pop("banner_img")

                if "img" in tokenMap:
                    cimg = ui.ExpandedImageBox()
                    cimg.SetParent(self.scrollBoard)
                    cimg.AddFlag("attach")
                    cimg.AddFlag("not_pick")
                    cimg.LoadImage(tokenMap["img"])
                    cimg.Show()
                    tokenMap.pop("img")

                    x = 0
                    if tokenMap.has_key("x"):
                        x = int(tokenMap["x"])
                        tokenMap.pop("x")

                    y = 0
                    if tokenMap.has_key("y"):
                        y = int(tokenMap["y"])
                        tokenMap.pop("y")

                    if tokenMap.has_key("center_align"):
                        cimg.SetHorizontalAlignCenter()
                        cimg.SetPosition(x, y)
                        tokenMap.pop("center_align")
                    elif tokenMap.has_key("right_align"):
                        cimg.SetHorizontalAlignRight()
                        cimg.SetPosition(x, y)
                        tokenMap.pop("right_align")
                    else:
                        cimg.SetPosition(x, y)

                    self.images.append(cimg)

            if ret and not len(i):
                continue

            tmp = ui.Window()
            tmp.SetParent(self.scrollBoard)
            tmp.AddFlag("attach")
            tmp.AddFlag("not_pick")

            tx = ui.TextLine()
            tx.SetParent(tmp)
            if tokenMap.has_key("font_size"):
                splt = localeInfo.UI_DEF_FONT.split(":")
                tx.SetFontName(splt[0] + ":" + tokenMap["font_size"])
                tokenMap.pop("font_size")
            else:
                tx.SetFontName(localeInfo.UI_DEF_FONT)
            tx.SetText(i)
            tmp.SetSize(tx.GetWidth(), tx.GetHeight())
            tx.Show()
            tmp.txt = tx

            if len(i) > 0 and i[0] == "*":
                tx.SetText(i[1:])

                img = ui.ExpandedImageBox()
                img.SetParent(tmp)
                img.AddFlag("attach")
                img.AddFlag("not_pick")
                img.LoadImage("d:/ymir work/ui/wiki/arrow_2.tga")
                img.Show()
                tmp.img = img

                tmp.SetSize(
                    img.GetWidth() + 5 + tx.GetWidth(),
                    max(img.GetHeight(), tx.GetHeight()),
                )
                img.SetPosition(0, int(abs(tmp.GetHeight() / 2 - img.GetHeight() / 2)))

                tx.SetPosition(
                    int(img.GetWidth() + 5),
                    int(abs(tmp.GetHeight() / 2 - tx.GetHeight() / 2) - 1),
                )

            if tokenMap.has_key("color"):
                fontColor = GetColorFromString(tokenMap["color"])
                tx.SetPackedFontColor(
                    grp.GenerateColor(
                        fontColor[0], fontColor[1], fontColor[2], fontColor[3]
                    )
                )
                tokenMap.pop("color")

            totalElem = len(self.elements)
            addPadding = 0
            if tokenMap.has_key("y_padding"):
                addPadding = int(tokenMap["y_padding"])
                tokenMap.pop("y_padding")

            if totalElem > 0:
                lastIndex = totalElem

                self.elements.insert(lastIndex, tmp)
                totalElem += 1

                for i in xrange(lastIndex, totalElem):
                    if i == 0:
                        self.elements[i].SetPosition(0, 0)
                    else:
                        self.elements[i].SetPosition(
                            0,
                            self.elements[i - 1].GetLocalPosition()[1]
                            + self.elements[i - 1].GetHeight()
                            + addPadding,
                        )

            else:
                self.elements.append(tmp)
                tmp.SetPosition(0, addPadding)

            if tokenMap.has_key("center_align"):
                tmp.SetHorizontalAlignCenter()
                tokenMap.pop("center_align")
            elif tokenMap.has_key("right_align"):
                tmp.SetHorizontalAlignRight()
                tokenMap.pop("right_align")
            elif tokenMap.has_key("x_padding"):
                tmp.SetPosition(int(tokenMap["x_padding"]), tmp.GetLocalPosition()[1])
                tokenMap.pop("x_padding")

            tmp.Show()
            self.scrollBoard.SetSize(
                self.peekWindow.GetWidth(),
                self.scrollBoard.GetHeight() + addPadding + tmp.GetHeight(),
            )

        for i in self.images:
            mxSize = i.GetLocalPosition()[1] + i.GetHeight()
            if mxSize > self.scrollBoard.GetHeight():
                self.scrollBoard.SetSize(self.peekWindow.GetWidth(), mxSize)

        self.Show()


class WikiMainCostumeWindow(ui.Window):
    class WikiItem(ui.Window):
        def __init__(self, vnum, parent):
            ui.Window.__init__(self)

            self.vnum = vnum
            self.parent = proxy(parent)

            self.base = ui.ExpandedImageBox()
            self.base.SetParent(self)
            self.base.AddFlag("attach")
            self.base.AddFlag("not_pick")
            self.base.LoadImage("d:/ymir work/ui/wiki/detail_item_small.tga")
            self.base.Show()
            itemData = itemManager().GetProto(self.vnum)

            self.costumeImage = ui.ExpandedImageBox()
            self.costumeImage.SetParent(self.base)
            self.costumeImage.LoadImage(itemData.GetIconImageFileName())
            self.costumeImage.SetPosition(
                int(1 + 125 / 2 - self.costumeImage.GetWidth() / 2),
                int(1 + 120 / 2 - self.costumeImage.GetHeight() / 2),
            )
            self.costumeImage.Show()

            self.costumeImage.SetOnMouseOverInEvent(
                Event(self.parent.OnOverIn, self.vnum)
            )
            self.costumeImage.SetOnMouseOverOutEvent(Event(self.parent.OnOverOut))
            self.costumeImage.SetClickEvent(
                Event(
                    wiki.GetBaseClass().OpenSpecialPage, self.parent, self.vnum, False
                )
            )

            self.contentText = ui.TextLine()
            self.contentText.SetParent(self.base)
            self.contentText.AddFlag("attach")
            self.contentText.AddFlag("not_pick")
            if enableDebugThings:
                self.contentText.SetText("%s (%i)" % (itemData.GetName(), self.vnum))
            else:
                self.contentText.SetText(itemData.GetName())
            self.contentText.SetPosition(
                int(1 + 125 / 2 - self.contentText.GetWidth() / 2),
                int(122 + 18 / 2 - self.contentText.GetHeight() / 2 - 1),
            )
            self.contentText.Show()

            self.SetSize(self.base.GetWidth(), self.base.GetHeight())

        def __del__(self):
            try:
                if wiki.GetBaseClass():
                    wiki.GetBaseClass().objList[self.vnum].pop(self.GetWindowName())
            except KeyError as e:
                logging.debug("Missing key {}", self.vnum)

    ELEM_X_PADDING = 10
    ELEM_PADDING = 10
    SCROLL_SPEED = 25
    ELEM_PER_LINE = 4
    ITEM_LOAD_PER_UPDATE = 1

    def __init__(self):
        super(WikiMainCostumeWindow, self).__init__()

        self.SetSize(mainBoardSize[0], mainBoardSize[1])
        self.EnableClipping()

        self.elements = []
        self.posMap = {}
        self.isOpened = False
        self.loadFrom = 0
        self.loadTo = 0
        self.costumeVnums = []
        self.toolTip = None

        self.peekWindow = ui.ScrollWindow()
        self.peekWindow.SetSize(550, self.GetHeight() - 70)
        self.scrollBoard = ui.Window()
        self.scrollBoard.AddFlag("attach")
        self.scrollBoard.AddFlag("not_pick")
        self.scrollBoard.Show()

        self.peekWindow.SetContentWindow(self.scrollBoard)
        self.peekWindow.FitWidth(550)
        self.peekWindow.SetParent(self)
        self.peekWindow.SetPosition(0, 0)
        self.peekWindow.Show()

    def SetItemToolTip(self, t):
        self.toolTip = t

    def OpenWindow(self):
        super(WikiMainCostumeWindow, self).Show()

    def Hide(self):
        super(WikiMainCostumeWindow, self).Hide()

    def Show(self, vnums):
        super(WikiMainCostumeWindow, self).Show()
        self.peekWindow.ScrollToStart()

        extractedLists = []
        for i in vnums:
            if isinstance(i, list):
                extractedLists.append(i)

        for i in extractedLists:
            pos = vnums.index(i)
            vnums.remove(i)
            for j in xrange(len(i)):
                vnums.insert(pos + j, i[j])

        isChanged = not len(vnums) == len(self.costumeVnums)
        if not isChanged:
            for i in vnums:
                if i not in self.costumeVnums:
                    isChanged = True
                    break

        if not isChanged:
            for i in self.costumeVnums:
                if i not in vnums:
                    isChanged = True
                    break

        self.costumeVnums = vnums[:]
        self.loadTo = len(self.costumeVnums)

        if not self.isOpened:
            self.isOpened = True
            self.loadFrom = 0

        if self.loadFrom > self.loadTo or isChanged:
            del self.elements[:]
            self.posMap = {}
            self.loadFrom = 0
            self.scrollBoard.SetSize(0, 0)

    def GetRandomChar(self):
        WARRIOR_M = 0
        ASSASSIN_W = 1
        SURA_M = 2
        SHAMAN_W = 3
        WARRIOR_W = 4
        ASSASSIN_M = 5
        SURA_W = 6
        SHAMAN_M = 7

        SEX_FEMALE = 0
        SEX_MALE = 1

        ASSASSINS = [ASSASSIN_W, ASSASSIN_M]
        WARRIORS = [WARRIOR_W, WARRIOR_M]
        SURAS = [SURA_W, SURA_M]
        SHAMANS = [SHAMAN_W, SHAMAN_M]
        # what characters can wear it
        ITEM_CHARACTERS = [ASSASSINS, WARRIORS, SURAS, SHAMANS]
        # what sex can wear it
        ITEM_SEX = [SEX_FEMALE, SEX_MALE]
        # anti flag male -> remove male from ITEM_SEX
        if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
            ITEM_SEX.remove(SEX_MALE)

        # anti flag female -> remove female from ITEM_SEX
        if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
            ITEM_SEX.remove(SEX_FEMALE)

        # get characters that can use it, bit ghetto code
        if item.IsAntiFlag(item.ITEM_ANTIFLAG_WARRIOR):
            ITEM_CHARACTERS.remove(WARRIORS)

        if item.IsAntiFlag(item.ITEM_ANTIFLAG_SURA):
            ITEM_CHARACTERS.remove(SURAS)

        if item.IsAntiFlag(item.ITEM_ANTIFLAG_ASSASSIN):
            ITEM_CHARACTERS.remove(ASSASSINS)

        if item.IsAntiFlag(item.ITEM_ANTIFLAG_SHAMAN):
            ITEM_CHARACTERS.remove(SHAMANS)

        return ITEM_CHARACTERS[app.GetRandom(0, len(ITEM_CHARACTERS) - 1)][
            ITEM_SEX[app.GetRandom(0, len(ITEM_SEX) - 1)]
        ]

    def OnOverIn(
        self, vnum, metinSlot=None, displayRefinedVnum=False, count=None, rarity=None
    ):
        if metinSlot is None:
            metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
        self.toolTip.ClearToolTip()
        self.toolTip.AddItemData(vnum, metinSlot, 0, showRefinedVnum=displayRefinedVnum)
        if count:
            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_DROP_COUNT").format(count)
            )
        if rarity:
            from uiTarget import FormatRarity

            self.toolTip.AppendTextLine(
                localeInfo.Get("TOOLTIP_RARITY").format(FormatRarity(rarity))
            )

    def OnOverOut(self):
        self.toolTip.Hide()

    def OnUpdate(self):
        if self.loadFrom < self.loadTo:
            for i in xrange(
                self.loadFrom,
                min(self.loadTo, self.loadFrom + self.ITEM_LOAD_PER_UPDATE),
            ):
                self.AddItem(self.costumeVnums[i])
                self.loadFrom += 1

    def AddItem(self, vnum):
        for i in self.elements:
            if vnum == i.vnum:
                return None
        if not itemManager().GetProto(vnum):
            return None

        tmp = self.WikiItem(vnum, self)
        tmp.SetParent(self.scrollBoard)
        tmp.AddFlag("attach")

        totalElem = len(self.elements)
        if totalElem > 0:
            # tmp.SetPosition(0, self.elements[-1].GetLocalPosition()[1] + self.elements[-1].GetHeight() + self.ELEM_PADDING)
            currAdd = 0
            while currAdd in self.posMap:
                currAdd += 1

            totalLine = currAdd % self.ELEM_PER_LINE
            currH = math.floor(currAdd / self.ELEM_PER_LINE) * (
                tmp.GetHeight() + self.ELEM_PADDING
            )

            self.posMap[currAdd] = True
            tmp.SetPosition(
                int(1 + totalLine * (tmp.GetWidth() + self.ELEM_X_PADDING)),
                int(0 + currH),
            )

        else:
            self.posMap[0] = True
            tmp.SetPosition(1, 0)

        self.elements.append(tmp)

        tmp.Show()
        self.scrollBoard.SetSize(
            self.peekWindow.GetWidth(),
            max(
                self.scrollBoard.GetHeight(),
                tmp.GetLocalPosition()[1] + tmp.GetHeight(),
            ),
        )


def InitMainWindow(self):
    self.AddFlag("moveable")
    self.AddFlag("float")

    for i in wikiInst().GetItemWhitelist():
        if isinstance(i, list):
            for k in i:
                wiki.RegisterItemBlacklist(k)
        else:
            wiki.RegisterItemBlacklist(i)

    for i in wikiInst().GetMobWhitelist():
        if isinstance(i, list):
            for k in i:
                wiki.RegisterMonsterBlacklist(k)
        else:
            wiki.RegisterMonsterBlacklist(i)

    nonplayer.BuildWikiSearchList()


def InitMainWeaponWindow(self):
    self.mainWeaponWindow = WikiMainWeaponWindow()
    self.mainWeaponWindow.SetParent(self.baseBoard)
    self.mainWeaponWindow.AddFlag("attach")
    self.mainWeaponWindow.SetPosition(mainBoardPos[0], mainBoardPos[1])


def InitMainChestWindow(self):
    self.mainChestWindow = WikiMainChestWindow()
    self.mainChestWindow.SetParent(self.baseBoard)
    self.mainChestWindow.AddFlag("attach")
    self.mainChestWindow.SetPosition(mainBoardPos[0], mainBoardPos[1])


def InitMainBossWindow(self):
    self.mainBossWindow = WikiMainBossWindow()
    self.mainBossWindow.SetParent(self.baseBoard)
    self.mainBossWindow.AddFlag("attach")
    self.mainBossWindow.SetPosition(mainBoardPos[0], mainBoardPos[1])


def InitCustomPageWindow(self):
    self.customPageWindow = SimpleTextLoader()
    self.customPageWindow.SetParent(self.baseBoard)
    self.customPageWindow.AddFlag("attach")
    self.customPageWindow.SetPosition(mainBoardPos[0], mainBoardPos[1])


def InitCostumePageWindow(self):
    self.costumePageWindow = WikiMainCostumeWindow()
    self.costumePageWindow.SetParent(self.baseBoard)
    self.costumePageWindow.AddFlag("attach")
    self.costumePageWindow.SetPosition(mainBoardPos[0], mainBoardPos[1])


def InitTitleBar(self):
    titleBar = ui.TitleBar()
    titleBar.SetParent(self.baseBoard)
    titleBar.MakeTitleBar(0, "red")
    titleBar.SetWidth(self.GetWidth() - 15)
    titleBar.SetTitleName(localeInfo.WIKI_TITLENAME)
    titleBar.SetPosition(8, 1)
    titleBar.Show()

    self.titleBar = titleBar
    self.titleName = titleBar.GetTitleName()
    self.titleBar.SetCloseEvent(self.Close)


def BuildSearchWindow(self):

    # self.searchBG = ui.ExpandedImageBox()
    # self.searchBG.SetParent(self.baseBoard)
    # self.searchBG.LoadImage("d:/ymir work/ui/wiki/searchfield.tga")
    # self.searchBG.SetPosition(13, 33)
    # self.searchBG.Show()

    # self.searchButton = ui.Button()
    # self.searchButton.SetParent(self.searchBG)
    # self.searchButton.SetUpVisual("d:/ymir work/ui/wiki/button_search_normal.tga")
    # self.searchButton.SetOverVisual("d:/ymir work/ui/wiki/button_search_hover.tga")
    # self.searchButton.SetDownVisual("d:/ymir work/ui/wiki/button_search_down.tga")
    # self.searchButton.SetPosition(self.searchBG.GetWidth() - self.searchButton.GetWidth() - 2, self.searchBG.GetHeight() / 2 - self.searchButton.GetHeight() / 2)
    # self.searchButton.SetEvent(self.StartSearch)
    # self.searchButton.Show()

    self.searchEdit = ui.ComboBoxAutoComplete(
        self,
        "d:/ymir work/ui/game/windows/combobox_bg.tga",
        15,
        44,
        1.0,
        1.0,
    )
    self.searchEdit.InsertItem(0, "/")
    self.searchEdit.SetCurrentItem("")
    self.searchEdit.SetAutoCompleteEvent(self.GetAutoCompleteEntries)
    self.searchEdit.SetEvent(self.StartSearch)
    self.searchEdit.Show()


def BuildBaseMain(self):
    #############################################################################

    BOSS_CHEST_VNUMS = wikiInst().GetBossChests()
    EVENT_CHEST_VNUMS = wikiInst().GetEventChests()
    ALT_CHEST_VNUMS = wikiInst().GetAltChests()
    COSTUME_WEAPON_VNUMS = wikiInst().GetCostumeWeapons()
    COSTUME_ARMOR_VNUMS = wikiInst().GetCostumeArmors()
    COSTUME_HAIR_VNUMS = wikiInst().GetCostumeHairs()
    COSTUME_WING_VNUMS = wikiInst().GetCostumeWings()
    COSTUME_SHINING_VNUMS = wikiInst().GetCostumeShinings()

    #############################################################################

    WIKI_CATEGORIES = [
        [
            localeInfo.WIKI_CATEGORY_EQUIPEMENT,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_WEAPONS,
                    (0,),
                    "d:/ymir work/ui/wiki/banners/weapons.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_ARMOR,
                    (1,),
                    "d:/ymir work/ui/wiki/banners/armors.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_HELMET,
                    (4,),
                    "d:/ymir work/ui/wiki/banners/helmets.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_SHIELD,
                    (6,),
                    "d:/ymir work/ui/wiki/banners/shields.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_EARRINGS,
                    (2,),
                    "d:/ymir work/ui/wiki/banners/earrings.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_BRACELET,
                    (7,),
                    "d:/ymir work/ui/wiki/banners/bracelets.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_NECKLACE,
                    (5,),
                    "d:/ymir work/ui/wiki/banners/neck.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_SHOES,
                    (3,),
                    "d:/ymir work/ui/wiki/banners/shoes.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_BELTS,
                    (9,),
                    "d:/ymir work/ui/wiki/banners/belts.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_TALISMANS,
                    (10,),
                    "d:/ymir work/ui/wiki/banners/talismans.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_RINGS,
                    (11,),
                    "d:/ymir work/ui/wiki/banners/talismans.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_ELIXIR,
                    (12,),
                    "d:/ymir work/ui/wiki/banners/talismans.png",
                ],
                # ["Talisman", ()],
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_CHESTS,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_CHESTS,
                    (BOSS_CHEST_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/boss_chests.png",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_EVENT_CHESTS,
                    (EVENT_CHEST_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/ev_chests.png",
                ],
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_BOSSES,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_LV1_75,
                    (0, 1, 999),
                    "d:/ymir work/ui/wiki/banners/bosses.png",
                ],
                # [localeInfo.WIKI_SUBCATEGORY_LV76_100, (0, 76, 100), "d:/ymir work/ui/wiki/banners/bosses.tga"],
                # [localeInfo.WIKI_SUBCATEGORY_LV100, (0, 100, 255), "d:/ymir work/ui/wiki/banners/bosses.tga"]
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_MONSTERS,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_LV1_75,
                    (1, 1, 999),
                    "d:/ymir work/ui/wiki/banners/monsters.png",
                ],
                # [localeInfo.WIKI_SUBCATEGORY_LV76_100, (1, 76, 100), "d:/ymir work/ui/wiki/banners/monsters.tga"],
                # [localeInfo.WIKI_SUBCATEGORY_LV100, (1, 100, 255), "d:/ymir work/ui/wiki/banners/monsters.tga"]
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_METINSTONES,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_LV1_75,
                    (2, 1, 999),
                    "d:/ymir work/ui/wiki/banners/monsters.png",
                ],
                # [localeInfo.WIKI_SUBCATEGORY_LV76_100, (2, 76, 100), "d:/ymir work/ui/wiki/banners/metinstones.tga"],
                # [localeInfo.WIKI_SUBCATEGORY_LV100, (2, 100, 255), "d:/ymir work/ui/wiki/banners/metinstones.tga"]
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_SYSTEMS,
            [
                # [localeInfo.WIKI_SUBCATEGORY_RUNES, ("systems/runes.txt",)],
                [
                    localeInfo.WIKI_SUBCATEGORY_DRAGONALCHEMY,
                    ("systems/dragon_alchemy.txt",),
                ],
                [localeInfo.WIKI_SUBCATEGORY_SKILLTREE, ("systems/skilltree.txt",)],
                [localeInfo.WIKI_SUBCATEGORY_TITLEITEM, ("systems/titleitem.txt",)],
                [localeInfo.WIKI_SUBCATEGORY_RANKSYSTEM, ("systems/ranksystem.txt",)],
                [localeInfo.WIKI_SUBCATEGORY_PETSYSTEM, ("systems/petsystem.txt",)],
                [localeInfo.WIKI_SUBCATEGORY_DAILYREWARD, ("systems/dailyreward.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_SHOULDER_SASH, ("systems/shoulder_sash.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_COSTUME_SASH, ("systems/costume_sash.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_BATTLEPASS, ("systems/battlepass.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_COSTUMES, ("systems/costumes.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_GAYA, ("systems/gaya.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_SKILL_COLOR, ("systems/skill_color.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_CONTER_BOOST, ("systems/conter_boost.txt",)]
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_DUNGEONS,
            [
                [localeInfo.WIKI_SUBCATEGORY_ORCMAZE, ("dungeons/orc_maze.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_SPIDER_BARONESS, ("dungeons/spider_baroness.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_AZRAEL, ("dungeons/azrael.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_BERAN_SETAOU, ("dungeons/beran_setaou.txt",)],
                # [localeInfo.QUEST_TIMER_SLIME, ("dungeons/slime.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_NEMERE, ("dungeons/nemere.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_RAZADOR, ("dungeons/razador.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_SHIPDEFENSE, ("dungeons/ship_defense.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_JOTUN_THRYM, ("dungeons/jotun_thrym.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_CRYSTAL_DRAGON, ("dungeons/crystal_dragon.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_MELEY, ("dungeons/meley.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_THRANDUILS_LAIR, ("dungeons/thranduil.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_ZODIAC, ("dungeons/zodiac.txt",)],
                # [localeInfo.QUEST_TIMER_INFECTED, ("dungeons/infected.txt",)]
            ],
        ],
        [
            localeInfo.WIKI_CATEGORY_COSTUMES,
            [
                [
                    localeInfo.WIKI_SUBCATEGORY_WEAPONS,
                    (COSTUME_WEAPON_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/weapons.tga",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_ARMOR,
                    (COSTUME_ARMOR_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/armors.tga",
                ],
                [
                    localeInfo.WIKI_SUBCATEGORY_HAIRSTYLES,
                    (COSTUME_HAIR_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/hairstyle.tga",
                ],
                [
                    "Wings",
                    (COSTUME_WING_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/wings.tga",
                ],
                [
                    "Shinings",
                    (COSTUME_SHINING_VNUMS,),
                    "d:/ymir work/ui/wiki/banners/shinings.tga",
                ],
                # "Shining"
            ],
        ],
        # [
        # localeInfo.WIKI_CATEGORY_EVENTS,
        #   [
        #      [localeInfo.WIKI_SUBCATEGORY_OKAY_CARD, ("events/okey_card.txt",)]
        # ]
        # ],
        [
            localeInfo.WIKI_CATEGORY_GUIDES,
            [
                [localeInfo.WIKI_SUBCATEGORY_THE_START, ("guides/the_start.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_RUNES, ("guides/runes.txt",)],
                # [localeInfo.WIKI_SUBCATEGORY_105_AND_NOW, ("guides/105_and_now.txt",)]
            ],
        ],
    ]

    self.baseBoard = ui.ImageBox()
    self.baseBoard.AddFlag("attach")
    self.baseBoard.SetParent(self)
    self.baseBoard.LoadImage("D:/ymir Work/ui/wiki/bg.tga")
    self.baseBoard.SetWindowName("InGameWikiBoard")
    self.baseBoard.SetPosition(0, 0)
    self.baseBoard.Show()

    self.SetSize(self.baseBoard.GetWidth(), self.baseBoard.GetHeight())

    self.header = ui.ExpandedImageBox()

    self.header.SetParent(self.baseBoard)
    self.header.SetPosition(149, 37)

    BuildSearchWindow(self)

    # self.categoriesScrollWindow = ScrollWindow(categoryPeakWindowSize[0], categoryPeakWindowSize[1], ScrollWindow.THIN)
    # self.categoriesScrollWindow

    self.categ = WikiCategories()
    self.categ.hideWindowsEvent = MakeEvent(self.CloseBaseWindows)
    self.categ.SetParent(self)
    self.categ.AddFlag("attach")
    # self.categ.SetHorizontalAlignCenter()
    self.categ.SetPosition(15, 90)
    self.categ.Show()

    self.prevButt = ui.Button()
    self.prevButt.SetParent(self.baseBoard)
    # self.prevButt.SetUpVisual("d:/ymir work/ui/wiki/btn_arrow_left_normal.tga")
    # self.prevButt.SetOverVisual("d:/ymir work/ui/wiki/btn_arrow_left_hover.tga")
    # self.prevButt.SetDownVisual("d:/ymir work/ui/wiki/btn_arrow_left_down.tga")
    # self.prevButt.SetDisableVisual("d:/ymir work/ui/wiki/btn_arrow_left_down.tga")
    self.prevButt.SetPosition(10, 10)
    self.prevButt.SetVerticalAlignBottom()
    self.prevButt.SetHorizontalAlignLeft()
    self.prevButt.SetEvent(self.OnPressPrevButton)
    self.prevButt.Show()
    self.prevButt.Disable()

    self.nextButt = ui.Button()
    self.nextButt.SetParent(self.baseBoard)
    # self.nextButt.SetUpVisual("d:/ymir work/ui/wiki/btn_arrow_right_normal.tga")
    # self.nextButt.SetOverVisual("d:/ymir work/ui/wiki/btn_arrow_right_hover.tga")
    # self.nextButt.SetDownVisual("d:/ymir work/ui/wiki/btn_arrow_right_down.tga")
    # self.nextButt.SetDisableVisual("d:/ymir work/ui/wiki/btn_arrow_right_down.tga")
    self.nextButt.SetEvent(self.OnPressNextButton)
    self.nextButt.SetPosition(10, 10)
    self.nextButt.SetVerticalAlignBottom()
    self.nextButt.SetHorizontalAlignRight()
    self.nextButt.Show()
    self.nextButt.Disable()

    InitTitleBar(self)
    InitMainWeaponWindow(self)
    InitMainChestWindow(self)
    InitMainBossWindow(self)
    InitCustomPageWindow(self)
    InitCostumePageWindow(self)

    funclist = [
        MakeEvent(self.mainWeaponWindow.Show),  # Equipment
        MakeEvent(self.mainChestWindow.Show),  # Chests
        MakeEvent(self.mainBossWindow.Show),  # Bosses
        MakeEvent(self.mainBossWindow.Show),  # Monster
        MakeEvent(self.mainBossWindow.Show),  # Stones
        MakeEvent(self.customPageWindow.LoadFile),  # Systems
        MakeEvent(self.customPageWindow.LoadFile),  # Dungeons
        MakeEvent(self.costumePageWindow.Show),  # Costumes
        MakeEvent(self.customPageWindow.LoadFile),
        MakeEvent(self.customPageWindow.LoadFile),
    ]

    for i in WIKI_CATEGORIES:
        ret = self.categ.AddCategory(i[0])
        for j in xrange(len(i[1])):
            ret.AddSubCategory(j, i[1][j])

        curr = WIKI_CATEGORIES.index(i)
        if curr < len(funclist):
            ret.clickEvent = funclist[curr]

    self.customPageWindow.LoadFile("landingpage.txt")
    self.header.SetClickEvent(self.GoToLanding)
    self.header.SetTop()
    self.titleBar.SetTop()
