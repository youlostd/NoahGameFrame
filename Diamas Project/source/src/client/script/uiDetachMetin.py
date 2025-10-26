# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import item
import snd
import wndMgr
from pygame.app import appInst
from pygame.player import playerInst

import ui


class DetachMetinDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.loaded = False
        self.targetItemPos = 0
        self.mainSlot = None
        self.subSlot = None
        self.explosionEffect = None
        self.selectedSlotIndex = -1
        self.accepted = False


    def __LoadScript(self):
        if self.loaded:
            return
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/DetachStoneDialog.py")

        except:
            logging.exception("AttachStoneDialog.__LoadScript.LoadObject")
            return

        try:
            self.board = self.GetChild("Board")
            self.titleBar = self.GetChild("TitleBar")
            self.mainSlot = self.GetChild("main_slot")
            self.subSlot = self.GetChild("sub_slot")
            self.subTitleText = self.GetChild("title_text")
            self.explosionEffect = self.GetChild("explosion_effect")
            self.GetChild("ExtractButton").SetEvent(self.Accept)
        except:
            logging.exception("AttachStoneDialog.__LoadScript.BindObject")
            return

        self.titleBar.SetCloseEvent(self.Close)
        self.subSlot.SetSelectItemSlotEvent(self.__OnSelectItemSlot)
        self.subSlot.SetSlotStyle(wndMgr.SLOT_STYLE_SELECT)
        self.explosionEffect.SetEndFrameEvent(self.__EffectEnd)
        self.explosionEffect.SetScale(0.5, 0.5)
        self.explosionEffect.Hide()
        self.loaded = True

    def __EffectEnd(self):
        if self.explosionEffect:
            self.explosionEffect.Hide()

    def Destroy(self):
        self.ClearDictionary()

    def __OnSelectItemSlot(self, slotNumber):
        self.subSlot.ClearSelected()
        metinIndex = playerInst().GetItemMetinSocket(self.targetItemPos, slotNumber)
        if metinIndex > 1:
            self.subTitleText.SetText(item.GetItemNameByVnum(metinIndex))
        self.subSlot.SelectSlot(slotNumber)
        self.selectedSlotIndex = slotNumber

    def Open(self, targetItemPos):
        self.__LoadScript()

        self.targetItemPos = targetItemPos

        itemIndex = playerInst().GetItemIndex(targetItemPos)
        self.mainSlot.SetItemSlot(0, itemIndex)

        for i in xrange(4):
            self.subSlot.SetItemSlot(i, long(playerInst().GetItemMetinSocket(targetItemPos, i)))

        self.SetTop()
        self.Show()

    def Accept(self):
        snd.PlaySound("sound/ui/metinstone_insert.wav")
        appInst.instance().GetNet().SendRemoveMetinPacket(self.targetItemPos, self.selectedSlotIndex)
        self.accepted = True

    def Refresh(self):
        if not self.accepted:
            return

        for i in xrange(4):
            self.subSlot.SetItemSlot(i, long(playerInst().GetItemMetinSocket(self.targetItemPos, i)))

        if playerInst().GetItemMetinSocket(self.targetItemPos, self.selectedSlotIndex) == 1:
            self.__TriggerExplosionEffect()
            self.accepted = False

    def GetTargetItemPos(self):
        return self.targetItemPos

    def __TriggerExplosionEffect(self):
        if self.explosionEffect:
            self.explosionEffect.SetPosition(72 + ((32 * self.selectedSlotIndex) + (10 * self.selectedSlotIndex)), 71)
            self.explosionEffect.ResetFrame()
            self.explosionEffect.SetDelay(3)
            self.explosionEffect.Show()

    def Close(self):
        self.subSlot.ClearSelected()
        self.selectedSlotIndex = -1
        self.targetItemPos = None
        self.Hide()
