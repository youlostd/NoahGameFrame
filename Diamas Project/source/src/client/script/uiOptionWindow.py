# -*- coding: utf-8 -*-

from _weakref import proxy

import app
import background
import chat
import player
import snd
from pygame.app import PickupIgnore
from pygame.app import appInst
from pygame.player import playerInst

import constInfo
import localeInfo
import ui
import uiPhaseCurtain
from ui_event import Event


class OptionsWindow(ui.ScriptWindow):
    PAGE_GAME_OPTIONS = 0
    PAGE_VIDEO_OPTIONS = 1
    PAGE_AUDIO_OPTIONS = 2

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.currentSubPage = None
        self.currentPage = self.PAGE_GAME_OPTIONS
        self.lastSubPage = None
        self.currentSubPage = 0
        self.currentPage = 0
        self.blockButtonList = []
        self.videoModes = []
        self.interface = None
        self.requiredClientRestart = False
        self.popup = None

        self.__LoadWindow()

        self.Initialize()

    def __LoadWindow(self):
        ui.PythonScriptLoader().LoadScriptFile(self, "UIScript/optionswindow.py")

    def Initialize(self):

        self.GetChild("OptionsWindowTitleBar").SetCloseEvent(self.Hide)
        self.GetChild("CancelBtn").SetEvent(self.Hide)
        self.GetChild("OKBtn").SetEvent(self.Hide)

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
        self.curtain = uiPhaseCurtain.PhaseCurtain()
        self.curtain.speed = 0.03
        self.curtain.Hide()

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

    def BindInterface(self, interface):
        self.interface = interface

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

        self.fovController = self.GetChild("camera_perspective_controller")
        self.fovController.SetSliderPos(float(appInst.instance().GetSettings().GetFieldPerspective() / 100))
        self.fovController.SetEvent(self.OnChangeFieldPerspective)

        self.viewDistComboBox = self.GetChild("ViewDistComboBox")
        self.cameraMaxDistComboBox = self.GetChild("CameraMaxDistComboBox")

        self.shadowLevelComboBox.SetEvent(self.OnSelectShadowLevel)
        self.viewDistComboBox.SetEvent(self.OnSelectViewDistance)
        self.cameraMaxDistComboBox.SetEvent(self.OnSelectCameraDistance)

        self.shadowLevelComboBox.SelectItem(appInst.instance().GetSettings().GetShadowLevel() - 2)
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

    def SetShowSalesTextFlag(self, val):
        appInst.instance().GetSettings().SetShowSalesTextFlag(val)
        self.GetChild("ShopAdvRangeLabel").SetVisible(val)
        self.GetChild("ShopAdvRangeSliderBar").SetVisible(val)

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
        self.GetChild("ShowShopsTitlesCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowSalesText, self.SetShowSalesTextFlag)

        self.GetChild("GMNotifyCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowGMNotifications, appInst.instance().GetSettings().SetShowGMNotifications)
        self.GetChild("FriendNotifyCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowFriendNotifications, appInst.instance().GetSettings().SetShowFriendNotifications)
        self.GetChild("GuildNotifyCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowGuildNotifications, appInst.instance().GetSettings().SetShowGuildNotifications)

    def SetGameOptionInterfaceEvents(self):
        self.GetChild("ShowChatCheckBox").BindStateFunc(appInst.instance().GetSettings().IsViewChat, appInst.instance().GetSettings().SetViewChatFlag)
        self.GetChild("YangDisplayCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowMoneyLog, appInst.instance().GetSettings().SetShowMoneyLog)
        self.GetChild("MarkNewItemsCheckBox").BindStateFunc(appInst.instance().GetSettings().IsUsingItemHighlight, appInst.instance().GetSettings().SetUsingItemHighlight)
        self.GetChild("BattleDmgInfoCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowDamage, appInst.instance().GetSettings().SetShowDamageFlag)
        self.GetChild("ShowEmojiCheckBox").BindStateFunc(appInst.instance().GetSettings().IsEnableEmojiSystem, appInst.instance().GetSettings().SetEnableEmojiSystem)
        self.GetChild("ItemDisplayWindowCheckBox").BindStateFunc(appInst.instance().GetSettings().IsPickupInfoWindowActive, appInst.instance().GetSettings().SetPickupInfoWindowActive)
        self.GetChild("ItemDisplayCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowItemLog, appInst.instance().GetSettings().SetShowItemLog)

    def SetAdvancedVideoOptionCheckBoxEvents(self):
        self.GetChild("ShowGeneralEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowGeneralEffects, appInst.instance().GetSettings().SetShowGeneralEffects)
        self.GetChild("ShowBuffEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowBuffEffects, appInst.instance().GetSettings().SetShowBuffEffects)
        self.GetChild("ShowSkillEftCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowSkillEffects, appInst.instance().GetSettings().SetShowSkillEffects)
        self.GetChild("DisplayFPSCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowFPS, appInst.instance().GetSettings().SetShowFPS)

        self.GetChild("NightModeCheckBox").BindStateFunc(appInst.instance().GetSettings().IsEnableNightMode, self.__SetNightMode)
        background.SetNightModeOption(appInst.instance().GetSettings().IsEnableNightMode())

        self.GetChild("SnowModeCheckBox").BindStateFunc(appInst.instance().GetSettings().IsEnableSnowFall, self.__SetSnowMode)
        self.GetChild("SnowTextureModeCheckBox").BindStateFunc(appInst.instance().GetSettings().IsEnableSnowTexture, self.__SetSnowTextureMode)
        self.GetChild("CostumeHideModeCheckBox").BindStateFunc(appInst.instance().GetSettings().IsShowCostume, self.__SetCostumeOption)

        self.GetChild("PickupSwordCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Sword),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Sword), True)
        self.GetChild("PickupDaggerCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Dagger),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Dagger), True)
        self.GetChild("PickupBowCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Bow),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Bow), True)
        self.GetChild("PickupTwoHandCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.TwoHand),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.TwoHand), True)
        self.GetChild("PickupBellCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Bell),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Bell), True)
        self.GetChild("PickupFanCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Fan),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Fan), True)
        self.GetChild("PickupArmorBodyCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Armor),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Armor), True)
        self.GetChild("PickupHelmetCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Head),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Head), True)
        self.GetChild("PickupShieldCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Shield),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Shield), True)
        self.GetChild("PickupBraceletCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Wrist),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Wrist), True)
        self.GetChild("PickupShoesCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Foots),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Foots), True)
        self.GetChild("PickupNecklaceCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Necklace),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Necklace), True)
        self.GetChild("PickupEarringsCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Ear),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Ear), True)

        self.GetChild("PickupTalismanCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Talisman),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Talisman), True)

        self.GetChild("PickupRingCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Ring),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Ring), True)

        self.GetChild("PickupElixirCheckBox").BindStateFunc(
            Event(appInst.instance().GetSettings().GetPickupIgnore, PickupIgnore.Elixir),
            Event(appInst.instance().GetSettings().SetPickupIgnore, PickupIgnore.Elixir), True)

    def RefreshOtherCharOptions(self):
        self.GetChild("ShopAdvRangeLabel").SetVisible(self.GetChild("ShowShopsTitlesCheckBox").IsChecked())
        self.GetChild("ShopAdvRangeSliderBar").SetVisible(self.GetChild("ShowShopsTitlesCheckBox").IsChecked())

    def SetMainPage(self, page):
        self.mainPages[self.currentPage]['window'].Hide()
        self.mainPages[page]['window'].Show()
        self.currentPage = page
        self.SetSubPage(0)
        self.mainPages[page]['buttons'].OnClick(0)

    if app.ENABLE_ENVIRONMENT_EFFECT_OPTION:
        def __SetSnowMode(self, index):
            appInst.instance().GetSettings().SetEnableSnowFall(index)
            background.SetSnowModeOption(index)
            background.EnableSnowMode(index)

    if app.ENABLE_ENVIRONMENT_EFFECT_OPTION:
        def __SetSnowTextureMode(self, index):
            appInst.instance().GetSettings().SetEnableeSnowTexture(index)
            background.SetSnowTextureModeOption(index)
            background.EnableSnowTextureMode()

    def __SetCostumeOption(self, val):
        appInst.instance().GetSettings().SetShowCostume(val)
        appInst.instance().GetNet().SendChatPacket("/rewarp_user")

    def __SetNightMode(self, index):
        appInst.instance().GetSettings().SetEnableNightMode(index)
        background.SetNightModeOption(index)

        if not background.IsBoomMap():
            return

        if 1 == index:
            self.curtain.FadeOut(self.__DayMode_OnCompleteChangeToDark)
        else:
            self.curtain.FadeOut(self.__DayMode_OnCompleteChangeToLight)

    def __DayMode_OnCompleteChangeToLight(self):
        background.ChangeEnvironmentData(background.DAY_MODE_LIGHT)
        self.curtain.FadeIn()

    def __DayMode_OnCompleteChangeToDark(self):
        background.RegisterEnvironmentData(background.DAY_MODE_DARK, constInfo.ENVIRONMENT_NIGHT)
        background.ChangeEnvironmentData(background.DAY_MODE_DARK)
        self.curtain.FadeIn()

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
        import uiCommon

        self.popup = uiCommon.PopupDialog()

        if self.requiredClientRestart:
            self.popup.SetText(localeInfo.Get("CLIENT_RESTART_REQUIRED"))  # TODO LOCALE_GAME
            self.popup.SetAcceptEvent(self.OnCloseQuestionDialog)
            self.popup.Open()
        else:
            self.popup.SetText(localeInfo.Get("OPTIONS_SETTINGS_SAVED"))  # TODO LOCALE_GAME
            self.popup.SetAcceptEvent(self.OnCloseQuestionDialog)
            self.popup.Open()

    def OnCloseQuestionDialog(self):
        if not self.popup:
            return

        self.popup = None

    def OnSelectResolution(self, index):
        appInst.instance().GetSettings().SetResolution(index)
        self.requiredClientRestart = True

    def OnSelectDisplayMode(self, index):
        if index == 0:
            appInst.instance().GetSettings().SetWindowed(True)
        else:
            appInst.instance().GetSettings().SetWindowed(False)
        self.requiredClientRestart = True

    def OnSelectShadowLevel(self, index):
        appInst.instance().GetSettings().SetShadowLevel(index + 2)
        background.SetShadowLevel(index + 2)

    def OnSelectViewDistance(self, index):
        appInst.instance().GetSettings().SetViewDistance(index + 1)

    def OnSelectCameraDistance(self, index):
        appInst.instance().GetSettings().SetCameraDistanceMode(index)

    def OnPressExitKey(self):
        self.Hide()
        return True

    def OnChangePKMode(self):
        self.__RefreshPVPButtonList()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True
        return False
    def OnChangeFieldPerspective(self):
        if not self.fovController:
            return

        pos = float(self.fovController.GetSliderPos())
        appInst.instance().GetSettings().SetFieldPerspective(pos * 100)
