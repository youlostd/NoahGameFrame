# -*- coding: utf-8 -*-


import logging

import app
import chat
import item
import player
import snd
from pygame.app import appInst
from pygame.item import IsItemUsedForDragonSoul
from pygame.item import ItemPosition
from pygame.item import MakeItemPosition
from pygame.player import OverItemSlot
from pygame.player import SetItemSlot
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
import uiCommon
import uiPrivateShopBuilder
import uiScriptLocale
from ui_event import Event

ITEM_FLAG_APPLICABLE = 1 << 14
import wndMgr


# ¿ëÈ¥¼® Vnum¿¡ ´ëÇÑ comment
# ITEM VNUMÀ» 10¸¸ ÀÚ¸®ºÎÅÍ, FEDCBA¶ó°í ÇÑ´Ù¸é
# FE : ¿ëÈ¥¼® Á¾·ù.    D : µî±Þ
# C : ´Ü°è            B : °­È­        
# A : ¿©¹úÀÇ ¹øÈ£µé...    

class DragonSoulWindow(ui.ScriptWindow):
    KIND_TAP_TITLES = [uiScriptLocale.DRAGONSOUL_TAP_TITLE_1, uiScriptLocale.DRAGONSOUL_TAP_TITLE_2,
                       uiScriptLocale.DRAGONSOUL_TAP_TITLE_3, uiScriptLocale.DRAGONSOUL_TAP_TITLE_4,
                       uiScriptLocale.DRAGONSOUL_TAP_TITLE_5, uiScriptLocale.DRAGONSOUL_TAP_TITLE_6]

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.questionDialog = None
        self.tooltipItem = None
        self.sellingSlotNumber = -1
        self.isLoaded = 0
        self.isActivated = False
        self.DSKindIndex = 0
        self.tabDict = None
        self.tabButtonDict = None
        self.deckPageIndex = 0
        self.inventoryPageIndex = 0
        self.SetWindowName("DragonSoulWindow")
        self.interface = None
        self.setGrade = 0
        self.listHighlightedSlot = []
        self.deckTab = []
        self.nextStatusChange = 0
        self.dlgQuestion = None
        self.wndPopupDialog = None

        self.inventoryTab = []

    def Show(self):
        self.__LoadWindow()

        ui.ScriptWindow.Show(self)
        self.SetTop()

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return
        self.isLoaded = 1
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/DragonSoulWindow.py")

        except:
            logging.exception("dragonsoulwindow.LoadWindow.LoadObject")
        try:
            if localeInfo.IsARABIC():
                self.board = self.GetChild("Equipment_Base")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_01")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_02")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_03")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_04")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_05")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)
                self.board = self.GetChild("Tab_06")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)

            wndItem = self.GetChild("ItemSlot")
            wndEquip = self.GetChild("EquipmentSlot")
            self.activateButton = self.GetChild("activate")
            self.refineButton = self.GetChild("refine_button")
            self.deckTab = []
            self.deckTab.append(self.GetChild("deck1"))
            self.deckTab.append(self.GetChild("deck2"))
            self.GetChild("TitleBar").SetCloseEvent(self.Close)
            self.inventoryTab = []
            self.inventoryTab.append(self.GetChild("Inventory_Tab_01"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_02"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_03"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_04"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_05"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_06"))
            self.inventoryTab.append(self.GetChild("Inventory_Tab_07"))
            self.tabDict = {
                0: self.GetChild("Tab_01"),
                1: self.GetChild("Tab_02"),
                2: self.GetChild("Tab_03"),
                3: self.GetChild("Tab_04"),
                4: self.GetChild("Tab_05"),
                5: self.GetChild("Tab_06"),
            }
            self.tabButtonDict = {
                0: self.GetChild("Tab_Button_01"),
                1: self.GetChild("Tab_Button_02"),
                2: self.GetChild("Tab_Button_03"),
                3: self.GetChild("Tab_Button_04"),
                4: self.GetChild("Tab_Button_05"),
                5: self.GetChild("Tab_Button_06"),
            }
            self.tabText = self.GetChild("tab_text_area")
        except:
            logging.exception("InventoryWindow.LoadWindow.BindObject")
        ## DragonSoul Kind Tap
        for (tabKey, tabButton) in self.tabButtonDict.items():
            tabButton.SetEvent(Event(self.SetDSKindIndex, tabKey))
        ## Item
        wndItem.SetOverInItemEvent(self.OverInItem)
        wndItem.SetOverOutItemEvent(self.OverOutItem)
        wndItem.SetSelectItemSlotEvent(self.SelectItemSlot)
        wndItem.SetSelectEmptySlotEvent(self.SelectEmptySlot)
        wndItem.SetUnselectItemSlotEvent(self.UseItemSlot)
        wndItem.SetUseSlotEvent(self.UseItemSlot)

        ## Equipment
        wndEquip.SetSelectEmptySlotEvent(self.SelectEmptyEquipSlot)
        wndEquip.SetSelectItemSlotEvent(self.SelectEquipItemSlot)
        wndEquip.SetUnselectItemSlotEvent(self.UseEquipItemSlot)
        wndEquip.SetUseSlotEvent(self.UseEquipItemSlot)
        wndEquip.SetOverInItemEvent(self.OverInEquipItem)
        wndEquip.SetOverOutItemEvent(self.OverOutEquipItem)

        ## Deck
        self.deckTab[0].SetToggleDownEvent(Event(self.SetDeckPage, 0))
        self.deckTab[1].SetToggleDownEvent(Event(self.SetDeckPage, 1))
        self.deckTab[0].SetToggleUpEvent(Event(self.__DeckButtonDown, 0))
        self.deckTab[1].SetToggleUpEvent(Event(self.__DeckButtonDown, 1))
        self.deckTab[0].Down()
        ## Grade button
        self.inventoryTab[0].SetEvent(Event(self.SetInventoryPage, 0))
        self.inventoryTab[1].SetEvent(Event(self.SetInventoryPage, 1))
        self.inventoryTab[2].SetEvent(Event(self.SetInventoryPage, 2))
        self.inventoryTab[3].SetEvent(Event(self.SetInventoryPage, 3))
        self.inventoryTab[4].SetEvent(Event(self.SetInventoryPage, 4))
        self.inventoryTab[5].SetEvent(Event(self.SetInventoryPage, 5))
        self.inventoryTab[6].SetEvent(Event(self.SetInventoryPage, 6))

        self.inventoryTab[0].Down()
        ## Etc
        self.wndItem = wndItem
        self.wndEquip = wndEquip
        self.dlgQuestion = None

        self.dlgQuestion = uiCommon.QuestionDialog2()
        self.dlgQuestion.Close()

        self.activateButton.SetToggleDownEvent(self.ActivateButtonClick)
        self.activateButton.SetToggleUpEvent(self.ActivateButtonClick)

        self.refineButton.SetEvent(self.OpenRefineDialog)
        self.wndPopupDialog = uiCommon.PopupDialog()

        ##
        self.listHighlightedSlot = []

        ## Refresh
        self.SetInventoryPage(0)
        self.RefreshItemSlot()
        self.RefreshEquipSlotWindow()
        self.RefreshBagSlotWindow()
        self.SetDSKindIndex(0)
        self.activateButton.Enable()
        self.deckTab[self.deckPageIndex].Down()
        self.activateButton.SetUp()

    def Destroy(self):
        self.ClearDictionary()
        self.tooltipItem = None
        self.wndItem = 0
        self.wndEquip = 0
        self.activateButton = None
        self.questionDialog = None
        self.mallButton = None
        self.inventoryTab = []
        self.deckTab = []
        self.equipmentTab = []
        self.tabDict = None
        self.tabButtonDict = None

    def OpenRefineDialog(self):
        if self.interface:
            self.interface.OpenDragonSoulRefineWindow()

    def Close(self):
        if None != self.tooltipItem:
            self.tooltipItem.HideToolTip()
        self.Hide()

    def __DeckButtonDown(self, deck):
        self.deckTab[deck].Down()

    def SetInventoryPage(self, page):
        if self.inventoryPageIndex != page:
            self.__HighlightSlot_ClearCurrentPage()

        self.inventoryTab[self.inventoryPageIndex].SetUp()
        self.inventoryPageIndex = page
        self.inventoryTab[self.inventoryPageIndex].Down()
        self.RefreshBagSlotWindow()

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def RefreshItemSlot(self):
        self.RefreshBagSlotWindow()
        self.RefreshEquipSlotWindow()

    def RefreshEquipSlotWindow(self):
        if not self.isLoaded:
            return

        for i in xrange(6):
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.EQUIPMENT,
                                                                     player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i)
            itemVnum = playerInst().GetItemIndex(MakeItemPosition(player.EQUIPMENT, slotNumber))
            self.wndEquip.SetItemSlot(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i, itemVnum, 0)
            self.wndEquip.EnableSlot(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i)

            if itemVnum != 0:
                item.SelectItem(itemVnum)
                for j in xrange(item.LIMIT_MAX_NUM):
                    (limitType, limitValue) = item.GetLimit(j)

                    # ¹Ø¿¡¼­ remain_timeÀÌ 0ÀÌÇÏÀÎÁö Ã¼Å© ÇÏ±â ¶§¹®¿¡ ÀÓÀÇÀÇ ¾ç¼ö·Î ÃÊ±âÈ­
                    remain_time = 999
                    # ÀÏ´Ü ÇöÀç Å¸ÀÌ¸Ó´Â ÀÌ ¼¼°³ »ÓÀÌ´Ù.
                    if item.LIMIT_REAL_TIME == limitType:
                        remain_time = player.GetItemMetinSocket(player.EQUIPMENT, slotNumber,
                                                                0) - app.GetGlobalTimeStamp()
                    elif item.LIMIT_REAL_TIME_START_FIRST_USE == limitType:
                        remain_time = player.GetItemMetinSocket(player.EQUIPMENT, slotNumber,
                                                                0) - app.GetGlobalTimeStamp()
                    elif item.LIMIT_TIMER_BASED_ON_WEAR == limitType:
                        remain_time = player.GetItemMetinSocket(player.EQUIPMENT, slotNumber, 0)

                    if remain_time <= 0:
                        self.wndEquip.DisableSlot(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i)
                        break

        self.wndEquip.RefreshSlot()

    def ActivateEquipSlotWindow(self, deck):
        for i in xrange(6):
            if deck == 2:
                plusCount = 6
            else:
                plusCount = 0
            self.wndEquip.ActivateSlot(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount)
            if i == 0:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_WHITE)
            elif i == 1:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_RED)
            elif i == 2:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_GREEN)
            elif i == 3:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_SKY)
            elif i == 4:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_ORANGE)
            elif i == 5:
                self.wndEquip.SetSlotDiffuseColor(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i + plusCount,
                                                  wndMgr.COLOR_TYPE_BLUE)


    def DeactivateEquipSlotWindow(self):
        for i in xrange(12):
            self.wndEquip.DeactivateSlot(player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i)

    def RefreshStatus(self):
        self.RefreshItemSlot()

    def __InventoryLocalSlotPosToGlobalSlotPos(self, window_type, local_slot_pos):
        if player.EQUIPMENT == window_type:
            return self.deckPageIndex * player.DRAGON_SOUL_EQUIPMENT_FIRST_SIZE + local_slot_pos

        return (self.DSKindIndex * player.DRAGON_SOUL_PAGE_COUNT * player.DRAGON_SOUL_PAGE_SIZE) + \
               self.inventoryPageIndex * player.DRAGON_SOUL_PAGE_SIZE + local_slot_pos

    def RefreshBagSlotWindow(self):
        if not self.isLoaded:
            return

        pos = ItemPosition()
        pos.windowType = player.DRAGON_SOUL_INVENTORY

        for i in xrange(player.DRAGON_SOUL_PAGE_SIZE):
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, i)
            pos.cell = slotNumber

            itemData = playerInst().GetItemData(pos)
            if not itemData:
                continue

            try:
                SetItemSlot(self.wndItem.hWnd, i, itemData)
            except Exception as e:
                logging.exception(e)

        self.__HighlightSlot_RefreshCurrentPage()
        self.wndItem.RefreshSlot()

    def PutAllInRefine(self):
        getItemVNum = player.GetItemIndex
        getItemCount = player.GetItemCount
        setItemVnum = self.wndItem.SetItemSlot

        for i in xrange(player.DRAGON_SOUL_PAGE_SIZE):
            # <- dragon soul kind
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, i)
            itemVnum = getItemVNum(player.DRAGON_SOUL_INVENTORY, slotNumber)
            if itemVnum != 0 and self.HasNeedCount(itemVnum) and self.wndDragonSoulRefine.CheckCanRefineSimple(
                    itemVnum):
                self.wndDragonSoulRefine.AutoSetItem((player.DRAGON_SOUL_INVENTORY, slotNumber), 1)

    def HasNeedCount(self, needVnum):
        getItemVNum = player.GetItemIndex

        needCount = self.wndDragonSoulRefine.GetCurrentNeedCount()
        count = 0
        for i in xrange(player.DRAGON_SOUL_PAGE_SIZE):
            # <- dragon soul kind
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, i)
            itemVnum = getItemVNum(player.DRAGON_SOUL_INVENTORY, slotNumber)
            if itemVnum != 0 and needVnum == itemVnum:
                count = count + 1

        return count >= needCount

    def ShowToolTip(self, window_type, slotIndex):
        if None != self.tooltipItem:
            self.tooltipItem.SetInventoryItem(slotIndex, window_type)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def OnTop(self):
        if None != self.tooltipItem:
            self.tooltipItem.SetTop()

    # item slot °ü·Ã ÇÔ¼ö
    def OverOutItem(self):
        self.wndItem.SetUsableItem(False)
        if None != self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OverInItem(self, overSlotPos):

        if self.wndItem:
            itemData = playerInst().GetItemData(MakeItemPosition(player.DRAGON_SOUL_INVENTORY, overSlotPos))
            if not itemData:
                return

            OverItemSlot(self.wndItem.hWnd, overSlotPos, itemData)

        overSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, overSlotPos)
        try:
            self.listHighlightedSlot.remove(overSlotPos)
        except:
            pass

        if mouseModule.mouseController.isAttached():
            attachedItemType = mouseModule.mouseController.GetAttachedType()
            if player.SLOT_TYPE_INVENTORY == attachedItemType:

                attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
                attachedItemVNum = mouseModule.mouseController.GetAttachedItemIndex()

                if IsItemUsedForDragonSoul(attachedItemVNum, player.DRAGON_SOUL_INVENTORY):
                    self.wndItem.SetUsableItem(True)

        self.ShowToolTip(player.DRAGON_SOUL_INVENTORY, overSlotPos)

    def __UseItem(self, slotIndex):
        if 0 == player.GetItemMetinSocket(player.DRAGON_SOUL_INVENTORY, slotIndex, 0):
            self.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_EXPIRED)
            self.wndPopupDialog.Open()
            return

        self.__EquipItem(slotIndex)

    def __EquipItem(self, slotIndex):
        ItemVNum = playerInst().GetItemIndex(MakeItemPosition(player.DRAGON_SOUL_INVENTORY, slotIndex))
        item.SelectItem(ItemVNum)
        subType = item.GetItemSubType()
        equipSlotPos = player.DRAGON_SOUL_EQUIPMENT_SLOT_START + self.deckPageIndex * player.DRAGON_SOUL_EQUIPMENT_FIRST_SIZE + subType
        srcItemPos = (player.DRAGON_SOUL_INVENTORY, slotIndex)
        dstItemPos = (player.EQUIPMENT, equipSlotPos)
        self.__OpenQuestionDialog_DS_Equip(True, srcItemPos, dstItemPos)

    def SelectItemSlot(self, itemSlotIndex):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        itemSlotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, itemSlotIndex)
        itemPos = MakeItemPosition(player.DRAGON_SOUL_INVENTORY, itemSlotIndex)

        if mouseModule.mouseController.isAttached():
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
            attachedItemVID = mouseModule.mouseController.GetAttachedItemIndex()

            attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
            if player.RESERVED_WINDOW != attachedInvenType:
                appInst.instance().GetNet().SendItemUseToItemPacket(MakeItemPosition(attachedInvenType, attachedSlotPos),
                                                    MakeItemPosition(player.DRAGON_SOUL_INVENTORY, itemSlotIndex))

            mouseModule.mouseController.DeattachObject()

        else:

            if app.IsPressed(app.VK_MENU):
                link = playerInst().GetItemLink(itemPos)
                active = wndMgr.GetFocus()
                if isinstance(active, ui.EditLine):
                    active.Insert(link)
            else:
                selectedItemVNum = playerInst().GetItemIndex(MakeItemPosition(player.DRAGON_SOUL_INVENTORY, itemSlotIndex))
                itemCount = playerInst().GetItemCount(itemPos)
                mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_DRAGON_SOUL_INVENTORY, (itemPos.windowType, itemPos.cell),
                                                         selectedItemVNum, itemCount)
                self.interface.SetUseItemMode(False)
                snd.PlaySound("sound/ui/pick.wav")

    def SelectEmptySlot(self, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        selectedSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, selectedSlotPos)

        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
            attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
            attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

            attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
            if player.SLOT_TYPE_PRIVATE_SHOP == attachedSlotType:
                mouseModule.mouseController.RunCallBack("INVENTORY")

            elif player.SLOT_TYPE_SHOP == attachedSlotType:
                appInst.instance().GetNet().SendShopBuyPacket(attachedSlotPos)

            elif player.SLOT_TYPE_SAFEBOX == attachedSlotType:
                if player.ITEM_MONEY == attachedItemIndex:
                    appInst.instance().GetNet().SendSafeboxWithdrawMoneyPacket(
                        mouseModule.mouseController.GetAttachedItemCount())
                    snd.PlaySound("sound/ui/money.wav")

                else:
                    appInst.instance().GetNet().SendSafeboxCheckoutPacket(attachedSlotPos, MakeItemPosition(player.DRAGON_SOUL_INVENTORY, selectedSlotPos))

            elif player.SLOT_TYPE_MALL == attachedSlotType:
                appInst.instance().GetNet().SendMallCheckoutPacket(attachedSlotPos, MakeItemPosition(player.DRAGON_SOUL_INVENTORY, selectedSlotPos))

            elif player.RESERVED_WINDOW != attachedInvenType:
                if player.IsDSEquipmentSlot(attachedInvenType, attachedSlotPos):
                    srcItemPos = (attachedInvenType, attachedSlotPos)
                    dstItemPos = (player.DRAGON_SOUL_INVENTORY, selectedSlotPos)
                    self.__OpenQuestionDialog_DS_Equip(False, srcItemPos, dstItemPos)
                else:
                    itemCount = playerInst().GetItemCount(MakeItemPosition(attachedInvenType, attachedSlotPos))
                    attachedCount = mouseModule.mouseController.GetAttachedItemCount()

                    self.__SendMoveItemPacket(attachedInvenType, attachedSlotPos, player.DRAGON_SOUL_INVENTORY,
                                              selectedSlotPos, attachedCount)

            mouseModule.mouseController.DeattachObject()

    def UseItemSlot(self, slotIndex):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
            return
        slotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, slotIndex)
        try:
            if self.wndDragonSoulRefine.IsShow():
                if uiPrivateShopBuilder.IsBuildingPrivateShop():
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
                    return

                if self.interface.IsShowDlgQuestionWindow():
                    self.interface.CloseDlgQuestionWindow()

                self.wndDragonSoulRefine.AutoSetItem((player.DRAGON_SOUL_INVENTORY, slotIndex), 1)
                return
        except:
            pass

        self.__UseItem(slotIndex)

        mouseModule.mouseController.DeattachObject()
        self.OverOutItem()

    def __SendMoveItemPacket(self, srcSlotWindow, srcSlotPos, dstSlotWindow, dstSlotPos, srcItemCount):
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
            return
        if self.interface.IsShowDlgQuestionWindow():
            self.interface.CloseDlgQuestionWindow()

        appInst.instance().GetNet().SendItemMovePacket(MakeItemPosition(srcSlotWindow, srcSlotPos), MakeItemPosition(dstSlotWindow, dstSlotPos), srcItemCount)

    # equip ½½·Ô °ü·Ã ÇÔ¼öµé.
    def OverOutEquipItem(self):
        self.OverOutItem()

    def OverInEquipItem(self, overSlotPos):
        overSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(player.EQUIPMENT, overSlotPos)
        self.wndItem.SetUsableItem(False)
        self.ShowToolTip(player.EQUIPMENT, overSlotPos)

    def UseEquipItemSlot(self, slotIndex):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
            return

        slotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(player.EQUIPMENT, slotIndex)

        self.__UseEquipItem(slotIndex)
        mouseModule.mouseController.DeattachObject()
        self.OverOutEquipItem()

    def __UseEquipItem(self, slotIndex):
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        self.__OpenQuestionDialog_DS_Equip(False, (player.EQUIPMENT, slotIndex), (-1, -1))

    def SelectEquipItemSlot(self, itemSlotIndex):

        ## ¸¶¿ì½º ¹öÆ°ÀÌ sell buy Ã¼Å© ÇØ¼­ return
        curCursorNum = app.GetCursor()
        if app.SELL == curCursorNum:
            return
        elif app.BUY == curCursorNum:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)
            return

        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        itemSlotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(player.EQUIPMENT, itemSlotIndex)

        if mouseModule.mouseController.isAttached():
            if self.wndDragonSoulRefine.IsShow():
                mouseModule.mouseController.DeattachObject()
                return

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            # ÀÚ±â ÀÚ½ÅÀ» ÀÚ±â ÀÚ½Å¿¡°Ô µå·¡±×ÇÏ´Â °æ¿ì
            if player.SLOT_TYPE_INVENTORY == attachedSlotType and itemSlotIndex == attachedSlotPos:
                return

            attachedItemVID = mouseModule.mouseController.GetAttachedItemIndex()

            attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
            if player.RESERVED_WINDOW != attachedInvenType:
                if IsItemUsedForDragonSoul(attachedItemVID, player.EQUIPMENT):
                    self.__OpenQuestionDialog(attachedInvenType, attachedSlotPos, player.EQUIPMENT, itemSlotIndex)

            mouseModule.mouseController.DeattachObject()
        else:
            itemPos = MakeItemPosition(player.EQUIPMENT, itemSlotIndex)
            selectedItemVNum = playerInst().GetItemIndex(itemPos)
            itemCount = playerInst().GetItemCount(itemPos)
            mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_INVENTORY, (player.EQUIPMENT, itemSlotIndex), selectedItemVNum,
                                                     itemCount)
            self.interface.SetUseItemMode(False)
            snd.PlaySound("sound/ui/pick.wav")

    def SelectEmptyEquipSlot(self, selectedSlot):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        selectedSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(player.INVENTORY, selectedSlot)

        if mouseModule.mouseController.isAttached():
            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
            attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
            attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

            if player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedSlotType:
                if 0 == player.GetItemMetinSocket(player.DRAGON_SOUL_INVENTORY, attachedSlotPos, 0):
                    self.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_EXPIRED)
                    self.wndPopupDialog.Open()
                    return

                item.SelectItem(attachedItemIndex)
                subType = item.GetItemSubType()
                if subType != (selectedSlot - player.DRAGON_SOUL_EQUIPMENT_SLOT_START):
                    self.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_UNMATCHED_SLOT)
                    self.wndPopupDialog.Open()
                else:
                    srcItemPos = (player.DRAGON_SOUL_INVENTORY, attachedSlotPos)
                    dstItemPos = (player.INVENTORY, selectedSlotPos)
                    self.__OpenQuestionDialog(player.DRAGON_SOUL_INVENTORY, attachedSlotPos, player.INVENTORY,
                                              selectedSlotPos)

            mouseModule.mouseController.DeattachObject()

    # equip ½½·Ô °ü·Ã ÇÔ¼öµé ³¡.
    def __OpenQuestionDialog_DS_Equip(self, Equip, srcItemPos, dstItemPos):
        if self.interface.IsShowDlgQuestionWindow():
            self.interface.CloseDlgQuestionWindow()

        self.srcItemPos = srcItemPos
        self.dstItemPos = dstItemPos

        self.dlgQuestion.SetAcceptEvent(self.__Accept_DS_Equip)
        self.dlgQuestion.SetCancelEvent(self.__Cancel)

        if Equip:
            self.dlgQuestion.SetText1(localeInfo.DRAGON_SOUL_EQUIP_WARNING1)
            self.dlgQuestion.SetText2(localeInfo.DRAGON_SOUL_EQUIP_WARNING2)
        else:
            self.dlgQuestion.SetText1(localeInfo.DRAGON_SOUL_UNEQUIP_WARNING1)
            self.dlgQuestion.SetText2(localeInfo.DRAGON_SOUL_UNEQUIP_WARNING2)

        self.dlgQuestion.Open()

    def __Accept_DS_Equip(self):
        self.dlgQuestion.Close()
        if (-1, -1) == self.dstItemPos:
            appInst.instance().GetNet().SendItemUsePacket(MakeItemPosition(self.srcItemPos))
        else:
            self.__SendMoveItemPacket(*(self.srcItemPos + self.dstItemPos + (0,)))

    # °æ°íÃ¢ °ü·Ã
    def __OpenQuestionDialog(self, srcItemInvenType, srcItemPos, dstItemInvenType, dstItemPos):
        if self.interface.IsShowDlgQuestionWindow():
            self.interface.CloseDlgQuestionWindow()

        if srcItemInvenType == dstItemInvenType:
            if srcItemPos == dstItemPos:
                return

        self.srcItemPos = (srcItemInvenType, srcItemPos)
        self.dstItemPos = (dstItemInvenType, dstItemPos)

        self.dlgQuestion.SetAcceptEvent(self.__Accept)
        self.dlgQuestion.SetCancelEvent(self.__Cancel)

        getItemVNum = player.GetItemIndex
        self.dlgQuestion.SetText1("%s" % item.GetItemNameByVnum(playerInst().GetItemIndex(self.srcItemPos)))
        self.dlgQuestion.SetText2(localeInfo.INVENTORY_REALLY_USE_ITEM)

        self.dlgQuestion.Open()

    def __Accept(self):
        self.dlgQuestion.Close()
        # °³ÀÎ»óÁ¡ ¿­°í ÀÖ´Â µ¿¾È ¾ÆÀÌÅÛ »ç¿ë ¹æÁö
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        if self.interface.IsShowDlgQuestionWindow():
            self.interface.CloseDlgQuestionWindow()

        appInst.instance().GetNet().SendItemUseToItemPacket(MakeItemPosition(self.srcItemPos),
                                            MakeItemPosition(self.dstItemPos))

        self.srcItemPos = (0, 0)
        self.dstItemPos = (0, 0)

    def __Cancel(self):
        self.srcItemPos = (0, 0)
        self.dstItemPos = (0, 0)
        self.dlgQuestion.Close()

    # °æ°íÃ¢ °ü·Ã ³¡

    def SetDSKindIndex(self, kindIndex):
        if self.DSKindIndex != kindIndex:
            self.__HighlightSlot_ClearCurrentPage()

        self.DSKindIndex = kindIndex

        for (tabKey, tabButton) in self.tabButtonDict.items():
            if kindIndex != tabKey:
                tabButton.SetUp()

        for tabValue in self.tabDict.itervalues():
            tabValue.Hide()

        self.tabDict[kindIndex].Show()
        self.tabText.SetText(DragonSoulWindow.KIND_TAP_TITLES[kindIndex])

        self.RefreshBagSlotWindow()

    def SetDeckPage(self, page):
        if page == self.deckPageIndex:
            return

        if self.isActivated:
            self.DeactivateDragonSoul()
            appInst.instance().GetNet().SendChatPacket("/dragon_soul deactivate")
        self.deckPageIndex = page
        self.deckTab[page].Down()
        self.deckTab[(page + 1) % 2].SetUp()

        self.RefreshEquipSlotWindow()

    def SetDSSetGrade(self, grade):
        self.setGrade = grade

    def GetDSSetGrade(self):
        if not self.isActivated:
            return 0

        return self.setGrade

    # ¿ëÈ¥¼® È°¼ºÈ­ °ü·Ã
    def ActivateDragonSoulByExtern(self, deck):
        if not self.isLoaded:
            self.__LoadWindow()
        self.isActivated = True
        self.activateButton.Down()
        self.deckPageIndex = deck
        self.deckTab[deck].Down()
        self.deckTab[(deck + 1) % 2].SetUp()
        self.RefreshEquipSlotWindow()
        self.ActivateEquipSlotWindow(deck)

    def DeactivateDragonSoul(self):
        self.isActivated = False
        self.activateButton.SetUp()
        self.DeactivateEquipSlotWindow()

    def ActivateButtonClick(self):
        if app.GetGlobalTimeStamp() > self.nextStatusChange:

            self.isActivated = self.isActivated ^ True
            if self.isActivated:
                if self.__CanActivateDeck():
                    appInst.instance().GetNet().SendChatPacket("/dragon_soul activate " + str(self.deckPageIndex))
                else:
                    self.isActivated = False
                    self.activateButton.SetUp()
            else:
                appInst.instance().GetNet().SendChatPacket("/dragon_soul deactivate")
            self.nextStatusChange = app.GetGlobalTimeStamp() + 10
        else:
            if self.isActivated:
                self.activateButton.Down()
            else:
                self.activateButton.SetUp()
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CANNOT_TOGGLE_DS_STATE_SO_FAST)

    def ActivateButtonClickByKey(self):
        if app.GetGlobalTimeStamp() > self.nextStatusChange:

            self.isActivated = self.isActivated ^ True
            if self.isActivated:
                if self.__CanActivateDeck():
                    appInst.instance().GetNet().SendChatPacket("/dragon_soul activate 0")
                else:
                    self.isActivated = False
                    self.activateButton.SetUp()
            else:
                appInst.instance().GetNet().SendChatPacket("/dragon_soul deactivate")
            self.nextStatusChange = app.GetGlobalTimeStamp() + 10
        else:
            if self.isActivated:
                self.activateButton.Down()
            else:
                self.activateButton.SetUp()
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.CANNOT_TOGGLE_DS_STATE_SO_FAST)

    def __CanActivateDeck(self):

        canActiveNum = 0
        for i in xrange(6):
            slotPos = MakeItemPosition(player.EQUIPMENT, player.DRAGON_SOUL_EQUIPMENT_SLOT_START + i)
            itemVnum = playerInst().GetItemIndex(slotPos)

            if itemVnum != 0:
                item.SelectItem(itemVnum)
                isNoLimit = True
                for j in xrange(item.LIMIT_MAX_NUM):
                    (limitType, limitValue) = item.GetLimit(j)

                    if item.LIMIT_TIMER_BASED_ON_WEAR == limitType:
                        isNoLimit = False
                        remain_time = playerInst().GetItemMetinSocket(slotPos, 0)
                        if 0 != remain_time:
                            canActiveNum += 1
                            break

                if isNoLimit:
                    canActiveNum += 1

        return canActiveNum > 0

    # È°¼ºÈ­ °ü·Ã ³¡

    # ½½·Ô highlight °ü·Ã
    def __HighlightSlot_ClearCurrentPage(self):
        for i in xrange(self.wndItem.GetSlotCount()):
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, i)
            if slotNumber in self.listHighlightedSlot:
                self.wndItem.DeactivateSlot(i)
                self.listHighlightedSlot.remove(slotNumber)

    def __HighlightSlot_RefreshCurrentPage(self):
        for i in xrange(self.wndItem.GetSlotCount()):
            slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(player.DRAGON_SOUL_INVENTORY, i)
            if slotNumber in self.listHighlightedSlot:
                self.wndItem.ActivateSlot(i)

    def HighlightSlot(self, slot):
        if not slot in self.listHighlightedSlot:
            self.listHighlightedSlot.append(slot)

    # ½½·Ô highlight °ü·Ã ³¡

    def SetDragonSoulRefineWindow(self, wndDragonSoulRefine):
        if app.ENABLE_DRAGON_SOUL_SYSTEM:
            from _weakref import proxy
            self.wndDragonSoulRefine = proxy(wndDragonSoulRefine)

    def BindInterfaceClass(self, interface):
        from _weakref import proxy
        self.interface = proxy(interface)

    def IsDlgQuestionShow(self):
        if self.dlgQuestion and self.dlgQuestion.IsShow():
            return True
        else:
            return False

    def CancelDlgQuestion(self):
        self.__Cancel()

    def SetUseItemMode(self, bUse):
        if not self.isLoaded:
            return
        if self.wndItem:
            self.wndItem.SetUseMode(bUse)


