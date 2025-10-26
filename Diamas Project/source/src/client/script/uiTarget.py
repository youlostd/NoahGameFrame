# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import random
from collections import OrderedDict

import app
import chr
import guild
import item
import messenger
import nonplayer
import player
import wndMgr
from pygame.app import appInst
from pygame.player import playerInst

import colorInfo
import constInfo
import localeInfo
import ui
import uiCommon
import uiToolTip
from ui_event import MakeEvent, Event
from uiofflineshop import OfflineShopManager


def FormatRarity(pct):
    if 0 < pct <= 1:
        return colorInfo.Colorize("Mystic", 0xFF84187E)
    elif 1 < pct <= 3:
        return colorInfo.Colorize("Legendary", 0xFFFF5423)
    elif 3 < pct <= 10:
        return colorInfo.Colorize("Epic", 0xFF793F1E)
    elif 10 < pct <= 25:
        return colorInfo.Colorize("Rare", 0xFF0029F8)
    elif 25 < pct <= 50:
        return colorInfo.Colorize("Uncommon", 0xFF657786)
    elif 50 < pct <= 99:
        return colorInfo.Colorize("Common", 0xFFFFFFFF)
    elif pct >= 100:
        return colorInfo.Colorize("Guaranteed", 0xFF)


def numberFormat(n, rep="."):
    return "{:,}".format(n).replace(",", rep)


import playerSettingModule

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


