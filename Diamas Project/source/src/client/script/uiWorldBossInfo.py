# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import calendar
import datetime

import app
from pygame.npc import npcManager

import localeInfo
import ui
from ui_event import Event


def next_day(given_date, weekday):
	day_shift = (weekday - given_date.weekday()) % 7
	return given_date + datetime.timedelta(days=day_shift)


class WorldBossInfo(ui.ScriptWindow):

	def __init__(
		self,
	):
		ui.ScriptWindow.__init__(self)
		self.AddFlag("animated_board")
		self.worldBossElements = []
		self.worldBossTexts = []
		self.worldBossInfo = []
		self.currentWorldBossIdx = 0
		self.loaded = False
		self.Initialize()

	def Show(self):
		if not self.loaded:
			self.Initialize()

		ui.ScriptWindow.Show(self)

	def Initialize(self):
		ui.PythonScriptLoader().LoadScriptFile(self, "UIScript/WorldBossInfo.py")

		self.GetChild("board").SetCloseEvent(self.Hide)
		self.infoScrollPanel = self.GetChild("WorldBossInfoScrollPaneContent")

		self.worldBossInfoText = self.GetChild("WorldBossInfo_SubText")

		for i in xrange(10):
			wbe = self.GetChild("WorldBoss_{}".format(i))
			wbe.SetMouseLeftButtonDownEvent(Event(self.OnWorldBossElementClick, i))
			wbe.RemoveFlag("not_pick")

			wbt = self.GetChild("WorldBoss_{}_SubText".format(i))

			self.worldBossTexts.append(wbt)
			self.worldBossElements.append(wbe)

		for wbe in self.worldBossElements:
			wbe.Hide()

		shownCount = 0
		for i, worldbossInfo in enumerate(npcManager().GetWorldBossInfos()):
			if worldbossInfo.bIsDaily:
				self.GetChild("WorldBoss_{}_Text".format(i)).SetText(
					localeInfo.WORLDBOSS_TEXT1.format(worldbossInfo.hour)
				)
			else:
				self.GetChild("WorldBoss_{}_Text".format(i)).SetText(
					localeInfo.WORLDBOSS_TEXT2.format(
						calendar.day_name[worldbossInfo.day], worldbossInfo.hour
					)
				)

			self.worldBossElements[i].Show()
			shownCount += 1
			self.worldBossInfo.append(worldbossInfo)

		self.GetChild("WorldBossScrollPaneContent").SetSize(357, shownCount * 60)
		self.OnWorldBossElementClick(0)
		self.loaded = True

	def OnWorldBossElementClick(self, idx):
		# for wbe in self.worldBossElements:
		#     wbe.SetColor(1.0, 1.0, 1.0)

		# self.worldBossElements[idx].SetColor(0.8, 0.9, 0.0)
		self.currentWorldBossIdx = idx

		self.worldBossInfoText.SetMultiLine()

		wbText = "{}: |Hmobname:{}|h\n".format(
			localeInfo.WORLD_BOSS_INFO_NAME, str(self.worldBossInfo[idx].boss)
		)
		wbText += "\n{}\n".format(localeInfo.WORLD_BOSS_INFO_SPAWN_LOCATIONS)

		for info in self.worldBossInfo[idx].mapIndexList:
			wbText += "\t * {}".format(localeInfo.GetMapNameByIndex(info.mapIndex))

		wbText += "\n\n{}\n".format(localeInfo.WORLD_BOSS_INFO_SPAWN_TIME)

		if self.worldBossInfo[idx].bIsDaily:
			wbText += "\t * {} {}\n".format(
				localeInfo.WORLD_BOSS_DAILY_AT, self.worldBossInfo[idx].hour
			)
		else:
			wbText += "\t* {} {} {}\n".format(
				calendar.day_name[self.worldBossInfo[idx].day],
				localeInfo.WORLD_BOSS_AT,
				self.worldBossInfo[idx].hour,
			)

		self.worldBossInfoText.SetText(wbText)

		self.infoScrollPanel.SetSize(248, self.worldBossInfoText.GetHeight())

	def OnUpdate(self):
		for idx, wb in enumerate(self.worldBossInfo):
			text = self.worldBossTexts[idx]
			if wb.bIsDaily:
				dt = datetime.datetime.now().replace(hour=wb.hour, minute=0, second=0, microsecond=0)
				diff = dt - datetime.datetime.now()
	
				if diff.total_seconds() < 0:
					dt = datetime.datetime.now().replace(hour=wb.hour, minute=0, second=0, microsecond=0) + datetime.timedelta(days=1)
					diff = dt - datetime.datetime.now()
				text.SetText(localeInfo.WORLD_BOSS_SPAWNS_IN.format(localeInfo.SecondToDHMS(diff.total_seconds())))
			else:
				n = next_day(datetime.datetime.now(), wb.day).replace(hour=wb.hour, second=0, minute=0, microsecond=0)
				diff = n - datetime.datetime.now()
	
				if diff.total_seconds() < 0:
					dt = datetime.datetime.now().replace(hour=wb.hour, second=0, minute=0, microsecond=0) + datetime.timedelta(days=7)
					diff = dt - datetime.datetime.now()
	
				text.SetText(localeInfo.WORLD_BOSS_SPAWNS_IN.format(localeInfo.SecondToDHMS(diff.total_seconds())))

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Hide()
			return True

		return False
