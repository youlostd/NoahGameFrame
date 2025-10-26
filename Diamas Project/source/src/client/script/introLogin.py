# -*- coding: utf-8 -*-

import logging
import os
import re
import time
import webbrowser

import app
import background
import snd
import wndMgr
from pygame.app import appInst, GetLanguageNameByID, GetLanguageIDByName,GetLangDisplayName
from pygame.auth import authSocket
from pygame.net import SetServerInfo

import localeInfo
import musicInfo
import serverInfo
import ui
import uiScriptLocale
from ui_event import MakeEvent, Event

SIB = "d:/ymir work/ui/gui/login/"
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
		raise TypeError('Singletons must be accessed through `instance()`.')

	def __instancecheck__(self, inst):
		return isinstance(inst, self._decorated)


def clamp(n, smallest, largest):
	return max(smallest, min(n, largest))


import re
def has_turkish_characters(text):
	"""
	This function checks if a string contains any Turkish characters.

	Args:
		text: The string to be checked.

	Returns:
		True if the text contains Turkish characters, False otherwise.
	"""
	turkish_chars_regex = r"[ığüşöçİĞÜŞÖÇ]"
	return bool(re.search(turkish_chars_regex, text))

def TRANSFORM_LANG(lang):
	langDict = {
		1: "ro",
		2: "en",
		3: "de",
		4: "tr",
		5: "pl",
		6: "pt",
	}

	for key, value in langDict.iteritems():
		if key == lang:
			return str(value)
		elif value == lang:
			return int(key)

	return "en"

REGISTER_INDEX_USERNAME = 0
REGISTER_INDEX_PASSWORD = 1
REGISTER_INDEX_DELETE_CODE = 2
REGISTER_INDEX_MAIL = 3
REGISTER_INDEX_BOT_CONTROL = 4
REGISTER_INDEX_MAX = 5

class RegisterDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")		
		ui.PythonScriptLoader().LoadScriptFile(self, "UIScript/RegisterDialog.py")

		self.boardWithTitleBar = self.GetChild("Board")
		self.boardWithTitleBar.SetCloseEvent(self.Close)

		self.inputValue = (
			self.GetChild("UserNameValue"),
			self.GetChild("PasswordValue"),
			self.GetChild("DeleteCodeValue"),
			self.GetChild("MailValue"),
			self.GetChild("BotControlValue"),
		)

		self.registerButton = self.GetChild("AcceptButton")

		self.inputValue[REGISTER_INDEX_BOT_CONTROL].SetNumberMode()
		self.inputValue[REGISTER_INDEX_DELETE_CODE].SetNumberMode()

		self.selectedInputIndex = REGISTER_INDEX_USERNAME

		self.BotControlQuestion = self.GetChild("BotControlQuestion")
		self.BotControlSumNumbers = [1,1]

		self.special_characters = "'.'{[]}^&()*|é\!@#$%^&*()-+?_=,<>/'€£¥•¶§•ªº«»~`:;´¨_ğüşöçİĞÜŞÖÇıæßðþÞÐŒœŠšŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ'"
		self.special_characters_mail = "''{[]}^&()*|é\!#$%^&*()+?=,<>/'€£¥•¶§•ªº«»~`:;´¨ğüşöçİĞÜŞÖÇıæßðþÞÐŒœŠšŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ'"

	def ClearInputValues(self):
		for i in xrange(len(self.inputValue)):
			self.inputValue[i].SetText("")

	def Open(self):
		self.Lock()
		self.SetTop()
		self.SetCenterPosition()
		self.inputValue[REGISTER_INDEX_USERNAME].SetFocus()
		self.RefreshBotControl()
		self.Show()

	def OnTab(self):
		self.inputValue[self.selectedInputIndex].KillFocus()
		self.selectedInputIndex = REGISTER_INDEX_USERNAME if self.selectedInputIndex == REGISTER_INDEX_BOT_CONTROL else self.selectedInputIndex + 1
		self.inputValue[self.selectedInputIndex].SetFocus()
		return True

	def Close(self):
		self.Unlock()
		self.Hide()
		return False
	
	def RefreshBotControl(self):
		self.BotControlSumNumbers[0] = app.GetRandom(1,20)
		self.BotControlSumNumbers[1] = app.GetRandom(1,20)
		self.BotControlQuestion.SetText("{} + {}".format(self.BotControlSumNumbers[0], self.BotControlSumNumbers[1]))
		self.inputValue[REGISTER_INDEX_BOT_CONTROL].SetText("")

	def CheckBotControl(self):
		resultText = self.inputValue[REGISTER_INDEX_BOT_CONTROL].GetText()
		if resultText and resultText.isdigit():
			result = int(resultText)
			if self.BotControlSumNumbers[0] + self.BotControlSumNumbers[1] == result:
				return True

		return False
	
	def CheckSpecialCharacter(self, text, specialCharacter):
		if any(chs in specialCharacter for chs in text):
			return False

		if has_turkish_characters(text):
			return False

		return True
	
	def CheckInputValue(self):
		if len(self.inputValue[REGISTER_INDEX_USERNAME].GetText()) < 5:
			return False, localeInfo.REGISTER_CHECK_CHAT_1
		
		if len(self.inputValue[REGISTER_INDEX_PASSWORD].GetText()) < 6:
			return False, localeInfo.REGISTER_CHECK_CHAT_2
		
		if not len(self.inputValue[REGISTER_INDEX_DELETE_CODE].GetText()) == 7:
			return False, localeInfo.REGISTER_CHECK_CHAT_3
		
		if len(self.inputValue[REGISTER_INDEX_MAIL].GetText()) < 10:
			return False, localeInfo.REGISTER_CHECK_CHAT_4

		if not self.CheckSpecialCharacter(self.inputValue[REGISTER_INDEX_MAIL].GetText(), self.special_characters_mail):
			return False, localeInfo.REGISTER_CHECK_CHAT_4

		if not self.CheckSpecialCharacter(self.inputValue[REGISTER_INDEX_USERNAME].GetText(), self.special_characters):
			return False, localeInfo.REGISTER_CHECK_CHAT_5
		
		if not self.CheckSpecialCharacter(self.inputValue[REGISTER_INDEX_PASSWORD].GetText(),self.special_characters):
			return False, localeInfo.REGISTER_CHECK_CHAT_6

		if not self.CheckBotControl():
			self.RefreshBotControl()
			return False, localeInfo.REGISTER_CHECK_CHAT_7

		return True, "Success"

	def GetRegisterData(self):
		return 	self.inputValue[REGISTER_INDEX_USERNAME].GetText(),\
				self.inputValue[REGISTER_INDEX_PASSWORD].GetText(),\
				self.inputValue[REGISTER_INDEX_DELETE_CODE].GetText(),\
				self.inputValue[REGISTER_INDEX_MAIL].GetText()

class PinInputDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")

		ui.PythonScriptLoader().LoadScriptFile(self, "UIScript/PinInputDialog.py")

		self.inputValue = self.GetChild("InputValue")
		self.boardWithTitleBar = self.GetChild("Board")
		self.acceptButton = self.GetChild("AcceptButton")
		self.cancelButton = self.GetChild("CancelButton")
		self.descriptionText = self.GetChild("Description")

	def SetEvents(self, accept, cancel):
		self.boardWithTitleBar.SetCloseEvent(cancel)
		self.acceptButton.SetEvent(accept)
		self.cancelButton.SetEvent(cancel)
		self.inputValue.SetReturnEvent(accept)
		self.inputValue.SetEscapeEvent(cancel)

	def Open(self):
		self.Lock()
		self.SetTop()
		self.SetCenterPosition()
		self.inputValue.SetFocus()
		self.inputValue.SetNumberMode()
		self.inputValue.SetSecret()
		self.Show()

	def Close(self):

		self.Unlock()
		self.Hide()

	def SetDescription(self, text):
		self.descriptionText.SetText(text)

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.cancelButton.CallEvent()
			return True

		if key == app.VK_RETURN:
			self.acceptButton.CallEvent()
			return True

		return False


class ConnectingDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")

		self.__LoadDialog()
		self.eventTimeOver = None
		self.eventExit = None
		self.endTime = 0

	def __LoadDialog(self):
		try:
			PythonScriptLoader = ui.PythonScriptLoader()
			PythonScriptLoader.LoadScriptFile(self, "UIScript/ConnectingDialog.py")

			self.board = self.GetChild("board")
			self.message = self.GetChild("message")
			self.countdownMessage = self.GetChild("countdown_message")
		except:
			logging.exception("ConnectingDialog.LoadDialog.BindObject")

	def Open(self, waitTime):
		curTime = time.clock()
		self.endTime = curTime + waitTime

		self.Lock()
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

	def Close(self):
		self.Unlock()
		self.Hide()

	def Destroy(self):
		self.Hide()
		self.ClearDictionary()

	def SetText(self, text):
		self.message.SetText(text)

	def SetCountDownMessage(self, waitTime):
		self.countdownMessage.SetText("%.0f%s" % (waitTime, localeInfo.SECOND))

	def SetTimeOverEvent(self, event):
		self.eventTimeOver = MakeEvent(event)

	def SetExitEvent(self, event):
		self.eventExit = MakeEvent(event)

	def OnUpdate(self):
		# text = self.idEditLine.GetText()
		# self.idEditLine.SetText("TEST") #str(text.upper()))
		lastTime = max(0, self.endTime - time.clock())
		if 0 == lastTime:
			self.Close()
			self.eventTimeOver()
		else:
			self.SetCountDownMessage(self.endTime - time.clock())

	def OnPressExitKey(self):
		self.eventExit()
		return True


