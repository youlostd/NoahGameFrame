# -*- coding: utf-8 -*-

import logging
from _weakref import proxy

import grp
import item
import player
from pygame.app import appInst
from pygame.item import ItemApply
from pygame.item import itemManager
from pygame.player import playerInst

import localeInfo
import ui
from uiToolTip import ItemToolTip
from ui_event import Event, MakeEvent


def FormatApplyAsString(affectType, affectValue):
    if 0 == affectType:
        return None

    try:
        return ItemToolTip.AFFECT_DICT[affectType].format(localeInfo.DottedNumber(affectValue))
    except TypeError:
        try:
            return ItemToolTip.AFFECT_DICT[affectType].format(localeInfo.DottedNumber(affectValue))
        except TypeError:
            return ItemToolTip.AFFECT_DICT[affectType]
    except KeyError:
        return "UNKNOWN_TYPE[%s] %s" % (affectType, affectValue)


def GetAttrMaxValue(vnum, attrNum):
    itemAttr = itemManager().GetItemAttr(attrNum)
    item.SelectItem(vnum)
    attrIndex = item.GetAttributeSetIndex()
    if attrIndex != -1:
        return itemAttr.values[itemAttr.maxBySet[attrIndex] - 1]
    else:
        return 0


def GetRareAttrMaxValue(vnum, attrNum):
    itemAttr = itemManager().GetRareItemAttr(attrNum)
    item.SelectItem(vnum)
    attrIndex = item.GetAttributeSetIndex()
    if attrIndex != -1:
        return itemAttr.values[itemAttr.maxBySet[attrIndex] - 1]
    else:
        return 0


def GetAvailableAttr(itemVnum, isRare=False):
    avail = []
    attrIndex = -1
    addonType = 0

    if itemVnum:
        proto = itemManager().GetProto(itemVnum)
        if proto:
            attrIndex = proto.GetAttributeSetIndex()
            addonType = proto.GetAddonType()

    for i in xrange(114):
        if not isRare:
            itemAttr = itemManager().GetItemAttr(i)
            if itemAttr.maxBySet[attrIndex] or (attrIndex == -1 and itemAttr.apply != 0):
                avail.append(i)
        else:
            itemAttr = itemManager().GetRareItemAttr(i)
            if itemAttr.maxBySet[attrIndex] or (attrIndex == -1 and itemAttr.apply != 0):
                avail.append(i)

        if i == 71 or i == 72:
            if addonType == -1:
                avail.append(i)

    return avail


# Max 10!!
MAX_NUM = 6

SWITCH_DELAY = 26

# Standardfarben:
COLOR_BG = grp.GenerateColor(0.0, 0.0, 0.0, 0.5)
COLOR_INACTIVE = grp.GenerateColor(1.0, 0.0, 0.0, 0.2)
COLOR_ACTIVE = grp.GenerateColor(1.0, 0.6, 0.1, 0.2)
COLOR_FINISHED = grp.GenerateColor(0.0, 1.0, 0.0, 0.2)

COLOR_INACTIVE_RARE = grp.GenerateColor(1.0, 0.2, 0.0, 0.2)
COLOR_ACTIVE_RARE = grp.GenerateColor(1.0, 0.7, 0.2, 0.2)

COLOR_HIGHLIGHT_RARE = grp.GenerateColor(1.0, 0.2, 0.2, 0.05)

COLOR_PIN_HINT = grp.GenerateColor(0.0, 0.5, 1.0, 0.3)

COLOR_CHECKBOX_NOT_SELECTED = grp.GenerateColor(1.0, 0.3, 0.0, 0.1)
COLOR_CHECKBOX_SELECTED = grp.GenerateColor(0.3, 1.0, 1.0, 0.3)

DISTANCE_BOTTOM = 36
AVAILABLE_AFFECT_DICT = {}
AVAILABLE_RARE_AFFECT_DICT = {}


