# -*- coding: utf-8 -*-


import app
import background
import chr
import grp
import grpText
import guild
import miniMap
import player
import skill
import wndMgr
from pygame.app import appInst
from pygame.guild import guildMgr
from pygame.player import playerInst

import localeInfo
import mouseModule
import playerSettingModule
import ui
import uiCommon
import uiPickMoney
import uiToolTip
import uiUploadMark
import constInfo
from ui_event import MakeEvent, Event

DISABLE_GUILD_SKILL = False
DISABLE_DECLARE_WAR = False

# Guilds online
ENEMY_GUILDS_PER_PAGE = 6


# End guilds online


def NumberToMoneyString(n):
	return localeInfo.NumberToMoneyString(n)


MATERIAL_STONE_INDEX = 0
MATERIAL_LOG_INDEX = 1
MATERIAL_PLYWOOD_INDEX = 2

MATERIAL_STONE_ID = 90010
MATERIAL_LOG_ID = 90011
MATERIAL_PLYWOOD_ID = 90012


def GetGVGKey(srcGuildID, dstGuildID):
	minID = min(srcGuildID, dstGuildID)
	maxID = max(srcGuildID, dstGuildID)
	return minID * 1000 + maxID


if app.ENABLE_NEW_GUILD_WAR:
	class DeclareGuildWarDialog(ui.ScriptWindow):

		def __init__(self):
			ui.ScriptWindow.__init__(self)

			self.type=1
			self.block_mount = 0
			self.block_warrior = 0
			self.block_assassin = 0
			self.block_sura = 0
			self.block_shaman = 0
			self.__CreateDialog()

		def Open(self):
			self.inputValue.SetFocus()
			self.SetCenterPosition()
			self.SetTop()
			self.Show()

		# if app.ENABLE_ONLINE_GUILD_LEADERS:
			# def Open2(self,lonca):
				# self.inputValue.SetFocus()
				# self.inputValue.SetText(lonca)
				# self.SetCenterPosition()
				# self.SetTop()
				# self.Show()

		def Close(self):
			self.ClearDictionary()
			self.board = None
			self.acceptButton = None
			self.cancelButton = None
			self.inputSlot = None
			self.inputSlot1 = None
			self.inputSlot2 = None
			self.inputSlot3 = None
			self.inputValue = None
			self.inputValue1 = None
			self.inputValue2 = None
			self.inputValue3 = None
			self.Hide()

		def __CreateDialog(self):
			pyScrLoader = ui.PythonScriptLoader()

			pyScrLoader.LoadScriptFile(self, "uiscript/declareguildwardialog_new.py")

			getObject = self.GetChild
			self.board = getObject("Board")

			self.typeButtonList=[]
			self.typeButtonList.append(getObject("NormalButton"))
			self.typeButtonList.append(getObject("WarpButton"))
			self.typeButtonList.append(getObject("CTFButton"))			
			
			
			self.blockButtonList=[]
			block_mount = getObject("block_mount")
			block_mount.SetToggleUpEvent(self.__OnClickBlockMount)
			block_mount.SetToggleDownEvent(self.__OnClickBlockMount)
			self.blockButtonList.append(block_mount)
			
			block_warrior = getObject("block_warrior")
			block_warrior.SetToggleUpEvent(self.__OnClickBlockWarrior)
			block_warrior.SetToggleDownEvent(self.__OnClickBlockWarrior)
			self.blockButtonList.append(block_warrior)
			
			block_assassin = getObject("block_assassin")
			block_assassin.SetToggleUpEvent(self.__OnClickBlockAssassin)
			block_assassin.SetToggleDownEvent(self.__OnClickBlockAssassin)
			self.blockButtonList.append(block_assassin)
			
			block_sura = getObject("block_sura")
			block_sura.SetToggleUpEvent(self.__OnClickBlockSura)
			block_sura.SetToggleDownEvent(self.__OnClickBlockSura)
			self.blockButtonList.append(block_sura)
			
			block_shaman = getObject("block_shaman")
			block_shaman.SetToggleUpEvent(self.__OnClickBlockShaman)
			block_shaman.SetToggleDownEvent(self.__OnClickBlockShaman)
			self.blockButtonList.append(block_shaman)
			
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputSlot1 = getObject("InputSlotScore")
			self.inputSlot2 = getObject("InputSlotUser")
			self.inputSlot3 = getObject("InputValueMinSeviye")
			self.inputValue = getObject("InputValue")
			self.inputValue1 = getObject("InputValueScore")
			self.inputValue2 = getObject("InputValueUser")
			self.inputValue3 = getObject("InputValueMinSeviye")
			self.inputValue1.SetNumberMode()
			self.inputValue2.SetNumberMode()
			self.inputValue3.SetNumberMode()
			self.inputValue1.SetText(str(100))
			self.inputValue2.SetText(str(100))
			self.inputValue3.SetText(str(70))

			gameType=getObject("GameType")

			if constInfo.GUILD_WAR_TYPE_SELECT_ENABLE==0:
				gameType.Hide()

			self.typeButtonList[0].SetEvent(self.__OnClickTypeButtonNormal)
			self.typeButtonList[1].SetEvent(self.__OnClickTypeButtonWarp)
			self.typeButtonList[2].SetEvent(self.__OnClickTypeButtonCTF)

			##self.typeButtonList[0].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_NORMAL_TITLE, localeInfo.GUILDWAR_NORMAL_DESCLIST))
			##self.typeButtonList[1].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_WARP_TITLE, localeInfo.GUILDWAR_WARP_DESCLIST))
			##self.typeButtonList[2].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_CTF_TITLE, localeInfo.GUILDWAR_CTF_DESCLIST))

			self.__ClickRadioButton(self.typeButtonList, 1)

			self.SetAcceptEvent(self.__OnOK)
			self.SetCancelEvent(self.__OnCancel)
		
		def __OnClickBlockMount(self):
			if self.block_mount == 0:
				self.block_mount = 1
			else:
				self.block_mount = 0
				
		def __OnClickBlockWarrior(self):
			if self.block_warrior == 0:
				self.block_warrior = 1
			else:
				self.block_warrior = 0
		def __OnClickBlockAssassin(self):
			if self.block_assassin == 0:
				self.block_assassin = 1
			else:
				self.block_assassin = 0
		def __OnClickBlockSura(self):
			if self.block_sura == 0:
				self.block_sura = 1
			else:
				self.block_sura = 0
		def __OnClickBlockShaman(self):
			if self.block_shaman == 0:
				self.block_shaman = 1
			else:
				self.block_shaman = 0

		def __OnOK(self):
			guild_name = self.GetText(0)
			skor = int(self.GetText(1))
			maxplayer = int(self.GetText(2))
			minseviye = int(self.GetText(3))
			binek = int(self.block_mount)
			savasci = int(self.block_warrior)
			ninja = int(self.block_assassin)
			sura = int(self.block_sura)
			shaman = int(self.block_shaman)
			type = self.GetType()

			if ""==guild_name:
				import chat
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Dusman ismi yazmayi unuttun!")
				return
			elif ""==skor or 5 > skor or 500 < skor:
				import chat
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Du?man ile atılacak sava? skorunu 5 ila 500 arasında yazabilirsin!")		
				return
			elif ""==maxplayer or 2 > maxplayer or 100 < maxplayer:
				import chat
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Du?man ile atılacak sava?ta girecek maksimum oyuncu sayısını 2 ila 100 arasında yazabilirsin!")
				return
			elif ""==minseviye or 70 > minseviye or 120 < minseviye:
				import chat
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Du?man ile atılacak sava?ta minimum oyuncu seviyesini 70 ila 120 arasında yazabilirsin!")		
				return			

			appInst.instance().GetNet().SendChatPacket("/new_guild_war " + str(skor)+" "+str(minseviye)+" "+str(maxplayer)+ " "+str(binek)+ " "+str(savasci)+ " "+str(ninja)+ " "+str(sura)+ " "+str(shaman))
			appInst.instance().GetNet().SendChatPacket("/war %s %d" % (guild_name, type))
			self.Close()

			return 1

		def __OnCancel(self):
			self.Close()
			return 1

		def __OnClickTypeButtonNormal(self):
			self.__ClickTypeRadioButton(0)

		def __OnClickTypeButtonWarp(self):
			self.__ClickTypeRadioButton(1)

		def __OnClickTypeButtonCTF(self):
			self.__ClickTypeRadioButton(2)

		def __ClickTypeRadioButton(self, type):
			self.__ClickRadioButton(self.typeButtonList, type)
			self.type=type

		def __ClickRadioButton(self, buttonList, buttonIndex):
			try:
				selButton=buttonList[buttonIndex]
			except IndexError:
				return

			for eachButton in buttonList:
				eachButton.SetUp()

			selButton.Down()

		def SetTitle(self, name):
			self.board.SetTitleName(name)

		def SetNumberMode(self):
			self.inputValue.SetNumberMode()

		def SetSecretMode(self):
			self.inputValue.SetSecret()

		def SetFocus(self):
			self.inputValue.SetFocus()

		def SetMaxLength(self, length):
			width = length * 6 + 10
			self.inputValue.SetMax(length)
			self.SetSlotWidth(width)
			self.SetBoardWidth(max(width + 50, 160))

		def SetSlotWidth(self, width):
			self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
			self.inputValue.SetSize(width, self.inputValue.GetHeight())

		def SetBoardWidth(self, width):
			self.board.SetSize(max(width + 50, 160), self.GetHeight())
			self.SetSize(max(width + 50, 160), self.GetHeight())
			self.UpdateRect()

		def SetAcceptEvent(self, event):
			self.acceptButton.SetEvent(event)
			self.inputValue.SetReturnEvent(event)

		def SetCancelEvent(self, event):
			self.board.SetCloseEvent(event)
			self.cancelButton.SetEvent(event)
			self.inputValue.OnPressEscapeKey = event

		def GetType(self):
			return self.type

		def GetText(self, value):
			if value == 0:
				return self.inputValue.GetText()
			if value == 1:
				return self.inputValue1.GetText()
			if value == 2:
				return self.inputValue2.GetText()
			if value == 3:
				return self.inputValue3.GetText()

		def __CreateGameTypeToolTip(self, title, descList):
			toolTip = uiToolTip.ToolTip()
			toolTip.SetTitle(title)
			toolTip.AppendSpace(5)

			for desc in descList:
				toolTip.AutoAppendTextLine(desc)

			toolTip.AlignHorizonalCenter()
			return toolTip
else:
	class DeclareGuildWarDialog(ui.ScriptWindow):
		def __init__(self):
			ui.ScriptWindow.__init__(self)
			self.AddFlag("animated_board")
			self.type = 0
			self.isLoaded = False
	
		def Open(self):
			self.__CreateDialog()
	
			self.inputValue.SetFocus()
			self.SetCenterPosition()
			self.SetTop()
			self.Show()
	
		def Close(self):
			self.ClearDictionary()
			self.board = None
			self.acceptButton = None
			self.cancelButton = None
			self.inputSlot = None
			self.inputValue = None
			self.isLoaded = False
			self.Hide()
	
		def __CreateDialog(self):
			if self.isLoaded:
				return
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/declareguildwardialog.py")
	
			getObject = self.GetChild
			self.board = getObject("Board")
	
			self.typeButtonList = []
			self.typeButtonList.append(getObject("NormalButton"))
			self.typeButtonList.append(getObject("WarpButton"))
			self.typeButtonList.append(getObject("CTFButton"))
	
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputValue = getObject("InputValue")
	
			gameType = getObject("GameType")
	
			self.typeButtonList[0].SetEvent(self.__OnClickTypeButtonNormal)
			self.typeButtonList[1].SetEvent(self.__OnClickTypeButtonWarp)
			self.typeButtonList[2].SetEvent(self.__OnClickTypeButtonCTF)
	
			self.typeButtonList[0].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_NORMAL_TITLE, localeInfo.GUILDWAR_NORMAL_DESCLIST))
			self.typeButtonList[1].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_WARP_TITLE, localeInfo.GUILDWAR_WARP_DESCLIST))
			self.typeButtonList[2].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_CTF_TITLE, localeInfo.GUILDWAR_CTF_DESCLIST))
	
			self.__ClickRadioButton(self.typeButtonList, 0)
	
			self.SetAcceptEvent(self.__OnOK)
			self.SetCancelEvent(self.__OnCancel)
	
			self.isLoaded = True
	
		def __OnOK(self):
			text = self.GetText()
			type = self.GetType()
	
			if "" == text:
				return
	
			appInst.instance().GetNet().SendChatPacket("/war %s %d" % (text, type))
			self.Close()
	
			return 1
	
		def __OnCancel(self):
			self.Close()
			return 1
	
		def __OnClickTypeButtonNormal(self):
			self.__ClickTypeRadioButton(0)
	
		def __OnClickTypeButtonWarp(self):
			self.__ClickTypeRadioButton(1)
	
		def __OnClickTypeButtonCTF(self):
			self.__ClickTypeRadioButton(2)
	
		def __ClickTypeRadioButton(self, type):
			self.__ClickRadioButton(self.typeButtonList, type)
			self.type = type
	
		def __ClickRadioButton(self, buttonList, buttonIndex):
			try:
				selButton = buttonList[buttonIndex]
			except IndexError:
				return
	
			for eachButton in buttonList:
				eachButton.SetUp()
	
			selButton.Down()
	
		def SetTitle(self, name):
			self.board.SetTitleName(name)
	
		def SetNumberMode(self):
			self.inputValue.SetNumberMode()
	
		def SetSecretMode(self):
			self.inputValue.SetSecret()
	
		def SetFocus(self):
			self.inputValue.SetFocus()
	
		def SetMaxLength(self, length):
			width = length * 6 + 10
			self.inputValue.SetMax(length)
			self.SetSlotWidth(width)
			self.SetBoardWidth(max(width + 50, 160))
	
		def SetSlotWidth(self, width):
			self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
			self.inputValue.SetSize(width, self.inputValue.GetHeight())
	
		def SetBoardWidth(self, width):
			self.board.SetSize(max(width + 50, 160), self.GetHeight())
			self.SetSize(max(width + 50, 160), self.GetHeight())
			self.UpdateRect()
	
		def SetAcceptEvent(self, event):
			self.acceptButton.SetEvent(event)
			self.inputValue.SetReturnEvent(event)
	
		def SetCancelEvent(self, event):
			self.board.SetCloseEvent(event)
			self.cancelButton.SetEvent(event)
			self.inputValue.SetEscapeEvent(event)
	
		def GetType(self):
			return self.type
	
		def GetText(self):
			return self.inputValue.GetText()
	
		def __CreateGameTypeToolTip(self, title, descList):
			toolTip = uiToolTip.ToolTip()
			toolTip.SetTitle(title)
			toolTip.AppendSpace(5)
	
			for desc in descList:
				toolTip.AppendTextLine(desc)
	
			return toolTip


