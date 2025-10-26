# -*- coding: utf-8 -*-

import logging
from _weakref import proxy

import app
import chat
import grpText
import messenger
import player
import wndMgr
from pygame.app import appInst

import colorInfo
import emoji_config
import localeInfo
import ui
from ui_event import MakeEvent, Event


class WhisperSystem(object):
	def __init__(self, interface):
		self.interface = proxy(interface)
		self.dialogs = {}  # by name
		self.buttons = []
		self.windowOpenPosition = 0
		self.emptyDialog = None

	def Destroy(self):
		for i in self.dialogs.itervalues():
			i.Destroy()

		for i in self.buttons:
			i.Destroy()

		self.dialogs = {}
		self.buttons = []

	def HideAll(self):
		for i in self.dialogs.itervalues():
			i.Minimize()

		for i in self.buttons:
			i.Hide()

	def ShowAll(self):
		for i in self.buttons:
			i.Show()

	def ArrangeButtons(self):
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()

		xPos = screenWidth - 90
		yPos = 190 * screenHeight / 600
		yCount = (screenHeight - 330) / 63

		count = 0
		for button in self.buttons:
			button.SetPosition(xPos + (int(count / yCount) * -50), yPos + (count % yCount * 63))
			count += 1

	def MakeButton(self, name, openButton=False):
		whisperButton = ui.Button("TOP_MOST")
		if openButton:
			whisperButton.SetUpVisual("d:/ymir work/ui/game/windows/btn_mail_down.png")
		else:
			whisperButton.SetUpVisual("d:/ymir work/ui/game/windows/btn_mail_up.png")
		whisperButton.SetOverVisual("d:/ymir work/ui/game/windows/btn_mail_up.png")
		whisperButton.SetDownVisual("d:/ymir work/ui/game/windows/btn_mail_up.png")
		whisperButton.SetButtonScale(0.5, 0.5)

		btnText = ui.TextLine()
		btnText.SetWindowName("WhisperBtnText")
		btnText.SetParent(whisperButton)
		btnText.SetVerticalAlignCenter()
		btnText.SetHorizontalAlignCenter()
		btnText.SetOutline()

		imageWidth = whisperButton.GetButtonImageWidth() * 0.5
		imageHeight = whisperButton.GetButtonImageHeight() * 0.5

		btnText.SetPosition(0, 40)
		btnText.SetText(name)
		btnText.Show()

		whisperButton.ClearText()
		whisperButton.AppendTextWindow(btnText)
		whisperButton.SetEvent(Event(self.ShowDialog, name))
		whisperButton.Show()
		whisperButton.name = name
		whisperButton.SetSize(imageWidth, imageHeight)
		whisperButton.UpdateRect()

		self.buttons.insert(0, whisperButton)
		self.ArrangeButtons()
		return whisperButton

	def GetButton(self, name, openButton=False):
		for b in self.buttons:
			if b.name == name:
				return b
		return self.MakeButton(name, openButton)

	def AppendWhisper(self, name, whisperType, message):

		try:
			self.dialogs[name].AppendMessage(message, whisperType)
		except KeyError:
			self.dialogs[name] = self.MakeDialog(name)
			self.dialogs[name].AppendMessage(message, whisperType)

		lang = self.interface.GetWhisperLangByName(name)
		if lang != "":
			self.dialogs[name].SetPlayerWhisperLanguage(lang)
		if self.interface.IsGameMaster(name) or "[" in name:
			self.dialogs[name].SetGameMasterLook()

		if not self.dialogs[name].IsShow():
			button = self.GetButton(name, True)
			if button:
				button.Flash()
			app.FlashApplication()

	def SetUnOpenedLook(self, button):
		button.SetUpVisual("d:/ymir work/ui/game/windows/btn_mail_down.png")
		button.SetOverVisual("d:/ymir work/ui/game/windows/btn_mail_up.png")
		button.SetDownVisual("d:/ymir work/ui/game/windows/btn_mail_up.png")
		button.SetHorizontalAlignLeft()
		button.Flash()

	def ShowDialog(self, name=None):
		if not name:
			if not self.emptyDialog:
				self.emptyDialog = self.MakeDialog()
				self.emptyDialog.OpenWithoutTarget(self.OnAcceptEmptyDialogName)
				self.emptyDialog.Show()
			else:
				self.emptyDialog.Show()
				self.emptyDialog.SetTop()
				self.emptyDialog.titleNameEdit.SetFocus()
			return

		wb = None
		try:
			wb = next(i for i in self.buttons if i.texts[0].GetText() == name)
		except StopIteration:
			pass
		except AttributeError:
			logging.debug("Whisper window not initialized")

		if wb:
			wb.SetEvent(0)
			wb.Destroy()
			self.buttons.remove(wb)
			self.ArrangeButtons()

		try:
			self.dialogs[name].Show()
			self.dialogs[name].DelegateFocus()
		except KeyError:
			try:
				self.dialogs[name] = self.MakeDialog(name)
				self.dialogs[name].Show()
				self.dialogs[name].DelegateFocus()
			except Exception as e:
				logging.exception(e)

	def MakeDialog(self, name=None):
		dlgWhisper = WhisperDialog(proxy(self), self.OnMinimizeDialog,
								   self.OnCloseDialog)
		dlgWhisper.LoadDialog()
		dlgWhisper.SetPosition(self.windowOpenPosition * 30, self.windowOpenPosition * 30)
		self.windowOpenPosition = (self.windowOpenPosition + 1) % 5

		if name:
			dlgWhisper.OpenWithTarget(name)
			if self.interface.IsGameMaster(name) or "[" in name:
				dlgWhisper.SetGameMasterLook()
				dlgWhisper.AppendMessage(localeInfo.WHISPER_GM_WARNING, WHISPER_TYPE_NORMAL)
			else:
				dlgWhisper.AppendMessage(localeInfo.WHISPER_USER_WARNING, WHISPER_TYPE_NORMAL)

			lang = self.interface.GetWhisperLangByName(name)
			if lang != "":
				dlgWhisper.SetPlayerWhisperLanguage(lang)

			dlgWhisper.Hide()

		return dlgWhisper

	def OnAcceptEmptyDialogName(self, name):
		assert self.emptyDialog
		try:
			self.dialogs[name].Destroy()
			del self.dialogs[name]
		except KeyError:
			pass
		self.dialogs[name] = self.emptyDialog
		self.emptyDialog.OpenWithTarget(name)
		if self.interface and self.interface.IsGameMaster(name) or "[" in name:
			self.emptyDialog.SetGameMasterLook()
			self.emptyDialog.AppendMessage(localeInfo.WHISPER_GM_WARNING, WHISPER_TYPE_NORMAL)
		else:
			self.emptyDialog.AppendMessage(localeInfo.WHISPER_USER_WARNING, WHISPER_TYPE_NORMAL)

		if self.interface:
			lang = self.interface.GetWhisperLangByName(name)
			if lang != 0:
				self.emptyDialog.SetPlayerWhisperLanguage(lang)

		self.emptyDialog = None

	def OnMinimizeDialog(self, name):
		if name:
			self.GetButton(name)

	def OnCloseDialog(self, name):
		if name:
			self.dialogs[name].Destroy()
			del self.dialogs[name]
		else:
			self.emptyDialog.Hide()

	def OnPhaseChange(self, stream, phase):
		if phase == "login":
			# Not strictly necessary since we use weakrefs
			self.interface = None
		elif phase != "game":
			# If we return to the login/charselect scene,
			# erase everything
			stream.RemovePersistentObject("whisper")


