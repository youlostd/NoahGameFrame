# -*- coding: utf-8 -*-

import app
import grp
import player
from pygame.app import appInst
from pygame.item import MakeItemPosition
from pygame.player import playerInst

import localeInfo
import mouseModule
import playerSettingModule
import ui
import uiToolTip
from ui_event import Event
from uiofflineshop import OfflineShopManager

FACE_IMAGE_DICT = {
    playerSettingModule.RACE_WARRIOR_M: "icon/face/warrior_m.tga",
    playerSettingModule.RACE_WARRIOR_W: "icon/face/warrior_w.tga",
    playerSettingModule.RACE_ASSASSIN_M: "icon/face/assassin_m.tga",
    playerSettingModule.RACE_ASSASSIN_W: "icon/face/assassin_w.tga",
    playerSettingModule.RACE_SURA_M: "icon/face/sura_m.tga",
    playerSettingModule.RACE_SURA_W: "icon/face/sura_w.tga",
    playerSettingModule.RACE_SHAMAN_M: "icon/face/shaman_m.tga",
    playerSettingModule.RACE_SHAMAN_W: "icon/face/shaman_w.tga",
}


class PartyMemberInfoBoard(ui.ScriptWindow):
    BOARD_WIDTH = 106
    BOARD_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.5)
    GAUGE_OUT_LINE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.3)

    LINK_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
    UNLINK_COLOR = grp.GenerateColor(0.5, 0.5, 0.5, 1.0)
    # UNLINK_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)

    PARTY_AFFECT_EXPERIENCE = 0
    PARTY_AFFECT_ATTACKER = 1
    PARTY_AFFECT_TANKER = 2
    PARTY_AFFECT_BUFFER = 3
    PARTY_AFFECT_SKILL_MASTER = 4
    PARTY_AFFECT_BERSERKER = 5
    PARTY_AFFECT_DEFENDER = 6
    # PARTY_AFFECT_TIME_BONUS		= 5
    # PARTY_AFFECT_REGEN_BONUS		= 6
    PARTY_AFFECT_INCREASE_AREA_150 = 7
    PARTY_AFFECT_INCREASE_AREA_200 = 8
    AFFECT_STRING_DICT = {
        PARTY_AFFECT_EXPERIENCE: localeInfo.PARTY_BONUS_EXP,
        PARTY_AFFECT_ATTACKER: localeInfo.PARTY_BONUS_ATTACKER,
        PARTY_AFFECT_TANKER: localeInfo.PARTY_BONUS_TANKER,
        PARTY_AFFECT_BUFFER: localeInfo.PARTY_BONUS_BUFFER,
        PARTY_AFFECT_SKILL_MASTER: localeInfo.PARTY_BONUS_SKILL_MASTER,
        PARTY_AFFECT_BERSERKER: localeInfo.PARTY_BONUS_BERSERKER,
        PARTY_AFFECT_DEFENDER: localeInfo.PARTY_BONUS_DEFENDER,
        PARTY_AFFECT_INCREASE_AREA_150: localeInfo.PARTY_INCREASE_AREA_150,
        PARTY_AFFECT_INCREASE_AREA_200: localeInfo.PARTY_INCREASE_AREA_200,
    }

    PARTY_SKILL_HEAL = 1
    PARTY_SKILL_WARP = 2
    MEMBER_BUTTON_NORMAL = 10
    MEMBER_BUTTON_WARP = 11
    MEMBER_BUTTON_EXPEL = 12
    MEMBER_BUTTON_PATH = "d:/ymir work/ui/game/windows/"
    MEMBER_BUTTON_IMAGE_FILE_NAME_DICT = {
        player.PARTY_STATE_LEADER: "party_state_leader",
        player.PARTY_STATE_ATTACKER: "party_state_attacker",
        player.PARTY_STATE_BERSERKER: "party_state_berserker",
        player.PARTY_STATE_TANKER: "party_state_tanker",
        player.PARTY_STATE_DEFENDER: "party_state_defender",
        player.PARTY_STATE_BUFFER: "party_state_buffer",
        player.PARTY_STATE_SKILL_MASTER: "party_state_skill_master",
        MEMBER_BUTTON_NORMAL: "party_state_normal",
        MEMBER_BUTTON_WARP: "party_skill_warp",
        MEMBER_BUTTON_EXPEL: "party_expel",
    }

    STATE_NAME_DICT = {
        player.PARTY_STATE_ATTACKER: localeInfo.PARTY_SET_ATTACKER,
        player.PARTY_STATE_BERSERKER: localeInfo.PARTY_SET_BERSERKER,
        player.PARTY_STATE_TANKER: localeInfo.PARTY_SET_TANKER,
        player.PARTY_STATE_DEFENDER: localeInfo.PARTY_SET_DEFENDER,
        player.PARTY_STATE_BUFFER: localeInfo.PARTY_SET_BUFFER,
        player.PARTY_STATE_SKILL_MASTER: localeInfo.PARTY_SET_SKILL_MASTER,
    }

    if app.WJ_SHOW_PARTY_ON_MINIMAP:

        class TextToolTip(ui.Window):
            def __init__(self):
                ui.Window.__init__(self, "TOP_MOST")

                textLine = ui.TextLine()
                textLine.SetParent(self)
                if localeInfo.IsARABIC():
                    textLine.SetHorizontalAlignRight()
                else:
                    textLine.SetHorizontalAlignLeft()
                textLine.SetOutline()
                textLine.Show()
                self.textLine = textLine

            def SetText(self, text, x, y):
                self.textLine.SetText(text)
                self.textLine.SetPosition(x, y)

    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.pid = None
        self.vid = None
        self.partyAffectImageList = []
        self.stateButtonDict = {}
        self.affectValueDict = {}
        self.state = -1
        self.isShowStateButton = False

        self.__LoadBoard()
        self.__CreateAffectToolTip()

        if app.WJ_SHOW_PARTY_ON_MINIMAP:
            self.tooltipMapName = self.TextToolTip()
            self.tooltipMapName.Hide()

        self.__CreateStateButton()
        self.Show()

    def __LoadBoard(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PartyMemberInfoBoard.py")

        self.nameTextLine = self.GetChild("NamePrint")
        # self.CharacterImg = self.GetChild("CharacterImg")
        self.gauge = self.GetChild("Gauge")
        self.stateButton = self.GetChild("StateButton")
        self.partyAffectImageList.append(self.GetChild("ExperienceImage"))
        self.partyAffectImageList.append(self.GetChild("AttackerImage"))
        self.partyAffectImageList.append(self.GetChild("DefenderImage"))
        self.partyAffectImageList.append(self.GetChild("BufferImage"))
        self.partyAffectImageList.append(self.GetChild("SkillMasterImage"))
        self.partyAffectImageList.append(self.GetChild("TimeBonusImage"))
        self.partyAffectImageList.append(self.GetChild("RegenBonus"))
        self.partyAffectImageList.append(self.GetChild("IncreaseArea150"))
        self.partyAffectImageList.append(self.GetChild("IncreaseArea200"))
        self.stateButton.SetEvent(self.OnMouseLeftButtonDown)

        if app.WJ_SHOW_PARTY_ON_MINIMAP:
            self.gauge.GaugeImgBoxAddFlag("not_pick")

        self.__SetAffectsMouseEvent()
        self.__HideAllAffects()

        race = appInst.instance().GetNet().GetMainActorRace()
        # self.CharacterImg.LoadImage(FACE_IMAGE_DICT[race])

    def Destroy(self):
        self.ClearDictionary()
        self.nameTextLine = None
        self.gauge = None
        self.stateButton = None
        self.partyAffectImageList = []
        self.stateButtonDict = {}

        self.leaderButton = None
        self.attackerButton = None
        self.tankerButton = None

        self.tooltipMapName.Hide()
        self.tooltipMapName = None

        self.Hide()

    def __SetAffectsMouseEvent(self):
        for i in xrange(len(self.partyAffectImageList)):
            self.partyAffectImageList[i].SetOnMouseOverInEvent(
                Event(self.OnAffectOverIn, i)
            )
        for i in xrange(len(self.partyAffectImageList)):
            self.partyAffectImageList[i].SetOnMouseOverOutEvent(
                Event(self.OnAffectOverOut, i)
            )

    def __HideAllAffects(self):
        for img in self.partyAffectImageList:
            img.Hide()

    def __CreateAffectToolTip(self):
        affectToolTip = uiToolTip.ToolTip(220)
        affectToolTip.Hide()
        self.affectToolTip = affectToolTip

    def __CreateStateButton(self):
        for key, name in self.MEMBER_BUTTON_IMAGE_FILE_NAME_DICT.items():
            if key == player.PARTY_STATE_LEADER:
                continue
            button = ui.Button()
            button.SetUpVisual(self.MEMBER_BUTTON_PATH + name + "_01.sub")
            button.SetOverVisual(self.MEMBER_BUTTON_PATH + name + "_02.sub")
            button.SetDownVisual(self.MEMBER_BUTTON_PATH + name + "_03.sub")
            button.Hide()
            self.stateButtonDict[key] = button

        for state, name in self.STATE_NAME_DICT.items():
            button = self.stateButtonDict[state]
            button.SetToolTipText(name)
            button.SetEvent(Event(self.OnSelectState, state))

        self.stateButtonDict[self.MEMBER_BUTTON_NORMAL].SetEvent(
            Event(self.OnSelectState, -1)
        )
        self.stateButtonDict[self.MEMBER_BUTTON_NORMAL].SetToolTipText(
            localeInfo.PARTY_SET_NORMAL
        )
        self.stateButtonDict[self.MEMBER_BUTTON_WARP].SetEvent(self.OnWarp)
        self.stateButtonDict[self.MEMBER_BUTTON_WARP].SetToolTipText(
            localeInfo.PARTY_RECALL_MEMBER
        )
        self.stateButtonDict[self.MEMBER_BUTTON_EXPEL].SetToolTipText(
            localeInfo.TARGET_BUTTON_EXCLUDE
        )
        self.stateButtonDict[self.MEMBER_BUTTON_EXPEL].SetEvent(self.OnExpel)

    def __GetPartySkillLevel(self):
        slotIndex = playerInst().GetSkillSlotIndexNew(player.SKILL_INDEX_TONGSOL)
        skillGrade = playerInst().GetSkillGrade(slotIndex)
        skillLevel = playerInst().GetSkillLevel(slotIndex)

        return skillLevel

    def __AppendStateButton(self, x, y, state):
        if state == self.state:
            button = self.stateButtonDict[self.MEMBER_BUTTON_NORMAL]
        else:
            button = self.stateButtonDict[state]

        button.SetPosition(x, y)
        button.Show()

    def __ShowStateButton(self):
        self.isShowStateButton = True

        (x, y) = self.GetGlobalPosition()
        xPos = x + 110

        skillLevel = self.__GetPartySkillLevel()

        ## Tanker
        if skillLevel >= 10:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_ATTACKER)
            xPos += 23

        ## Attacker
        if skillLevel >= 20:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_BERSERKER)
            xPos += 23

        ## Tanker
        if skillLevel >= 20:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_TANKER)
            xPos += 23

        ## Buffer
        if skillLevel >= 25:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_BUFFER)
            xPos += 23

        ## Skill Master
        if skillLevel >= 35:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_SKILL_MASTER)
            xPos += 23

        ## Defender
        if skillLevel >= 40:
            self.__AppendStateButton(xPos, y, player.PARTY_STATE_DEFENDER)
            xPos += 23

        # ## Warp
        # if skillLevel >= 35:
        #    if self.stateButtonDict.has_key(self.MEMBER_BUTTON_WARP):
        #        button = self.stateButtonDict[self.MEMBER_BUTTON_WARP]
        #        button.SetPosition(xPos, y)
        #        button.Show()
        #        xPos += 23

        ## Expel
        if self.MEMBER_BUTTON_EXPEL in self.stateButtonDict:
            button = self.stateButtonDict[self.MEMBER_BUTTON_EXPEL]
            button.SetPosition(xPos, y)
            button.Show()
            xPos += 1

    def __HideStateButton(self):
        self.isShowStateButton = False
        for button in self.stateButtonDict.values():
            button.Hide()

    def __GetAffectNumber(self, img):
        for i in xrange(self.partyAffectImageList):
            if img == self.partyAffectImageList[i]:
                return i

        return -1

    def SetCharacterName(self, name):
        self.nameTextLine.SetText(name)

    def GetCharacterName(self):
        return self.nameTextLine.GetText()

    def SetCharacterPID(self, pid):
        self.pid = pid

    def SetCharacterVID(self, vid):
        self.vid = vid

    def GetCharacterPID(self):
        return self.pid

    def GetCharacterVID(self):
        return self.vid

    def SetCharacterHP(self, hpPercentage):
        hpPercentage = max(0, hpPercentage)
        self.gauge.SetPercentage(hpPercentage, 100)

    def SetRace(self, race):
        if race == 10:
            # self.CharacterImg.Hide()
            self.SetCurrentMapName(localeInfo.PARTY_MEMBER_OFFLINE)
            ##self.EmptyFace.Show()
        # else:
        #     try:
        #         self.CharacterImg.LoadImage(FACE_IMAGE_DICT[race])
        #         self.CharacterImg.Show()
        #     except:
        #         self.CharacterImg.Hide()

    def SetCharacterState(self, state):

        if self.state == state:
            return

        self.state = state
        self.stateButton.Show()

        name = self.MEMBER_BUTTON_IMAGE_FILE_NAME_DICT[self.MEMBER_BUTTON_NORMAL]
        if state in self.MEMBER_BUTTON_IMAGE_FILE_NAME_DICT:
            name = self.MEMBER_BUTTON_IMAGE_FILE_NAME_DICT[state]

        self.stateButton.SetUpVisual(self.MEMBER_BUTTON_PATH + name + "_01.sub")
        self.stateButton.SetOverVisual(self.MEMBER_BUTTON_PATH + name + "_02.sub")
        self.stateButton.SetDownVisual(self.MEMBER_BUTTON_PATH + name + "_03.sub")

    def SetAffect(self, affectSlotIndex, affectValue):

        if affectSlotIndex >= len(self.partyAffectImageList):
            return

        if affectValue > 0:
            self.partyAffectImageList[affectSlotIndex].Show()
        else:
            self.partyAffectImageList[affectSlotIndex].Hide()

        self.affectValueDict[affectSlotIndex] = affectValue

    def Link(self):
        self.nameTextLine.SetPackedFontColor(self.LINK_COLOR)
        self.gauge.Show()

    def Unlink(self):
        self.vid = None
        self.nameTextLine.SetPackedFontColor(self.UNLINK_COLOR)
        self.gauge.Hide()
        self.__HideAllAffects()

    def OnSelectState(self, state):

        self.__HideStateButton()
        if state <= 0:
            appInst.instance().GetNet().SendPartySetStatePacket(
                self.pid, self.state, False
            )

        else:

            if self.state <= 0:
                appInst.instance().GetNet().SendPartySetStatePacket(
                    self.pid, state, True
                )

            else:
                appInst.instance().GetNet().SendPartySetStatePacket(
                    self.pid, self.state, False
                )
                appInst.instance().GetNet().SendPartySetStatePacket(
                    self.pid, state, True
                )

    def OnWarp(self):
        self.__HideStateButton()
        if not self.pid:
            return

        appInst.instance().GetNet().SendPartyUseSkillPacket(
            self.PARTY_SKILL_WARP, self.pid
        )

    def OnExpel(self):
        self.__HideStateButton()

        if not self.pid:
            return
        appInst.instance().GetNet().SendPartyRemovePacket(self.pid)

    def OnMouseLeftButtonDown(self):

        if self.vid:
            player.SetTarget(self.vid)
            player.OpenCharacterMenu(self.vid)

            if mouseModule.mouseController.isAttached():
                attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
                if (
                    OfflineShopManager.GetInstance().GetBuilderInstance().IsShow()
                    or OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
                ):
                    return
                appInst.instance().GetNet().SendExchangeStartPacket(self.vid)
                appInst.instance().GetNet().SendExchangeItemAddPacket(
                    MakeItemPosition(attachedSlotPos), 0
                )
                mouseModule.mouseController.DeattachObject()
                return

        if player.IsPartyLeader(playerInst().GetMainCharacterIndex()):
            if self.isShowStateButton:
                self.__HideStateButton()

            else:
                self.__ShowStateButton()

    def OnMouseLeftButtonUp(self):

        if self.vid:
            player.SetTarget(self.vid)
            player.OpenCharacterMenu(self.vid)

            if mouseModule.mouseController.isAttached():
                attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
                if (
                    OfflineShopManager.GetInstance().GetBuilderInstance().IsShow()
                    or OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
                ):
                    return
                appInst.instance().GetNet().SendExchangeStartPacket(self.vid)
                appInst.instance().GetNet().SendExchangeItemAddPacket(
                    MakeItemPosition(attachedSlotPos), 0
                )
                mouseModule.mouseController.DeattachObject()

    def OnMouseRightButtonDown(self):
        self.OnMouseLeftButtonDown()

    def OnAffectOverIn(self, index):

        if index not in self.AFFECT_STRING_DICT:
            return False
        if index not in self.affectValueDict:
            return False

        (x, y) = self.GetGlobalPosition()

        self.affectToolTip.ClearToolTip()
        self.affectToolTip.SetTitle(
            self.AFFECT_STRING_DICT[index].format(self.affectValueDict[index])
        )
        self.affectToolTip.SetToolTipPosition(index * 12, 0)
        self.affectToolTip.SetParent(self)
        self.affectToolTip.ShowToolTip()

        return True

    def OnAffectOverOut(self, index):
        self.affectToolTip.HideToolTip()
        return True

    if app.WJ_SHOW_PARTY_ON_MINIMAP:

        def SetCurrentMapName(self, MapName):
            (x, y) = self.GetGlobalPosition()
            self.tooltipMapName.SetText(MapName, x + 120, y + 10)

        # def OnMouseOverIn(self):
        #     self.tooltipMapName.Show()
        #     return True

        # def OnMouseOverOut(self):
        #     self.tooltipMapName.Hide()
        #     return True


