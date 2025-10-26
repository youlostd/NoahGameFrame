# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
import chat
import item
import player
import safebox
import snd
from pygame.app import appInst
from pygame.item import MakeItemPosition

import localeInfo
import mouseModule
import ui
from ui_event import MakeCallback, Event


class PasswordDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.__LoadDialog()

        self.sendMessage = "/safebox_password "

    def __LoadDialog(self):
        ui.PythonScriptLoader().LoadScriptFile(self, "UIScript/PasswordDialog.py")

        self.passwordValue = self.GetChild("password_value")
        self.acceptButton = self.GetChild("accept_button")
        self.cancelButton = self.GetChild("cancel_button")
        self.titleBar = self.GetChild("titlebar")
        self.titleBar.SetCloseEvent(self.CloseDialog)

        self.passwordValue.SetReturnEvent(self.OnAccept)
        self.passwordValue.OnPressEscapeKey = MakeCallback(self.OnCancel)
        self.acceptButton.SetEvent(self.OnAccept)
        self.cancelButton.SetEvent(self.OnCancel)

    def Destroy(self):
        self.ClearDictionary()
        self.passwordValue = None
        self.acceptButton = None
        self.cancelButton = None
        self.titleBar = None

    def SetTitle(self, title):
        self.titleBar.SetTitleName(title)

    def SetSendMessage(self, msg):
        self.sendMessage = msg

    def ShowDialog(self):
        if app.ENABLE_GROWTH_PET_SYSTEM:
            if player.IsOpenPetHatchingWindow():
                chat.AppendChat(
                    CHAT_TYPE_INFO, localeInfo.PET_HATCHING_WINDOW_OPEN_CAN_NOT_USE
                )
                return False

        self.passwordValue.SetText("")
        self.passwordValue.SetFocus()
        self.SetCenterPosition()
        self.Show()

    def CloseDialog(self):
        self.passwordValue.KillFocus()
        self.Hide()

    def OnAccept(self):
        appInst.instance().GetNet().SendChatPacket(
            self.sendMessage + self.passwordValue.GetText()
        )
        self.CloseDialog()
        return True

    def OnCancel(self):
        self.CloseDialog()
        return True


class ChangePasswordDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.oldPassword = None
        self.newPassword = None
        self.newPasswordCheck = None

        self.__LoadDialog()

    def __LoadDialog(self):
        self.dlgMessage = ui.ScriptWindow()
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self.dlgMessage, "uiscript/popupdialog.py")

        self.dlgMessage.GetChild("message").SetText(localeInfo.SAFEBOX_WRONG_PASSWORD)
        self.dlgMessage.GetChild("accept").SetEvent(self.OnCloseMessageDialog)

    def LoadDialog(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "uiscript/changepassworddialog.py")

        self.GetChild("accept_button").SetEvent(self.OnAccept)
        self.GetChild("cancel_button").SetEvent(self.OnCancel)
        self.GetChild("titlebar").SetCloseEvent(self.OnCancel)
        oldPassword = self.GetChild("old_password_value")
        newPassword = self.GetChild("new_password_value")
        newPasswordCheck = self.GetChild("new_password_check_value")

        oldPassword.SetTabEvent(Event(self.OnNextFocus, 1))
        newPassword.SetTabEvent(Event(self.OnNextFocus, 2))
        newPasswordCheck.SetTabEvent(Event(self.OnNextFocus, 3))
        oldPassword.SetReturnEvent(Event(self.OnNextFocus, 1))
        newPassword.SetReturnEvent(Event(self.OnNextFocus, 2))
        newPasswordCheck.SetReturnEvent(self.OnAccept)
        oldPassword.OnPressEscapeKey = self.OnCancel
        newPassword.OnPressEscapeKey = self.OnCancel
        newPasswordCheck.OnPressEscapeKey = self.OnCancel

        self.oldPassword = oldPassword
        self.newPassword = newPassword
        self.newPasswordCheck = newPasswordCheck

    def OnNextFocus(self, arg):
        if 1 == arg:
            self.oldPassword.KillFocus()
            self.newPassword.SetFocus()
        elif 2 == arg:
            self.newPassword.KillFocus()
            self.newPasswordCheck.SetFocus()
        elif 3 == arg:
            self.newPasswordCheck.KillFocus()
            self.oldPassword.SetFocus()

    def Destroy(self):
        self.ClearDictionary()
        self.dlgMessage.ClearDictionary()
        self.oldPassword = None
        self.newPassword = None
        self.newPasswordCheck = None

    def Open(self):
        self.oldPassword.SetText("")
        self.newPassword.SetText("")
        self.newPasswordCheck.SetText("")
        self.oldPassword.SetFocus()
        self.SetCenterPosition()
        self.SetTop()
        self.SetFocus()
        self.Show()

    def Close(self):
        self.oldPassword.SetText("")
        self.newPassword.SetText("")
        self.newPasswordCheck.SetText("")
        self.oldPassword.KillFocus()
        self.newPassword.KillFocus()
        self.newPasswordCheck.KillFocus()
        self.Hide()

    def OnAccept(self):
        oldPasswordText = self.oldPassword.GetText()
        newPasswordText = self.newPassword.GetText()
        newPasswordCheckText = self.newPasswordCheck.GetText()
        if newPasswordText != newPasswordCheckText:
            self.dlgMessage.SetCenterPosition()
            self.dlgMessage.SetTop()
            self.dlgMessage.Show()
            return True
        appInst.instance().GetNet().SendChatPacket(
            "/safebox_change_password {} {}".format(oldPasswordText, newPasswordText)
        )
        self.Close()
        return True

    def OnCancel(self):
        self.Close()
        return True

    def OnCloseMessageDialog(self):
        self.newPassword.SetText("")
        self.newPasswordCheck.SetText("")
        self.newPassword.SetFocus()
        self.dlgMessage.Hide()


