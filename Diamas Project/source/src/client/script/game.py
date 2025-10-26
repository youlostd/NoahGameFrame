# -*- coding: utf-8 -*-

import logging
import math
import os
import weakref

import app
import background
import chat
import chr
import event
import exchange
import grp
import guild
import item
import messenger
import player
import quest
import snd
import textTail
import wndMgr

import consoleModule
import constInfo
import emoji_config
import interfaceModule
import localeInfo
import mouseModule
import musicInfo
import stringCommander
import ui
import uiAffectShower
import uiCommon
import uiMapNameShower
import uiPhaseCurtain
import uiPlayerGauge
import uiPrivateShopBuilder
import uiTarget

if os.path.isfile("./camy_enable"):
	import camModule

kScreenshotDirectory = "screenshot"

from pygame.player import playerInst
from pygame.app import appInst
from pygame.net import GetPreservedServerCommand
from pygame.item import MakeItemPosition
from ui_event import MakeEvent, Event, MakeCallback
from uiofflineshop import OfflineShopManager
import uiInfoChat
from pygame.item import itemManager
from uiTaskBar import SaveMouseButtonSettings
import uiMiniGameRoulette
import chrmgr
import playerSettingModule
import dungeonInfo

cameraDistance = 1550.0
cameraPitch = 27.0
cameraRotation = 0.0
cameraHeight = 100.0

testAlignment = 0


class Singleton:
	"""
	A non-thread-safe helper class to ease implementing singletons.
	This should be used as a decorator -- not a metaclass -- to the
	class that should be a singleton.

	The decorated class can define one `__init__` function that
	takes only the `self` argument. Also, the decorated class cannot be
	inherited from. Other than that, there are no restrictions that apply
	to the decorated class.

	To get the singleton instance, use the `instance` method. Trying
	to use `__call__` will result in a `TypeError` being raised.

	"""

	def __init__(self, decorated):
		self._decorated = decorated

	def instance(self, *args, **kwargs):
		"""
		Returns the singleton instance. Upon its first call, it creates a
		new instance of the decorated class and calls its `__init__` method.
		On all subsequent calls, the already created instance is returned.

		"""
		try:
			return self._instance
		except AttributeError:
			self._instance = self._decorated(*args, **kwargs)
			return self._instance

	def __call__(self):
		raise TypeError("Singletons must be accessed through `instance()`.")

	def __instancecheck__(self, inst):
		return isinstance(inst, self._decorated)