class PartyMenu(ui.ThinBoardOld):
    PARTY_HEAL_ALL_MEMBER = 0
    PARTY_BREAK_UP = 1
    PARTY_LEAVE = 2

    BUTTON_NAME = (
        [PARTY_HEAL_ALL_MEMBER, localeInfo.PARTY_HEAL_ALL_MEMBER],
        [PARTY_BREAK_UP, localeInfo.PARTY_BREAK_UP],
        [PARTY_LEAVE, localeInfo.PARTY_LEAVE],
    )

    def __init__(self):
        ui.ThinBoardOld.__init__(self)
        self.buttonDict = {}
        self.distributionMode = 0
        self.isLeader = False
        self.showingButtonList = []
        self.modeButtonList = {}
        self.__CreateButtons()
        self.__CreateModeButtons()

    def Destroy(self):
        self.buttonDict = {}
        self.showingButtonList = []
        self.modeButtonList = {}

    def __CreateModeButtons(self):

        self.modeTitle = ui.MakeTextLine(self)
        self.modeTitle.SetText(localeInfo.PARTY_EXP_DISTRIBUTION_MODE)

        self.modeButtonList = {}

        level = ui.RadioButton()
        level.SetParent(self)
        level.SetHorizontalAlignCenter()
        level.SetEvent(self.OnClickEXPLevel)
        level.SetUpVisual("d:/ymir work/ui/public/large_button_01.sub")
        level.SetOverVisual("d:/ymir work/ui/public/large_button_02.sub")
        level.SetDownVisual("d:/ymir work/ui/public/large_button_03.sub")
        level.SetText(localeInfo.PARTY_EXP_DISTRIBUTION_MODE_LEVEL)
        level.SetToolTipText(localeInfo.PARTY_EXP_DISTRIBUTION_MODE_LEVEL_TOOLTIP, 70)
        level.Show()
        self.modeButtonList[player.PARTY_EXP_NON_DISTRIBUTION] = level

        parity = ui.RadioButton()
        parity.SetParent(self)
        parity.SetHorizontalAlignCenter()
        parity.SetEvent(self.OnClickEXPDistributeParity)
        parity.SetUpVisual("d:/ymir work/ui/public/large_button_01.sub")
        parity.SetOverVisual("d:/ymir work/ui/public/large_button_02.sub")
        parity.SetDownVisual("d:/ymir work/ui/public/large_button_03.sub")
        parity.SetText(localeInfo.PARTY_EXP_DISTRIBUTION_MODE_PARITY)
        parity.SetToolTipText(localeInfo.PARTY_EXP_DISTRIBUTION_MODE_PARITY_TOOLTIP, 70)
        parity.Show()
        self.modeButtonList[player.PARTY_EXP_DISTRIBUTION_PARITY] = parity

        self.ChangePartyParameter(self.distributionMode)

    def __CreateButtons(self):

        for btnData in self.BUTTON_NAME:
            button = ui.Button()
            button.SetParent(self)
            button.SetHorizontalAlignCenter()
            button.SetToolTipText(btnData[1])
            self.buttonDict[btnData[0]] = button

        self.buttonDict[self.PARTY_HEAL_ALL_MEMBER].SetEvent(self.OnPartyUseSkill)
        self.buttonDict[self.PARTY_HEAL_ALL_MEMBER].SetUpVisual(
            "d:/ymir work/ui/game/windows/Party_Skill_Heal_01.sub"
        )
        self.buttonDict[self.PARTY_HEAL_ALL_MEMBER].SetOverVisual(
            "d:/ymir work/ui/game/windows/Party_Skill_Heal_02.sub"
        )
        self.buttonDict[self.PARTY_HEAL_ALL_MEMBER].SetDownVisual(
            "d:/ymir work/ui/game/windows/Party_Skill_Heal_03.sub"
        )

        self.buttonDict[self.PARTY_BREAK_UP].SetEvent(
            appInst.instance().GetNet().SendPartyExitPacket
        )
        self.buttonDict[self.PARTY_BREAK_UP].SetUpVisual(
            "d:/ymir work/ui/game/windows/Party_Disband_01.sub"
        )
        self.buttonDict[self.PARTY_BREAK_UP].SetOverVisual(
            "d:/ymir work/ui/game/windows/Party_Disband_02.sub"
        )
        self.buttonDict[self.PARTY_BREAK_UP].SetDownVisual(
            "d:/ymir work/ui/game/windows/Party_Disband_03.sub"
        )

        self.buttonDict[self.PARTY_LEAVE].SetEvent(
            appInst.instance().GetNet().SendPartyExitPacket
        )
        self.buttonDict[self.PARTY_LEAVE].SetUpVisual(
            "d:/ymir work/ui/game/windows/Party_Exit_01.sub"
        )
        self.buttonDict[self.PARTY_LEAVE].SetOverVisual(
            "d:/ymir work/ui/game/windows/Party_Exit_02.sub"
        )
        self.buttonDict[self.PARTY_LEAVE].SetDownVisual(
            "d:/ymir work/ui/game/windows/Party_Exit_03.sub"
        )

    def __ClearShowingButtons(self):
        if app.ENABLE_PARTY_MATCH:
            for button in self.showingButtonList:
                button.Hide()

        self.showingButtonList = []

    def __ArrangeButtons(self):

        STEP_SIZE = 37

        showingButtonCount = len(self.showingButtonList)
        xPos = (showingButtonCount - 1) * (-STEP_SIZE / 2)
        for button in self.showingButtonList:
            button.SetPosition(xPos, 15)
            button.Show()
            xPos += 37

        yPos = 85
        for button in self.modeButtonList.values():
            button.SetPosition(0, yPos)
            yPos += 25

        self.UpdateRect()

    def __ShowButton(self, name):
        if name not in self.buttonDict:
            return

        self.showingButtonList.append(self.buttonDict[name])
        self.__ArrangeButtons()

    def __HideButton(self, name):
        if name not in self.buttonDict:
            return

        searchingButton = self.buttonDict[name]
        searchingButton.Hide()
        for btn in self.showingButtonList:
            if btn == searchingButton:
                self.showingButtonList.remove(btn)

        self.__ArrangeButtons()

    def ShowLeaderButton(self):
        self.isLeader = True
        self.__ClearShowingButtons()
        self.__ShowButton(self.PARTY_BREAK_UP)

    def ShowMemberButton(self):
        self.isLeader = False
        self.__ClearShowingButtons()
        self.__ShowButton(self.PARTY_LEAVE)

    def OnPartyUseSkill(self):
        appInst.instance().GetNet().SendPartyUseSkillPacket(
            PartyMemberInfoBoard.PARTY_SKILL_HEAL, 0
        )
        self.__HideButton(self.PARTY_HEAL_ALL_MEMBER)

    def PartyHealReady(self):
        self.__ShowButton(self.PARTY_HEAL_ALL_MEMBER)

    def __UpAllModeButtons(self):
        for button in self.modeButtonList.values():
            button.SetUp()

    def __SetModeButton(self, mode):
        self.__UpAllModeButtons()
        self.modeButtonList[mode].Down()
        self.distributionMode = mode

    def OnClickEXPLevel(self):
        self.__SetModeButton(self.distributionMode)
        if self.isLeader:
            appInst.instance().GetNet().SendPartyParameterPacket(
                player.PARTY_EXP_NON_DISTRIBUTION
            )

    def OnClickEXPDistributeParity(self):
        self.__SetModeButton(self.distributionMode)
        if self.isLeader:
            appInst.instance().GetNet().SendPartyParameterPacket(
                player.PARTY_EXP_DISTRIBUTION_PARITY
            )

    def ChangePartyParameter(self, distributionMode):
        try:
            self.__SetModeButton(distributionMode)
        except:
            pass