class SafeboxWindow(ui.ScriptWindow):
    BOX_WIDTH = 216 + 25

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.tooltipItem = None
        self.sellingSlotNumber = -1
        self.pageButtonList = []
        self.curPageIndex = 0
        self.isLoaded = 0
        self.xSafeBoxStart = 0
        self.ySafeBoxStart = 0
        self.interface = None
        self.dlgPickMoney = None
        self.dlgChangePassword = None
        self.wndItem = None

    def Show(self):
        self.__LoadWindow()

        ui.ScriptWindow.Show(self)

    def Destroy(self):
        self.ClearDictionary()

        if self.dlgPickMoney:
            self.dlgPickMoney.Destroy()
            self.dlgPickMoney = None

        if self.dlgChangePassword:
            self.dlgChangePassword.Destroy()
            self.dlgChangePassword = None

        self.tooltipItem = None
        self.wndMoneySlot = None
        self.wndMoney = None
        self.wndBoard = None
        self.wndItem = None

        self.pageButtonList = []

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/SafeboxWindow.py")

        ## Item
        wndItem = ui.GridSlotWindow()
        wndItem.SetParent(self)
        wndItem.SetPosition(8, 40)
        wndItem.SetSelectEmptySlotEvent(self.SelectEmptySlot)
        wndItem.SetSelectItemSlotEvent(self.SelectItemSlot)
        wndItem.SetUnselectItemSlotEvent(self.UseItemSlot)
        wndItem.SetUseSlotEvent(self.UseItemSlot)
        wndItem.SetOverInItemEvent(self.OverInItem)
        wndItem.SetOverOutItemEvent(self.OverOutItem)
        wndItem.Show()

        ## PickMoneyDialog
        import uiPickMoney

        dlgPickMoney = uiPickMoney.PickMoneyDialog()
        dlgPickMoney.LoadDialog()
        dlgPickMoney.SetAcceptEvent(self.OnPickMoney)
        dlgPickMoney.Hide()

        ## ChangePasswrod
        dlgChangePassword = ChangePasswordDialog()
        dlgChangePassword.LoadDialog()
        dlgChangePassword.Hide()

        ## Close Button
        self.wndTitleBar = self.GetChild("TitleBar")
        self.wndTitleBar.SetCloseEvent(self.Close)
        self.GetChild("ChangePasswordButton").SetEvent(self.OnChangePassword)
        self.GetChild("ExitButton").SetEvent(self.Close)

        self.wndItem = wndItem
        self.dlgPickMoney = dlgPickMoney
        self.dlgChangePassword = dlgChangePassword
        self.wndBoard = self.GetChild("board")
        # self.wndMoney = self.GetChild("Money")
        # self.wndMoneySlot = self.GetChild("Money_Slot")
        # self.wndMoneySlot.SetEvent(self.OpenPickMoneyDialog)

        ## Initialize
        self.SetTableSize(15)
        self.RefreshSafeboxMoney()

    def OpenPickMoneyDialog(self):

        if mouseModule.mouseController.isAttached():
            attachedSlotPos = mouseModule.mouseController.GetRealAttachedSlotNumber()
            if (
                player.SLOT_TYPE_INVENTORY
                == mouseModule.mouseController.GetAttachedType()
            ):

                if (
                    player.ITEM_MONEY
                    == mouseModule.mouseController.GetAttachedItemIndex()
                ):
                    appInst.instance().GetNet().SendSafeboxSaveMoneyPacket(
                        mouseModule.mouseController.GetAttachedItemCount()
                    )
                    snd.PlaySound("sound/ui/money.wav")

            mouseModule.mouseController.DeattachObject()

        else:
            curMoney = safebox.GetMoney()

            if curMoney <= 0:
                return

            if self.dlgPickMoney:
                self.dlgPickMoney.Open(curMoney)

    def ShowWindow(self, size):
        self.__LoadWindow()

        (self.xSafeBoxStart, self.ySafeBoxStart, z) = player.GetMainCharacterPosition()

        self.SetTableSize(size)
        self.Show()
        self.SetTop()

        safebox.SetOpen(True)
        if app.ENABLE_GROWTH_PET_SYSTEM:
            player.SetOpenSafeBox(True)

    def __MakePageButton(self, pageCount):

        self.curPageIndex = 0
        self.pageButtonList = []
        imageUp = "d:/ymir work/ui/game/windows/tab_button_small_01.sub"
        imageOver = "d:/ymir work/ui/game/windows/tab_button_small_02.sub"
        imageDown = "d:/ymir work/ui/game/windows/tab_button_small_03.sub"
        posY = 335
        for i in xrange(pageCount):
            button = ui.RadioButton()
            button.SetParent(self)
            button.SetUpVisual(imageUp)
            button.SetOverVisual(imageOver)
            button.SetDownVisual(imageDown)
            button.SetHorizontalAlignRight()
            button.SetVerticalAlignBottom()
            button.SetPosition(25, posY - 20)
            button.SetText(localeInfo.NumberToRoman(i + 1))
            button.SetEvent(Event(self.SelectPage, i))
            button.Show()
            self.pageButtonList.append(button)

            posY -= 18

        self.pageButtonList[0].Down()

    def SelectPage(self, index):

        self.curPageIndex = index

        for btn in self.pageButtonList:
            btn.SetUp()

        self.pageButtonList[index].Down()
        self.RefreshSafebox()

    def __LocalPosToGlobalPos(self, local):
        return self.curPageIndex * safebox.SAFEBOX_PAGE_SIZE + local

    def SetTableSize(self, size):

        pageCount = max(1, size / safebox.SAFEBOX_SLOT_Y_COUNT)
        pageCount = min(15, pageCount)
        size = safebox.SAFEBOX_SLOT_Y_COUNT

        self.__MakePageButton(pageCount)

        if self.wndItem:
            self.wndItem.ArrangeSlot(
                0, safebox.SAFEBOX_SLOT_X_COUNT, size, 32, 32, 0, 0
            )
            self.wndItem.RefreshSlot()
            self.wndItem.SetSlotBaseImage(
                "d:/ymir work/ui/public/Slot_Base.sub", 1.0, 1.0, 1.0, 1.0
            )

        wnd_height = 100 + 32 * size
        self.wndBoard.SetSize(self.BOX_WIDTH, wnd_height)
        self.wndTitleBar.SetWidth(self.BOX_WIDTH - 15)
        self.SetSize(self.BOX_WIDTH, wnd_height)
        self.UpdateRect()

    def RefreshSafebox(self):
        if self.wndItem:
            getItemID = safebox.GetItemID
            getItemCount = safebox.GetItemCount
            setItemID = self.wndItem.SetItemSlot

            for i in xrange(safebox.SAFEBOX_PAGE_SIZE):
                slotIndex = self.__LocalPosToGlobalPos(i)
                itemCount = getItemCount(slotIndex)
                if itemCount <= 1:
                    itemCount = 0
                setItemID(i, getItemID(slotIndex), itemCount)
                if app.ENABLE_CHANGE_LOOK_SYSTEM:
                    if safebox.GetItemChangeLookVnum(slotIndex) == 0:
                        self.wndItem.EnableSlotCoverImage(i, False)
                    else:
                        self.wndItem.SetSlotCoverImage(
                            i, "icon/item/ingame_convert_Mark.tga"
                        )

            self.wndItem.RefreshSlot()

    def RefreshSafeboxMoney(self):
        pass

    # self.wndMoney.SetText(str(safebox.GetMoney()))

    def SetItemToolTip(self, tooltip):
        self.tooltipItem = tooltip

    def Close(self):
        safebox.SetOpen(False)
        appInst.instance().GetNet().SendChatPacket("/safebox_close")

    def CommandCloseSafebox(self):
        safebox.SetOpen(False)
        if app.ENABLE_GROWTH_PET_SYSTEM:
            player.SetOpenSafeBox(False)

        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

        if app.WJ_ENABLE_TRADABLE_ICON:
            if self.interface:
                self.interface.SetOnTopWindow(player.ON_TOP_WND_NONE)
                self.interface.RefreshMarkInventoryBag()
        if self.dlgPickMoney:
            self.dlgPickMoney.Close()
        if self.dlgChangePassword:
            self.dlgChangePassword.Close()
        self.Hide()

    ## Slot Event
    def SelectEmptySlot(self, selectedSlotPos):

        selectedSlotPos = self.__LocalPosToGlobalPos(selectedSlotPos)

        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            attachedSlotPosReal = (
                mouseModule.mouseController.GetRealAttachedSlotNumber()
            )

            if player.SLOT_TYPE_SAFEBOX == attachedSlotType:

                appInst.instance().GetNet().SendSafeboxItemMovePacket(
                    attachedSlotPosReal, selectedSlotPos, 0
                )
            # snd.PlaySound("sound/ui/drop.wav")
            else:
                attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
                if player.RESERVED_WINDOW == attachedInvenType:
                    return

                if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                    attPos = MakeItemPosition(attachedSlotPos[0], attachedSlotPos[1])
                else:
                    attPos = MakeItemPosition(attachedInvenType, attachedSlotPosReal)

                if (
                    player.ITEM_MONEY
                    == mouseModule.mouseController.GetAttachedItemIndex()
                ):
                    # appInst.instance().GetNet().SendSafeboxSaveMoneyPacket(mouseModule.mouseController.GetAttachedItemCount())
                    snd.PlaySound("sound/ui/money.wav")

                else:
                    appInst.instance().GetNet().SendSafeboxCheckinPacket(
                        attPos, selectedSlotPos
                    )
                    # snd.PlaySound("sound/ui/drop.wav")

            mouseModule.mouseController.DeattachObject()

    def SelectItemSlot(self, selectedSlotPos):

        selectedSlotPos = self.__LocalPosToGlobalPos(selectedSlotPos)

        if mouseModule.mouseController.isAttached():

            attachedSlotType = mouseModule.mouseController.GetAttachedType()
            attachedSlotPos = mouseModule.mouseController.GetAttachedSlotPosition()
            if player.SLOT_TYPE_INVENTORY == attachedSlotType:
                pass
            elif player.SLOT_TYPE_SAFEBOX == attachedSlotType:
                appInst.instance().GetNet().SendSafeboxItemMovePacket(
                    attachedSlotPos, selectedSlotPos, 0
                )

            mouseModule.mouseController.DeattachObject()

        else:

            curCursorNum = app.GetCursor()
            if app.SELL == curCursorNum:
                chat.AppendChat(
                    CHAT_TYPE_INFO, localeInfo.SAFEBOX_SELL_DISABLE_SAFEITEM
                )

            elif app.BUY == curCursorNum:
                chat.AppendChat(CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)

            else:
                selectedItemID = safebox.GetItemID(selectedSlotPos)
                mouseModule.mouseController.AttachObject(
                    self, player.SLOT_TYPE_SAFEBOX, selectedSlotPos, selectedItemID
                )
                snd.PlaySound("sound/ui/pick.wav")

    def UseItemSlot(self, slotIndex):
        appInst.instance().GetNet().SendSafeboxCheckoutPacket(
            self.__LocalPosToGlobalPos(slotIndex),
            MakeItemPosition(player.RESERVED_WINDOW, 0),
        )
        mouseModule.mouseController.DeattachObject()

    def __ShowToolTip(self, slotIndex):
        if self.tooltipItem:
            self.tooltipItem.SetSafeBoxItem(slotIndex)

    def OverInItem(self, slotIndex):
        slotIndex = self.__LocalPosToGlobalPos(slotIndex)
        self.wndItem.SetUsableItem(False)
        self.__ShowToolTip(slotIndex)

    def OverOutItem(self):
        self.wndItem.SetUsableItem(False)
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OnPickMoney(self, money):
        mouseModule.mouseController.AttachMoney(self, player.SLOT_TYPE_SAFEBOX, money)

    def OnChangePassword(self):
        self.dlgChangePassword.Open()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True

    def OnUpdate(self):

        USE_SAFEBOX_LIMIT_RANGE = 1000

        (x, y, z) = player.GetMainCharacterPosition()
        if (
            abs(x - self.xSafeBoxStart) > USE_SAFEBOX_LIMIT_RANGE
            or abs(y - self.ySafeBoxStart) > USE_SAFEBOX_LIMIT_RANGE
        ):
            self.Close()

    if app.WJ_ENABLE_TRADABLE_ICON:

        def CantCheckInItem(self, slotIndex):
            itemIndex = player.GetItemIndex(slotIndex)

            if itemIndex:
                return player.IsAntiFlagBySlot(slotIndex, item.ANTIFLAG_SAFEBOX)

            return False

        def BindInterface(self, interface):
            from _weakref import proxy

            self.interface = proxy(interface)

        def OnSetFocus(self):
            if not self.interface:
                return

            self.interface.SetOnTopWindow(player.ON_TOP_WND_SAFEBOX)
            self.interface.RefreshMarkInventoryBag()


