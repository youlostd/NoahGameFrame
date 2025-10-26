# -*- coding: utf-8 -*-

import logging

import app
import event
import wndMgr
import _weakref
import localeInfo
import ui
# from grp import GenerateColor
import uiScriptLocale
from ui_event import Event

if app.NEW_SELECT_CHARACTER:
    import snd
    import musicInfo
    from pygame.app import appInst
    # import uiToolTip

LOCALE_PATH = "uiscript/" + uiScriptLocale.CODEPAGE + "_"


class SelectEmpireWindow(ui.ScriptWindow):
    EMPIRE_NAME = {
        EMPIRE_A: localeInfo.EMPIRE_A,
        EMPIRE_B: localeInfo.EMPIRE_B,
        EMPIRE_C: localeInfo.EMPIRE_C
    }

    EMPIRE_DESCRIPTION_TEXT_FILE_NAME = {
        EMPIRE_A: uiScriptLocale.EMPIREDESC_A,
        EMPIRE_B: uiScriptLocale.EMPIREDESC_B,
        EMPIRE_C: uiScriptLocale.EMPIREDESC_C,
    }

    class EmpireButton(ui.Window, ui.BaseEvents):
        def __init__(self, owner, arg):
            ui.Window.__init__(self)
            ui.BaseEvents.__init__(self)

            self.owner = owner
            self.arg = arg
        def OnMouseOverIn(self):
            self.owner.OnOverInEmpire(self.arg)
        def OnMouseOverOut(self):
            self.owner.OnOverOutEmpire(self.arg)
        def OnMouseLeftButtonDown(self):
            if self.owner.empireID != self.arg:
                self.owner.OnSelectEmpire(self.arg)

    class DescriptionBox(ui.Window):
        def __init__(self):
            ui.Window.__init__(self)
            self.SetWindowName(self.__class__.__name__)
            self.descIndex = 0

        def SetIndex(self, index):
            self.descIndex = index

        def OnRender(self):
            event.RenderEventSet(self.descIndex)

    def __init__(self, stream):
        ui.ScriptWindow.__init__(self)
        self.SetWindowName(self.__class__.__name__)
        appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_EMPIRE, self)
        self.SetWindowName("SelectEmpireWindow")

        self.stream = stream
        self.empireID = app.GetRandom(1, 3)
        # self.descriptionBoxes = []
        # self.textBoards = []

        self.descIndex=0
        self.empireArea = {}
        self.empireAreaFlag = {}
        self.empireFlag = {}
        self.empireAreaButton = {}
        self.empireAreaCurAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }
        self.empireAreaDestAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }
        self.empireAreaFlagCurAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }
        self.empireAreaFlagDestAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }
        self.empireFlagCurAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }
        self.empireFlagDestAlpha = { EMPIRE_A:0.0, EMPIRE_B:0.0, EMPIRE_C:0.0 }


    def Close(self):

        self.ClearDictionary()
        # self.descriptionBoxes = []

        if app.NEW_SELECT_CHARACTER:
            if musicInfo.selectMusic != "":
                snd.FadeOutMusic("BGM/" + musicInfo.selectMusic)

            # self.toolTip = None
            # self.ShowToolTip = None

        self.KillFocus()
        self.Hide()
        self.descriptionBox.Hide()
        app.HideCursor()
        event.Destroy()
        appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_EMPIRE, 0)

    def Open(self):
        self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())

        self.descIndices = {
            EMPIRE_A: event.RegisterEventSet(self.EMPIRE_DESCRIPTION_TEXT_FILE_NAME[EMPIRE_A]),
            EMPIRE_B: event.RegisterEventSet(self.EMPIRE_DESCRIPTION_TEXT_FILE_NAME[EMPIRE_B]),
            EMPIRE_C: event.RegisterEventSet(self.EMPIRE_DESCRIPTION_TEXT_FILE_NAME[EMPIRE_C]),
        }

        self.__LoadScript("UIScript/SelectEmpireWindow.py")

        self.OnSelectEmpire(self.empireID)
        self.__CreateButtons()
        self.__CreateDescriptionBoxes()

        for key, val in self.descIndices.iteritems():
            event.SetFontColor(val, 127.0 / 255.0, 149.0 / 255.0, 184.0 / 255.0)
            event.SetRestrictedCount(val, 65)
            event.SetVisibleLineCount(val, 6)

        app.ShowCursor()

        if app.NEW_SELECT_CHARACTER:
            if musicInfo.selectMusic != "":
                snd.SetMusicVolume(appInst.instance().GetSettings().GetMusicVolume())
                snd.FadeInMusic("BGM/" + musicInfo.selectMusic)

                # self.toolTip = uiToolTip.ToolTip()
                # self.toolTip.ClearToolTip()

                # self.ShowToolTip = False


        self.Show()

    def __CreateButtons(self):
        for key, img in self.empireArea.items():

            img.SetAlpha(0.0)

            (x, y) = img.GetGlobalPosition()
            btn = self.EmpireButton(_weakref.proxy(self), key)
            btn.SetParent(self)
            btn.SetPosition(x, y)
            btn.SetSize(img.GetWidth(), img.GetHeight())
            btn.Show()
            self.empireAreaButton[key] = btn


    def __CreateDescriptionBoxes(self):
        self.descriptionBox = self.DescriptionBox()
        self.descriptionBox.SetIndex(0)
        self.descriptionBox.SetParent(self.textBoard)
        self.descriptionBox.Show()

    def OnOverInEmpire(self, arg):
        self.empireAreaDestAlpha[arg] = 1.0

    def OnOverOutEmpire(self, arg):
        if arg != self.empireID:
            self.empireAreaDestAlpha[arg] = 0.0

    def OnSelectEmpire(self, arg):
        for key in self.empireArea.keys():
            self.empireAreaDestAlpha[key] = 0.0
            self.empireAreaFlagDestAlpha[key] = 0.0
            self.empireFlagDestAlpha[key] = 0.0

        self.empireAreaDestAlpha[arg] = 1.0
        self.empireAreaFlagDestAlpha[arg] = 1.0
        self.empireFlagDestAlpha[arg] = 1.0

        self.empireName.SetText(self.EMPIRE_NAME.get(arg, ""))
        if arg == 1:
            self.empireName.SetFontColor(1.0, 0, 0)
        elif arg == 2:
            self.empireName.SetFontColor(1.0, 1.0, 0.0)
        elif arg == 3:
            self.empireName.SetFontColor(0.0, 0, 1.0)

        event.ClearEventSet(self.descIndex)
        if self.EMPIRE_DESCRIPTION_TEXT_FILE_NAME.has_key(arg):
            self.descIndex = event.RegisterEventSet(self.EMPIRE_DESCRIPTION_TEXT_FILE_NAME[arg])
            event.SetRestrictedCount(self.descIndex, 35)

        self.empireID = arg
        snd.PlaySound("sound/ui/click.wav")

    def PrevDescriptionPage(self):
        if event.IsWait(self.descIndex):
            if event.GetVisibleStartLine(self.descIndex)-14 >= 0:
                event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex)-14)
                event.Skip(self.descIndex)
        else:
            event.Skip(self.descIndex)

    def NextDescriptionPage(self):
        if  event.IsWait(self.descIndex):
            event.SetVisibleStartLine(self.descIndex, event.GetVisibleStartLine(self.descIndex)+14)
            event.Skip(self.descIndex)
        else:
            event.Skip(self.descIndex)


    def __LoadScript(self, fileName):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, fileName)
        except:
            logging.exception("Failed to LoadScriptFile {}".format(fileName))

        try:
            GetObject = self.GetChild

            self.leftButton		= GetObject("left_button")
            self.rightButton	= GetObject("right_button")
            self.selectButton	= GetObject("select_button")
            self.exitButton		= GetObject("exit_button")
            self.textBoard		= GetObject("text_board")
            self.empireName		= GetObject("EmpireName")
            self.empireArea[EMPIRE_A]	= GetObject("EmpireArea_A")
            self.empireArea[EMPIRE_B]	= GetObject("EmpireArea_B")
            self.empireArea[EMPIRE_C]	= GetObject("EmpireArea_C")
            self.empireAreaFlag[EMPIRE_A]	= GetObject("EmpireAreaFlag_A")
            self.empireAreaFlag[EMPIRE_B]	= GetObject("EmpireAreaFlag_B")
            self.empireAreaFlag[EMPIRE_C]	= GetObject("EmpireAreaFlag_C")
            self.empireFlag[EMPIRE_A]	= GetObject("EmpireFlag_A")
            self.empireFlag[EMPIRE_B]	= GetObject("EmpireFlag_B")
            self.empireFlag[EMPIRE_C]	= GetObject("EmpireFlag_C")
            GetObject("prev_text_button").SetEvent(self.PrevDescriptionPage)
            GetObject("next_text_button").SetEvent(self.NextDescriptionPage)
        except:
            logging.exception("Failed to bind script objects")

        # self.toolTip = uiToolTip.ToolTip()
        # self.toolTip.ClearToolTip()

        self.selectButton.SetEvent(self.ClickSelectButton)
        self.exitButton.SetEvent(self.ClickExitButton)
        self.leftButton.SetEvent(self.ClickLeftButton)
        self.rightButton.SetEvent(self.ClickRightButton)

        # GetObject("prev_text_button").SetOverEvent(Event(self.OverInButton, 1))
        # GetObject("prev_text_button").SetOverOutEvent(Event(self.OverOutButton, 1))

        # GetObject("next_text_button").SetOverEvent(Event(self.OverInButton, 2))
        # GetObject("next_text_button").SetOverOutEvent(Event(self.OverOutButton, 2))

        # GetObject("left_button").SetOverEvent(Event(self.OverInButton, 3))
        # GetObject("left_button").SetOverOutEvent(Event(self.OverOutButton, 3))

        # GetObject("right_button").SetOverEvent(Event(self.OverInButton, 4))
        # GetObject("right_button").SetOverOutEvent(Event(self.OverOutButton, 4))

        # GetObject("select_button").SetOverEvent(Event(self.OverInButton, 5))
        # GetObject("select_button").SetOverOutEvent(Event(self.OverOutButton, 5))

        # GetObject("exit_button").SetOverEvent(Event(self.OverInButton, 6))
        # GetObject("exit_button").SetOverOutEvent(Event(self.OverOutButton, 6))

        for flag in self.empireAreaFlag.values():
            flag.SetAlpha(0.0)
        for flag in self.empireFlag.values():
            flag.SetAlpha(0.0)

        return 1


    def ClickLeftButton(self):
        self.empireID-=1
        if self.empireID<1:
            self.empireID=3

        self.OnSelectEmpire(self.empireID)

    def ClickRightButton(self):
        self.empireID+=1
        if self.empireID>3:
            self.empireID=1

        self.OnSelectEmpire(self.empireID)

    def ClickSelectButton(self):
        self.stream.SetSelectedEmpire(self.empireID)
        self.stream.SetSelectCharacterPhase()
        self.Hide()
        # self.toolTip.Hide()

    def ClickExitButton(self):
        self.stream.SetLoginPhase()
        self.Hide()
        # self.toolTip.Hide()

    # def OverInButton(self, stat):
    #     if stat == 1:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(uiScriptLocale.CREATE_PREV, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()
    #     elif stat == 2:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(uiScriptLocale.CREATE_NEXT, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()
    #     elif stat == 3:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(localeInfo.UI_PREVPAGE, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()
    #     elif stat == 4:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(localeInfo.UI_NEXT, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()
    #     elif stat == 5:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(localeInfo.EMPIRE_SELECT, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()
    #     elif stat == 6:
    #         self.toolTip.ClearToolTip()
    #         self.toolTip.AppendTextLine(uiScriptLocale.CANCEL, GenerateColor(1.0, 1.0, 0.0, 1.0))
    #         self.toolTip.Show()

    # def OverOutButton(self):
    #     self.toolTip.Hide()


    def OnUpdate(self):
        # for key, idx in self.descIndices.iteritems():
        #     (xposEventSet, yposEventSet) = self.textBoards[key - 1].GetGlobalPosition()
        #     event.UpdateEventSet(idx, xposEventSet + 7, -(yposEventSet + 7))
        #     event.SetFontColor(idx, 203.0 / 255.0, 183.0 / 255.0, 121.0 / 255.0)

            # self.descriptionBoxes[key].SetIndex(idx)
        (xposEventSet, yposEventSet) = self.textBoard.GetGlobalPosition()
        event.UpdateEventSet(self.descIndex, xposEventSet+7, -(yposEventSet+7))
        self.descriptionBox.SetIndex(self.descIndex)

        self.__UpdateAlpha(self.empireArea, self.empireAreaCurAlpha, self.empireAreaDestAlpha)
        self.__UpdateAlpha(self.empireAreaFlag, self.empireAreaFlagCurAlpha, self.empireAreaFlagDestAlpha)
        self.__UpdateAlpha(self.empireFlag, self.empireFlagCurAlpha, self.empireFlagDestAlpha)


    def __UpdateAlpha(self, dict, curAlphaDict, destAlphaDict):
        for key, img in dict.items():

            curAlpha = curAlphaDict[key]
            destAlpha = destAlphaDict[key]

            if abs(destAlpha - curAlpha) / 10 > 0.0001:
                curAlpha += (destAlpha - curAlpha) / 7
            else:
                curAlpha = destAlpha

            curAlphaDict[key] = curAlpha
            img.SetAlpha(curAlpha)


    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.ClickExitButton()
            return True