def CreateAvailableDict():
    global AVAILABLE_AFFECT_DICT, AVAILABLE_RARE_AFFECT_DICT
    availAttr = GetAvailableAttr(0)
    availAttrRare = GetAvailableAttr(0, True)
    AVAILABLE_AFFECT_DICT = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if k in availAttr}
    AVAILABLE_RARE_AFFECT_DICT = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if k in availAttrRare}


CreateAvailableDict()
SWITCHBOT_SLOT_COUNT = 6


class AttrInputRow(ui.Window):
    def __init__(self, parent, index):
        ui.Window.__init__(self)
        self.switchbot = proxy(parent)

        self.eventAccept = None
        self.eventCancel = None
        self.applyIndex = index
        self.applyComboBox = None
        self.applyButton = None
        self.bonusSelected = False

        self.width = 0

        self.__CreateAutoCompleteBox()
        self.__CreateValueInputBox()
        self.__CreateCancelButton()

    def __CreateAutoCompleteBox(self):
        ## Attr Type Selection
        applyChoose = ui.ComboBoxAutoComplete(self, "d:/ymir work/ui/gui/dropdown_background.sub", 0, 0)
        applyChoose.InsertItem(0, "None")
        applyChoose.SetCurrentItem("None")
        global AVAILABLE_AFFECT_DICT

        for index, data in AVAILABLE_AFFECT_DICT.iteritems():
            applyChoose.InsertItem(index, FormatApplyAsString(index, 0))

        applyChoose.Show()
        self.applyComboBox = applyChoose

        self.width += self.applyComboBox.GetWidth() + 6

    def __CreateValueInputBox(self):
        applyValueInput = ui.InputBox(self, "d:/ymir work/ui/gui/small_input.sub",
                                      self.width, 3, 4, "0", True)
        self.applyValueInput = applyValueInput
        self.width += self.applyValueInput.GetWidth() + 6

    def __CreateCancelButton(self):
        ## Cancel Button
        applyCancelButton = ui.MakeButton(self, self.width, 3, localeInfo.UI_ACCEPT, "d:/ymir work/ui/gui/",
                                          "accept.sub", "accept_over.sub", "accept_down.sub")
        self.applyButton = applyCancelButton
        self.applyButton.SetEvent(self.OnAcceptButtonPress)

    def Disable(self):
        self.applyButton.SetUpVisual("d:/ymir work/ui/gui/cancel.sub")
        self.applyButton.SetOverVisual("d:/ymir work/ui/gui/cancel_over.sub")
        self.applyButton.SetDownVisual("d:/ymir work/ui/gui/cancel_down.sub")
        self.applyButton.SetToolTipText(localeInfo.GUILD_DELETE)
        self.applyButton.SetEvent(self.OnCancelButtonPress)
        self.DisableComboBox()

    def Enable(self):
        self.applyButton.SetUpVisual("d:/ymir work/ui/gui/accept.sub")
        self.applyButton.SetOverVisual("d:/ymir work/ui/gui/accept_over.sub")
        self.applyButton.SetDownVisual("d:/ymir work/ui/gui/accept_down.sub")
        self.applyButton.SetToolTipText(localeInfo.UI_ACCEPT)
        self.applyButton.SetEvent(self.OnAcceptButtonPress)

    def GetComboBox(self):
        return self.applyComboBox

    def GetValueInput(self):
        return self.applyValueInput

    ## Event Setters
    ## Event Setters
    def SetAttributeAutoCompleteEvent(self, event):
        self.applyComboBox.SetAutoCompleteEvent(event)

    def SetAttributeFocusEvent(self, event):
        self.applyComboBox.SetOnMouseLeftButtonDownEvent(event)

    def SetAttributeKillFocusEvent(self, event):
        self.applyComboBox.SetKillFocusEvent(event)

    def SetAttributeSelectEvent(self, event):
        self.applyComboBox.SetEvent(event)

    def SetAttributeCancelEvent(self, event):
        self.applyButton.SetEvent(event)

    ## Utility
    def DisableComboBox(self):
        self.applyComboBox.Disable()

    def EnableComboBox(self):
        self.applyComboBox.Enable()

    def SetConfigured(self, v):
        self.bonusSelected = v

    def IsConfigured(self):
        return self.bonusSelected

    ## Data getters
    def GetIndex(self):
        return self.applyIndex

    ## Internal events (can be overwritten)
    def OnCancelButtonPress(self):
        self.applyComboBox.SelectItem(0)
        self.applyComboBox.Enable()
        self.applyValueInput.ClearInput()
        if self.eventCancel:
            self.eventCancel()

    def OnAcceptButtonPress(self):
        if self.eventAccept:
            self.eventAccept()

    def MarkInvalidAttribute(self):
        self.applyComboBox.GetImageBox().SetDiffuseColor(1.0, 69 / 255.0, 69 / 255.0)

    def ResetMark(self):
        self.applyComboBox.GetImageBox().SetDiffuseColor(1.0, 1.0, 1.0)

    def SetAcceptEvent(self, event):
        self.eventAccept = MakeEvent(event)

    def SetCancelEvent(self, event):
        self.eventCancel = MakeEvent(event)