if app.ENABLE_NEW_GUILD_WAR:
	class AcceptGuildWarDialog(ui.ScriptWindow):

		def __init__(self):
			ui.ScriptWindow.__init__(self)

			self.type=1
			self.__CreateDialog()

		def Open(self, guildName, warType, guildmaxscore, guildminlevel, guildmaxuser, mount, savasci, ninja, sura, shaman):
			self.guildName=guildName
			self.warType=warType
			self.maxscore = guildmaxscore
			self.maxoyuncu = guildmaxuser
			self.__ClickSelectBlockList(mount, savasci, ninja, sura, shaman)
			self.__ClickSelectedTypeRadioButton()
			self.inputValue.SetText(guildName)
			self.inputValue1.SetText(str(guildmaxscore)+" Puan")
			self.inputValue2.SetText(str(guildmaxuser)+" Oyuncu")
			self.inputValue3.SetText(str(guildminlevel)+" Seviye")
			self.SetCenterPosition()
			self.SetTop()
			self.Show()

		def GetGuildName(self):
			return self.guildName

		def GetWarType(self):
			return self.warType
		def Close(self):
			self.ClearDictionary()
			self.board = None
			self.acceptButton = None
			self.cancelButton = None
			self.inputSlot = None
			self.inputSlot1 = None
			self.inputSlot2 = None
			self.inputSlot3 = None
			self.inputValue = None
			self.inputValue1 = None
			self.inputValue2 = None
			self.inputValue3 = None
			self.Hide()

		def __ClickSelectedTypeRadioButton(self):
			self.__ClickTypeRadioButton(self.warType)
			
		def __ClickSelectBlockList(self, mount, savasci, ninja, sura, shaman):
			if int(mount) == 1:
				self.blockButtonList[0].Down()
			else:
				self.blockButtonList[0].SetUp()
			if int(savasci) == 1:
				self.blockButtonList[1].Down()
			else:
				self.blockButtonList[1].SetUp()
			if int(ninja) == 1:
				self.blockButtonList[2].Down()
			else:
				self.blockButtonList[2].SetUp()
			if int(sura) == 1:
				self.blockButtonList[3].Down()
			else:
				self.blockButtonList[3].SetUp()
			if int(shaman) == 1:
				self.blockButtonList[4].Down()
			else:
				self.blockButtonList[4].SetUp()

		def __CreateDialog(self):

			try:
				pyScrLoader = ui.PythonScriptLoader()
				pyScrLoader.LoadScriptFile(self, "uiscript/acceptguildwardialog_new.py")
			except:
				import logging
				logging.exception("DeclareGuildWarWindow.__CreateDialog - LoadScript")
			try:
				getObject = self.GetChild
				self.board = getObject("Board")

				self.typeButtonList=[]
				self.typeButtonList.append(getObject("NormalButton"))
				self.typeButtonList.append(getObject("WarpButton"))
				self.typeButtonList.append(getObject("CTFButton"))		
				
				self.blockButtonList=[]
				block_mount = getObject("block_mount")
				self.blockButtonList.append(block_mount)
				
				block_warrior = getObject("block_warrior")
				self.blockButtonList.append(block_warrior)
				
				block_assassin = getObject("block_assassin")
				self.blockButtonList.append(block_assassin)
				
				block_sura = getObject("block_sura")
				self.blockButtonList.append(block_sura)
				
				block_shaman = getObject("block_shaman")
				self.blockButtonList.append(block_shaman)
				
				self.acceptButton = getObject("AcceptButton")
				self.cancelButton = getObject("CancelButton")
				self.inputSlot = getObject("InputSlot")
				self.inputSlot1 = getObject("InputSlotScore")
				self.inputSlot2 = getObject("InputSlotUser")
				self.inputSlot3 = getObject("InputValueMinSeviye")
				self.inputValue = getObject("InputValue")
				self.inputValue1 = getObject("InputValueScore")
				self.inputValue2 = getObject("InputValueUser")
				self.inputValue3 = getObject("InputValueMinSeviye")

				gameType=getObject("GameType")

			except:
				import logging
				logging.exception("DeclareGuildWarWindow.__CreateDialog - BindObject")

			if constInfo.GUILD_WAR_TYPE_SELECT_ENABLE==0:
				gameType.Hide()

			self.typeButtonList[0].SetEvent(self.__OnClickTypeButtonNormal)
			self.typeButtonList[1].SetEvent(self.__OnClickTypeButtonWarp)
			self.typeButtonList[2].SetEvent(self.__OnClickTypeButtonCTF)

			##self.typeButtonList[0].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_NORMAL_TITLE, localeInfo.GUILDWAR_NORMAL_DESCLIST))
			##self.typeButtonList[1].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_WARP_TITLE, localeInfo.GUILDWAR_WARP_DESCLIST))
			##self.typeButtonList[2].SetToolTipWindow(self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_CTF_TITLE, localeInfo.GUILDWAR_CTF_DESCLIST))

			self.__ClickRadioButton(self.typeButtonList, 1)

		def __OnClickTypeButtonNormal(self):
			self.__ClickSelectedTypeRadioButton()

		def __OnClickTypeButtonWarp(self):
			self.__ClickSelectedTypeRadioButton()

		def __OnClickTypeButtonCTF(self):
			self.__ClickSelectedTypeRadioButton()

		def __ClickTypeRadioButton(self, type):
			self.__ClickRadioButton(self.typeButtonList, type)
			self.type=type

		def __ClickRadioButton(self, buttonList, buttonIndex):
			try:
				selButton=buttonList[buttonIndex]
			except IndexError:
				return

			for eachButton in buttonList:
				eachButton.SetUp()

			selButton.Down()

		def SetTitle(self, name):
			self.board.SetTitleName(name)

		def SetNumberMode(self):
			self.inputValue.SetNumberMode()

		def SetSecretMode(self):
			self.inputValue.SetSecret()

		def SetFocus(self):
			self.inputValue.SetFocus()

		def SetMaxLength(self, length):
			width = length * 6 + 10
			self.inputValue.SetMax(length)
			self.SetSlotWidth(width)
			self.SetBoardWidth(max(width + 50, 160))

		def SetSlotWidth(self, width):
			self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
			self.inputValue.SetSize(width, self.inputValue.GetHeight())

		def SetBoardWidth(self, width):
			self.board.SetSize(max(width + 50, 160), self.GetHeight())
			self.SetSize(max(width + 50, 160), self.GetHeight())
			self.UpdateRect()

		def SAFE_SetAcceptEvent(self, event):
			self.SetAcceptEvent(event)

		def SAFE_SetCancelEvent(self, event):
			self.SetCancelEvent(event)

		def SetAcceptEvent(self, event):
			self.acceptButton.SetEvent(event)
			self.inputValue.OnIMEReturn = event
			self.inputValue1.OnIMEReturn = event
			self.inputValue2.OnIMEReturn = event
			self.inputValue3.OnIMEReturn = event

		def SetCancelEvent(self, event):
			self.board.SetCloseEvent(event)
			self.cancelButton.SetEvent(event)
			self.inputValue.OnPressEscapeKey = event
			self.inputValue1.OnPressEscapeKey = event
			self.inputValue2.OnPressEscapeKey = event
			self.inputValue3.OnPressEscapeKey = event

		def GetType(self):
			return self.type

		def GetText(self):
			return self.inputValue.GetText()

		def __CreateGameTypeToolTip(self, title, descList):
			toolTip = uiToolTip.ToolTip()
			toolTip.SetTitle(title)
			toolTip.AppendSpace(5)

			for desc in descList:
				toolTip.AutoAppendTextLine(desc)

			toolTip.AlignHorizonalCenter()
			return toolTip
else:
	class AcceptGuildWarDialog(ui.ScriptWindow):
		def __init__(self):
			ui.ScriptWindow.__init__(self)
			self.AddFlag("animated_board")
			self.isLoaded = False
			self.warType = 0
	
		def Open(self, guildName, warType):
			self.__CreateDialog()
	
			self.guildName = guildName
			self.warType = warType
			self.__ClickSelectedTypeRadioButton()
			self.inputValue.SetText(guildName)
			self.SetCenterPosition()
			self.SetTop()
			self.Show()
	
		def GetGuildName(self):
			return self.guildName
	
		def Close(self):
			self.ClearDictionary()
			self.board = None
			self.acceptButton = None
			self.cancelButton = None
			self.inputSlot = None
			self.inputValue = None
			self.Hide()
	
		def __ClickSelectedTypeRadioButton(self):
			self.__ClickTypeRadioButton(self.warType)
	
		def __CreateDialog(self):
			if self.isLoaded:
				return
			self.isLoaded = True
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/acceptguildwardialog.py")
	
			getObject = self.GetChild
			self.board = getObject("Board")
	
			self.typeButtonList = []
			self.typeButtonList.append(getObject("NormalButton"))
			self.typeButtonList.append(getObject("WarpButton"))
			self.typeButtonList.append(getObject("CTFButton"))
	
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputValue = getObject("InputValue")
	
			gameType = getObject("GameType")
	
			self.typeButtonList[0].SetEvent(self.__OnClickTypeButtonNormal)
			self.typeButtonList[1].SetEvent(self.__OnClickTypeButtonWarp)
			self.typeButtonList[2].SetEvent(self.__OnClickTypeButtonCTF)
	
			self.typeButtonList[0].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_NORMAL_TITLE, localeInfo.GUILDWAR_NORMAL_DESCLIST))
			self.typeButtonList[1].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_WARP_TITLE, localeInfo.GUILDWAR_WARP_DESCLIST))
			self.typeButtonList[2].SetToolTipWindow(
				self.__CreateGameTypeToolTip(localeInfo.GUILDWAR_CTF_TITLE, localeInfo.GUILDWAR_CTF_DESCLIST))
	
			self.__ClickRadioButton(self.typeButtonList, 0)
	
		def __OnClickTypeButtonNormal(self):
			self.__ClickSelectedTypeRadioButton()
	
		def __OnClickTypeButtonWarp(self):
			self.__ClickSelectedTypeRadioButton()
	
		def __OnClickTypeButtonCTF(self):
			self.__ClickSelectedTypeRadioButton()
	
		def __ClickTypeRadioButton(self, type):
			self.__ClickRadioButton(self.typeButtonList, type)
			self.warType = type
	
		def __ClickRadioButton(self, buttonList, buttonIndex):
			try:
				selButton = buttonList[buttonIndex]
			except IndexError:
				return
	
			for eachButton in buttonList:
				eachButton.SetUp()
	
			selButton.Down()
	
		def SetTitle(self, name):
			self.board.SetTitleName(name)
	
		def SetNumberMode(self):
			self.inputValue.SetNumberMode()
	
		def SetSecretMode(self):
			self.inputValue.SetSecret()
	
		def SetFocus(self):
			self.inputValue.SetFocus()
	
		def SetMaxLength(self, length):
			width = length * 6 + 10
			self.inputValue.SetMax(length)
			self.SetSlotWidth(width)
			self.SetBoardWidth(max(width + 50, 160))
	
		def SetSlotWidth(self, width):
			self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
			self.inputValue.SetSize(width, self.inputValue.GetHeight())
	
		def SetBoardWidth(self, width):
			self.board.SetSize(max(width + 50, 160), self.GetHeight())
			self.SetSize(max(width + 50, 160), self.GetHeight())
			self.UpdateRect()
	
		def SetAcceptEvent(self, event):
			self.acceptButton.SetEvent(event)
	
		def SetCancelEvent(self, event):
			self.board.SetCloseEvent(event)
			self.cancelButton.SetEvent(event)
			self.inputValue.OnPressEscapeKey = MakeEvent(event)
	
		def GetType(self):
			return self.warType
	
		def GetText(self):
			return self.inputValue.GetText()
	
		def __CreateGameTypeToolTip(self, title, descList):
			toolTip = uiToolTip.ToolTip()
			toolTip.SetTitle(title)
			toolTip.AppendSpace(5)
	
			for desc in descList:
				toolTip.AppendTextLine(desc)
	
			toolTip.SetHorizontalAlignCenter()
			return toolTip

class GuildWarScoreBoard(ui.ThinBoardOld):
	def __init__(self):
		ui.ThinBoardOld.__init__(self)
		self.Initialize()

	def Initialize(self):
		self.allyGuildID = 0
		self.enemyGuildID = 0
		self.allyDataDict = {}
		self.enemyDataDict = {}

	def Open(self, allyGuildID, enemyGuildID):

		self.allyGuildID = allyGuildID
		self.enemyGuildID = enemyGuildID

		self.SetPosition(10, wndMgr.GetScreenHeight() - 200)

		mark = ui.MarkBox()
		mark.SetParent(self)
		mark.SetIndex(allyGuildID)
		mark.SetPosition(10, 10 + 18 * 0)
		mark.Show()
		scoreText = ui.TextLine()
		scoreText.SetParent(self)
		scoreText.SetPosition(30, 10 + 18 * 0)
		scoreText.SetHorizontalAlignLeft()
		scoreText.Show()
		self.allyDataDict["NAME"] = guild.GetGuildName(allyGuildID)
		self.allyDataDict["SCORE"] = 0
		self.allyDataDict["MEMBER_COUNT"] = -1
		self.allyDataDict["MARK"] = mark
		self.allyDataDict["TEXT"] = scoreText

		mark = ui.MarkBox()
		mark.SetParent(self)
		mark.SetIndex(enemyGuildID)
		mark.SetPosition(10, 10 + 18 * 1)
		mark.Show()
		scoreText = ui.TextLine()
		scoreText.SetParent(self)
		scoreText.SetPosition(30, 10 + 18 * 1)
		scoreText.SetHorizontalAlignLeft()
		scoreText.Show()
		self.enemyDataDict["NAME"] = guild.GetGuildName(enemyGuildID)
		self.enemyDataDict["SCORE"] = 0
		self.enemyDataDict["MEMBER_COUNT"] = -1
		self.enemyDataDict["MARK"] = mark
		self.enemyDataDict["TEXT"] = scoreText

		self.__RefreshName()
		self.Show()

	def __GetDataDict(self, ID):
		if self.allyGuildID == ID:
			return self.allyDataDict
		if self.enemyGuildID == ID:
			return self.enemyDataDict

		return None

	def SetScore(self, gainGuildID, opponetGuildID, point):
		dataDict = self.__GetDataDict(gainGuildID)

		if not dataDict:
			return

		dataDict["SCORE"] = point
		self.__RefreshName()

	def UpdateMemberCount(self, guildID1, memberCount1, guildID2, memberCount2):
		dataDict1 = self.__GetDataDict(guildID1)
		dataDict2 = self.__GetDataDict(guildID2)

		if dataDict1:
			dataDict1["MEMBER_COUNT"] = memberCount1

		if dataDict2:
			dataDict2["MEMBER_COUNT"] = memberCount2
		self.__RefreshName()

	def __RefreshName(self):
		nameMaxLen = max(
			len(self.allyDataDict["NAME"]), len(self.enemyDataDict["NAME"])
		)

		if (
			-1 == self.allyDataDict["MEMBER_COUNT"]
			or -1 == self.enemyDataDict["MEMBER_COUNT"]
		):
			self.SetSize(30 + nameMaxLen * 6 + 8 * 5, 50)
			self.allyDataDict["TEXT"].SetText(
				"%s %d" % (self.allyDataDict["NAME"], self.allyDataDict["SCORE"])
			)
			self.enemyDataDict["TEXT"].SetText(
				"%s %d" % (self.enemyDataDict["NAME"], self.enemyDataDict["SCORE"])
			)
		else:
			self.SetSize(30 + nameMaxLen * 6 + 8 * 5 + 15, 50)
			self.allyDataDict["TEXT"].SetText(
				"%s(%d) %d"
				% (
					self.allyDataDict["NAME"],
					self.allyDataDict["MEMBER_COUNT"],
					self.allyDataDict["SCORE"],
				)
			)
			self.enemyDataDict["TEXT"].SetText(
				"%s(%d) %d"
				% (
					self.enemyDataDict["NAME"],
					self.enemyDataDict["MEMBER_COUNT"],
					self.enemyDataDict["SCORE"],
				)
			)


