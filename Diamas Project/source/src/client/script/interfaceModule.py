# -*- coding: utf-8 -*-


from _weakref import proxy

import app
import chat
import chr
import guild
import item
import player
import wndMgr
from pygame.app import appInst

import playerSettingModule
import ui
import uiCharacter
import uiChat
import uiCommon
import uiCube
import uiDragonSoul
import uiEquipmentDialog
import uiExchange
import uiGameButton
import uiGuild
import uiInventory
import uiMessagePopup
import uiMessenger
import uiMiniMap
import uiNation
import uiParty
import uiPointReset
import uiPrivateShopBuilder
import uiPrivateShopSearch
import uiQuest
import uiRefine
import uiRestart
import uiSafebox
# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE
import uiScriptLocale
# ACCESSORY_REFINE_ADD_METIN_STONE
import uiSelectItem
import uiSelectItemEx
import uiShop
import uiSystem
import uiTaskBar
import uiTip
import uiToolTip
import uiWeb
import uiWhisper
from ui_event import Event

if app.ENABLE_PLAYTIME_ICON:
	import uiPlayTime

if app.ENABLE_MOVE_COSTUME_ATTR:
	import uiItemCombination

if app.ENABLE_GROWTH_PET_SYSTEM:
	import uiPetInfo

if app.ENABLE_GEM_SYSTEM:
	import uiGemShop
if app.ENABLE_CHANGED_ATTR:
	import uiSelectAttr

from uiItemPreview import PreviewWindow
import inGameWiki
import uiSkillTree

if app.ENABLE_CHANGE_LOOK_SYSTEM:
	import uiChangeLook
import uiAcce
import event
import localeInfo
import uiMaintenance
import uiQuestTimer
import uiHuntingMissions
if app.ENABLE_GROWTH_PET_SYSTEM:
	import uiPetInfo

if app.ENABLE_MYSHOP_DECO:
	import uiMyShopDecoration
if app.ENABLE_12ZI:
	import ui12zi
if app.ENABLE_BATTLE_PASS:
	import uiBattlePass

import webbrowser

import uiWorldBossInfo

IsQBHide = 0
from uiofflineshop import OfflineShopManager
import uiPetSystem
import uiMiniGame
from pygame.player import playerInst
import uiFishingGame
import uiWarBoard
import uiDungeonInfo
import dungeonInfo
import uiDungeonCooltime

