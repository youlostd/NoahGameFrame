# -*- coding: utf-8 -*-
import logging
from _weakref import proxy
import random
import localeInfo
import app
import wiki
from pack import GetVfsFile
from pygame.npc import npcManager
from pygame.player import playerInst
from pygame.hunting import huntingInst
import item
import ui
from ui_event import Event
import player

ROOT_PATH = "d:/ymir work/ui/game/battle_pass/"
MISSION_PATH = "d:/ymir work/ui/game/battle_pass/mission/"


class Item(ui.WindowWithBaseEvents):
    def __init__(self, mission):
        ui.WindowWithBaseEvents.__init__(self)
        self.SetWindowName("ListBoxMissions_Item")
        self.bIsSelected = False
        self.missionId = mission.id
        self.percentActual = 0
        self.percentTotal = 0
        self.missionInfo1 = 0
        self.mainWindow = None
        self.listImages = []
        self.huntingMission = mission
        iconName = ROOT_PATH + "mission_small_clean.png"

        bgImage = ui.MakeExpandedImageBox(
            self, ROOT_PATH + "mission_bg_normal_no_slots.png", 0, 0, "not_pick"
        )
        iconImage = ui.MakeExpandedImageBox(bgImage, iconName, 2, 5, "not_pick")
        bgGauge = ui.MakeExpandedImageBox(
            bgImage, ROOT_PATH + "mission_progress_empty.tga", 47, 28 + 5, "not_pick"
        )
        self.bgGaugeFull = ui.MakeExpandedImageBox(
            bgGauge, ROOT_PATH + "mission_progress_full.tga", 10, 2, "not_pick"
        )
        self.bgGaugeFull.SetWindowName("gaugeFull")

        self.completedBadge = ui.MakeExpandedImageBox(
            bgImage, ROOT_PATH + "badge_completed.png", 0, 0, "not_pick"
        )
        self.completedBadge.SetWindowName("completedBadge")
        self.completedBadge.SetHorizontalAlignRight()
        self.completedBadge.Hide()

        self.dailyBadge = ui.MakeExpandedImageBox(
            bgImage, ROOT_PATH + "badge_daily.png", 0, 0, "not_pick"
        )
        self.dailyBadge.SetWindowName("completedBadge")
        self.dailyBadge.SetHorizontalAlignRight()
        if mission.isDaily:
            self.dailyBadge.Show()
        else:
            self.dailyBadge.Hide()

        self.requirementCount = 0
        self.requirementsMet = 0

        self.countInfo = ui.TextLine()
        self.countInfo.SetParent(self)
        self.countInfo.SetFontName("Dosis:14m")
        self.countInfo.SetPosition(190, 28)
        self.countInfo.Show()

        self.modelView = ui.ExpandedImageBox()
        self.modelView.SetParent(iconImage)
        self.modelView.RemoveFlag("attach")
        self.modelView.SetPosition(0, 0)

        self.listImages.append(bgImage)
        self.listImages.append(iconImage)
        self.listImages.append(bgGauge)

        self.missionName = ui.MakeTextLineNew(bgImage, 50, -11, "")
        self.missionName.SetFontName("Dosis:14m")

        self.SetSize(bgImage.GetWidth(), bgImage.GetHeight())

    def GetMissionId(self):
        return self.huntingMission.id

    def OnMouseLeftButtonUp(self):
        self.Select()

    def Select(self):
        self.bIsSelected = True
        self.mainWindow.SetSelectedMission(proxy(self), self.huntingMission.id)
        self.UpdateRequirements()

    def Deselect(self):
        self.bIsSelected = False
        self.UpdateRequirements()

    def SetProgress(self, progressActual, pregressTotal):
        self.percentActual = progressActual
        self.percentTotal = pregressTotal

        self.UpdateGauge()

    def UpdateProgress(self, newProgress):
        if newProgress >= self.percentActual:
            self.percentActual = newProgress
            self.UpdateGauge()

    def UpdateRequirements(self):
        try:
            self.requirementCount = len(self.huntingMission.required)
            self.requirementsMet = 0
            for requirement in self.huntingMission.required:
                mission = playerInst().GetHuntingMission(requirement)
                if mission and (
                    mission.bCompleted
                    or mission.killCount
                    == huntingInst().GetHuntingMissionById(requirement).needCount
                ):
                    self.requirementsMet += 1

            if self.requirementsMet != self.requirementCount:
                if self.bIsSelected:
                    if len(self.listImages) > 0:
                        self.listImages[0].LoadImage(
                            ROOT_PATH + "mission_bg_disabled_selected_no_slots.png"
                        )
                else:
                    if len(self.listImages) > 0:
                        self.listImages[0].LoadImage(
                            ROOT_PATH + "mission_bg_disabled_no_slots.png"
                        )

                self.listImages[1].SetDiffuseColor(0.5, 0.5, 0.5, 1.0)
                self.listImages[2].SetDiffuseColor(0.5, 0.5, 0.5, 1.0)
            else:
                if self.bIsSelected:
                    if len(self.listImages) > 0:
                        self.listImages[0].LoadImage(
                            ROOT_PATH + "mission_bg_selected_no_slots.png"
                        )
                else:
                    if len(self.listImages) > 0:
                        self.listImages[0].LoadImage(
                            ROOT_PATH + "mission_bg_normal_no_slots.png"
                        )

                self.listImages[1].SetDiffuseColor(1.0, 1.0, 1.0, 1.0)
                self.listImages[2].SetDiffuseColor(1.0, 1.0, 1.0, 1.0)

        except Exception as e:
            logging.exception(e)

    def UpdateGauge(self):
        self.countInfo.SetText("{}/{}".format(self.percentActual, self.percentTotal))
        self.bgGaugeFull.SetPercentage(self.percentActual, self.percentTotal)
        if self.IsCompleted():
            self.dailyBadge.Hide()
            self.completedBadge.Show()
        self.UpdateRequirements()

    def IsCompleted(self):
        if self.percentActual >= self.percentTotal:
            return True

        return False

    def SetMissionName(self, missionName):
        if self.missionName:
            self.missionName.SetText(missionName)

    def SetMissionTarget(self, vnum):
        self.modelView.LoadImage("{}missions/{}.png".format(ROOT_PATH, vnum))
        self.modelView.Show()

    def SetMissionInfo1(self, missionInfo):
        self.missionInfo1 = missionInfo

    def GetMissionInfo(self):
        return (self.missionInfo1, self.percentActual, self.percentTotal)

    def SetMainWindow(self, window):
        self.mainWindow = proxy(window)

    def GetRewards(self):
        return self.huntingMission.rewards

    def GetMissionData(self):
        return self.huntingMission


