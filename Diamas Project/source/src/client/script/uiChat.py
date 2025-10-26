# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import weakref

import app
import chat
import grp
import wndMgr
from pygame.app import GetLangDisplayName, GetLanguageNameByID
from pygame.app import appInst

import colorInfo
import emoji_config
import localeInfo
import ui
import uiToolTip
from ui_event import MakeEvent, Event

CHAT_COLOR_DICT = {
	CHAT_TYPE_TALKING: colorInfo.CHAT_RGB_TALK,
	CHAT_TYPE_INFO: colorInfo.CHAT_RGB_INFO,
	CHAT_TYPE_NOTICE: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_PARTY: colorInfo.CHAT_RGB_PARTY,
	CHAT_TYPE_GUILD: colorInfo.CHAT_RGB_GUILD,
	CHAT_TYPE_COMMAND: colorInfo.CHAT_RGB_COMMAND,
	CHAT_TYPE_SHOUT: colorInfo.CHAT_RGB_SHOUT,
	CHAT_TYPE_WHISPER: colorInfo.CHAT_RGB_WHISPER,
	CHAT_TYPE_BIG_NOTICE: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_DICE_INFO: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_BIG_CONTROL: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_MISSION: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_SUB_MISSION: colorInfo.CHAT_RGB_NOTICE,
	CHAT_TYPE_TEAM: colorInfo.CHAT_RGB_TEAM,
	CHAT_TYPE_EMPIRE: colorInfo.CHAT_RGB_NOTICE,
}


class ChatModeButton(ui.Window):
	"""Chat mode select button"""

	OUTLINE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 1.0)
	OVER_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.3)

	BUTTON_STATE_UP = 0
	BUTTON_STATE_OVER = 1
	BUTTON_STATE_DOWN = 2

	def __init__(self):
		ui.Window.__init__(self)
		self.state = None
		self.buttonText = None
		self.eventChatButton = None

	def SetEvent(self, event):
		self.eventChatButton = MakeEvent(event)

	def SetText(self, text):
		if not self.buttonText:
			textLine = ui.TextLine()
			textLine.SetParent(self)
			textLine.SetHorizontalAlignCenter()
			textLine.SetVerticalAlignCenter()
			textLine.SetPackedFontColor(self.OUTLINE_COLOR)
			textLine.Show()
			self.buttonText = textLine

		self.buttonText.SetText(text)

	def OnMouseOverIn(self):
		self.state = self.BUTTON_STATE_OVER
		return True

	def OnMouseOverOut(self):
		self.state = self.BUTTON_STATE_UP
		return True

	def OnMouseLeftButtonDown(self):
		self.state = self.BUTTON_STATE_DOWN
		return True

	def OnMouseLeftButtonUp(self):
		if self.IsIn():
			self.state = self.BUTTON_STATE_OVER
		else:
			self.state = self.BUTTON_STATE_UP

		if self.eventChatButton:
			self.eventChatButton()
			return True
		return False

	def OnRender(self):
		(x, y, w, h) = self.GetRect()

		grp.SetColor(self.OUTLINE_COLOR)
		grp.RenderRoundBox(x, y, w, h)

		if self.state >= self.BUTTON_STATE_OVER:
			grp.RenderRoundBox(x + 1, y, w - 2, h)
			grp.RenderRoundBox(x, y + 1, w, h - 2)

			if self.BUTTON_STATE_DOWN == self.state:
				grp.SetColor(self.OVER_COLOR)
				grp.RenderBar(x + 1, y + 1, w - 2, h - 2)


class ChatLines(ui.Window):
	"""Chat windows content, multiple stacked lines"""

	MODE_FULL = 0
	MODE_ONLY_NEW = 1
	SHOW_NEW_FOR = 2.0
	SHOW_NEW_ALPHA_DEC = 0.5

	class Line(ui.TextLine):
		def __init__(self, parent):
			ui.TextLine.__init__(self)

			self.addTime = app.GetTime()
			self.color = (1.0, 1.0, 1.0)
			self.RemoveFlag("not_pick")
			self.SetParent(parent)
			self.SetWindowName("ChatLogLine")

		def Reset(self):
			self.addTime = app.GetTime()

		def SetText(self, text, color=(1.0, 1.0, 1.0), type=0):
			self.color = color
			self.type = type
			ui.TextLine.SetText(self, text)
			ui.TextLine.SetFontColor(self, *color)

		def UpdateAlpha(self, alpha):
			self.SetFontColor(self.color[0], self.color[1], self.color[2], alpha)

		def OnUpdate(self):
			parent = self.GetParent()

			if parent.mode != ChatLines.MODE_ONLY_NEW:
				return

			if not appInst.instance().GetSettings().IsViewChat():
				self.Hide()
				parent.DecHeight()
				return

			time = self.addTime + ChatLines.SHOW_NEW_FOR
			if app.GetTime() < time:
				return

			time = app.GetTime() - time
			if time > ChatLines.SHOW_NEW_ALPHA_DEC:
				self.Hide()
				parent.DecHeight()
			else:
				self.UpdateAlpha((ChatLines.SHOW_NEW_ALPHA_DEC - time) / ChatLines.SHOW_NEW_ALPHA_DEC)

		def OnCanIgnorePick(self):
			return self.GetHyperlinkAtPos(*self.GetMouseLocalPosition()) == ""

		def OnMouseLeftButtonDown(self):
			hyperlink = ui.GetHyperlink()
			if hyperlink:
				if app.IsPressed(app.VK_MENU):
					link = chat.GetLinkFromHyperlink(hyperlink)
					active = wndMgr.GetFocus()
					if isinstance(active, ui.EditLine):
						active.Insert(link)
				else:
					self.GetParent().interface.MakeHyperlinkTooltip(hyperlink)
				return True

	def __init__(self, maxCount=100):
		ui.Window.__init__(self)
		self.AddFlag("not_pick")

		self.maxCount = maxCount
		self.lines = []
		self.step = 18
		self.mode = self.MODE_FULL
		self.visibleLines = 0
		self.scrollPosition = None
		self.filterModes = set()
		self.left_padding = 8
		self.interface = None

	def AddFilter(self, mode):
		self.filterModes.add(mode)
		self.FixPosition()

	def RemoveFilter(self, mode):
		try:
			self.filterModes.remove(mode)
		except KeyError:
			pass

		self.FixPosition()

	def ToggleFilter(self, mode):
		try:
			self.filterModes.remove(mode)
		except KeyError:
			self.filterModes.add(mode)

		self.FixPosition()

	def Append(self, text, color, chatType):
		try:
			if len(self.lines) > self.maxCount:
				line = self.lines.pop(0)
				line.Reset()
			else:
				line = ChatLines.Line(self)

			self.lines.append(line)

			line.SetText(text, color, chatType)

			if self.mode == self.MODE_ONLY_NEW:
				# IncHeight should call FixPosition at some point due to size updates
				self.IncHeight()
			else:
				self.FixPosition()
		except TypeError:
			return

	def FixPosition(self):
		# We start at the end
		last_y = self.GetHeight() - self.step
		pos = last_y

		if None != self.scrollPosition:
			lines = 0
			for line in self.lines:
				if line.type not in self.filterModes:
					lines += 1

			scrollLineHeight = max(0, lines * self.step - self.GetHeight())

			# We simply extend the start pos by the height of the area we want to hide
			pos += scrollLineHeight - scrollLineHeight * self.scrollPosition

		self.visibleLines = 0
		for line in reversed(self.lines):
			if pos < 0 or line.type in self.filterModes:
				# Line is before the beginning of our currently visible region
				line.Hide()
			elif pos > last_y:
				# Line is near the end of our currently visible region
				line.Hide()
				pos -= self.step
			else:
				if self.mode != self.MODE_ONLY_NEW:
					line.UpdateAlpha(min(1.0, float(pos) / self.step))

				line.SetPosition(self.left_padding, pos)
				line.Show()

				pos -= self.step
				self.visibleLines += 1

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		self.FixPosition()

	def DecHeight(self):
		assert self.mode == self.MODE_ONLY_NEW, "Should only be used in only_new mode"
		self.visibleLines -= 1
		self.SetSize(self.GetWidth(), self.GetVisibleHeight())

	def IncHeight(self):
		assert self.mode == self.MODE_ONLY_NEW, "Should only be used in only_new mode"
		self.visibleLines += 1
		self.SetSize(self.GetWidth(), self.GetVisibleHeight())

	def GetVisibleHeight(self):
		return self.visibleLines * self.step

	def SetFull(self):
		self.mode = self.MODE_FULL

	def SetNew(self):
		self.mode = self.MODE_ONLY_NEW

	def BindInterface(self, interface):
		from _weakref import proxy
		self.interface = proxy(interface)

	def HideViewingChat(self):
		for l in self.lines:
			l.UpdateAlpha(0.0)

	def ShowViewingChat(self):
		for l in self.lines:
			self.IncHeight()
			l.Show()
			l.Reset()
			l.UpdateAlpha(1.0)
			l.OnUpdate()

	def OnUpdate(self):
		# Necessary because CTextLine overrides OnUpdate()
		# => ui.TextLine subclasses never receive OnUpdate() events
		for l in self.lines:
			if l.IsShow():
				l.OnUpdate()