class LoginWindow(ui.ScriptWindow):
	dropped = 0

	def __init__(self, stream):
		ui.ScriptWindow.__init__(self)
		self.SetWindowName(self.__class__.__name__)

		self.stream = stream
		self.authSocket = authSocket()
		self.authSocket.SetHandler(self)

		appInst.instance().GetNet().SetPhaseWindow(PHASE_WINDOW_LOGIN, self)

		self.connectingDialog = None

		self.pinInputDialog = PinInputDialog()
		self.pinInputDialog.SetEvents(self.__OnPinInputAccept, self.__OnPinInputCancel)

		self.registerDialog = RegisterDialog()
		self.registerDialog.registerButton.SetEvent(self.RegisterAccept)

		self.serverInfo = serverInfo.Get()
		self.serverName, self.info = next(self.serverInfo.iteritems())

		self.updateNumber = 0
		self.pickedChannel = 1
		self.channelButtons = []
		self.serverButtons = []
		self.loginButton = None
		self.registerButton = None
		self.idEditLine = None
		self.pwdEditLine = None
		self.hGuardCode = None
		self.sendHGuardCodeButton = None
		self.hGuardBoard = None
		self.cancelHGuardCodeButton = None
		self.selectedAccountIndex = -1
		self.extraButtons = []
		self.extraButtonsUrls = []
		self.background = None
		self.loginFailureFuncDict = {
			"WRONGPWD": self.__DisconnectAndInputPassword,
			"PIN": self.__ClearSavedPin,
			"PINBLK": self.__ClearSavedPin,
			"QUIT": appInst.instance().Exit,
		}
		self.loginBoard = None

		self.loginFailureMsgDict = {
			# "DEFAULT" : localeInfo.LOGIN_FAILURE_UNKNOWN,

			"ALREADY": localeInfo.LOGIN_FAILURE_ALREAY,
			"NOID": localeInfo.LOGIN_FAILURE_NOT_EXIST_ID,
			"WRONGPWD": localeInfo.LOGIN_FAILURE_WRONG_PASSWORD,
			"FULL": localeInfo.LOGIN_FAILURE_TOO_MANY_USER,
			"SHUTDOWN": localeInfo.LOGIN_FAILURE_SHUTDOWN,
			"REPAIR": localeInfo.LOGIN_FAILURE_REPAIR_ID,
			"BLOCK": localeInfo.LOGIN_FAILURE_BLOCK_ID,
			"BESAMEKEY": localeInfo.LOGIN_FAILURE_BE_SAME_KEY,
			"NOTAVAIL": localeInfo.LOGIN_FAILURE_NOT_AVAIL,
			"NOBILL": localeInfo.LOGIN_FAILURE_NOBILL,
			"BLKLOGIN": localeInfo.LOGIN_FAILURE_BLOCK_LOGIN,
			"WEBBLK": localeInfo.LOGIN_FAILURE_WEB_BLOCK,
			"VERSION": localeInfo.LOGIN_FAILURE_VERSION,
			"PIN": localeInfo.LOGIN_FAILURE_PIN,
			"PINBLK": localeInfo.LOGIN_FAILURE_PINBLK,
			"HACK": "Your account has been suspended because you hacked",
			"M2BOB": "Botting detected/Permanently Banned",
			"m2bob": "Botting detected/Permanently Banned",
			"SCAM": "Brutforce/Scaming",
			"DON": "Donation Error",
			"TS3": "Please come to our teamspeak server",
			"EPVP": "Versuch auf EPVP zu handeln",
			"PWCHANGE": "Please change your password",
			"MAINT": "Maintenance/Wartung",
			"WHWID": "PC is not enabled for this account",
			"HWIDB": "Kalıcı olarak yasaklandın!",
			"TRADEB": "Trading Detected/Permanently Banned.",
			"PINSET": localeInfo.LOGIN_PIN_SET_SUCCESSFULLY,
			"EINVAL": localeInfo.LOGIN_FAILURE_EINVAL,
			"REG0": localeInfo.REGISTER_STATUS_0,
			"REG1": localeInfo.REGISTER_STATUS_1,
			"REG2": localeInfo.REGISTER_STATUS_2,

		}

		self.ChannelStatusImage = [None, None, None, None]
		self.ChannelStatusTextImage = [None, None, None, None]
		self.Languages = [None, None, None, None, None, None, None, None, None, None]
		self.AccountManager = [
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
			[None, None, None, None],
		]
		self.AccountManagerData = [
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
			["", "", 0],
		]

		self.LanguageButtonGroup = None


	def Open(self):
		self.isRegisterConnect = False
		self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		self.SetWindowName("LoginWindow")

		self.__LoadScript("UIScript/LoginWindow.py")

		if os.path.isfile("account_data.cfg"):
			self.LoadAccountData()

		if musicInfo.loginMusic:
			snd.SetMusicVolume(appInst.instance().GetSettings().GetMusicVolume())
			snd.FadeInMusic("BGM/" + musicInfo.loginMusic)

		snd.SetSoundVolume(appInst.instance().GetSettings().GetSoundVolume())

		self.loginButton.Show()
		self.registerButton.Show()

		self.Show()

		self.__OpenLoginBoard()
		self.__OnClickChannel(1)

		app.ShowCursor()

	def Close(self):

		if self.connectingDialog:
			self.connectingDialog.Close()

		self.connectingDialog = None

		if musicInfo.loginMusic != "" and musicInfo.selectMusic != "":
			snd.FadeOutMusic("BGM/" + musicInfo.loginMusic)

		self.idEditLine.SetTabEvent(0)
		self.idEditLine.SetReturnEvent(0)
		self.pwdEditLine.SetReturnEvent(0)
		self.pwdEditLine.SetTabEvent(0)

		self.loginBoard = None
		self.isRegisterConnect = False
		self.idEditLine = None
		self.pwdEditLine = None
		self.connectingDialog = None

		self.hGuardCode = None
		self.sendHGuardCodeButton = None
		self.hGuardBoard = None
		self.cancelHGuardCodeButton = None

		self.background = None
		self.channelButtons = []
		self.KillFocus()
		self.Hide()

		self.stream.popupWindow.Close()

		self.loginButton = None
		self.registerButton = None

		app.HideCursor()
		appInst.instance().GetNet().ClearPhaseWindow(PHASE_WINDOW_LOGIN, self)
		self.authSocket.SetHandler(None)
		self.authSocket = None

	@staticmethod
	def __ExitGame():
		appInst.instance().Exit()

	def SetIDEditLineFocus(self):
		if self.idEditLine is not None:
			self.idEditLine.SetFocus()

	def SetPasswordEditLineFocus(self):
		if self.pwdEditLine is not None:
			self.pwdEditLine.SetFocus()

		if self.idEditLine is not None:
			self.idEditLine.KillFocus()

	def FocusInDetails(self):
		if self.idEditLine is not None and self.idEditLine.GetText() != "":
			self.idEditLine.SetFocus()
		elif self.pwdEditLine is not None:
			self.pwdEditLine.SetFocus()

	def SetNowCountDown(self, value):
		self.isNowCountDown = value

	def OnEndCountDown(self):
		self.isNowCountDown = False
		self.OnConnectFailure()

	def OnConnectFailure(self):

		snd.PlaySound("sound/ui/loginfail.wav")

		if self.connectingDialog:
			self.connectingDialog.Close()
		self.connectingDialog = None

		self.PopupNotifyMessage(localeInfo.LOGIN_CONNECT_FAILURE, self.FocusInDetails)

	def OnConnectSuccess(self):
		snd.PlaySound("sound/ui/loginok.wav")
		if self.isRegisterConnect:
			userName, password, deleteCode, mailAdress = self.registerDialog.GetRegisterData()
			self.PopupNotifyMessage(localeInfo.REGISTER_CHECK_CHAT_8)
			self.registerDialog.Close()
			self.registerDialog.ClearInputValues()
			self.authSocket.SendRegisterPacket(userName, password, deleteCode, mailAdress)					
		else:
			self.authSocket.SendAuthPacket()

	def OnHandShake(self):
		snd.PlaySound("sound/ui/loginok.wav")
		self.PopupDisplayMessage(localeInfo.LOGIN_CONNECT_SUCCESS)

	def OnLoginStart(self):
		self.PopupDisplayMessage(localeInfo.LOGIN_PROCESSING)

	def OnLoginFailure(self, error):
		if self.connectingDialog:
			self.connectingDialog.Close()
		self.connectingDialog = None

		if error.startswith("THROTTLE"):
			(statusCode, throttleTime) = error.split("_")
			self.PopupNotifyMessage(localeInfo.LOGIN_THROTTLE_TEXT.format(localeInfo.SecondToDHMS(int(throttleTime))),
									self.loginFailureFuncDict.get(statusCode, self.FocusInDetails))
			return

		try:
			loginFailureMsg = self.loginFailureMsgDict[error]
		except KeyError:
			loginFailureMsg = localeInfo.LOGIN_FAILURE_UNKNOWN + error

		self.PopupNotifyMessage(loginFailureMsg, self.loginFailureFuncDict.get(error, self.FocusInDetails))

		snd.PlaySound("sound/ui/loginfail.wav")

	def DropConnection(self):
		if not self.authSocket.IsClosed():
			self.authSocket.Disconnect()

	# self.PopupNotifyMessage(localeInfo.LOGIN_CONNECT_CANCEL)

	def __DisconnectAndInputPassword(self):
		if self.connectingDialog:
			self.connectingDialog.Close()
		self.connectingDialog = None

		self.SetPasswordEditLineFocus()
		if not self.authSocket.IsClosed():
			self.authSocket.Disconnect()

	def __ClearSavedPin(self):
		self.AccountManagerData[self.selectedAccountIndex][2] = 0
		self.SaveAccountData()
		self.selectedAccountIndex = -1


	def OnHGuardPrompt(self):
		self.loginBoard.Hide()
		self.stream.popupWindow.Close()
		self.hGuardBoard.Show()

	def OnHGuardFailure(self):
		self.PopupNotifyMessage(localeInfo.HGUARD_CODE_ERROR)

	def LangToIndex(self, lang):
		langCodeReplace = {
			"de": 0,
			"en": 1,
			"pl": 2,
			"ro": 3,
			"tr": 4,
			"pt": 5,
			"it": 6,
			"hu": 7,
		}

		return langCodeReplace[lang]

	def __LoadScript(self, filename):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, filename)

		try:
			GetObject = self.GetChild
			self.loginBoard = GetObject("LoginBoard")
			self.passwordSlot = GetObject("Password_Slot")
			self.idSlot = GetObject("ID_Slot")
			self.idEditLine = GetObject("ID_EditLine")
			self.pwdEditLine = GetObject("Password_EditLine")
			self.DropDownBoard = self.GetChild("dropdownboard")

			# HGuard
			self.hGuardCode = GetObject("HGuardCode")
			self.sendHGuardCodeButton = GetObject("SendHGuardCode")
			self.hGuardBoard = GetObject("HGuardBoard")
			self.cancelHGuardCodeButton = GetObject("CancelHGuardCode")

			self.background = GetObject("Background")
			self.loginButton = GetObject('LoginButton')
			self.registerButton = GetObject('RegisterButton')

			self.extraButtons = [
				GetObject("btn1"),
				GetObject("btn2"),
				GetObject("btn3"),
				GetObject("btn4"),
				GetObject("btn5"),
				GetObject("btn6"),
			]

			self.extraButtonUrls = [
				"#"
				"#",
				"#",
				"#",
				"#",
				"#",
			]

			for i, btn in enumerate(self.extraButtons):
				btn.SetEvent(Event(self.ClickExtraButton, i))

			for i in xrange(10):
				self.AccountManager[i][0] = GetObject("account_" + str(i))
				self.AccountManager[i][1] = GetObject("account_save_" + str(i))
				self.AccountManager[i][2] = GetObject("account_delete_" + str(i))
				self.AccountManager[i][3] = GetObject("account_prefix_" + str(i))
				# self.AccountManager[i][4] = GetObject("account_input_" + str(i))

			self.channelButtons = []
			self.channelButtons.append(GetObject("channel1Button"))
			self.channelButtons.append(GetObject("channel2Button"))
			self.channelButtons.append(GetObject("channel3Button"))
			self.channelButtons.append(GetObject("channel4Button"))

			self.exitButton = GetObject("exit_button")

			self.channelSlots = []

			for i in xrange(4):
				self.channelSlots.append(GetObject("channelSlot" + str(i)))

			(x, y) = self.idEditLine.GetGlobalPosition()

			if len(self.serverInfo) > 1:
				for i, name in enumerate(self.serverInfo):
					if self.serverInfo[name]["active"]:
						self.serverButtons.append(
							ui.MakeButtonWithText(GetObject("Login"), name, 35, (25 * (i + 10)), "d:/ymir work/ui/game/myshop_deco/",
												"select_btn_01.sub", "select_btn_02.sub", "select_btn_03.sub"))
		except Exception as e:
			logging.exception(e)

		for idx, slot in enumerate(self.AccountManager):
			slot[1].SetEvent(Event(self.__OnClickAccountAdd, idx))
			slot[2].SetEvent(Event(self.__OnClickAccountErase, idx))
			slot[2].Hide()
			slot[1].SetPosition(-12, 10)
			slot[1].Show()
			slot[0].SetText("")
			slot[3].SetText(uiScriptLocale.SAVE_EMPTY)

		for i, btn in enumerate(self.channelButtons):
			btn.SetEvent(Event(self.__OnClickChannel, i + 1))

		for i, btn in enumerate(self.serverButtons):
			btn.SetEvent(Event(self.__OnClickServer, btn))

		self.exitButton.SetEvent(self.__ExitGame)
		self.DropDownLang()
		self.hGuardBoard.Hide()
		self.cancelHGuardCodeButton.SetEvent(self.__CancelHGuardCode)
		self.sendHGuardCodeButton.SetEvent(self.__SendHGuardCode)

		self.loginButton.SetEvent(Event(self.__OnClickLoginButton, -1))
		self.registerButton.SetEvent(Event(self.__OnClickRegisterButton))

		self.idEditLine.SetReturnEvent(self.pwdEditLine.SetFocus)
		self.idEditLine.SetTabEvent(self.pwdEditLine.SetFocus)

		self.pwdEditLine.SetReturnEvent(Event(self.__OnClickLoginButton, -1))
		self.pwdEditLine.SetTabEvent(self.idEditLine.SetFocus)

		self.idEditLine.SetText("")
		self.pwdEditLine.SetText("")

		if len(self.serverButtons) > 0:
			self.__OnClickServer(self.serverButtons[0])

		return 1

	def ClickExtraButton(self, index):
		webbrowser.open_new_tab(self.extraButtonUrls[index])

	def DropDownLang(self):

		self.langButton = {}
		self.landFlagButton = {}
		self.landTextTitleButton = {}

		i = 0
		langars = [

			[[25 - 12, GetLangDisplayName(1), GetLanguageNameByID(1)]],
			[[52 - 12, GetLangDisplayName(2), GetLanguageNameByID(2)]],
			[[79 - 12, GetLangDisplayName(3), GetLanguageNameByID(3)]],
			[[106 - 12, GetLangDisplayName(4), GetLanguageNameByID(4)]],
			[[133 - 12, GetLangDisplayName(5), GetLanguageNameByID(5)]],
			[[160 - 12, GetLangDisplayName(6), GetLanguageNameByID(6)]],
			[[187 - 12, GetLangDisplayName(7), GetLanguageNameByID(7)]],
			[[214 - 12, GetLangDisplayName(8), GetLanguageNameByID(8)]]]

		self.titleTextLang = ui.TextLine()
		self.titleTextLang.SetParent(self.DropDownBoard)
		self.titleTextLang.SetPosition(10, 245)
		# self.titleTextLang.SetText("Select Language")
		self.titleTextLang.SetPackedFontColor(0xffdbb667)
		self.titleTextLang.Show()

		dropBoardX, dropBoardY = self.DropDownBoard.GetGlobalPosition()
		self.dropDownBg = ui.ImageBox("TOP_MOST")
		self.dropDownBg.SetPosition(dropBoardX, dropBoardY+ self.DropDownBoard.GetHeight() - 29)
		self.dropDownBg.LoadImage(SIB + "bg-dropdown.png")
		self.dropDownBg.Hide()

		self.dropDownButton = ui.Button()
		self.dropDownButton.SetParent(self.DropDownBoard)
		self.dropDownButton.SetPosition(0, 185 + 64)
		self.dropDownButton.SetUpVisual(SIB + "drop-down-botton-normal.sub")
		self.dropDownButton.SetOverVisual(SIB + "drop-down-botton-over.sub")
		self.dropDownButton.SetDownVisual(SIB + "drop-down-botton-normal.sub")
		self.dropDownButton.SetText(localeInfo.INTROLOGIN_TEXT1)  # TODO
		self.dropDownButton.SetPackedFontColor(0xffdbb667)
		self.dropDownButton.SetEvent(self.dropDownButtonFuntion)
		self.dropDownButton.Show()

		self.landFlag = ui.ImageBox()
		self.landFlag.SetParent(self.dropDownButton)
		self.landFlag.SetPosition(12, 4)

		self.textLangTitle = ui.TextLine()
		self.textLangTitle.SetParent(self.dropDownButton)
		self.textLangTitle.SetPosition(50, 9)
		# self.textLangTitle.SetText("English")
		self.textLangTitle.SetPackedFontColor(0xff5a5a5a)
		self.textLangTitle.Show()

		# langCode = appInst.instance().GetSettings().GetLanguageCode()
		# if langCode != 0:
		#    self.landFlag.LoadImage(SIB + "flag/flag_%s.tga" % LANG_CODE_NAME[langCode][0])
		#    if self.textLangTitle:
		#        self.textLangTitle.SetText("%s" % LANG_CODE_NAME[langCode][1])
		# else:
		#    self.landFlag.LoadImage(SIB + "flag/flag_%s.tga" % LANG_CODE_NAME[DEFAULT_LANG_CODE][0])
		#    if self.textLangTitle:
		#        self.textLangTitle.SetText("%s" % LANG_CODE_NAME[DEFAULT_LANG_CODE][1])
		#    self.OnClickLanguageButton(LANG_CODE_NAME[DEFAULT_LANG_CODE][0])
		# self.landFlag.Show()

		for a in langars:
			self.langButton[i] = ui.Button()
			self.langButton[i].SetParent(self.dropDownBg)
			self.langButton[i].SetPosition(0, a[0][0])
			self.langButton[i].SetUpVisual(SIB + "drop-down-botton-list-normal.sub")
			self.langButton[i].SetOverVisual(SIB + "drop-down-botton-list-over.sub")
			self.langButton[i].SetDownVisual(SIB + "drop-down-botton-list-normal.sub")
			# [proxy(self.Languages[0]), Event(self.OnClickLanguageButton, "de"), None],
			# self.langButton[0].SetEvent(self.OnClickLanguageButton["de"]),
			# self.langButton[i].SetEvent(Event(self.OnClickLanguageButton, "de"), None)
			# self.langButton[i](Event(self.OnClickLanguageButton), constInfo.TRANSFORM_LANG(i + 1))
			self.langButton[i].SetEvent(Event(self.OnClickLanguageButton, GetLanguageNameByID(i + 1)))

			self.langButton[i].Show()
			self.landFlagButton[i] = ui.ImageBox()
			self.landFlagButton[i].SetParent(self.langButton[i])
			self.landFlagButton[i].SetPosition(12, 4)
			self.landFlagButton[i].LoadImage(SIB + "flag_%s.sub" % a[0][2])
			self.landFlagButton[i].Show()
			self.landTextTitleButton[i] = ui.TextLine()
			self.landTextTitleButton[i].SetParent(self.langButton[i])
			self.landTextTitleButton[i].SetPosition(50, 8)
			self.landTextTitleButton[i].SetText(a[0][1])
			self.landTextTitleButton[i].SetPackedFontColor(0xff5a5a5a)
			self.landTextTitleButton[i].Show()

			i += 1

	def dropDownButtonFuntion(self):
		if self.dropped == 1:
			self.dropDownBg.Hide()
			self.dropped = 0
		else:
			self.dropDownBg.Show()
			self.dropDownBg.SetTop()
			self.dropped = 1

	def HideDropDown(self):
		self.dropDownBg.Hide()
		self.dropped = 0

	def OnIdInputClick(self):
		self.idEditLine.OnMouseLeftButtonDown()

	def OnKeyDown(self, key):

		if app.VK_ESCAPE == key:  # ESCLang4
			self.__ExitGame()
		elif key == app.VK_F1:
			self.__OnClickAccountAdd(0)
		elif key == app.VK_F2:
			self.__OnClickAccountAdd(1)
		elif key == app.VK_F3:
			self.__OnClickAccountAdd(2)
		elif key == app.VK_F4:
			self.__OnClickAccountAdd(3)
		elif key == app.VK_F5:
			self.__OnClickAccountAdd(4)
		elif key == app.VK_F6:
			self.__OnClickAccountAdd(5)
		elif key == app.VK_F7:
			self.__OnClickAccountAdd(6)
		elif key == app.VK_F8:
			self.__OnClickAccountAdd(7)
		elif key == app.VK_F9:
			self.__OnClickAccountAdd(8)
		elif key == app.VK_F10:
			self.__OnClickAccountAdd(9)
		elif key == app.VK_F12:
			self.__ExitGame()
		return True

	def SaveAccountData(self):
		with open('account_data.cfg', 'wb') as content_file:
			data = ""
			for idx, account in enumerate(self.AccountManagerData):
				if idx != 9:
					if account[2] != 0:
						data += account[0] + ":#:" + account[1] + ":#:" + str(account[2]) + ";"
					else:
						data += account[0] + ":#:" + account[1] + ";"
				else:
					if account[2] != 0:
						data += account[0] + ":#:" + account[1] + ":#:" + str(account[2])
					else:
						data += account[0] + ":#:" + account[1]

			encData = app.EncryptByComputer(data)
			content_file.write(encData)

	def LoadAccountData(self):
		with open('account_data.cfg', 'rb') as content_file:
			encContent = content_file.read()
			try:
				content = app.DecryptByComputer(encContent)
			except WindowsError:
				content = ""

			if ';' in content:
				accounts = content.split(';')
				for idx, account in enumerate(accounts):
					if ':#:' in account:
						data = account.split(':#:')
						self.AccountManagerData[idx][0] = data[0]
						self.AccountManagerData[idx][1] = data[1]
						if len(data) > 2:
							self.AccountManagerData[idx][2] = long(data[2])

			for idx, account in enumerate(self.AccountManagerData):
				if account[0] != "":
					self.AccountManager[idx][3].SetText("{0}. ".format(idx + 1))
					self.AccountManager[idx][0].SetText(account[0])
					self.AccountManager[idx][1].SetUpVisual("d:/ymir work/ui/gui/login/okay_normal.sub")
					self.AccountManager[idx][1].SetOverVisual("d:/ymir work/ui/gui/login/okay_hover.sub")
					self.AccountManager[idx][1].SetDownVisual("d:/ymir work/ui/gui/login/okay_down.sub")
					self.AccountManager[idx][2].SetUpVisual("d:/ymir work/ui/gui/login/nichtokay_normal.sub")
					self.AccountManager[idx][2].SetOverVisual("d:/ymir work/ui/gui/login/nichtokay_hover.sub")
					self.AccountManager[idx][2].SetDownVisual("d:/ymir work/ui/gui/login/nichtokay_down.sub")
					self.AccountManager[idx][1].SetPosition(33, 17)
					self.AccountManager[idx][2].SetPosition(3, 17)
					self.AccountManager[idx][1].Show()
					self.AccountManager[idx][2].Show()

	def __OnClickAccountErase(self, idx):
		self.AccountManagerData[idx][0] = ""
		self.AccountManagerData[idx][1] = ""

		self.AccountManager[idx][0].SetText("")
		self.AccountManager[idx][1].Show()
		self.AccountManager[idx][1].SetUpVisual("d:/ymir work/ui/gui/login/accountsave2.dds")
		self.AccountManager[idx][1].SetOverVisual("d:/ymir work/ui/gui/login/accountsave.dds")
		self.AccountManager[idx][1].SetDownVisual("d:/ymir work/ui/gui/login/accountsave.dds")
		self.AccountManager[idx][1].SetPosition(-12, 10)
		self.AccountManager[idx][3].SetText(uiScriptLocale.SAVE_EMPTY)
		self.AccountManager[idx][2].SetPosition(9999, 9999)

		self.SaveAccountData()

	def __OnClickAccountAdd(self, idx):

		if self.AccountManagerData[idx][0] == "":
			id = self.idEditLine.GetText()
			pwd = self.pwdEditLine.GetText()

			if id == "" or pwd == "":
				return

			self.AccountManagerData[idx][0] = id
			self.AccountManagerData[idx][1] = pwd
			self.SaveAccountData()
			self.AccountManager[idx][0].SetText(id)
			self.AccountManager[idx][3].SetText("{0}. ".format(idx + 1))
			self.AccountManager[idx][1].SetUpVisual("d:/ymir work/ui/gui/login/okay_normal.sub")
			self.AccountManager[idx][1].SetOverVisual("d:/ymir work/ui/gui/login/okay_hover.sub")
			self.AccountManager[idx][1].SetDownVisual("d:/ymir work/ui/gui/login/okay_down.sub")
			self.AccountManager[idx][2].SetUpVisual("d:/ymir work/ui/gui/login/nichtokay_normal.sub")
			self.AccountManager[idx][2].SetOverVisual("d:/ymir work/ui/gui/login/nichtokay_hover.sub")
			self.AccountManager[idx][2].SetDownVisual("d:/ymir work/ui/gui/login/nichtokay_down.sub")
			self.AccountManager[idx][1].Show()
			self.AccountManager[idx][2].Show()
			self.AccountManager[idx][1].SetPosition(33, 17)
			self.AccountManager[idx][2].SetPosition(3, 17)
		else:
			self.selectedAccountIndex = idx
			self.idEditLine.SetText(self.AccountManagerData[idx][0])
			self.pwdEditLine.SetText(self.AccountManagerData[idx][1])
			self.__OnClickLoginButton(self.GetChannelDownIndex())

	def __OnClickServer(self, btn):
		name = btn.GetText()
		self.info = self.serverInfo[name]
		self.serverName = name
		self.stream.SetServerName(name)

		for svBtn in self.serverButtons:
			svBtn.SetUp()
			svBtn.Enable()

		btn.Down()
		btn.Disable()

		channelCount = len(self.info["channel"])
		for i, slot in enumerate(self.channelSlots):
			if i + 1 <= channelCount:
				slot.Show()
			else:
				slot.Hide()

	def __OnClickChannel(self, idx):
		self.SetChannelDownIndex(idx)

		for i, btn in enumerate(self.channelButtons):
			btn.SetUpVisual("d:/ymir work/ui/login_interface/channel_checkbox.sub")
			btn.SetOverVisual("d:/ymir work/ui/login_interface/channel_checkbox_hover.sub")
			btn.SetDownVisual("d:/ymir work/ui/login_interface/channel_checkbox_hover.sub")

		activeBtn = self.channelButtons[idx - 1]
		if activeBtn is not None:
			activeBtn.SetUpVisual("d:/ymir work/ui/login_interface/channel_checkbox_selected.sub")
			activeBtn.SetOverVisual("d:/ymir work/ui/login_interface/channel_checkbox_selected.sub")
			activeBtn.SetDownVisual("d:/ymir work/ui/login_interface/channel_checkbox_selected.sub")

	def Connect(self, id, pwd):
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(localeInfo.LOGIN_CONNETING, self.SetPasswordEditLineFocus,
									localeInfo.UI_CANCEL)

		self.authSocket.SetLoginInfo(id, pwd)

		try:
			channelDict = self.info["channel"]
		except KeyError:
			return

		SetServerInfo("{}, {} ".format(self.serverName, self.pickedChannel))

		print("Picked Channel is {}".format(self.pickedChannel))
		addr, port = channelDict[self.pickedChannel - 1]
		account_addr, account_port = self.info["auth"]

		appInst.instance().GetNet().SetMarkServer(*channelDict[self.info["mark"]])
		app.SetGuildMarkPath(self.serverName)
		app.SetGuildSymbolPath(self.serverName + ".tga")
		self.isRegisterConnect = False
		self.authSocket.SetCharAddr(addr)
		self.authSocket.SetCharPort(str(port))
		self.authSocket.Connect(account_addr, str(account_port))
		self.HideDropDown()

	def ConnectToRegister(self):
		self.isRegisterConnect = True
		account_addr, account_port = self.info["auth"]
		self.authSocket.Connect(account_addr, str(account_port))


	def PopupDisplayMessage(self, msg):
		self.pinInputDialog.Close()
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg)

	def PopupNotifyMessage(self, msg, func=None):
		if not func:
			func = self.EmptyFunc

		self.pinInputDialog.Close()

		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg, func, localeInfo.UI_OK)

	def OnPressExitKey(self):
		self.stream.popupWindow.Close()
		self.stream.SetPhaseWindow(0)
		return True

	def OnExit(self):
		self.stream.popupWindow.Close()

	def OnUpdate(self):
		if self.updateNumber and self.updateNumber % 10 == 0:
			if self.hGuardCode:
				text = self.hGuardCode.GetText()
				self.hGuardCode.SetText(str(text.upper()))

		self.updateNumber += + 1

	def GetChannelDownIndex(self):
		return self.pickedChannel

	def SetChannelDownIndex(self, idx):
		self.pickedChannel = idx

	def EmptyFunc(self):
		pass

	def OnClickLanguageButton(self, lang):

		if app.GetLocaleName() != lang:
			app.ForceSetLocale(lang, "locale/" + lang)
			self.dropDownBg.Hide()
			self.dropped = 0
			with open("lang.cfg", "w+") as localeConfig:
				localeConfig.write(lang)

			self.PopupNotifyMessage(
				getattr(localeInfo, 'YOU_NEED_TO_RESTART_YOUR_CLIENT', 'You need to restart your client'), self.__ExitGame)



	#####################################################################################

	def __OpenLoginBoard(self):
		self.loginBoard.Show()
		self.idEditLine.SetTop()
		self.pwdEditLine.SetTop()
		self.idEditLine.SetFocus()

	def __SendHGuardCode(self):
		code = self.hGuardCode.GetText()
		valid = re.match('^[A-Z0-9]+$', self.hGuardCode.GetText()) is not None
		if not valid:
			self.PopupNotifyMessage(localeInfo.HGUARD_CODE_ONLY_ALPHANUM)
			return

		self.authSocket.SendHGuardCodePacket(code)

	def __CancelHGuardCode(self):
		self.hGuardBoard.Hide()
		self.loginBoard.Show()

		self.DropConnection()

	def __OnClickLoginButton(self, channel):
		self.HideDropDown()
		channel = clamp(self.GetChannelDownIndex() - 1, 0, 3)

		id = self.idEditLine.GetText()
		pwd = self.pwdEditLine.GetText()

		if not id:
			self.PopupNotifyMessage(localeInfo.LOGIN_INPUT_ID, self.SetIDEditLineFocus)
			return

		if not pwd:
			self.PopupNotifyMessage(localeInfo.LOGIN_INPUT_PASSWORD, self.SetPasswordEditLineFocus)
			return

		self.Connect(id, pwd)

	def __OnClickRegisterButton(self):
		self.registerDialog.Open()

	def SameLogin_OpenUI(self):
		self.PopupNotifyMessage(localeInfo.LOGIN_FAILURE_SAMELOGIN)

	def OnPinRequest(self):
		if self.selectedAccountIndex != -1 and self.AccountManagerData[self.selectedAccountIndex][2]:
			self.authSocket.SendPinResponsePacket(self.AccountManagerData[self.selectedAccountIndex][2])
			return

		self.stream.popupWindow.Hide()  # so we don't trigger the event
		self.stream.popupWindow.Unlock()
		self.stream.popupWindow.Close()

		self.pinInputDialog.SetDescription(uiScriptLocale.PIN_INPUT_DESCRIPTION)
		self.pinInputDialog.Open()

	def OnPinSetRequest(self):
		self.stream.popupWindow.Hide()  # so we don't trigger the event
		self.stream.popupWindow.Unlock()
		self.stream.popupWindow.Close()

		self.pinInputDialog.SetDescription(uiScriptLocale.PIN_SET_INPUT_DESCRIPTION)
		self.pinInputDialog.Open()

	def __OnPinInputCancel(self):
		if not self.authSocket.IsClosed():
			self.authSocket.Disconnect()

		self.pinInputDialog.Close()

	def __OnPinInputAccept(self):
		code = long(self.pinInputDialog.inputValue.GetText())

		if self.selectedAccountIndex != -1:
			self.AccountManagerData[self.selectedAccountIndex][2] = code
			self.SaveAccountData()

		self.authSocket.SendPinResponsePacket(code)

		self.pinInputDialog.Close()

	def OnPhaseChange(self, stream, phase):
		if phase == "login-same":
			self.SameLogin_OpenUI()

	def __StartGame(self):
		background.SetViewDistanceSet(background.DISTANCE0, 25600)
		background.SetViewDistanceSet(background.DISTANCE1, 19200)
		background.SetViewDistanceSet(background.DISTANCE2, 12800)
		background.SetViewDistanceSet(background.DISTANCE3, 9600)
		background.SetViewDistanceSet(background.DISTANCE4, 6400)
		background.SelectViewDistanceNum(background.DISTANCE0)

	def RegisterAccept(self):
		check, message = self.registerDialog.CheckInputValue()
		if check:
			self.ConnectToRegister()

		self.PopupNotifyMessage(message)