class RewardItem(ui.WindowWithBaseEvents):
    VNUMS = (
        28030,
        28031,
        28032,
        28033,
        28034,
        28035,
        28036,
        28037,
        28038,
        28039,
        28040,
        28041,
        28042,
        28043,
    )

    def __init__(self, width):
        ui.WindowWithBaseEvents.__init__(self)
        self.RemoveFlag("not_pick")
        self.isMetin = False
        self.metinCounter = 30

        icon = ui.ExpandedImageBox()
        icon.SetParent(self)
        icon.Show()
        self.icon = icon

        nameLine = ui.TextLine()
        nameLine.SetParent(self)
        nameLine.SetPosition(32 + 5, 0)
        nameLine.SetFontName("Verdana:13")
        nameLine.SetPackedFontColor(0xFFE6B233)
        nameLine.Show()
        self.nameLine = nameLine

        self.SetSize(width, 32 + 5)

    def LoadImage(self, image, name=None):
        self.icon.LoadImage(image)
        self.SetSize(
            self.GetWidth(), self.icon.GetHeight() + 5 * (self.icon.GetHeight() / 32)
        )

        if name is not None:
            self.SetText(name)

    def SetText(self, text):
        self.nameLine.SetText(text)

    def OnUpdate(self):
        if self.isMetin:
            if self.metinCounter == 30:
                self.metinCounter = 0

                item.SelectItem(self.VNUMS[random.randint(0, len(self.VNUMS))])
                self.icon.LoadImage(item.GetIconImageFileName())

            self.metinCounter += 1