class TargetBoard(ui.ThinBoardOld):
    EXCHANGE_LIMIT_RANGE = 300000000000000000000000

    if app.ENABLE_ELEMENT_ADD:
        ELEMENT_IMG_PATH = {
            nonplayer.RACE_FLAG_ATT_ELEC: "d:/ymir work/ui/game/12zi/element/elect.sub",
            nonplayer.RACE_FLAG_ATT_FIRE: "d:/ymir work/ui/game/12zi/element/fire.sub",
            nonplayer.RACE_FLAG_ATT_ICE: "d:/ymir work/ui/game/12zi/element/ice.sub",
            nonplayer.RACE_FLAG_ATT_WIND: "d:/ymir work/ui/game/12zi/element/wind.sub",
            nonplayer.RACE_FLAG_ATT_EARTH: "d:/ymir work/ui/game/12zi/element/earth.sub",
            nonplayer.RACE_FLAG_ATT_DARK: "d:/ymir work/ui/game/12zi/element/dark.sub",
        }

    def __init__(self):
        ui.ThinBoardOld.__init__(self)

        name = ui.TextLine()
        name.SetParent(self)
        name.SetFontName("Nunito Sans:14")
        name.SetPosition(75, 12)
        name.Show()

        self.row = ui.Window()
        self.row.SetParent(self)
        self.row.SetPosition(0, 13)
        self.row.SetHorizontalAlignLeft()
        self.row.Show()

        hpGauge = ui.Gauge()
        hpGauge.SetParent(self)
        hpGauge.MakeGauge(157, "red")
        hpGauge.SetPosition(55, 22)
        hpGauge.SetHorizontalAlignRight()
        hpGauge.Show()

        hpInfo = ui.TextLine()
        hpInfo.SetParent(hpGauge)
        hpInfo.SetPosition(0, -17)
        hpInfo.SetHorizontalAlignCenter()
        hpInfo.Hide()
        hpInfo.SetFontColor(1, 1, 1, 0.85)
        hpInfo.SetFontName("Nunito Sans:11b")

        closeButton = ui.Button()
        closeButton.SetParent(self)
        closeButton.SetUpVisual("d:/ymir work/ui/public/close_button_01.sub")
        closeButton.SetOverVisual("d:/ymir work/ui/public/close_button_02.sub")
        closeButton.SetDownVisual("d:/ymir work/ui/public/close_button_03.sub")
        closeButton.SetPosition(10, 13)
        closeButton.SetHorizontalAlignRight()
        closeButton.SetEvent(self.OnPressedCloseButton)
        closeButton.Show()

        if app.ENABLE_ELEMENT_ADD:
            self.elementImgDict = {}
            for element, path in self.ELEMENT_IMG_PATH.items():
                elementImg = ui.ExpandedImageBox()
                elementImg.SetParent(self)
                elementImg.LoadImage(path)
                elementImg.SetPosition(0, 0)
                if localeInfo.IsARABIC():
                    elementImg.SetHorizontalAlignRight()
                else:
                    elementImg.SetPosition(-48, 0)
                    elementImg.SetVerticalAlignCenter()
                    elementImg.SetHorizontalAlignRight()
                elementImg.Hide()
                self.elementImgDict[element] = elementImg

        infoButton = ui.Button()
        infoButton.SetParent(self)
        infoButton.SetUpVisual("d:/ymir work/ui/pattern/q_mark_01.tga")
        infoButton.SetOverVisual("d:/ymir work/ui/pattern/q_mark_02.tga")
        infoButton.SetDownVisual("d:/ymir work/ui/pattern/q_mark_01.tga")

        infoButton.SetEvent(self.ToggleInfoBoard)
        infoButton.SetHorizontalAlignCenter()
        infoButton.Hide()

        infoBoard = InfoBoard()
        infoBoard.Hide()

        self.buttonDict = []
        self.showingButtonList = []
        for buttonName in (
            localeInfo.TARGET_BUTTON_WHISPER,
            localeInfo.TARGET_BUTTON_EXCHANGE,
            localeInfo.TARGET_BUTTON_FIGHT,
            localeInfo.TARGET_BUTTON_ACCEPT_FIGHT,
            localeInfo.TARGET_BUTTON_AVENGE,
            localeInfo.TARGET_BUTTON_FRIEND,
            localeInfo.TARGET_BUTTON_INVITE_PARTY,
            localeInfo.TARGET_BUTTON_LEAVE_PARTY,
            localeInfo.TARGET_BUTTON_EXCLUDE,
            localeInfo.TARGET_BUTTON_INVITE_GUILD,
            localeInfo.TARGET_BUTTON_DISMOUNT,
            localeInfo.TARGET_BUTTON_EXIT_OBSERVER,
            localeInfo.TARGET_BUTTON_VIEW_EQUIPMENT,
            localeInfo.TARGET_BUTTON_REQUEST_ENTER_PARTY,
            localeInfo.TARGET_BUTTON_BUILDING_DESTROY,
            localeInfo.TARGET_BUTTON_EMOTION_ALLOW,
            "Kill",
            "Kick",
            localeInfo.TARGET_BUTTON_REVIVE,
        ):
            button = ui.Button()
            button.SetParent(self.row)
            button.SetUpVisual("d:/ymir work/ui/game/windows/button_normal.tga")
            button.SetOverVisual("d:/ymir work/ui/game/windows/button_hover.tga")
            button.SetDownVisual("d:/ymir work/ui/game/windows/button_down.tga")
            button.SetButtonScale(1.0, 1.0)
            button.SetText(buttonName)
            button.SetFontName("Verdana:11")
            button.Hide()

            self.buttonDict.append((buttonName, button))
            # self.showingButtonList.append(button)

        self.buttonDict = OrderedDict(self.buttonDict)

        self.buttonDict[localeInfo.TARGET_BUTTON_WHISPER].SetEvent(self.OnWhisper)
        self.buttonDict[localeInfo.TARGET_BUTTON_EXCHANGE].SetEvent(self.OnExchange)
        self.buttonDict[localeInfo.TARGET_BUTTON_FIGHT].SetEvent(self.OnPVP)
        self.buttonDict[localeInfo.TARGET_BUTTON_ACCEPT_FIGHT].SetEvent(self.OnPVP)
        self.buttonDict[localeInfo.TARGET_BUTTON_AVENGE].SetEvent(self.OnPVP)
        self.buttonDict[localeInfo.TARGET_BUTTON_FRIEND].SetEvent(
            self.OnAppendToMessenger
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_FRIEND].SetEvent(
            self.OnAppendToMessenger
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_INVITE_PARTY].SetEvent(
            self.OnPartyInvite
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_LEAVE_PARTY].SetEvent(self.OnPartyExit)
        self.buttonDict[localeInfo.TARGET_BUTTON_EXCLUDE].SetEvent(self.OnPartyRemove)

        self.buttonDict[localeInfo.TARGET_BUTTON_INVITE_GUILD].SetEvent(
            self.__OnGuildAddMember
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_DISMOUNT].SetEvent(self.__OnDismount)
        self.buttonDict[localeInfo.TARGET_BUTTON_EXIT_OBSERVER].SetEvent(
            self.__OnExitObserver
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_VIEW_EQUIPMENT].SetEvent(
            self.__OnViewEquipment
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_REQUEST_ENTER_PARTY].SetEvent(
            self.__OnRequestParty
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_BUILDING_DESTROY].SetEvent(
            self.__OnDestroyBuilding
        )
        self.buttonDict[localeInfo.TARGET_BUTTON_EMOTION_ALLOW].SetEvent(
            self.__OnEmotionAllow
        )
        if app.ENABLE_12ZI:
            self.buttonDict[localeInfo.TARGET_BUTTON_REVIVE].SetEvent(
                self.__OnReviveQustionDialog
            )
        self.buttonDict["Kill"].SetEvent(self.__OnKill)
        self.buttonDict["Kick"].SetEvent(self.__OnKick)

        self.name = name
        self.hpGauge = hpGauge
        self.hpReduction = 0.0
        self.hpInfo = hpInfo
        self.closeButton = closeButton
        self.infoButton = infoButton
        self.infoBoard = infoBoard
        self.nameString = ""
        self.nameLength = 0
        self.vid = 0
        self.eventWhisper = None
        self.isShowButton = False
        if app.ENABLE_12ZI:
            self.questionDialog = None

        self.__Initialize()
        self.ResetTargetBoard()

    def __Initialize(self):
        self.nameString = ""
        self.nameLength = 0
        self.vid = 0
        self.isShowButton = False

    def Destroy(self):
        self.Hide()
        self.eventWhisper = None
        self.closeButton = None
        self.showingButtonList = None
        self.buttonDict = None
        self.name = None
        self.hpGauge = None

        if app.ENABLE_ELEMENT_ADD:
            self.elementImgDict = None

        if self.infoBoard:
            self.infoBoard.Close()
            self.infoBoard = None
        self.__Initialize()

    def OnPressedCloseButton(self):
        player.ClearTarget()
        self.Close()

    def Close(self):
        self.__Initialize()
        self.infoBoard.Close()
        self.Hide()

    def Open(self, vid, name):
        if player.IsPVPInstance(vid) or (
            player.IsObserverMode() and not player.IsGameMaster()
        ):
            return

        if app.ENABLE_ELEMENT_ADD:
            self.__HideAllElementImg()

        if vid:
            if not constInfo.GET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD():
                if not player.IsSameEmpire(vid):
                    self.Hide()
                    return

            if vid != self.vid:
                self.ResetTargetBoard()
                self.SetTargetVID(vid)
                self.SetTargetName(name)

            if playerInst().IsMainCharacterIndex(vid):
                if self.__CanShowMainCharacterMenu():
                    self.__ShowMainCharacterMenu()
                else:
                    self.Hide()
            elif chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(self.vid):
                self.Hide()
            else:
                self.RefreshButton()
                self.Show()
        else:
            self.HideAllButton()
            self.__ShowButton(localeInfo.TARGET_BUTTON_WHISPER)
            self.__ArrangeButtonPosition()
            self.SetTargetName(name)
            self.Show()

    # OFFLINE SHOP
    SPLIT_TOKEN = "-"

    def OpenOfflineShopBoard(self, shopName):

        firstTokenPosition = shopName.find(self.SPLIT_TOKEN)
        if firstTokenPosition == -1:
            self.Hide()
            return

        playerName, postfix = shopName.split(self.SPLIT_TOKEN)

        self.hpInfo.Hide()
        self.hpGauge.Hide()
        self.HideAllButton()
        self.__ShowButton(localeInfo.TARGET_BUTTON_WHISPER)
        self.__ArrangeButtonPosition()
        self.SetTargetName(playerName.strip())
        self.SetSize(250, 60)
        self.Show()

    def Refresh(self):
        if self.IsShow():
            if self.IsShowButton():
                self.RefreshButton()

    def RefreshByVID(self, vid):
        try:
            if vid == self.vid:
                self.Refresh()
        except TypeError:
            return

    def RefreshByName(self, name):
        if name == self.GetTargetName():
            self.Refresh()

    def __CanShowMainCharacterMenu(self):
        return False

    def __ShowMainCharacterMenu(self):
        hasButton = False

        self.HideAllButton()

        if player.IsMountingHorse():
            self.__ShowButton(localeInfo.TARGET_BUTTON_DISMOUNT)
            hasButton = True

        if player.IsObserverMode():
            self.__ShowButton(localeInfo.TARGET_BUTTON_EXIT_OBSERVER)
            hasButton = True

        self.hpInfo.Hide()
        self.hpGauge.Hide()

        if hasButton:
            self.__ArrangeButtonPosition()

        self.Show()

    def SetWhisperEvent(self, event):
        self.eventWhisper = MakeEvent(event)

    def UpdatePosition(self):
        self.SetPosition(wndMgr.GetScreenWidth() / 2 - self.GetWidth() / 2, 10)
        self.row.UpdateRect()

    def ResetTargetBoard(self):
        for btn in self.buttonDict.values():
            btn.Hide()

        self.__Initialize()

        self.hpInfo.Hide()
        self.name.SetParent(self)
        self.name.SetHorizontalAlignLeft()
        self.name.SetPosition(25, 12)
        self.hpGauge.Hide()
        self.infoButton.Hide()
        self.infoBoard.Close()
        self.SetSize(250, 40)
        self.UpdatePosition()

    def SetTargetVID(self, vid):
        self.vid = vid

    def SetEnemyVID(self, vid):
        self.SetTargetVID(vid)

        name = chr.GetNameByVID(vid)
        level = nonplayer.GetLevelByVID(vid)
        grade = nonplayer.GetGradeByVID(vid)
        isAggressive = chr.IsAggressiveByVid(vid)

        nameBack = ""
        if isAggressive:
            nameBack += "*"

        if -1 != level:
            nameBack += "Lv." + str(level) + " "

        if grade in localeInfo.GRADES_TO_NAME:
            nameBack += " (" + localeInfo.GRADES_TO_NAME[grade] + ") "

        self.SetTargetName(nameBack + name)

        if app.ENABLE_ELEMENT_ADD:
            self.__HideAllElementImg()
            element = nonplayer.GetAttElementFlagByVID(vid)
            if element:
                self.__ShowElementImg(element)

        (textWidth, textHeight) = (self.name.GetWidth(), self.name.GetHeight())

        if (
            chr.IsStone(self.vid)
            or chr.IsEnemy(self.vid)
            or player.IsPVPInstance(self.vid)
        ):
            self.infoButton.SetPosition(textWidth + 32, 12)
            self.infoButton.SetHorizontalAlignLeft()
            self.infoButton.Show()

        self.SetSize(250 + len(self.name.GetText()) * 8, 40)
        self.UpdatePosition()

    def GetTargetVID(self):
        return self.vid

    def GetTargetName(self):
        return self.nameString

    def SetTargetName(self, name):
        name = name
        self.nameString = name
        self.nameLength = len(name)
        self.name.SetText(name)

    def SetHP(self, hp, maxHp, prevHp):
        hpPercentage = 1.0
        if maxHp != 0:
            hpPercentage = float(hp) / float(maxHp)

        self.hpInfo.SetText(
            "{} / {} ({:.2f}%) ".format(
                numberFormat(hp), numberFormat(maxHp), hpPercentage * 100
            )
        )

        if chr.HasAffectByVid(int(self.vid), chr.AFFECT_POISON):
            if self.hpGauge.GetGaugeColor() is not "green":
                self.hpGauge.SetGaugeColor("green")
        elif chr.HasAffectByVid(int(self.vid), chr.AFFECT_FIRE):
            if self.hpGauge.GetGaugeColor() is not "orange":
                self.hpGauge.SetGaugeColor("orange")
        elif chr.HasAffectByVid(int(self.vid), chr.AFFECT_SLOW):
            if self.hpGauge.GetGaugeColor() is not "purple":
                self.hpGauge.SetGaugeColor("purple")
        else:
            if self.hpGauge.GetGaugeColor() is not "red":
                self.hpGauge.SetGaugeColor("red")

        self.hpGauge.SetPercentage(hpPercentage * 100, 100)
        self.hpGauge.Show()
        self.hpInfo.Show()

    def ShowDefaultButton(self):
        self.isShowButton = True
        self.showingButtonList.append(self.buttonDict[localeInfo.TARGET_BUTTON_WHISPER])
        self.showingButtonList.append(
            self.buttonDict[localeInfo.TARGET_BUTTON_EXCHANGE]
        )
        self.showingButtonList.append(self.buttonDict[localeInfo.TARGET_BUTTON_FIGHT])
        self.showingButtonList.append(
            self.buttonDict[localeInfo.TARGET_BUTTON_EMOTION_ALLOW]
        )

        for button in self.showingButtonList:
            button.Show()

    def HideAllButton(self):
        self.isShowButton = False
        for button in self.showingButtonList:
            button.Hide()
        self.showingButtonList = []

    def __ShowButton(self, name):

        if name not in self.buttonDict:
            return

        self.buttonDict[name].Show()
        self.showingButtonList.append(self.buttonDict[name])

    def __HideButton(self, name):

        if name not in self.buttonDict:
            return

        button = self.buttonDict[name]
        button.Hide()

        for btnInList in self.showingButtonList:
            if btnInList == button:
                self.showingButtonList.remove(button)
                break

    def ToggleInfoBoard(self):
        if self.infoBoard.IsShow():
            self.infoBoard.Close()
            return

        vnum = chr.GetRaceByVID(self.vid)
        if vnum == 0:
            return

        if vnum not in constInfo.DROP_INFO:
            appInst.instance().GetNet().SendLoadTargetInfo(self.vid)

        self.infoBoard.Open(self, vnum)

    def RefreshTargetInfo(self):
        if self.infoBoard.IsShow():
            self.infoBoard.Refresh()

    def OnWhisper(self):
        if self.eventWhisper:
            self.eventWhisper(self.nameString)

    def OnExchange(self):
        if (
            OfflineShopManager.GetInstance().GetBuilderInstance().IsShow()
            or OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
        ):
            return
        appInst.instance().GetNet().SendExchangeStartPacket(self.vid)

    def OnPVP(self):
        appInst.instance().GetNet().SendChatPacket("/pvp %d" % self.vid)

    def OnAppendToMessenger(self):
        appInst.instance().GetNet().SendMessengerAddByVIDPacket(self.vid)

    def OnPartyInvite(self):
        appInst.instance().GetNet().SendPartyInvitePacket(self.vid)

    def OnPartyExit(self):
        appInst.instance().GetNet().SendPartyExitPacket()

    def OnPartyRemove(self):
        appInst.instance().GetNet().SendPartyRemovePacket(self.vid)

    def __OnGuildAddMember(self):
        appInst.instance().GetNet().SendGuildAddMemberPacket(self.vid)

    def __OnDismount(self):
        appInst.instance().GetNet().SendChatPacket("/ride")

    def __OnExitObserver(self):
        appInst.instance().GetNet().SendChatPacket("/observer_exit")

    def __OnViewEquipment(self):
        appInst.instance().GetNet().SendChatPacket("/view_equip " + str(self.vid))

    def __OnRequestParty(self):
        appInst.instance().GetNet().SendChatPacket("/party_request " + str(self.vid))

    def __OnDestroyBuilding(self):
        appInst.instance().GetNet().SendChatPacket("/build d %d" % self.vid)

    def __OnEmotionAllow(self):
        appInst.instance().GetNet().SendChatPacket("/emotion_allow %d" % self.vid)

    def __OnKill(self):
        cmd = "/kill %s" % self.nameString
        appInst.instance().GetNet().SendChatPacket(cmd)

    def __OnKick(self):
        cmd = "/dc %s" % self.nameString
        appInst.instance().GetNet().SendChatPacket(cmd)

    def __OnVoteBlockChat(self):
        cmd = "/vote_block_chat %s" % self.nameString
        appInst.instance().GetNet().SendChatPacket(cmd)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.OnPressedCloseButton()
            return True

    def IsShowButton(self):
        return self.isShowButton

    def RefreshButton(self):

        self.HideAllButton()

        # if app.ENABLE_12ZI:
        #    if chrmgr.IsDead(self.vid):
        #        if background.IsReviveTargetMap():
        #            self.__ShowButton(localeInfo.TARGET_BUTTON_REVIVE)
        #            self.__ArrangeButtonPosition()
        #        return

        if chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(self.vid):
            self.SetSize(200 + 7 * self.nameLength, 40)
            self.__ArrangeButtonPosition()
            self.UpdatePosition()
            return

        if player.IsPVPInstance(self.vid) or player.IsObserverMode():
            # PVP_INFO_SIZE_BUG_FIX
            self.SetSize(200 + 7 * self.nameLength, 40)
            self.__ArrangeButtonPosition()

            self.UpdatePosition()
            self.Close()
            # END_OF_PVP_INFO_SIZE_BUG_FIX
            return

        self.ShowDefaultButton()

        if guild.MainPlayerHasAuthority(long(guild.AUTH_ADD_MEMBER)):
            if not guild.IsMemberByName(self.nameString):
                if 0 == chr.GetGuildID(self.vid):
                    self.__ShowButton(localeInfo.TARGET_BUTTON_INVITE_GUILD)

        if player.IsGameMaster() or app.IsEnableTestServerFlag():
            self.__ShowButton(localeInfo.TARGET_BUTTON_VIEW_EQUIPMENT)
            self.__ShowButton("Kick")
            self.__ShowButton("Kill")

        if not messenger.IsFriendByName(self.nameString) and (
            not messenger.IsTeamByName(self.nameString) or player.IsGameMaster()
        ):
            self.__ShowButton(localeInfo.TARGET_BUTTON_FRIEND)

        if player.IsPartyMember(self.vid):

            self.__HideButton(localeInfo.TARGET_BUTTON_FIGHT)

            if player.IsPartyLeader(self.vid):
                self.__ShowButton(localeInfo.TARGET_BUTTON_LEAVE_PARTY)
            elif player.IsPartyLeader(playerInst().GetMainCharacterIndex()):
                self.__ShowButton(localeInfo.TARGET_BUTTON_EXCLUDE)

        else:
            if player.IsPartyMember(playerInst().GetMainCharacterIndex()):
                if player.IsPartyLeader(playerInst().GetMainCharacterIndex()):
                    self.__ShowButton(localeInfo.TARGET_BUTTON_INVITE_PARTY)
            else:
                if chr.IsPartyMember(self.vid):
                    self.__ShowButton(localeInfo.TARGET_BUTTON_REQUEST_ENTER_PARTY)
                else:
                    self.__ShowButton(localeInfo.TARGET_BUTTON_INVITE_PARTY)

            if player.IsRevengeInstance(self.vid):
                self.__HideButton(localeInfo.TARGET_BUTTON_FIGHT)
                self.__ShowButton(localeInfo.TARGET_BUTTON_AVENGE)
            elif player.IsChallengeInstance(self.vid):
                self.__HideButton(localeInfo.TARGET_BUTTON_FIGHT)
                self.__ShowButton(localeInfo.TARGET_BUTTON_ACCEPT_FIGHT)
            elif player.IsCantFightInstance(self.vid):
                self.__HideButton(localeInfo.TARGET_BUTTON_FIGHT)

            if not player.IsSameEmpire(self.vid):
                self.__HideButton(localeInfo.TARGET_BUTTON_INVITE_PARTY)

        distance = player.GetCharacterDistance(self.vid)
        if distance > self.EXCHANGE_LIMIT_RANGE:
            self.__HideButton(localeInfo.TARGET_BUTTON_EXCHANGE)

        self.__ArrangeButtonPosition()

    def __ArrangeButtonPosition(self):
        self.row.SetSize(525, 55)
        buttonCount = 0
        pos = 25
        posY = 15
        for button in self.showingButtonList:
            if button.IsShow():
                button.SetPosition(pos, posY)
                button.SetTop()
                pos += 95
                buttonCount += 1

            if buttonCount == 5:
                posY += 30
                pos = 25
                # buttonCount = 0
                self.row.SetSize(self.row.GetWidth(), self.row.GetHeight() + posY)

        self.SetSize(525, 55 + posY)

        self.UpdateRect()
        self.row.SetHorizontalAlignLeft()
        self.row.UpdateRect()

        self.name.SetParent(self)
        self.name.SetHorizontalAlignCenter()
        self.name.SetPosition(0, 5)
        self.UpdatePosition()

    @staticmethod
    def clamp(n, smallest, largest):
        return max(smallest, min(n, largest))

    def OnUpdate(self):
        if self.isShowButton:

            exchangeButton = self.buttonDict[localeInfo.TARGET_BUTTON_EXCHANGE]
            distance = player.GetCharacterDistance(self.vid)

            if distance < 0:
                if app.WJ_NEW_USER_CARE:
                    player.ClearTarget()
                    self.Close()
                return

            if exchangeButton.IsShow():
                if distance > self.EXCHANGE_LIMIT_RANGE:
                    self.RefreshButton()

            else:
                if distance < self.EXCHANGE_LIMIT_RANGE:
                    self.RefreshButton()

    if app.ENABLE_ELEMENT_ADD:

        def __HideAllElementImg(self):

            for elementImg in self.elementImgDict.values():
                elementImg.Hide()

        def __ShowElementImg(self, key):
            if key not in self.elementImgDict:
                return False

            self.elementImgDict[key].Show()
            return True

    if app.ENABLE_12ZI:

        def __OnReviveQustionDialog(self):
            appInst.instance().GetNet().SendChatPacket(
                "/revivedialog {}".format(self.vid)
            )

        def OpenReviveDialog(self, vid, itemcount):
            self.questionDialog = uiCommon.QuestionDialog()
            if player.IsMainCharacterIndex(vid):
                self.questionDialog.SetText(
                    localeInfo.REVIVE_SELF_QUESTION.format(itemcount)
                )
            else:
                self.questionDialog.SetText(
                    localeInfo.REVIVE_QUESTION.format(chr.GetNameByVID(vid), itemcount)
                )

            self.questionDialog.SetAcceptEvent(Event(self.OnRevive, vid))
            self.questionDialog.SetCancelEvent(self.OnQuestionDialogClose)
            self.questionDialog.Open()

        def OnRevive(self, vid):
            appInst.instance().GetNet().SendChatPacket("/revive {}".format(vid))
            self.OnQuestionDialogClose()

        def OnQuestionDialogClose(self):
            self.questionDialog.Close()
            self.questionDialog = None
            return True


