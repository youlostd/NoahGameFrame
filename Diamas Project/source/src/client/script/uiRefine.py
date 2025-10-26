# -*- coding: utf-8 -*-


from _weakref import proxy

import app
import item
import player
from pygame.app import appInst
from pygame.item import MakeItemPosition
from pygame.item import itemManager
from pygame.player import playerInst

import constInfo
import localeInfo
import ui
import uiCommon
import uiToolTip
from ui_event import Event


class RefineMaterialWindow(ui.Window):
    def __init__(self):
        super(RefineMaterialWindow, self).__init__()

        self.itemImageBackground = ui.ImageBox()
        self.itemImageBackground.SetParent(self)
        self.itemImageBackground.SetPosition(15, 0)
        self.itemImageBackground.LoadImage("d:/ymir work/ui/public/slot_base.sub")
        self.itemImageBackground.Show()

        self.itemImage = ui.ImageBox()
        self.itemImage.SetParent(self.itemImageBackground)
        self.itemImage.SetPosition(0, 0)
        self.itemImage.Show()

        self.thinBoard = ui.ThinBoardOld()
        self.thinBoard.SetParent(self)
        self.thinBoard.SetPosition(55, 0)
        self.thinBoard.SetSize(231, 20)
        self.thinBoard.Show()

        self.textLine = ui.TextLine()
        self.textLine.SetParent(self.thinBoard)
        self.textLine.SetFontName(localeInfo.UI_DEF_FONT)

        self.textLine.SetOutline()
        self.textLine.SetVerticalAlignCenter()
        self.textLine.SetHorizontalAlignCenter()
        self.textLine.SetPosition(0, 0)
        self.textLine.Show()
        self.SetSize(227, 32)

        self.vnum = 0
        self.count = 0
        self.enhanceProb = 0

    def SetItem(self, vnum, count, enhanceProb=None):
        self.vnum = vnum
        self.count = count
        if enhanceProb is not None:
            self.enhanceProb = enhanceProb

        proto = itemManager().GetProto(vnum)
        if proto:
            self.itemImage.LoadImage(proto.GetIconImageFileName())

            itemCount = playerInst().GetItemCountByVnum(vnum)
            if itemCount >= count:
                self.textLine.SetPackedFontColor(0xFFB3EE3A)
            else:
                self.textLine.SetPackedFontColor(0xFFEE3B3B)

            if self.enhanceProb > 0:
                self.textLine.SetText(
                    "%s x%02d (%d) Bonus+%d%%"
                    % (proto.GetName(), self.count, itemCount, self.enhanceProb)
                )
            else:
                self.textLine.SetText(
                    "%s x%02d (%d)" % (proto.GetName(), self.count, itemCount)
                )

            ySlotCount = proto.GetSize()
            self.SetSize(227, 32 * ySlotCount)

    def GetTextLine(self):
        return self.textLine

    def GetVnum(self):
        return self.vnum

    def GetCount(self):
        return self.count

    def GetEnhanceProb(self):
        return self.enhanceProb