class ChatLine(ui.EditLine):
	"""Chat input line"""

	CHAT_MODE_NAME = {
		CHAT_TYPE_TALKING: localeInfo.CHAT_NORMAL,
		CHAT_TYPE_PARTY: localeInfo.CHAT_PARTY,
		CHAT_TYPE_GUILD: localeInfo.CHAT_GUILD,
		CHAT_TYPE_SHOUT: localeInfo.CHAT_SHOUT,
	}

	def __init__(self):
		ui.EditLine.__init__(self)

		self.lastShoutTime = 0
		self.eventEscape = None
		self.eventReturn = None
		self.eventTab = None
		self.chatMode = CHAT_TYPE_TALKING
		self.SetOnChangeEvent(self.OnChange)

		self.overTextLine = ui.TextLine()
		self.overTextLine.SetParent(self)
		self.overTextLine.SetPosition(-1, 0)
		self.overTextLine.SetFontColor(1.0, 1.0, 0.0)
		self.overTextLine.SetOutline()
		self.overTextLine.Hide()

		self.lastSentenceStack = []
		self.lastSentencePos = 0

	def SetChatMode(self, mode):
		self.chatMode = mode

	def GetChatMode(self):
		return self.chatMode

	def ChangeChatMode(self):
		if CHAT_TYPE_TALKING == self.GetChatMode():
			self.SetChatMode(CHAT_TYPE_PARTY)
			self.SetText("#")
			self.SetEndPosition()
		elif CHAT_TYPE_PARTY == self.GetChatMode():
			self.SetChatMode(CHAT_TYPE_GUILD)
			self.SetText("%")
			self.SetEndPosition()
		elif CHAT_TYPE_GUILD == self.GetChatMode():
			self.SetChatMode(CHAT_TYPE_SHOUT)
			self.SetText("!")
			self.SetEndPosition()
		elif CHAT_TYPE_SHOUT == self.GetChatMode():
			self.SetChatMode(CHAT_TYPE_TALKING)
			self.SetText("")
			self.SetEndPosition()

		self.__CheckChatMark()

	def GetCurrentChatModeName(self):
		return self.CHAT_MODE_NAME[self.chatMode]

	def SetEscapeEvent(self, event):
		self.eventReturn = MakeEvent(event)

	def SetReturnEvent(self, event):
		self.eventEscape = MakeEvent(event)

	def SetTabEvent(self, event):
		self.eventTab = MakeEvent(event)

	def OpenChat(self):
		self.SetFocus()
		self.__ResetChat()

	def RunCloseEvent(self):
		if self.eventEscape:
			self.eventEscape()

	def OnChange(self):
		self.__CheckChatMark()

	def __ClearChat(self):
		self.SetText("")
		self.lastSentencePos = 0

	def __ResetChat(self):
		if CHAT_TYPE_PARTY == self.GetChatMode():
			self.SetText("#")
			self.SetEndPosition()
		elif CHAT_TYPE_GUILD == self.GetChatMode():
			self.SetText("%")
			self.SetEndPosition()
		elif CHAT_TYPE_SHOUT == self.GetChatMode():
			self.SetText("!")
			self.SetEndPosition()
		else:
			self.__ClearChat()

		self.__CheckChatMark()

	def GetLinks(self, string, ret):
		import re
		links = re.findall(
			"(?:(?:https?|ftp|file):\/\/|www\.|ftp\.)(?:\([-A-Z0-9+&@#/%=~_|$?!:,.]*\)|[-A-Z0-9+&@#/%=~_|$?!:,.])*(?:\([-A-Z0-9+&@#/%=~_|$?!:,.]*\)|[A-Z0-9+&@#/%=~_|$])",
			string, re.I)
		if not (hasattr(ret, "clear") and hasattr(ret, "update")):
			return False

		ret.clear()
		map(lambda link: (
			ret.update({link: "|cFF00C0FC|Hweb:%s|h[%s]|h|r" % (re.sub("://", "w<?", link), link)})) if link else None,
			links)
		return len(links) > 0

	def __SendChatPacket(self, text, type):
		text = emoji_config.ReplaceEmoticons(text, type == CHAT_TYPE_SHOUT)
		import player
		if player.IsGameMaster():
			links = {}
			if self.GetLinks(text, links):
				for k, v in links.iteritems():
					text = text.replace(k, v)

		if appInst.instance().GetNet().IsInsultIn(text) and not player.IsGameMaster():
			chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHAT_INSULT_STRING)
		else:
			appInst.instance().GetNet().SendChatPacketWithType(text, type)

	def __SendPartyChatPacket(self, text):
		if 1 == len(text):
			self.RunCloseEvent()
			return

		self.__SendChatPacket(text[1:], CHAT_TYPE_PARTY)
		self.__ResetChat()

	def __SendGuildChatPacket(self, text):
		if 1 == len(text):
			self.RunCloseEvent()
			return

		self.__SendChatPacket(text[1:], CHAT_TYPE_GUILD)
		self.__ResetChat()

	def __SendShoutChatPacket(self, text):
		if 1 == len(text):
			self.RunCloseEvent()
			return

		if app.GetTime() < self.lastShoutTime + 8:
			chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CHAT_SHOUT_LIMIT)
			self.__ResetChat()
			return

		self.__SendChatPacket(text[1:], CHAT_TYPE_SHOUT)
		self.__ResetChat()

		self.lastShoutTime = app.GetTime()

	def __SendTalkingChatPacket(self, text):
		self.__SendChatPacket(text, CHAT_TYPE_TALKING)
		self.__ResetChat()

	def OnTab(self):
		if self.eventTab:
			self.eventTab()

		return True

	def OnChar(self, ch):
		self.__CheckChatMark()
		return ui.EditLine.OnChar(self, ch)

	def OnKeyDown(self, key):
		# LAST_SENTENCE_STACK
		if app.VK_UP == key:
			self.__PrevLastSentenceStack()
			return True

		if app.VK_DOWN == key:
			self.__NextLastSentenceStack()
			return True
		# END_OF_LAST_SENTENCE_STACK

		if app.VK_RETURN == key:
			self.__HandleReturn()
			return True

		if key == app.VK_ESCAPE:
			self.__ClearChat()
			self.eventEscape()
			return True

		return ui.EditLine.OnKeyDown(self, key)

	# LAST_SENTENCE_STACK
	def __PrevLastSentenceStack(self):
		if self.lastSentencePos < len(self.lastSentenceStack):
			self.lastSentencePos += 1
			self.SetText(self.lastSentenceStack[-self.lastSentencePos])
			self.SetEndPosition()

	def __NextLastSentenceStack(self):
		if self.lastSentencePos > 1:
			self.lastSentencePos -= 1
			self.SetText(self.lastSentenceStack[-self.lastSentencePos])
			self.SetEndPosition()

	def __PushLastSentenceStack(self, text):
		if len(text) <= 0:
			return

		LAST_SENTENCE_STACK_SIZE = 64
		if len(self.lastSentenceStack) > LAST_SENTENCE_STACK_SIZE:
			self.lastSentenceStack.pop(0)

		self.lastSentenceStack.append(text)

	# END_OF_LAST_SENTENCE_STACK

	def __CheckChatMark(self):
		self.overTextLine.Hide()

		text = self.GetText()
		if len(text) > 0:
			if '#' == text[0]:
				self.overTextLine.SetText("#")
				self.overTextLine.Show()
			elif '%' == text[0]:
				self.overTextLine.SetText("%")
				self.overTextLine.Show()
			elif '!' == text[0]:
				self.overTextLine.SetText("!")
				self.overTextLine.Show()

	def __HandleReturn(self):
		text = self.GetText()
		textLen = len(text)

		# LAST_SENTENCE_STACK
		self.__PushLastSentenceStack(text)
		# END_OF_LAST_SENTENCE_STACK

		textSpaceCount = text.count(' ')

		if (textLen > 0) and (textLen != textSpaceCount):
			if '#' == text[0]:
				self.__SendPartyChatPacket(text)
			elif '%' == text[0]:
				self.__SendGuildChatPacket(text)
			elif '!' == text[0]:
				self.__SendShoutChatPacket(text)
			else:
				self.__SendTalkingChatPacket(text)
		else:
			self.__ClearChat()
			if self.eventReturn:
				self.eventReturn()


