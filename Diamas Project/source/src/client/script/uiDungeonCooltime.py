import ui
import interfaceModule

import chat
import app
import chr
import logging

class DungeonCoolTimeWindow(ui.ScriptWindow):
    def __init__(self, layer = "UI"):
        ui.ScriptWindow.__init__(self, layer)

        self.__Initialize()
        self.__LoadWindow()

    def __Initialize(self):    
        self.floorText = None
        self.coolTimeImage = None
        self.coolTimeText = None
        self.startTime = 0.0
        self.endTime = 0.0
        self.floor = 0

    def __Load_LoadScript(self, fileName):
        try:
            pyScriptLoader = ui.PythonScriptLoader()
            pyScriptLoader.LoadScriptFile(self, fileName)
        except:
            logging.exception("DungeonCoolTimeWindow.__Load_LoadScript")

    def __Load_BindObject(self):
        try:
            self.floorText = self.GetChild("floorText")
            self.coolTimeImage = self.GetChild("coolTimeImage")
            self.coolTimeText = self.GetChild("coolTimeText")
        except:
            logging.exception("DungeonCoolTimeWindow.__Load_BindObject")

    def __Load_BindEvents(self):
        try:
            pass
        except:
            logging.exception("DungeonCoolTimeWindow.__Load_BindEvents")

    def GetLeftTime(self, leftTime):
        leftMin = int(leftTime / 60)
        leftSecond = int(leftTime % 60)

        return leftMin, leftSecond

    def Start(self, cooltime):
        self.startTime = float(app.GetTime())
        self.endTime = self.startTime + cooltime

        self.coolTimeImage.SetCoolTime(cooltime)
        self.coolTimeImage.SetStartCoolTime(self.startTime)

        if cooltime:
            self.coolTimeImage.Show()
        else:
            self.coolTimeImage.Hide()

    def Clear(self):
        self.SetCoolTime(0)

    def OnUpdate(self):
        self.RefreshLeftTime()

    def RefreshLeftTime(self):
        leftTime = max(0, self.endTime - app.GetTime() + 0.5)
        (leftMin, leftSecond) = self.GetLeftTime(leftTime)
        self.coolTimeText.SetText("%02d:%02d" % (leftMin, leftSecond))

    def __LoadWindow(self):
        self.__Load_LoadScript("UIScript/DungeonCoolTimeWindow.py")
        self.__Load_BindObject()
        self.__Load_BindEvents()

    def Open(self):
        self.Show()
        self.SetTop()

    def SetFloor(self, floor):
        self.floor = floor
        self.floorText.SetText(str(floor))

    def SetCoolTime(self, cooltime):
        self.Start(cooltime)

    def Close(self):
        self.Hide()

    def Destroy(self):
        self.ClearDictionary()
        self.__Initialize()