class SwitchbotSlot(ui.Window):
    def __init__(self, index):
        ui.Window.__init__(self)
        self.SetWindowName("SwitchbotSlot_{}".format(index))
        self.slotIndex = index

        self.selectedAttributes = [
            [0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0],
        ]
        self.tabs = [ui.AutoGrowingVerticalContainer(), ui.AutoGrowingVerticalContainer(), ui.AutoGrowingVerticalContainer()]
        self.currentTabIdx = 0

    def Build(self):

        for i in xrange(3):
            self.tabs[i].SetWindowName("SwitchBotTab_{}".format(i))
            self.tabs[i].SetParent(self)
            self.tabs[i].SetWidth(301)
            for j in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                attrInputRow = AttrInputRow(self.tabs[i], j)
                attrInputRow.SetParent(self.tabs[i])
                attrInputRow.SetAttributeAutoCompleteEvent(Event(self.OnChangeApplyName, j))
                attrInputRow.SetAttributeSelectEvent(Event(self.OnSelectApply, j))
                attrInputRow.SetAttributeFocusEvent(Event(self.OnApplyFocus, j))
                attrInputRow.SetAttributeKillFocusEvent(Event(self.OnApplyKillFocus, j))
                attrInputRow.SetAcceptEvent(Event(self.OnAcceptButtonClick, i, j))
                attrInputRow.SetCancelEvent(Event(self.OnCancelButtonClick, i, j))

                attrInputRow.SetPosition(0, (25 * j))
                attrInputRow.SetSize(260, 25)

                attrInputRow.UpdateRect()
                attrInputRow.Show()

                self.tabs[i].AppendItem(attrInputRow)

            self.tabs[i].Hide()

    def CurrentTab(self):
        # type: () -> ui.AutoGrowingVerticalContainer
        return self.tabs[self.currentTabIdx]

    def GetAttributeRow(self, index):
        # type: (int) -> AttrInputRow
        return self.tabs[self.currentTabIdx].GetElementByIndex(index)

    def SetTabIndex(self, index):
        # type: (int) -> None
        self.tabs[self.currentTabIdx].Hide()
        self.currentTabIdx = index
        self.tabs[self.currentTabIdx].Show()

    def GetTabIndex(self):
        return self.currentTabIdx

    def GetItem(self):
        return player.GetItemIndex(player.SWITCHBOT, self.slotIndex)

    def GetItemType(self):
        return player.GetItemTypeBySlot(player.SWITCHBOT, self.slotIndex)

    def GetItemSubType(self):
        return player.GetItemSubTypeBySlot(player.SWITCHBOT, self.slotIndex)

    def HasItem(self):
        return player.GetItemIndex(player.SWITCHBOT, self.slotIndex) != 0

    # Returns the selected attributes for the current tab
    def SelectedAttributes(self):
        return self.selectedAttributes[self.currentTabIdx]

    def OnSelectApply(self, comboBoxIndex, index):
        self.selectedAttributes[self.currentTabIdx][comboBoxIndex] = index
        if comboBoxIndex < 5:
            self.tabs[self.currentTabIdx].GetElementByIndex(comboBoxIndex).GetValueInput().SetText(
                GetAttrMaxValue(self.GetItem(), index))
        else:
            self.tabs[self.currentTabIdx].GetElementByIndex(comboBoxIndex).GetValueInput().SetText(
                GetRareAttrMaxValue(self.GetItem(), index))

        self.tabs[self.currentTabIdx].GetElementByIndex(comboBoxIndex).SetConfigured(True)

    def OnApplyFocus(self, index):
        if not self.GetAttributeRow(index):
            return False
        comboBox = self.GetAttributeRow(index).GetComboBox()
        comboBox.SetCurrentItem("")
        self.selectedAttributes[self.currentTabIdx][index] = 0

        comboBox = self.PopulateApplyCombobox(index)
        if comboBox:
            comboBox.OpenListBox()

    def OnApplyKillFocus(self, index):
        if not self.GetAttributeRow(index):
            return False

        comboBox = self.GetAttributeRow(index).GetComboBox()
        comboBox.CloseListBox()

    def OnChangeApplyName(self, index):
        comboBox = self.PopulateApplyCombobox(index, True)
        if comboBox:
            comboBox.OpenListBox()
        return True

    def PopulateApplyCombobox(self, index, useInput=False):
        if not self.GetAttributeRow(index):
            return None

        comboBox = self.GetAttributeRow(index).GetComboBox()
        data = self.GetApplyAutoCompleteEntries(comboBox.GetInput() if useInput else "", index)
        if not data:
            comboBox.CloseListBox()
            return None

        comboBox.CloseListBox()
        comboBox.ClearItem()
        comboBox.InsertItem(0, "None")
        for item in data:
            try:
                comboBox.InsertItem(item[0], FormatApplyAsString(item[0], 0))
            except Exception:
                pass

        return comboBox

    def Refresh(self):
        switchbotSlotData = playerInst().GetSwitchbotSlotData(self.slotIndex)
        for i in xrange(3):
            for j in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                if switchbotSlotData:
                    attribute = switchbotSlotData.attr[i][j]
                    if attribute.type != 0 and attribute.value != 0:
                        self.tabs[i].GetElementByIndex(j).GetComboBox().SetCurrentItem(
                            FormatApplyAsString(attribute.type, 0))
                        self.tabs[i].GetElementByIndex(j).GetValueInput().SetText(str(attribute.value))
                        self.tabs[i].GetElementByIndex(j).Disable()
                        self.tabs[i].GetElementByIndex(j).SetConfigured(True)

                    else:
                        if not self.tabs[i].GetElementByIndex(j).IsConfigured():
                            self.tabs[i].GetElementByIndex(j).GetComboBox().SetCurrentItem("")
                            self.tabs[i].GetElementByIndex(j).GetValueInput().SetText("")
                            self.tabs[i].GetElementByIndex(j).Enable()
                            self.tabs[i].GetElementByIndex(j).EnableComboBox()


                if not self.IsValidAttributeByAttributeId(self.selectedAttributes[i][j], j):
                    self.tabs[i].GetElementByIndex(j).MarkInvalidAttribute()
                else:
                    self.tabs[i].GetElementByIndex(j).ResetMark()

    def GetAvailableAttributes(self, index, checkExisting=True):
        CreateAvailableDict()

        if index < 5:
            global AVAILABLE_AFFECT_DICT
            availableAttributes = AVAILABLE_AFFECT_DICT

            if self.HasItem():
                boniAvailByType = GetAvailableAttr(self.GetItem(), False)
                if boniAvailByType is not None:
                    if isinstance(boniAvailByType, list):
                        availableAttributes = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if
                                               k in boniAvailByType}
                    elif isinstance(boniAvailByType, dict):
                        boniAvailBySubType = boniAvailByType.get(self.GetItemSubType(), None)
                        if boniAvailByType is not None:
                            availableAttributes = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if
                                                   k in boniAvailBySubType}

            if checkExisting:
                normalAttr = self.selectedAttributes[self.currentTabIdx][0:5]
                availableAttributes = {k: v for k, v in availableAttributes.iteritems() if k not in normalAttr}

            return availableAttributes
        else:
            global AVAILABLE_RARE_AFFECT_DICT
            availableAttributes = AVAILABLE_RARE_AFFECT_DICT

            if checkExisting:
                normalAttr = self.selectedAttributes[self.currentTabIdx][-2:]
                availableRareAttributes = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if
                                           k in availableAttributes and k not in normalAttr}
            else:
                availableRareAttributes = {k: v for k, v in ItemToolTip.AFFECT_DICT.iteritems() if
                                           k in availableAttributes}

            return availableRareAttributes

    def IsValidAttributeByAttributeId(self, attrId, index):
        if attrId == 0:
            return True

        return attrId in self.GetAvailableAttributes(index, checkExisting=False)

    def GetApplyAutoCompleteEntries(self, textInput, index):
        availableAttributes = self.GetAvailableAttributes(index)

        autoComplete = []
        for idx, name in availableAttributes.iteritems():
            if textInput in name or textInput == "":
                autoComplete.append((idx, name))

        return autoComplete

    def OnAcceptButtonClick(self, altIndex, attrIndex):
        applyType = self.selectedAttributes[altIndex][attrIndex]
        applyValue = float(self.tabs[altIndex].GetElementByIndex(attrIndex).GetValueInput().GetText())
        appInst.instance().GetNet().SendUpdateSwitchbotAttributePacket(self.slotIndex, altIndex, attrIndex,
                                                                       ItemApply(applyType, applyValue))

    def OnCancelButtonClick(self, altIndex, attrIndex):
        appInst.instance().GetNet().SendUpdateSwitchbotAttributePacket(self.slotIndex, altIndex, attrIndex,
                                                                       ItemApply(0, 0.0))
        self.tabs[altIndex].GetElementByIndex(attrIndex).SetConfigured(False)


