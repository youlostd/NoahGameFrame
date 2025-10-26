# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import app
import chat
import event
import item
import player
import snd
from pygame.app import appInst
from pygame.player import playerInst
from wiki import SetModelViewRotationStatus as WikiSetModelViewRotationStatus
from wiki import ShowModelViewManager

import constInfo
import localeInfo
import ui
import uiCommon
import uiScriptLocale
from ui_event import Event


# need to set position before .Open()
class NotificationWindow(ui.Window):
    WIDTH = 122
    HEIGHT = 46

    def __init__(self, title, index):
        ui.Window.__init__(self)

        self.title = title
        self.index = index
        self.openTime = 0

    def PlayOpenSound(self):
        if not constInfo.NEW_QUEST_TIMER_LAST_SOUND:
            constInfo.NEW_QUEST_TIMER_LAST_SOUND = app.GetTime()
            snd.PlaySound("sound/ui/quest_receive.wav")

        if app.GetTime() > constInfo.NEW_QUEST_TIMER_LAST_SOUND + 5.0:
            snd.PlaySound("sound/ui/quest_receive.wav")
            constInfo.NEW_QUEST_TIMER_LAST_SOUND = app.GetTime()

    def LoadWindowAndGUI(self):
        self.SetSize(self.WIDTH, self.HEIGHT)

        index = int(self.index) + 1

        # main background
        self.backgroundImage = ui.ImageBox()
        self.backgroundImage.SetParent(self)
        self.backgroundImage.SetPosition(0, 0)
        self.backgroundImage.LoadImage(
            "d:/ymir work/ui/game/questtimer/notifications/%s.tga" % str(index)
        )
        self.backgroundImage.Show()

        self.questTitle = ui.TextLine()
        self.questTitle.SetParent(self.backgroundImage)
        self.questTitle.SetPosition(51, 10)
        self.questTitle.SetHorizontalAlignLeft()
        self.questTitle.SetText(self.title)
        self.questTitle.Show()

        self.isAvailableText = ui.TextLine()
        self.isAvailableText.SetParent(self.backgroundImage)
        self.isAvailableText.SetPosition(51, 22)
        self.isAvailableText.SetHorizontalAlignLeft()
        self.isAvailableText.SetText(localeInfo.QUEST_TIMER_AVAILABLE_TEXT)
        self.isAvailableText.Show()

    def CanOpen(self):
        return bool(self.openTime == 0)

    def Open(self):
        if not self.CanOpen():
            return

        self.LoadWindowAndGUI()

        if constInfo.NEW_QUEST_TIMER_PLAY_SOUND:
            self.PlayOpenSound()

        self.openTime = app.GetGlobalTime()
        self.Show()

    def Reset(self):
        self.openTime = 0
        self.Close()

    def Close(self):
        self.Hide()

    def OnUpdate(self):
        closeAfterMs = 4000
        timeDiff = app.GetGlobalTime() - self.openTime

        # simple alpha anim (1000)
        if timeDiff >= closeAfterMs - 1000:

            diff = timeDiff - closeAfterMs
            floatDiff = float((float(diff) / 1000.0) * -1)

            # parse
            if floatDiff > 1.0:
                floatDiff = 1.0

            if floatDiff < 0.0:
                floatDiff = 0.0

            self.SetAllAlpha(floatDiff)

        if timeDiff >= closeAfterMs:
            self.Close()

    def Destroy(self):
        self.backgroundImage = None
        self.questTitle = None
        self.isAvailableText = None


