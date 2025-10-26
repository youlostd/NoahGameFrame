# -*- coding: utf-8 -*-
import logging

import app
import item
import player
import skill
import wndMgr
from pygame.app import appInst
from pygame.item import ItemPosition
from pygame.player import SetItemSlot
from pygame.player import playerInst

import localeInfo
import mouseModule
import ui
from ui_event import MakeEvent, Event, MakeCallback

MOUSE_SETTINGS = [0, 0]

def InitMouseButtonSettings(left, right):
	global MOUSE_SETTINGS
	MOUSE_SETTINGS = [left, right]

def SetMouseButtonSetting(dir, event):
	global MOUSE_SETTINGS
	MOUSE_SETTINGS[dir] = event

def GetMouseButtonSettings():
	global MOUSE_SETTINGS
	return MOUSE_SETTINGS

def SaveMouseButtonSettings():
	global MOUSE_SETTINGS
	open("mouse.cfg", "w").write("%s\t%s" % tuple(MOUSE_SETTINGS))


def LoadMouseButtonSettings():
	global MOUSE_SETTINGS
	tokens = open("mouse.cfg", "r").read().split()

	if len(tokens) != 2:
		raise RuntimeError, "MOUSE_SETTINGS_FILE_ERROR"

	MOUSE_SETTINGS[0] = int(tokens[0])
	MOUSE_SETTINGS[1] = int(tokens[1])


# -------------------Giftbox Begin------------------------------

