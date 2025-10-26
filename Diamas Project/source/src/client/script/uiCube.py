# -*- coding: utf-8 -*-

import logging

import app
import grp
import item
import player
from pygame.app import appInst
from pygame.cube import cubeMgr
from pygame.item import itemManager

import localeInfo
import mouseModule
import ui
from ui_event import MakeEvent


class CubeRenewalItem(ui.Window):

    def __init__(self, getParentEvent):
        ui.Window.__init__(self)
        self.SetParent(getParentEvent().groupContainer)

        self.index = -1
        self.type = -1

        self.count_all = -1

        self.cubeData = None

        self.index = -1

        self.button = ui.Button()
        self.button.SetParent(self)
        self.button.SetEvent(self.Select)
        self.button.Show()

        self.image = ui.ImageBox()
        self.image.SetParent(self)
        self.image.SetPosition(7, 6)
        self.image.Show()

        self.text = ui.TextLine()
        self.text.SetParent(self)
        self.text.SetPosition(40, 2)
        self.text.SetFontName("Verdana:12")
        self.text.Show()

        self.count_total = ui.TextLine()
        self.count_total.SetParent(self)
        self.count_total.SetPosition(16, 2)
        self.count_total.Hide()

        self.getParentEvent = MakeEvent(getParentEvent)

        self.SetSize(280, 25)

    def SetName(self, name):
        self.name = name
        self.text.SetText(name)

    def SetCountTotal(self, count):
        self.count_all = count

        if count <= 0:
            self.count_total.Hide()
            self.text.SetPosition(17, 2)
            return

        self.count_total.SetText("[%d]" % (count * self.GetCountReward()))
        self.count_total.SetPackedFontColor(
            grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)
        )
        self.count_total.Show()

        w, h = (self.count_total.GetWidth(), self.count_total.GetHeight())
        self.text.SetPosition(w + 20, 2)

    def GetCount(self):
        return self.count_all

    def SetType(self, type):
        self.type = type

    def AppendIndex(self, recipe):
        self.index = recipe.vnum
        self.cubeData = recipe

    def GetIndex(self):
        return self.index

    def GetVnumReward(self):
        return self.cubeData.reward.vnum

    def GetCountReward(self):
        return self.cubeData.reward.count

    def GetItemStackable(self):
        proto = itemManager().GetProto(self.cubeData.reward.vnum)
        if proto:
            return proto.IsFlag(item.ITEM_FLAG_STACKABLE)
        else:
            return False

    def GetVnumMaterial(self, index):
        try:
            return self.cubeData.items[index].vnum
        except:
            return 0

    def GetCountMaterial(self, index):
        try:
            return self.cubeData.items[index].count
        except:
            return 0

    def GetGold(self):
        return self.cubeData.cost

    def GetProbability(self):
        return self.cubeData.probability

    def GetName(self):
        return self.name

    def GetType(self):
        return self.type

    def GetStepWidth(self):
        return 0

    def Select(self):
        if self.getParentEvent:
            self.getParentEvent().OnSelectItem(self)


class CubeRenewalCategoryItem(CubeRenewalItem):

    def __init__(self, getParentEvent):
        self.OpenFunc = False
        self.SubCategorys = []
        self.index = -1

        CubeRenewalItem.__init__(self, getParentEvent)

        self.button.SetUpVisual("d:/ymir work/ui/game/cube/cube_menu_tab1.sub")
        self.button.SetOverVisual("d:/ymir work/ui/game/cube/cube_menu_tab1.sub")
        self.button.SetDownVisual("d:/ymir work/ui/game/cube/cube_menu_tab1.sub")
        # self.text.SetPackedFontColor(0xffffce3c)
        self.text.SetOutline()
        self.SetType(1)

    def SetCategory(self, index, name):
        self.SetName(name)
        self.IndexCategory(index)

    def AppendSubCategorys(self, recipe):
        subCategorys = CubeRenewalSubCategoryItem(self.getParentEvent)
        subCategorys.AppendIndex(recipe)
        subCategorys.LoadInfo()
        subCategorys.Select()
        self.SubCategorys.append(subCategorys)

    def GetListSubCategorys(self):
        return self.SubCategorys

    def IndexCategory(self, index):
        self.index = index

    def GetIndexCategory(self):
        return self.index

    def FindVnumSubCategory(self, key):
        list = filter(
            lambda argMember, argKey=key: argMember.IsSameVnum(argKey),
            self.SubCategorys,
        )
        if list:
            return list[0]

        return None

    def Open(self):
        self.image.LoadImage("d:/ymir work/ui/game/cube/cube_menu_tab1_minus.sub")
        self.OpenFunc = True

    def Close(self):
        self.image.LoadImage("d:/ymir work/ui/game/cube/cube_menu_tab1_plus.sub")
        self.OpenFunc = False
        map(ui.Window.Hide, self.SubCategorys)

    def IsOpen(self):
        return self.OpenFunc

    def Select(self):
        if self.OpenFunc:
            self.Close()
        else:
            self.Open()

        for x in self.GetListSubCategorys():
            x.Select()

        if self.getParentEvent:
            self.getParentEvent().OnRefresh()


