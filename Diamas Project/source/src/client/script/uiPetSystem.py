# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import player
from pygame.app import appInst
from pygame.item import MakeItemPosition, itemManager
from pygame.player import playerInst

import ui
from localeInfo import DottedNumber
from switchbot import FormatApplyAsString
from ui_event import Event


class LevelPetWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.slotIndex = -1
        self.__CreateDialog()

    def __CreateDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/SibPetSystem.py")
        self.board = self.GetChild("board")
        self.statPoints = self.GetChild("Status_Plus_Value")
        self.petLevel = self.GetChild("PetLevelValue")
        self.petIcon = self.GetChild("PetIcon")
        self.board.SetCloseEvent(self.Close)

        self.petAttributes = []
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            self.petAttributes.append({
                "name": self.GetChild("PetAttr{}Text".format(i)),
                "field": self.GetChild("PetAttr{}ValueField".format(i)),
                "value": self.GetChild("PetAttrValueField{}_Value".format(i)),
                "button": self.GetChild("PetAttr{}_Plus".format(i))
                })

        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            self.petAttributes[i]["button"].SetEvent(Event(self.DistributePoint, i))

    def GetPetAttributeCount(self):
        attrCount = 0
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attr = playerInst().GetItemAttribute(MakeItemPosition(player.INVENTORY, self.slotIndex), i)
            if attr.type != 0:
                attrCount += 1
        return attrCount

    def DistributePoint(self, index):
        appInst.instance().GetNet().SendAttrPlusLevelPet(index)

    def Open(self, slotIndex):
        self.SetCenterPosition()
        self.SetTop()
        self.slotIndex = slotIndex
        self.LoadPetData()

        ui.ScriptWindow.Show(self)

        for children in self.GetChildrenByType("board_with_titlebar"):
            children.SetTop()

        for children in self.GetChildrenByType("titlebar"):
            children.SetTop()
        for children in self.GetChildrenByType("thinboard"):
            children.SetTop()

    def HideAttributeByIndex(self, index):
        self.petAttributes[index]["name"].Hide()
        self.petAttributes[index]["value"].Hide()
        self.petAttributes[index]["field"].Hide()
        self.petAttributes[index]["button"].Hide()

    def ShowAttributeByIndex(self, index):
        self.petAttributes[index]["name"].Show()
        self.petAttributes[index]["value"].Show()
        self.petAttributes[index]["field"].Show()
        self.petAttributes[index]["button"].Show()

    def SetAttributeByIndex(self, index, attr):
        self.petAttributes[index]["name"].SetText(FormatApplyAsString(attr.type, 0))
        self.petAttributes[index]["value"].SetText(DottedNumber(attr.value))

    def LoadPetData(self):
        if self.slotIndex == -1:
            return
        itemPos = MakeItemPosition(player.INVENTORY, self.slotIndex)
        itemVnum = playerInst().GetItemIndex(itemPos)

        self.statPoints.SetText(str(playerInst().GetItemMetinSocket(itemPos, 2)))
        self.petLevel.SetText(str(playerInst().GetItemMetinSocket(itemPos, 0)))

        proto = itemManager().GetProto(itemVnum)
        if not proto:
            return
        itemIcon = proto.GetIconImageFileName()

        self.petIcon.LoadImage(itemIcon)

        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            self.HideAttributeByIndex(i)

        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attr = playerInst().GetItemAttribute(MakeItemPosition(player.INVENTORY, self.slotIndex), i)
            if attr.type != 0:
                self.ShowAttributeByIndex(i)
                self.SetAttributeByIndex(i, attr)

    def Close(self):
        appInst.instance().GetNet().SendCloseLevelPet()
        self.slotIndex = 0

    def OnKeyDown(self, key):
        import app
        if key == app.VK_ESCAPE:
            self.OnPressEscapeKey()
            return True
        return False

    def OnPressEscapeKey(self):
        self.Close()