class ChatInputSet(ui.Window):
	"""Input, chat mode and send button for chat window"""

	CHAT_OUTLINE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 1.0)

	def __init__(self):
		ui.Window.__init__(self)

		self.chatModeButton = None
		self.chatLine = None
		self.btnSend = None
		self.__Create()

	def __Create(self):
		chatModeButton = ChatModeButton()
		chatModeButton.SetParent(self)
		chatModeButton.SetSize(40, 17)
		chatModeButton.SetText(localeInfo.CHAT_NORMAL)
		chatModeButton.SetPosition(7, 2)
		chatModeButton.SetEvent(self.OnChangeChatMode)
		chatModeButton.Show()
		self.chatModeButton = chatModeButton

		chatLine = ChatLine()
		chatLine.SetParent(self)
		chatLine.SetMax(512)
		chatLine.SetMaxVisible(76)
		chatLine.SetText("")
		chatLine.SetTabEvent(self.OnChangeChatMode)
		chatLine.SetPosition(57, 5)
		chatLine.Show()
		self.chatLine = chatLine

		btnSend = ui.Button()
		btnSend.SetParent(self)
		btnSend.SetUpVisual("d:/ymir work/ui/game/taskbar/Send_Chat_Button_01.sub")
		btnSend.SetOverVisual("d:/ymir work/ui/game/taskbar/Send_Chat_Button_02.sub")
		btnSend.SetDownVisual("d:/ymir work/ui/game/taskbar/Send_Chat_Button_03.sub")
		btnSend.SetToolTipText(localeInfo.CHAT_SEND_CHAT)
		btnSend.Show()
		self.btnSend = btnSend

	def Destroy(self):
		self.chatModeButton = None
		self.chatLine = None
		self.btnSend = None

	def Open(self):
		self.chatLine.OpenChat()
		self.Show()
		return True

	def Close(self):
		self.chatLine.KillFocus()
		self.Hide()
		return True

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)

		self.chatLine.SetSize(width - 121, 13)
		self.btnSend.SetPosition(width - 53, 2)

	def SetEscapeEvent(self, event):
		self.chatLine.SetEscapeEvent(event)

	def SetReturnEvent(self, event):
		self.chatLine.SetReturnEvent(event)
		self.btnSend.SetEvent(event)

	def OnChangeChatMode(self):
		self.OnRefreshChatMode()

	def OnRefreshChatMode(self):
		self.chatLine.ChangeChatMode()
		self.chatModeButton.SetText(self.chatLine.GetCurrentChatModeName())

	def SetChatFocus(self):
		self.chatLine.SetFocus()

	def KillChatFocus(self):
		self.chatLine.KillFocus()

	def ChangeText(self, msg):
		self.chatLine.Insert(msg)

	def OnRender(self):
		x, y, width, height = self.chatLine.GetRect()
		ui.RenderRoundBox(x - 4, y - 3, width + 7, height + 4, self.CHAT_OUTLINE_COLOR)