class GiftBox(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.SetWindowName(self.__class__.__name__)
		self.tooltipGift = ui.TextToolTip()
		self.loaded = False
		self.giftBoxIcon = None
		self.giftBoxToolTip = None

	def Show(self):
		if not self.loaded:
			self.LoadWindow()

		ui.ScriptWindow.Show(self)

	def LoadWindow(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/GiftBox.py")

		self.giftBoxIcon = self.GetChild("GiftBox_Icon")
		self.giftBoxToolTip = self.GetChild("GiftBox_ToolTip")

	def Destroy(self):
		self.giftBoxIcon = 0
		self.giftBoxToolTip = 0


# -------------------Giftbox End------------------------------

class EnergyBar(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.SetWindowName(self.__class__.__name__)
		self.tooltipEnergy = ui.TextToolTip()
		self.tooltipEnergy.Show()

	def LoadWindow(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/EnergyBar.py")

		self.energyEmpty = self.GetChild("EnergyGauge_Empty")
		self.energyHungry = self.GetChild("EnergyGauge_Hungry")
		self.energyFull = self.GetChild("EnergyGauge_Full")

		self.energyGaugeBoard = self.GetChild("EnergyGauge_Board")
		self.energyGaugeToolTip = self.GetChild("EnergyGauge_ToolTip")

	def Destroy(self):
		self.energyEmpty = None
		self.energyHungry = None
		self.energyFull = None
		self.energyGaugeBoard = None
		self.energyGaugeToolTip = None
		self.tooltipEnergy = None

	## Gauge
	def RefreshStatus(self):
		pointEnergy = playerInst().GetPoint(player.ENERGY)
		leftTimeEnergy = playerInst().GetPoint(player.ENERGY_END_TIME) - app.GetGlobalTimeStamp()
		# Ãæ±âÈ¯ Áö¼Ó ½Ã°£ = 2½Ã°£.
		self.SetEnergy(pointEnergy, leftTimeEnergy, 7200)

	def SetEnergy(self, point, leftTime, maxTime):
		leftTime = max(leftTime, 0)
		maxTime = max(maxTime, 0)

		self.energyEmpty.Hide()
		self.energyHungry.Hide()
		self.energyFull.Hide()

		if leftTime == 0:
			self.energyEmpty.Show()
			self.energyHungry.Hide()
			self.energyFull.Hide()
		elif ((leftTime * 100) / maxTime) < 15:
			self.energyEmpty.Hide()
			self.energyHungry.Show()
			self.energyFull.Hide()
		else:
			self.energyEmpty.Hide()
			self.energyHungry.Hide()
			self.energyFull.Show()

		self.tooltipEnergy.SetText("{}".format(localeInfo.TOOLTIP_ENERGY.format(point)))

	# [esper] Use callbacks for visibility determination instead of OnUpdate()
	def __OnToolTipMouseOverIn(self):
		self.tooltipEnergy.Show()
		return True

	def __OnToolTipMouseOverOut(self):
		self.tooltipEnergy.Hide()
		return True

	def OnUpdate(self):
		if self.energyGaugeToolTip:
			if True == self.energyGaugeToolTip.IsIn():
				self.RefreshStatus()
				self.tooltipEnergy.Show()
			else:
				self.tooltipEnergy.Hide()


class ExpandedTaskBar(ui.ScriptWindow):
	BUTTON_DRAGON_SOUL = 0

	if app.ENABLE_GROWTH_PET_SYSTEM:
		BUTTON_PET_INFO = 1

	def __init__(self):
		ui.Window.__init__(self)
		self.SetWindowName("ExpandedTaskBar")

		self.exclusion_list = []

	def LoadWindow(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/ExpandedTaskBar.py")

		self.expandedTaskBarBoard = self.GetChild("ExpanedTaskBar_Board")

		self.toggleButtonDict = {ExpandedTaskBar.BUTTON_DRAGON_SOUL: self.GetChild("DragonSoulButton")}
		self.toggleButtonDict[ExpandedTaskBar.BUTTON_DRAGON_SOUL].SetParent(self)

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.toggleButtonDict[ExpandedTaskBar.BUTTON_PET_INFO] = self.GetChild("PetInfoButton")
			self.toggleButtonDict[ExpandedTaskBar.BUTTON_PET_INFO].SetParent(self)

	def SetTop(self):
		super(ExpandedTaskBar, self).SetTop()
		for button in self.toggleButtonDict.values():
			button.SetTop()

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

	def SetToolTipText(self, eButton, text):
		self.toggleButtonDict[eButton].SetToolTipText(text)

	def SetToggleButtonEvent(self, eButton, kEventFunc):
		self.toggleButtonDict[eButton].SetEvent(kEventFunc)

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True


class ExpandedMoneyTaskBar(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.SetWindowName("ExpandedMoneyTaskBar")

		self.wndMoney = None
		self.wndMoneySlot = None
		self.wndMoneyIcon = None

		if app.ENABLE_CHEQUE_SYSTEM:
			self.wndCheque = None
			self.wndChequeSlot = None
			self.wndChequeIcon = None

		if app.ENABLE_GEM_SYSTEM:
			self.wndGem = None
			self.wndGemSlot = None
			self.wndGemIcon = None
	def Destroy(self):
		self.wndMoney = None
		self.wndMoneySlot = None
		self.wndMoneyIcon = None

		if app.ENABLE_CHEQUE_SYSTEM:
			self.wndCheque = None
			self.wndChequeSlot = None
			self.wndChequeIcon = None

		if app.ENABLE_GEM_SYSTEM:
			self.wndGem = None
			self.wndGemSlot = None
			self.wndGemIcon = None

	# def RefreshStatus(self):
		# self.GetChild("Money").SetText(player.GetElk())
		# self.GetChild("Cheque").SetText(player.GetCheque())
		# if app.ENABLE_GEM_SYSTEM:
			# self.GetChild("Gem").SetText(player.GetGem())

	def LoadWindow(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/ExpandedMoneyTaskBar.py")
		getChild = self.GetChild

		self.wndMoney = getChild("Money")
		self.wndMoneySlot = getChild("Money_Slot")
		self.wndMoneyIcon = getChild("Money_Icon")

		self.wndCheque = getChild("Cheque")
		self.wndChequeSlot = getChild("Cheque_Slot")
		self.wndChequeIcon = getChild("Cheque_Icon")

		if app.ENABLE_GEM_SYSTEM:
			self.wndGem = getChild("Gem")
			self.wndGemSlot = getChild("Gem_Slot")
			self.wndGemIcon = getChild("Gem_Icon")

	def GetMoneySlot(self):
		return self.wndMoneySlot

	def GetMoney(self):
		return self.wndMoney

	def GetMoneyIcon(self):
		return self.wndMoneyIcon

	if app.ENABLE_GEM_SYSTEM:
		def GetGemSlot(self):
			return self.wndGemSlot

		def GetGem(self):
			return self.wndGem

		def GetGemIcon(self):
			return self.wndGemIcon

	def GetChequeSlot(self):
		return self.wndChequeSlot

	def GetCheque(self):
		return self.wndCheque

	def GetChequeIcon(self):
		return self.wndChequeIcon

	def Show(self):
		self.SetTop()
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

	# def OnUpdate(self):
		# self.GetChild("Money").OnUpdateCool()
		# self.GetChild("Cheque").OnUpdateCool()
		# if app.ENABLE_GEM_SYSTEM:
			# self.GetChild("Gem").OnUpdateCool()


class TaskBar(ui.ScriptWindow):
	BUTTON_CHARACTER = 0
	BUTTON_INVENTORY = 1
	BUTTON_MESSENGER = 2
	BUTTON_SYSTEM = 3
	BUTTON_CHAT = 4
	BUTTON_EXPAND = 4
	IS_EXPANDED = False

	MOUSE_BUTTON_LEFT = 0
	MOUSE_BUTTON_RIGHT = 1
	NONE = 255

	EVENT_MOVE = 0
	EVENT_ATTACK = 1
	EVENT_MOVE_AND_ATTACK = 2
	EVENT_CAMERA = 3
	EVENT_SKILL = 4
	EVENT_AUTO = 5

	GAUGE_WIDTH = 161
	GAUGE_HEIGHT = 13

	QUICKPAGE_NUMBER_FILENAME = [
		"d:/ymir work/ui/game/taskbar/1.sub",
		"d:/ymir work/ui/game/taskbar/2.sub",
		"d:/ymir work/ui/game/taskbar/3.sub",
		"d:/ymir work/ui/game/taskbar/4.sub",
	]

	# gift icon show and hide
	def ShowGift(self):
		self.wndGiftBox.Show()

	def HideGift(self):
		self.wndGiftBox.Hide()

	class SkillButton(ui.SlotWindow):
		def __init__(self):
			ui.SlotWindow.__init__(self)

			self.event = ui.SlotWindow.NoneMethod

			self.slotIndex = 0
			self.skillIndex = 0

			slotIndex = 0
			wndMgr.SetSlotBaseImage(self.hWnd, "d:/ymir work/ui/public/slot_base.sub", 1.0, 1.0, 1.0, 1.0)
			wndMgr.AppendSlot(self.hWnd, slotIndex, 0, 0, 32, 32)
			self.SetCoverButton(slotIndex, "d:/ymir work/ui/public/slot_cover_button_01.sub", \
								"d:/ymir work/ui/public/slot_cover_button_02.sub", \
								"d:/ymir work/ui/public/slot_cover_button_03.sub", \
								"d:/ymir work/ui/public/slot_cover_button_04.sub", True, False)
			self.SetSize(32, 32)

		def Destroy(self):
			if 0 != self.tooltipSkill:
				self.tooltipSkill.HideToolTip()

		def RefreshSkill(self):
			if 0 != self.slotIndex:
				self.SetSkill(self.slotIndex)

		def SetSkillToolTip(self, tooltip):
			self.tooltipSkill = tooltip

		def SetSkill(self, skillSlotNumber):
			slotNumber = 0
			skillIndex = playerInst().GetSkillIndex(skillSlotNumber)
			skillGrade = playerInst().GetSkillGrade(skillSlotNumber)
			skillLevel = playerInst().GetSkillDisplayLevel(skillSlotNumber)
			skillType = skill.GetSkillType(skillIndex)

			self.skillIndex = skillIndex
			if 0 == self.skillIndex:
				self.ClearSlot(slotNumber)
				return

			self.slotIndex = skillSlotNumber

			self.SetSkillSlotNew(slotNumber, skillIndex, skillGrade, skillLevel)
			self.SetSlotCountNew(slotNumber, skillGrade, skillLevel)

			## NOTE : CoolTime Ã¼Å©
			if player.IsSkillCoolTime(skillSlotNumber):
				(coolTime, elapsedTime) = playerInst().GetSkillCoolTime(skillSlotNumber)
				self.SetSlotCoolTime(slotNumber, coolTime, elapsedTime)

			## NOTE : Activate µÇ¾î ÀÖ´Ù¸é ¾ÆÀÌÄÜµµ ¾÷µ¥ÀÌÆ®
			if player.IsSkillActive(skillSlotNumber):
				self.ActivateSlot(slotNumber)
			else:
				self.DeactivateSlot(slotNumber)

		def SetSkillEvent(self, event, arg=0):
			self.event = MakeEvent(event)

		def GetSkillIndex(self):
			return self.skillIndex

		def GetSlotIndex(self):
			return self.slotIndex

		def Activate(self, coolTime):
			self.SetSlotCoolTime(0, coolTime)

			if skill.IsToggleSkill(self.skillIndex):
				self.ActivateSlot(0)

		def Deactivate(self):
			if skill.IsToggleSkill(self.skillIndex):
				self.DeactivateSlot(0)

		def OnOverInItem(self, dummy):
			self.tooltipSkill.SetSkill(self.skillIndex)

		def OnOverOutItem(self):
			self.tooltipSkill.HideToolTip()

		def OnSelectItemSlot(self, dummy):
			if 0 != self.event:
				self.event()

	def __init__(self):
		ui.ScriptWindow.__init__(self, "TOP_MOST")

		self.quickPageNumImageBox = None
		self.tooltipItem = 0
		self.tooltipSkill = 0
		self.mouseModeButtonList = [ui.ScriptWindow("TOP_MOST"), ui.ScriptWindow("TOP_MOST")]

		self.tooltipHP = ui.TextToolTip()
		self.tooltipHP.Show()
		self.tooltipSP = ui.TextToolTip()
		self.tooltipSP.Show()
		self.tooltipST = ui.TextToolTip()
		self.tooltipST.Show()
		self.tooltipEXP = ui.TextToolTip()
		self.tooltipEXP.Show()

		self.skillCategoryNameList = ["ACTIVE_1", "ACTIVE_2", "ACTIVE_3"]
		self.skillPageStartSlotIndexDict = {
			"ACTIVE_1": 1,
			"ACTIVE_2": 21,
			"ACTIVE_3": 41,
		}

		self.selectSkillButtonList = []

		self.lastUpdateQuickSlot = 0
		self.SetWindowName("TaskBar")
		self.RemoveFlag("focusable")

	def LoadWindow(self):
		logging.debug("Load TaskBar")
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/TaskBar.py")
		pyScrLoader.LoadScriptFile(self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT],
									"UIScript/MouseButtonWindow.py")
		pyScrLoader.LoadScriptFile(self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT],
									"UIScript/RightMouseButtonWindow.py")

		self.quickslot = []
		self.quickslot.append(self.GetChild("quick_slot_1"))
		self.quickslot.append(self.GetChild("quick_slot_2"))
		for slot in self.quickslot:
			slot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
			slot.SetSelectEmptySlotEvent(self.SelectEmptyQuickSlot)
			slot.SetSelectItemSlotEvent(self.SelectItemQuickSlot)
			slot.SetUnselectItemSlotEvent(self.UnselectItemQuickSlot)
			slot.SetOverInItemEvent(self.OverInItem)
			slot.SetOverOutItemEvent(self.OverOutItem)

		toggleButtonDict = {TaskBar.BUTTON_CHARACTER: self.GetChild("CharacterButton"),
							TaskBar.BUTTON_INVENTORY: self.GetChild("InventoryButton"),
							TaskBar.BUTTON_MESSENGER: self.GetChild("MessengerButton"),
							TaskBar.BUTTON_SYSTEM: self.GetChild("SystemButton"),
							}

		try:
			toggleButtonDict[TaskBar.BUTTON_CHAT] = self.GetChild("ChatButton")
		except:
			toggleButtonDict[TaskBar.BUTTON_EXPAND] = self.GetChild("ExpandButton")
			TaskBar.IS_EXPANDED = True

		if localeInfo.IsARABIC():
			systemButton = toggleButtonDict[TaskBar.BUTTON_SYSTEM]
			if systemButton.ToolTipText:
				tx, ty = systemButton.ToolTipText.GetLocalPosition()
				tw = systemButton.ToolTipText.GetWidth()
				systemButton.ToolTipText.SetPosition(-tw / 2, ty)

		expGauge = [self.GetChild("EXPGauge_01"), self.GetChild("EXPGauge_02"), self.GetChild("EXPGauge_03"),
					self.GetChild("EXPGauge_04")]

		for exp in expGauge:
			exp.SetSize(0, 0)

		self.quickPageNumImageBox = self.GetChild("QuickPageNumber")

		self.GetChild("QuickPageUpButton").SetEvent(self.__OnClickQuickPageUpButton)
		self.GetChild("QuickPageDownButton").SetEvent(self.__OnClickQuickPageDownButton)

		mouseLeftButtonModeButton = self.GetChild("LeftMouseButton")
		mouseRightButtonModeButton = self.GetChild("RightMouseButton")

		mouseLeftButtonModeButton.SetEvent(self.ToggleLeftMouseButtonModeWindow)
		mouseRightButtonModeButton.SetEvent(self.ToggleRightMouseButtonModeWindow)

		self.curMouseModeButton = [mouseLeftButtonModeButton, mouseRightButtonModeButton]
		(xLeft, yLeft) = mouseLeftButtonModeButton.GetGlobalPosition()
		(xRight, yRight) = mouseRightButtonModeButton.GetGlobalPosition()

		(xLocalRight, yLocalRight) = mouseRightButtonModeButton.GetLocalPosition()
		self.curSkillButton = self.SkillButton()
		self.curSkillButton.SetParent(self)
		self.curSkillButton.SetPosition(xRight, yLocalRight)
		self.curSkillButton.SetSkillEvent(self.ToggleRightMouseButtonModeWindow)
		self.curSkillButton.Hide()

		leftModeButtonList = self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT]
		leftModeButtonList.SetPosition(xLeft, yLeft - leftModeButtonList.GetHeight() - 5)

		rightModeButtonList = self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT]
		rightModeButtonList.SetPosition(xRight - rightModeButtonList.GetWidth() + 32,
										yRight - rightModeButtonList.GetHeight() -5)

		rightModeButtonList.GetChild("button_skill").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_RIGHT, self.EVENT_SKILL))
		rightModeButtonList.GetChild("button_skill").Hide()

		mouseImage = ui.ImageBox("TOP_MOST")
		mouseImage.AddFlag("float")
		mouseImage.LoadImage("d:/ymir work/ui/game/taskbar/mouse_button_camera_01.sub")
		mouseImage.SetPosition(xRight, wndMgr.GetScreenHeight() - 34)
		mouseImage.Hide()
		self.mouseImage = mouseImage

		wnd = self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT]
		wnd.GetChild("button_move_and_attack").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_LEFT, self.EVENT_MOVE_AND_ATTACK))
		wnd.GetChild("button_auto_attack").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_LEFT, self.EVENT_AUTO))
		wnd.GetChild("button_camera").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_LEFT, self.EVENT_CAMERA))

		wnd = self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT]
		wnd.GetChild("button_move_and_attack").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_RIGHT, self.EVENT_MOVE_AND_ATTACK))
		wnd.GetChild("button_camera").SetEvent(
			Event(self.SelectMouseButtonEvent, self.MOUSE_BUTTON_RIGHT, self.EVENT_CAMERA))

		self.toggleButtonDict = toggleButtonDict
		self.expGauge = expGauge

		self.rampageGauge = self.GetChild("RampageGauge")
		self.rampageGauge.OnMouseLeftButtonUp = MakeCallback(self.__RampageGauge_Click)

		self.hpGauge = self.GetChild("HPGauge")
		self.mpGauge = self.GetChild("SPGauge")
		self.stGauge = self.GetChild("STGauge")
		self.hpRecoveryGaugeBar = self.GetChild("HPRecoveryGaugeBar")
		self.spRecoveryGaugeBar = self.GetChild("SPRecoveryGaugeBar")

		self.hpGaugeBoard = self.GetChild("HPGauge_Board")
		self.mpGaugeBoard = self.GetChild("SPGauge_Board")
		self.stGaugeBoard = self.GetChild("STGauge_Board")
		self.expGaugeBoard = self.GetChild("EXP_Gauge_Board")

		# giftbox object
		self.wndGiftBox = GiftBox()

		self.__LoadMouseSettings()
		self.RefreshStatus()
		self.RefreshQuickSlot()

	def __RampageGauge_Click(self):

		appInst.instance().GetNet().SendChatPacket("/in_game_mall")
		# gift icon hide when click mall icon
		self.wndGiftBox.Hide()

	def __LoadMouseSettings(self):
		try:
			LoadMouseButtonSettings()
			(mouseLeftButtonEvent, mouseRightButtonEvent) = GetMouseButtonSettings()
			if not self.__IsInSafeMouseButtonSettingRange(
					mouseLeftButtonEvent) or not self.__IsInSafeMouseButtonSettingRange(mouseRightButtonEvent):
				raise RuntimeError, "INVALID_MOUSE_BUTTON_SETTINGS"
		except:
			InitMouseButtonSettings(self.EVENT_MOVE_AND_ATTACK, self.EVENT_CAMERA)
			(mouseLeftButtonEvent, mouseRightButtonEvent) = GetMouseButtonSettings()

		try:
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_LEFT, mouseLeftButtonEvent)
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_RIGHT, mouseRightButtonEvent)
		except:
			InitMouseButtonSettings(self.EVENT_MOVE_AND_ATTACK, self.EVENT_CAMERA)
			(mouseLeftButtonEvent, mouseRightButtonEvent) = GetMouseButtonSettings()

			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_LEFT, mouseLeftButtonEvent)
			self.SelectMouseButtonEvent(self.MOUSE_BUTTON_RIGHT, mouseRightButtonEvent)

	def __IsInSafeMouseButtonSettingRange(self, arg):
		return arg >= self.EVENT_MOVE and arg <= self.EVENT_AUTO

	def Destroy(self):
		SaveMouseButtonSettings()

		self.ClearDictionary()
		self.mouseModeButtonList[0].ClearDictionary()
		self.mouseModeButtonList[1].ClearDictionary()
		self.mouseModeButtonList = 0
		self.curMouseModeButton = 0
		self.curSkillButton = 0
		self.selectSkillButtonList = 0

		self.expGauge = None
		self.hpGauge = None
		self.mpGauge = None
		self.stGauge = None
		self.hpRecoveryGaugeBar = None
		self.spRecoveryGaugeBar = None

		self.tooltipItem = 0
		self.tooltipSkill = 0
		self.quickslot = []
		self.toggleButtonDict = 0

		self.hpGaugeBoard = 0
		self.mpGaugeBoard = 0
		self.stGaugeBoard = 0

		self.expGaugeBoard = 0

		self.tooltipHP = 0
		self.tooltipSP = 0
		self.tooltipST = 0
		self.tooltipEXP = 0

		self.mouseImage = None

	def __OnClickQuickPageUpButton(self):
		player.SetQuickPage(player.GetQuickPage() - 1)

	def __OnClickQuickPageDownButton(self):
		player.SetQuickPage(player.GetQuickPage() + 1)

	def SetToggleButtonEvent(self, eButton, kEventFunc):
		self.toggleButtonDict[eButton].SetEvent(kEventFunc)

	def SetItemToolTip(self, tooltipItem):
		self.tooltipItem = tooltipItem

	def SetSkillToolTip(self, tooltipSkill):
		self.tooltipSkill = tooltipSkill
		self.curSkillButton.SetSkillToolTip(self.tooltipSkill)

	## Mouse Image
	def ShowMouseImage(self):
		self.mouseImage.SetTop()
		self.mouseImage.Show()

	def HideMouseImage(self):
		player.SetQuickCameraMode(False)
		self.mouseImage.Hide()

	## Gauge
	def RefreshStatus(self):
		curHP = playerInst().GetPoint(player.HP)
		maxHP = playerInst().GetPoint(player.MAX_HP)
		curSP = playerInst().GetPoint(player.SP)
		maxSP = playerInst().GetPoint(player.MAX_SP)
		curEXP = (playerInst().GetPoint(player.EXP))
		nextEXP = (playerInst().GetPoint(player.NEXT_EXP))
		recoveryHP = playerInst().GetPoint(player.HP_RECOVERY)
		recoverySP = playerInst().GetPoint(player.SP_RECOVERY)

		self.RefreshStamina()

		self.SetHP(curHP, recoveryHP, maxHP)
		self.SetSP(curSP, recoverySP, maxSP)
		self.SetExperience(curEXP, nextEXP)

	def RefreshStamina(self):
		curST = long(playerInst().GetPoint(player.STAMINA))
		maxST = long(playerInst().GetPoint(player.MAX_STAMINA))
		self.SetST(curST, maxST)

	def RefreshSkill(self):
		self.curSkillButton.RefreshSkill()
		for button in self.selectSkillButtonList:
			button.RefreshSkill()

	def SetHP(self, curPoint, recoveryPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.hpGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipHP.SetText("%s : %d / %d" % (localeInfo.TASKBAR_HP, curPoint, maxPoint))

			if 0 == recoveryPoint:
				self.hpRecoveryGaugeBar.Hide()
			else:
				destPoint = min(maxPoint, curPoint + recoveryPoint)
				newWidth = int(self.GAUGE_WIDTH * (float(destPoint) / float(maxPoint)))
				self.hpRecoveryGaugeBar.SetSize(151, self.GAUGE_HEIGHT + 2)
				self.hpRecoveryGaugeBar.SetPosition(10, 2)
				self.hpRecoveryGaugeBar.SetColor(0x55ff0000)
				self.hpRecoveryGaugeBar.Show()

	def SetSP(self, curPoint, recoveryPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.mpGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipSP.SetText("%s : %d / %d" % (localeInfo.TASKBAR_SP, curPoint, maxPoint))

			if 0 == recoveryPoint:
				self.spRecoveryGaugeBar.Hide()
			else:
				destPoint = min(maxPoint, curPoint + recoveryPoint)
				newWidth = int(self.GAUGE_WIDTH * (float(destPoint) / float(maxPoint)))
				self.spRecoveryGaugeBar.SetSize(newWidth, self.GAUGE_HEIGHT)
				self.spRecoveryGaugeBar.Show()

	def SetST(self, curPoint, maxPoint):
		curPoint = min(curPoint, maxPoint)
		if maxPoint > 0:
			self.stGauge.SetPercentage(curPoint, maxPoint)
			self.tooltipST.SetText("%s : %d / %d" % (localeInfo.TASKBAR_ST, curPoint, maxPoint))

	def SetExperience(self, curPoint, maxPoint):

		curPoint = min(curPoint, maxPoint)
		curPoint = max(curPoint, 0)
		maxPoint = max(maxPoint, 0)

		quarterPoint = maxPoint / 4
		FullCount = 0

		if 0 != quarterPoint:
			FullCount = min(4, curPoint / quarterPoint)

		for i in xrange(4):
			self.expGauge[i].Hide()

		for i in xrange(int(FullCount)):
			self.expGauge[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.expGauge[i].Show()

		if 0 != quarterPoint:
			if FullCount < 4.0:
				Percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
				self.expGauge[int(FullCount)].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
				self.expGauge[int(FullCount)].Show()

		#####
		self.tooltipEXP.SetText(
			"%s : %.2f%%" % (localeInfo.TASKBAR_EXP, float(curPoint) / max(1, float(maxPoint)) * 100))

	## QuickSlot
	def RefreshQuickSlot(self):
		if not self.quickslot:
			return

		pageNum = player.GetQuickPage()

		pos = ItemPosition()
		pos.windowType = player.INVENTORY

		try:
			self.quickPageNumImageBox.LoadImage(TaskBar.QUICKPAGE_NUMBER_FILENAME[pageNum])
		except:
			pass

		startNumber = 0
		for slot in self.quickslot:
			for i in xrange(4):

				slotNumber = i + startNumber

				try:
					quickSlot = playerInst().GetLocalQuickSlot(slotNumber)

					if player.SLOT_TYPE_NONE == quickSlot.Type:
						slot.ClearSlot(slotNumber)
						continue

					if player.SLOT_TYPE_INVENTORY == quickSlot.Type:
						pos.cell = quickSlot.Position

						itemData = playerInst().GetItemData(pos)
						if not itemData:
							continue

						try:
							SetItemSlot(slot.hWnd, slotNumber, itemData)
						except Exception as e:
							logging.exception(e)
					elif player.SLOT_TYPE_SKILL == quickSlot.Type:

						skillIndex = playerInst().GetSkillIndex(quickSlot.Position)
						if 0 == skillIndex:
							slot.ClearSlot(slotNumber)
							continue

						skillType = skill.GetSkillType(skillIndex)
						if skill.SKILL_TYPE_GUILD == skillType:
							import guild
							skillGrade = 0
							skillLevel = guild.GetSkillLevel(quickSlot.Position)

						else:
							skillGrade = playerInst().GetSkillGrade(quickSlot.Position)
							skillLevel = playerInst().GetSkillDisplayLevel(quickSlot.Position)

						slot.SetSkillSlotNew(slotNumber, skillIndex, skillGrade, skillLevel)
						slot.SetSlotCountNew(slotNumber, skillGrade, skillLevel)
						slot.SetCoverButton(slotNumber)

						## NOTE : CoolTime 체크
						if player.IsSkillCoolTime(quickSlot.Position):
							(coolTime, elapsedTime) = playerInst().GetSkillCoolTime(quickSlot.Position)
							slot.SetSlotCoolTime(slotNumber, coolTime, elapsedTime)

						## NOTE : Activate 되어 있다면 아이콘도 업데이트
						if player.IsSkillActive(quickSlot.Position):
							slot.ActivateSlot(slotNumber)

					elif player.SLOT_TYPE_EMOTION == quickSlot.Type:

						emotionIndex = quickSlot.Position
						slot.SetEmotionSlot(slotNumber, emotionIndex)
						slot.SetCoverButton(slotNumber)
						slot.SetSlotCount(slotNumber, 0)
				except TypeError as e:
					pass

			slot.RefreshSlot()
			startNumber += 4

	def canAddQuickSlot(self, Type, slotNumber):

		if player.SLOT_TYPE_INVENTORY == Type:
			itemIndex = player.GetItemIndex(slotNumber)
			return item.CanAddToQuickSlotItem(itemIndex)

		return True

	def AddQuickSlot(self, localSlotIndex):
		AttachedSlotType = mouseModule.mouseController.GetAttachedType()
		AttachedSlotNumber = mouseModule.mouseController.GetRealAttachedSlotNumber()
		AttachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

		if player.SLOT_TYPE_QUICK_SLOT == AttachedSlotType:
			player.RequestMoveGlobalQuickSlotToLocalQuickSlot(AttachedSlotNumber, localSlotIndex)

		elif player.SLOT_TYPE_EMOTION == AttachedSlotType:

			player.RequestAddLocalQuickSlot(localSlotIndex, AttachedSlotType, AttachedItemIndex)

		elif True == self.canAddQuickSlot(AttachedSlotType, AttachedSlotNumber):

			## Online Code
			player.RequestAddLocalQuickSlot(localSlotIndex, AttachedSlotType, AttachedSlotNumber)

		mouseModule.mouseController.DeattachObject()
		self.RefreshQuickSlot()

	def SelectEmptyQuickSlot(self, slotIndex):

		if True == mouseModule.mouseController.isAttached():
			self.AddQuickSlot(slotIndex)

	def SelectItemQuickSlot(self, localQuickSlotIndex):

		if True == mouseModule.mouseController.isAttached():
			self.AddQuickSlot(localQuickSlotIndex)

		else:
			globalQuickSlotIndex = player.LocalQuickSlotIndexToGlobalQuickSlotIndex(localQuickSlotIndex)
			mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_QUICK_SLOT, globalQuickSlotIndex,
													 globalQuickSlotIndex)

	def UnselectItemQuickSlot(self, localSlotIndex):

		if False == mouseModule.mouseController.isAttached():
			player.RequestUseLocalQuickSlot(localSlotIndex)
			return

		elif mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()
			return

	def OnUseSkill(self, usedSlotIndex, coolTime):

		QUICK_SLOT_SLOT_COUNT = 4
		slotIndex = 0

		## Current Skill Button
		if usedSlotIndex == self.curSkillButton.GetSlotIndex():
			self.curSkillButton.Activate(coolTime)

		## Quick Slot
		for slotWindow in self.quickslot:

			for i in xrange(QUICK_SLOT_SLOT_COUNT):

				quickSlot = playerInst().GetLocalQuickSlot(slotIndex)

				if quickSlot.Type == player.SLOT_TYPE_SKILL:
					if usedSlotIndex == quickSlot.Position:
						slotWindow.SetSlotCoolTime(slotIndex, coolTime)
						return

				slotIndex += 1

	def OnActivateSkill(self, usedSlotIndex):
		slotIndex = 0

		## Current Skill Button
		if usedSlotIndex == self.curSkillButton.GetSlotIndex():
			self.curSkillButton.Deactivate()

		## Quick Slot
		for slotWindow in self.quickslot:

			for i in xrange(4):

				quickSlot = playerInst().GetLocalQuickSlot(slotIndex)

				if quickSlot.Type == player.SLOT_TYPE_SKILL:
					if usedSlotIndex == quickSlot.Position:
						slotWindow.ActivateSlot(slotIndex)
						return

				slotIndex += 1

	def OnDeactivateSkill(self, usedSlotIndex):
		slotIndex = 0

		## Current Skill Button
		if usedSlotIndex == self.curSkillButton.GetSlotIndex():
			self.curSkillButton.Deactivate()

		## Quick Slot
		for slotWindow in self.quickslot:

			for i in xrange(4):

				quickSlot = playerInst().GetLocalQuickSlot(slotIndex)

				if quickSlot.Type == player.SLOT_TYPE_SKILL:
					if usedSlotIndex == quickSlot.Position:
						slotWindow.DeactivateSlot(slotIndex)
						return

				slotIndex += 1

	## ToolTip
	def OverInItem(self, slotNumber):
		if mouseModule.mouseController.isAttached():
			return

		quickSlot = playerInst().GetLocalQuickSlot(slotNumber)

		if player.SLOT_TYPE_INVENTORY == quickSlot.Type:
			self.tooltipItem.SetInventoryItem(quickSlot.Position)
			self.tooltipSkill.HideToolTip()

		elif player.SLOT_TYPE_SKILL == quickSlot.Type:

			skillIndex = playerInst().GetSkillIndex(quickSlot.Position)
			skillType = skill.GetSkillType(skillIndex)

			if skill.SKILL_TYPE_GUILD == skillType:
				import guild
				skillGrade = 0
				skillLevel = guild.GetSkillLevel(quickSlot.Position)

			else:
				skillGrade = playerInst().GetSkillGrade(quickSlot.Position)
				skillLevel = playerInst().GetSkillDisplayLevel(quickSlot.Position)

			self.tooltipSkill.SetSkillNew(quickSlot.Position, skillIndex, skillGrade, skillLevel)
			self.tooltipItem.HideToolTip()

	def OverOutItem(self):
		if 0 != self.tooltipItem:
			self.tooltipItem.HideToolTip()
		if 0 != self.tooltipSkill:
			self.tooltipSkill.HideToolTip()

	def OnUpdate(self):
		if app.GetGlobalTime() - self.lastUpdateQuickSlot > 500:
			self.lastUpdateQuickSlot = app.GetGlobalTime()
			self.RefreshQuickSlot()

		if self.hpGaugeBoard and self.hpGaugeBoard.IsIn():
			self.tooltipHP.Show()
		else:
			self.tooltipHP.Hide()

		if self.mpGaugeBoard and self.mpGaugeBoard.IsIn():
			self.tooltipSP.Show()
		else:
			self.tooltipSP.Hide()

		if self.stGaugeBoard and self.stGaugeBoard.IsIn():
			self.tooltipST.Show()
		else:
			self.tooltipST.Hide()

		if self.expGaugeBoard and self.expGaugeBoard.IsIn():
			self.tooltipEXP.Show()
		else:
			self.tooltipEXP.Hide()

	## Skill
	def ToggleLeftMouseButtonModeWindow(self):
		wndMouseButtonMode = self.mouseModeButtonList[self.MOUSE_BUTTON_LEFT]

		if wndMouseButtonMode.IsShow():
			wndMouseButtonMode.Hide()
		else:
			wndMouseButtonMode.Show()

	def ToggleRightMouseButtonModeWindow(self):
		wndMouseButtonMode = self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT]

		if wndMouseButtonMode.IsShow():
			wndMouseButtonMode.Hide()
			self.CloseSelectSkill()
		else:
			wndMouseButtonMode.Show()
			self.OpenSelectSkill()

	def OpenSelectSkill(self):
		PAGE_SLOT_COUNT = 6

		mouseRightButtonModeButton = self.GetChild("RightMouseButton")

		(xSkillButton, y) = mouseRightButtonModeButton.GetGlobalPosition()
		y -= (37 + 16 + 1 + 36)

		for key in self.skillCategoryNameList:
			appendCount = 0
			startNumber = self.skillPageStartSlotIndexDict[key]
			x = xSkillButton

			getSkillIndex = playerInst().GetSkillIndex
			getSkillLevel = playerInst().GetSkillLevel
			for i in xrange(PAGE_SLOT_COUNT):
				skillIndex = getSkillIndex(startNumber + i)
				skillLevel = playerInst().GetSkillDisplayLevel(startNumber + i)

				if 0 == skillIndex:
					continue

				if 0 == skillLevel:
					continue

				if skill.IsStandingSkill(skillIndex):
					continue

				## FIXME : 스킬 하나당 슬롯 하나씩 할당하는건 아무리 봐도 부하가 크다.
				##		 이 부분은 시간을 나면 고치도록. - [levites]
				skillButton = self.SkillButton()
				skillButton.SetSkill(startNumber + i)
				skillButton.SetPosition(x, y)
				skillButton.SetSkillEvent(Event(self.CloseSelectSkill, startNumber + i + 1))
				skillButton.SetSkillToolTip(self.tooltipSkill)
				skillButton.SetTop()
				skillButton.Show()
				self.selectSkillButtonList.append(skillButton)

				appendCount += 1
				x -= 32

			if appendCount > 0:
				y -= 32

	def CloseSelectSkill(self, slotIndex=-1):
		self.mouseModeButtonList[self.MOUSE_BUTTON_RIGHT].Hide()

		for button in self.selectSkillButtonList:
			button.Destroy()

		self.selectSkillButtonList = []

		if -1 != slotIndex:
			self.curSkillButton.Show()
			self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Hide()
			player.SetMouseFunc(player.MBT_RIGHT, player.MBF_SKILL)
			player.ChangeCurrentSkillNumberOnly(slotIndex - 1)
		else:
			self.curSkillButton.Hide()
			self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Show()

	def SelectMouseButtonEvent(self, dir, event):
		SetMouseButtonSetting(dir, event)

		self.CloseSelectSkill()
		self.mouseModeButtonList[dir].Hide()

		btn = 0
		type = self.NONE
		func = self.NONE
		tooltip_text = ""

		if self.MOUSE_BUTTON_LEFT == dir:
			type = player.MBT_LEFT
		elif self.MOUSE_BUTTON_RIGHT == dir:
			type = player.MBT_RIGHT

		if self.EVENT_MOVE == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_move")
			func = player.MBF_MOVE
			tooltip_text = localeInfo.TASKBAR_MOVE
		elif self.EVENT_ATTACK == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_attack")
			func = player.MBF_ATTACK
			tooltip_text = localeInfo.TASKBAR_ATTACK
		elif self.EVENT_AUTO == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_auto_attack")
			func = player.MBF_AUTO
			tooltip_text = localeInfo.TASKBAR_AUTO
		elif self.EVENT_MOVE_AND_ATTACK == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_move_and_attack")
			func = player.MBF_SMART
			tooltip_text = localeInfo.TASKBAR_ATTACK
		elif self.EVENT_CAMERA == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_camera")
			func = player.MBF_CAMERA
			tooltip_text = localeInfo.TASKBAR_CAMERA
		elif self.EVENT_SKILL == event:
			btn = self.mouseModeButtonList[dir].GetChild("button_skill")
			func = player.MBF_SKILL
			tooltip_text = localeInfo.TASKBAR_SKILL

		if 0 != btn:
			self.curMouseModeButton[dir].SetToolTipText(tooltip_text, 0, -18)
			self.curMouseModeButton[dir].SetUpVisual(btn.GetUpVisualFileName())
			self.curMouseModeButton[dir].SetOverVisual(btn.GetOverVisualFileName())
			self.curMouseModeButton[dir].SetDownVisual(btn.GetDownVisualFileName())
			self.curMouseModeButton[dir].Show()

		player.SetMouseFunc(type, func)

	def OnChangeCurrentSkill(self, skillSlotNumber):
		self.curSkillButton.SetSkill(skillSlotNumber)
		self.curSkillButton.Show()
		self.curMouseModeButton[self.MOUSE_BUTTON_RIGHT].Hide()