class Interface(object):
	CHARACTER_STATUS_TAB = 1
	CHARACTER_SKILL_TAB = 2
	hideQuestButtons = 0

	def __init__(self, stream):
		self.stream = proxy(stream)

		self.inputDialog = None
		self.tipBoard = None
		self.bigBoard = None

		self.wndMessagePopup = None
		self.refreshCooldowns = True

		self.windowOpenPosition = 0
		self.dlgWhisperWithoutTarget = None
		# self.inputListener = False
		self.tipBoard = None
		self.bigBoard = None
		if app.ENABLE_OX_RENEWAL:
			self.bigBoardControl = None
		if app.ENABLE_BATTLE_PASS:
			self.wndBattlePass = None

		if app.ENABLE_12ZI:
			self.missionBoard = None
		# ITEM_MALL
		self.mallPageDlg = None
		# END_OF_ITEM_MALL

		self.wndWeb = None
		self.wndTaskBar = None
		self.wndCharacter = None
		self.wndInventory = None
		self.wndExpandedTaskBar = None
		self.wndDragonSoul = None
		self.wndDragonSoulRefine = None
		self.wndChat = None
		self.wndMessenger = None
		self.wndMiniMap = None
		self.wndGuild = None
		self.wndGuildBuilding = None
		self.wndDungeonCoolTime = None
		self.wndSkillColor = None
		self.wndNation = None
		self.wndPrivateShopSearch = None
		self.wndSkillGroupSelect = None
		self.wndPreviewWindow = None
		self.wndLevelPet = None
		self.wndSkillTree = None
		self.wndWiki = None
		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			self.wndChangeLook = None

		if app.WJ_ENABLE_TRADABLE_ICON or app.ENABLE_MOVE_COSTUME_ATTR:
			self.OnTopWindow = None
			self.dlgShop = None
			self.dlgExchange = None
			self.privateShopBuilder = None
			self.wndSafebox = None
		if app.ENABLE_MOVE_COSTUME_ATTR:
			self.wndItemCombination = None
		self.wndMaintenanceInfo = None

		self.listGMName = {}
		self.listLangByName = {}
		self.wndQuestWindow = {}
		self.wndQuestWindowNewKey = 0
		self.privateShopAdvertisementBoardDict = {}
		self.guildScoreBoardDict = {}
		self.equipmentDialogDict = {}

		if app.ENABLE_BATTLE_PASS:
			self.wndBattlePass = None
			self.wndBattlePassButton = None

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.wndPetInfoWindow = None

		if app.ENABLE_SKILLBOOK_COMBINATION:
			self.wndSkillBookCombination = None

		if app.ENABLE_CHANGED_ATTR:
			self.wndSelectAttr = None

		if app.ENABLE_12ZI:
			self.wndBead = None

		self.wndQuestTimer = None
		self.wndNotificationWindow = [None] * len(uiQuestTimer.TimerWindow.QUESTS)
		self.lastUpdateTime = 0

		self.wndWorldBossInfo = None
		self.wndFishingGame = None
		self.wndHuntingMissions = None
		self.wndWarBoard = None
		self.wndDungeonInfoWindow = None
		self.wndDungeonRankingWindow = None

		if app.ENABLE_PLAYTIME_ICON:
			self.playtime = uiPlayTime.PlayTimeWindow()
			self.playtime.Hide()

		event.SetInterfaceWindow(self)


	################################
	## Make Windows & Dialogs
	def __MakeUICurtain(self):
		wndUICurtain = ui.Bar("TOP_MOST")
		wndUICurtain.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		wndUICurtain.SetColor(0x77000000)
		wndUICurtain.Hide()
		self.wndUICurtain = wndUICurtain

	if app.ENABLE_PLAYTIME_ICON:
		def OpenPlayTimeWindow(self):
			if self.playtime.IsShow():
				self.playtime.Hide()
			else:
				self.playtime.Show()

	def MakeWarBoard(self):
		self.wndWarBoard = uiWarBoard.WarBoardWindow()

	def MakeWhisperWindow(self):
		self.whisper = self.stream.GetPersistentObject("whisper")
		if not self.whisper:
			self.whisper = uiWhisper.WhisperSystem(self)
			self.stream.RegisterPersistentObject("whisper", self.whisper)
		else:
			self.whisper.interface = proxy(self)
			self.whisper.ShowAll()

	def MakeMessengerWindow(self):
		if self.wndMessenger:
			self.wndMessenger.Hide()
			self.wndMessenger.Destroy()
			self.wndMessenger = None

		self.wndMessenger = uiMessenger.MessengerWindow(self)
		self.wndMessenger.SetWhisperButtonEvent(self.whisper.ShowDialog)
		self.wndMessenger.SetGuildButtonEvent(self.ToggleGuildWindow)

	def OpenWhisperDialog(self, name):
		if self.whisper:
			self.whisper.ShowDialog(name)

	def MakeGuildWindow(self):
		self.wndGuild = uiGuild.GuildWindow()

	def __MakeDungeonCoolTime(self):
		self.wndDungeonCoolTime = uiDungeonCooltime.DungeonCoolTimeWindow()
		self.wndDungeonCoolTime.Hide()

	def __MakeChatWindow(self):
		self.wndChat = self.stream.GetPersistentObject("chat")
		if not self.wndChat:
			self.wndChat = uiChat.ChatWindow()
			self.stream.RegisterPersistentObject("chat", self.wndChat)

		self.wndChat.BindInterface(self)
		self.wndChat.SetSendWhisperEvent(self.whisper.ShowDialog)
		self.wndChat.SetOpenChatLogEvent(self.ToggleChatLogWindow)

		self.wndChat.SetSize(self.wndChat.CHAT_WINDOW_WIDTH, 0)
		self.wndChat.SetPosition(wndMgr.GetScreenWidth() / 1.8 - self.wndChat.CHAT_WINDOW_WIDTH / 1.7,
								 wndMgr.GetScreenHeight() - self.wndChat.EDIT_LINE_HEIGHT - 72)
		self.wndChat.SetOpenHeight(210)
		self.wndChat.Refresh()
		self.wndChat.Show()

	def __MakeTaskBar(self):
		wndTaskBar = uiTaskBar.TaskBar()
		wndTaskBar.LoadWindow()

		self.wndTaskBar = wndTaskBar
		self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_CHARACTER,
											 self.ToggleCharacterWindowStatusPage)
		self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_INVENTORY,
											 self.ToggleInventoryWindow)
		self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_MESSENGER, self.ToggleMessenger)
		self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_SYSTEM, self.ToggleSystemDialog)
		self.wndExpandedMoneyTaskbar = uiTaskBar.ExpandedMoneyTaskBar()
		# self.wndExpandedMoneyTaskbar.LoadWindow()
		if uiTaskBar.TaskBar.IS_EXPANDED:
			self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_EXPAND,
												 self.ToggleExpandedButton)
			self.wndExpandedTaskBar = uiTaskBar.ExpandedTaskBar()
			self.wndExpandedTaskBar.LoadWindow()

			import app
			if app.ENABLE_GROWTH_PET_SYSTEM:
				self.PetInformationActivate()
				self.wndExpandedTaskBar.SetToggleButtonEvent(uiTaskBar.ExpandedTaskBar.BUTTON_PET_INFO,
															 self.TogglePetInformationWindow)

		else:
			self.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_CHAT, self.ToggleChat)

	def __MakeSkillTree(self):
		wndSkillTree = uiSkillTree.SkillTree()
		wndSkillTree.Hide()
		self.wndSkillTree = wndSkillTree

	def __MakeWiki(self):
		wndWiki = inGameWiki.InGameWiki()
		wndWiki.Hide()
		self.wndWiki = wndWiki

	def __MakeParty(self):
		wndParty = uiParty.PartyWindow()
		wndParty.Hide()
		self.wndParty = wndParty

	def __MakeGameButtonWindow(self):
		wndGameButton = uiGameButton.GameButtonWindow()
		wndGameButton.SetTop()
		wndGameButton.Show()
		wndGameButton.SetButtonEvent("STATUS", self.__OnClickStatusPlusButton)
		wndGameButton.SetButtonEvent("SKILL", self.__OnClickSkillPlusButton)
		# wndGameButton.SetButtonEvent("QUEST", self.__OnClickQuestButton)
		wndGameButton.SetButtonEvent("BUILD", self.__OnClickBuildButton)

		self.wndGameButton = wndGameButton

	def __IsChatOpen(self):
		return True

	def __MakeWindows(self):

		wndCharacter = uiCharacter.CharacterWindow()
		wndCharacter.BindInterfaceClass(self)

		wndInventory = uiInventory.InventoryWindow()
		wndInventory.BindInterfaceClass(self)

		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			wndDragonSoul = uiDragonSoul.DragonSoulWindow()
			wndDragonSoulRefine = uiDragonSoul.DragonSoulRefineWindow()
			wndDragonSoulRefine.BindInterfaceClass(self)
			wndDragonSoul.BindInterfaceClass(self)
		else:
			wndDragonSoul = None
			wndDragonSoulRefine = None

		wndMiniMap = uiMiniMap.MiniMap()
		wndSafebox = uiSafebox.SafeboxWindow()
		wndSafebox.BindInterface(self)

		if app.ENABLE_GROWTH_PET_SYSTEM:
			wndPetInfoWindow = uiPetInfo.PetInformationWindow()
			wndPetInfoWindow.BindInterfaceClass(self)
			wndInventory.SetPetHatchingWindow(wndPetInfoWindow.GetPetHatchingWindow())
			wndInventory.SetPetFeedWindow(wndPetInfoWindow.GetPetFeedWindow())

		if app.ENABLE_ACCE_COSTUME_SYSTEM:
			self.wndAcce = uiAcce.AcceWindow()

		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			self.wndChangeLook = uiChangeLook.ChangeLookWindow()

		if app.ENABLE_MOVE_COSTUME_ATTR:
			wndItemCombination = uiItemCombination.ItemCombinationWindow()
			wndItemCombination.BindInterface(self)
			self.wndItemCombination = wndItemCombination

		if app.ENABLE_GEM_SYSTEM:
			self.wndGemShop = None

		wndPrivateShopSearch = uiPrivateShopSearch.PrivateShopSearchDialog()
		wndPrivateShopSearch.BindInterfaceClass(self)

		if app.ENABLE_MYSHOP_DECO:
			wndMyShopDeco = uiMyShopDecoration.MyShopDecoration()
			self.wndMyShopDeco = wndMyShopDeco
		# ITEM_MALL
		wndMall = uiSafebox.MallWindow()
		self.wndMall = wndMall
		# END_OF_ITEM_MALL

		wndChatLog = uiChat.ChatLogWindow()
		wndChatLog.BindInterface(self)

		# Nation Window
		wndNation = uiNation.NationWindow()

		self.wndCharacter = wndCharacter
		self.wndInventory = wndInventory
		self.wndDragonSoul = wndDragonSoul
		self.wndDragonSoulRefine = wndDragonSoulRefine
		self.wndMiniMap = wndMiniMap
		self.wndSafebox = wndSafebox
		self.wndPrivateShopSearch = wndPrivateShopSearch

		if app.ENABLE_CHANGED_ATTR:
			self.wndSelectAttr = uiSelectAttr.SelectAttrWindow()

		self.wndChatLog = wndChatLog
		self.wndNation = wndNation

		self.wndDungeonInfoWindow = uiDungeonInfo.DungeonInfoWindow()
		self.wndDungeonInfoWindow.LoadWindow()
		self.wndDungeonInfoWindow.BindInterface(self)

		self.wndDungeonRankingWindow = uiDungeonInfo.DungeonRankingWindow()
		self.wndDungeonRankingWindow.LoadWindow()

		OfflineShopManager.GetInstance().HideAllWindows()

		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			self.wndDragonSoul.SetDragonSoulRefineWindow(self.wndDragonSoulRefine)
			self.wndDragonSoulRefine.SetInventoryWindows(self.wndInventory, self.wndDragonSoul)
			self.wndInventory.SetDragonSoulRefineWindow(self.wndDragonSoulRefine)

		if app.ENABLE_BATTLE_PASS:
			self.wndBattlePass = uiBattlePass.BattlePassWindow()
			self.wndBattlePassButton = uiBattlePass.BattlePassButton()
			self.wndBattlePassButton.BindInterface(self)

		if not self.wndPreviewWindow:
			self.wndPreviewWindow = PreviewWindow()

		if not self.wndLevelPet:
			self.wndLevelPet = uiPetSystem.LevelPetWindow()

		if not self.wndQuestTimer:
			self.wndQuestTimer = uiQuestTimer.TimerWindow()
			self.wndQuestTimer.Close()

		for i in range(0, len(self.wndQuestTimer.QUESTS)):
			if self.wndQuestTimer.QUESTS[i] == None:
				continue
			self.wndNotificationWindow[i] = uiQuestTimer.NotificationWindow(self.wndQuestTimer.QUESTS[i][0], i)
			self.wndNotificationWindow[i].Close()

			self.refreshCooldowns = True


		self.wndWorldBossInfo = uiWorldBossInfo.WorldBossInfo()
		self.wndHuntingMissions = uiHuntingMissions.HuntingMissions()

	def Open(self):
		if not self.wndHuntingMissions:
			self.wndHuntingMissions = uiHuntingMissions.HuntingMissions()


		self.MakeWhisperWindow()
		if self.wndChat:
			self.wndChat.SetSendWhisperEvent(self.whisper.ShowDialog)

		self.MakeMessengerWindow()
		self.MakeGuildWindow()
		self.ShowGameButtonWindow()
		self.wndHuntingMissions.SetItemToolTip(self.tooltipItem)

		if not self.wndGameButton:
			self.__MakeGameButtonWindow()

		if self.wndInventory:
			self.wndInventory.Reconstruct()

	def __MakeDialogs(self):
		self.dlgExchange = uiExchange.ExchangeDialog()
		self.dlgExchange.BindInterface(self)
		self.dlgExchange.LoadDialog()
		self.dlgExchange.SetCenterPosition()
		self.dlgExchange.Hide()

		self.dlgPointReset = uiPointReset.PointResetDialog()
		self.dlgPointReset.LoadDialog()
		self.dlgPointReset.Hide()

		self.dlgShop = uiShop.ShopDialog()
		self.dlgShop.BindInterface(self)
		self.dlgShop.LoadDialog()
		self.dlgShop.Hide()

		self.dlgRestart = uiRestart.RestartDialog()
		self.dlgRestart.LoadDialog()
		self.dlgRestart.Hide()

		self.dlgSystem = uiSystem.SystemDialog()
		self.dlgSystem.BindInterface(self)
		self.dlgSystem.BindStream(self.stream)
		self.dlgSystem.LoadDialog()

		self.dlgSystem.Hide()

		self.dlgPassword = uiSafebox.PasswordDialog()
		self.dlgPassword.Hide()

		self.hyperlinkItemTooltip = uiToolTip.HyperlinkItemToolTip()
		self.hyperlinkItemTooltip.Hide()

		self.tooltipItem = uiToolTip.ItemToolTip()
		self.tooltipItem.BindInterface(proxy(self))
		self.tooltipItem.Hide()

		self.tooltipSkill = uiToolTip.SkillToolTip()
		self.tooltipSkill.Hide()

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.tooltipPetSkill = uiToolTip.PetSkillToolTip()
			self.tooltipPetSkill.Hide()

		self.privateShopBuilder = uiPrivateShopBuilder.PrivateShopBuilder()
		if app.WJ_ENABLE_TRADABLE_ICON:
			self.privateShopBuilder.BindInterface(self)
		self.privateShopBuilder.Hide()

		self.dlgRefineNew = uiRefine.RefineDialogNew(self)
		self.dlgRefineNew.Hide()

	def __MakeTipBoard(self):
		self.tipBoard = uiTip.TipBoard()
		self.tipBoard.Hide()

		self.bigBoard = uiTip.BigBoard()
		self.bigBoard.Hide()

		if app.ENABLE_OX_RENEWAL:
			self.bigBoardControl = uiTip.BigBoardControl()
			self.bigBoardControl.Hide()

		if app.ENABLE_12ZI:
			self.missionBoard = uiTip.MissionBoard()
			self.missionBoard.Hide()

	def __MakeWebWindow(self):
		if not self.wndWeb:
			self.wndWeb = uiWeb.WebWindow()

		self.wndWeb.Hide()

	def __MakeCubeWindow(self):
		self.wndCube = uiCube.CubeWindow()
		self.wndCube.Hide()

	# ACCESSORY_REFINE_ADD_METIN_STONE
	def __MakeItemSelectWindow(self):
		self.wndItemSelect = uiSelectItem.SelectItemWindow()
		self.wndItemSelect.Hide()

	def __MakeSkillColorPicker(self):
		self.wndSkillColor = ui.ScriptColorPicker()
		self.wndSkillColor.SetFinishEvent(self.__SelectSkillColor)

	if app.ENABLE_GEM_SYSTEM:
		def __MakeItemSelectWindowEx(self):
			self.wndItemSelectEx = uiSelectItemEx.SelectItemWindowEx()
			self.wndItemSelectEx.Hide()

	# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE

	def MakeOnOpen(self):
		self.wndParty = None
		self.__MakeParty()

	def MakeInterface(self):

		self.wndMessagePopup = uiMessagePopup.MessagePopUp()
		self.wndMessagePopup.Show()

		self.MakeWhisperWindow()
		self.MakeMessengerWindow()
		self.MakeGuildWindow()
		self.MakeWarBoard()

		self.__MakeChatWindow()
		self.__MakeParty()
		self.__MakeWindows()
		self.__MakeSkillTree()
		self.__MakeWiki()

		self.__MakeDialogs()

		self.__MakeUICurtain()
		self.__MakeTaskBar()
		self.__MakeGameButtonWindow()
		self.__MakeTipBoard()
		self.__MakeWebWindow()
		self.__MakeCubeWindow()

		# ACCESSORY_REFINE_ADD_METIN_STONE
		self.__MakeItemSelectWindow()
		# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE

		self.__MakeSkillColorPicker()

		if app.ENABLE_GEM_SYSTEM:
			self.__MakeItemSelectWindowEx()

		if app.ENABLE_12ZI:
			self.__Make12ziTimerWindow()
			self.__Make12ziRewardWindow()

		self.__MakeDungeonCoolTime()

		self.questButtonList = []
		self.privateShopAdvertisementBoardDict = {}

		self.wndInventory.SetItemToolTip(self.tooltipItem)
		self.wndWiki.SetItemToolTip(self.tooltipItem)
		self.wndDungeonInfoWindow.SetItemToolTip(self.tooltipItem)

		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			self.wndDragonSoul.SetItemToolTip(self.tooltipItem)
			self.wndDragonSoulRefine.SetItemToolTip(self.tooltipItem)
		self.wndPrivateShopSearch.SetItemToolTip(self.tooltipItem)

		if app.ENABLE_SKILLBOOK_COMBINATION:
			self.wndSkillBookCombination.SetItemToolTip(self.tooltipItem)

		if app.ENABLE_ACCE_COSTUME_SYSTEM:
			self.wndAcce.SetItemToolTip(self.tooltipItem)

		if app.ENABLE_MOVE_COSTUME_ATTR:
			self.wndItemCombination.SetInven(self.wndInventory)
			self.wndItemCombination.SetItemToolTip(self.tooltipItem)
		self.wndSafebox.SetItemToolTip(self.tooltipItem)
		self.wndCube.SetItemToolTip(self.tooltipItem)
		if app.ENABLE_BATTLE_PASS:
			self.wndBattlePass.SetItemToolTip(self.tooltipItem)
		# ITEM_MALL
		self.wndMall.SetItemToolTip(self.tooltipItem)
		# END_OF_ITEM_MALL
		self.wndPrivateShopSearch.SetItemToolTip(self.tooltipItem)

		if app.ENABLE_SKILLBOOK_COMBINATION:
			self.wndSkillBookCombination.SetInven(self.wndInventory)

		if app.ENABLE_GEM_SYSTEM:
			self.wndGemShop = uiGemShop.GemShopWindow()
			self.wndGemShop.Hide()

		if self.wndSkillTree:
			self.wndSkillTree.SetSkillToolTip(self.tooltipSkill)
		self.wndCharacter.SetSkillToolTip(self.tooltipSkill)
		self.wndTaskBar.SetItemToolTip(self.tooltipItem)
		self.wndTaskBar.SetSkillToolTip(self.tooltipSkill)
		self.wndGuild.SetSkillToolTip(self.tooltipSkill)
		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			self.wndChangeLook.SetItemToolTip(self.tooltipItem)

		# ACCESSORY_REFINE_ADD_METIN_STONE
		self.wndItemSelect.SetItemToolTip(self.tooltipItem)
		# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE
		if app.ENABLE_GEM_SYSTEM:
			self.wndItemSelectEx.SetItemToolTip(self.tooltipItem)

		self.dlgShop.SetItemToolTip(self.tooltipItem)
		self.dlgExchange.SetItemToolTip(self.tooltipItem)
		self.privateShopBuilder.SetItemToolTip(self.tooltipItem)

		self.DRAGON_SOUL_IS_QUALIFIED = False

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.wndPetInfoWindow.SetItemToolTip(self.tooltipItem)
			self.wndPetInfoWindow.SetInven(self.wndInventory)
			self.wndPetInfoWindow.SetPetSkillToolTip(self.tooltipPetSkill)

		if app.ENABLE_GEM_SYSTEM:
			self.wndGemShop.SetItemToolTip(self.tooltipItem)

		self.wndHuntingMissions.SetItemToolTip(self.tooltipItem)


	def MakeHyperlinkTooltip(self, hyperlink):
		tokens = hyperlink.split(":")
		if tokens and len(tokens):
			hyperLinkType = tokens[0]
			if "item" == hyperLinkType:
				self.hyperlinkItemTooltip.SetHyperlinkItem(tokens)
			elif "web" == hyperLinkType:
				questionDialog = uiCommon.QuestionDialog()
				questionDialog.SetText(localeInfo.DO_YOU_WANT_TO_OPEN_THIS_WEB_LINK)
				questionDialog.SetAcceptEvent(self.__OnAcceptOpenWebLink)
				questionDialog.SetCancelEvent(self.__OnCancelOpenWebLink)
				questionDialog.link = tokens[1].replace("w<?", "://")
				self.wndQuestionDialog = questionDialog
				self.wndQuestionDialog.Open()
			elif "pmn" == hyperLinkType:
				self.whisper.ShowDialog(str(tokens[1]))
			elif app.ENABLE_GROWTH_PET_SYSTEM and "itempet" == hyperLinkType:
				self.hyperlinkItemTooltip.SetHyperlinkPetItem(tokens)

	def __OnAcceptOpenWebLink(self):
		link = self.wndQuestionDialog.link
		self.OpenWebWindow(link)
		self.__OnCancelOpenWebLink()

	def __OnCancelOpenWebLink(self):
		if self.wndQuestionDialog:
			self.wndQuestionDialog.Close()
		self.wndQuestionDialog = None

	if app.ENABLE_12ZI:
		def __MakeBeadWindow(self):
			self.wndBead = ui12zi.BeadWindow()
			self.wndBead.Hide()

		def __Make12ziRewardWindow(self):
			self.wnd12ziReward = ui12zi.Reward12ziWindow()
			self.wnd12ziReward.SetItemToolTip(self.tooltipItem)
			self.wnd12ziReward.Hide()

		def __Make12ziTimerWindow(self):
			self.wnd12ziTimer = ui12zi.FloorLimitTimeWindow()
			self.wnd12ziTimer.Hide()

	## Make Windows & Dialogs
	################################

	def Reload(self):
		self.HideAllWindows()
		OfflineShopManager.GetInstance().HideAllWindows()
		self.Close()

		self.__init__(self.stream)

		self.MakeInterface()
		self.ShowDefaultWindows()

	def INTERFACE_OnUpdate(self):
		self.UpdateQuestTimerNotifications()

	def Destroy(self):
		if app.ENABLE_MOVE_COSTUME_ATTR and self.wndItemCombination:
			self.wndItemCombination.Destroy()

		if "QuestCurtain" in uiQuest.QuestDialog.__dict__:
			uiQuest.QuestDialog.QuestCurtain.Close()

		if self.wndQuestWindow:
			for key, eachQuestWindow in self.wndQuestWindow.items():
				eachQuestWindow.nextCurtainMode = -1
				eachQuestWindow.CloseSelf()
				eachQuestWindow = None

		self.wndQuestWindow = {}

		if self.wndNation:
			self.wndNation.Destroy()

		if self.wndInventory:
			self.wndInventory.Hide()
			self.wndInventory.Destroy()
			self.wndInventory = None


		if app.ENABLE_12ZI:
			if self.wndBead:
				self.wndBead.Hide()
				self.wndBead.Destroy()
				del self.wndBead
			if self.wnd12ziTimer:
				self.wnd12ziTimer.Hide()
				self.wnd12ziTimer.Destroy()
				del self.wnd12ziTimer
			if self.wnd12ziReward:
				self.wnd12ziReward.Hide()
				self.wnd12ziReward.Destroy()
				del self.wnd12ziReward

		for notificationWindow in self.wndNotificationWindow:
			if notificationWindow:
				notificationWindow.Close()
				notificationWindow.Destroy()
				del notificationWindow

		if self.wndQuestTimer:
			self.wndQuestTimer.Destroy()
			del self.wndQuestTimer

		if self.dlgExchange:
			self.dlgExchange.Destroy()

		if self.dlgPointReset:
			self.dlgPointReset.Destroy()

		if self.dlgShop:
			self.dlgShop.Close()
			self.dlgShop.Destroy()

		if self.dlgRestart:
			self.dlgRestart.Destroy()

		if self.dlgSystem:
			self.dlgSystem.Destroy()

		if self.dlgPassword:
			self.dlgPassword.Destroy()

		if self.wndMiniMap:
			self.wndMiniMap.Destroy()

		if self.wndSafebox:
			self.wndSafebox.Destroy()

		if app.ENABLE_SKILLBOOK_COMBINATION:
			if self.wndSkillBookCombination:
				self.wndSkillBookCombination.Destroy()

		if app.ENABLE_MYSHOP_DECO:
			if self.wndMyShopDeco:
				self.wndMyShopDeco.Destroy()

		if self.wndWeb:
			self.wndWeb.Destroy()
			self.wndWeb = None

		if self.wndParty:
			self.wndParty.Destroy()

		if self.wndCube:
			self.wndCube.Destroy()

		if self.wndMessenger:
			self.wndMessenger.Destroy()

		if self.wndGuild:
			self.wndGuild.Destroy()

		if app.ENABLE_GEM_SYSTEM:
			if self.wndGemShop:
				self.wndGemShop.Destroy()

		if self.privateShopBuilder:
			self.privateShopBuilder.Destroy()

		if self.dlgRefineNew:
			self.dlgRefineNew.Destroy()

		if self.wndGuildBuilding:
			self.wndGuildBuilding.Destroy()

		if self.wndGameButton:
			self.wndGameButton.Destroy()

		# ITEM_MALL
		if self.mallPageDlg:
			self.mallPageDlg.Destroy()
		# END_OF_ITEM_MALL
		if app.ENABLE_BATTLE_PASS:
			if self.wndBattlePass:
				self.wndBattlePass.Destroy()

			if self.wndBattlePassButton:
				self.wndBattlePassButton.Destroy()
		# ACCESSORY_REFINE_ADD_METIN_STONE
		if self.wndItemSelect:
			self.wndItemSelect.Destroy()
		# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE

		if self.wndDungeonCoolTime:
			if self.wndDungeonCoolTime.IsShow():
				self.wndDungeonCoolTime.Close()

			self.wndDungeonCoolTime.Destroy()

		if app.ENABLE_GEM_SYSTEM:
			if self.wndItemSelectEx:
				self.wndItemSelectEx.Destroy()

		if self.wndDungeonInfoWindow:
			self.wndDungeonInfoWindow.Destroy()
			del self.wndDungeonInfoWindow

		if self.wndDungeonRankingWindow:
			self.wndDungeonRankingWindow.Destroy()
			del self.wndDungeonRankingWindow

		if self.wndSkillColor:
			self.wndSkillColor.Destroy()

		if self.wndMaintenanceInfo:
			self.wndMaintenanceInfo.Destroy()

		if app.ENABLE_SKILLBOOK_COMBINATION:
			self.wndSkillBookCombination.Destroy()
			del self.wndSkillBookCombination

		if app.ENABLE_CHANGED_ATTR:
			self.wndSelectAttr.Destroy()
			del self.wndSelectAttr

		self.wndChatLog.Destroy()

		for btn in self.questButtonList:
			btn.SetEvent(None)
			btn.SetOverEvent(None)
		for brd in self.guildScoreBoardDict.itervalues():
			brd.Destroy()
		for dlg in self.equipmentDialogDict.itervalues():
			dlg.Destroy()

		self.DestroyWnd(self.wndPreviewWindow)
		self.DestroyWnd(self.wndLevelPet)
		self.DestroyWnd(self.wndMessagePopup)
		self.DestroyWnd(self.mallPageDlg)
		self.DestroyWnd(self.wndGuild)
		self.DestroyWnd(self.wndMessenger)
		self.DestroyWnd(self.wndUICurtain)
		self.DestroyWnd(self.wndMall)

		self.DestroyWnd(self.wndTaskBar)
		self.DestroyWnd(self.wndExpandedMoneyTaskbar)
		self.DestroyWnd(self.wndExpandedTaskBar)
		self.DestroyWnd(self.wndCharacter)
		self.DestroyWnd(self.wndInventory)
		self.DestroyWnd(self.wndDragonSoul)
		self.DestroyWnd(self.wndDragonSoulRefine)
		if app.ENABLE_PLAYTIME_ICON:
			self.DestroyWnd(self.playtime)

		if app.ENABLE_BATTLE_PASS:
			self.DestroyWnd(self.wndBattlePass)
			self.DestroyWnd(self.wndBattlePassButton)

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.DestroyWnd(self.wndPetInfoWindow)

		self.DestroyWnd(self.wndAcce)
		self.DestroyWnd(self.wndPrivateShopSearch)

		if app.ENABLE_MYSHOP_DECO:
			self.DestroyWnd(self.wndMyShopDeco)

		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			self.DestroyWnd(self.wndChangeLook)

		self.DestroyWnd(self.wndWorldBossInfo)
		self.DestroyWnd(self.wndHuntingMissions)

		if app.ENABLE_GEM_SYSTEM:
			del self.wndItemSelectEx

		if app.ENABLE_GROWTH_PET_SYSTEM:
			del self.tooltipPetSkill

		if app.ENABLE_12ZI:
			del self.missionBoard

		# Clear all the advertisement boards
		for board in self.privateShopAdvertisementBoardDict.itervalues():
			board.Destroy()

		self.privateShopAdvertisementBoardDict = {}
		self.guildScoreBoardDict = {}
		self.equipmentDialogDict = {}

		self.wndChat.Hide()
		self.wndChat.CloseChat()
		#self.wndChat = None

		self.whisper.HideAll()
		self.whisper = None

	def Close(self):
		if app.ENABLE_MOVE_COSTUME_ATTR and self.wndItemCombination:
			self.wndItemCombination.Hide()

		if "QuestCurtain" in uiQuest.QuestDialog.__dict__:
			uiQuest.QuestDialog.QuestCurtain.Close()

		if self.wndQuestWindow:
			for key, eachQuestWindow in self.wndQuestWindow.items():
				eachQuestWindow.nextCurtainMode = -1
				eachQuestWindow.CloseSelf()
				eachQuestWindow = None

		self.wndQuestWindow = {}

		if self.wndNation:
			self.wndNation.Hide()

		if self.wndInventory:
			self.wndInventory.Hide()
			self.wndInventory.DestroySwitchbot()

		if app.ENABLE_12ZI:
			if self.wndBead:
				self.wndBead.Hide()
			if self.wnd12ziTimer:
				self.wnd12ziTimer.Hide()
			if self.wnd12ziReward:
				self.wnd12ziReward.Hide()

		for notificationWindow in self.wndNotificationWindow:
			if notificationWindow:
				notificationWindow.Close()

		if self.wndQuestTimer:
			self.wndQuestTimer.Hide()

		if self.dlgExchange:
			self.dlgExchange.Hide()

		if self.dlgPointReset:
			self.dlgPointReset.Hide()

		if self.dlgShop:
			self.dlgShop.Close()

		if self.dlgRestart:
			self.dlgRestart.Hide()

		if self.dlgSystem:
			self.dlgSystem.Hide()

		if self.dlgPassword:
			self.dlgPassword.Hide()

		if self.wndMiniMap:
			self.wndMiniMap.Hide()
			self.wndMiniMap.AtlasWindow.Close()

		if self.wndSafebox:
			self.wndSafebox.Hide()

		if app.ENABLE_SKILLBOOK_COMBINATION:
			if self.wndSkillBookCombination:
				self.wndSkillBookCombination.Hide()

		if app.ENABLE_MYSHOP_DECO:
			if self.wndMyShopDeco:
				self.wndMyShopDeco.Hide()

		if self.wndWeb:
			self.wndWeb.Hide()
			self.wndWeb = None

		if self.wndParty:
			self.wndParty.Hide()
			self.wndParty.Destroy()

		if self.wndCube:
			self.wndCube.Hide()

		if self.wndMessenger:
			self.wndMessenger.Hide()

		if self.wndGuild:
			self.wndGuild.Hide()
			self.wndGuild.Destroy()
			self.wndGuild = None

		if app.ENABLE_GEM_SYSTEM:
			if self.wndGemShop:
				self.wndGemShop.Hide()

		if self.privateShopBuilder:
			self.privateShopBuilder.Hide()

		if self.dlgRefineNew:
			self.dlgRefineNew.Hide()

		if self.wndGuildBuilding:
			self.wndGuildBuilding.Hide()

		if self.wndGameButton:
			self.wndGameButton.Hide()
			self.wndGameButton.Destroy()
			self.wndGameButton = None

		# ITEM_MALL
		if self.mallPageDlg:
			self.mallPageDlg.Hide()
		# END_OF_ITEM_MALL
		if app.ENABLE_BATTLE_PASS:
			if self.wndBattlePass:
				self.wndBattlePass.Hide()

			if self.wndBattlePassButton:
				self.wndBattlePassButton.Hide()
		# ACCESSORY_REFINE_ADD_METIN_STONE
		if self.wndItemSelect:
			self.wndItemSelect.Hide()
		# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE
		if app.ENABLE_GEM_SYSTEM:
			if self.wndItemSelectEx:
				self.wndItemSelectEx.Hide()

		if self.wndSkillColor:
			self.wndSkillColor.Hide()

		if self.wndMaintenanceInfo:
			self.wndMaintenanceInfo.Hide()

		if app.ENABLE_SKILLBOOK_COMBINATION:
			self.wndSkillBookCombination.Hide()

		if app.ENABLE_CHANGED_ATTR:
			self.wndSelectAttr.Hide()

		self.wndChatLog.Hide()

		for btn in self.questButtonList:
			btn.SetEvent(None)
			btn.SetOverEvent(None)
		for brd in self.guildScoreBoardDict.itervalues():
			brd.Hide()
		for dlg in self.equipmentDialogDict.itervalues():
			dlg.Hide()

		self.HideWnd(self.wndPreviewWindow)
		self.HideWnd(self.wndLevelPet)
		self.HideWnd(self.wndMessagePopup)
		self.HideWnd(self.mallPageDlg)
		self.HideWnd(self.wndGuild)
		self.HideWnd(self.wndMessenger)
		self.HideWnd(self.wndUICurtain)
		self.HideWnd(self.wndMall)

		self.HideWnd(self.wndTaskBar)
		self.HideWnd(self.wndExpandedMoneyTaskbar)
		self.HideWnd(self.wndExpandedTaskBar)
		self.HideWnd(self.wndCharacter)
		self.HideWnd(self.wndInventory)
		self.HideWnd(self.wndDragonSoul)
		self.HideWnd(self.wndDragonSoulRefine)

		if self.wndCharacter:
			self.wndCharacter.Reconstruct()

		if app.ENABLE_BATTLE_PASS:
			self.HideWnd(self.wndBattlePass)
			self.HideWnd(self.wndBattlePassButton)

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.HideWnd(self.wndPetInfoWindow)

		self.HideWnd(self.wndAcce)
		self.HideWnd(self.wndPrivateShopSearch)

		if app.ENABLE_MYSHOP_DECO:
			self.HideWnd(self.wndMyShopDeco)

		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			self.HideWnd(self.wndChangeLook)
		self.HideWnd(self.wndWorldBossInfo)
		if self.wndHuntingMissions:
			self.wndHuntingMissions.Hide()
			self.wndHuntingMissions.Destroy()
			self.wndHuntingMissions = None
		# Clear all the advertisement boards
		for board in self.privateShopAdvertisementBoardDict.itervalues():
			board.Destroy()

		self.DestroyAllQuestButtons()

		self.privateShopAdvertisementBoardDict = {}
		self.guildScoreBoardDict = {}
		self.equipmentDialogDict = {}

		self.wndChat.Hide()
		self.wndChat.CloseChat()

		self.whisper.HideAll()
		self.whisper = None

		self.DestroyWnd(self.wndWarBoard)

	def DestroyWnd(self, wnd):
		if wnd:
			wnd.Hide()
			wnd.Destroy()
			del wnd

	def HideWnd(self, wnd):
		if wnd:
			wnd.Hide()

	def ShowGameButtonWindow(self):
		if self.wndGameButton:
			self.wndGameButton.Show()

	## Self
	def SetOpenBoniSwitcherEvent(self, event):
		if self.wndInventory:
			self.wndInventory.SetOpenBoniSwitcherEvent(event)

	## Skill
	def OnUseSkill(self, slotIndex, coolTime):
		self.wndCharacter.OnUseSkill(slotIndex, coolTime)
		self.wndTaskBar.OnUseSkill(slotIndex, coolTime)
		self.wndGuild.OnUseSkill(slotIndex, coolTime)

	def OnSkillGradeChange(self, slotIndex, oldGrade, newGrade):
		self.wndCharacter.OnSkillGradeChange(slotIndex, oldGrade, newGrade)

	def OnActivateSkill(self, slotIndex):
		self.wndCharacter.OnActivateSkill(slotIndex)
		self.wndTaskBar.OnActivateSkill(slotIndex)

	def OnDeactivateSkill(self, slotIndex):
		self.wndCharacter.OnDeactivateSkill(slotIndex)
		self.wndTaskBar.OnDeactivateSkill(slotIndex)

	def OnChangeCurrentSkill(self, skillSlotNumber):
		self.wndTaskBar.OnChangeCurrentSkill(skillSlotNumber)

	def SelectMouseButtonEvent(self, dir, event):
		self.wndTaskBar.SelectMouseButtonEvent(dir, event)

	## Refresh
	def RefreshAlignment(self):
		self.wndCharacter.RefreshAlignment()

	def RefreshStatus(self):
		self.wndTaskBar.RefreshStatus()
		self.wndCharacter.RefreshStatus()
		self.wndInventory.RefreshStatus()
		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			self.wndDragonSoul.RefreshStatus()

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.wndPetInfoWindow.RefreshStatus()

	def RefreshStamina(self):
		self.wndTaskBar.RefreshStamina()

	def RefreshHuntingMissions(self):
		if self.wndHuntingMissions:
			self.wndHuntingMissions.Refresh()

	def RefreshSkill(self):
		self.wndCharacter.RefreshSkill()
		self.wndTaskBar.RefreshSkill()
		self.wndSkillTree.RefreshSkill()

	def RefreshInventory(self):
		self.wndTaskBar.RefreshQuickSlot()
		self.wndInventory.RefreshItemSlot()
		self.wndDragonSoul.RefreshItemSlot()
		if player.GetAcceRefineWindowOpen() == 1:
			self.wndAcce.RefreshAcceWindow()

		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			if player.GetChangeLookWindowOpen() == 1:
				self.wndChangeLook.RefreshChangeLookWindow()

	def RefreshSwitchbot(self):
		self.wndInventory.RefreshSwitchbot()

	def RefreshCharacter(self):
		self.wndCharacter.RefreshCharacter()
		self.wndTaskBar.RefreshQuickSlot()

	# THIS FUNCTION IS CALLED FROM BINARY
	def RefreshQuest(self):
		self.wndCharacter.RefreshQuest()

	def RefreshSafebox(self):
		self.wndSafebox.RefreshSafebox()

	# ITEM_MALL
	def RefreshMall(self):
		self.wndMall.RefreshMall()

	def OpenItemMall(self):
		if not self.mallPageDlg:
			self.mallPageDlg = uiShop.MallPageDialog()

		self.mallPageDlg.Open()

	# END_OF_ITEM_MALL

	def RefreshMessenger(self):
		self.wndMessenger.RefreshMessenger()

	def RefreshGuildInfoPage(self):
		self.wndGuild.RefreshGuildInfoPage()

	def RefreshGuildBoardPage(self):
		self.wndGuild.RefreshGuildBoardPage()

	def RefreshGuildMemberPage(self):
		self.wndGuild.RefreshGuildMemberPage()

	def RefreshGuildMemberPageGradeComboBox(self):
		self.wndGuild.RefreshGuildMemberPageGradeComboBox()

	def RefreshGuildSkillPage(self):
		self.wndGuild.RefreshGuildSkillPage()

	def RefreshGuildGradePage(self):
		self.wndGuild.RefreshGuildGradePage()

	def DeleteGuild(self):
		self.wndMessenger.ClearGuildMember()
		self.wndGuild.DeleteGuild()


	def OnBlockMode(self, mode):
		self.dlgSystem.OnBlockMode(mode)

	# Calling Functions
	def ShowPopupMessage(self, message):
		if not self.wndMessagePopup:
			self.wndMessagePopup = uiMessagePopup.MessagePopUp()

		self.wndMessagePopup.Show()
		self.wndMessagePopup.PopUp(message)

	# PointReset
	def OpenPointResetDialog(self):
		self.dlgPointReset.Show()
		self.dlgPointReset.SetTop()

	def ClosePointResetDialog(self):
		self.dlgPointReset.Close()

	# Shop
	def OpenShopDialog(self, vid):
		if not self.wndInventory.IsShow():
			self.wndInventory.Show()
		self.wndInventory.SetTop()
	
		self.dlgShop.Open(vid)
		self.dlgShop.SetTop()
		
		if self.wndExpandedMoneyTaskbar:
			self.wndExpandedMoneyTaskbar.Show()
			

	def CloseShopDialog(self):
		self.dlgShop.Close()

	def RefreshShopDialog(self):
		self.dlgShop.Refresh()

	## Quest
	def OpenCharacterWindowQuestPage(self):
		self.wndCharacter.Show()
		self.wndCharacter.SetState("QUEST")

	def OpenQuestWindow(self, skin, idx):
		wnds = ()

		q = uiQuest.QuestDialog(skin, idx)
		q.SetWindowName("QuestWindow" + str(idx))
		q.Show()

		if skin:
			q.Lock()
			wnds = self.__HideWindows()

			q.AddOnDoneEvent(Event(self.__ShowWindows, wnds))
			q.AddOnCloseEvent(q.Unlock)

		q.AddOnCloseEvent(Event(self.RemoveQuestDialog, self.wndQuestWindowNewKey))

		self.wndQuestWindow[self.wndQuestWindowNewKey] = q
		self.wndQuestWindowNewKey += 1

	def RemoveQuestDialog(self, key):
		del self.wndQuestWindow[key]

	# Quest Slide Dialog(QuestDialog?? ??? ???? ?)
	def OpenQuestSlideWindow(self, skin=0, idx=0):
		self.wndCharacter.OpenQuestSlideWindow(skin, idx)

	## Exchange
	def StartExchange(self):
		self.dlgExchange.OpenDialog()
		self.dlgExchange.Refresh()

	def EndExchange(self):
		self.dlgExchange.CloseDialog()

	def RefreshExchange(self):
		self.dlgExchange.Refresh()

	def ExchangeInfo(self, unixTime, info, isError):
		self.dlgExchange.AppendInformation(unixTime, info, isError)

	## Party
	def AddPartyMember(self, pid, name, mapIdx, channel, race):
		self.wndParty.AddPartyMember(pid, name, mapIdx, channel, race)
		self.__ArrangeQuestButton()

	def UpdatePartyMemberInfo(self, pid):
		self.wndParty.UpdatePartyMemberInfo(pid)

	def RemovePartyMember(self, pid):
		self.wndParty.RemovePartyMember(pid)

		##!! 20061026.levites.???_??_??
		self.__ArrangeQuestButton()

	def LinkPartyMember(self, pid, vid, mapIdx, channel, race):
		self.wndParty.LinkPartyMember(pid, vid, mapIdx, channel, race)

	def UnlinkPartyMember(self, pid):
		self.wndParty.UnlinkPartyMember(pid)

	def UnlinkAllPartyMember(self):
		self.wndParty.UnlinkAllPartyMember()

	def ExitParty(self):
		self.wndParty.ExitParty()

		##!! 20061026.levites.???_??_??
		self.__ArrangeQuestButton()

	def PartyHealReady(self):
		self.wndParty.PartyHealReady()

	def ChangePartyParameter(self, distributionMode):
		self.wndParty.ChangePartyParameter(distributionMode)

	## Safebox
	def AskSafeboxPassword(self):
		if self.wndSafebox.IsShow():
			return

		# SAFEBOX_PASSWORD
		self.dlgPassword.SetTitle(localeInfo.PASSWORD_TITLE)
		self.dlgPassword.SetSendMessage("/safebox_password ")
		# END_OF_SAFEBOX_PASSWORD

		self.dlgPassword.ShowDialog()

	## Acce Combine
	def AcceCombineDialog(self):
		if self.wndAcce.IsShow():
			return

		self.wndAcce.Open(0)

	## Acce Combine
	def AcceAbsorbDialog(self):
		if self.wndAcce.IsShow():
			return

		self.wndAcce.Open(1)

	def OpenShopSearch(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if self.wndPrivateShopSearch is not None:
				if not self.wndPrivateShopSearch.IsShow():
					self.wndPrivateShopSearch.Open(1)
				else:
					self.wndPrivateShopSearch.Close()

	def RefreshPrivateShopSearch(self):
		self.wndPrivateShopSearch.RefreshMe()

	def RefreshRequest(self):
		self.wndPrivateShopSearch.RefreshRequest()

	def OpenSafeboxWindow(self, size):
		self.dlgPassword.CloseDialog()
		self.wndSafebox.ShowWindow(size)

	def RefreshSafeboxMoney(self):
		self.wndSafebox.RefreshSafeboxMoney()

	def CommandCloseSafebox(self):
		self.wndSafebox.CommandCloseSafebox()

	def RefreshPShopSearchDialog(self):
		self.wndPrivateShopSearch.RefreshList()

	def AcceDialogOpen(self, type):
		self.wndAcce.Open(type)
		if not self.wndInventory.IsShow():
			self.wndInventory.Show()

	def RefreshAcceWindow(self):
		self.wndAcce.RefreshAcceWindow()

	## HilightSlot Change
	def DeactivateSlot(self, slotindex, type):
		self.wndInventory.DeactivateSlot(slotindex, type)

	## HilightSlot Change
	def ActivateSlot(self, slotindex, type):
		self.wndInventory.ActivateSlot(slotindex, type)

	if app.ENABLE_CHANGE_LOOK_SYSTEM:
		def ChangeWindowOpen(self):
			if self.wndChangeLook:
				self.wndChangeLook.Open()

				if self.dlgRefineNew:
					if self.dlgRefineNew.IsShow():
						self.wndChangeLook.Close()

	if app.ENABLE_MOVE_COSTUME_ATTR:
		def ItemCombinationDialogOpen(self):
			self.wndItemCombination.Open()
			if not self.wndInventory.IsShow():
				self.wndInventory.Show()

	if app.ENABLE_CHANGED_ATTR:
		def OpenSelectAttrDialog(self, window_type, slotIdx):
			if self.wndSelectAttr.IsShow():
				return

			if self.privateShopBuilder.IsShow():
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SELECT_ATTR_NOT_OPEN)
				return

			self.wndSelectAttr.Open(window_type, slotIdx)
			self.wndSelectAttr.Show()

	# ITEM_MALL
	def AskMallPassword(self):
		if self.wndMall.IsShow():
			return
		self.dlgPassword.SetTitle(localeInfo.MALL_PASSWORD_TITLE)
		self.dlgPassword.SetSendMessage("/mall_password ")
		self.dlgPassword.ShowDialog()

	def OpenMallWindow(self, size):
		self.dlgPassword.CloseDialog()
		self.wndMall.ShowWindow(size)

	def CommandCloseMall(self):
		self.wndMall.CommandCloseMall()

	# END_OF_ITEM_MALL

	## Guild
	def OnStartGuildWar(self, guildSelf, guildOpp):
		self.wndGuild.OnStartGuildWar(guildSelf, guildOpp)

		guildWarScoreBoard = uiGuild.GuildWarScoreBoard()
		guildWarScoreBoard.Open(guildSelf, guildOpp)
		guildWarScoreBoard.Show()
		self.guildScoreBoardDict[uiGuild.GetGVGKey(guildSelf, guildOpp)] = guildWarScoreBoard

	def OnEndGuildWar(self, guildSelf, guildOpp):
		self.wndGuild.OnEndGuildWar(guildSelf, guildOpp)

		key = uiGuild.GetGVGKey(guildSelf, guildOpp)

		if key not in self.guildScoreBoardDict:
			return

		self.guildScoreBoardDict[key].Destroy()
		del self.guildScoreBoardDict[key]

	# GUILDWAR_MEMBER_COUNT
	def UpdateMemberCount(self, gulidID1, memberCount1, guildID2, memberCount2):
		key = uiGuild.GetGVGKey(gulidID1, guildID2)

		if key not in self.guildScoreBoardDict:
			return

		self.guildScoreBoardDict[key].UpdateMemberCount(gulidID1, memberCount1, guildID2, memberCount2)

	# END_OF_GUILDWAR_MEMBER_COUNT

	### UPDATE NATION WAR
	def UpdateNationScoreCount(self, empire1, empire2, empire3):
		if empire1 < 0:
			self.wndNation.empire1.Hide()
			self.wndNation.empiretext1.SetTextColor(0xfff03232)
		elif empire1 >= 100:
			self.wndNation.empire1.SetTextColor(0xff08cc15)
		else:
			self.wndNation.empire1.SetTextColor(0xffffffff)

		if empire2 < 0:
			self.wndNation.empire2.Hide()
			self.wndNation.empiretext2.SetTextColor(0xfff03232)
		elif empire2 >= 100:
			self.wndNation.empire2.SetTextColor(0xff08cc15)
		else:
			self.wndNation.empire2.SetTextColor(0xffffffff)

		if empire3 < 0:
			self.wndNation.empire3.Hide()
			self.wndNation.empiretext3.SetTextColor(0xfff03232)
		elif empire3 >= 100:
			self.wndNation.empire3.SetTextColor(0xff08cc15)
		else:
			self.wndNation.empire3.SetTextColor(0xffffffff)
		self.wndNation.empire1.SetText(str(empire1))
		self.wndNation.empire2.SetText(str(empire2))
		self.wndNation.empire3.SetText(str(empire3))

	def UpdateLivesCount(self, lives):
		if lives < 1:
			self.wndNation.live.SetTextColor(0xfff03232)
		elif lives > 10:
			self.wndNation.live.SetTextColor(0xffF8BF24)
		else:
			self.wndNation.live.SetTextColor(0xffffffff)
		self.wndNation.live.SetText(str(lives))

	### UPDATE NATION WAR

	def OnRecvGuildWarPoint(self, gainGuildID, opponentGuildID, point):
		key = uiGuild.GetGVGKey(gainGuildID, opponentGuildID)
		if key not in self.guildScoreBoardDict:
			return

		guildBoard = self.guildScoreBoardDict[key]
		guildBoard.SetScore(gainGuildID, opponentGuildID, point)

	## PK Mode
	def OnChangePKMode(self):
		if self.wndCharacter:
			self.wndCharacter.RefreshAlignment()
		if self.dlgSystem:
			self.dlgSystem.OnChangePKMode()

	## Refine
	def OpenRefineDialog(self, targetItemPos, nextGradeItemVnum, cost, prob, type):
		self.dlgRefineNew.Open(targetItemPos, nextGradeItemVnum, cost, prob, type)

	def AppendMaterialToRefineDialog(self, vnum, count):
		self.dlgRefineNew.AppendMaterial(vnum, count)

	def AppendEnhanceMaterialToRefineDialog(self, vnum, count, prob):
		self.dlgRefineNew.AppendEnhanceMaterial(vnum, count, prob)

	## Show & Hide
	def ShowDefaultWindows(self):
		self.wndTaskBar.Show()
		self.wndMiniMap.Show()
		self.wndMiniMap.ShowMiniMap()

		if self.wndParty:
			self.wndParty.Show()

		# if app.ENABLE_12ZI:
		#    if self.wndBead:
		#        self.wndBead.Show()

		if self.wndChat:
			self.wndChat.Show()