class ChatWindow(ui.Window):
	BOARD_START_COLOR = grp.GenerateColor(0.1, 0.1, 0.1, 0.66);
	BOARD_END_COLOR = grp.GenerateColor(0.1, 0.1, 0.1, 0.66);
	BOARD_MIDDLE_COLOR = grp.GenerateColor(0.2, 0.2, 0.2, 0.3);
	CHAT_OUTLINE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 1.0);

	EDIT_LINE_HEIGHT = 20
	RESIZE_BAR_HEIGHT = 22

	CHAT_WINDOW_WIDTH = 625
	CHAT_WINDOW_MIN_HEIGHT = EDIT_LINE_HEIGHT + RESIZE_BAR_HEIGHT

	class ResizeBar(ui.DragButton):
		def __init__(self):
			ui.DragButton.__init__(self)

			self.AddFlag("float")
			self.AddFlag("moveable")
			self.AddFlag("restrict_x")
			self.SetWindowName("ChatWindow:ChatButton")

			self.owner = None
			self.topFlag = False

		def SetOwner(self, owner):
			self.owner = weakref.proxy(owner)

		def OnMouseOverIn(self):
			app.SetCursor(app.VSIZE)
			return True

		def OnMouseOverOut(self):
			app.SetCursor(app.NORMAL)
			return True

		def OnTop(self):
			if self.topFlag:
				return

			self.topFlag = True
			self.owner.SetTop()
			self.topFlag = False

		def SetWidth(self, width):
			ui.DragButton.SetSize(self, width, 22)

	class ChatEmoticon(ui.Window):

		def __init__(self):
			ui.Window.__init__(self)
			self.AddFlag("float")
			# self.AddFlag("moveable")
			# self.AddFlag("restrict_x")
			self.topFlag = False
			self.SetWindowName("ChatWindow:ChatEmoticon")

		def SetOwner(self, owner):
			self.owner = weakref.proxy(owner)

		def OnTop(self):
			if self.topFlag:
				return

			self.topFlag = True
			self.owner.SetTop()
			self.topFlag = False

		def OnRender(self):
			try:
				(x, y) = self.GetGlobalPosition()

				grp.SetColor(colorInfo.UI_COLOR_SELECTED)
				grp.RenderBar(x - 4, y - 4, self.GetWidth(), self.GetHeight())
			except OverflowError:
				# Cython bug causes this...
				pass

	def __init__(self):
		ui.Window.__init__(self)
		self.AddFlag("float")
		self.AddFlag("not_pick")

		self.xBar = 0
		self.yBar = 0
		self.curHeightBar = 0

		self.chatLines = ChatLines()
		self.chatLines.SetParent(self)
		self.chatLines.Show()

		chatInputSet = ChatInputSet()
		chatInputSet.SetParent(self)
		chatInputSet.SetEscapeEvent(self.CloseChat)
		chatInputSet.SetReturnEvent(self.CloseChat)
		chatInputSet.SetSize(550 - 20, 100)
		self.chatInputSet = chatInputSet

		btnEmoticon = []
		self.btnEmoticon = []

		btnSendEmoticon = ui.Button()
		btnSendEmoticon.SetParent(self)
		btnSendEmoticon.SetUpVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_01.sub")
		btnSendEmoticon.SetOverVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_02.sub")
		btnSendEmoticon.SetDownVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_01.sub")
		btnSendEmoticon.SetToolTipText(localeInfo.CHAT_OPEN_EMOJI)
		btnSendEmoticon.SetEvent(self.ToggleEmoticonWindow)
		btnSendEmoticon.Hide()
		self.btnSendEmoticon = btnSendEmoticon

		btnSendWhisper = ui.Button()
		btnSendWhisper.SetParent(self)
		btnSendWhisper.SetUpVisual("d:/ymir work/ui/game/taskbar/Send_Whisper_Button_01.sub")
		btnSendWhisper.SetOverVisual("d:/ymir work/ui/game/taskbar/Send_Whisper_Button_02.sub")
		btnSendWhisper.SetDownVisual("d:/ymir work/ui/game/taskbar/Send_Whisper_Button_03.sub")
		btnSendWhisper.SetToolTipText(localeInfo.CHAT_SEND_MEMO)
		btnSendWhisper.Hide()
		self.btnSendWhisper = btnSendWhisper

		wndFilterChat = ChatFilterWindow()
		wndFilterChat.AddFlag("float")
		wndFilterChat.Hide()
		self.wndFilterChat = wndFilterChat

		btnFilterChat = ui.Button()
		btnFilterChat.SetParent(self)
		btnFilterChat.SetUpVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_01.sub")
		btnFilterChat.SetOverVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_02.sub")
		btnFilterChat.SetDownVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_03.sub")
		btnFilterChat.SetToolTipText(localeInfo.CHAT_FILTER)
		btnFilterChat.SetEvent(self.ToggleFilterChatWindow)
		btnFilterChat.Hide()
		self.btnFilterChat = btnFilterChat

		btnChatLog = ui.Button()
		btnChatLog.SetParent(self)
		btnChatLog.SetUpVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_01.sub")
		btnChatLog.SetOverVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_02.sub")
		btnChatLog.SetDownVisual("d:/ymir work/ui/game/taskbar/Open_Chat_Log_Button_03.sub")
		btnChatLog.SetToolTipText(localeInfo.CHAT_LOG)
		btnChatLog.Hide()
		self.btnChatLog = btnChatLog

		EmoticonWindows = self.ChatEmoticon()
		EmoticonWindows.SetOwner(self)
		EmoticonWindows.Hide()
		self.EmoticonWindows = EmoticonWindows
		for k, v in emoji_config.EMOJI_DICT.iteritems():
			if v["disable_for_players"]:
				continue

			emojiButton = ui.Button()
			emojiButton.SetParent(self.EmoticonWindows)
			emojiButton.SetUpVisual(v["file"])
			emojiButton.SetOverVisual(v["file"])
			emojiButton.SetDownVisual(v["file"])
			emojiButton.SetEvent(Event(self.EmoticonWrite, v["codes"][0]))
			#emojiButton.SetToolTipText(self.EmoticonWrite, k)
			emojiButton.Hide()
			btnEmoticon.append(emojiButton)
		self.btnEmoticon = btnEmoticon

		resizeBar = self.ResizeBar()
		resizeBar.SetOwner(self)
		resizeBar.SetMoveEvent(self.Refresh)
		resizeBar.Hide()
		self.resizeBar = resizeBar

		imgChatBarLeft = ui.ImageBox()
		imgChatBarLeft.SetParent(self.resizeBar)
		imgChatBarLeft.AddFlag("not_pick")
		imgChatBarLeft.LoadImage("d:/ymir work/ui/pattern/chat_bar_left.tga")
		imgChatBarLeft.Show()
		self.imgChatBarLeft = imgChatBarLeft

		imgChatBarRight = ui.ImageBox()
		imgChatBarRight.SetParent(self.resizeBar)
		imgChatBarRight.AddFlag("not_pick")
		imgChatBarRight.LoadImage("d:/ymir work/ui/pattern/chat_bar_right.tga")
		imgChatBarRight.Show()
		self.imgChatBarRight = imgChatBarRight

		imgChatBarMiddle = ui.ExpandedImageBox()
		imgChatBarMiddle.SetParent(self.resizeBar)
		imgChatBarMiddle.AddFlag("not_pick")
		imgChatBarMiddle.LoadImage("d:/ymir work/ui/pattern/chat_bar_middle.tga")
		imgChatBarMiddle.Show()
		self.imgChatBarMiddle = imgChatBarMiddle

		self.chatLines.SetNew()
		self.Refresh()

	def Destroy(self):
		self.chatInputSet.Destroy()
		self.chatInputSet = None
		self.btnEmoticon = []
		self.btnSendEmoticon = None
		self.btnFilterChat = None
		self.EmoticonWindows = None

	################
	## Open & Close
	def OpenChat(self):
		self.chatLines.SetFull()

		if self.IsShow():
			self.resizeBar.Show()

		self.Refresh()

		def chunks(l, n):
			n = max(1, n)
			return (l[i:i + n] for i in xrange(0, len(l), n))

		for i, chunk in enumerate(chunks(self.btnEmoticon, 8)):
			for j, item in enumerate(chunk):
				item.SetPosition(25 * i, 20 * j)

		(x, y, width, height) = self.GetRect()

		self.btnSendEmoticon.SetPosition(self.GetWidth() - 78, self.GetHeight() - 22)
		self.btnSendEmoticon.Show()

		self.btnSendWhisper.SetPosition(self.GetWidth() - 52, self.GetHeight() - 24)
		self.btnSendWhisper.Show()

		self.wndFilterChat.GetGlobalPositionByChatParent(x + width - 60, y + self.GetHeight() - 24)
		self.wndFilterChat.SetTop()
		self.wndFilterChat.UpdateRect()

		self.btnFilterChat.SetPosition(self.GetWidth() - 95, self.GetHeight() - 22)
		self.btnFilterChat.Show()

		self.btnChatLog.SetPosition(self.GetWidth() - 29, self.GetHeight() - 22)
		self.btnChatLog.Show()

		self.chatInputSet.SetPosition(0, self.GetHeight() - 22)
		self.chatInputSet.Open()

	def CloseChat(self):
		self.chatLines.SetNew()
		self.SetSize(self.CHAT_WINDOW_WIDTH, self.chatLines.GetVisibleHeight())

		if self.chatInputSet:
			self.chatInputSet.Close()

		if self.btnSendWhisper:
			self.btnSendWhisper.Hide()

		if self.btnChatLog:
			self.btnChatLog.Hide()

		if self.resizeBar:
			self.resizeBar.Hide()

		if self.btnSendEmoticon:
			self.btnSendEmoticon.Hide()

		if self.EmoticonWindows:
			self.EmoticonWindows.Hide()

		if self.btnFilterChat:
			self.btnFilterChat.Hide()

		self.Refresh()

	def HideViewingChat(self):
		self.chatLines.HideViewingChat()

	def ShowViewingChat(self):
		self.chatLines.ShowViewingChat()

	def ToggleEmoticonWindow(self):
		if self.EmoticonWindows.IsShow():
			for emoticon in self.btnEmoticon:
				emoticon.Hide()
			self.EmoticonWindows.Hide()
		else:
			for emoticon in self.btnEmoticon:
				emoticon.Show()
			self.EmoticonWindows.Show()

	def EmoticonWrite(self, text):
		self.chatInputSet.ChangeText(text)

	def SetSendWhisperEvent(self, event):
		self.btnSendWhisper.SetEvent(event)

	def SetOpenChatLogEvent(self, event):
		self.btnChatLog.SetEvent(event)

	def IsFullMode(self):
		return self.chatLines.mode == self.chatLines.MODE_FULL

	def SetPosition(self, x, y):
		ui.Window.SetPosition(self, x, y)
		self.resizeBar.SetPosition(x, self.resizeBar.GetLocalPosition()[1])

		_, gyChat = self.resizeBar.GetGlobalPosition()
		(_, _, width, _) = self.GetRect()

		self.EmoticonWindows.SetPosition(x + width - 300, gyChat + 25)
		self.EmoticonWindows.SetSize(303, 125 + 18 + 18)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)
		ui.Window.SetPosition(self,
							  self.GetGlobalPosition()[0],
							  self.__GetBaseY() - height)

		self.resizeBar.SetWidth(width)

		if self.IsFullMode():
			self.chatLines.SetSize(width, height - self.EDIT_LINE_HEIGHT)
			self.btnSendWhisper.SetPosition(width - 52, height - 23)
			self.btnSendEmoticon.SetPosition(width - 75, height - 23)
			self.btnFilterChat.SetPosition(width - 98, height - 23)
			self.btnChatLog.SetPosition(width - 29, height - 23)
			self.chatInputSet.SetPosition(0, height - 25)
		else:
			self.chatLines.SetSize(width, height)

	# Set the height of the opened chat window (MODE_FULL)
	# height is expected to already include the size of the edit-line etc.
	def SetOpenHeight(self, height):
		(x, y, width, _) = self.GetRect()
		gxChat, gyChat = self.resizeBar.GetGlobalPosition()
		self.resizeBar.SetPosition(gxChat,
								   self.__GetBaseY() - self.RESIZE_BAR_HEIGHT - height)

		gxChat, gyChat = self.resizeBar.GetGlobalPosition()

		self.EmoticonWindows.SetPosition(x + width - 300, gyChat + 25)
		self.EmoticonWindows.SetSize(303, 125 + 18 + 18)

	def SetChatFocus(self):
		self.chatInputSet.SetChatFocus()

	def BindInterface(self, interface):
		self.chatLines.BindInterface(interface)

	def AppendLine(self, type, line):
		self.chatLines.Append(line, CHAT_COLOR_DICT.get(type, (1.0, 1.0, 1.0)), type)

	def ToggleFilterChatWindow(self):
		if self.wndFilterChat.IsShow():
			self.wndFilterChat.Hide()
		else:
			self.wndFilterChat.Show()
			self.wndFilterChat.SetTop()
	###########
	## Refresh
	def Refresh(self):
		if self.IsFullMode():
			self.RefreshBoardFull()
		else:
			self.RefreshBoardView()

	def RefreshBoardFull(self):
		(x, y, width, height) = self.GetRect()
		(btnX, btnY) = self.resizeBar.GetGlobalPosition()

		maxY = self.__GetBaseY() - self.RESIZE_BAR_HEIGHT - self.EDIT_LINE_HEIGHT
		if btnY > maxY:
			self.resizeBar.SetPosition(btnX, maxY)
			btnY = maxY

		gxChat, gyChat = self.resizeBar.GetGlobalPosition()

		self.EmoticonWindows.SetPosition(x + width - 300, gyChat + 25)
		self.EmoticonWindows.SetSize(303, 125 + 18 + 18)
		self.EmoticonWindows.SetTop()

		self.SetSize(self.GetWidth(), self.__GetBaseY() - btnY)
		self.xBar = x
		self.yBar = btnY
		self.curHeightBar = self.GetHeight()

		self.imgChatBarLeft.SetPosition(0, -8)
		self.imgChatBarRight.SetPosition(width - 64, -8)
		self.imgChatBarMiddle.SetPosition(64, -8)
		self.imgChatBarMiddle.SetRenderingRect(0.0, 0.0, float(width - 128) / 64.0 - 1.0, 0.0)

	def RefreshBoardView(self):
		self.SetSize(self.GetWidth(), self.chatLines.GetVisibleHeight())

		(x, y, width, height) = self.GetRect()

		self.xBar = x
		self.yBar = y - 10

	##########
	## Event

	def OnRender(self):
		self.Refresh()

		# View-mode only. ChatLines modifies its own height
		# (and thus our height)
		if self.curHeightBar != self.GetHeight():
			self.curHeightBar += (self.GetHeight() - self.curHeightBar) / 10

		if self.IsFullMode():
			grp.RenderGradationBar(self.xBar, self.yBar + (self.GetHeight() - self.curHeightBar),
								   self.GetWidth(), self.curHeightBar,
								   self.BOARD_START_COLOR, self.BOARD_END_COLOR)
		elif appInst.instance().GetSettings().IsViewChat():
			grp.RenderGradationBar(self.xBar, self.yBar + (self.GetHeight() - self.curHeightBar),
								   self.GetWidth(), self.curHeightBar + 10 if self.curHeightBar > 0 else 0,
								   self.BOARD_START_COLOR, self.BOARD_END_COLOR)

	def OnTop(self):
		self.resizeBar.SetTop()
		self.EmoticonWindows.SetTop()
		self.wndFilterChat.SetTop()

	#def OnChangeChatMode(self):
	#    self.chatInputSet.OnChangeChatMode()

	def OnPhaseChange(self, stream, phase):
		if phase == "loading":
			# Not strictly necessary since we use weakrefs
			self.interface = None
		elif phase != "game":
			# If we return to the login/charselect scene,
			# erase everything
			stream.RemovePersistentObject("chat")

	def __GetBaseY(self):
		# 66 = task-bar height
		return wndMgr.GetScreenHeight() - 37

	def SetInput(self, what):
		self.OpenChat()
		self.chatInputSet.chatLine.SetFocus()
		self.chatInputSet.chatLine.SetText(what)
		self.chatInputSet.chatLine.SetEndPosition()


