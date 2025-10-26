# -*- coding: utf-8 -*-
import os

import app

import localeInfo
import ui
from ui_event import MakeEvent

FILE_NAME_LEN = 20
DEFAULT_THEMA = localeInfo.MUSIC_METIN2_DEFAULT_THEMA


class Item(ui.ListBoxEx.Item):
    def __init__(self, fileName):
        ui.ListBoxEx.Item.__init__(self)
        self.canLoad = 0
        self.text = fileName
        self.textLine = self.__CreateTextLine(fileName[:FILE_NAME_LEN])

    def GetText(self):
        return self.text

    def SetSize(self, width, height):
        ui.ListBoxEx.Item.SetSize(self, 6 * len(self.textLine.GetText()) + 4, height)

    def __CreateTextLine(self, fileName):
        textLine = ui.TextLine()
        textLine.SetParent(self)

        if localeInfo.IsARABIC():
            textLine.SetPosition(6 * len(fileName) + 6, 0)
        else:
            textLine.SetPosition(0, 0)

        textLine.SetText(fileName)
        textLine.Show()
        return textLine


class PopupDialog(ui.ScriptWindow):
    def __init__(self, parent):
        ui.ScriptWindow.__init__(self)

        self.__Load()
        self.__Bind()

    def __Load(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PopupDialog.py")

    def __Bind(self):
        self.textLine = self.GetChild("message")
        self.okButton = self.GetChild("accept")

        self.okButton.SetEvent(self.__OnOK)

    def Open(self, msg):
        self.textLine.SetText(msg)
        self.SetCenterPosition()
        self.Show()
        self.SetTop()

    def __OnOK(self):
        self.Hide()


class FileListDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)

        self.isLoaded = 0
        self.selectEvent = None
        self.languageListBox = None

    def Show(self):
        if self.isLoaded == 0:
            self.isLoaded = 1
            self.__Load()

        ui.ScriptWindow.Show(self)

    def Open(self):
        self.Show()

        self.SetCenterPosition()
        self.SetTop()

        if self.languageListBox.IsEmpty():
            self.__PopupMessage("No non-default language available")

    def Close(self):
        self.popupDialog.Hide()
        self.Hide()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def SetSelectEvent(self, event):
        self.selectEvent = MakeEvent(event)

    def __CreateLanguageListBox(self):
        languageListBox = ui.ListBoxEx()
        languageListBox.SetParent(self)

        if localeInfo.IsARABIC():
            languageListBox.SetPosition(self.GetWidth() - languageListBox.GetWidth() - 10, 50)
        else:
            languageListBox.SetPosition(15, 50)

        languageListBox.Show()
        return languageListBox

    def __Load(self):
        self.popupDialog = PopupDialog(self)
        self.__Load_LoadScript("UIScript/MusicListWindow.py")

        self.__Load_BindObject()

        self.refreshButton.SetEvent(self.__OnRefresh)
        self.cancelButton.SetEvent(self.__OnCancel)
        self.okButton.SetEvent(self.__OnOK)
        self.board.SetCloseEvent(self.__OnCancel)
        self.board.SetTitleName("Language Select")
        self.UpdateRect()

        self.__RefreshLangList()

    def __Load_LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)


    def __Load_BindObject(self):
        self.languageListBox = self.__CreateLanguageListBox()
        self.languageListBox.SetScrollBar(self.GetChild("ScrollBar"))

        self.board = self.GetChild("board")
        self.okButton = self.GetChild("ok")
        self.cancelButton = self.GetChild("cancel")
        self.refreshButton = self.GetChild("refresh")

        self.popupText = self.popupDialog.GetChild("message")


    def __PopupMessage(self, msg):
        self.popupDialog.Open(msg)

    def __OnOK(self):
        selItem = self.languageListBox.GetSelectedItem()
        if selItem:
            if self.selectEvent:
                self.selectEvent(selItem.GetText())

            self.Hide()
        else:
            self.__PopupMessage("No language selected!")

    def __OnCancel(self):
        self.Hide()

    def __OnRefresh(self):
        self.__RefreshLangList()

    def __RefreshLangList(self):
        self.__ClearLanguageList()
        self.__AppendLanguage("de")
        self.__AppendLanguage("en")
        self.__AppendLanguage("ro")
        self.__AppendLanguage("tr")

    def __ClearLanguageList(self):
        self.languageListBox.RemoveAllItems()

    def WalkDir(self, top):
        join, isdir = os.path.join, os.path.isdir

        try:
            # Note that listdir and error are globals in this module due
            # to earlier import-*.
            names = os.listdir(top)
        except IndexError, err:
            return None

        dirs = []
        for name in names:
            if isdir(join(top, name)):
                dirs.append(name)

        return dirs

    def __ReplaceCodeWitName(self, code):
        langCodeReplace = {
            "de": localeInfo.LANG_GERMAN,
            #"en": localeInfo.LANG_ENGLISH,
            #"tr": localeInfo.LANG_TURKEY,
            #"ro": localeInfo.LANG_ROMANIAN,
        }
        try:
            return langCodeReplace[code]
        except:
            return code

    def __AppendLanguage(self, folderName):
        self.languageListBox.AppendItem(Item(self.__ReplaceCodeWitName(folderName)))