class WhisperDialog(ui.ScriptWindow):
	class ResizeButton(ui.DragButton):
		def OnMouseOverIn(self):
			app.SetCursor(app.HVSIZE)
			return True

		def OnMouseOverOut(self):
			app.SetCursor(app.NORMAL)
			return True

	class Text(ui.TextLine):
		def __init__(self):
			ui.TextLine.__init__(self)

			self.scrollPosition = 0.0
			self.height = 0
			self.width = 0
			self.x = 0
			self.y = 0

			self.RemoveFlag("not_pick")
			self.SetMultiLine()

		def SetScrollPosition(self, s):
			self.scrollPosition = s
			self.FixPosition()

		def SetSize(self, width, height):
			self.SetLimitWidth(width)
			self.height = height
			self.width = width
			self.FixPosition()

		def SetPosition(self, x, y):
			self.x = x
			self.y = y
			self.FixPosition()

		def FixPosition(self):
			y_offset = (self.GetHeight() - self.height) * self.scrollPosition
			ui.TextLine.SetPosition(self, self.x, self.y - y_offset)

			clip_y = max(0, y_offset) + 10
			self.SetClipRect(0, clip_y, self.width, clip_y + self.height)

		def Append(self, text, color=None):
			text = emoji_config.ReplaceEmoticons(text)
			selfText = self.GetText()
			colorText = colorInfo.TextTag(color)
			self.SetText("{0}\n{1}{2}".format(selfText, colorText, text))
			self.FixPosition()

		def OnCanIgnorePick(self):
			return self.GetHyperlinkAtPos(*self.GetMouseLocalPosition()) == ""

		def OnMouseLeftButtonDown(self):
			hyperlink = self.GetHyperlinkAtPos(*self.GetMouseLocalPosition())
			if hyperlink:
				if app.IsPressed(app.VK_MENU):
					link = chat.GetLinkFromHyperlink(hyperlink)

					active = wndMgr.GetFocus()
					if isinstance(active, ui.EditLine):
						active.Insert(link)
				else:
					self.GetParent().GetParent().system.interface.MakeHyperlinkTooltip(hyperlink)

				return True

			return False

	def __init__(self, system, eventMinimize, eventClose):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")
		self.system = system

		self.targetName = ""
		self.board = None
		self.eventMinimize = MakeEvent(eventMinimize)
		self.eventClose = MakeEvent(eventClose)
		self.eventAcceptTarget = None

	def LoadDialog(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/WhisperDialog.py")

			GetObject = self.GetChild
			self.titleName = GetObject("name")
			self.titleNameEdit = GetObject("titlename_edit")
			self.closeButton = GetObject("closebutton")
			self.scrollBar = GetObject("scrollbar")
			self.chatLine = GetObject("chatline")
			self.minimizeButton = GetObject("minimizebutton")
			self.ignoreButton = GetObject("ignorebutton")
			self.acceptButton = GetObject("acceptbutton")
			self.sendButton = GetObject("sendbutton")
			self.board = GetObject("board")
			self.emojiBoard = GetObject("emoji_board")
			self.editBar = GetObject("editbar")
			self.gamemasterMark = GetObject("gamemastermark")
			self.languageFlag = GetObject("language_flag")
			self.addFriendButton = GetObject("AddFriendButton")
			self.emojiToggleButton = GetObject("ToggleEmojiButton")
			self.totalMessageCount = 0

			self.OnToggleEmojiBoard()

			self.emojiButtons = []
			for k, v in emoji_config.EMOJI_DICT.iteritems():
				if v["disable_for_players"]:
					continue

				emojiButton = ui.Button()
				emojiButton.SetParent(self.emojiBoard)
				emojiButton.SetUpVisual(v["file"])
				emojiButton.SetOverVisual(v["file"])
				emojiButton.SetDownVisual(v["file"])
				emojiButton.SetEvent(Event(self.EmoticonWrite, v["codes"][0]))
				#emojiButton.SetToolTipText(self.EmoticonWrite, k)
				emojiButton.Show()
				self.emojiButtons.append(emojiButton)

			def chunks(l, n):
				n = max(1, n)
				return (l[i:i + n] for i in xrange(0, len(l), n))

			for i, chunk in enumerate(chunks(self.emojiButtons, 8)):
				for j, item in enumerate(chunk):
					item.SetPosition(25 + (25 * i), 20 + (20 * j))


			self.gamemasterMark.Hide()
			self.languageFlag.Hide()
			self.titleName.SetText("")
			self.titleNameEdit.SetText("")
			self.minimizeButton.SetEvent(self.Minimize)
			self.closeButton.SetEvent(self.Close)
			self.scrollBar.SetPos(1.0)
			self.scrollBar.SetScrollEvent(self.OnScroll)
			self.chatLine.SetReturnEvent(self.SendWhisper)
			self.chatLine.SetEscapeEvent(self.Minimize)
			self.chatLine.SetMultiLine()
			self.sendButton.SetEvent(self.SendWhisper)
			self.titleNameEdit.SetReturnEvent(self.AcceptTarget)
			self.titleNameEdit.SetEscapeEvent(self.Close)
			self.ignoreButton.SetToggleDownEvent(self.IgnoreTarget)
			self.ignoreButton.SetToggleUpEvent(self.IgnoreTarget)
			self.acceptButton.SetEvent(self.AcceptTarget)
			self.addFriendButton.SetEvent(self.OnPressAddFriendButton)
			self.emojiToggleButton.SetEvent(self.OnToggleEmojiBoard)
			self.textLine = self.Text()
			self.textLine.SetParent(self.board)
			self.textLine.SetPosition(20, 30)
			self.textLine.Show()
			self.resizeButton = self.ResizeButton()
			self.resizeButton.SetParent(self.board)
			self.resizeButton.SetSize(20, 20)
			self.resizeButton.SetPosition(315, 195)
			self.resizeButton.SetMoveEvent(self.ResizeWhisperDialog)
			self.resizeButton.Show()

			self.ResizeWhisperDialog()
		except Exception as e:
			logging.exception(e)

	def Destroy(self):
		self.eventMinimize = None
		self.eventClose = None
		self.eventAcceptTarget = None

		self.ClearDictionary()
		self.scrollBar.Destroy()
		self.textLine.Destroy()
		self.titleName = None
		self.titleNameEdit = None
		self.closeButton = None
		self.scrollBar = None
		self.chatLine = None
		self.sendButton = None
		self.ignoreButton = None
		self.acceptButton = None
		self.minimizeButton = None

		self.textLine = None
		self.board = None
		self.editBar = None
		self.resizeButton = None
		self.totalMessageCount = 0

	def ResizeWhisperDialog(self):
		(xPos, yPos) = self.resizeButton.GetLocalPosition()

		if xPos < 280:
			xPos = 280
			self.resizeButton.SetPosition(xPos, yPos)

		if yPos < 200:
			yPos = 200
			self.resizeButton.SetPosition(xPos, yPos)

		self.SetWhisperDialogSize(xPos + 20, yPos + 20)

	def OnPressAddFriendButton(self):
		if self.titleName.GetText():
			appInst.instance().GetNet().SendMessengerAddByNamePacket(self.titleName.GetText())

	def OnToggleEmojiBoard(self):
		if self.emojiBoard.IsShow():
			self.SetSize(self.GetWidth(), self.GetHeight() - self.emojiBoard.GetHeight())
			self.emojiBoard.Hide()
		else:
			self.SetSize(self.GetWidth(), self.GetHeight() + self.emojiBoard.GetHeight())
			self.emojiBoard.Show()

	def SetWhisperDialogSize(self, width, height):
		max = int((width - 90) / 6) * 3 - 6

		self.emojiBoard.SetPosition(0, height)
		self.scrollBar.SetScrollBarSize(height - 130)
		self.scrollBar.SetMiddleBarSize(0.3)
		self.scrollBar.SetPos(1.0)
		self.scrollBar.Hide()
		self.editBar.SetSize(width-18, 55)
		self.editBar.SetPosition(10, height-70)
		self.chatLine.SetSize(width-90, 40)
		self.chatLine.SetLimitWidth(width-90)
		self.SetSize(width, height)
		self.board.SetSize(width, height)

		self.SetChatLineMax(max)

		self.textLine.SetPosition(20, 30)
		self.textLine.SetSize(width - 55, height - 112)

		self.SetSize(width, height)
		self.SetChatLineMax(max)

	def SetChatLineMax(self, max):
		self.chatLine.SetMax(max)

		text = self.chatLine.GetText()
		if text:
			self.chatLine.SetText(grpText.GetLines(text, max)[0])

	def OpenWithTarget(self, targetName):
		self.titleName.SetText(targetName)
		self.targetName = targetName
		self.titleNameEdit.Hide()
		self.ignoreButton.Hide()
		self.acceptButton.Hide()
		self.gamemasterMark.Hide()
		self.languageFlag.Hide()
		self.minimizeButton.Show()
		if messenger.IsFriendByName(targetName) or messenger.IsTeamByName(targetName):
			self.addFriendButton.Hide()

	def OpenWithoutTarget(self, event):
		self.eventAcceptTarget = MakeEvent(event)
		self.titleName.SetText("")
		self.titleNameEdit.SetText("")
		self.targetName = ""
		self.titleNameEdit.Show()
		self.ignoreButton.Hide()
		self.acceptButton.Show()
		self.minimizeButton.Hide()
		self.gamemasterMark.Hide()
		self.languageFlag.Hide()
		self.titleNameEdit.SetFocus()

	def SetGameMasterLook(self):
		self.gamemasterMark.Show()
		self.addFriendButton.Hide()

	def SetPlayerWhisperLanguage(self, lang):
		self.languageFlag.LoadImage("d:/ymir work/ui/game/emote/{}.sub".format(lang))
		self.languageFlag.Show()

	def Minimize(self):
		self.titleNameEdit.KillFocus()
		self.chatLine.KillFocus()
		self.Hide()

		if self.eventMinimize:
			self.eventMinimize(self.targetName)

	def Close(self):
		self.titleNameEdit.KillFocus()
		self.chatLine.KillFocus()
		self.Hide()

		if self.eventClose:
			self.eventClose(self.targetName)

	def OnUpdate(self):
		if len(self.targetName) > 1 and (messenger.IsFriendByName(self.targetName) or messenger.IsTeamByName(self.targetName)):
			self.addFriendButton.Hide()
		else:
			self.addFriendButton.Show()

	def IgnoreTarget(self):
		appInst.instance().GetNet().SendChatPacket("/ignore " + self.targetName)

	def AcceptTarget(self):
		name = self.titleNameEdit.GetText()
		if len(name) <= 0:
			self.Close()
			return

		if self.eventAcceptTarget:
			self.titleNameEdit.KillFocus()
			self.chatLine.SetFocus()
			if messenger.IsFriendByName(name) or messenger.IsTeamByName(name):
				self.addFriendButton.Hide()
			self.eventAcceptTarget(name)

	def OnScroll(self):
		self.textLine.SetScrollPosition(self.scrollBar.GetPos())

	def SendWhisper(self):
		text = self.chatLine.GetText()
		if not text:
			return

		if appInst.instance().GetNet().IsInsultIn(text):
			chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHAT_INSULT_STRING)
			return

		appInst.instance().GetNet().SendWhisperPacket(self.targetName, text)
		self.chatLine.SetText("")

		line = localeInfo.FormatWhisperLine(player.GetName(), text)
		self.AppendMessage(line, WHISPER_TYPE_NORMAL)

	def AppendMessage(self, message, type):
		color = None
		try:
			color = colorInfo.WHISPER_COLOR[type]
		except KeyError:
			pass

		self.textLine.Append(message, color)
		self.totalMessageCount += 1

		if self.totalMessageCount > 2:
			self.scrollBar.Show()


	def DelegateFocus(self):
		if not self.titleName.GetText():
			self.titleNameEdit.SetFocus()
		else:
			self.chatLine.SetFocus()

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Minimize()
			return True

	def OnMouseLeftButtonUp(self):
		self.DelegateFocus()

	def EmoticonWrite(self, msg):
		self.chatLine.Insert(msg)
