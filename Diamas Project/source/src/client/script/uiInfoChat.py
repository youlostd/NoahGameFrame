# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import grp

import ui


class InfoChat(ui.Window):
	def __init__(self, parent, x = 0, y = 0):
		ui.Window.__init__(self)
		self.texts = []
		self.parent = parent
		self.SpaceBet = 14
		self.maxY = 0
		self.x = x
		self.y = y
		self.ColorValue = 0xffffc539
		self.SetTop()
		
		self.showSpecialChat()

	def showSpecialChat(self):
		for text in self.texts:
			text.Show()
		self.Show()

	def OnRender(self):
		if len(self.texts) > 0:
			x, y = self.texts[0].GetGlobalPosition()
			w, h = (self.texts[0].GetWidth(), self.texts[0].GetHeight())
			grp.SetColor(grp.GenerateColor(0.0, 0.0, 0.0, 0.3))
			grp.RenderBar(x-2, y+h-12, 200, (h+2)*len(self.texts)+4)
			
	def AddSpecialChatLine(self, text):
		for i in xrange(len(self.texts)):
			if len(self.texts) == 6 and i == 0:
				self.texts[i].Hide()
			x, y = self.texts[i].GetLocalPosition()
			self.texts[i].SetPosition(x, y-self.SpaceBet)

		if len(self.texts) > 5:
			self.texts.pop(0)

		textLine = ui.TextLine()
		textLine.SetParent(self.parent)
		textLine.SetPosition(self.x, self.y)
		textLine.SetPackedFontColor(self.ColorValue)
		textLine.SetHorizontalAlignLeft()
		textLine.SetOutline(True)
		textLine.SetParent(self)
		textLine.SetText(text)
		textLine.Show()
		textLine.spawnTime = app.GetGlobalTime()
		self.texts.append(textLine)

	def ClearAll(self):
		self.Hide()
		self.texts = {}
					
	def OnUpdate(self):
		if len(self.texts) > 0:
			self.texts = [text for text in self.texts if app.GetGlobalTime() < text.spawnTime + 4000]

