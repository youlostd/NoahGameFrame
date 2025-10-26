# -*- coding: utf-8 -*-

import app
import chr
import event
import grp
import player
import snd
import wndMgr
from pygame.app import appInst
from pygame.net import EvaluatePartData

import localeInfo
import musicInfo
import playerSettingModule
import ui
import uiScriptLocale
import uiToolTip
from ui_event import Event

ENABLE_ENGNUM_DELETE_CODE = True

M2_INIT_VALUE = -1
CHARACTER_SLOT_COUNT_MAX = 5  # ÃÖ´ë °¡Áú¼ö ÀÖ´Â ÄÉ¸¯ÅÍ ¼ö

JOB_WARRIOR = 0
JOB_ASSASSIN = 1
JOB_SURA = 2
JOB_SHAMAN = 3
if app.ENABLE_WOLFMAN_CHARACTER:
    JOB_WOLFMAN = 4

SELECTED_CHAR = 1


class MyCharacters:
    class MyUnit:

        def __init__(self, const_id, name, level, race, playtime, guildname, form, hair, acce, stat_str, stat_dex,
                     stat_hth, stat_int, change_name, change_empire):
            self.UnitDataDic = {
                "ID": const_id,
                "NAME": name,
                "LEVEL": level,
                "RACE": race,
                "PLAYTIME": playtime,
                "GUILDNAME": guildname,
                "FORM": form,
                "HAIR": hair,
                "ACCE": acce,
                "STR": stat_str,
                "DEX": stat_dex,
                "HTH": stat_hth,
                "INT": stat_int,
                "CHANGENAME": change_name,
                "CHANGEEMPIRE": change_empire,
            }

        def __repr__(self):
            return "{}".format(self.UnitDataDic)

        def GetUnitData(self):
            return self.UnitDataDic

    def __init__(self, stream):
        self.MainStream = stream
        self.PriorityData = []
        self.myUnitDic = {}
        self.HowManyChar = 0
        self.EmptySlot = []
        self.Race = [None, None, None, None, None]
        self.Job = [None, None, None, None, None]
        self.Guild_Name = [None, None, None, None, None]
        self.Play_Time = [0, 0, 0, 0, 0]
        self.Change_Name = [None, None, None, None, None]
        self.Change_Empire = [None, None, None, None, None]
        self.Stat_Point = {0: None, 1: None, 2: None, 3: None, 4: None}
        self.Name = [None, None, None, None, None]
        self.Level = [None, None, None, None, None]

    def Destroy(self):
        self.MainStream = None

        for i in xrange(self.HowManyChar):
            chr.DeleteInstance(i)

        self.PriorityData = None
        self.myUnitDic = None
        self.HowManyChar = None
        self.EmptySlot = None
        self.Race = None
        self.Job = None
        self.Guild_Name = None
        self.Play_Time = None
        self.Change_Name = None
        self.Stat_Point = None

    def LoadCharacterData(self):
        self.RefreshData()
        self.MainStream.All_ButtonInfoHide()
        empire = 0
        for i in xrange(CHARACTER_SLOT_COUNT_MAX):
            # print "LOAD %d" % i
            simplePlayer = appInst.instance().GetNet().GetPlayer(i)
            pid = simplePlayer.id
            if not pid:
                self.EmptySlot.append(i)
                continue

            name = appInst.instance().GetNet().GetPlayerName(i)
            level = simplePlayer.level
            race = simplePlayer.job
            playtime = simplePlayer.playTime
            guildname = appInst.instance().GetNet().GetGuildName(i)
            form = simplePlayer.parts[PART_MAIN]
            hair = simplePlayer.parts[PART_HAIR]
            acce = simplePlayer.parts[PART_ACCE]
            stat_str = simplePlayer.st
            stat_dex = simplePlayer.dx
            stat_hth = simplePlayer.ht
            stat_int = simplePlayer.iq
            last_playtime = long(simplePlayer.lastPlay)
            change_name = simplePlayer.changeName
            change_empire = simplePlayer.changeEmpire
            if simplePlayer.empire:
                empire = simplePlayer.empire

            if last_playtime == 0:
                last_playtime = i

            self.SetPriorityData(last_playtime)
            self.myUnitDic[last_playtime] = self.MyUnit(i, name, level, race, playtime, guildname, form, hair, acce,
                                                        stat_str, stat_dex, stat_hth, stat_int, change_name,
                                                        change_empire)



        self.PriorityData.sort(reverse=True)

        self.MainStream.stream.SetSelectedEmpire(empire)
        prLen = len(self.PriorityData)

        for i in xrange(prLen):
            time = long(self.PriorityData[i])
            DestDataDic = self.myUnitDic[time].GetUnitData()

            self.SetSortingData(i, DestDataDic["NAME"], DestDataDic["LEVEL"], DestDataDic["RACE"],
                                DestDataDic["GUILDNAME"], DestDataDic["PLAYTIME"],
                                DestDataDic["STR"], DestDataDic["DEX"], DestDataDic["HTH"], DestDataDic["INT"],
                                DestDataDic["CHANGENAME"], DestDataDic["CHANGEEMPIRE"])

            self.MakeCharacter(i, DestDataDic["NAME"], DestDataDic["RACE"], DestDataDic["FORM"],
                               DestDataDic["HAIR"], DestDataDic["ACCE"])

            self.MainStream.InitDataSet(i, DestDataDic["NAME"], DestDataDic["LEVEL"], DestDataDic["ID"])

        ## Default Setting ##
        if self.HowManyChar:
            self.MainStream.SelectButton(0)

        return self.HowManyChar

    def SetPriorityData(self, last_playtime):
        self.PriorityData.append(last_playtime)

    def MakeCharacter(self, slot, name, race, form, hair, acce):
        chr.CreateInstance(slot)
        chr.SelectInstance(slot)
        chr.SetVirtualID(slot)
        chr.SetNameString(name)

        chr.SetRace(race)
        chr.SetArmor(EvaluatePartData(form))
        chr.SetHair(EvaluatePartData(hair))
        chr.SetAcce(EvaluatePartData(acce))
        chr.SetWeapon(0)

        chr.SetMotionMode(chr.MOTION_MODE_GENERAL)
        chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)

        if app.ENABLE_WOLFMAN_CHARACTER:  ## ¼öÀÎÁ·¸¸, ½ºÄÉÀÏ Á¶±Ý Ãà¼Ò.
            if chr.RaceToJob(race) == JOB_WOLFMAN:
                chr.SetScale(0.95, 0.95, 0.95)

        chr.SetRotation(0.0)
        chr.Hide()

    def SetSortingData(self, slot, name, level, race, guildname, playtime, pStr, pDex, pHth, pInt, changename,
                       changeempire):
        self.HowManyChar += 1
        self.Race[slot] = race
        self.Job[slot] = chr.RaceToJob(race)
        self.Guild_Name[slot] = guildname
        self.Play_Time[slot] = playtime
        self.Change_Name[slot] = changename
        self.Change_Empire[slot] = changeempire
        self.Stat_Point[slot] = [pHth, pInt, pStr, pDex]
        self.Level[slot] = level
        self.Name[slot] = name

    def GetRace(self, slot):
        return self.Race[slot]

    def GetName(self, slot):
        return self.Name[slot]

    def GetLevel(self, slot):
        return self.Level[slot]

    def GetJob(self, slot):
        return self.Job[slot]

    def GetMyCharacterCount(self):
        return self.HowManyChar

    def GetEmptySlot(self):
        if not len(self.EmptySlot):
            return M2_INIT_VALUE

        # print "GetEmptySlot %s" % self.EmptySlot[0]
        return self.EmptySlot[0]

    def GetStatPoint(self, slot):
        return self.Stat_Point[slot]

    def GetGuildNamePlayTime(self, slot):
        return self.Guild_Name[slot], self.Play_Time[slot]

    def GetChangeName(self, slot):
        return self.Change_Name[slot]

    def GetChangeEmpire(self, slot):
        return self.Change_Empire[slot]

    def SetChangeNameSuccess(self, slot):
        self.Change_Name[slot] = 0

    def SetChangeEmpireSuccess(self, slot):
        self.Change_Empire[slot] = 0

    def RefreshData(self):
        self.HowManyChar = 0
        self.EmptySlot = []
        self.PriorityData = []
        self.Race = [None, None, None, None, None]
        self.Guild_Name = [None, None, None, None, None]
        self.Play_Time = [0, 0, 0, 0, 0]
        self.Change_Name = [None, None, None, None, None]
        self.Change_Empire = [None, None, None, None, None]
        self.Stat_Point = {0: None, 1: None, 2: None, 3: None, 4: None}