class InfoBoard(ui.ThinBoardOld):
    class ItemListBoxItem(ui.ListBoxExNew.Item):
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
            ui.ListBoxExNew.Item.__init__(self)

            self.onMouseOverIn = None
            self.isMetin = False
            self.metinCounter = 30

            icon = ui.ExpandedImageBox()
            icon.SetParent(self)
            icon.Show()
            self.icon = icon

            nameLine = ui.TextLine()
            nameLine.SetParent(self)
            nameLine.SetPosition(32 + 5, 0)
            nameLine.SetFontName("Verdana:12")
            nameLine.SetFontColor(1, 1, 1, 0.80)
            nameLine.Show()
            self.nameLine = nameLine

            self.SetSize(210, 32 + 5)
            self.UpdateRect()

        def LoadImage(self, image, name=None):
            self.icon.LoadImage(image)
            self.SetSize(
                self.GetWidth(),
                self.icon.GetHeight() + 5 * (self.icon.GetHeight() / 32),
            )

            if name is not None:
                self.SetText(name)
            self.UpdateRect()

        def SetText(self, text):
            self.nameLine.SetText(text)

        def RefreshHeight(self):
            ui.ListBoxExNew.Item.RefreshHeight(self)
            self.icon.SetRenderingRect(
                0.0,
                0.0 - float(self.removeTop) / float(max(0.1, self.GetHeight())),
                0.0,
                0.0 - float(self.removeBottom) / float(max(0.1, self.GetHeight())),
            )
            self.icon.SetPosition(0, -self.removeTop)

        def SetOnMouseOverInEvent(self, event):
            self.onMouseOverIn = MakeEvent(event)

        def SetOnMouseOverOutEvent(self, event):
            self.onMouseOverOut = MakeEvent(event)

        def OnMouseOverOut(self):
            if self.onMouseOverOut:
                self.onMouseOverOut()
                return True
            return False

        def OnMouseOverIn(self):
            if self.onMouseOverIn:
                self.onMouseOverIn()
                return True
            return False

        def OnUpdate(self):
            if self.isMetin:
                if self.metinCounter == 30:
                    self.metinCounter = 0

                    item.SelectItem(self.VNUMS[random.randint(0, len(self.VNUMS))])
                    self.icon.LoadImage(item.GetIconImageFileName())

                self.metinCounter += 1

    STONE_START_VNUM = 28030
    STONE_LAST_VNUM = 28042

    EXP_BASE_LVDELTA = [
        1,  # -15 0
        5,  # -14 1
        10,  # -13 2
        20,  # -12 3
        30,  # -11 4
        50,  # -10 5
        70,  # -9  6
        80,  # -8  7
        85,  # -7  8
        90,  # -6  9
        92,  # -5  10
        94,  # -4  11
        96,  # -3  12
        98,  # -2  13
        100,  # -1  14
        100,  # 0   15
        105,  # 1   16
        110,  # 2   17
        115,  # 3   18
        120,  # 4   19
        125,  # 5   20
        130,  # 6   21
        135,  # 7   22
        140,  # 8   23
        145,  # 9   24
        150,  # 10  25
        155,  # 11  26
        160,  # 12  27
        165,  # 13  28
        170,  # 14  29
        180,  # 15  30
    ]

    BOARD_WIDTH = 250
    SHOW_ITEM_MAX_COUNT = 5

    def __init__(self):
        ui.ThinBoardOld.__init__(self)

        self.race = 0

        self.itemTooltip = uiToolTip.ItemToolTip()
        self.itemTooltip.HideToolTip()

        self.SetSize(self.BOARD_WIDTH, 0)

    def __UpdatePosition(self, targetBoard):
        self.SetPosition(
            targetBoard.GetLeft() + (targetBoard.GetWidth() - self.GetWidth()) / 2,
            targetBoard.GetBottom(),
        )
        targetBoard.SetTop()

    def Open(self, targetBoard, race):
        self.__LoadInformation(race)

        self.SetSize(self.BOARD_WIDTH, self.yPos + 10)
        self.__UpdatePosition(targetBoard)

        self.Show()

    def Refresh(self):
        self.__LoadInformation(self.race)
        self.SetSize(self.BOARD_WIDTH, self.yPos + 10)

    def Close(self):
        self.itemTooltip.HideToolTip()
        self.Hide()

    def __LoadInformation(self, race):
        self.yPos = 7
        self.children = []
        self.race = race

        self.__AppendDefault(race)
        self.__AppendRace(race)
        self.__AppendResists(race)
        self.__AppendDrops(race)

    @staticmethod
    def __GetDefaultHitRate(race):
        attacker_dx = nonplayer.GetMonsterDX(race)
        attacker_level = nonplayer.GetMonsterLevel(race)

        self_dx = playerInst().GetPoint(player.DX)
        self_level = playerInst().GetPoint(player.LEVEL)

        iARSrc = min(90, (attacker_dx * 4 + attacker_level * 2) / 6)
        iERSrc = min(90, (self_dx * 4 + self_level * 2) / 6)

        fAR = (float(iARSrc) + 210.0) / 300.0
        fER = (float(iERSrc) * 2 + 5) / (float(iERSrc) + 95) * 3.0 / 10.0

        return fAR - fER

    def __AppendResists(self, race):
        self.AppendSeperator()
        self.AppendTextLine(localeInfo.TARGET_INFO_RESISTS)
        self.AppendTextLine(
            localeInfo.TARGET_INFO_RESISTS_LINE0.format(
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_SWORD),
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_TWOHAND),
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_BELL),
            )
        )
        self.AppendTextLine(
            localeInfo.TARGET_INFO_RESISTS_LINE1.format(
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_DAGGER),
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_FAN),
                nonplayer.GetResist(race, nonplayer.MOB_RESIST_BOW),
            )
        )

    def __AppendDefault(self, race):
        self.AppendSeperator()

        self.AppendTextLine(
            localeInfo.TARGET_INFO_MAX_HP.format(
                localeInfo.DottedNumber(nonplayer.GetMonsterMaxHP(race))
            )
        )

        # calc att damage
        monsterLevel = nonplayer.GetMonsterLevel(race)
        fHitRate = self.__GetDefaultHitRate(race)
        iDamMin, iDamMax = nonplayer.GetMonsterDamage(race)
        iDamMin = (
            int((iDamMin + nonplayer.GetMonsterST(race)) * 2 * fHitRate)
            + monsterLevel * 2
        )
        iDamMax = (
            int((iDamMax + nonplayer.GetMonsterST(race)) * 2 * fHitRate)
            + monsterLevel * 2
        )
        iDef = (
            playerInst().GetPoint(player.DEF_GRADE)
            * (100 + playerInst().GetPoint(player.DEF_BONUS))
            / 100
        )
        fDamMulti = nonplayer.GetMonsterDamageMultiply(race)
        iDamMin = int(max(0, iDamMin - iDef) * fDamMulti)
        iDamMax = int(max(0, iDamMax - iDef) * fDamMulti)
        if iDamMin < 1:
            iDamMin = 1
        if iDamMax < 5:
            iDamMax = 5
        self.AppendTextLine(
            localeInfo.TARGET_INFO_DAMAGE.format(str(iDamMin), str(iDamMax))
        )

        idx = min(
            len(self.EXP_BASE_LVDELTA) - 1,
            max(0, (monsterLevel + 15) - playerInst().GetPoint(player.LEVEL)),
        )
        iExp = nonplayer.GetMonsterExp(race) * self.EXP_BASE_LVDELTA[int(idx)] / 100
        self.AppendTextLine(
            localeInfo.TARGET_INFO_EXP.format(localeInfo.DottedNumber(iExp))
        )

        self.AppendTextLine(
            localeInfo.TARGET_INFO_GOLD_MIN_MAX.format(
                localeInfo.DottedNumber(nonplayer.GetMobGoldMin(race)),
                localeInfo.DottedNumber(nonplayer.GetMobGoldMax(race)),
            )
        )
        self.AppendTextLine(
            localeInfo.TARGET_INFO_REGEN_INFO.format(
                nonplayer.GetMobRegenPercent(race), nonplayer.GetMobRegenCycle(race)
            )
        )

    def __AppendRace(self, race):
        self.AppendSeperator()

        raceFlag = nonplayer.GetMonsterRaceFlag(race)

        mainrace = ""
        subrace = ""
        for i in xrange(17):
            curFlag = 1 << i
            if (raceFlag & curFlag) == curFlag:

                if curFlag in localeInfo.RACE_FLAG_TO_NAME:
                    mainrace += localeInfo.RACE_FLAG_TO_NAME[curFlag] + ", "
                elif curFlag in localeInfo.SUB_RACE_FLAG_TO_NAME:
                    subrace += localeInfo.SUB_RACE_FLAG_TO_NAME[curFlag] + ", "

        if nonplayer.IsMonsterStone(race):
            mainrace += localeInfo.TARGET_INFO_RACE_METIN + ", "

        if mainrace == "":
            mainrace = localeInfo.TARGET_INFO_NO_RACE
        else:
            mainrace = mainrace[:-2]

        if subrace == "":
            subrace = localeInfo.TARGET_INFO_NO_RACE
        else:
            subrace = subrace[:-2]

        self.AppendTextLine(localeInfo.TARGET_INFO_MAINRACE.format(mainrace))
        self.AppendTextLine(localeInfo.TARGET_INFO_SUBRACE.format(subrace))

    def __AppendDrops(self, race):
        self.AppendSeperator()

        if race in constInfo.DROP_INFO:
            if len(constInfo.DROP_INFO[race]) == 0:
                self.AppendTextLine(localeInfo.TARGET_INFO_NO_ITEM_TEXT)
            else:
                scrollWindow = ui.ScrollWindow()
                scrollWindow.SetWindowName("TargetInfoScroll")
                scrollWindow.SetPosition(0, 0)
                from ui_modern_controls import AutoGrowingVerticalContainerEx

                itemListBox = AutoGrowingVerticalContainerEx()
                itemListBox.SetWidth(246)

                count = 0
                height = 0
                for vnum, item in constInfo.DROP_INFO[race].iteritems():
                    for info in item:
                        height += self.AppendItem(
                            itemListBox, info["VNUM"], info["COUNT"], info["PCT"]
                        )
                        count += 1

                window_width = 37
                window_size = window_width * count

                if count > self.SHOW_ITEM_MAX_COUNT:
                    window_size = window_width * self.SHOW_ITEM_MAX_COUNT

                scrollWindow.SetSize(210, window_size)

                scrollWindow.SetContentWindow(itemListBox)
                scrollWindow.FitWidth(210)
                scrollWindow.SetParent(self)
                scrollWindow.Show()
                itemListBox.Show()
                self.children.append(itemListBox)

                if count == 0:
                    self.AppendTextLine(localeInfo.TARGET_INFO_NO_ITEM_TEXT)
                else:
                    self.AppendWindow(scrollWindow, 30)
        else:
            self.AppendTextLine(localeInfo.TARGET_INFO_NO_ITEM_TEXT)

    def AppendTextLine(self, text):
        textLine = ui.TextLine()
        textLine.SetParent(self)
        textLine.SetHorizontalAlignCenter()
        textLine.SetHorizontalAlignCenter()
        textLine.SetFontName("Verdana:11")
        textLine.SetFontColor(1, 1, 1, 0.80)
        textLine.SetText(text)
        textLine.SetPosition(0, self.yPos)
        textLine.Show()

        self.children.append(textLine)
        self.yPos += 17

    def AppendSeperator(self):
        img = ui.ImageBox()
        img.LoadImage("d:/ymir work/ui/seperator.tga")
        self.AppendWindow(img)
        img.SetPosition(img.GetLeft(), img.GetTop() - 15)
        self.yPos -= 15

    def AppendItem(self, listBox, vnum, count, pct):
        myItem = self.ItemListBoxItem(210)
        myItem.SetParent(listBox)
        countString = str(count) + "x "
        if type(vnum) == int:
            item.SelectItem(vnum)
            itemName = item.GetItemName()

            myItem.SetOnMouseOverInEvent(Event(self.OnShowItemTooltip, vnum))
        else:
            item.SelectItem(vnum[0])
            itemName = item.GetItemName()
            gradeFrom = vnum[0] % 10
            gradeTo = vnum[1] % 10
            if gradeFrom == gradeTo:
                itemName = "{} +{}".format(
                    itemName[: itemName.find("+")], gradeFrom, pct
                )
            else:
                itemName = "{} +{} - +{}".format(
                    itemName[: itemName.find("+")], gradeFrom, gradeTo, pct
                )

            myItem.SetOnMouseOverInEvent(Event(self.OnShowItemTooltip, vnum[0]))

        myItem.LoadImage(item.GetIconImageFileName())
        myItem.SetText("{} ({})".format(countString + itemName, FormatRarity(pct)))
        myItem.SetOnMouseOverOutEvent(self.OnHideItemTooltip)
        myItem.SetTop()

        listBox.AppendItem(myItem)
        myItem.Show()

        return myItem.GetHeight()

    def AppendMetinStone(self, listBox):
        myItem = self.ItemListBoxItem(listBox.GetWidth())
        myItem.isMetin = True
        myItem.SetText("{} +0 - +4".format(localeInfo.TARGET_INFO_STONE_NAME))
        listBox.AppendItem(myItem)

        return myItem.GetHeight()

    def OnShowItemTooltip(self, vnum):
        item.SelectItem(vnum)
        self.itemTooltip.SetItemToolTip(vnum)

    def OnHideItemTooltip(self):
        self.itemTooltip.HideToolTip()

    def AppendWindow(self, wnd, x=0, width=0, height=0):
        if width == 0:
            width = wnd.GetWidth()
        if height == 0:
            height = wnd.GetHeight()

        wnd.SetParent(self)
        if x == 0:
            wnd.SetPosition((self.GetWidth() - width) / 2, self.yPos)
        else:
            wnd.SetPosition(x, self.yPos)
        wnd.Show()

        self.children.append(wnd)
        self.yPos += height + 5