class DragonSoulRefineWindow(ui.ScriptWindow):
    REFINE_TYPE_GRADE, REFINE_TYPE_STEP, REFINE_TYPE_STRENGTH = xrange(3)
    DS_SUB_HEADER_DIC = {
        REFINE_TYPE_GRADE: player.DS_SUB_HEADER_DO_UPGRADE,
        REFINE_TYPE_STEP: player.DS_SUB_HEADER_DO_IMPROVEMENT,
        REFINE_TYPE_STRENGTH: player.DS_SUB_HEADER_DO_REFINE
    }
    REFINE_STONE_SLOT, DRAGON_SOUL_SLOT = xrange(2)

    INVALID_DRAGON_SOUL_INFO = -1

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.tooltipItem = None
        self.sellingSlotNumber = -1
        self.isLoaded = 0
        self.refineChoiceButtonDict = None
        self.doRefineButton = None
        self.doRefineAllButton = None
        self.wndMoney = None
        self.SetWindowName("DragonSoulRefineWindow")
        self.interface = None

    def BindInterfaceClass(self, interface):
        from _weakref import proxy
        self.interface = proxy(interface)

    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return
        self.isLoaded = 1
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/DragonSoulRefineWindow.py")
        except:
            logging.exception("dragonsoulrefinewindow.LoadWindow.LoadObject")

        try:
            if localeInfo.IsARABIC():
                self.board = self.GetChild("DragonSoulRefineWindowBaseImage")
                self.board.SetScale(-1.0, 1.0)
                self.board.SetRenderingRect(-1.0, 0.0, 1.0, 0.0)

            wndRefineSlot = self.GetChild("RefineSlot")
            wndResultSlot = self.GetChild("ResultSlot")
            self.GetChild("TitleBar").SetCloseEvent(self.Close)
            self.refineChoiceButtonDict = {
                self.REFINE_TYPE_GRADE: self.GetChild("GradeButton"),
                self.REFINE_TYPE_STEP: self.GetChild("StepButton"),
                self.REFINE_TYPE_STRENGTH: self.GetChild("StrengthButton"),
            }
            self.doRefineButton = self.GetChild("DoRefineButton")
            self.doRefineAllButton = self.GetChild("DoRefineAllButton")

            self.wndMoney = self.GetChild("Money_Slot")

        except:
            logging.exception("DragonSoulRefineWindow.LoadWindow.BindObject")

        ## Item Slots
        wndRefineSlot.SetOverInItemEvent(self.__OverInRefineItem)
        wndRefineSlot.SetOverOutItemEvent(self.__OverOutItem)
        wndRefineSlot.SetSelectEmptySlotEvent(self.__SelectRefineEmptySlot)
        wndRefineSlot.SetSelectItemSlotEvent(self.__SelectRefineItemSlot)
        wndRefineSlot.SetUseSlotEvent(self.__SelectRefineItemSlot)
        wndRefineSlot.SetUnselectItemSlotEvent(self.__SelectRefineItemSlot)

        wndResultSlot.SetOverInItemEvent(self.__OverInResultItem)
        wndResultSlot.SetOverOutItemEvent(self.__OverOutItem)
        self.wndRefineSlot = wndRefineSlot
        self.wndResultSlot = wndResultSlot

        ## Button
        self.refineChoiceButtonDict[self.REFINE_TYPE_GRADE].SetToggleDownEvent(self.__ToggleDownGradeButton)
        self.refineChoiceButtonDict[self.REFINE_TYPE_STEP].SetToggleDownEvent(self.__ToggleDownStepButton)
        self.refineChoiceButtonDict[self.REFINE_TYPE_STRENGTH].SetToggleDownEvent(self.__ToggleDownStrengthButton)
        self.refineChoiceButtonDict[self.REFINE_TYPE_GRADE].SetToggleUpEvent(
            Event(self.__ToggleUpButton, self.REFINE_TYPE_GRADE))
        self.refineChoiceButtonDict[self.REFINE_TYPE_STEP].SetToggleUpEvent(
            Event(self.__ToggleUpButton, self.REFINE_TYPE_STEP))
        self.refineChoiceButtonDict[self.REFINE_TYPE_STRENGTH].SetToggleUpEvent(
            Event(self.__ToggleUpButton, self.REFINE_TYPE_STRENGTH))
        self.doRefineButton.SetEvent(self.__PressDoRefineButton)
        self.doRefineAllButton.SetEvent(self.__PressDoRefineAllButton)

        ## Dialog
        self.wndPopupDialog = uiCommon.PopupDialog()

        self.currentRefineType = self.REFINE_TYPE_GRADE
        self.refineItemInfo = {}
        self.resultItemInfo = {}
        self.currentRecipe = {}

        self.wndMoney.SetText(localeInfo.NumberToMoneyString(0))
        self.refineChoiceButtonDict[self.REFINE_TYPE_GRADE].Down()

        self.__Initialize()

    def Destroy(self):
        self.ClearDictionary()
        self.tooltipItem = None
        self.wndItem = 0
        self.wndEquip = 0
        self.activateButton = 0
        self.questionDialog = None
        self.mallButton = None
        self.inventoryTab = []
        self.deckTab = []
        self.equipmentTab = []
        self.tabDict = None
        self.tabButtonDict = None

    def Close(self):
        if None != self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.__FlushRefineItemSlot()
        player.SendDragonSoulRefine(player.DRAGON_SOUL_REFINE_CLOSE)
        self.Hide()

    def Show(self):
        self.__LoadWindow()
        self.currentRefineType = self.REFINE_TYPE_GRADE
        self.wndMoney.SetText(localeInfo.NumberToMoneyString(0))
        self.refineChoiceButtonDict[self.REFINE_TYPE_GRADE].Down()
        self.refineChoiceButtonDict[self.REFINE_TYPE_STEP].SetUp()
        self.refineChoiceButtonDict[self.REFINE_TYPE_STRENGTH].SetUp()

        self.Refresh()

        ui.ScriptWindow.Show(self)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    # ¹öÆ° ´­·Á ÀÖ´Â »óÅÂ¸¦ Á¦¿ÜÇÑ ¸ðµç °­È­Ã¢ °ü·Ã º¯¼öµéÀ» ÃÊ±âÈ­.
    def __Initialize(self):
        self.currentRecipe = {}
        self.refineItemInfo = {}
        self.resultItemInfo = {}

        self.refineSlotLockStartIndex = 1

        for i in xrange(self.refineSlotLockStartIndex):
            self.wndRefineSlot.HideSlotBaseImage(i)

        self.wndMoney.SetText(localeInfo.NumberToMoneyString(0))

    def __FlushRefineItemSlot(self):
        ## Item slot settings
        # ¿ø·¡ ÀÎº¥ÀÇ ¾ÆÀÌÅÛ Ä«¿îÆ® È¸º¹
        for invenType, invenPos, itemCount in self.refineItemInfo.values():
            remainCount = player.GetItemCount(invenType, invenPos)
            #player.SetItemCount(invenType, invenPos, remainCount + itemCount)
        self.__Initialize()

    def __ToggleUpButton(self, idx):
        # if self.REFINE_TYPE_GRADE == self.currentRefineType:
        self.refineChoiceButtonDict[idx].Down()

    def __ToggleDownGradeButton(self):
        if self.REFINE_TYPE_GRADE == self.currentRefineType:
            return
        self.refineChoiceButtonDict[self.currentRefineType].SetUp()
        self.currentRefineType = self.REFINE_TYPE_GRADE
        self.__FlushRefineItemSlot()
        self.Refresh()

    def __ToggleDownStepButton(self):
        if self.REFINE_TYPE_STEP == self.currentRefineType:
            return
        self.refineChoiceButtonDict[self.currentRefineType].SetUp()
        self.currentRefineType = self.REFINE_TYPE_STEP
        self.__FlushRefineItemSlot()
        self.Refresh()

    def __ToggleDownStrengthButton(self):
        if self.REFINE_TYPE_STRENGTH == self.currentRefineType:
            return
        self.refineChoiceButtonDict[self.currentRefineType].SetUp()
        self.currentRefineType = self.REFINE_TYPE_STRENGTH
        self.__FlushRefineItemSlot()
        self.Refresh()

    def __PopUp(self, message):
        self.wndPopupDialog.SetText(message)
        self.wndPopupDialog.Open()

    def __SetItem(self, inven, dstSlotIndex, itemCount):
        invenType, invenPos = inven

        if dstSlotIndex >= self.refineSlotLockStartIndex:
            return False

        itemVnum = playerInst().GetItemIndex(MakeItemPosition(invenType, invenPos))
        maxCount = playerInst().GetItemCount(MakeItemPosition(invenType, invenPos))

        if itemCount > maxCount:
            raise Exception("Invalid attachedItemCount(%d). (base pos (%d, %d), base itemCount(%d))" % (
                itemCount, invenType, invenPos, maxCount))
        # return False

        # ºó ½½·ÔÀÌ¾î¾ßÇÔ.
        if dstSlotIndex in self.refineItemInfo:
            return False

        # °­È­Ã¢¿¡ ¿Ã¸± ¼ö ÀÖ´Â ¾ÆÀÌÅÛÀÎÁö °Ë»ç.
        if False == self.__CheckCanRefine(itemVnum):
            return False

        # ²ø¾î´Ù ³õÀº ¾ÆÀÌÅÛ Ä«¿îÆ®¸¸Å­ ¿ø·¡ ÀÚ¸®ÀÇ ¾ÆÀÌÅÛ Ä«¿îÆ® °¨¼Ò
        #player.SetItemCount(invenType, invenPos, maxCount - itemCount)
        self.refineItemInfo[dstSlotIndex] = (invenType, invenPos, itemCount)
        self.Refresh()

        return True

    def CheckCanRefineSimple(self, vnum):
        if self.REFINE_TYPE_GRADE == self.currentRefineType:
            return self.CanRefineGradeSimple(vnum)

        elif self.REFINE_TYPE_STEP == self.currentRefineType:
            return self.CanRefineStepSimple(vnum)

        elif self.REFINE_TYPE_STRENGTH == self.currentRefineType:
            return self.CanRefineStrengthSimple(vnum)

        else:
            return False

    def CanRefineGradeSimple(self, vnum):
        ds_info = self.__GetDragonSoulTypeInfo(vnum)

        if DragonSoulRefineWindow.INVALID_DRAGON_SOUL_INFO == ds_info:
            return False

        currentRecipe = self.__GetRefineGradeRecipe(vnum)
        if not currentRecipe:
            return False

        ds_type, grade, step, strength = ds_info
        cur_refine_ds_type, cur_refine_grade, cur_refine_step, cur_refine_strength = currentRecipe["ds_info"]
        if not (cur_refine_ds_type == ds_type and cur_refine_grade == grade):
            return False

        return True

    def CanRefineStepSimple(self, vnum):
        ds_info = self.__GetDragonSoulTypeInfo(vnum)

        if DragonSoulRefineWindow.INVALID_DRAGON_SOUL_INFO == ds_info:
            return False

        currentRecipe = self.__GetRefineStepRecipe(vnum)
        if not currentRecipe:
            return False

        ds_type, grade, step, strength = ds_info
        cur_refine_ds_type, cur_refine_grade, cur_refine_step, cur_refine_strength = currentRecipe["ds_info"]
        if not (cur_refine_ds_type == ds_type and cur_refine_grade == grade and cur_refine_step == step):
            return False

        return True

    def CanRefineStrengthSimple(self, vnum):
        # ¿ëÈ¥¼®ÀÎ °æ¿ì, ´õ ÀÌ»ó strength °­È­¸¦ ÇÒ ¼ö ¾ø´ÂÁö Ã¼Å©ÇØ¾ßÇÔ.
        if self.__IsDragonSoul(vnum):
            ds_type, grade, step, strength = self.__GetDragonSoulTypeInfo(vnum)

            import dragon_soul_refine_settings
            if strength >= dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["strength_max_table"][grade][
                step]:
                return False

            else:
                return True

        # strength °­È­ÀÇ °æ¿ì, refine_recipe°¡ ¿ëÈ¥¼®ÀÇ Á¾·ù°¡ ¾Æ´Ñ, °­È­¼®ÀÇ Á¾·ù¿¡ µû¶ó ´Þ¶óÁø´Ù.
        # µû¶ó¼­ ¿ëÈ¥¼®ÀÌ ¾Æ´Ï¶ó¸é,
        # ÀÌ¹Ì ·¹½ÃÇÇ°¡ ÀÖ´Â °æ¿ì´Â, °­È­¼®ÀÌ °­È­Ã¢¿¡ ÀÖ´Ù´Â °ÍÀÌ¹Ç·Î, return False
        # ·¹½ÃÇÇ°¡ ¾ø´Â °æ¿ì´Â, °­È­¼®ÀÎÁö È®ÀÎÇÏ°í, ·¹½ÃÇÇ¸¦ ¼ÂÆÃÇÑ´Ù.
        else:
            return False

    # °­È­ °¡´ÉÇÑ ¾ÆÀÌÅÛÀÎÁö Ã¼Å©
    # ¿ëÈ¥¼® °­È­´Â °­È­ ·¹½ÃÇÇ¸¦ Á¤ÇØ³õ°í ½ÃÀÛÇÏ´Â °ÍÀÌ ¾Æ´Ï¶ó,
    # Ã³À½¿¡ °­È­Ã¢¿¡ ¿Ã¸° ¿ëÈ¥¼®¿¡ ÀÇÇØ °­È­ ·¹½ÃÇÇ°¡ °áÁ¤µÈ´Ù.
    # ±×·¡¼­ __CanRefineGrade, __CanRefineStep, __CanRefineStrength ÇÔ¼ö¿¡¼­
    # °­È­ ·¹½ÃÇÇ°¡ ¾ø´Ù¸é(Ã³À½ ¿Ã¸®´Â ¾ÆÀÌÅÛÀÌ¶ó¸é), °­È­ ·¹½ÃÇÇ¸¦ ¼³Á¤ÇØÁÖ´Â ¿ªÇÒµµ ÇÑ´Ù.
    def __CheckCanRefine(self, vnum):
        if self.REFINE_TYPE_GRADE == self.currentRefineType:
            return self.__CanRefineGrade(vnum)

        elif self.REFINE_TYPE_STEP == self.currentRefineType:
            return self.__CanRefineStep(vnum)

        elif self.REFINE_TYPE_STRENGTH == self.currentRefineType:
            return self.__CanRefineStrength(vnum)

        else:
            return False

    def GetCurrentNeedCount(self):
        return 2

    def __CanRefineGrade(self, vnum):
        ds_info = self.__GetDragonSoulTypeInfo(vnum)

        if DragonSoulRefineWindow.INVALID_DRAGON_SOUL_INFO == ds_info:
            self.__PopUp(localeInfo.DRAGON_SOUL_IS_NOT_DRAGON_SOUL)
            return False

        if self.currentRecipe:
            ds_type, grade, step, strength = ds_info
            cur_refine_ds_type, cur_refine_grade, cur_refine_step, cur_refine_strength = self.currentRecipe["ds_info"]
            if not (cur_refine_ds_type == ds_type and cur_refine_grade == grade):
                self.__PopUp(localeInfo.DRAGON_SOUL_INVALID_DRAGON_SOUL)
                return False
        # °­È­ Ã¢¿¡ Ã³À½ ¾ÆÀÌÅÛÀ» ¿Ã¸®´Â °æ¿ì, °­È­ Àç·á¿¡ °üÇÑ Á¤º¸°¡ ¾ø´Ù.
        # ¿ëÈ¥¼® °­È­°¡, ·¹½ÃÇÇ¸¦ °¡Áö°í ½ÃÀÛÇÏ´Â °ÍÀÌ ¾Æ´Ï¶ó, °­È­Ã¢¿¡ Ã³À½ ¿Ã¸®´Â ¾ÆÀÌÅÛÀÌ ¹«¾ùÀÌ³Ä¿¡ µû¶ó,
        # ¹«¾ùÀ» °­È­ÇÏ°í, Àç·á°¡ ¹«¾ùÀÎÁö(ÀÌÇÏ ·¹½ÃÇÇ)°¡ Á¤ÇØÁø´Ù.
        # ·¹½ÃÇÇ°¡ ¾ø´Ù¸é, Ã³À½ ¿Ã¸° ¾ÆÀÌÅÛÀÌ¶ó »ý°¢ÇÏ°í, vnumÀ» ¹ÙÅÁÀ¸·Î ·¹½ÃÇÇ¸¦ ¼ÂÆÃ.
        else:
            self.currentRecipe = self.__GetRefineGradeRecipe(vnum)

            if self.currentRecipe:
                self.refineSlotLockStartIndex = self.currentRecipe["need_count"]
                self.wndMoney.SetText(localeInfo.NumberToMoneyString(self.currentRecipe["fee"]))
                return True
            else:
                # °­È­ Á¤º¸ ¼ÂÆÃ¿¡ ½ÇÆÐÇÏ¸é ¿Ã¸± ¼ö ¾ø´Â ¾ÆÀÌÅÛÀ¸·Î ÆÇ´Ü.
                self.__PopUp(localeInfo.DRAGON_SOUL_CANNOT_REFINE)
                return False

    def __CanRefineStep(self, vnum):
        ds_info = self.__GetDragonSoulTypeInfo(vnum)

        if DragonSoulRefineWindow.INVALID_DRAGON_SOUL_INFO == ds_info:
            self.__PopUp(localeInfo.DRAGON_SOUL_IS_NOT_DRAGON_SOUL)
            return False

        if self.currentRecipe:
            ds_type, grade, step, strength = ds_info
            cur_refine_ds_type, cur_refine_grade, cur_refine_step, cur_refine_strength = self.currentRecipe["ds_info"]
            if not (cur_refine_ds_type == ds_type and cur_refine_grade == grade and cur_refine_step == step):
                self.__PopUp(localeInfo.DRAGON_SOUL_INVALID_DRAGON_SOUL)
                return False
        # °­È­ Ã¢¿¡ Ã³À½ ¾ÆÀÌÅÛÀ» ¿Ã¸®´Â °æ¿ì, Àç·á¿¡ °üÇÑ Á¤º¸°¡ ¾ø´Ù.
        # ¿ëÈ¥¼® °­È­°¡, ·¹½ÃÇÇ¸¦ °¡Áö°í ½ÃÀÛÇÏ´Â °ÍÀÌ ¾Æ´Ï¶ó, °­È­Ã¢¿¡ Ã³À½ ¿Ã¸®´Â ¾ÆÀÌÅÛÀÌ ¹«¾ùÀÌ³Ä¿¡ µû¶ó,
        # ¹«¾ùÀ» °­È­ÇÏ°í, Àç·á°¡ ¹«¾ùÀÎÁö(ÀÌÇÏ ·¹½ÃÇÇ)°¡ Á¤ÇØÁø´Ù.
        # ·¹½ÃÇÇ°¡ ¾ø´Ù¸é, Ã³À½ ¿Ã¸° ¾ÆÀÌÅÛÀÌ¶ó »ý°¢ÇÏ°í, vnumÀ» ¹ÙÅÁÀ¸·Î ·¹½ÃÇÇ¸¦ ¼ÂÆÃ.
        else:
            self.currentRecipe = self.__GetRefineStepRecipe(vnum)

            if self.currentRecipe:
                self.refineSlotLockStartIndex = self.currentRecipe["need_count"]
                self.wndMoney.SetText(localeInfo.NumberToMoneyString(self.currentRecipe["fee"]))
                return True

            else:
                # °­È­ Á¤º¸ ¼ÂÆÃ¿¡ ½ÇÆÐÇÏ¸é ¿Ã¸± ¼ö ¾ø´Â ¾ÆÀÌÅÛÀ¸·Î ÆÇ´Ü.
                self.__PopUp(localeInfo.DRAGON_SOUL_CANNOT_REFINE)
                return False

    def __CanRefineStrength(self, vnum):
        # ¿ëÈ¥¼®ÀÎ °æ¿ì, ´õ ÀÌ»ó strength °­È­¸¦ ÇÒ ¼ö ¾ø´ÂÁö Ã¼Å©ÇØ¾ßÇÔ.
        if self.__IsDragonSoul(vnum):
            ds_type, grade, step, strength = self.__GetDragonSoulTypeInfo(vnum)

            import dragon_soul_refine_settings
            if strength >= dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["strength_max_table"][grade][
                step]:
                self.__PopUp(localeInfo.DRAGON_SOUL_CANNOT_REFINE_MORE)
                return False

            else:
                self.wndMoney.SetText(localeInfo.NumberToMoneyString(10000000))
                return True

    def __GetRefineGradeRecipe(self, vnum):
        ds_type, grade, step, strength = self.__GetDragonSoulTypeInfo(vnum)
        try:
            import dragon_soul_refine_settings

            return {
                "ds_info": (ds_type, grade, step, strength),
                "need_count": dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["grade_need_count"][grade],
                "fee": dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["grade_fee"][grade]
            }
        except:
            return None

    def __GetRefineStepRecipe(self, vnum):
        ds_type, grade, step, strength = self.__GetDragonSoulTypeInfo(vnum)
        try:
            import dragon_soul_refine_settings

            return {
                "ds_info": (ds_type, grade, step, strength),
                "need_count": dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["step_need_count"][step],
                "fee": dragon_soul_refine_settings.dragon_soul_refine_info[ds_type]["step_fee"][step]
            }
        except:
            return None

    # strength °­È­ÀÇ °æ¿ì, refineInfo´Â °­È­¼®¿¡ µû¶ó ´Þ¶óÁø´Ù.
    def __GetRefineStrengthInfo(self, itemVnum):
        try:
            # ÀÌ³ðÀÇ À§Ä¡¸¦ ¾îÂîÇÏÁö....
            # °­È­¼®ÀÌ ¾Æ´Ï¸é ¾ÈµÊ.
            item.SelectItem(itemVnum)
            if not (item.ITEM_TYPE_MATERIAL == item.GetItemType() and
                    (item.MATERIAL_DS_REFINE_NORMAL <= item.GetItemSubType() <= item.MATERIAL_DS_REFINE_HOLLY)):
                return None

            import dragon_soul_refine_settings
            return {"fee": dragon_soul_refine_settings.strength_fee[item.GetItemSubType()]}
        except:
            return None

    def __IsDragonSoul(self, vnum):
        item.SelectItem(vnum)
        return item.GetItemType() == item.ITEM_TYPE_DS

    # ¿ëÈ¥¼® Vnum¿¡ ´ëÇÑ comment
    # ITEM VNUMÀ» 10¸¸ ÀÚ¸®ºÎÅÍ, FEDCBA¶ó°í ÇÑ´Ù¸é
    # FE : ¿ëÈ¥¼® Á¾·ù.    D : µî±Þ
    # C : ´Ü°è            B : °­È­
    # A : ¿©¹úÀÇ ¹øÈ£µé...
    def __GetDragonSoulTypeInfo(self, vnum):
        if not self.__IsDragonSoul(vnum):
            return DragonSoulRefineWindow.INVALID_DRAGON_SOUL_INFO
        ds_type = vnum / 10000
        grade = vnum % 10000 / 1000
        step = vnum % 1000 / 100
        strength = vnum % 100 / 10

        return ds_type, grade, step, strength

    def __MakeDragonSoulVnum(self, ds_type, grade, step, strength):
        return ds_type * 10000 + grade * 1000 + step * 100 + strength * 10

    ## ºó ½½·Ô ¼±ÅÃ Event
    def __SelectRefineEmptySlot(self, selectedSlotPos):
        try:
            if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
                return

            if selectedSlotPos >= self.refineSlotLockStartIndex:
                return

            if mouseModule.mouseController.isAttached():
                attachedSlotType = mouseModule.mouseController.GetAttachedType()
                attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
                attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
                attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()
                mouseModule.mouseController.DeattachObject()

                if uiPrivateShopBuilder.IsBuildingPrivateShop():
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
                    return

                attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)

                if player.INVENTORY == attachedInvenType and player.IsEquipmentSlot(attachedSlotPos):
                    return

                if player.INVENTORY != attachedInvenType and player.DRAGON_SOUL_INVENTORY != attachedInvenType:
                    return

                if True == self.__SetItem((attachedInvenType, attachedSlotPos), selectedSlotPos, attachedItemCount):
                    self.Refresh()

        except Exception as e:
            logging.exception("Exception : __SelectRefineEmptySlot, %s" % e)

    # Å¬¸¯À¸·Î ½½·Ô¿¡¼­ »èÁ¦.
    def __SelectRefineItemSlot(self, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        try:
            if not selectedSlotPos in self.refineItemInfo:
                # »õ·Î¿î ¾ÆÀÌÅÛÀ» °­È­Ã¢¿¡ ¿Ã¸®´Â ÀÛ¾÷.
                if mouseModule.mouseController.isAttached():
                    attachedSlotType = mouseModule.mouseController.GetAttachedType()
                    attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
                    attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
                    attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()
                    mouseModule.mouseController.DeattachObject()

                    if uiPrivateShopBuilder.IsBuildingPrivateShop():
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
                        return

                    attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)

                    if player.INVENTORY == attachedInvenType and player.IsEquipmentSlot(attachedSlotPos):
                        return

                    if player.INVENTORY != attachedInvenType and player.DRAGON_SOUL_INVENTORY != attachedInvenType:
                        return

                    self.AutoSetItem((attachedInvenType, attachedSlotPos), 1)
                return
            elif mouseModule.mouseController.isAttached():
                return

            attachedInvenType, attachedSlotPos, attachedItemCount = self.refineItemInfo[selectedSlotPos]
            selectedItemVnum = player.GetItemIndex(attachedSlotPos)

            # °­È­Ã¢¿¡¼­ »èÁ¦ ¹× ¿ø·¡ ÀÎº¥ÀÇ ¾ÆÀÌÅÛ Ä«¿îÆ® È¸º¹
            invenType, invenPos, itemCount = self.refineItemInfo[selectedSlotPos]
            remainCount = player.GetItemCount(invenType, invenPos)
            #player.SetItemCount(invenType, invenPos, remainCount + itemCount)
            del self.refineItemInfo[selectedSlotPos]

            # °­È­Ã¢ÀÌ ºñ¾ú´Ù¸é, ÃÊ±âÈ­
            if not self.refineItemInfo:
                self.__Initialize()
            else:
                item.SelectItem(selectedItemVnum)
                # ¾ø¾Ø ¾ÆÀÌÅÛÀÌ °­È­¼®ÀÌ¾ú´Ù¸é °­È­ ·¹ÇÇ½Ã ÃÊ±âÈ­
                if (item.ITEM_TYPE_MATERIAL == item.GetItemType() \
                        and (
                                item.MATERIAL_DS_REFINE_NORMAL <= item.GetItemSubType() and item.GetItemSubType() <= item.MATERIAL_DS_REFINE_HOLLY)):
                    self.currentRecipe = {}
                    self.wndMoney.SetText(localeInfo.NumberToMoneyString(0))
                # ¿ëÈ¥¼®ÀÌ¾ú´Ù¸é,
                # strength°­È­°¡ ¾Æ´Ñ °æ¿ì, °­È­Ã¢¿¡ ´Ù¸¥ ¿ëÈ¥¼®ÀÌ ³²¾ÆÀÖÀ¸¹Ç·Î, ·¹½ÃÇÇ¸¦ ÃÊ±âÈ­ÇÏ¸é ¾ÈµÊ.
                # strength°­È­ÀÇ °æ¿ì, °­È­ ·¹½ÃÇÇ´Â °­È­¼®¿¡ Á¾¼ÓµÈ °ÍÀÌ¹Ç·Î ´Ù¸¥ Ã³¸®ÇÒ ÇÊ¿ä°¡ ¾øÀ½.
                else:
                    pass

        except Exception as e:
            logging.exception("Exception : __SelectRefineItemSlot, %s" % e)

        self.Refresh()

    def __OverInRefineItem(self, slotIndex):
        if slotIndex in self.refineItemInfo:
            inven_type, inven_pos, item_count = self.refineItemInfo[slotIndex]
            self.tooltipItem.SetInventoryItem(inven_pos, inven_type)

    def __OverInResultItem(self, slotIndex):
        if slotIndex in self.resultItemInfo:
            inven_type, inven_pos, item_count = self.resultItemInfo[slotIndex]
            self.tooltipItem.SetInventoryItem(inven_pos, inven_type)

    def __OverOutItem(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def __PressDoRefineButton(self):
        for i in xrange(self.refineSlotLockStartIndex):
            if not i in self.refineItemInfo:
                self.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_NOT_ENOUGH_MATERIAL)
                self.wndPopupDialog.Open()

                return

        player.SendDragonSoulRefine(DragonSoulRefineWindow.DS_SUB_HEADER_DIC[self.currentRefineType],
                                    self.refineItemInfo)

    def __PressDoRefineAllButton(self):
        self.interface.PutAllInRefine()

        for i in xrange(self.refineSlotLockStartIndex):
            if not i in self.refineItemInfo:
                self.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_NOT_ENOUGH_MATERIAL)
                self.wndPopupDialog.Open()
                return

        player.SendDragonSoulRefine(DragonSoulRefineWindow.DS_SUB_HEADER_DIC[self.currentRefineType],
                                    self.refineItemInfo)

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def Refresh(self):
        self.__RefreshRefineItemSlot()
        self.__ClearResultItemSlot()

    def __RefreshRefineItemSlot(self):
        try:
            for slotPos in xrange(self.wndRefineSlot.GetSlotCount()):
                self.wndRefineSlot.ClearSlot(slotPos)
                if slotPos < self.refineSlotLockStartIndex:
                    # self.refineItemInfo[slotPos]ÀÇ Á¤º¸È®ÀÎ
                    # (½ÇÁ¦·Î ¾ÆÀÌÅÛÀÌ Á¸ÀçÇÏ´ÂÁö È®ÀÎ)
                    # Á¸Àç -> ¾ÆÀÌÅÛ ¾ÆÀÌÄÜÀ» ½½·Ô¿¡ ¼ÂÆÃ.
                    # ºñÁ¸Àç -> ¾ÆÀÌÅÛÀÌ ¾øÀ¸¹Ç·Î °­È­Ã¢¿¡¼­ »èÁ¦.
                    if slotPos in self.refineItemInfo:
                        invenType, invenPos, itemCount = self.refineItemInfo[slotPos]
                        itemVnum = player.GetItemIndex(invenType, invenPos)

                        # if itemVnum:
                        if itemVnum:
                            self.wndRefineSlot.SetItemSlot(slotPos, player.GetItemIndex(invenType, invenPos), itemCount)
                        else:
                            del self.refineItemInfo[slotPos]

                    # ºó ½½·Ô¿¡ reference ¾ÆÀÌÄÜÀ» alpha 0.5·Î ¼ÂÆÃ.
                    if not slotPos in self.refineItemInfo:
                        try:
                            reference_vnum = 0
                            # strength °­È­ÀÏ ¶§´Â,
                            # 0¹ø ½½·Ô¿¡ °­È­¼®À», 1¹ø ½½·Ô¿¡ ¿ëÈ¥¼®À» ³õ´Â´Ù.
                            reference_vnum = self.__MakeDragonSoulVnum(*self.currentRecipe["ds_info"])
                            if 0 != reference_vnum:
                                item.SelectItem(reference_vnum)
                                itemIcon = item.GetIconImageFileName()
                                (width, height) = item.GetItemSize()
                                self.wndRefineSlot.SetSlot(slotPos, 0, width, height, itemIcon, (1.0, 1.0, 1.0, 0.5))
                                # slot ¿ìÃø ÇÏ´Ü¿¡ ¼ýÀÚ ¶ß¸é ¾È ¿¹»Ý...
                                self.wndRefineSlot.SetSlotCount(slotPos, 0)
                        except:
                            pass
                    # refineSlotLockStartIndex º¸´Ù ÀÛÀº ½½·ÔÀº ´ÝÈù ÀÌ¹ÌÁö¸¦ º¸¿©ÁÖ¸é ¾ÈµÊ.
                    self.wndRefineSlot.HideSlotBaseImage(slotPos)
                # slotPos >= self.refineSlotLockStartIndex:
                else:
                    # Á¤»óÀûÀÎ °æ¿ì¶ó¸é ÀÌ if¹®¿¡ µé¾î°¥ ÀÏÀº ¾ø°ÚÁö¸¸,
                    # (¾ÖÃÊ¿¡ ÀÎµ¦½º°¡ refineSlotLockStartIndex ÀÌ»óÀÎ ½½·Ô¿¡´Â ¾ÆÀÌÅÛÀ» ³ÖÁö ¸øÇÏ°Ô Çß±â ¶§¹®)
                    # È¤½Ã ¸ð¸¦ ¿¡·¯¿¡ ´ëºñÇÔ.
                    if slotPos in self.refineItemInfo:
                        invenType, invenPos, itemCount = self.refineItemInfo[slotPos]
                        remainCount = player.GetItemCount(invenType, invenPos)
                        #player.SetItemCount(invenType, invenPos, remainCount + itemCount)
                        del self.refineItemInfo[slotPos]
                    # refineSlotLockStartIndex ÀÌ»óÀÎ ½½·ÔÀº ´ÝÈù ÀÌ¹ÌÁö¸¦ º¸¿©Áà¾ßÇÔ.
                    self.wndRefineSlot.ShowSlotBaseImage(slotPos)

            # °­È­Ã¢¿¡ ¾Æ¹«·± ¾ÆÀÌÅÛÀÌ ¾ø´Ù¸é, ÃÊ±âÈ­ÇØÁÜ.
            # À§¿¡¼­ Áß°£ Áß°£¿¡ "del self.refineItemInfo[slotPos]"¸¦ Çß±â ¶§¹®¿¡,
            # ¿©±â¼­ ÇÑ¹ø Ã¼Å©ÇØÁà¾ßÇÔ.
            if not self.refineItemInfo:
                self.__Initialize()

            self.wndRefineSlot.RefreshSlot()
        except Exception as e:
            logging.exception("Exception : __RefreshRefineItemSlot, %s" % e)

    def __GetEmptySlot(self, itemVnum=0):
        # STRENGTH °­È­ÀÇ °æ¿ì, ¿ëÈ¥¼® ½½·Ô°ú °­È­¼® ½½·ÔÀÌ ±¸ºÐµÇ¾îÀÖ±â ‹š¹®¿¡
        # vnumÀ» ¾Ë¾Æ¾ß ÇÑ´Ù.
        if DragonSoulRefineWindow.REFINE_TYPE_STRENGTH == self.currentRefineType:
            if 0 == itemVnum:
                return -1

            for slotPos in xrange(self.wndRefineSlot.GetSlotCount()):
                if not slotPos in self.refineItemInfo:
                    return slotPos
        else:
            for slotPos in xrange(self.wndRefineSlot.GetSlotCount()):
                if not slotPos in self.refineItemInfo:
                    return slotPos

        return -1

    def AutoSetItem(self, inven, itemCount):
        invenType, invenPos = inven
        itemVnum = player.GetItemIndex(invenType, invenPos)
        emptySlot = self.__GetEmptySlot(itemVnum)
        if -1 == emptySlot:
            return False

        return self.__SetItem((invenType, invenPos), emptySlot, itemCount)

    def __ClearResultItemSlot(self):
        self.wndResultSlot.ClearSlot(0)
        self.resultItemInfo = {}

    def RefineSucceed(self, inven_type, inven_pos):
        self.__Initialize()
        self.Refresh()

        itemCount = player.GetItemCount(inven_type, inven_pos)
        if itemCount > 0:
            self.resultItemInfo[0] = (inven_type, inven_pos, itemCount)
            self.wndResultSlot.SetItemSlot(0, playerInst().GetItemIndex(MakeItemPosition(inven_type, inven_pos)), itemCount)

    def RefineFail(self, reason, inven_type, inven_pos):
        if DS_SUB_HEADER_REFINE_FAIL == reason:
            self.__Initialize()
            self.Refresh()
            itemCount = player.GetItemCount(inven_type, inven_pos)
            if itemCount > 0:
                self.resultItemInfo[0] = (inven_type, inven_pos, itemCount)
                self.wndResultSlot.SetItemSlot(0, playerInst().GetItemIndex(MakeItemPosition(inven_type, inven_pos)), itemCount)
        else:
            self.Refresh()

    def SetInventoryWindows(self, Inventory, DragonSoul):
        from _weakref import proxy
        self.wndInventory = proxy(Inventory)
        self.wndDragonSoul = proxy(DragonSoul)
