import logging

import ui
import player
import localeInfo
import app
import grp
import dungeonInfo
import pack
import background
from pygame.app import appInst
from ui_event import MakeEvent, Event
from pack import GetVfsFile
from pygame.player import playerInst

ROOT_PATH = "d:/ymir work/ui/game/dungeon_info/"

DUNGEON_RANKING_MAX_NUM = 10

DUNGEON_MAX_VIEW_RANGE = 10
DUNGEON_BUTTON_HEIGHT = 34
DUNGEON_BUTTON_GAP = 2

DUNGEON_TYPE_TEXT = {
    dungeonInfo.DUNGEON_TYPE_PRIVATE: localeInfo.DUNGEON_TYPE_PRIVATE,
    dungeonInfo.DUNGEON_TYPE_PUBLIC: localeInfo.DUNGEON_TYPE_PUBLIC,
}

DUNGEON_ORG_TEXT = {
    dungeonInfo.DUNGEON_ORG_SINGLE: localeInfo.DUNGEON_ORG_SINGLE,
    dungeonInfo.DUNGEON_ORG_PARTY: localeInfo.DUNGEON_ORG_PARTY,
    dungeonInfo.DUNGEON_ORG_HYBRID: localeInfo.DUNGEON_ORG_HYBRID,
}

RACE_FLAG_TEXT = {
    "ANIMAL": localeInfo.DUNGEON_RF_ANIMAL,
    "UNDEAD": localeInfo.DUNGEON_RF_UNDEAD,
    "DEVIL": localeInfo.DUNGEON_RF_DEVIL,
    "HUMAN": localeInfo.DUNGEON_RF_HUMAN,
    "ORC": localeInfo.DUNGEON_RF_ORC,
    "MILGYO": localeInfo.DUNGEON_RF_MILGYO,
    "INSECT": localeInfo.DUNGEON_RF_INSECT,
    "ELEC": localeInfo.DUNGEON_RF_ELEC,
    "FIRE": localeInfo.DUNGEON_RF_FIRE,
    "ICE": localeInfo.DUNGEON_RF_ICE,
    "WIND": localeInfo.DUNGEON_RF_WIND,
    "EARTH": localeInfo.DUNGEON_RF_EARTH,
    "DARK": localeInfo.DUNGEON_RF_DARK,
    "SHADOW": localeInfo.DUNGEON_RF_SHADOW,
    "CZ": localeInfo.DUNGEON_RF_CZ,
    "ATT_ELEC": localeInfo.DUNGEON_RF_ELEC,
    "ATT_FIRE": localeInfo.DUNGEON_RF_FIRE,
    "ATT_ICE": localeInfo.DUNGEON_RF_ICE,
    "ATT_WIND": localeInfo.DUNGEON_RF_WIND,
    "ATT_EARTH": localeInfo.DUNGEON_RF_EARTH,
    "ATT_DARK": localeInfo.DUNGEON_RF_DARK,
}