class CubeRenewalSubCategoryItem(CubeRenewalItem):

    def __init__(self, getParentEvent):

        self.OpenFunc = True
        self.SubCategoryObjects = []

        CubeRenewalItem.__init__(self, getParentEvent)

        self.button.SetUpVisual("d:/ymir work/ui/game/cube/cube_menu_tab2.sub")
        self.button.SetOverVisual("d:/ymir work/ui/game/cube/cube_menu_tab2.sub")
        self.button.SetDownVisual("d:/ymir work/ui/game/cube/cube_menu_tab2.sub")

        self.image.SetPosition(7, 4)
        self.text.SetPosition(25, 2)
        # self.text.SetPackedFontColor(0xffffce3c)

        self.SetType(2)

    def LoadInfo(self):
        item.SelectItem(self.GetVnumReward())
        self.SetName(item.GetItemName())

    def AppendSubCategoryObjects(self, recipe):
        subCategoryobj = CubeRenewalSubCategoryObjects(self.getParentEvent)
        subCategoryobj.AppendIndex(recipe)
        subCategoryobj.LoadInfo()
        self.SubCategoryObjects.append(subCategoryobj)

    def IsSameVnum(self, vnum):
        return self.GetVnumReward() == vnum

    def Open(self):
        self.image.LoadImage("d:/ymir work/ui/game/cube/cube_menu_tab2_minus.sub")
        self.OpenFunc = True

    def Close(self):
        self.image.LoadImage("d:/ymir work/ui/game/cube/cube_menu_tab2_plus.sub")
        self.OpenFunc = False
        map(ui.Window.Hide, self.SubCategoryObjects)

    def IsOpen(self):
        return self.OpenFunc

    def GetListSubCategorysObjects(self):
        return self.SubCategoryObjects

    def Select(self):
        if self.OpenFunc:
            self.Close()
        else:
            self.Open()

        self.getParentEvent().OnRefresh()


class CubeRenewalSubCategoryObjects(CubeRenewalItem):
    def __init__(self, getParentEvent):
        CubeRenewalItem.__init__(self, getParentEvent)

        self.button.SetUpVisual("d:/ymir work/ui/game/cube/cube_menu_tab3_default.sub")
        self.button.SetOverVisual("d:/ymir work/ui/game/cube/cube_menu_tab3_select.sub")
        self.button.SetDownVisual("d:/ymir work/ui/game/cube/cube_menu_tab3_select.sub")

        # self.text.SetPackedFontColor(0xff985856)
        self.SetType(3)
        self.text.SetPosition(17, 2)

    def LoadInfo(self):
        item.SelectItem(self.GetVnumReward())
        self.SetName(item.GetItemName() + " (%d%%)" % (self.GetProbability()))
        self.CountTotal()

    def CountTotal(self):
        count_items = []
        count_all_items = 0

        for i in xrange(1, 6):
            if self.GetVnumMaterial(i - 1) != 0:
                if player.GetItemCountByVnum(
                    self.GetVnumMaterial(i - 1)
                ) >= self.GetCountMaterial(i - 1):
                    number = player.GetItemCountByVnum(self.GetVnumMaterial(i - 1)) / (
                        self.GetCountMaterial(i - 1)
                    )
                    count_items.append(number)
                count_all_items += 1

        if len(count_items) >= count_all_items:
            self.SetCountTotal(min(count_items))
        else:
            self.SetCountTotal(0)

    def UpdateProbability(self):
        item.SelectItem(self.GetVnumReward())
        Probability_actual = 0

        if self.GetProbabilityUpdate() != 0:
            if self.GetProbability() + self.GetProbabilityUpdate() > 100:
                Probability_actual = 100
            else:
                Probability_actual = self.GetProbability() + self.GetProbabilityUpdate()
        else:
            Probability_actual = self.GetProbability()

        self.SetName(item.GetItemName() + " (%d%%)" % Probability_actual)

    def GetProbabilityUpdate(self):
        return self.getParentEvent().GetProbability()

    def UnSelect(self):
        self.button.SetUp()
        self.button.Enable()

    def SetSelect(self):
        self.button.Down()
        self.button.Disable()

    def Select(self):
        self.getParentEvent().SelectObject(self)


