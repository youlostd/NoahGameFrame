# -*- coding: utf-8 -*-


import logging
import os
import shlex
import stat

import app
import background
import chat
import chr
import chrmgr
import grp
import player
import snd
from pygame.app import appInst
from pygame.player import playerInst

import localeInfo
import ui
import uiScriptLocale


class Console(object):
	def __init__(self, output):
		self.dirNameList = []
		self.fileNameList = []

		self.output = output
		self.curPathName = "D:\\Ymir Work\\"
		self.collision = 0

		self.bgPartDict = {
			"terrain": background.PART_TERRAIN,
			"object": background.PART_OBJECT,
			"cloud": background.PART_CLOUD,
			"tree": background.PART_TREE,
			"water": background.PART_WATER,
			"sky": background.PART_SKY,
		}
		self.scriptWindow = None

		self.bgSortDict = {
			"distance": background.DISTANCE_SORT,
			"texture": background.TEXTURE_SORT,
		}

		self.game = None

	def Close(self):
		self.output = 0

	def BindGameClass(self, game):
		self.game = game

	def Exit(self):
		"""Exit Program"""
		appInst.instance().Exit()

	def ReloadLocale(self):
		"""Reload Locale"""
		reload(localeInfo)
		reload(uiScriptLocale)
		self.Print("RELOAD LOCALE")

	def ReloadDevel(self):
		"""ReloadDevel"""
		import uiInventory
		reload(uiInventory)

		self.Print("ReloadInventory")

	def ShowPerformanceInfo(self):
		"""Shows Performance Info"""
		self.Print(app.GetInfo(app.INFO_ACTOR))
		self.Print(app.GetInfo(app.INFO_ITEM))
		self.Print(app.GetInfo(app.INFO_EFFECT))
		self.Print(app.GetInfo(app.INFO_TEXTTAIL))

	def Disconnect(self):
		"""Disconnect by sending unknown packet"""
		pass


	def ToggleActorDirectionLine(self):
		"""Shows character's direction line"""
		chrmgr.ToggleDirectionLine()

	def RegisterCharacterEffect(self, effectType, effectFileName):
		"""Set character's effect state (state number, bone name, effect filename/number)"""
		if effectFileName.isdigit():
			effectFileName = self.GetFileName(int(effectFileName))

		effectFullPathFileName = self.GetFullPathName(effectFileName)
		chrmgr.RegisterEffect(chrmgr.EFFECT_STATE+int(effectType), "", effectFullPathFileName)
		self.Print("effect state %d: %s" % (int(effectType), effectFullPathFileName))

	def SetCharacterAffect(self, affectType, isVisible):
		"""Set character's affect (number, visiblity 1|0)"""
		chrmgr.SetAffect(-1, int(affectType), int(isVisible))
		self.Print("affect set %d: %d" % (int(affectType), int(isVisible)))

	def SetCharacterEmoticon(self, EmoticonNum):
		"""Show emoticon (number)"""
		chrmgr.SetEmoticon(-1, int(EmoticonNum))
		self.Print("Emoticon %d" % (int(EmoticonNum)))

	def ShowPickedCharacterInfo(self):
		"""Show information of picked character"""
		vid=chrmgr.GetPickedVID()
		info=chrmgr.GetVIDInfo(vid)
		self.Print(info)

	def ShowCharacterInfo(self, arg):
		"""Show information of character (vid)"""
		if arg.isdigit():
			vid=int(arg)
		else:
			vid=0

		info=chrmgr.GetVIDInfo(vid)
		self.Print(info)

	def SetWeaponTraceTexture(self, arg):
		"""Sets sword afterimage texture (filename)"""
		if arg.isdigit():
			textureFileName = self.GetFileName(int(arg))
		else:
			textureFileName = arg

		chr.WeaponTraceSetTexture(textureFileName)

	def SetAutoCameraRotationSpeed(self, arg):
		"""Auto camera rotation speed (angle per sec)"""
		spd=float(arg)
		player.SetAutoCameraRotationSpeed(spd)

		self.Print("Auto camera rotation speed: %f" % spd)

	def SetWeaponTraceMode(self, *mode):
		"""Sword afterimage mode (0 = Use alpha, 1 = Use texture)"""
		if mode and mode[0].isdigit() and int(mode[0]):
			chr.WeaponTraceUseTexture()
		else:
			chr.WeaponTraceUseAlpha()

	def SetCollision(self, *mode):
		"""Show collison objects (0 | 1)"""
		if mode and mode[0].isdigit():
			self.collision = int(mode[0])
		else:
			self.collision = not self.collision

	def SetMovingSpeed(self, arg):
		"""Set walking speed"""
		chrmgr.SetMovingSpeed(int(arg))

	def SetMusicVolume(self, arg):
		"""Set BGM volumn (0 ~ 1.0)"""
		snd.SetMusicVolume(float(arg))

	def SetSoundVolume(self, arg):
		"""Set Effect sound volumn (0 ~ 5)"""
		snd.SetSoundVolume(int(arg))

	def SetSoundScale(self, arg):
		"""3D Sound scale (default : 200)"""
		snd.SetSoundScale(int(arg))

	def SetAmbienceSoundScale(self, arg):
		"""3D Ambience sound scale (default : 200)"""
		snd.SetAmbienceSoundScale(int(arg))

	def SetCoolTime(self):
		"""Cooltime toggle"""
		flag = player.ToggleCoolTime()
		if flag:
			self.Print("Cooltime will be applied")
		else:
			self.Print("Cooltime disabled")

	def SetLevelLimit(self):
		"""Sets Level limit"""
		flag = player.ToggleLevelLimit()
		if flag:
			self.Print("Level limit will be applied")
		else:
			self.Print("Level limit disabled")

	def ShowCursor(self):
		app.ShowCursor()

	def HideCursor(self):
		app.HideCursor()

	def Print(self, msg):
		self.output.Print(msg)

	def RefreshPath(self):
		self.SetPath(self.curPathName)

	def	SetPath(self, newPathName):
		if '\\'!=newPathName[-1]:
			newPathName += '\\'

		if os.access(newPathName, os.R_OK)==0:
			self.Print(newPathName+" Cannot find path")
			return

		nameList=os.listdir(newPathName)

		dirNameList=[]
		fileNameList=[]
		for sName in nameList:
			mode=os.stat(newPathName+sName)[stat.ST_MODE]
			if stat.S_ISDIR(mode):
				dirNameList.append(sName)
			else:
				fileNameList.append(sName)

		self.curPathName=newPathName
		self.dirNameList=dirNameList
		self.fileNameList=fileNameList

		self.ShowPath()

	def GetPath(self):
		return self.curPathName

	def GetDirName(self, iDir):
		if len(self.dirNameList)>iDir:
			return self.dirNameList[iDir]
		else:
			return ""

	def GetFileName(self, iFile):
		if len(self.fileNameList)>iFile:
			return self.fileNameList[iFile]
		else:
			return ""

	def	MoveParentPath(self):
		"""Go to Parent directory"""
		newPathName=self.GetPath()

		lastPos=-1
		if '\\'==newPathName[-1]:
			lastPos=-2

		lastPos=newPathName.rfind('\\', 0,	lastPos)
		if 0>lastPos:
			self.Print("Here is root")
			return

		newPathName=newPathName[0:lastPos]+'\\'

		self.SetPath(newPathName)

	def GetFullPathName(self, sPathName):
		sParentPathName=self.GetPath()

		if '\\'!=sParentPathName[-1]:
			sParentPathName += '\\'

		return sParentPathName+sPathName

	def	MoveChildPath(self, directory):
		"""[Directory name/number] change to child directory"""
		if ""==directory:
			self.Print("You need directory name or number")
			return

		if directory.isdigit():
			directory=self.GetDirName(int(directory))

		newPathName=self.GetFullPathName(directory)

		self.SetPath(newPathName)

	def SetHitEffect(self, arg):
		"""Sets hit effect"""
		if arg.isdigit():
			effectFileName = self.GetFileName(int(arg))
		else:
			effectFileName = arg

		effectFullPathFileName=self.GetFullPathName(effectFileName)
		chrmgr.RegisterEffect(chrmgr.EFFECT_HIT, "", effectFullPathFileName)

		self.Print("Hit effect: "+effectFullPathFileName)

	def SetStunEffect(self, arg):
		"""Sets stun effect"""
		if arg.isdigit():
			effectFileName = self.GetFileName(int(arg))
		else:
			effectFileName = arg

		effectFullPathFileName = self.GetFullPathName(effectFileName)
		chrmgr.RegisterEffect(chrmgr.EFFECT_STUN, "Bip01 Head", effectFullPathFileName)

		self.Print("Stun effect: "+effectFullPathFileName)

	def SetDustEffect(self, arg):
		"""Sets dirt effect"""
		if arg.isdigit():
			effectFileName = self.GetFileName(int(arg))
		else:
			effectFileName = arg

		effectFullPathFileName = self.GetFullPathName(effectFileName)
		chrmgr.RegisterEffect(chrmgr.EFFECT_DUST, "", effectFullPathFileName)

		self.Print("Dirt effect: "+effectFullPathFileName)

	def SetDustGap(self, arg):
		"""Sets dirt effect gap"""
		if arg.isdigit() == 0:
			self.Print("Dirt effect gap argument must be a number")
		else:
			gap = int(arg)
			chrmgr.SetDustGap(gap)
			self.Print("Dirt effect gap: %d" % gap)

	def ShowBackgroundPart(self, arg):
		"""Show background part"""
		try:
			background.SetVisiblePart(self.bgPartDict[arg], 1)
		except KeyError:
			self.Print("Unknown part: ", arg)
		except:
			raise

	def HideBackgroundPart(self, arg):
		"""Hide background part"""
		try:
			background.SetVisiblePart(self.bgPartDict[arg], 0)
		except KeyError:
			self.Print("Unknown part: ", arg)
		except:
			raise

	def SetShadowLevel(self, arg):
		"""Shadow level setting (0~5)"""
		if arg.isdigit() == 0:
			self.Print("Shadow setting must be a number")
		else:
			level = int(arg)

			if level < 0 or level > 5:
				self.Print("Shadow level range is 0~5")
			else:
				background.SetShadowLevel(level)
				self.Print("Shadow level: %d" % level)

	def SetSplatLimit(self, arg):
		"""Splat number limit"""
		if arg.isdigit() == 0:
			self.Print("Splat number limit must be a number")
		else:
			limit = int(arg)

			background.SetSplatLimit(limit)
			self.Print("Splat number limit: %d" % limit)

	def SelectViewDistanceNum(self, arg):
		"""View distance (0~4)"""
		if arg.isdigit() == 0:
			self.Print("Enter between 0 to 4")
		else:
			settingnum = int(arg)

			if settingnum < 0 or settingnum > 4:
				self.Print("View distance range is 0 to 4")

			else:
				background.SelectViewDistanceNum(settingnum)
				self.Print("View distance: %d" % settingnum)

	def SetBGLoading(self, bBGLoading):
		"""Background loading (0, 1)"""
		bResultBoolean = int(bBGLoading)

		if bResultBoolean != 0 and bResultBoolean != 1:
			self.Print("Enter 0 or 1")
		else:
			background.SetBGLoading(bResultBoolean)
			self.Print("Background loading: %d" % bResultBoolean)

	def SetTerrainRenderSort(self, arg):
		"""Terrain rendering type"""
		try:
			background.SetRenderSort(self.bgSortDict[arg])
		except KeyError:
			self.Print("Unknown part: ", arg)
		except:
			raise

	def SetTransparentTree(self, arg):
		"""Tree's leaf transparency"""
		bTransparent = int(arg)
		try:
			background.SetTransparentTree(bTransparent)
		except KeyError:
			self.Print("must be 0 or 1: ", bTransparent)
		except:
			raise



	def ToggleDebugInfo(self):
		"""Show DebugInfo"""
		appInst.instance().GetNet().ToggleGameDebugInfo()

	def ShowPath(self):
		"""Show current path"""
		self.Print("Current path is "+self.GetPath())

	def	ShowList(self):
		"""Show current path's list"""
		self.ShowDirList()
		self.ShowFileList()

	def	ShowDirList(self):
		"""Show current path's child directories"""
		self.Print(self.GetPath()+" directories:")
		self.ShowNameList(self.dirNameList)
		self.Print("")

	def	ShowFileList(self, Filter=None):
		"""Show current path's files (extension filter)"""
		if not Filter:
			Filter = []
		self.Print(self.GetPath()+" files:")
		self.ShowNameList(self.fileNameList, Filter)
		self.Print("")

	def	ShowEffectList(self):
		"""Show current path's effect"""
		self.Print(self.GetPath()+" effects:")
		self.ShowNameList(self.fileNameList, ["mse"])
		self.Print("")

	def ShowWeb(self):
		"""ShowWeb"""
		pass

	def ShowUI(self):
		"""Shows UI"""
		self.game.CheckGameButton()
		self.game.interface.ShowDefaultWindows()
		self.game.interface.wndChat.Show()

	def HideUI(self):
		"""Hides UI"""
		self.game.interface.HideAllWindows()
		self.game.interface.wndChat.Hide()

	def SetCameraSpeed(self, percentage):
		"""Sets camera speed (Default = 100)"""
		app.SetCameraSpeed(int(percentage))

	def SaveCameraSetting(self, filename):
		"""Save current camera attributes (filename)"""
		app.SaveCameraSetting(filename)

	def LoadCameraSetting(self, filename):
		"""Load camera attributes (filename)"""
		if not app.LoadCameraSetting(filename):
			self.Print("Could not load camera attribute")

	def SetDefaultCamera(self):
		"""Return to default camera attribute"""
		app.SetDefaultCamera()

	def ShowClock(self, second):
		self.game.ShowClock(int(second))

	def HideClock(self):
		self.game.HideClock()

	def ShowNotice(self):
		app.SetVisibleNotice(True)
		self.Print("Show Notice")

	def HideNotice(self):
		app.SetVisibleNotice(False)
		self.Print("Hide Notice")

	def SetSight(self, sightRange):
		"""Set Force Sight Range"""
		app.SetSightRange(int(sightRange))

	def SetComboType(self, comboType):
		chr.testSetComboType(int(comboType))

	def SetCharacterRenderModeTypeAdd(self, color):
		vid = playerInst().GetTargetVID()
		chr.testSetAddRenderMode(vid, int(color,16))

	def SetCharacterRenderModeTypeMod(self, color):
		vid = playerInst().GetTargetVID()
		chr.testSetModulateRenderMode(vid, int(color,16))

	def SetCharacterRenderModeTypeAddRGB(self, r, g, b):
		vid = playerInst().GetTargetVID()
		chr.testSetAddRenderModeRGB(vid, float(r)/255.0, float(g)/255.0, float(b)/255.0)

	def SetCharacterRenderModeTypeModRGB(self, r, g, b):
		vid = playerInst().GetTargetVID()
		chr.testSetModulateRenderModeRGB(vid, float(r)/255.0, float(g)/255.0, float(b)/255.0)

	def SetCharacterRenderModeSpecular(self, alpha):
		vid = playerInst().GetTargetVID()
		chr.testSetSpecularRenderMode(vid, float(alpha))

	def RestoreCharacterRenderModeType(self):
		vid = playerInst().GetTargetVID()
		chr.testRestoreRenderMode(vid)

	def TestCounting(self, base):
		base = int(base)
		for i in xrange(100):
			chat.AppendChat(CHAT_TYPE_INFO, str(base + i))

	def OpenScriptWindow(self, filename):
		self.scriptWindow = ui.ScriptWindow()

		try:
			ui.PythonScriptLoader().LoadScriptFile(self.scriptWindow, filename)
			self.scriptWindow.SetCenterPosition()
			self.scriptWindow.Show()
		except Exception:
			logging.exception("Failed to load %s", filename)
			self.scriptWindow = None

	def CloseScriptWindow(self):
		if not self.scriptWindow:
			self.Print("No window")
			return

		self.scriptWindow.Hide()
		self.scriptWindow.ClearDictionary()
		self.scriptWindow = None

	def RecvServerCommand(self, cmdline):
		"""Allows faking server-commands"""
		self.game.BINARY_ServerCommand_Run(cmdline)

	def	ShowNameList(self, lstsName, lstsFilter=None):
		if len(lstsName)==0:
			self.Print("None")
			return

		if lstsFilter:
			isFilter=1
		else:
			isFilter=0

		sLine=""
		iCol=0

		iName=0
		for sName in lstsName:
			if isFilter:
				iDotPos=sName.rfind('.', 0, -1)
				if iDotPos<0:
					iName += 1
					continue

				if (sName[iDotPos+1:] in lstsFilter)==0:
					iName += 1
					continue

			sLine += "%3d %-15s " % (iName, sName)

			if iCol>4:
				iCol=0
				self.Print(sLine)
				sLine=""

			iCol += 1

			iName += 1

		if ""!=sLine:
			self.Print(sLine)