class SwitchSlotIndicators(object):
    STATUS_INACTIVE = 0
    STATUS_ACTIVE = 1
    STATUS_DONE = 2

    STATUS_INDICATION_IMAGES = [
        "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
        "d:/ymir work/ui/gui/switchbot_icon_active.sub",
        "d:/ymir work/ui/gui/switchbot_icon_done.sub",
    ]

    def __init__(self):
        self.statusIndicators = []
        self.slotMarkers = []
        self.currentSlotIndex = 0

    def GetMarkers(self):
        return self.slotMarkers

    def GetMarker(self, index):
        return self.slotMarkers[index]

    def GetIndicators(self):
        return self.statusIndicators

    def GetSelectedSlotIndex(self):
        return self.currentSlotIndex

    def SetSlotIndex(self, index):
        self.OnBeforeSlotIndexChange()
        self.currentSlotIndex = index
        self.OnAfterSlotIndexChange()

    def SetSlotStatus(self, index, status):
        self.statusIndicators[index].LoadImage(self.STATUS_INDICATION_IMAGES[status])

    def SetIndictorClickEvent(self, index, event):
        self.statusIndicators[index].SetClickEvent(event)

    # Build
    def AddMarker(self, marker):
        self.slotMarkers.append(marker)

    def AddIndicator(self, indicator):
        self.statusIndicators.append(indicator)

    def RefreshAfterBuild(self):
        for marker in self.slotMarkers:
            marker.Hide()

        self.slotMarkers[self.currentSlotIndex].Show()

    def Refresh(self):
        for i in xrange(SWITCHBOT_SLOT_COUNT):
            switchbotSlotData = playerInst().GetSwitchbotSlotData(i)
            if switchbotSlotData:
                self.SetSlotStatus(i, switchbotSlotData.status)

    # Events
    def OnBeforeSlotIndexChange(self):
        self.slotMarkers[self.currentSlotIndex].Hide()

    def OnAfterSlotIndexChange(self):
        self.slotMarkers[self.currentSlotIndex].Show()


