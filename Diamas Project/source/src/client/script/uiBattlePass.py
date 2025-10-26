# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging
from _weakref import proxy
from datetime import datetime

import app
import chat
import item
import nonplayer
import player
import wndMgr
from pack import OpenVfsFile, GetVfsFile
from pygame.app import appInst

import localeInfo
import ui
import uiToolTip
from ui_event import Event

ROOT_PATH = "d:/ymir work/ui/game/battle_pass/"
MISSION_PATH = "d:/ymir work/ui/game/battle_pass/mission/"

RANKING_MAX_NUM = 9
MISSION_REWARD_COUNT = 3

BATTLE_PASS_NAMES = {
    1: localeInfo.BATTLEPASS_TEXT1,  # TODO LOCALE_GAME
    2: localeInfo.BATTLEPASS_ID_2_NAME,
}

MISSION_INFO_DICT = {}


class MissionScrollBar(ui.DragButton):
    def __init__(self):
        ui.DragButton.__init__(self)
        self.AddFlag("float")
        self.AddFlag("moveable")
        self.AddFlag("restrict_x")

        self.eventScroll = lambda *arg: None
        self.currentPos = 0.0
        self.scrollStep = 0.20

    def SetScrollEvent(self, event):
        self.eventScroll = event

    def SetScrollStep(self, step):
        self.scrollStep = step

    def SetPos(self, pos):
        pos = max(0.0, pos)
        pos = min(1.0, pos)

        yPos = float(pos * 165)  # restrictedHeight - imageHeight - 3

        self.SetPosition(11, yPos + 3)  # restrictedY
        self.OnMove()

    def GetPos(self):
        return self.currentPos

    def OnUp(self):
        self.SetPos(self.currentPos - self.scrollStep)

    def OnDown(self):
        self.SetPos(self.currentPos + self.scrollStep)

    def OnMove(self):
        (xLocal, yLocal) = self.GetLocalPosition()
        self.currentPos = float(yLocal - 3) / float(165)

        self.eventScroll()


