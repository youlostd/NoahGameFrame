# -*- coding: utf-8 -*-


import logging

import app
import chat
import item
import player
import skill
import wndMgr

import localeInfo
import mouseModule
import ui
import uiCommon
import uiScriptLocale

SKILL_SLOT_ENABLE = "d:/ymir work/ui/pet/skill_button/skill_enable_button.sub"
SKILL_SLOT_MAX = 3

TOTAL_EXP_GAUGE_COUNT = 5
BATTLE_EXP_GAUGE_MAX = 4
ITEM_EXP_GAUGE_POS = 4

FEED_WINDOW_X_SIZE = 3
FEED_WINDOW_Y_SIZE = 3


# Æê ¹Ì´Ï Á¤º¸Ã¢
class PetMiniInfomationWindow(ui.ScriptWindow):
    def __init__(self, wndPetInformation):
        

        if not wndPetInformation:
            logging.exception("wndPetInformation parameter must be set to PetInformationWindow")
            return

        ui.ScriptWindow.__init__(self)

        self.skillSlot = []
        self.isLoaded = 0
        self.wndPetInformation = wndPetInformation
        self.petSlot = 0
        self.petSlotAniImg = None
        self.expGauge = None
        self.expGaugeBoard = None

        self.__LoadWindow()

    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)

    def Close(self):
        if self.petSlot:
            self.petSlot.SetItemSlot(0, 0)

        if self.tooltipEXP:
            self.tooltipEXP.Hide()

        self.Hide()

    def Destroy(self):
        self.isLoaded = 0
        self.wndPetInformation = 0
        self.lifeTimeGauge = None
        self.petSlot = 0
        self.petSlotAniImg = None
        self.expGauge = None
        self.expGaugeBoard = None
        self.tooltipEXP = None

        if self.skillSlot:
            del self.skillSlot[:]

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.__LoadScript("UIScript/petMiniInformationWindow.py")
        self.isLoaded = 1


        try:
            ###        BG
            if localeInfo.IsARABIC():
                self.GetChild("main_bg").LeftRightReverse()

            ## Pet Icon Slot
            self.petSlot = self.GetChild("pet_icon_slot")
            self.petSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
            self.petSlot.SetButtonEvent("LEFT", "EXIST", self.SelectItemSlot)
            if localeInfo.IsARABIC():
                self.petSlot.SetPosition(0, 6)

            ## Pet Icon Slot Animation Image - Flash
            self.petSlotAniImg = self.GetChild("pet_icon_slot_ani_img")
            self.petSlotAniImg.Hide()
            if localeInfo.IsARABIC():
                self.petSlotAniImg.SetPosition(34, 3)

            ##        EXP GAUGE
            expGauge = []
            self.expGaugeBoard = self.GetChild("pet_mini_info_exp_gauge_board")
            expGauge.append(self.GetChild("pet_mini_EXPGauge_01"))
            expGauge.append(self.GetChild("pet_mini_EXPGauge_02"))
            expGauge.append(self.GetChild("pet_mini_EXPGauge_03"))
            expGauge.append(self.GetChild("pet_mini_EXPGauge_04"))
            expGauge.append(self.GetChild("pet_mini_EXPGauge_05"))

            for exp in expGauge:
                exp.SetSize(0, 0)

            self.expGauge = expGauge
            self.tooltipEXP = ui.TextToolTip()
            self.tooltipEXP.Hide()

            ## Mini Info Skill Slot Scale
            for value in range(SKILL_SLOT_MAX):
                self.skillSlot.append(self.GetChild("mini_skill_slot" + str(value)))
                self.skillSlot[value].SetCoverButton(0, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE,
                                                     SKILL_SLOT_ENABLE, False, False)
                self.skillSlot[value].SetAlwaysRenderCoverButton(0)

                if localeInfo.IsARABIC():
                    ## 13, 33, 53 / 36, 56, 76
                    arabic_start_pos_x = -23
                    self.skillSlot[value].SetPosition(arabic_start_pos_x, 0)

            ##    Life Time Gauge
            self.lifeTimeGauge = self.GetChild("LifeGauge")
            self.lifeTimeGauge.SetHorizontalAlignLeft()
            if localeInfo.IsARABIC():
                self.GetChild("gauge_left").LeftRightReverse()
                self.GetChild("gauge_right").LeftRightReverse()

        except:
            
            logging.exception("PetMiniInfomationWindow.LoadWindow.BindObject")

        self.Hide()

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)

    def OnUpdate(self):

        if self.expGaugeBoard.IsIn():
            self.tooltipEXP.Show()
        else:
            self.tooltipEXP.Hide()

        return

    def SetItemSlot(self, CurPetItemVNum):
        self.petSlot.SetItemSlot(0, CurPetItemVNum)
        self.petSlot.RefreshSlot()

    def SetSkillSlot(self, slotNumber, slotIndex, skillVnum):
        if 0 > slotNumber or slotNumber >= SKILL_SLOT_MAX:
            return

        self.skillSlot[slotNumber].SetPetSkillSlotNew(slotIndex, skillVnum)
        self.skillSlot[slotNumber].SetCoverButton(slotIndex)

    def SetSkillCoolTime(self, slotNumber, slotIndex, max_cool_time, cur_cool_time):
        self.skillSlot[slotNumber].SetSlotCoolTime(slotIndex, max_cool_time, cur_cool_time)
        self.skillSlot[slotNumber].SetSlotCoolTimeColor(slotIndex, 0.0, 1.0, 0.0, 0.5)

    def ClearSkillSlot(self):

        for value in range(SKILL_SLOT_MAX):
            self.skillSlot[value].ClearSlot(0)
            self.skillSlot[value].SetCoverButton(0, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE,
                                                 SKILL_SLOT_ENABLE, False, False)
            self.skillSlot[value].SetAlwaysRenderCoverButton(0)

    def SetAlwaysRenderCoverButton(self, slotNumber):
        self.skillSlot[slotNumber].SetAlwaysRenderCoverButton(0, False)

    def SelectItemSlot(self):

        if not self.wndPetInformation:
            return

        if self.wndPetInformation.IsShow():
            self.wndPetInformation.Close()
        else:
            self.wndPetInformation.Show()

    def SetLifeTime(self, curPoint, maxPoint):
        curPoint = min(curPoint, maxPoint)
        if maxPoint > 0:
            self.lifeTimeGauge.SetPercentageWithScale(curPoint, maxPoint)

    def SetExperience(self, curPoint, maxPoint, itemExp, itemExpMax):

        curPoint = min(curPoint, maxPoint)
        curPoint = max(curPoint, 0)
        maxPoint = max(maxPoint, 0)

        itemExp = min(itemExp, itemExpMax)
        itemExp = max(itemExp, 0)
        itemExpMax = max(itemExpMax, 0)

        ## »ç³ÉÀ¸·Î È¹µæÇÑ °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
        quarterPoint = maxPoint / BATTLE_EXP_GAUGE_MAX
        FullCount = 0

        if 0 != quarterPoint:
            FullCount = min(BATTLE_EXP_GAUGE_MAX, curPoint / quarterPoint)

        for i in xrange(TOTAL_EXP_GAUGE_COUNT):
            self.expGauge[i].Hide()

        for i in xrange(int(FullCount)):
            self.expGauge[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
            self.expGauge[i].Show()

        if 0 != quarterPoint:
            if FullCount < BATTLE_EXP_GAUGE_MAX:
                Percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
                self.expGauge[int(FullCount)].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
                self.expGauge[int(FullCount)].Show()

        ## ¾ÆÀÌÅÛÀ¸·Î È¹µæÇÑ °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
        ## self.expGauge ÀÇ ¸¶Áö¸· °ªÀÌ item exp ±¸½½ÀÌ´Ù.
        ## Top °ªÀÌ 0 ÀÌ¸é ²ËÂù ±¸½½
        ## Top °ªÀÌ -1 ÀÌ¸é ºó ±¸½½
        if 0 != itemExpMax:
            itemExpGauge = self.expGauge[ITEM_EXP_GAUGE_POS]
            Percentage = float(itemExp) / float(itemExpMax) - float(1.0)
            itemExpGauge.SetRenderingRect(0.0, Percentage, 0.0, 0.0)
            itemExpGauge.Show()

        output_cur_exp = curPoint + itemExp
        output_max_exp = maxPoint + itemExpMax

        ## TEXT Ãâ·ÂÀº »ç³É°æÇèÄ¡ + ¾ÆÀÌÅÛ °æÇèÄ¡·Î ÇÑ´Ù.
        if app.WJ_MULTI_TEXTLINE:

            if localeInfo.IsARABIC():
                tooltip_text = str(curPoint) + ' :' + str(localeInfo.PET_INFO_EXP) + '\\n' \
                               + str(maxPoint - curPoint) + ' :' + str(localeInfo.PET_INFO_NEXT_EXP) + '\\n' \
                               + str(itemExp) + ' :' + str(localeInfo.PET_INFO_ITEM_EXP) + '\\n' \
                               + str(itemExpMax - itemExp) + ' :' + str(localeInfo.PET_INFO_NEXT_ITEM_EXP)
                self.tooltipEXP.SetText(tooltip_text)
            else:
                tooltip_text = str(localeInfo.PET_INFO_EXP) + ': ' + str(curPoint) + '\\n' \
                               + str(localeInfo.PET_INFO_NEXT_EXP) + ': ' + str(maxPoint - curPoint) + '\\n' \
                               + str(localeInfo.PET_INFO_ITEM_EXP) + ': ' + str(itemExp) + '\\n' \
                               + str(localeInfo.PET_INFO_NEXT_ITEM_EXP) + ': ' + str(itemExpMax - itemExp)

                self.tooltipEXP.SetText(tooltip_text)
        else:
            self.tooltipEXP.SetText("%s : %.2f%%" % (
            localeInfo.TASKBAR_EXP, float(output_cur_exp) / max(1, float(output_max_exp - output_cur_exp)) * 100))

    def OnFlashEvent(self):
        if self.petSlotAniImg:
            self.petSlotAniImg.Show()

    def OffFlashEvent(self):
        if self.petSlotAniImg:
            self.petSlotAniImg.Hide()


# Æê ºÎÈ­ Ã¢
class PetHatchingWindow(ui.ScriptWindow):
    def __init__(self, wndPetInformation):
        

        if not wndPetInformation:
            logging.exception("wndPetInformation parameter must be set to PetInformationWindow")
            return

        ui.ScriptWindow.__init__(self)

        self.isLoaded = 0
        self.hatchingSlot = 0
        self.eggItemSlotIndex = -1
        self.wndPetInformation = wndPetInformation
        self.hatchingButton = 0
        self.petNameEdit = 0
        self.petName = 0
        self.questionDialog = 0
        self.popupDialog = 0

        self.__LoadWindow()


    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)
        self.SetTop()

    def Close(self):
        self.ClearMouseEventEggItem()
        self.hatchingSlot.SetItemSlot(0, 0)
        self.hatchingSlot.RefreshSlot()
        self.petName = 0

        self.Hide()
        player.SetOpenPetHatchingWindow(False)
        #appInst.instance().GetNet().SendPetHatchingWindowPacket(False)

        if self.questionDialog:
            self.questionDialog.Close()

        if self.popupDialog:
            self.popupDialog.Close()

        if self.petNameEdit:
            self.petNameEdit.KillFocus()

    def Destroy(self):
        self.isLoaded = 0
        self.hatchingSlot = 0
        self.wndPetInformation = 0
        self.hatchingButton = 0
        self.petName = 0

        if self.popupDialog:
            self.popupDialog.Destroy()

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        try:
            self.__LoadScript("UIScript/petHatchingWindow.py")
        except:
            logging.exception("petHatchingWindow.LoadWindow.LoadObject")

        try:
            self.GetChild("PetHatching_TitleBar").SetCloseEvent(self.Close)
            self.hatchingSlot = self.GetChild("HatchingItemSlot")
            self.hatchingSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
            self.hatchingSlot.SetOverInItemEvent(self.OverInItem)
            self.hatchingSlot.SetOverOutItemEvent(self.OverOutItem)

            ## ºÎÈ­ ¹öÆ°
            self.hatchingButton = self.GetChild("HatchingButton")
            self.hatchingButton.SetEvent(self.ClickHatchingButton)

            ## ºÎÈ­ °ñµå TEXT
            hatchingMoneyText = self.GetChild("HatchingMoney");
            hatchingMoneyText.SetText(
                localeInfo.PET_HATCHING_MONEY.format(localeInfo.NumberToMoneyString(item.PET_HATCHING_MONEY)))

            ## Æê ÀÌ¸§
            self.petNameEdit = self.GetChild("pet_name")
            self.petNameEdit.SetText("")
            self.petNameEdit.SetReturnEvent(self.ClickHatchingButton)
            self.petNameEdit.SetEscapeEvent(self.Close)
            self.petNameEdit.SetOnMouseLeftButtonUpEvent(self.OnMouseLeftButtonUpEvent)
            self.petNameEdit.SetFocus()
            self.petNameEdit.Show()

            ##´ÙÀÌ¾ó ·Î±× »ý¼º
            self.__MakeQuestionDialog()
            self.__MakePopupDialog()


        except:
            
            logging.exception("petHatchingWindow.LoadWindow.BindObject")

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)

    def __MakeQuestionDialog(self):

        if not self.questionDialog:
            self.questionDialog = uiCommon.QuestionDialog()
            self.questionDialog.SetText("")

            self.questionDialog.SetAcceptEvent(self.__HatchingQuestionDialogAccept)
            self.questionDialog.SetCancelEvent(self.__HatchingQuestionDialogCancel)

    def __MakePopupDialog(self):
        self.popupDialog = uiCommon.PopupDialog()
        self.popupDialog.SetText("")

    def ClickHatchingButton(self):
        if self.popupDialog:
            if self.popupDialog.IsShow():
                return

        self.__OpenHatchingQuestionDialog()

    def OnMouseLeftButtonUpEvent(self):
        if self.petName == self.petNameEdit.GetText():
            self.petNameEdit.SetText("")
            self.petNameEdit.SetEndPosition()

    def OverInItem(self, slotIndex):
        return

    def OverOutItem(self):
        return

    def ClearMouseEventEggItem(self):
        if self.eggItemSlotIndex == -1:
            return

        inven_slot_pos = self.eggItemSlotIndex

        if inven_slot_pos >= player.INVENTORY_PAGE_SIZE:
            inven_slot_pos -= player.INVENTORY_PAGE_SIZE

        self.wndPetInformation.inven.wndItem.SetCanMouseEventSlot(inven_slot_pos)

        self.eggItemSlotIndex = -1

    def OnUpdate(self):

        if not self.wndPetInformation.inven:
            return

        if self.eggItemSlotIndex == -1:
            return

        if not self.hatchingSlot:
            return

        try:
            inven = self.wndPetInformation.inven
            invenPage = inven.GetInventoryPageIndex()  ## 0 or 1

            min_range = invenPage * player.INVENTORY_PAGE_SIZE  ## 0 or 45
            max_range = (invenPage + 1) * player.INVENTORY_PAGE_SIZE  ## 45 or 90

            inven_slot_pos = self.eggItemSlotIndex

            if min_range <= inven_slot_pos < max_range:
                inven_slot_pos = inven_slot_pos - min_range
                inven.wndItem.SetCantMouseEventSlot(inven_slot_pos)

        except:
            pass

        return

    def HatchingWindowOpen(self, slotIndex):
        return
        # checkMsg = appInst.instance().GetNet().CheckUsePetItem()
        #
        # if checkMsg == item.PET_EGG_USE_FAILED_BECAUSE_TRADING:
        #     chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_EGG_ITEM_USE)
        #
        # elif checkMsg == item.PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN:
        #     chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_EGG_ITEM_USE)
        #
        # elif checkMsg == item.PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN:
        #     chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_EGG_ITEM_USE)
        #
        # elif checkMsg == item.PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN:
        #     chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_EGG_ITEM_USE)
        #
        # if checkMsg != item.PET_EGG_USE_True:
        #     return
        #
        # if not self.hatchingSlot:
        #     return
        #
        # ItemVNum = player.GetItemIndex(slotIndex)
        #
        # if ItemVNum == 0:
        #     return
        #
        # item.SelectItem(ItemVNum)
        #
        # growthPetVnum = item.GetValue(0)
        #
        # if growthPetVnum == 0:
        #     return
        #
        # self.Close()
        # self.eggItemSlotIndex = slotIndex
        # self.hatchingSlot.SetItemSlot(0, growthPetVnum)
        # self.hatchingSlot.RefreshSlot()
        # item_string = item.GetItemName()
        # item_string = item_string.split()
        # self.petName = reduce(lambda str1, str2: str1 + str2, item_string)
        # self.petName = self.petName[:item.PET_NAME_MAX_SIZE]
        # self.petNameEdit.SetText(self.petName)
        # self.petNameEdit.SetEndPosition()
        # self.petNameEdit.SetFocus()
        # self.petNameEdit.Show()
        # self.Show()
        # appInst.instance().GetNet().SendPetHatchingWindowPacket(True)
        # player.SetOpenPetHatchingWindow(True)

    def __OpenHatchingQuestionDialog(self):
        interface = self.wndPetInformation.interface

        if interface.IsShowDlgQuestionWindow():
            interface.CloseDlgQuestionWindow()

        if not self.questionDialog:
            self.__MakeQuestionDialog()

        # ºó ¹®ÀÚ¸é ¿ø·¡ ÀÌ¸§ ¼ÂÆÃ
        if "" == self.petNameEdit.GetText():
            self.petNameEdit.SetText(self.petName)
            self.petNameEdit.SetEndPosition()

        self.questionDialog.SetText(localeInfo.PET_HATCHING_ACCEPT.format(self.petNameEdit.GetText()))
        self.questionDialog.SetTop()
        self.questionDialog.Open()

    def __HatchingQuestionDialogAccept(self):
        self.questionDialog.Close()

        ## ÀÌ¸§Àº ÃÖ¼Ò 4byte ÀÌ»ó ÀÌ¾î¾ß ÇÑ´Ù
        if len(self.petNameEdit.GetText()) < item.PET_NAME_MIN_SIZE:
            self.petNameEdit.SetText("")
            self.__OpenPopupDialog(localeInfo.PET_NAME_MIN)
            return

        ## ÃæºÐÇÑ µ·À» º¸À¯ÇÏ°í ÀÖ´ÂÁö °Ë»ç
        if player.GetMoney() < item.PET_HATCHING_MONEY:
            self.__OpenPopupDialog(localeInfo.PET_MSG_NOT_ENOUGH_MONEY)
            return

        #result = appInst.instance().GetNet().SendPetHatchingPacket(self.petNameEdit.GetText(), self.eggItemSlotIndex)

    def __HatchingQuestionDialogCancel(self):
        self.questionDialog.Close()

    def __OpenPopupDialog(self, str):
        interface = self.wndPetInformation.interface

        if interface.IsShowDlgQuestionWindow():
            interface.CloseDlgQuestionWindow()

        if not self.popupDialog:
            self.__MakePopupDialog()

        self.popupDialog.SetText(str)
        self.popupDialog.SetTop()
        self.popupDialog.Open()

    def PetHatchingWindowCommand(self, command, pos):

        if command == item.EGG_USE_SUCCESS:
            self.Close()

        elif command == item.EGG_USE_FAILED_BECAUSE_NAME:
            self.petNameEdit.SetText("")
            self.petNameEdit.SetEndPosition()

        elif command == item.EGG_USE_FAILED_TIMEOVER:
            if self.eggItemSlotIndex == pos:
                self.Close()


