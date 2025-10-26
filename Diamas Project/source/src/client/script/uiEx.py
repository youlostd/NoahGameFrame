# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
from _weakref import proxy

import app
import background
import chat
import grp
import player
import snd
import wndMgr
from pygame.app import LoadGameData
from pygame.app import LoadLocaleData
from pygame.app import appInst
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
from ui_event import Event


class Window(ui.Window):
    def __init__(self, skinFileName=""):
        ui.Window.__init__(self, "UI")

        self.children = []
        self.childDict = {}
        self.TypeDict = {}

        self.fileName = skinFileName
        self.loader = ui.PythonScriptLoader()
        self.__LoadSkin(skinFileName)

        self.Show()

    def Show(self):
        ui.Window.Show(self)

        for children in self.GetChildrenByType("titlebar"):
            children.SetTop()

    def ClearDictionary(self):
        self.children = []
        self.childDict = {}
        self.TypeDict = {}

    def InsertChild(self, name, child, Type):
        self.childDict[name] = child
        if Type in self.TypeDict:
            self.TypeDict[Type].append(child)
        else:
            self.TypeDict[Type] = [child]

    def IsChild(self, name):
        return name in self.childDict

    def GetChild(self, name):
        return self.childDict[name]

    def GetChildrenByType(self, Type):
        return self.TypeDict.get(Type, [])

    def __LoadSkin(self, fileName):
        self.loader.LoadScriptFile(self, fileName)

    def Reload(self):
        self.ClearDictionary()
        self.__LoadSkin(self.fileName)


# wndMgr.SetOutlineFlag(True)

class App:
    def __init__(self, title="METIN2 UI TEST"):
        app.SetMouseHandler(mouseModule.mouseController)
        app.SetHairColorEnable(True)
        wndMgr.SetMouseHandler(mouseModule.mouseController)

        width = appInst.instance().GetSettings().GetWidth()
        height = appInst.instance().GetSettings().GetHeight()
        wndMgr.SetScreenSize(width, height)
        app.Create(title, width, height, 1)

        app.SetCamera(1500.0, 30.0, 0.0, 180.0)
        app.MyShopDecoBGCreate()

        # Gets and sets the floating-point control word
        # app.SetControlFP()

        if not mouseModule.mouseController.Create():
            return

        if not LoadGameData():
            return

        if not LoadLocaleData(app.GetLocalePath()):
            return
        self.OnInit()

    def MainLoop(self):
        appInst.instance().Loop()

    def OnInit(self):
        pass


