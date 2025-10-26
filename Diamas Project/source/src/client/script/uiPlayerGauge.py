# -*- coding: utf-8 -*-
import player
import textTail
from pygame.player import playerInst

import ui

class PlayerGauge(ui.Gauge):
	def __init__(self, parent):
		ui.Gauge.__init__(self)
		self.SetParent(parent)
		self.AddFlag("not_pick")
		self.MakeGauge(100, "red")
		# ui.Gauge.SetScale(self, 1.0, 0.3)

		self.curHP = 0
		self.maxHP = 0
		self.hpPer = 0

		self.showAlways = False

	def Hide(self):
		self.SetPosition(-100, -100)
		ui.Gauge.Hide(self)

	def OnUpdate(self):
		playerIndex = playerInst().GetMainCharacterIndex()

		(x, y, z)=textTail.GetPosition(playerIndex)

		isChat = textTail.IsChat(playerIndex)
		ui.Gauge.SetPosition(self, int(x - self.GetWidth()/2), int(y + 5) + isChat*17)

	def RefreshGauge(self):

		self.curHP = playerInst().GetPoint(player.HP)
		self.maxHP = playerInst().GetPoint(player.MAX_HP)
		if self.maxHP < 1:
			return

		self.hpPer = self.curHP * 100 / self.maxHP
		self.SetPercentage(self.hpPer, 100)

		if self.showAlways:
			self.Show()
		else:
			if self.IsShow():
				if self.curHP > self.maxHP / 2:
					self.Hide()

			else:
				if self.curHP < self.maxHP / 2:
					self.OnUpdate()
					self.Show()

	def EnableShowAlways(self):
		self.showAlways = True
		self.RefreshGauge()

	def DisableShowAlways(self):
		self.showAlways = False
		self.RefreshGauge()
