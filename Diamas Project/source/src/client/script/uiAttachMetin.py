# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import item
import player
import snd
from pygame.app import appInst
from pygame.player import playerInst

import localeInfo
import ui
import uiToolTip


class AttachMetinDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.isLoaded = False

        self.metinItemPos = 0
        self.targetItemPos = 0

    def __LoadScript(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "uiscript/attachstonedialog.py")
        except Exception as e:
            logging.exception(e)
            return

        try:
            self.board = self.GetChild("Board")
            self.titleBar = self.GetChild("TitleBar")
            self.GetChild("AcceptButton").SetEvent(self.Accept)
            self.GetChild("CancelButton").SetEvent(self.Close)
        except Exception as e:
            logging.exception(e)
            return

        oldToolTip = uiToolTip.ItemToolTip()
        oldToolTip.SetParent(self)
        oldToolTip.SetPosition(15, 38)
        oldToolTip.SetFollow(False)
        oldToolTip.Show()
        self.oldToolTip = oldToolTip

        newToolTip = uiToolTip.ItemToolTip()
        newToolTip.SetParent(self)
        newToolTip.SetPosition(230 + 20, 38)
        newToolTip.SetFollow(False)
        newToolTip.Show()
        self.newToolTip = newToolTip

        self.titleBar.SetCloseEvent(self.Close)

    def Destroy(self):
        self.ClearDictionary()
        self.board = 0
        self.titleBar = 0
        self.toolTip = 0

    def CanAttachMetin(self, slot, metin):
        if item.METIN_NORMAL == metin:
            if player.METIN_SOCKET_TYPE_SILVER == slot or player.METIN_SOCKET_TYPE_GOLD == slot:
                return True

        elif item.METIN_GOLD == metin:
            if player.METIN_SOCKET_TYPE_GOLD == slot:
                return True

    def Open(self, metinItemPos, targetItemPos):
        if not self.isLoaded:
            self.__LoadScript()
            self.isLoaded = True

        self.metinItemPos = metinItemPos
        self.targetItemPos = targetItemPos

        metinIndex = playerInst().GetItemIndex(metinItemPos)
        itemIndex = playerInst().GetItemIndex(targetItemPos)
        self.oldToolTip.ClearToolTip()
        self.newToolTip.ClearToolTip()

        item.SelectItem(metinIndex)
        attrSlot = [(0, 0)] * player.ATTRIBUTE_SLOT_MAX_NUM

        ## Old Item ToolTip
        metinSlot = []
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            metinSlot.append(long(playerInst().GetItemMetinSocket(targetItemPos, i)))
        self.oldToolTip.AddItemData(itemIndex, metinSlot, attrSlot)

        ## New Item ToolTip
        item.SelectItem(metinIndex)
        metinSubType = item.GetItemSubType()

        metinSlot = []
        for i in xrange(player.METIN_SOCKET_MAX_NUM ):
            metinSlot.append(long(playerInst().GetItemMetinSocket(targetItemPos, i)))
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            slotData = metinSlot[i]
            if self.CanAttachMetin(slotData, metinSubType):
                metinSlot[i] = metinIndex
                break

        self.newToolTip.AddItemData(itemIndex, metinSlot, attrSlot)

        self.UpdateDialog()
        self.SetTop()
        self.Show()

    def UpdateDialog(self):
        newWidth = self.newToolTip.GetWidth() + 230 + 15 + 20
        newHeight = self.newToolTip.GetHeight() + 98

        if localeInfo.IsARABIC():
            self.board.SetPosition(newWidth, 0)

            (x, y) = self.titleBar.GetLocalPosition()
            self.titleBar.SetPosition(newWidth - 15, y)

        self.board.SetSize(newWidth, newHeight)
        self.titleBar.SetWidth(newWidth - 15)
        self.SetSize(newWidth, newHeight)

        (x, y) = self.GetLocalPosition()
        self.SetPosition(x, y)

    def Accept(self):
        appInst.instance().GetNet().SendItemUseToItemPacket(self.metinItemPos, self.targetItemPos)
        snd.PlaySound("sound/ui/metinstone_insert.wav")
        self.Close()

    def Close(self):
        self.Hide()