#Envanter ile birlikte açılması ve kapanması için default olarak kapalı geliyor @Adalet
		# if self.wndExpandedMoneyTaskbar:
			# self.wndExpandedMoneyTaskbar.Show()

	def ShowAllWindows(self):
		self.wndTaskBar.Show()
		self.wndCharacter.Show()
		if not self.wndInventory.IsShow():
			self.wndInventory.Show()
		if self.wndExpandedMoneyTaskbar:
			self.wndExpandedMoneyTaskbar.Show()
		self.wndDragonSoul.Show()
		self.wndDragonSoulRefine.Show()
		self.wndChat.Show()
		self.wndMiniMap.Show()
		# if app.ENABLE_12ZI:
		# if self.wndBead:
		#    self.wndBead.Show()
		# if self.wnd12ziTimer:
		#    self.wnd12ziTimer.Show()
		# if self.wnd12ziReward:
		#    self.wnd12ziReward.Show()

		# PET_SYSTEM
		# self.wndPet.Show()
		# END_OF_PET_SYSTEM
		# if app.ENABLE_GEM_SYSTEM:
		# if self.wndExpandedMoneyTaskbar:
		#   self.wndExpandedMoneyTaskbar.Show()


		if self.wndExpandedTaskBar:
			self.wndExpandedTaskBar.Show()
			self.wndExpandedTaskBar.SetTop()

	def HideAllWindows(self):
		OfflineShopManager.GetInstance().HideAllWindows()

		if self.wndParty:
			self.wndParty.Hide()

		if self.wndTaskBar:
			self.wndTaskBar.Hide()

		if self.wndExpandedMoneyTaskbar:
			self.wndExpandedMoneyTaskbar.Hide()

		if self.wndCharacter:
			self.wndCharacter.Hide()

		if self.wndPrivateShopSearch:
			self.wndPrivateShopSearch.Hide()

		if self.wndSkillTree:
			self.wndSkillTree.Hide()

		if self.wndWiki:
			self.wndWiki.Hide()
		# if self.wndPet:
		#    self.wndPet.Hide()

		# if self.wndSlots:
		#    self.wndSlots.Hide()
		# if self.wndPetEvolved:
		#     self.wndPetEvolved.Hide()

		if self.wndInventory:
			self.wndInventory.Hide()

		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			self.wndDragonSoul.Hide()
			self.wndDragonSoulRefine.Hide()

		if app.ENABLE_GROWTH_PET_SYSTEM:
			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.Hide()

		if app.ENABLE_12ZI:
			if self.wndBead:
				self.wndBead.Hide()
			if self.wnd12ziTimer:
				self.wnd12ziTimer.Hide()
			if self.wnd12ziReward:
				self.wnd12ziReward.Hide()

		if self.wndChat:
			self.wndChat.Hide()

		if self.wndMiniMap:
			self.wndMiniMap.Hide()

		if self.wndMessenger:
			self.wndMessenger.Hide()

		if self.wndGuild:
			self.wndGuild.Hide()

		if self.wndExpandedTaskBar:
			self.wndExpandedTaskBar.Hide()

		if app.ENABLE_SKILLBOOK_COMBINATION:
			if self.wndSkillBookCombination:
				self.wndSkillBookCombination.Hide()

		if app.ENABLE_CHANGED_ATTR:
			if self.wndSelectAttr:
				self.wndSelectAttr.Hide()

		if self.wndMyShopDeco:
			self.wndMyShopDeco.Hide()

		if self.dlgSystem:
			if self.dlgSystem.SwitchChannelDlg:
				self.dlgSystem.SwitchChannelDlg.Hide()

	def ShowMouseImage(self):
		self.wndTaskBar.ShowMouseImage()

	def HideMouseImage(self):
		self.wndTaskBar.HideMouseImage()

	def ToggleChat(self):
		if self.wndChat.IsFullMode():
			self.wndChat.CloseChat()
		else:
			# 웹페이지가 열렸을때는 채팅 입력이 안됨
			if self.wndWeb and self.wndWeb.IsShow():
				pass
			else:
				self.wndChat.OpenChat()

	def IsOpenChat(self):
		return self.wndChat.IsFullMode()

	def SetChatFocus(self):
		self.wndChat.SetChatFocus()

	def OpenRestartDialog(self):
		self.dlgRestart.OpenDialog()
		self.dlgRestart.SetTop()

	if app.ENABLE_12ZI:
		def OpenUI12zi(self, yellowmark, greenmark, yellowreward, greenreward, goldreward):
			if self.wnd12ziReward == None:
				self.wnd12ziReward = ui12zi.Reward12ziWindow()
			self.wnd12ziReward.Open(yellowmark, greenmark, yellowreward, greenreward, goldreward)

		def Refresh12ziTimer(self, currentFloor, jumpCount, limitTime, elapseTime):
			if self.wndMiniMap:
				self.wndMiniMap.Hide()

			if self.wnd12ziTimer == None:
				self.wnd12ziTimer = ui12zi.FloorLimitTimeWindow()

			self.wnd12ziTimer.Refresh12ziTimer(currentFloor, jumpCount, limitTime, elapseTime)
			self.wnd12ziTimer.Open()

		def Show12ziJumpButton(self):
			self.wnd12ziTimer.Show12ziJumpButton()

		def Hide12ziTimer(self):
			self.wnd12ziTimer.Hide()

		def RefreshShopItemToolTip(self):
			if self.tooltipItem:
				self.tooltipItem.RefreshShopToolTip()

	def CloseRestartDialog(self):
		self.dlgRestart.Close()

	def ToggleSystemDialog(self):
		if False == self.dlgSystem.IsShow():
			self.dlgSystem.OpenDialog()
			self.dlgSystem.SetTop()
		else:
			self.dlgSystem.Close()

	def IsSystemDialogOpen(self):
		return self.dlgSystem.IsShow()

	def OpenSystemDialog(self):
		self.dlgSystem.OpenDialog()
		self.dlgSystem.SetTop()

	def ToggleMessenger(self):
		if self.wndMessenger.IsShow():
			self.wndMessenger.Hide()
		else:
			self.wndMessenger.SetTop()
			self.wndMessenger.Show()

	def ToggleMiniMap(self):
		if app.IsPressed(app.VK_MENU):
			if not self.wndMiniMap.isShowMiniMap():
				self.wndMiniMap.ShowMiniMap()
				self.wndMiniMap.SetTop()
			else:
				self.wndMiniMap.HideMiniMap()
		else:
			self.wndMiniMap.ToggleAtlasWindow()

	def PressMKey(self):
		if app.IsPressed(app.VK_MENU) or app.IsPressed(app.VK_MENU):
			self.ToggleMessenger()
		else:
			self.ToggleMiniMap()

	def SetMapName(self, mapName):
		self.wndMiniMap.SetMapName(mapName)

	def MiniMapScaleUp(self):
		self.wndMiniMap.ScaleUp()

	def MiniMapScaleDown(self):
		self.wndMiniMap.ScaleDown()

	def MiniMapUpdateServerInfo(self, text):
		self.wndMiniMap.UpdateServerInfo(text)

	def ToggleCharacterWindow(self, state):
		if not player.IsObserverMode() or player.IsGameMaster():
			if False == self.wndCharacter.IsShow():
				self.OpenCharacterWindowWithState(state)
			else:
				if state == self.wndCharacter.GetState():
					self.wndCharacter.OverOutItem()
					self.wndCharacter.Close()
				else:
					self.wndCharacter.SetState(state)

	def OpenCharacterWindowWithState(self, state):
		if not player.IsObserverMode() or player.IsGameMaster():
			self.wndCharacter.SetState(state)
			self.wndCharacter.Show()
			self.wndCharacter.SetTop()

	def ToggleCharacterWindowStatusPage(self):
		self.ToggleCharacterWindow("STATUS")

	def ToggleSwitchBot(self):
		if self.wndInventory:
			self.wndInventory.ToggleSwitchbotButton()

	def ToggleSkillTree(self):
		if self.wndSkillTree:
			if self.wndSkillTree.IsShow():
				self.wndSkillTree.Hide()
			else:
				self.wndSkillTree.Open()

	def ToggleWikiWindow(self):
		if self.wndWiki:
			if self.wndWiki.IsShow():
				self.wndWiki.Hide()
			else:
				self.wndWiki.Show()

	def ToggleNationWindow(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if False == self.wndNation.IsShow():
				self.wndNation.Show()
				self.wndNation.SetTop()
				# else:
				# self.wndNation.Hide()

#Moneytaskbar Envanter ile birlikte açılsın düzeltme @Adalet
	def ToggleInventoryWindow(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if not self.wndInventory.IsShow():
				self.wndInventory.Show()
				self.wndInventory.SetTop()
				self.wndExpandedMoneyTaskbar.Show()
			else:
				self.wndInventory.OverOutItem()
				self.wndInventory.Close()
				self.wndExpandedMoneyTaskbar.Hide()

	def ToggleExpandedButton(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if False == self.wndExpandedTaskBar.IsShow():
				self.wndExpandedTaskBar.Show()
				self.wndExpandedTaskBar.SetTop()
			else:
				self.wndExpandedTaskBar.Close()

	def IsShowDlgQuestionWindow(self):
		if self.wndInventory.IsDlgQuestionShow():
			return True
		elif self.wndDragonSoul.IsDlgQuestionShow():
			return True
		else:
			return False

	def CloseDlgQuestionWindow(self):
		if self.wndInventory.IsDlgQuestionShow():
			self.wndInventory.CancelDlgQuestion()
		if self.wndDragonSoul.IsDlgQuestionShow():
			self.wndDragonSoul.CancelDlgQuestion()

	def SetUseItemMode(self, bUse):
		self.wndInventory.SetUseItemMode(bUse)
		self.wndDragonSoul.SetUseItemMode(bUse)

	def DragonSoulSetGrade(self, grade):
		self.wndDragonSoul.SetDSSetGrade(grade)

	def DragonSoulActivate(self, deck):
		self.wndDragonSoul.ActivateDragonSoulByExtern(deck)

	def DragonSoulActivateByKey(self):
		self.wndDragonSoul.ActivateButtonClickByKey()

	def DragonSoulDeactivate(self):
		self.wndDragonSoul.DeactivateDragonSoul()

	def SetCantMouseSlot(self, slotPos):
		if self.wndInventory:
			self.wndInventory.SetCantMouseEvent(slotPos)

	def SetCanMouseSlot(self, slotPos):
		if self.wndInventory:
			self.wndInventory.SetCanMouseEvent(slotPos)

	def Highligt_Item(self, inven_type, inven_pos):
		if player.DRAGON_SOUL_INVENTORY == inven_type:
			if app.ENABLE_DRAGON_SOUL_SYSTEM:
				self.wndDragonSoul.HighlightSlot(inven_pos)
		elif app.WJ_ENABLE_PICKUP_ITEM_EFFECT and player.INVENTORY == inven_type:
			self.wndInventory.HighlightSlot(inven_pos)

	def ClearInventoryHighlight(self):
		if self.wndInventory:
			self.wndInventory.HighlightSlot_Clear()

	def DragonSoulGiveQuilification(self):
		self.DRAGON_SOUL_IS_QUALIFIED = True
		if uiTaskBar.TaskBar.IS_EXPANDED:
			self.wndExpandedTaskBar.SetToolTipText(uiTaskBar.ExpandedTaskBar.BUTTON_DRAGON_SOUL,
												   uiScriptLocale.TASKBAR_DRAGON_SOUL)

	def ToggleDragonSoulWindow(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if not self.wndDragonSoul.IsShow():
				if app.ENABLE_DS_PASSWORD:
					self.wndDragonSoul.Open()
				else:
					self.wndDragonSoul.Show()
			else:
				self.wndDragonSoul.Close()

	def ToggleDragonSoulWindowWithNoInfo(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if False == self.wndDragonSoul.IsShow():
				if app.ENABLE_DS_PASSWORD:
					self.wndDragonSoul.Open()
				else:
					self.wndDragonSoul.Show()
			else:
				self.wndDragonSoul.Close()

	def FailDragonSoulRefine(self, reason, inven_type, inven_pos):
		if not player.IsObserverMode() or player.IsGameMaster():
			if app.ENABLE_DRAGON_SOUL_SYSTEM:
				if True == self.wndDragonSoulRefine.IsShow():
					self.wndDragonSoulRefine.RefineFail(reason, inven_type, inven_pos)

	def SucceedDragonSoulRefine(self, inven_type, inven_pos):
		if not player.IsObserverMode() or player.IsGameMaster():
			if app.ENABLE_DRAGON_SOUL_SYSTEM:
				if True == self.wndDragonSoulRefine.IsShow():
					self.wndDragonSoulRefine.RefineSucceed(inven_type, inven_pos)

	def OpenDragonSoulRefineWindow(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if app.ENABLE_DRAGON_SOUL_SYSTEM:
				if False == self.wndDragonSoulRefine.IsShow():
					self.wndDragonSoulRefine.Show()
					if None != self.wndDragonSoul:
						if False == self.wndDragonSoul.IsShow():
							self.wndDragonSoul.Show()

	def CloseDragonSoulRefineWindow(self):
		if not player.IsObserverMode() or player.IsGameMaster():
			if app.ENABLE_DRAGON_SOUL_SYSTEM:
				if True == self.wndDragonSoulRefine.IsShow():
					self.wndDragonSoulRefine.Close()

	def ClearBattlePass(self):
		if self.wndBattlePass:
			self.wndBattlePass.ClearMissions()
			self.wndBattlePass.Close()
			self.wndBattlePass.Hide()

	if app.ENABLE_BATTLE_PASS:
		def OpenBattlePass(self):
			if False == player.IsObserverMode():
				if not self.wndBattlePass.IsShow():
					self.wndBattlePass.Open()
					self.wndBattlePassButton.CompleteLoading()
				else:
					self.wndBattlePass.Close()

		def AddBattlePassMission(self, missionType, missionInfo1, missionInfo2, missionInfo3):
			if self.wndBattlePass:
				self.wndBattlePass.AddMission(missionType, missionInfo1, missionInfo2, missionInfo3)

		def UpdateBattlePassMission(self, missionType, newProgress):
			if self.wndBattlePass:
				self.wndBattlePass.UpdateMission(missionType, newProgress)

		def AddBattlePassMissionReward(self, missionType, itemVnum, itemCount):
			if self.wndBattlePass:
				self.wndBattlePass.AddMissionReward(missionType, itemVnum, itemCount)

		def AddBattlePassReward(self, itemVnum, itemCount):
			if self.wndBattlePass:
				self.wndBattlePass.AddReward(itemVnum, itemCount)

		def AddBattlePassRanking(self, pos, playerName, finishTime):
			if self.wndBattlePass:
				self.wndBattlePass.AddRanking(pos, playerName, finishTime)

		def RefreshBattlePassRanking(self):
			if self.wndBattlePass:
				self.wndBattlePass.RefreshRanking()

		def OpenBattlePassRanking(self):
			if self.wndBattlePass:
				self.wndBattlePass.OpenRanking()

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetInformationActivate(self):
			self.wndExpandedTaskBar.SetToolTipText(uiTaskBar.ExpandedTaskBar.BUTTON_PET_INFO,
												   uiScriptLocale.TASKBAR_PET_INFO)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def TogglePetInformationWindow(self):
			if not player.IsObserverMode() or player.IsGameMaster():
				if not self.wndPetInfoWindow.IsShow():
					self.wndPetInfoWindow.Show()
				else:
					self.wndPetInfoWindow.Close()
				return True
			return False

	# ??? ?
	def ToggleDungeonInfoWindow(self, justOpen = False):
		if not justOpen and dungeonInfo.IsNeedRequest():
			appInst.instance().GetNet().SendRequestDungeonInfo()
			print "Send request to server. Info vector is empty."
			return

		if False == player.IsObserverMode():
			if justOpen:
				self.wndDungeonInfoWindow.Open()
			else:
				print "Open from client. Already have info."
				if self.wndDungeonInfoWindow.IsShow():
					self.wndDungeonInfoWindow.Close()
				else:
					self.wndDungeonInfoWindow.Open()

	def RefreshDungeonInfo(self):
		if self.wndDungeonInfoWindow:
			self.wndDungeonInfoWindow.RefreshButtonInfo()

	def ToggleDungeonRankingWindow(self, dungeonID, rankType, justOpen = False):
		if not justOpen and dungeonInfo.IsRankingNeedRequest(dungeonID, rankType):
			appInst.instance().GetNet().SendRequestDungeonRanking(dungeonID, rankType)
			return

		if False == player.IsObserverMode():
			if justOpen:
				if self.wndDungeonRankingWindow.IsShow():
					self.wndDungeonRankingWindow.RefreshData(dungeonID, rankType)
				else:
					self.wndDungeonRankingWindow.Open(dungeonID, rankType)
			else:
				if self.wndDungeonRankingWindow.IsShow():
					self.wndDungeonRankingWindow.RefreshData(dungeonID, rankType)
				else:
					self.wndDungeonRankingWindow.Open(dungeonID, rankType)

	def ToggleGuildWindow(self):
		if not self.wndGuild.IsShow():
			if self.wndGuild.CanOpen():
				self.wndGuild.SetSkillToolTip(self.tooltipSkill)
				self.wndGuild.Open()
			else:
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.GUILD_YOU_DO_NOT_JOIN)
		else:
			self.wndGuild.OverOutItem()
			self.wndGuild.Hide()

	def ToggleChatLogWindow(self):
		if self.wndChatLog.IsShow():
			self.wndChatLog.Hide()
		else:
			self.wndChatLog.Show()

	def ToggleQuestButtons(self):
		if not Interface.hideQuestButtons:
			Interface.hideQuestButtons = 1
			self.HideAllQuestButton()
		else:
			Interface.hideQuestButtons = 0
			self.ShowAllQuestButton()

	def ToggleQuestWindow(self):
		self.ToggleCharacterWindow("QUEST")

	def CheckGameButton(self):
		if self.wndGameButton:
			self.wndGameButton.CheckGameButton()

	def __OnClickStatusPlusButton(self):
		self.ToggleCharacterWindow("STATUS")

	def __OnClickSkillPlusButton(self):
		self.ToggleCharacterWindow("SKILL")

	def __OnClickQuestButton(self):
		self.ToggleCharacterWindow("QUEST")

	def __OnClickBuildButton(self):
		self.BUILD_OpenWindow()

	def OpenWebWindow(self, url):
		if not self.wndWeb:
			self.__MakeWebWindow()

		if self.wndWeb:
			self.wndWeb.Open(url)
			self.wndChat.CloseChat()

	def HideViewingChat(self):
		self.wndChat.HideViewingChat()

	def ShowViewingChat(self):
		self.wndChat.ShowViewingChat()

		# show GIFT

	def ShowGift(self):
		self.wndTaskBar.ShowGift()

	def CreateSkillGroupSelect(self):
		SKILL_GROUP_NAME_DICT = {
			playerSettingModule.JOB_WARRIOR: {1: localeInfo.SKILL_GROUP_WARRIOR_1,
											  2: localeInfo.SKILL_GROUP_WARRIOR_2, },
			playerSettingModule.JOB_ASSASSIN: {1: localeInfo.SKILL_GROUP_ASSASSIN_1,
											   2: localeInfo.SKILL_GROUP_ASSASSIN_2, },
			playerSettingModule.JOB_SURA: {1: localeInfo.SKILL_GROUP_SURA_1, 2: localeInfo.SKILL_GROUP_SURA_2, },
			playerSettingModule.JOB_SHAMAN: {1: localeInfo.SKILL_GROUP_SHAMAN_1, 2: localeInfo.SKILL_GROUP_SHAMAN_2, },
			playerSettingModule.JOB_WOLFMAN: {1: localeInfo.JOB_WOLFMAN1, 2: localeInfo.JOB_WOLFMAN2, },
		}

		race = player.GetRace()
		job = chr.RaceToJob(race)

		if job not in SKILL_GROUP_NAME_DICT:
			return

		nameList = SKILL_GROUP_NAME_DICT[job]

		q = uiCommon.QuestionDialog4()
		q.SetText("")
		q.SetCancelText(localeInfo.UI_CANCEL)
		q.SetAccept1Text(nameList[1])
		q.SetAccept2Text(nameList[2])
		q.SetAccept1Event(Event(self.SelectSkillGroupEvent, 1))
		q.SetAccept2Event(Event(self.SelectSkillGroupEvent, 2))
		q.SetCancelEvent(Event(self.SelectSkillGroupEvent, 0))

		self.wndSkillGroupSelect = q

	def OpenSkillGroupSelect(self):
		if not self.wndSkillGroupSelect:
			self.CreateSkillGroupSelect()
			self.wndSkillGroupSelect.Show()
			return

		self.wndSkillGroupSelect.Show()

	def SelectSkillGroupEvent(self, skillGroup):
		if skillGroup > 0:
			appInst.instance().GetNet().SendSkillGroupSelectPacket(skillGroup)
		self.wndSkillGroupSelect.Close()
		self.wndSkillGroupSelect = None

	def CloseWebWindow(self):
		self.wndWeb.Close()

	if app.ENABLE_GEM_SYSTEM:
		def OpenGemShop(self):
			if self.wndGemShop:
				self.wndGemShop.Open()

		def CloseGemShop(self):
			if self.wndGemShop:
				self.wndGemShop.Close()

		def RefreshGemShopWIndow(self):
			if self.wndGemShop:
				self.wndGemShop.RefreshGemShopWIndow()

		def GemShopSlotBuy(self, slotindex, enable):
			if self.wndGemShop:
				self.wndGemShop.GemShopSlotBuy(slotindex, enable)

		def GemShopSlotAdd(self, slotindex, enable):
			if self.wndGemShop:
				self.wndGemShop.GemShopSlotAdd(slotindex, enable)

	def OpenCubeWindow(self):
		self.wndCube.Open()

		if not self.wndInventory.IsShow():
			self.wndInventory.Show()

	def CloseCubeWindow(self):
		self.wndCube.Close()

	def IsCubeWindowOpen(self):
		return self.wndCube and self.wndCube.IsShow()

	def FailedCubeWork(self):
		pass

	def SucceedCubeWork(self, itemVnum, count):
		pass

	def __HideWindows(self):
		hideWindows = self.wndTaskBar, \
					  self.wndCharacter, \
					  self.wndInventory, \
					  self.wndMiniMap, \
					  self.wndGuild, \
					  self.wndMessenger, \
					  self.wndChat, \
					  self.wndParty, \
					  self.wndGameButton,

		if self.wndExpandedTaskBar:
			hideWindows += self.wndExpandedTaskBar,

		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			hideWindows += self.wndDragonSoul, \
						   self.wndDragonSoulRefine,

		if app.ENABLE_BATTLE_PASS:
			if self.wndBattlePass and self.wndBattlePassButton:
				hideWindows += self.wndBattlePass,
				hideWindows += self.wndBattlePassButton,

		if self.wndDungeonInfoWindow:
			hideWindows += self.wndDungeonInfoWindow,

		if self.wndDungeonRankingWindow:
			hideWindows += self.wndDungeonRankingWindow,

		if app.ENABLE_GROWTH_PET_SYSTEM:
			if self.wndPetInfoWindow:
				hideWindows += self.wndPetInfoWindow,

		if self.wndDungeonCoolTime:
			hideWindows += self.wndDungeonCoolTime,

		if self.wndPrivateShopSearch:
			hideWindows += self.wndPrivateShopSearch,

		if app.ENABLE_12ZI:
			if self.wndBead:
				hideWindows += self.wndBead,
			if self.wnd12ziTimer:
				hideWindows += self.wnd12ziTimer,
			if self.wnd12ziReward:
				hideWindows += self.wnd12ziReward,

		if app.ENABLE_MYSHOP_DECO:
			if self.wndMyShopDeco:
				hideWindows += self.wndMyShopDeco,

		if self.wndExpandedMoneyTaskbar:
			hideWindows += self.wndExpandedMoneyTaskbar,

		if self.wndSkillTree:
			hideWindows += self.wndSkillTree,

		if self.wndWiki:
			hideWindows += self.wndWiki,

		if self.wndWorldBossInfo:
			hideWindows += self.wndWorldBossInfo,

		if self.wndHuntingMissions:
			hideWindows += self.wndHuntingMissions,

		hideWindows = filter(lambda x: x.IsShow(), hideWindows)
		map(lambda x: x.Hide(), hideWindows)

		self.HideAllQuestButton()
		self.whisper.HideAll()

		if self.wndChat.IsFullMode():
			self.wndChat.CloseChat()

		self.wndMiniMap.AtlasWindow.Hide()

		return hideWindows

	def __ShowWindows(self, wnds):

		map(lambda x: x.Show(), wnds)
		global IsQBHide
		if not IsQBHide:
			self.ShowAllQuestButton()
		else:
			self.HideAllQuestButton()

		self.whisper.ShowAll()

		if self.wndMiniMap.AtlasWindow and self.wndMiniMap.AtlasWindow.IsShowWindow():
			self.wndMiniMap.AtlasWindow.Show()

	def BINARY_OpenAtlasWindow(self):
		if self.wndMiniMap:
			self.wndMiniMap.ShowAtlas()

	def BINARY_SetObserverMode(self, flag):
		self.wndGameButton.SetObserverMode(flag, True)

	# ACCESSORY_REFINE_ADD_METIN_STONE
	def BINARY_OpenSelectItemWindow(self):
		self.wndItemSelect.Open()

	if app.ENABLE_GEM_SYSTEM:
		def BINARY_OpenSelectItemWindowEx(self):
			self.wndItemSelectEx.Open()

		def BINARY_RefreshSelectItemWindowEx(self):
			self.wndItemSelectEx.RefreshSlot()

	# END_OF_ACCESSORY_REFINE_ADD_METIN_STONE

	#####################################################################################
	### Private Shop ###

	def OpenPrivateShopInputNameDialog(self, isCashItem=False, tabCount=1):

		if self.wndMyShopDeco.IsShow():
			return

		if app.ENABLE_GROWTH_PET_SYSTEM:
			pet_id = player.GetActivePetItemId()
			if pet_id:
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_OPEN_SHOP_BECAUSE_SUMMON)
				return

		if app.ENABLE_SKILLBOOK_COMBINATION:
			if self.wndSkillBookCombination.IsShow():
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.COMB_NOTICE)
				return

		if app.ENABLE_CHANGED_ATTR:
			if self.wndSelectAttr.IsShow():
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SELECT_ATTR_NOTICE)
				return

		#if app.ENABLE_CHANGE_LOOK_SYSTEM:
		#    shop.SetNameDialogOpen(True)

		inputDialog = uiCommon.InputDialog()
		inputDialog.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_NAME_DIALOG_TITLE)
		inputDialog.SetMaxLength(32)

		inputDialog.SetAcceptEvent(Event(self.OpenPrivateShopBuilder, [isCashItem, tabCount]))
		inputDialog.SetCancelEvent(self.ClosePrivateShopInputNameDialog)
		inputDialog.Open()

		self.inputDialog = inputDialog

	def ClosePrivateShopInputNameDialog(self):
		self.inputDialog = None
		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			shop.SetNameDialogOpen(False)
		return True

	def OpenPrivateShopBuilder(self, isCashItem=False, tabCount=0):
		if not self.inputDialog:
			return True

		if not len(self.inputDialog.GetText()):
			return True

		if app.ENABLE_GROWTH_PET_SYSTEM:
			pet_id = player.GetActivePetItemId()
			if pet_id:
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_OPEN_SHOP_BECAUSE_SUMMON)
				return

		if app.ENABLE_SKILLBOOK_COMBINATION:
			if self.wndSkillBookCombination.IsShow():
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.COMB_NOTICE)
				return

		if app.ENABLE_CHANGED_ATTR:
			if self.wndSelectAttr.IsShow():
				chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SELECT_ATTR_NOTICE)
				return

		self.privateShopBuilder.Open(self.inputDialog.GetText())
		# self.privateShopBuilder.SetIsCashItem(isCashItem)
		# self.privateShopBuilder.SetTabCount(tabCount)
		# if app.ENABLE_CHANGE_LOOK_SYSTEM:
		#    self.SetNameDialogOpen()
		self.ClosePrivateShopInputNameDialog()
		if app.ENABLE_CHANGE_LOOK_SYSTEM:
			shop.SetNameDialogOpen(True)

		return True

	def AppearPrivateShop(self, vid, text, type):
		pass

	def DisappearPrivateShop(self, vid):
		if vid not in self.privateShopAdvertisementBoardDict:
			return

		del self.privateShopAdvertisementBoardDict[vid]
		uiPrivateShopBuilder.DeleteADBoard(vid)

	#####################################################################################
	### Equipment ###

	def OpenEquipmentDialog(self, vid):
		dlg = uiEquipmentDialog.EquipmentDialog()
		dlg.SetItemToolTip(self.tooltipItem)
		dlg.SetCloseEvent(self.CloseEquipmentDialog)
		dlg.Open(vid)

		self.equipmentDialogDict[vid] = dlg

	if app.ENABLE_CHANGE_LOOK_SYSTEM:
		def SetEquipmentDialogItem(self, vid, slotIndex, vnum, count, dwChangeLookVnum):
			if not vid in self.equipmentDialogDict:
				return
			self.equipmentDialogDict[vid].SetEquipmentDialogItem(slotIndex, vnum, count, dwChangeLookVnum)
	else:
		def SetEquipmentDialogItem(self, vid, slotIndex, vnum, count):
			if not vid in self.equipmentDialogDict:
				return
			self.equipmentDialogDict[vid].SetEquipmentDialogItem(slotIndex, vnum, count)

	def SetEquipmentDialogSocket(self, vid, slotIndex, socketIndex, value):
		if not vid in self.equipmentDialogDict:
			return

		self.equipmentDialogDict[vid].SetEquipmentDialogSocket(slotIndex, socketIndex, value)

	def SetEquipmentDialogAttr(self, vid, slotIndex, attrIndex, type, value):
		if not vid in self.equipmentDialogDict:
			return

		self.equipmentDialogDict[vid].SetEquipmentDialogAttr(slotIndex, attrIndex, type, value)

	def CloseEquipmentDialog(self, vid):
		if not vid in self.equipmentDialogDict:
			return

		del self.equipmentDialogDict[vid]

	#####################################################################################

	#####################################################################################
	### Quest ###
	def BINARY_ClearQuest(self, index):
		btn = self.__FindQuestButton(index)
		if 0 != btn:
			self.__DestroyQuestButton(btn)

	def RecvQuest(self, index, name):
		# QUEST_LETTER_IMAGE
		# print "Received quest index: ["+str(index)+"], name: ["+name+"]"
		self.BINARY_RecvQuest(index, name, "file", localeInfo.GetLetterImageName())

	# END_OF_QUEST_LETTER_IMAGE
	# Q?? ??

	def BINARY_RecvQuest(self, index, name, iconType, iconName):

		btn = self.__FindQuestButton(index)
		if 0 != btn:
			self.__DestroyQuestButton(btn)

		btn = ui.Button("TOP_MOST")
		btn.SetWindowName("QuestButton")

		if "item" == iconType:
			item.SelectItem(int(iconName))
			buttonImageFileName = item.GetIconImageFileName()
		else:
			buttonImageFileName = iconName

		if "highlight" == iconType:
			btn.SetUpVisual("d:/ymir work/ui/highlighted_quest.tga")
			btn.SetOverVisual("d:/ymir work/ui/highlighted_quest_r.tga")
			btn.SetDownVisual("d:/ymir work/ui/highlighted_quest_r.tga")
		else:
			if iconName is not "":
				btn.SetUpVisual(buttonImageFileName)
				btn.SetOverVisual(buttonImageFileName)
				btn.SetDownVisual(buttonImageFileName)

				if iconType == "info":
					imageWidth = btn.GetButtonImageWidth()
					imageHeight = btn.GetButtonImageHeight()
					btn.SetButtonScale(64 * 1.0 / imageWidth, 32 * 1.0 / imageHeight)

			else:
				btn.SetUpVisual(localeInfo.GetLetterCloseImageName())
				btn.SetOverVisual(localeInfo.GetLetterOpenImageName())
				btn.SetDownVisual(localeInfo.GetLetterOpenImageName())

		btn.Flash()

		app.FlashApplication()

		# END_OF_QUEST_LETTER_IMAGE

		btnText = ui.TextLine()
		btnText.SetWindowName("QuestBtnText")
		btnText.SetParent(btn)
		btnText.SetVerticalAlignTop()
		btnText.SetHorizontalAlignLeft()
		btnText.SetOutline()
		btnText.SetPosition(0, 40)
		btnText.SetText(name)
		btnText.Show()

		btn.ClearText()
		btn.AppendTextWindow(btnText)

		btn.SetEvent(Event(self.__StartQuest, btn))
		btn.SetOverEvent(Event(self.__QuestButtonOver, btn))
		btn.Show()
		# ??
		btn.index = index
		btn.name = name

		# self.wndGameButton.ShowQuestButton()
		self.wndCharacter.ReceiveNewQuest(index)

		self.questButtonList.insert(0, btn)
		self.__ArrangeQuestButton()

		# chat.AppendChat(CHAT_TYPE_NOTICE, localeInQUEST_APPEND % name)

	def __QuestButtonOver(self, btn):
		btn.DisableFlash()

	def __ArrangeQuestButton(self):
		screenHeight = wndMgr.GetScreenHeight()

		if self.wndParty.IsShow():
			xPos = 130
		else:
			xPos = 20

		yPos = 170 * screenHeight / 600
		yCount = (screenHeight - 330) / 63

		count = 0
		for btn in self.questButtonList:
			btn.SetPosition(xPos + (int(count / yCount) * 100), yPos + (count % yCount * 63))
			count += 1
			global IsQBHide
			if IsQBHide:
				btn.Hide()
			else:
				btn.Show()

	def __StartQuest(self, btn):
		event.QuestButtonClick(btn.index)
		self.__DestroyQuestButton(btn)

	def __FindQuestButton(self, index):
		for btn in self.questButtonList:
			if btn.index == index:
				return btn

		return 0

	def __DestroyQuestButton(self, btn):
		btn.SetEvent(None)
		btn.SetOverEvent(None)
		self.questButtonList.remove(btn)
		self.__ArrangeQuestButton()

	def HideAllQuestButton(self):
		for btn in self.questButtonList:
			btn.Hide()

	def DestroyAllQuestButtons(self):
		for btn in self.questButtonList:
			btn.Hide()
			btn.Destroy()

		self.questButtonList = []

	def ShowAllQuestButton(self):
		for btn in self.questButtonList:
			btn.Show()

	#####################################################################################

	#####################################################################################
	### Whisper ###
	def RegisterGameMasterName(self, name):
		if name in self.listGMName:
			return
		self.listGMName[name] = True

	def IsGameMaster(self, name):
		try:
			return self.listGMName[name]
		except KeyError:
			return False

	def RegisterWhisperLanguageByName(self, name, lang):
		if name in self.listLangByName:
			return

		self.listLangByName[name] = lang

	def GetWhisperLangByName(self, name):
		try:
			return self.listLangByName[name]
		except KeyError:
			return ""

	#####################################################################################

	#####################################################################################
	### Guild Building ###

	def BUILD_OpenWindow(self):
		self.wndGuildBuilding = uiGuild.BuildGuildBuildingWindow()
		self.wndGuildBuilding.Open()
		self.wndGuildBuilding.wnds = self.__HideWindows()
		self.wndGuildBuilding.SetCloseEvent(self.BUILD_CloseWindow)

	def BUILD_CloseWindow(self):
		if self.wndGuildBuilding:
			self.__ShowWindows(self.wndGuildBuilding.wnds)
			self.wndGuildBuilding = None

	def BUILD_OnUpdate(self):
		if not self.wndGuildBuilding:
			return

		if self.wndGuildBuilding.IsPositioningMode():
			import background
			x, y, z = background.GetPickingPoint()
			self.wndGuildBuilding.SetBuildingPosition(x, y, z)

		if self.wndGuildBuilding.IsPositionChangeMode():
			self.wndGuildBuilding.ChangeWindowUpdate()

	def BUILD_OnMouseLeftButtonDown(self):
		if not self.wndGuildBuilding:
			return

		# GUILD_BUILDING
		if self.wndGuildBuilding.IsPositioningMode():
			self.wndGuildBuilding.SettleCurrentPosition()
			return True
		elif self.wndGuildBuilding.IsPreviewMode():
			pass
		else:
			if self.wndGuildBuilding.IsPositionChangeMode():
				self.wndGuildBuilding.EndPositionChangeMode()
			return True
		# END_OF_GUILD_BUILDING
		return False

	def BUILD_OnMouseLeftButtonUp(self):
		if not self.wndGuildBuilding:
			return False

		if not self.wndGuildBuilding.IsPreviewMode():
			return True

		return False

	def BULID_EnterGuildArea(self, areaID):
		# GUILD_BUILDING
		mainCharacterName = player.GetMainCharacterName()
		masterName = guild.GetGuildMasterName()

		if mainCharacterName != masterName:
			return

		if areaID != player.GetGuildID():
			return
		# END_OF_GUILD_BUILDING

		self.wndGameButton.ShowBuildButton()

	def BULID_ExitGuildArea(self, areaID):
		self.wndGameButton.HideBuildButton()

	#####################################################################################

	def IsEditLineFocus(self):
		if self.ChatWindow.chatLine.IsFocus():
			return 1

		if self.ChatWindow.chatToLine.IsFocus():
			return 1

		return 0

	def StartMaintenance(self, timeLimit, duration):
		if not self.wndMaintenanceInfo:
			self.wndMaintenanceInfo = uiMaintenance.MaintenanceInfoBoard()
		if timeLimit > 0:
			self.wndMaintenanceInfo.SetTimeLimit(timeLimit, duration)
			self.wndMaintenanceInfo.Show()
		else:
			self.wndMaintenanceInfo.Hide()

	def UpateMaintenancePos(self, y):
		if self.wndMaintenanceInfo:
			self.wndMaintenanceInfo.UpdatePos(y)

	def EmptyFunction(self):
		pass

	# Skill color changer
	def OpenSkillColorSelection(self, skillSlot):
		self.selectedSkill = skillSlot

		self.wndSkillColor.SetColor(playerInst().GetSkillColor(skillSlot))
		self.wndSkillColor.Open()

	def __SelectSkillColor(self):
		appInst.instance().GetNet().SendChangeSkillColorPacket(playerInst().GetSkillIndex(self.selectedSkill),
															   self.wndSkillColor.GetColor())
		self.wndSkillColor.Close()

	if app.WJ_ENABLE_TRADABLE_ICON or app.ENABLE_MOVE_COSTUME_ATTR or app.ENABLE_GROWTH_PET_SYSTEM:
		def AttachInvenItemToOtherWindowSlot(self, slotIndex):

			if app.ENABLE_MOVE_COSTUME_ATTR:
				if self.GetOnTopWindow() == player.ON_TOP_WND_ITEM_COMB and self.wndItemCombination and self.wndItemCombination.IsShow():
					self.wndItemCombination.AttachToCombinationSlot(player.INVENTORY, slotIndex)
					return True

			if app.ENABLE_GROWTH_PET_SYSTEM:
				if self.GetOnTopWindow() == player.ON_TOP_WND_PET_FEED and player.IsOpenPetFeedWindow() == True:
					if self.wndInventory:
						self.wndInventory.ItemMoveFeedWindow(slotIndex)
						return True

			return False

		def MarkUnusableInvenSlotOnTopWnd(self, onTopWnd, InvenSlot):
			if onTopWnd == player.ON_TOP_WND_SHOP and self.dlgShop and self.dlgShop.CantSellInvenItem(InvenSlot):
				return True
			elif onTopWnd == player.ON_TOP_WND_SAFEBOX and self.wndSafebox and self.wndSafebox.CantCheckInItem(
					InvenSlot):
				return True
			elif onTopWnd == player.ON_TOP_WND_PRIVATE_SHOP and self.privateShopBuilder and self.privateShopBuilder.CantTradableItem(
					InvenSlot):
				return True
			elif onTopWnd == player.ON_TOP_WND_EXCHANGE and self.dlgExchange and self.dlgExchange.CantTradableItem(
					InvenSlot):
				return True
			elif onTopWnd == player.ON_TOP_WND_SHOP_BUILD and OfflineShopManager.GetInstance().GetBuilderInstance() and OfflineShopManager.GetInstance().GetBuilderInstance().IsBlockedItem(
					InvenSlot):
				return True

			if onTopWnd == player.ON_TOP_WND_ITEM_COMB and self.wndItemCombination and self.wndItemCombination.IsShow() and self.wndItemCombination.CantAttachToCombSlot(
					InvenSlot):
				return True

			if app.ENABLE_GROWTH_PET_SYSTEM:
				if onTopWnd == player.ON_TOP_WND_PET_FEED and self.wndPetInfoWindow and self.wndPetInfoWindow.CantFeedItem(
						InvenSlot):
					return True

			return False

		def SetOnTopWindow(self, onTopWnd):
			self.OnTopWindow = onTopWnd

		def GetOnTopWindow(self):
			return self.OnTopWindow

		def RefreshMarkInventoryBag(self):
			if self.wndInventory:
				self.wndInventory.RefreshBagSlotWindow()

		def OpenMyShopDecoWnd(self):
			if self.inputDialog:
				return

			if self.privateShopBuilder.IsShow():
				return

			if not self.wndMyShopDeco:
				return

			self.wndMyShopDeco.Open()

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetHatchingWindowCommand(self, command, pos):
			if self.wndPetInfoWindow:
				if self.wndPetInfoWindow.wndPetHatching:
					self.wndPetInfoWindow.wndPetHatching.PetHatchingWindowCommand(command, pos)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetSkillUpgradeDlgOpen(self, slot, index, gold):
			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.OpenPetSkillUpGradeQuestionDialog(slot, index, gold)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetFlashEvent(self, index):
			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.PetFlashEvent(index)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetInfoBindAffectShower(self, affect_shower):

			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.PetInfoBindAffectShower(affect_shower)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetAffectShowerRefresh(self):

			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.PetAffectShowerRefresh()

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetEvolInfo(self, index, value):

			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.PetEvolInfo(index, value)

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def PetFeedReuslt(self, result):

			if self.wndPetInfoWindow:
				self.wndPetInfoWindow.PetFeedReuslt(result)

	if app.ENABLE_12ZI:
		def SetBeadCount(self, value):
			if self.wndBead:
				self.wndBead.SetBeadCount(value)

		def NextBeadUpdateTime(self, value):
			if self.wndBead:
				self.wndBead.NextBeadUpdateTime(value)

	def CheckRefineDialog(self, isFail):
		self.dlgRefineNew.CheckRefine(isFail)

	def PutAllInRefine(self):
		if self.wndDragonSoul:
			self.wndDragonSoul.PutAllInRefine()

	def RefreshQuickslot(self):
		self.wndTaskBar.RefreshQuickSlot()

	def UpdateQuestTimerNotifications(self):

		# thats the only way to make it work, refresh cdrs every teleportation etc.
		if self.refreshCooldowns:

			if not self.wndQuestTimer:
				return

			# notifications not enabled
			if self.wndQuestTimer.NOTIFICATION_ENABLED == [0] * len(self.wndQuestTimer.QUESTS):
				return

			appInst.instance().GetNet().SendChatPacket("/get_timer_cdrs")
			self.UpdateNotifications()
			self.refreshCooldowns = False

		if (app.GetTime() - self.lastUpdateTime) >= 3.0:
			self.lastUpdateTime = app.GetTime()
			self.UpdateNotifications()

		self.DrawNotifications()

	def UpdateNotifications(self):

		# first open the windows we need
		if self.wndQuestTimer:

			for i in range(0, len(self.wndQuestTimer.QUESTS)):
				if self.wndQuestTimer.QUESTS[i] is None:
					continue

				isEnabled = self.wndQuestTimer.NOTIFICATION_ENABLED[i]

				if not isEnabled:
					self.wndNotificationWindow[i].Reset()
					continue

				cooldown = self.wndQuestTimer.COOLDOWNS[i]

				# fix biolog quest... if time => 0 then open notification (part of this fix is in UpdateTimers)
				if i == 0 and cooldown == 0:
					self.wndNotificationWindow[i].Open()
					continue

				timeleft = int(cooldown - app.GetTime())

				if timeleft <= 0:
					self.wndNotificationWindow[i].Open()
				else:
					self.wndNotificationWindow[i].Reset()

	def DrawNotifications(self):

		openedNotifications = []

		for notification in self.wndNotificationWindow:

			if notification and notification.IsShow():

				notificationTime = notification.openTime
				globalTime = app.GetGlobalTime()

				if notificationTime > globalTime:
					notification.openTime = app.GetGlobalTime()

				openedNotifications.append(notification)

		if not len(openedNotifications):
			return

		# 46 + 4px padding
		imageWidth = openedNotifications[0].backgroundImage.GetRight()
		imageHeight = openedNotifications[0].backgroundImage.GetBottom()

		imageStep = imageHeight + 4

		screenHeight = wndMgr.GetScreenHeight()
		screenWidth = wndMgr.GetScreenWidth()

		elementsPositionStart = 80
		drawX = (screenWidth / 2) - (imageWidth / 2)

		for i in range(0, len(openedNotifications)):
			openedNotifications[i].SetPosition(drawX, elementsPositionStart + (imageStep * i))

	def ToggleTimerWindow(self):
		if self.wndQuestTimer:
			self.wndQuestTimer.Hide()
			self.wndQuestTimer.Destroy()
			self.wndQuestTimer = None

		reload(uiQuestTimer)

		if not self.wndQuestTimer:
			self.wndQuestTimer = uiQuestTimer.TimerWindow()

		if self.wndQuestTimer.IsShow():
			self.wndQuestTimer.Close()
		else:
			self.wndQuestTimer.Open()
			self.wndQuestTimer.SetItemToolTip(self.tooltipItem)

	def UpdateBiologInfo(self, currentLevel):
		if self.wndQuestTimer:
			self.wndQuestTimer.UpdateBiologInfo(currentLevel)

	def UpdateTimerWindow(self, key, value):
		if self.wndQuestTimer:
			self.wndQuestTimer.UpdateTimers(key, value)

	def OpenPreview(self, data):
		if self.wndPreviewWindow:
			self.wndPreviewWindow.Open(data)

	def OpenLevelPetWindow(self, data):
		if self.wndLevelPet:
			self.wndLevelPet.Open(data)

	def CloseLevelPetWindow(self):
		if self.wndLevelPet:
			self.wndLevelPet.Hide()

	def ToggleWorldBossInfo(self):
		if self.wndWorldBossInfo:
			if self.wndWorldBossInfo.IsShow():
				self.wndWorldBossInfo.Hide()
			else:
				self.wndWorldBossInfo.Show()

	def ToggleHuntingMissions(self):
		if self.wndHuntingMissions:
			if self.wndHuntingMissions.IsShow():
				self.wndHuntingMissions.Hide()
			else:
				self.wndHuntingMissions.Show()

	def AddHuntingMission(self, missionId, mobVnum, needCount, minLevel):
		if self.wndHuntingMissions:
			self.wndHuntingMissions.AddMission(missionId, mobVnum, needCount, minLevel)

	def AddHuntingMissionReward(self, missionId, vnum, count):
		if self.wndHuntingMissions:
			self.wndHuntingMissions.AddMissionReward(missionId, vnum, count)

	def MiniGameOkey(self):

		isOpen = player.GetMiniGameWindowOpen()

		if isOpen == True:
			if not self.wndMiniGame:
				self.wndMiniGame = uiMiniGame.MiniGameWindow()

			self.wndMiniGame.MiniGameOkeyEvent(True)
		else:
			if self.wndMiniGame:
				self.wndMiniGame.MiniGameOkeyEvent(False)

	def MiniGameStart(self):
		self.wndMiniGame.MiniGameStart()

	def RumiMoveCard(self, srcCard, dstCard):
		self.wndMiniGame.RumiMoveCard(srcCard, dstCard)

	def MiniGameRumiSetDeckCount(self, deck_card_count):
		self.wndMiniGame.MiniGameRumiSetDeckCount(deck_card_count)

	def RumiIncreaseScore(self, score, total_score):
		self.wndMiniGame.RumiIncreaseScore(score, total_score)

	def MiniGameEnd(self):
		self.wndMiniGame.MiniGameEnd()

	if app.ENABLE_MONSTER_BACK or app.ENABLE_CARNIVAL2016 or app.ENABLE_MINI_GAME_OKEY_NORMAL or app.ENABLE_FISH_EVENT or app.ENABLE_MINI_GAME_YUTNORI:
		def IntegrationEventBanner(self):
			isOpen = []

			if app.ENABLE_MONSTER_BACK or app.ENABLE_CARNIVAL2016:
				isOpen.append(player.GetAttendance())

			if app.ENABLE_MONSTER_BACK and app.ENABLE_ACCUMULATE_DAMAGE_DISPLAY:
				isOpen.append(player.GetMonsterBackEvent())

			if app.ENABLE_MINI_GAME_OKEY_NORMAL:
				isOpen.append(player.GetMiniGameWindowOpen())

			if app.ENABLE_FISH_EVENT:
				isOpen.append(player.GetFishEventGame())

			if app.ENABLE_MINI_GAME_YUTNORI:
				isOpen.append(player.GetYutnoriGame())

			if True in isOpen:
				if not self.wndMiniGame:
					self.wndMiniGame = uiMiniGame.MiniGameWindow()

					if app.ENABLE_FISH_EVENT:
						self.wndMiniGame.SetInven(self.wndInventory)
						self.wndMiniGame.BindInterface(self)
					if self.tooltipItem:
						self.wndMiniGame.SetItemToolTip(self.tooltipItem)

				self.wndMiniGame.IntegrationMiniGame(True)
			else:
				if self.wndMiniGame:
					self.wndMiniGame.IntegrationMiniGame(False)

		def Attendance(self):
			isOpen = player.GetAttendance()

			if isOpen:
				if not self.wndMiniGame:
					self.wndMiniGame = uiMiniGame.MiniGameWindow()

					if self.tooltipItem:
						self.wndMiniGame.SetItemToolTip(self.tooltipItem)

				self.wndMiniGame.MiniGameAttendance(True)
			else:
				if self.wndMiniGame:
					self.wndMiniGame.MiniGameAttendance(False)

		def MiniGameAttendanceSetData(self, type, value):
			if self.wndMiniGame:
				self.wndMiniGame.MiniGameAttendanceSetData(type, value)

		def MiniGameAttendanceRequestRewardList(self):
			if self.wndMiniGame:
				self.wndMiniGame.MiniGameAttendanceRequestRewardList()

	def ShowGuildWarButton(self):
		if self.wndGameButton:
			self.wndGameButton.ShowGuildWarButton()

	def HideGuildWarButton(self):
		if self.wndGameButton:
			self.wndGameButton.HideGuildWarButton()

	def ShowDungeonRejoinButton(self):
		if self.wndGameButton:
			self.wndGameButton.ShowDungeonRejoinButton()

	def HideDungeonRejoinButton(self):
		if self.wndGameButton:
			self.wndGameButton.HideDungeonRejoinButton()

	def OpenItemWiki(self, itemVnum):
		if self.wndWiki:
			self.wndWiki.OpenSpecialPage(None, itemVnum, False)
			if not self.wndWiki.IsShow():
				self.wndWiki.Show()

	if app.ENABLE_MINI_GAME_OKEY_NORMAL:
		def SetOkeyNormalBG(self):
			if not self.wndMiniGame:
				return

			self.wndMiniGame.SetOkeyNormalBG()

	def OpenFishingGameWindow(self):
		if not self.wndFishingGame:
			self.wndFishingGame = uiFishingGame.FishingGame()

		self.wndFishingGame.Open()

	def CloseFishingGameWindow(self):
		if self.wndFishingGame and self.wndFishingGame.IsShow():
			self.wndFishingGame.Close()
			self.wndFishingGame = None

	def FishingHitEffect(self):
		self.wndFishingGame.CreateHitEffect()

	def FishingMissEffect(self, x, y):
		self.wndFishingGame.CreateMissEffect(x, y)

	def FishingWaveEffect(self, x, y, delay):
		self.wndFishingGame.CreateWaveEffect(x, y, delay)

	def OpenDungeonCoolTimeWindow(self, floor, cooltime):
		self.wndDungeonCoolTime.Open()
		self.wndDungeonCoolTime.SetFloor(floor)
		self.wndDungeonCoolTime.SetCoolTime(cooltime)

	def SetShadowPotionEndTime(self, endTime):
		self.wndDungeonCoolTime.SetShadowPotionEndTime(endTime)

	def ClearDungeonCoolTime(self):
		self.wndDungeonCoolTime.Clear()