class SwitchbotWindow(ui.ScriptWindow):

    def __init__(self, wndInventory):
        if not wndInventory:
            logging.exception("wndInventory parameter must be set to InventoryWindow")
            return

        ui.ScriptWindow.__init__(self, "UI")
        self.AddFlag("animated_board")

        self.indicators = SwitchSlotIndicators()


        self.isLoaded = 0
        self.wndInventory = proxy(wndInventory)
        self.wndSwitchbotSlot = None
        self.switchbotDataSlots = [] * SWITCHBOT_SLOT_COUNT
        self.currentSlotIndex = 0

    def Show(self):
        self.__LoadWindow()

        self.RefreshSlot()

        ui.ScriptWindow.Show(self)

    def Close(self):


        self.Hide()

    def GetBasePosition(self):
        x, y = self.wndInventory.GetGlobalPosition()
        return x - 148, y + 241

    def AdjustPositionAndSize(self):
        x, y = self.GetBasePosition()

        self.SetPosition(x + 138, y)
        self.SetSize(10, self.GetHeight())

    def __LoadWindow(self):
        if self.isLoaded == 1:
            return

        self.isLoaded = 1

        GetObject = self.GetChild

        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/SwitchbotWindow.py")
        GetObject("Board").SetCloseEvent(self.Close)

        self.ORIGINAL_WIDTH = self.GetWidth()
        self.switchbotBase = GetObject("SwitchbotBaseImage")
        self.switchbotStatusButton = GetObject("startbtn")
        self.switchbotStatusButton.SetText("START")
        self.switchbotStatusButton.SetFontName("Verdana:12")
        # self.switchbotStatusButton.SetPackedFontColor(0xff000000)
        self.switchbotStatusButton.SetEvent(self.OnSwitchbotStatusButtonClick)
        self.switchbotStatusButton.Show()

        self.tabElements = []
        self.tabElements = [
            GetObject("Bonus_Tab_01"),
            GetObject("Bonus_Tab_02"),
            GetObject("Bonus_Tab_03"),
        ]

        self.tabElements[0].SetEvent(Event(self.SetSwitchbotTab, 0))
        self.tabElements[1].SetEvent(Event(self.SetSwitchbotTab, 1))
        self.tabElements[2].SetEvent(Event(self.SetSwitchbotTab, 2))
        self.tabElements[0].Down()

        # Switchbot Slots
        wndSwitchbotSlot = GetObject("SwitchbotSlot")
        wndSwitchbotSlot.SetSelectEmptySlotEvent(Event(self.wndInventory.SelectEmptySlot, player.SWITCHBOT))
        wndSwitchbotSlot.SetSelectItemSlotEvent(Event(self.wndInventory.SelectItemSlot, player.SWITCHBOT))
        wndSwitchbotSlot.SetUnselectItemSlotEvent(Event(self.wndInventory.UseItemSlot, player.SWITCHBOT))
        wndSwitchbotSlot.SetUseSlotEvent(Event(self.wndInventory.UseItemSlot, player.SWITCHBOT))
        wndSwitchbotSlot.SetOverInItemEvent(Event(self.ShowToolTip, player.SWITCHBOT))
        wndSwitchbotSlot.SetOverOutItemEvent(self.HideToolTip)

        self.wndSwitchbotSlot = wndSwitchbotSlot

        for i in xrange(SWITCHBOT_SLOT_COUNT):
            self.indicators.AddIndicator(GetObject("SwitchbotSlotButton_{}".format(i)))
            self.indicators.AddMarker(GetObject("SwitchbotSlotButtonMarker_{}".format(i)))

        for i in xrange(SWITCHBOT_SLOT_COUNT):
            self.indicators.SetIndictorClickEvent(i, Event(self.SetSwitchbotSlot, i))

        self.indicators.RefreshAfterBuild()

        # Switchbot Slot Bonus Selectors
        for k in xrange(SWITCHBOT_SLOT_COUNT):
            s = SwitchbotSlot(k)
            s.SetParent(self)
            s.SetPosition(9, 115 + 135 + 25)
            s.SetSize(281, 183)
            s.Build()
            self.switchbotDataSlots.insert(k, s)

        self.SetSwitchbotSlot(0)

    def ShowToolTip(self, window, slotIndex):
        try:
            if self.wndInventory.tooltipItem:
                self.wndInventory.tooltipItem.SetInventoryItem(slotIndex, window, True)
        except Exception:
            pass

    def HideToolTip(self):
        try:
            if self.wndInventory.tooltipItem:
                self.wndInventory.tooltipItem.HideToolTip()
        except Exception:
            pass

    def OnSwitchbotStatusButtonClick(self):
        switchbotSlotData = playerInst().GetSwitchbotSlotData(self.currentSlotIndex)
        if switchbotSlotData:
            if switchbotSlotData.status == 0 or switchbotSlotData.status == 2:
                appInst.instance().GetNet().SendSwitchbotSlotActivatePacket(self.currentSlotIndex)
            else:
                appInst.instance().GetNet().SendSwitchbotSlotDeactivatePacket(self.currentSlotIndex)

    def SetSwitchbotTab(self, index):
        currentSwitchbotSlot = self.switchbotDataSlots[self.GetSelectedSlotIndex()]

        for tab in self.tabElements:
            tab.SetUp()
        currentSwitchbotSlot.SetTabIndex(index)
        self.tabElements[currentSwitchbotSlot.GetTabIndex()].Down()

    def SetSwitchbotSlot(self, index):
        self.GetSwitchbotSlot().Hide()
        self.indicators.SetSlotIndex(index)
        self.currentSlotIndex = index
        self.GetSwitchbotSlot().Show()
        self.SetSwitchbotTab(0)

        self.RefreshSwitchbotData()

    def GetSwitchbotSlot(self, index=-1):
        if index == -1:
            return self.switchbotDataSlots[self.GetSelectedSlotIndex()]
        else:
            return self.switchbotDataSlots[index]

    def GetSelectedSlotIndex(self):
        return self.indicators.GetSelectedSlotIndex()


    def RefreshSlot(self):
        if not self.isLoaded:
            return

        getItemVNum = player.GetItemIndex
        getItemCount = player.GetItemCount
        setItemVNum = self.wndSwitchbotSlot.SetItemSlot
        for i in xrange(6):
            slotNumber = i
            itemCount = getItemCount(player.SWITCHBOT, slotNumber)
            if itemCount <= 1:
                itemCount = 0
            setItemVNum(slotNumber, getItemVNum(player.SWITCHBOT, slotNumber), itemCount)

    def ShowStopButton(self):
        self.switchbotStatusButton.SetUpVisual("d:/ymir work/ui/gui/fix/switchbot/buton_normal.png")
        self.switchbotStatusButton.SetOverVisual("d:/ymir work/ui/gui/fix/switchbot/buton_uzerine.png")
        self.switchbotStatusButton.SetDownVisual("d:/ymir work/ui/gui/fix/switchbot/buton_basıldı.png")
        self.switchbotStatusButton.SetText("Durdur")
        self.switchbotStatusButton.SetFontName("Nunito Sans:15b")
    def ShowStartButton(self):
        self.switchbotStatusButton.SetUpVisual("d:/ymir work/ui/gui/fix/switchbot/buton_normal.png")
        self.switchbotStatusButton.SetOverVisual("d:/ymir work/ui/gui/fix/switchbot/buton_uzerine.png")
        self.switchbotStatusButton.SetDownVisual("d:/ymir work/ui/gui/fix/switchbot/buton_basıldı.png")
        self.switchbotStatusButton.SetText("Başlat")
        self.switchbotStatusButton.SetFontName("Nunito Sans:15b")

    def RefreshSwitchbotData(self):
        if not self.isLoaded:
            return
        for slot in self.switchbotDataSlots:
            slot.Refresh()

        self.indicators.Refresh()

        switchbotSlotData = playerInst().GetSwitchbotSlotData(self.currentSlotIndex)
        if switchbotSlotData:
            if switchbotSlotData.status == 1:
                self.ShowStopButton()
            else:
                self.ShowStartButton()
