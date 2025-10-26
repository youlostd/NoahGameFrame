# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import chat
import player

import localeInfo
import ui
import uiToolTip
from ui_event import Event

INVENTORY_PAGE_SIZE = player.INVENTORY_PAGE_SIZE  # 45


class SelectAttrWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.inven = None
        self.itemToolTip = uiToolTip.ItemToolTip()


    def Open(self, window_type, slotIdx):
        self.window_type = window_type
        self.slotIdx = slotIdx
        self.SelectIdx = -1

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/SelectAttrDialog.py")

        self.SelectBtn = []
        self.SelectBtn.append(self.GetChild("SelectBtn0"))
        self.SelectBtn.append(self.GetChild("SelectBtn1"))

        self.lTextList = []
        self.lTextList.append(self.GetChild("lText0"))
        self.lTextList.append(self.GetChild("lText1"))
        self.lTextList.append(self.GetChild("lText2"))
        self.lTextList.append(self.GetChild("lText3"))
        self.lTextList.append(self.GetChild("lText4"))
        self.lTextList.append(self.GetChild("lText5"))
        self.lTextList.append(self.GetChild("lText6"))

        self.rTextList = []
        self.rTextList.append(self.GetChild("rText0"))
        self.rTextList.append(self.GetChild("rText1"))
        self.rTextList.append(self.GetChild("rText2"))
        self.rTextList.append(self.GetChild("rText3"))
        self.rTextList.append(self.GetChild("rText4"))
        self.rTextList.append(self.GetChild("rText5"))
        self.rTextList.append(self.GetChild("rText6"))

        Slot = self.GetChild("toolTipItemSlot")
        Slot.SetOverInItemEvent(self.OverInItem)
        Slot.SetOverOutItemEvent(self.OverOutItem)
        Slot.SetItemSlot(0, player.GetItemIndex(self.slotIdx))
        Slot.RefreshSlot()

        self.TitleBar = self.GetChild("SelectAttr_TitleBar")
        self.TitleBar.CloseButtonHide()
        # self.TitleBar.SetCloseEvent(ui.__mem_func__(self.Close))
        self.GetChild("accept_button").SetEvent(self.Close)


        self.SelectBtn[0].SetEvent(Event(self.SelectButton, 0))
        self.SelectBtn[1].SetEvent(Event(self.SelectButton, 1))

        ## ������ �Ӽ� ��������
        CurAttr = [player.GetItemAttribute(self.window_type, self.slotIdx, i) for i in
                   xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
        ChangedAttr = [player.GetItemChangedAttribute(i) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]

        self.CurrentTextList = []
        self.ChangedTextList = []
        self.CurrentTextList = self.GetAttributeStringList(CurAttr)
        self.ChangedTextList = self.GetAttributeStringList(ChangedAttr)

        TotalLines = len(self.CurrentTextList)
        for i in xrange(len(self.lTextList)):
            if i < TotalLines:
                self.lTextList[i].SetText(self.CurrentTextList[i])
            else:
                self.lTextList[i].Hide()

        for i in xrange(len(self.rTextList)):
            if i < TotalLines:
                self.rTextList[i].SetText(self.ChangedTextList[i])
            else:
                self.rTextList[i].Hide()

    def Close(self):
        if self.SelectIdx < 0:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SELECT_ATTR_NOT_SELECT)
            return
        else:
            pass #appInst.instance().GetNet().SendChagedItemAttributePacket(self.SelectIdx, self.window_type, self.slotIdx)

        self.SetCanMouseEventSlot(self.slotIdx)
        self.Hide()

    def OverInItem(self):
        self.itemToolTip.SetFollow(False)
        self.itemToolTip.SetInventoryItem(self.slotIdx, self.window_type)
        width = self.itemToolTip.GetWidth()
        x, y = self.TitleBar.GetGlobalPosition()
        self.itemToolTip.SetPosition(x - width, y)

    def OverOutItem(self):
        self.itemToolTip.HideToolTip()

    def GetAttributeStringList(self, attrSlot):
        if 0 != attrSlot:

            toolTipString = []
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                type = attrSlot[i][0]
                value = attrSlot[i][1]

                if 0 == value:
                    continue

                toolTipString.append(self.itemToolTip.GetAffectString(type, value))

            return toolTipString

    def SetFontColor(self, idx):
        if idx == 0:
            for l, r in zip(self.lTextList, self.rTextList):
                l.SetFontColor(1.0, 1.0, 1.0)
                r.SetFontColor(0.3020, 0.3020, 0.3020)
        else:
            for l, r in zip(self.lTextList, self.rTextList):
                l.SetFontColor(0.3020, 0.3020, 0.3020)
                r.SetFontColor(1.0, 1.0, 1.0)

    def SelectButton(self, idx):
        if not (-1 < idx < 2):
            return

        self.SetFontColor(idx)

        for button in self.SelectBtn:
            button.SetUp()

        self.SelectBtn[idx].Down()
        self.SelectIdx = idx

    def SetCanMouseEventSlot(self, idx):
        if idx >= INVENTORY_PAGE_SIZE:
            idx -= INVENTORY_PAGE_SIZE

        self.inven.wndItem.SetCanMouseEventSlot(idx)

    def SetCantMouseEventSlot(self, idx):
        page = self.inven.GetInventoryPageIndex()
        lock_idx = idx - (page * INVENTORY_PAGE_SIZE)
        self.inven.wndItem.SetCantMouseEventSlot(lock_idx)

    # def OnPressEscapeKey(self) :
    # self.Close()
    # return True;

    def SetInven(self, inven):
        from _weakref import proxy
        self.inven = proxy(inven)

    def OnUpdate(self):
        if not self.inven:
            return

        if -1 < self.slotIdx < INVENTORY_PAGE_SIZE * 2:
            self.SetCantMouseEventSlot(self.slotIdx)