class PartyWindow(ui.Window):

    def __init__(self):
        ui.Window.__init__(self)
        self.SetPosition(10, 52)
        self.partyMemberInfoBoardList = []

        self.__CreatePartyMenuButton()
        self.__CreatePartyMenu()

    def Destroy(self):
        self.DestroyPartyMemberInfoBoard()
        self.partyMenu.Destroy()
        self.partyMenuButton = None
        self.partyMenu = None

    def DestroyPartyMemberInfoBoard(self):
        for board in self.partyMemberInfoBoardList:
            board.Destroy()

        self.partyMemberInfoBoardList = []

    def __CreatePartyMenuButton(self):
        partyMenuButton = ui.Button()
        partyMenuButton.SetParent(self)
        partyMenuButton.SetHorizontalAlignCenter()

        partyMenuButton.SetVerticalAlignBottom()
        partyMenuButton.SetPosition(0, 0)
        partyMenuButton.SetUpVisual(
            "d:/ymir work/ui/game/windows/Party_Menu_Open_01.sub"
        )
        partyMenuButton.SetOverVisual(
            "d:/ymir work/ui/game/windows/Party_Menu_Open_02.sub"
        )
        partyMenuButton.SetDownVisual(
            "d:/ymir work/ui/game/windows/Party_Menu_Open_03.sub"
        )
        partyMenuButton.SetEvent(self.OnTogglePartyMenu)
        partyMenuButton.Hide()
        self.partyMenuButton = partyMenuButton

    def __CreatePartyMenu(self):
        partyMenu = PartyMenu()
        partyMenu.SetSize(106, 70 + 70)
        partyMenu.Hide()
        self.partyMenu = partyMenu

    def AddPartyMember(self, pid, name, mapIdx, channel, race):
        board = self.__FindPartyMemberInfoBoardByPID(pid)

        if None == board:
            board = PartyMemberInfoBoard()
            board.SetParent(self)
            board.SetCharacterPID(pid)
            board.SetRace(race)

            self.partyMemberInfoBoardList.append(board)
            self.__ArrangePartyMemberInfoBoard()
            self.UpdateRect()

        if not name:
            name = localeInfo.PARTY_MEMBER_OFFLINE

        view_channel = channel
        if 99 == channel or mapIdx > 10000:
            view_channel = 0

        if mapIdx > 10000:
            mapIdx = mapIdx / 10000

        board.SetCharacterName(name)
        if mapIdx == 0:
            MapName = localeInfo.PARTY_MEMBER_OFFLINE
            board.SetCurrentMapName(MapName)
        else:
            MapName = localeInfo.GetMapNameByIndex(mapIdx)
            board.SetCurrentMapName("CH-" + str(view_channel) + " " + MapName)

        if 0 == channel:
            board.Unlink()
        else:
            board.Link()

        self.Show()
        self.partyMenuButton.Show()

    def RemovePartyMember(self, pid):

        board = self.__FindPartyMemberInfoBoardByPID(pid)

        if None == board:
            return

        vid = board.GetCharacterVID()

        if None != vid and playerInst().IsMainCharacterIndex(vid):

            self.ExitParty()
            player.ExitParty()

        else:

            board.Destroy()
            player.RemovePartyMember(pid)
            self.partyMemberInfoBoardList.remove(board)
            self.__ArrangePartyMemberInfoBoard()
            self.UpdateRect()

    def UpdatePartyMemberInfo(self, pid):
        import chr

        board = self.__FindPartyMemberInfoBoardByPID(pid)

        if None == board:
            return

        state = player.GetPartyMemberState(pid)
        hpPercentage = player.GetPartyMemberHPPercentage(pid)
        affectsList = player.GetPartyMemberAffects(pid)
        race = player.GetPartyMemberRaceByPID(pid)

        # vid = board.GetCharacterVID()
        myVid = playerInst().GetMainCharacterIndex()
        # if vid != None:
        #    chr.SelectInstance(int(vid))
        #    race = chr.GetRace()

        chr.SelectInstance(int(myVid))
        board.SetRace(race)

        board.SetCharacterState(state)
        board.SetCharacterHP(hpPercentage)
        for i in xrange(len(affectsList)):
            board.SetAffect(i, affectsList[i])

        vid = board.GetCharacterVID()
        if None != vid:
            if playerInst().IsMainCharacterIndex(vid):
                if player.PARTY_STATE_LEADER == player.GetPartyMemberState(pid):
                    self.partyMenu.ShowLeaderButton()
                else:
                    self.partyMenu.ShowMemberButton()

    def LinkPartyMember(self, pid, vid, mapIdx, channel, race):

        board = self.__FindPartyMemberInfoBoardByPID(pid)

        if None == board:
            return

        if 99 == channel or mapIdx > 10000:
            channel = 0

        if mapIdx > 10000:
            mapIdx = mapIdx / 10000

        board.Link()
        board.SetCharacterVID(vid)
        board.SetRace(race)

        if mapIdx == 0:
            MapName = localeInfo.PARTY_MEMBER_OFFLINE
            board.SetCurrentMapName(MapName)
        else:
            MapName = localeInfo.GetMapNameByIndex(mapIdx)
            board.SetCurrentMapName("CH-" + str(channel) + " " + MapName)

    def UnlinkPartyMember(self, pid):

        board = self.__FindPartyMemberInfoBoardByPID(pid)

        if None == board:
            return

        board.Unlink()

    def UnlinkAllPartyMember(self):
        for board in self.partyMemberInfoBoardList:
            board.Unlink()

    def ExitParty(self):
        self.partyMenu.Hide()
        self.DestroyPartyMemberInfoBoard()
        self.Hide()
        self.partyMenuButton.Hide()

    def __ArrangePartyMemberInfoBoard(self):

        count = 0
        newHeight = 20

        for board in self.partyMemberInfoBoardList:
            board.SetPosition(0, count * (board.GetHeight() + 2))
            count += 1
            newHeight += board.GetHeight() + 2

        self.SetSize(PartyMemberInfoBoard.BOARD_WIDTH, newHeight)

        (x, y) = self.GetGlobalPosition()
        self.partyMenu.SetPosition(10, y + newHeight + 7)

    def __FindPartyMemberInfoBoardByVID(self, vid):
        for board in self.partyMemberInfoBoardList:
            if vid == board.GetCharacterVID():
                return board

        return None

    def __FindPartyMemberInfoBoardByPID(self, pid):
        for board in self.partyMemberInfoBoardList:
            if pid == board.GetCharacterPID():
                return board

        return None

    def PartyHealReady(self):
        self.partyMenu.PartyHealReady()

    def ChangePartyParameter(self, distributionMode):
        self.partyMenu.ChangePartyParameter(distributionMode)

    def OnTogglePartyMenu(self):
        if self.partyMenu.IsShow():
            self.partyMenuButton.SetUpVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Open_01.sub"
            )
            self.partyMenuButton.SetOverVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Open_02.sub"
            )
            self.partyMenuButton.SetDownVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Open_03.sub"
            )
            self.partyMenu.Hide()
        else:
            self.partyMenuButton.SetUpVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Close_01.sub"
            )
            self.partyMenuButton.SetOverVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Close_02.sub"
            )
            self.partyMenuButton.SetDownVisual(
                "d:/ymir work/ui/game/windows/Party_Menu_Close_03.sub"
            )
            self.partyMenu.Show()