class ReselectEmpireWindow(SelectEmpireWindow):
    def ClickSelectButton(self):
        self.stream.SetSelectedEmpire(self.empireID)

        # If we selected an existing character, then we need to change its empire.
        if 0 != appInst.instance().GetNet().GetPlayer(self.stream.GetCharacterSlot()).id:
            appInst.instance().GetNet().SendChangeEmpirePacket(self.stream.GetCharacterSlot(),
                                                        self.empireID)
            self.stream.SetSelectCharacterPhase()
        else:
            self.stream.SetCreateCharacterPhase()
        # self.toolTip.Hide()

    def ClickExitButton(self):
        self.stream.SetSelectCharacterPhase()
        # self.toolTip.Hide()

#
# class ReselectEmpireWindow(SelectEmpireWindow):
#     def ClickSelectButton(self):
#         appInst.instance().GetNet().KNSW4ZCTMVWGKY3UIVWXA2LSMVIGCY3LMV2A(self.empireID)
#         self.stream.SetCreateCharacterPhase()
#         if app.NEW_SELECT_CHARACTER:
#             self.Hide()
#
#     def ClickExitButton(self):
#         if app.NEW_SELECT_CHARACTER:
#             self.stream.SetLoginPhase()
#             self.Hide()
#         else:
#             self.stream.SetSelectCharacterPhase()
