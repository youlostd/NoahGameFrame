import ui
import app
import localeInfo
import player
import logging

class PlayTimeWindow(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__Initialize()
		self.__Load()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __Initialize(self):
		self.titleBar = None
		self.playTimeImage = None
		self.playTimeNextImage = None

	def Destroy(self):
		self.ClearDictionary()

		self.__Initialize()
	
	def __Load_LoadScript(self, fileName):
		try:
			pyScriptLoader = ui.PythonScriptLoader()
			pyScriptLoader.LoadScriptFile(self, fileName)
		except:
			logging.exception("PlayTime.__Load_LoadScript")

	def __Load_BindObject(self):
		try:
			GetObject = self.GetChild
			self.titleBar = GetObject("titlebar")
			self.board = self.GetChild("board")

			self.myPlayTime = self.GetChild("my_playtime")
			self.myNextPlayTime = self.GetChild("my_next_playtime")
			self.playTimeImage = self.GetChild("time_Image")
			self.playTimeNextImage = self.GetChild("time_Next_Image")
		except:
			logging.exception("PlayTime.__Load_BindObject")

	def __Load(self):
		self.__Load_LoadScript("UIScript/PlayTime.py")
		self.__Load_BindObject()

		self.SetCenterPosition()

		self.titleBar.SetCloseEvent(self.Close)

	def OnUpdate(self):
		timelevel = player.GetPlayTime()
		reallevel = (timelevel / 24) / 13
		nextlevel = reallevel + 1

		difftime = nextlevel * 24 * 13
		difftime2 = difftime - timelevel 
		
		if reallevel == 0:
			reallevel = 1
			
		if nextlevel in [0, 1]:
			nextlevel = 2
			
		if reallevel >= 55:
			reallevel = 55
	
		if nextlevel >= 55:
			nextlevel = 55

		self.myPlayTime.SetText("Şuanki oyun süren: %d dakika" % timelevel)
		self.myNextPlayTime.SetText("Sonraki rütbe için kalan süre: %d dakika" % difftime2)
		
		self.playTimeImage.LoadImage("d:/ymir work/effect/etc/rutbeler/rutbe_%d.png" % reallevel)
		self.playTimeNextImage.LoadImage("d:/ymir work/effect/etc/rutbeler/rutbe_%d.png" % nextlevel)

	def Destroy(self):
		self.ClearDictionary()
		self.board = None

	def OnKeyDown(self, key):
		if key == app.VK_ESCAPE:
			self.Close()
			return True

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()