class ChatLogWindow(ui.Window):
	BLOCK_WIDTH = 32

	CHAT_MODE_NAME = (
		localeInfo.CHAT_NORMAL,
		localeInfo.CHAT_PARTY,
		localeInfo.CHAT_GUILD,
		localeInfo.CHAT_SHOUT,
		localeInfo.CHAT_INFORMATION,
		localeInfo.CHAT_NOTICE,
	)

	CHAT_MODE_INDEX = (
		CHAT_TYPE_TALKING,
		CHAT_TYPE_PARTY,
		CHAT_TYPE_GUILD,
		CHAT_TYPE_SHOUT,
		CHAT_TYPE_INFO,
		CHAT_TYPE_NOTICE,
	)

	CHAT_LOG_WINDOW_MINIMUM_WIDTH = 450
	CHAT_LOG_WINDOW_MINIMUM_HEIGHT = 120

	class ResizeButton(ui.DragButton):
		def OnMouseOverIn(self):
			app.SetCursor(app.HVSIZE)
			return True

		def OnMouseOverOut(self):
			app.SetCursor(app.NORMAL)
			return True

	def __init__(self):
		ui.Window.__init__(self)

		self.allChatMode = True
		self.chatInputSet = None

		self.AddFlag("float")
		self.AddFlag("moveable")
		self.SetWindowName("ChatLogWindow")

		self.__CreateChatInputSet()
		self.__CreateWindow()
		self.__CreateButton()

		self.chatLines = ChatLines(1000)
		self.chatLines.left_padding = 0
		self.chatLines.SetParent(self)
		self.chatLines.SetPosition(10, 45)
		self.chatLines.Show()

		self.__CreateScrollBar()

		self.SetPosition(20, 20)
		self.SetSize(self.CHAT_LOG_WINDOW_MINIMUM_WIDTH, self.CHAT_LOG_WINDOW_MINIMUM_HEIGHT)
		self.btnSizing.SetPosition(self.CHAT_LOG_WINDOW_MINIMUM_WIDTH - self.btnSizing.GetWidth(),
								   self.CHAT_LOG_WINDOW_MINIMUM_HEIGHT - self.btnSizing.GetHeight() + 2)

		self.OnResize()

	def __CreateChatInputSet(self):
		chatInputSet = ChatInputSet()
		chatInputSet.SetParent(self)
		chatInputSet.SetEscapeEvent(self.Close)
		chatInputSet.SetVerticalAlignBottom()
		chatInputSet.SetPosition(0, 5)
		self.chatInputSet = chatInputSet

	def __CreateWindow(self):
		imgLeft = ui.ImageBox()
		imgLeft.AddFlag("not_pick")
		imgLeft.SetParent(self)

		imgCenter = ui.ExpandedImageBox()
		imgCenter.AddFlag("not_pick")
		imgCenter.SetParent(self)

		imgRight = ui.ImageBox()
		imgRight.AddFlag("not_pick")
		imgRight.SetParent(self)

		if localeInfo.IsARABIC():
			imgLeft.LoadImage("locale/ae/ui/pattern/titlebar_left.tga")
			imgCenter.LoadImage("locale/ae/ui/pattern/titlebar_center.tga")
			imgRight.LoadImage("locale/ae/ui/pattern/titlebar_right.tga")
		else:
			imgLeft.LoadImage("d:/ymir work/ui/pattern/chatlogwindow_titlebar_left.tga")
			imgCenter.LoadImage("d:/ymir work/ui/pattern/chatlogwindow_titlebar_middle.tga")
			imgRight.LoadImage("d:/ymir work/ui/pattern/chatlogwindow_titlebar_right.tga")

		imgLeft.Show()
		imgCenter.Show()
		imgRight.Show()

		btnClose = ui.Button()
		btnClose.SetParent(self)
		btnClose.SetUpVisual("d:/ymir work/ui/public/close_button_01.sub")
		btnClose.SetOverVisual("d:/ymir work/ui/public/close_button_02.sub")
		btnClose.SetDownVisual("d:/ymir work/ui/public/close_button_03.sub")
		btnClose.SetToolTipText(localeInfo.UI_CLOSE, 0, -23)
		btnClose.SetEvent(self.Close)
		btnClose.Show()

		btnSizing = self.ResizeButton()
		btnSizing.SetParent(self)
		btnSizing.SetMoveEvent(self.OnResize)
		btnSizing.SetSize(16, 16)
		btnSizing.Show()

		titleName = ui.TextLine()
		titleName.SetParent(self)

		if localeInfo.IsARABIC():
			titleName.SetPosition(self.GetWidth() - 20, 6)
		else:
			titleName.SetPosition(20, 6)

		titleName.SetFontColor(0.95, 0.71, 0.04)

		titleName.SetText(localeInfo.CHAT_LOG_TITLE)
		titleName.Show()

		self.imgLeft = imgLeft
		self.imgCenter = imgCenter
		self.imgRight = imgRight
		self.btnClose = btnClose
		self.btnSizing = btnSizing
		self.titleName = titleName

	def __CreateButton(self):
		if localeInfo.IsARABIC():
			bx = 20
		else:
			bx = 13

		btnAll = ui.RadioButton()
		btnAll.SetParent(self)
		btnAll.SetPosition(bx, 24)
		btnAll.SetUpVisual("d:/ymir work/ui/public/xsmall_button_01.sub")
		btnAll.SetOverVisual("d:/ymir work/ui/public/xsmall_button_02.sub")
		btnAll.SetDownVisual("d:/ymir work/ui/public/xsmall_button_03.sub")
		btnAll.SetText(localeInfo.CHAT_ALL)
		btnAll.SetEvent(self.ToggleAllChatMode)
		btnAll.Down()
		btnAll.Show()
		self.btnAll = btnAll

		x = bx + 48
		i = 0
		self.modeButtonList = []
		for name in self.CHAT_MODE_NAME:
			btn = ui.ToggleButton()
			btn.SetParent(self)
			btn.SetPosition(x, 24)
			btn.SetUpVisual("d:/ymir work/ui/public/xsmall_button_01.sub")
			btn.SetOverVisual("d:/ymir work/ui/public/xsmall_button_02.sub")
			btn.SetDownVisual("d:/ymir work/ui/public/xsmall_button_03.sub")
			btn.SetText(name)
			btn.Show()

			mode = self.CHAT_MODE_INDEX[i]
			btn.SetToggleUpEvent(Event(self.ToggleChatMode, mode))
			btn.SetToggleDownEvent(Event(self.ToggleChatMode, mode))
			self.modeButtonList.append(btn)

			x += 48
			i += 1

	def __CreateScrollBar(self):
		scrollBar = ui.SmallThinScrollBar()
		scrollBar.SetParent(self)
		scrollBar.Show()
		scrollBar.SetScrollEvent(self.OnScroll)
		self.scrollBar = scrollBar
		self.scrollBarPos = 1.0

	def Destroy(self):
		self.imgLeft = None
		self.imgCenter = None
		self.imgRight = None
		self.btnClose = None
		self.btnSizing = None
		self.modeButtonList = []
		self.scrollBar = None
		self.chatInputSet = None
		self.wndFilterChat.Destroy()
		self.wndFilterChat = None
		self.btnFilterChat = 0

	def ToggleAllChatMode(self):
		if self.allChatMode:
			return

		self.allChatMode = True

		for i in self.CHAT_MODE_INDEX:
			self.chatLines.RemoveFilter(i)

		for btn in self.modeButtonList:
			btn.SetUp()

	def ToggleChatMode(self, mode):
		if self.allChatMode:
			self.allChatMode = False
			for i in self.CHAT_MODE_INDEX:
				self.chatLines.AddFilter(i)
			self.chatLines.RemoveFilter(mode)
			self.btnAll.SetUp()
		else:
			self.chatLines.ToggleFilter(mode)

	def SetSize(self, width, height):
		ui.Window.SetSize(self, width, height)

		self.imgCenter.SetRenderingRect(0.0, 0.0,
										float((width - self.BLOCK_WIDTH * 2) - self.BLOCK_WIDTH) / self.BLOCK_WIDTH,
										0.0)
		self.imgCenter.SetPosition(self.BLOCK_WIDTH, 0)
		self.imgRight.SetPosition(width - self.BLOCK_WIDTH, 0)

		self.btnClose.SetPosition(width - self.btnClose.GetWidth() - 5, 5)

		self.scrollBar.LockScroll()
		self.scrollBar.SetPosition(width - 15, 45)
		self.scrollBar.SetScrollBarSize(height - 45 - 12)
		self.scrollBar.SetPos(self.scrollBarPos)
		self.scrollBar.UnlockScroll()

		self.chatLines.SetSize(width, height - 45 - 25)
		self.chatInputSet.SetSize(width - 20, 20)

	# TODO(tim): Distinction visible/control characters
	# self.chatInputSet.SetChatMax(width / 8)

	def Open(self):
		self.OnResize()
		self.chatInputSet.Open()
		self.Show()

	def Close(self):
		self.chatInputSet.Close()
		self.Hide()

	def BindInterface(self, interface):
		self.chatLines.BindInterface(interface)

	def AppendLine(self, type, line):
		self.chatLines.Append(line, CHAT_COLOR_DICT.get(type, (1.0, 1.0, 1.0)), type)

	def OnResize(self):

		x, y = self.btnSizing.GetLocalPosition()
		width = self.btnSizing.GetWidth()
		height = self.btnSizing.GetHeight()

		cropped_x = max(x, self.CHAT_LOG_WINDOW_MINIMUM_WIDTH - width)
		cropped_y = max(y, self.CHAT_LOG_WINDOW_MINIMUM_HEIGHT - height)

		if cropped_x != x or cropped_y != y:
			self.btnSizing.SetPosition(cropped_x, cropped_y)

		self.SetSize(cropped_x + width, cropped_y + height)

	def OnScroll(self):
		self.scrollBarPos = self.scrollBar.GetPos()
		self.chatLines.scrollPosition = self.scrollBarPos
		self.chatLines.FixPosition()

	def OnRender(self):
		(x, y, width, height) = self.GetRect()

		grp.SetColor(0x77000000)
		grp.RenderBar(x + width - 15, y + 45, 13, height - 45)

		grp.SetColor(0x77000000)
		grp.RenderBar(x, y, width, height)
		grp.SetColor(0x77000000)
		grp.RenderBox(x, y, width - 2, height)
		grp.SetColor(0x77000000)
		grp.RenderBox(x + 1, y + 1, width - 2, height)

		grp.SetColor(0xff989898)
		grp.RenderLine(x + width - 13, y + height - 1, 11, -11)
		grp.RenderLine(x + width - 9, y + height - 1, 7, -7)
		grp.RenderLine(x + width - 5, y + height - 1, 3, -3)

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True


