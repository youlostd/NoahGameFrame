# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import grp
import player
import wndMgr
from pygame.app import LoadGameData
from pygame.app import LoadLocaleData
from pygame.app import appInst

import mouseModule
import ui
import uiWhisper


class Window(ui.Window):
    def __init__(self, skinFileName=""):
        ui.Window.__init__(self, "UI")

        self.children = []
        self.childDict = {}
        self.TypeDict = {}

        self.fileName = skinFileName
        self.loader = ui.PythonScriptLoader()
        self.__LoadSkin(skinFileName)

        self.Show()

    def Show(self):
        ui.Window.Show(self)


        for children in self.GetChildrenByType("board_with_titlebar"):
            children.SetTop()
        for children in self.GetChildrenByType("titlebar"):
            children.SetTop()
        for children in self.GetChildrenByType("thinboard"):
            children.SetTop()

    def ClearDictionary(self):
        self.children = []
        self.childDict = {}
        self.TypeDict = {}

    def InsertChild(self, name, child, Type):
        self.childDict[name] = child
        if Type in self.TypeDict:
            self.TypeDict[Type].append(child)
        else:
            self.TypeDict[Type] = [child]

    def IsChild(self, name):
        return name in self.childDict

    def GetChild(self, name):
        return self.childDict[name]

    def GetChildrenByType(self, Type):
        return self.TypeDict.get(Type, [])

    def __LoadSkin(self, fileName):
        self.loader.LoadScriptFile(self, fileName)

    def Reload(self):
        self.ClearDictionary()
        self.__LoadSkin(self.fileName)


# wndMgr.SetOutlineFlag(True)

class App:
    def __init__(self, title="UI TEST"):
        app.SetHairColorEnable(1)
        app.SetArmorSpecularEnable(1)
        app.SetWeaponSpecularEnable(1)

        app.SetMouseHandler(mouseModule.mouseController)
        wndMgr.SetMouseHandler(mouseModule.mouseController)
        wndMgr.SetScreenSize(appInst.instance().GetSettings().GetWidth(), appInst.instance().GetSettings().GetHeight())

        app.Create(title, appInst.instance().GetSettings().GetWidth(), appInst.instance().GetSettings().GetHeight(), 1)

        app.SetCamera(1500.0, 30.0, 0.0, 180.0)

        # Gets and sets the floating-point control word
        # app.SetControlFP()

        if not mouseModule.mouseController.Create():
            return

        if not LoadGameData():
            return

        if not LoadLocaleData(app.GetLocalePath()):
            return

        for i in xrange(23):
            player.SetSkill(i + 180, i + 180)

        self.OnInit()

    def MainLoop(self):
        appInst.instance().Loop()

    def OnInit(self):
        pass


class TestWindow(Window):
    PAGE_GAME_OPTIONS = 0
    PAGE_VIDEO_OPTIONS = 1
    PAGE_AUDIO_OPTIONS = 2

    def __init__(self, skinFileName):
        Window.__init__(self, skinFileName)
        self.Initialize()

    def Initialize(self):
        print "init"

    def OnRender(self):
        grp.Clear()
        appInst.instance().RenderGame()
        grp.SetOmniLight()

        grp.PopState()
        grp.SetInterfaceRenderState()

    def OnUpdate(self):
        appInst.instance().UpdateGame()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            appInst.instance().Exit()
            return True

        if key == app.VK_F5:
            grp.ClearDepthBuffer()
            grp.Clear()
            reload(ui)
            reload(uiWhisper)
            self.Reload()
            self.Initialize()
            for children in self.GetChildrenByType("board_with_titlebar"):
                children.SetTop()
            for children in self.GetChildrenByType("titlebar"):
                children.SetTop()
            for children in self.GetChildrenByType("thinboard"):
                children.SetTop()


class TestApp(App):
    def OnInit(self):
        self.test = TestWindow("UIScript/CubeRenewalWindow.py")

TestApp().MainLoop()
