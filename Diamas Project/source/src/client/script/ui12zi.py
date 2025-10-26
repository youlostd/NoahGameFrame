# -*- coding: utf-8 -*-

import logging

import app
import player
import wndMgr
from pygame.app import appInst
from pygame.player import playerInst

import constInfo
import localeInfo
import ui
import uiCommon
import uiToolTip
from ui_event import Event


class BeadWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.SetWindowName("Bead")
        self.BeadIcon = None
        self.beadInfo = None
        self.nextBeadUpdateTime = 0
        self.tooltipInfo = None
        self.ShowButtonToolTip = False
        self.isLoaded = False
        self.__LoadWindow()

    def __LoadWindow(self):
        if self.isLoaded:
            return
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/Bead.py")

        except:
            logging.exception("BeadWindow.__LoadWindow.UIScript/Bead.py")

        self.BeadIcon = self.GetChild("Bead_image")
        self.beadInfo = self.GetChild("Beadcount")

        self.tooltipInfo = ui.TextToolTip()
        self.tooltipInfo.SetText(localeInfo.BEAD)
        self.tooltipInfo.Show()
        self.isLoaded = True

    def OnUpdate(self):
        if True == self.BeadIcon.IsIn():
            curTime = app.GetGlobalTime() / 1000
            leftTime = max(0, self.nextBeadUpdateTime - curTime)
            if leftTime == 0:
                self.tooltipInfo.SetText(localeInfo.BEAD)
            else:
                self.tooltipInfo.SetText(localeInfo.BEAD_TIME_REMAINING_TO_GET.format(leftTime / 60))

            self.tooltipInfo.Show()
        else:
            self.tooltipInfo.Hide()

    def NextBeadUpdateTime(self, value):
        curTime = app.GetGlobalTime() / 1000
        self.nextBeadUpdateTime = (app.GetGlobalTime() / 1000) + value

    def SetBeadCount(self, value):
        self.beadInfo.SetNumber("%d" % value)

    def Close(self):
        self.Hide()

    def Destroy(self):
        self.Hide()
        self.ClearDictionary()


class FloorLimitTimeWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.JumpStep = None
        self.LeftTime = None
        self.CurrentFloor = None
        self.CoolTime = 25 * 60
        self.CoolTimeImage = None
        self.btnJump = None
        self.startTime = 0
        self.jumpCount = 0

    def __LoadWindow(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/12FloorInfo.py")
        except Exception as e:
            logging.exception(e)

        self.LeftTime = self.GetChild("LeftTime")
        self.JumpStep = self.GetChild("JumpStep")
        self.CurrentFloor = self.GetChild("CurrentFloor")
        self.CoolTimeImage = self.GetChild("CoolTime")
        self.btnJump = self.GetChild("JumpButton")
        self.btnJump.SetEvent(self.PressJumpButton)
        self.btnJump.Hide()
        self.StartCoolTime(1)

    def Open(self):
        self.__LoadWindow()
        self.btnJump.Hide()
        self.CoolTimeImage.SetCoolTime(self.CoolTime)
        self.CoolTimeImage.SetStartCoolTime(self.startTime)

        self.Show()

    def OnUpdate(self):
        leftTime = max(0, self.startTime + self.CoolTime - app.GetTime() + 0.5)

        self.LeftTime.SetText(localeInfo.SecondToColonTypeHMS(leftTime))

        if self.btnJump.IsShow() and leftTime == 0:
            self.NextMission()

    def StartCoolTime(self, time):
        self.startTime = time

    def Refresh12ziTimer(self, currentFloor, jumpCount, limitTime, elapseTime):
        self.CoolTime = limitTime
        self.startTime = app.GetTime() - elapseTime + 0.5
        self.jumpCount = jumpCount
        self.CurrentFloor.SetText("%d%s" % (currentFloor, "F"))
        if jumpCount > 0:
            self.JumpStep.SetText("%d%s" % (currentFloor + jumpCount, "F"))
        else:
            self.JumpStep.SetText("-")

    def Show12ziJumpButton(self):
        self.btnJump.Show()
        self.btnJump.EnableFlash()

    def PressJumpButton(self):
        self.btnJump.DisableFlash()
        self.btnJump.Hide()
        appInst.instance().GetNet().SendChatPacket("/jumpfloor")

    def NextMission(self):
        appInst.instance().GetNet().SendChatPacket("/nextfloor")
        self.btnJump.Hide()

    def Close(self):
        self.Hide()

    def Destroy(self):
        self.Hide()
        self.ClearDictionary()


class Reward12ziWindow(ui.ScriptWindow):
    COLUMN_SIZE = 12
    ROW_SIZE = 10

    COLUMN_ITEM_LIST = (33001, 33002, 33003, 33004, 33005, 33006, 33007, 33008, 33009, 33010, 33011, 33012)
    ROW_ITEM_LIST = (33013, 33014, 33015, 33016, 33017, 33018, 33019, 33020, 33021, 33022)

    WEEKLY_ROW_LIST = (localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_MON, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_TUE,
                       localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_WED, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_THU,
                       localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_FRI, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_SAT)
    WEEKLY_COLUMN_LIST = (localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_MON, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_TUE,
                          localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_WED, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_THU,
                          localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_FRI, localeInfo.CZ_REWARD_TOOLTIP_WEEKLY_SAT_SUN)

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.enableYellow = True
        self.enableGreen = True
        self.CoverImage1 = {}
        self.btnYellow = None
        self.btnGreen = None
        self.btnAll = None
        self.YellowRewardCount = None
        self.GreenRewardCount = None
        self.questionDialog = None
        self.COLUMN_ITEM_COUNT = {}
        self.COLUMN_ITEM = {}
        self.ROW_ITEM_COUNT = {}
        self.ROW_ITEM = {}
        self.CHECK_BUTTON = ({}, {},)
        self.buttontooltip = None
        self.tooltipItem = None
        self.ShowButtonToolTip = False

    def OverInToolTipButton(self, arg):
        arglen = len(str(arg))
        pos_x, pos_y = wndMgr.GetMousePosition()

        self.buttontooltip.ClearToolTip()
        self.buttontooltip.SetThinBoardSize(11 * arglen)
        self.buttontooltip.SetToolTipPosition(pos_x + 50, pos_y + 50)
        self.buttontooltip.AppendTextLine(arg, 0xffffffff)
        self.buttontooltip.Show()
        self.ShowButtonToolTip = True

    def OverOutToolTipButton(self):
        self.buttontooltip.Hide()
        self.ShowButtonToolTip = False

    def ButtonToolTipProgress(self):
        if self.buttontooltip and self.ShowButtonToolTip:
            pos_x, pos_y = wndMgr.GetMousePosition()
            self.buttontooltip.SetToolTipPosition(pos_x, pos_y - 20)

    def EventProgress(self, event_type, arg):
        # print "EventProcess %s, %s" %(event_type, slot)
        if "mouse_click" == event_type:
            self.OverInToolTipButton(arg)
        elif "mouse_over_in" == event_type:
            self.OverInToolTipButton(arg)
        elif "mouse_over_out" == event_type:
            self.OverOutToolTipButton()
        else:
            print("ui12zi.py ::EventProgress : False")

    def SetItemToolTip(self, tooltipItem):
        from _weakref import proxy
        self.tooltipItem = proxy(tooltipItem)

    def ItemToolTipEvent(self, event_type, arg):
        if None == self.tooltipItem:
            return

        if "mouse_over_in" == event_type:
            self.tooltipItem.SetItemToolTip(arg)
        elif "mouse_over_out" == event_type:
            self.tooltipItem.HideToolTip()

    ## Update
    def OnUpdate(self):
        self.ButtonToolTipProgress()

    def __LoadWindow(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/12ziRewardWindow.py")
        except:
            logging.exception("Could not load script file UIScript/12ziRewardWindow.py")

        self.buttontooltip = uiToolTip.ToolTip()
        self.buttontooltip.ClearToolTip()

        self.GetChild("bg_weekly_row").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                      localeInfo.CZ_REWARD_TOOLTIP_WEEKLY))
        self.GetChild("bg_weekly_row").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        self.GetChild("bg_weekly_column").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                         localeInfo.CZ_REWARD_TOOLTIP_WEEKLY))
        self.GetChild("bg_weekly_column").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        for i in xrange(6):
            self.GetChild("bg_weekly_row_%d" % (i + 1)).SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                                       self.WEEKLY_ROW_LIST[i]))
            self.GetChild("bg_weekly_row_%d" % (i + 1)).SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out",
                                                                       ""))
            self.GetChild("bg_weekly_column_%d" % (i + 1)).SetOnMouseOverInEvent(Event(self.EventProgress,
                                                                          "mouse_over_in", self.WEEKLY_COLUMN_LIST[i]))
            self.GetChild("bg_weekly_column_%d" % (i + 1)).SetOnMouseOverOutEvent(Event(self.EventProgress,
                                                                          "mouse_over_out", ""))

        self.GetChild("bg_charm_12zi").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                      localeInfo.CZ_REWARD_TOOLTIP_12ZI))
        self.GetChild("bg_charm_12zi").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        for i in xrange(self.COLUMN_SIZE):
            self.GetChild("bg_charm_12zi_%d" % (i + 1)).SetOnMouseOverInEvent(Event(self.ItemToolTipEvent,
                                                                       "mouse_over_in", self.COLUMN_ITEM_LIST[i]))
            self.GetChild("bg_charm_12zi_%d" % (i + 1)).SetOnMouseOverOutEvent(Event(self.ItemToolTipEvent,
                                                                       "mouse_over_out", ""))

        self.GetChild("bg_charm_10gan").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                       localeInfo.CZ_REWARD_TOOLTIP_10GAN))
        self.GetChild("bg_charm_10gan").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        for i in xrange(self.ROW_SIZE):
            self.GetChild("bg_charm_10gan_%d" % (i + 1)).SetOnMouseOverInEvent(Event(self.ItemToolTipEvent,
                                                                        "mouse_over_in", self.ROW_ITEM_LIST[i]))
            self.GetChild("bg_charm_10gan_%d" % (i + 1)).SetOnMouseOverOutEvent(Event(self.ItemToolTipEvent,
                                                                        "mouse_over_out", ""))

        self.GetChild("bg_count_slot_row").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                          localeInfo.CZ_REWARD_TOOLTIP_12ZI_COUNT))
        self.GetChild("bg_count_slot_row").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))
        self.GetChild("bg_count_slot_column").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                             localeInfo.CZ_REWARD_TOOLTIP_10GAN_COUNT))
        self.GetChild("bg_count_slot_column").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))
        self.GetChild("bg_need_count_slot_row").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                               localeInfo.CZ_REWARD_TOOLTIP_12ZI_NEED_COUNT))
        self.GetChild("bg_need_count_slot_row").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))
        self.GetChild("bg_need_count_slot_column").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                                  localeInfo.CZ_REWARD_TOOLTIP_10GAN_NEED_COUNT))
        self.GetChild("bg_need_count_slot_column").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        self.GetChild("YellowInactive").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                       localeInfo.CZ_REWARD_TOOLTIP_YELLOW_BOX))
        self.GetChild("YellowInactive").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))
        self.GetChild("GreenInactive").SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in",
                                                      localeInfo.CZ_REWARD_TOOLTIP_GREEN_BOX))
        self.GetChild("GreenInactive").SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", ""))

        self.btnYellow = self.GetChild("YellowButton")
        self.btnYellow.SetAlwaysToolTip(True)
        self.btnGreen = self.GetChild("GreenButton")
        self.btnGreen.SetAlwaysToolTip(True)
        self.btnAll = self.GetChild("AllClearButton")
        self.btnAll.SetAlwaysToolTip(True)

        self.btnYellow.SetEvent(Event(self.PressButton, 1))
        self.btnYellow.SetAlwaysToolTip(True)
        self.btnYellow.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                           localeInfo.CZ_REWARD_TOOLTIP_YELLOW_BOX))
        self.btnYellow.SetHideToolTipEvent(self.OverOutToolTipButton)
        self.btnGreen.SetEvent(Event(self.PressButton, 2))
        self.btnGreen.SetAlwaysToolTip(True)
        self.btnGreen.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                          localeInfo.CZ_REWARD_TOOLTIP_GREEN_BOX))
        self.btnGreen.SetHideToolTipEvent(self.OverOutToolTipButton)
        self.btnAll.SetEvent(Event(self.PressButton, 3))
        self.btnAll.SetAlwaysToolTip(True)
        self.btnAll.SetShowToolTipEvent(Event(self.OverInToolTipButton, localeInfo.CZ_REWARD_TOOLTIP_ALL_BOX))
        self.btnAll.SetHideToolTipEvent(self.OverOutToolTipButton)
        self.GetChild("TitleBar").SetCloseEvent(self.Close)

        self.CheckButtonWindow = self.GetChild("CheckButtonWindow")
        self.YellowRewardCount = self.GetChild("YellowRewardCount")
        self.GreenRewardCount = self.GetChild("GreenRewardCount")

        if localeInfo.IsARABIC():
            self.GetChild("BgImage").LeftRightReverse()
            self.GetChild("bg_count_slot_row").LeftRightReverse()
            self.GetChild("bg_need_count_slot_row").LeftRightReverse()
            self.GetChild("bg_count_slot_column").LeftRightReverse()
            self.GetChild("bg_need_count_slot_column").LeftRightReverse()
            self.GetChild("bg_check").LeftRightReverse()
            self.GetChild("YellowInactive").LeftRightReverse()
            self.GetChild("GreenInactive").LeftRightReverse()
            self.btnYellow.LeftRightReverse()
            self.btnGreen.LeftRightReverse()
            self.btnAll.LeftRightReverse()

        if len(self.COLUMN_ITEM_COUNT) == 0:
            for i in xrange(self.COLUMN_SIZE):
                TempText = ui.MakeTextLine(self.GetChild("12zi%d" % (i + 1)))
                TempText.SetPosition(0, -21)
                TempText.SetText("%d" % (0))
                self.COLUMN_ITEM_COUNT[i] = TempText
                self.InsertChild("12ziCount%d" % (i), TempText, "text")

            for i in xrange(self.ROW_SIZE):
                TempText = ui.MakeTextLine(self.GetChild("10gan%d" % (i + 1)))
                if localeInfo.IsARABIC():
                    TempText.SetPosition(28, 7)
                else:
                    TempText.SetPosition(-28, 7)
                TempText.SetText("%d" % (0))
                self.ROW_ITEM_COUNT[i] = TempText
                self.InsertChild("10ganCount%d" % (i), TempText, "text")

        if self.CheckButtonWindow.GetChildCount() != 0:
            return
        else:
            for i in xrange(self.COLUMN_SIZE):
                for j in xrange(self.ROW_SIZE):
                    if i % 2 == j % 2:
                        if j % 2 == 0:
                            if localeInfo.IsARABIC():
                                CheckButton = ui.MakeButton(self.CheckButtonWindow, 311 - i * 28, j * 28, "",
                                                            "d:/ymir work/ui/game/12zi/reward/", "check_yellow.sub",
                                                            "clear_yellow.sub", "clear_yellow.sub")
                            else:
                                CheckButton = ui.MakeButton(self.CheckButtonWindow, i * 28, j * 28, "",
                                                            "d:/ymir work/ui/game/12zi/reward/", "check_yellow.sub",
                                                            "clear_yellow.sub", "clear_yellow.sub")

                            index = i / 2 + (j / 2) * 6
                            self.CHECK_BUTTON[0][index] = CheckButton
                            CheckButton.SetEvent(Event(self.PressCheckButton, index))
                            CheckButton.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                            localeInfo.CZ_REWARD_TOOLTIP_COMPLETABLE))
                            CheckButton.SetHideToolTipEvent(Event(self.OverOutToolTipButton))
                            self.InsertChild("%d_%d" % (i, j), CheckButton, "button")
                        else:
                            if localeInfo.IsARABIC():
                                CheckButton = ui.MakeButton(self.CheckButtonWindow, 311 - i * 28, j * 28, "",
                                                            "d:/ymir work/ui/game/12zi/reward/", "check_green.sub",
                                                            "clear_green.sub", "clear_green.sub")
                            else:
                                CheckButton = ui.MakeButton(self.CheckButtonWindow, i * 28, j * 28, "",
                                                            "d:/ymir work/ui/game/12zi/reward/", "check_green.sub",
                                                            "clear_green.sub", "clear_green.sub")

                            index = i / 2 + (j / 2) * 6
                            self.CHECK_BUTTON[1][index] = CheckButton
                            CheckButton.SetEvent(Event(self.PressCheckButton, 30 + index))
                            CheckButton.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                            localeInfo.CZ_REWARD_TOOLTIP_COMPLETABLE))
                            CheckButton.SetHideToolTipEvent(Event(self.OverOutToolTipButton))
                            self.InsertChild("%d_%d" % (i, j), CheckButton,"button")

    def PressButton(self, type):
        if type == 1:
            self.btnYellow.Down()
            self.btnYellow.Disable()
        elif type == 2:
            self.btnGreen.Down()
            self.btnGreen.Disable()
        else:
            self.btnAll.Down()
            self.btnAll.Disable()

        appInst.instance().GetNet().SendChatPacket("/cz_reward %d" % (type))

    def PressCheckButton(self, value):
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
        questionDialog = uiCommon.QuestionDialog()
        questionDialog.SetText(localeInfo.CZ_REWARD_QUESTION_COMPLETE)

        questionDialog.SetAcceptEvent(Event(self.SendClear, value))
        questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
        questionDialog.Open()
        self.questionDialog = questionDialog

    def SendClear(self, value):
        type = 0
        if value >= 30:
            type = 1
            value = value - 30

        appInst.instance().GetNet().SendChatPacket("/cz_check_box %d %d" % (type, value))
        self.OnCloseQuestionDialog()

    def OnCloseQuestionDialog(self):
        if not self.questionDialog:
            return

        self.questionDialog.Close()
        self.questionDialog = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    def Open(self, yellowmark, greenmark, yellowreward, greenreward, goldreward):
        self.__LoadWindow()
        for i in xrange(self.COLUMN_SIZE):
            itemCount = playerInst().GetItemCountByVnum(self.COLUMN_ITEM_LIST[i])
            self.COLUMN_ITEM_COUNT[i].SetText("%d" % itemCount)

            if itemCount >= 50:
                self.COLUMN_ITEM[i] = True
            else:
                self.COLUMN_ITEM[i] = False

        for i in xrange(self.ROW_SIZE):
            itemCount = player.GetItemCountByVnum(self.ROW_ITEM_LIST[i])
            self.ROW_ITEM_COUNT[i].SetText("%d" % itemCount)

            if itemCount >= 50:
                self.ROW_ITEM[i] = True
            else:
                self.ROW_ITEM[i] = False

        self.enableYellow = True
        self.enableGreen = True

        for i in xrange(self.COLUMN_SIZE):
            for j in xrange(self.ROW_SIZE):
                if i % 2 == j % 2:
                    color = i % 2
                    index = i / 2 + (j / 2) * 6
                    if color == 0:
                        if yellowmark & (1 << index):
                            self.CHECK_BUTTON[color][index].SetAlwaysToolTip(False)
                            self.CHECK_BUTTON[color][index].Show()
                            self.CHECK_BUTTON[color][index].Down()
                            self.CHECK_BUTTON[color][index].Disable()
                            continue
                        else:
                            self.enableYellow = False
                    else:
                        if greenmark & (1 << index):
                            self.CHECK_BUTTON[color][index].SetAlwaysToolTip(False)
                            self.CHECK_BUTTON[color][index].Show()
                            self.CHECK_BUTTON[color][index].Down()
                            self.CHECK_BUTTON[color][index].Disable()
                            continue
                        else:
                            self.enableGreen = False

                    count = 0
                    if self.COLUMN_ITEM[i]:
                        count += 1
                    if self.ROW_ITEM[j]:
                        count += 1

                    if count == 2:
                        self.CHECK_BUTTON[color][index].SetAlwaysToolTip(True)
                        self.CHECK_BUTTON[color][index].Show()
                        self.CHECK_BUTTON[color][index].SetUp()
                        self.CHECK_BUTTON[color][index].Enable()
                    else:
                        self.CHECK_BUTTON[color][index].Hide()

        if self.enableYellow:
            self.btnYellow.Show()
            self.btnYellow.SetUp()
            self.btnYellow.Enable()
            self.btnYellow.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                     localeInfo.CZ_REWARD_TOOLTIP_CAN_GET_YELLOW_BOX))
        else:
            if yellowreward > goldreward:
                self.btnYellow.Show()
                self.btnYellow.Down()
                self.btnYellow.Disable()
                self.btnYellow.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                         localeInfo.CZ_REWARD_TOOLTIP_YELLOW_BOX_COMP_CNT.format(
                                                             yellowreward - goldreward)))
            else:
                self.btnYellow.Disable()
                self.btnYellow.Hide()

        if self.enableGreen:
            self.btnGreen.Show()
            self.btnGreen.SetUp()
            self.btnGreen.Enable()
            self.btnGreen.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                    localeInfo.CZ_REWARD_TOOLTIP_CAN_GET_GREEN_BOX))
        else:
            if greenreward > goldreward:
                self.btnGreen.Show()
                self.btnGreen.Down()
                self.btnGreen.Disable()
                self.btnGreen.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                        localeInfo.CZ_REWARD_TOOLTIP_GREEN_BOX_COMP_CNT.format(
                                                            greenreward - goldreward)))
            else:
                self.btnGreen.Disable()
                self.btnGreen.Hide()

        if goldreward < yellowreward and goldreward < greenreward:
            self.btnAll.SetUp()
            self.btnAll.Enable()
            self.btnAll.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                  localeInfo.CZ_REWARD_TOOLTIP_CAN_GET_ALL_BOX))
        else:
            self.btnAll.Down()
            self.btnAll.Disable()
            self.btnAll.SetShowToolTipEvent(Event(self.OverInToolTipButton,
                                                  localeInfo.CZ_REWARD_TOOLTIP_ALL_BOX))

        self.YellowRewardCount.SetNumber("%d" % (yellowreward - goldreward))
        self.GreenRewardCount.SetNumber("%d" % (greenreward - goldreward))

        self.Show()

    def Close(self):
        if self.buttontooltip:
            self.buttontooltip.Hide()
            self.ShowButtonToolTip = False

        self.OnCloseQuestionDialog()
        self.Hide()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def Destroy(self):
        self.Hide()
        self.enableYellow = True
        self.enableGreen = True
        self.CoverImage1 = {}
        self.btnYellow = None
        self.btnGreen = None
        self.btnAll = None
        self.ClearDictionary()