class MouseReflector(ui.Window):
	def __init__(self, parent):
		ui.Window.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		self.width = self.height = 0
		self.isDown = False

	def Down(self):
		self.isDown = True

	def Up(self):
		self.isDown = False

	def OnRender(self):
		if self.isDown:
			grp.SetColor(ui.WHITE_COLOR)
		else:
			grp.SetColor(ui.HALF_WHITE_COLOR)

		x, y = self.GetGlobalPosition()
		grp.RenderBar(x + 2, y + 2, self.GetWidth() - 4, self.GetHeight() - 4)


class EditableTextSlot(ui.ImageBox):
	def __init__(self, parent, x, y):
		ui.ImageBox.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.LoadImage("d:/ymir work/ui/public/Parameter_Slot_02.sub")

		self.mouseReflector = MouseReflector(self)
		self.mouseReflector.SetSize(self.GetWidth(), self.GetHeight())

		self.Enable = True
		self.textLine = ui.MakeTextLine(self)
		self.event = None
		self.Show()

		self.mouseReflector.UpdateRect()

	def SetText(self, text):
		self.textLine.SetText(text)

	def SetEvent(self, event):
		self.event = MakeEvent(event)

	def Disable(self):
		self.Enable = False

	def OnMouseOverIn(self):
		if not self.Enable:
			return False
		self.mouseReflector.Show()
		return True

	def OnMouseOverOut(self):
		if not self.Enable:
			return
		self.mouseReflector.Hide()

	def OnMouseLeftButtonDown(self):
		if not self.Enable:
			return
		self.mouseReflector.Down()

	def OnMouseLeftButtonUp(self):
		if not self.Enable:
			return

		self.mouseReflector.Up()
		if self.event:
			self.event()


class CheckBox(ui.ImageBox):
	def __init__(
		self,
		parent,
		x,
		y,
		event,
		filename="d:/ymir work/ui/public/Parameter_Slot_01.sub",
	):
		ui.ImageBox.__init__(self)
		self.SetParent(parent)
		self.SetPosition(x, y)
		self.LoadImage(filename)

		self.mouseReflector = MouseReflector(self)
		self.mouseReflector.SetSize(self.GetWidth(), self.GetHeight())

		image = ui.MakeImageBox(self, "d:/ymir work/ui/public/check_image.sub", 0, 0)
		image.AddFlag("not_pick")
		image.SetHorizontalAlignCenter()
		image.SetVerticalAlignCenter()
		image.Hide()
		self.Enable = True
		self.image = image
		self.event = MakeEvent(event)
		self.Show()

		self.mouseReflector.UpdateRect()

	def SetCheck(self, flag):
		if flag:
			self.image.Show()
		else:
			self.image.Hide()

	def Disable(self):
		self.Enable = False

	def OnMouseOverIn(self):
		if not self.Enable:
			return False

		self.mouseReflector.Show()
		return True

	def OnMouseOverOut(self):
		if not self.Enable:
			return False

		self.mouseReflector.Hide()
		return True

	def OnMouseLeftButtonDown(self):
		if not self.Enable:
			return False

		self.mouseReflector.Down()
		return True

	def OnMouseLeftButtonUp(self):
		if not self.Enable:
			return False

		self.mouseReflector.Up()
		self.event()
		return True


class ChangeGradeNameDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")

	def Open(self):
		self.gradeNameSlot.SetText("")
		self.gradeNameSlot.SetFocus()
		xMouse, yMouse = wndMgr.GetMousePosition()
		self.SetPosition(xMouse - self.GetWidth() / 2, yMouse + 50)
		self.SetTop()
		self.Show()

	def Close(self):
		self.gradeNameSlot.KillFocus()
		self.Hide()
		return True

	def SetGradeNumber(self, gradeNumber):
		self.gradeNumber = gradeNumber

	def GetGradeNumber(self):
		return self.gradeNumber

	def GetGradeName(self):
		return self.gradeNameSlot.GetText()

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True


class CommentSlot(ui.Window):
	TEXT_LIMIT = 35

	def __init__(self):
		ui.Window.__init__(self)

		self.slotImage = ui.MakeImageBox(
			self, "d:/ymir work/ui/public/Parameter_Slot_06.sub", 0, 0
		)
		self.slotImage.AddFlag("not_pick")

		self.slotSimpleText = ui.MakeTextLine(self)
		self.slotSimpleText.SetPosition(2, 0)

		self.slotSimpleText.SetHorizontalAlignLeft()

		self.bar = ui.SlotBar()
		self.bar.SetParent(self)
		self.bar.AddFlag("not_pick")
		self.bar.Hide()

		self.slotFullText = ui.MakeTextLine(self)
		self.slotFullText.SetPosition(2, 0)

		self.slotFullText.SetHorizontalAlignLeft()

		self.SetSize(self.slotImage.GetWidth(), self.slotImage.GetHeight())
		self.len = 0

	def SetText(self, text):
		self.len = len(text)
		if len(text) > self.TEXT_LIMIT:
			lines = grpText.GetLines(text, self.TEXT_LIMIT - 3)
			self.slotSimpleText.SetText(lines[0] + "...")
			self.bar.SetSize(self.len * 6 + 5, 17)
		else:
			self.slotSimpleText.SetText(text)

		self.slotFullText.SetText(text)
		self.slotFullText.SetPosition(2, 0)
		self.slotFullText.Hide()

	def OnMouseOverIn(self):
		if self.len > self.TEXT_LIMIT:
			self.bar.Show()
			self.slotFullText.Show()
			return True
		return False

	def OnMouseOverOut(self):
		if self.len > self.TEXT_LIMIT:
			self.bar.Hide()
			self.slotFullText.Hide()
			return True
		return False