class MallWindow(ui.ScriptWindow):
    BOX_WIDTH = 176

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.tooltipItem = None
        self.sellingSlotNumber = -1
        self.pageButtonList = []
        self.curPageIndex = 0
        self.isLoaded = 0
        self.xSafeBoxStart = 0
        self.ySafeBoxStart = 0
        self.interface = None
        self.wndBoard = None
        self.wndItem = None
        self.exitButton = None

    def Show(self):
        self.__LoadWindow()

        ui.ScriptWindow.Show(self)

    def Destroy(self):
        self.ClearDictionary()

        self.tooltipItem = None
        self.wndBoard = None
        self.wndItem = None
        self.exitButton = None

        self.pageButtonList = []

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/MallWindow.py")

        ## Item
        wndItem = ui.GridSlotWindow()
        wndItem.SetParent(self)
        wndItem.SetPosition(8, 39)
        wndItem.SetSelectEmptySlotEvent(self.SelectEmptySlot)
        wndItem.SetSelectItemSlotEvent(self.SelectItemSlot)
        wndItem.SetUnselectItemSlotEvent(self.UseItemSlot)
        wndItem.SetUseSlotEvent(self.UseItemSlot)
        wndItem.SetOverInItemEvent(self.OverInItem)
        wndItem.SetOverOutItemEvent(self.OverOutItem)
        wndItem.Show()

        ## Close Button
        self.GetChild("TitleBar").SetCloseEvent(self.Close)
        self.exitButton = self.GetChild("ExitButton")
        self.exitButton.SetEvent(self.Close)
        self.exitButton.Hide()

        self.wndItem = wndItem
        self.wndBoard = self.GetChild("board")

        ## Initialize
        self.SetTableSize(3)

    def ShowWindow(self, size):
        self.__LoadWindow()

        (self.xSafeBoxStart, self.ySafeBoxStart, z) = player.GetMainCharacterPosition()

        self.SetTableSize(size)
        self.Show()
        self.SetTop()

        if app.ENABLE_GROWTH_PET_SYSTEM:
            player.SetOpenMall(True)

    def SetTableSize(self, size):
        pageCount = max(1, size / safebox.SAFEBOX_SLOT_Y_COUNT)
        pageCount = min(3, pageCount)
        size = safebox.SAFEBOX_SLOT_Y_COUNT

        self.__MakePageButton(pageCount)
        if self.wndItem:
            self.wndItem.ArrangeSlot(
                0, safebox.SAFEBOX_SLOT_X_COUNT, size, 32, 32, 0, 0
            )
            self.wndItem.RefreshSlot()
            self.wndItem.SetSlotBaseImage(
                "d:/ymir work/ui/public/Slot_Base.sub", 1.0, 1.0, 1.0, 1.0
            )

        self.wndBoard.SetSize(self.BOX_WIDTH, 82 + 32 * size)
        self.SetSize(self.BOX_WIDTH, 85 + 32 * size)
        self.UpdateRect()

    def __MakePageButton(self, pageCount):

        self.curPageIndex = 0
        self.pageButtonList = []

        imageUp = "d:/ymir work/ui/game/windows/tab_button_middle_01.sub"
        imageOver = "d:/ymir work/ui/game/windows/tab_button_middle_02.sub"
        imageDown = "d:/ymir work/ui/game/windows/tab_button_middle_03.sub"

        pos = -109
        for i in xrange(pageCount):
            button = ui.RadioButton()
            button.SetParent(self)
            button.SetUpVisual(imageUp)
            button.SetOverVisual(imageOver)
            button.SetDownVisual(imageDown)
            button.SetButtonScale(1, 1)
            button.SetHorizontalAlignCenter()
            button.SetVerticalAlignBottom()
            button.SetPosition(pos, 20)
            button.SetText(localeInfo.NumberToRoman(i + 1))
            button.SetEvent(Event(self.SelectPage, i))
            button.Show()
            self.pageButtonList.append(button)

            pos += 109

        self.pageButtonList[0].Down()

    def SelectPage(self, index):

        self.curPageIndex = index

        for btn in self.pageButtonList:
            btn.SetUp()

        self.pageButtonList[index].Down()
        self.RefreshMall()

    def __LocalPosToGlobalPos(self, local):
        return self.curPageIndex * safebox.SAFEBOX_PAGE_SIZE + local

    def RefreshMall(self):
        getItemID = safebox.GetMallItemID
        getItemCount = safebox.GetMallItemCount
        setItemID = self.wndItem.SetItemSlot

        for i in xrange(safebox.SAFEBOX_PAGE_SIZE):
            slotIndex = self.__LocalPosToGlobalPos(i)
            itemCount = getItemCount(slotIndex)
            if itemCount <= 1:
                itemCount = 0
            setItemID(i, getItemID(slotIndex), itemCount)
            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                if safebox.GetMallItemChangeLookVnum(slotIndex) == 0:
                    self.wndItem.EnableSlotCoverImage(i, False)
                else:
                    self.wndItem.SetSlotCoverImage(
                        i, "icon/item/ingame_convert_Mark.tga"
                    )

        self.wndItem.RefreshSlot()

    def SetItemToolTip(self, tooltip):
        self.tooltipItem = tooltip

    def Close(self):
        appInst.instance().GetNet().SendChatPacket("/mall_close")

    def CommandCloseMall(self):
        if app.ENABLE_GROWTH_PET_SYSTEM:
            player.SetOpenMall(False)
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

        self.Hide()

    ## Slot Event
    def SelectEmptySlot(self, selectedSlotPos):
        selectedSlotPos = self.__LocalPosToGlobalPos(selectedSlotPos)

        if mouseModule.mouseController.isAttached():
            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MALL_CANNOT_INSERT)
            mouseModule.mouseController.DeattachObject()

    def SelectItemSlot(self, selectedSlotPos):
        selectedSlotPos = self.__LocalPosToGlobalPos(selectedSlotPos)

        if mouseModule.mouseController.isAttached():

            chat.AppendChat(CHAT_TYPE_INFO, localeInfo.MALL_CANNOT_INSERT)
            mouseModule.mouseController.DeattachObject()

        else:

            curCursorNum = app.GetCursor()
            selectedItemID = safebox.GetMallItemID(selectedSlotPos)
            mouseModule.mouseController.AttachObject(
                self, player.SLOT_TYPE_MALL, selectedSlotPos, selectedItemID
            )
            snd.PlaySound("sound/ui/pick.wav")

    def UseItemSlot(self, slotIndex):
        appInst.instance().GetNet().SendMallCheckoutPacket(
            self.__LocalPosToGlobalPos(slotIndex),
            MakeItemPosition(player.RESERVED_WINDOW, 0),
        )
        mouseModule.mouseController.DeattachObject()

    def __ShowToolTip(self, slotIndex):
        if self.tooltipItem:
            self.tooltipItem.SetMallItem(slotIndex)

    def OverInItem(self, slotIndex):
        self.__ShowToolTip(slotIndex)

    def OverOutItem(self):
        self.wndItem.SetUsableItem(False)
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
