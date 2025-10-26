import logging

import app
import wndMgr
from pygame.fishing import fishingInst

import ui


class FishingGame(ui.ScriptWindow):
    FISHING_TIME_LIMIT = 15

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.__Initialize()
        self.__Load()

    def __Initialize(self):
        self.FISHING_TIME_LIMIT = 15
        self.backGroundWater = None
        self.navigationArea = None
        self.goalCircle = None
        self.hitCount = None
        self.hitCountText = None
        self.timerGauge = None
        self.timerGaugeVal = self.FISHING_TIME_LIMIT
        self.debugText_fish_pos_Local = None
        self.debugText_goal_pos_Local = None
        self.debugText_mouse_pos_Local = None
        self.hit_image_list = []
        self.miss_image_list = []
        self.wave_image_list = []
        self.fishImage = None
        self.board = None

    def __Load(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/FishingGameWindow.py")

        self.SetCenterPosition(wndMgr.GetScreenWidth() * 0.2 * -1, wndMgr.GetScreenHeight() * 0.2 * -1)

        self.board = self.GetChild("board")
        self.board.SetCloseEvent(self.QuitFishing)
        self.backGroundWater = self.GetChild("fishing_background_water")
        self.navigationArea = self.GetChild("fishing_water_navArea")
        self.goalCircle = self.GetChild("fishing_goal_circle")
        self.timerGauge = self.GetChild("fishing_timer_gauge")
        self.hitCount = self.GetChild("fishing_goal_count")
        self.hitCountText = self.GetChild("fishing_goal_count_text")
        self.debugText_fish_pos_Local = self.GetChild("debug_text_fish_pos")
        self.debugText_goal_pos_Local = self.GetChild("debug_text_circle_pos")
        self.debugText_mouse_pos_Local = self.GetChild("debug_text_mouse_pos")

        self.backGroundWater.SetClickEvent(self.OnMouseLeftButtonDownEvent)
        self.backGroundWater.SetOnMouseOverInEvent(self.OnMouseOverIn)
        self.backGroundWater.SetOnMouseOverOutEvent(self.OnMouseOverOut)

        self.CreateFishImage()
        self.Hide("Loaded")

    def Show(self):
        ui.ScriptWindow.Show(self)

    def Hide(self, wat):
        logging.debug("Hiding! {}".format(wat))
        ui.ScriptWindow.Hide(self)

    def Open(self):
        self.Show()
        self.SetTop()
        self.InitFishing()

    def Close(self):
        logging.debug("Close")
        app.SetCursor(app.NORMAL)
        self.ClearFishing()
        self.Hide("Close called")

    def Destroy(self):
        self.Close()
        self.ClearDictionary()
        self.__Initialize()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.OnPressEscapeKey()
            return True
        return False

    def OnPressEscapeKey(self):
        self.QuitFishing()

    def QuitFishing(self):
        logging.debug("QuitFishing")

        for elem in self.hit_image_list:
            elem.Hide()
            elem.Destroy()

        self.hit_image_list = []

        for elem in self.miss_image_list:
            elem.Hide()
            elem.Destroy()

        self.miss_image_list = []

        for elem in self.wave_image_list:
            elem.Hide()
            elem.Destroy()

        self.wave_image_list = []
        fishingInst().Quit()
        self.Close()

    def InitFishing(self):
        fishingInst().SetBackgroundWaterWindow(self.backGroundWater.GetWindowHandle())
        fishingInst().SetNavigationAreaWindow(self.navigationArea.GetWindowHandle())
        fishingInst().SetGoalCircleWindow(self.goalCircle.GetWindowHandle())
        fishingInst().SetTimeGaugeWindow(self.timerGauge.GetWindowHandle())
        fishingInst().SetTouchCountWindow(self.hitCount.GetWindowHandle())
        fishingInst().SetTouchCountTextWindow(self.hitCountText.GetWindowHandle())
        fishingInst().SetFishWindow(self.fishImage.GetWindowHandle())
        fishingInst().SetDebugText1(self.debugText_fish_pos_Local.GetWindowHandle())
        fishingInst().SetDebugText2(self.debugText_goal_pos_Local.GetWindowHandle())
        fishingInst().SetDebugText3(self.debugText_mouse_pos_Local.GetWindowHandle())

    def ClearFishing(self):
        fishingInst().SetBackgroundWaterWindow(None)
        fishingInst().SetNavigationAreaWindow(None)
        fishingInst().SetGoalCircleWindow(None)
        fishingInst().SetTimeGaugeWindow(None)
        fishingInst().SetTouchCountWindow(None)
        fishingInst().SetTouchCountTextWindow(None)
        fishingInst().SetFishWindow(None)
        fishingInst().SetDebugText1(None)
        fishingInst().SetDebugText2(None)
        fishingInst().SetDebugText3(None)
        logging.debug("ClearFishing")

    def CreateFishImage(self):
        self.fishImage = ui.AniImageBox()
        self.fishImage.SetWindowName("FishingGameWindow.fish_img")
        self.fishImage.SetParent(self.backGroundWater)
        self.fishImage.SetSize(30, 30)
        self.fishImage.SetDelay(5)
        self.fishImage.AppendImage("d:/ymir work/ui/game/fishing/fish/fishing_fish_1.sub")
        self.fishImage.AppendImage("d:/ymir work/ui/game/fishing/fish/fishing_fish_2.sub")
        self.fishImage.AppendImage("d:/ymir work/ui/game/fishing/fish/fishing_fish_3.sub")
        self.fishImage.AppendImage("d:/ymir work/ui/game/fishing/fish/fishing_fish_4.sub")
        self.fishImage.AddFlag("not_pick")
        self.fishImage.Show()

    def CreateHitEffect(self):
        hitImage = ui.MoveImageBox()
        hitImage.SetWindowName("FishingGame.hit_img")
        hitImage.SetParent(self.backGroundWater)
        hitImage.SetMoveSpeed(1)
        hitImage.LoadImage("d:/ymir work/ui/game/fishing/fishing_effect_hit.sub")
        hitImage.AddFlag("not_pick")
        x, y = self.backGroundWater.GetGlobalPosition()
        fishX, fishY = self.fishImage.GetLocalPosition()

        hitImage.SetPosition(fishX, fishY)
        hitImage.SetMovePosition(x + fishX, x + fishY - 30)
        hitImage.SetEndMoveEvent(hitImage.Hide)
        hitImage.Show()
        hitImage.MoveStart()
        self.hit_image_list.append(hitImage)

    def CreateMissEffect(self,  x, y):
        missImage = ui.MoveImageBox()
        missImage.SetWindowName("FishingGame.miss_img")
        missImage.SetParent(self.board)
        missImage.SetMoveSpeed(2)
        missImage.LoadImage("d:/ymir work/ui/game/fishing/fishing_effect_miss.sub")
        missImage.AddFlag("not_pick")
        waterX, waterY = self.backGroundWater.GetGlobalPosition()

        missImage.SetPosition(x, x)
        missImage.SetMovePosition(waterX + x, waterY + y - 50)
        missImage.SetEndMoveEvent(missImage.Hide)
        missImage.Show()
        missImage.MoveStart()
        self.miss_image_list.append(missImage)

    def CreateWaveEffect(self, x, y, delay):
        waveImage = ui.AniImageBox()
        waveImage.SetWindowName("FishingGame.wave_img")
        waveImage.SetParentProxy(self.backGroundWater)
        waveImage.SetSize(30, 30)
        waveImage.SetDelay(delay)
        waveImage.AppendImage("d:/ymir work/ui/game/fishing/wave/fishing_effect_wave_1.sub")
        waveImage.AppendImage("d:/ymir work/ui/game/fishing/wave/fishing_effect_wave_2.sub")
        waveImage.AppendImage("d:/ymir work/ui/game/fishing/wave/fishing_effect_wave_3.sub")
        waveImage.AppendImage("d:/ymir work/ui/game/fishing/wave/fishing_effect_wave_4.sub")
        waveImage.SetPosition(x, y)
        waveImage.AddFlag("not_pick")
        waveImage.SetEndFrameEvent(waveImage.Hide)
        waveImage.ResetFrame()
        waveImage.Show()
        self.wave_image_list.append(waveImage)

    def OnMouseLeftButtonDownEvent(self):
        fishingInst().OnClickEvent()

    def OnMouseOverOut(self):
        app.SetCursor(app.NORMAL)

    def OnMouseOverIn(self):
        app.SetCursor(app.FISHING)