class TimerWindow(ui.Window):
    WIDTH = 611
    HEIGHT = 612

    PATH_BASE = "d:/ymir work/ui/game/questtimer/"
    PATH_ICONS = PATH_BASE + "icons/"
    PATH_BANNERS = "d:/ymir work/ui/timer/"
    PATH_SCROLLBAR = PATH_BASE + "scrollbar/"
    PATH_NOTIFICATIONS = PATH_BASE + "notifications/"

    # NAME -> DESCFILE -> MIN. LEVEL -> GROUP REQ. -> COOLDOWN -> ITEM VNUM -> MAX -> ICONFILE.
    # BIOLOG QUEST WILL GET OVERWRITTEN

    QUESTS = [
        ["Biolog", "biolog.txt", "None", False, "0:00", 0, "None", 20084],
        ["Azrael", "azrael.txt", "149", False, "0:00", 76002, "None", 2598],
        ["Lord Bashor", "dragon.txt", "None", False, "0:00", 0, "None", 2493],
        ["Nemere", "snow.txt", "149", False, "0:00", 71219, "None", 6191],
        ["Razador", "flame.txt", "149", False, "0:00", 71174, "None", 6091],
        ["Demon Tower", "dt.txt", "95", True, "0:00", 0, "None", 1093],
        ["Meleys Lair", "meley.txt", "105", True, "0:00", 50283, "115", 6193],
    ]

    BIOLOG = [
        [0, "-"],
        [30220, localeInfo.GetFormattedTimeString(300)],
        [30221, localeInfo.GetFormattedTimeString(600)],
        [30222, localeInfo.GetFormattedTimeString(900)],
        [30223, localeInfo.GetFormattedTimeString(1200)],
        [30224, localeInfo.GetFormattedTimeString(1500)],
        [30225, localeInfo.GetFormattedTimeString(1800)],
        [30226, localeInfo.GetFormattedTimeString(2100)],
        [30227, localeInfo.GetFormattedTimeString(2400)],
    ]

    COOLDOWNS = len(QUESTS) * [0]
    NOTIFICATION_ENABLED = len(QUESTS) * [0]

    BIOLOG_ZERO_FIX = False

    class DescriptionWindow(ui.ScriptWindow):
        LINES_VIEW_COUNT = 17

        class DescriptionBox(ui.Window):
            def __init__(self):
                ui.Window.__init__(self)
                self.descriptionIndex = 0

            def SetIndex(self, index):
                self.descriptionIndex = index

            def OnRender(self):
                event.RenderEventSet(self.descriptionIndex)

        def __init__(self, width, height):
            ui.ScriptWindow.__init__(self)
            self.AddFlag("animated_board")

            self.descriptionIndex = 0
            self.scrollPos = 0

            self.descriptionFile = ""
            self.width = width
            self.height = height

            self.LoadDescriptionWindow()

        def LoadDescriptionWindow(self):
            self.SetSize(self.width, self.height)
            self.SetPosition(0, 0)

            scrollPath = "d:/ymir work/ui/game/questtimer/scrollbar/"

            self.scrollBar = ui.SmallThinScrollBar()
            self.scrollBar.SetParent(self)
            self.scrollBar.SetPosition(self.width - 5, 0)
            self.scrollBar.SetMiddleBarSize(0.5)
            self.scrollBar.SetScrollBarSize(286)
            self.scrollBar.SetScrollEvent(self.OnScroll)
            self.scrollBar.Show()

            self.textBoard = ui.Bar()
            self.textBoard.SetParent(self)
            self.textBoard.SetPosition(4, 0)
            self.textBoard.SetSize(self.width - 12, self.height - 2)
            self.textBoard.Show()

        def Open(self):
            event.ClearEventSet(self.descriptionIndex)

            self.__SetDescriptionEvent()
            self.__CreateDescriptionBox()
            self.scrollBar.SetPos(0.0)

            self.Show()
            ui.ScriptWindow.Show(self)

        def OnScroll(self):
            self.scrollPos = int(
                self.scrollBar.GetPos()
                * max(
                    0,
                    event.GetLineCount(self.descriptionIndex)
                    + 1
                    - self.LINES_VIEW_COUNT,
                )
            )
            event.SetVisibleStartLine(self.descriptionIndex, self.scrollPos)
            event.Skip(self.descriptionIndex)

        def OnMouseWheel(self, len):
            lineCount = event.GetTotalLineCount(self.descriptionIndex)

            if self.IsInPosition() and self.scrollBar.IsShow() and lineCount > 0:

                dir = constInfo.WHEEL_TO_SCROLL(len)

                newPos = self.scrollBar.GetPos() + ((1.0 / lineCount) * dir)
                newPos = max(0.0, newPos)
                newPos = min(1.0, newPos)

                self.scrollBar.SetPos(newPos)
                return True

            return False

        def Close(self):
            event.ClearEventSet(self.descriptionIndex)
            self.descriptionIndex = 0
            self.Hide()

        def __SetDescriptionEvent(self):
            event.ClearEventSet(self.descriptionIndex)
            self.descriptionIndex = event.RegisterEventSet(
                "%s/questtimer%s/%s" % (app.GetLocalePath(), "", self.descriptionFile)
            )
            event.SetEventSetWidth(
                self.descriptionIndex, self.textBoard.GetWidth() - 7 * 2
            )
            event.SetVisibleLineCount(self.descriptionIndex, self.LINES_VIEW_COUNT)

        def __CreateDescriptionBox(self):
            self.descriptionBox = self.DescriptionBox()
            self.descriptionBox.SetParent(self.textBoard)
            self.descriptionBox.Show()

        def OnUpdate(self):
            (xposEventSet, yposEventSet) = self.textBoard.GetGlobalPosition()
            event.UpdateEventSet(
                self.descriptionIndex,
                xposEventSet + 7,
                -(yposEventSet + 7 - (self.scrollPos * 16)),
            )
            self.descriptionBox.SetIndex(self.descriptionIndex)

            linesCount = event.GetTotalLineCount(self.descriptionIndex)

            if linesCount > 0 and linesCount > self.LINES_VIEW_COUNT:
                # self.scrollBar.SetMiddleBarSize(float(self.LINES_VIEW_COUNT) / float(linesCount))
                self.scrollBar.SetMiddleBarSize(0.35)
                self.scrollBar.Show()
            else:
                self.scrollBar.Hide()

        def ChangeDescription(self, fileName):
            self.Close()
            self.descriptionFile = fileName
            self.Open()

        def Destroy(self):
            self.ClearDictionary()
            self.scrollBar = None
            self.descriptionBox = None
            self.textBoard = None

    def __init__(self):
        ui.Window.__init__(self)

        self.itemToolTip = None
        self.questIndex = None
        self.wndDescription = None
        self.itemVnum = None
        self.lastUpdateTime = 0
        self.initialized = False

    def LoadWindowAndGUI(self):
        self.SetSize(self.WIDTH, self.HEIGHT)
        self.SetCenterPosition()
        self.AddFlag("float")
        self.AddFlag("moveable")
        self.AddFlag("animated_board")

        # main background
        self.backgroundImage = ui.Board()
        self.backgroundImage.SetParent(self)
        self.backgroundImage.SetPosition(0, 0)
        self.backgroundImage.SetSize(self.WIDTH, self.HEIGHT)
        self.backgroundImage.Show()

        self.sideInfos = ui.ThinBoardRedBg()
        self.sideInfos.SetParent(self.backgroundImage)
        self.sideInfos.SetSize(280, 565)
        self.sideInfos.SetHorizontalAlignRight()
        self.sideInfos.SetPosition(5, 40)
        self.sideInfos.Show()

        self.mainBG = ui.ThinBoardRedBg()
        self.mainBG.SetParent(self.backgroundImage)
        self.mainBG.SetSize(self.GetWidth() - 10, 565)
        self.mainBG.SetPosition(6, 40)
        self.mainBG.Show()

        # title bar
        self.titleBar = ui.TitleBar()
        self.titleBar.SetParent(self.backgroundImage)
        self.titleBar.MakeTitleBar(0, "red")
        self.titleBar.SetPosition(7, 0)
        self.titleBar.SetWidth(self.WIDTH - 16)
        self.titleBar.btnClose.SetEvent(self.Close)
        self.titleBar.Show()

        # title bar name
        self.titleName = ui.TextLine()
        self.titleName.SetParent(self.titleBar)
        self.titleName.SetPosition(0, 12)
        self.titleName.SetFontName("Verdana:12")
        self.titleName.SetFontColor(115.0 / 255.0, 51.0 / 255.0, 27.0 / 255.0, 1.0)
        self.titleName.SetHorizontalAlignCenter()
        self.titleName.SetText(localeInfo.QUEST_TIMER_INFO)
        self.titleName.Show()

        # banner image
        self.bannerImage = ui.ImageBox()
        self.bannerImage.SetParent(self.backgroundImage)
        self.bannerImage.SetPosition(326, 35)
        self.bannerImage.LoadImage("d:/ymir work/ui/timer/itemslot.png")
        self.bannerImage.Show()

        # box used to center the text inside it
        self.dungeonNameBox = ui.ExpandedImageBox()
        self.dungeonNameBox.SetParent(self.backgroundImage)
        self.dungeonNameBox.SetPosition(326, 102)
        self.dungeonNameBox.SetSize(240, 28)
        self.dungeonNameBox.LoadImage("d:/ymir work/ui/timer/buttom.png")

        self.dungeonNameBox.Show()

        # dungeon name text
        self.dungeonName = ui.TextLine()
        self.dungeonName.SetParent(self.dungeonNameBox)
        self.dungeonName.SetPosition(0, 5)
        self.dungeonName.SetFontName("Verdana:12")
        self.dungeonName.SetFontColor(228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0)

        self.dungeonName.SetHorizontalAlignCenter()
        self.dungeonName.SetText("")
        self.dungeonName.Show()

        # box used to center the text inside it
        self.dungeonDescriptionTitleBox = ui.ExpandedImageBox()
        self.dungeonDescriptionTitleBox.SetParent(self.backgroundImage)
        self.dungeonDescriptionTitleBox.SetPosition(326, 244)
        self.dungeonDescriptionTitleBox.SetSize(251, 28)
        self.dungeonDescriptionTitleBox.LoadImage("d:/ymir work/ui/timer/buttom.png")

        self.dungeonDescriptionTitleBox.Show()

        # dungeon name text
        self.dungeonDescriptionTitle = ui.TextLine()
        self.dungeonDescriptionTitle.SetParent(self.dungeonDescriptionTitleBox)
        self.dungeonDescriptionTitle.SetPosition(0, 5)
        self.dungeonDescriptionTitle.SetFontColor(
            228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0
        )
        self.dungeonDescriptionTitle.SetFontName("Verdana:12")
        self.dungeonDescriptionTitle.SetHorizontalAlignCenter()
        self.dungeonDescriptionTitle.SetText("")
        self.dungeonDescriptionTitle.Show()

        self.dungeonInfo = []

        baseX = 348
        baseY = 131

        # create box/text with level requirement etc
        for i in range(0, 3):
            infoBox = ui.Window()
            infoBox.SetParent(self.backgroundImage)
            infoBox.SetPosition(baseX, baseY + (i * 18))
            infoBox.SetSize(237, 18)
            infoBox.Show()

            infoTitle = ui.TextLine()
            infoTitle.SetParent(infoBox)
            infoTitle.SetFontName("Verdana:12")
            infoTitle.SetFontColor(143.0 / 255.0, 79.0 / 255.0, 76.0 / 255.0, 1.0)

            infoTitle.SetPosition(8, 2)
            infoTitle.SetHorizontalAlignLeft()
            infoTitle.Show()

            infoValue = ui.TextLine()
            infoValue.SetParent(infoBox)
            infoValue.SetPosition(8, 2)
            infoValue.SetFontColor(228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0)

            infoValue.SetFontName("Verdana:12")

            infoValue.SetHorizontalAlignRight()
            infoValue.Show()

            self.dungeonInfo.append([infoBox, infoTitle, infoValue])

        self.dungeonInfo[0][1].SetText(localeInfo.QUEST_TIMER_MINLVL)
        self.dungeonInfo[1][1].SetText(localeInfo.QUEST_TIMER_GROUPREQ)
        self.dungeonInfo[2][1].SetText(localeInfo.QUEST_TIMER_COOLDOWN)

        # passage ticket box
        self.passageTicketBox = ui.SmallDarkThinBoard()
        self.passageTicketBox.SetParent(self.backgroundImage)
        self.passageTicketBox.SetPosition(326, 193)
        self.passageTicketBox.SetSize(285, 46)
        self.passageTicketBox.SetAlpha(0.3)
        self.passageTicketBox.Show()

        self.passageTicketBox.Show()

        # passage ticket text
        self.passageTicketText = ui.TextLine()
        self.passageTicketText.SetParent(self.passageTicketBox)
        self.passageTicketText.SetPosition(10, 15)
        self.passageTicketText.SetFontColor(
            228.0 / 255.0, 176.0 / 255.0, 60.0 / 255.0, 1.0
        )
        self.passageTicketText.SetFontName("Verdana:12")
        self.passageTicketText.SetHorizontalAlignLeft()
        self.passageTicketText.SetText(localeInfo.QUEST_TIMER_PASSAGETICKET)
        self.passageTicketText.Show()

        # default test image
        item.SelectItem(30179)
        image = item.GetIconImageFileName()

        # passage ticket item
        self.passageTicketItem = ui.ExpandedImageBox()
        self.passageTicketItem.SetParent(self.passageTicketBox)
        self.passageTicketItem.SetHorizontalAlignRight()
        self.passageTicketItem.SetPosition(28, 7)
        self.passageTicketItem.SetOnMouseOverInEvent(self.OnPassageTicketHover)
        self.passageTicketItem.SetOnMouseOverOutEvent(self.OnPassageTicketMouseOut)
        self.passageTicketItem.LoadImage(image)
        self.passageTicketItem.Show()

        self.passageTicketItemNone = ui.TextLine()
        self.passageTicketItemNone.SetParent(self.passageTicketBox)
        self.passageTicketItemNone.SetHorizontalAlignCenter()
        self.passageTicketItemNone.SetFontName("Verdana:12")
        self.passageTicketItemNone.SetPosition(30, 15)
        self.passageTicketItemNone.SetText("")
        self.passageTicketItemNone.Show()

        # alarm image
        self.alarmImage = ui.TextLine()
        self.alarmImage.SetParent(self.backgroundImage)
        self.alarmImage.SetPosition(550, 574)
        self.alarmImage.SetFontName("Font Awesome:14")
        self.alarmImage.SetText("\uf0f3".encode("utf-8"))
        self.alarmImage.Show()

        # alarm button
        self.alarmButton = ui.Button()
        self.alarmButton.SetParent(self.backgroundImage)
        self.alarmButton.SetUpVisual("d:/ymir work/ui/checkbox.png")
        self.alarmButton.SetOverVisual("d:/ymir work/ui/checkbox.png")
        self.alarmButton.SetDownVisual("d:/ymir work/ui/game/windows/box_checked.dds")
        self.alarmButton.SetPosition(569, 571)
        self.alarmButton.SetEvent(self.OnAlarmButton)
        self.alarmButton.Show()

        # teleport button
        self.btnTeleport = ui.Button()
        self.btnTeleport.SetParent(self.backgroundImage)
        self.btnTeleport.SetUpVisual("d:/ymir work/ui/timer/btn_normal.png")
        self.btnTeleport.SetOverVisual("d:/ymir work/ui/timer/btn_hover.png")
        self.btnTeleport.SetDownVisual("d:/ymir work/ui/timer/btn_down.png")
        self.btnTeleport.SetPosition(383, 566)
        self.btnTeleport.SetText(uiScriptLocale.EVENT_JOIN_ACCEPT_BUTTON)
        self.btnTeleport.SetFontName("Verdana:13")
        self.btnTeleport.SetEvent(self.OnTeleportButton)
        self.btnTeleport.Show()

        self.wndDescription = self.DescriptionWindow(291, 287)
        self.wndDescription.SetParent(self.backgroundImage)
        self.wndDescription.SetPosition(323, 273)
        # self.wndDescription.ChangeDescription(localeInfo.DESCRIPTION_EXAMPLE)
        self.wndDescription.Open()

        self.questElements = []

        questX = 8
        questY = 2

        self.questListBox = ui.ListBoxEx()
        self.questListBox.SetPosition(questX, questY)
        self.questListBox.SetSize(320, 570)
        self.questListBox.SetItemSize(320, 50)
        self.questListBox.SetItemStep(50)
        self.questListBox.SetViewItemCount(40)
        self.questListBox.Show()

        self.questScrollWindow = ui.ScrollWindow()
        self.questScrollWindow.SetSize(320, 554)
        self.questScrollWindow.SetPosition(0, 42)
        self.questScrollWindow.SetParent(self.backgroundImage)
        self.questScrollWindow.SetContentWindow(self.questListBox)
        self.questScrollWindow.FitWidth(324)
        self.questScrollWindow.Show()

        # create quests
        for i, quest in enumerate(self.QUESTS):
            index = i + 1

            if self.QUESTS[i] == None:
                continue

            background = ui.SmallDarkThinBoard()
            background.SetParent(self)
            background.SetPosition(questX, questY + (i * 50))
            background.SetColor(50.0 / 255.0, 184.0 / 255.0, 204.0 / 255.0)
            background.SetSize(80, self.HEIGHT)
            background.SetMouseLeftButtonDownEvent(Event(self.OnClickQuest, i))
            background.RemoveFlag("not_pick")
            background.Show()

            icon = ui.WikiRenderTarget(42, 47)
            icon.SetParent(background)
            icon.SetPosition(3, 3)
            icon.SetModel(quest[7])
            icon.SetMouseLeftButtonDownEvent(Event(self.OnClickQuest, i))
            icon.Show()

            # WikiSetModelV3Eye(icon.moduleID, 311.4753, -16.3934, -32.7869)
            # WikiSetModelV3Target(icon.moduleID, -1000.0, 49.1803, -16.3934)
            WikiSetModelViewRotationStatus(icon.moduleID, 0)

            name = ui.TextLine()
            name.SetParent(background)
            name.SetPosition(50, 8)
            name.SetHorizontalAlignLeft()
            name.SetText(self.QUESTS[i][0])
            name.Show()

            cooldown = ui.TextLine()
            cooldown.SetParent(background)
            cooldown.SetPosition(50, 23)
            cooldown.SetHorizontalAlignLeft()
            cooldown.SetText("")
            cooldown.Show()

            state = ui.TextLine()
            state.SetParent(background)
            state.SetPosition(14, 15)
            state.SetHorizontalAlignRight()
            state.SetText(localeInfo.QUEST_TIMER_AVAILABLE)
            state.Show()

            self.questElements.append([background, icon, name, cooldown, state])

            self.questListBox.AppendItem(background)

        self.questListBox.SetSize(300, self.questListBox.GetItemCount() * 50)
        # open biolog as default
        self.OnClickQuest(0)
        self.questScrollWindow.ScrollToStart()
        self.backgroundImage.SetTop()
        self.initialized = True

    def TimeToString(self, timeLeft):
        timeLeft = int(timeLeft - app.GetTime())

        if timeLeft > 0:

            hours = int(timeLeft / 3600)
            minutes = int((timeLeft - hours * 3600) / 60)
            seconds = int(timeLeft - hours * 3600 - minutes * 60)

            timeLeftString = ""

            if hours < 10:
                timeLeftString += "0"

            timeLeftString += str(hours) + ":"

            if minutes < 10:
                timeLeftString += "0"

            timeLeftString += str(minutes) + ":"

            if seconds < 10:
                timeLeftString += "0"

            return timeLeftString + str(seconds)

        return "00:00:00"

    def UpdateTimers(self, key, value):
        key = int(key)
        value = float(value)

        if self.COOLDOWNS[key] == 0 and value == 0:
            return

        # fix biolog zero on teleportation :/ thats the simplest way
        if key == 0 and value == 0.0:
            self.COOLDOWNS[0] = 0
            return

        # tchat("UpdateTimers(%s, %s)" % (str(key), str(value)))

        self.COOLDOWNS[key] = app.GetTime() + value

        # tchat(str(self.COOLDOWNS))

    def ChangeAlarmButtonState(self, state):
        # swap visuals -> radio button doesnt work good here...
        visualUp = "d:/ymir work/ui/game/windows/box_uncheck.dds"
        visualDown = "d:/ymir work/ui/game/windows/box_checked.dds"

        if state:
            self.alarmButton.SetUpVisual(visualDown)
            self.alarmButton.SetDownVisual(visualUp)
            self.alarmButton.SetOverVisual(visualDown)
        else:
            self.alarmButton.SetUpVisual(visualUp)
            self.alarmButton.SetDownVisual(visualDown)
            self.alarmButton.SetOverVisual(visualUp)

    def OnAlarmButton(self):
        isEnabled = bool(self.NOTIFICATION_ENABLED[self.questIndex])

        self.ChangeAlarmButtonState(not isEnabled)
        self.NOTIFICATION_ENABLED[self.questIndex] = not isEnabled

    def OnPassageTicketHover(self):
        self.itemToolTip.SetItemToolTip(self.itemVnum)
        self.itemToolTip.ShowToolTip()

    def OnPassageTicketMouseOut(self):
        self.itemToolTip.HideToolTip()

    def SetItemToolTip(self, itemToolTip):
        self.itemToolTip = itemToolTip

    def OnClickQuest(self, id, isBiologRefresh=False, playSound=True):
        if self.questIndex == id and not isBiologRefresh:
            return

        if self.initialized and playSound:
            snd.PlaySound("sound/ui/click.wav")

        self.questIndex = id

        # change images etc.
        self.ChangeBanner(id)
        self.ChangePassageTicket(id)
        self.ChangeDungeonName(id)

        if not isBiologRefresh:
            self.ChangeDescription(id)

        self.ChangeDungeonInfo(id)

        # update button state (checked or not?)
        self.UpdateNotifyButtonState(id)

        if self.IsBiologQuest():
            self.dungeonInfo[0][1].SetText(localeInfo.QUEST_TIMER_CURRENT)
            self.passageTicketText.SetText(localeInfo.QUEST_TIMER_QUESTITEM)
            self.dungeonDescriptionTitle.SetText(localeInfo.QUEST_TIMER_REWARDS)
            self.dungeonInfo[1][1].SetText(localeInfo.QUEST_TIMER_BIOLOGREADY)
            return

        self.dungeonInfo[0][1].SetText(localeInfo.QUEST_TIMER_MINLVL)
        self.passageTicketText.SetText(localeInfo.QUEST_TIMER_PASSAGETICKET)
        self.dungeonDescriptionTitle.SetText(localeInfo.QUEST_TIMER_DUNGEON_DESC)
        self.dungeonInfo[1][1].SetText(localeInfo.QUEST_TIMER_GROUPREQ)
        return True

    def UpdateNotifyButtonState(self, id):
        self.ChangeAlarmButtonState(self.NOTIFICATION_ENABLED[self.questIndex])

    def OnTeleportButton(self):
        # tchat("Teleport to: %d" % self.questIndex)

        # dont teleport players below lvl 75 to grotto
        if self.questIndex == 4 and playerInst().GetPoint(player.LEVEL) < 75:
            chat.AppendChat(1, localeInfo.UIQUEST_TEXT1)
            return

        self.dialog = uiCommon.QuestionDialog()
        self.dialog.index = self.questIndex
        self.dialog.SetText(localeInfo.TIMER_WARP_QUESTION)
        self.dialog.SetAcceptText(localeInfo.YES)
        self.dialog.SetCancelText(localeInfo.NO)
        self.dialog.SetAcceptEvent(lambda arg=True: self.SendWarpCommand(arg))
        self.dialog.SetCancelEvent(lambda arg=False: self.SendWarpCommand(arg))
        self.dialog.Open()

    def GetBiologIndexFromLevel(self, currentLevel):
        biologQuests = [
            [0, 0],
            [30, 1],
            [40, 2],
            [50, 3],
            [60, 4],
            [70, 5],
            [80, 6],
            [85, 7],
            [90, 8],
            [92, 9],
            [94, 10],
            [8, 11],  # 108
            [12, 12],  # 112
        ]

        for quest in biologQuests:
            if quest[0] == currentLevel:
                return quest[1]

        return 0

    def UpdateBiologInfo(self, currentLevel):
        currentLevel = currentLevel.replace("_", "")
        currentLevel = int(currentLevel)
        biologIndex = self.GetBiologIndexFromLevel(currentLevel)

        # tchat("UpdateBiologInfo(currentLevel: %d => %d)" % (currentLevel, biologIndex))

        levelStr = "None"

        if currentLevel != 0:
            levelStr = str(currentLevel)

        self.QUESTS[0][2] = levelStr
        self.QUESTS[0][4] = str(self.BIOLOG[biologIndex][1])
        self.QUESTS[0][5] = int(self.BIOLOG[biologIndex][0])

        if self.IsBiologQuest():
            self.OnClickQuest(0, True, False)

    def SendWarpCommand(self, answer):
        if answer:
            appInst.instance().GetNet().SendChatPacket(
                "/timer_warp %d" % self.dialog.index
            )
            self.Close()

        self.dialog.Hide()
        self.dialog.Close()

    def ChangeDungeonInfo(self, id):
        minLevel = self.QUESTS[id][2]
        maxLevel = self.QUESTS[id][6]
        groupReq = self.QUESTS[id][3]
        cooldown = self.QUESTS[id][4]

        groupReqStr = localeInfo.NO

        if groupReq:
            groupReqStr = localeInfo.YES

        levelStr = minLevel
        if minLevel != maxLevel and id > 0:
            levelStr = "%s - %s" % (minLevel, maxLevel)
        elif minLevel == "None" and id > 0:
            levelStr = "-"

        self.dungeonInfo[0][2].SetText(levelStr)
        self.dungeonInfo[1][2].SetText(groupReqStr)
        self.dungeonInfo[2][2].SetText(cooldown)

    def ChangeBanner(self, id):
        self.bannerImage.LoadImage(self.PATH_BANNERS + "questbio.png")
        # self.bannerImage.LoadImage(self.PATH_BANNERS + "%s.tga" % str(id + 1))

    def ChangePassageTicket(self, id):
        itemVnum = self.QUESTS[id][5]

        if not itemVnum or itemVnum == 0:
            self.passageTicketItem.Hide()
            self.passageTicketItemNone.Show()
            return

        item.SelectItem(itemVnum)
        image = item.GetIconImageFileName()

        self.passageTicketItem.LoadImage(image)
        self.itemVnum = itemVnum
        self.passageTicketItem.Show()
        self.passageTicketItemNone.Hide()

    def ChangeDungeonName(self, id):
        dungeonName = self.QUESTS[id][0]
        self.dungeonName.SetText(dungeonName)

    def ChangeDescription(self, id):
        dungeonDescription = self.QUESTS[id][1]
        self.wndDescription.ChangeDescription(dungeonDescription)

    def Close(self):
        ShowModelViewManager(False)

        self.Hide()

    def Open(self):
        if not appInst.instance().GetNet().IsGamePhase():
            return

        self.LoadWindowAndGUI()

        ShowModelViewManager(True)

        appInst.instance().GetNet().SendChatPacket("/get_timer_cdrs")
        self.SetCenterPosition()
        self.Show()
        self.SetTop()

    # change only if different string so it wont read it every time it gets called
    def ChangeQuestBackground(self, idx, newBackground):
        self.questElements[idx][0].SetColor(*newBackground)

    def OnPressEscapeKey(self):
        self.Close()
        return True

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
        return False

    def IsBiologQuest(self):
        return self.questIndex == 0

    def OnUpdate(self):
        for i, quest in enumerate(self.QUESTS):
            # WTF ???
            # if self.QUESTS[i] == NONE or i not in self.COOLDOWNS:
            #     tchat(str(self.QUESTS[i]))
            #     tchat("SKIP %i [%d, %d]" % (i, self.QUESTS[i] == None, i not in self.COOLDOWNS))
            #     continue
            cooldownTextLine = None
            try:
                cooldownTextLine = self.questElements[i][3]
            except Exception as e:
                logging.exception(e)

            cooldown = self.TimeToString(self.COOLDOWNS[i])
            if cooldownTextLine is not None:
                cooldownTextLine.SetText(cooldown)

            hasMinLevel = quest[2] != "None"
            hasMaxLevel = quest[6] != "None"

            correctMinLevel = True
            correctMaxLevel = True
            correctCooldown = cooldown == "00:00:00"

            if hasMinLevel:
                correctMinLevel = playerInst().GetPoint(player.LEVEL) >= int(quest[2])

            if hasMaxLevel:
                correctMaxLevel = playerInst().GetPoint(player.LEVEL) <= int(quest[6])

            isAvailable = correctCooldown and correctMinLevel and correctMaxLevel

            if isAvailable:
                self.ChangeQuestBackground(i, (20.0 / 255.0, 234.0 / 255.0, 0.0))
                self.questElements[i][4].SetFontColor(0.0, 1.0, 0.1)

                textAvailable = localeInfo.QUEST_TIMER_AVAILABLE

                # fix old label, no need to translate again i think ;d
                if textAvailable.endswith("!"):
                    textAvailable = textAvailable[:-1]

                self.questElements[i][4].SetText(textAvailable)

            else:
                self.ChangeQuestBackground(i, (207.0 / 255.0, 0.0, 15.0 / 255.0))
                self.questElements[i][4].SetFontColor(1.0, 0.1, 0.0)
                if not correctMinLevel or not correctMaxLevel:
                    self.questElements[i][4].SetText(
                        localeInfo.QUEST_TIMER_LOCKED_LEVEL
                    )
                else:
                    self.questElements[i][4].SetText(
                        localeInfo.QUEST_TIMER_LOCKED_COOLDOWN
                    )

        # biolog quest choosen
        if self.IsBiologQuest():

            # quest is ready
            if self.questElements[0][3].GetText() == "00:00:00":
                self.dungeonInfo[1][2].SetText(localeInfo.YES)

            else:
                self.dungeonInfo[1][2].SetText(localeInfo.NO)

    def Destroy(self):
        self.backgroundImage = None
        self.titleBar = None
        self.titleName = None
        self.bannerImage = None
        self.dungeonNameBox = None
        self.dungeonName = None
        self.dungeonDescriptionTitle = None
        self.dungeonInfo = []
        self.passageTicketBox = None
        self.passageTicketText = None
        self.passageTicketItem = None
        self.passageTicketItemNone = None
        self.alarmImage = None
        self.alarmButton = None
        self.btnTeleport = None
        if self.wndDescription:
            self.wndDescription.Destroy()
            self.wndDescription = None
        self.questElements = []
        self.questListBox = None
        self.scrollBarQLB = None