@Singleton
class GameWindow(ui.ScriptWindow):
	def __init__(self, stream):
		ui.ScriptWindow.__init__(self, "GAME")
		self.SetWindowName(self.__class__.__name__)

		appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_GAME, self)
		playerInst().SetGameWindow(self)
		OfflineShopManager.GetInstance().Initialize(self)

		app.MyShopDecoBGCreate()

		self.curtain = uiPhaseCurtain.PhaseCurtain()
		self.curtain.speed = 0.03

		self.stream = stream
		self.interface = interfaceModule.Interface(self.stream)
		self.interface.MakeInterface()
		self.interface.ShowDefaultWindows()

		self.quickSlotPageIndex = 0
		self.lastPKModeSendedTime = 0

		self.pressNumber = None
		self.guildWarQuestionDialog = None
		self.targetBoard = None
		self.console = None

		self.guildNameBoard = None
		self.itemDestroyQuestionDialog = None
		self.partyInviteQuestionDialog = None
		self.cubeInformation = {}

		self.confirmDialog = None

		self.PrintCoord = None
		self.FrameRate = None
		self.Pitch = None
		self.Splat = None
		self.TextureNum = None
		self.ObjectNum = None
		self.ViewDistance = None
		self.PrintMousePos = None

		self.PickingCharacterIndex = -1
		self.PickingItemIndex = -1
		self.consoleEnable = False
		self.isShowDebugInfo = False
		self.ShowNameFlag = False
		self.wndRoulette = None

		self.enableXMasBoom = False
		self.startTimeXMasBoom = 0.0
		self.indexXMasBoom = 0

		self.partyRequestQuestionDialog = None
		self.messengerAddFriendQuestion = None
		self.guildInviteQuestionDialog = None
		self.infoChat = None

		self.CreateTargetBoard()

		OfflineShopManager.GetInstance().SetInterface(self.interface)

		self.console = consoleModule.ConsoleWindow()
		self.console.BindGameClass(self)
		self.console.SetConsoleSize(wndMgr.GetScreenWidth(), 200)
		self.console.Hide()

		self.mapNameShower = uiMapNameShower.MapNameShower()
		self.affectShower = uiAffectShower.AffectShower()
		self.affectShower.SetPosition(10, 20)
		OfflineShopManager.GetInstance().SetAffectShowerInstance(self.affectShower)

		self.playerGauge = uiPlayerGauge.PlayerGauge(self)
		self.playerGauge.MakeGauge(115, "red")

		self.playerGauge.Hide()

		self.__SetQuickSlotMode()

		self.__ServerCommand_Build()
		self.__ProcessPreservedServerCommand()
		self.escFocusCheck = False

		self.wndCubeConfirmDialog = None
		self.isFreeCamera = False
		self.freeCam = {
			"speed": 100,
			"accelerate": 0,
			"spVariation": 0,
			"nextSpeedup": 0,
			"inertia": 0,
		}

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.interface.PetInfoBindAffectShower(self.affectShower)

		if os.path.isfile("./camy_enable"):
			self.camy = camModule.Interface(self)

		self.AllianceTargetBoard = uiTarget.AllianceTargetBoard()
		self.AllianceTargetBoard.Hide()
		# self.interface.SetAllianceTargetBoard(self.AllianceTargetBoard)
		self.__LoadNamePrefix()
		self.__LoadShopDeco()
		self.__RegisterTitleName()
		self.curtain.Hide()

	def CreateTargetBoard(self):
		if self.targetBoard:
			del self.targetBoard

		self.targetBoard = uiTarget.TargetBoard()
		self.targetBoard.Hide()

		if self.interface and self.interface.whisper:
			self.targetBoard.SetWhisperEvent(self.interface.whisper.ShowDialog)

	def Open(self):

		self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())

		if self.interface:
			self.__BuildKeyDict()
			self.interface.Open()
			self.CreateTargetBoard()
			self.interface.ClearBattlePass()
			if OfflineShopManager.GetInstance():
				OfflineShopManager.GetInstance().CleanUp()

		self.quickSlotPageIndex = 0
		self.PickingCharacterIndex = -1
		self.PickingItemIndex = -1
		self.consoleEnable = False
		self.isShowDebugInfo = False
		self.ShowNameFlag = False

		self.enableXMasBoom = False
		self.startTimeXMasBoom = 0.0
		self.indexXMasBoom = 0

		global cameraDistance, cameraPitch, cameraRotation, cameraHeight

		app.SetCamera(cameraDistance, cameraPitch, cameraRotation, cameraHeight)

		appInst.instance().GetSettings().SetCameraDistanceMode(
			appInst.instance().GetSettings().GetCameraDistanceMode()
		)
		constInfo.SET_DEFAULT_CHRNAME_COLOR()
		constInfo.SET_DEFAULT_FOG_LEVEL()
		constInfo.SET_DEFAULT_USE_ITEM_WEAPON_TABLE_ATTACK_BONUS()
		constInfo.SET_DEFAULT_USE_SKILL_EFFECT_ENABLE()

		# TWO_HANDED_WEAPON_ATTACK_SPEED_UP
		constInfo.SET_TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE()
		# END_OF_TWO_HANDED_WEAPON_ATTACK_SPEED_UP

		event.SetLeftTimeString(localeInfo.UI_LEFT_TIME)

		textTail.EnablePKTitle(constInfo.PVPMODE_ENABLE)

		self.__BuildDebugInfo()

		if self.infoChat:
			self.infoChat.Hide()
			self.infoChat.Destroy()
			del self.infoChat

		self.infoChat = uiInfoChat.InfoChat(
			self, wndMgr.GetScreenWidth() - 200 - 40, wndMgr.GetScreenHeight() / 2
		)

		uiPrivateShopBuilder.Clear()
		exchange.InitTrading()

		snd.SetMusicVolume(
			appInst.instance().GetSettings().GetMusicVolume()
			* background.GetCurrentMapMusicVolume()
		)
		snd.SetSoundVolume(appInst.instance().GetSettings().GetSoundVolume())

		netFieldMusicFileName = background.GetCurrentMapMusic()

		if netFieldMusicFileName:
			snd.FadeInMusic("BGM/" + netFieldMusicFileName)
		elif musicInfo.fieldMusic != "":
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

		self.__SetQuickSlotMode()
		self.__SelectQuickPage(self.quickSlotPageIndex)

		# UPDATE NATION WAR
		maps = (
			"metin2_map_sungzi",
			"metin2_map_sungzi_snow",
			"metin2_map_sungzi_desert_01",
		)
		if background.GetCurrentMapName() in maps:
			self.interface.ToggleNationWindow()
		# UPDATE NATION WAR
		appInst.instance().GetNet().SendEnterGame()
		self.curtain.SetAlpha(1.0)
		self.curtain.speed = 0.01
		self.SetFocus()
		self.Show()

		app.ShowCursor()

		self.StartGame()

		self.curtain.FadeIn()
		# NPC°¡ Å¥ºê½Ã½ºÅÛÀ¸·Î ¸¸µé ¼ö ÀÖ´Â ¾ÆÀÌÅÛµéÀÇ ¸ñ·ÏÀ» Ä³½Ì

		self.cubeInformation = {}

		self.interface.ShowDefaultWindows()
		self.interface.MakeOnOpen()

		if self.affectShower:
			self.affectShower.SetDropState(appInst.instance().GetNet().GetMultiState())

		mouseModule.mouseController.CreateNumberLine()

	def Close(self):
		self.Hide()

		global cameraDistance, cameraPitch, cameraRotation, cameraHeight
		(cameraDistance, cameraPitch, cameraRotation, cameraHeight) = app.GetCamera()

		if musicInfo.fieldMusic != "":
			snd.FadeOutMusic("BGM/" + musicInfo.fieldMusic)

		self.onPressKeyDict = None
		self.onClickKeyDict = None

		snd.StopAllSound()
		grp.InitScreenEffect()
		chr.Destroy()
		textTail.Clear()
		quest.Clear()
		guild.Destroy()
		messenger.Destroy()
		wndMgr.Unlock()
		mouseModule.mouseController.DeattachObject()

		if self.guildWarQuestionDialog:
			self.guildWarQuestionDialog.Close()

		self.guildNameBoard = None
		self.partyRequestQuestionDialog = None
		self.partyInviteQuestionDialog = None
		self.guildInviteQuestionDialog = None
		self.messengerAddFriendQuestion = None

		# UNKNOWN_UPDATE
		self.itemDestroyQuestionDialog = None
		# END_OF_UNKNOWN_UPDATE

		# QUEST_CONFIRM
		self.confirmDialog = None
		# END_OF_QUEST_CONFIRM

		self.PrintCoord = None
		self.FrameRate = None
		self.Pitch = None
		self.Splat = None
		self.TextureNum = None
		self.ObjectNum = None
		self.ViewDistance = None
		self.PrintMousePos = None

		if self.interface:
			SaveMouseButtonSettings()
			self.interface.HideAllWindows()
			self.interface.Close()

		if self.infoChat:
			self.infoChat.ClearAll()
			self.infoChat.Hide()

		if self.affectShower:
			self.affectShower.ClearAllAffects()

		player.ResetCameraRotation()
		dungeonInfo.ClearInfo()

		self.KillFocus()
		app.HideCursor()

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def __BuildKeyDict(self):
		onPressKeyDict = {
			app.VK_1: Event(self.__PressNumKey, 1),
			app.VK_2: Event(self.__PressNumKey, 2),
			app.VK_3: Event(self.__PressNumKey, 3),
			app.VK_4: Event(self.__PressNumKey, 4),
			app.VK_5: Event(self.__PressNumKey, 5),
			app.VK_6: Event(self.__PressNumKey, 6),
			app.VK_7: Event(self.__PressNumKey, 7),
			app.VK_8: Event(self.__PressNumKey, 8),
			app.VK_9: Event(self.__PressNumKey, 9),
			app.VK_F1: Event(self.__PressQuickSlot, 4),
			app.VK_F2: Event(self.__PressQuickSlot, 5),
			app.VK_F3: Event(self.__PressQuickSlot, 6),
			app.VK_F4: Event(self.__PressQuickSlot, 7),
			app.VK_F5: MakeCallback(self.interface.ToggleSwitchBot),
			app.VK_F6: MakeCallback(self.interface.OpenPlayTimeWindow),
			# app.VK_F6: MakeCallback(self.interface.ToggleSkillTree),
			# app.VK_F7: MakeCallback(self.interface.ToggleHuntingMissions),
			#app.VK_F8: MakeCallback(self.interface.ToggleDungeonInfoWindow),
			app.VK_MENU: MakeCallback(self.ShowName),
			app.VK_CONTROL: MakeCallback(self.ShowMouseImage),
			app.VK_SPACE: MakeCallback(self.StartAttack),
			app.VK_UP: MakeCallback(self.MoveUp),
			app.VK_DOWN: MakeCallback(self.MoveDown),
			app.VK_LEFT: MakeCallback(self.MoveLeft),
			app.VK_RIGHT: MakeCallback(self.MoveRight),
			app.VK_W: MakeCallback(self.MoveUp),
			app.VK_S: MakeCallback(self.MoveDown),
			app.VK_A: MakeCallback(self.MoveLeft),
			app.VK_D: MakeCallback(self.MoveRight),
			app.VK_E: lambda: app.RotateCamera(app.CAMERA_TO_POSITIVE),
			app.VK_R: lambda: app.ZoomCamera(app.CAMERA_TO_NEGATIVE),
			app.VK_T: lambda: app.PitchCamera(app.CAMERA_TO_NEGATIVE),
			app.VK_G: MakeCallback(self.__PressGKey),
			app.VK_Q: MakeCallback(self.__PressQKey),
			app.VK_NUMPAD9: MakeCallback(app.MovieResetCamera),
			app.VK_NUMPAD4: lambda: app.MovieRotateCamera(app.CAMERA_TO_NEGATIVE),
			app.VK_NUMPAD6: lambda: app.MovieRotateCamera(app.CAMERA_TO_POSITIVE),
			app.VK_PRIOR: lambda: app.MovieZoomCamera(app.CAMERA_TO_NEGATIVE),
			app.VK_NEXT: lambda: app.MovieZoomCamera(app.CAMERA_TO_POSITIVE),
			app.VK_NUMPAD8: lambda: app.MoviePitchCamera(app.CAMERA_TO_NEGATIVE),
			app.VK_NUMPAD2: lambda: app.MoviePitchCamera(app.CAMERA_TO_POSITIVE),
			app.VK_OEM3: MakeCallback(self.PickUpItem),
			app.VK_OEM5: MakeCallback(self.PickUpItem),
			app.VK_Z: MakeCallback(self.PickUpItem),
			app.VK_Y: MakeCallback(self.PickUpItem),
			app.VK_C: Event(self.interface.ToggleCharacterWindow, "STATUS"),
			app.VK_V: Event(self.interface.ToggleCharacterWindow, "SKILL"),
			app.VK_N: Event(self.interface.ToggleCharacterWindow, "QUEST"),
			app.VK_I: MakeCallback(self.interface.ToggleInventoryWindow),
			# app.VK_O: MakeCallback(self.__ClickOKey),
			app.VK_M: MakeCallback(self.interface.PressMKey),
			app.VK_ADD: MakeCallback(self.interface.MiniMapScaleUp),
			app.VK_SUBTRACT: MakeCallback(self.interface.MiniMapScaleDown),
			app.VK_L: MakeCallback(self.interface.ToggleChatLogWindow),
			# app.VK_COMMA: MakeCallback(self.ShowConsole),
			app.VK_SHIFT: MakeCallback(self.__SetQuickPageMode),
			app.VK_J: MakeCallback(self.__PressJKey),
			app.VK_H: MakeCallback(self.__PressHKey),
			app.VK_B: MakeCallback(self.__PressBKey),
			app.VK_F: MakeCallback(self.__PressFKey),
			# app.VK_X: MakeCallback(self.__PressXKey),
			app.VK_ESCAPE: MakeCallback(self.__PressEscapeKey),
			app.VK_RETURN: MakeCallback(self.__PressReturnKey),
			app.VK_TAB: MakeCallback(self.__PressTabKey),
		}

		if app.ENABLE_GROWTH_PET_SYSTEM:
			onPressKeyDict[app.VK_P] = MakeCallback(
				self.interface.TogglePetInformationWindow
			)  ##À°¼ºÆê

		self.onPressKeyDict = onPressKeyDict

		onClickKeyDict = {
			app.VK_UP: MakeCallback(self.StopUp),
			app.VK_DOWN: MakeCallback(self.StopDown),
			app.VK_LEFT: MakeCallback(self.StopLeft),
			app.VK_RIGHT: MakeCallback(self.StopRight),
			app.VK_SPACE: MakeCallback(self.EndAttack),
			app.VK_PRINT: MakeCallback(self.SaveScreen),
			app.VK_SNAPSHOT: MakeCallback(self.SaveScreen),
			app.VK_W: MakeCallback(self.StopUp),
			app.VK_S: MakeCallback(self.StopDown),
			app.VK_A: MakeCallback(self.StopLeft),
			app.VK_D: MakeCallback(self.StopRight),
			app.VK_Q: Event(app.RotateCamera, app.CAMERA_STOP),
			app.VK_E: Event(app.RotateCamera, app.CAMERA_STOP),
			app.VK_R: Event(app.ZoomCamera, app.CAMERA_STOP),
			app.VK_F: Event(app.ZoomCamera, app.CAMERA_STOP),
			app.VK_T: Event(app.PitchCamera, app.CAMERA_STOP),
			app.VK_G: MakeCallback(self.__ReleaseGKey),
			app.VK_NUMPAD4: Event(app.MovieRotateCamera, app.CAMERA_STOP),
			app.VK_NUMPAD6: Event(app.MovieRotateCamera, app.CAMERA_STOP),
			app.VK_PRIOR: Event(app.MovieZoomCamera, app.CAMERA_STOP),
			app.VK_NEXT: Event(app.MovieZoomCamera, app.CAMERA_STOP),
			app.VK_NUMPAD8: Event(app.MoviePitchCamera, app.CAMERA_STOP),
			app.VK_NUMPAD2: Event(app.MoviePitchCamera, app.CAMERA_STOP),
			app.VK_MENU: MakeCallback(self.HideName),
			app.VK_CONTROL: MakeCallback(self.HideMouseImage),
			app.VK_SHIFT: MakeCallback(self.__SetQuickSlotMode),
		}

		if os.path.isfile("./camy_enable"):
			onClickKeyDict[app.VK_NUMPAD0] = Event(self.camy.UpKey, app.VK_NUMPAD0)
			onClickKeyDict[app.VK_NUMPAD1] = Event(self.camy.UpKey, app.VK_NUMPAD1)
			onClickKeyDict[app.VK_NUMPAD3] = Event(self.camy.UpKey, app.VK_NUMPAD3)
			onClickKeyDict[app.VK_NUMPAD5] = Event(self.camy.UpKey, app.VK_NUMPAD5)
			onClickKeyDict[app.VK_NUMPAD7] = Event(self.camy.UpKey, app.VK_NUMPAD7)
			onClickKeyDict[app.VK_NUMPAD9] = Event(self.camy.UpKey, app.VK_NUMPAD9)
			onClickKeyDict[app.VK_F9] = MakeCallback(self.__PressCamyKey)

		self.onClickKeyDict = onClickKeyDict

	if os.path.isfile("./camy_enable"):

		def __PressCamyKey(self):
			if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
				self.camy.RedoLast()
			else:
				if self.camy.IsShow():
					self.camy.Hide()
				else:
					self.camy.Show()

	def __PressShiftKey(self):
		if not self.isFreeCamera:
			self.__SetQuickPageMode()
		else:
			if not self.consoleEnable:
				return

			self.AccelerateFreeCam(True)

	def __ReleaseShiftKey(self):
		if not self.isFreeCamera:
			self.__SetQuickSlotMode()
		else:
			if not self.consoleEnable:
				return

			self.freeCam["accelerate"] = 0

	def __PressCtrlKey(self):
		if not self.isFreeCamera:
			self.ShowMouseImage()
		else:
			if not self.consoleEnable:
				return

			self.AccelerateFreeCam(False)

	def __ReleaseCtrlKey(self):
		if not self.isFreeCamera:
			self.HideMouseImage()
		else:
			if not self.consoleEnable:
				return
			self.freeCam["accelerate"] = 0

	def PrintStats(self):
		if not self.consoleEnable:
			return

	def __PressOfflineShopKey(self):
		OfflineShopManager.GetInstance().CreateShop()

	def __ToggleFreeCamera(self):
		if not self.isFreeCamera:
			app.SetFreeCamera()
			app.SetCameraSpeed(75)
			self.isFreeCamera = True
		else:
			app.SetCameraSpeed(100)
			app.SetDefaultCamera()
			self.isFreeCamera = False
			self.ResetFreeCam()

	def ResetFreeCam(self):
		self.freeCam = {
			"speed": 100,
			"accelerate": 0,
			"spVariation": 0,
			"nextSpeedup": 0,
			"inertia": 0,
		}

	def AccelerateFreeCam(self, positive):
		if positive:
			var = 1
		else:
			var = -1

		# Switching control! High inertia switch to compensate current one
		if self.freeCam["spVariation"] != var:
			self.freeCam["inertia"] *= -1

		self.freeCam["accelerate"] = 1
		self.freeCam["spVariation"] = var

	def __PressNumKey(self, num):
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			if 1 <= num <= 9:
				if chrmgr.IsPossibleEmoticon(-1):
					chrmgr.SetEmoticon(-1, int(num) - 1)
					appInst.instance().GetNet().SendEmoticon(int(num) - 1)
		else:
			if 1 <= num <= 4:
				self.pressNumber(num - 1)

	def __ClickOKey(self):
		if app.IsPressed(app.VK_CONTROL):
			self.interface.DragonSoulActivateByKey()
		else:
			self.interface.ToggleDragonSoulWindowWithNoInfo()

	def __ClickBKey(self):
		if app.IsPressed(app.VK_CONTROL):
			return
		else:
			if constInfo.PVPMODE_ACCELKEY_ENABLE:
				self.ChangePKMode()

	@staticmethod
	def __PressJKey():
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			if player.IsMountingHorse():
				appInst.instance().GetNet().SendChatPacket("/unmount")

	def __PressHKey(self):
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			appInst.instance().GetNet().SendChatPacket("/user_horse_ride")

	def __PressBKey(self):
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			appInst.instance().GetNet().SendChatPacket("/user_horse_back")
		else:
			state = "EMOTICON"
			self.interface.ToggleCharacterWindow(state)

	@staticmethod
	def __PressFKey():
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			appInst.instance().GetNet().SendChatPacket("/user_horse_feed")
		else:
			app.ZoomCamera(app.CAMERA_TO_POSITIVE)

	def __PressGKey(self):
		if app.IsPressed(app.VK_CONTROL):
			appInst.instance().GetNet().SendChatPacket("/ride")
		else:
			if self.ShowNameFlag:
				self.interface.ToggleGuildWindow()
			else:
				app.PitchCamera(app.CAMERA_TO_POSITIVE)

	def __ReleaseGKey(self):
		app.PitchCamera(app.CAMERA_STOP)

	def __PressQKey(self):
		if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
			self.interface.ToggleQuestButtons()
		else:
			app.RotateCamera(app.CAMERA_TO_NEGATIVE)

	def __PressNKey(self):
		if app.IsPressed(app.VK_MENU):
			if appInst.instance().GetSettings().IsShowSalesText():
				appInst.instance().GetSettings().SetShowSalesTextFlag(False)
			else:
				appInst.instance().GetSettings().SetShowSalesTextFlag(True)
		else:
			self.interface.ToggleCharacterWindow("QUEST")

		return True

	def __SetQuickSlotMode(self):
		self.pressNumber = MakeEvent(self.__PressQuickSlot)

	def __SetQuickPageMode(self):
		self.pressNumber = MakeEvent(self.__SelectQuickPage)

	def __PressQuickSlot(self, localSlotIndex):
		player.RequestUseLocalQuickSlot(localSlotIndex)

	def __PressEscapeKey(self):

		if app.TARGET == app.GetCursor():
			app.SetCursor(app.NORMAL)
		elif mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()
		elif not self.interface.IsSystemDialogOpen():
			if self.IsFocus() and not self.escFocusCheck:
				self.escFocusCheck = True
				return True
			self.interface.OpenSystemDialog()
		else:
			return True

	def __PressReturnKey(self):
		if app.IsPressed(app.VK_SHIFT):
			if self.interface and self.interface.whisper:
				self.interface.whisper.ShowDialog()
		else:
			self.interface.ToggleChat()

	def __PressTabKey(self):
		if self.interface and self.interface.wndWarBoard:
			if self.interface.wndWarBoard.IsShow():
				self.interface.wndWarBoard.Close()
			else:
				self.interface.wndWarBoard.Open()

	def __SelectQuickPage(self, pageIndex):
		self.quickSlotPageIndex = pageIndex
		player.SetQuickPage(pageIndex)

	def ToggleDebugInfo(self):
		self.isShowDebugInfo = not self.isShowDebugInfo

		if self.isShowDebugInfo:
			self.PrintCoord.Show()
			self.FrameRate.Show()
			self.Pitch.Show()
			self.Splat.Show()
			self.TextureNum.Show()
			self.ObjectNum.Show()
			self.ViewDistance.Show()
			self.PrintMousePos.Show()
		else:
			self.PrintCoord.Hide()
			self.FrameRate.Hide()
			self.Pitch.Hide()
			self.Splat.Hide()
			self.TextureNum.Hide()
			self.ObjectNum.Hide()
			self.ViewDistance.Hide()
			self.PrintMousePos.Hide()

	def __BuildDebugInfo(self):
		# Character Position Coordinate
		self.PrintCoord = ui.TextLine()
		self.PrintCoord.SetFontName(localeInfo.UI_DEF_FONT)
		self.PrintCoord.SetPosition(wndMgr.GetScreenWidth() - 270, 0)

		# Frame Rate
		self.FrameRate = ui.TextLine()
		self.FrameRate.SetFontName(localeInfo.UI_DEF_FONT)
		self.FrameRate.SetPosition(wndMgr.GetScreenWidth() - 270, 20)

		# Camera Pitch
		self.Pitch = ui.TextLine()
		self.Pitch.SetFontName(localeInfo.UI_DEF_FONT)
		self.Pitch.SetPosition(wndMgr.GetScreenWidth() - 270, 40)

		# Splat
		self.Splat = ui.TextLine()
		self.Splat.SetFontName(localeInfo.UI_DEF_FONT)
		self.Splat.SetPosition(wndMgr.GetScreenWidth() - 270, 60)

		##
		self.PrintMousePos = ui.TextLine()
		self.PrintMousePos.SetFontName(localeInfo.UI_DEF_FONT)
		self.PrintMousePos.SetPosition(wndMgr.GetScreenWidth() - 270, 80)

		# TextureNum
		self.TextureNum = ui.TextLine()
		self.TextureNum.SetFontName(localeInfo.UI_DEF_FONT)
		self.TextureNum.SetPosition(wndMgr.GetScreenWidth() - 270, 100)

		# ¿ÀºêÁ§Æ® ±×¸®´Â °³¼ö
		self.ObjectNum = ui.TextLine()
		self.ObjectNum.SetFontName(localeInfo.UI_DEF_FONT)
		self.ObjectNum.SetPosition(wndMgr.GetScreenWidth() - 270, 120)

		# ½Ã¾ß°Å¸®
		self.ViewDistance = ui.TextLine()
		self.ViewDistance.SetFontName(localeInfo.UI_DEF_FONT)
		self.ViewDistance.SetPosition(0, 0)

	def ChangePKMode(self):
		if not app.IsPressed(app.VK_CONTROL):
			return

		if playerInst().GetPoint(player.LEVEL) < constInfo.PVPMODE_PROTECTED_LEVEL:
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO,
				localeInfo.OPTION_PVPMODE_PROTECT.format(
					constInfo.PVPMODE_PROTECTED_LEVEL
				),
			)
			return

		curTime = app.GetTime()

		if curTime - self.lastPKModeSendedTime < constInfo.PVPMODE_ACCELKEY_DELAY:
			return

		self.lastPKModeSendedTime = curTime

		curPKMode = player.GetPKMode()
		nextPKMode = curPKMode + 1

		if nextPKMode == player.PK_MODE_PROTECT:
			if 0 == player.GetGuildID():
				self.BINARY_AppendChat(
					CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_CANNOT_SET_GUILD_MODE
				)
				nextPKMode = 0
			else:
				nextPKMode = player.PK_MODE_GUILD

		elif nextPKMode == player.PK_MODE_MAX_NUM:
			nextPKMode = 0

		appInst.instance().GetNet().SendChatPacket("/PKMode " + str(nextPKMode))

	def OnChangePKMode(self):
		if self.interface:
			self.interface.OnChangePKMode()

		try:
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO,
				localeInfo.OPTION_PVPMODE_MESSAGE_DICT[player.GetPKMode()],
			)
		except KeyError:
			print("UNKNOWN PVPMode[%d]" % (player.GetPKMode()))

	###############################################################################################
	###############################################################################################
	## Game Callback Functions

	# Start
	def StartGame(self):
		self.RefreshInventory()
		self.RefreshEquipment()
		self.RefreshCharacter()
		self.RefreshSkill()

	# Refresh
	def CheckGameButton(self):
		if self.interface:
			self.interface.CheckGameButton()

	def RefreshHuntingMissions(self):
		if self.interface:
			self.interface.RefreshHuntingMissions()

	def RefreshAlignment(self):
		self.interface.RefreshAlignment()

	def RefreshStatus(self):
		self.CheckGameButton()

		if self.interface:
			self.interface.RefreshStatus()

		if self.playerGauge:
			self.playerGauge.RefreshGauge()

	def RefreshStamina(self):
		self.interface.RefreshStamina()

	def RefreshSkill(self):
		self.CheckGameButton()
		if self.interface:
			self.interface.RefreshSkill()

	def RefreshQuest(self):
		self.interface.RefreshQuest()

	def RefreshMessenger(self):
		self.interface.RefreshMessenger()

	def RefreshGuildInfoPage(self):
		self.interface.RefreshGuildInfoPage()

	def RefreshGuildBoardPage(self):
		self.interface.RefreshGuildBoardPage()

	def RefreshGuildMemberPage(self):
		self.interface.RefreshGuildMemberPage()

	def RefreshGuildMemberPageGradeComboBox(self):
		self.interface.RefreshGuildMemberPageGradeComboBox()

	def RefreshGuildSkillPage(self):
		self.interface.RefreshGuildSkillPage()

	def RefreshGuildGradePage(self):
		self.interface.RefreshGuildGradePage()

	def UpdateChannelDisplayInfo(self, channel):
		self.BINARY_AppendChat(
			CHAT_TYPE_NOTICE, localeInfo.MOVE_CHANNEL_NOTICE.format(channel)
		)
		self.stream.SetChannelIndex(channel - 1)
		self.interface.MiniMapUpdateServerInfo("|cffffcc00{}, |cffffcc00CH{}".format("|cffffcc00BETA", channel))

	# Events
	def OnBlockMode(self, mode):
		self.interface.OnBlockMode(mode)

	def OpenQuestWindow(self, skin, idx):
		self.interface.OpenQuestWindow(skin, idx)

	def OpenQuestSlideWindow(self, skin, idx):
		self.interface.OpenQuestSlideWindow(skin, idx)

	def AskGuildName(self):
		guildNameBoard = uiCommon.InputDialog()
		guildNameBoard.SetMaxLength(16)
		guildNameBoard.SetTitle(localeInfo.GUILD_NAME)
		guildNameBoard.SetAcceptEvent(self.ConfirmGuildName)
		guildNameBoard.SetCancelEvent(self.CancelGuildName)
		guildNameBoard.Open()

		self.guildNameBoard = guildNameBoard

	def ConfirmGuildName(self):
		guildName = self.guildNameBoard.GetText()
		if not guildName:
			return

		if appInst.instance().GetNet().IsInsultIn(guildName):
			self.PopupMessage(localeInfo.GUILD_CREATE_ERROR_INSULT_NAME)
			return

		appInst.instance().GetNet().SendAnswerMakeGuildPacket(guildName)
		self.guildNameBoard.Close()
		self.guildNameBoard = None
		return True

	def CancelGuildName(self):
		self.guildNameBoard.Close()
		self.guildNameBoard = None
		return True

	def MiniGameTest(self):
		self.wndRoulette = uiMiniGameRoulette.RouletteWindow()
		self.wndRoulette.RouletteProcess(
			player.ROULETTE_GC_OPEN,
			(
				app.GetGlobalTime() + 30,
				[
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
					(39028, 1),
				],
			),
		)

	# Refine
	def PopupMessage(self, msg):
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg, 0, localeInfo.UI_OK)
		self.stream.popupWindow.SetTop()
		self.stream.popupWindow.SetFocus()

	def OpenRefineDialog(
		self, targetItemPos, nextGradeItemVnum, cost, prob, refineType=0
	):
		self.interface.OpenRefineDialog(
			targetItemPos, nextGradeItemVnum, cost, prob, refineType
		)

	def AppendMaterialToRefineDialog(self, vnum, count):
		self.interface.AppendMaterialToRefineDialog(vnum, count)

	def AppendEnhanceMaterialToRefineDialog(self, vnum, count, prob):
		self.interface.AppendEnhanceMaterialToRefineDialog(vnum, count, prob)

	def RunUseSkillEvent(self, slotIndex, coolTime):
		self.interface.OnUseSkill(slotIndex, coolTime)

	def BINARY_RefreshMultiAllow(self, status):
		if self.affectShower:
			self.affectShower.SetDropState(status)

	def BINARY_NEW_AddAffect(self, affectType, subType, pointIdx, value, duration):
		self.affectShower.BINARY_NEW_AddAffect(
			affectType, subType, pointIdx, value, duration
		)
		if (
			chrmgr.AFFECT_DRAGON_SOUL_DECK_0 == affectType
			or chrmgr.AFFECT_DRAGON_SOUL_DECK_1 == affectType
		):
			self.interface.DragonSoulActivate(
				affectType - chrmgr.AFFECT_DRAGON_SOUL_DECK_0
			)
		elif chrmgr.AFFECT_DS_SET == affectType:
			self.interface.DragonSoulSetGrade(value)

		if app.ENABLE_BATTLE_PASS:
			if affectType == chrmgr.AFFECT_BATTLE_PASS:
				if self.interface:
					if self.interface.wndBattlePassButton:
						self.interface.wndBattlePassButton.ShowButton()
						self.interface.wndBattlePassButton.Show()

					if self.interface.wndBattlePass:
						self.interface.wndBattlePass.SetBattlePassInfo(value, duration)

	def BINARY_NEW_RemoveAffect(self, affectType, pointIdx):
		self.affectShower.BINARY_NEW_RemoveAffect(affectType, pointIdx)
		if (
			chrmgr.AFFECT_DRAGON_SOUL_DECK_0 == affectType
			or chrmgr.AFFECT_DRAGON_SOUL_DECK_1 == affectType
		):
			self.interface.DragonSoulDeactivate()
		elif chrmgr.AFFECT_DS_SET == affectType:
			self.interface.DragonSoulSetGrade(0)
		if app.ENABLE_BATTLE_PASS:
			if affectType == chrmgr.AFFECT_BATTLE_PASS:
				if self.interface and self.interface.wndBattlePassButton:
					self.interface.wndBattlePassButton.HideButton()
					self.interface.wndBattlePassButton.Hide()
				if self.interface:
					self.interface.wndBattlePass.ClearMissions()
					self.interface.ClearBattlePass()

	def ActivateSkillSlot(self, slotIndex):
		if self.interface:
			self.interface.OnActivateSkill(slotIndex)

	def SkillGradeChange(self, slotIndex, oldGrade, newGrade):
		pass

	def DeactivateSkillSlot(self, slotIndex):
		if self.interface:
			self.interface.OnDeactivateSkill(slotIndex)

	def RefreshEquipment(self):
		if self.interface:
			self.interface.RefreshInventory()

	def RefreshInventory(self):
		if self.interface:
			self.interface.RefreshInventory()
			if self.interface.wndLevelPet and self.interface.wndLevelPet.IsShow():
				self.interface.wndLevelPet.LoadPetData()

		if self.affectShower:
			self.affectShower.RefreshInventory()

	def RefreshSwitchbot(self):
		if self.interface:
			self.interface.RefreshSwitchbot()

	def RefreshCharacter(self):
		if self.interface:
			self.interface.RefreshCharacter()

	def OnGameOver(self):
		self.CloseTargetBoard()

		self.OpenRestartDialog()

	def OpenRestartDialog(self):
		if self.interface:
			self.interface.OpenRestartDialog()

	if app.ENABLE_12ZI:

		def OpenUI12zi(
			self, yellowmark, greenmark, yellowreward, greenreward, goldreward
		):
			self.interface.OpenUI12zi(
				yellowmark, greenmark, yellowreward, greenreward, goldreward
			)

		def Refresh12ziTimer(self, currentFloor, jumpCount, limitTime, elapseTime):
			self.interface.Refresh12ziTimer(
				currentFloor, jumpCount, limitTime, elapseTime
			)

		def Show12ziJumpButton(self):
			self.interface.Show12ziJumpButton()

		def Hide12ziTimer(self):
			self.interface.Hide12ziTimer()

		def OpenReviveDialog(self, vid, itemcount):
			self.targetBoard.OpenReviveDialog(vid, itemcount)

		def RefreshShopItemToolTip(self):
			self.interface.RefreshShopItemToolTip()

	def ChangeCurrentSkill(self, skillSlotNumber):
		self.interface.OnChangeCurrentSkill(skillSlotNumber)

	# TargetBoard
	def SetPCTargetBoard(self, vid, name):
		self.interface.UpateMaintenancePos(88)
		self.targetBoard.Open(vid, name)

		if app.IsPressed(app.VK_CONTROL):

			# if not player.IsSameEmpire(vid):
			#    return

			if playerInst().IsMainCharacterIndex(vid):
				return
			elif chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(vid):
				return
			if self.interface and self.interface.whisper:
				self.interface.whisper.ShowDialog(name)

	def RefreshTargetBoardByVID(self, vid):
		if self.targetBoard:
			self.targetBoard.RefreshByVID(vid)

	def RefreshTargetBoardByName(self, name):
		self.targetBoard.RefreshByName(name)

	def __RefreshTargetBoard(self):
		self.targetBoard.Refresh()

	def SetHPTargetBoard(self, vid, hp, maxHp, prevHp):
		# Do not show the players HP himself
		if playerInst().IsMainCharacterIndex(vid):
			return

		self.interface.UpateMaintenancePos(88)

		if vid != self.targetBoard.GetTargetVID():
			self.targetBoard.ResetTargetBoard()
			if chr.IsEnemy(vid) or chr.IsStone(vid):
				self.targetBoard.SetEnemyVID(vid)

		self.targetBoard.SetHP(hp, maxHp, prevHp)
		self.targetBoard.Show()

	def SetHPAllianceTargetBoard(self, targetVid, hp, maxHp):
		if self.AllianceTargetBoard.GetTargetVID() == targetVid:
			self.AllianceTargetBoard.ResetTargetBoard()
			self.AllianceTargetBoard.SetTarget()

		self.AllianceTargetBoard.SetHP(hp, maxHp)
		self.AllianceTargetBoard.Show()

	def CloseTargetBoardIfDifferent(self, vid):
		if vid != self.targetBoard.GetTargetVID():
			self.targetBoard.Close()
			self.interface.UpateMaintenancePos(2)

	def CloseTargetBoard(self):
		self.targetBoard.Close()
		self.interface.UpateMaintenancePos(2)

	# OFFLINE SHOP
	def SetOfflineShopTargetBoard(self, shopName):
		self.targetBoard.ResetTargetBoard()
		self.targetBoard.OpenOfflineShopBoard(shopName)

	# View Equipment
	def OpenEquipmentDialog(self, vid):
		self.interface.OpenEquipmentDialog(vid)

	if app.ENABLE_CHANGE_LOOK_SYSTEM:

		def SetEquipmentDialogItem(self, vid, slotIndex, vnum, count, dwChangeLookVnum):
			self.interface.SetEquipmentDialogItem(
				vid, slotIndex, vnum, count, dwChangeLookVnum
			)

	else:

		def SetEquipmentDialogItem(self, vid, slotIndex, vnum, count):
			self.interface.SetEquipmentDialogItem(vid, slotIndex, vnum, count)

	def SetEquipmentDialogSocket(self, vid, slotIndex, socketIndex, value):
		self.interface.SetEquipmentDialogSocket(vid, slotIndex, socketIndex, value)

	def SetEquipmentDialogAttr(self, vid, slotIndex, attrIndex, attrType, value):
		self.interface.SetEquipmentDialogAttr(
			vid, slotIndex, attrIndex, attrType, value
		)

	# SHOW_LOCAL_MAP_NAME
	def ShowMapName(self, mapName, x, y):
		if self.mapNameShower:
			self.mapNameShower.ShowMapName(mapName, x, y)

		if self.interface:
			self.interface.SetMapName(mapName)

	# END_OF_SHOW_LOCAL_MAP_NAME

	def BINARY_OpenAtlasWindow(self):
		self.interface.BINARY_OpenAtlasWindow()

	# Chat
	def OnRecvWhisper(self, mode, lang, name, line):
		if mode == WHISPER_TYPE_GM:
			self.interface.RegisterGameMasterName(name)

		self.interface.RegisterWhisperLanguageByName(name, lang)
		line = localeInfo.FormatWhisperLine(name, line)
		if self.interface and self.interface.whisper:
			self.interface.whisper.AppendWhisper(name, mode, line)

	def OnRecvWhisperSystemMessage(self, mode, lang, name, line):
		self.interface.whisper.AppendWhisper(name, WHISPER_TYPE_SYSTEM, line)

	def OnRecvWhisperError(self, mode, lang, name, line):
		if localeInfo.WHISPER_ERROR.has_key(mode):
			self.interface.whisper.AppendWhisper(
				name, WHISPER_TYPE_SYSTEM, localeInfo.WHISPER_ERROR[mode].format(name)
			)
		else:
			self.interface.whisper.AppendWhisper(
				name,
				WHISPER_TYPE_SYSTEM,
				"Whisper Unknown Error(mode={}, name={})".format(mode, name),
			)

	# Exchange
	def OnRecvExchangeInfo(self, unixTime, isError, info):
		if isError == 1:
			error = True
		else:
			error = False

		self.interface.ExchangeInfo(unixTime, info, error)

	def RecvBlockChatMessage(self, name, hour, m, sec):
		self.interface.whisper.AppendWhisper(
			name,
			WHISPER_TYPE_SYSTEM,
			localeInfo.ERROR_TARGET_HAVE_BLOCK_CHAT.format(int(hour), int(m), int(sec)),
		)

	def OnPickMoney(self, money):
		if appInst.instance().GetSettings().IsShowMoneyLog():
			self.InfoChatOnPick(money, None)

		if self.interface and self.interface.dlgRefineNew:
			self.interface.dlgRefineNew.UpdateMaterials()

	def OnPickItem(self, vnum, count):
		if appInst.instance().GetSettings().IsShowItemLog():
			self.InfoChatOnPick(None, (vnum, count))

		if self.interface and self.interface.dlgRefineNew:
			self.interface.dlgRefineNew.UpdateMaterials()

	def InfoChatOnPick(self, money=None, item_data=None):
		if money is not None:
			money_str = localeInfo.NumberToMoneyString(money)
			if appInst.instance().GetSettings().IsPickupInfoWindowActive():
				self.infoChat.AddSpecialChatLine(
					localeInfo.GAME_PICK_MONEY.format(money_str)
				)
			else:
				self.BINARY_AppendChat(
					CHAT_TYPE_INFO, localeInfo.GAME_PICK_MONEY.format(money_str)
				)

		if item_data is not None:
			proto = itemManager().GetProto(item_data[0])
			if proto:
				if appInst.instance().GetSettings().IsPickupInfoWindowActive():
					self.infoChat.AddSpecialChatLine(
						localeInfo.GAME_PICK_ITEM.format(
							proto.GetName(), long(item_data[1])
						)
					)
				else:
					self.BINARY_AppendChat(
						CHAT_TYPE_INFO,
						localeInfo.GAME_PICK_ITEM.format(
							proto.GetName(), long(item_data[1])
						),
					)

	if app.ENABLE_12ZI:

		def SetBeadCount(self, value):
			self.interface.SetBeadCount(value)

		def NextBeadUpdateTime(self, value):
			self.interface.NextBeadUpdateTime(value)

	def OnShopError(self, errType):
		try:
			self.PopupMessage(localeInfo.SHOP_ERROR_DICT[errType])
		except KeyError:
			self.PopupMessage(localeInfo.SHOP_ERROR_UNKNOWN.format(errType))

	def OnShopCoinError(self, coinVnum):
		proto = itemManager().GetProto(coinVnum)
		if proto:
			self.PopupMessage(
				localeInfo.SHOP_NOT_ENOUGH_MONEY_EX.format(
					"|Eitem:{}|e {}".format(coinVnum, proto.GetName())
				)
			)

	def OnSafeBoxError(self):
		self.PopupMessage(localeInfo.SAFEBOX_ERROR)

	def OnFishingSuccess(self, isFish, fishName):

		self.BINARY_AppendChat(
			CHAT_TYPE_INFO, localeInfo.FISHING_SUCCESS(isFish, fishName)
		)

	# ADD_FISHING_MESSAGE
	def OnFishingNotifyUnknown(self):
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.FISHING_UNKNOWN)

	def OnFishingWrongPlace(self):
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.FISHING_WRONG_PLACE)

	# END_OF_ADD_FISHING_MESSAGE

	def OnFishingNotify(self, isFish, fishName):
		self.BINARY_AppendChat(
			CHAT_TYPE_INFO, localeInfo.FISHING_NOTIFY(isFish, fishName)
		)

	def OnFishingGameStart(self):
		self.interface.OpenFishingGameWindow()

	def OnFishingStopGame(self):
		self.interface.CloseFishingGameWindow()

	def OnFishingFailure(self):
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.FISHING_FAILURE)

	def OnCannotPickItem(self):
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.GAME_CANNOT_PICK_ITEM)

	def FishingMissEffect(self, x, y):
		self.interface.FishingMissEffect(x, y)

	def FishingHitEffect(self):
		self.interface.FishingHitEffect()

	def OpenFishingGameWindow(self):
		self.interface.OpenFishingGameWindow()

	def FishingWaveEffect(self, x, y, delay):
		self.interface.FishingWaveEffect(x, y, delay)

	def CloseFishingGameWindow(self):
		self.interface.CloseFishingGameWindow()

	# MINING
	def OnCannotMining(self):
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.GAME_CANNOT_MINING)

	# END_OF_MINING

	def OnCannotUseSkill(self, vid, type):
		if type in localeInfo.USE_SKILL_ERROR_TAIL_DICT:
			textTail.RegisterInfoTail(vid, localeInfo.USE_SKILL_ERROR_TAIL_DICT[type])

		if type in localeInfo.USE_SKILL_ERROR_CHAT_DICT:
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO, localeInfo.USE_SKILL_ERROR_CHAT_DICT[type]
			)

	def OnCannotShotError(self, vid, errType):
		textTail.RegisterInfoTail(
			vid,
			localeInfo.SHOT_ERROR_TAIL_DICT.get(
				errType, localeInfo.SHOT_ERROR_UNKNOWN.format(errType)
			),
		)

	# Pet
	def PetRefresh(self):
		pass

	# self.interface.wndPet.RefreshStatus()

	# PointReset
	def StartPointReset(self):
		self.interface.OpenPointResetDialog()

	# Shop
	def StartShop(self, vid):
		self.interface.OpenShopDialog(vid)

	def EndShop(self):
		self.interface.CloseShopDialog()

	def RefreshShop(self):
		self.interface.RefreshShopDialog()

	def SetShopSellingPrice(self, Price):
		pass

	# Exchange
	def StartExchange(self):
		self.interface.StartExchange()

	def EndExchange(self):
		self.interface.EndExchange()

	def RefreshExchange(self):
		self.interface.RefreshExchange()

	# Party
	if app.WJ_NEW_USER_CARE:

		def RecvPartyInviteQuestion(self, leaderVID, leaderName):
			partyInviteQuestionDialog = uiCommon.QuestionDialogWithTimeLimit()
			partyInviteQuestionDialog.SetAcceptEvent(
				Event(self.AnswerPartyInvite, True)
			)
			partyInviteQuestionDialog.SetCancelEvent(
				Event(self.AnswerPartyInvite, False)
			)
			partyInviteQuestionDialog.Open(
				leaderName + " " + localeInfo.PARTY_DO_YOU_JOIN, 10
			)
			partyInviteQuestionDialog.SetTimeOverMsg(localeInfo.PARTY_ANSWER_TIMEOVER)
			partyInviteQuestionDialog.SetCancelOnTimeOver()
			partyInviteQuestionDialog.partyLeaderVID = leaderVID
			self.partyInviteQuestionDialog = partyInviteQuestionDialog

	else:

		def RecvPartyInviteQuestion(self, leaderVID, leaderName):
			partyInviteQuestionDialog = uiCommon.QuestionDialog()
			partyInviteQuestionDialog.SetText(
				leaderName + " " + localeInfo.PARTY_DO_YOU_JOIN
			)
			partyInviteQuestionDialog.SetAcceptEvent(
				Event(self.AnswerPartyInvite, True)
			)
			partyInviteQuestionDialog.SetCancelEvent(
				Event(self.AnswerPartyInvite, False)
			)
			partyInviteQuestionDialog.Open()
			partyInviteQuestionDialog.partyLeaderVID = leaderVID
			self.partyInviteQuestionDialog = partyInviteQuestionDialog

	def AnswerPartyInvite(self, answer):

		if not self.partyInviteQuestionDialog:
			return

		partyLeaderVID = self.partyInviteQuestionDialog.partyLeaderVID

		if not app.WJ_NEW_USER_CARE:
			distance = player.GetCharacterDistance(partyLeaderVID)
			if distance < 0.0 or distance > 5000:
				answer = False

		appInst.instance().GetNet().SendPartyInviteAnswerPacket(partyLeaderVID, answer)

		self.partyInviteQuestionDialog.Close()
		self.partyInviteQuestionDialog = None

	def AddPartyMember(self, pid, name, mapIdx, channel, race):
		self.interface.AddPartyMember(pid, name, mapIdx, channel, race)

	def UpdatePartyMemberInfo(self, pid):
		self.interface.UpdatePartyMemberInfo(pid)

	def RemovePartyMember(self, pid):
		self.interface.RemovePartyMember(pid)
		self.__RefreshTargetBoard()

	def LinkPartyMember(self, pid, vid, mapIdx, channel, race):
		self.interface.LinkPartyMember(pid, vid, mapIdx, channel, race)

	def UnlinkPartyMember(self, pid):
		self.interface.UnlinkPartyMember(pid)

	def UnlinkAllPartyMember(self):
		self.interface.UnlinkAllPartyMember()

	def ExitParty(self):
		self.interface.ExitParty()
		self.RefreshTargetBoardByVID(self.targetBoard.GetTargetVID())

	def ChangePartyParameter(self, distributionMode):
		self.interface.ChangePartyParameter(distributionMode)

	# Messenger
	def OnMessengerAddFriendQuestion(self, name):
		messengerAddFriendQuestion = uiCommon.QuestionDialogWithTimeLimit()
		messengerAddFriendQuestion.SetTimeOverMsg(
			localeInfo.MESSENGER_ADD_FRIEND_ANSWER_TIMEOVER
		)
		messengerAddFriendQuestion.SetAcceptEvent(self.OnAcceptAddFriend)
		messengerAddFriendQuestion.SetCancelEvent(self.OnDenyAddFriend)
		messengerAddFriendQuestion.SetCancelOnTimeOver()
		messengerAddFriendQuestion.Open(
			localeInfo.MESSENGER_DO_YOU_ACCEPT_ADD_FRIEND.format(name), 10
		)
		messengerAddFriendQuestion.name = name
		self.messengerAddFriendQuestion = messengerAddFriendQuestion

	def OnAcceptAddFriend(self):
		name = self.messengerAddFriendQuestion.name
		appInst.instance().GetNet().SendChatPacket("/messenger_auth y " + name)
		self.OnCloseAddFriendQuestionDialog()
		return True

	def OnDenyAddFriend(self):
		name = self.messengerAddFriendQuestion.name
		appInst.instance().GetNet().SendChatPacket("/messenger_auth n " + name)
		self.OnCloseAddFriendQuestionDialog()
		return True

	def OnCloseAddFriendQuestionDialog(self):
		self.messengerAddFriendQuestion.Close()
		self.messengerAddFriendQuestion = None
		return True

	## SafeBox
	def OpenSafeboxWindow(self, size):
		self.interface.OpenSafeboxWindow(size)

	def RefreshSafebox(self):
		self.interface.RefreshSafebox()

	def RefreshSafeboxMoney(self):
		self.interface.RefreshSafeboxMoney()

	# ITEM_MALL
	def OpenMallWindow(self, size):
		self.interface.OpenMallWindow(size)

	def RefreshMall(self):
		self.interface.RefreshMall()

	# END_OF_ITEM_MALL

	## Guild
	def RecvGuildInviteQuestion(self, guildID, guildName):
		guildInviteQuestionDialog = uiCommon.QuestionDialog()
		guildInviteQuestionDialog.SetText(
			guildName + " " + localeInfo.GUILD_DO_YOU_JOIN
		)
		guildInviteQuestionDialog.SetAcceptEvent(Event(self.AnswerGuildInvite, True))
		guildInviteQuestionDialog.SetCancelEvent(Event(self.AnswerGuildInvite, False))
		guildInviteQuestionDialog.Open()
		guildInviteQuestionDialog.guildID = guildID
		self.guildInviteQuestionDialog = guildInviteQuestionDialog

	def AnswerGuildInvite(self, answer):

		if not self.guildInviteQuestionDialog:
			return

		guildLeaderVID = self.guildInviteQuestionDialog.guildID
		appInst.instance().GetNet().SendGuildInviteAnswerPacket(guildLeaderVID, answer)

		self.guildInviteQuestionDialog.Close()
		self.guildInviteQuestionDialog = None

	def DeleteGuild(self):
		self.interface.DeleteGuild()

	## Emotion
	def BINARY_ActEmotion(self, emotionIndex):
		if self.interface.wndCharacter:
			self.interface.wndCharacter.ActEmotion(emotionIndex)

	###############################################################################################
	###############################################################################################
	## Keyboard Functions

	def CheckFocus(self):
		if not self.IsFocus():
			if self.interface and self.interface.IsOpenChat():
				self.interface.ToggleChat()

			self.SetFocus()

	def SaveScreen(self):
		global kScreenshotDirectory
		if not os.path.exists(kScreenshotDirectory):
			os.makedirs(kScreenshotDirectory)

		(succeeded, name) = grp.SaveScreenShotToPath(kScreenshotDirectory + "/")

		if succeeded:
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO, localeInfo.SCREENSHOT_SAVE_SUCCESS.format(name)
			)
		else:
			self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.SCREENSHOT_SAVE_FAILURE)

	def ShowConsole(self):
		if self.consoleEnable:
			# player.EndKeyWalkingImmediately() # Not needed
			self.console.OpenWindow()

	def ShowName(self):
		self.ShowNameFlag = True
		self.playerGauge.EnableShowAlways()
		player.SetQuickPage(self.quickSlotPageIndex + 1)
		return False

	def ShowTextTails(self):
		if (
			appInst.instance().GetSettings().IsAlwaysShowPlayerName()
			or self.ShowNameFlag
		):
			textTail.ShowAllPCTextTail()

		if appInst.instance().GetSettings().IsAlwaysShowNPCName() or self.ShowNameFlag:
			textTail.ShowAllNPCTextTail()

		if (
			appInst.instance().GetSettings().IsAlwaysShowMonsterName()
			or self.ShowNameFlag
		):
			textTail.ShowAllMonsterTextTail()

		if appInst.instance().GetSettings().IsAlwaysShowItemName() or self.ShowNameFlag:
			textTail.ShowAllItemTextTail()

	# ADD_ALWAYS_SHOW_NAME
	def __IsShowItemName(self):

		if appInst.instance().GetSettings().IsAlwaysShowItemName():
			return True

		if self.ShowNameFlag:
			return True

		return False

	# END_OF_ADD_ALWAYS_SHOW_NAME

	def HideName(self):
		self.ShowNameFlag = False
		self.playerGauge.DisableShowAlways()
		player.SetQuickPage(self.quickSlotPageIndex)
		return False

	def ShowMouseImage(self):
		self.interface.ShowMouseImage()

	def HideMouseImage(self):
		self.interface.HideMouseImage()

	@staticmethod
	def StartAttack():
		player.SetAttackKeyState(True)

	@staticmethod
	def EndAttack():
		player.SetAttackKeyState(False)

	@staticmethod
	def MoveUp():
		player.SetSingleDIKKeyState(player.DIR_UP, True)

	@staticmethod
	def MoveDown():
		player.SetSingleDIKKeyState(player.DIR_DOWN, True)

	@staticmethod
	def MoveLeft():
		player.SetSingleDIKKeyState(player.DIR_LEFT, True)

	@staticmethod
	def MoveRight():
		player.SetSingleDIKKeyState(player.DIR_RIGHT, True)

	@staticmethod
	def StopUp():
		player.SetSingleDIKKeyState(player.DIR_UP, False)

	@staticmethod
	def StopDown():
		player.SetSingleDIKKeyState(player.DIR_DOWN, False)

	@staticmethod
	def StopLeft():
		player.SetSingleDIKKeyState(player.DIR_LEFT, False)

	@staticmethod
	def StopRight():
		player.SetSingleDIKKeyState(player.DIR_RIGHT, False)

	@staticmethod
	def PickUpItem():
		playerInst().PickCloseItem()

	###############################################################################################
	###############################################################################################
	# Event Handler

	def OnKeyDown(self, key):
		## if self.interface.wndWeb and self.interface.wndWeb.IsShow():
		#     return
		if key == app.VK_ESCAPE and self.CloseItemDestroyDialog():
			return True

		try:
			return not self.onPressKeyDict[key]()
		except KeyError:
			return False
		except TypeError:
			return False

	def SetFocus(self):
		pass

	def OnKeyUp(self, key):
		try:
			fn = self.onClickKeyDict[key]
		except KeyError:
			return False
		except TypeError:
			return False
		fn()
		return True

	def OnMouseLeftButtonDown(self):
		if self.interface.BUILD_OnMouseLeftButtonDown():
			return True

		if mouseModule.mouseController.isAttached():
			self.CheckFocus()
		else:
			hyperlink = ui.GetHyperlink()
			if hyperlink:
				return True
			else:
				self.CheckFocus()
				player.SetMouseState(player.MBT_LEFT, player.MBS_PRESS)
		return False

	def OnMouseLeftButtonUp(self):
		if OfflineShopManager.GetInstance().POSITIONING_OnMouseLeftButtonUp():
			return
		if self.interface and self.interface.BUILD_OnMouseLeftButtonUp():
			return

		if mouseModule.mouseController.isAttached():
			self.OnMouseLeftButtonUp_WithItem()
		else:
			hyperlink = ui.GetHyperlink()
			if hyperlink:
				self.OnMouseLeftButtonUp_HandleLinks(hyperlink)
			else:
				player.SetMouseState(player.MBT_LEFT, player.MBS_CLICK)

		return True

	def OnMouseLeftButtonUp_WithItem(self):
		attachedType = mouseModule.mouseController.GetAttachedType()
		attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()
		attachedItemSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
		attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()

		## QuickSlot
		if player.SLOT_TYPE_QUICK_SLOT == attachedType:
			player.RequestDeleteGlobalQuickSlot(attachedItemSlotPos)

		## Inventory
		elif player.SLOT_TYPE_INVENTORY == attachedType:

			if player.ITEM_MONEY == attachedItemIndex:
				self.__PutMoney(
					attachedType, attachedItemCount, self.PickingCharacterIndex
				)
			else:
				self.__PutItem(
					attachedType,
					attachedItemIndex,
					attachedItemSlotPos,
					attachedItemCount,
					self.PickingCharacterIndex,
				)

		## DragonSoul
		elif player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedType:
			self.__PutItem(
				attachedType,
				attachedItemIndex,
				attachedItemSlotPos,
				attachedItemCount,
				self.PickingCharacterIndex,
			)

		if player.SLOT_TYPE_ACCE == attachedType:
			self.__PutItem(
				attachedType,
				attachedItemIndex,
				attachedItemSlotPos,
				attachedItemCount,
				self.PickingCharacterIndex,
			)
		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			if player.SLOT_TYPE_CHANGE_LOOK == attachedType:
				self.__PutItem(
					attachedType,
					attachedItemIndex,
					attachedItemSlotPos,
					attachedItemCount,
					self.PickingCharacterIndex,
				)

		mouseModule.mouseController.DeattachObject()

	def OnMouseLeftButtonUp_HandleLinks(self, hyperlink):
		if app.IsPressed(app.VK_MENU):
			link = chat.GetLinkFromHyperlink(hyperlink)

			active = wndMgr.GetFocus()
			if isinstance(active, ui.EditLine):
				active.Insert(link)
		else:
			self.interface.MakeHyperlinkTooltip(hyperlink)

		return True

	def __PutItem(
		self,
		attachedType,
		attachedItemIndex,
		attachedItemSlotPos,
		attachedItemCount,
		dstChrID,
	):
		if attachedType in (
			player.SLOT_TYPE_INVENTORY,
			player.SLOT_TYPE_DRAGON_SOUL_INVENTORY,
		):
			if chr.HasInstance(self.PickingCharacterIndex):
				self.__PutItemWithPickedChar(
					attachedType,
					attachedItemIndex,
					attachedItemSlotPos,
					attachedItemCount,
					dstChrID,
				)
				return

			if attachedItemSlotPos[0] == player.EQUIPMENT:
				self.stream.popupWindow.Close()
				self.stream.popupWindow.Open(
					localeInfo.EXCHANGE_FAILURE_EQUIP_ITEM, 0, localeInfo.UI_OK
				)
				return

		self.__DropItem(
			attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount
		)

	def __PutItemWithPickedChar(
		self,
		attachedType,
		attachedItemIndex,
		attachedItemSlotPos,
		attachedItemCount,
		dstChrID,
	):
		if playerInst().GetMainCharacterIndex() == dstChrID:
			return
		if (
			OfflineShopManager.GetInstance().GetBuilderInstance().IsShow()
			or OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
		):
			return
		if chr.IsNPC(dstChrID) or chr.IsStone(dstChrID) or chr.IsEnemy(dstChrID):
			appInst.instance().GetNet().SendGiveItemPacket(
				dstChrID, MakeItemPosition(attachedItemSlotPos), attachedItemCount
			)
			constInfo.AUTO_REFINE_TYPE = 2
			constInfo.AUTO_REFINE_DATA["NPC"][0] = dstChrID
			constInfo.AUTO_REFINE_DATA["NPC"][1] = attachedItemSlotPos[0]
			constInfo.AUTO_REFINE_DATA["NPC"][2] = attachedItemSlotPos[1]
			constInfo.AUTO_REFINE_DATA["NPC"][3] = attachedItemCount
			return

		appInst.instance().GetNet().SendExchangeStartPacket(dstChrID)
		appInst.instance().GetNet().SendExchangeItemAddPacket(
			MakeItemPosition(attachedItemSlotPos), 0
		)

	def __PutMoney(self, attachedType, attachedMoney, dstChrID):
		if (
			True == chr.HasInstance(dstChrID)
			and playerInst().GetMainCharacterIndex() != dstChrID
		):
			if (
				OfflineShopManager.GetInstance().GetBuilderInstance().IsShow()
				or OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
			):
				return
			appInst.instance().GetNet().SendExchangeStartPacket(dstChrID)
			appInst.instance().GetNet().SendExchangeElkAddPacket(long(attachedMoney))

	def __DropItem(
		self, attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount
	):
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP
			)
			return

		if OfflineShopManager.GetInstance().GetBuilderInstance().IsShow():
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP
			)
			return

		if (
			player.SLOT_TYPE_INVENTORY == attachedType
			and attachedItemSlotPos[0] == player.EQUIPMENT
		):
			self.stream.popupWindow.Close()
			self.stream.popupWindow.Open(
				localeInfo.DROP_ITEM_FAILURE_EQUIP_ITEM, 0, localeInfo.UI_OK
			)
			return

		if player.SLOT_TYPE_ACCE == attachedType:
			appInst.instance().GetNet().SendAcceRefineCheckOut(attachedItemSlotPos[1])
			return

		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			if player.SLOT_TYPE_CHANGE_LOOK == attachedType:
				appInst.instance().GetNet().SendChangeLookCheckoutPacket(
					attachedItemSlotPos
				)
				return

		if attachedType not in (
			player.SLOT_TYPE_INVENTORY,
			player.SLOT_TYPE_DRAGON_SOUL_INVENTORY,
		):
			return

		## Default case -> asking to destroy the item
		proto = itemManager().GetProto(attachedItemIndex)
		if not proto:
			return

		dropItemName = proto.GetName()
		## Question Text
		questionText = localeInfo.HOW_MANY_ITEM_DO_YOU_DROP(
			dropItemName, attachedItemCount
		)

		## Dialog
		itemDestroyQuestionDialog = uiCommon.ItemQuestionDialog()
		itemDestroyQuestionDialog.SetMessage(questionText)
		itemDestroyQuestionDialog.SetAcceptEvent(Event(self.RequestDestroyItem, True))
		itemDestroyQuestionDialog.SetCancelEvent(Event(self.RequestDestroyItem, False))
		itemDestroyQuestionDialog.dropNumber = attachedItemSlotPos
		itemDestroyQuestionDialog.dropCount = attachedItemCount
		itemDestroyQuestionDialog.window_type = player.INVENTORY
		itemDestroyQuestionDialog.count = attachedItemCount
		itemDestroyQuestionDialog.Open(attachedItemIndex, attachedItemSlotPos)
		self.itemDestroyQuestionDialog = itemDestroyQuestionDialog
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

	def RequestDestroyItem(self, answer):
		if not self.itemDestroyQuestionDialog:
			return

		if answer:
			dropNumber = self.itemDestroyQuestionDialog.dropNumber
			appInst.instance().GetNet().SendItemDestroyPacket(
				MakeItemPosition(dropNumber)
			)

		self.CloseItemDestroyDialog()

	def CloseItemDestroyDialog(self):
		if not self.itemDestroyQuestionDialog:
			return False

		self.itemDestroyQuestionDialog.Hide()
		self.itemDestroyQuestionDialog = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)
		return True

	def OnMouseRightButtonDown(self):
		self.CheckFocus()

		if mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()

		else:
			player.SetMouseState(player.MBT_RIGHT, player.MBS_PRESS)

		return True

	def OnMouseRightButtonUp(self):
		if mouseModule.mouseController.isAttached():
			return True

		player.SetMouseState(player.MBT_RIGHT, player.MBS_CLICK)
		return True

	def OnMouseMiddleButtonDown(self):
		player.SetMouseMiddleButtonState(player.MBS_PRESS)

	def OnMouseMiddleButtonUp(self):
		player.SetMouseMiddleButtonState(player.MBS_CLICK)

	def OnUpdate(self):

		if self.mapNameShower.IsShow():
			self.mapNameShower.Update()

		if self.isShowDebugInfo:
			self.UpdateDebugInfo()

		if self.enableXMasBoom:
			self.__XMasBoom_Update()

		if app.ENABLE_FLY_MOUNT:
			if app.IsPressed(app.VK_CONTROL) and app.IsPressed(app.VK_F10):
				chrmgr.UcanAdam(5.0)
			elif app.IsPressed(app.VK_CONTROL) and app.IsPressed(app.VK_H):
				chrmgr.UcanAdam(-5.0)

		if self.isFreeCamera:
			self.__FreeCamSpeedUpdate()

		tempNewLogoutList = []
		for player in constInfo.REAL_LOGOUT_LIST:
			if app.GetGlobalTimeStamp() > player[1] + app.GetGlobalTimeStamp() + 5 * 60:
				constInfo.ALREADY_NOTIFY_LIST[0].remove(player[0])
				constInfo.ALREADY_NOTIFY_LIST[1].remove(player[0])
				constInfo.ALREADY_NOTIFY_LIST[2].remove(player[0])
			else:
				tempNewLogoutList.append(player)

		constInfo.REAL_LOGOUT_LIST = tempNewLogoutList

		self.interface.BUILD_OnUpdate()
		self.escFocusCheck = False

	def __FreeCamSpeedUpdate(self):
		if (
			self.freeCam["accelerate"] != 0
			and app.GetTime() > self.freeCam["nextSpeedup"]
		):

			# Tweak inertia to allow log10(n)
			inMult = 1
			inValue = self.freeCam["inertia"]

			if self.freeCam["inertia"] < 0:
				inMult = -1
				inValue = self.freeCam["inertia"] * -1

			self.freeCam["speed"] += self.freeCam["spVariation"] + inMult * math.log(
				math.fabs(100 - inValue) + 1
			)

			# Min-max
			if self.freeCam["speed"] <= 0:
				self.freeCam["speed"] = 1
			elif self.freeCam["speed"] > 750:
				self.freeCam["speed"] = 750

			self.freeCam["inertia"] += self.freeCam["spVariation"] * 5

			app.SetFreeCameraSpeed(self.freeCam["speed"])
			self.freeCam["nextSpeedup"] = app.GetTime() + 0.1

	def UpdateDebugInfo(self):
		(x, y, z) = player.GetMainCharacterPosition()

		nUpdateTime = app.GetUpdateTime()
		nUpdateFPS = app.GetUpdateFPS()
		nRenderFPS = app.GetRenderFPS()
		nFaceCount = app.GetFaceCount()
		fFaceSpeed = app.GetFaceSpeed()

		nST = background.GetRenderShadowTime()
		(fAveRT, nCurRT) = app.GetRenderTime()
		(iNum, fFogStart, fFogEnd, fFarCilp) = background.GetDistanceSetInfo()
		(iPatch, iSplat, fSplatRatio, sTextureNum) = background.GetRenderedSplatNum()

		if iPatch == 0:
			iPatch = 1

		# (dwRenderedThing, dwRenderedCRC) = background.GetRenderedGraphicThingInstanceNum()

		self.PrintCoord.SetText(
			"Coordinate: %.2f %.2f %.2f ATM: %d"
			% (x, y, z, app.GetAvailableTextureMemory() / (1024 * 1024))
		)
		xMouse, yMouse = wndMgr.GetMousePosition()
		self.PrintMousePos.SetText("MousePosition: %d %d" % (xMouse, yMouse))

		self.FrameRate.SetText(
			"UFPS: %3d UT: %3d FS %.2f" % (nUpdateFPS, nUpdateTime, fFaceSpeed)
		)

		if fAveRT > 1.0:
			self.Pitch.SetText(
				"RFPS: %3d RT:%.2f(%3d) FC: %d(%.2f) "
				% (nRenderFPS, fAveRT, nCurRT, nFaceCount, nFaceCount / fAveRT)
			)

		self.Splat.SetText(
			"PATCH: %d SPLAT: %d BAD(%.2f)" % (iPatch, iSplat, fSplatRatio)
		)
		# self.Pitch.SetText("Pitch: %.2f" % (app.GetCameraPitch())
		# self.TextureNum.SetText("TN : %s" % (sTextureNum))
		# self.ObjectNum.SetText("GTI : %d, CRC : %d" % (dwRenderedThing, dwRenderedCRC))
		self.ViewDistance.SetText(
			"Num : %d, FS : %f, FE : %f, FC : %f" % (iNum, fFogStart, fFogEnd, fFarCilp)
		)

	def OnRender(self):
		appInst.instance().RenderGame()

		(x, y) = wndMgr.GetMousePosition()

		########################
		# Picking
		########################
		textTail.UpdateAllTextTail()

		if wndMgr.IsPickedWindow(self.hWnd):

			self.PickingCharacterIndex = chr.Pick()

			if -1 != self.PickingCharacterIndex:
				textTail.ShowCharacterTextTail(self.PickingCharacterIndex)

			if 0 != self.targetBoard.GetTargetVID():
				textTail.ShowCharacterTextTail(self.targetBoard.GetTargetVID())

			# ADD_ALWAYS_SHOW_NAME
			if not self.__IsShowItemName():
				self.PickingItemIndex = item.Pick()

				if -1 != self.PickingItemIndex:
					textTail.ShowItemTextTail(self.PickingItemIndex)
					# END_OF_ADD_ALWAYS_SHOW_NAME

		## Show all name in the range
		self.ShowTextTails()

		if self.__IsShowItemName():
			self.PickingItemIndex = textTail.Pick(x, y)

		textTail.UpdateShowingTextTail()
		textTail.ArrangeTextTail()

		if -1 != self.PickingItemIndex:
			textTail.SelectItemName(self.PickingItemIndex)

		grp.PopState()
		grp.SetInterfaceRenderState()

		textTail.Render()
		textTail.HideAllTextTail()

	def OnPressExitKey(self):
		self.interface.ToggleSystemDialog()
		return True

	## BINARY CALLBACK
	######################################################################################
	def BINARY_SetSwitchbotItem(self, index):
		if self.interface:
			self.interface.BINARY_SetSwitchbotItem(index)

	def BINARY_AppendChat(self, type, line):
		self.interface.wndChat.AppendLine(type, line)
		self.interface.wndChatLog.AppendLine(type, line)

	# WEDDING
	def BINARY_LoverInfo(self, name, lovePoint):
		if self.interface.wndMessenger:
			self.interface.wndMessenger.OnAddLover(name, lovePoint)

		if self.affectShower:
			self.affectShower.SetLoverInfo(name, lovePoint)

	def BINARY_UpdateLovePoint(self, lovePoint):
		if self.interface.wndMessenger:
			self.interface.wndMessenger.OnUpdateLovePoint(lovePoint)

		if self.affectShower:
			self.affectShower.OnUpdateLovePoint(lovePoint)

	# END_OF_WEDDING

	# QUEST_CONFIRM
	def BINARY_OnQuestConfirm(self, msg, timeout, pid):
		confirmDialog = uiCommon.QuestionDialogWithTimeLimit()
		confirmDialog.Open(msg, timeout)

		def OnAccept(self=weakref.proxy(self)):
			appInst.instance().GetNet().SendQuestConfirmPacket(True, pid)
			self.confirmDialog.Hide()

		def OnCancel(self=weakref.proxy(self)):
			appInst.instance().GetNet().SendQuestConfirmPacket(False, pid)
			self.confirmDialog.Hide()

		confirmDialog.SetAcceptEvent(OnAccept)
		confirmDialog.SetCancelEvent(OnCancel)
		self.confirmDialog = confirmDialog

	# END_OF_QUEST_CONFIRM

	# GIFT command
	def Gift_Show(self):
		self.interface.ShowGift()

	# CUBE
	def BINARY_Cube_Confirm(self, successChance):
		self.wndCubeConfirmDialog = uiCommon.QuestionDialog()
		self.wndCubeConfirmDialog.SetText(localeInfo.CUBE_CONFIRM.format(successChance))
		self.wndCubeConfirmDialog.SetAcceptEvent(self.__CubeConfirmOk)
		self.wndCubeConfirmDialog.SetCancelEvent(self.__CubeConfirmCancel)
		self.wndCubeConfirmDialog.Open()

	def __CubeConfirmOk(self):
		self.wndCubeConfirmDialog.Close()
		del self.wndCubeConfirmDialog

		appInst.instance().GetNet().SendChatPacket("/cube make")

	def __CubeConfirmCancel(self):
		self.wndCubeConfirmDialog.Close()
		del self.wndCubeConfirmDialog

		appInst.instance().GetNet().SendChatPacket("/cube close")

	def BINARY_Cube_Open(self):
		self.interface.OpenCubeWindow()
		self.interface.wndCube.Refresh()

	def BINARY_Cube_Close(self):
		self.interface.CloseCubeWindow()

	def BINARY_Cube_Succeed(self, itemVnum, count):
		self.interface.SucceedCubeWork(itemVnum, count)
		pass

	def BINARY_Cube_Failed(self):
		self.interface.FailedCubeWork()
		pass

	# END_OF_CUBE

	# ¿ëÈ¥¼®
	def BINARY_Highlight_Item(self, inven_type, inven_pos):
		if appInst.instance().GetSettings().IsUsingItemHighlight():
			if self.interface:
				self.interface.Highligt_Item(inven_type, inven_pos)

	def BINARY_DragonSoulGiveQuilification(self):
		self.interface.DragonSoulGiveQuilification()

	def BINARY_DragonSoulRefineWindow_Open(self):
		self.interface.OpenDragonSoulRefineWindow()

	def BINARY_DragonSoulRefineWindow_RefineFail(self, reason, inven_type, inven_pos):
		self.interface.FailDragonSoulRefine(reason, inven_type, inven_pos)

	def BINARY_DragonSoulRefineWindow_RefineSucceed(self, inven_type, inven_pos):
		self.interface.SucceedDragonSoulRefine(inven_type, inven_pos)

	# END of DRAGON SOUL REFINE WINDOW

	def BINARY_SetBigMessage(self, message):
		self.interface.bigBoard.SetTip(message)

	def BINARY_SetTipMessage(self, message):
		logging.debug("BINARY_SetTipMessage %s", message)
		self.interface.tipBoard.SetTip(message)

	if app.ENABLE_12ZI:

		def BINARY_SetMissionMessage(self, message):
			self.interface.missionBoard.SetMission(message)

		def BINARY_SetSubMissionMessage(self, message):
			self.interface.missionBoard.SetSubMission(message)

		def BINARY_CleanMissionMessage(self):
			self.interface.missionBoard.CleanMission()

	def BINARY_AppendNotifyMessage(self, type):
		if not type in localeInfo.NOTIFY_MESSAGE:
			return
		self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.NOTIFY_MESSAGE[type])

	def BINARY_Guild_EnterGuildArea(self, areaID):
		self.interface.BULID_EnterGuildArea(areaID)

	def BINARY_Guild_ExitGuildArea(self, areaID):
		self.interface.BULID_ExitGuildArea(areaID)

	def BINARY_GuildWar_OnSendDeclare(self, guildID):
		pass

	if app.ENABLE_NEW_GUILD_WAR:
		def BINARY_GuildWar_OnRecvDeclare(self, guildID, warType, guildmaxscore, guildminlevel, guildmaxuser, mount, savasci, ninja, sura, shaman):
			mainCharacterName = player.GetMainCharacterName()
			masterName = guild.GetGuildMasterName()
			if mainCharacterName == masterName:
				self.__GuildWar_OpenAskDialog(guildID, warType, guildmaxscore, guildminlevel, guildmaxuser, mount, savasci, ninja, sura, shaman)
	else:
		def BINARY_GuildWar_OnRecvDeclare(self, guildID, warType):
			mainCharacterName = player.GetMainCharacterName()
			masterName = guild.GetGuildMasterName()
			if mainCharacterName == masterName:
				self.__GuildWar_OpenAskDialog(guildID, warType)

	def BINARY_GuildWar_OnRecvPoint(self, gainGuildID, opponentGuildID, point):
		self.interface.OnRecvGuildWarPoint(gainGuildID, opponentGuildID, point)

	### UPDATE NATION WAR
	def BINARY_NationWar_Status(self, score1, score2, score3):
		self.interface.UpdateNationScoreCount(score1, score2, score3)

	def BINARY_NationWar_Lives(self, lives):
		self.interface.UpdateLivesCount(lives)

	### UPDATE NATION WAR

	def BINARY_GuildWar_OnStart(self, guildSelf, guildOpp):
		self.interface.OnStartGuildWar(guildSelf, guildOpp)

	def BINARY_GuildWar_OnEnd(self, guildSelf, guildOpp):
		self.interface.OnEndGuildWar(guildSelf, guildOpp)
		if self.guildWarQuestionDialog:
			self.__GuildWar_CloseAskDialog()

	def BINARY_BettingGuildWar_SetObserverMode(self, isEnable):
		self.interface.BINARY_SetObserverMode(isEnable)

	def BINARY_BettingGuildWar_UpdateObserverCount(self, observerCount):
		self.interface.wndMiniMap.UpdateObserverCount(observerCount)

	def __GuildWar_UpdateObserver(self, obs):
		observerCount = int(obs)
		self.interface.wndMiniMap.UpdateObserverCount(observerCount)

	def __GuildWar_UpdateMemberCount(
		self, guildID1, memberCount1, guildID2, memberCount2, observerCount
	):
		guildID1 = int(guildID1)
		guildID2 = int(guildID2)
		memberCount1 = int(memberCount1)
		memberCount2 = int(memberCount2)
		observerCount = int(observerCount)

		self.interface.UpdateMemberCount(guildID1, memberCount1, guildID2, memberCount2)
		self.interface.wndMiniMap.UpdateObserverCount(observerCount)

	if app.ENABLE_NEW_GUILD_WAR:
		def __GuildWar_OpenAskDialog(self, guildID, warType, guildmaxscore, guildminlevel, guildmaxuser, mount, savasci, ninja, sura, shaman):

			guildName = guild.GetGuildName(guildID)

			# REMOVED_GUILD_BUG_FIX
			if "Noname" == guildName:
				return
			# END_OF_REMOVED_GUILD_BUG_FIX

			import uiGuild
			questionDialog = uiGuild.AcceptGuildWarDialog()
			questionDialog.SAFE_SetAcceptEvent(self.__GuildWar_OnAccept)
			questionDialog.SAFE_SetCancelEvent(self.__GuildWar_OnDecline)
			questionDialog.Open(guildName, warType, guildmaxscore, guildminlevel, guildmaxuser, mount, savasci, ninja, sura, shaman)

			self.guildWarQuestionDialog = questionDialog
	else:
		def __GuildWar_OpenAskDialog(self, guildID, warType):

			guildName = guild.GetGuildName(guildID)

			# REMOVED_GUILD_BUG_FIX
			if "Noname" == guildName:
				return
			# END_OF_REMOVED_GUILD_BUG_FIX

			import uiGuild
			questionDialog = uiGuild.AcceptGuildWarDialog()
			questionDialog.SAFE_SetAcceptEvent(self.__GuildWar_OnAccept)
			questionDialog.SAFE_SetCancelEvent(self.__GuildWar_OnDecline)
			questionDialog.Open(guildName, warType)

			self.guildWarQuestionDialog = questionDialog

	def __GuildWar_CloseAskDialog(self):
		self.guildWarQuestionDialog.Close()
		self.guildWarQuestionDialog = None

	def __GuildWar_OnAccept(self):
		guildName = self.guildWarQuestionDialog.GetGuildName()

		appInst.instance().GetNet().SendChatPacket(
			"/war " + guildName + " " + str(self.guildWarQuestionDialog.GetType())
		)
		self.__GuildWar_CloseAskDialog()
		return 1

	def __GuildWar_OnDecline(self):
		guildName = self.guildWarQuestionDialog.GetGuildName()

		appInst.instance().GetNet().SendChatPacket("/nowar " + guildName)
		self.__GuildWar_CloseAskDialog()

		return 1

	# Guilds online
	def BINARY_ReceiveGuildOnline(self, gName):
		if constInfo.CONSOLE_ENABLE:
			self.BINARY_AppendChat(CHAT_TYPE_GUILD, "Guild goes online: " + gName)

		self.interface.wndGuild.ReceiveGuildOnline(gName)

	def BINARY_ReceiveGuildOffline(self, gName):
		if constInfo.CONSOLE_ENABLE:
			self.BINARY_AppendChat(CHAT_TYPE_GUILD, "Guild goes offline: " + gName)

		self.interface.wndGuild.ReceiveGuildOffline(gName)

	## BINARY CALLBACK
	######################################################################################
	def __ServerCommand_Build(self):
		serverCommandList = {
			"ConsoleEnable": self.__Console_Enable,
			"warboard": self.__WarBoard,
			"DayMode": self.__DayMode_Update,
			"PRESERVE_DayMode": self.__PRESERVE_DayMode_Update,
			"CloseRestartWindow": self.__RestartDialog_Close,
			"OpenPrivateShop": self.__PrivateShop_Open,
			"PartyHealReady": self.PartyHealReady,
			"ShowMeSafeboxPassword": self.AskSafeboxPassword,
			"CloseSafebox": self.CommandCloseSafebox,
			"CloseMall": self.CommandCloseMall,
			"ShowMeMallPassword": self.AskMallPassword,
			"item_mall": self.__ItemMall_Open,
			"RefineSuceeded": self.RefineSuceededMessage,
			"RefineFailed": self.RefineFailedMessage,
			"CantRefine": self.CantRefine,
			"xmas_snow": self.__XMasSnow_Enable,
			"xmas_boom": self.__XMasBoom_Enable,
			"xmas_song": self.__XMasSong_Enable,
			"xmas_tree": self.__XMasTree_Enable,
			"newyear_boom": self.__XMasBoom_Enable,
			"PartyRequest": self.__PartyRequestQuestion,
			"PartyRequestDenied": self.__PartyRequestDenied,
			"horse_state": self.__Horse_UpdateState,
			"hide_horse_state": self.__Horse_HideState,
			"ObserverCount": self.__GuildWar_UpdateObserver,
			"WarUC": self.__GuildWar_UpdateMemberCount,
			"test_server": self.__EnableTestServerFlag,
			"mall": self.__InGameShop_Show,
			"lover_login": self.__LoginLover,
			"lover_logout": self.__LogoutLover,
			"lover_near": self.__LoverNear,
			"lover_far": self.__LoverFar,
			"lover_divorce": self.__LoverDivorce,
			"PlayMusic": self.__PlayMusic,
			"MyShopPriceList": self.__PrivateShop_PriceList,
			"TeamLogin": self.TeamLogin,
			"TeamLogout": self.TeamLogout,
			"AddTeam": self.AddToTeamList,
			"timer_cdr": self.interface.UpdateTimerWindow,
			"curr_biolog": self.interface.UpdateBiologInfo,
			"RefreshPrivateShopSearch": self.__RefreshPrivateShopSearch,
			"ShopSearchRefresh": self.__ShopSearchRefresh,
			"ShowAcceCombineDialog": self.__AcceCombineDialog,
			"ShowAcceAbsorbDialog": self.__AcceAbsorbDialog,
			"ShowItemCombinationDialog": self.__ItemCombinationDialog,
			"OpenMyShopDecoWnd": self.__OpenMyShopDecoWnd,
			"OpenSkillGroupSelect": self.__OpenSkillGroupSelect,
			"ShowTitleInputDialog": self.ShowTitleInputDialog,
			"LevelPetClose": self.CloseLevelPetWindow,
			"LevelPetOpen": self.OpenLevelPetWindow,
			"guild_war": self.__Check_Guild_War,
			"RecvBlockChatMessage": self.RecvBlockChatMessage,
			"ShowChangeDialog": self.__ChangeWindowOpen,
			"exp_bonus_event_start": self.__ExpBonusEventStart,
			"SetDungeonCoolTime": self.SetDungeonCoolTime,
			"ClearDungeonCoolTime": self.ClearDungeonCoolTime,
			"ShowDungeonRejoin": self.__Check_Dungeon_Rejoin,
		}

		self.serverCommander = stringCommander.Analyzer()
		for serverCommandItem in serverCommandList.items():
			self.serverCommander.SAFE_RegisterCallBack(
				serverCommandItem[0], serverCommandItem[1]
			)

	def __ExpBonusEventStart(self, state_idx, exp_bonus, end_time):
		state = int(state_idx)
		# print "__ExpBonusEventStart %s %s" % (state_idx, type(state_idx))
		if self.affectShower:
			self.affectShower.SetExpBonus(state, int(exp_bonus), int(end_time))

	def __expEventImageOverIn(self):
		self.toolTip.Show()

	def __expEventImageOverOut(self):
		self.toolTip.Hide()

	def __WarBoard(self, warBoardInput):
		if self.interface and self.interface.wndWarBoard:
			self.interface.wndWarBoard.Handle(warBoardInput)

	def __Check_Guild_War(self, enable):
		logging.debug(self.interface)
		if self.interface:
			logging.debug(str(enable))
			if int(enable) == 1:
				logging.debug("ShowGuildWarButton")
				self.interface.ShowGuildWarButton()
			else:
				logging.debug("HideGuildWarButton")
				self.interface.HideGuildWarButton()

	def __Check_Dungeon_Rejoin(self, enable):
		if self.interface:
			if int(enable) == 1:
				self.interface.ShowDungeonRejoinButton()
			else:
				self.interface.HideDungeonRejoinButton()

	def BINARY_ServerCommand_Run(self, line):
		try:
			logging.debug("ServerComand: %s", line)
			return self.serverCommander.Run(line)
		except RuntimeError as msg:
			logging.exception("Failed to run cmd: %s", line)
			return False
		except Exception as e:
			logging.exception("Failed to run cmd: %s", e)
			return False

	def __ProcessPreservedServerCommand(self):
		try:
			command = GetPreservedServerCommand()
			while command:
				self.serverCommander.Run(command)
				command = GetPreservedServerCommand()
		except RuntimeError as msg:
			logging.exception("Failed to run preserved cmd")
			return False

	if app.ENABLE_CHANGE_LOOK_SYSTEM:

		def __ChangeWindowOpen(self):
			self.interface.ChangeWindowOpen()

	def __OpenSkillGroupSelect(self):
		if self.interface:
			self.interface.OpenSkillGroupSelect()

	def PartyHealReady(self):
		self.interface.PartyHealReady()

	def AskSafeboxPassword(self):
		self.interface.AskSafeboxPassword()

	if app.ENABLE_ACCE_COSTUME_SYSTEM:
		# ACCE
		def __AcceCombineDialog(self):
			self.interface.AcceDialogOpen(player.ACCE_SLOT_TYPE_COMBINE)

		def __AcceAbsorbDialog(self):
			self.interface.AcceDialogOpen(player.ACCE_SLOT_TYPE_ABSORB)

		def RefreshAcceWindow(self):
			self.interface.RefreshAcceWindow()

		def DeactivateSlot(self, slotindex, slotType=wndMgr.HILIGHTSLOT_ACCE):
			self.interface.DeactivateSlot(slotindex, slotType)

		def ActivateSlot(self, slotindex, slotType=wndMgr.HILIGHTSLOT_ACCE):
			self.interface.ActivateSlot(slotindex, slotType)

	def BINARY_SetChatFilter(self, bLanguageID, bValue):
		if (
			self.interface
			and self.interface.wndChat
			and self.interface.wndChat.wndFilterChat
		):
			self.interface.wndChat.wndFilterChat.SetChatFilter(bLanguageID, bValue)

	def MyPrivShopOpen(self, isCashItem, tabCount):
		self.interface.OpenPrivateShopInputNameDialog(isCashItem, tabCount)

	if app.ENABLE_MYSHOP_DECO:

		def __OpenMyShopDecoWnd(self):
			self.interface.OpenMyShopDecoWnd()

	else:

		def __OpenMyShopDecoWnd(self):
			return

	if app.ENABLE_MOVE_COSTUME_ATTR:

		def __ItemCombinationDialog(self):
			self.interface.ItemCombinationDialogOpen()

	else:

		def __ItemCombinationDialog(self):
			return

	## Shop Search
	def __ShopSearchRefresh(self):
		self.interface.RefreshRequest()

	def __RefreshPrivateShopSearch(self):
		self.interface.RefreshPrivateShopSearch()

	def RefreshPrivateShopSearch(self):
		self.interface.RefreshPrivateShopSearch()

	# ITEM_MALL
	def AskMallPassword(self):
		self.interface.AskMallPassword()

	def __ItemMall_Open(self):
		self.interface.OpenItemMall()

	def CommandCloseMall(self):
		self.interface.CommandCloseMall()

	# END_OF_ITEM_MALL

	def __GiftRandomPopup(self):
		snd.PlaySound("sound/ui/make_soket.wav")  # Because why not!
		self.PopupMessage("You've been gifted an item! Find it in your mall storage.")

	# Level Pet
	def CloseLevelPetWindow(self):
		if self.interface:
			self.interface.CloseLevelPetWindow()

	def OpenLevelPetWindow(self, pos):
		if self.interface:
			self.interface.OpenLevelPetWindow(int(pos))

	def RefineSuceededMessage(self):
		snd.PlaySound("sound/ui/make_soket.wav")
		self.PopupMessage(localeInfo.REFINE_SUCCESS)
		self.interface.CheckRefineDialog(False)

	def RefineFailedMessage(self):
		snd.PlaySound("sound/ui/jaeryun_fail.wav")
		self.PopupMessage(localeInfo.REFINE_FAILURE)
		self.interface.CheckRefineDialog(True)

	def CantRefine(self, refineType):
		snd.PlaySound("sound/ui/jaeryun_fail.wav")

		if refineType == "DDType":
			self.PopupMessage(localeInfo.REFINE_CANT_DD)
			return

		self.PopupMessage(localeInfo.REFINE_CANT)

	def CommandCloseSafebox(self):
		self.interface.CommandCloseSafebox()

	# PRIVATE_SHOP_PRICE_LIST
	def __PrivateShop_PriceList(self, itemVNum, itemPrice):
		uiPrivateShopBuilder.SetPrivateShopItemPrice(itemVNum, itemPrice)

	# END_OF_PRIVATE_SHOP_PRICE_LIST

	def __MarkUnusableFreeUpgrade(self):
		try:
			self.interface.dlgRefineNew.freeUpgrade.Show()
		except:
			pass

	def __Horse_HideState(self):
		if self.affectShower:
			self.affectShower.SetHorseState(0, 0, 0)

	def __Horse_UpdateState(self, level, health, battery):
		if self.affectShower:
			self.affectShower.SetHorseState(int(level), int(health), int(battery))

	@staticmethod
	def __IsXMasMap():
		mapDict = (
			"metin2_map_n_flame_01",
			"metin2_map_deviltower1",
			"metin2_map_n_desert_01",
			"metin2_map_spiderdungeon",
			"metin2_map_spiderdungeon_02",
			"metin2_map_spiderdungeon_03",
			"metin2_map_monkeydungeon",
			"metin2_map_monkeydungeon_02",
			"metin2_map_devilsCatacomb",
			"metin2_map_skipia_dungeon_boss",
			"metin2_map_skipia_dungeon_01",
			"metin2_map_skipia_dungeon_02",
			"metin2_map_monkeydungeon_03",
		)

		if background.GetCurrentMapName() in mapDict:
			return False

		return True

	def __XMasSnow_Enable(self, mode):
		self.__XMasSong_Enable(mode)

		if "1" == mode:
			if app.ENABLE_ENVIRONMENT_EFFECT_OPTION:
				background.SetXMasShowEvent(1)
			if not self.__IsXMasMap():
				return

			background.EnableSnow(1)

		else:
			if app.ENABLE_ENVIRONMENT_EFFECT_OPTION:
				background.SetXMasShowEvent(0)
			background.EnableSnow(0)

	def __XMasBoom_Enable(self, mode):
		if "1" == mode:

			if app.ENABLE_ENVIRONMENT_EFFECT_OPTION:
				if not background.IsBoomMap():
					return
			else:
				if not self.__IsXMasMap():
					return

			self.__DayMode_Update("dark")
			self.enableXMasBoom = True
			self.startTimeXMasBoom = app.GetTime()
		else:
			self.__DayMode_Update("light")
			self.enableXMasBoom = False

	def __XMasTree_Enable(self, grade):
		background.SetXMasTree(int(grade))

	def __XMasSong_Enable(self, mode):
		if "1" == mode:
			XMAS_BGM = "xmas.mp3"

			if app.IsExistFile("BGM/" + XMAS_BGM) == 1:
				if musicInfo.fieldMusic != "":
					snd.FadeOutMusic("BGM/" + musicInfo.fieldMusic)

				musicInfo.fieldMusic = XMAS_BGM
				snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)
		else:
			if musicInfo.fieldMusic != "":
				snd.FadeOutMusic("BGM/" + musicInfo.fieldMusic)

			musicInfo.fieldMusic = musicInfo.METIN2THEMA
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

	def __RestartDialog_Close(self):
		self.interface.CloseRestartDialog()

	# PET_SYSTEM
	# if self.interface.wndPet:
	#    self.interface.wndPet.Revive()
	# END_OF_PET_SYSTEM

	def __Console_Enable(self):
		constInfo.CONSOLE_ENABLE = True
		self.consoleEnable = True
		app.EnableSpecialCameraMode()

	## PrivateShop
	def __PrivateShop_Open(self):
		self.interface.OpenPrivateShopInputNameDialog()

	def BINARY_PrivateShop_Appear(self, vid, text, titleType):
		self.interface.AppearPrivateShop(vid, text, titleType)

	def BINARY_PrivateShop_Disappear(self, vid):
		self.interface.DisappearPrivateShop(vid)

	## DayMode
	def __PRESERVE_DayMode_Update(self, mode):
		if "light" == mode:
			if not background.IsBoomMap():
				return

			background.SetEnvironmentData(background.DAY_MODE_LIGHT)
		elif "dark" == mode:
			if not background.IsBoomMap():
				return

			background.RegisterEnvironmentData(
				background.DAY_MODE_DARK, constInfo.ENVIRONMENT_NIGHT
			)
			background.SetEnvironmentData(background.DAY_MODE_DARK)

	def __DayMode_Update(self, mode):
		if "light" == mode:
			if not background.IsBoomMap():
				return
			self.curtain.FadeOut(self.__DayMode_OnCompleteChangeToLight)
		elif "dark" == mode:
			if not background.IsBoomMap():
				return

			self.curtain.FadeOut(self.__DayMode_OnCompleteChangeToDark)

	def __DayMode_OnCompleteChangeToLight(self):
		background.SetEnvironmentData(background.DAY_MODE_LIGHT)
		self.curtain.FadeIn()

	def __DayMode_OnCompleteChangeToDark(self):
		background.RegisterEnvironmentData(
			background.DAY_MODE_DARK, constInfo.ENVIRONMENT_NIGHT
		)
		background.SetEnvironmentData(background.DAY_MODE_DARK)
		self.curtain.FadeIn()

	if app.ENABLE_12ZI:

		def BINARY_SetEnvironment(self, idx):
			self.indexEnv = idx
			self.curtain.FadeOut(self.__SetEnvironment)

		def __SetEnvironment(self):
			background.SetEnvironmentData(self.indexEnv)
			self.curtain.FadeIn()

	## XMasBoom
	def __XMasBoom_Update(self):

		self.BOOM_DATA_LIST = ((2, 5), (5, 2), (7, 3), (10, 3), (20, 5))
		if self.indexXMasBoom >= len(self.BOOM_DATA_LIST):
			return

		boomTime = self.BOOM_DATA_LIST[self.indexXMasBoom][0]
		boomCount = self.BOOM_DATA_LIST[self.indexXMasBoom][1]

		if app.GetTime() - self.startTimeXMasBoom > boomTime:

			self.indexXMasBoom += 1

			for i in xrange(boomCount):
				self.__XMasBoom_Boom()

	def __XMasBoom_Boom(self):
		x, y, z = player.GetMainCharacterPosition()
		randX = app.GetRandom(-150, 150)
		randY = app.GetRandom(-150, 150)

		snd.PlaySound3D(x + randX, -y + randY, z, "sound/common/etc/salute.mp3")

	def __PartyRequestQuestion(self, vid, name):
		vid = int(vid)
		partyRequestQuestionDialog = uiCommon.QuestionDialogWithTimeLimit()
		partyRequestQuestionDialog.SetAcceptText(localeInfo.UI_ACCEPT)
		partyRequestQuestionDialog.SetCancelText(localeInfo.UI_DENY)
		partyRequestQuestionDialog.SetAcceptEvent(
			Event(self.__AnswerPartyRequest, True, vid)
		)
		partyRequestQuestionDialog.SetCancelEvent(
			Event(self.__AnswerPartyRequest, False, vid)
		)
		partyRequestQuestionDialog.Open(name + " " + localeInfo.PARTY_DO_YOU_ACCEPT, 10)
		partyRequestQuestionDialog.SetTimeOverMsg(localeInfo.PARTY_ANSWER_TIMEOVER)
		partyRequestQuestionDialog.SetCancelOnTimeOver()
		self.partyRequestQuestionDialog = partyRequestQuestionDialog

	def __AnswerPartyRequest(self, answer, vid):
		if not self.partyRequestQuestionDialog:
			return

		if answer:
			appInst.instance().GetNet().SendChatPacket(
				"/party_request_accept " + str(vid)
			)
		else:
			appInst.instance().GetNet().SendChatPacket(
				"/party_request_deny " + str(vid)
			)

		self.partyRequestQuestionDialog.Close()
		self.partyRequestQuestionDialog = None

	def __PartyRequestDenied(self):
		self.PopupMessage(localeInfo.PARTY_REQUEST_DENIED)

	def __EnableTestServerFlag(self):
		app.EnableTestServerFlag()

	def __InGameShop_Show(self, url):
		self.interface.OpenWebWindow(url)

	# TEAM
	def AddToTeamList(self, name):
		self.interface.wndMessenger.OnAddTeam(name)

	def TeamLogin(self, name):
		self.interface.wndMessenger.OnLogin(2, name)

	def TeamLogout(self, name):
		self.interface.wndMessenger.OnLogout(2, name)

	# WEDDING
	def __LoginLover(self):
		if self.interface.wndMessenger:
			self.interface.wndMessenger.OnLoginLover()

	def __LogoutLover(self):
		if self.interface.wndMessenger:
			self.interface.wndMessenger.OnLogoutLover()
		if self.affectShower:
			self.affectShower.HideLoverState()

	def __LoverNear(self):
		if self.affectShower:
			self.affectShower.ShowLoverState()

	def __LoverFar(self):
		if self.affectShower:
			self.affectShower.HideLoverState()

	def __LoverDivorce(self):
		if self.interface.wndMessenger:
			self.interface.wndMessenger.ClearLoverInfo()
		if self.affectShower:
			self.affectShower.ClearLoverState()

	def __PlayMusic(self, flag, filename):
		flag = int(flag)
		if flag:
			snd.FadeOutAllMusic()
			musicInfo.SaveLastPlayFieldMusic()
			snd.FadeInMusic("BGM/" + filename)
		else:
			snd.FadeOutAllMusic()
			musicInfo.LoadLastPlayFieldMusic()
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

	# END_OF_WEDDING

	def TestZiTimer(self):
		self.Refresh12ziTimer(1, 0, 30, 0)

	def OFFLINESHOP_SetOwnerShopKeeperVID(self, cmdStr):
		OfflineShopManager.GetInstance().SetOwnerShopKeeperVID(int(cmdStr))

	def BINARY_AddDropInfo(self, race, vnum, count, minLevel, maxLevel, pct):
		if race not in constInfo.DROP_INFO:
			constInfo.DROP_INFO[race] = {}

		proto = itemManager().GetProto(vnum)
		if not proto:
			return

		logging.debug(
			"BINARY_AddDropInfo %d %d %d %d %d %d",
			race,
			vnum,
			count,
			minLevel,
			maxLevel,
			pct,
		)

		if proto.GetType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
			ovnum = vnum
			vnum -= vnum % 10

			if vnum not in constInfo.DROP_INFO[race]:
				constInfo.DROP_INFO[race][vnum] = []
				constInfo.DROP_INFO[race][vnum].append(
					{
						"LEVEL_RANGE": (minLevel, maxLevel),
						"VNUM": (ovnum, ovnum),
						"COUNT": count,
						"PCT": pct,
					}
				)
			else:
				constInfo.DROP_INFO[race][vnum].append(
					{
						"LEVEL_RANGE": (minLevel, maxLevel),
						"VNUM": (ovnum, ovnum),
						"COUNT": count,
						"PCT": pct,
					}
				)

		else:
			if vnum not in constInfo.DROP_INFO[race]:
				constInfo.DROP_INFO[race][vnum] = []
				constInfo.DROP_INFO[race][vnum].append(
					{
						"LEVEL_RANGE": (minLevel, maxLevel),
						"VNUM": vnum,
						"COUNT": count,
						"PCT": pct,
					}
				)
			else:
				constInfo.DROP_INFO[race][vnum].append(
					{
						"LEVEL_RANGE": (minLevel, maxLevel),
						"VNUM": vnum,
						"COUNT": count,
						"PCT": pct,
					}
				)

	def BINARY_RefreshDropInfo(self):
		self.targetBoard.RefreshTargetInfo()

	def BINARY_Maintenance(self, sec, dur):
		sec = int(sec)
		dur = int(dur)
		self.interface.StartMaintenance(sec, dur)

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetHatchingWindowCommand(self, command, pos):
			self.interface.PetHatchingWindowCommand(command, pos)

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetSkillUpgradeDlgOpen(self, slot, index, gold):
			self.interface.PetSkillUpgradeDlgOpen(slot, index, gold)

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetFlashEvent(self, index):
			self.interface.PetFlashEvent(index)

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetAffectShowerRefresh(self):
			self.interface.PetAffectShowerRefresh()

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetEvolInfo(self, index, value):
			self.interface.PetEvolInfo(index, value)

	if app.ENABLE_GROWTH_PET_SYSTEM:

		def PetFeedReuslt(self, result):
			self.interface.PetFeedReuslt(result)

	if app.ENABLE_CHANGED_ATTR:

		def OpenSelectAttrDialog(self, window_type, slotIdx):
			self.interface.OpenSelectAttrDialog(window_type, slotIdx)

	if app.ENABLE_GEM_SYSTEM:

		def OnPickGem(self, gem):
			self.BINARY_AppendChat(
				CHAT_TYPE_INFO, localeInfo.GEM_SYSTEM_PICK_GEM.format(gem)
			)

		def OpenGemShop(self):
			self.interface.OpenGemShop()

		def CloseGemShop(self):
			self.interface.CloseGemShop()

		def RefreshGemShopWIndow(self):
			self.interface.RefreshGemShopWIndow()

		def GemShopSlotBuy(self, slotindex, enable):
			self.interface.GemShopSlotBuy(slotindex, enable)

		def BINARY_OpenSelectItemWindowEx(self):
			self.interface.BINARY_OpenSelectItemWindowEx()

		def BINARY_RefreshSelectItemWindowEx(self):
			self.interface.BINARY_RefreshSelectItemWindowEx()

		def GemShopSlotAdd(self, slotindex, enable):
			self.interface.GemShopSlotAdd(slotindex, enable)

	def ShowTitleInputDialog(self):
		self.selectedTitleColor = 0
		self.colorPicker = ui.ScriptColorPicker()
		self.colorPicker.SetFinishEvent(self.__SelectTitleColor)

		self.titleInput = uiCommon.InputDialogWithColorPicker()
		self.titleInput.SetMaxLength(24)
		self.titleInput.SetTitle(localeInfo.CHOOSE_TITLE)
		self.titleInput.SetDescription(localeInfo.CHOOSE_TITLE_TEXT)
		self.titleInput.SetExtraButtonEvent(self.__OpenTitleColorPicker)
		self.titleInput.SetAcceptEvent(self.__AcceptTitle)
		self.titleInput.SetCancelEvent(self.OnCloseTitleInputDialog)
		self.titleInput.SetPreviewEvent(self.__OnPreviewTitle)
		self.titleInput.GetPreviewButton().SetText(localeInfo.PREVIEW)
		self.titleInput.Open()

	def __OpenTitleColorPicker(self):
		self.colorPicker.Open()
		self.colorPicker.SetColor(0)

	def __SelectTitleColor(self):
		self.selectedTitleColor = self.colorPicker.GetColor()
		if self.titleInput:
			self.titleInput.SetInputValueColor(self.selectedTitleColor)
		self.colorPicker.Close()

	def __AcceptTitle(self):

		text = self.titleInput.GetText()
		color = self.titleInput.GetInputValueColor()

		if not text:
			return

		if appInst.instance().GetNet().IsInsultIn(text):
			self.BINARY_AppendChat(CHAT_TYPE_INFO, localeInfo.CHAT_INSULT_STRING)
			return

		text = emoji_config.ReplaceEmoticons(text)

		appInst.instance().GetNet().SendSetTitlePacket(text, long(color))

		self.OnCloseTitleInputDialog()
		return True

	def __OnPreviewTitle(self):
		if self.titleInput.GetPreviewButton().GetText() == localeInfo.END_PREVIEW:
			playerInst().SetPreviewTitle("", 0)
			self.titleInput.GetPreviewButton().SetText(localeInfo.PREVIEW)
			return

		text = self.titleInput.GetText()
		color = self.titleInput.GetInputValueColor()

		if not text:
			return

		playerInst().SetPreviewTitle(text, color)
		self.titleInput.GetPreviewButton().SetText(localeInfo.END_PREVIEW)

	def OnCloseTitleInputDialog(self):
		self.titleInput.Close()
		self.titleInput = None
		playerInst().SetPreviewTitle("", 0)
		return True

	# def __ToggleSwitchbot2(self):
	#    if self.interface:
	#        self.interface.ToggleSwitchbot2()

	def ShowPopUp(self, msg):
		if self.interface:
			self.interface.ShowPopupMessage(msg)

	def BINARY_HuntingAddMission(self, missionId, mobVnum, needCount, minLevel):
		if self.interface:
			self.interface.AddHuntingMission(missionId, mobVnum, needCount, minLevel)

	def BINARY_HuntingAddMissionReward(self, missionId, vnum, count):
		if self.interface:
			self.interface.AddHuntingMissionReward(missionId, vnum, count)

	if app.ENABLE_BATTLE_PASS:

		def BINARY_BattlePassOpen(self):
			if self.interface:
				self.interface.OpenBattlePass()

		def BINARY_BattlePassClearMission(self):
			if self.interface:
				self.interface.wndBattlePass.ClearMissions()

		def BINARY_BattlePassAddMission(
			self, missionType, missionInfo1, missionInfo2, missionInfo3
		):
			if self.interface:
				self.interface.AddBattlePassMission(
					missionType, missionInfo1, missionInfo2, missionInfo3
				)

		def BINARY_BattlePassAddMissionReward(self, missionType, itemVnum, itemCount):
			if self.interface:
				self.interface.AddBattlePassMissionReward(
					missionType, itemVnum, itemCount
				)

		def BINARY_BattlePassUpdate(self, missionType, newProgress):
			if self.interface:
				self.interface.UpdateBattlePassMission(missionType, newProgress)

		def BINARY_BattlePassAddReward(self, itemVnum, itemCount):
			if self.interface:
				self.interface.AddBattlePassReward(itemVnum, itemCount)

		def BINARY_BattlePassAddRanking(self, pos, playerName, finishTime):
			if self.interface:
				self.interface.AddBattlePassRanking(pos, playerName, finishTime)

		def BINARY_BattlePassRefreshRanking(self):
			if self.interface:
				self.interface.RefreshBattlePassRanking()

		def BINARY_BattlePassOpenRanking(self):
			if self.interface:
				self.interface.OpenBattlePassRanking()

	def __RegisterTitleName(self):
		for i in xrange(len(localeInfo.TITLE_NAME_LIST)):
			chrmgr.RegisterTitleName(
				i, localeInfo.TITLE_NAME_LIST[i], localeInfo.TITLE_NAME_LIST_FEMALE[i]
			)

	def __LoadShopDeco(self):
		playerSettingModule.LoadGameData("SHOPDECO")

	def __LoadNamePrefix(self):
		playerSettingModule.LoadGameData("NAMEPREFIX")

	def BINARY_DungeonInfoOpen(self):
		if self.interface:
			self.interface.ToggleDungeonInfoWindow(True)

	def BINARY_DungeonInfoRefresh(self):
		if self.interface:
			self.interface.RefreshDungeonInfo()

	def BINARY_DungeonRankingOpen(self, dungeonID, rankType):
		if self.interface:
			self.interface.ToggleDungeonRankingWindow(dungeonID, rankType, True)

	def SetDungeonCoolTime(self, floor, cooltime):
		self.interface.OpenDungeonCoolTimeWindow(int(floor), int(cooltime))

	def ClearDungeonCoolTime(self):
		self.interface.ClearDungeonCoolTime()