class ListBoxMissions(ui.Window):
    class Item(ui.Window):
        def __init__(self, missionId):
            ui.Window.__init__(self)
            self.SetWindowName("ListBoxMissions_Item")
            self.bIsSelected = False
            self.missionId = missionId
            self.percentActual = 0
            self.percentTotal = 0
            self.rewardCount = 0
            self.missionInfo1 = 0
            self.xBase, self.yBase = 0, 0

            self.tooltipItem = None

            self.listImages = []
            self.rewardList = [[0, 0], [0, 0], [0, 0]]
            self.rewardImages = []
            self.rewardListCount = []

            iconName = MISSION_PATH + "mission_%d_small.tga" % (missionId)
            if GetVfsFile(iconName) is None:
                iconName = ROOT_PATH + "mission_small_clean.tga".format(missionId)

            bgImage = ui.MakeExpandedImageBox(
                self, ROOT_PATH + "mission_bg_normal.tga", 0, 0, "not_pick"
            )
            iconImage = ui.MakeExpandedImageBox(bgImage, iconName, 1, 1, "not_pick")
            bgGauge = ui.MakeExpandedImageBox(
                bgImage, ROOT_PATH + "mission_progress_empty.tga", 47, 28, "not_pick"
            )
            self.bgGaugeFull = ui.MakeExpandedImageBox(
                bgGauge, ROOT_PATH + "mission_progress_full.tga", 10, 2, "not_pick"
            )
            self.bgGaugeFull.SetWindowName("gaugeFull")

            self.listImages.append(bgImage)
            self.listImages.append(iconImage)
            self.listImages.append(bgGauge)

            for i in xrange(MISSION_REWARD_COUNT):
                rewardImage = ui.MakeExpandedImageBox(
                    self,
                    "d:/ymir work/ui/game/belt_inventory/slot_disabled.tga",
                    187 + (32 * i),
                    6,
                )
                rewardImage.SetOnMouseOverInEvent(Event(self.OverInItem, i))
                rewardImage.SetOnMouseOverOutEvent(self.OverOutItem)
                self.rewardImages.append(rewardImage)

                itemCount = ui.NumberLine()
                itemCount.SetParent(rewardImage)
                itemCount.SetWindowName("itemCount_%d" % i)
                itemCount.SetHorizontalAlignRight()
                itemCount.SetPosition(32 - 4, 32 - 10)
                itemCount.Show()
                self.rewardListCount.append(itemCount)

            self.missionName = ui.MakeTextLineNew(bgImage, 50, -11, "")

        def OverInItem(self, rewardIndex):
            if self.tooltipItem:
                self.tooltipItem.ClearToolTip()
                if self.rewardList[rewardIndex][0]:
                    self.tooltipItem.AddItemData(
                        self.rewardList[rewardIndex][0],
                        metinSlot=[0 for i in xrange(player.METIN_SOCKET_MAX_NUM)],
                    )
                    self.tooltipItem.ShowToolTip()

        def OverOutItem(self):
            if self.tooltipItem:
                self.tooltipItem.HideToolTip()

        def SetBasePosition(self, x, y):
            self.xBase = x
            self.yBase = y

        def GetBasePosition(self):
            return (self.xBase, self.yBase)

        def GetMissionId(self):
            return self.missionId

        def OnMouseLeftButtonUp(self):
            self.Select()

        def Select(self):
            self.bIsSelected = True
            self.parent.SetSelectedMission(self.missionId)

            if len(self.listImages) > 0:
                self.listImages[0].LoadImage(ROOT_PATH + "mission_bg_selected.tga")

        def Deselect(self):
            self.bIsSelected = False

            if len(self.listImages) > 0:
                self.listImages[0].LoadImage(ROOT_PATH + "mission_bg_normal.tga")

        def SetProgress(self, progressActual, pregressTotal):
            self.percentActual = progressActual
            self.percentTotal = pregressTotal

            self.UpdateGauge()

        def UpdateProgress(self, newProgress):
            if newProgress >= self.percentActual:
                self.percentActual = newProgress
                self.UpdateGauge()
            else:
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    "[Error]: Update progress error, new progress is lower than old progress.",
                )

        def UpdateGauge(self):
            self.bgGaugeFull.SetPercentage(self.percentActual, self.percentTotal)

        def IsCompleted(self):
            if self.percentActual >= self.percentTotal:
                return True

            return False

        def SetMissionName(self, missionName):
            if self.missionName:
                self.missionName.SetText(missionName)

        def SetItemToolTip(self, tooltipItem):
            self.tooltipItem = tooltipItem

        def SetMissionInfo1(self, missionInfo):
            self.missionInfo1 = missionInfo

        def GetMissionInfo(self):
            return (self.missionInfo1, self.percentActual, self.percentTotal)

        def AddMissionReward(self, itemVnum, itemCount):
            if self.rewardCount == -1:
                return

            if itemVnum and itemCount > 0:
                if self.rewardCount < len(self.rewardImages):
                    item.SelectItem(itemVnum)
                    self.rewardImages[self.rewardCount].LoadImage(
                        item.GetIconImageFileName()
                    )
                    self.rewardListCount[self.rewardCount].SetNumber(str(itemCount))
                    self.rewardList[self.rewardCount] = [itemVnum, itemCount]
                    self.rewardCount += 1
            else:
                self.rewardCount = -1

        def Show(self):
            ui.Window.Show(self)

        def SetParent(self, parent):
            ui.Window.SetParent(self, parent)
            self.parent = proxy(parent)

        def OnRender(self):
            xList, yList = self.parent.GetGlobalPosition()

            for item in self.listImages + self.rewardImages:
                if item.GetWindowName() == "gaugeFull":
                    if self.percentTotal == 0:
                        self.percentTotal = 1
                    item.SetClipRect(
                        0.0,
                        yList,
                        -1.0 + float(self.percentActual) / float(self.percentTotal),
                        yList + self.parent.GetHeight(),
                        True,
                    )
                else:
                    item.SetClipRect(
                        xList,
                        yList,
                        xList + self.parent.GetWidth(),
                        yList + self.parent.GetHeight(),
                    )

            for count in self.rewardListCount:
                xList, yList = self.parent.GetGlobalPosition()
                xText, yText = count.GetGlobalPosition()
                wText, hText = count.GetWidth(), 7

                if yText < yList or (yText + hText > yList + self.parent.GetHeight()):
                    count.Hide()
                else:
                    count.Show()

    def __init__(self):
        ui.Window.__init__(self)
        self.SetWindowName("ListBoxMissions")
        self.itemList = []

        self.selectedMission = 0
        self.toolTipItem = None

    def SetItemToolTip(self, toolTipItem):
        self.toolTipItem = toolTipItem

    def SetSelectedMission(self, missionId):
        self.selectedMission = missionId

        for itemH in self.itemList:
            if itemH.GetMissionId() != self.selectedMission:
                itemH.Deselect()

        if self.globalParent:
            self.globalParent.SetMissionInfo(self.selectedMission)

    def GetSelectedMission(self):
        return self.selectedMission

    def SelectMission(self, missionId):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                item.Select()

    def HaveMission(self, missionId):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                return True

        return False

    def GetMissionInfo(self, missionId):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                return item.GetMissionInfo()

        return (0, 0, 0)

    def GetMissionCount(self):
        return len(self.itemList)

    def GetCompletedMissionCount(self):
        completedCount = 0
        for item in self.itemList:
            if item.IsCompleted():
                completedCount += 1

        return completedCount

    def SetProgress(self, missionId, progressActual, pregressTotal):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                item.SetProgress(progressActual, pregressTotal)

    def UpdateProgress(self, missionId, newProgress):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                item.UpdateProgress(newProgress)

    def AddMissionReward(self, missionId, itemVnum, itemCount):
        for item in self.itemList:
            if missionId == item.GetMissionId():
                item.AddMissionReward(itemVnum, itemCount)

    def SetGlobalParent(self, parent):
        self.globalParent = proxy(parent)

    def OnScroll(self, scrollPos):
        totalHeight = 0
        for itemH in self.itemList:
            totalHeight += itemH.GetHeight()

        totalHeight -= self.GetHeight()

        for i in xrange(len(self.itemList)):
            x, y = self.itemList[i].GetLocalPosition()
            xB, yB = self.itemList[i].GetBasePosition()
            setPos = yB - int(scrollPos * totalHeight)
            self.itemList[i].SetPosition(xB, setPos)

    def AppendMission(self, itemHeight, missionId, missionName, missionInfo1):
        item = self.Item(missionId)
        item.SetParent(self)
        item.SetSize(self.GetWidth() - 3, itemHeight)
        item.SetItemToolTip(self.toolTipItem)
        item.SetMissionName(missionName)
        item.SetMissionInfo1(missionInfo1)

        if len(self.itemList) == 0:
            item.SetPosition(0, 0)
            item.SetBasePosition(0, 0)
        else:
            x, y = self.itemList[-1].GetLocalPosition()
            item.SetPosition(0, y + self.itemList[-1].GetHeight())
            item.SetBasePosition(0, y + self.itemList[-1].GetHeight())

        item.Show()
        self.itemList.append(item)

    def ClearMissions(self):
        for item in self.itemList:
            item.Hide()
            item.Destroy()
        self.itemList = []


class BattlePassWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.Initialize()
        self.isLoaded = False
        self.LoadMissionList()

    def Initialize(self):
        self.board = None
        self.scrollBar = None
        self.tooltipItem = None

        self.currentState = "MISSION"
        self.tabButtonDict = None

        self.rankingButton = None
        self.rewardButton = None

        self.scrollBarWindow = None
        self.missionWindow = None
        self.rankingWindow = None

        self.missionNameText = None
        self.totalProgressGauge = None
        self.bgImageMission = None
        self.rewardItems = None

        self.loadingBackground = None
        self.loadingAnimation = None

        self.missionInfoDict = {}
        self.generalInfoDict = {}

        self.rankingImages = []
        self.rankingSpecialImages = []
        self.rankingTexts = {}
        self.rankingInfo = {}
        self.rewardDict = {}

        self.battlePassID = 0
        self.battlePassEndTime = 0
        self.rewardSlotIndex = 0

        self.currentRankingPage = 1
        self.firstPrevButton = None
        self.prevButton = None
        self.nextButton = None
        self.lastNextButton = None
        self.pageButtonList = [None, None, None, None, None]
        self.requestEndTime = 0
        self.missionList = None

    def Clear(self):
        self.rewardDict = {}
        self.rewardSlotIndex = 0
        if self.rewardItems:
            for slotPos in xrange(self.rewardItems.GetSlotCount()):
                self.rewardItems.ClearSlot(slotPos)
            self.rewardItems.ClearAllSlot()

    def SetItemToolTip(self, itemTooltip):
        self.tooltipItem = itemTooltip
        if self.missionList:
            self.missionList.SetItemToolTip(self.tooltipItem)

    def LoadMissionList(self):
        for line in OpenVfsFile(app.GetLocalePath() + "/battle_pass.txt"):
            if line[0] == "#":
                continue
            if line == "\r\n":
                continue

            print(line)

            tokens = line[:-1].split("\t")
            if len(tokens) == 0 or not tokens[0]:
                continue

            if tokens[0] == "#":
                continue

            if len(tokens) < 7:
                continue

            MISSION_INFO_DICT[int(tokens[0])] = [
                tokens[1],
                tokens[2],
                tokens[3],
                tokens[4],
                tokens[5],
                tokens[6],
            ]

    def LoadWindow(self):
        logging.debug("Load BattlePassWindow")
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/BattlePassWindow.py")
        except Exception as e:
            logging.exception(e)

        try:
            self.board = self.GetChild("board")

            self.board.SetCloseEvent(self.Close)

            self.tabButtonDict = {
                "MISSION": [
                    self.GetChild("tab_button_01"),
                    self.GetChild("tab_01"),
                    self.GetChild("BorderInfoMission"),
                ],
                "GENERAL": [
                    self.GetChild("tab_button_02"),
                    self.GetChild("tab_02"),
                    self.GetChild("BorderInfoGeneral"),
                ],
            }

            for i in xrange(5):
                self.generalInfoDict[i] = self.GetChild("general_text_%d" % int(i))

            for j in xrange(7):
                self.missionInfoDict[j] = self.GetChild("mission_text_%d" % int(j))

            self.missionNameText = self.GetChild("missionTitleText")
            self.totalProgressGauge = self.GetChild("gaugeImage")

            self.scrollBarWindow = self.GetChild("BorderScroll")
            self.missionWindow = self.GetChild("BorderMissions")
            self.rankingWindow = self.GetChild("BorderRanking")

            self.rankingButton = self.GetChild("RankingButton")
            self.rewardButton = self.GetChild("RewardButton")
            self.bgImageMission = self.GetChild("bgImageMission")
            self.rewardItems = self.GetChild("RewardItems")

            self.firstPrevButton = self.GetChild("first_prev_button")
            self.prevButton = self.GetChild("prev_button")
            self.nextButton = self.GetChild("next_button")
            self.lastNextButton = self.GetChild("last_next_button")

            for i in xrange(5):
                self.pageButtonList[i] = self.GetChild("page%d_button" % int(i + 1))
        except Exception as e:
            logging.exception(e)

        self.rankingWindow.Hide()

        for tabKey, tabButton in self.tabButtonDict.items():
            tabButton[0].SetEvent(Event(self.OnClickTabButton, tabKey))

        self.rankingButton.SetEvent(self.OnClickRankingButton)
        self.rewardButton.SetEvent(self.OnClickRewardButton)
        self.rewardButton.Disable()

        self.firstPrevButton.SetEvent(self.OnClickRankingFirstPrevButton)
        self.prevButton.SetEvent(self.OnClickRankingPrevButton)
        self.nextButton.SetEvent(self.OnClickRankingNextButton)
        self.lastNextButton.SetEvent(self.OnClickRankingLastNextButton)

        for i in xrange(len(self.pageButtonList)):
            self.pageButtonList[i].SetEvent(Event(self.ClickPageButton, i))

        self.rewardItems.SetOverInItemEvent(self.OverInItem)
        self.rewardItems.SetOverOutItemEvent(self.OverOutItem)

        self.scrollBar = MissionScrollBar()
        self.scrollBar.SetParent(self.scrollBarWindow)
        self.scrollBar.SetScrollEvent(self.OnScroll)
        self.scrollBar.SetUpVisual(ROOT_PATH + "scroll_bar.tga")
        self.scrollBar.SetOverVisual(ROOT_PATH + "scroll_bar.tga")
        self.scrollBar.SetDownVisual(ROOT_PATH + "scroll_bar.tga")
        self.scrollBar.SetRestrictMovementArea(11, 3, 6, 272)
        self.scrollBar.SetPosition(5, 7)
        self.scrollBar.Show()

        self.missionList = ListBoxMissions()
        self.missionList.SetParent(self.missionWindow)
        self.missionList.SetGlobalParent(self)
        self.missionList.SetPosition(1, 8)
        self.missionList.SetSize(
            300, 272
        )  # 290(H) without scrollbar, 249(W) with Render
        self.missionList.Show()
        self.missionList.EnableClipping()
        if self.tooltipItem:
            if self.missionList:
                self.missionList.SetItemToolTip(self.tooltipItem)

        self.CreateRankingItems()

    def GetTextByInfo(self, missionId, info_1):
        if missionId in [1, 3]:
            return nonplayer.GetMonsterName(info_1)
        elif missionId in [2]:
            return nonplayer.GetMonsterName(info_1)
        elif missionId in [5, 6, 7, 8, 9, 10]:
            item.SelectItem(info_1)
            return item.GetItemName()

        return ""

    def SetMissionInfo(self, missionId):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True
        missionName = (
            MISSION_INFO_DICT[missionId][0]
            if MISSION_INFO_DICT.has_key(missionId)
            else "Unknown name"
        )
        self.missionNameText.SetText(missionName)

        (info_1, info_2, info_3) = self.missionList.GetMissionInfo(missionId)

        if info_2 >= info_3:
            self.missionInfoDict[0].SetText(
                localeInfo.BATTLEPASS_TEXT2
            )  # TODO LOCALE_GAME
        else:
            self.missionInfoDict[0].SetText(
                localeInfo.BATTLEPASS_TEXT3
            )  # TODO LOCALE_GAME

        textInfo = self.GetTextByInfo(missionId, info_1)
        if textInfo != "":
            self.missionInfoDict[1].SetText(
                MISSION_INFO_DICT[missionId][2].format(textInfo)
            )
            self.missionInfoDict[4].SetText(
                MISSION_INFO_DICT[missionId][3].format(textInfo)
            )
        else:
            self.missionInfoDict[1].SetText(MISSION_INFO_DICT[missionId][2])
            self.missionInfoDict[4].SetText(MISSION_INFO_DICT[missionId][3])

        # if missionId == 9:
        # 	self.missionInfoDict[2].SetText(MISSION_INFO_DICT[missionId][1] % localeInfo.AddPointToNumberString(int(info_2)))
        # else:
        self.missionInfoDict[2].SetText(
            MISSION_INFO_DICT[missionId][1].format(
                localeInfo.MoneyFormat(int(info_3 - info_2))
            )
        )

        self.missionInfoDict[3].SetText(
            localeInfo.BATTLEPASS_TEXT4.format(float(info_2 * 100 / info_3))
        )  # TODO LOCALE_GAME

        self.missionInfoDict[5].SetText(MISSION_INFO_DICT[missionId][4])
        self.missionInfoDict[6].SetText(MISSION_INFO_DICT[missionId][5])

        self.missionInfoDict[4].SetPackedFontColor(0xFFF8FFCE)
        self.missionInfoDict[5].SetPackedFontColor(0xFFF8FFCE)
        self.missionInfoDict[6].SetPackedFontColor(0xFFF8FFCE)

    def AddReward(self, itemVnum, itemCount):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True
        if self.rewardItems:
            self.rewardItems.SetItemSlot(self.rewardSlotIndex, itemVnum, itemCount)
            self.rewardDict[self.rewardSlotIndex] = [itemVnum, itemCount]

            self.rewardSlotIndex += 1 if self.rewardSlotIndex != 2 else 2

    def AddMission(self, missionType, missionInfo1, missionInfo2, missionInfo3):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True

        if self.missionList:
            if self.missionList.HaveMission(missionType):
                self.missionList.SetProgress(missionType, missionInfo3, missionInfo2)
            else:
                missionName = (
                    MISSION_INFO_DICT[missionType][0]
                    if MISSION_INFO_DICT.has_key(missionType)
                    else "Unknown name"
                )
                self.missionList.AppendMission(
                    47, missionType, missionName, missionInfo1
                )
                self.missionList.SetProgress(missionType, missionInfo3, missionInfo2)

    def UpdateMission(self, missionType, newProgress):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True

        if self.missionList:
            if self.missionList.HaveMission(missionType):
                self.missionList.UpdateProgress(missionType, newProgress)
                if self.missionList.GetSelectedMission() == missionType:
                    self.SetMissionInfo(missionType)
                self.RefreshGeneralInfo()

    def ClearMissions(self):
        if self.missionList:
            self.missionList.ClearMissions()
        self.Clear()

    # else:
    # 	chat.AppendChat(CHAT_TYPE_INFO, "[Error]: Can't update mission progress.")

    def AddMissionReward(self, missionType, itemVnum, itemCount):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True
        if self.missionList:
            self.missionList.AddMissionReward(missionType, itemVnum, itemCount)

    def OnScroll(self):
        if self.missionList:
            self.missionList.OnScroll(self.scrollBar.GetPos())

    def OnClickTabButton(self, stateKey):
        self.currentState = stateKey

        for tabKey, tabButton in self.tabButtonDict.items():
            if stateKey != tabKey:
                tabButton[0].SetUp()

            tabButton[1].Hide()
            tabButton[2].Hide()

        self.tabButtonDict[stateKey][1].Show()
        self.tabButtonDict[stateKey][2].Show()

        if stateKey == "GENERAL":
            self.RefreshGeneralInfo()

        self.OnClickRankingButton(True)

    def SetBattlePassInfo(self, battlePassID, endTime):
        self.battlePassID = battlePassID

        if endTime > 0:
            self.battlePassEndTime = app.GetGlobalTimeStamp() + endTime

    def RefreshGeneralInfo(self):
        if self.missionList:
            if self.generalInfoDict.has_key(0) and BATTLE_PASS_NAMES.has_key(
                self.battlePassID
            ):
                self.generalInfoDict[0].SetText(
                    localeInfo.BATTLEPASS_TEXT5.format(
                        str(BATTLE_PASS_NAMES[self.battlePassID])
                    )
                )

            missionCount = self.missionList.GetMissionCount()
            completedMissionCount = self.missionList.GetCompletedMissionCount()
            if self.generalInfoDict.has_key(1):
                self.generalInfoDict[1].SetText(
                    localeInfo.BATTLEPASS_TEXT6.format(missionCount)
                )

            if self.generalInfoDict.has_key(2):
                self.generalInfoDict[2].SetText(
                    localeInfo.BATTLEPASS_TEXT7.format(completedMissionCount)
                )

            if missionCount == completedMissionCount:
                self.rewardButton.Enable()
            # self.rewardButton.EnableFlash()

            if self.totalProgressGauge:
                self.totalProgressGauge.SetPercentage(
                    completedMissionCount, missionCount
                )

    def OnClickRewardButton(self):
        appInst.instance().GetNet().SendBattlePassAction(2)
        self.Close()

    def OnClickRankingButton(self, isHide=False):
        if self.rankingWindow.IsShow() or isHide:
            self.rankingWindow.Hide()
            self.missionWindow.Show()
            self.scrollBarWindow.Show()
        else:
            self.rankingWindow.Show()
            self.missionWindow.Hide()
            self.scrollBarWindow.Hide()

            if self.rankingInfo.has_key(1):
                self.RefreshRankingInfo()
            else:
                appInst.instance().GetNet().SendBattlePassAction(3)
                self.requestEndTime = app.GetGlobalTimeStamp() + 5

    def OnClickRankingFirstPrevButton(self):
        self.currentRankingPage = 1
        self.RefreshRankingInfo()

    def OnClickRankingPrevButton(self):
        prevPage = max(1, self.currentRankingPage - 1)
        self.currentRankingPage = prevPage
        self.RefreshRankingInfo()

    def OnClickRankingNextButton(self):
        nextPage = min(5, self.currentRankingPage + 1)
        self.currentRankingPage = nextPage
        self.RefreshRankingInfo()

    def OnClickRankingLastNextButton(self):
        self.currentRankingPage = 5
        self.RefreshRankingInfo()

    def ClickPageButton(self, pageIndex):
        toPage = int(self.pageButtonList[pageIndex].GetText())
        if toPage > 5:
            return

        if toPage == self.currentRankingPage:
            return

        self.currentRankingPage = toPage
        self.RefreshRankingInfo()

    def GetPageByPos(self, pos):
        return int(pos / 10) + 1

    def GetRealPos(self, pos):
        return (pos - 1) if pos < 10 else (pos % 10)

    def AddRanking(self, pos, playerName, finishTime):
        page = self.GetPageByPos(pos)
        if not self.rankingInfo.has_key(page):
            self.rankingInfo[page] = {}

        realPos = self.GetRealPos(pos)
        self.rankingInfo[page][realPos] = {
            "pos": pos,
            "name": playerName,
            "time": finishTime,
        }

    def RefreshRanking(self):
        self.RefreshRankingInfo()

        self.requestEndTime = 0

        if self.loadingBackground:
            self.loadingBackground.Hide()

        if self.loadingAnimation:
            self.loadingAnimation.Hide()

    def RefreshRankingInfo(self):
        for image in self.rankingSpecialImages:
            if self.currentRankingPage == 1:
                image.Show()
            else:
                image.Hide()

        if self.rankingInfo.has_key(self.currentRankingPage):
            for i in xrange(RANKING_MAX_NUM):
                if self.rankingInfo[self.currentRankingPage].has_key(i):
                    self.rankingTexts[i]["pos"].SetText(
                        str(self.rankingInfo[self.currentRankingPage][i]["pos"])
                    )
                    self.rankingTexts[i]["name"].SetText(
                        self.rankingInfo[self.currentRankingPage][i]["name"]
                    )

                    stringTime = datetime.fromtimestamp(
                        self.rankingInfo[self.currentRankingPage][i]["time"]
                    ).strftime("%Y-%m-%d %H:%M")
                    self.rankingTexts[i]["time"].SetText(str(stringTime))
                else:
                    self.rankingTexts[i]["pos"].SetText("")
                    self.rankingTexts[i]["name"].SetText("")
                    self.rankingTexts[i]["time"].SetText("")
        else:
            for i in xrange(RANKING_MAX_NUM):
                self.rankingTexts[i]["pos"].SetText("")
                self.rankingTexts[i]["name"].SetText("")
                self.rankingTexts[i]["time"].SetText("")

        self.RefreshPagination()

    def RefreshPagination(self):
        for page in self.pageButtonList:
            if int(page.GetText()) == self.currentRankingPage:
                page.Down()
                page.Disable()
            else:
                page.SetUp()
                page.Enable()

    def CreateRankingItems(self):
        for i in xrange(RANKING_MAX_NUM):
            # imageName = ROOT_PATH + "ranking_item_over.tga" if name == player.GetName() else ROOT_PATH + "ranking_item_normal.tga"
            itemImage = ui.MakeImageBox(
                self.rankingWindow,
                ROOT_PATH + "ranking_item_normal.tga",
                3,
                25 + (i * 23),
            )
            self.rankingImages.append(itemImage)

            if i < 3:
                imageBorder = ui.MakeImageBox(
                    itemImage, ROOT_PATH + "place_%s_border.tga" % (int(i + 1)), 0, 0
                )
                self.rankingSpecialImages.append(imageBorder)

            self.rankingTexts[i] = {
                "pos": ui.MakeTextLineNew(itemImage, 10, -2),
                "name": ui.MakeTextLineNew(itemImage, 90, -2),
                "time": ui.MakeTextLineNew(itemImage, 191, -2),
            }

        if not self.loadingBackground:
            self.loadingBackground = ui.ImageBox()
            self.loadingBackground.SetParent(self.rankingWindow)
            self.loadingBackground.SetPosition(2, 2)
            self.loadingBackground.LoadImage(ROOT_PATH + "loading_background.tga")
            self.loadingBackground.Show()

        if not self.loadingAnimation:
            self.loadingAnimation = ui.AniImageBox()
            self.loadingAnimation.SetParent(self.rankingWindow)
            self.loadingAnimation.SetDelay(5)

            for i in xrange(12):
                self.loadingAnimation.AppendImage(
                    "d:/ymir work/ui/game/battle_pass/big_loading/%d.tga" % i
                )

            self.loadingAnimation.Show()

    def OnRunMouseWheel(self, nLen):
        if self.IsInPosition():
            x, y = self.GetMouseLocalPosition()

            if not self.scrollBar or not self.scrollBar.IsShow():
                return

            if nLen > 0 and self.scrollBar:
                self.scrollBar.OnUp()
            else:
                self.scrollBar.OnDown()
            return True
        else:
            return False

    def OverInItem(self, slotPos):
        if self.tooltipItem:
            self.tooltipItem.ClearToolTip()
            if slotPos in self.rewardDict:
                metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
                self.tooltipItem.AddItemData(self.rewardDict[slotPos][0], metinSlot, 0)
                self.tooltipItem.ShowToolTip()

    def OverOutItem(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OnUpdate(self):
        if self.currentState == "GENERAL":
            if self.generalInfoDict.has_key(3):
                if self.battlePassEndTime > 0:
                    leftTime = localeInfo.SecondToDHM(
                        self.battlePassEndTime - app.GetGlobalTimeStamp()
                    )
                    # self.generalInfoDict[3].SetText("Timp Ramas: %s" % str(leftTime))
                    self.generalInfoDict[3].SetText(
                        localeInfo.BATTLEPASS_TEXT8.format(leftTime)
                    )
        if self.requestEndTime > 0:
            if self.requestEndTime - app.GetGlobalTimeStamp() <= 0:
                if self.loadingBackground:
                    self.loadingBackground.Hide()

                if self.loadingAnimation:
                    self.loadingAnimation.Hide()

                self.requestEndTime = 0

    def Destroy(self):
        self.ClearDictionary()
        self.Initialize()

    def Open(self):
        if not self.isLoaded:
            self.LoadWindow()
            self.isLoaded = True

        self.SetSize(537, 327)
        self.board.SetSize(537, 327)

        for tabKey, tabButton in self.tabButtonDict.items():
            for tab in tabButton:
                tab.Show()

        self.OnClickTabButton("MISSION")

        if self.missionList:
            self.missionList.SelectMission(1)

            if self.scrollBar:
                missionCount = self.missionList.GetMissionCount()
                if missionCount <= 5:
                    self.scrollBar.Hide()
                else:
                    self.scrollBar.SetScrollStep(float(1.0 / missionCount))

        self.Show()
        self.SetCenterPosition()
        self.SetTop()

    def OpenRanking(self):
        self.rankingWindow.Show()
        self.missionWindow.Hide()
        self.scrollBarWindow.Hide()
        self.RefreshRanking()

        self.SetSize(325, 297)
        self.board.SetSize(325, 297)

        for tabKey, tabButton in self.tabButtonDict.items():
            for tab in tabButton:
                tab.Hide()

        self.Show()
        self.SetCenterPosition()
        self.SetTop()

    def Close(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.Hide()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False

    def OnPressEscapeKey(self):
        self.Close()
        return True


class BattlePassButton(ui.Window):
    def __init__(self):
        ui.Window.__init__(self)

        self.SetPosition(wndMgr.GetScreenWidth() - 120, 250)
        self.SetSize(85, 23)

        self.requestEndTime = 0
        self.openButton = None
        self.loadingImage = None
        self.Initialize()

    def Destroy(self):
        self.openButton = None

    def Initialize(self):
        self.openButton = ui.Button()
        self.openButton.SetParent(self)
        self.openButton.SetPosition(0, 0)
        self.openButton.SetUpVisual(ROOT_PATH + "battle_pass_normal.tga")
        self.openButton.SetOverVisual(ROOT_PATH + "battle_pass_over.tga")
        self.openButton.SetDownVisual(ROOT_PATH + "battle_pass_down.tga")
        self.openButton.SetEvent(self.RequestOpenBattlePass)
        self.openButton.Hide()

        self.loadingImage = ui.AniImageBox()
        self.loadingImage.SetParent(self)
        self.loadingImage.SetDelay(6)

        for i in xrange(12):
            self.loadingImage.AppendImage(ROOT_PATH + "loading/%d.tga" % int(i))

        self.loadingImage.SetPosition(0, 0)
        self.loadingImage.Hide()

    def BindInterface(self, interface):
        self.interface = interface

    def RequestOpenBattlePass(self):
        if self.interface:
            if self.interface.wndBattlePass:
                if self.interface.wndBattlePass.IsShow():
                    self.interface.wndBattlePass.Close()
                    return

        appInst.instance().GetNet().SendBattlePassAction(1)

        self.openButton.Hide()
        self.loadingImage.Show()
        self.requestEndTime = app.GetGlobalTimeStamp() + 5

    def OnUpdate(self):
        if self.requestEndTime > 0:
            if self.requestEndTime - app.GetGlobalTimeStamp() <= 0:
                self.CompleteLoading()
                self.requestEndTime = 0

    def CompleteLoading(self):
        self.requestEndTime = 0
        self.openButton.Show()
        self.loadingImage.Hide()

    def ShowButton(self):
        if self.openButton:
            self.openButton.Show()

    def HideButton(self):
        if self.openButton:
            self.openButton.Hide()
