# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import player
import skill
from pygame.app import appInst
from pygame.player import playerInst

import localeInfo
import mouseModule
import ui
from ui_event import Event
import app

class SkillTree(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.skillToolTip = None
        self.isLoaded = False
        self.refreshToolTip = None

    def __LoadWindow(self, filename):
        if self.isLoaded:
            return

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, filename)
        self.GetChild("destroy").SetEvent(self.Hide)

        self.fireTreeSkills = list(range(180 + 300, 186 + 300)) + [200 + 300]
        self.natureTreeSkills = list(range(186 + 300, 194 + 300)) + [201 + 300]
        self.arcanceTreeSkills = list(range(194 + 300, 200 + 300)) + [202 + 300]

        self.skillPoints = self.GetChild("SkillPointsLabel")

        self.fireTreeSlots = self.GetChild("Skill_Active_Slot_1")
        self.natureTreeSlots = self.GetChild("Skill_Active_Slot_2")
        self.arcanceTreeSlots = self.GetChild("Skill_Active_Slot_3")
        self.skillTrees = [self.fireTreeSlots, self.natureTreeSlots, self.arcanceTreeSlots]
        self.skillTreeIndices = [self.fireTreeSkills, self.natureTreeSkills, self.arcanceTreeSkills]

        for slotIndex in self.fireTreeSkills:
            self.fireTreeSlots.SetSkillSlotNew(slotIndex, slotIndex - 300, 0, 0)
            self.fireTreeSlots.SetItemDiffuseColor(slotIndex, 0.3, 0.3, 0.3, 1.0)
            self.fireTreeSlots.SetOverInItemEvent(self.OverInItem)
            self.fireTreeSlots.SetOverOutItemEvent(self.OverOutItem)
            self.fireTreeSlots.SetPressedSlotButtonEvent(self.OnPressedSlotButton)

            self.fireTreeSlots.SetSlotButton(slotIndex,
                                             "d:/ymir work/ui/skillbaum/plus_button_up.png",
                                             "d:/ymir work/ui/skillbaum/plus_button_over.png",
                                             "d:/ymir work/ui/skillbaum/plus_button_down.png")
            self.fireTreeSlots.SetSlotButtonPosition(slotIndex, 3, 38)

            if slotIndex - 300 == 200:
                self.fireTreeSlots.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radl_off.png")
                self.fireTreeSlots.SetSlotButtonPosition(slotIndex, 100, 220)
                self.fireTreeSlots.SetItemDiffuseColor(slotIndex, 0.0, 0.0, 0.0, 0.0)

        for slotIndex in self.natureTreeSkills:
            self.natureTreeSlots.SetSkillSlotNew(slotIndex, slotIndex - 300, 0, 0)
            self.natureTreeSlots.SetItemDiffuseColor(slotIndex, 0.3, 0.3, 0.3, 1.0)
            self.natureTreeSlots.SetOverInItemEvent(self.OverInItem)
            self.natureTreeSlots.SetOverOutItemEvent(self.OverOutItem)
            self.natureTreeSlots.SetSlotButton(slotIndex,
                                               "d:/ymir work/ui/skillbaum/plus_button_up.png",
                                               "d:/ymir work/ui/skillbaum/plus_button_over.png",
                                               "d:/ymir work/ui/skillbaum/plus_button_down.png")
            self.natureTreeSlots.SetPressedSlotButtonEvent(self.OnPressedSlotButton)

            self.natureTreeSlots.SetSlotButtonPosition(slotIndex, 3, 38)
            if slotIndex - 300 == 201:
                self.natureTreeSlots.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radm_off.png")
                self.natureTreeSlots.SetSlotButtonPosition(slotIndex, 100, 220)
                self.natureTreeSlots.SetItemDiffuseColor(slotIndex, 0.0, 0.0, 0.0, 0.0)

        for slotIndex in self.arcanceTreeSkills:
            self.arcanceTreeSlots.SetSkillSlotNew(slotIndex, slotIndex - 300, 0, 0)
            self.arcanceTreeSlots.SetItemDiffuseColor(slotIndex, 0.3, 0.3, 0.3, 1.0)
            self.arcanceTreeSlots.SetOverInItemEvent(self.OverInItem)
            self.arcanceTreeSlots.SetOverOutItemEvent(self.OverOutItem)
            self.arcanceTreeSlots.SetSlotButton(slotIndex,
                                                "d:/ymir work/ui/skillbaum/plus_button_up.png",
                                                "d:/ymir work/ui/skillbaum/plus_button_over.png",
                                                "d:/ymir work/ui/skillbaum/plus_button_down.png")
            self.arcanceTreeSlots.SetPressedSlotButtonEvent(self.OnPressedSlotButton)

            self.arcanceTreeSlots.SetSlotButtonPosition(slotIndex, 3, 38)
            if slotIndex - 300 == 202:
                self.arcanceTreeSlots.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radr_off.png")
                self.arcanceTreeSlots.SetSlotButtonPosition(slotIndex, 100, 220)
                self.arcanceTreeSlots.SetItemDiffuseColor(slotIndex, 0.0, 0.0, 0.0, 0.0)

        self.RefreshSkill()
        self.isLoaded = True

    def Open(self):
        self.__LoadWindow("UIScript/skillbaum.py")
        self.RefreshSkill()
        self.Show()

    def OnPressedSlotButton(self, slotNumber):
        skillIndex = playerInst().GetSkillIndex(slotNumber)
        appInst.instance().GetNet().SendChatPacket("/skillup " + str(skillIndex))

    def __RefreshSkillPlusButton(self, index):
        slotWindow = self.skillTrees[index]
        slotWindow.HideAllSlotButton()

        statPoint = playerInst().GetPoint(player.SKILL_TREE)

        displayStatPlus = True
        slotWindow.HideAllSlotButton()

        for i in self.skillTreeIndices[index]:
            slotIndex = i
            skillIndex = playerInst().GetSkillIndex(slotIndex)
            skillGrade = playerInst().GetSkillGrade(slotIndex)
            skillLevel = playerInst().GetSkillLevel(slotIndex)
            skillCurrentPercentage = playerInst().GetSkillCurrentEfficientPercentage(slotIndex)

            if not displayStatPlus or skillGrade == 4:
                continue

            if skillIndex == 0:
                continue

            if self.CanShowPlusButton(skillIndex, skillLevel, statPoint, skillCurrentPercentage):
                slotWindow.ShowSlotButton(slotIndex)

        # self.GetChild("Skill_Active_Slot_3").ShowSlotButton(196)
        # self.GetChild("Skill_Active_Slot_3").SetSlotButtonPosition(196, 32, 32)

    def CanShowPlusButton(self, skillIndex, skillLevel, curStatPoint, skillCurrentPercentage):

        if 0 == skillIndex:
            return False

        if skillLevel >= skill.GetSkillMaxLevel(skillIndex):
            return False

        if not skill.CanLevelUpSkill(skillIndex, skillLevel):
            return False

        if curStatPoint < skill.GetSkillLevelStep(skillIndex, skillCurrentPercentage):
            return False

        requirementData = skill.GetSkillRequirementData(skillIndex)
        for index, requireSkill in enumerate(requirementData):
            if not skill.CheckRequirementSueccess(skillIndex, index):
                return False

        return True

    def __RefreshSkillPage(self, index):
        skillPage = self.skillTrees[index]
        skillPage.SetItemDiffuseColor(200 + 300, 0.0, 0.0, 0.0, 0.0)
        skillPage.SetItemDiffuseColor(201 + 300, 0.0, 0.0, 0.0, 0.0)
        skillPage.SetItemDiffuseColor(202 + 300, 0.0, 0.0, 0.0, 0.0)
        getSkillType = skill.GetSkillType
        getSkillIndex = playerInst().GetSkillIndex
        getSkillGrade = playerInst().GetSkillGrade
        getSkillLevel = playerInst().GetSkillDisplayLevel
        getRealSkillLevel = playerInst().GetSkillLevel
        getSkillLevelUpPoint = skill.GetSkillLevelUpPoint
        getSkillMaxLevel = skill.GetSkillMaxLevel

        self.skillPageSlotButtonType = {}

        for slotIndex in self.skillTreeIndices[index]:
            skillIndex = getSkillIndex(slotIndex)
            if 0 == skillIndex:
                continue

            skillGrade = getSkillGrade(slotIndex)
            skillLevel = getSkillLevel(slotIndex)
            realSkillLevel = getRealSkillLevel(slotIndex)
            skillMaxLevel = getSkillMaxLevel(skillIndex)
            skillType = getSkillType(skillIndex)

            skillPage.SetSkillSlotNew(slotIndex, skillIndex, skillLevel, 0)
            if realSkillLevel == skillMaxLevel:
                skillPage.SetSlotSlotText(slotIndex, "Max")
                skillPage.SetSlotTextPositon(slotIndex, 6, 22)
            else:
                skillPage.SetSlotSlotNumber(slotIndex, 0, realSkillLevel)
                skillPage.SetSlotTextPositon(slotIndex, 12, 22)

            if skillIndex in (200, 201, 202):
                skillPage.SetSlotTextPositon(slotIndex, 105, 140)

            if skillLevel == 0:
                if skillIndex not in (200, 201, 202):
                    skillPage.SetItemDiffuseColor(slotIndex, 0.3, 0.3, 0.3, 1.0)
                if skillIndex == 200:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radr_off.png")
                elif skillIndex == 201:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radm_off.png")
                elif skillIndex == 202:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radl_off.png")
            else:
                if skillIndex == 200:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radr_n.png")
                elif skillIndex == 201:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radm_n.png")
                elif skillIndex == 202:
                    skillPage.SetSlotBackground(slotIndex, "d:/ymir work/ui/skillbaum/radl_n.png")

                if skillIndex not in (200, 201, 202):
                    skillPage.SetItemDiffuseColor(slotIndex, 1.0, 1.0, 1.0, 1.0)

            skillPage.RefreshSlot()

    def RefreshSkill(self):
        if not self.isLoaded:
            return
        self.__RefreshSkillPage(0)
        self.__RefreshSkillPage(1)
        self.__RefreshSkillPage(2)
        self.__RefreshSkillPlusButton(0)
        self.__RefreshSkillPlusButton(1)
        self.__RefreshSkillPlusButton(2)
        self.skillPoints.SetText(localeInfo.DottedNumber(playerInst().GetPoint(player.SKILL_TREE)))
        if self.refreshToolTip:
            self.refreshToolTip()

    def SetSkillToolTip(self, skillToolTip):
        self.skillToolTip = skillToolTip

    def OverInItem(self, slotNumber):

        if mouseModule.mouseController.isAttached():
            return

        if not self.skillToolTip:
            return

        skillIndex = playerInst().GetSkillIndex(slotNumber)
        skillLevel = playerInst().GetSkillDisplayLevel(slotNumber)
        skillGrade = playerInst().GetSkillGrade(slotNumber)
        skillType = skill.GetSkillType(skillIndex)

        self.skillToolTip.SetSkillNew(slotNumber, skillIndex, skillGrade, skillLevel)
        self.refreshToolTip = Event(self.OverInItem, slotNumber)

    def OverOutItem(self):
        if 0 != self.skillToolTip:
            self.skillToolTip.HideToolTip()

        self.refreshToolTip = 0
        
    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True