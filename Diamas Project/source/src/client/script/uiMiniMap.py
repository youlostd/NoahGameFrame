# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging
from time import strftime

import app
import miniMap
import player
import wndMgr

import constInfo
import localeInfo
import ui
import uiScriptLocale

if app.WJ_SHOW_PARTY_ON_MINIMAP:
    QUEST_TOOLTIP_COLOR = 0xFFF2CB61

from pygame.net import GetServerInfo
from pygame.background import bgInst


class AtlasWindow(ui.ScriptWindow):
    class AtlasRenderer(ui.Window):
        def __init__(self):
            ui.Window.__init__(self)
            self.AddFlag("not_pick")

        def OnUpdate(self):
            miniMap.UpdateAtlas()

        def OnRender(self):
            (x, y) = self.GetGlobalPosition()
            fx = float(x)
            fy = float(y)
            miniMap.RenderAtlas(fx + 4, fy + 4)

        def HideAtlas(self):
            miniMap.HideAtlas()

        def ShowAtlas(self):
            miniMap.ShowAtlas()

    def __init__(self):
        self.tooltipInfo = ui.TextToolTip()
        self.tooltipInfo.Hide()
        self.infoGuildMark = ui.MarkBox()
        self.infoGuildMark.Hide()
        self.AtlasMainWindow = None
        self.mapName = ""
        self.board = 0
        self.IsShowWindowValue = False
        ui.ScriptWindow.__init__(self)

    def SetMapName(self, mapName):
        try:
            mapInfo = bgInst().GetMapInfoByName(mapName)
            if mapInfo:
                self.board.SetTitleName(
                    getattr(localeInfo, mapInfo.mapTranslation, "Unknown")
                )
        except Exception as e:
            logging.exception(e)

    def LoadWindow(self):
        logging.debug("Load Window AtlasWindow")

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/AtlasWindow.py")

        self.board = self.GetChild("board")

        self.AtlasMainWindow = self.AtlasRenderer()
        self.board.SetCloseEvent(self.Close)
        self.AtlasMainWindow.SetParent(self.board)
        self.AtlasMainWindow.SetPosition(2, 38)

        self.SetPosition(wndMgr.GetScreenWidth() - 136 - 256 - 10 - 165, 78)
        self.Hide()

        miniMap.RegisterAtlasWindow(self)

    def Destroy(self):
        miniMap.UnregisterAtlasWindow()
        self.ClearDictionary()
        self.AtlasMainWindow = None
        self.tooltipAtlasClose = 0
        self.tooltipInfo = None
        self.infoGuildMark = None
        self.board = None

    def OnUpdate(self):
        if not self.infoGuildMark:
            return

        self.infoGuildMark.Hide()
        self.tooltipInfo.Hide()

        (mouseX, mouseY) = wndMgr.GetMousePosition()
        (bFind, sName, iPosX, iPosY, dwTextColor, dwGuildID) = miniMap.GetAtlasInfo(
            mouseX, mouseY
        )

        if False == bFind:
            return

        if "empty_guild_area" == sName:
            sName = localeInfo.GUILD_EMPTY_AREA

        if app.WJ_SHOW_PARTY_ON_MINIMAP:
            splitsName = sName.split("|")
            isQuest = len(splitsName) == 2
            if localeInfo.IsARABIC() and sName[-1].isalnum():
                if isQuest and len(splitsName[0]) == 0:
                    self.tooltipInfo.SetText(
                        "(%s)%d, %d"
                        % (uiScriptLocale.GUILD_BUILDING_POSITION, iPosX, iPosY)
                    )
                else:
                    self.tooltipInfo.SetText(
                        "(%s)%d, %d" % (splitsName[0], iPosX, iPosY)
                    )
            else:
                if isQuest and len(splitsName[0]) == 0:
                    self.tooltipInfo.SetText(
                        "%s(%d, %d)"
                        % (uiScriptLocale.GUILD_BUILDING_POSITION, iPosX, iPosY)
                    )
                else:
                    self.tooltipInfo.SetText(
                        "%s(%d, %d)" % (splitsName[0], iPosX, iPosY)
                    )
        else:
            if localeInfo.IsARABIC() and sName[-1].isalnum():
                self.tooltipInfo.SetText("(%s)%d, %d" % (sName, iPosX, iPosY))
            else:
                self.tooltipInfo.SetText("%s(%d, %d)" % (sName, iPosX, iPosY))

        self.tooltipInfo.SetPackedFontColor(dwTextColor)
        self.tooltipInfo.Show()

        if 0 != dwGuildID:
            textWidth = self.tooltipInfo.GetWidth()
            tx, ty = self.tooltipInfo.GetGlobalPosition()
            self.infoGuildMark.SetIndex(dwGuildID)
            self.infoGuildMark.SetPosition(tx - textWidth - 18 - 5, ty)
            self.infoGuildMark.Show()

    def Hide(self):
        if self.AtlasMainWindow:
            self.AtlasMainWindow.HideAtlas()
            self.AtlasMainWindow.Hide()
        ui.ScriptWindow.Hide(self)

    def Show(self):
        if self.AtlasMainWindow:
            (bGet, iSizeX, iSizeY) = miniMap.GetAtlasSize()
            if bGet:
                self.SetSize(iSizeX, iSizeY + 38 + 3 + 10)

                if localeInfo.IsARABIC():
                    self.board.SetPosition(iSizeX + 15, 0)

                self.board.SetSize(iSizeX + 12, iSizeY + 38 + 3 + 10)
                self.SetPosition(wndMgr.GetScreenWidth() - 136 - iSizeX - 25 - 200, 78)
                # self.AtlasMainWindow.SetSize(iSizeX, iSizeY)
                self.AtlasMainWindow.ShowAtlas()
                self.AtlasMainWindow.Show()
        ui.ScriptWindow.Show(self)
        self.IsShowWindowValue = True

    def SetCenterPositionAdjust(self, x, y):
        self.SetPosition(
            (wndMgr.GetScreenWidth() - self.GetWidth()) / 2 + x,
            (wndMgr.GetScreenHeight() - self.GetHeight()) / 2 + y,
        )

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

    def Close(self):
        self.IsShowWindowValue = False
        self.Hide()

    def IsShowWindow(self):
        return self.IsShowWindowValue


