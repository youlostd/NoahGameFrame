# -*- coding: utf-8 -*-

import logging

import app
import dbg
import wndMgr
from pygame.app import LoadGameData
from pygame.app import LoadLocaleData
from pygame.app import appInst

import localeInfo
import mouseModule
import musicInfo
import networkModule
import serverCommandParser


def RunApp():
    musicInfo.LoadLastPlayFieldMusic()

    app.SetHairColorEnable(1)
    app.SetArmorSpecularEnable(1)
    app.SetWeaponSpecularEnable(1)

    app.SetMouseHandler(mouseModule.mouseController)
    wndMgr.SetMouseHandler(mouseModule.mouseController)
    wndMgr.SetScreenSize(appInst.instance().GetSettings().GetWidth(), appInst.instance().GetSettings().GetHeight())

    try:
        app.Create(localeInfo.APP_TITLE, appInst.instance().GetSettings().GetWidth(), appInst.instance().GetSettings().GetHeight(), 1)
    except RuntimeError as msg:
        msg = str(msg)
        if "CREATE_DEVICE" == msg:
            dbg.LogBox(localeInfo.START_ERROR_3D)
        else:
            dbg.LogBox("N2.%s" % msg)
        return

    app.SetCamera(1500.0, 30.0, 0.0, 180.0)

    # Gets and sets the floating-point control word
    # app.SetControlFP()

    if not mouseModule.mouseController.Create():
        return

    if not LoadGameData():
        return

    if not LoadLocaleData(app.GetLocalePath()):
        return

    parserWnd = serverCommandParser.serverCommandParser()
    appInst.instance().GetNet().SetSvPsWindow(parserWnd)

    mainStream = networkModule.MainStream()
    mainStream.SetLoginPhase()
    try:
        appInst.instance().Loop()
    except Exception as e:
        logging.error("Python exception occured in main loop %s", e)
        
    mainStream.Destroy()


RunApp()