class HuntingMissions(ui.ScriptWindow):
    def __init__(
        self,
    ):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.loaded = False
        self.selectedMission = 0
        self.tooltipItem = None

        self.Initialize()

    def Show(self):
        if not self.loaded:
            self.Initialize()

        ui.ScriptWindow.Show(self)
        wiki.ShowModelViewManager(True)
        self.SetCenterPosition()

    def Hide(self):
        super(HuntingMissions, self).Hide()
        wiki.ShowModelViewManager(False)

    def Initialize(self):
        ui.PythonScriptLoader().LoadScriptFile(
            self, "UIScript/HuntingMissionsWindow.py"
        )

        self.GetChild("board").SetCloseEvent(self.Hide)
        self.missionsContent = self.GetChild("BorderMissionsContent")
        self.scrollwindow = self.GetChild("BorderMissions")

        self.rewardScrollWindow = self.GetChild("mission_reward_scroll")
        self.rewardContentWindow = self.GetChild("mission_reward_content")
        self.targetScrollWindow = self.GetChild("mission_target_scroll")
        self.targetContentWindow = self.GetChild("mission_target_content")

        self.missionsContent.SetWidth(298)
        self.rewardContentWindow.SetWidth(207)
        self.targetContentWindow.SetWidth(207)
        for key, value in huntingInst().GetMissions().iteritems():
            self.AddMission(value)

        try:
            self.missionsContent.GetItems()[0].Select()
        except Exception as e:
            logging.debug("No missions?")

        self.loaded = True

    def SetItemToolTip(self, tooltip):
        self.tooltipItem = proxy(tooltip)

    def Refresh(self):
        for itemH in self.missionsContent.GetItems():
            mission = playerInst().GetHuntingMission(itemH.GetMissionId())
            if mission:
                itemH.UpdateProgress(mission.killCount)

            itemH.UpdateRequirements()

        items = self.missionsContent.GetItems()
        items.sort(key=self.MissionSort)
        self.missionsContent.RecalculateHeight()

    def MissionSort(self, key):
        return key.IsCompleted()

    def AddMission(self, huntingMission):

        item = Item(huntingMission)
        item.SetMissionName(localeInfo.Get(huntingMission.name))
        item.SetParent(self.missionsContent)
        item.SetProgress(0, huntingMission.needCount)
        item.SetMissionInfo1(str(huntingMission.minLevel))
        item.SetMissionTarget(huntingMission.mobVnums[0])
        item.SetMainWindow(self)
        item.Show()
        self.missionsContent.AppendItem(item)
        self.missionsContent.RecalculateHeight()

    def SetSelectedMission(self, item, missionId):
        self.selectedMission = missionId

        for itemH in self.missionsContent.GetItems():
            if itemH.GetMissionId() != self.selectedMission:
                itemH.Deselect()

        self.UpdateMissionInfo(item)

    def UpdateMissionInfo(self, item):

        self.targetContentWindow.ClearItems()
        self.rewardContentWindow.ClearItems()

        for reward in item.GetRewards():
            self.AppendRewardItem(self.rewardContentWindow, reward.vnum, reward.count)
            self.rewardContentWindow.RecalculateHeight()

        missionData = item.GetMissionData()

        for idx, vnum in enumerate(missionData.mobVnums):
            self.AppendTarget(self.targetContentWindow, vnum)
            self.targetContentWindow.RecalculateHeight()

    def OnShowItemTooltip(self, vnum):
        self.tooltipItem.ClearToolTip()
        self.tooltipItem.AddItemData(
            vnum, metinSlot=[0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
        )

    def OnHideItemTooltip(self):
        self.tooltipItem.Hide()

    def AppendTarget(self, listBox, vnum):
        myItem = ui.TextLine()
        myItem.SetParent(listBox)
        myItem.SetText(npcManager().GetMonsterName(vnum))
        myItem.SetPackedFontColor(0xFFE6B233)
        listBox.AppendItem(myItem)
        myItem.Show()
        myItem.SetTop()

    def AppendRewardItem(self, listBox, vnum, count):
        myItem = RewardItem(280)
        myItem.SetParent(listBox)
        countString = str(count) + "x "
        if type(vnum) == int:
            if vnum == 3:
                itemName = localeInfo.SKILL_TREE_POINTS
                myItem.LoadImage("icon/item/59009.tga")

            else:
                item.SelectItem(vnum)
                itemName = item.GetItemName()

                myItem.SetOverInEvent(Event(self.OnShowItemTooltip, vnum))
                myItem.LoadImage(item.GetIconImageFileName())

        else:
            item.SelectItem(vnum[0])
            itemName = item.GetItemName()
            gradeFrom = vnum[0] % 10
            gradeTo = vnum[1] % 10
            if gradeFrom == gradeTo:
                itemName = "{} +{}".format(itemName[: itemName.find("+")], gradeFrom)
            else:
                itemName = "{} +{} - +{}".format(
                    itemName[: itemName.find("+")], gradeFrom, gradeTo
                )

            myItem.SetOverInEvent(Event(self.OnShowItemTooltip, vnum[0]))

            myItem.LoadImage(item.GetIconImageFileName())
        myItem.SetText("{}".format(countString + itemName))
        myItem.SetOverOutEvent(self.OnHideItemTooltip)
        listBox.AppendItem(myItem)
        myItem.Show()
        myItem.SetTop()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

        return False