class MiniMap(ui.ScriptWindow):
    CANNOT_SEE_INFO_MAP_DICT = {
        "metin2_map_monkeydungeon": False,
        "metin2_map_monkeydungeon_02": False,
        "metin2_map_monkeydungeon_03": False,
        "metin2_map_devilscatacomb": False,
        "metin2_map_Mt_Th_dungeon_01": False,
        "metin2_map_orclabyrinth": False,
        "metin2_map_deviltower1": False,
    }

    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.__Initialize()

        miniMap.Create()
        miniMap.SetScale(2.0)

        self.AtlasWindow = AtlasWindow()
        self.AtlasWindow.LoadWindow()
        self.AtlasWindow.Hide()

        self.tooltipMiniMapOpen = ui.TextToolTip()
        self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP)
        self.tooltipMiniMapOpen.Show()
        self.tooltipMiniMapClose = ui.TextToolTip()
        self.tooltipMiniMapClose.SetText(localeInfo.UI_CLOSE)
        self.tooltipMiniMapClose.Show()
        self.tooltipScaleUp = ui.TextToolTip()
        self.tooltipScaleUp.SetText(localeInfo.MINIMAP_INC_SCALE)
        self.tooltipScaleUp.Show()
        self.tooltipScaleDown = ui.TextToolTip()
        self.tooltipScaleDown.SetText(localeInfo.MINIMAP_DEC_SCALE)
        self.tooltipScaleDown.Show()
        self.tooltipAtlasOpen = ui.TextToolTip()
        self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_SHOW_AREAMAP)
        self.tooltipAtlasOpen.Show()
        self.tooltipInfo = ui.TextToolTip()
        self.tooltipInfo.Show()

        if miniMap.IsAtlas():
            self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_SHOW_AREAMAP)
        else:
            self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_CAN_NOT_SHOW_AREAMAP)

        self.mapName = ""

        self.isLoaded = 0
        self.canSeeInfo = True

        # AUTOBAN
        self.imprisonmentDuration = 0
        self.imprisonmentEndTime = 0
        self.imprisonmentEndTimeText = ""

    # END_OF_AUTOBAN

    def __Initialize(self):
        self.positionInfo = 0
        self.observerCount = 0
        self.dateLine = None
        self.timeLine = None

        self.OpenWindow = 0
        self.CloseWindow = 0
        self.ScaleUpButton = 0
        self.ScaleDownButton = 0
        self.MiniMapHideButton = 0
        self.MiniMapShowButton = 0
        self.AtlasShowButton = 0

        self.tooltipMiniMapOpen = 0
        self.tooltipMiniMapClose = 0
        self.tooltipScaleUp = 0
        self.tooltipScaleDown = 0
        self.tooltipAtlasOpen = 0
        self.tooltipInfo = None
        self.serverInfo = None

    def SetMapName(self, mapName):
        self.mapName = mapName
        self.AtlasWindow.SetMapName(mapName)

        if mapName in self.CANNOT_SEE_INFO_MAP_DICT:
            self.canSeeInfo = False
            self.HideMiniMap()
            self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP_CANNOT_SEE)
        else:
            self.canSeeInfo = True
            self.ShowMiniMap()
            self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP)

    # AUTOBAN
    def SetImprisonmentDuration(self, duration):
        self.imprisonmentDuration = duration
        self.imprisonmentEndTime = app.GetGlobalTimeStamp() + duration

        self.__UpdateImprisonmentDurationText()

    def __UpdateImprisonmentDurationText(self):
        restTime = max(self.imprisonmentEndTime - app.GetGlobalTimeStamp(), 0)

        imprisonmentEndTimeText = localeInfo.SecondToDHM(restTime)
        if imprisonmentEndTimeText != self.imprisonmentEndTimeText:
            self.imprisonmentEndTimeText = imprisonmentEndTimeText
            self.serverInfo.SetText(
                "%s: %s"
                % (uiScriptLocale.AUTOBAN_QUIZ_REST_TIME, self.imprisonmentEndTimeText)
            )

    # END_OF_AUTOBAN

    def Show(self):
        self.__LoadWindow()

        ui.ScriptWindow.Show(self)

    def UpdateServerInfo(self, text):
        self.serverInfo.SetText(text)

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/MiniMap.py")

        self.OpenWindow = self.GetChild("OpenWindow")
        self.MiniMapWindow = self.GetChild("MiniMapWindow")
        self.ScaleUpButton = self.GetChild("ScaleUpButton")
        self.ScaleDownButton = self.GetChild("ScaleDownButton")
        self.MiniMapHideButton = self.GetChild("MiniMapHideButton")
        self.AtlasShowButton = self.GetChild("AtlasShowButton")
        self.CloseWindow = self.GetChild("CloseWindow")
        self.MiniMapShowButton = self.GetChild("MiniMapShowButton")
        self.positionInfo = self.GetChild("PositionInfo")
        self.timeLine = self.GetChild("TimeLine")
        self.dateLine = self.GetChild("DateLine")

        self.observerCount = self.GetChild("ObserverCount")
        self.serverInfo = self.GetChild("ServerInfo")

        if constInfo.MINIMAP_POSITIONINFO_ENABLE == 0:
            self.positionInfo.Hide()

        self.serverInfo.SetText(GetServerInfo())
        self.ScaleUpButton.SetEvent(self.ScaleUp)
        self.ScaleDownButton.SetEvent(self.ScaleDown)
        self.MiniMapHideButton.SetEvent(self.HideMiniMap)
        self.MiniMapShowButton.SetEvent(self.ShowMiniMap)

        if miniMap.IsAtlas():
            self.AtlasShowButton.SetEvent(self.ToggleAtlasWindow)

        self.ShowMiniMap()

    def Destroy(self):
        miniMap.Destroy()

        self.HideMiniMap()

        self.AtlasWindow.Destroy()
        self.AtlasWindow = None

        self.ClearDictionary()

        self.__Initialize()

    def UpdateClock(self):
        self.dateLine.SetText(strftime("%d/%m/%Y"))
        self.timeLine.SetText(strftime("%H:%M:%S"))

    def UpdateObserverCount(self, observerCount):
        if observerCount > 0:
            self.observerCount.Show()
        elif observerCount <= 0:
            self.observerCount.Hide()

        self.observerCount.SetText(
            localeInfo.MINIMAP_OBSERVER_COUNT.format(observerCount)
        )

    def OnUpdate(self):
        (x, y, z) = player.GetMainCharacterPosition()
        if self.isShowMiniMap():
            miniMap.Update(x, y)

        self.positionInfo.SetText("(%.0f, %.0f)" % (x / 100, y / 100))
        if self.IsShow() and self.dateLine.IsShow():
            self.UpdateClock()

        if self.tooltipInfo:
            if 1 == self.MiniMapWindow.IsIn():
                (mouseX, mouseY) = wndMgr.GetMousePosition()
                (bFind, sName, iPosX, iPosY, dwTextColor) = miniMap.GetInfo(
                    mouseX, mouseY
                )
                if bFind == 0:
                    self.tooltipInfo.Hide()
                elif not self.canSeeInfo:
                    self.tooltipInfo.SetText(
                        "%s(%s)" % (sName, localeInfo.UI_POS_UNKNOWN)
                    )
                    self.tooltipInfo.SetPackedFontColor(dwTextColor)
                    self.tooltipInfo.Show()
                else:
                    if localeInfo.IsARABIC() and sName[-1].isalnum():
                        self.tooltipInfo.SetText("(%s)%d, %d" % (sName, iPosX, iPosY))
                    else:
                        self.tooltipInfo.SetText("%s(%d, %d)" % (sName, iPosX, iPosY))
                    self.tooltipInfo.SetPackedFontColor(dwTextColor)
                    self.tooltipInfo.Show()
                    self.tooltipInfo.SetTop()
            else:
                self.tooltipInfo.Hide()

            # AUTOBAN
            if self.imprisonmentDuration:
                self.__UpdateImprisonmentDurationText()
                # END_OF_AUTOBAN

        if True == self.MiniMapShowButton.IsIn():
            self.tooltipMiniMapOpen.Show()
        else:
            self.tooltipMiniMapOpen.Hide()

        if True == self.MiniMapHideButton.IsIn():
            self.tooltipMiniMapClose.Show()
        else:
            self.tooltipMiniMapClose.Hide()

        if True == self.ScaleUpButton.IsIn():
            self.tooltipScaleUp.Show()
        else:
            self.tooltipScaleUp.Hide()

        if True == self.ScaleDownButton.IsIn():
            self.tooltipScaleDown.Show()
        else:
            self.tooltipScaleDown.Hide()

        if True == self.AtlasShowButton.IsIn():
            self.tooltipAtlasOpen.Show()
        else:
            self.tooltipAtlasOpen.Hide()

    def OnRender(self):
        if self.isShowMiniMap():
            (x, y) = self.GetGlobalPosition()
            fx = float(x)
            fy = float(y)
            miniMap.Render(fx + 4.0, fy + 5.0)

    def Close(self):
        self.HideMiniMap()

    def HideMiniMap(self):
        miniMap.Hide()
        self.OpenWindow.Hide()
        self.CloseWindow.Show()

    def ShowMiniMap(self):
        if not self.canSeeInfo:
            return

        miniMap.Show()
        self.OpenWindow.Show()
        self.CloseWindow.Hide()

    def isShowMiniMap(self):
        return miniMap.isShow()

    def ScaleUp(self):
        miniMap.ScaleUp()

    def ScaleDown(self):
        miniMap.ScaleDown()

    def ShowAtlas(self):
        if not miniMap.IsAtlas():
            self.AtlasWindow.Close()
            return
        if not self.AtlasWindow.IsShow():
            self.AtlasWindow.Show()

    def ToggleAtlasWindow(self):
        if not miniMap.IsAtlas():
            return
        if self.AtlasWindow.IsShow():
            self.AtlasWindow.Hide()
        else:
            self.AtlasWindow.Show()