class GuildWindow(ui.ScriptWindow):
	JOB_NAME = {
		0: localeInfo.JOB_WARRIOR,
		1: localeInfo.JOB_ASSASSIN,
		2: localeInfo.JOB_SURA,
		3: localeInfo.JOB_SHAMAN,
	}

	GUILD_SKILL_PASSIVE_SLOT = 0
	GUILD_SKILL_ACTIVE_SLOT = 1
	GUILD_SKILL_AFFECT_SLOT = 2

	GRADE_SLOT_NAME = 0
	GRADE_ADD_MEMBER_AUTHORITY = 1
	GRADE_REMOVE_MEMBER_AUTHORITY = 2
	GRADE_NOTICE_AUTHORITY = 3
	GRADE_SKILL_AUTHORITY = 4

	MEMBER_LINE_COUNT = 13

	class PageWindow(ui.ScriptWindow):

		def __init__(self, parent, filename):
			ui.ScriptWindow.__init__(self)

			self.filename = filename

			self.SetWindowName("GuildWindow {0}".format(filename))
			self.SetParent(parent)

		def GetScriptFileName(self):
			return self.filename

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")
		self.isLoaded = 0

		self.isLoaded = 0

		self.SetWindowName("GuildWindow")
		self.__Initialize()

	def __Initialize(self):
		self.board = None
		self.pageName = None
		self.tabDict = None
		self.tabButtonDict = None
		self.pickDialog = None
		self.questionDialog = None
		self.offerDialog = None
		self.popupDialog = None
		self.moneyDialog = None
		self.changeGradeNameDialog = None
		self.popup = None

		self.popupMessage = None
		self.commentSlot = None

		self.pageWindow = None
		self.tooltipSkill = None

		self.memberLinePos = 0

		self.enemyGuildNameList = []

	def Open(self):
		self.Show()
		self.SetTop()

		guildID = appInst.instance().GetNet().GetGuildID()
		self.largeMarkBox.SetIndex(guildID)
		self.largeMarkBox.SetScale(3)

	def Close(self):
		self.__CloseAllGuildMemberPageGradeComboBox()
		self.offerDialog.Close()
		self.popupDialog.Hide()
		self.changeGradeNameDialog.Hide()
		self.tooltipSkill.Hide()
		self.Hide()

		self.pickDialog = None
		self.questionDialog = None
		self.popup = None

	def Destroy(self):
		self.ClearDictionary()

		if self.offerDialog:
			self.offerDialog.Destroy()

		if self.popupDialog:
			self.popupDialog.ClearDictionary()

		if self.changeGradeNameDialog:
			self.changeGradeNameDialog.ClearDictionary()

		if self.pageWindow:
			for window in self.pageWindow.values():
				window.ClearDictionary()

		self.__Initialize()

	def Show(self):
		if self.isLoaded == 0:
			self.isLoaded = 1

			self.__LoadWindow()

		self.RefreshGuildInfoPage()
		self.RefreshGuildBoardPage()
		self.RefreshGuildMemberPage()
		self.RefreshGuildSkillPage()
		self.RefreshGuildGradePage()

		ui.ScriptWindow.Show(self)

	def __LoadWindow(self):
		global DISABLE_GUILD_SKILL

		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/GuildWindow.py")

		self.popupDialog = ui.ScriptWindow()
		pyScrLoader.LoadScriptFile(self.popupDialog, "UIScript/PopupDialog.py")

		self.changeGradeNameDialog = ChangeGradeNameDialog()
		pyScrLoader.LoadScriptFile(
			self.changeGradeNameDialog, "UIScript/ChangeGradeNameDialog.py"
		)

		self.board = self.GetChild("Board")

		self.pageWindow = {
			"GUILD_INFO": self.PageWindow(
				self.board, "UIScript/GuildWindow_GuildinfoPage.py"
			),
			"BOARD": self.PageWindow(self.board, "UIScript/GuildWindow_BoardPage.py"),
			"MEMBER": self.PageWindow(self.board, "UIScript/GuildWindow_MemberPage.py"),
			"BASE_INFO": self.PageWindow(
				self.board, "UIScript/GuildWindow_BaseInfoPage.py"
			),
			"SKILL": self.PageWindow(
				self.board, "UIScript/GuildWindow_GuildSkillPage.py"
			),
			"GRADE": self.PageWindow(self.board, "UIScript/GuildWindow_GradePage.py"),
		}

		for window in self.pageWindow.values():
			pyScrLoader.LoadScriptFile(window, window.GetScriptFileName())

		self.pageName = {
			"GUILD_INFO": localeInfo.GUILD_TILE_INFO,
			"BOARD": localeInfo.GUILD_TILE_BOARD,
			"MEMBER": localeInfo.GUILD_TILE_MEMBER,
			"BASE_INFO": localeInfo.GUILD_TILE_BASEINFO,
			"SKILL": localeInfo.GUILD_TILE_SKILL,
			"GRADE": localeInfo.GUILD_TILE_GRADE,
		}

		self.tabDict = {
			"GUILD_INFO": self.GetChild("Tab_01"),
			"BOARD": self.GetChild("Tab_02"),
			"MEMBER": self.GetChild("Tab_03"),
			"BASE_INFO": self.GetChild("Tab_04"),
			"SKILL": self.GetChild("Tab_05"),
			"GRADE": self.GetChild("Tab_06"),
		}
		self.tabButtonDict = {
			"GUILD_INFO": self.GetChild("Tab_Button_01"),
			"BOARD": self.GetChild("Tab_Button_02"),
			"MEMBER": self.GetChild("Tab_Button_03"),
			"BASE_INFO": self.GetChild("Tab_Button_04"),
			"SKILL": self.GetChild("Tab_Button_05"),
			"GRADE": self.GetChild("Tab_Button_06"),
		}

		## QuestionDialog
		self.popupMessage = self.popupDialog.GetChild("message")
		self.popupDialog.GetChild("accept").SetEvent(self.popupDialog.Hide)

		## ChangeGradeName
		self.changeGradeNameDialog.GetChild("AcceptButton").SetEvent(
			self.OnChangeGradeName
		)
		self.changeGradeNameDialog.GetChild("CancelButton").SetEvent(
			self.changeGradeNameDialog.Hide
		)
		self.changeGradeNameDialog.GetChild("Board").SetCloseEvent(
			self.changeGradeNameDialog.Hide
		)
		self.changeGradeNameDialog.gradeNameSlot = self.changeGradeNameDialog.GetChild(
			"GradeNameValue"
		)
		self.changeGradeNameDialog.gradeNameSlot.SetReturnEvent(self.OnChangeGradeName)
		self.changeGradeNameDialog.gradeNameSlot.SetEscapeEvent(
			self.changeGradeNameDialog.Close
		)

		## Comment
		self.commentSlot = self.pageWindow["BOARD"].GetChild("CommentValue")
		# TODO(tim): Use Event class
		self.commentSlot.OnKeyDown = (
			lambda key, argSelf=self: argSelf.OnKeyDownInBoardPage(key)
		)

		## RefreshButton
		self.pageWindow["BOARD"].GetChild("RefreshButton").SetEvent(
			self.OnRefreshComments
		)

		## ScrollBar
		scrollBar = self.pageWindow["MEMBER"].GetChild("ScrollBar")
		scrollBar.SetScrollEvent(self.OnScrollMemberLine)
		self.pageWindow["MEMBER"].scrollBar = scrollBar

		self.__MakeInfoPage()
		self.__MakeBoardPage()
		self.__MakeMemberPage()
		self.__MakeBaseInfoPage()
		self.__MakeSkillPage()
		self.__MakeGradePage()

		for page in self.pageWindow.values():
			page.UpdateRect()

		for key, btn in self.tabButtonDict.items():
			btn.SetEvent(Event(self.SelectPage, key))

		self.tabButtonDict["BASE_INFO"].Disable()

		if DISABLE_GUILD_SKILL:
			self.tabButtonDict["SKILL"].Disable()

		self.board.SetCloseEvent(self.Close)
		self.SelectPage("GUILD_INFO")

		self.offerDialog = uiPickMoney.PickMoneyDialog()
		self.offerDialog.LoadDialog()
		self.offerDialog.SetMax(13)
		self.offerDialog.SetTitleName(localeInfo.GUILD_OFFER_EXP)
		self.offerDialog.SetAcceptEvent(self.OnOffer)

	def __MakeInfoPage(self):
		page = self.pageWindow["GUILD_INFO"]

		page.nameSlot = page.GetChild("GuildNameValue")
		page.masterNameSlot = page.GetChild("GuildMasterNameValue")
		page.guildLevelSlot = page.GetChild("GuildLevelValue")
		page.curExpSlot = page.GetChild("CurrentExperienceValue")
		page.lastExpSlot = page.GetChild("LastExperienceValue")
		page.memberCountSlot = page.GetChild("GuildMemberCountValue")
		page.levelAverageSlot = page.GetChild("GuildMemberLevelAverageValue")
		page.uploadMarkButton = page.GetChild("UploadGuildMarkButton")
		page.uploadSymbolButton = page.GetChild("UploadGuildSymbolButton")
		page.declareWarButton = page.GetChild("DeclareWarButton")

		try:
			page.guildMoneySlot = page.GetChild("GuildMoneyValue")
		except KeyError:
			page.guildMoneySlot = None

		try:
			page.GetChild("DepositButton").SetEvent(self.__OnClickDepositButton)
			page.GetChild("WithdrawButton").SetEvent(self.__OnClickWithdrawButton)
		except KeyError:
			pass

		page.uploadMarkButton.SetEvent(self.__OnClickSelectGuildMarkButton)
		page.uploadSymbolButton.SetEvent(self.__OnClickSelectGuildSymbolButton)
		page.declareWarButton.SetEvent(self.__OnClickDeclareWarButton)
		page.GetChild("OfferButton").SetEvent(self.__OnClickOfferButton)
		page.GetChild("EnemyGuildCancel1").Hide()
		page.GetChild("EnemyGuildCancel2").Hide()
		page.GetChild("EnemyGuildCancel3").Hide()
		page.GetChild("EnemyGuildCancel4").Hide()
		page.GetChild("EnemyGuildCancel5").Hide()
		page.GetChild("EnemyGuildCancel6").Hide()

		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName1"))
		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName2"))
		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName3"))
		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName4"))
		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName5"))
		self.enemyGuildNameList.append(page.GetChild("EnemyGuildName6"))

		self.largeMarkBox = page.GetChild("LargeGuildMark")

		self.largeMarkBox.AddFlag("not_pick")

		self.markSelectDialog = uiUploadMark.MarkSelectDialog()
		self.markSelectDialog.SetSelectEvent(self.__OnSelectMark)

		self.symbolSelectDialog = uiUploadMark.SymbolSelectDialog()
		self.symbolSelectDialog.SetSelectEvent(self.__OnSelectSymbol)

	def __MakeBoardPage(self):
		i = 0
		lineStep = 20
		page = self.pageWindow["BOARD"]

		page.boardDict = {}

		for i in xrange(12):
			yPos = 25 + i * lineStep

			## NoticeMark
			noticeMarkImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/game/guild/notice_mark.sub", 5, yPos + 3
			)
			noticeMarkImage.Hide()
			page.Children.append(noticeMarkImage)

			## Name
			## 13.12.02 아랍수정
			nameSlotImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/public/Parameter_Slot_03.sub", 15, yPos
			)
			nameSlot = ui.MakeTextLine(nameSlotImage)
			page.Children.append(nameSlotImage)
			page.Children.append(nameSlot)

			## Delete Button
			deleteButton = ui.MakeButton(
				page,
				340,
				yPos + 3,
				localeInfo.GUILD_DELETE,
				"d:/ymir work/ui/public/",
				"close_button_01.sub",
				"close_button_02.sub",
				"close_button_03.sub",
			)
			deleteButton.SetEvent(Event(self.OnDeleteComment, i))
			deleteButton.SetTop()
			page.Children.append(deleteButton)

			## Comment
			## 13.12.02 아랍수정
			commentSlot = CommentSlot()
			commentSlot.SetParent(page)
			if localeInfo.IsARABIC():
				commentSlot.SetPosition(25, yPos)
			else:
				commentSlot.SetPosition(114, yPos)
			commentSlot.Show()
			page.Children.append(commentSlot)

			boardSlotList = [noticeMarkImage, nameSlot, commentSlot]
			page.boardDict[i] = boardSlotList

		## PostComment - Have to make this here for that fit tooltip's position.
		## 13.12.02 아랍수정

		postCommentButton = page.GetChild("PostCommentButton")
		postCommentButton.SetEvent(self.OnPostComment)

	def __MakeMemberPage(self):
		page = self.pageWindow["MEMBER"]

		lineStep = 20
		page.memberDict = {}

		for i in xrange(self.MEMBER_LINE_COUNT):
			inverseLineIndex = self.MEMBER_LINE_COUNT - i - 1
			yPos = 28 + inverseLineIndex * lineStep
			## 13.12.02 아랍 수정
			## Name
			nameSlotImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/public/Parameter_Slot_03.sub", 10, yPos
			)
			nameSlot = ui.MakeTextLine(nameSlotImage)
			page.Children.append(nameSlotImage)
			page.Children.append(nameSlot)

			## Grade
			gradeSlot = ui.ComboBox()
			gradeSlot.SetParent(page)
			gradeSlot.SetPosition(101, yPos - 1)
			gradeSlot.SetSize(61, 18)
			gradeSlot.SetEvent(Event(self.OnChangeMemberGrade, inverseLineIndex))
			gradeSlot.Show()
			page.Children.append(gradeSlot)

			## Job
			jobSlotImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/public/Parameter_Slot_00.sub", 170, yPos
			)
			jobSlot = ui.MakeTextLine(jobSlotImage)
			page.Children.append(jobSlotImage)
			page.Children.append(jobSlot)

			## Level
			levelSlotImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/public/Parameter_Slot_00.sub", 210, yPos
			)
			levelSlot = ui.MakeTextLine(levelSlotImage)
			page.Children.append(levelSlotImage)
			page.Children.append(levelSlot)

			## Offer
			offerSlotImage = ui.MakeImageBox(
				page, "d:/ymir work/ui/public/Parameter_Slot_00.sub", 250, yPos
			)
			offerSlot = ui.MakeTextLine(offerSlotImage)
			page.Children.append(offerSlotImage)
			page.Children.append(offerSlot)

			## General Enable
			event = Event(self.OnEnableGeneral, inverseLineIndex)
			generalEnableCheckBox = CheckBox(
				page, 297, yPos, event, "d:/ymir work/ui/public/Parameter_Slot_00.sub"
			)
			page.Children.append(generalEnableCheckBox)

			memberSlotList = [
				nameSlot,
				gradeSlot,
				jobSlot,
				levelSlot,
				offerSlot,
				generalEnableCheckBox,
			]
			page.memberDict[inverseLineIndex] = memberSlotList

	def __MakeBaseInfoPage(self):
		page = self.pageWindow["BASE_INFO"]

		page.buildingDataDict = {}

		lineStep = 20
		GUILD_BUILDING_MAX_NUM = 7

		yPos = 95 + 35

		for i in xrange(GUILD_BUILDING_MAX_NUM):

			nameSlotImage = ui.MakeSlotBar(page, 15, yPos, 78, 17)
			nameSlot = ui.MakeTextLine(nameSlotImage)
			page.Children.append(nameSlotImage)
			page.Children.append(nameSlot)
			nameSlot.SetText(localeInfo.GUILD_BUILDING_NAME)

			gradeSlotImage = ui.MakeSlotBar(page, 99, yPos, 26, 17)
			gradeSlot = ui.MakeTextLine(gradeSlotImage)
			page.Children.append(gradeSlotImage)
			page.Children.append(gradeSlot)
			gradeSlot.SetText(localeInfo.GUILD_BUILDING_GRADE)

			RESOURCE_MAX_NUM = 6
			for j in xrange(RESOURCE_MAX_NUM):
				resourceSlotImage = ui.MakeSlotBar(page, 131 + 29 * j, yPos, 26, 17)
				resourceSlot = ui.MakeTextLine(resourceSlotImage)
				page.Children.append(resourceSlotImage)
				page.Children.append(resourceSlot)
				resourceSlot.SetText(localeInfo.GUILD_GEM)

			event = lambda *arg: None
			powerSlot = CheckBox(
				page, 308, yPos, event, "d:/ymir work/ui/public/Parameter_Slot_00.sub"
			)
			page.Children.append(powerSlot)

			yPos += lineStep

	def __MakeSkillPage(self):
		page = self.pageWindow["SKILL"]

		page.skillPoint = page.GetChild("Skill_Plus_Value")
		page.passiveSlot = page.GetChild("Passive_Skill_Slot_Table")
		page.activeSlot = page.GetChild("Active_Skill_Slot_Table")
		page.affectSlot = page.GetChild("Affect_Slot_Table")
		page.gpGauge = page.GetChild("Dragon_God_Power_Gauge")
		page.gpValue = page.GetChild("Dragon_God_Power_Value")
		page.btnHealGSP = page.GetChild("Heal_GSP_Button")

		page.activeSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		page.activeSlot.SetOverInItemEvent(
			lambda slotNumber, type=self.GUILD_SKILL_ACTIVE_SLOT: self.OverInItem(
				slotNumber, type
			)
		)
		page.activeSlot.SetOverOutItemEvent(self.OverOutItem)
		page.activeSlot.SetSelectItemSlotEvent(
			lambda slotNumber, type=self.GUILD_SKILL_ACTIVE_SLOT: self.OnPickUpGuildSkill(
				slotNumber, type
			)
		)
		page.activeSlot.SetUnselectItemSlotEvent(
			lambda slotNumber, type=self.GUILD_SKILL_ACTIVE_SLOT: self.OnUseGuildSkill(
				slotNumber, type
			)
		)
		page.activeSlot.SetPressedSlotButtonEvent(
			lambda slotNumber, type=self.GUILD_SKILL_ACTIVE_SLOT: self.OnUpGuildSkill(
				slotNumber, type
			)
		)
		page.activeSlot.AppendSlotButton(
			"d:/ymir work/ui/game/windows/btn_plus_up.sub",
			"d:/ymir work/ui/game/windows/btn_plus_over.sub",
			"d:/ymir work/ui/game/windows/btn_plus_down.sub",
		)
		page.passiveSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		page.passiveSlot.SetOverInItemEvent(
			lambda slotNumber, type=self.GUILD_SKILL_PASSIVE_SLOT: self.OverInItem(
				slotNumber, type
			)
		)
		page.passiveSlot.SetOverOutItemEvent(self.OverOutItem)
		page.passiveSlot.SetPressedSlotButtonEvent(
			lambda slotNumber, type=self.GUILD_SKILL_PASSIVE_SLOT: self.OnUpGuildSkill(
				slotNumber, type
			)
		)
		page.passiveSlot.AppendSlotButton(
			"d:/ymir work/ui/game/windows/btn_plus_up.sub",
			"d:/ymir work/ui/game/windows/btn_plus_over.sub",
			"d:/ymir work/ui/game/windows/btn_plus_down.sub",
		)
		page.affectSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		page.affectSlot.SetOverInItemEvent(
			lambda slotNumber, type=self.GUILD_SKILL_AFFECT_SLOT: self.OverInItem(
				slotNumber, type
			)
		)
		page.affectSlot.SetOverOutItemEvent(self.OverOutItem)
		page.btnHealGSP.SetEvent(self.__OnOpenHealGSPBoard)

		## Passive
		"""
		for i in xrange(len(playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST)):

			slotIndex = page.passiveSlot.GetStartIndex()+i
			skillIndex = playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST[i]

			page.passiveSlot.SetSkillSlot(slotIndex, skillIndex, 0)
			page.passiveSlot.RefreshSlot()
			guild.SetSkillIndex(slotIndex, i)
		"""

		## Active
		for i in xrange(len(playerSettingModule.ACTIVE_GUILD_SKILL_INDEX_LIST)):
			slotIndex = page.activeSlot.GetStartIndex() + i
			skillIndex = playerSettingModule.ACTIVE_GUILD_SKILL_INDEX_LIST[i]

			page.activeSlot.SetSkillSlot(slotIndex, skillIndex, 0)
			page.activeSlot.SetCoverButton(slotIndex)
			page.activeSlot.RefreshSlot()
			guild.SetSkillIndex(
				slotIndex, len(playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST) + i
			)

	def __MakeGradePage(self):
		lineStep = 18
		page = self.pageWindow["GRADE"]

		page.gradeDict = {}

		for i in xrange(15):
			yPos = 22 + i * lineStep
			index = i + 1
			## 13.12.02 아랍 수정
			## GradeNumber
			if localeInfo.IsARABIC():
				gradeNumberSlotImage = ui.MakeImageBox(
					page, "d:/ymir work/ui/public/Parameter_Slot_00.sub", 310, yPos
				)
			else:
				gradeNumberSlotImage = ui.MakeImageBox(
					page, "d:/ymir work/ui/public/Parameter_Slot_00.sub", 14, yPos
				)
			gradeNumberSlot = ui.MakeTextLine(gradeNumberSlotImage)
			gradeNumberSlot.SetText(str(i + 1))
			page.Children.append(gradeNumberSlotImage)
			page.Children.append(gradeNumberSlot)

			## GradeName
			if localeInfo.IsARABIC():
				gradeNameSlot = EditableTextSlot(page, 242, yPos)
			else:
				gradeNameSlot = EditableTextSlot(page, 58, yPos)
			gradeNameSlot.SetEvent(Event(self.OnOpenChangeGradeName, index))
			page.Children.append(gradeNameSlot)

			## Invite Authority
			event = Event(self.OnCheckAuthority, index, 1 << 0)
			if localeInfo.IsARABIC():
				inviteAuthorityCheckBox = CheckBox(page, 185, yPos, event)
			else:
				inviteAuthorityCheckBox = CheckBox(page, 124, yPos, event)
			page.Children.append(inviteAuthorityCheckBox)

			## DriveOut Authority
			event = Event(self.OnCheckAuthority, index, 1 << 1)
			if localeInfo.IsARABIC():
				driveoutAuthorityCheckBox = CheckBox(page, 128, yPos, event)
			else:
				driveoutAuthorityCheckBox = CheckBox(page, 181, yPos, event)
			page.Children.append(driveoutAuthorityCheckBox)

			## Notice Authority
			event = Event(self.OnCheckAuthority, index, 1 << 2)
			if localeInfo.IsARABIC():
				noticeAuthorityCheckBox = CheckBox(page, 71, yPos, event)
			else:
				noticeAuthorityCheckBox = CheckBox(page, 238, yPos, event)
			page.Children.append(noticeAuthorityCheckBox)

			## Skill Authority
			event = Event(self.OnCheckAuthority, index, 1 << 3)
			if localeInfo.IsARABIC():
				skillAuthorityCheckBox = CheckBox(page, 14, yPos, event)
			else:
				skillAuthorityCheckBox = CheckBox(page, 295, yPos, event)
			page.Children.append(skillAuthorityCheckBox)

			gradeSlotList = [
				gradeNameSlot,
				inviteAuthorityCheckBox,
				driveoutAuthorityCheckBox,
				noticeAuthorityCheckBox,
				skillAuthorityCheckBox,
			]
			page.gradeDict[index] = gradeSlotList

		masterSlotList = page.gradeDict[1]
		for slot in masterSlotList:
			slot.Disable()

	def CanOpen(self):
		return guild.IsGuildEnable()

	def Open(self):
		self.Show()
		self.SetTop()

		guildID = appInst.instance().GetNet().GetGuildID()
		self.largeMarkBox.SetIndex(guildID)
		self.largeMarkBox.SetScale(3)
		## 13.12.02 아랍수정
		if localeInfo.IsARABIC():
			self.largeMarkBox.SetPosition(self.largeMarkBox.GetWidth() + 32, 1)

	def Close(self):
		self.__CloseAllGuildMemberPageGradeComboBox()
		self.offerDialog.Close()
		self.popupDialog.Hide()
		self.changeGradeNameDialog.Close()
		self.Hide()

		if self.tooltipSkill:
			self.tooltipSkill.Hide()

		self.pickDialog = None
		self.questionDialog = None
		self.moneyDialog = None

	def Destroy(self):
		self.ClearDictionary()
		self.board = None
		self.pageName = None
		self.tabDict = None
		self.tabButtonDict = None
		self.pickDialog = None
		self.questionDialog = None
		self.markSelectDialog = None
		self.symbolSelectDialog = None

		if self.offerDialog:
			self.offerDialog.Destroy()
			self.offerDialog = None

		if self.popupDialog:
			self.popupDialog.ClearDictionary()
			self.popupDialog = None

		if self.changeGradeNameDialog:
			self.changeGradeNameDialog.ClearDictionary()
			self.changeGradeNameDialog = None

		self.popupMessage = None
		self.commentSlot = None

		if self.pageWindow:
			for window in self.pageWindow.values():
				window.ClearDictionary()

		self.pageWindow = None
		self.tooltipSkill = None
		self.moneyDialog = None

		self.enemyGuildNameList = []

	def DeleteGuild(self):
		self.RefreshGuildInfoPage()
		self.RefreshGuildBoardPage()
		self.RefreshGuildMemberPage()
		self.RefreshGuildSkillPage()
		self.RefreshGuildGradePage()
		self.Hide()

	def SetSkillToolTip(self, tooltipSkill):
		self.tooltipSkill = tooltipSkill

	def SelectPage(self, arg):
		if "BOARD" == arg:
			self.OnRefreshComments()

		for key, btn in self.tabButtonDict.items():
			if arg != key:
				btn.SetUp()

		for key, img in self.tabDict.items():
			if arg == key:
				img.Show()
			else:
				img.Hide()

		for key, page in self.pageWindow.items():
			if arg == key:
				page.Show()
			else:
				page.Hide()

		self.board.SetTitleName(self.pageName[arg])
		self.__CloseAllGuildMemberPageGradeComboBox()

	def __CloseAllGuildMemberPageGradeComboBox(self):
		page = self.pageWindow["MEMBER"]
		for key, slotList in page.memberDict.items():
			slotList[1].CloseListBox()

	def RefreshGuildInfoPage(self):
		if self.isLoaded == 0:
			return

		global DISABLE_DECLARE_WAR
		page = self.pageWindow["GUILD_INFO"]
		page.nameSlot.SetText(guild.GetGuildName())
		page.masterNameSlot.SetText(guild.GetGuildMasterName())
		page.guildLevelSlot.SetText(str(guild.GetGuildLevel()))
		if page.guildMoneySlot:
			page.guildMoneySlot.SetText(str(guild.GetGuildMoney()))

		curExp, lastExp = guild.GetGuildExperience()
		curExp *= 100
		lastExp *= 100
		page.curExpSlot.SetText(str(curExp))
		page.lastExpSlot.SetText(str(lastExp))

		curMemberCount, maxMemberCount = guild.GetGuildMemberCount()
		if maxMemberCount == 0xFFFF:
			page.memberCountSlot.SetText(
				"%d / %s " % (curMemberCount, localeInfo.GUILD_MEMBER_COUNT_INFINITY)
			)
		else:
			page.memberCountSlot.SetText("%d / %d" % (curMemberCount, maxMemberCount))

		page.levelAverageSlot.SetText(str(guild.GetGuildMemberLevelAverage()))

		## 길드장만 길드 마크와 길드전 신청 버튼을 볼 수 있음
		mainCharacterName = player.GetMainCharacterName()
		masterName = guild.GetGuildMasterName()

		if mainCharacterName == masterName:
			page.uploadMarkButton.Show()

			if DISABLE_DECLARE_WAR:
				page.declareWarButton.Hide()
			else:
				page.declareWarButton.Show()

			if guild.HasGuildLand():
				page.uploadSymbolButton.Show()
			else:
				page.uploadSymbolButton.Hide()
		else:
			page.uploadMarkButton.Hide()
			page.declareWarButton.Hide()
			page.uploadSymbolButton.Hide()

		## Refresh 시에 길드전 정보 업데이트
		for i in xrange(guild.ENEMY_GUILD_SLOT_MAX_COUNT):
			name = guild.GetEnemyGuildName(i)
			nameTextLine = self.enemyGuildNameList[i]
			if name:
				nameTextLine.SetText(name)
			else:
				nameTextLine.SetText(localeInfo.GUILD_INFO_ENEMY_GUILD_EMPTY)

	def __GetGuildBoardCommentData(self, index):
		commentID, chrName, comment = guild.GetGuildBoardCommentData(index)
		if 0 == commentID:
			if "" == chrName:
				chrName = localeInfo.UI_NONAME
			if "" == comment:
				comment = localeInfo.UI_NOCONTENTS

		return commentID, chrName, comment

	def RefreshGuildBoardPage(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["BOARD"]

		self.BOARD_LINE_MAX_NUM = 12
		lineIndex = 0

		commentCount = guild.GetGuildBoardCommentCount()
		for i in xrange(commentCount):
			commentID, chrName, comment = self.__GetGuildBoardCommentData(i)

			if not comment:
				continue

			slotList = page.boardDict[lineIndex]

			if "!" == comment[0]:
				slotList[0].Show()
				slotList[1].SetText(chrName)
				slotList[2].SetText(comment[1:])
			else:
				slotList[0].Hide()
				slotList[1].SetText(chrName)
				slotList[2].SetText(comment)

			lineIndex += 1

		for i in xrange(self.BOARD_LINE_MAX_NUM - lineIndex):
			slotList = page.boardDict[lineIndex + i]
			slotList[0].Hide()
			slotList[1].SetText("")
			slotList[2].SetText("")

	def RefreshGuildMemberPage(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["MEMBER"]

		## ScrollBar
		count = guild.GetMemberCount()
		if count > self.MEMBER_LINE_COUNT:
			page.scrollBar.SetMiddleBarSize(
				float(self.MEMBER_LINE_COUNT) / float(count)
			)
			page.scrollBar.Show()
		else:
			page.scrollBar.Hide()
		self.RefreshGuildMemberPageGradeComboBox()
		self.RefreshGuildMemberPageMemberList()

	def RefreshGuildMemberPageMemberList(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["MEMBER"]

		for line, slotList in page.memberDict.items():

			gradeComboBox = slotList[1]
			gradeComboBox.Disable()

			if not guild.IsMember(line):
				slotList[0].SetText("")
				slotList[2].SetText("")
				slotList[3].SetText("")
				slotList[4].SetText("")
				slotList[5].SetCheck(False)
				continue

			pid, name, grade, race, level, offer, general = self.GetMemberData(line)
			if pid < 0:
				continue

			job = chr.RaceToJob(race)

			guildExperienceSummary = guild.GetGuildExperienceSummary()

			offerPercentage = 0
			if guildExperienceSummary > 0:
				offerPercentage = int(
					float(offer) / float(guildExperienceSummary) * 100.0
				)

			slotList[0].SetText(name)
			slotList[2].SetText(playerSettingModule.JOB_NAME.get(job, "?"))
			slotList[3].SetText(str(level))
			slotList[4].SetText(str(offerPercentage) + "%")
			slotList[5].SetCheck(general)
			gradeComboBox.SetCurrentItem(guild.GetGradeName(grade))
			if 1 != grade:
				gradeComboBox.Enable()

	def RefreshGuildMemberPageGradeComboBox(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["MEMBER"]

		self.CAN_CHANGE_GRADE_COUNT = 15 - 1
		for key, slotList in page.memberDict.items():

			gradeComboBox = slotList[1]
			gradeComboBox.Disable()

			if not guild.IsMember(key):
				continue

			pid, name, grade, job, level, offer, general = self.GetMemberData(key)
			if pid < 0:
				continue

			gradeComboBox.ClearItem()
			for i in xrange(self.CAN_CHANGE_GRADE_COUNT):
				gradeComboBox.InsertItem(i + 2, guild.GetGradeName(i + 2))
			gradeComboBox.SetCurrentItem(guild.GetGradeName(grade))
			if 1 != grade:
				gradeComboBox.Enable()

	def RefreshGuildSkillPage(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["SKILL"]

		curPoint, maxPoint = guild.GetDragonPowerPoint()
		maxPoint = max(maxPoint, 1)
		page.gpValue.SetText(str(curPoint) + " / " + str(maxPoint))

		percentage = (float(curPoint) / float(maxPoint) * 100) * (
			float(173) / float(95)
		)
		page.gpGauge.SetPercentage(curPoint, maxPoint)

		skillPoint = guild.GetGuildSkillPoint()
		page.skillPoint.SetText(str(skillPoint))

		page.passiveSlot.HideAllSlotButton()
		page.activeSlot.HideAllSlotButton()

		## Passive
		"""
		for i in xrange(len(playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST)):

			slotIndex = page.passiveSlot.GetStartIndex()+i
			skillIndex = playerSettingModule.PASSIVE_GUILD_SKILL_INDEX_LIST[i]
			skillLevel = guild.GetSkillLevel(slotIndex)
			skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)

			page.passiveSlot.SetSlotCount(slotIndex, skillLevel)
			if skillPoint > 0:
				if skillLevel < skillMaxLevel:
					page.passiveSlot.ShowSlotButton(slotIndex)
		"""

		## Active
		for i in xrange(len(playerSettingModule.ACTIVE_GUILD_SKILL_INDEX_LIST)):

			slotIndex = page.activeSlot.GetStartIndex() + i
			skillIndex = playerSettingModule.ACTIVE_GUILD_SKILL_INDEX_LIST[i]
			skillLevel = guild.GetSkillLevel(slotIndex)
			skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)

			page.activeSlot.SetSlotCount(slotIndex, skillLevel)

			if skillLevel <= 0:
				page.activeSlot.DisableCoverButton(slotIndex)
			else:
				page.activeSlot.EnableCoverButton(slotIndex)

			if skillPoint > 0:
				if skillLevel < skillMaxLevel:
					page.activeSlot.ShowSlotButton(slotIndex)

	def RefreshGuildGradePage(self):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["GRADE"]

		for key, slotList in page.gradeDict.items():
			name, authority = guild.GetGradeData(int(key))

			slotList[self.GRADE_SLOT_NAME].SetText(name)
			slotList[self.GRADE_ADD_MEMBER_AUTHORITY].SetCheck(
				authority & guild.AUTH_ADD_MEMBER
			)
			slotList[self.GRADE_REMOVE_MEMBER_AUTHORITY].SetCheck(
				authority & guild.AUTH_REMOVE_MEMBER
			)
			slotList[self.GRADE_NOTICE_AUTHORITY].SetCheck(
				authority & guild.AUTH_NOTICE
			)
			slotList[self.GRADE_SKILL_AUTHORITY].SetCheck(authority & guild.AUTH_SKILL)

	## GuildInfo

	def __PopupMessage(self, msg):
		self.popupMessage.SetText(msg)
		self.popupDialog.SetTop()
		self.popupDialog.Show()

	def __OnClickSelectGuildMarkButton(self):
		if guild.GetGuildLevel() < int(localeInfo.GUILD_MARK_MIN_LEVEL):
			self.__PopupMessage(localeInfo.GUILD_MARK_NOT_ENOUGH_LEVEL)
		elif not guild.MainPlayerHasAuthority(long(guild.AUTH_NOTICE)):
			self.__PopupMessage(localeInfo.GUILD_NO_NOTICE_PERMISSION)
		else:
			self.markSelectDialog.Open()

	def __OnClickSelectGuildSymbolButton(self):
		if guild.MainPlayerHasAuthority(long(guild.AUTH_NOTICE)):
			self.symbolSelectDialog.Open()
		else:
			self.__PopupMessage(localeInfo.GUILD_NO_NOTICE_PERMISSION)

	def __OnClickDeclareWarButton(self):
		inputDialog = DeclareGuildWarDialog()
		inputDialog.Open()
		self.inputDialog = inputDialog

	def __OnSelectMark(self, markFileName):
		ret = appInst.instance().GetNet().UploadMark("upload/" + markFileName)

		# MARK_BUG_FIX
		if ERROR_MARK_UPLOAD_NEED_RECONNECT == ret:
			self.__PopupMessage(localeInfo.UPLOAD_MARK_UPLOAD_NEED_RECONNECT)

		return ret
		# END_OF_MARK_BUG_FIX

	def __OnSelectSymbol(self, symbolFileName):
		appInst.instance().GetNet().UploadSymbol("upload/" + symbolFileName)

	def __OnClickOfferButton(self):

		curEXP = long(playerInst().GetPoint(player.EXP))

		if curEXP <= 100:
			self.__PopupMessage(localeInfo.GUILD_SHORT_EXP)
			return

		self.offerDialog.Open(curEXP, curEXP)

	def __OnClickDepositButton(self):
		moneyDialog = uiPickMoney.PickMoneyDialog()
		moneyDialog.LoadDialog()
		moneyDialog.SetMax(6)
		moneyDialog.SetTitleName(localeInfo.GUILD_DEPOSIT)
		moneyDialog.SetAcceptEvent(self.OnDeposit)
		moneyDialog.Open(player.GetMoney())
		self.moneyDialog = moneyDialog

	def __OnClickWithdrawButton(self):
		moneyDialog = uiPickMoney.PickMoneyDialog()
		moneyDialog.LoadDialog()
		moneyDialog.SetMax(6)
		moneyDialog.SetTitleName(localeInfo.GUILD_WITHDRAW)
		moneyDialog.SetAcceptEvent(self.OnWithdraw)
		moneyDialog.Open(guild.GetGuildMoney())
		self.moneyDialog = moneyDialog

	def __OnBlock(self):
		popup = uiCommon.PopupDialog()
		popup.SetText(localeInfo.NOT_YET_SUPPORT)
		popup.SetAcceptEvent(self.__OnClosePopupDialog)
		popup.Open()
		self.popup = popup

	def __OnClosePopupDialog(self):
		self.popup = None

	def OnDeposit(self, money):  # T95GUILD_WITHDRAW
		appInst.instance().GetNet().SendGuildDepositMoneyPacket(money)

	def OnWithdraw(self, money):
		appInst.instance().GetNet().SendGuildWithdrawMoneyPacket(money)

	def OnOffer(self, exp):
		appInst.instance().GetNet().SendGuildOfferPacket(exp)

	## Board
	def OnPostComment(self):
		text = self.commentSlot.GetText()

		if not text:
			return False

		appInst.instance().GetNet().SendGuildPostCommentPacket(text[:50])
		self.commentSlot.SetText("")
		return True

	def OnDeleteComment(self, index):
		commentID, chrName, comment = self.__GetGuildBoardCommentData(index)
		appInst.instance().GetNet().SendGuildDeleteCommentPacket(commentID)

	def OnRefreshComments(self):
		appInst.instance().GetNet().SendGuildRefreshCommentsPacket(0)

	def OnKeyDownInBoardPage(self, key):
		if key == app.VK_F5:
			self.OnRefreshComments()

		if key == app.VK_RETURN:
			self.OnPostComment()

		return True

	## Member
	## OnEnableGeneral
	def OnChangeMemberGrade(self, lineIndex, gradeNumber):
		PID = guild.MemberIndexToPID(lineIndex + self.memberLinePos)
		appInst.instance().GetNet().SendGuildChangeMemberGradePacket(PID, gradeNumber)

	def OnEnableGeneral(self, lineIndex):
		if not guild.IsMember(lineIndex):
			return

		pid, name, grade, job, level, offer, general = self.GetMemberData(lineIndex)
		if pid < 0:
			return

		appInst.instance().GetNet().SendGuildChangeMemberGeneralPacket(pid, 1 - general)

	## Grade
	def OnOpenChangeGradeName(self, arg):
		self.changeGradeNameDialog.SetGradeNumber(arg)
		self.changeGradeNameDialog.Open()

	def OnChangeGradeName(self):
		self.changeGradeNameDialog.Close()
		gradeNumber = self.changeGradeNameDialog.GetGradeNumber()
		gradeName = self.changeGradeNameDialog.GetGradeName()

		if len(gradeName) == 0:
			gradeName = localeInfo.GUILD_DEFAULT_GRADE

		appInst.instance().GetNet().SendGuildChangeGradeNamePacket(
			gradeNumber, gradeName
		)
		return True

	def OnCheckAuthority(self, argIndex, argAuthority):
		name, authority = guild.GetGradeData(argIndex)
		appInst.instance().GetNet().SendGuildChangeGradeAuthorityPacket(
			argIndex, long(authority ^ argAuthority)
		)

	def OnScrollMemberLine(self):
		scrollBar = self.pageWindow["MEMBER"].scrollBar
		pos = scrollBar.GetPos()

		count = guild.GetMemberCount()
		newLinePos = int(float(count - self.MEMBER_LINE_COUNT) * pos)

		if newLinePos != self.memberLinePos:
			self.memberLinePos = newLinePos
			self.RefreshGuildMemberPageMemberList()
			self.__CloseAllGuildMemberPageGradeComboBox()

	def GetMemberData(self, localPos):
		return guild.GetMemberData(localPos + self.memberLinePos)

	## Guild Skill
	def __OnOpenHealGSPBoard(self):
		curPoint, maxPoint = guild.GetDragonPowerPoint()

		if maxPoint - curPoint <= 0:
			self.__PopupMessage(localeInfo.GUILD_CANNOT_HEAL_GSP_ANYMORE)
			return

		pickDialog = uiPickMoney.PickMoneyDialog()
		pickDialog.LoadDialog()
		pickDialog.SetMax(9)
		pickDialog.SetTitleName(localeInfo.GUILD_HEAL_GSP)
		pickDialog.SetAcceptEvent(self.__OnOpenHealGSPQuestionDialog)
		pickDialog.Open(maxPoint - curPoint, 1)
		self.pickDialog = pickDialog

	def __OnOpenHealGSPQuestionDialog(self, healGSP):
		money = healGSP * 100

		questionDialog = uiCommon.QuestionDialog()
		questionDialog.SetText(localeInfo.GUILD_DO_YOU_HEAL_GSP.format(money, healGSP))
		questionDialog.SetAcceptEvent(self.__OnHealGSP)
		questionDialog.SetCancelEvent(self.__OnCloseQuestionDialog)
		questionDialog.SetWidth(400)
		questionDialog.Open()
		questionDialog.healGSP = healGSP
		self.questionDialog = questionDialog

	def __OnHealGSP(self):
		appInst.instance().GetNet().SendGuildChargeGSPPacket(
			self.questionDialog.healGSP
		)
		self.__OnCloseQuestionDialog()

	def __OnCloseQuestionDialog(self):
		if self.questionDialog:
			self.questionDialog.Close()
		self.questionDialog = None

	def OnPickUpGuildSkill(self, skillSlotIndex, type):
		mouseController = mouseModule.mouseController

		if not mouseController.isAttached():

			skillIndex = playerInst().GetSkillIndex(skillSlotIndex)
			skillLevel = guild.GetSkillLevel(skillSlotIndex)

			if skill.CanUseSkill(skillIndex) and skillLevel > 0:

				if app.IsPressed(app.VK_CONTROL):
					player.RequestAddToEmptyLocalQuickSlot(
						player.SLOT_TYPE_SKILL, skillSlotIndex
					)
					return

				mouseController.AttachObject(
					self, player.SLOT_TYPE_SKILL, skillSlotIndex, skillIndex
				)

		else:
			mouseController.DeattachObject()

	def OnUseGuildSkill(self, slotNumber, type):
		skillIndex = playerInst().GetSkillIndex(slotNumber)
		skillLevel = guild.GetSkillLevel(slotNumber)

		if skillLevel <= 0:
			return

		player.UseGuildSkill(slotNumber)

	def OnUpGuildSkill(self, slotNumber, type):
		skillIndex = playerInst().GetSkillIndex(slotNumber)
		appInst.instance().GetNet().SendChatPacket("/gskillup " + str(skillIndex))

	def OnUseSkill(self, slotNumber, coolTime):
		if self.isLoaded == 0:
			return

		page = self.pageWindow["SKILL"]

		if page.activeSlot.HasSlot(slotNumber):
			page.activeSlot.SetSlotCoolTime(slotNumber, coolTime)

	def OnStartGuildWar(self, guildSelf, guildOpp):
		if self.isLoaded == 0:
			return

		if guild.GetGuildID() != guildSelf:
			return

		guildName = guild.GetGuildName(guildOpp)
		for guildNameTextLine in self.enemyGuildNameList:
			if localeInfo.GUILD_INFO_ENEMY_GUILD_EMPTY == guildNameTextLine.GetText():
				guildNameTextLine.SetText(guildName)
				return

	def OnEndGuildWar(self, guildSelf, guildOpp):
		if self.isLoaded == 0:
			return

		if guild.GetGuildID() != guildSelf:
			return

		guildName = guild.GetGuildName(guildOpp)
		for guildNameTextLine in self.enemyGuildNameList:
			if guildName == guildNameTextLine.GetText():
				guildNameTextLine.SetText(localeInfo.GUILD_INFO_ENEMY_GUILD_EMPTY)
				return

	## ToolTip
	def OverInItem(self, slotNumber, type):
		if mouseModule.mouseController.isAttached():
			return

		if self.tooltipSkill:
			skillIndex = playerInst().GetSkillIndex(slotNumber)
			skillLevel = guild.GetSkillLevel(slotNumber)

			self.tooltipSkill.SetSkill(skillIndex, skillLevel)

	def OverOutItem(self):
		if self.tooltipSkill:
			self.tooltipSkill.HideToolTip()

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True


class BuildGuildBuildingChangeWindow(ui.ScriptWindow):
	SHOW_LIST_MAX = 10
	LIST_BOX_BASE_WIDHT = 165

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")

		self.IsOpen = False
		self.ChangeBuildingList = None
		self.Scrollbar = None
		self.PositionButton = None
		self.DeleteButton = None
		self.AcceptButton = None
		self.CancelButton = None
		self.ctrlRotationZ = None
		self.PositionChangeMode = False
		self.ChangePositionBaseX = 0
		self.ChangePositionBaseY = 0
		self.SelectObjectIndex = 0
		self.x = 0
		self.y = 0
		self.z = 0
		self.rot_x = 0
		self.rot_y = 0
		self.rot_z = 0
		self.rot_x_limit = 0
		self.rot_y_limit = 0
		self.rot_z_limit = 0
		self.BuildGuildBuildingWindow = None
		self.questionDialog = None
		self.emptyindex = 0
		self.selectindexlist = 0

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(
				self, "uiscript/BuildGuildBuildingChangeWindow.py"
			)
		except:
			import logging

			logging.exception(
				"BuildGuildBuildingChangeWindow.__LoadWindow - LoadScript"
			)

		try:
			self.GetChild("Board").SetCloseEvent(self.Close)
			self.ChangeBuildingList = self.GetChild("ChangeBuildingList")
			self.ChangeBuildingList.SetEvent(self.SelectList)
			self.Scrollbar = self.GetChild("ChangeListScrollBar")
			self.Scrollbar.SetScrollEvent(self.__OnScrollBuildingList)
			self.Scrollbar.Hide()
			self.PositionButton = self.GetChild("PositionButton")
			self.PositionButton.SetEvent(self.SelectObject)
			self.DeleteButton = self.GetChild("DeleteButton")
			self.DeleteButton.SetEvent(
				Event(
					self.MakeQuestionDialog,
					localeInfo.GUILD_BUILDING_FIX_DELETE,
					self.Delete,
				)
			)
			self.AcceptButton = self.GetChild("AcceptButton")
			self.AcceptButton.SetEvent(
				Event(
					self.MakeQuestionDialog2,
					localeInfo.GUILD_BUILDING_FIX_CHANGE_GOLD,
					localeInfo.GUILD_BUILDING_FIX_CHANGE,
					self.Accept,
				)
			)
			self.CancelButton = self.GetChild("CancelButton")
			self.CancelButton.SetEvent(self.Close)
			self.ctrlRotationZ = self.GetChild("BuildingRotationZ")
			self.ctrlRotationZ.SetEvent(self.__OnChangeRotation)
			self.ctrlRotationZ.SetSliderPos(0.5)
			if localeInfo.IsARABIC():
				self.ChangeBuildingList.SetPosition(-9, 1)
				self.ChangeBuildingList.SetWindowHorizontalAlignCenter()
		except:
			import logging

			logging.exception("BuildGuildBuildingWindow.__LoadWindow - BindObject")

	def Open(self):
		self.SetTop()
		self.Show()
		self.IsOpen = True
		self.SetChangeBuildingList()
		if self.BuildGuildBuildingWindow:
			self.BuildGuildBuildingWindow.Hide()

	def Close(self):
		if not self.SelectObjectIndex == 0:
			guildObject = guildMgr().GetGuildObject(
				self.ChangeBuildingList.GetSelectedItem()
			)
			rot_z = guildObject.zRot
			chr.SelectInstance(int(self.SelectObjectIndex))
			chr.SetPixelPosition(int(guildObject.x), int(guildObject.y), int(self.z))
			chr.SetRotationAll(0, 0, rot_z)
		chr.DeleteInstance(self.emptyindex)
		self.RealClose()

	def DestoryWindow(self):
		self.BuildGuildBuildingWindow = None
		self.Close()
		self.ClearDictionary()
		self.IsOpen = False
		self.ChangeBuildingList = None
		self.Scrollbar = None
		self.PositionButton = None
		self.DeleteButton = None
		self.AcceptButton = None
		self.CancelButton = None
		self.ctrlRotationZ = None
		self.PositionChangeMode = False
		self.ChangePositionBaseX = 0
		self.ChangePositionBaseY = 0
		self.SelectObjectIndex = 0
		self.x = 0
		self.y = 0
		self.z = 0
		self.rot_x = 0
		self.rot_y = 0
		self.rot_z = 0
		self.rot_z_limit = 0
		self.questionDialog = None
		self.emptyindex = 0
		self.selectindexlist = 0

	def RealClose(self):
		self.Hide()
		self.IsOpen = False
		if self.BuildGuildBuildingWindow:
			self.BuildGuildBuildingWindow.ChangeShow()
			self.BuildGuildBuildingWindow = None

	def AllClose(self):
		if not self.SelectObjectIndex == 0:
			guildObject = guildMgr().GetGuildObject(
				self.ChangeBuildingList.GetSelectedItem()
			)
			rot_z = guildObject.zRot
			chr.SelectInstance(int(self.SelectObjectIndex))
			chr.SetPixelPosition(int(guildObject.x), int(guildObject.y), int(self.z))
			chr.SetRotationAll(0, 0, rot_z)
		chr.DeleteInstance(self.emptyindex)
		self.Hide()
		self.IsOpen = False
		if self.BuildGuildBuildingWindow:
			self.BuildGuildBuildingWindow.Close()

	def SetBuildGuildBuildingWindow(self, window):
		self.BuildGuildBuildingWindow = window
		self.emptyindex = self.BuildGuildBuildingWindow.START_INSTANCE_INDEX

	def __OnChangeRotation(self):
		self.rot_z = (self.ctrlRotationZ.GetSliderPos() * 360 + 180) % 360
		chr.SelectInstance(int(self.SelectObjectIndex))
		chr.SetRotationAll(self.rot_x, self.rot_y, self.rot_z)

	def Accept(self):
		## 위치 정보 업데이트 패킷.
		appInst.instance().GetNet().SendChatPacket(
			"/build u %d %d %d %d %d %d"
			% (
				self.SelectObjectIndex,
				int(self.x),
				int(self.y),
				self.rot_x,
				self.rot_y,
				self.rot_z,
			)
		)
		self.SelectObjectIndex = 0
		self.CloseQuestionDialog()
		self.AllClose()

	def Delete(self):
		## 삭제 패킷 커맨드
		appInst.instance().GetNet().SendChatPacket(
			"/build d %d" % self.SelectObjectIndex
		)
		self.SelectObjectIndex = 0
		self.CloseQuestionDialog()
		self.AllClose()

	def Update(self):
		if self.PositionChangeMode == True:
			x, y, z = background.GetPickingPoint()
			self.x = x
			self.y = y
			self.z = z
			chr.SelectInstance(int(self.SelectObjectIndex))
			chr.SetPixelPosition(int(x), int(y), int(z))

	def CreateEmptyObject(self, race, objectx, objecty, rot_z):

		idx = self.emptyindex

		chr.DeleteInstance(idx)

		chr.CreateInstance(idx)
		chr.SelectInstance(idx)
		chr.SetVirtualID(idx)
		chr.SetInstanceType(chr.INSTANCE_TYPE_OBJECT)

		chr.SetRace(race)
		chr.SetArmor(0)
		chr.Refresh()
		chr.SetLoopMotion(chr.MOTION_WAIT)
		chr.SetBlendRenderMode(idx, 0.55)
		chr.SetPixelPosition(int(objectx), int(objecty), int(self.z))
		chr.SetRotationAll(0.0, 0.0, rot_z)

	def SelectObject(self):
		ChangeBuilding = self.ChangeBuildingList.GetSelectedItem()
		guildObject = guildMgr().GetGuildObject(ChangeBuilding)

		rot_z = guildObject.zRot
		self.CreateEmptyObject(guildObject.vnum, guildObject.x, guildObject.y, rot_z)

		idx = guildObject.vid
		self.SelectObjectIndex = idx
		self.PositionChangeMode = True
		self.Hide()

	def SetChangeBuildingList(self):
		self.ChangeBuildingList.ClearItem()
		self.ChangeBuildingList.SetBasePos(0)
		##scrollbar
		if guildMgr().GetGuildObjectCount() > self.SHOW_LIST_MAX:
			self.ChangeBuildingList.SetSize(
				self.LIST_BOX_BASE_WIDHT, self.ChangeBuildingList.GetHeight()
			)
			if localeInfo.IsARABIC():
				self.ChangeBuildingList.SetPosition(9, 1)
			self.Scrollbar.SetMiddleBarSize(
				float(self.SHOW_LIST_MAX) / float(guildMgr().GetGuildObjectCount())
			)
			self.Scrollbar.SetPos(0.0)
			self.Scrollbar.Show()
		else:
			self.ChangeBuildingList.SetSize(
				self.LIST_BOX_BASE_WIDHT + 15, self.ChangeBuildingList.GetHeight()
			)
			self.Scrollbar.Hide()

		index = 0
		for guildObject in guildMgr().GetGuildObjects():
			for data in guildMgr().GetObjectProto():
				if data and data.vnum == guildObject.vnum:
					name = guildMgr().GetObjectName(guildObject.vnum)
					self.ChangeBuildingList.InsertItem(index, name)
					index += 1
		self.ChangeBuildingList.SelectItem(0)
		self.SetCameraSetting(0)
		guildObject = guildMgr().GetGuildObject(0)
		idx = guildObject.vid
		self.SelectObjectIndex = idx

	def __OnScrollBuildingList(self):
		itemCount = guildMgr().GetGuildObjectCount()
		pos = self.Scrollbar.GetPos() * (itemCount - self.SHOW_LIST_MAX)
		self.ChangeBuildingList.SetBasePos(int(pos))

	def SelectList(self):

		if not self.selectindexlist == self.ChangeBuildingList.GetSelectedItem():
			guildObject = guildMgr().GetGuildObject(self.selectindexlist)
			rot_z = guildObject.zRot
			chr.SelectInstance(int(self.SelectObjectIndex))
			chr.SetPixelPosition(int(guildObject.x), int(guildObject.y), int(self.z))
			chr.SetRotationAll(0, 0, rot_z)

		ChangeBuilding = self.ChangeBuildingList.GetSelectedItem()
		if ChangeBuilding >= len(guildMgr().GetObjectProto()):
			return

		guildObject = guildMgr().GetGuildObject(ChangeBuilding)

		index = 0
		for data in guildMgr().GetObjectProto():
			if data.vnum == guildObject.vnum:
				self.__SetBuildingData(data)
			index += 1

		self.SetCameraSetting(int(ChangeBuilding))
		self.PositionChangeMode = False
		idx = guildObject.vid
		self.SelectObjectIndex = idx
		self.selectindexlist = ChangeBuilding
		self.rot_z = guildObject.zRot

		if self.rot_z == 180:
			self.ctrlRotationZ.SetSliderPos(0.0)
		elif self.rot_z > 180:
			sliderpos = (self.rot_z - 180.0) / 360.0
			self.ctrlRotationZ.SetSliderPos(sliderpos)
		elif self.rot_z < 180:
			sliderpos = (360.0 + self.rot_z - 180.0) / 360.0
			self.ctrlRotationZ.SetSliderPos(sliderpos)
		else:
			self.ctrlRotationZ.SetSliderPos(0.5)

	def __SetBuildingData(self, data):
		self.rot_z_limit = data.rotationLimits[2]
		self.ctrlRotationZ.Enable()
		if 0 == self.rot_z_limit:
			self.ctrlRotationZ.Disable()
		self.ctrlRotationZ.SetSliderPos(0.5)

	def SetCameraSetting(self, index):
		guildObject = guildMgr().GetGuildObject(index)

		x, y, z = player.GetMainCharacterPosition()
		self.x = guildObject.x
		self.y = guildObject.y
		self.z = z
		app.SetCameraSetting(
			int(guildObject.x), int(-guildObject.y), int(z), 7000, 0, 30
		)

	def IsPositionChangeMode(self):
		return self.PositionChangeMode

	def EndPositionChangeMode(self):
		app.SetCameraSetting(int(self.x), int(-self.y), int(self.z), 7000, 0, 30)
		self.PositionChangeMode = False
		self.Show()

	def IsOpen(self):
		return self.IsOpen

	def MakeQuestionDialog(self, str, acceptevent):
		questionDialog = uiCommon.QuestionDialog()
		questionDialog.SetText(str)
		questionDialog.SetAcceptEvent(acceptevent)
		questionDialog.SetCancelEvent(self.CloseQuestionDialog)
		questionDialog.Open()
		self.questionDialog = questionDialog

	def MakeQuestionDialog2(self, str1, str2, acceptevent):
		questionDialog = uiCommon.QuestionDialog2()
		questionDialog.SetText1(str1)
		questionDialog.SetText2(str2)
		questionDialog.SetAcceptEvent(acceptevent)
		questionDialog.SetCancelEvent(self.CloseQuestionDialog)
		questionDialog.Open()
		self.questionDialog = questionDialog

	def CloseQuestionDialog(self):
		if self.questionDialog:
			self.questionDialog.Close()
		self.questionDialog = None

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.OnPressEscapeKey()
			return True
		return False

	def OnPressEscapeKey(self):
		self.Close()
		return True


class BuildGuildBuildingWindow(ui.ScriptWindow):
	GUILD_CATEGORY_LIST = (
		(BUILDING_HEADQUARTERS, localeInfo.GUILD_HEADQUARTER),
		(BUILDING_FACILITY, localeInfo.GUILD_FACILITY),
		(BUILDING_OBJECT, localeInfo.GUILD_OBJECT),
	)

	MODE_VIEW = 0
	MODE_POSITIONING = 1
	MODE_PREVIEW = 2

	BUILDING_ALPHA = 0.55

	ENABLE_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
	DISABLE_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)

	BUILDINGLIST_BASE_WIDTH = 135

	START_INSTANCE_INDEX = 123450

	# WALL_SET_INSTANCE = 14105

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")

		self.closeEvent = None
		self.popup = None
		self.mode = self.MODE_VIEW
		self.race = 0
		self.type = None
		self.x = 0
		self.y = 0
		self.z = 0
		self.rot_x = 0
		self.rot_y = 0
		self.rot_z = 0
		self.rot_x_limit = 0
		self.rot_y_limit = 0
		self.rot_z_limit = 0
		self.needMoney = 0
		self.needStoneCount = 0
		self.needLogCount = 0
		self.needPlywoodCount = 0

		# self.index = 0
		self.indexList = []
		self.raceList = []
		self.posList = []
		self.rotList = []

		index = 0
		for category in self.GUILD_CATEGORY_LIST:
			self.categoryList.InsertItem(index, category[1])
			index += 1

		self.wndBuildingChangeWindow = None
		self.wndBuildingChangeWindow = BuildGuildBuildingChangeWindow()
		self.ChangeButton = None

	def __LoadWindow(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "UIScript/BuildGuildBuildingWindow.py")

		self.board = self.GetChild("Board")
		self.categoryList = self.GetChild("CategoryList")
		self.buildingList = self.GetChild("BuildingList")
		self.listScrollBar = self.GetChild("ListScrollBar")
		self.positionButton = self.GetChild("PositionButton")
		self.previewButton = self.GetChild("PreviewButton")
		self.posValueX = self.GetChild("BuildingPositionXValue")
		self.posValueY = self.GetChild("BuildingPositionYValue")
		self.ctrlRotationX = self.GetChild("BuildingRotationX")
		self.ctrlRotationY = self.GetChild("BuildingRotationY")
		self.ctrlRotationZ = self.GetChild("BuildingRotationZ")
		self.buildingPriceValue = self.GetChild("BuildingPriceValue")
		self.buildingMaterialStoneValue = self.GetChild("BuildingMaterialStoneValue")
		self.buildingMaterialLogValue = self.GetChild("BuildingMaterialLogValue")
		self.buildingMaterialPlywoodValue = self.GetChild(
			"BuildingMaterialPlywoodValue"
		)

		self.positionButton.SetEvent(self.__OnSelectPositioningMode)
		self.previewButton.SetToggleDownEvent(self.__OnEnterPreviewMode)
		self.previewButton.SetToggleUpEvent(self.__OnLeavePreviewMode)
		self.ctrlRotationX.SetEvent(self.__OnChangeRotation)
		self.ctrlRotationY.SetEvent(self.__OnChangeRotation)
		self.ctrlRotationZ.SetEvent(self.__OnChangeRotation)
		self.listScrollBar.SetScrollEvent(self.__OnScrollBuildingList)

		self.GetChild("CategoryList").SetEvent(self.__OnSelectCategory)
		self.GetChild("BuildingList").SetEvent(self.__OnSelectBuilding)
		self.GetChild("AcceptButton").SetEvent(self.Build)
		self.GetChild("CancelButton").SetEvent(self.Close)
		self.board.SetCloseEvent(self.Close)

		self.ChangeButton = self.GetChild("ChangeButton")
		self.ChangeButton.SetEvent(self.__ChangeWindowOpen)

	def __ChangeWindowOpen(self):
		if self.wndBuildingChangeWindow:
			if not self.IsPreviewMode():
				if guildMgr().GetGuildObjectCount() != 0:
					self.__DeleteInstance()
					self.wndBuildingChangeWindow.SetBuildGuildBuildingWindow(self)
					self.wndBuildingChangeWindow.Open()
				else:
					self.__PopupDialog(localeInfo.GUILD_BUILDING_FIX_NEED_BUILDING)

	def IsPositionChangeMode(self):
		if self.wndBuildingChangeWindow:
			return self.wndBuildingChangeWindow.IsPositionChangeMode()

	def EndPositionChangeMode(self):
		if self.wndBuildingChangeWindow:
			return self.wndBuildingChangeWindow.EndPositionChangeMode()

	def ChangeWindowUpdate(self):
		if self.wndBuildingChangeWindow:
			return self.wndBuildingChangeWindow.Update()

	def __CreateWallBlock(self, race, x, y, rot=0.0):
		idx = self.START_INSTANCE_INDEX + len(self.indexList)
		self.indexList.append(idx)
		self.raceList.append(race)
		self.posList.append((x, y))
		self.rotList.append(rot)
		chr.CreateInstance(idx)
		chr.SelectInstance(idx)
		chr.SetVirtualID(idx)
		chr.SetInstanceType(chr.INSTANCE_TYPE_OBJECT)

		chr.SetRace(race)
		chr.SetArmor(0)
		chr.Refresh()
		chr.SetLoopMotion(chr.MOTION_WAIT)
		chr.SetBlendRenderMode(idx, self.BUILDING_ALPHA)
		chr.SetRotationAll(0.0, 0.0, rot)

		self.ctrlRotationX.SetSliderPos(0.5)
		self.ctrlRotationY.SetSliderPos(0.5)
		self.ctrlRotationZ.SetSliderPos(0.5)

	def __GetObjectSize(self, race):
		idx = self.START_INSTANCE_INDEX + 1000
		chr.CreateInstance(idx)
		chr.SelectInstance(idx)
		chr.SetVirtualID(idx)
		chr.SetInstanceType(chr.INSTANCE_TYPE_OBJECT)

		chr.SetRace(race)
		chr.SetArmor(0)
		chr.Refresh()
		chr.SetLoopMotion(chr.MOTION_WAIT)
		sx, sy, ex, ey = chr.GetBoundBoxOnlyXY(idx)
		chr.DeleteInstance(idx)
		return sx, sy, ex, ey

	def __GetBuildInPosition(self):
		zList = [
			background.GetHeight(self.x + self.sxPos, self.y + self.syPos),
			background.GetHeight(self.x + self.sxPos, self.y + self.eyPos),
			background.GetHeight(self.x + self.exPos, self.y + self.syPos),
			background.GetHeight(self.x + self.exPos, self.y + self.eyPos),
			background.GetHeight(
				self.x + (self.exPos + self.sxPos) / 2,
				self.y + (self.eyPos + self.syPos) / 2,
			),
		]

		zList.sort()
		return zList[3]

	def __CreateBuildInInstance(self, race):
		self.__DeleteInstance()

		object_base = race - race % 10

		door_minX, door_minY, door_maxX, door_maxY = self.__GetObjectSize(
			object_base + 4
		)
		corner_minX, corner_minY, corner_maxX, corner_maxY = self.__GetObjectSize(
			object_base + 1
		)
		line_minX, line_minY, line_maxX, line_maxY = self.__GetObjectSize(
			object_base + 2
		)
		line_width = line_maxX - line_minX
		line_width_half = line_width / 2

		X_SIZE_STEP = 2 * 2  ## 2의 단위로만 증가해야 함
		Y_SIZE_STEP = 8
		sxPos = door_maxX - corner_minX + (line_width_half * X_SIZE_STEP)
		exPos = -sxPos
		syPos = 0
		eyPos = -(corner_maxY * 2 + line_width * Y_SIZE_STEP)

		self.sxPos = sxPos
		self.syPos = syPos
		self.exPos = exPos
		self.eyPos = eyPos

		z = self.__GetBuildInPosition()

		## Door
		self.__CreateWallBlock(object_base + 4, 0.0, syPos)

		## Corner
		self.__CreateWallBlock(object_base + 1, sxPos, syPos)
		self.__CreateWallBlock(object_base + 1, exPos, syPos, 270.0)
		self.__CreateWallBlock(object_base + 1, sxPos, eyPos, 90.0)
		self.__CreateWallBlock(object_base + 1, exPos, eyPos, 180.0)

		## Line
		lineBlock = object_base + 2
		line_startX = -door_maxX - line_minX - (line_width_half * X_SIZE_STEP)
		self.__CreateWallBlock(lineBlock, line_startX, eyPos)
		self.__CreateWallBlock(lineBlock, line_startX + line_width * 1, eyPos)
		self.__CreateWallBlock(lineBlock, line_startX + line_width * 2, eyPos)
		self.__CreateWallBlock(lineBlock, line_startX + line_width * 3, eyPos)
		for i in xrange(X_SIZE_STEP):
			self.__CreateWallBlock(
				lineBlock, line_startX + line_width * (3 + i + 1), eyPos
			)
		for i in xrange(X_SIZE_STEP / 2):
			self.__CreateWallBlock(
				lineBlock, door_minX - line_maxX - line_width * i, syPos
			)
			self.__CreateWallBlock(
				lineBlock, door_maxX - line_minX + line_width * i, syPos
			)
		for i in xrange(Y_SIZE_STEP):
			self.__CreateWallBlock(
				lineBlock, sxPos, line_minX + corner_minX - line_width * i, 90.0
			)
			self.__CreateWallBlock(
				lineBlock, exPos, line_minX + corner_minX - line_width * i, 90.0
			)

		self.SetBuildingPosition(int(self.x), int(self.y), self.__GetBuildInPosition())

	def __DeleteInstance(self):
		if not self.indexList:
			return

		for index in self.indexList:
			chr.DeleteInstance(index)

		self.indexList = []
		self.raceList = []
		self.posList = []
		self.rotList = []

	def __CreateInstance(self, race):
		self.__DeleteInstance()

		self.race = race

		idx = self.START_INSTANCE_INDEX
		self.indexList.append(idx)
		self.posList.append((0, 0))
		self.rotList.append(0)

		chr.CreateInstance(idx)
		chr.SelectInstance(idx)
		chr.SetVirtualID(idx)
		chr.SetInstanceType(chr.INSTANCE_TYPE_OBJECT)

		chr.SetRace(race)
		chr.SetArmor(0)
		chr.Refresh(chr.MOTION_WAIT, True)
		chr.SetBlendRenderMode(idx, self.BUILDING_ALPHA)

		self.SetBuildingPosition(int(self.x), int(self.y), 0)
		self.ctrlRotationX.SetSliderPos(0.5)
		self.ctrlRotationY.SetSliderPos(0.5)
		self.ctrlRotationZ.SetSliderPos(0.5)

	def Build(self):
		if not self.__IsEnoughMoney():
			self.__PopupDialog(localeInfo.GUILD_NOT_ENOUGH_MONEY)
			return

		if not self.__IsEnoughMaterialStone():
			self.__PopupDialog(localeInfo.GUILD_NOT_ENOUGH_MATERIAL)
			return

		if not self.__IsEnoughMaterialLog():
			self.__PopupDialog(localeInfo.GUILD_NOT_ENOUGH_MATERIAL)
			return

		if not self.__IsEnoughMaterialPlywood():
			self.__PopupDialog(localeInfo.GUILD_NOT_ENOUGH_MATERIAL)
			return

		## /build c vnum x y x_rot y_rot z_rot
		## /build d vnum
		if "BUILDIN" == self.type:
			for i in xrange(len(self.raceList)):
				race = self.raceList[i]
				xPos, yPos = self.posList[i]
				rot = self.rotList[i]
				appInst.instance().GetNet().SendChatPacket(
					"/build c %d %d %d %d %d %d"
					% (
						race,
						int(self.x + xPos),
						int(self.y + yPos),
						self.rot_x,
						self.rot_y,
						rot,
					)
				)
		else:
			appInst.instance().GetNet().SendChatPacket(
				"/build c %d %d %d %d %d %d"
				% (
					self.race,
					int(self.x),
					int(self.y),
					self.rot_x,
					self.rot_y,
					self.rot_z,
				)
			)

		self.Close()

	def ChangeShow(self):
		x, y, z = player.GetMainCharacterPosition()
		app.SetCameraSetting(int(x), int(-y), int(z), 3000, 0, 30)
		self.x = x
		self.y = y - 500
		self.z = z
		self.categoryList.SelectItem(0)
		self.buildingList.SelectItem(0)
		self.SetTop()
		self.Show()

	def Open(self):
		self.__LoadWindow()
		x, y, z = player.GetMainCharacterPosition()
		app.SetCameraSetting(int(x), int(-y), int(z), 3000, 0, 30)

		background.VisibleGuildArea()

		self.x = x
		self.y = y
		self.z = z
		self.categoryList.SelectItem(0)
		self.buildingList.SelectItem(0)
		self.SetTop()
		self.Show()
		self.__DisablePCBlocker()

		if __debug__:
			self.categoryList.SelectItem(2)
			self.buildingList.SelectItem(0)

		player.SetParalysis(True)

	def Close(self):
		self.__DeleteInstance()

		background.DisableGuildArea()

		self.Hide()
		self.__OnClosePopupDialog()
		self.__EnablePCBlocker()
		self.__UnlockCameraMoving()
		if self.closeEvent:
			self.closeEvent()

		player.SetParalysis(False)

	def Destory(self):
		self.Close()

		self.ClearDictionary()
		self.board = None
		self.categoryList = None
		self.buildingList = None
		self.listScrollBar = None
		self.positionButton = None
		self.previewButton = None
		self.posValueX = None
		self.posValueY = None
		self.ctrlRotationX = None
		self.ctrlRotationY = None
		self.ctrlRotationZ = None
		self.buildingPriceValue = None
		self.buildingMaterialStoneValue = None
		self.buildingMaterialLogValue = None
		self.buildingMaterialPlywoodValue = None
		self.closeEvent = None

	def SetCloseEvent(self, event):
		self.closeEvent = event

	def __PopupDialog(self, text):
		popup = uiCommon.PopupDialog()
		popup.SetText(text)
		popup.SetAcceptEvent(self.__OnClosePopupDialog)
		popup.Open()
		self.popup = popup

	def __OnClosePopupDialog(self):
		self.popup = None

	def __EnablePCBlocker(self):
		## PC Blocker 처리를 켠다. (투명해짐)
		chr.SetInstanceType(chr.INSTANCE_TYPE_BUILDING)

		for idx in self.indexList:
			chr.SetBlendRenderMode(idx, 1.0)

	def __DisablePCBlocker(self):
		## PC Blocker 처리를 끈다. (안투명해짐)
		chr.SetInstanceType(chr.INSTANCE_TYPE_OBJECT)

		for idx in self.indexList:
			chr.SetBlendRenderMode(idx, self.BUILDING_ALPHA)

	def __OnSelectPositioningMode(self):
		if self.MODE_PREVIEW == self.mode:
			self.positionButton.SetUp()
			return

		self.mode = self.MODE_POSITIONING
		self.Hide()

	def __OnEnterPreviewMode(self):
		if self.MODE_POSITIONING == self.mode:
			self.previewButton.SetUp()
			return

		self.mode = self.MODE_PREVIEW
		self.positionButton.SetUp()
		self.__UnlockCameraMoving()
		self.__EnablePCBlocker()

	def __OnLeavePreviewMode(self):
		self.__RestoreViewMode()

	def __RestoreViewMode(self):
		self.__DisablePCBlocker()
		self.__LockCameraMoving()
		self.mode = self.MODE_VIEW
		self.positionButton.SetUp()
		self.previewButton.SetUp()

	def __IsEnoughMoney(self):
		curMoney = player.GetMoney()
		if curMoney < self.needMoney:
			return False
		return True

	def __IsEnoughMaterialStone(self):
		curStoneCount = player.GetItemCountByVnum(MATERIAL_STONE_ID)
		if curStoneCount < self.needStoneCount:
			return False
		return True

	def __IsEnoughMaterialLog(self):
		curLogCount = player.GetItemCountByVnum(MATERIAL_LOG_ID)
		if curLogCount < self.needLogCount:
			return False
		return True

	def __IsEnoughMaterialPlywood(self):
		curPlywoodCount = player.GetItemCountByVnum(MATERIAL_PLYWOOD_ID)
		if curPlywoodCount < self.needPlywoodCount:
			return False
		return True

	def __OnSelectCategory(self):
		self.listScrollBar.SetPos(0.0)
		self.__RefreshItem()

	def __OnSelectBuilding(self):
		buildingIndex = self.buildingList.GetSelectedItem()

		proto = guildMgr().GetObjectProto()

		if buildingIndex >= len(proto):
			return

		data = proto[buildingIndex]

		self.type = data.type
		if BUILDING_BUILD_IN == data.type:
			self.__CreateBuildInInstance(data.vnum)
		else:
			self.__CreateInstance(data.vnum)

		self.buildingPriceValue.SetText(NumberToMoneyString(data.price))
		self.needMoney = data.price

		for i in xrange(OBJECT_MATERIAL_MAX_NUM):
			if data.materials[i].itemVnum == MATERIAL_STONE_ID:
				self.needStoneCount = data.materials[i].count
			elif data.materials[i].itemVnum == MATERIAL_LOG_ID:
				self.needLogCount = data.materials[i].count
			elif data.materials[i].itemVnum == MATERIAL_PLYWOOD_ID:
				self.needPlywoodCount = data.materials[i].count

		self.buildingMaterialStoneValue.SetText(
			str(self.needStoneCount) + localeInfo.THING_COUNT
		)
		self.buildingMaterialLogValue.SetText(
			str(self.needLogCount) + localeInfo.THING_COUNT
		)
		self.buildingMaterialPlywoodValue.SetText(
			str(self.needPlywoodCount) + localeInfo.THING_COUNT
		)

		if self.__IsEnoughMoney():
			self.buildingPriceValue.SetPackedFontColor(self.ENABLE_COLOR)
		else:
			self.buildingPriceValue.SetPackedFontColor(self.DISABLE_COLOR)

		if self.__IsEnoughMaterialStone():
			self.buildingMaterialStoneValue.SetPackedFontColor(self.ENABLE_COLOR)
		else:
			self.buildingMaterialStoneValue.SetPackedFontColor(self.DISABLE_COLOR)

		if self.__IsEnoughMaterialLog():
			self.buildingMaterialLogValue.SetPackedFontColor(self.ENABLE_COLOR)
		else:
			self.buildingMaterialLogValue.SetPackedFontColor(self.DISABLE_COLOR)

		if self.__IsEnoughMaterialPlywood():
			self.buildingMaterialPlywoodValue.SetPackedFontColor(self.ENABLE_COLOR)
		else:
			self.buildingMaterialPlywoodValue.SetPackedFontColor(self.DISABLE_COLOR)

		self.rot_x_limit = data.rotationLimits[0]
		self.rot_y_limit = data.rotationLimits[1]
		self.rot_z_limit = data.rotationLimits[2]

		self.ctrlRotationX.Enable()
		self.ctrlRotationY.Enable()
		self.ctrlRotationZ.Enable()

		if 0 == self.rot_x_limit:
			self.ctrlRotationX.Disable()
		if 0 == self.rot_y_limit:
			self.ctrlRotationY.Disable()
		if 0 == self.rot_z_limit:
			self.ctrlRotationZ.Disable()

	def __OnScrollBuildingList(self):
		viewItemCount = self.buildingList.GetViewItemCount()
		itemCount = self.buildingList.GetItemCount()
		pos = self.listScrollBar.GetPos() * (itemCount - viewItemCount)
		self.buildingList.SetBasePos(int(pos))

	def __OnChangeRotation(self):
		if self.rot_x_limit != 0.0:
			self.rot_x = (
				self.ctrlRotationX.GetSliderPos() * self.rot_x_limit
				+ self.rot_x_limit / 2
			) % self.rot_x_limit
		if self.rot_y_limit != 0.0:
			self.rot_y = (
				self.ctrlRotationY.GetSliderPos() * self.rot_y_limit
				+ self.rot_y_limit / 2
			) % self.rot_y_limit
		if self.rot_z_limit != 0.0:
			self.rot_z = (
				self.ctrlRotationZ.GetSliderPos() * self.rot_z_limit
				+ self.rot_z_limit / 2
			) % self.rot_z_limit
		chr.SetRotationAll(self.rot_x, self.rot_y, self.rot_z)

	def __LockCameraMoving(self):
		app.SetCameraSetting(int(self.x), int(-self.y), int(self.z), 3000, 0, 30)

	def __UnlockCameraMoving(self):
		app.SetDefaultCamera()

	def __RefreshItem(self):
		self.buildingList.ClearItem()

		categoryIndex = self.categoryList.GetSelectedItem()
		if categoryIndex >= len(self.GUILD_CATEGORY_LIST):
			return
		selectedType = self.GUILD_CATEGORY_LIST[categoryIndex][0]

		proto = guildMgr().GetObjectProto()
		index = 0
		for guildObject in proto:
			if selectedType != guildObject.type:
				index += 1
				continue

			if guildObject.allowPlacement:
				name = guildMgr().GetObjectName(guildObject.vnum)
				self.buildingList.InsertItem(index, name)

			index += 1

		self.buildingList.SelectItem(0)

		if self.buildingList.GetItemCount() < self.buildingList.GetViewItemCount():
			self.buildingList.SetSize(
				self.BUILDINGLIST_BASE_WIDTH + 15, self.buildingList.GetHeight()
			)
			self.buildingList.LocateItem()
			self.listScrollBar.Hide()
		else:
			self.buildingList.SetSize(
				self.BUILDINGLIST_BASE_WIDTH, self.buildingList.GetHeight()
			)
			self.buildingList.LocateItem()
			self.listScrollBar.Show()

	def SettleCurrentPosition(self):
		guildID = miniMap.GetGuildAreaID(self.x, self.y)
		if not __debug__ and guildID != player.GetGuildID():
			return

		self.__RestoreViewMode()
		self.__LockCameraMoving()
		self.Show()

	def SetBuildingPosition(self, x, y, z):
		self.x = x
		self.y = y
		self.posValueX.SetText(str(int(x)))
		self.posValueY.SetText(str(int(y)))

		for i in xrange(len(self.indexList)):
			idx = self.indexList[i]
			xPos, yPos = self.posList[i]

			chr.SelectInstance(idx)
			if 0 != z:
				self.z = z
				chr.SetPixelPosition(int(x + xPos), int(y + yPos), int(z))
			else:
				chr.SetPixelPosition(int(x + xPos), int(y + yPos))

	def IsPositioningMode(self):
		if self.MODE_POSITIONING == self.mode:
			return True
		return False

	def IsPreviewMode(self):
		if self.MODE_PREVIEW == self.mode:
			return True
		return False

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True