# Æê ¸ÔÀÌÁÖ±â Ã¢
class PetFeedWindow(ui.ScriptWindow):
    def __init__(self, wndPetInformation):
        

        if not wndPetInformation:
            logging.exception("wndPetInformation parameter must be set to PetInformationWindow")
            return

        ui.ScriptWindow.__init__(self)

        self.isLoaded = 0
        self.feedButtonClickTime = 0
        self.backupFeedItems = []
        self.wndPetInformation = wndPetInformation

        self.__LoadWindow()

    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)
        player.SetOpenPetFeedWindow(True)

        if self.wndPetInformation and self.wndPetInformation.inven:
            self.wndPetInformation.inven.Show()

    def ClearFeedItems(self):
        self.ClearMouseEventFeedItems()

        if self.FeedItemSlot:
            for slotPos in xrange(self.FeedItemSlot.GetSlotCount()):
                self.FeedItemSlot.ClearSlot(slotPos)

            self.FeedItemSlot.RefreshSlot()

    def SetOnTopWindowNone(self):

        if not self.wndPetInformation:
            return

        if not self.wndPetInformation.interface:
            return

        interface = self.wndPetInformation.interface
        interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
        interface.RefreshMarkInventoryBag()

    def Close(self):
        self.SetOnTopWindowNone()
        self.wndPetInformation.PetFeedToggleButtonUpAll()
        self.ClearFeedItems()
        self.Hide()
        player.SetOpenPetFeedWindow(False)

    def Destroy(self):
        del self.FeedItems[:]
        del self.FeedItemsCount[:]
        del self.FeedItemDummy[:]
        self.FeedItems = None
        self.FeedItemsCount = None
        self.FeedItemSlot = None
        self.FeedItemDummy = None

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        try:
            self.__LoadScript("UIScript/petFeedWindow.py")

        except:
            
            logging.exception("petFeedWindow.LoadWindow.LoadObject")

        try:
            self.GetChild("PetFeed_TitleBar").SetCloseEvent(self.Close)
            FeedItemSlot = self.GetChild("FeedItemSlot")

            FeedItemSlot.SetOverInItemEvent(self.OverInItem)
            FeedItemSlot.SetOverOutItemEvent(self.OverOutItem)
            FeedItemSlot.SetUnselectItemSlotEvent(self.UnselectItemSlot)
            FeedItemSlot.SetUseSlotEvent(self.UseItemSlot)
            FeedItemSlot.SetSelectEmptySlotEvent(self.SelectEmptySlot)
            FeedItemSlot.SetSelectItemSlotEvent(self.SelectItemSlot)
            self.FeedItemSlot = FeedItemSlot

            self.FeedItems = []
            self.FeedItemsCount = []
            self.FeedItemDummy = []

            self.ClearMouseEventFeedItems()

            self.feedButton = self.GetChild("FeedButton")
            if self.feedButton:
                self.feedButton.SetEvent(self.ClickPetFeedButton)
        except:
            
            logging.exception("petFeedWindow.LoadWindow.BindObject")

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)

    def OverInItem(self, slotIndex):
        if None != self.wndPetInformation.tooltipItem:
            invenPos = self.FeedItems[slotIndex]
            if invenPos != -1:
                self.wndPetInformation.tooltipItem.SetInventoryItem(invenPos, player.INVENTORY)

        return

    def OverOutItem(self):
        if None != self.wndPetInformation.tooltipItem:
            self.wndPetInformation.tooltipItem.HideToolTip()
        return

    def UseItemSlot(self, slotIndex):
        self.RemoveItemSlot(slotIndex)
        return

    def UnselectItemSlot(self, slotIndex):
        self.RemoveItemSlot(slotIndex)
        return

    def SelectItemSlot(self, slotIndex):

        self.RemoveItemSlot(slotIndex)
        return

    def RemoveItemSlot(self, slotIndex):
        if mouseModule.mouseController.isAttached():
            return

        inven_slot_pos = self.FeedItems[slotIndex]
        if inven_slot_pos != -1:
            if inven_slot_pos >= player.INVENTORY_PAGE_SIZE:
                inven_slot_pos -= player.INVENTORY_PAGE_SIZE

            self.wndPetInformation.inven.wndItem.SetCanMouseEventSlot(inven_slot_pos)

        self.DeleteDataDummySlot(slotIndex, self.FeedItems[slotIndex])
        self.FeedItems[slotIndex] = -1
        self.FeedItemsCount[slotIndex] = 0
        self.FeedItemSlot.ClearSlot(slotIndex)
        self.FeedItemSlot.RefreshSlot()

    ## ÇØ´ç ¾ÆÀÌÅÛÀ» ³ÖÀ» ¼ö ºó slot °Ë»ö
    ## -1 Àº ³ÖÀ»¼ö ÀÖ´Â ½½·ÔÀÌ ¾ø´Ù´Â ¶æ
    def SerachEmptySlot(self, size):

        for value in range(player.PET_FEED_SLOT_MAX):

            if 0 == self.FeedItemDummy[value]:  # ºó½½·ÔÀÌ´Ù

                if 1 == size:
                    return value

                emptySlotIndex = value
                searchSucceed = True

                for i in range(size - 1):
                    emptySlotIndex = emptySlotIndex + FEED_WINDOW_X_SIZE

                    if emptySlotIndex >= player.PET_FEED_SLOT_MAX:
                        searchSucceed = False
                        continue

                    if 1 == self.FeedItemDummy[emptySlotIndex]:
                        searchSucceed = False

                if True == searchSucceed:
                    return value

        return -1

    ## ÀÎº¥Åä¸® ¾ÆÀÌÅÛÀ» ¸¶¿ì½º ¿À¸¥Å¬¸¯À¸·Î »ç¿ëÇßÀ»¶§
    ## ÀÎº¥ -> Feed Ã¢
    def ItemMoveFeedWindow(self, slotIndex):

        checkTime = app.GetGlobalTimeStamp() - self.feedButtonClickTime
        if checkTime < 2:
            if slotIndex in self.backupFeedItems:
                return
        else:
            self.backupFeedItems = []

        mouseModule.mouseController.DeattachObject()

        selectedItemVNum = player.GetItemIndex(slotIndex)
        count = player.GetItemCount(slotIndex)

        mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_INVENTORY, slotIndex, selectedItemVNum,
                                                 count)

        item.SelectItem(selectedItemVNum)
        itemSize = item.GetItemSize()

        emptySlotPos = self.SerachEmptySlot(itemSize[1])

        if -1 != emptySlotPos:
            self.SelectEmptySlot(emptySlotPos)

        mouseModule.mouseController.DeattachObject()

    # ÀÎº¥ -> Feed Ã¢
    def SelectEmptySlot(self, slotIndex):

        checkTime = app.GetGlobalTimeStamp() - self.feedButtonClickTime
        if checkTime < 2:
            if slotIndex in self.backupFeedItems:
                return
        else:
            self.backupFeedItems = []

        if not mouseModule.mouseController.isAttached():
            return False

        attachedSlotType = mouseModule.mouseController.GetAttachedType()
        attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
        attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
        attachedItemVNum = player.GetItemIndex(attachedSlotPos)
        item.SelectItem(attachedItemVNum)

        itemType = item.GetItemType()
        itemSubType = item.GetItemSubType()

        ## ÀÎº¥Åä¸®¿¡ ÀÖ´Â ¾ÆÀÌÅÛ¸¸ ¿Ã¼ö ÀÖ´Ù.
        if player.SLOT_TYPE_INVENTORY != attachedSlotType:
            return False

        ## ÀÎº¥Ã¢ ¾È¿¡ ÀÖ´Â °Í¸¸.
        if attachedSlotPos >= player.ITEM_SLOT_COUNT:
            return False

        ## ÆêÀÌ È°¼ºÈ­ »óÅÂ¿©¾ß ÇÑ´Ù.
        petVNum = player.GetActivePetItemVNum()
        if 0 == petVNum:
            return False

        ## È°¼ºÁßÀÎ ÆêÀº ³ÖÀ»¼ö ¾ø´Ù.
        if item.ITEM_TYPE_PET == itemType and itemSubType == item.PET_UPBRINGING:
            activePetId = player.GetActivePetItemId()
            petId = player.GetItemMetinSocket(attachedSlotPos, 2)
            if petId == activePetId:
                return False

        if self.wndPetInformation.CantFeedItem(attachedSlotPos):
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_FEED_TYPE)
            return False

        if -1 != self.FeedItems[slotIndex]:
            return False

        if attachedSlotPos not in self.FeedItems:
            mouseModule.mouseController.DeattachObject()

            invenItemCount = player.GetItemCount(attachedSlotPos)
            if attachedItemCount != invenItemCount:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_FEED_SPLIT_ITEM)
                return False

            self.FeedItems[slotIndex] = attachedSlotPos
            self.FeedItemsCount[slotIndex] = attachedItemCount
            self.InsertDataDummySlot(slotIndex, attachedItemVNum)
            self.FeedItemSlot.SetItemSlot(slotIndex, attachedItemVNum, attachedItemCount)
            self.FeedItemSlot.RefreshSlot()

        return True

    def InsertDataDummySlot(self, slotIndex, vnum):

        self.FeedItemDummy[slotIndex] = 1

        item.SelectItem(vnum)
        itemSize = item.GetItemSize(vnum)  # ³ÖÀ¸·Á´Â ¾ÆÀÌÅÛ size

        if 1 == itemSize[1]:
            return

        addSlotIndex = slotIndex

        for value in range(itemSize[1] - 1):
            addSlotIndex = addSlotIndex + FEED_WINDOW_X_SIZE

            if addSlotIndex >= player.PET_FEED_SLOT_MAX:
                return

            self.FeedItemDummy[addSlotIndex] = 1

    def DeleteDataDummySlot(self, slotIndex, InvenPos):

        vnum = player.GetItemIndex(InvenPos)
        item.SelectItem(vnum)
        itemSize = item.GetItemSize(vnum)  # »©·Á´Â ¾ÆÀÌÅÛ size

        self.FeedItemDummy[slotIndex] = 0

        if 1 == itemSize[1]:
            return

        delSlotIndex = slotIndex

        for value in range(itemSize[1] - 1):
            delSlotIndex = delSlotIndex + FEED_WINDOW_X_SIZE

            if delSlotIndex >= player.PET_FEED_SLOT_MAX:
                return

            self.FeedItemDummy[delSlotIndex] = 0

    ## ¸ÔÀÌÁÖ±â ¹öÆ° Å¬¸¯ Event
    def ClickPetFeedButton(self):
        # resultFeedItems = [value for value in self.FeedItems if value != -1]
        # resultFeedItemCounts = [value for value in self.FeedItemsCount if value != 0]
        # if resultFeedItems:
        #     if appInst.instance().GetNet().SendPetFeedPacket(self.wndPetInformation.feedIndex, resultFeedItems, resultFeedItemCounts):
        #         self.feedButtonClickTime = app.GetGlobalTimeStamp()
        return



    def ClearMouseEventFeedItems(self):

        for inven_slot_pos in self.FeedItems:
            if inven_slot_pos != -1:
                if inven_slot_pos >= player.INVENTORY_PAGE_SIZE:
                    inven_slot_pos -= player.INVENTORY_PAGE_SIZE

                self.wndPetInformation.inven.wndItem.SetCanMouseEventSlot(inven_slot_pos)

        del self.FeedItems[:]
        del self.FeedItemsCount[:]
        del self.FeedItemDummy[:]

        for value in range(0, player.PET_FEED_SLOT_MAX):
            self.FeedItems.append(-1)
            self.FeedItemsCount.append(0)
            self.FeedItemDummy.append(0)

    def BackUpSucceedFeedItems(self):
        self.backupFeedItems = self.FeedItems[:]

    def OnUpdate(self):
        if self.wndPetInformation.inven == 0:
            return

        inven = self.wndPetInformation.inven
        invenPage = inven.GetInventoryPageIndex()  ## 0 or 1

        min_range = invenPage * player.INVENTORY_PAGE_SIZE  ## 0 or 45
        max_range = (invenPage + 1) * player.INVENTORY_PAGE_SIZE  ## 45 or 90

        for inven_slot_pos in self.FeedItems:
            if inven_slot_pos == -1:
                continue

            if min_range <= inven_slot_pos < max_range:
                inven_slot_pos = inven_slot_pos - min_range
                inven.wndItem.SetCantMouseEventSlot(inven_slot_pos)

        return

    def OnSetFocus(self):
        if not self.wndPetInformation:
            return

        if not self.wndPetInformation.interface:
            return

        interface = self.wndPetInformation.interface
        interface.SetOnTopWindow(player.ON_TOP_WND_PET_FEED)
        interface.RefreshMarkInventoryBag()