LOCALE_PATH_FLAG = "d:/ymir work/ui/game/emote/%s.sub"


class ChatFilterWindow(ui.Window):
	WND_WIDTH = 63
	WND_HEIGHT = 184
	WND_TOOLTIP_WIDTH = 125
	X_PARSER_TOOLTIP = 35
	X_START = 42
	Y_SPACE = 18
	ADD_SPACE = 3

	def __init__(self):
		ui.Window.__init__(self)
		self.checkBoxDataList, self.imageBoxList = [], []
		self.__LoadWindow()

	def AddImageBox(self, x, y, fileName):
		return self.imageBoxList.append(ui.MakeImageBox(self, fileName, x, y))

	def GetGlobalPositionByChatParent(self, chatPosX, chatPosY):
		self.SetPosition(chatPosX - self.WND_WIDTH, chatPosY - self.Y_SPACE * (app.LANGUAGE_MAX_NUM + 1))
		self.UpdateRect()

	def __LoadWindow(self):
		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			self.checkBoxDataList.append(ui.ChatFilterCheckBoxLanguage(self, i, 0, (self.Y_SPACE * i),
																	   Event(self.OnSelectLanguage, i)))
			self.AddImageBox(self.X_START, (self.Y_SPACE * i) + self.ADD_SPACE,
							 LOCALE_PATH_FLAG % GetLanguageNameByID(i))
			if app.LANGUAGE_MAX_NUM == i:
				self.AddImageBox(self.X_START, (self.Y_SPACE * i) + self.ADD_SPACE, LOCALE_PATH_FLAG % "gl")

		self.SetSize(self.WND_WIDTH, self.Y_SPACE * (app.LANGUAGE_MAX_NUM + 1))
		self.Show()

	def OnSelectLanguage(self, state, bLanguageID):
		def Reload():
			map(ui.ChatFilterCheckBoxLanguage.UnSelect, self.checkBoxDataList)

		if bLanguageID == app.GetLocaleID():
			self.checkBoxDataList[app.LANGUAGE_MAX_NUM].SetState(1)
			return

		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if bLanguageID <> i:
				continue

			checkBox = self.checkBoxDataList[i]
			if self.checkBoxDataList[app.LANGUAGE_MAX_NUM].IsChecked():
				Reload()

			if bLanguageID == app.LANGUAGE_MAX_NUM:
				Reload()
				checkBox.SetState(1)

				self.OnCheckLanguages(True)
				return

		self.OnCheckLanguages()

	def OnCheckLanguages(self, bLanguageGlobal=False):
		baseDict = {i: 0 for i in xrange(app.LANGUAGE_MAX_NUM + 1)}

		if bLanguageGlobal:
			baseDict.update({app.LANGUAGE_MAX_NUM: 1})
			appInst.instance().GetNet().SendChatFilter(list(baseDict.values()))
			return

		for i in xrange(app.LANGUAGE_MAX_NUM):
			if self.checkBoxDataList[i].IsChecked():
				baseDict.update({i: 1})

		appInst.instance().GetNet().SendChatFilter(list(baseDict.values()))

	def OnUpdate(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()

		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if self.checkBoxDataList[i].IsIn():
				self.toolTipDialog = uiToolTip.ToolTip()
				self.toolTipDialog.SetPosition(mouseX + self.X_PARSER_TOOLTIP, mouseY)
				self.toolTipDialog.RectSize(self.WND_TOOLTIP_WIDTH, 0)
				if app.LANGUAGE_MAX_NUM == i:
					self.toolTipDialog.AppendDescription(localeInfo.CHAT_GLOBAL, None, 0xffffa879)
				else:
					self.toolTipDialog.AppendDescription(GetLangDisplayName(i) or "", None, 0xffffa879)

	def Destroy(self):
		self.checkBoxDataList = []
		self.imageBoxList = []

	def SetChatFilter(self, bLanguageID, bValue):
		if bLanguageID > len(self.checkBoxDataList) and bValue not in (1, 0):
			return

		if bValue == 1:
			self.checkBoxDataList[bLanguageID].Select()
		else:
			self.checkBoxDataList[bLanguageID].UnSelect()