class ConsoleEditLine(ui.EditLine):
	def __init__(self):
		ui.EditLine.__init__(self)

		self.lastSentenceStack = []
		self.lastSentencePos = 0

	def OnKeyDown(self, key):
		# LAST_SENTENCE_STACK
		if app.VK_UP == key:
			self.__PrevLastSentenceStack()
			return True

		if app.VK_DOWN == key:
			self.__NextLastSentenceStack()
			return True
		# END_OF_LAST_SENTENCE_STACK

		text = self.GetText()

		if app.VK_RETURN == key:
			if text:
				self.__PushLastSentenceStack(text)
				self.eventReturn(text)
				self.SetText("")
			else:
				self.eventEscape()
			return True

		if key == app.VK_ESCAPE:
			self.SetText("")
			self.eventEscape()
			return True

		return ui.EditLine.OnKeyDown(self, key)

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
			self.lastSentenceStack.pop()

		self.lastSentenceStack.append(text)

class ConsoleWindow(ui.Window):
	BACK_GROUND_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 0.5)
	EDIT_LINE_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 1.0)
	BUTTON_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 1.0)
	HEIGHT = 200
	LINE_STEP = 15
	MAX_LINE_COUNT = 50

	class ResizingButton(ui.DragButton):
		BUTTON_NORMAL_COLOR = grp.GenerateColor(0.3320, 0.2929, 0.2578, 1.0)
		BUTTON_OVER_COLOR = grp.GenerateColor(0.5320, 0.4929, 0.4578, 1.0)
		BUTTON_LIGHT_OUTLINE_COLOR = grp.GenerateColor(0.6666, 0.6509, 0.6313, 1.0)
		BUTTON_DARK_OUTLINE_COLOR = grp.GenerateColor(0.1647, 0.1450, 0.1294, 1.0)

		def __init__(self):
			ui.DragButton.__init__(self)
			#self.AddFlag("limit")
			self.AddFlag("restrict_x")

	def __init__(self):
		ui.Window.__init__(self)

		self.TextList = []
		self.game = None
		self.Console = Console(self)

		self.ResizingButton = self.ResizingButton()
		self.ResizingButton.SetParent(self)
		self.ResizingButton.SetSize(15, 15)
		self.ResizingButton.SetPosition(100, 100)
		self.ResizingButton.Show()

		self.EditLine = ConsoleEditLine()
		self.EditLine.SetParent(self)
		self.EditLine.SetMax(100)
		self.EditLine.SetFontName(localeInfo.UI_DEF_FONT)
		self.EditLine.SetText("")
		self.EditLine.Show()

		self.ResizingButton.SetMoveEvent(self.UpdatePosition)
		self.EditLine.SetReturnEvent(self.ProcessCommand)
		self.EditLine.SetEscapeEvent(self.CloseWindow)

		self.UpdatePosition()

		self.functionDict = {}
		self.InitFunction()

	def BindGameClass(self, game):
		self.Console.BindGameClass(game)

	def Close(self):
		self.Console.Close()
		self.ResizingButton = None
		self.EditLine = None

	def SetConsoleSize(self, width, height):
		self.ResizingButton.SetPosition(width-20, height-20)
		self.UpdatePosition()

	def OnRender(self):
		grp.SetColor(self.BACK_GROUND_COLOR)
		grp.RenderBar(self.gx, self.gy, self.width, self.height)

		grp.SetColor(ConsoleWindow.EDIT_LINE_COLOR)
		grp.RenderBar(self.gxEditLine-2, self.gyEditLine-3, self.width - 40, 17)

		grp.SetColor(ConsoleWindow.BUTTON_COLOR)
		grp.RenderBar(self.gxButton-2, self.gyButton-3, self.widthButton, self.heightButton)

	def UpdatePosition(self):
		self.width = self.GetWidth()
		self.height = self.GetHeight()
		self.widthButton = self.ResizingButton.GetWidth()
		self.heightButton = self.ResizingButton.GetHeight()
		(self.gx, self.gy) = self.GetGlobalPosition()
		(self.gxButton, self.gyButton) = self.ResizingButton.GetGlobalPosition()

		self.SetSize(self.gxButton - self.gx + 20, self.gyButton - self.gy + 20)

		self.EditLine.SetSize(self.width-30, 16)
		self.EditLine.SetPosition(7, self.height-20)
		(self.gxEditLine, self.gyEditLine) = self.EditLine.GetGlobalPosition()

		yPosition = (self.height-20) - self.LINE_STEP
		ItemCount = len(self.TextList)

		for i in xrange(ItemCount):
			TextLine = self.TextList[ItemCount-i-1]

			TextLine.SetPosition(10, yPosition)
			yPosition -= self.LINE_STEP

			if yPosition < 0:
				TextLine.Hide()
			else:
				TextLine.Show()

	def OpenWindow(self):
		# TODO(tim): Delay the focus change. Otherwise we end up receiving
		# an OnChar event for the key that opened the console.
		#self.EditLine.SetFocus()
		self.Show()
		self.Console.RefreshPath()

	def CloseWindow(self):
		self.Hide()

	## NOTE : ì´ê³³ì—ì„œ Commandë¥¼ ì²˜ë¦¬í•©ë‹ˆë‹¤ - [levites]
	def ProcessCommand(self, text):
		if '/' == text[0]:
			appInst.instance().GetNet().SendChatPacket(text)
			return

		self.Console.Print(">> " + text)

		args = shlex.split(text)
		if not args:
			return

		cmd = args[0]

		if cmd in self.functionDict:
			try:
				self.functionDict[cmd](self.Console, *args[1:])
			except Exception as e:
				self.Console.Print(str(e))
		else:
			if cmd == 'help' or cmd == 'h':
				try:
					k = args[1]
					v = self.functionDict[k]
					argcount = v.im_func.func_code.co_argcount - 1 # -1 for self
					if v.im_func.func_code.co_flags & 4:
						argcount+=1
					argnames = v.im_func.func_code.co_varnames[1:argcount+1]
					if argcount:
						self.Console.Print("%s(%s) : %s" % (k,argcount, v.__doc__))
						self.Console.Print("   arg : %s" % argnames)
					else:
						self.Console.Print("%s : %s" % (k,v.__doc__))
				except:
					for k,v in self.functionDict.iteritems():
						argcount = v.im_func.func_code.co_argcount - 1 # -1 for self
						if v.im_func.func_code.co_flags & 4:
							argcount+=1
						if argcount:
							self.Console.Print("%s(%s) : %s" % (k,argcount, v.__doc__))
						else:
							self.Console.Print("%s : %s" % (k,v.__doc__))
					self.Console.Print("? : All commands list")
					self.Console.Print("h : Help for all commands")
					self.Console.Print("h blah : Help for blah")

				self.Console.Print("")
			elif cmd == '?':
				self.Console.ShowNameList(sorted(self.functionDict.keys()))
			pass

	def InitFunction(self):
		#self.AddFunction("help",		Console.PrintHelp)
		#self.AddFunction("?",			Console.PrintHelp)

		self.AddFunction("exit",		Console.Exit)
		self.AddFunction("mvol",		Console.SetMusicVolume)
		self.AddFunction("svol",		Console.SetSoundVolume)
		self.AddFunction("snds",		Console.SetSoundScale)
		self.AddFunction("asnds",		Console.SetAmbienceSoundScale)
		self.AddFunction("mspd",		Console.SetMovingSpeed)
		self.AddFunction("pwd",			Console.ShowPath)
		self.AddFunction("ls", 			Console.ShowList)
		self.AddFunction("shadow",		Console.SetShadowLevel)
		self.AddFunction("splat",		Console.SetSplatLimit)
		self.AddFunction("distance", 	Console.SelectViewDistanceNum)
		self.AddFunction("bgloading",	Console.SetBGLoading)
		self.AddFunction("terrainrender",	Console.SetTerrainRenderSort)
		self.AddFunction("transtree",		Console.SetTransparentTree)
		self.AddFunction("stune",		Console.SetStunEffect)
		self.AddFunction("duste",		Console.SetDustEffect)
		self.AddFunction("dustt",		Console.SetDustGap)
		self.AddFunction("hite",		Console.SetHitEffect)
		self.AddFunction("cd",			Console.MoveChildPath)
		self.AddFunction("up",			Console.MoveParentPath)
		self.AddFunction("lsd",			Console.ShowDirList)
		self.AddFunction("lsf",			Console.ShowFileList)
		self.AddFunction("lse",			Console.ShowEffectList)
		self.AddFunction("show",		Console.ShowBackgroundPart)
		self.AddFunction("hide",		Console.HideBackgroundPart)
		self.AddFunction("debuginfo",	Console.ToggleDebugInfo)

		self.AddFunction("collision",	Console.SetCollision)
		self.AddFunction("colli",	Console.SetCollision)

		#self.AddFunction("wt",	Console.SetWeaponTrace)
		self.AddFunction("wtt",	Console.SetWeaponTraceTexture)
		self.AddFunction("wtm",	Console.SetWeaponTraceMode)

		self.AddFunction("disconnect",	Console.Disconnect)
		self.AddFunction("autorot", Console.SetAutoCameraRotationSpeed)

		self.AddFunction("dirline", Console.ToggleActorDirectionLine)
		self.AddFunction("pickc", Console.ShowPickedCharacterInfo)
		self.AddFunction("infoc", Console.ShowCharacterInfo)
		self.AddFunction("regchre", Console.RegisterCharacterEffect)
		self.AddFunction("setchra", Console.SetCharacterAffect)
		self.AddFunction("emoticon", Console.SetCharacterEmoticon)
		self.AddFunction("perfinfo", Console.ShowPerformanceInfo)
		self.AddFunction("reload_locale", Console.ReloadLocale)
		self.AddFunction("re", Console.ReloadDevel)
		self.AddFunction("cooltime", Console.SetCoolTime)
		self.AddFunction("levellimit", Console.SetLevelLimit)
		self.AddFunction("showcursor", Console.ShowCursor)
		self.AddFunction("hidecursor", Console.HideCursor)

		self.AddFunction("web", Console.ShowWeb)

		self.AddFunction("showui", Console.ShowUI)
		self.AddFunction("hideui", Console.HideUI)

		self.AddFunction("setcspd", Console.SetCameraSpeed)
		self.AddFunction("savecmr", Console.SaveCameraSetting)
		self.AddFunction("loadcmr", Console.LoadCameraSetting)
		self.AddFunction("setdefcmr", Console.SetDefaultCamera)

		self.AddFunction("showclock", Console.ShowClock)
		self.AddFunction("hideclock", Console.HideClock)

		self.AddFunction("setsight", Console.SetSight)

		self.AddFunction("setcombotype", Console.SetComboType)

		self.AddFunction("shownotice", Console.ShowNotice)
		self.AddFunction("hidenotice", Console.HideNotice)

		self.AddFunction("setrmadd", Console.SetCharacterRenderModeTypeAdd)
		self.AddFunction("setrmmod", Console.SetCharacterRenderModeTypeMod)
		self.AddFunction("setrmaddrgb", Console.SetCharacterRenderModeTypeAddRGB)
		self.AddFunction("setrmmodrgb", Console.SetCharacterRenderModeTypeModRGB)
		self.AddFunction("setspec", Console.SetCharacterRenderModeSpecular)
		self.AddFunction("restorerm", Console.RestoreCharacterRenderModeType)

		self.AddFunction("testcount", Console.TestCounting)

		self.AddFunction("openwnd", Console.OpenScriptWindow)
		self.AddFunction("closewnd", Console.CloseScriptWindow)

		self.AddFunction("srvcmd", Console.RecvServerCommand)

	def AddFunction(self, cmd, func):
		self.functionDict[cmd] = func

	def Print(self, text):
		TextLine = ui.TextLine()
		TextLine.SetParent(self)
		TextLine.SetFontName(localeInfo.UI_DEF_FONT)
		TextLine.Show()
		TextLine.SetText(text)
		self.TextList.append(TextLine)

		count = len(self.TextList)
		if count > self.MAX_LINE_COUNT:
			for i in xrange(count - self.MAX_LINE_COUNT):
				del self.TextList[0]

		self.UpdatePosition()