class TestWindow(Window):
    PAGE_GAME_OPTIONS = 0
    PAGE_VIDEO_OPTIONS = 1
    PAGE_AUDIO_OPTIONS = 2

    def __init__(self, skinFileName):
        Window.__init__(self, skinFileName)
        self.currentSubPage = None
        self.currentPage = self.PAGE_GAME_OPTIONS
        self.lastSubPage = None

        self.Initialize()

    def Initialize(self):
        self.currentSubPage = 0
        self.currentPage = 0
        self.blockButtonList = []
        self.videoModes = []

        self.mainPages = [
            {
                'window': self.GetChild("Page0_GameOptions"),
                'subwindows': (
                    self.GetChild("SubPage0_Character"),
                    self.GetChild("SubPage0_UserInterface"),
                    self.GetChild("SubPage0_Community"),
                    self.GetChild("SubPage0_PickupFilter"),
                ),
            },
            {
                'window': self.GetChild("Page1_VideoOptions"),
                'subwindows': (
                    self.GetChild("SubPage1_DisplaySettings"),
                    self.GetChild("SubPage1_AdvancedSettings"),
                ),
            },
            {
                'window': self.GetChild("Page2_AudioOptions"),
                'subwindows': (
                    self.GetChild("SubPage2_General"),
                ),
            },

        ]

        for page in self.mainPages:
            page['window'].Hide()
            for subpage in page['subwindows']:
                subpage.Hide()

        self.mainPages[self.PAGE_GAME_OPTIONS]['buttons'] = ui.RadioButtonGroup.CreateSelectDefault([
            [proxy(self.GetChild("LeftBtn0_Character")), Event(self.OnClickChildPageButton, 0), None],
            [proxy(self.GetChild("LeftBtn0_UserInterface")), Event(self.OnClickChildPageButton, 1), None],
            [proxy(self.GetChild("LeftBtn0_Community")), Event(self.OnClickChildPageButton, 2), None],
            [proxy(self.GetChild("LeftBtn0_PickupFilter")), Event(self.OnClickChildPageButton, 3), None],

        ], 0)

        self.mainPages[self.PAGE_VIDEO_OPTIONS]['buttons'] = ui.RadioButtonGroup.CreateSelectDefault([
            [proxy(self.GetChild("LeftBtn1_DisplaySettings")), Event(self.OnClickChildPageButton, 0), None],
            [proxy(self.GetChild("LeftBtn1_AdvancedSettings")), Event(self.OnClickChildPageButton, 1), None],

        ], 0)
        self.mainPages[self.PAGE_AUDIO_OPTIONS]['buttons'] = ui.RadioButtonGroup.CreateSelectDefault([
            [proxy(self.GetChild("LeftBtn2_General")), Event(self.OnClickChildPageButton, 0), None],
        ], 0)

        self.pageButtonGroup = ui.RadioButtonGroup.CreateSelectDefault([
            [proxy(self.GetChild("Tab0")), Event(self.OnClickPageButton, 0), None],
            [proxy(self.GetChild("Tab1")), Event(self.OnClickPageButton, 1), None],
            [proxy(self.GetChild("Tab2")), Event(self.OnClickPageButton, 2), None],
        ], 0)

        self.ctrlMusicVolume = self.GetChild("BGMVolumeSliderBar")
        self.ctrlSoundVolume = self.GetChild("SFXVolumeSliderBar")

        self.ctrlMusicVolumeVal = self.GetChild("BGMVolumeVal")
        self.ctrlSoundVolumeVal = self.GetChild("SFXVolumeVal")

        self.ctrlMusicVolumeVal.SetText(str(int(appInst.instance().GetSettings().GetMusicVolume()*100)))
        self.ctrlSoundVolumeVal.SetText(str(int(appInst.instance().GetSettings().GetSoundVolume()*100)))

        self.ctrlMusicVolume.SetSliderPos(float(appInst.instance().GetSettings().GetMusicVolume()))
        self.ctrlMusicVolume.SetEvent(self.OnChangeMusicVolume)

        self.ctrlSoundVolume.SetSliderPos(float(appInst.instance().GetSettings().GetSoundVolume()))
        self.ctrlSoundVolume.SetEvent(self.OnChangeSoundVolume)

        self.GetChild("ApplyBtn").SetEvent(self.OnApplyBtnClick)
        self.SetGameOptionCharacterEvents()
        self.SetGameOptionInterfaceEvents()
        self.SetAdvancedVideoOptionCheckBoxEvents()
        self.RefreshOtherCharOptions()
        self.SetCommunityOptions()
        self.RefreshBlock()
        self.BuildResolutionList()
        self.BuildAdvancedVideoOptions()

    def BuildResolutionList(self):
        self.videoComboBox = self.GetChild("DisplayResolutionComboBox")
        self.videoComboBox.SetEvent(self.OnSelectResolution)
        resCount = appInst.instance().GetSettings().GetResolutionCount()
        for i in xrange(resCount):
            res = appInst.instance().GetSettings().GetResolution(i)
            self.videoComboBox.InsertItem(i, "{}x{} {}bpp".format(res[0], res[1], res[2]))
            self.videoModes.append(res)

        for i in xrange(resCount):
            res = appInst.instance().GetSettings().GetResolution(i)
            if res[0] == appInst.instance().GetSettings().GetWidth() and res[1] == appInst.instance().GetSettings().GetHeight():
                self.videoComboBox.SelectItem(i)
                break

        self.displayMode = self.GetChild("DisplayModeComboBox")
        self.displayMode.SelectItem(int(not appInst.instance().GetSettings().IsWindowed()))
        self.displayMode.SetEvent(self.OnSelectDisplayMode)

    def BuildAdvancedVideoOptions(self):
        self.shadowLevelComboBox = self.GetChild("ShadowLevelComboBox")
        self.viewDistComboBox = self.GetChild("ViewDistComboBox")
        self.cameraMaxDistComboBox = self.GetChild("CameraMaxDistComboBox")

        self.shadowLevelComboBox.SetEvent(self.OnSelectShadowLevel)
        self.viewDistComboBox.SetEvent(self.OnSelectViewDistance)
        self.cameraMaxDistComboBox.SetEvent(self.OnSelectCameraDistance)

        self.shadowLevelComboBox.SelectItem(appInst.instance().GetSettings().GetShadowLevel())
        self.viewDistComboBox.SelectItem(appInst.instance().GetSettings().GetViewDistance() - 1)
        self.cameraMaxDistComboBox.SelectItem(appInst.instance().GetSettings().GetCameraDistanceMode())

    def SetCommunityOptions(self):
        self.blockButtonList.append((self.GetChild("IgnoreTradeCheckBox"), player.BLOCK_EXCHANGE))
        self.blockButtonList.append((self.GetChild("IgnorePartyInviteCheckBox"), player.BLOCK_PARTY))
        self.blockButtonList.append((self.GetChild("IgnoreGuildInviteCheckBox"), player.BLOCK_GUILD))
        self.blockButtonList.append((self.GetChild("IgnoreWhisperCheckBox"), player.BLOCK_WHISPER))
        self.blockButtonList.append((self.GetChild("IgnoreFriendAddCheckBox"), player.BLOCK_FRIEND))
        self.blockButtonList.append((self.GetChild("IgnorePartyJoinReqCheckBox"), player.BLOCK_PARTY_REQUEST))
        self.blockButtonList.append((self.GetChild("IgnoreDuelRequestCheckBox"), player.BLOCK_DUEL_REQUEST))

        for checkbox, flag in self.blockButtonList:
            checkbox.SetEvent(Event(self.BlockModeCheckBoxEvent, flag))

        self.pvpModeButtonDict = {}

        GetObject = self.GetChild

        self.pvpModeButtonDict[player.PK_MODE_PEACE] = GetObject("PVPPeaceRadioBtn")
        self.pvpModeButtonDict[player.PK_MODE_REVENGE] = GetObject("PVPHostileRadioBtn")
        self.pvpModeButtonDict[player.PK_MODE_GUILD] = GetObject("PVPGuildRadioBtn")
        self.pvpModeButtonDict[player.PK_MODE_FREE] = GetObject("PVPFreeRadioBtn")

        self.pvpModeButtonDict[player.PK_MODE_PEACE].SetEvent(self.__OnClickPvPModePeaceButton)
        self.pvpModeButtonDict[player.PK_MODE_REVENGE].SetEvent(self.__OnClickPvPModeRevengeButton)
        self.pvpModeButtonDict[player.PK_MODE_GUILD].SetEvent(self.__OnClickPvPModeGuildButton)
        self.pvpModeButtonDict[player.PK_MODE_FREE].SetEvent(self.__OnClickPvPModeFreeButton)

    ## PVP MODE BEGIN
    def __CheckPvPProtectedLevelPlayer(self):
        if playerInst().GetPoint(player.LEVEL) < constInfo.PVPMODE_PROTECTED_LEVEL:
            self.__SetPeacePKMode()
            chat.AppendChat(CHAT_TYPE_INFO,
                            localeInfo.OPTION_PVPMODE_PROTECT.format(constInfo.PVPMODE_PROTECTED_LEVEL))
            return 1

        return 0

    def __SetPKMode(self, mode):
        for btn in self.pvpModeButtonDict.values():
            btn.SetUp()
        if mode in self.pvpModeButtonDict:
            self.pvpModeButtonDict[mode].Down()

    def __SetPeacePKMode(self):
        self.__SetPKMode(player.PK_MODE_PEACE)

    def __RefreshPVPButtonList(self):
        self.__SetPKMode(player.GetPKMode())

    if app.ENABLE_MELEY_LAIR_DUNGEON:
        def setMeleyMap(self):
            mapName = background.GetCurrentMapName()
            if mapName == "metin2_map_n_flame_dragon":
                if player.GetGuildID() != 0 and player.GetPKMode() != player.PK_MODE_GUILD:
                    for btn in self.pvpModeButtonDict.values():
                        btn.SetUp()

                    appInst.instance().GetNet().SendChatPacket("/pkmode 4")
                    self.pvpModeButtonDict[player.PK_MODE_GUILD].Down()

        def isMeleyMap(self, button):
            mapName = background.GetCurrentMapName()
            if mapName == "metin2_map_n_flame_dragon":
                if self.pvpModeButtonDict[button]:
                    self.pvpModeButtonDict[button].SetUp()

                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CANNOT_CHANGE_FIGHT_MODE)
                return 1

            return 0

    def __OnClickPvPModePeaceButton(self):
        if app.ENABLE_MELEY_LAIR_DUNGEON:
            if self.isMeleyMap(player.PK_MODE_PEACE):
                return

        if self.__CheckPvPProtectedLevelPlayer():
            return

        self.__RefreshPVPButtonList()

        if constInfo.PVPMODE_ENABLE:
            appInst.instance().GetNet().SendChatPacket("/pkmode 0")
        else:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_NOT_SUPPORT)

    def __OnClickPvPModeRevengeButton(self):
        if app.ENABLE_MELEY_LAIR_DUNGEON:
            if self.isMeleyMap(player.PK_MODE_REVENGE):
                return

        if self.__CheckPvPProtectedLevelPlayer():
            return

        self.__RefreshPVPButtonList()

        if constInfo.PVPMODE_ENABLE:
            appInst.instance().GetNet().SendChatPacket("/pkmode 1")
        else:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_NOT_SUPPORT)

    def __OnClickPvPModeFreeButton(self):
        if app.ENABLE_MELEY_LAIR_DUNGEON:
            if self.isMeleyMap(player.PK_MODE_FREE):
                return

        if self.__CheckPvPProtectedLevelPlayer():
            return

        self.__RefreshPVPButtonList()

        if constInfo.PVPMODE_ENABLE:
            appInst.instance().GetNet().SendChatPacket("/pkmode 2")
        else:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_NOT_SUPPORT)

    def __OnClickPvPModeGuildButton(self):
        if app.ENABLE_MELEY_LAIR_DUNGEON:
            if self.isMeleyMap(player.PK_MODE_GUILD):
                return

        if self.__CheckPvPProtectedLevelPlayer():
            return

        self.__RefreshPVPButtonList()

        if 0 == player.GetGuildID():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_CANNOT_SET_GUILD_MODE)
            return

        if constInfo.PVPMODE_ENABLE:
            appInst.instance().GetNet().SendChatPacket("/pkmode 4")
        else:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_NOT_SUPPORT)

    ## PVP MODE END

    def BlockModeCheckBoxEvent(self, mode, state):
        playerInst().ChangeBlockMode(mode)
        self.RefreshBlock()

    def RefreshBlock(self):
        blockMode = playerInst().GetBlockMode()
        for b, block in self.blockButtonList:
            if 0 != (blockMode & block):
                b.SetState(1)
            else:
                b.SetState(0)

    def SetGameOptionCharacterEvents(self):
        self.GetChild("ShowNPCNameCheckBox").BindStateFunc(appInst.instance().GetSettings().IsAlwaysShowNPCName, appInst.instance().GetSettings().SetAlwaysShowNPCNameFlag)
        self.GetChild("ShowPlayerNameCheckBox").BindStateFunc(appInst.instance().GetSettings().IsAlwaysShowPlayerName, appInst.instance().GetSettings().SetAlwaysShowPlayerNameFlag)
        self.GetChild("ShowMonsterNameCheckBox").BindStateFunc(appInst.instance().GetSettings().IsAlwaysShowMonsterName, appInst.instance().GetSettings().SetAlwaysShowMonsterNameFlag)
        self.GetChild("ShowItemNameCheckBox").BindStateFunc(appInst.instance().GetSettings().IsAlwaysShowItemName, appInst.instance().GetSettings().SetAlwaysShowItemNameFlag)

        self.GetChild("ShowNPCPetsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsHidePets, appInst.instance().GetSettings().SetHidePets, negated=True)
        self.GetChild("ShowNPCNewPetsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsHideGrowthPets, appInst.instance().GetSettings().SetHideGrowthPets, negated=True)
        self.GetChild("ShowNPCNewShopsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsHideShops, appInst.instance().GetSettings().SetHideShops, negated=True)
        self.GetChild("ShowNPCMountsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsHideMounts, appInst.instance().GetSettings().SetHideMounts, negated=True)

        self.GetChild("ShowPlayerSashesCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowAcce, appInst.instance().GetSettings().SetShowAcce)
        self.GetChild("ShowSpecularCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowSpecular, appInst.instance().GetSettings().SetShowSpecular)
        self.GetChild("ShowCountryFlagCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowCountryFlags, appInst.instance().GetSettings().SetShowCountryFlags)
        self.GetChild("ShowMobLevelCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowMobLevel, appInst.instance().GetSettings().SetShowMobLevel)
        self.GetChild("ShowMobAggroCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowMobAIFlag, appInst.instance().GetSettings().SetShowMobAIFlag)
        self.GetChild("ShowShopsTitlesCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowSalesText, appInst.instance().GetSettings().SetShowSalesTextFlag)

    def SetGameOptionInterfaceEvents(self):
        self.GetChild("ShowChatCheckBox").BindStateFunc(appInst.instance().GetSettings().IsViewChat, appInst.instance().GetSettings().SetViewChatFlag)
        self.GetChild("YangDisplayCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowMoneyLog, appInst.instance().GetSettings().SetShowMoneyLog)
        self.GetChild("MarkNewItemsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsUsingItemHighlight, appInst.instance().GetSettings().SetUsingItemHighlight)
        self.GetChild("BattleDmgInfoCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowDamage, appInst.instance().GetSettings().SetShowDamageFlag)
        self.GetChild("ShowEmojiCheckBox").BindStateFunc(appInst.instance().GetSettings().IsEnableEmojiSystem, appInst.instance().GetSettings().SetEnableEmojiSystem)

    def SetAdvancedVideoOptionCheckBoxEvents(self):
        self.GetChild("ShowGeneralEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowGeneralEffects, appInst.instance().GetSettings().SetShowGeneralEffects)
        self.GetChild("ShowBuffEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowBuffEffects, appInst.instance().GetSettings().SetShowBuffEffects)
        self.GetChild("ShowSkillEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowSkillEffects, appInst.instance().GetSettings().SetShowSkillEffects)

    def RefreshOtherCharOptions(self):
        self.GetChild("ShopAdvRangeLabel").SetVisible(self.GetChild("ShowShopsTitlesCheckBox").IsChecked())
        self.GetChild("ShopAdvRangeSliderBar").SetVisible(self.GetChild("ShowShopsTitlesCheckBox").IsChecked())

    def SetMainPage(self, page):
        self.mainPages[self.currentPage]['window'].Hide()
        self.mainPages[page]['window'].Show()
        self.currentPage = page
        self.SetSubPage(0)
        self.mainPages[page]['buttons'].OnClick(0)

    def SetSubPage(self, page):
        if self.lastSubPage is not None:
            try:
                self.lastSubPage.Hide()
            except ReferenceError:
                pass

        self.mainPages[self.currentPage]['subwindows'][page].Show()
        self.currentSubPage = page
        self.lastSubPage = proxy(self.mainPages[self.currentPage]['subwindows'][page])

    def OnClickPageButton(self, page):
        self.SetMainPage(page)

    def OnClickChildPageButton(self, page):
        self.SetSubPage(page)

    def OnChangeMusicVolume(self):
        pos = self.ctrlMusicVolume.GetSliderPos()
        snd.SetMusicVolume(pos * background.GetCurrentMapMusicVolume())
        appInst.instance().GetSettings().SetMusicVolume(pos)
        self.ctrlMusicVolumeVal.SetText(str(int(appInst.instance().GetSettings().GetMusicVolume()*100)))

    def OnChangeSoundVolume(self):
        pos = self.ctrlSoundVolume.GetSliderPos()
        snd.SetSoundVolume(pos)
        appInst.instance().GetSettings().SetSoundVolumef(pos)

        self.ctrlSoundVolumeVal.SetText(str(int(pos*100)))

    def OnApplyBtnClick(self):
        appInst.instance().GetSettings().SaveConfig()

    def OnSelectResolution(self, index):
        appInst.instance().GetSettings().SetResolution(index)

    def OnSelectDisplayMode(self, index):
        if index == 0:
            appInst.instance().GetSettings().SetWindowed(True)
        else:
            appInst.instance().GetSettings().SetWindowed(False)

    def OnSelectShadowLevel(self, index):
        appInst.instance().GetSettings().SetShadowLevel(index)

    def OnSelectViewDistance(self, index):
        appInst.instance().GetSettings().SetViewDistance(index + 1)

    def OnSelectCameraDistance(self, index):
        appInst.instance().GetSettings().SetCameraDistanceMode(index)

    def OnPressExitKey(self):
        appInst.instance().Exit()
        return True

    def OnChangePKMode(self):
        self.__RefreshPVPButtonList()

    def OnRender(self):
        grp.Clear()
        appInst.instance().RenderGame()
        grp.SetOmniLight()

        grp.PopState()
        grp.SetInterfaceRenderState()

    def OnUpdate(self):
        appInst.instance().UpdateGame()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            appInst.instance().Exit()
            return True

        if key == app.VK_F5:
            grp.ClearDepthBuffer()
            grp.Clear()
            reload(ui)
            self.Reload()
            self.Initialize()

            for children in self.GetChildrenByType("titlebar"):
                children.SetTop()


class TestApp(App):
    def OnInit(self):
        self.test = TestWindow("UIScript/optionswindow.py")
        pass


TestApp().MainLoop()
