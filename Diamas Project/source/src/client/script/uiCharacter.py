# -*- coding: utf-8 -*-


import logging
import math

import app
import chr
import chat
import player
import quest
import skill
import wndMgr
from pygame.app import RegisterSkill
from pygame.app import appInst
from pygame.player import playerInst

import colorInfo
import constInfo
import emotion
import localeInfo
import mouseModule
import playerSettingModule
import ui
import uiCharacterDetails
import uiPickMoney
import uiQuest
import uiToolTip
from ui_event import Event

SHOW_ONLY_ACTIVE_SKILL = False
SHOW_LIMIT_SUPPORT_SKILL_LIST = []
HIDE_SUPPORT_SKILL_POINT = True

BLOCK_SUPPORT_SKILL_LIST = [125]

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

quest_slot_listbar = {
    "name": "Quest_Slot",
    "type": "listbar",
    "x": 0,
    "y": 0,
    "width": 210,
    "height": 25,
    "text_left": "Quest title",
    "text_position": (27, 0),
}


class CharacterWindow(ui.ScriptWindow):
    ACTIVE_PAGE_SLOT_COUNT = 8
    SUPPORT_PAGE_SLOT_COUNT = 12

    PAGE_SLOT_COUNT = 12

    QUEST_SLOT_MAX_NUM = 10

    # Horse
    HORSE_SLOT_COUNT = 6
    PAGE_HORSE = 2

    SKILL_GROUP_NAME_DICT = {
        playerSettingModule.JOB_WARRIOR: {
            1: localeInfo.SKILL_GROUP_WARRIOR_1,
            2: localeInfo.SKILL_GROUP_WARRIOR_2,
        },
        playerSettingModule.JOB_ASSASSIN: {
            1: localeInfo.SKILL_GROUP_ASSASSIN_1,
            2: localeInfo.SKILL_GROUP_ASSASSIN_2,
        },
        playerSettingModule.JOB_SURA: {
            1: localeInfo.SKILL_GROUP_SURA_1,
            2: localeInfo.SKILL_GROUP_SURA_2,
        },
        playerSettingModule.JOB_SHAMAN: {
            1: localeInfo.SKILL_GROUP_SHAMAN_1,
            2: localeInfo.SKILL_GROUP_SHAMAN_2,
        },
        playerSettingModule.JOB_WOLFMAN: {
            1: localeInfo.JOB_WOLFMAN1,
            2: localeInfo.JOB_WOLFMAN2,
        },
    }

    STAT_DESCRIPTION = {
        "HTH": localeInfo.STAT_TOOLTIP_CON,
        "INT": localeInfo.STAT_TOOLTIP_INT,
        "STR": localeInfo.STAT_TOOLTIP_STR,
        "DEX": localeInfo.STAT_TOOLTIP_DEX,
    }

    MAX_QUEST_HEIGHT = 275

    STAT_MINUS_DESCRIPTION = localeInfo.STAT_MINUS_DESCRIPTION
    SKILL_SLOT_BUTTON_PLUS, SKILL_SLOT_BUTTON_COLOR = xrange(2)

    def __init__(self):
        self.state = "STATUS"
        self.isLoaded = 0
        self.finishFirstLoad = False

        self.toolTipSkill = 0

        if app.ENABLE_DETAILS_UI:
            self.chDetailsWnd = None
            self.ExpandBtn = None
            self.MinimizeBtn = None
            self.isOpenedDetailsWnd = 0
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__Initialize()
        self.__LoadWindow()

        self.statusPlusCommandDict = {
            "HTH": "/con+",
            "INT": "/int+",
            "STR": "/str+",
            "DEX": "/dex+",
        }

        self.statusMinusCommandDict = {
            "HTH-": "/stat- ht",
            "INT-": "/stat- iq",
            "STR-": "/stat- st",
            "DEX-": "/stat- dx",
        }

    def __Initialize(self):
        self.refreshToolTip = None
        self.curSelectedSkillGroup = 0
        self.canUseHorseSkill = -1

        self.toolTip = None
        self.toolTipJob = None
        self.toolTipAlignment = None
        self.toolTipSkill = None

        self.faceImage = None
        self.statusPlusLabel = None
        self.statusPlusValue = None
        self.activeSlot = None
        self.tabDict = None
        self.tabButtonDict = None
        self.pageDict = None
        self.titleBarDict = None
        self.statusPlusButtonDict = None
        self.statusMinusButtonDict = None

        self.skillPageDict = None
        self.skillGroupButton = ()
        self.skillPageSlotButtonType = {}

        self.activeSlot = None
        self.activeSkillPointValue = None
        self.supportSkillPointValue = None
        self.horseSkillPointValue = None
        self.skillGroupButton1 = None
        self.skillGroupButton2 = None
        self.activeSkillGroupName = None

        self.guildNameSlot = None
        self.guildNameValue = None
        self.characterNameSlot = None
        self.characterNameValue = None

        self.emotionToolTip = None
        self.soloEmotionSlot = None
        self.dualEmotionSlot = None
        self.specialEmotionSlot = None

        self.questScrollBar = None
        self.lastScrollPosition = 0
        self.questBoard = None
        self.questPage = None
        self.questTitleBar = None
        self.questShowingStartIndex = 0
        self.wndQuestSlideWindow = {}
        self.wndQuestSlideWindowNewKey = 0
        self.category_flag = -1
        self.cnt = -1
        # self.lp_x = 0
        # self.lp_y = 0
        self.slotIndex = 0
        self.questCategoryList = None
        # self.IsOpenCategoryList = None
        self.IsSameFlag = False
        self.willOpenCategory = None

        # Colored quests/categories
        self.qColorList = {
            "green": 0xFF83C055,
            "blue": 0xFF45678D,
            "golden": 0xFFCAB62F,
            "default_title": 0xFFFFE3AD,
        }

        # New quest stuff
        self.maxConcurrentCategories = 1  # NOTE! The current implementation of the system assumes at some places that
        # there can only be one category open at a given time (as the original did)
        self.openCategories = []
        self.clickedQuests = []
        self.questIndexMap = {}
        self.displayY = 0
        self.extraSpace = 0
        self.counterList = []
        self.clockList = []
        self.catPositionRender = []
        self.baseCutY = 0
        self.questShowingStartIndex = 0
        self.questSlotList = []
        self.questCategoryList = []
        # self.IsOpenCategoryList=[]

        self.tabDict = {}

        self.tabButtonDict = {}

        self.pageDict = {}

        self.titleBarDict = {}

        self.statusPlusButtonDict = {}

        self.statusMinusButtonDict = {
            "HTH-": None,
            "INT-": None,
            "STR-": None,
            "DEX-": None,
        }

        self.skillPageDict = {
            "ACTIVE": None,
            "SUPPORT": None,
            "HORSE": None,
        }

        self.skillPageStatDict = {
            "SUPPORT": player.SKILL_SUPPORT,
            "ACTIVE": player.SKILL_ACTIVE,
            "HORSE": player.SKILL_HORSE,
        }

        self.skillGroupButton = ()

    def Show(self):
        self.__LoadWindow()

        if app.ENABLE_DETAILS_UI:
            self.__InitCharacterDetailsUIButton()
            if self.chDetailsWnd and self.isOpenedDetailsWnd:
                self.chDetailsWnd.Show()

        ui.ScriptWindow.Show(self)

    def Hide(self):
        if app.ENABLE_DETAILS_UI:
            if self.chDetailsWnd:
                self.isOpenedDetailsWnd = self.chDetailsWnd.IsShow()
                self.chDetailsWnd.Close()

        wndMgr.Hide(self.hWnd)

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)

    def __BindObject(self):
        self.toolTip = uiToolTip.ToolTip()
        self.toolTipJob = uiToolTip.ToolTip()
        self.toolTipAlignment = uiToolTip.ToolTip(130)

        self.faceImage = self.GetChild("Face_Image")

        self.statusPlusLabel = self.GetChild("Status_Plus_Label")
        self.statusPlusValue = self.GetChild("Status_Plus_Value")

        self.characterNameSlot = self.GetChild("Character_Name_Slot")
        self.characterNameValue = self.GetChild("Character_Name")
        self.guildNameSlot = self.GetChild("Guild_Name_Slot")
        self.guildNameValue = self.GetChild("Guild_Name")

        self.activeSlot = self.GetChild("Skill_Active_Slot")
        self.activeSkillPointValue = self.GetChild("Active_Skill_Point_Value")
        self.supportSkillPointValue = self.GetChild("Support_Skill_Point_Value")
        self.horseSkillPointValue = self.GetChild("horse_Skill_Point_Value")
        self.skillGroupButton1 = self.GetChild("Skill_Group_Button_1")
        self.skillGroupButton2 = self.GetChild("Skill_Group_Button_2")
        self.activeSkillGroupName = self.GetChild("Active_Skill_Group_Name")

        self.questBoard = self.GetChild("board")
        self.questPage = self.GetChild("Quest_Page")
        self.questTitleBar = self.GetChild("Quest_TitleBar")
        self.questShowingStartIndex = 0
        self.questSlotList = []
        self.questCategoryList = []
        # self.IsOpenCategoryList=[]

        self.tabDict = {
            "STATUS": self.GetChild("Tab_01"),
            "SKILL": self.GetChild("Tab_02"),
            "EMOTICON": self.GetChild("Tab_03"),
            "QUEST": self.GetChild("Tab_04"),
        }

        self.tabButtonDict = {
            "STATUS": self.GetChild("Tab_Button_01"),
            "SKILL": self.GetChild("Tab_Button_02"),
            "EMOTICON": self.GetChild("Tab_Button_03"),
            "QUEST": self.GetChild("Tab_Button_04"),
        }

        self.pageDict = {
            "STATUS": self.GetChild("Character_Page"),
            "SKILL": self.GetChild("Skill_Page"),
            "EMOTICON": self.GetChild("Emoticon_Page"),
            "QUEST": self.GetChild("Quest_Page"),
        }

        self.titleBarDict = {
            "STATUS": self.GetChild("Character_TitleBar"),
            "SKILL": self.GetChild("Skill_TitleBar"),
            "EMOTICON": self.GetChild("Emoticon_TitleBar"),
            "QUEST": self.GetChild("Quest_TitleBar"),
        }

        self.statusPlusButtonDict = {
            "HTH": self.GetChild("HTH_Plus"),
            "INT": self.GetChild("INT_Plus"),
            "STR": self.GetChild("STR_Plus"),
            "DEX": self.GetChild("DEX_Plus"),
        }

        self.statusMinusButtonDict = {
            "HTH-": self.GetChild("HTH_Minus"),
            "INT-": self.GetChild("INT_Minus"),
            "STR-": self.GetChild("STR_Minus"),
            "DEX-": self.GetChild("DEX_Minus"),
        }

        self.skillPageDict = {
            "ACTIVE": self.GetChild("Skill_Active_Slot"),
            "SUPPORT": self.GetChild("Skill_ETC_Slot"),
            "HORSE": self.GetChild("Skill_HORSE_Slot"),
        }

        self.skillPageStatDict = {
            "SUPPORT": player.SKILL_SUPPORT,
            "ACTIVE": player.SKILL_ACTIVE,
            "HORSE": player.SKILL_HORSE,
        }

        self.GetChild("change_conqueror_button").Disable()
        self.GetChild("change_conqueror_button").Down()
        self.GetChild("passive_expanded_btn").Disable()
        self.GetChild("passive_expanded_btn").Down()

        from _weakref import proxy

        self.skillGroupButton = ui.RadioButtonGroup.CreateSelectDefault(
            [
                [proxy(self.skillGroupButton1), Event(self.SelectSkillGroup, 0), None],
                [proxy(self.skillGroupButton2), Event(self.SelectSkillGroup, 1), None],
            ],
            0,
        )

        global SHOW_ONLY_ACTIVE_SKILL
        global HIDE_SUPPORT_SKILL_POINT
        if SHOW_ONLY_ACTIVE_SKILL or HIDE_SUPPORT_SKILL_POINT:
            self.GetChild("Support_Skill_Point_Label").Hide()

        self.soloEmotionSlot = self.GetChild("SoloEmotionSlot")
        self.dualEmotionSlot = self.GetChild("DualEmotionSlot")
        self.specialEmotionSlot = self.GetChild("SpecialEmotionSlot")
        self.__SetEmotionSlot()

        for i in xrange(quest.QUEST_CATEGORY_MAX_NUM):
            self.questCategoryList.append(self.GetChild("Quest_Category_0" + str(i)))
            self.catPositionRender.append(0)

        self.RearrangeCategories(xrange(quest.QUEST_CATEGORY_MAX_NUM))

        self.characterNameSlot.SetOnMouseOverInEvent(self.__ShowAlignmentToolTip)
        self.characterNameSlot.SetOnMouseOverOutEvent(self.__HideAlignmentToolTip)

        if app.ENABLE_DETAILS_UI:
            self.MainBoard = self.GetChild("board")
            self.ExpandBtn = ui.MakeButton(
                self.MainBoard,
                240,
                150,
                "",
                "d:/ymir work/ui/game/belt_inventory/",
                "btn_minimize_normal.tga",
                "btn_minimize_over.tga",
                "btn_minimize_down.tga",
            )

            self.ExpandBtn.SetEvent(self.__ClickExpandButton)

            self.MinimizeBtn = ui.MakeButton(
                self.MainBoard,
                240,
                150,
                "",
                "d:/ymir work/ui/game/belt_inventory/",
                "btn_expand_normal.tga",
                "btn_expand_over.tga",
                "btn_expand_down.tga",
            )
            self.MinimizeBtn.SetEvent(self.__ClickMinimizeButton)

            self.__InitCharacterDetailsUIButton()

    def __InitCharacterDetailsUIButton(self):
        self.ExpandBtn.Show()
        self.MinimizeBtn.Hide()

    def __ClickExpandButton(self):
        if self.chDetailsWnd:
            self.chDetailsWnd.Show()
        else:
            self.chDetailsWnd = uiCharacterDetails.CharacterDetailsUI(self)
            self.chDetailsWnd.Show()

        self.ExpandBtn.Hide()
        self.MinimizeBtn.Show()

    def __ClickMinimizeButton(self):
        if self.chDetailsWnd:
            self.chDetailsWnd.Close()

        self.ExpandBtn.Show()
        self.MinimizeBtn.Hide()

    def __SetSkillSlotEvent(self):
        for skillPageValue in self.skillPageDict.itervalues():
            skillPageValue.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
            skillPageValue.SetSelectItemSlotEvent(self.SelectSkill)
            skillPageValue.SetSelectEmptySlotEvent(self.SelectEmptySlot)
            skillPageValue.SetUnselectItemSlotEvent(self.ClickSkillSlot)
            skillPageValue.SetUseSlotEvent(self.ClickSkillSlot)
            skillPageValue.SetOverInItemEvent(self.OverInItem)
            skillPageValue.SetOverOutItemEvent(self.OverOutItem)
            skillPageValue.SetPressedSlotButtonEvent(self.OnPressedSlotButton)
            skillPageValue.AppendSlotButton(
                "d:/ymir work/ui/game/windows/btn_plus_up.sub",
                "d:/ymir work/ui/game/windows/btn_plus_over.sub",
                "d:/ymir work/ui/game/windows/btn_plus_down.sub",
            )

    def __SetEmotionSlot(self):

        self.emotionToolTip = uiToolTip.ToolTip()

        for slot in (
            self.soloEmotionSlot,
            self.dualEmotionSlot,
            self.specialEmotionSlot,
        ):
            slot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
            slot.SetSelectItemSlotEvent(self.__SelectEmotion)
            slot.SetUnselectItemSlotEvent(self.__ClickEmotionSlot)
            slot.SetUseSlotEvent(self.__ClickEmotionSlot)
            slot.SetOverInItemEvent(self.__OverInEmotion)
            slot.SetOverOutItemEvent(self.__OverOutEmotion)
            slot.AppendSlotButton(
                "d:/ymir work/ui/game/windows/btn_plus_up.sub",
                "d:/ymir work/ui/game/windows/btn_plus_over.sub",
                "d:/ymir work/ui/game/windows/btn_plus_down.sub",
            )

        for slotIdx, datadict in emotion.EMOTION_DICT.items():
            emotionIdx = slotIdx

            slot = self.soloEmotionSlot

            if slotIdx > 20 and slotIdx <= 50:
                slot = self.specialEmotionSlot
            elif slotIdx > 50:
                slot = self.dualEmotionSlot

            slot.SetEmotionSlot(slotIdx, emotionIdx)
            slot.SetCoverButton(slotIdx)

    def __SelectEmotion(self, slotIndex):
        if not slotIndex in emotion.EMOTION_DICT:
            return

        if app.IsPressed(app.VK_CONTROL):
            player.RequestAddToEmptyLocalQuickSlot(player.SLOT_TYPE_EMOTION, slotIndex)
            return

        mouseModule.mouseController.AttachObject(
            self, player.SLOT_TYPE_EMOTION, slotIndex, slotIndex
        )

    def __ClickEmotionSlot(self, slotIndex):
        if not slotIndex in emotion.EMOTION_DICT:
            return

        if player.IsActingEmotion():
            return

        if playerInst().IsFishing():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.FISHING_RENEWAL_CANNOT_EMOTION)
            return

        command = emotion.EMOTION_DICT[slotIndex]["command"]

        if slotIndex > 50:
            vid = playerInst().GetTargetVID()

            if (
                0 == vid
                or vid == playerInst().GetMainCharacterIndex()
                or chr.IsNPC(vid)
                or chr.IsEnemy(vid)
            ):
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.EMOTION_CHOOSE_ONE)
                return

            command += " " + chr.GetNameByVID(vid)

        appInst.instance().GetNet().SendChatPacket(command)

    def ActEmotion(self, emotionIndex):
        self.__ClickEmotionSlot(emotionIndex)

    def __OverInEmotion(self, slotIndex):
        if self.emotionToolTip:

            if not slotIndex in emotion.EMOTION_DICT:
                return

            self.emotionToolTip.ClearToolTip()
            self.emotionToolTip.SetTitle(emotion.EMOTION_DICT[slotIndex]["name"])
            if emotion.EMOTION_DICT[slotIndex]["desc"] != "":
                self.emotionToolTip.AppendDescription(
                    emotion.EMOTION_DICT[slotIndex]["desc"], 27
                )
            self.emotionToolTip.ShowToolTip()

    def __OverOutEmotion(self):
        if self.emotionToolTip:
            self.emotionToolTip.HideToolTip()

    def __BindEvent(self):
        self.RefreshQuest()
        self.__HideJobToolTip()

        for tabKey, tabButton in self.tabButtonDict.items():
            tabButton.SetEvent(Event(self.__OnClickTabButton, tabKey))

        for statusPlusKey, statusPlusButton in self.statusPlusButtonDict.items():
            statusPlusButton.SetEvent(
                Event(self.__OnClickStatusPlusButton, statusPlusKey)
            )
            statusPlusButton.SetShowToolTipEvent(
                Event(self.__OverInStatButton, statusPlusKey)
            )
            statusPlusButton.SetHideToolTipEvent(
                Event(self.__OverOutStatButton, statusPlusKey)
            )

        for statusMinusKey, statusMinusButton in self.statusMinusButtonDict.items():
            statusMinusButton.SetEvent(
                Event(self.__OnClickStatusMinusButton, statusMinusKey)
            )
            statusMinusButton.SetShowToolTipEvent(
                Event(self.__OverInStatMinusButton, statusMinusKey)
            )
            statusMinusButton.SetHideToolTipEvent(
                Event(self.__OverOutStatMinusButton, statusMinusKey)
            )

        for titleBarValue in self.titleBarDict.itervalues():
            titleBarValue.SetCloseEvent(self.Close)

        self.questTitleBar.SetCloseEvent(self.Close)

        for i in xrange(quest.QUEST_CATEGORY_MAX_NUM):
            self.questCategoryList[i].SetEvent(
                Event(self.__OnClickQuestCategoryButton, i)
            )

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.__LoadScript("UIScript/CharacterWindow.py")
        self.isLoaded = 1

        self.__BindObject()
        self.__BindEvent()
        self.questScrollBar = self.GetChild("QuestScrollBar")
        self.questScrollBar.SetScrollEvent(self.__OnScrollQuest)
        self.GetChild("TabControl").SetTop()

        self.SetState("STATUS")

    def Destroy(self):
        self.ClearDictionary()

        self.__Initialize()

    def Close(self):
        if 0 != self.toolTipSkill:
            self.toolTipSkill.Hide()

        if self.chDetailsWnd and self.chDetailsWnd.IsShow():
            self.chDetailsWnd.Hide()

        # 퀘스트 슬라이드 다이얼로그도 같이 없앰
        if self.wndQuestSlideWindowNewKey > 0:
            if self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1] is not None:
                self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1].CloseSelf()
        self.Hide()

    def BindInterfaceClass(self, interface):
        from _weakref import proxy

        self.interface = proxy(interface)

    def SetSkillToolTip(self, toolTipSkill):
        self.toolTipSkill = toolTipSkill

    def __OnClickStatusPlusButton(self, statusKey):
        pickDialog = uiPickMoney.PickMoneyDialog()
        pickDialog.LoadDialog()
        pickDialog.SetMax(3)
        pickDialog.SetTitleName(statusKey)
        pickDialog.SetAcceptEvent(self.__OnAcceptStatusPointDistribute)
        pickDialog.Open(999)
        pickDialog.statusKey = statusKey
        self.pickDialog = pickDialog

    def __OnAcceptStatusPointDistribute(self, value):
        try:
            statusPlusCommand = self.statusPlusCommandDict[self.pickDialog.statusKey]
            maxStat = min(playerInst().GetPoint(player.STAT), value)
            appInst.instance().GetNet().SendChatPacket(
                statusPlusCommand + " " + str(maxStat)
            )
        except KeyError as msg:
            logging.exception(
                "CharacterWindow.__OnClickStatusPlusButton KeyError: %s", msg
            )

    def __OnClickStatusMinusButton(self, statusKey):
        try:
            statusMinusCommand = self.statusMinusCommandDict[statusKey]
            appInst.instance().GetNet().SendChatPacket(statusMinusCommand)
        except KeyError as msg:
            logging.exception(
                "CharacterWindow.__OnClickStatusMinusButton KeyError: %s", msg
            )

    def __OnClickTabButton(self, stateKey):
        self.SetState(stateKey)

    def __OnClickQuestButton(self):
        self.interface.OnClickQuestButton()

    def SetState(self, stateKey):

        self.state = stateKey

        # 탭 눌릴 시 퀘스트 스크롤 및 슬라이드 처리
        if stateKey != "QUEST":
            self.questPage.Hide()
            if self.wndQuestSlideWindowNewKey > 0:
                if (
                    self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1]
                    is not None
                ):
                    self.wndQuestSlideWindow[
                        self.wndQuestSlideWindowNewKey - 1
                    ].CloseSelf()
        else:
            self.__LoadQuestCat()

        for tabKey, tabButton in self.tabButtonDict.items():
            if stateKey != tabKey:
                tabButton.SetUp()

        for tabValue in self.tabDict.itervalues():
            tabValue.Hide()

        for pageValue in self.pageDict.itervalues():
            pageValue.Hide()

        for titleBarValue in self.titleBarDict.itervalues():
            titleBarValue.Hide()

        self.skillGroupButton.OnClick(0)

        self.titleBarDict[stateKey].Show()
        self.tabDict[stateKey].Show()
        self.pageDict[stateKey].Show()

    def GetState(self):
        return self.state

    def __GetTotalAtkText(self):
        minAtk = playerInst().GetPoint(player.ATT_MIN)
        maxAtk = playerInst().GetPoint(player.ATT_MAX)
        atkBonus = playerInst().GetPoint(player.ATT_BONUS)
        attackerBonus = playerInst().GetPoint(player.ATTACKER_BONUS)

        # [MT-360] È£°¢±Í°ÉÀÌ ÀÌ½´
        # Å¬¶ó¿Í ¼­¹ö°£ Ç¥±â Â÷ÀÌ·Î º¯½Å ÁßÀÏ¶§´Â ATT_BONUS °ªÀ» Á¦¿ÜÇÑ´Ù.
        # if player.IsPoly():
        #   atkBonus = 0

        if round(minAtk) == round(maxAtk):
            return "%s" % localeInfo.NumberToString(minAtk)
        else:
            return "%s-%s" % (
                localeInfo.NumberToString(minAtk),
                localeInfo.NumberToString(maxAtk),
            )

    def __GetTotalMagAtkText(self):
        minMagAtk = playerInst().GetPoint(player.MAG_ATT) + playerInst().GetPoint(
            player.MIN_MAGIC_WEP
        )
        maxMagAtk = playerInst().GetPoint(player.MAG_ATT) + playerInst().GetPoint(
            player.MAX_MAGIC_WEP
        )

        if minMagAtk == maxMagAtk:
            return "%s" % localeInfo.NumberToString(minMagAtk)
        else:
            return "%s-%s" % (
                localeInfo.NumberToString(minMagAtk),
                localeInfo.NumberToString(maxMagAtk),
            )

    def __GetTotalDefText(self):
        defValue = playerInst().GetPoint(player.DEF_GRADE)
        if constInfo.ADD_DEF_BONUS_ENABLE:
            defValue += playerInst().GetPoint(player.DEF_BONUS)
        return "%s" % localeInfo.NumberToString(defValue)

    def RefreshStatus(self):
        if self.isLoaded == 0:
            return

        restExp = localeInfo.NumberToString(
            playerInst().GetPoint(player.NEXT_EXP) - playerInst().GetPoint(player.EXP)
        )

        self.GetChild("Level_Value").SetText(
            str(localeInfo.NumberToString(playerInst().GetPoint(player.LEVEL)))
        )
        self.GetChild("Exp_Value").SetText(
            "{} / {}".format(
                localeInfo.NumberToString(playerInst().GetPoint(player.EXP)), restExp
            )
        )
        self.GetChild("Exp_Value").SetHorizontalAlignCenter()
        self.GetChild("HP_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.HP))
            + "/"
            + localeInfo.NumberToString(
                playerInst().GetPoint(player.MAX_HP)
                + playerInst().GetPoint(player.TANKER_BONUS)
            )
        )
        self.GetChild("HP_Value").SetHorizontalAlignCenter()

        self.GetChild("SP_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.SP))
            + "/"
            + localeInfo.NumberToString(playerInst().GetPoint(player.MAX_SP))
        )
        self.GetChild("SP_Value").SetHorizontalAlignCenter()

        self.GetChild("STR_Value").SetText(
            str(localeInfo.NumberToString(playerInst().GetPoint(player.ST)))
        )
        self.GetChild("STR_Value").SetHorizontalAlignCenter()

        self.GetChild("DEX_Value").SetText(
            str(localeInfo.NumberToString(playerInst().GetPoint(player.DX)))
        )
        self.GetChild("DEX_Value").SetHorizontalAlignCenter()

        self.GetChild("HTH_Value").SetText(
            str(localeInfo.NumberToString(playerInst().GetPoint(player.HT)))
        )
        self.GetChild("HTH_Value").SetHorizontalAlignCenter()

        self.GetChild("INT_Value").SetText(
            str(localeInfo.NumberToString(playerInst().GetPoint(player.IQ)))
        )
        self.GetChild("INT_Value").SetHorizontalAlignCenter()

        self.GetChild("ATT_Value").SetText(self.__GetTotalAtkText())
        self.GetChild("ATT_Value").SetHorizontalAlignCenter()

        self.GetChild("DEF_Value").SetText(self.__GetTotalDefText())
        self.GetChild("DEF_Value").SetHorizontalAlignCenter()

        self.GetChild("MATT_Value").SetText(self.__GetTotalMagAtkText())
        self.GetChild("MATT_Value").SetHorizontalAlignCenter()

        self.GetChild("MDEF_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.MAG_DEF))
        )
        self.GetChild("MDEF_Value").SetHorizontalAlignCenter()

        self.GetChild("ASPD_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.ATT_SPEED))
        )
        self.GetChild("ASPD_Value").SetHorizontalAlignCenter()

        self.GetChild("MSPD_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.MOVING_SPEED))
        )
        self.GetChild("MSPD_Value").SetHorizontalAlignCenter()

        self.GetChild("CSPD_Value").SetText(
            localeInfo.NumberToString(playerInst().GetPoint(player.CASTING_SPEED))
        )
        self.GetChild("CSPD_Value").SetHorizontalAlignCenter()

        self.GetChild("ER_Value").SetText("0")
        self.GetChild("ER_Value").SetHorizontalAlignCenter()

        self.__RefreshStatusPlusButtonList()
        self.__RefreshStatusMinusButtonList()
        self.RefreshAlignment()

        if self.refreshToolTip:
            self.refreshToolTip()

        if self.chDetailsWnd:
            self.chDetailsWnd.RefreshLabel()

    def __RefreshStatusPlusButtonList(self):
        if self.isLoaded == 0:
            return

        statusPlusPoint = playerInst().GetPoint(player.STAT)

        if statusPlusPoint > 0:
            self.statusPlusValue.SetText(localeInfo.DottedNumber(statusPlusPoint))
            self.statusPlusLabel.Show()
            self.ShowStatusPlusButtonList()
        else:
            self.statusPlusValue.SetText(str(0))
            self.statusPlusLabel.Hide()
            self.HideStatusPlusButtonList()

    def __RefreshStatusMinusButtonList(self):
        if self.isLoaded == 0:
            return

        statusMinusPoint = self.__GetStatMinusPoint()

        if statusMinusPoint > 0:
            self.__ShowStatusMinusButtonList()
        else:
            self.__HideStatusMinusButtonList()

    def RefreshAlignment(self):
        try:
            point, grade = player.GetAlignmentData()

            COLOR_DICT = {
                0: colorInfo.TITLE_RGB_GOOD_35,
                1: colorInfo.TITLE_RGB_GOOD_34,
                2: colorInfo.TITLE_RGB_GOOD_33,
                3: colorInfo.TITLE_RGB_GOOD_32,
                4: colorInfo.TITLE_RGB_GOOD_31,
                5: colorInfo.TITLE_RGB_GOOD_30,
                6: colorInfo.TITLE_RGB_GOOD_29,
                7: colorInfo.TITLE_RGB_GOOD_28,
                8: colorInfo.TITLE_RGB_GOOD_27,
                9: colorInfo.TITLE_RGB_GOOD_26,
                10: colorInfo.TITLE_RGB_GOOD_25,
                11: colorInfo.TITLE_RGB_GOOD_24,
                12: colorInfo.TITLE_RGB_GOOD_23,
                13: colorInfo.TITLE_RGB_GOOD_22,
                14: colorInfo.TITLE_RGB_GOOD_21,
                15: colorInfo.TITLE_RGB_GOOD_20,
                16: colorInfo.TITLE_RGB_GOOD_19,
                17: colorInfo.TITLE_RGB_GOOD_18,
                18: colorInfo.TITLE_RGB_GOOD_17,
                19: colorInfo.TITLE_RGB_GOOD_16,
                20: colorInfo.TITLE_RGB_GOOD_15,
                21: colorInfo.TITLE_RGB_GOOD_14,
                22: colorInfo.TITLE_RGB_GOOD_13,
                23: colorInfo.TITLE_RGB_GOOD_12,
                24: colorInfo.TITLE_RGB_GOOD_11,
                25: colorInfo.TITLE_RGB_GOOD_10,
                26: colorInfo.TITLE_RGB_GOOD_9,
                27: colorInfo.TITLE_RGB_GOOD_8,
                28: colorInfo.TITLE_RGB_GOOD_7,
                29: colorInfo.TITLE_RGB_GOOD_6,
                30: colorInfo.TITLE_RGB_GOOD_5,
                31: colorInfo.TITLE_RGB_GOOD_4,
                32: colorInfo.TITLE_RGB_GOOD_3,
                33: colorInfo.TITLE_RGB_GOOD_2,
                34: colorInfo.TITLE_RGB_GOOD_1,
                35: colorInfo.TITLE_RGB_NORMAL,
                36: colorInfo.TITLE_RGB_EVIL_1,
                37: colorInfo.TITLE_RGB_EVIL_2,
                38: colorInfo.TITLE_RGB_EVIL_3,
                39: colorInfo.TITLE_RGB_EVIL_4,
                40: colorInfo.TITLE_RGB_EVIL_5,
                41: colorInfo.TITLE_RGB_EVIL_6,
                42: colorInfo.TITLE_RGB_EVIL_7,
                43: colorInfo.TITLE_RGB_EVIL_8,
                44: colorInfo.TITLE_RGB_EVIL_9,
                45: colorInfo.TITLE_RGB_EVIL_10,
                46: colorInfo.TITLE_RGB_EVIL_11,
                47: colorInfo.TITLE_RGB_EVIL_12,
                48: colorInfo.TITLE_RGB_EVIL_13,
                49: colorInfo.TITLE_RGB_EVIL_14,
                50: colorInfo.TITLE_RGB_EVIL_15,
                51: colorInfo.TITLE_RGB_EVIL_16,
                52: colorInfo.TITLE_RGB_EVIL_17,
                53: colorInfo.TITLE_RGB_EVIL_18,
                54: colorInfo.TITLE_RGB_EVIL_19,
                55: colorInfo.TITLE_RGB_EVIL_20,
                56: colorInfo.TITLE_RGB_EVIL_21,
                57: colorInfo.TITLE_RGB_EVIL_22,
                58: colorInfo.TITLE_RGB_EVIL_23,
                59: colorInfo.TITLE_RGB_EVIL_24,
                60: colorInfo.TITLE_RGB_EVIL_25,
                61: colorInfo.TITLE_RGB_EVIL_26,
            }
            colorList = COLOR_DICT.get(grade, colorInfo.TITLE_RGB_NORMAL)
            gradeColor = ui.GenerateColor(colorList[0], colorList[1], colorList[2])

            self.toolTipAlignment.ClearToolTip()

            race = player.GetRace()
            job = chr.RaceToJob(race)
            sex = chr.RaceToSex(race)

            if sex == 0:
                self.toolTipAlignment.AppendTextLine(
                    localeInfo.TITLE_NAME_LIST[grade], gradeColor
                )
            else:
                self.toolTipAlignment.AppendTextLine(
                    localeInfo.TITLE_NAME_LIST_FEMALE[grade], gradeColor
                )

            self.toolTipAlignment.AppendTextLine(localeInfo.ALIGNMENT_NAME + str(point))
            self.toolTipAlignment.SetSize(170, 60)
        except:
            pass

    def __ShowStatusMinusButtonList(self):
        for stateMinusKey, statusMinusButton in self.statusMinusButtonDict.items():
            statusMinusButton.Show()

    def __HideStatusMinusButtonList(self):
        for stateMinusKey, statusMinusButton in self.statusMinusButtonDict.items():
            statusMinusButton.Hide()

    def ShowStatusPlusButtonList(self):
        for statePlusKey, statusPlusButton in self.statusPlusButtonDict.items():
            statusPlusButton.Show()

    def HideStatusPlusButtonList(self):
        for statePlusKey, statusPlusButton in self.statusPlusButtonDict.items():
            statusPlusButton.Hide()

    def SelectSkill(self, skillSlotIndex):

        mouseController = mouseModule.mouseController

        if False == mouseController.isAttached():

            srcSlotIndex = self.__RealSkillSlotToSourceSlot(skillSlotIndex)
            selectedSkillIndex = playerInst().GetSkillIndex(srcSlotIndex)

            if skill.CanUseSkill(selectedSkillIndex):

                if app.IsPressed(app.VK_CONTROL):
                    player.RequestAddToEmptyLocalQuickSlot(
                        player.SLOT_TYPE_SKILL, srcSlotIndex
                    )
                    return

                mouseController.AttachObject(
                    self, player.SLOT_TYPE_SKILL, srcSlotIndex, selectedSkillIndex
                )

        else:

            mouseController.DeattachObject()

    def SelectEmptySlot(self, SlotIndex):
        mouseModule.mouseController.DeattachObject()

    ## ToolTip
    def OverInItem(self, slotNumber):

        if mouseModule.mouseController.isAttached():
            return

        if 0 == self.toolTipSkill:
            return

        srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotNumber)
        skillIndex = playerInst().GetSkillIndex(srcSlotIndex)
        skillLevel = playerInst().GetSkillDisplayLevel(srcSlotIndex)
        skillGrade = playerInst().GetSkillGrade(srcSlotIndex)
        skillType = skill.GetSkillType(skillIndex)
        ## ACTIVE
        if skill.SKILL_TYPE_ACTIVE == skillType:
            overInSkillGrade = self.__GetSkillGradeFromSlot(slotNumber)

            if overInSkillGrade >= 2 or overInSkillGrade == skillGrade:
                self.toolTipSkill.SetSkillNew(
                    srcSlotIndex, skillIndex, skillGrade, skillLevel
                )
            else:
                self.toolTipSkill.SetSkillOnlyName(
                    srcSlotIndex, skillIndex, overInSkillGrade
                )
        else:
            self.toolTipSkill.SetSkillNew(
                srcSlotIndex, skillIndex, skillGrade, skillLevel
            )

    def OverOutItem(self):
        if 0 != self.toolTipSkill:
            self.toolTipSkill.HideToolTip()

    def __GetStatMinusPoint(self):
        POINT_STAT_RESET_COUNT = 112
        return playerInst().GetPoint(POINT_STAT_RESET_COUNT)

    def __OverInStatMinusButton(self, stat):
        try:
            self.__ShowStatToolTip(
                self.STAT_MINUS_DESCRIPTION[stat] % self.__GetStatMinusPoint()
            )
        except KeyError:
            pass

        self.refreshToolTip = Event(self.__OverInStatMinusButton, stat)

    def __OverOutStatMinusButton(self):
        self.__HideStatToolTip()
        self.refreshToolTip = 0

    def __OverInStatButton(self, stat):
        try:
            self.__ShowStatToolTip(self.STAT_DESCRIPTION[stat])
        except KeyError:
            pass

    def __OverOutStatButton(self):
        self.__HideStatToolTip()

    def __ShowStatToolTip(self, statDesc):
        self.toolTip.ClearToolTip()
        self.toolTip.AppendTextLine(statDesc)
        self.toolTip.Show()

    def __HideStatToolTip(self):
        self.toolTip.Hide()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def OnPressEscapeKey(self):
        if self.wndQuestSlideWindowNewKey > 0:
            if self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1] is not None:
                self.wndQuestSlideWindow[
                    self.wndQuestSlideWindowNewKey - 1
                ].OnPressEscapeKey()
        self.Close()
        return True

    def OnUpdate(self):
        self.__UpdateQuestClock()

    ## Skill Process
    def __RefreshSkillPage(self, name, slotCount):
        global SHOW_LIMIT_SUPPORT_SKILL_LIST
        global BLOCK_SUPPORT_SKILL_LIST

        skillPage = self.skillPageDict[name]

        startSlotIndex = skillPage.GetStartIndex()
        if "ACTIVE" == name:
            if self.PAGE_HORSE == self.curSelectedSkillGroup:
                startSlotIndex += slotCount

        getSkillType = skill.GetSkillType
        self.skillPageSlotButtonType = {}

        for i in xrange(slotCount + 1):
            slotIndex = i + startSlotIndex
            skillIndex = playerInst().GetSkillIndex(slotIndex)

            for j in xrange(skill.SKILL_GRADE_COUNT):
                skillPage.ClearSlot(self.__GetRealSkillSlot(j, i))

            if 0 == skillIndex:
                continue

            if skillIndex in BLOCK_SUPPORT_SKILL_LIST:
                continue

            skillGrade = playerInst().GetSkillGrade(slotIndex)
            skillLevel = playerInst().GetSkillDisplayLevel(slotIndex)
            skillType = getSkillType(skillIndex)

            ## 승마 스킬 예외 처리
            if player.SKILL_INDEX_RIDING == skillIndex:
                skillPage.SetSkillSlotNew(
                    slotIndex, skillIndex, max(skillLevel - 1, 0), skillLevel
                )
                skillPage.SetSlotCount(slotIndex, skillLevel)

            ## ACTIVE
            elif skill.SKILL_TYPE_ACTIVE == skillType:
                for j in xrange(skill.SKILL_GRADE_COUNT):
                    realSlotIndex = self.__GetRealSkillSlot(j, slotIndex)
                    skillPage.SetSkillSlotNew(realSlotIndex, skillIndex, j, skillLevel)
                    skillPage.SetCoverButton(realSlotIndex)

                    if (skillGrade >= skill.SKILL_GRADE_COUNT - 2) and j >= (
                        skill.SKILL_GRADE_COUNT - 2
                    ):
                        skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)
                    elif (not self.__CanUseSkillNow()) or (skillGrade != j):
                        skillPage.SetSlotCount(realSlotIndex, 0)
                        skillPage.DisableCoverButton(realSlotIndex)
                        skillPage.DeactivateSlot(realSlotIndex)
                    else:
                        skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)

                    if (
                        player.IsSkillActive(slotIndex)
                        and min(skillGrade, skill.SKILL_GRADE_COUNT - 2) == j
                    ):
                        skillPage.ActivateSlot(realSlotIndex)

            ## 그외
            else:
                if (
                    not SHOW_LIMIT_SUPPORT_SKILL_LIST
                    or skillIndex in SHOW_LIMIT_SUPPORT_SKILL_LIST
                ):
                    realSlotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)
                    skillPage.SetSkillSlot(realSlotIndex, skillIndex, skillLevel)
                    skillPage.SetSlotCountNew(realSlotIndex, skillGrade, skillLevel)

                    if skill.CanUseSkill(skillIndex):
                        skillPage.SetCoverButton(realSlotIndex)

            skillPage.RefreshSlot()

        self.__RestoreSlotCoolTime(skillPage)

    def __RestoreSlotCoolTime(self, skillPage):

        restoreType = skill.SKILL_TYPE_NONE
        if self.PAGE_HORSE == self.curSelectedSkillGroup:
            restoreType = skill.SKILL_TYPE_HORSE
        else:
            restoreType = skill.SKILL_TYPE_ACTIVE

        skillPage.RestoreSlotCoolTime(restoreType)

    def RefreshSkill(self):

        if self.isLoaded == 0:
            return

        if self.__IsChangedHorseRidingSkillLevel():
            self.RefreshCharacter()
            return

        global SHOW_ONLY_ACTIVE_SKILL
        if SHOW_ONLY_ACTIVE_SKILL:
            self.__RefreshSkillPage("ACTIVE", self.ACTIVE_PAGE_SLOT_COUNT)
        else:
            self.__RefreshSkillPage("ACTIVE", self.ACTIVE_PAGE_SLOT_COUNT)
            self.__RefreshSkillPage("HORSE", self.HORSE_SLOT_COUNT)
            self.__RefreshSkillPage("SUPPORT", self.SUPPORT_PAGE_SLOT_COUNT)

        self.RefreshSkillPlusPointLabel()
        self.RefreshSkillSlotButtonList()

    def CanShowPlusButton(self, skillIndex, skillLevel, curStatPoint):

        ## 스킬이 있으면
        if 0 == skillIndex:
            return False

        ## 레벨업 조건을 만족한다면
        if not skill.CanLevelUpSkill(skillIndex, skillLevel):
            return False

        return True

    def __RefreshSkillPlusButton(self, name):
        global HIDE_SUPPORT_SKILL_POINT
        if HIDE_SUPPORT_SKILL_POINT and "SUPPORT" == name:
            return

        slotWindow = self.skillPageDict[name]
        slotWindow.HideAllSlotButton()

        slotStatType = self.skillPageStatDict[name]
        if 0 == slotStatType:
            return

        statPoint = playerInst().GetPoint(slotStatType)
        startSlotIndex = slotWindow.GetStartIndex()
        if "HORSE" == name:
            startSlotIndex += self.ACTIVE_PAGE_SLOT_COUNT

        displayStatPlus = statPoint > 0

        for i in xrange(self.PAGE_SLOT_COUNT):
            slotIndex = i + startSlotIndex
            skillIndex = playerInst().GetSkillIndex(slotIndex)
            skillGrade = playerInst().GetSkillGrade(slotIndex)
            skillLevel = playerInst().GetSkillLevel(slotIndex)

            if displayStatPlus and not skillGrade == 4:
                continue

            if skillIndex == 0:
                continue
            if skillGrade != 0:
                continue

            if name == "HORSE":
                if playerInst().GetPoint(player.LEVEL) >= skill.GetSkillLevelLimit(
                    skillIndex
                ):
                    if skillLevel < 20:
                        slotWindow.ShowSlotButton(
                            self.__GetETCSkillRealSlotIndex(slotIndex)
                        )

            else:
                if "SUPPORT" == name:
                    if (
                        not SHOW_LIMIT_SUPPORT_SKILL_LIST
                        or skillIndex in SHOW_LIMIT_SUPPORT_SKILL_LIST
                    ):
                        if self.CanShowPlusButton(skillIndex, skillLevel, statPoint):
                            slotWindow.ShowSlotButton(slotIndex)
                else:
                    if self.CanShowPlusButton(skillIndex, skillLevel, statPoint):
                        slotWindow.ShowSlotButton(slotIndex)

    def __RefreshSkillSlotButton(self, name):
        global HIDE_SUPPORT_SKILL_POINT
        if HIDE_SUPPORT_SKILL_POINT and "SUPPORT" == name:
            return

        slotWindow = self.skillPageDict[name]
        slotWindow.HideAllSlotButton()

        slotStatType = self.skillPageStatDict[name]
        if 0 == slotStatType:
            return

        statPoint = playerInst().GetPoint(slotStatType)
        startSlotIndex = slotWindow.GetStartIndex()
        if "HORSE" == name:
            startSlotIndex += self.ACTIVE_PAGE_SLOT_COUNT

        self.skillPageSlotButtonType = {}

        for i in xrange(self.PAGE_SLOT_COUNT):
            slotIndex = i + startSlotIndex
            skillIndex = playerInst().GetSkillIndex(slotIndex)
            skillGrade = playerInst().GetSkillGrade(slotIndex)
            skillLevel = playerInst().GetSkillLevel(slotIndex)

            if skillIndex == 0:
                continue

            showPlus = False
            showEditColor = skillGrade == 4

            if statPoint != 0 and skillGrade == 0:
                if name == "HORSE":
                    if playerInst().GetPoint(player.LEVEL) >= skill.GetSkillLevelLimit(
                        skillIndex
                    ):
                        if skillLevel < 20:
                            slotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)
                            showPlus = True
                else:
                    if "SUPPORT" == name:
                        if (
                            not SHOW_LIMIT_SUPPORT_SKILL_LIST
                            or skillIndex in SHOW_LIMIT_SUPPORT_SKILL_LIST
                        ):
                            showPlus = self.CanShowPlusButton(
                                skillIndex, skillLevel, statPoint
                            )
                    else:
                        showPlus = self.CanShowPlusButton(
                            skillIndex, skillLevel, statPoint
                        )

            if showPlus:
                slotWindow.SetSlotButton(
                    slotIndex,
                    "d:/ymir work/ui/game/windows/btn_plus_up.sub",
                    "d:/ymir work/ui/game/windows/btn_plus_over.sub",
                    "d:/ymir work/ui/game/windows/btn_plus_down.sub",
                )
                slotWindow.ShowSlotButton(slotIndex)

                self.skillPageSlotButtonType[slotIndex] = self.SKILL_SLOT_BUTTON_PLUS

            if name == "ACTIVE" and showEditColor and not showPlus:
                realSlotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)
                slotWindow.SetSlotButton(
                    realSlotIndex,
                    "d:/ymir work/ui/game/windows/btn_skillcolor_up.png",
                    "d:/ymir work/ui/game/windows/btn_skillcolor_over.png",
                    "d:/ymir work/ui/game/windows/btn_skillcolor_down.png",
                )
                slotWindow.ShowSlotButton(realSlotIndex)

                self.skillPageSlotButtonType[realSlotIndex] = (
                    self.SKILL_SLOT_BUTTON_COLOR
                )

    def RefreshSkillSlotButtonList(self):
        if self.isLoaded == 0:
            return

        if not self.__CanUseSkillNow():
            return

        if self.PAGE_HORSE == self.curSelectedSkillGroup:
            self.__RefreshSkillSlotButton("HORSE")
        else:
            self.__RefreshSkillSlotButton("ACTIVE")

        self.__RefreshSkillSlotButton("SUPPORT")

    def RefreshSkillPlusPointLabel(self):
        if self.isLoaded == 0:
            return

        if self.PAGE_HORSE == self.curSelectedSkillGroup:
            activeStatPoint = playerInst().GetPoint(player.SKILL_HORSE)
            self.activeSkillPointValue.SetText(
                localeInfo.NumberToString(activeStatPoint)
            )
        else:
            activeStatPoint = playerInst().GetPoint(player.SKILL_ACTIVE)
            self.activeSkillPointValue.SetText(
                localeInfo.NumberToString(activeStatPoint)
            )

        supportStatPoint = max(0, playerInst().GetPoint(player.SKILL_SUPPORT))
        horseStatPoint = max(0, playerInst().GetPoint(player.SKILL_HORSE))
        self.supportSkillPointValue.SetText(localeInfo.NumberToString(supportStatPoint))
        self.horseSkillPointValue.SetText(localeInfo.NumberToString(horseStatPoint))

    ## Skill Level Up Button
    def OnPressedSlotButton(self, slotNumber):
        srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotNumber)
        buttonType = self.skillPageSlotButtonType[slotNumber]
        skillIndex = playerInst().GetSkillIndex(srcSlotIndex)

        if buttonType == self.SKILL_SLOT_BUTTON_PLUS:
            appInst.instance().GetNet().SendChatPacket("/skillup " + str(skillIndex))
            if self.interface:
                self.interface.RefreshQuickslot()

        if buttonType == self.SKILL_SLOT_BUTTON_COLOR:
            self.interface.OpenSkillColorSelection(srcSlotIndex)

    ## Use Skill
    def ClickSkillSlot(self, slotIndex):

        srcSlotIndex = self.__RealSkillSlotToSourceSlot(slotIndex)
        skillIndex = playerInst().GetSkillIndex(srcSlotIndex)
        skillType = skill.GetSkillType(skillIndex)

        if not self.__CanUseSkillNow():
            if skill.SKILL_TYPE_ACTIVE == skillType:
                return

        for slotWindow in self.skillPageDict.values():
            if slotWindow.HasSlot(slotIndex):
                if skill.CanUseSkill(skillIndex):
                    player.ClickSkillSlot(srcSlotIndex)
                    return

        mouseModule.mouseController.DeattachObject()

    ## FIXME : 스킬을 사용했을때 슬롯 번호를 가지고 해당 슬롯을 찾아서 업데이트 한다.
    ##         매우 불합리. 구조 자체를 개선해야 할듯.
    def OnUseSkill(self, slotIndex, coolTime):

        skillIndex = playerInst().GetSkillIndex(slotIndex)
        skillType = skill.GetSkillType(skillIndex)

        ## ACTIVE
        if skill.SKILL_TYPE_ACTIVE == skillType:
            skillGrade = playerInst().GetSkillGrade(slotIndex)
            slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)
        ## ETC
        else:
            slotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)

        for slotWindow in self.skillPageDict.values():
            if slotWindow.HasSlot(slotIndex):
                slotWindow.StoreSlotCoolTime(skillType, slotIndex, coolTime)
                self.__RestoreSlotCoolTime(slotWindow)
                # slotWindow.SetSlotCoolTime(slotIndex, coolTime)
                return

    def OnActivateSkill(self, slotIndex):

        skillGrade = playerInst().GetSkillGrade(slotIndex)
        slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)

        for slotWindow in self.skillPageDict.values():
            if slotWindow.HasSlot(slotIndex):
                slotWindow.ActivateSlot(slotIndex)
                return

    def OnDeactivateSkill(self, slotIndex):

        skillGrade = playerInst().GetSkillGrade(slotIndex)
        slotIndex = self.__GetRealSkillSlot(skillGrade, slotIndex)

        for slotWindow in self.skillPageDict.values():
            if slotWindow.HasSlot(slotIndex):
                slotWindow.DeactivateSlot(slotIndex)
                return

    def __ShowJobToolTip(self):
        self.toolTipJob.ShowToolTip()

    def __HideJobToolTip(self):
        self.toolTipJob.HideToolTip()

    def __SetJobText(self, mainJob, subJob):
        if playerInst().GetPoint(player.LEVEL) < 5:
            subJob = 0

    def __ShowAlignmentToolTip(self):
        self.toolTipAlignment.ShowToolTip()
        return True

    def __HideAlignmentToolTip(self):
        self.toolTipAlignment.HideToolTip()
        return True

    def RefreshCharacter(self):
        if not self.isLoaded:
            return

        ## Name
        characterName = player.GetName()
        guildName = player.GetGuildName()
        self.characterNameValue.SetText(characterName)
        self.guildNameValue.SetText(guildName)

        race = appInst.instance().GetNet().GetMainActorRace()
        group = appInst.instance().GetNet().GetMainActorSkillGroup()
        empire = appInst.instance().GetNet().GetMainActorEmpire()

        ## Job Text
        job = chr.RaceToJob(race)
        self.__SetJobText(job, group)

        ## FaceImage
        try:
            faceImageName = FACE_IMAGE_DICT[race]

            try:
                self.faceImage.LoadImage(faceImageName)
            except:
                print(
                    "CharacterWindow.RefreshCharacter(race={}, faceImageName={})".format(
                        race, faceImageName
                    )
                )
                self.faceImage.Hide()

        except KeyError:
            self.faceImage.Hide()

        ## GroupName
        self.__SetSkillGroupName(race, group)

        ## Skill
        if 0 == group:
            self.__SelectSkillGroup(0)

        else:
            self.__SetSkillSlotData(race, group, empire)

            if self.__CanUseHorseSkill():
                self.__SelectSkillGroup(0)

        if self.chDetailsWnd:
            self.chDetailsWnd.RefreshLabel()

    def Reconstruct(self):
        for slotWindow in self.skillPageDict.values():
            count = slotWindow.GetSlotCount()
            for i in xrange(count):
                slotWindow.DeactivateSlot(i)

    def __SetSkillGroupName(self, race, group):

        job = chr.RaceToJob(race)

        if job not in self.SKILL_GROUP_NAME_DICT:
            return

        nameList = self.SKILL_GROUP_NAME_DICT[job]

        if 0 == group:
            self.skillGroupButton1.SetText(nameList[1])
            self.skillGroupButton2.SetText(nameList[2])
            self.skillGroupButton1.Show()
            # self.skillGroupButton2.Show()
            self.activeSkillGroupName.Hide()

        else:

            # if self.__CanUseHorseSkill():
            #     self.activeSkillGroupName.Hide()
            #     self.skillGroupButton1.SetText(nameList.get(group, "Noname"))
            #     self.skillGroupButton2.SetText(localeInfo.SKILL_GROUP_HORSE)
            #     self.skillGroupButton1.Show()
            #     self.skillGroupButton2.Show()

            # else:
            self.activeSkillGroupName.SetText(nameList.get(group, "Noname"))
            self.activeSkillGroupName.Show()
            self.skillGroupButton1.Hide()
            self.skillGroupButton2.Hide()

    def __SetSkillSlotData(self, race, group, empire=0):

        ## SkillIndex
        RegisterSkill(race, group, empire)

        ## Event
        self.__SetSkillSlotEvent()

        ## Refresh
        self.RefreshSkill()

    def __SelectSkillGroup(self, index):
        group = appInst.instance().GetNet().GetMainActorSkillGroup()
        if self.__CanUseHorseSkill() and group != 0:
            if 0 == index:
                index = appInst.instance().GetNet().GetMainActorSkillGroup() - 1
            elif 1 == index:
                index = self.PAGE_HORSE

        self.curSelectedSkillGroup = index
        self.__SetSkillSlotData(
            appInst.instance().GetNet().GetMainActorRace(),
            index + 1,
            appInst.instance().GetNet().GetMainActorEmpire(),
        )

    def __CanUseSkillNow(self):
        if 0 == appInst.instance().GetNet().GetMainActorSkillGroup():
            return False

        return True

    def __CanUseHorseSkill(self):

        slotIndex = playerInst().GetSkillSlotIndexNew(player.SKILL_INDEX_RIDING)

        if not slotIndex:
            return False

        grade = playerInst().GetSkillGrade(slotIndex)
        level = playerInst().GetSkillDisplayLevel(slotIndex)
        if level < 0:
            level *= -1
        if grade >= 1 and level >= 1:
            return True

        return False

    def __IsChangedHorseRidingSkillLevel(self):
        ret = False

        if -1 == self.canUseHorseSkill:
            self.canUseHorseSkill = self.__CanUseHorseSkill()

        if self.canUseHorseSkill != self.__CanUseHorseSkill():
            ret = True

        self.canUseHorseSkill = self.__CanUseHorseSkill()
        return ret

    def __GetRealSkillSlot(self, skillGrade, skillSlot):
        return (
            skillSlot
            + min(skill.SKILL_GRADE_COUNT - 2, skillGrade)
            * skill.SKILL_GRADE_STEP_COUNT
        )

    def __GetETCSkillRealSlotIndex(self, skillSlot):
        if skillSlot > 100:
            return skillSlot
        return skillSlot % self.ACTIVE_PAGE_SLOT_COUNT

    def __RealSkillSlotToSourceSlot(self, realSkillSlot):
        if realSkillSlot > 100:
            return realSkillSlot
        if self.PAGE_HORSE == self.curSelectedSkillGroup:
            return realSkillSlot + self.ACTIVE_PAGE_SLOT_COUNT
        return realSkillSlot % skill.SKILL_GRADE_STEP_COUNT

    def __GetSkillGradeFromSlot(self, skillSlot):
        return int(skillSlot / skill.SKILL_GRADE_STEP_COUNT)

    def SelectSkillGroup(self, index):
        self.__SelectSkillGroup(index)

    ###################################
    ############# QUESTS ##############
    ###################################

    def __OnScrollQuest(self):
        if self.state != "QUEST":
            return

        curPos = self.questScrollBar.GetPos()
        if math.fabs(curPos - self.lastScrollPosition) >= 0.001:
            self.RerenderArea()
            self.lastScrollPosition = curPos

    def ResetQuestScroll(self):
        self.questScrollBar.Hide()

        if self.questScrollBar.GetPos() != 0:
            self.questScrollBar.SetPos(0)

    def RerenderArea(self):
        overflowingY = self.displayY - self.MAX_QUEST_HEIGHT
        if overflowingY < 0:
            overflowingY = 0

        self.baseCutY = math.ceil(overflowingY * self.questScrollBar.GetPos() / 20) * 20

        self.displayY = 0
        self.RearrangeCategories(xrange(quest.QUEST_CATEGORY_MAX_NUM))
        self.RefreshCategory()

        if overflowingY > 0:
            self.questScrollBar.Show()
        else:
            self.ResetQuestScroll()

    def __LoadQuestCat(self):
        self.questPage.Show()

        if self.isLoaded == 0:
            return

        for i in xrange(quest.QUEST_CATEGORY_MAX_NUM):
            cat = self.questCategoryList[i]

            catName = cat.GetProperty("name")
            if not catName:
                cat.SetProperty("name", cat.GetText())
                catName = cat.GetText()

            count = self.GetCountQuestInCategory(i)

            self.questCategoryList[i].SetTextAlignLeft(
                catName + " (" + str(count) + ")"
            )
            self.questCategoryList[i].SetTextColor(self.GetCategoryColor(i))
            self.questCategoryList[i].Show()

        self.RefreshCategory()
        if self.finishFirstLoad == False:
            self.questScrollBar.Hide()
        else:
            self.RerenderArea()
        self.finishFirstLoad = True

    def OnRunMouseWheel(self, nLen):
        if not self.IsInPosition():
            return False
        if nLen > 0:
            self.questScrollBar.OnUp()
        else:
            self.questScrollBar.OnDown()

        return True

    def GetCategoryColor(self, cat):
        return self.qColorList["default_title"]

    def GetQuestProperties(self, questName):
        # If it finds one of the tokens in the list it colors the whole quest that way
        find = {"&": "green", "*": "blue", "~": "golden"}

        if questName[0] in find:
            return questName[1:], find[questName[0]]

        return questName, None

    def IsCategoryOpen(self, cat):
        return cat in self.openCategories

    def ToggleCategory(self, cat):
        if self.IsCategoryOpen(cat):
            self.CloseCategory(cat)
        else:
            self.OpenCategory(cat)

    def RearrangeCategories(self, categoryRange):
        i = 0
        for i in categoryRange:
            if (self.displayY - self.baseCutY) >= 0 and (
                self.displayY - self.baseCutY
            ) < self.MAX_QUEST_HEIGHT - 20:
                self.questCategoryList[i].SetPosition(
                    13, (self.displayY - self.baseCutY) + 10
                )
                self.questCategoryList[i].Show()
            else:
                self.questCategoryList[i].Hide()

            self.displayY += 20
            self.catPositionRender[i] = self.displayY

    def CloseCategory(self, cat):
        if cat in self.openCategories:
            self.openCategories.remove(cat)

        for curSlot in self.questSlotList:
            if curSlot.GetProperty("category") == cat:
                curSlot.Hide()
                self.displayY -= curSlot.GetHeight()

        self.questCategoryList[cat].CloseImage()
        self.RerenderArea()

    def OpenCategory(self, cat):
        while (
            len(self.openCategories) >= self.maxConcurrentCategories
        ):  # close opened ones if we go over the limit
            ccat = self.openCategories.pop()
            self.CloseCategory(ccat)

        self.openCategories.append(cat)
        self.RefreshCategory(cat)
        self.questCategoryList[cat].OpenImage()
        self.RerenderArea()

    def __SelectQuest(self, slotIndex):
        questIndex = self.questIndexMap[slotIndex]

        if not questIndex in self.clickedQuests:
            self.clickedQuests.append(questIndex)

        import event

        event.QuestButtonClick(-2147483648 + questIndex)

    def RefreshCategory(self, cat=-1):
        if self.isLoaded == 0 or self.state != "QUEST":
            return

        cats = []
        if cat == -1:
            cats = self.openCategories
        elif not cat in self.openCategories:
            self.OpenCategory(cat)
            return
        else:
            cats.append(cat)

        for curCat in cats:
            self.displayY = self.catPositionRender[curCat]
            # Load
            self.LoadCategory(curCat)
            # Rearrange
            self.RearrangeCategories(xrange(curCat + 1, quest.QUEST_CATEGORY_MAX_NUM))

    def ReceiveNewQuest(self, idx):
        if not self.finishFirstLoad:
            return

        for cat in xrange(quest.QUEST_CATEGORY_MAX_NUM):
            for q in self.GetQuestsInCategory(cat):
                (
                    questID,
                    questIndex,
                    questName,
                    questCategory,
                    questIcon,
                    questCounterName,
                    questCounterValue,
                ) = q
                if questIndex == idx:
                    self.RefreshCategory(cat)
                    self.RefreshCategoriesCount()
                    self.RerenderArea()
                    return

    def RefreshCategoriesCount(self):
        for cat in xrange(quest.QUEST_CATEGORY_MAX_NUM):
            # Refresh category count
            catName = self.questCategoryList[cat].GetProperty("name")
            count = self.GetCountQuestInCategory(cat)
            self.questCategoryList[cat].SetTextAlignLeft(
                catName + " (" + str(count) + ")"
            )

    def RefreshQuest(self):
        if self.isLoaded == 0 or self.state != "QUEST":
            return

        for cat in self.openCategories:
            self.RefreshCategory(cat)

        self.RefreshCategoriesCount()

    def CreateQuestSlot(self, name):
        for cSlot in self.questSlotList:
            if cSlot.GetWindowName() == name:
                return cSlot

        pyScrLoader = ui.PythonScriptLoader()
        slot = ui.ListBar()
        pyScrLoader.LoadElementListBar(slot, quest_slot_listbar, self.questPage)

        slot.SetParent(self.questPage)
        slot.SetWindowName(name)
        slot.Hide()

        # Store it
        self.questSlotList.append(slot)

        return slot

    def SetQuest(self, slot, questID, questName, questCounterName, questCounterValue):
        (name, color) = self.GetQuestProperties(questName)
        slot.SetTextAlignLeft(name)
        slot.SetTextPosition(20, 0)
        if color:
            slot.SetTextColor(self.qColorList[color])
        slot.SetEvent(Event(self.__SelectQuest, questID))
        slot.SetHorizontalAlignLeft()
        slot.Show()

    def LoadCategory(self, cat):
        # Reinitialize everything
        self.slotIndex = 0
        self.questIndexMap = {}
        self.extraSpace = 0
        self.counterList = []
        self.clockList = []

        # Hidea all the slots first
        for slot in self.questSlotList:
            slot.Hide()

        qPos = 0
        for q in self.GetQuestsInCategory(cat):
            (
                questID,
                questIndex,
                questName,
                questCategory,
                questIcon,
                questCounterName,
                questCounterValue,
            ) = q
            (questClockName, questClockTime) = quest.GetQuestLastTime(questID)

            # Generate slot
            slot = self.CreateQuestSlot(
                "QuestSlotList_" + str(questCategory) + "_" + str(questID)
            )

            # Position it
            slot.SetPosition(10, self.displayY - self.baseCutY)
            baseDisplayY = self.displayY

            # Set counters if any
            hasCounter = False
            if questCounterName != "":
                self.displayY += 15

                counter = ui.TextLine()
                counter.SetParent(slot)
                counter.SetPosition(35, 20)
                counter.SetText(questCounterName + ": " + str(questCounterValue))
                counter.Show()

                self.counterList.append(counter)
                hasCounter = True

            # Show clocks if any
            if len(questClockName) > 1:
                if questClockTime <= 0:
                    clockText = localeInfo.QUEST_TIMEOVER
                else:
                    questLastMinute = questClockTime / 60
                    questLastSecond = questClockTime % 60

                    clockText = questClockName + ": "

                    if questLastMinute > 0:
                        clockText += str(questLastMinute) + localeInfo.QUEST_MIN
                        if questLastSecond > 0:
                            clockText += " "

                    if questLastSecond > 0:
                        clockText += str(questLastSecond) + localeInfo.QUEST_SEC

                clock = ui.TextLine()
                clock.SetParent(slot)
                clock.SetPosition(35, 20 + int(hasCounter) * 14)
                clock.SetText(clockText)
                clock.SetProperty("idx", questID)
                clock.Show()

                self.clockList.append(clock)

                self.displayY += 25

            slot.SetProperty("category", questCategory)

            if questIndex in self.clickedQuests:
                slot.OnClickEvent()  # mark it

            # Display and save it if necessary
            if (
                baseDisplayY - self.baseCutY >= 0
                and baseDisplayY - self.baseCutY < self.MAX_QUEST_HEIGHT
            ):
                # print "Saved "+questName+" at "+str(self.slotIndex)+", id "+str(self.questIndexMap[questID])
                self.questIndexMap[questID] = questIndex
                self.SetQuest(
                    slot, questID, questName, questCounterName, questCounterValue
                )

            self.displayY += 25

        # Clean non-displayed slots!
        oldList = self.questSlotList
        newList = []
        for slot in self.questSlotList:
            if slot.IsShow():
                newList.append(slot)

        self.questSlotList = newList

    def __UpdateQuestClock(self):
        if "QUEST" == self.state:
            for clock in self.clockList:
                if not clock.GetProperty("idx"):
                    return

                (questClockName, questClockTime) = quest.GetQuestLastTime(
                    clock.GetProperty("idx")
                )

                if questClockTime <= 0:
                    clockText = localeInfo.QUEST_TIMEOVER
                else:
                    questLastMinute = questClockTime / 60
                    questLastSecond = questClockTime % 60

                    clockText = questClockName + ": "

                    if questLastMinute > 0:
                        clockText += str(questLastMinute) + localeInfo.QUEST_MIN
                        if questLastSecond > 0:
                            clockText += " "

                    if questLastSecond > 0:
                        clockText += str(questLastSecond) + localeInfo.QUEST_SEC

                clock.SetText(clockText)

    def __OnClickQuestCategoryButton(self, category):
        self.ToggleCategory(category)

    def GetQuestsInCategory(self, category, retCount=False):
        qlist = []
        count = 0
        for i in xrange(quest.GetQuestCount()):
            (
                questIndex,
                questName,
                questCategory,
                questIcon,
                questCounterName,
                questCounterValue,
            ) = quest.GetQuestData(i)
            if questCategory == category:
                count += 1
                qlist.append(
                    (
                        i,
                        questIndex,
                        questName,
                        questCategory,
                        questIcon,
                        questCounterName,
                        questCounterValue,
                    )
                )

        if retCount:
            return count

        return qlist

    def GetCountQuestInCategory(self, category):
        return self.GetQuestsInCategory(category, True)

    def OpenQuestSlideWindow(self, skin, idx):
        return

        wnds = ()

        if self.IsShow() == False:
            return
        q = uiQuest.QuestSlideDialog(self, skin, idx)
        q.SetWindowName("QuestSlideWindow" + str(idx))
        self.SetTop()
        q.Show()

        # 이미 퀘스트가 열려있는데 또 누르면 기존꺼 삭제하고 새로운 퀘스트 기열
        if (
            self.wndQuestSlideWindowNewKey > 0
            and self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1] is not None
        ):
            self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1].CloseSelf()

        self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey] = q

        self.wndQuestSlideWindowNewKey = self.wndQuestSlideWindowNewKey + 1
        if self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1] is not None:
            self.wndQuestSlideWindow[
                self.wndQuestSlideWindowNewKey - 1
            ].AddOnCloseEvent(
                Event(self.RemoveQuestSlideDialog, self.wndQuestSlideWindowNewKey - 1)
            )

    def OnMoveWindow(self, x, y):
        if self.wndQuestSlideWindowNewKey - 1 >= 0:
            if self.wndQuestSlideWindow[self.wndQuestSlideWindowNewKey - 1] is not None:
                self.wndQuestSlideWindow[
                    self.wndQuestSlideWindowNewKey - 1
                ].AdjustPositionAndSize()

        if self.chDetailsWnd:
            self.chDetailsWnd.AdjustPosition(x + 3, y)

    def RemoveQuestSlideDialog(self, key):
        if self.wndQuestSlideWindow[key]:
            self.wndQuestSlideWindow[key] = None