# Æê Á¤º¸ Ã¢
class PetInformationWindow(ui.ScriptWindow):
    wndPetFeed = None
    tooltipItem = None
    inven = None
    wndPetHatching = None
    wndPetMiniInfo = None
    feedIndex = player.FEED_BUTTON_MAX
    skillSlot = []
    feedButton = []

    SkillBookSlotIndex = -1
    SkillBookInvenIndex = -1

    SkillBookDelSlotIndex = -1
    SkillBookDelInvenIndex = -1

    evolInfo = {}

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.isLoaded = 0
        self.SetWindowName("PetInformationWindow")
        self.__LoadWindow()
        self.wndPetHatching = PetHatchingWindow(self)
        self.wndPetMiniInfo = PetMiniInfomationWindow(self)
        self.wndPetFeed = PetFeedWindow(self)
        self.AffectShower = None
        self.popupDialog = None
        self.skillUpgradeGold = 0
        self.skillUpgradeSlot = -1
        self.skillUpgradeIndex = -1
        self.tooptipPetSkill = None


    def Show(self):
        self.__LoadWindow()
        ui.ScriptWindow.Show(self)
        self.SetTop()

    def Hide(self):
        if self.wndPetFeed:
            self.wndPetFeed.Close()

        wndMgr.Hide(self.hWnd)

    def BindInterfaceClass(self, interface):
        from _weakref import proxy
        self.interface = proxy(interface)

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return
        self.isLoaded = 1

        try:
            self.__LoadScript("UIScript/petInformationWindow.py")

        except:
            
            logging.exception("petInformationWindow.LoadWindow.__LoadScript")

        try:
            ###        BG
            if localeInfo.IsARABIC():
                self.GetChild("PetUIBG").LeftRightReverse()

            ###        Close Button Event
            self.GetChild("CloseButton").SetEvent(self.Close)

            ###        UpBringing Pet Slot
            wndUpBringingPetSlot = self.GetChild("UpBringing_Pet_Slot")
            wndUpBringingPetSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)

            if localeInfo.IsARABIC():
                wndUpBringingPetSlot.SetPosition(295, 55)

            self.wndUpBringingPetSlot = wndUpBringingPetSlot

            ##        Feed LifeTime Button
            feedLifeTimeButton = self.GetChild("FeedLifeTimeButton")
            if feedLifeTimeButton:
                feedLifeTimeButton.SetToggleDownEvent(self.ClickFeedLifeTimeButtonDown)
                feedLifeTimeButton.SetToggleUpEvent(self.ClickFeedLifeTimeButtonUp)
            self.feedButton.append(feedLifeTimeButton)

            ##        Feed Evolution Button
            feedEvolButton = self.GetChild("FeedEvolButton")
            if feedEvolButton:
                feedEvolButton.SetToggleDownEvent(self.ClickFeedEvolButtonDown)
                feedEvolButton.SetToggleUpEvent(self.ClickFeedEvolButtonUp)
            self.feedButton.append(feedEvolButton)

            ##        Feed EXP Button
            feedExpButton = self.GetChild("FeedExpButton")
            if feedExpButton:
                feedExpButton.SetToggleDownEvent(self.ClickFeedExpButtonDown)
                feedExpButton.SetToggleUpEvent(self.ClickFeedExpButtonUp)
            self.feedButton.append(feedExpButton)

            for value in range(player.FEED_BUTTON_MAX):
                self.feedButton[value].DisableFlash()

            ##        Life Time Gauge
            self.lifeTimeGauge = self.GetChild("LifeGauge")
            self.lifeTimeGauge.SetScale(1.61, 1.0)
            self.lifeTimeGauge.SetWindowHorizontalAlignLeft()

            if localeInfo.IsARABIC():
                self.lifeTimeGauge.SetPosition(26, 0)

            ##        EXP GAUGE
            expGauge = []
            self.expGaugeBoard = self.GetChild("UpBringing_Pet_EXP_Gauge_Board")
            expGauge.append(self.GetChild("UpBringing_Pet_EXPGauge_01"))
            expGauge.append(self.GetChild("UpBringing_Pet_EXPGauge_02"))
            expGauge.append(self.GetChild("UpBringing_Pet_EXPGauge_03"))
            expGauge.append(self.GetChild("UpBringing_Pet_EXPGauge_04"))
            expGauge.append(self.GetChild("UpBringing_Pet_EXPGauge_05"))

            for exp in expGauge:
                exp.SetSize(0, 0)

            self.expGauge = expGauge
            self.tooltipEXP = ui.TextToolTip()
            self.tooltipEXP.Hide()

            ## skill slot
            arabic_start_pos_x = 36

            for value in range(SKILL_SLOT_MAX):
                self.skillSlot.append(self.GetChild("PetSkillSlot" + str(value)))
                self.skillSlot[value].SetCoverButton(0, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE,
                                                     SKILL_SLOT_ENABLE, False, False)
                self.skillSlot[value].SetAlwaysRenderCoverButton(0)
                self.skillSlot[value].AppendSlotButton("d:/ymir work/ui/game/windows/btn_plus_up.sub", \
                                                       "d:/ymir work/ui/game/windows/btn_plus_over.sub", \
                                                       "d:/ymir work/ui/game/windows/btn_plus_down.sub")
                if localeInfo.IsARABIC():
                    ## 36, 100, 164
                    self.skillSlot[value].SetPosition(arabic_start_pos_x, 365)
                    arabic_start_pos_x = arabic_start_pos_x + 64

            ## skill slot empty event
            self.skillSlot[0].SetSelectEmptySlotEvent(self.SelectEmptySkillSlot1)
            self.skillSlot[1].SetSelectEmptySlotEvent(self.SelectEmptySkillSlot2)
            self.skillSlot[2].SetSelectEmptySlotEvent(self.SelectEmptySkillSlot3)

            ## skill slot select Item event
            self.skillSlot[0].SetSelectItemSlotEvent(self.SetSelectItemSlotEvent1)
            self.skillSlot[1].SetSelectItemSlotEvent(self.SetSelectItemSlotEvent2)
            self.skillSlot[2].SetSelectItemSlotEvent(self.SetSelectItemSlotEvent3)

            ## skill slot  over in event
            self.skillSlot[0].SetOverInItemEvent(self.OverInSkillSlot1)
            self.skillSlot[1].SetOverInItemEvent(self.OverInSkillSlot2)
            self.skillSlot[2].SetOverInItemEvent(self.OverInSkillSlot3)
            ## skill slot  over out event
            self.skillSlot[0].SetOverOutItemEvent(self.OverOutSkillSlot1)
            self.skillSlot[1].SetOverOutItemEvent(self.OverOutSkillSlot2)
            self.skillSlot[2].SetOverOutItemEvent(self.OverOutSkillSlot3)

            self.skillSlot[0].SetPressedSlotButtonEvent(self.OnPressedSkill1SlotButton)
            self.skillSlot[1].SetPressedSlotButtonEvent(self.OnPressedSkill2SlotButton)
            self.skillSlot[2].SetPressedSlotButtonEvent(self.OnPressedSkill3SlotButton)

            ## ½ºÅ³ Áö¿ì´Â Áú¹®Ã¢
            self.questionSkillDelDlg = uiCommon.QuestionDialog()
            self.questionSkillDelDlg.SetAcceptEvent(self.__SkillDeleteQuestionDialogAccept)
            self.questionSkillDelDlg.SetCancelEvent(self.__SkillDeleteQuestionDialogCancel)
            self.questionSkillDelDlg.Close()

            ## ½ºÅ³ ¹è¿ì´Â Áú¹®Ã¢
            self.questionDialog1 = uiCommon.QuestionDialog()
            self.questionDialog1.SetAcceptEvent(self.__SkillLearnQuestionDialogAccept)
            self.questionDialog1.SetCancelEvent(self.__SkillLearnQuestionDialogCancel)
            self.questionDialog1.Close()

            ## ½ºÅ³ ¾÷±×·¹ÀÌµå Áú¹®Ã¢
            self.questionDialog2 = uiCommon.QuestionDialog2()
            self.questionDialog2.SetText1(localeInfo.PET_SKILL_UPGRADE_QUESTION_DLG_MSG1)
            self.questionDialog2.SetText2("")
            self.questionDialog2.SetAcceptEvent(self.__SkillUpgradeQuestionDialogAccept)
            self.questionDialog2.SetCancelEvent(self.__SkillUpgradeQuestionDialogCancel)
            self.questionDialog2.Close()

            ## ÁøÈ­ Á¤º¸
            for evolInfoIndex in range(player.PET_GROWTH_EVOL_MAX):
                self.evolInfo[evolInfoIndex] = 0

        except:
            
            logging.exception("petInformationWindow.LoadWindow.BindObject")

    def Destroy(self):
        self.isLoaded = 0

        if self.wndPetFeed:
            self.wndPetFeed.Destroy()
            self.wndPetFeed = None

        self.interface = None
        self.inven = None
        self.tooltipItem = None

        self.ClearDictionary()
        self.wndUpBringingPetSlot = None

        self.lifeTimeGauge = None
        self.expGauge = None
        self.expGaugeBoard = None
        self.tooltipEXP = None

        if self.wndPetHatching:
            self.wndPetHatching.Destroy()
            self.wndPetHatching = None

        if self.wndPetMiniInfo:
            self.wndPetMiniInfo.Destroy()
            self.wndPetMiniInfo = None

        if self.skillSlot:
            del self.skillSlot[:]

        if self.feedButton:
            del self.feedButton[:]

        self.feedIndex = player.FEED_BUTTON_MAX
        self.SkillBookSlotIndex = -1
        self.SkillBookInvenIndex = -1

        SkillBookDelSlotIndex = -1
        SkillBookDelInvenIndex = -1

        self.skillUpgradeGold = 0
        self.skillUpgradeSlot = -1
        self.skillUpgradeIndex = -1

        self.AffectShower = None
        self.tooptipPetSkill = None

        if self.questionDialog1:
            self.questionDialog1.Destroy()

        if self.questionDialog2:
            self.questionDialog2.Destroy()

        if self.questionSkillDelDlg:
            self.questionSkillDelDlg.Destroy()

        if self.popupDialog:
            self.popupDialog.Destroy()

        self.questionDialog1 = None
        self.questionDialog2 = None
        self.questionSkillDelDlg = None
        self.popupDialog = None

        self.evolInfo = {}

    def Close(self):
        if self.tooltipEXP:
            self.tooltipEXP.Hide()

        if self.wndPetFeed:
            self.wndPetFeed.Close()

        self.PetFeedToggleButtonUpAll()

        self.__ClearPetSkillSlot()

        self.__ClearSkillBookLearnEvent()
        self.__ClearSkillUpgradeEvnet()

        if self.popupDialog:
            self.popupDialog.Close()

        self.Hide()

    def __ClearPetSkillSlot(self):
        for value in range(SKILL_SLOT_MAX):
            self.skillSlot[value].ClearSlot(0)
            self.skillSlot[value].SetCoverButton(0, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE,
                                                 SKILL_SLOT_ENABLE, False, False)
            self.skillSlot[value].SetAlwaysRenderCoverButton(0)

    ## À°¼ºÆê ½ºÅ³ slot ÀÇ + ¹öÆ° Å¬¸¯½Ã
    def OnPressedSkill1SlotButton(self, slotIndex):
        self.OnPressedSkillSlotButton(0, slotIndex)

    def OnPressedSkill2SlotButton(self, slotIndex):
        self.OnPressedSkillSlotButton(1, slotIndex)

    def OnPressedSkill3SlotButton(self, slotIndex):
        self.OnPressedSkillSlotButton(2, slotIndex)

    ##slotPos : 0 ~ 2
    def OnPressedSkillSlotButton(self, slotPos, slotIndex):
        #appInst.instance().GetNet().SendPetSkillUpgradeRequest(slotPos, slotIndex)
        pass

    def __MakePopupDialog(self):
        self.popupDialog = uiCommon.PopupDialog()
        self.popupDialog.SetText("")

    def __OpenPopupDialog(self, str):

        if self.interface.IsShowDlgQuestionWindow():
            self.interface.CloseDlgQuestionWindow()

        if not self.popupDialog:
            self.__MakePopupDialog()

        self.popupDialog.SetText(str)
        self.popupDialog.SetTop()
        self.popupDialog.Open()

    def OpenPetSkillUpGradeQuestionDialog(self, slot, index, gold):

        if not self.questionDialog2:
            self.questionDialog2 = uiCommon.QuestionDialog2()
            self.questionDialog2.SetText1(localeInfo.PET_SKILL_UPGRADE_QUESTION_DLG_MSG1)
            self.questionDialog2.SetAcceptEvent(self.__SkillUpgradeQuestionDialogAccept)
            self.questionDialog2.SetCancelEvent(self.__SkillUpgradeQuestionDialogCancel)

        self.skillUpgradeGold = gold
        self.skillUpgradeSlot = slot
        self.skillUpgradeIndex = index

        self.questionDialog2.SetText2(
            localeInfo.PET_SKILL_UPGRADE_QUESTION_DLG_MSG2.format(localeInfo.NumberToMoneyString(self.skillUpgradeGold)))
        self.questionDialog2.SetTop()
        self.questionDialog2.Open()

    def __SkillUpgradeQuestionDialogAccept(self):

        slot = self.skillUpgradeSlot
        gold = self.skillUpgradeGold
        index = self.skillUpgradeIndex

        self.__ClearSkillUpgradeEvnet()

        ## ÃæºÐÇÑ µ·À» º¸À¯ÇÏ°í ÀÖ´ÂÁö °Ë»ç
        if player.GetMoney() < gold:
            self.__OpenPopupDialog(localeInfo.PET_MSG_NOT_ENOUGH_MONEY)
            return

        ## ¼­¹ö·Î ¾÷±×·¹ÀÌµå ÇÑ´Ù°í º¸³¿
        #appInst.instance().GetNet().SendPetSkillUpgrade(slot, index)

    def __SkillUpgradeQuestionDialogCancel(self):
        self.__ClearSkillUpgradeEvnet()

    def __ClearSkillUpgradeEvnet(self):
        if self.questionDialog2:
            self.questionDialog2.Close()

        self.skillUpgradeGold = 0
        self.skillUpgradeSlot = -1
        self.skillUpgradeIndex = -1

    ## skill slot select item event
    def SetSelectItemSlotEvent1(self, slotIndex):
        self.SetSelectItemSlotEvent(0)

    def SetSelectItemSlotEvent2(self, slotIndex):
        self.SetSelectItemSlotEvent(1)

    def SetSelectItemSlotEvent3(self, slotIndex):
        self.SelectEmptySkillSlot(2)

    def SetSelectItemSlotEvent(self, skillSlotIndex):

        ## ÆêÀÌ È°¼ºÈ­ »óÅÂ¿©¾ß ÇÑ´Ù.
        pet_id = player.GetActivePetItemId()
        if 0 == pet_id:
            return

        if not mouseModule.mouseController.isAttached():
            return

        if skillSlotIndex < 0 or skillSlotIndex >= SKILL_SLOT_MAX:
            return

        ## Skill
        (skill_count, pet_skill1, pet_skill_level1, pet_skill_cool1, pet_skill2, pet_skill_level2, pet_skill_cool2,
         pet_skill3, pet_skill_level3, pet_skill_cool3) = player.GetPetSkill(pet_id)

        if 0 == skill_count:
            return

        if skillSlotIndex >= skill_count:
            return

        if 0 == skillSlotIndex:
            if not pet_skill1:
                return
        elif 1 == skillSlotIndex:
            if not pet_skill2:
                return
        elif 2 == skillSlotIndex:
            if not pet_skill3:
                return

        attachedSlotType = mouseModule.mouseController.GetAttachedType()
        attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
        attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
        attachedItemVNum = player.GetItemIndex(attachedSlotPos)
        item.SelectItem(attachedItemVNum)

        itemType = item.GetItemType()
        itemSubType = item.GetItemSubType()

        ## ÀÎº¥Åä¸®¿¡ ÀÖ´Â ¾ÆÀÌÅÛ¸¸ ¿Ã¼ö ÀÖ´Ù.
        if player.SLOT_TYPE_INVENTORY != attachedSlotType:
            return

        ## ÀÎº¥Ã¢ ¾È¿¡ ÀÖ´Â °Í¸¸.
        if attachedSlotPos >= player.ITEM_SLOT_COUNT:
            return

        ## Æê ½ºÅ³ºÏÀÌ¾î¾ß ÇÑ´Ù.
        if item.ITEM_TYPE_PET != itemType:
            return

        if item.PET_SKILL_DEL_BOOK != itemSubType:
            return

        ## ¿©±â ±îÁö Åë°ú ÇßÀ¸¸é ´ÙÀÌ¾ó·Î±× Ã¢ ¶ç¿ìÀÚ
        if not self.questionSkillDelDlg:
            self.questionSkillDelDlg = uiCommon.QuestionDialog()
            self.questionSkillDelDlg.SetAcceptEvent(self.__SkillDeleteQuestionDialogAccept)
            self.questionSkillDelDlg.SetCancelEvent(self.__SkillDeleteQuestionDialogCancel)

        self.questionSkillDelDlg.SetText(localeInfo.PET_SKILL_DELETE_QUESTION_DLG_MSG)
        mouseModule.mouseController.DeattachObject()
        self.SkillBookDelSlotIndex = skillSlotIndex
        self.SkillBookDelInvenIndex = attachedSlotPos
        self.questionSkillDelDlg.SetTop()
        self.questionSkillDelDlg.Open()

    def __SkillDeleteQuestionDialogAccept(self):
        pet_id = player.GetActivePetItemId()
        #if pet_id:
        #    #appInst.instance().GetNet().SendPetDeleteSkill(self.SkillBookDelSlotIndex, self.SkillBookDelInvenIndex)

        self.__ClearSkillDeleteBookEvent()
        return

    def __SkillDeleteQuestionDialogCancel(self):
        self.__ClearSkillDeleteBookEvent()
        return

    def __ClearSkillDeleteBookEvent(self):

        self.CanInvenSlot(self.SkillBookDelInvenIndex)

        self.SkillBookDelSlotIndex = -1
        self.SkillBookDelInvenIndex = -1

        if self.questionSkillDelDlg:
            self.questionSkillDelDlg.Close()

    ## skill slot over in event
    def OverInSkillSlot1(self, slotIndex):
        self.OverInPetSkillSlot(0, slotIndex)

    def OverInSkillSlot2(self, slotIndex):
        self.OverInPetSkillSlot(1, slotIndex)

    def OverInSkillSlot3(self, slotIndex):
        self.OverInPetSkillSlot(2, slotIndex)

    def OverInPetSkillSlot(self, slot, index):
        pet_id = player.GetActivePetItemId()
        if 0 == pet_id:
            return

        if self.tooptipPetSkill:
            self.tooptipPetSkill.SetPetSkill(pet_id, slot, index)

    ## skill slot over out event
    def OverOutSkillSlot1(self):
        self.tooptipPetSkill.HideToolTip()

    def OverOutSkillSlot2(self):
        self.tooptipPetSkill.HideToolTip()

    def OverOutSkillSlot3(self):
        self.tooptipPetSkill.HideToolTip()

    ## skill slot empty event
    def SelectEmptySkillSlot1(self, slotIndex):
        self.SelectEmptySkillSlot(0)

    def SelectEmptySkillSlot2(self, slotIndex):
        self.SelectEmptySkillSlot(1)

    def SelectEmptySkillSlot3(self, slotIndex):
        self.SelectEmptySkillSlot(2)

    def SelectEmptySkillSlot(self, skillSlotIndex):

        ## ÆêÀÌ È°¼ºÈ­ »óÅÂ¿©¾ß ÇÑ´Ù.
        pet_id = player.GetActivePetItemId()
        if 0 == pet_id:
            return

        if not mouseModule.mouseController.isAttached():
            return

        if skillSlotIndex < 0 or skillSlotIndex >= SKILL_SLOT_MAX:
            return

        ## Skill
        (skill_count, pet_skill1, pet_skill_level1, pet_skill_cool1, pet_skill2, pet_skill_level2, pet_skill_cool2,
         pet_skill3, pet_skill_level3, pet_skill_cool3) = player.GetPetSkill(pet_id)

        if 0 == skill_count:
            return

        if skillSlotIndex >= skill_count:
            return

        if 0 == skillSlotIndex:
            if pet_skill1:
                return
        elif 1 == skillSlotIndex:
            if pet_skill2:
                return
        elif 2 == skillSlotIndex:
            if pet_skill3:
                return

        attachedSlotType = mouseModule.mouseController.GetAttachedType()
        attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
        attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
        attachedItemVNum = player.GetItemIndex(attachedSlotPos)
        item.SelectItem(attachedItemVNum)

        itemType = item.GetItemType()
        itemSubType = item.GetItemSubType()

        ## ÀÎº¥Åä¸®¿¡ ÀÖ´Â ¾ÆÀÌÅÛ¸¸ ¿Ã¼ö ÀÖ´Ù.
        if player.SLOT_TYPE_INVENTORY != attachedSlotType:
            return

        ## ÀÎº¥Ã¢ ¾È¿¡ ÀÖ´Â °Í¸¸.
        if attachedSlotPos >= player.ITEM_SLOT_COUNT:
            return

        ## Æê ½ºÅ³ºÏÀÌ¾î¾ß ÇÑ´Ù.
        if item.ITEM_TYPE_PET != itemType:
            return

        if item.PET_SKILL != itemSubType:
            return

        if not self.questionDialog1:
            self.questionDialog1 = uiCommon.QuestionDialog()
            self.questionDialog1.SetAcceptEvent(self.__SkillLearnQuestionDialogAccept)
            self.questionDialog1.SetCancelEvent(self.__SkillLearnQuestionDialogCancel)

        self.questionDialog1.SetText(localeInfo.PET_SKILL_LEARN_QUESTION_DLG_MSG.format(item.GetItemName()))
        mouseModule.mouseController.DeattachObject()
        self.SkillBookSlotIndex = skillSlotIndex
        self.SkillBookInvenIndex = attachedSlotPos
        self.questionDialog1.SetTop()
        self.questionDialog1.Open()

    def __SkillLearnQuestionDialogAccept(self):
        pet_id = player.GetActivePetItemId()
        #if pet_id:
        #    #appInst.instance().GetNet().SendPetLearnSkill(self.SkillBookSlotIndex, self.SkillBookInvenIndex)

        self.__ClearSkillBookLearnEvent()
        return

    def __SkillLearnQuestionDialogCancel(self):
        self.__ClearSkillBookLearnEvent()
        return

    def __ClearSkillBookLearnEvent(self):

        self.CanInvenSlot(self.SkillBookInvenIndex)

        self.SkillBookSlotIndex = -1
        self.SkillBookInvenIndex = -1

        if self.questionDialog1:
            self.questionDialog1.Close()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def __LoadScript(self, fileName):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, fileName)

    def PetFeedToggleButtonUpAll(self, exclusion_index=player.FEED_BUTTON_MAX):
        for value in range(player.FEED_BUTTON_MAX):
            if exclusion_index == value:
                continue
            self.feedButton[value].SetUp()

    def ClickFeedLifeTimeButtonDown(self):
        self.ClickPetFeedButton(player.FEED_LIFE_TIME_EVENT)

    def ClickFeedLifeTimeButtonUp(self):
        if self.feedIndex == player.FEED_LIFE_TIME_EVENT:
            self.feedIndex = player.FEED_BUTTON_MAX
            self.wndPetFeed.Close()

    def ClickFeedEvolButtonDown(self):
        self.ClickPetFeedButton(player.FEED_EVOL_EVENT)

    def ClickFeedEvolButtonUp(self):
        if self.feedIndex == player.FEED_EVOL_EVENT:
            self.feedIndex = player.FEED_BUTTON_MAX
            self.wndPetFeed.Close()

    def ClickFeedExpButtonDown(self):
        self.ClickPetFeedButton(player.FEED_EXP_EVENT)

    def ClickFeedExpButtonUp(self):

        if self.feedIndex == player.FEED_EXP_EVENT:
            self.feedIndex = player.FEED_BUTTON_MAX
            self.wndPetFeed.Close()

    def IsActivateEvolButton(self, pet_id):

        if 0 == pet_id:
            return False

        (pet_level, evol_level, birthday, pet_nick, pet_hp, pet_def, pet_sp, evol_name) = player.GetPetItem(pet_id)

        if evol_level == player.PET_GROWTH_SKILL_OPEN_EVOL_LEVEL:
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_OPEN_EVOL_BUTTON_LEVEL_MAX)
            return False

        evol_require = self.GetEvolInfo(evol_level - 1)
        if 0 == evol_require:
            return False

        ## ÇöÀç ÁøÈ­´Ü°è°¡ 1,2´Ü°èÀÏ¶§
        if evol_level < player.PET_GROWTH_SKILL_OPEN_EVOL_LEVEL - 1:
            ## ·¹º§ÀÌ ¸¸Á·ÇÏ´ÂÁö
            if pet_level < evol_require:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_OPEN_EVOL_BUTTON.format(evol_require))
                return False
            else:
                ## °æÇèÄ¡¸¦ °¡µæ Ã¼¿ü´ÂÁö
                (curEXP, nextEXP, itemEXP, itemMaxEXP) = player.GetPetExpPoints(pet_id)

                if curEXP != nextEXP or itemEXP != itemMaxEXP:
                    chat.AppendChat(CHAT_TYPE_INFO, localeInfo.PET_CAN_NOT_OPEN_EVOL_BUTTON_EXP_LACK)
                    return False

        ## ÇöÀç ÁøÈ­´Ü°è°¡ 3´Ü°èÀÏ¶§´Â ³ªÀÌÁ¶°Ç °Ë»ç
        elif evol_level == player.PET_GROWTH_SKILL_OPEN_EVOL_LEVEL - 1:
            curTime = app.GetGlobalTimeStamp()
            birthSec = max(0, curTime - birthday)
            day = localeInfo.SecondToDayNumber(birthSec)

            if day < evol_require:
                chat.AppendChat(CHAT_TYPE_INFO,
                                   localeInfo.PET_CAN_NOT_OPEN_SPECIAL_EVOL_BUTTON.format(evol_require))
                return False

        return True

    def ClickPetFeedButton(self, index):

        pet_id = player.GetActivePetItemId()
        if 0 == pet_id:
            self.PetFeedToggleButtonUpAll()
            return

        if player.FEED_EVOL_EVENT == index:
            if False == self.IsActivateEvolButton(pet_id):
                self.PetFeedToggleButtonUpAll(self.feedIndex)
                return

        if not self.wndPetFeed:
            self.wndPetFeed = PetFeedWindow(self)

        self.feedIndex = index
        self.wndPetFeed.ClearFeedItems()
        self.wndPetFeed.Show()
        self.wndPetFeed.SetTop()

        self.PetFeedToggleButtonUpAll(self.feedIndex)

    def OnUpdate(self):
        self.RefreshStatus()

        self.CantInvenSlot(self.SkillBookInvenIndex)
        self.CantInvenSlot(self.SkillBookDelInvenIndex)

        if self.expGaugeBoard.IsIn():
            self.tooltipEXP.Show()
        else:
            self.tooltipEXP.Hide()

    def CantInvenSlot(self, invenIndex):

        if invenIndex == -1:
            return

        inven = self.inven
        invenPage = inven.GetInventoryPageIndex()  ## 0 or 1

        min_range = invenPage * player.INVENTORY_PAGE_SIZE  ## 0 or 45
        max_range = (invenPage + 1) * player.INVENTORY_PAGE_SIZE  ## 45 or 90

        inven_slot_pos = invenIndex

        if min_range <= inven_slot_pos < max_range:
            inven_slot_pos = inven_slot_pos - min_range
            inven.wndItem.SetCantMouseEventSlot(inven_slot_pos)

    def CanInvenSlot(self, invenIndex):

        if invenIndex == -1:
            return

        inven = self.inven
        invenPage = inven.GetInventoryPageIndex()  ## 0 or 1

        min_range = invenPage * player.INVENTORY_PAGE_SIZE  ## 0 or 45
        max_range = (invenPage + 1) * player.INVENTORY_PAGE_SIZE  ## 45 or 90

        inven_slot_pos = invenIndex

        if min_range <= inven_slot_pos < max_range:
            inven_slot_pos = inven_slot_pos - min_range
            inven.wndItem.SetCanMouseEventSlot(inven_slot_pos)

    def RefreshStatus(self):

        if self.isLoaded == 0:
            return

        try:
            pet_id = player.GetActivePetItemId()
            if 0 == pet_id:
                self.ClearStatus()
                if self.wndPetMiniInfo:
                    self.wndPetMiniInfo.Close()
                return

            if not self.wndPetMiniInfo:
                return

            (pet_level, evol_level, birthday, pet_nick, pet_hp, pet_def, pet_sp, evol_name) = player.GetPetItem(
                pet_id)
            curTime = app.GetGlobalTimeStamp()

            ##    UpBringing Pet Slot Image
            CurPetItemVNum = player.GetActivePetItemVNum()
            self.wndUpBringingPetSlot.SetItemSlot(0, CurPetItemVNum)
            self.wndPetMiniInfo.SetItemSlot(CurPetItemVNum)

            ##    Pet Name
            # item.SelectItem(CurPetItemVNum)
            # self.GetChild("PetName").SetText( item.GetItemName() )
            self.GetChild("PetName").SetText(pet_nick);

            ## Evol Name
            self.GetChild("EvolName").SetText(self.__GetEvolName(evol_level))

            ##    LEVEL
            self.GetChild("LevelValue").SetText(str(pet_level))

            ##    AGE
            birthSec = max(0, curTime - birthday)
            self.GetChild("AgeValue").SetText(localeInfo.SecondToDay(birthSec))

            ## Life Time Text
            (endTime, maxTime) = player.GetPetLifeTime(pet_id)
            lifeTime = max(0, endTime - curTime)
            self.GetChild("LifeTextValue").SetText(localeInfo.SecondToH(lifeTime) + " / " + localeInfo.SecondToH(
                maxTime) + " " + uiScriptLocale.PET_INFORMATION_LIFE_TIME)

            ## Life Time Gauge
            self.SetLifeTime(lifeTime, maxTime)
            self.wndPetMiniInfo.SetLifeTime(lifeTime, maxTime)

            ## HP, Def, SP Bonus Text
            self.GetChild("HpValue").SetText("+" + pet_hp + "%")
            self.GetChild("DefValue").SetText("+" + pet_def + "%")
            self.GetChild("SpValue").SetText("+" + pet_sp + "%")

            ## EXP
            (curEXP, nextEXP, itemEXP, itemMaxEXP) = player.GetPetExpPoints(pet_id)
            curEXP = (curEXP)
            nextEXP = (nextEXP)
            itemEXP = (itemEXP)
            itemMaxEXP = (itemMaxEXP)
            self.SetExperience(curEXP, nextEXP, itemEXP, itemMaxEXP)
            self.wndPetMiniInfo.SetExperience(curEXP, nextEXP, itemEXP, itemMaxEXP)

            # ½ºÅ³ ½½·Ô Clear
            self.__ClearPetSkillSlot()

            # mini Á¤º¸Ã¢ ½ºÅ³ ½½·Ô Clear
            self.wndPetMiniInfo.ClearSkillSlot()

            ## Æ¯¼ö ÁøÈ­ Flash Event Check
            self.PetSpecialEvolFlashEventCheck(evol_level, birthSec)

            ## ÇÇ·Îµµ Flash Event Check
            self.PetLifeTimeFlashEventCheck(lifeTime)

            ## Æ¯¼ö ÁøÈ­ ÀüÀÌ¶ó¸é return
            if evol_level < player.PET_GROWTH_SKILL_OPEN_EVOL_LEVEL:
                self.wndPetMiniInfo.Show()
                return

            ## Skill
            (skill_count, pet_skill1, pet_skill_level1, pet_skill_cool1, pet_skill2, pet_skill_level2, pet_skill_cool2,
             pet_skill3, pet_skill_level3, pet_skill_cool3) = player.GetPetSkill(pet_id)

            if skill_count:
                for value in range(skill_count):
                    self.skillSlot[value].SetAlwaysRenderCoverButton(0, False)
                    self.wndPetMiniInfo.SetAlwaysRenderCoverButton(value)

            if pet_skill1:
                self.skillSlot[0].SetPetSkillSlotNew(0, pet_skill1)
                self.skillSlot[0].SetSlotCount(0, pet_skill_level1)
                self.skillSlot[0].SetCoverButton(0)
                self.wndPetMiniInfo.SetSkillSlot(0, 0, pet_skill1)

                if player.PET_GROWTH_SKILL_LEVEL_MAX > pet_skill_level1:
                    self.skillSlot[0].ShowSlotButton(0)

                (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                    pet_skill1)
                if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                    if curTime <= pet_skill_cool1:
                        curCoolTime = pet_skill_cool1 - curTime
                        curCoolTime = pet_skill_cool_time - curCoolTime
                        self.skillSlot[0].SetSlotCoolTime(0, pet_skill_cool_time, curCoolTime)
                        self.wndPetMiniInfo.SetSkillCoolTime(0, 0, pet_skill_cool_time, curCoolTime)
                    else:
                        if self.AffectShower:
                            self.AffectShower.SetPetSkillAffect(1, pet_skill1)

                elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                    if self.AffectShower:
                        self.AffectShower.SetPetSkillAffect(1, pet_skill1)

            if pet_skill2:
                self.skillSlot[1].SetPetSkillSlotNew(0, pet_skill2)
                self.skillSlot[1].SetSlotCount(0, pet_skill_level2)
                self.skillSlot[1].SetCoverButton(0)
                self.wndPetMiniInfo.SetSkillSlot(1, 0, pet_skill2)

                if player.PET_GROWTH_SKILL_LEVEL_MAX > pet_skill_level2:
                    self.skillSlot[1].ShowSlotButton(0)

                (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                    pet_skill2)
                if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                    if curTime <= pet_skill_cool2:
                        curCoolTime = pet_skill_cool2 - curTime
                        curCoolTime = pet_skill_cool_time - curCoolTime
                        self.skillSlot[1].SetSlotCoolTime(0, pet_skill_cool_time, curCoolTime)
                        self.wndPetMiniInfo.SetSkillCoolTime(1, 0, pet_skill_cool_time, curCoolTime)
                    else:
                        if self.AffectShower:
                            self.AffectShower.SetPetSkillAffect(2, pet_skill2)

                elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                    if self.AffectShower:
                        self.AffectShower.SetPetSkillAffect(2, pet_skill2)

            if pet_skill3:
                self.skillSlot[2].SetPetSkillSlotNew(0, pet_skill3)
                self.skillSlot[2].SetSlotCount(0, pet_skill_level3)
                self.skillSlot[2].SetCoverButton(0)
                self.wndPetMiniInfo.SetSkillSlot(2, 0, pet_skill3)

                if player.PET_GROWTH_SKILL_LEVEL_MAX > pet_skill_level3:
                    self.skillSlot[2].ShowSlotButton(0)

                (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                    pet_skill3)
                if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                    if curTime <= pet_skill_cool3:
                        curCoolTime = pet_skill_cool3 - curTime
                        curCoolTime = pet_skill_cool_time - curCoolTime
                        self.skillSlot[2].SetSlotCoolTime(0, pet_skill_cool_time, curCoolTime)
                        self.wndPetMiniInfo.SetSkillCoolTime(2, 0, pet_skill_cool_time, curCoolTime)
                    else:
                        if self.AffectShower:
                            self.AffectShower.SetPetSkillAffect(3, pet_skill3)

                elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                    if self.AffectShower:
                        self.AffectShower.SetPetSkillAffect(3, pet_skill3)

            self.wndPetMiniInfo.Show()

        except:
            pass

    def __GetEvolName(self, evol_level):

        if 1 == evol_level:
            return localeInfo.PET_INFORMATION_STAGE1
        elif 2 == evol_level:
            return localeInfo.PET_INFORMATION_STAGE2
        elif 3 == evol_level:
            return localeInfo.PET_INFORMATION_STAGE3
        elif 4 == evol_level:
            return localeInfo.PET_INFORMATION_STAGE4

        return localeInfo.PET_INFORMATION_STAGE1

    def ClearStatus(self):
        self.wndUpBringingPetSlot.SetItemSlot(0, 0)
        self.GetChild("PetName").SetText("")
        self.GetChild("EvolName").SetText("")
        self.GetChild("LevelValue").SetText("")
        self.GetChild("AgeValue").SetText("")
        self.GetChild("LifeTextValue").SetText("")
        self.GetChild("DefValue").SetText("")
        self.GetChild("SpValue").SetText("")
        self.GetChild("HpValue").SetText("")
        self.SetExperience(0, 0, 0, 0)
        self.SetLifeTime(100, 100)
        self.__ClearPetSkillSlot()  ##½ºÅ³ clear

        if self.wndPetFeed:
            if self.wndPetFeed.IsShow():
                self.wndPetFeed.Close()

        self.__ClearSkillBookLearnEvent()
        self.__ClearSkillDeleteBookEvent()
        self.__ClearSkillUpgradeEvnet()

        if self.AffectShower:
            self.AffectShower.ClearPetSkillAffect()

        self.AllOffPetInfoFlashEvent()

        for evolInfoIndex in range(player.PET_GROWTH_EVOL_MAX):
            self.evolInfo[evolInfoIndex] = 0

    def PetAffectShowerRefresh(self):

        # ¿ÞÂÊ »ó´Ü ¾ÆÀÌÄÜ Clear
        if not self.AffectShower:
            return

        self.AffectShower.ClearPetSkillAffect()

        pet_id = player.GetActivePetItemId()
        if 0 == pet_id:
            return

        curTime = app.GetGlobalTimeStamp()

        ## Skill
        (skill_count, pet_skill1, pet_skill_level1, pet_skill_cool1, pet_skill2, pet_skill_level2, pet_skill_cool2,
         pet_skill3, pet_skill_level3, pet_skill_cool3) = player.GetPetSkill(pet_id)

        if pet_skill1:

            (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                pet_skill1)

            if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                if curTime <= pet_skill_cool1:
                    pass
                else:
                    self.AffectShower.SetPetSkillAffect(1, pet_skill1)

            elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                self.AffectShower.SetPetSkillAffect(1, pet_skill1)

        if pet_skill2:
            (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                pet_skill2)

            if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                if curTime <= pet_skill_cool2:
                    pass
                else:
                    self.AffectShower.SetPetSkillAffect(2, pet_skill2)

            elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                self.AffectShower.SetPetSkillAffect(2, pet_skill2)

        if pet_skill3:

            (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(
                pet_skill3)

            if skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                if curTime <= pet_skill_cool3:
                    pass
                else:
                    self.AffectShower.SetPetSkillAffect(3, pet_skill3)

            elif skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                self.AffectShower.SetPetSkillAffect(3, pet_skill3)

    def PetSpecialEvolFlashEventCheck(self, evol_level, birthSec):

        if evol_level == player.PET_GROWTH_SKILL_OPEN_EVOL_LEVEL - 1:
            if birthSec > player.SPECIAL_EVOL_MIN_AGE:
                self.OnPetInfoFlashEvent(player.FEED_EVOL_EVENT)

    def PetLifeTimeFlashEventCheck(self, lifeTime):

        if lifeTime < player.LIFE_TIME_FLASH_MIN_TIME:
            self.OnPetInfoFlashEvent(player.FEED_LIFE_TIME_EVENT)

    def PetFlashEvent(self, index):

        if player.FEED_BUTTON_MAX == index:
            self.AllOffPetInfoFlashEvent()
        else:
            self.OnPetInfoFlashEvent(index)

    def OnPetInfoFlashEvent(self, index):

        if self.wndPetMiniInfo:
            self.wndPetMiniInfo.OnFlashEvent()

        self.EnableFlashButtonEvent(index)

    def AllOffPetInfoFlashEvent(self):

        if self.wndPetMiniInfo:
            self.wndPetMiniInfo.OffFlashEvent()

        for i in xrange(player.FEED_BUTTON_MAX):
            self.DisableFlashButtonEvent(i)

    def EnableFlashButtonEvent(self, index):

        if index < 0 or index >= player.FEED_BUTTON_MAX:
            return

        if self.feedButton[index]:
            self.feedButton[index].EnableFlash()

    def DisableFlashButtonEvent(self, index):

        if index < 0 or index >= player.FEED_BUTTON_MAX:
            return

        if self.feedButton[index]:
            self.feedButton[index].DisableFlash()

    def SetExperience(self, curPoint, maxPoint, itemExp, itemExpMax):

        curPoint = min(curPoint, maxPoint)
        curPoint = max(curPoint, 0)
        maxPoint = max(maxPoint, 0)

        itemExp = min(itemExp, itemExpMax)
        itemExp = max(itemExp, 0)
        itemExpMax = max(itemExpMax, 0)

        ## »ç³ÉÀ¸·Î È¹µæÇÑ °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
        quarterPoint = maxPoint / BATTLE_EXP_GAUGE_MAX
        FullCount = 0

        if 0 != quarterPoint:
            FullCount = min(BATTLE_EXP_GAUGE_MAX, curPoint / quarterPoint)

        for i in xrange(TOTAL_EXP_GAUGE_COUNT):
            self.expGauge[i].Hide()

        for i in xrange(int(FullCount)):
            self.expGauge[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
            self.expGauge[i].Show()

        if 0 != quarterPoint:
            if FullCount < BATTLE_EXP_GAUGE_MAX:
                Percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
                self.expGauge[int(FullCount)].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
                self.expGauge[int(FullCount)].Show()

        ## ¾ÆÀÌÅÛÀ¸·Î È¹µæÇÑ °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
        ## self.expGauge ÀÇ ¸¶Áö¸· °ªÀÌ item exp ±¸½½ÀÌ´Ù.
        ## Top °ªÀÌ 0 ÀÌ¸é ²ËÂù ±¸½½
        ## Top °ªÀÌ -1 ÀÌ¸é ºó ±¸½½
        if 0 != itemExpMax:
            itemExpGauge = self.expGauge[ITEM_EXP_GAUGE_POS]
            Percentage = float(itemExp) / float(itemExpMax) - float(1.0)
            itemExpGauge.SetRenderingRect(0.0, Percentage, 0.0, 0.0)
            itemExpGauge.Show()

        output_cur_exp = curPoint + itemExp
        output_max_exp = maxPoint + itemExpMax

        ## TEXT Ãâ·ÂÀº »ç³É°æÇèÄ¡ + ¾ÆÀÌÅÛ °æÇèÄ¡·Î ÇÑ´Ù.
        if app.WJ_MULTI_TEXTLINE:

            if localeInfo.IsARABIC():
                tooltip_text = str(curPoint) + ' :' + str(localeInfo.PET_INFO_EXP) + '\\n' \
                               + str(maxPoint - curPoint) + ' :' + str(localeInfo.PET_INFO_NEXT_EXP) + '\\n' \
                               + str(itemExp) + ' :' + str(localeInfo.PET_INFO_ITEM_EXP) + '\\n' \
                               + str(itemExpMax - itemExp) + ' :' + str(localeInfo.PET_INFO_NEXT_ITEM_EXP)
                self.tooltipEXP.SetText(tooltip_text)
            else:
                tooltip_text = str(localeInfo.PET_INFO_EXP) + ': ' + str(curPoint) + '\\n' \
                               + str(localeInfo.PET_INFO_NEXT_EXP) + ': ' + str(maxPoint - curPoint) + '\\n' \
                               + str(localeInfo.PET_INFO_ITEM_EXP) + ': ' + str(itemExp) + '\\n' \
                               + str(localeInfo.PET_INFO_NEXT_ITEM_EXP) + ': ' + str(itemExpMax - itemExp)

                self.tooltipEXP.SetText(tooltip_text)
        else:
            self.tooltipEXP.SetText("%s : %.2f%%" % (
            localeInfo.TASKBAR_EXP, float(output_cur_exp) / max(1, float(output_max_exp - output_cur_exp)) * 100))

    def SetLifeTime(self, curPoint, maxPoint):
        curPoint = min(curPoint, maxPoint)
        if maxPoint > 0:
            self.lifeTimeGauge.SetPercentageWithScale(curPoint, maxPoint)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = tooltipItem

    def SetInven(self, inven):
        from _weakref import proxy
        self.inven = proxy(inven)

    def IsFeedWindowOpen(self):
        if self.wndPetFeed:
            if self.wndPetFeed.IsShow():
                return True

        return False

    def GetPetHatchingWindow(self):
        return self.wndPetHatching

    def GetPetFeedWindow(self):
        return self.wndPetFeed

    def CantFeedItem(self, InvenSlot):

        if self.feedIndex == player.FEED_LIFE_TIME_EVENT:
            return self.__CantLifeTimeFeedItem(InvenSlot)

        elif self.feedIndex == player.FEED_EVOL_EVENT:
            return self.__CantEvolFeedItem(InvenSlot)

        elif self.feedIndex == player.FEED_EXP_EVENT:
            return self.__CantExpFeedItem(InvenSlot)

        return False

    def __CantLifeTimeFeedItem(self, InvenSlot):
        ItemVNum = player.GetItemIndex(InvenSlot)

        if ItemVNum == 0:
            return False

        item.SelectItem(ItemVNum)
        if item.GetItemType() == item.ITEM_TYPE_PET:
            if item.GetItemSubType() in [item.PET_UPBRINGING, item.PET_EGG]:
                return False

        return True

    def __CantEvolFeedItem(self, InvenSlot):
        return False

    def __CantExpFeedItem(self, InvenSlot):
        ItemVNum = player.GetItemIndex(InvenSlot)

        if ItemVNum == 0:
            return False

        item.SelectItem(ItemVNum)
        if item.GetItemType() in [item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR, item.ITEM_TYPE_BELT]:
            return False

        return True

    def PetInfoBindAffectShower(self, affect_shower):
        from _weakref import proxy
        self.AffectShower = proxy(affect_shower)

    def SetPetSkillToolTip(self, tooltipPetSkill):
        self.tooptipPetSkill = tooltipPetSkill

    def PetEvolInfo(self, index, value):

        if index < 0 or index >= player.PET_GROWTH_EVOL_MAX:
            return

        self.evolInfo[index] = value

    def GetEvolInfo(self, index):

        if index < 0 or index >= player.PET_GROWTH_EVOL_MAX:
            return 0

        return self.evolInfo[index]

    def PetFeedReuslt(self, result):

        if not self.wndPetFeed:
            return

        if True == result:
            self.wndPetFeed.BackUpSucceedFeedItems()

        self.wndPetFeed.ClearFeedItems()

