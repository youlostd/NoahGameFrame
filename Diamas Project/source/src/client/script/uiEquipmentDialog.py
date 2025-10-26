# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import chr
import item
import player

import ui
from ui_event import MakeEvent


class EquipmentDialog(ui.ScriptWindow):
    COSTUME_SLOTS = [
        item.WEAR_COSTUME_BODY,
        item.WEAR_COSTUME_HAIR,
        item.WEAR_COSTUME_WEAPON,
        item.WEAR_COSTUME_ACCE,
        item.WEAR_COSTUME_BODY_EFFECT,
        item.WEAR_COSTUME_WEAPON_EFFECT,
        item.WEAR_COSTUME_WING_EFFECT,
        item.WEAR_COSTUME_RANK,
    ]

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.vid = None
        self.eventClose = None
        self.itemDataDict = {}
        self.tooltipItem = None
        self.loaded = False

    def __LoadDialog(self):
        if self.loaded:
            return

        PythonScriptLoader = ui.PythonScriptLoader()
        PythonScriptLoader.LoadScriptFile(self, "UIScript/EquipmentDialog.py")

        getObject = self.GetChild
        self.board = getObject("Board")
        self.slotWindow = getObject("EquipmentSlot")
        self.costumeSlotWindow = getObject("CostumeSlot")

        getObject("TitleBarCostume").DeactivateCloseButton()

        self.board.SetCloseEvent(self.Close)
        self.slotWindow.SetOverInItemEvent(self.OverInItem)
        self.slotWindow.SetOverOutItemEvent(self.OverOutItem)

        self.costumeSlotWindow.SetOverInItemEvent(self.OverInCostumeItem)
        self.costumeSlotWindow.SetOverOutItemEvent(self.OverOutItem)

    def Open(self, vid):
        self.__LoadDialog()

        self.vid = vid
        self.itemDataDict = {}

        name = chr.GetNameByVID(vid)
        self.board.SetTitleName(name)

        self.SetCenterPosition()
        self.SetTop()
        self.Show()

    def Close(self):
        self.itemDataDict = {}
        self.tooltipItem = None
        self.Hide()

        if self.eventClose:
            self.eventClose(self.vid)

    def Destroy(self):
        self.eventClose = None

        self.Close()
        self.ClearDictionary()

        self.board = None
        self.slotWindow = None
        self.costumeSlotWindow = None

    if app.ENABLE_CHANGE_LOOK_SYSTEM:
        def SetEquipmentDialogItem(self, slotIndex, vnum, count, dwChangeLookVnum):
            if count <= 1:
                count = 0

            if slotIndex in self.COSTUME_SLOTS:
                for i, slot in enumerate(self.COSTUME_SLOTS):
                    if slot == slotIndex:
                        self.costumeSlotWindow.SetItemSlot(i, vnum, count)
            else:
                self.slotWindow.SetItemSlot(slotIndex, vnum, count)

            emptySocketList = []
            emptyAttrList = []
            for i in xrange(player.METIN_SOCKET_MAX_NUM):
                emptySocketList.append(0)
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                emptyAttrList.append((0, 0))

            self.itemDataDict[slotIndex] = (vnum, count, emptySocketList, emptyAttrList, 0)
    else:
        def SetEquipmentDialogItem(self, slotIndex, vnum, count):
            if count <= 1:
                count = 0
            self.slotWindow.SetItemSlot(slotIndex, vnum, count)

            emptySocketList = []
            emptyAttrList = []
            for i in xrange(player.METIN_SOCKET_MAX_NUM):
                emptySocketList.append(0)
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                emptyAttrList.append((0, 0))
            self.itemDataDict[slotIndex] = (vnum, count, emptySocketList, emptyAttrList)

    def SetEquipmentDialogSocket(self, slotIndex, socketIndex, value):
        if not slotIndex in self.itemDataDict:
            return

        if socketIndex < 0 or socketIndex > player.METIN_SOCKET_MAX_NUM:
            return

        self.itemDataDict[slotIndex][2][socketIndex] = value

    def SetEquipmentDialogAttr(self, slotIndex, attrIndex, type, value):
        if not slotIndex in self.itemDataDict:
            return

        if attrIndex < 0 or attrIndex > player.ATTRIBUTE_SLOT_MAX_NUM:
            return

        self.itemDataDict[slotIndex][3][attrIndex] = (type, value)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def SetCloseEvent(self, event):
        self.eventClose = MakeEvent(event)

    def OverInCostumeItem(self, slotIndex):
        self.OverInItem(self.COSTUME_SLOTS[slotIndex])

    def OverInItem(self, slotIndex):
        if self.tooltipItem is None:
            return

        if slotIndex not in self.itemDataDict:
            return

        itemVnum = self.itemDataDict[slotIndex][0]
        if 0 == itemVnum:
            return

        self.tooltipItem.ClearToolTip()
        metinSlot = self.itemDataDict[slotIndex][2]
        attrSlot = self.itemDataDict[slotIndex][3]
        self.tooltipItem.AddItemData(itemVnum, metinSlot, attrSlot)

        #if app.ENABLE_CHANGE_LOOK_SYSTEM:
        #    self.tooltipItem.AppendChangeLookInfoItemVnum(self.itemDataDict[slotIndex][4])
        self.tooltipItem.ShowToolTip()

    def OverOutItem(self):
        if None != self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