class AllianceTargetBoard(ui.ThinBoard):
    def __init__(self):
        ui.ThinBoard.__init__(self)

        self.name = ui.TextLine()
        self.name.SetParent(self)
        self.name.SetDefaultFontName()
        self.name.SetOutline()
        self.name.Show()

        self.hpGauge = ui.Gauge()
        self.hpGauge.SetParent(self)
        self.hpGauge.MakeGauge(84, "red")
        self.hpGauge.SetHorizontalAlignLeft()
        self.hpGauge.SetPosition(11, 21)
        self.hpGauge.Hide()

        self.tooltipHP = ui.TextToolTip()
        self.tooltipHP.Hide()

        self.ResetTargetBoard()

    def Initialize(self):
        self.vid = 0
        self.nameString = ""
        self.nameLength = 0
        self.hp = 0
        self.hpMax = 0
        self.hpPer = 0

    def Destroy(self):
        self.name = None
        self.hpGauge = None
        self.tooltipHP = None
        self.Initialize()

    def GetTargetVID(self):
        return self.vid

    def SetTargetVID(self, vid):
        self.vid = vid

    def SetTarget(self, vid):
        self.SetTargetVID(vid)
        self.SetTargetName(chr.GetNameByVID(vid))

    def SetTargetName(self, name):
        self.nameString = name
        self.nameLength = len(name)
        self.name.SetText(self.nameString)

    def SetHP(self, hp, maxHp):
        if not self.hpGauge.IsShow():
            self.SetSize(106, 36)
            if localeInfo.IsARABIC():
                self.name.SetPosition(self.GetWidth() - 11, 6)
            else:
                self.name.SetPosition(11, 6)
            self.name.SetHorizontalAlignLeft()
            self.hpGauge.Show()
            self.SetPosition(wndMgr.GetScreenWidth() - 117, 154)

        if not maxHp <= 0:
            self.hp = hp
            self.hpMax = maxHp
            self.hpPer = self.hp * 100 / self.hpMax
            self.hpGauge.SetPercentage(self.hpPer, 100)

            self.tooltipHP.SetText(
                "{:s} : {:d} / {:d}".format(localeInfo.TASKBAR_HP, self.hp, self.hpMax)
            )

    def ResetTargetBoard(self):
        self.Initialize()

        self.name.SetPosition(11, 6)
        self.name.SetHorizontalAlignCenter()

        self.hpGauge.Hide()

        self.SetSize(106, 36)