class CubeWindow(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.xShopStart = 0
        self.yShopStart = 0
        self.isUsable = False
        self.selectedItem = None
        self.categoryList = []
        self.cubeElements = {}
        self.showingItemList = []
        self.vnum_item_improve = 79605
        self.max_count_item_improve = 40
        self.slot_item_improve = -1
        self.loaded = False
        self.categories = [
            [0, "POTION", localeInfo.Get("CUBE_RENEWAL_POTION")],
            [1, "WEAPON", localeInfo.Get("CUBE_RENEWAL_WEAPON")],
            [2, "ARMOR", localeInfo.Get("CUBE_RENEWAL_ARMOR")],
            [3, "ACCESSORY", localeInfo.Get("CUBE_RENEWAL_ACCESSORIES")],
            [4, "ETC", localeInfo.Get("CUBE_RENEWAL_ETC")],
        ]

        self.tooltipItem = None

    def LoadWindow(self):
        if self.loaded:
            return
        logging.debug("Load BattlePassWindow")

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/CubeRenewalWindow.py")

        GetObject = self.GetChild
        self.board = GetObject("board")
        self.GetChild("board").SetCloseEvent(self.__OnCloseButtonClick)
        for i in xrange(1, 6):
            self.cubeElements["count_elements_%d" % i] = self.GetChild(
                "material_qty_text_%d" % i
            )

        self.btnAccept = self.GetChild("button_ok")
        self.btnCancel = self.GetChild("button_cancel")

        self.needMoney = self.GetChild("yang_text")
        self.contentScrollbar = self.GetChild("cube_list_scroll_bar")
        self.contentScrollbar.Hide()
        self.result_qty = self.GetChild("result_qty")
        self.board = self.GetChild("item_list_board")
        self.item_slot = self.GetChild("item_slot")
        self.qty_sub_button = self.GetChild("qty_sub_button")
        self.qty_add_button = self.GetChild("qty_add_button")

        self.imporve_slot = self.GetChild("imporve_slot")
        self.GetChild("impove_text_window").Hide()

        self.result_qty.SetReturnEvent(self.AceptInputItem)
        self.icons_items = ui.SlotWindow()
        self.icons_items.SetParent(self.item_slot)
        self.icons_items.SetPosition(25, 13)
        self.icons_items.SetSize((32 * 9), 32 * 3)
        self.icons_items.SetOverInItemEvent(self.OverInItem)
        self.icons_items.SetOverOutItemEvent(self.OverOutItem)
        self.icons_items.AppendSlot(0, 0, 0, 32, 32 * 3)

        for i in xrange(0, 6):
            self.icons_items.AppendSlot(i + 1, 46 * i + 62, 0, 32, 32 * 3)
        self.icons_items.Show()

        self.slot_improve = ui.SlotWindow()
        self.slot_improve.SetParent(self.imporve_slot)
        self.slot_improve.SetSize(32, 32)
        self.slot_improve.SetPosition(6, 5)
        self.slot_improve.SetSelectEmptySlotEvent(self.__OnSelectEmptySlot)
        self.slot_improve.SetSelectItemSlotEvent(self.__OnSelectItemSlot)
        self.slot_improve.AppendSlot(0, 0, 0, 32, 32)
        self.slot_improve.Hide()

        self.imporve_slot.Hide()
        # self.board.SetCloseEvent(self.__OnCloseButtonClick)
        self.btnCancel.SetEvent(self.__OnCloseButtonClick)
        self.btnAccept.SetEvent(self.__OnAcceptButtonClick)
        self.qty_sub_button.SetEvent(self.QtySubButton)
        self.qty_add_button.SetEvent(self.QtyAddButton)

        self.groupContainer = ui.Window()
        self.groupContainer.SetSize(313, 999)
        self.groupContainer.AddFlag("not_pick")
        self.groupContainer.Show()

        self.scrollWindow = ui.ScrollWindow()
        self.scrollWindow.SetSize(313, 246)
        self.scrollWindow.SetContentWindow(self.groupContainer)
        self.scrollWindow.AddFlag("not_pick")
        self.scrollWindow.SetParent(self.board)
        self.scrollWindow.Show()
        self.scrollWindow.FitWidth(313)
        self.loaded = True

    def SetItemToolTip(self, itemTooltip):
        self.tooltipItem = itemTooltip

    def Destroy(self):
        self.ClearDictionary()

    def Open(self):
        self.LoadWindow()
        self.Refresh()
        self.Show()
        self.isUsable = True
        (self.xShopStart, self.yShopStart, z) = player.GetMainCharacterPosition()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.__OnCloseButtonClick()
            return True

    def Close(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.Hide()

        if self.needMoney:
            self.needMoney.SetText("0")

    def Clear(self):
        self.Refresh()

    def Refresh(self):
        self.ClearElements()
        self.LoadCategory()
        self.LoadSubCategory()
        self.LoadSubObjects()

    def __OnCloseButtonClick(self):
        if self.isUsable:
            self.isUsable = False

            appInst.instance().GetNet().SendChatPacket("/cube close")

        self.Close()

    def __OnAcceptButtonClick(self):
        if not self.selectedItem:
            return
        for i in xrange(self.count_item_reward):
            appInst.instance().GetNet().SendChatPacket(
                "/cube make {} {}".format(
                    self.selectedItem.GetIndex(), self.count_item_reward
                )
            )

    def GetSelf(self):
        return self

    def GetBoard(self):
        return self.board

    def OverInItem(self, index):
        self.tooltipItem.ClearToolTip()

        vnum = 0
        if self.selectedItem:
            if index == 0:
                vnum = self.selectedItem.GetVnumReward()
            else:
                vnum = self.selectedItem.GetVnumMaterial(index - 1)

        if vnum:
            metinSlot = []
            for i in xrange(player.METIN_SOCKET_MAX_NUM):
                metinSlot.append(0)

            self.tooltipItem.AddItemData(vnum, metinSlot, 0)

    def OverOutItem(self):
        self.tooltipItem.Hide()

    def OnRefresh(self):
        self.showingItemList = []

        for group in self.categoryList:
            self.showingItemList.append(group)

            if group.IsOpen():
                for Categorys in group.GetListSubCategorys():
                    self.showingItemList.append(Categorys)

                    if Categorys.IsOpen():
                        for a in Categorys.GetListSubCategorysObjects():
                            self.showingItemList.append(a)

        self.SetExtraInfo()

    def ClearElements(self):
        self.selectedItem = None
        if len(self.showingItemList) > 0:
            map(ui.Window.Hide, self.showingItemList)

        for group in self.categoryList:
            group.Close()

            for Categorys in group.GetListSubCategorys():
                Categorys.Close()

        for i in xrange(0, 6):
            self.icons_items.ClearSlot(i)
            if i > 0:
                self.cubeElements["count_elements_%d" % i].SetText("")
                self.cubeElements["count_elements_%d" % i].Hide()

        self.needMoney.Hide()
        self.result_qty.Hide()

        self.categoryList = []

    def SetExtraInfo(self):
        map(ui.Window.Hide, self.showingItemList)

        yPos = 11

        for item in self.showingItemList:
            XPos = 0
            if item.GetType() == 2:
                XPos += 15
            if item.GetType() == 3:
                XPos += 35

            item.SetPosition(6 + XPos, yPos)
            item.SetTop()
            item.Show()

            yPos += 24
            self.groupContainer.SetSize(self.groupContainer.GetWidth(), yPos)

    def CheckListCategorysTotal(self):
        categoryIdSet = set()
        for recipe in cubeMgr().GetCubeProto():
            categoryIdSet.add(recipe.category)

        return categoryIdSet

    def LoadCategory(self):
        world = self.CheckListCategorysTotal()
        for i in world:
            cat = next((x for x in self.categories if x[0] == i), None)
            if cat:
                member = CubeRenewalCategoryItem(self.GetSelf)
                member.SetCategory(cat[0], cat[2])
                member.Close()
                member.Show()
                self.categoryList.append(member)

    def CheckCategoryIndex(self, name):
        vnum_default = 0
        for i in xrange(len(self.categories)):
            if name == self.categories[i][0]:
                return self.categories[i][0]

    def LoadSubCategory(self):
        for group in self.categoryList:
            for i, recipe in enumerate(cubeMgr().GetCubeProto()):
                if self.CheckCategoryIndex(recipe.category) == group.GetIndexCategory():
                    if not group.FindVnumSubCategory(recipe.reward.vnum):
                        group.AppendSubCategorys(recipe)

    def LoadSubObjects(self):
        for group in self.categoryList:
            for i, recipe in enumerate(cubeMgr().GetCubeProto()):
                if self.CheckCategoryIndex(recipe.category) == group.GetIndexCategory():
                    for Categorys in group.GetListSubCategorys():
                        if recipe.reward.vnum == Categorys.GetVnumReward():
                            Categorys.AppendSubCategoryObjects(recipe)

    def UpdateCountSubCategory(self):
        for group in self.categoryList:
            if group.IsOpen():
                for Categorys in group.GetListSubCategorys():
                    if Categorys.IsOpen():
                        for a in Categorys.GetListSubCategorysObjects():
                            a.CountTotal()
                            a.UpdateProbability()

    def UpdateSelectSubCategory(self):
        for group in self.categoryList:
            for Categorys in group.GetListSubCategorys():
                for a in Categorys.GetListSubCategorysObjects():
                    a.UnSelect()
        self.selectedItem.SetSelect()

    def OnSelectItem(self, item):
        self.selectedItem = item

        if self.selectedItem:
            self.selectedItem.Select()

    def GetProbability(self):
        index_improve = self.slot_item_improve
        if index_improve != -1:
            itemVNum = player.GetItemIndex(index_improve)
            itemCount = player.GetItemCount(index_improve)
            if (
                itemVNum == self.vnum_item_improve
                and itemCount <= self.max_count_item_improve
            ):
                return itemCount
        return 0

    def SelectObject(self, item):
        self.selectedItem = item
        if self.selectedItem:
            self.UpdateInfoSelect()
            self.UpdateSelectSubCategory()

    def UpdateInfoSelect(self):

        self.icons_items.ClearSlot(0)
        self.icons_items.SetItemSlot(
            0, self.selectedItem.GetVnumReward(), self.selectedItem.GetCountReward()
        )

        for i in xrange(1, 6):
            self.icons_items.ClearSlot(i)
            self.icons_items.SetItemSlot(
                i,
                self.selectedItem.GetVnumMaterial(i - 1),
                self.selectedItem.GetCountMaterial(i - 1),
            )

        self.result_qty.KillFocus()
        self.result_qty.CanEdit(self.selectedItem.GetItemStackable())
        self.result_qty.SetText("%s" % (self.selectedItem.GetCountReward()))
        self.result_qty.Show()

        self.count_item_reward = self.selectedItem.GetCountReward()

        self.UpdateMaterialCount()
        self.UpdateSlotItemImprove()

    def UpdateSlotItemImprove(self):
        if self.selectedItem.GetProbability() >= 100 and self.slot_item_improve != -1:
            self.slot_improve.SetItemSlot(0, 0, 0)
            self.slot_item_improve = -1

    def UpdateMaterialCount(self):
        count = self.count_item_reward / (self.selectedItem.GetCountReward())

        for i in xrange(1, 6):
            if self.selectedItem.GetVnumMaterial(i - 1) != 0:
                if player.GetItemCountByVnum(
                    self.selectedItem.GetVnumMaterial(i - 1)
                ) >= (self.selectedItem.GetCountMaterial(i - 1) * count):
                    self.cubeElements["count_elements_%d" % i].SetPackedFontColor(
                        grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)
                    )
                else:
                    self.cubeElements["count_elements_%d" % i].SetPackedFontColor(
                        grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)
                    )

                self.cubeElements["count_elements_%d" % i].SetText(
                    "%d"
                    % player.GetItemCountByVnum(
                        self.selectedItem.GetVnumMaterial(i - 1)
                    )
                )
                self.cubeElements["count_elements_%d" % i].Show()
            else:
                self.cubeElements["count_elements_%d" % i].Hide()

        self.needMoney.SetText(
            localeInfo.NumberToMoneyString(self.selectedItem.GetGold() * count)
        )
        self.needMoney.Show()

    def ConvertInputResult(self):
        edit = int(self.result_qty.GetText())
        total = self.selectedItem.GetCountReward()
        convert = edit % total
        return convert

    def AceptInputItem(self):
        self.result_qty.KillFocus()

        result_total = int(self.result_qty.GetText())

        if int(self.result_qty.GetText()) < self.selectedItem.GetCountReward():
            result_total = self.selectedItem.GetCountReward()
        else:
            result = self.ConvertInputResult()
            if result != 0:
                result_total = result_total - result

        self.count_item_reward = result_total
        self.result_qty.SetText("%d" % (result_total))

        count = self.count_item_reward / (self.selectedItem.GetCountReward())

        for i in xrange(0, 6):
            self.icons_items.ClearSlot(i)
            if i == 0:
                self.icons_items.SetItemSlot(
                    i, self.selectedItem.GetVnumReward(), self.count_item_reward
                )
            else:
                self.icons_items.SetItemSlot(
                    i,
                    self.selectedItem.GetVnumMaterial(i - 1),
                    self.selectedItem.GetCountMaterial(i - 1) * count,
                )

    def QtyAddButton(self):
        if self.selectedItem and self.selectedItem.GetItemStackable():
            self.result_qty.SetText(
                "%d"
                % (self.selectedItem.GetCountReward() * self.selectedItem.GetCount())
            )
            self.AceptInputItem()

    def QtySubButton(self):
        if self.selectedItem and self.selectedItem.GetItemStackable():
            self.result_qty.SetText("%d" % (self.selectedItem.GetCountReward()))
            self.AceptInputItem()

    def CheckInputFocus(self):
        if self.result_qty.IsFocus():
            self.result_qty.SetText("%d" % (self.selectedItem.GetCountReward()))
            self.AceptInputItem()

    def OnUpdate(self):
        if self.showingItemList:
            self.UpdateCountSubCategory()
        if self.selectedItem:
            self.UpdateMaterialCount()

        index_improve = self.slot_item_improve
        if index_improve != -1:
            itemVNum = player.GetItemIndex(index_improve)
            itemCount = player.GetItemCount(index_improve)
            # if itemVNum == self.vnum_item_improve and itemCount <= self.max_count_item_improve:
            #    self.slot_improve.SetItemSlot(0, itemVNum, itemCount)
            # else:
            #    self.slot_improve.SetItemSlot(0, 0, 0)
            #    self.slot_item_improve = -1
        else:
            # self.slot_improve.SetItemSlot(0, 0, 0)
            self.slot_item_improve = -1

    def __OnSelectEmptySlot(self, selectedSlotPos):
        isAttached = mouseModule.mouseController.isAttached()

        if not self.selectedItem:
            return

        if self.selectedItem.GetProbability() >= 100:
            return

        if isAttached:
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
            mouseModule.mouseController.DeattachObject()

            itemVNum = player.GetItemIndex(attachedSlotPos)
            itemCount = player.GetItemCount(attachedSlotPos)

            if (
                itemVNum == self.vnum_item_improve
                and itemCount <= self.max_count_item_improve
            ):
                self.slot_improve.SetItemSlot(selectedSlotPos, itemVNum, itemCount)
                self.slot_item_improve = attachedSlotPos

    def __OnSelectItemSlot(self, selectedSlotPos):
        isAttached = mouseModule.mouseController.isAttached()

        if not self.selectedItem:
            return

        if self.selectedItem.GetProbability() >= 100:
            return

        if isAttached:
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
            mouseModule.mouseController.DeattachObject()

            itemVNum = player.GetItemIndex(attachedSlotPos)
            itemCount = player.GetItemCount(attachedSlotPos)

            if (
                itemVNum == self.vnum_item_improve
                and itemCount <= self.max_count_item_improve
            ):
                self.slot_improve.SetItemSlot(selectedSlotPos, itemVNum, itemCount)
                self.slot_item_improve = attachedSlotPos
        else:
            self.slot_improve.SetItemSlot(0, 0, 0)
            self.slot_item_improve = -1