class DungeonButton(ui.RadioButton):
    def __init__(self, dungeonID):
        ui.RadioButton.__init__(self)

        self.cooldownEndEvent = None

        self.dungeonID = dungeonID
        self.doubleTextMode = False
        self.cooldownEnd = 0

        icon = ui.ImageBox()
        icon.SetParent(self)
        icon.SetPosition(2, 2)

        iconName = "%sicons/%02d.tga" % (ROOT_PATH, dungeonID)
        if GetVfsFile(iconName) is not None:
            icon.LoadImage(iconName)
        else:
            icon.LoadImage("%sicons/00.tga" % ROOT_PATH)

        icon.Show()
        self.dungIcon = icon

        nameText = ui.TextLine()
        nameText.SetParent(self)
        nameText.SetText(localeInfo.Get(dungeonInfo.GetDungeonName(dungeonID)))
        nameText.SetPosition(40, 11)
        nameText.Show()
        self.dungName = nameText

        availTextTop = ui.TextLine()
        availTextTop.SetParent(self)
        availTextTop.SetText("Cooldown:")
        availTextTop.SetFontColor(0.76, 0.73, 0.23)
        availTextTop.SetHorizontalAlignCenter()
        availTextTop.SetPosition(100, 5)
        availTextTop.Hide()
        self.availTextTop = availTextTop

        availTextBottom = ui.TextLine()
        availTextBottom.SetParent(self)
        availTextBottom.SetText("00:52:40")
        availTextBottom.SetFontColor(0.76, 0.73, 0.23)
        availTextBottom.SetHorizontalAlignCenter()
        availTextBottom.SetPosition(100, 16)
        availTextBottom.Hide()
        self.availTextBottom = availTextBottom

        middleText = ui.TextLine()
        middleText.SetParent(self)
        middleText.SetText("Level too low!")
        middleText.SetHorizontalAlignCenter()
        middleText.SetPosition(100, 11)
        middleText.Hide()
        self.middleText = middleText

        self.RefreshButtonInfo()

    def RefreshButtonInfo(self):
        (minLevel, maxLevel) = dungeonInfo.GetDungeonLevel(self.dungeonID)
        cooldownEnd = dungeonInfo.GetDungeonCooldownEnd(self.dungeonID)
        leftSec = max(0, cooldownEnd - app.GetGlobalTimeStamp())

        if playerInst().GetPoint(player.LEVEL) < minLevel:
            self.middleText.SetFontColor(0.89, 0.23, 0.23)
            self.middleText.SetText("Level too low!")
            self.SetDoubleTextMode(False)
        elif playerInst().GetPoint(player.LEVEL) > maxLevel:
            self.middleText.SetFontColor(0.89, 0.23, 0.23)
            self.middleText.SetText("Level too high!")
            self.SetDoubleTextMode(False)
        elif leftSec > 0:
            self.SetDoubleTextMode(True)
            self.availTextBottom.SetText(localeInfo.SecondToNiceTime(leftSec))
        else:
            self.middleText.SetFontColor(0.45, 0.76, 0.23)
            self.middleText.SetText("Available")
            self.SetDoubleTextMode(False)

    def SetDoubleTextMode(self, bMode):
        self.doubleTextMode = bMode
        if bMode:
            self.availTextTop.Show()
            self.availTextBottom.Show()
            self.middleText.Hide()
        else:
            self.availTextTop.Hide()
            self.availTextBottom.Hide()
            self.middleText.Show()

    def SetCooldownEnd(self, endTime):
        leftSec = max(0, endTime - app.GetGlobalTimeStamp())
        if leftSec > 0:
            self.cooldownEnd = endTime
            self.SetDoubleTextMode(True)
        else:
            self.RefreshButtonInfo()

    def OnRender(self):
        if self.cooldownEnd and self.doubleTextMode:
            leftSec = max(0, self.cooldownEnd - app.GetGlobalTimeStamp())
            if leftSec > 0:
                self.availTextBottom.SetText(localeInfo.SecondToNiceTime(leftSec))
            else:
                self.RefreshButtonInfo()

                if self.cooldownEndEvent:
                    self.cooldownEndEvent()

    def SetCooldownEndEvent(self, event):
        self.cooldownEndEvent = MakeEvent(event)

    def GetDungeonID(self):
        return self.dungeonID


class DungeonInfoWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.SetWindowName("DungeonInfoWindow")

        self.Initialize()

    def Initialize(self):
        # Constants
        self.selectedDungeonID = 0

        # Objects
        self.board = None
        self.buttonsClipper = None

        # Extern objects
        self.tooltipItem = None
        self.wndInterface = None

        # Info Objects
        self.dungeonBackground = None

        self.dungeonNameText = None
        self.dungeonTypeText = None
        self.dungeonOrgText = None
        self.dungeonLevelText = None
        self.dungeonPartyText = None
        self.dungeonCooldownText = None
        self.dungeonAvailTimeText = None
        self.dungeonEntryText = None
        self.dungeonStRaceFlagText = None
        self.dungeonResRaceFlagText = None

        self.dungeonPersCountText = None
        self.dungeonPersFastText = None
        self.dungeonPersDmgText = None

        self.topCountBtn = None
        self.topFastestBtn = None
        self.topDamageBtn = None
        self.teleportBtn = None
        self.reqSlot = None

        # Containers
        self.buttonList = []

    def SetItemToolTip(self, itemTooltip):
        self.tooltipItem = itemTooltip

    def BindInterface(self, interface):
        self.wndInterface = interface

    def LoadWindow(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/DungeonInfoWindow.py")
        except:
            import logging

            logging.exception("DungeonInfoWindow.LoadWindow.LoadScript")

        try:
            self.board = self.GetChild("board")
            self.buttonsClipper = self.GetChild("ButtonsScissorsClip")

            # Info Objects
            self.dungeonBackground = self.GetChild("Dungeon_BG")

            self.dungeonNameText = self.GetChild("Text_DungeonName")
            self.dungeonTypeText = self.GetChild("Text_Type")
            self.dungeonOrgText = self.GetChild("Text_Organization")
            self.dungeonLevelText = self.GetChild("Text_Level_Limits")
            self.dungeonPartyText = self.GetChild("Text_Party_Limits")
            self.dungeonCooldownText = self.GetChild("Text_Cooldown")
            self.dungeonAvailTimeText = self.GetChild("Text_Avail_Time")
            self.dungeonEntryText = self.GetChild("Text_Entrance")
            self.dungeonStRaceFlagText = self.GetChild("Text_Strong_Against")
            self.dungeonResRaceFlagText = self.GetChild("Text_Defence")

            self.dungeonPersCountText = self.GetChild("Text_Personal_Count")
            self.dungeonPersFastText = self.GetChild("Text_Personal_Fastest")
            self.dungeonPersDmgText = self.GetChild("Text_Personal_Dmg")

            self.topCountBtn = self.GetChild("Top_Count_Button")
            self.topFastestBtn = self.GetChild("Top_Fastest_Button")
            self.topDamageBtn = self.GetChild("Top_Damage_Button")
            self.teleportBtn = self.GetChild("Button_Teleport")
            self.reqSlot = self.GetChild("Slot_ReqItem")
        except:
            import logging

            logging.exception("DungeonInfoWindow.LoadWindow.BindObject")

        try:
            self.board.SetCloseEvent(self.Close)

            self.reqSlot.SetOverInItemEvent(self.OnOverInItem)
            self.reqSlot.SetOverOutItemEvent(self.OnOverOutItem)

            self.teleportBtn.SetEvent(self.OnTeleportClick)

            self.topCountBtn.SetEvent(
                Event(self.OnRankingClick, dungeonInfo.DUNGEON_RANK_TYPE_COUNT)
            )
            self.topFastestBtn.SetEvent(
                Event(self.OnRankingClick, dungeonInfo.DUNGEON_RANK_TYPE_TIME)
            )
            self.topDamageBtn.SetEvent(
                Event(self.OnRankingClick, dungeonInfo.DUNGEON_RANK_TYPE_DMG)
            )
        except:
            import logging

            logging.exception("DungeonInfoWindow.LoadWindow.BindEvents")

        self.CreateButtons()
        self.OnClickDungeon(1)

    def CreateButtons(self):
        k = 0

        dungeonList = [
            dungeonInfo.DUNGEON_ID_OWL,
            dungeonInfo.DUNGEON_ID_GARDEN,
            dungeonInfo.DUNGEON_ID_DEEP_CAVE,
            dungeonInfo.DUNGEON_ID_NEVERLAND,
            dungeonInfo.DUNGEON_ID_WOOD,
            dungeonInfo.DUNGEON_ID_ICESTORM,
            dungeonInfo.DUNGEON_ID_SHADOW_TOWER,
            dungeonInfo.DUNDEON_ID_PYRA,
            dungeonInfo.DUNGEON_ID_DEMON,
            dungeonInfo.DUNGEON_ID_RATHALOS_1,
        ]

        # TOday i do this feature, you can sort all dungeons in custom order, just active 'for i in dungeonList';
        # then do the changes from: dungeonList!
        for i in xrange(
            1, dungeonInfo.DUNGEON_ID_MAX
        ):  # in order from locale/common/global list.
            # for i in dungeonList: # in order from dungeonList; to remeber, all indexes is loaded from src/bin
            if not dungeonInfo.IsValidDungeon(i):
                continue

            btn = DungeonButton(i)
            btn.SetParent(self.buttonsClipper)
            btn.SetPosition(0, (DUNGEON_BUTTON_HEIGHT + DUNGEON_BUTTON_GAP) * k)
            # btn.SetBasePosition(0, (DUNGEON_BUTTON_HEIGHT + DUNGEON_BUTTON_GAP) * k)
            btn.SetEvent(Event(self.OnClickDungeon, i))
            btn.SetCooldownEndEvent(Event(self.OnEndCooldownDungeon, i))
            btn.SetUpVisual(ROOT_PATH + "dungeon_button.tga")
            btn.SetOverVisual(ROOT_PATH + "dungeon_button.tga")
            btn.SetDownVisual(ROOT_PATH + "dungeon_button_selected.tga")
            btn.Show()

            k += 1
            self.buttonList.append(btn)
            self.buttonsClipper.SetSize(
                self.buttonsClipper.GetWidth(),
                (DUNGEON_BUTTON_HEIGHT + DUNGEON_BUTTON_GAP) * k,
            )

    def OnTeleportClick(self):
        if self.selectedDungeonID:
            appInst.instance().GetNet().SendRequestDungeonWarp(self.selectedDungeonID)

    def OnRankingClick(self, rankType):
        if self.wndInterface:
            self.wndInterface.ToggleDungeonRankingWindow(
                self.selectedDungeonID, rankType
            )

    def OnEndCooldownDungeon(self, dungeonID):
        # There, on request on one of my customers, when the cooldown is finished, you can do something for inform the players.
        # Code below, for moment let's pass it!
        pass

    def OnClickDungeon(self, dungeonID):
        self.selectedDungeonID = dungeonID
        for btn in self.buttonList:
            if btn.GetDungeonID() == dungeonID:
                btn.Down()
            else:
                btn.SetUp()

        bgName = "%sbg/%02d.tga" % (ROOT_PATH, dungeonID)
        if GetVfsFile(bgName) is not None:
            self.dungeonBackground.LoadImage(bgName)
        else:
            self.dungeonBackground.LoadImage("%sbg/00.tga" % ROOT_PATH)

        self.dungeonNameText.SetText(
            localeInfo.Get(dungeonInfo.GetDungeonName(dungeonID))
        )
        self.dungeonTypeText.SetText(
            localeInfo.DUNGEON_TYPE_TEXT.format(
                DUNGEON_TYPE_TEXT.get(
                    dungeonInfo.GetDungeonType(dungeonID, localeInfo.DUNGEON_NO_NAME)
                )
            )
        )
        self.dungeonOrgText.SetText(
            localeInfo.DUNGEON_ORG_TEXT.format(
                DUNGEON_ORG_TEXT.get(
                    dungeonInfo.GetDungeonOrganization(
                        dungeonID, localeInfo.DUNGEON_NO_NAME
                    )
                )
            )
        )

        (minLevel, maxLevel) = dungeonInfo.GetDungeonLevel(dungeonID)
        self.dungeonLevelText.SetText(
            localeInfo.DUNGEON_LEVEL_LIMIT_TEXT.format(minLevel, maxLevel)
        )

        (minParty, maxParty) = dungeonInfo.GetDungeonPartyMembers(dungeonID)
        if minParty == 0 and maxParty == 0:
            self.dungeonPartyText.SetText(localeInfo.DUNGEON_PARTY_LIMIT_TEXT + " -")
        else:
            self.dungeonPartyText.SetText(
                localeInfo.DUNGEON_PARTY_LIMIT_TEXT + " %d - %d" % (minParty, maxParty)
            )

        self.dungeonCooldownText.SetText(
            localeInfo.DUNGEON_COOLDOWN_TEXT.format(
                localeInfo.MinuteToHM(dungeonInfo.GetDungeonCooldown(dungeonID))
            )
        )
        self.dungeonAvailTimeText.SetText(
            localeInfo.DUNGEON_AVAILTIME_TEXT.format(
                localeInfo.MinuteToHM(dungeonInfo.GetDungeonRuntime(dungeonID))
            )
        )

        (enterMapIndex, xPos, yPos) = dungeonInfo.GetDungeonEnterCoord(dungeonID)
        self.dungeonEntryText.SetText(
            localeInfo.DUNGEON_ENTRANCE_TEXT.format(
                localeInfo.GetMapNameByIndex(enterMapIndex)
            )
        )

        stRaceFlag = dungeonInfo.GetDungeonStRaceFlag(dungeonID)
        if stRaceFlag == "NO_ST_RACE_FLAG":
            self.dungeonStRaceFlagText.SetText(
                localeInfo.DUNGEON_ST_RACEFLAG_TEXT.format(localeInfo.DUNGEON_RF_EMPTY)
            )
        else:
            raceFlagList = stRaceFlag.split(",")
            realRaceFlag = ""
            for flag in raceFlagList:
                realRaceFlag += (
                    RACE_FLAG_TEXT.get(flag, localeInfo.DUNGEON_NO_NAME) + ", "
                )

            self.dungeonStRaceFlagText.SetText(
                localeInfo.DUNGEON_ST_RACEFLAG_TEXT.format(realRaceFlag[:-2])
            )

        stResRaceFlag = dungeonInfo.GetDungeonResRaceFlag(dungeonID)
        if stResRaceFlag == "NO_RES_RACE_FLAG":
            self.dungeonResRaceFlagText.SetText(
                localeInfo.DUNGEON_RES_RACEFLAG_TEXT.format(localeInfo.DUNGEON_RF_EMPTY)
            )
        else:
            raceFlagList = stResRaceFlag.split(",")
            realRaceFlag = ""
            for flag in raceFlagList:
                realRaceFlag += (
                    RACE_FLAG_TEXT.get(flag, localeInfo.DUNGEON_NO_NAME) + ", "
                )

            self.dungeonResRaceFlagText.SetText(
                localeInfo.DUNGEON_RES_RACEFLAG_TEXT.format(realRaceFlag[:-2])
            )

        (ticketVnum, ticketCount) = dungeonInfo.GetDungeonTicket(dungeonID)
        if ticketVnum:
            self.reqSlot.SetItemSlot(
                0, ticketVnum, 1 if not ticketCount else ticketCount
            )
        else:
            self.reqSlot.ClearSlot(0)

        (completeCount, fastestTime, highestDmg) = dungeonInfo.GetDungeonPersonalStats(
            dungeonID
        )
        self.dungeonPersCountText.SetText(
            localeInfo.DUNGEON_PERS_COUNT_TEXT.format(completeCount)
        )
        self.dungeonPersFastText.SetText(
            localeInfo.DUNGEON_FASTEST_TIME_TEXT.format(
                localeInfo.SecondToHMS(fastestTime)
            )
        )
        self.dungeonPersDmgText.SetText(
            localeInfo.DUNGEON_HIGHEST_DMG_TEXT.format(
                localeInfo.DottedNumber(highestDmg)
            )
        )

    def RefreshButtonInfo(self):
        for btn in self.buttonList:
            dungeonID = btn.GetDungeonID()
            if not dungeonInfo.IsValidDungeon(dungeonID):
                continue

            cooldownEnd = dungeonInfo.GetDungeonCooldownEnd(dungeonID)
            btn.SetCooldownEnd(cooldownEnd)

        if self.selectedDungeonID:
            self.OnClickDungeon(self.selectedDungeonID)

    def OnOverInItem(self, slotIndex):
        if self.tooltipItem and self.selectedDungeonID:
            (ticketVnum, ticketCount) = dungeonInfo.GetDungeonTicket(
                self.selectedDungeonID
            )
            if ticketVnum:
                self.tooltipItem.SetItemToolTip(ticketVnum)

    def OnOverOutItem(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def Destroy(self):
        for btn in self.buttonList:
            del btn

        self.Initialize()
        self.ClearDictionary()

    def Open(self):
        self.RefreshButtonInfo()
        self.Show()
        self.SetTop()

    def Close(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.Hide()

    def OnPressEscapeKey(self):
        self.Close()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False


class DungeonRankingWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.SetWindowName("DungeonRankingWindow")
        self.Initialize()

    def Initialize(self):
        self.board = None
        self.otherTopText = None

        self.posTexts = []
        self.nameTexts = []
        self.otherTexts = []

    def SetItemToolTip(self, itemTooltip):
        self.tooltipItem = itemTooltip

    def LoadWindow(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/DungeonRankingWindow.py")
        except:
            import logging

            logging.exception("DungeonRankingWindow.LoadWindow.LoadScript")

        try:
            self.board = self.GetChild("Board")
            self.otherTopText = self.GetChild("OtherText")

            for i in xrange(DUNGEON_RANKING_MAX_NUM):
                self.posTexts.append(self.GetChild("Raking%d_Pos" % i))
                self.nameTexts.append(self.GetChild("Raking%d_Name" % i))
                self.otherTexts.append(self.GetChild("Raking%d_Other" % i))
        except:
            import logging

            logging.exception("DungeonRankingWindow.LoadWindow.BindObject")

        try:
            self.board.SetCloseEvent(self.Close)
        except:
            import logging

            logging.exception("DungeonRankingWindow.LoadWindow.BindEvents")

    def RefreshData(self, dungeonId, rankType):
        dungeonName = (
            "|cffc5b44a|H|h"
            + localeInfo.Get(dungeonInfo.GetDungeonName(dungeonId))
            + "|h|r"
        )
        dungeonTypeListT = [
            localeInfo.DUNGEON_TITLE_TYPE_COUNT,
            localeInfo.DUNGEON_TITLE_TYPE_TIME,
            localeInfo.DUNGEON_TITLE_TYPE_DMG,
        ]
        realDungeonType = "|cffc5b44a|H|h" + dungeonTypeListT[rankType] + "|h|r"
        self.board.SetTitleName(
            localeInfo.DUNGEON_RANKING_TITLE
            + " "
            + dungeonName
            + " "
            + localeInfo.DUNGEON_RANKING_TITLE_2
            + " "
            + realDungeonType
        )

        dungeonTypeList = [
            localeInfo.DUNGEON_TYPE_COUNT,
            localeInfo.DUNGEON_TYPE_TIME,
            localeInfo.DUNGEON_TYPE_DMG,
        ]
        self.otherTopText.SetText(dungeonTypeList[rankType])

        rankingData = dungeonInfo.GetRankingData(dungeonId, rankType)

        for i in xrange(DUNGEON_RANKING_MAX_NUM):
            self.posTexts[i].SetText("-")
            self.nameTexts[i].SetText("-")
            self.otherTexts[i].SetText("-")

            if i < len(rankingData):
                (pos, playerName, value) = rankingData[i]
                if value:
                    self.posTexts[i].SetText(str(pos))
                    self.nameTexts[i].SetText(str(playerName))

                    if rankType == dungeonInfo.DUNGEON_RANK_TYPE_COUNT:
                        self.otherTexts[i].SetText(str(value))
                    elif rankType == dungeonInfo.DUNGEON_RANK_TYPE_TIME:
                        self.otherTexts[i].SetText(localeInfo.SecondToHMS(value))
                    elif rankType == dungeonInfo.DUNGEON_RANK_TYPE_DMG:
                        self.otherTexts[i].SetText(localeInfo.DottedNumber(value))

    def Destroy(self):
        self.Initialize()
        self.ClearDictionary()

    def Open(self, dungeonId, rankType):
        self.RefreshData(dungeonId, rankType)
        self.Show()
        self.SetTop()

    def Close(self):
        self.Hide()

    def OnPressEscapeKey(self):
        self.Close()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False
