# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app

import ui
import uiScriptLocale

LOCALE_PATH = uiScriptLocale.MAPNAME_PATH


class MapNameShower(ui.ExpandedImageBox):
    MAP_NAME_IMAGE = {}

    STATE_HIDE = 0
    STATE_FADE_IN = 1
    STATE_SHOW = 2
    STATE_FADE_OUT = 3

    def __init__(self):
        self.MAP_NAME_IMAGE = {

        }

        ui.ExpandedImageBox.__init__(self, "TOP_MOST")
        self.AddFlag("not_pick")
        self.__Initialize()


    def __Initialize(self):
        self.floorImage = None
        self.objectiveImage = None
        self.fadeStartTime = 0
        self.state = self.STATE_HIDE
        self.curAlpha = 0.0
        self.SetAlpha(0.0)
        self.SetHorizontalAlignCenter()
        self.SetPosition(0, 80)
        self.Hide()

    def __GetDevilTowerFloor(self, x, y):
        if x > 10000 and y > 58000 and x < 25000 and y < 72000:
            return 1
        elif x > 10000 and y > 35000 and x < 25000 and y < 50000:
            return 2
        elif x > 10000 and y > 10000 and x < 25000 and y < 25000:
            return 3
        elif x > 35000 and y > 61000 and x < 43500 and y < 70500:
            return 4
        elif x > 35000 and y > 38000 and x < 43500 and y < 48000:
            return 5
        elif x > 14000 and y > 14000 and x < 43500 and y < 24500:
            return 6
        elif x > 56000 and y > 60000 and x < 68000 and y < 73000:
            return 7
        elif x > 56000 and y > 38000 and x < 68000 and y < 49000:
            return 8
        elif x > 56000 and y > 13000 and x < 68000 and y < 23000:
            return 9
        return 0

    def __GetDevilBase(self, x, y):
        if x > 3000 and y > 4500 and x < 45000 and y < 45000:
            return 1
        elif x > 54000 and y > 3900 and x < 100000 and y < 46200:
            return 2
        elif x > 104800 and y > 3500 and x < 145500 and y < 45800:
            return 3
        elif x > 3100 and y > 54100 and x < 56400 and y < 105800:
            return 4
        elif x > 65000 and y > 54000 and x < 105000 and y < 95500:
            return 5
        elif x > 117500 and y > 57600 and x < 142000 and y < 81000:
            return 6
        elif x > 5000 and y > 104900 and x < 15000 and y < 122000:
            return 7
        return 0

    def ShowMapName(self, mapName, x, y):
        if mapName not in self.MAP_NAME_IMAGE:
            return

        try:
            self.LoadImage(self.MAP_NAME_IMAGE[mapName])
        except RuntimeError:
            return

        self.__Initialize()

        if mapName == "metin2_map_deviltower1":
            self.SetPosition(-60, 80)

            self.floorImage = ui.ExpandedImageBox()
            self.floorImage.AddFlag("not_pick")
            self.floorImage.SetHorizontalAlignCenter()
            self.floorImage.SetPosition(100, 80)
            self.floorImage.SetAlpha(0.0)
            self.floorImage.Show()
            # ¸ÊÀÌ¸§ (ex: ¾Æ±Íµ¿±¼) ÀÌ¹ÌÁö ·Îµù & Ç¥½Ã
            try:
                floor = self.__GetDevilTowerFloor(x, y)
                self.floorImage.LoadImage(LOCALE_PATH + "devil1_%df.tga" % floor)
            except RuntimeError:
                self.SetPosition(0, 80)
                self.floorImage.Hide()
                self.floorImage = None


        if mapName == "metin2_map_devilsCatacomb":
            self.SetPosition(-75, 80)

            self.floorImage = ui.ExpandedImageBox()
            self.floorImage.AddFlag("not_pick")
            self.floorImage.SetHorizontalAlignCenter()
            self.floorImage.SetPosition(100, 80)
            self.floorImage.SetAlpha(0.0)
            self.floorImage.Show()

            # ¸ÊÀÌ¸§ (ex: ¾Æ±Íµ¿±¼) ÀÌ¹ÌÁö ·Îµù & Ç¥½Ã
            try:
                floor = self.__GetDevilBase(x, y)
                self.floorImage.LoadImage(LOCALE_PATH + "devil1_%df.tga" % floor)
            except RuntimeError:
                self.SetPosition(0, 80)
                self.floorImage.Hide()
                self.floorImage = None


                # Ãþº° ¸ñÇ¥ ÀÌ¹ÌÁö ·Îµù & Ç¥½Ã
                # ´øÀüÀº ÇöÀç ¸îÃþÀÎÁö ¾Ë¾Æ¿À´Â ºÎºÐ ¶§¹®¿¡ ÇÏµåÄÚµùÀ» ÇÇÇÏ±â°¡ Èûµé´Ù...
                try:
                    floor = self.__GetDevilBase(x, y)
                    self.objectiveImage.LoadImage(LOCALE_PATH + mapName + "/obj_%02df.tga" % floor)
                except RuntimeError:
                    self.SetPosition(0, 80)
                    self.objectiveImage.Hide()
                    self.objectiveImage = None

        self.state = self.STATE_FADE_IN
        self.fadeStartTime = app.GetTime() + 1.0
        self.Show()

    def Update(self):

        self.SetAlpha(self.curAlpha)
        if self.floorImage:
            self.floorImage.SetAlpha(self.curAlpha)

        if self.objectiveImage:
            self.objectiveImage.SetAlpha(self.curAlpha)

        if self.STATE_FADE_IN == self.state:
            if app.GetTime() > self.fadeStartTime:
                self.curAlpha += 0.05

                if self.curAlpha > 0.9:
                    self.state = self.STATE_SHOW
                    self.fadeStartTime = app.GetTime() + 5.0

        elif self.STATE_SHOW == self.state:
            if app.GetTime() > self.fadeStartTime:
                self.state = self.STATE_FADE_OUT

        elif self.STATE_FADE_OUT == self.state:
            self.curAlpha -= 0.05

            if self.curAlpha < 0.0001:
                self.Hide()
                if self.floorImage:
                    self.floorImage.Hide()
                    self.floorImage = None

                if self.objectiveImage:
                    self.objectiveImage.Hide()
                    self.objectiveImage = None
                return