class SelectCharacterWindow(ui.Window):
    EMPIRE_NAME = {
        EMPIRE_A: localeInfo.EMPIRE_A,
        EMPIRE_B: localeInfo.EMPIRE_B,
        EMPIRE_C: localeInfo.EMPIRE_C
    }
    EMPIRE_NAME_COLOR = {
        0: (0, 0, 0),
        EMPIRE_A: (0.7450, 0, 0),
        EMPIRE_B: (0.8666, 0.6156, 0.1843),
        EMPIRE_C: (0.2235, 0.2549, 0.7490)
    }
    RACE_FACE_PATH = {
        playerSettingModule.RACE_WARRIOR_M		:	"D:/ymir work/ui/intro/public_intro/face/face_warrior_m_0",
        playerSettingModule.RACE_ASSASSIN_W		:	"D:/ymir work/ui/intro/public_intro/face/face_assassin_w_0",
        playerSettingModule.RACE_SURA_M			:	"D:/ymir work/ui/intro/public_intro/face/face_sura_m_0",
        playerSettingModule.RACE_SHAMAN_W		:	"D:/ymir work/ui/intro/public_intro/face/face_shaman_w_0",
        playerSettingModule.RACE_WARRIOR_W		:	"D:/ymir work/ui/intro/public_intro/face/face_warrior_w_0",
        playerSettingModule.RACE_ASSASSIN_M		:	"D:/ymir work/ui/intro/public_intro/face/face_assassin_m_0",
        playerSettingModule.RACE_SURA_W			:	"D:/ymir work/ui/intro/public_intro/face/face_sura_w_0",
        playerSettingModule.RACE_SHAMAN_M		:	"D:/ymir work/ui/intro/public_intro/face/face_shaman_m_0",
        playerSettingModule.RACE_WOLFMAN_M		:	"D:/ymir work/ui/intro/public_intro/face/face_wolfman_m_0",
    }
    DISC_FACE_PATH = {
        playerSettingModule.RACE_WARRIOR_M: "icon/face/warrior_m.tga",
        playerSettingModule.RACE_ASSASSIN_W: "icon/face/assassin_w.tga",
        playerSettingModule.RACE_SURA_M: "icon/face/sura_m.tga",
        playerSettingModule.RACE_SHAMAN_W: "icon/face/shaman_w.tga",
        playerSettingModule.RACE_WARRIOR_W: "icon/face/warrior_w.tga",
        playerSettingModule.RACE_ASSASSIN_M: "icon/face/assassin_m.tga",
        playerSettingModule.RACE_SURA_W: "icon/face/sura_w.tga",
        playerSettingModule.RACE_SHAMAN_M: "icon/face/shaman_m.tga",
        playerSettingModule.RACE_WOLFMAN_M: "icon/face/wolfman_m.tga",
    }
	##Job Description##
    DESCRIPTION_FILE_NAME =	(
        uiScriptLocale.JOBDESC_WARRIOR_PATH,
        uiScriptLocale.JOBDESC_ASSASSIN_PATH,
        uiScriptLocale.JOBDESC_SURA_PATH,
        uiScriptLocale.JOBDESC_SHAMAN_PATH,
        uiScriptLocale.JOBDESC_WOLFMAN_PATH,
    )
    ##Job List##
    JOB_LIST = {
        0: localeInfo.JOB_WARRIOR,
        1: localeInfo.JOB_ASSASSIN,
        2: localeInfo.JOB_SURA,
        3: localeInfo.JOB_SHAMAN,
    }
    if app.ENABLE_WOLFMAN_CHARACTER:
        JOB_LIST[4] = localeInfo.JOB_WOLFMAN


    class DescriptionBox(ui.Window):
        def __init__(self):
            ui.Window.__init__(self)
            self.SetWindowName(self.__class__.__name__)
            self.descIndex = 0

        def SetIndex(self, index):
            self.descIndex = index

        def OnRender(self):
            event.RenderEventSet(self.descIndex)

    class CharacterRenderer(ui.Window):
        def OnRender(self):
            grp.ClearDepthBuffer()

            grp.SetGameRenderState()
            grp.PushState()
            grp.SetOmniLight()

            screenWidth = wndMgr.GetScreenWidth()
            screenHeight = wndMgr.GetScreenHeight()
            newScreenWidth = float(screenWidth)
            newScreenHeight = float(screenHeight)

            grp.SetViewport(0.0, 0.0, newScreenWidth / screenWidth, newScreenHeight / screenHeight)

            app.SetCenterPosition(0.0, 0.0, 0.0)
            app.SetCamera(1550.0, 15.0, 180.0, 95.0)
            grp.SetPerspective(11.0, newScreenWidth / newScreenHeight, 1000.0, 3000.0)

            (x, y) = app.GetCursorPosition()
            grp.SetCursorPosition(x, y)

            chr.Deform()
            chr.Render()

            grp.RestoreViewport()
            grp.PopState()
            grp.SetInterfaceRenderState()

    def __init__(self, stream):
        ui.Window.__init__(self)
        self.SetWindowName("SelectCharacterWindow")
        appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_SELECT, self)
        self.stream = stream
        self.mycharacters = None
        ##Init Value##
        self.SelectSlot = M2_INIT_VALUE
        self.ShowToolTip = False
        self.select_job = M2_INIT_VALUE
        self.select_race = M2_INIT_VALUE
        self.LEN_STATPOINT = 4
        self.descIndex = 0
        self.statpoint = [0, 0, 0, 0]
        self.curGauge = [0.0, 0.0, 0.0, 0.0]
        self.Name_FontColor_Def	 = grp.GenerateColor(0.7215, 0.7215, 0.7215, 1.0)
        self.Name_FontColor		 = grp.GenerateColor(197.0/255.0, 134.0/255.0, 101.0/255.0, 1.0)
        self.Level_FontColor 	 = grp.GenerateColor(250.0/255.0, 211.0/255.0, 136.0/255.0, 1.0)
        self.Not_SelectMotion = False
        self.MotionStart = False
        self.MotionTime = 0.0
        self.RealSlot = []
        self.Disable = False
        self.SetTop()

    def Open(self):
        # print "##---------------------------------------- NEW INTRO SELECT OPEN"

        dlgBoard = ui.ScriptWindow()
        self.dlgBoard = dlgBoard

        pythonScriptLoader = ui.PythonScriptLoader()  # uiScriptLocale.LOCALE_UISCRIPT_PATH = locale/ymir_ui/
        pythonScriptLoader.LoadScriptFile(dlgBoard, "UIScript/New_SelectCharacterWindow.py")
        getChild = self.dlgBoard.GetChild

        self.NameList = []
        self.NameList.append(getChild("name_warrior"))
        self.NameList.append(getChild("name_assassin"))
        self.NameList.append(getChild("name_sura"))
        self.NameList.append(getChild("name_shaman"))
        # self.NameList.append(getChild("name_wolfman"))

        ##Background##
        self.backGroundDict = {
            EMPIRE_B: "d:/ymir work/ui/intro/empire/background/empire_chunjo.sub",
            EMPIRE_C: "d:/ymir work/ui/intro/empire/background/empire_jinno.sub",
        }
        self.backGround = dlgBoard.GetChild("BackGround")

        ##Empire Flag##
        self.empireName = dlgBoard.GetChild("EmpireName")
        self.flagDict = {
            EMPIRE_A: "d:/ymir work/ui/intro/empire/empireflag_a.sub",
            EMPIRE_B: "d:/ymir work/ui/intro/empire/empireflag_b.sub",
            EMPIRE_C: "d:/ymir work/ui/intro/empire/empireflag_c.sub",
        }
        self.flag = dlgBoard.GetChild("EmpireFlag")

        ##Button List##
        self.btnStart = dlgBoard.GetChild("start_button")
        self.btnCreate = dlgBoard.GetChild("create_button")
        self.btnDelete = dlgBoard.GetChild("delete_button")
        self.btnExit = dlgBoard.GetChild("exit_button")

        ##Face Image##
        self.FaceImage = []
        self.FaceImage.append(dlgBoard.GetChild("CharacterFace_0"))
        self.FaceImage.append(dlgBoard.GetChild("CharacterFace_1"))
        self.FaceImage.append(dlgBoard.GetChild("CharacterFace_2"))
        self.FaceImage.append(dlgBoard.GetChild("CharacterFace_3"))
        self.FaceImage.append(dlgBoard.GetChild("CharacterFace_4"))

        ##Select Character List##
        self.CharacterButtonList = []
        self.CharacterButtonList.append(dlgBoard.GetChild("CharacterSlot_0"))
        self.CharacterButtonList.append(dlgBoard.GetChild("CharacterSlot_1"))
        self.CharacterButtonList.append(dlgBoard.GetChild("CharacterSlot_2"))
        self.CharacterButtonList.append(dlgBoard.GetChild("CharacterSlot_3"))
        self.CharacterButtonList.append(dlgBoard.GetChild("CharacterSlot_4"))

        ##ToolTip : GuildName, PlayTime##
        dlgBoard.GetChild("CharacterSlot_0").SetShowToolTipEvent(Event(self.OverInToolTip, 0))
        dlgBoard.GetChild("CharacterSlot_0").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("CharacterSlot_1").SetShowToolTipEvent(Event(self.OverInToolTip, 1))
        dlgBoard.GetChild("CharacterSlot_1").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("CharacterSlot_2").SetShowToolTipEvent(Event(self.OverInToolTip, 2))
        dlgBoard.GetChild("CharacterSlot_2").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("CharacterSlot_3").SetShowToolTipEvent(Event(self.OverInToolTip, 3))
        dlgBoard.GetChild("CharacterSlot_3").SetHideToolTipEvent(self.OverOutToolTip)
        # dlgBoard.GetChild("CharacterSlot_4").SetShowToolTipEvent(Event(self.OverInToolTip, 4))
        # dlgBoard.GetChild("CharacterSlot_4").SetHideToolTipEvent(self.OverOutToolTip)

        ## ToolTip etc : Create, Delete, Start, Exit, Prev, Next ##
        dlgBoard.GetChild("create_button").SetShowToolTipEvent(
            Event(self.OverInToolTipETC, uiScriptLocale.SELECT_CREATE))
        dlgBoard.GetChild("create_button").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("delete_button").SetShowToolTipEvent(
            Event(self.OverInToolTipETC, uiScriptLocale.SELECT_DELETE))
        dlgBoard.GetChild("delete_button").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("start_button").SetShowToolTipEvent(
            Event(self.OverInToolTipETC, uiScriptLocale.SELECT_SELECT))
        dlgBoard.GetChild("start_button").SetHideToolTipEvent(self.OverOutToolTip)
        dlgBoard.GetChild("exit_button").SetShowToolTipEvent(Event(self.OverInToolTipETC, uiScriptLocale.SELECT_EXIT))
        dlgBoard.GetChild("exit_button").SetHideToolTipEvent(self.OverOutToolTip)


        self.characterBoard = dlgBoard.GetChild("character_board")

        self.statValue = []
        self.statValue.append(getChild("hth_value"))
        self.statValue.append(getChild("int_value"))
        self.statValue.append(getChild("str_value"))
        self.statValue.append(getChild("dex_value"))

        ##Gauge UI##
        self.GaugeList = []
        self.GaugeList.append(dlgBoard.GetChild("hth_gauge"))
        self.GaugeList.append(dlgBoard.GetChild("int_gauge"))
        self.GaugeList.append(dlgBoard.GetChild("str_gauge"))
        self.GaugeList.append(dlgBoard.GetChild("dex_gauge"))
        self.textBoard = getChild("text_board")
        self.btnPrev = getChild("prev_button")
        self.btnNext = getChild("next_button")

        ##DescFace##
        self.discFace = getChild("DiscFace")
        self.raceNameText = getChild("raceName_Text")

        ##MyID##
        # self.descPhaseText = getChild("desc_phase_text")
        self.myID = dlgBoard.GetChild("my_id")
        self.myID.SetText(appInst.instance().GetNet().GetLoginId())

        ##Button Event##
        self.btnStart.SetEvent(self.StartGameButton)
        self.btnCreate.SetEvent(self.CreateCharacterButton)
        self.btnExit.SetEvent(self.ExitButton)
        self.btnDelete.SetEvent(self.InputPrivateCode)

        ##Select MyCharacter##
        self.CharacterButtonList[0].SetEvent(Event(self.SelectButton, 0))
        self.CharacterButtonList[1].SetEvent(Event(self.SelectButton, 1))
        self.CharacterButtonList[2].SetEvent(Event(self.SelectButton, 2))
        self.CharacterButtonList[3].SetEvent(Event(self.SelectButton, 3))
        self.CharacterButtonList[4].SetEvent(Event(self.SelectButton, 4))

        self.FaceImage[0].SetClickEvent(Event(self.EventProgress, "mouse_click", 0))
        self.FaceImage[1].SetClickEvent(Event(self.EventProgress, "mouse_click", 1))
        self.FaceImage[2].SetClickEvent(Event(self.EventProgress, "mouse_click", 2))
        self.FaceImage[3].SetClickEvent(Event(self.EventProgress, "mouse_click", 3))
        self.FaceImage[4].SetClickEvent(Event(self.EventProgress, "mouse_click", 4))

        self.FaceImage[0].SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in", 0))
        self.FaceImage[1].SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in", 1))
        self.FaceImage[2].SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in", 2))
        self.FaceImage[3].SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in", 3))
        self.FaceImage[4].SetOnMouseOverInEvent(Event(self.EventProgress, "mouse_over_in", 4))

        self.FaceImage[0].SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", 0))
        self.FaceImage[1].SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", 1))
        self.FaceImage[2].SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", 2))
        self.FaceImage[3].SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", 3))
        self.FaceImage[4].SetOnMouseOverOutEvent(Event(self.EventProgress, "mouse_over_out", 4))

        self.btnPrev.SetEvent(self.PrevDescriptionPage)
        self.btnNext.SetEvent(self.NextDescriptionPage)

        self.SelectSlot = M2_INIT_VALUE

        if self.mycharacters:
            self.mycharacters.Destroy()
            self.mycharacters = None

        ##MyCharacter CLASS##
        self.mycharacters = MyCharacters(self)
        self.mycharacters.LoadCharacterData()

        self.descriptionBox = self.DescriptionBox()
        self.descriptionBox.SetIndex(0)
        self.descriptionBox.SetParent(self.textBoard)
        self.descriptionBox.Show()

        # if not self.mycharacters.GetMyCharacterCount():
        #     self.stream.SetCharacterSlot(self.mycharacters.GetEmptySlot())
        #     self.SelectEmpireEvent()

        self.dlgBoard = dlgBoard

        ##Tool Tip(Guild Name, PlayTime)##
        self.toolTip = uiToolTip.ToolTip()
        self.toolTip.ClearToolTip()

        self.dlgBoard.Show()
        self.Show()

        app.ShowCursor()
        self.SetFocus()

        if musicInfo.selectMusic != "":
            snd.SetMusicVolume(appInst.instance().GetSettings().GetMusicVolume())
            snd.FadeInMusic("BGM/" + musicInfo.selectMusic)

        ##Character Render##
        self.chrRenderer = self.CharacterRenderer()
        self.chrRenderer.SetParent(self.backGround)
        self.chrRenderer.Show()


    def ClickLeftButton(self):
        global SELECTED_CHAR

        if (SELECTED_CHAR == 1):
            SELECTED_CHAR = self.mycharacters.GetMyCharacterCount()
        else:
            SELECTED_CHAR = SELECTED_CHAR - 1

        self.SelectButton(SELECTED_CHAR - 1)

    def ClickRightButton(self):
        global SELECTED_CHAR

        if (SELECTED_CHAR == self.mycharacters.GetMyCharacterCount()):
            SELECTED_CHAR = 1
        else:
            SELECTED_CHAR = SELECTED_CHAR + 1

        self.SelectButton(SELECTED_CHAR - 1)

    ##Default Setting##
    def EventProgress(self, event_type, slot):
        if self.Disable:
            return

        if "mouse_click" == event_type:
            if slot == self.SelectSlot:
                return

            snd.PlaySound("sound/ui/click.wav")
            self.SelectButton(slot)
        elif "mouse_over_in" == event_type:
            for button in self.CharacterButtonList:
                button.SetUp()

            self.CharacterButtonList[slot].Over()
            self.CharacterButtonList[self.SelectSlot].Down()
            self.OverInToolTip(slot)
        elif "mouse_over_out" == event_type:
            for button in self.CharacterButtonList:
                button.SetUp()

            self.CharacterButtonList[self.SelectSlot].Down()
            self.OverOutToolTip()
        else:
            print("New_introSelect.py ::EventProgress : False")

    def SelectButton(self, slot):
        if slot == self.SelectSlot:
            return
        # print "self.RealSlot = %s" % self.RealSlot
        # slot 0 ~ 4
        if slot >= self.mycharacters.GetMyCharacterCount() or slot == self.SelectSlot:
            # self.CreateCharacterButton()
            return

        if self.Not_SelectMotion or self.MotionTime != 0.0:
            self.CharacterButtonList[slot].SetUp()
            self.CharacterButtonList[slot].Over()
            return

        for button in self.CharacterButtonList:
            button.SetUp()

        self.SelectSlot = slot
        self.CharacterButtonList[self.SelectSlot].Down()
        self.stream.SetCharacterSlot(self.RealSlot[self.SelectSlot])

        self.select_job = self.mycharacters.GetJob(self.SelectSlot)
        self.SetEmpire(appInst.instance().GetNet().GetPlayer(self.RealSlot[self.SelectSlot]).empire)
        ##Job Descirption##
        event.ClearEventSet(self.descIndex)
        self.descIndex = event.RegisterEventSet(self.DESCRIPTION_FILE_NAME[self.select_job])
        event.SetFontColor(self.descIndex, 255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0)
        event.SetRestrictedCount(self.descIndex, 35)

        if event.BOX_VISIBLE_LINE_COUNT >= event.GetTotalLineCount(self.descIndex) :
            self.btnPrev.Hide()
            self.btnNext.Hide()
        else :
            self.btnPrev.Show()
            self.btnNext.Show()
        self.ResetStat()

        ## Setting ##
        for i in xrange(len(self.NameList)):
            if self.select_job == i	:
                self.NameList[i].SetAlpha(1)
            else:
                self.NameList[i].SetAlpha(0)

        ## Face Setting & Font Color Setting ##
        self.select_race = self.mycharacters.GetRace(self.SelectSlot)
        # print "self.mycharacters.GetMyCharacterCount() = %s" % self.mycharacters.GetMyCharacterCount()
        # for i in xrange(self.mycharacters.GetMyCharacterCount()):
        #     if slot == i:
        #         self.FaceImage[slot].LoadImage(self.RACE_FACE_PATH[self.select_race] + "1.sub")
        #         self.FaceImage[slot].SetPosition(8, 0)
        #         self.CharacterButtonList[slot].SetAppendTextColor(0, 0xffffffff)
        #     else:
                # self.FaceImage[i].LoadImage(self.RACE_FACE_PATH[self.mycharacters.GetRace(i)] + "2.sub")
                # self.CharacterButtonList[i].SetAppendTextColor(0, 0xffffffff)
                # self.FaceImage[slot].SetPosition(8, 0)

        ## Desc Face & raceText Setting ##

        self.discFace.LoadImage(self.DISC_FACE_PATH[self.select_race])
        self.raceNameText.SetText(self.JOB_LIST[self.select_job])

        chr.Hide()
        chr.SelectInstance(self.SelectSlot)
        chr.Show()

    def Close(self):
        # print "##---------------------------------------- NEW INTRO SELECT CLOSE"
        self.mycharacters.Destroy()
        del self.mycharacters
        self.mycharacters = None

        self.chrRenderer.Hide()
        chr.Hide()
        self.NameList = None
        self.EMPIRE_NAME = None
        self.EMPIRE_NAME_COLOR = None
        self.RACE_FACE_PATH = None
        self.DISC_FACE_PATH = None
        self.JOB_LIST = None

        ##Default Value##
        self.SelectSlot = None
        self.ShowToolTip = None
        self.LEN_STATPOINT = None
        self.statpoint = None  # []
        self.curGauge = None  # []
        self.Not_SelectMotion = None
        self.MotionStart = None
        self.MotionTime = None
        self.RealSlot = None

        self.select_job = None
        self.select_race = None

        ##Open Func##
        self.dlgBoard = None
        self.backGround = None
        self.backGroundDict = None
        self.empireName = None
        self.flag = None
        self.flagDict = None  # {}
        self.btnStart = None
        self.btnCreate = None
        self.btnDelete = None
        self.btnExit = None
        self.FaceImage = None  # []
        self.CharacterButtonList = None  # []
        self.GaugeList = None  # []
        self.statValue = None
        # self.descPhaseText = None
        self.myID = None
        # self.textBoard = None
        self.descriptionBox.Hide()
        self.toolTip = None
        self.Disable = None

        if musicInfo.selectMusic != "":
            snd.FadeOutMusic("BGM/" + musicInfo.selectMusic)

        self.Hide()
        self.KillFocus()
        app.HideCursor()
        event.Destroy()

        appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_SELECT, 0)

    def SetEmpire(self, empire_id):
        self.empireName.SetText(self.EMPIRE_NAME.get(empire_id, ""))
        rgb = self.EMPIRE_NAME_COLOR[empire_id]
        self.empireName.SetFontColor(rgb[0], rgb[1], rgb[2])
        if empire_id != EMPIRE_A:
            if empire_id in self.flagDict:
                self.flag.LoadImage(self.flagDict[empire_id])
                self.flag.SetScale(0.45, 0.45)
                self.backGround.LoadImage(self.backGroundDict[empire_id])
                self.backGround.SetScale(float(wndMgr.GetScreenWidth()) / 1024.0, float(wndMgr.GetScreenHeight()) / 768.0)

    def CreateCharacterButton(self):
        # ºó ½½·ÔÀÇ ¹øÈ£¸¦ °¡Á®¿Í¼­ ÄÉ¸¯ÅÍ¸¦ ¸¸µç´Ù.
        slotNumber = self.mycharacters.GetEmptySlot()

        if slotNumber == M2_INIT_VALUE:
            self.stream.popupWindow.Close()
            self.stream.popupWindow.Open(localeInfo.CREATE_FULL, 0, localeInfo.UI_OK)
            return

        pid = self.GetCharacterSlotPID(slotNumber)

        if not pid:
            self.stream.SetCharacterSlot(slotNumber)

            if not self.mycharacters.GetMyCharacterCount():
                self.SelectEmpireEvent()
            else:
                self.stream.SetCreateCharacterPhase()
                self.Hide()

    def ExitButton(self):
        self.stream.SetLoginPhase()
        self.Hide()

    def StartGameButton(self):
        if not self.mycharacters.GetMyCharacterCount() or self.MotionTime != 0.0:
            return

        self.DisableWindow()

        IsChangeName = self.mycharacters.GetChangeName(self.SelectSlot)
        if IsChangeName:
            self.OpenChangeNameDialog()
            return

        IsChangeEmpire = self.mycharacters.GetChangeEmpire(self.SelectSlot)
        if IsChangeEmpire:
            self.OpenChangeEmpireDialog()
            return

        # chr.PushOnceMotion(chr.MOTION_INTRO_SELECTED)
        self.MotionStart = True
        self.MotionTime = app.GetTime()

    def SelectEmpireEvent(self):
        self.stream.SetReselectEmpirePhase()
        self.Hide()

    def OnUpdate(self):
        chr.Update()
        self.ToolTipProgress()

        chrSlot = self.stream.GetCharacterSlot()

        if self.MotionStart:
            self.MotionStart = False

            if musicInfo.selectMusic != "":
                snd.FadeLimitOutMusic("BGM/" + musicInfo.selectMusic, appInst.instance().GetSettings().GetMusicVolume() * 0.05)
            appInst.instance().GetNet().ConnectGameServer(chrSlot)
            playTime = appInst.instance().GetNet().GetPlayer(chrSlot).playTime
            player.SetPlayTime(playTime)


        (xposEventSet, yposEventSet) = self.textBoard.GetGlobalPosition()
        event.UpdateEventSet(self.descIndex, xposEventSet+7, -(yposEventSet+7))
        self.descriptionBox.SetIndex(self.descIndex)

        for i in xrange(self.LEN_STATPOINT):
            self.GaugeList[i].SetPercentage(self.curGauge[i], 1.0)

    # def Refresh(self):
    def GetCharacterSlotPID(self, slotIndex):
        return appInst.instance().GetNet().GetPlayer(slotIndex).id

    def All_ButtonInfoHide(self):
        for i in xrange(CHARACTER_SLOT_COUNT_MAX):
            self.CharacterButtonList[i].Hide()
            self.FaceImage[i].Hide()

    def InitDataSet(self, slot, name, level, real_slot):
        width = self.CharacterButtonList[slot].GetWidth()
        height = self.CharacterButtonList[slot].GetHeight()
        self.CharacterButtonList[slot].AppendTextLine(name[:10] , localeInfo.UI_DEF_FONT, self.Name_FontColor_Def	, "right", width - 142, height*3/4 -40)
        self.CharacterButtonList[slot].AppendTextLine("Lv." + str(level), localeInfo.UI_DEF_FONT, self.Level_FontColor		, "left", width - 42, height*3/4 -9)

        self.CharacterButtonList[slot].Show()
        self.FaceImage[slot].LoadImage(self.RACE_FACE_PATH[self.mycharacters.GetRace(slot)] + "2.sub")
        self.FaceImage[slot].Show()
        self.RealSlot.append(real_slot)

    def InputPrivateCode(self):
        if not self.mycharacters.GetMyCharacterCount():
            return

        import uiCommon
        privateInputBoard = uiCommon.InputDialogWithDescription()
        privateInputBoard.SetTitle(localeInfo.INPUT_PRIVATE_CODE_DIALOG_TITLE)
        privateInputBoard.SetAcceptEvent(self.AcceptInputPrivateCode)
        privateInputBoard.SetCancelEvent(self.CancelInputPrivateCode)

        if ENABLE_ENGNUM_DELETE_CODE:
            pass
        else:
            privateInputBoard.SetNumberMode()

        privateInputBoard.SetSecretMode()
        privateInputBoard.SetMaxLength(7)

        privateInputBoard.SetBoardWidth(250)
        privateInputBoard.SetDescription(localeInfo.INPUT_PRIVATE_CODE_DIALOG_DESCRIPTION)
        privateInputBoard.Open()
        self.privateInputBoard = privateInputBoard

        self.DisableWindow()

        if not self.Not_SelectMotion:
            self.Not_SelectMotion = True
            chr.PushOnceMotion(chr.MOTION_INTRO_NOT_SELECTED, 0.1)

    def AcceptInputPrivateCode(self):
        privateCode = self.privateInputBoard.GetText()
        if not privateCode:
            return

        pid = appInst.instance().GetNet().GetPlayer(self.RealSlot[self.SelectSlot]).id

        if not pid:
            self.PopupMessage(localeInfo.SELECT_EMPTY_SLOT)
            return

        appInst.instance().GetNet().SendDestroyCharacterPacket(self.RealSlot[self.SelectSlot], privateCode)
        self.PopupMessage(localeInfo.SELECT_DELEING)

        self.CancelInputPrivateCode()
        return True

    def CancelInputPrivateCode(self):
        self.privateInputBoard = None
        self.Not_SelectMotion = False
        chr.SetLoopMotion(chr.MOTION_INTRO_WAIT)
        self.EnableWindow()
        return True

    def OnDeleteSuccess(self, slot):
        self.PopupMessage(localeInfo.SELECT_DELETED)
        for i in xrange(len(self.RealSlot)):
            chr.DeleteInstance(i)

        self.RealSlot = []
        self.SelectSlot = M2_INIT_VALUE

        for button in self.CharacterButtonList:
            button.ClearText()

        if not self.mycharacters.LoadCharacterData():
            self.stream.popupWindow.Close()
            self.stream.SetCharacterSlot(self.mycharacters.GetEmptySlot())
            self.SelectEmpireEvent()

    def OnDeleteFailure(self):
        self.PopupMessage(localeInfo.SELECT_CAN_NOT_DELETE)

    def EmptyFunc(self):
        pass

    def PopupMessage(self, msg, func=0):
        if not func:
            func = self.EmptyFunc

        self.stream.popupWindow.Close()
        self.stream.popupWindow.Open(msg, func, localeInfo.UI_OK)

    def RefreshStat(self):
        statSummary = 90.0
        self.curGauge = [
            float(self.statpoint[0]) / statSummary,
            float(self.statpoint[1]) / statSummary,
            float(self.statpoint[2]) / statSummary,
            float(self.statpoint[3]) / statSummary,
        ]

        for i in xrange(self.LEN_STATPOINT):
            self.statValue[i].SetText(str(self.statpoint[i]))

    def ResetStat(self):
        myStatPoint = self.mycharacters.GetStatPoint(self.SelectSlot)

        if not myStatPoint:
            return

        for i in xrange(self.LEN_STATPOINT):
            self.statpoint[i] = myStatPoint[i]

        self.RefreshStat()

    ##ToolTip : GuildName, PlayTime##
    def OverInToolTip(self, slot):
        # print "OverInToolTip %d" % slot
        GuildName = localeInfo.GUILD_NAME
        (myGuildName, myPlayTime) = self.mycharacters.GetGuildNamePlayTime(slot)
        # print "OverInToolTip %d %s %d " % (slot, myGuildName, myPlayTime)
        pos_x, pos_y = self.CharacterButtonList[slot].GetGlobalPosition()

        if not myGuildName:
            myGuildName = localeInfo.SELECT_NOT_JOIN_GUILD

        guild_name = GuildName + " : " + myGuildName
        play_time = uiScriptLocale.SELECT_PLAYTIME + " :"
        day = myPlayTime / (60 * 24)
        if day:
            play_time = play_time + " " + str(day) + localeInfo.DAY
        hour = (myPlayTime - (day * 60 * 24)) / 60
        if hour:
            play_time = play_time + " " + str(hour) + localeInfo.HOUR
        min = myPlayTime - (hour * 60) - (day * 60 * 24)

        play_time = play_time + " " + str(min) + localeInfo.MINUTE

        textlen = max(len(guild_name), len(play_time))
        tooltip_width = 6 * textlen + 22

        self.toolTip.ClearToolTip()
        self.toolTip.SetThinBoardSize(tooltip_width)
        self.toolTip.AppendSpace(10)
        if localeInfo.IsARABIC():
            self.toolTip.SetToolTipPosition(pos_x - 23 - tooltip_width / 2, pos_y + 34)
            self.toolTip.AppendTextLine(guild_name, 0xffe4cb1b, centerAlign=True)  ##YELLOW##
            self.toolTip.AppendTextLine(play_time, 0xffffff00, centerAlign=True)  ##YELLOW##
        else:
            self.toolTip.SetToolTipPosition(pos_x + 173 + tooltip_width / 2, pos_y + 34)
            self.toolTip.AppendTextLine(guild_name, 0xffe4cb1b, centerAlign=True)  ##YELLOW##
            self.toolTip.AppendTextLine(play_time, 0xffffff00, centerAlign=True)  ##YELLOW##
        self.toolTip.AppendSpace(20)
        self.toolTip.Show()

    def OverInToolTipETC(self, arg):
        arglen = len(str(arg))
        pos_x, pos_y = wndMgr.GetMousePosition()

        self.toolTip.ClearToolTip()
        self.toolTip.SetThinBoardSize(11 * arglen)
        self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
        self.toolTip.AppendTextLine(arg, 0xffffff00)
        self.toolTip.Show()
        self.ShowToolTip = True

    def OverOutToolTip(self):
        self.toolTip.Hide()
        self.ShowToolTip = False

    def ToolTipProgress(self):
        if self.ShowToolTip:
            pos_x, pos_y = wndMgr.GetMousePosition()
            self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)

    def SameLoginDisconnect(self):
        self.stream.popupWindow.Close()
        self.stream.popupWindow.Open(localeInfo.LOGIN_FAILURE_SAMELOGIN, self.ExitButton, localeInfo.UI_OK)

    def OnKeyDown(self, key):
        if self.MotionTime != 0.0:
            return

        if app.VK_ESCAPE == key:  # ESC
            self.ExitButton()
        elif app.VK_1 == key:  # 1
            self.SelectButton(0)
        elif app.VK_2 == key:
            self.SelectButton(1)
        elif app.VK_1 == key:
            self.SelectButton(2)
        elif app.VK_4 == key:
            self.SelectButton(3)
        elif app.VK_5 == key:
            self.SelectButton(4)
        elif app.VK_F1 == key:
            self.SelectButton(0)
            self.StartGameButton()
        elif app.VK_F2 == key:
            self.SelectButton(1)
            self.StartGameButton()
        elif app.VK_F3 == key:
            self.SelectButton(2)
            self.StartGameButton()
        elif app.VK_F4 == key:
            self.SelectButton(3)
            self.StartGameButton()
        elif app.VK_F5 == key:
            self.SelectButton(4)
            self.StartGameButton()
        elif app.VK_RETURN == key:
            self.StartGameButton()
        elif app.VK_LEFT == key or app.VK_RIGHT == key:
            self.KeyInputUpDown(key)
        else:
            return True

        return True

    def KeyInputUpDown(self, key):
        idx = self.SelectSlot
        maxValue = self.mycharacters.GetMyCharacterCount()
        if 200 == key:  # UP
            idx = idx - 1
            if idx < 0:
                idx = maxValue - 1

        elif 208 == key:  # DOWN
            idx = idx + 1
            if idx >= maxValue:
                idx = 0
        else:  # ÀÌ·± °æ¿ì ¾ø°ÚÁö¸¸, È¤½Ã ¸ô¶ó ¿¹¿Ü Ã³¸®
            self.SelectButton(0)

        self.SelectButton(idx)

    def OnPressExitKey(self):
        self.ExitButton()
        return True

    def DisableWindow(self):
        self.btnStart.Disable()
        self.btnCreate.Disable()
        self.btnExit.Disable()
        self.btnDelete.Disable()

        self.toolTip.Hide()
        self.ShowToolTip = False
        self.Disable = True
        for button in self.CharacterButtonList:
            button.Disable()

    def EnableWindow(self):
        self.btnStart.Enable()
        self.btnCreate.Enable()
        self.btnExit.Enable()
        self.btnDelete.Enable()

        self.Disable = False
        for button in self.CharacterButtonList:
            button.Enable()

    def OpenChangeNameDialog(self):
        import uiCommon
        nameInputBoard = uiCommon.InputDialogWithDescription()
        nameInputBoard.SetTitle(localeInfo.SELECT_CHANGE_NAME_TITLE)
        nameInputBoard.SetAcceptEvent(self.AcceptInputName)
        nameInputBoard.SetCancelEvent(self.CancelInputName)
        nameInputBoard.SetMaxLength(chr.PLAYER_NAME_MAX_LEN)
        nameInputBoard.SetBoardWidth(200)
        nameInputBoard.SetDescription(localeInfo.SELECT_INPUT_CHANGING_NAME)
        nameInputBoard.Open()
        nameInputBoard.slot = self.RealSlot[self.SelectSlot]
        self.nameInputBoard = nameInputBoard

    def AcceptInputName(self):
        changeName = self.nameInputBoard.GetText()
        if not changeName:
            return

        appInst.instance().GetNet().SendChangeNamePacket(self.nameInputBoard.slot, changeName)
        return self.CancelInputName()

    def CancelInputName(self):
        self.nameInputBoard.Close()
        self.nameInputBoard = None
        self.EnableWindow()
        return True

    def OpenChangeEmpireDialog(self):
        self.stream.SetCharacterSlot(self.RealSlot[self.SelectSlot])
        self.SelectEmpireEvent()

    def OnCreateFailure(self, type):
        if 0 == type:
            self.PopupMessage(localeInfo.SELECT_CHANGE_FAILURE_STRANGE_NAME)
        elif 1 == type:
            self.PopupMessage(localeInfo.SELECT_CHANGE_FAILURE_ALREADY_EXIST_NAME)
        elif 100 == type:
            self.PopupMessage(localeInfo.SELECT_CHANGE_FAILURE_STRANGE_INDEX)

    ##Job Description Prev & Next Button##
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



    def OnChangeName(self, slot, name):
        for i in xrange(len(self.RealSlot)):
            if self.RealSlot[i] == slot:
                self.ChangeNameButton(i, name)
                self.SelectButton(i)
                self.PopupMessage(localeInfo.SELECT_CHANGED_NAME)
                break

    def ChangeNameButton(self, slot, name):
        self.CharacterButtonList[slot].SetAppendTextChangeText(0, name)
        self.mycharacters.SetChangeNameSuccess(slot)
