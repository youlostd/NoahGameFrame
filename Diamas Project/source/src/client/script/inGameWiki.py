# -*- coding: utf-8 -*-
import logging

import app
import nonplayer
import wiki

import inGameWikiUI
import ui


class InGameWiki(ui.Window):
	def __init__(self):
		self.searchEdit = None
		super(InGameWiki, self).__init__()
		self.objList = {}
		self.windowHistory = []
		self.currSelected = 0
		self.selectedVnum = 0
		self.tooltipItem = None
		wiki.RegisterClass(self)
		self.SetWindowName("InGameWiki")

		self.BuildUI()
		self.SetCenterPosition()
		self.AddFlag("animated_board")
		self.AddFlag("focusable")

		self.Hide()

	def Destroy(self):
		wiki.UnregisterClass()

	def Show(self):
		super(InGameWiki, self).Show()
		wiki.ShowModelViewManager(True)
		self.SetTop()
		self.header.SetTop()
		self.titleBar.SetTop()

	def Hide(self):
		super(InGameWiki, self).Hide()
		wiki.ShowModelViewManager(False)
		if self.searchEdit:
			self.searchEdit.KillFocus()
			self.searchEdit.CloseListBox()

	def Close(self):
		self.Hide()

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True
		return False

	def OnPressEscapeKey(self):
		self.Close()
		#self.customPageWindow.LoadFile("testload.txt")
		return True

	def BINARY_LoadInfo(self, objID, vnum):
		if vnum in self.objList:
			for key, elem in self.objList[vnum].iteritems():
				if elem and elem():
					elem().NoticeMe()
		else:
			logging.debug("Wiki Missing objID %d in objList", objID)

	def BuildUI(self):
		inGameWikiUI.InitMainWindow(self)
		inGameWikiUI.BuildBaseMain(self)

	def SetItemToolTip(self, tooltipItemvalue):
		from _weakref import proxy
		self.tooltipItem = proxy(tooltipItemvalue)
		if self.mainWeaponWindow:
			self.mainWeaponWindow.SetItemToolTip(self.tooltipItem)
		if self.mainChestWindow:
			self.mainChestWindow.SetItemToolTip(self.tooltipItem)
		if self.mainBossWindow:
			self.mainBossWindow.SetItemToolTip(self.tooltipItem)
		if self.costumePageWindow:
			self.costumePageWindow.SetItemToolTip(self.tooltipItem)

	def OpenSpecialPage(self, oldWindow, vnum, isMonster=False):
		self.CloseBaseWindows()
		del self.windowHistory[self.currSelected + 1:]

		if oldWindow:
			del self.windowHistory[:]
			self.currSelected = 0
			self.windowHistory.append(oldWindow)
		if len(self.windowHistory) > 0:
			self.windowHistory[-1].Hide()

		newSpec = inGameWikiUI.SpecialPageWindow(vnum, isMonster)
		newSpec.AddFlag("attach")
		newSpec.SetParent(self)
		newSpec.SetSize(*inGameWikiUI.mainBoardSize)
		newSpec.SetPosition(*inGameWikiUI.mainBoardPos)
		newSpec.SetItemToolTip(self.tooltipItem)
		newSpec.EnableClipping()
		newSpec.Show()

		self.tooltipItem.Hide()

		self.windowHistory.append(newSpec)
		self.currSelected = self.windowHistory.index(newSpec)
		if self.currSelected > 0:
			self.prevButt.Enable()
		else:
			self.prevButt.Disable()
		self.nextButt.Disable()
		self.SetTop()
		self.header.SetTop()
		self.titleBar.SetTop()
		return True

	def OnPressNameEscapeKey(self):
		wnd = self.searchEdit
		if not wnd.IsShowCursor() or wnd.GetText() == "":
			self.OnPressEscapeKey()
		else:
			wnd.SetText("")

	def GetAutoCompleteEntries(self):
		if self.searchEdit.GetInput() == "":
			self.selectedVnum = 0
			return True
		import item
		data = item.GetAutoCompleEntries(self.searchEdit.GetInput()) + nonplayer.GetAutocompleteEntries(self.searchEdit.GetInput())
		if not data:
			self.searchEdit.CloseListBox()
			return False

		self.searchEdit.ClearItem()
		self.searchEdit.InsertItem((0, 0), "/")
		for item in data:
			try:
				self.searchEdit.InsertItem((item[0], item[2]), str(item[1]))
			except:
				pass
		self.searchEdit.OpenListBox()
		return True

	def StartSearch(self, index):
		if len(index) > 1:
			vnum = index[0]

			if vnum == 0:
				return False

			entityType = index[1]

			if entityType == 0:
				self.OpenSpecialPage(None, vnum, False)
			else:
				self.OpenSpecialPage(None, vnum, True)
		return True

	def GoToLanding(self):
		self.CloseBaseWindows()
		self.categ.NotifyCategorySelect(None)
		self.customPageWindow.LoadFile("landingpage.txt")

	def OnPressNextButton(self):
		if len(self.windowHistory) - 1 > self.currSelected:
			self.windowHistory[self.currSelected].Hide()
			self.currSelected += 1
			self.windowHistory[self.currSelected].OpenWindow()

			self.prevButt.Enable()
			if len(self.windowHistory) - 1 == self.currSelected:
				self.nextButt.Disable()

	def OnPressPrevButton(self):
		if self.currSelected > 0:
			self.windowHistory[self.currSelected].Hide()
			self.currSelected -= 1
			self.windowHistory[self.currSelected].OpenWindow()

			self.nextButt.Enable()
			if self.currSelected == 0:
				self.prevButt.Disable()

	def CloseBaseWindows(self):
		self.mainWeaponWindow.Hide()
		self.mainChestWindow.Hide()
		self.mainBossWindow.Hide()
		self.customPageWindow.Hide()
		self.costumePageWindow.Hide()

		for win in self.windowHistory:
			win.Hide()
		del self.windowHistory[:]
		self.prevButt.Disable()
		self.nextButt.Disable()
		self.currSelected = 0