class RefineDialogNew(ui.ScriptWindow):

    def __init__(self, interface):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__Initialize()
        self.interface = proxy(interface)
        self.isLoaded = False

    def __Initialize(self):
        self.dlgQuestion = None
        self.children = []
        self.vnum = 0
        self.targetItemPos = 0
        self.dialogHeight = 0
        self.cost = 0
        self.percentage = 0
        self.type = 0
        self.refineSet = 0
        self.oldVnum = 0
        self.materials = []
        self.enhanceMaterials = []

    def __InitializeOpen(self):
        self.children = []
        self.vnum = 0
        self.targetItemPos = 0
        self.dialogHeight = 0
        self.cost = 0
        self.percentage = 0
        self.type = 0
        self.refineSet = 0
        self.oldVnum = 0
        self.materials = []
        self.enhanceMaterials = []

    def BindInterface(self, interface):
        from _weakref import proxy

        self.interface = proxy(interface)

    def __LoadScript(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/refinedialog.py")

        self.board = self.GetChild("Board")
        self.titleBar = self.GetChild("TitleBar")
        self.costText = self.GetChild("Cost")
        self.costText.SetMultiLine()
        self.GetChild("AcceptButton").SetEvent(self.OpenQuestionDialog)
        self.GetChild("CancelButton").SetEvent(self.CancelRefine)

        toolTip = uiToolTip.ItemToolTip()
        toolTip.SetParent(self)
        toolTip.SetFollow(False)
        toolTip.SetPosition(15, 38)
        toolTip.Show()
        self.toolTip = toolTip

        self.slotList = []
        for i in xrange(4):
            slot = self.__MakeSlot()
            slot.SetParent(toolTip)
            slot.SetVerticalAlignCenter()
            self.slotList.append(slot)

        itemImage = ui.ImageBox()
        itemImage.SetParent(toolTip)
        itemImage.SetVerticalAlignCenter()
        itemImage.SetPosition(-35, 0)
        self.itemImage = itemImage
        self.itemImage.Show()

        self.titleBar.SetCloseEvent(self.CancelRefine)

        self.checkBox = ui.ExpandedCheckBox()
        self.checkBox.SetParent(self.board)
        self.checkBox.SetPosition(0, 70)
        self.checkBox.SetHorizontalAlignCenter()
        self.checkBox.SetVerticalAlignBottom()
        self.checkBox.SetEvent(Event(self.AutoRefine))
        self.checkBox.SetText(localeInfo.REFINE_KEEP_WINDOW_OPEN)
        self.checkBox.SetChecked(constInfo.IS_AUTO_REFINE)
        self.checkBox.Show()

        self.isLoaded = True

    def __MakeSlot(self):
        slot = ui.ImageBox()
        slot.LoadImage("d:/ymir work/ui/public/slot_base.sub")
        slot.Show()
        self.children.append(slot)
        return slot

    def Destroy(self):
        self.ClearDictionary()
        self.dlgQuestion = None
        self.board = 0
        self.costText = None
        self.titleBar = 0
        self.toolTip = 0
        self.slotList = []
        self.children = []

    def UpdateMaterials(self):
        if not self.isLoaded:
            return

        for material in self.materials:
            proto = itemManager().GetProto(material.GetVnum())
            if proto:
                itemCount = playerInst().GetItemCountByVnum(material.GetVnum())
                if material.GetVnum() == self.oldVnum:
                    itemCount = itemCount - 1

                if itemCount >= material.GetCount():
                    material.GetTextLine().SetPackedFontColor(0xFFB3EE3A)
                else:
                    material.GetTextLine().SetPackedFontColor(0xFFEE3B3B)

                material.GetTextLine().SetText(
                    "%s x%02d (%d)" % (proto.GetName(), material.GetCount(), itemCount)
                )

        for enhanceMaterial in self.enhanceMaterials:
            proto = itemManager().GetProto(enhanceMaterial.GetVnum())
            if proto:
                itemCount = playerInst().GetItemCountByVnum(enhanceMaterial.GetVnum())

                if enhanceMaterial.GetVnum() == self.oldVnum:
                    itemCount = itemCount - 1

                if itemCount >= enhanceMaterial.GetCount():
                    enhanceMaterial.GetTextLine().SetPackedFontColor(0xFFB3EE3A)
                else:
                    enhanceMaterial.GetTextLine().SetPackedFontColor(0xFFEE3B3B)

                enhanceMaterial.GetTextLine().SetText(
                    "%s x%02d (%d) Bonus+%d%%"
                    % (
                        proto.GetName(),
                        enhanceMaterial.GetCount(),
                        itemCount,
                        enhanceMaterial.GetEnhanceProb(),
                    )
                )

        if self.costText is not None:
            if player.GetMoney() >= self.cost:
                self.costText.SetPackedFontColor(0xFFB3EE3A)
            else:
                self.costText.SetPackedFontColor(0xFFEE3B3B)

    def Open(self, targetItemPos, nextGradeItemVnum, cost, prob, type):
        if False == self.isLoaded:
            self.__LoadScript()

        self.__InitializeOpen()

        self.targetItemPos = targetItemPos
        self.oldVnum = player.GetItemIndex(targetItemPos)
        self.vnum = nextGradeItemVnum
        item.SelectItem(self.oldVnum)
        self.refineSet = item.GetRefineSet()
        self.cost = cost
        self.percentage = prob
        self.type = type

        self.costText.SetText(
            localeInfo.REFINE_COST_AND_PERC.format(
                self.percentage, localeInfo.NumberToString(self.cost)
            )
        )
        self.costText.SetPosition(35, 40)

        if player.GetMoney() >= self.cost:
            self.costText.SetPackedFontColor(0xFFB3EE3A)
        else:
            self.costText.SetPackedFontColor(0xFFEE3B3B)

        self.toolTip.ClearToolTip()
        metinSlot = []
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            metinSlot.append(player.GetItemMetinSocket(targetItemPos, i))

        attrSlot = []
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attrSlot.append(player.GetItemAttribute(targetItemPos, i))
        self.toolTip.AddRefineItemData(nextGradeItemVnum, metinSlot, attrSlot)

        item.SelectItem(nextGradeItemVnum)
        self.itemImage.LoadImage(item.GetIconImageFileName())
        xSlotCount, ySlotCount = item.GetItemSize()

        for slot in self.slotList:
            slot.Hide()

        for i in xrange(min(3, ySlotCount)):
            self.slotList[i].SetPosition(-35, i * 64 - (ySlotCount - 1) * 34)
            self.slotList[i].Show()

        self.scrollWindow = ui.ScrollWindow()
        self.scrollWindow.SetSize(288, 160)
        self.scrollWindow.SetWindowName("TargetInfoScroll")
        self.scrollWindow.SetParent(self)
        self.scrollWindow.SetPosition(0, 0)
        from ui_modern_controls import AutoGrowingVerticalContainerEx

        self.itemListBox = AutoGrowingVerticalContainerEx()
        self.itemListBox.SetWidth(288)

        self.scrollWindow.SetContentWindow(self.itemListBox)
        self.scrollWindow.FitWidth(288)
        self.scrollWindow.SetParent(self)
        self.scrollWindow.Show()
        self.itemListBox.Show()
        self.children.append(self.scrollWindow)
        self.children.append(self.itemListBox)

        self.dialogHeight = self.toolTip.GetHeight() + 46
        self.scrollWindow.SetPosition(0, self.dialogHeight)
        self.dialogHeight = 6 * 38 + self.dialogHeight - 80

        self.UpdateDialog()

        self.SetTop()
        self.SetFocus()
        self.Show()

    def Close(self):
        self.dlgQuestion = None
        if app.WJ_ENABLE_TRADABLE_ICON and self.interface:
            self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
            self.interface.RefreshMarkInventoryBag()
        self.Hide()

    def AppendMaterial(self, vnum, count):
        self.scrollWindow.Show()
        refineMaterial = RefineMaterialWindow()
        refineMaterial.SetItem(vnum, count)
        refineMaterial.SetParent(self.itemListBox)
        refineMaterial.Show()
        self.itemListBox.AppendItem(refineMaterial)
        self.materials.append(refineMaterial)
        self.UpdateMaterials()

    def AppendEnhanceMaterial(self, vnum, count, prob):
        self.scrollWindow.Show()
        refineMaterial = RefineMaterialWindow()
        refineMaterial.SetItem(vnum, count, prob)
        refineMaterial.SetParent(self.itemListBox)
        refineMaterial.Show()
        self.itemListBox.AppendItem(refineMaterial)
        self.enhanceMaterials.append(refineMaterial)
        self.UpdateMaterials()

    def UpdateDialog(self):
        newWidth = self.toolTip.GetWidth() + 75
        newHeight = self.dialogHeight + 101

        self.board.SetSize(newWidth, newHeight)
        self.toolTip.SetPosition(15 + 35, 38)
        self.titleBar.SetWidth(newWidth - 15)
        self.SetSize(newWidth, newHeight)

        (x, y) = self.GetLocalPosition()
        self.SetPosition(x, y)

    def OpenQuestionDialog(self):

        if self.refineSet == 1666:
            dlgQuestion = uiCommon.QuestionDialog2()
            dlgQuestion.SetText1(localeInfo.REFINE_SET_1666_WARNING_1)
            dlgQuestion.SetText2(localeInfo.REFINE_SET_1666_WARNING_2)
            dlgQuestion.SetAcceptEvent(self.Accept)
            dlgQuestion.SetCancelEvent(dlgQuestion.Close)
            dlgQuestion.Open()
            self.dlgQuestion = dlgQuestion
            return

        if 100 == self.percentage:
            self.Accept()
            return

        if 5 == self.type:
            self.Accept()
            return

        dlgQuestion = uiCommon.QuestionDialog2()
        dlgQuestion.SetText2(localeInfo.REFINE_WARNING2)
        dlgQuestion.SetAcceptEvent(self.Accept)
        dlgQuestion.SetCancelEvent(dlgQuestion.Close)

        if 3 == self.type:  ## ÇöÃ¶
            dlgQuestion.SetText1(localeInfo.REFINE_DESTROY_WARNING_WITH_BONUS_PERCENT_1)
            dlgQuestion.SetText2(localeInfo.REFINE_DESTROY_WARNING_WITH_BONUS_PERCENT_2)
        elif 2 == self.type:  ## Ãàº¹¼­
            dlgQuestion.SetText1(localeInfo.REFINE_DOWN_GRADE_WARNING)
        else:
            dlgQuestion.SetText1(localeInfo.REFINE_DESTROY_WARNING)

        dlgQuestion.Open()
        self.dlgQuestion = dlgQuestion

    def Accept(self):
        appInst.instance().GetNet().SendRefinePacket(self.targetItemPos, self.type)

    def CancelRefine(self):
        appInst.instance().GetNet().SendRefinePacket(255, 255)
        self.Close()
        constInfo.AUTO_REFINE_TYPE = 0
        constInfo.AUTO_REFINE_DATA = {"ITEM": [-1, -1], "NPC": [0, -1, -1, 0]}

    def AutoRefine(self, autoFlag):
        constInfo.IS_AUTO_REFINE = autoFlag

    def CheckRefine(self, isFail):
        if constInfo.IS_AUTO_REFINE == True:
            if constInfo.AUTO_REFINE_TYPE == 1:
                if (
                    constInfo.AUTO_REFINE_DATA["ITEM"][0] != -1
                    and constInfo.AUTO_REFINE_DATA["ITEM"][1] != -1
                ):
                    scrollPos = constInfo.AUTO_REFINE_DATA["ITEM"][0]
                    itemPos = constInfo.AUTO_REFINE_DATA["ITEM"][1]
                    scrollIndex = playerInst().GetItemIndex(scrollPos)
                    itemIndex = playerInst().GetItemIndex(itemPos)
                    proto = itemManager().GetProto(itemIndex)
                    if not proto:
                        self.Close()
                        return

                    indexCheck = itemIndex % 10 == 8
                    if proto.GetType() == item.ITEM_TYPE_METIN:
                        indexCheck = ((itemIndex / 100) % 10) == 8

                    if scrollIndex == 0 or (indexCheck and not isFail):
                        self.Close()
                    else:
                        appInst.instance().GetNet().SendItemUseToItemPacket(
                            constInfo.AUTO_REFINE_DATA["ITEM"][0],
                            constInfo.AUTO_REFINE_DATA["ITEM"][1],
                        )

            elif constInfo.AUTO_REFINE_TYPE == 2:
                npcData = constInfo.AUTO_REFINE_DATA["NPC"]
                if (
                    npcData[0] != 0
                    and npcData[1] != -1
                    and npcData[2] != -1
                    and npcData[3] != 0
                ):
                    itemIndex = playerInst().GetItemIndex(
                        MakeItemPosition(npcData[1], npcData[2])
                    )
                    proto = itemManager().GetProto(itemIndex)
                    if not proto:
                        self.Close()
                        return

                    itemType = proto.GetType()
                    indexCheck = itemIndex % 10 == 8
                    if itemType == item.ITEM_TYPE_METIN:
                        indexCheck = (itemIndex % 100) % 10 == 8

                    if (indexCheck and not isFail) or isFail:
                        self.Close()
                    else:
                        appInst.instance().GetNet().SendGiveItemPacket(
                            npcData[0],
                            MakeItemPosition(npcData[1], npcData[2]),
                            npcData[3],
                        )
            else:
                self.Close()
        else:
            self.Close()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.CancelRefine()
            return True

    if app.WJ_ENABLE_TRADABLE_ICON:

        def HasntEnough(self, slotPos):
            if self.interface is not None:
                itemIndex = player.GetItemIndex(slotPos)
                itemCount = playerInst().GetItemCountByVnum(itemIndex)

                for material in self.materials:
                    if material.GetVnum() == self.oldVnum:
                        if (
                            material.GetVnum() == itemIndex
                            and itemCount - 1 >= material.GetCount()
                        ):
                            return True
                    else:
                        if (
                            material.GetVnum() == itemIndex
                            and itemCount >= material.GetCount()
                        ):
                            return True

            return False

        def OnSetFocus(self):
            if self.interface is not None:
                self.interface.SetOnTopWindow(player.ON_TOP_WND_REFINE)
                self.interface.RefreshMarkInventoryBag()
