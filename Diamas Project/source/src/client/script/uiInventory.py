# -*- coding: utf-8 -*-

import logging

import app
import chat
import item
import player
import snd
import wndMgr
from pygame.app import appInst
from pygame.item import IsUsableItemToItem
from pygame.item import ItemPosition
from pygame.item import MakeItemPosition
from pygame.item import itemManager
from pygame.player import OverItemSlot
from pygame.player import SetItemSlot
from pygame.player import playerInst

import constInfo
import localeInfo
import mouseModule
import ui
import uiAttachMetin
import uiCommon
import uiDetachMetin
import uiPrivateShopBuilder  # Blocks on item move etc.
import uiSelectStorage
import uiSplitItem
from ui_event import Event, MakeEvent
from utils import clamp_index

if app.ENABLE_GROWTH_PET_SYSTEM:
    pass
from uiofflineshop import OfflineShopManager
import switchbot

ITEM_MALL_BUTTON_ENABLE = True
ITEM_FLAG_APPLICABLE = 1 << 14


class InventoryWindow(ui.ScriptWindow):
    USE_TYPE_TUPLE = (
        item.USE_CLEAN_SOCKET,
        item.USE_CHANGE_ATTRIBUTE,
        item.USE_ADD_ATTRIBUTE,
        item.USE_ADD_ATTRIBUTE2,
        item.USE_ADD_ACCESSORY_SOCKET,
        item.USE_PUT_INTO_ACCESSORY_SOCKET,
        item.USE_PUT_INTO_BELT_SOCKET,
        item.USE_PUT_INTO_RING_SOCKET,
        item.USE_CHANGE_COSTUME_ATTR,
        item.USE_RESET_COSTUME_ATTR,
        item.USE_ADD_ATTRIBUTE_RARE,
        item.USE_CHANGE_ATTRIBUTE_RARE,
        item.USE_ENHANCE_TIME,
        item.USE_CHANGE_ATTRIBUTE_PERM,
        item.USE_ADD_ATTRIBUTE_PERM,
        item.USE_MAKE_ACCESSORY_SOCKET_PERM,
        item.USE_LEVEL_PET_FOOD,
        item.USE_LEVEL_PET_CHANGE_ATTR,
        item.USE_ADD_SOCKETS,
    )

    questionDialog = None
    popupDialog = None
    tooltipItem = None
    wndSwitchBot = None
    dlgPickMoney = None

    sellingSlotNumber = -1
    isLoaded = 0

    skipUse = False
    restoreUseMode = False
    listHighlightedSlot = []
    listCantMouseSlot = []
    interface = None

    if app.ENABLE_GROWTH_PET_SYSTEM:
        petHatchingWindow = None
        petFeedWindow = None

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.inventoryTab = []
        self.inventoryCat = []
        self.equipmentTabButtons = []
        self.wndItem = None
        self.wndEquip = None
        self.wndMoney = None
        self.wndMoneySlot = None
        self.wndCashSlot = None
        self.wndCash = None
        self.dlgSplitItem = None

        if app.ENABLE_GEM_SYSTEM:
            self.wndGem = None
            self.wndGemSlot = None

        self.mallButton = None
        self.shopSearchButton = None
        self.DSSButton = None
        self.DungeonInfoButton = None
        self.attachMetinDialog = 0
        self.inventoryPageIndex = 0
        self.inventoryCatIndex = 0
        self.equipmentPageIndex = 0
        self.petHatchingWindow = None
        self.petFeedWindow = None
        self.OpenBoniSwitcherEvent = None
        self.selectStorageDialog = None
        self.isLoaded = False
        self.detachMetinDialog = None
        self.equipmentSlots = None
        self.__LoadWindow()

    def Show(self):
        self.__LoadWindow()

        ui.ScriptWindow.Show(self)

        self.RefreshItemSlot()
        self.RefreshStatus()
        self.SetTop()
        self.GetChild("TitleBar").SetTop()

    def BindInterfaceClass(self, interface):
        from _weakref import ref

        self.interface = ref(interface)

    def __LoadWindow(self):
        if self.isLoaded:
            return

        self.isLoaded = True

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/InventoryWindow.py")

        self.equipmentTabs = []
        self.equipmentTabs.append(self.GetChild("Equipment_Base"))
        self.equipmentTabs.append(self.GetChild("Equipment_Base2"))
        self.equipmentTabs.append(self.GetChild("Equipment_Base3"))
        self.equipmentTabs.append(self.GetChild("Equipment_Base4"))

        for i, tab in enumerate(self.equipmentTabs):
            # tab.SetOnRunMouseWheelEvent(self.OnRunMouseWheelInEquipment)
            tab.Hide()

        wndItem = self.GetChild("ItemSlot")

        self.equipmentSlots = []
        self.equipmentSlots.append(self.GetChild("EquipmentSlot"))
        self.equipmentSlots.append(self.GetChild("EquipmentSlot2"))
        self.equipmentSlots.append(self.GetChild("EquipmentSlot3"))
        self.equipmentSlots.append(self.GetChild("EquipmentSlot4"))

        self.GetChild("TitleBar").SetCloseEvent(self.Close)
        self.wndMoney = self.GetChild("Money")
        self.wndCash = self.GetChild("Cheque")
        self.wndMoneySlot = self.GetChild("Money_Slot")
        self.mallButton = self.GetOptionalChild("MallButton")
        self.shopSearchButton = self.GetOptionalChild("ShopSearchButton")
        self.DSSButton = self.GetOptionalChild("DSSButton")
        self.DungeonInfoButton = self.GetOptionalChild("DungeonInfoButton")
        self.ShopButton = self.GetOptionalChild("ShopButton")
        self.SwitchbotButton = self.GetOptionalChild("SwitchbotButton")
        self.WikiButton = self.GetOptionalChild("WikiButton")
        self.CubeButton = self.GetOptionalChild("CubeButton")
        self.HuntingButton = self.GetOptionalChild("HuntingButton")
        self.WorldBossButton = self.GetOptionalChild("WorldBossButton")
        # self.SkillTreeButton = self.GetOptionalChild("SkillTreeButton")

        self.inventoryTab = []
        self.inventoryTab.append(self.GetChild("Inventory_Tab_01"))
        self.inventoryTab.append(self.GetChild("Inventory_Tab_02"))
        self.inventoryTab.append(self.GetChild("Inventory_Tab_03"))
        self.inventoryTab.append(self.GetChild("Inventory_Tab_04"))
        self.inventoryTab.append(self.GetChild("Inventory_Tab_05"))

        self.wndMoneySlot.SetEvent(self.OpenPickMoneyDialog)

        for i, tab in enumerate(self.equipmentTabs):
            # tab.SetOnRunMouseWheelEvent(self.OnRunMouseWheelInEquipment)
            tab.Hide()

        self.inventoryCat = []
        self.inventoryCat.append(self.GetChild("Inventory_Cat_01"))
        self.inventoryCat.append(self.GetChild("Inventory_Cat_02"))
        self.inventoryCat.append(self.GetChild("Inventory_Cat_03"))

        self.equipmentTabButtons = []
        self.equipmentTabButtons.append(self.GetChild("Equipment_Tab_01"))
        self.equipmentTabButtons.append(self.GetChild("Equipment_Tab_02"))
        self.equipmentTabButtons.append(self.GetChild("Equipment_Tab_03"))
        self.equipmentTabButtons.append(self.GetChild("Equipment_Tab_04"))

        self.sortButton = self.GetChild("SortButton")
        self.sortButton.SetEvent(self.__SendSortInventoryPacket)

        #  Inventory Window
        self.wndSwitchBot = None

        self.dlgQuestion = uiCommon.QuestionDialog2()
        self.dlgQuestion.Close()

        self.wndSwitchBot = switchbot.SwitchbotWindow(self)

        self.listHighlightedAcceSlot = []

        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            self.listHighlightedChangeLookSlot = []

        if app.WJ_ENABLE_PICKUP_ITEM_EFFECT:
            self.listHighlightedSlot = []

        if app.ENABLE_GROWTH_PET_SYSTEM:
            self.PetItemQuestionDlg = uiCommon.QuestionDialog()
            self.PetItemQuestionDlg.Close()

        self.listCantMouseSlot = []

        # Item
        wndItem.SetWindowType(wndMgr.SLOT_WND_INVENTORY)
        wndItem.SetSelectEmptySlotEvent(Event(self.SelectEmptySlot, player.INVENTORY))
        wndItem.SetSelectItemSlotEvent(Event(self.SelectItemSlot, player.INVENTORY))
        wndItem.SetUnselectItemSlotEvent(Event(self.UseItemSlot, player.INVENTORY))
        wndItem.SetUseSlotEvent(Event(self.UseItemSlot, player.INVENTORY))
        wndItem.SetOverInItemEvent(Event(self.OverInItem, player.INVENTORY))
        wndItem.SetOverOutItemEvent(self.OverOutItem)

        # Equipment
        for wndEquip in self.equipmentSlots:
            wndEquip.SetSelectEmptySlotEvent(
                Event(self.SelectEmptySlot, player.EQUIPMENT)
            )
            wndEquip.SetSelectItemSlotEvent(
                Event(self.SelectItemSlot, player.EQUIPMENT)
            )
            wndEquip.SetUnselectItemSlotEvent(Event(self.UseItemSlot, player.EQUIPMENT))
            wndEquip.SetUseSlotEvent(Event(self.UseItemSlot, player.EQUIPMENT))
            wndEquip.SetOverInItemEvent(Event(self.OverInItem, player.EQUIPMENT))
            wndEquip.SetOverOutItemEvent(self.OverOutItem)

        ## SplitItemDialog
        dlgSplitItem = uiSplitItem.SplitItemDialog()
        dlgSplitItem.LoadDialog()
        dlgSplitItem.Hide()

        # AttachMetinDialog
        self.attachMetinDialog = uiAttachMetin.AttachMetinDialog()
        self.attachMetinDialog.Hide()

        self.detachMetinDialog = uiDetachMetin.DetachMetinDialog()
        self.detachMetinDialog.Hide()

        self.inventoryTab[0].SetEvent(Event(self.SetInventoryPage, 0))
        self.inventoryTab[1].SetEvent(Event(self.SetInventoryPage, 1))
        self.inventoryTab[2].SetEvent(Event(self.SetInventoryPage, 2))
        self.inventoryTab[3].SetEvent(Event(self.SetInventoryPage, 3))
        self.inventoryTab[4].SetEvent(Event(self.SetInventoryPage, 4))
        self.inventoryTab[0].Down()

        self.inventoryCat[0].SetEvent(Event(self.SetInventoryCat, 0))
        self.inventoryCat[1].SetEvent(Event(self.SetInventoryCat, 1))
        self.inventoryCat[2].SetEvent(Event(self.SetInventoryCat, 2))
        self.inventoryCat[0].Down()

        self.inventoryPageIndex = 0
        self.inventoryCatIndex = 0
        self.equipmentPageIndex = 0

        self.equipmentTabButtons[0].SetEvent(Event(self.SetEquipmentPage, 0))
        self.equipmentTabButtons[1].SetEvent(Event(self.SetEquipmentPage, 1))
        self.equipmentTabButtons[2].SetEvent(Event(self.SetEquipmentPage, 2))
        self.equipmentTabButtons[3].SetEvent(Event(self.SetEquipmentPage, 3))
        self.equipmentTabButtons[0].Down()
        self.equipmentTabButtons[0].Show()
        self.equipmentTabButtons[1].Show()
        self.equipmentTabButtons[2].Show()
        self.equipmentTabButtons[3].Show()

        self.wndItem = wndItem
        self.dlgSplitItem = dlgSplitItem

        # MallButton
        if self.mallButton:
            self.mallButton.SetEvent(self.ClickMallButton)

        if self.shopSearchButton:
            self.shopSearchButton.SetEvent(self.ClickShopSearchButton)

        # DSSButton
        if self.DSSButton:
            self.DSSButton.SetEvent(self.ClickDSSButton)
            self.DSSButton.Show()

        # DungeonInfoButton
        if self.DungeonInfoButton:
            self.DungeonInfoButton.SetEvent(self.ClickTimerButton)
            self.DungeonInfoButton.Show()

        # Shop button
        if self.ShopButton:
            self.ShopButton.SetEvent(self.ClickShopButton)

        if self.SwitchbotButton:
            self.SwitchbotButton.SetEvent(self.ToggleSwitchbotButton)

        if self.WikiButton:
            self.WikiButton.SetEvent(self.ToggleWikiButton)

        if self.CubeButton:
            self.CubeButton.SetEvent(self.ToggleCubeButton)
        if self.HuntingButton:
            self.HuntingButton.SetEvent(self.ToggleHuntingButton)
        if self.WorldBossButton:
            self.WorldBossButton.SetEvent(self.ToggleWorldBossButton)
        # if self.SkillTreeButton:
        #     self.SkillTreeButton.SetEvent(self.ToggleSkillTreeButton)

        self.MainBoard = self.GetChild("board")

        # Refresh
        self.SetInventoryPage(0)
        self.SetEquipmentPage(0)
        self.RefreshItemSlot()
        self.RefreshStatus()

    def Reconstruct(self):
        if self.wndSwitchBot:
            self.wndSwitchBot.Hide()
            self.wndSwitchBot.Destroy()
            self.wndSwitchBot = None

        self.wndSwitchBot = switchbot.SwitchbotWindow(self)

    def DestroySwitchbot(self):
        if self.wndSwitchBot:
            self.wndSwitchBot.Hide()
            self.wndSwitchBot.Destroy()
            self.wndSwitchBot = None

    def ClickShopButton(self):
        if not OfflineShopManager.GetInstance().GetEditorInstance().IsShow():
            OfflineShopManager.GetInstance().CreateShop()
        else:
            OfflineShopManager.GetInstance().GetEditorInstance().SetCenterPosition()
            OfflineShopManager.GetInstance().GetEditorInstance().Hide()

    def ToggleSwitchbotButton(self):
        if self.wndSwitchBot:
            if self.wndSwitchBot.IsShow():
                self.wndSwitchBot.Hide()
            else:
                self.wndSwitchBot.Show()

    def ToggleWikiButton(self):
        if self.interface:
            self.interface().ToggleWikiWindow()

    def ToggleHuntingButton(self):
        if self.interface:
            self.interface().ToggleHuntingMissions()

    def ToggleWorldBossButton(self):
        if self.interface:
            self.interface().ToggleWorldBossInfo()

    def ToggleSkillTreeButton(self):
        logging.debug("ToggleSkillTreeButton")
        if self.interface:
            self.interface().ToggleSkillTree()

    def ToggleCubeButton(self):
        if self.interface:
            if not self.interface().IsCubeWindowOpen():
                appInst.instance().GetNet().SendChatPacket("/cube open")
            else:
                appInst.instance().GetNet().SendChatPacket("/cube close")

    def Destroy(self):
        self.ClearDictionary()

        if self.dlgSplitItem:
            self.dlgSplitItem.Destroy()
            self.dlgSplitItem = 0

        if self.attachMetinDialog:
            self.attachMetinDialog.Destroy()
            self.attachMetinDialog = None

        if self.detachMetinDialog:
            self.detachMetinDialog.Destroy()
            self.detachMetinDialog = None

        self.tooltipItem = None
        self.wndItem = None
        self.wndEquip = None

        if self.equipmentSlots:
            del self.equipmentSlots[:]

        self.wndMoney = None
        self.wndMoneySlot = None
        self.mallButton = None
        self.DSSButton = None
        self.DungeonInfoButton = None
        self.interface = None
        self.questionDialog = None

        if self.wndSwitchBot:
            self.wndSwitchBot.Destroy()
            self.wndSwitchBot = None

        self.inventoryTab = []
        self.inventoryCat = []
        self.equipmentTabButtons = []

        if self.selectStorageDialog:
            self.selectStorageDialog.Destroy()
            self.selectStorageDialog = None

        if app.ENABLE_GROWTH_PET_SYSTEM:
            if self.petHatchingWindow:
                self.petHatchingWindow = None

            if self.petFeedWindow:
                self.petFeedWindow = None

    def Hide(self):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
            self.OnCloseQuestionDialog()
            return

        if None != self.tooltipItem:
            self.tooltipItem.HideToolTip()

        if self.wndSwitchBot:
            self.wndSwitchBot.Close()

        if self.dlgSplitItem:
            self.dlgSplitItem.Close()

        wndMgr.Hide(self.hWnd)

    def Close(self):
        if self.wndSwitchBot and self.wndSwitchBot.IsShow():
            self.wndSwitchBot.Hide()

        self.Hide()

    def SetInventoryPage(self, page):
        self.inventoryTab[self.inventoryPageIndex].SetUp()
        self.inventoryPageIndex = page
        self.inventoryTab[self.inventoryPageIndex].Down()
        self.RefreshBagSlotWindow()

    def SetInventoryCat(self, cat):
        self.inventoryCat[self.inventoryCatIndex].SetUp()
        self.inventoryCatIndex = cat
        self.inventoryCat[self.inventoryCatIndex].Down()
        self.SetInventoryPage(0)

    def SetEquipmentPage(self, page):
        self.equipmentTabButtons[self.equipmentPageIndex].SetUp()
        self.equipmentTabs[self.equipmentPageIndex].Hide()
        self.equipmentPageIndex = page
        self.equipmentTabButtons[page].Down()
        self.equipmentTabs[self.equipmentPageIndex].Show()

        self.RefreshEquipSlotWindow()

    def ClickMallButton(self):
        if not self.selectStorageDialog:
            self.selectStorageDialog = uiSelectStorage.SelectStorageDialog()
            self.selectStorageDialog.Show()
            return

        if not self.selectStorageDialog.IsShow():
            self.selectStorageDialog.Show()
        else:
            self.selectStorageDialog.Hide()

    def ClickShopSearchButton(self):
        if self.interface:
            self.interface().OpenShopSearch()

    # DSSButton
    def ClickDSSButton(self):
        if self.interface:
            self.interface().ToggleDragonSoulWindow()

    def ClickTimerButton(self):
        if self.interface:
            self.interface().ToggleDungeonInfoWindow()

    if app.ENABLE_GEM_SYSTEM:

        def OnPickGem(self, Gem=None):
            mouseModule.mouseController.AttachGem(self, player.SLOT_TYPE_INVENTORY, Gem)

    def OnRunMouseWheelInEquipment(self, nLen):
        inAny = [x for x in self.equipmentTabs if x.IsInPosition()]
        if inAny:
            if nLen > 0:
                self.SetEquipmentPage(
                    clamp_index(self.equipmentPageIndex + 1, self.equipmentTabs)
                )
            else:
                self.SetEquipmentPage(
                    clamp_index(self.equipmentPageIndex - 1, self.equipmentTabs)
                )

            return True
        return False

    def OpenPickMoneyDialog(self, focus_idx = 0):
        if mouseModule.mouseController.isAttached():
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
            if player.SLOT_TYPE_SAFEBOX == mouseModule.mouseController.GetAttachedType():
                if player.ITEM_MONEY == mouseModule.mouseController.GetAttachedItemIndex():
                    net.SendSafeboxWithdrawMoneyPacket(mouseModule.mouseController.GetAttachedItemCount())
                    snd.PlaySound("sound/ui/money.wav")

            mouseModule.mouseController.DeattachObject()

        else:
            curMoney = player.GetElk()
            if curMoney <= 0:
                return

            self.dlgPickMoney.SetTitleName(localeInfo.PICK_MONEY_TITLE)
            self.dlgPickMoney.SetAcceptEvent(self.OnPickMoney)
            self.dlgPickMoney.Open(curMoney)
            self.dlgPickMoney.SetMax(9)


    def OnPickMoney(self, money):
        mouseModule.mouseController.AttachMoney(self, player.SLOT_TYPE_INVENTORY, money)

    def OnPickItem(self, count):
        selectedItemVNum = playerInst().GetItemIndex(self.dlgSplitItem.pos)

        mouseModule.mouseController.AttachObject(
            self,
            player.SLOT_TYPE_INVENTORY,
            (self.dlgSplitItem.pos.windowType, self.dlgSplitItem.pos.cell),
            selectedItemVNum,
            count,
        )
        mouseModule.mouseController.SetRealAttachedSlotNumber(
            self.dlgSplitItem.pos.cell
        )

    def __InventoryLocalSlotPosToGlobalSlotPos(self, local):

        if (
            player.IsEquipmentSlot(local)
            or player.IsCostumeSlot(local)
            or player.IsBeltInventorySlot(local)
        ):
            return local

        base = self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE
        return base + local

    def __InventoryGlobalSlotPosToLocalSlotPos(self, globalPos):
        return globalPos - self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE

    def __LocalToGlobalSlot(self, local):
        base = (
            self.inventoryCatIndex * 5 * player.INVENTORY_PAGE_SIZE
        ) + self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE
        return base + local

    def RefreshSwitchbot(self):
        if self.wndSwitchBot:
            self.wndSwitchBot.RefreshSwitchbotData()

    def RefreshBagSlotWindow(self):
        if not self.isLoaded:
            return

        if not self.wndItem:
            return

        if self.interface:
            o = self.interface()
        else:
            o = self.interface

        onTopWindow = player.ON_TOP_WND_NONE
        if o:
            onTopWindow = o.GetOnTopWindow()

        pos = ItemPosition()
        pos.windowType = player.INVENTORY

        for i in xrange(player.INVENTORY_PAGE_SIZE):
            pos.cell = self.__LocalToGlobalSlot(i)
            self.wndItem.SetRealSlotNumber(i, pos.cell)

            if (
                self.detachMetinDialog
                and self.detachMetinDialog.GetTargetItemPos()
                and self.detachMetinDialog.GetTargetItemPos() == pos
            ):
                self.detachMetinDialog.Refresh()

            itemData = playerInst().GetItemData(pos)
            if not itemData:
                continue

            try:
                SetItemSlot(self.wndItem.hWnd, i, itemData)
            except Exception as e:
                logging.exception(e)

            if itemData.vnum and o and onTopWindow != player.ON_TOP_WND_NONE:
                if o.MarkUnusableInvenSlotOnTopWnd(onTopWindow, pos.cell):
                    self.wndItem.SetUnusableSlotOnTopWnd(i)
                else:
                    self.wndItem.SetUsableSlotOnTopWnd(i)
            else:
                self.wndItem.SetUsableSlotOnTopWnd(i)

        self.__CantMouseSlot_Refresh()

        self.wndItem.RefreshSlot()

        if o and o.dlgRefineNew:
            o.dlgRefineNew.UpdateMaterials()

        if self.wndSwitchBot:
            self.wndSwitchBot.RefreshSlot()

    def RefreshEquipSlotWindow(self):
        if not self.isLoaded:
            return

        pos = ItemPosition()
        pos.windowType = player.EQUIPMENT

        for wndEquip in self.equipmentSlots:
            for i in wndEquip.slotList:
                pos.cell = i
                itemData = playerInst().GetItemData(pos)
                if not itemData:
                    continue

                try:
                    SetItemSlot(wndEquip.hWnd, i, itemData)
                except Exception as e:
                    logging.exception(e)

            wndEquip.RefreshSlot()

    def SetOpenBoniSwitcherEvent(self, event):
        self.OpenBoniSwitcherEvent = MakeEvent(event)

    def RefreshItemSlot(self):
        self.RefreshBagSlotWindow()
        self.RefreshEquipSlotWindow()

    def RefreshStatus(self):
        money = playerInst().GetGold()
        if self.wndMoney:
            self.wndMoney.SetText("{} Yang".format(localeInfo.MoneyFormat(money)))
        if self.wndCash:
            self.wndCash.SetText(
                "{} Coins".format(localeInfo.MoneyFormat(playerInst().GetPoint(player.POINT_CASH)))
            )
        if app.ENABLE_GEM_SYSTEM:
            gem = player.GetGem()
            if self.wndGem:
                self.wndGem.SetText(localeInfo.MoneyFormat(gem))

    def SetItemToolTip(self, tooltipItemvalue):
        from _weakref import proxy

        self.tooltipItem = proxy(tooltipItemvalue)

    @staticmethod
    def ValuableItem(itemSlotPos):
        if player.GetItemGrade(itemSlotPos) >= 5:
            return True

        sockets = 0
        for i in xrange(5):
            if player.GetItemAttribute(itemSlotPos, i) != (0, 0):
                sockets += 1
        if sockets >= 3:
            return True

        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            if player.GetItemMetinSocket(itemSlotPos, i) not in (
                0,
                1,
                constInfo.ERROR_METIN_STONE,
            ):
                return True

        return False

    def SellItem(self):
        if self.sellingSlotitemIndex == player.GetItemIndex(self.sellingSlotNumber):
            if self.sellingSlotitemCount == player.GetItemCount(self.sellingSlotNumber):
                ## ¿ëÈ¥¼®µµ ÆÈ¸®°Ô ÇÏ´Â ±â´É Ãß°¡ÇÏ¸é¼­ ÀÎÀÚ type Ãß°¡
                appInst.instance().GetNet().SendShopSellPacketNew(
                    MakeItemPosition(player.INVENTORY, self.sellingSlotNumber),
                    self.questionDialog.count,
                )
                snd.PlaySound("sound/ui/money.wav")
        self.OnCloseQuestionDialog()

    def OnDetachMetinFromItem(self):
        if None == self.questionDialog:
            return

        # appInst.instance().GetNet().KNSW4ZCJORSW2VLTMVKG6SLUMVWVAYLDNNSXI(self.questionDialog.sourcePos, self.questionDialog.targetPos)
        self.__SendUseItemToItemPacket(
            self.questionDialog.sourcePos, self.questionDialog.targetPos
        )
        self.OnCloseQuestionDialog()

    def OnCloseQuestionDialog(self):
        if not self.questionDialog:
            return

        self.questionDialog.Close()
        self.questionDialog = None
        constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

    ## Slot Event
    def SelectEmptySlot(self, window, selectedSlotPos):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        if player.GetAcceRefineWindowOpen() == 1:
            return

        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            if player.GetChangeLookWindowOpen() == 1:
                return

        if window == player.INVENTORY:
            selectedSlotPos = self.__LocalToGlobalSlot(selectedSlotPos)

        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
            attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

            if OfflineShopManager.GetInstance().GetBuilderInstance().IsShow():
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_CREATING"),
                )
                mouseModule.mouseController.DeattachObject()
                return
            elif (
                OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
                and OfflineShopManager.GetInstance().GetItemCount() > 0
                and player.SLOT_TYPE_OFFLINE_SHOP != attachedSlotType
            ):
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_EDITING"),
                )
                mouseModule.mouseController.DeattachObject()
                return
            elif OfflineShopManager.GetInstance().GetViewerInstance().IsShow():
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_SHOPPING"),
                )
                mouseModule.mouseController.DeattachObject()
                return

            attPos = 0
            if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                attPos = MakeItemPosition(attachedSlotPos[0], attachedSlotPos[1])
            else:
                if isinstance(attachedSlotPos, tuple):
                    attPos = MakeItemPosition(attachedSlotPos)
                else:
                    attPos = MakeItemPosition(player.INVENTORY, attachedSlotPos)
            targetPos = MakeItemPosition(window, selectedSlotPos)

            if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                self.__SendMoveItemPacket(attPos, targetPos, attachedItemCount)

                if item.IsRefineScroll(attachedItemIndex):
                    o = self.interface()
                    o.SetUseItemMode(False)

            elif player.SLOT_TYPE_PRIVATE_SHOP == attachedSlotType:
                mouseModule.mouseController.RunCallBack("INVENTORY")

            elif player.SLOT_TYPE_SHOP == attachedSlotType:
                appInst.instance().GetNet().SendShopBuyPacket(attachedSlotPos)

            elif player.SLOT_TYPE_SAFEBOX == attachedSlotType:

                if player.ITEM_MONEY == attachedItemIndex:
                    appInst.instance().GetNet().SendSafeboxWithdrawMoneyPacket(
                        mouseModule.mouseController.GetAttachedItemCount()
                    )
                    snd.PlaySound("sound/ui/money.wav")

                else:
                    appInst.instance().GetNet().SendSafeboxCheckoutPacket(
                        attachedSlotPos, targetPos
                    )

            elif player.SLOT_TYPE_ACCE == attachedSlotType:
                appInst.instance().GetNet().SendAcceRefineCheckOut(attachedSlotPos)

            elif player.SLOT_TYPE_MALL == attachedSlotType:
                appInst.instance().GetNet().SendMallCheckoutPacket(
                    attachedSlotPos, targetPos
                )

            elif player.SLOT_TYPE_MYSHOP == attachedSlotType:
                mouseModule.mouseController.RunCallBack(
                    "INVENTORY", player.SLOT_TYPE_INVENTORY, selectedSlotPos
                )
            elif player.SLOT_TYPE_OFFLINE_SHOP == attachedSlotType:
                owner = mouseModule.mouseController.GetAttachedOwner()
                if not owner:
                    return

                owner.OnClickItemSlot(attachedSlotPos, selectedSlotPos)

            elif player.SLOT_TYPE_ACCE == attachedSlotType:
                appInst.instance().GetNet().SendAcceRefineCheckOut(attachedSlotPos)

            elif player.SLOT_TYPE_CHANGE_LOOK == attachedSlotType:
                appInst.instance().GetNet().SendChangeLookCheckoutPacket(
                    attachedSlotPos
                )

            mouseModule.mouseController.DeattachObject()

    def SelectItemSlot(self, window, itemSlotIndex):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return

        if window == player.INVENTORY:
            itemSlotIndex = self.__LocalToGlobalSlot(itemSlotIndex)

        dstPos = MakeItemPosition(window, itemSlotIndex)

        if mouseModule.mouseController.isAttached():
            if OfflineShopManager.GetInstance().GetBuilderInstance().IsShow():
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_CREATING"),
                )
                mouseModule.mouseController.DeattachObject()
                return
            elif (
                OfflineShopManager.GetInstance().GetEditorInstance().IsShow()
                and OfflineShopManager.GetInstance().GetItemCount() > 0
            ):
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_EDITING"),
                )
                mouseModule.mouseController.DeattachObject()
                return
            elif OfflineShopManager.GetInstance().GetViewerInstance().IsShow():
                chat.AppendChat(
                    CHAT_TYPE_INFO,
                    localeInfo.Get("OFFLINE_SHOP_CANNOT_MOVE_ITEMS_WHILE_SHOPPING"),
                )
                mouseModule.mouseController.DeattachObject()
                return

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            attachedItemVID = mouseModule.mouseController.GetAttachedItemIndex()

            if isinstance(attachedSlotPos, int):
                srcPos = MakeItemPosition(player.INVENTORY, attachedSlotPos)
            else:
                srcPos = MakeItemPosition(attachedSlotPos[0], attachedSlotPos[1])

            if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                self.__DropSrcItemToDestItemInInventory(attachedItemVID, srcPos, dstPos)
            mouseModule.mouseController.DeattachObject()

        else:

            curCursorNum = app.GetCursor()
            if app.SELL == curCursorNum:
                self.__SellItem(dstPos)

            elif app.BUY == curCursorNum:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)

            elif app.IsPressed(app.VK_MENU):
                link = playerInst().GetItemLink(dstPos)
                active = wndMgr.GetFocus()
                if isinstance(active, ui.EditLine):
                    active.Insert(link)

            elif app.IsPressed(app.VK_SHIFT):
                itemCount = playerInst().GetItemCount(dstPos)

                if itemCount > 1:
                    self.dlgSplitItem.SetTitleName(localeInfo.PICK_ITEM_TITLE)
                    self.dlgSplitItem.SetAcceptEvent(self.OnSplitItem)
                    self.dlgSplitItem.Open(itemCount)
                    self.dlgSplitItem.pos = dstPos

            elif app.IsPressed(app.VK_CONTROL):
                itemIndex = playerInst().GetItemIndex(dstPos)

                if item.CanAddToQuickSlotItem(itemIndex):
                    player.RequestAddToEmptyLocalQuickSlot(
                        player.SLOT_TYPE_INVENTORY, itemSlotIndex
                    )
                else:
                    chat.AppendChat(
                        CHAT_TYPE_INFO, localeInfo.QUICKSLOT_REGISTER_DISABLE_ITEM
                    )

            else:
                selectedItemVNum = playerInst().GetItemIndex(dstPos)
                itemCount = playerInst().GetItemCount(dstPos)
                if app.ENABLE_GROWTH_PET_SYSTEM:
                    if self.__CanAttachGrowthPetItem(selectedItemVNum, itemSlotIndex):
                        mouseModule.mouseController.AttachObject(
                            self,
                            player.SLOT_TYPE_INVENTORY,
                            (window, itemSlotIndex),
                            selectedItemVNum,
                            itemCount,
                        )
                else:
                    mouseModule.mouseController.AttachObject(
                        self,
                        player.SLOT_TYPE_INVENTORY,
                        (window, itemSlotIndex),
                        selectedItemVNum,
                        itemCount,
                    )

                mouseModule.mouseController.SetRealAttachedSlotNumber(itemSlotIndex)

                o = self.interface()
                if o:
                    if IsUsableItemToItem(selectedItemVNum, dstPos):
                        o.SetUseItemMode(True)
                    else:
                        o.SetUseItemMode(False)

                snd.PlaySound("sound/ui/pick.wav")

    def OnKeyDown(self, key):
        if key == app.VK_MENU:
            if self.wndItem and self.wndItem.GetUseMode():
                self.interface.SetUseItemMode(False)
                self.restoreUseMode = True

            self.skipUse = True
            return True
        return False

    def OnKeyUp(self, key):
        if key == app.VK_MENU:
            if self.restoreUseMode:
                self.interface.SetUseItemMode(True)

            self.restoreUseMode = False
            self.skipUse = False
            return True
        return False

    def OnSplitItem(self, count, full_split):
        if full_split:
            self.__SendSplitItemPacket(self.dlgSplitItem.pos, count)
        else:
            self.OnPickItem(count)

    def __DropSrcItemToDestItemInInventory(self, srcItemVnum, srcItemPos, dstItemPos):
        if srcItemPos == dstItemPos:
            return

        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            if player.GetChangeLookWindowOpen() == 1:
                return

        if player.GetAcceRefineWindowOpen() == 1:
            return

        if self.skipUse:
            self.__SendMoveItemPacket(srcItemPos, dstItemPos, 0)
            return

        srcVnum = playerInst().GetItemIndex(srcItemPos)
        targetVnum = playerInst().GetItemIndex(dstItemPos)

        if targetVnum == srcVnum:
            if (
                playerInst().GetItemFlags(dstItemPos) & item.ITEM_FLAG_STACKABLE
            ) == item.ITEM_FLAG_STACKABLE:
                self.__SendUseItemToItemPacket(srcItemPos, dstItemPos)
                return

        if app.ENABLE_ACCE_COSTUME_SYSTEM:
            # ¾Ç¼¼¼­¸® Ã¢ÀÌ ¿­·ÁÀÖÀ¸¸é
            # ¾ÆÀÌÅÛ ÀÌµ¿ ±ÝÁö.
            if player.GetAcceRefineWindowOpen() == 1:
                return

        srcItemData = itemManager().GetProto(srcItemVnum)

        if not srcItemData:
            return

        item.SelectItem(srcItemVnum)
        # cyh itemseal 2013 11 08
        if srcItemData.GetIndex() == 50263 or srcItemData.GetIndex() == 50264:
            if playerInst().CanSealItem(srcItemVnum, dstItemPos):
                self.__OpenQuestionDialog(srcItemPos, dstItemPos)
        elif srcItemData.GetIndex() == 72325:
            itemData = playerInst().GetItemData(dstItemPos)
            if itemData and itemData.transVnum != 0:
                self.__OpenQuestionDialog(srcItemPos, dstItemPos)

        elif (
            srcItemData.GetType() == item.ITEM_TYPE_USE
            and srcItemData.GetSubType() == item.USE_TUNING
            and playerInst().GetItemIndex(dstItemPos) != srcItemVnum
        ):
            self.RefineItem(srcItemPos, dstItemPos)
            o = self.interface()
            if o:
                o.SetUseItemMode(False)

        elif srcItemData.GetType() == item.ITEM_TYPE_METIN:
            self.AttachMetinToItem(srcItemPos, dstItemPos)

        elif (
            srcItemData.GetType() == item.ITEM_TYPE_USE
            and srcItemData.GetSubType() == item.USE_DETACHMENT
        ):
            self.detachMetinDialog.Open(dstItemPos)

        elif (
            srcItemData.GetType() == item.ITEM_TYPE_USE
            and srcItemData.GetSubType() == item.USE_DETACHMENT
        ):
            self.DetachMetinFromItem(srcItemPos, dstItemPos)

        elif srcItemData.GetType() == item.ITEM_TYPE_TREASURE_KEY:
            self.__SendUseItemToItemPacket(srcItemPos, dstItemPos)
        elif srcItemData.IsFlag(ITEM_FLAG_APPLICABLE):
            self.__SendUseItemToItemPacket(srcItemPos, dstItemPos)

        elif srcItemData.GetSubType() in self.USE_TYPE_TUPLE:
            self.__SendUseItemToItemPacket(srcItemPos, dstItemPos)
        else:

            if app.ENABLE_GROWTH_PET_SYSTEM:
                if self.__IsPetItem(srcItemVnum):
                    if self.__SendUsePetItemToItemPacket(
                        srcItemVnum, srcItemPos, dstItemPos
                    ):
                        return
            # snd.PlaySound("sound/ui/drop.wav")
            # if app.ENABLE_ACCE_COSTUME_SYSTEM:
            #    if item.IsAcceScroll(srcItemVnum):
            #        if player.CanAcceClearItem(srcItemVnum, player.INVENTORY, dstItemSlotPos):
            #            self.__OpenQuestionDialog(srcItemSlotPos, dstItemSlotPos)
            #            return

            if dstItemPos.windowType == player.EQUIPMENT:
                self.__UseItem(srcItemPos)
            else:
                self.__SendMoveItemPacket(srcItemPos, dstItemPos, 0)
                # appInst.instance().GetNet().KNSW4ZCJORSW2TLPOZSVAYLDNNSXI(srcItemSlotPos, dstItemSlotPos, 0)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __UseItemToItemPetItem(self, srcItemSlotPos, dstItemSlotPos):

            getItemVNum = player.GetItemIndex
            self.srcItemPos = srcItemSlotPos
            self.dstItemPos = dstItemSlotPos

            srcItemVnum = getItemVNum(srcItemSlotPos)
            dstItemVnum = getItemVNum(dstItemSlotPos)

            item.SelectItem(srcItemVnum)
            srcItemType = item.GetItemType()
            srcItemSubType = item.GetItemSubType()

            item.SelectItem(dstItemVnum)
            dstItemType = item.GetItemType()
            dstItemSubType = item.GetItemSubType()

            ## src item ÀÌ »ç·áÀÏ°æ¿ì...
            if item.ITEM_TYPE_PET == srcItemType:
                if item.PET_FEEDSTUFF == srcItemSubType:
                    self.__OpenFeedQuestionDialog(srcItemSlotPos, dstItemSlotPos)

                elif item.PET_BAG == srcItemSubType:
                    self.__OpenPetBagQuestionDialog(srcItemSlotPos, dstItemSlotPos)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __OpenPetBagQuestionDialog(self, srcItemSlotPos, dstItemSlotPos):
            o = self.interface()
            if o and o.IsShowDlgQuestionWindow():
                o.CloseDlgQuestionWindow()

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __OpenPetItemQuestionDialog(self, srcItemPos, dstItemPos):
            o = self.interface()

            if o and o.IsShowDlgQuestionWindow():
                o.CloseDlgQuestionWindow()

            getItemVNum = player.GetItemIndex
            self.srcItemPos = srcItemPos
            self.dstItemPos = dstItemPos

            srcItemVnum = getItemVNum(srcItemPos)
            dstItemVnum = getItemVNum(dstItemPos)

            item.SelectItem(srcItemVnum)
            src_item_name = item.GetItemNameByVnum(srcItemVnum)
            srcItemType = item.GetItemType()
            srcItemSubType = item.GetItemSubType()

            item.SelectItem(dstItemVnum)
            dst_item_name = item.GetItemName(getItemVNum(dstItemPos))

            self.PetItemQuestionDlg.SetAcceptEvent(self.__PetItemAccept)
            self.PetItemQuestionDlg.SetCancelEvent(self.__PetItemCancel)

            if item.ITEM_TYPE_PET == srcItemType:
                if item.PET_FEEDSTUFF == srcItemSubType:
                    self.PetItemQuestionDlg.SetText(
                        localeInfo.INVENTORY_REALLY_USE_PET_FEEDSTUFF_ITEM.format(
                            src_item_name, dst_item_name
                        )
                    )
                    self.PetItemQuestionDlg.Open()

                elif item.PET_BAG == srcItemSubType:
                    self.PetItemQuestionDlg.SetText(
                        localeInfo.INVENTORY_REALLY_USE_PET_BAG_ITEM
                    )
                    self.PetItemQuestionDlg.Open()

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __PetItemAccept(self):
            self.PetItemQuestionDlg.Close()
            self.__SendUseItemToItemPacket(self.srcItemPos, self.dstItemPos)
            self.srcItemPos = MakeItemPosition(0, 0)
            self.dstItemPos = MakeItemPosition(0, 0)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __PetItemCancel(self):
            self.srcItemPos = MakeItemPosition(0, 0)
            self.dstItemPos = MakeItemPosition(0, 0)
            self.PetItemQuestionDlg.Close()

    def __OpenQuestionDialog(self, srcItemPos, dstItemPos):
        o = self.interface()

        if o and o.IsShowDlgQuestionWindow():
            o.CloseDlgQuestionWindow()

        self.srcItemPos = srcItemPos
        self.dstItemPos = dstItemPos

        self.dlgQuestion.SetAcceptEvent(self.__Accept)
        self.dlgQuestion.SetCancelEvent(self.__Cancel)

        self.dlgQuestion.SetText1(
            "%s" % item.GetItemName(playerInst().GetItemIndex(srcItemPos))
        )
        self.dlgQuestion.SetText2(localeInfo.INVENTORY_REALLY_USE_ITEM)

        self.dlgQuestion.Open()

    def __Accept(self):
        self.dlgQuestion.Close()
        self.__SendUseItemToItemPacket(self.srcItemPos, self.dstItemPos)
        self.srcItemPos = MakeItemPosition(0, 0)
        self.dstItemPos = MakeItemPosition(0, 0)

    def __Cancel(self):
        self.srcItemPos = MakeItemPosition(0, 0)
        self.dstItemPos = MakeItemPosition(0, 0)
        self.dlgQuestion.Close()

    def __SellItem(self, itemSlotPos):
        if not player.IsEquipmentSlot(itemSlotPos):
            self.sellingSlotNumber = itemSlotPos
            itemIndex = player.GetItemIndex(itemSlotPos)
            itemCount = player.GetItemCount(itemSlotPos)

            self.sellingSlotitemIndex = itemIndex
            self.sellingSlotitemCount = itemCount

            item.SelectItem(itemIndex)
            itemPrice = item.GetISellItemPrice()

            if item.Is1GoldItem():
                itemPrice = itemCount / itemPrice / 5
            else:
                itemPrice = itemPrice * itemCount / 5

            itemName = self._GetItemName(itemIndex)

            self.questionDialog = uiCommon.QuestionDialog()
            self.questionDialog.SetText(
                localeInfo.DO_YOU_SELL_ITEM(itemName, itemCount, itemPrice)
            )
            self.questionDialog.SetAcceptEvent(self.SellItem)
            self.questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
            self.questionDialog.Open()
            self.questionDialog.count = itemCount

            constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

    def __OnClosePopupDialog(self):
        self.pop = None

    def RefineItem(self, scrollSlotPos, targetSlotPos):
        scrollIndex = playerInst().GetItemIndex(scrollSlotPos)
        targetIndex = playerInst().GetItemIndex(targetSlotPos)

        if player.REFINE_OK != playerInst().CanRefine(scrollIndex, targetSlotPos):
            return

        constInfo.AUTO_REFINE_TYPE = 1
        constInfo.AUTO_REFINE_DATA["ITEM"][0] = scrollSlotPos
        constInfo.AUTO_REFINE_DATA["ITEM"][1] = targetSlotPos

        self.__SendUseItemToItemPacket(scrollSlotPos, targetSlotPos)
        return

    def DetachMetinFromItem(self, scrollPos, targetPos):
        scrollIndex = playerInst().GetItemIndex(scrollPos)
        targetIndex = playerInst().GetItemIndex(targetPos)

        if not playerInst().CanDettachMetin(scrollIndex, targetPos):
            chat.AppendChat(
                CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_METIN_INSEPARABLE_ITEM
            )
            return

        self.questionDialog = uiCommon.QuestionDialog()
        self.questionDialog.SetText(localeInfo.REFINE_DO_YOU_SEPARATE_METIN)
        self.questionDialog.SetAcceptEvent(self.OnDetachMetinFromItem)
        self.questionDialog.SetCancelEvent(self.OnCloseQuestionDialog)
        self.questionDialog.Open()
        self.questionDialog.sourcePos = scrollPos
        self.questionDialog.targetPos = targetPos

    def AttachMetinToItem(self, metinSlotPos, targetSlotPos):
        metinIndex = playerInst().GetItemIndex(metinSlotPos)
        targetIndex = playerInst().GetItemIndex(targetSlotPos)

        item.SelectItem(metinIndex)
        itemName = item.GetItemName()

        result = playerInst().CanAttachMetin(metinIndex, targetSlotPos)

        if player.ATTACH_METIN_NOT_MATCHABLE_ITEM == result:
            chat.AppendChat(
                CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_CAN_NOT_ATTACH(itemName)
            )

        if player.ATTACH_METIN_NO_MATCHABLE_SOCKET == result:
            chat.AppendChat(
                CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NO_SOCKET(itemName)
            )

        elif player.ATTACH_METIN_NOT_EXIST_GOLD_SOCKET == result:
            chat.AppendChat(
                CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NO_GOLD_SOCKET(itemName)
            )

        elif player.ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT == result:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_EQUIP_ITEM)

        elif player.ATTACH_METIN_SEALED == result:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_SEALITEM)

        if player.ATTACH_METIN_OK != result:
            return

        self.attachMetinDialog.Open(metinSlotPos, targetSlotPos)

    def _GetItemName(self, index):
        item.SelectItem(index)

        return item.GetItemName()

    def OverOutItem(self):
        app.SetCursor(app.NORMAL)
        if self.wndItem:
            self.wndItem.SetUsableItem(False)
            if None != self.tooltipItem:
                self.tooltipItem.HideToolTip()

    def OverInItem(self, window, slot):
        app.SetCursor(app.PICK)

        overSlot = MakeItemPosition(
            window,
            self.__LocalToGlobalSlot(slot) if window == player.INVENTORY else slot,
        )

        if self.wndItem:
            self.wndItem.SetUsableItem(False)

        self.DelHighlightSlot(slot, overSlot)

        if mouseModule.mouseController.isAttached():
            attachedItemType = mouseModule.mouseController.GetAttachedType()
            if player.SLOT_TYPE_INVENTORY == attachedItemType:
                attachedLocalSlotPos = (
                    mouseModule.mouseController.GetAttachedSlotPosition()
                )
                attachedItemVNum = mouseModule.mouseController.GetAttachedItemIndex()

                if self.__CanUseSrcItemToDstItem(
                    attachedItemVNum,
                    MakeItemPosition(attachedLocalSlotPos[0], attachedLocalSlotPos[1]),
                    overSlot,
                ):
                    if self.wndItem:
                        self.wndItem.SetUsableItem(True)
                        self.ShowToolTip(window, overSlot.cell)
                    return

        self.ShowToolTip(window, overSlot.cell)

    def __CanUseSrcItemToDstItem(self, srcItemVNum, srcSlotPos, dstSlotPos):
        if srcSlotPos == dstSlotPos:
            return False

        itemData = itemManager().GetProto(srcItemVNum)

        if not itemData:
            return False

        if (
            itemData.GetType() == item.ITEM_TYPE_USE
            and itemData.GetSubType() == item.USE_TUNING
        ):
            if player.REFINE_OK == playerInst().CanRefine(srcItemVNum, dstSlotPos):
                return True
        elif itemData.GetType() == item.ITEM_TYPE_METIN:
            if player.ATTACH_METIN_OK == playerInst().CanAttachMetin(
                srcItemVNum, dstSlotPos
            ):
                return True
        elif (
            itemData.GetType() == item.ITEM_TYPE_USE
            and itemData.GetSubType() == item.USE_DETACHMENT
        ):
            if player.DETACH_METIN_OK == playerInst().CanDettachMetin(
                srcItemVNum, dstSlotPos
            ):
                return True
        elif itemData.GetIndex() == 50263 or itemData.GetIndex() == 50264:
            if playerInst().CanSealItem(srcItemVNum, dstSlotPos):
                return True
        elif itemData.GetIndex() == 72325:
            itemData = playerInst().GetItemData(dstSlotPos)
            if itemData and itemData.transVnum != 0:
                return True
        elif itemData.GetType() == item.ITEM_TYPE_TREASURE_KEY:
            if playerInst().CanUnlockTreasure(srcItemVNum, dstSlotPos):
                return True

        elif itemData.IsFlag(ITEM_FLAG_APPLICABLE):
            return True

        else:
            if app.ENABLE_GROWTH_PET_SYSTEM:
                if self.__CanUseSrcPetItemToDstPetItem(
                    srcItemVNum, srcSlotPos, dstSlotPos
                ):
                    return True
            useType = itemData.GetSubType()
            if item.USE_CLEAN_SOCKET == useType:
                if self.__CanCleanBrokenMetinStone(dstSlotPos):
                    return True
            elif (
                item.USE_CHANGE_ATTRIBUTE == useType
                or item.USE_CHANGE_ATTRIBUTE_PERM == useType
            ):
                if self.__CanChangeItemAttrList(dstSlotPos):
                    return True
            elif (
                item.USE_ADD_ATTRIBUTE == useType
                or item.USE_ADD_ATTRIBUTE_PERM == useType
            ):
                if self.__CanAddItemAttr(dstSlotPos):
                    return True
            elif item.USE_ADD_ATTRIBUTE2 == useType:
                if self.__CanAddItemAttr2(dstSlotPos):
                    return True
            elif item.USE_ADD_ATTRIBUTE_RARE == useType:
                if self.__CanAddItemAttrRare(dstSlotPos):
                    return True
            elif item.USE_CHANGE_ATTRIBUTE_RARE == useType:
                if self.__CanChangeItemAttrRare(dstSlotPos):
                    return True
            elif item.USE_ADD_ACCESSORY_SOCKET == useType:
                if self.__CanAddAccessorySocket(dstSlotPos):
                    return True
            elif item.USE_MAKE_ACCESSORY_SOCKET_PERM == useType:
                if self.__CanMakeAccessorySocketPerm(dstSlotPos):
                    return True
            elif item.USE_PUT_INTO_ACCESSORY_SOCKET == useType:
                if self.__CanPutAccessorySocket(dstSlotPos, srcItemVNum):
                    return True
            elif item.USE_PUT_INTO_BELT_SOCKET == useType:
                dstItemVNum = playerInst().GetItemIndex(dstSlotPos)

                item.SelectItem(dstItemVNum)

                if item.ITEM_TYPE_BELT == item.GetItemType():
                    return True
            elif item.USE_CHANGE_COSTUME_ATTR == useType:
                if self.__CanChangeCostumeAttrList(dstSlotPos):
                    return True
            elif item.USE_RESET_COSTUME_ATTR == useType:
                if self.__CanResetCostumeAttr(dstSlotPos):
                    return True
            elif item.USE_LEVEL_PET_FOOD == useType:
                if self.__CanFeedLevelPet(dstSlotPos):
                    return True
            elif item.USE_ADD_SOCKETS == useType:
                if self.__CanAddSockets(dstSlotPos):
                    return True

        return False

    def __CanCleanBrokenMetinStone(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if item.ITEM_TYPE_WEAPON != item.GetItemType():
            return False

        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            if (
                playerInst().GetItemMetinSocket(dstSlotPos, i)
                == constInfo.ERROR_METIN_STONE
            ):
                return True

        return False

    def __CanChangeItemAttrList(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
            return False

        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            if playerInst().GetItemAttribute(dstSlotPos, i).type != 0:
                return True

        return False

    def __CanChangeItemAttrRare(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)

        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
            return False
        for i in xrange(2):
            if playerInst().GetItemAttribute(dstSlotPos, i + 5).type != 0:
                return True
        return False

    def __CanPutAccessorySocket(self, dstSlotPos, mtrlVnum):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if item.GetItemType() != item.ITEM_TYPE_ARMOR:
            return False

        if not item.GetItemSubType() in (
            item.ARMOR_WRIST,
            item.ARMOR_NECK,
            item.ARMOR_EAR,
        ):
            return False

        curCount = playerInst().GetItemMetinSocket(dstSlotPos, 0)
        maxCount = playerInst().GetItemMetinSocket(dstSlotPos, 1)

        if mtrlVnum != constInfo.GET_ACCESSORY_MATERIAL_VNUM(
            dstItemVNum, item.GetItemSubType()
        ):
            return False

        if curCount >= maxCount:
            return False

        return True

    def __CanMakeAccessorySocketPerm(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if item.GetItemType() != item.ITEM_TYPE_BELT:
            if item.GetItemType() != item.ITEM_TYPE_ARMOR:
                return False

            if not item.GetItemSubType() in (
                item.ARMOR_WRIST,
                item.ARMOR_NECK,
                item.ARMOR_EAR,
            ):
                return False

        isPerm = playerInst().GetItemMetinSocket(dstSlotPos, 3)
        if isPerm:
            return False

        return True

    def __CanAddAccessorySocket(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if item.GetItemType() != item.ITEM_TYPE_BELT:
            if item.GetItemType() != item.ITEM_TYPE_ARMOR:
                return False

            if not item.GetItemSubType() in (
                item.ARMOR_WRIST,
                item.ARMOR_NECK,
                item.ARMOR_EAR,
            ):
                return False

        curCount = playerInst().GetItemMetinSocket(dstSlotPos, 0)
        maxCount = playerInst().GetItemMetinSocket(dstSlotPos, 1)

        ACCESSORY_SOCKET_MAX_SIZE = 3
        if maxCount >= ACCESSORY_SOCKET_MAX_SIZE:
            return False

        return True

    def __CanAddItemAttr(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
            return False

        attrCount = 0
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attr = playerInst().GetItemAttribute(dstSlotPos, i)
            if attr.type != 0:
                attrCount += 1

        if attrCount < 4:
            return True

        return False

    def __CanAddItemAttrRare(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)

        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
            return False

        attrCount = 0
        for i in xrange(2):
            if playerInst().GetItemAttribute(dstSlotPos, i + 5).type != 0:
                attrCount += 1

        if attrCount < 2:
            return True

        return False

    def __CanAddItemAttr2(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        item.SelectItem(dstItemVNum)

        if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
            return False

        attrCount = 0
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attr = playerInst().GetItemAttribute(i)
            if attr.type != 0:
                attrCount += 1

        if attrCount == 4:
            return True

        return False

    def __CanChangeCostumeAttrList(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        itemData = itemManager().GetProto(dstItemVNum)
        itemType = itemData.GetType()
        itemSubType = itemData.GetSubType()

        if not itemType == item.ITEM_TYPE_COSTUME:
            return False

        if itemSubType not in (item.COSTUME_TYPE_BODY, item.COSTUME_TYPE_HAIR):
            return False

        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            if playerInst().GetItemAttribute(dstSlotPos, i).type != 0:
                return True

        return False

    def __CanFeedLevelPet(self, dstSlotPos):
        return True

    def __CanAddSockets(self, dstSlotPos):
        itemData = playerInst().GetItemData(dstSlotPos)
        if not itemData:
            return False

        if itemData.vnum == 0:
            return False

        proto = itemManager().GetProto(itemData.vnum)
        itemType = proto.GetType()
        itemSubType = proto.GetSubType()

        if itemType != item.ITEM_TYPE_WEAPON and (
            itemType != item.ITEM_TYPE_ARMOR or itemSubType != item.ARMOR_BODY
        ):
            return False

        socketCount = 0
        for i, val in enumerate(itemData.sockets):
            if val == 0:
                socketCount = i
                logging.debug("SocketCount %d", socketCount)
                break

        if socketCount < 2:
            return False

        return True

    def __CanResetCostumeAttr(self, dstSlotPos):
        dstItemVNum = playerInst().GetItemIndex(dstSlotPos)
        if dstItemVNum == 0:
            return False

        itemData = itemManager().GetProto(dstItemVNum)
        itemType = itemData.GetType()
        itemSubType = itemData.GetSubType()

        if not itemType == item.ITEM_TYPE_COSTUME:
            return False

        if itemSubType not in (item.COSTUME_TYPE_BODY, item.COSTUME_TYPE_HAIR):
            return False

        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            if playerInst().GetItemAttribute(dstSlotPos, i).type != 0:
                return True

        return False

    def ShowToolTip(self, window, slotIndex):
        if self.tooltipItem:
            self.tooltipItem.SetInventoryItem(slotIndex, window)

    def OnTop(self):
        if self.tooltipItem is not None:
            self.tooltipItem.SetTop()

        if self.wndSwitchBot:
            self.wndSwitchBot.SetTop()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Hide()
            return True

    def UseItemSlot(self, window, slotIndex):
        curCursorNum = app.GetCursor()
        if app.SELL == curCursorNum:
            return

        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
            return

        if window == player.INVENTORY:
            slotIndex = self.__LocalToGlobalSlot(slotIndex)

        if app.ENABLE_DRAGON_SOUL_SYSTEM:
            if self.wndDragonSoulRefine.IsShow():
                self.wndDragonSoulRefine.AutoSetItem((player.INVENTORY, slotIndex), 1)
                return

        if OfflineShopManager.GetInstance().GetBuilderInstance().IsShow():
            OfflineShopManager.GetInstance().GetBuilderInstance().AddItemByClick(
                MakeItemPosition(window, slotIndex)
            )
            return

        self.__UseItem(MakeItemPosition(window, slotIndex))

        mouseModule.mouseController.DeattachObject()
        self.OverOutItem()

    def __UseItem(self, slotPos):
        o = self.interface()
        if o:
            if (
                app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL)
            ) and o.wndSafebox.IsShow():
                appInst.instance().GetNet().SendSafeboxCheckinPacket(slotPos, 0)
                return
            if (
                app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL)
            ) and o.dlgExchange.IsShow():
                appInst.instance().GetNet().SendExchangeItemAddPacket(slotPos, 0)
                return

            ItemVNum = playerInst().GetItemIndex(slotPos)
            item.SelectItem(ItemVNum)

            if item.GetItemType() == item.ITEM_TYPE_GIFTBOX:
                if app.IsPressed(app.VK_CONTROL) or app.IsPressed(app.VK_CONTROL):
                    self.__SendUseItemMultiplePacket(slotPos)
                    return

            if player.GetAcceRefineWindowOpen() == 1:
                self.__UseItemAcce(slotPos)
                return

            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                if player.GetChangeLookWindowOpen() == 1:
                    return

            if o.AttachInvenItemToOtherWindowSlot(slotPos.cell):
                return

            if app.ENABLE_GROWTH_PET_SYSTEM:
                itemType = item.GetItemType()
                if item.ITEM_TYPE_PET == itemType:
                    self.__UseItemPet(slotPos.windowType, slotPos.cell)
                    return

            if item.IsFlag(item.ITEM_FLAG_CONFIRM_WHEN_USE):
                self.questionDialog = uiCommon.QuestionDialog()
                if not slotPos.windowType == player.EQUIPMENT:
                    self.questionDialog.SetText(localeInfo.INVENTORY_REALLY_USE_ITEM2)
                elif (
                    item.GetItemType() == item.ITEM_TYPE_USE
                    and item.GetItemSubType() == item.USE_BATTLEPASS
                ):
                    self.questionDialog.SetText(
                        localeInfo.Get("CONFIRM_WHEN_USE_BATTLE_PASS")
                    )
                else:
                    self.questionDialog.SetText(
                        localeInfo.INVENTORY_REALLY_STOP_USE_ITEM
                    )

                self.questionDialog.SetAcceptEvent(
                    self.__UseItemQuestionDialog_OnAccept
                )
                self.questionDialog.SetCancelEvent(
                    self.__UseItemQuestionDialog_OnCancel
                )
                self.questionDialog.slotPos = slotPos
                self.questionDialog.Open()

                constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
            else:
                if ItemVNum == 71084:
                    if self.wndSwitchBot:
                        self.wndSwitchBot.Show()
                else:
                    self.__SendUseItemPacket(slotPos)

    if app.ENABLE_GROWTH_PET_SYSTEM:
        ##Æê UseItemPet
        def __UseItemPet(self, slotWindow, slotIndex):
            itemSubType = item.GetItemSubType()
            if item.PET_EGG == itemSubType:
                self.petHatchingWindow.HatchingWindowOpen(slotIndex)

            elif item.PET_UPBRINGING == itemSubType:
                if player.CanUsePetCoolTimeCheck():
                    if self.__CanUseGrowthPet(slotIndex):
                        self.__SendUseItemPacket(slotWindow, slotIndex)

            elif item.PET_BAG == itemSubType:
                if self.__CanUsePetBagItem(slotIndex):
                    if self.questionDialog:
                        self.questionDialog.Close()

                    self.questionDialog = uiCommon.QuestionDialog()
                    self.questionDialog.SetText(
                        localeInfo.INVENTORY_REALLY_USE_PET_BAG_TAKE_OUT
                    )
                    self.questionDialog.SetAcceptEvent(
                        self.__UseItemQuestionDialog_OnAccept
                    )
                    self.questionDialog.SetCancelEvent(
                        self.__UseItemQuestionDialog_OnCancel
                    )
                    self.questionDialog.slotIndex = slotIndex
                    self.questionDialog.Open()

    if app.ENABLE_GROWTH_PET_SYSTEM:
        # Æê ITEM_FLAG_CONFIRM_WHEN_USE ÇÃ·¡±×°¡ ¹ö±×°¡ ÀÖ¾î ¸¸µë
        # ¾ÆÀÌÅÛ »ç¿ë½Ã ´ÙÀÌ¾ó·Î±× Ã¢À» ¶ç¿ö¼­ »ç¿ë¿©ºÎ¸¦ µÎÁÙ Ãâ·ÂÇÏ¿© ¹°¾îº½.
        # Ã¹ÁÙ : ¾ÆÀÌÅÛ¸í, µÎ¹øÂ°ÁÙ : localeInfo.INVENTORY_REALLY_USE_ITEM
        def __OpenQuestionDialog2(self, slotIndex):
            o = self.interface()
            if o and o.IsShowDlgQuestionWindow():
                o.CloseDlgQuestionWindow()

            self.OpenQuestionDialog2SlotIndex = slotIndex
            self.dlgQuestion.SetAcceptEvent(self.__QuestionDialog2Accept)
            self.dlgQuestion.SetCancelEvent(self.__QuestionDialog2Cancel)

            self.dlgQuestion.SetText1(
                "%s" % item.GetItemNameByVnum(player.GetItemIndex(slotIndex))
            )
            self.dlgQuestion.SetText2(localeInfo.INVENTORY_REALLY_USE_ITEM)

            self.dlgQuestion.Open()

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __QuestionDialog2Accept(self):
            self.dlgQuestion.Close()
            self.__SendUseItemPacket(self.OpenQuestionDialog2SlotIndex)
            self.OpenQuestionDialog2SlotIndex = (0, 0)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __QuestionDialog2Cancel(self):
            self.OpenQuestionDialog2SlotIndex = (0, 0)
            self.dlgQuestion.Close()

    if app.ENABLE_ACCE_COSTUME_SYSTEM:

        def __UseItemAcce(self, slotPos):

            AcceSlot = player.FineMoveAcceItemSlot()
            UsingAcceSlot = player.FindActivedAcceSlot(slotPos.cell)
            if slotPos.windowType == player.EQUIPMENT:  ## ÀÎº¥Ã¢ ¾È¿¡ ÀÖ´Â °Í¸¸.
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_USINGITEM)
                return

            ## ºÀÀÎ¾ÆÀÌÅÛ °É·¯³¿
            if player.GetItemSealDate(slotPos.windowType, slotPos.cell) == -1:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_NOT_SEALITEM)
                return

            ## Á¶ÇÕÃ¢
            if player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_COMBINE:
                if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                    if item.GetItemSubType() == item.COSTUME_TYPE_ACCE:
                        if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                            ## Àü¼³µî±Þ ÃÖ°í Èí¼öÀ² 25%
                            socketInDrainValue = player.GetItemMetinSocket(
                                slotPos.cell, 0
                            )
                            if socketInDrainValue >= app.ACCE_MAX_DRAINRATE:
                                chat.AppendChat(
                                    CHAT_TYPE_INFO, localeInfo.ACCE_MAX_DRAINRATE
                                )
                                return
                        else:
                            if item.GetRefinedVnum() == 0:  ## Àü¼±µî±Þ ¾ÆÀÌÅÛÀº °É·¯³¿.
                                chat.AppendChat(
                                    CHAT_TYPE_INFO, localeInfo.ACCE_MAXGRADE
                                )
                                return

                        if UsingAcceSlot == player.ACCE_SLOT_MAX:
                            if AcceSlot != player.ACCE_SLOT_MAX:
                                if (
                                    not player.FindUsingAcceSlot(AcceSlot)
                                    == player.ITEM_SLOT_COUNT
                                ):
                                    return
                                if (
                                    player.FindUsingAcceSlot(player.ACCE_SLOT_LEFT)
                                    == player.ITEM_SLOT_COUNT
                                ):
                                    if AcceSlot == player.ACCE_SLOT_RIGHT:
                                        return
                                player.SetAcceActivedItemSlot(AcceSlot, slotPos.cell)
                                appInst.instance().GetNet().SendAcceRefineCheckIn(
                                    slotPos, AcceSlot, player.GetAcceRefineWindowType()
                                )
                    else:
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                        return
                else:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCE)
                    return

            ## Èí¼öÃ¢
            elif player.GetAcceRefineWindowType() == player.ACCE_SLOT_TYPE_ABSORB:
                isAbsorbItem = 0
                if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                    if item.GetItemSubType() == item.COSTUME_TYPE_ACCE:
                        if not player.GetItemMetinSocket(slotPos.cell, 1) == 0:
                            chat.AppendChat(
                                CHAT_TYPE_INFO, localeInfo.ACCE_NOTABSORBITEM
                            )
                            return
                        if UsingAcceSlot == player.ACCE_SLOT_MAX:
                            if (
                                not AcceSlot == player.ACCE_SLOT_LEFT
                            ):  ## ¾Ç¼¼¼­¸®¾ÆÀÌÅÛÀº ¿ÞÂÊ ½½·Ô¿¡¸¸
                                return
                            player.SetAcceActivedItemSlot(AcceSlot, slotPos.cell)
                            appInst.instance().GetNet().SendAcceRefineCheckIn(
                                slotPos, AcceSlot, player.GetAcceRefineWindowType()
                            )
                    else:
                        chat.AppendChat(
                            CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCEITEM
                        )
                        return

                elif item.GetItemType() == item.ITEM_TYPE_WEAPON:  ## ¹«±â·ù.
                    isAbsorbItem = 1

                elif item.GetItemType() == item.ITEM_TYPE_ARMOR:  ## °©¿Ê·ù
                    if item.GetItemSubType() == item.ARMOR_BODY:
                        isAbsorbItem = 1
                    else:
                        chat.AppendChat(
                            CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCEITEM
                        )
                        return
                else:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.ACCE_POSSIBLE_ACCEITEM)
                    return

                if isAbsorbItem:
                    if UsingAcceSlot == player.ACCE_SLOT_MAX:
                        if (
                            not AcceSlot == player.ACCE_SLOT_RIGHT
                        ):  ## Èí¼öµÉ ¾ÆÀÌÅÛÀº ¿À¸¥ÂÊ ½½·Ô¿¡¸¸
                            if (
                                player.FindUsingAcceSlot(player.ACCE_SLOT_RIGHT)
                                == player.ITEM_SLOT_COUNT
                            ):
                                AcceSlot = player.ACCE_SLOT_RIGHT
                            else:
                                return

                        popup = uiCommon.QuestionDialog()
                        popup.SetText(localeInfo.ACCE_DEL_ABSORDITEM)
                        popup.SetAcceptEvent(
                            Event(self.OnAcceAcceptEvent, slotPos.cell, AcceSlot)
                        )
                        popup.SetCancelEvent(self.OnAcceCloseEvent)
                        popup.Open()
                        popup.SetTop()
                        self.pop = popup

            ## °æ°í ¸Þ½ÃÁö ¶ç¿ì±â.
            if (
                not player.FindUsingAcceSlot(player.ACCE_SLOT_RIGHT)
                == player.ITEM_SLOT_COUNT
                and not player.FindUsingAcceSlot(player.ACCE_SLOT_LEFT)
                == player.ITEM_SLOT_COUNT
            ):
                if AcceSlot != player.ACCE_SLOT_MAX:
                    popup = uiCommon.PopupDialog()
                    if (
                        player.GetAcceRefineWindowType()
                        == player.ACCE_SLOT_TYPE_COMBINE
                    ):

                        if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                            socketInDrainValue = player.GetAcceItemMetinSocket(0, 0)
                            socketInDrainValue2 = player.GetAcceItemMetinSocket(1, 0)
                            socketInDrainValue3 = player.GetItemMetinSocket(
                                slotPos.cell, 0
                            )
                            ## ¸ÞÀÎ ¼­¹ö Áß. µî·ÏµÈ ¾ÆÀÌÅÛÀÌ Àü¼³ÀÏ¶§ °æ°í ¸Þ½ÃÁö º¯°æ.
                            if (
                                socketInDrainValue > 0
                                or socketInDrainValue2 > 0
                                or socketInDrainValue3 > 0
                            ):
                                popup.SetText(localeInfo.ACCE_DEL_SERVEITEM2)
                            else:
                                popup.SetText(localeInfo.ACCE_DEL_SERVEITEM)
                        else:
                            popup.SetText(localeInfo.ACCE_DEL_SERVEITEM)

                        popup.SetAcceptEvent(self.__OnClosePopupDialog)
                        popup.Open()
                        popup.SetTop()
                        self.pop = popup

    ## ¾ÆÀÌÅÛ Èí¼ö½Ã Èí¼öµÉ ¾ÆÀÌÅÛ ÇÒÁö ¾ÈÇÒÁö ¼±ÅÃ ÆË¾÷
    if app.ENABLE_ACCE_COSTUME_SYSTEM:

        def OnAcceAcceptEvent(self, slotIndex, AcceSlot):
            self.pop.Close()
            self.pop = None
            player.SetAcceActivedItemSlot(AcceSlot, slotIndex)
            appInst.instance().GetNet().SendAcceRefineCheckIn(
                player.INVENTORY, slotIndex, AcceSlot, player.GetAcceRefineWindowType()
            )

        def OnAcceCloseEvent(self):
            self.pop.Close()
            self.pop = None

    def __UseItemQuestionDialog_OnCancel(self):
        self.OnCloseQuestionDialog()

    def __UseItemQuestionDialog_OnAccept(self):
        self.__SendUseItemPacket(self.questionDialog.slotPos)
        self.OnCloseQuestionDialog()

    def __SendSortInventoryPacket(self):
        if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
            return
        appInst.instance().GetNet().SendChatPacket("/sort_inventory")

    def __SendUseItemToItemPacket(self, srcItemPos, dstItemPos):
        # ???λ??? ???? ?ִ? ???? ?????? ???? ????
        o = self.interface()
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        appInst.instance().GetNet().SendItemUseToItemPacket(srcItemPos, dstItemPos)

    def __SendUseItemPacket(self, slotPos):
        # ???λ??? ???? ?ִ? ???? ?????? ???? ????
        o = self.interface()
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        appInst.instance().GetNet().SendItemUsePacket(slotPos)

    def __SendUseItemMultiplePacket(self, slotPos):
        o = self.interface()

        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        appInst.instance().GetNet().SendItemUseMultiplePacket(slotPos)

    def __SendMoveItemPacket(self, srcPos, dstPos, count):
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        appInst.instance().GetNet().SendItemMovePacket(srcPos, dstPos, count)

    def __SendSplitItemPacket(self, srcPos, count):
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
            return

        appInst.instance().GetNet().SendItemSplitPacket(srcPos, count)

    def SetDragonSoulRefineWindow(self, wndDragonSoulRefine):
        from _weakref import proxy

        self.wndDragonSoulRefine = proxy(wndDragonSoulRefine)

    def SetCantMouseEvent(self, slotindex):
        self.CantMouseSlot(slotindex)

    def SetCanMouseEvent(self, slotindex):
        self.DelCantMouseSlot(slotindex)

    # ¾Ç¼¼¼­¸® ½½·Ô Å×µÎ¸® Ãß°¡, Á¦°Å.
    ## HilightSlot Change
    def DeactivateSlot(self, slotindex, type):
        if type == wndMgr.HILIGHTSLOT_ACCE:
            self.__DelHighlightSlotAcce(slotindex)
        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            if type == wndMgr.HILIGHTSLOT_CHANGE_LOOK:
                self.__DelHighlightSlotChangeLook(slotindex)

    ## HilightSlot Change
    def ActivateSlot(self, slotindex, type):
        if type == wndMgr.HILIGHTSLOT_ACCE:
            self.__AddHighlightSlotAcce(slotindex)
        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            if type == wndMgr.HILIGHTSLOT_CHANGE_LOOK:
                self.__AddHighlightSlotChangeLook(slotindex)

    if app.ENABLE_CHANGE_LOOK_SYSTEM:

        def __AddHighlightSlotChangeLook(self, slotIndex):
            if not slotIndex in self.listHighlightedChangeLookSlot:
                self.listHighlightedChangeLookSlot.append(slotIndex)

        def __DelHighlightSlotChangeLook(self, slotIndex):
            if slotIndex in self.listHighlightedChangeLookSlot:
                if slotIndex >= player.INVENTORY_PAGE_SIZE:
                    self.wndItem.DeactivateSlot(
                        slotIndex
                        - (self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE)
                    )
                else:
                    self.wndItem.DeactivateSlot(slotIndex)
                self.listHighlightedChangeLookSlot.remove(slotIndex)

    ## ¾Ç¼¼¼­¸® ¿ë ÇÏÀÌ¶óÀÌÆ® list Ãß°¡.
    def __AddHighlightSlotAcce(self, slotIndex):
        if not slotIndex in self.listHighlightedAcceSlot:
            self.listHighlightedAcceSlot.append(slotIndex)

    ## ¾Ç¼¼¼­¸® ¿ë ÇÏÀÌ¶óÀÌÆ® list »èÁ¦.
    def __DelHighlightSlotAcce(self, slotIndex):
        if slotIndex in self.listHighlightedAcceSlot:

            if slotIndex >= player.INVENTORY_PAGE_SIZE:
                self.wndItem.DeactivateSlot(
                    slotIndex - (self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE)
                )
            else:
                self.wndItem.DeactivateSlot(slotIndex)

            self.listHighlightedAcceSlot.remove(slotIndex)

    ## ÇÏÀÌ¶óÀÌÆ® ¸®ÇÁ·¹½¬.
    def __HighlightSlot_Refresh(self):
        pass

    ## ¾Ç¼¼¼­¸® ¿ë ÇÏÀÌ¶óÀÌÆ® list Å¬¸®¾î.
    def HighlightSlot_Clear(self):
        ## ¾Ç¼¼¼­¸®
        for i in xrange(self.wndItem.GetSlotCount()):
            slotNumber = self.__LocalToGlobalSlot(i)
            if slotNumber in self.listHighlightedSlot:
                self.wndItem.DeactivateSlot(i)
                self.listHighlightedSlot.remove(slotNumber)

    def HighlightSlot(self, slot):
        pass

    ## »èÁ¦
    def DelHighlightSlot(self, localSlot, globalPos):
        if self.wndItem:
            itemData = playerInst().GetItemData(globalPos)
            if not itemData:
                return

            OverItemSlot(self.wndItem.hWnd, localSlot, itemData)

    def CantMouseSlot(self, slot):
        # slot°ª¿¡ ´ëÇÑ ¿¹¿ÜÃ³¸®.
        if slot > player.INVENTORY_PAGE_SIZE * player.INVENTORY_PAGE_COUNT:
            return

        if not slot in self.listCantMouseSlot:
            self.listCantMouseSlot.append(slot)

    ## »èÁ¦
    def DelCantMouseSlot(self, invenSlot):
        if invenSlot in self.listCantMouseSlot:
            self.wndItem.SetCanMouseEventSlot(
                self.__InventoryGlobalSlotPosToLocalSlotPos(invenSlot)
            )

            self.listCantMouseSlot.remove(invenSlot)

    def __CantMouseSlot_Clear(self):
        for i in xrange(self.wndItem.GetSlotCount()):
            slotNumber = self.__LocalToGlobalSlot(i)
            if slotNumber in self.listCantMouseSlot:
                self.wndItem.SetCanMouseEventSlot(i)
                self.listCantMouseSlot.remove(slotNumber)

    def __CantMouseSlot_Refresh(self):
        for i in xrange(self.wndItem.GetSlotCount()):
            slotNumber = self.__LocalToGlobalSlot(i)
            if slotNumber in self.listCantMouseSlot:
                self.wndItem.SetCantMouseEventSlot(i)

    def IsDlgQuestionShow(self):
        if self.dlgQuestion.IsShow():
            return True
        else:
            return False

    def CancelDlgQuestion(self):
        self.__Cancel()

    def SetUseItemMode(self, bUse):
        self.wndItem.SetUseMode(bUse)

    if app.ENABLE_GROWTH_PET_SYSTEM:
        ## È°¼ºÇÑ Æê ÇÏÀÌ¶óÀÌÆ® Ç¥½Ã
        def __ActivePetHighlightSlot(self, slotNumber):
            active_id = player.GetActivePetItemId()

            if active_id == 0:
                return

            if active_id == player.GetItemMetinSocket(
                player.INVENTORY, slotNumber, 2
            ):  ## 0 ~ 89

                if slotNumber >= player.INVENTORY_PAGE_SIZE:
                    slotNumber -= player.INVENTORY_PAGE_SIZE

                self.wndItem.ActivateSlot(slotNumber)  ## 0 ~ 44

    if app.ENABLE_GROWTH_PET_SYSTEM:
        ## ³²Àº ½Ã°£¿¡ µû¸¥ ÄðÅ¸ÀÓ Ç¥½Ã( slotNumber ´Â 0 ~ 89 )
        def __SetCollTimePetItemSlot(self, slotNumber, itemVnum):

            item.SelectItem(itemVnum)
            itemSubType = item.GetItemSubType()

            if itemSubType not in [item.PET_UPBRINGING, item.PET_BAG]:
                return

            if itemSubType == item.PET_BAG:
                id = player.GetItemMetinSocket(player.INVENTORY, slotNumber, 2)
                if id == 0:
                    return

            (limitType, limitValue) = item.GetLimit(0)

            # À°¼ºÆêÀÇ LimitValue ´Â 1¹ø ¼ÒÄÏ¿¡ ÀÖ´Ù.
            if itemSubType == item.PET_UPBRINGING:
                limitValue = player.GetItemMetinSocket(player.INVENTORY, slotNumber, 1)

            if limitType in [
                item.LIMIT_REAL_TIME,
                item.LIMIT_REAL_TIME_START_FIRST_USE,
            ]:

                sock_time = player.GetItemMetinSocket(player.INVENTORY, slotNumber, 0)

                remain_time = max(0, sock_time - app.GetGlobalTimeStamp())

                if slotNumber >= player.INVENTORY_PAGE_SIZE:
                    slotNumber -= player.INVENTORY_PAGE_SIZE

                ## SetSlotCoolTimeInverse ÀÇ slotNumber´Â 0 ~ 44
                self.wndItem.SetSlotCoolTimeInverse(
                    slotNumber, limitValue, limitValue - remain_time
                )

                # print "item Limit TYPE: ", limitType
                # print "item Limit VALUE: ", limitValue
                # print "³²Àº ½Ã°£ : ", remain_time
                # print "°æ°ú ½Ã°£ : ", limitValue - remain_time

    if app.ENABLE_GROWTH_PET_SYSTEM or app.ENABLE_MOVE_COSTUME_ATTR:

        def GetInventoryPageIndex(self):
            return self.inventoryPageIndex

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __IsPetItem(self, srcItemVID):
            item.SelectItem(srcItemVID)

            if item.GetItemType() == item.ITEM_TYPE_PET:
                return True

            return False

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __SendUsePetItemToItemPacket(
            self, srcItemVID, srcItemSlotPos, dstItemSlotPos
        ):
            if self.__CanUseSrcPetItemToDstPetItem(
                srcItemVID, srcItemSlotPos, dstItemSlotPos
            ):
                self.__OpenPetItemQuestionDialog(srcItemSlotPos, dstItemSlotPos)
                return True

            return False

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __IsUsablePetItem(self, srcItemVNum):

            item.SelectItem(srcItemVNum)
            srcItemType = item.GetItemType()
            srcItemSubType = item.GetItemSubType()

            if srcItemType != item.ITEM_TYPE_PET:
                return False

            if srcItemSubType not in [item.PET_FEEDSTUFF, item.PET_BAG]:
                return False

            return True

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __CanUseSrcPetItemToDstPetItem(self, srcItemVNum, srcSlotPos, dstSlotPos):

            item.SelectItem(srcItemVNum)
            srcItemType = item.GetItemType()
            srcItemSubType = item.GetItemSubType()

            if srcItemType != item.ITEM_TYPE_PET:
                return False

            if srcItemSubType == item.PET_FEEDSTUFF:
                detIndex = player.GetItemIndex(dstSlotPos)
                item.SelectItem(detIndex)

                dstItemType = item.GetItemType()
                dstItemSubType = item.GetItemSubType()

                if dstItemType != item.ITEM_TYPE_PET:
                    return False

                if dstItemSubType not in [item.PET_UPBRINGING]:
                    return False

                if dstItemSubType == item.PET_BAG:
                    incaseTime = player.GetItemMetinSocket(
                        player.INVENTORY, dstSlotPos, 1
                    )
                    if incaseTime == 0:
                        return False

            elif srcItemSubType == item.PET_BAG:

                if (
                    player.GetItemSealDate(player.INVENTORY, dstSlotPos)
                    != item.E_SEAL_DATE_DEFAULT_TIMESTAMP
                ):
                    return False

                detIndex = player.GetItemIndex(dstSlotPos)
                item.SelectItem(detIndex)

                dstItemType = item.GetItemType()
                dstItemSubType = item.GetItemSubType()

                if dstItemType != item.ITEM_TYPE_PET:
                    return False

                if dstItemSubType not in [item.PET_UPBRINGING, item.PET_BAG]:
                    return False

                lifeTime = player.GetItemMetinSocket(player.INVENTORY, dstSlotPos, 0)

                if dstItemSubType == item.PET_UPBRINGING:
                    ## °¡¹æ ¾ÆÀÌÅÛÀ» ±â°£ÀÌ Áö³­ À°¼ºÆê¿¡ »ç¿ëºÒ°¡
                    if lifeTime < app.GetGlobalTimeStamp():
                        return False

                    ## °¡¹æ ¾ÆÀÌÅÛ¿¡ ÀÌ¹Ì µé¾îÀÖ´Â »óÅÂ¿¡¼­´Â À°¼ºÆê¿¡ »ç¿ëºÒ°¡
                    srcIncase = playerInst().GetItemMetinSocket(srcSlotPos, 1)
                    if srcIncase != 0:
                        return False

                elif dstItemSubType == item.PET_BAG:
                    ## °¡¹æ ¾ÆÀÌÅÛÀ» °¡¹æ¿¡ »ç¿ëÇÒ¶§´Â dest°¡¹æÀÇ ±â°£ÀÌ ´Ù Áö³ª¾ß ÇÑ´Ù.
                    if lifeTime > app.GetGlobalTimeStamp():
                        return False

                    ## »ç¿ëÇÏ·Á´Â °¡¹æÀº ºñ¾î ÀÖ¾î¾ß ÇÑ´Ù.
                    srcIncase = playerInst().GetItemMetinSocket(srcSlotPos, 1)
                    if srcIncase != 0:
                        return False

                    ## ´ë»ó °¡¹æÀº ºñ¾îÀÖÀ¸¸é ¾ÈµÈ´Ù.
                    destIncase = playerInst().GetItemMetinSocket(dstSlotPos, 1)
                    if destIncase == 0:
                        return False

            else:
                return False

            return True

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __CanUseGrowthPet(self, slotIndex):

            if not player.GetItemMetinSocket(player.INVENTORY, slotIndex, 2):
                return False

            (limitType, limitValue) = item.GetLimit(0)
            remain_time = 999
            if item.LIMIT_REAL_TIME == limitType:
                sock_time = player.GetItemMetinSocket(player.INVENTORY, slotIndex, 0)
                if app.GetGlobalTimeStamp() > sock_time:
                    chat.AppendChat(
                        CHAT_TYPE_INFO,
                        localeInfo.PET_CAN_NOT_SUMMON_BECAUSE_LIFE_TIME_END,
                    )
                    return False

            return True

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __CanUsePetBagItem(self, slotIndex):

            if not player.GetItemMetinSocket(player.INVENTORY, slotIndex, 2):
                return False

            (limitType, limitValue) = item.GetLimit(0)
            remain_time = 999
            if item.LIMIT_REAL_TIME_START_FIRST_USE == limitType:
                sock_time = player.GetItemMetinSocket(player.INVENTORY, slotIndex, 0)
                use_cnt = player.GetItemMetinSocket(player.INVENTORY, slotIndex, 1)

                if use_cnt:
                    if app.GetGlobalTimeStamp() > sock_time:
                        chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_USE_BAG)
                        return False

            return True

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __CanAttachGrowthPetItem(self, itemVNum, itemSlotIndex):

            activePetId = player.GetActivePetItemId()
            if activePetId == 0:
                return True

            item.SelectItem(itemVNum)
            itemType = item.GetItemType()
            itemSubType = item.GetItemSubType()

            if item.ITEM_TYPE_PET == itemType and itemSubType == item.PET_UPBRINGING:
                petId = player.GetItemMetinSocket(itemSlotIndex, 2)
                if petId == activePetId:
                    return False

            return True

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def SetPetHatchingWindow(self, window):
            self.petHatchingWindow = window

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def SetPetFeedWindow(self, window):
            self.petFeedWindow = window

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def ItemMoveFeedWindow(self, slotIndex):

            if not self.petFeedWindow:
                return

            self.petFeedWindow.ItemMoveFeedWindow(slotIndex)

    def SetExpandedMoneyBar(self, wndMoneyTaskbar):
        from _weakref import proxy

        self.wndExpandedMoneyBar = proxy(wndMoneyTaskbar)
        if self.wndExpandedMoneyBar:
            self.wndMoneySlot = self.wndExpandedMoneyBar.GetMoneySlot()
            self.wndMoney = self.wndExpandedMoneyBar.GetMoney()
            self.wndCashSlot = self.wndExpandedMoneyBar.GetChequeSlot()
            self.wndCash = self.wndExpandedMoneyBar.GetCheque()

            if app.ENABLE_GEM_SYSTEM:
                self.wndGemSlot = self.wndExpandedMoneyBar.GetGemSlot()
                self.wndGem = self.wndExpandedMoneyBar.GetGem()
