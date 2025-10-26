# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging
import math
from _weakref import proxy

import app
import background
import item
import player
import skill
from pygame.app import appInst
from pygame.item import itemManager
from pygame.npc import npcManager
from pygame.privateShopSearch import shopSearchInst, ShopSearchFilter

import emoji_config
import localeInfo
import ui
import uiToolTip
import uiScriptLocale
from ui_event import Event
from ui_modern_controls import ToggleAbleTitledWindow


class PrivateShopSearchSettingsDialog(ui.ScriptWindow):
    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")
        self.minLevel = None
        self.maxLevel = None

        self.minAvgDmg = None
        self.maxAvgDmg = None

        self.minSkillDmg = None
        self.maxSkillDmg = None

        self.saveBtn = None
        self.LoadWindow()

    def LoadWindow(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PrivateShopSearchSettingsDialog.py")

        GetObject = self.GetChild
        self.board = GetObject("board")

        self.minLevel = GetObject("minLevelValue")
        self.maxLevel = GetObject("maxLevelValue")

        self.minAvgDmg = GetObject("minAvgDmgValue")
        self.maxAvgDmg = GetObject("maxAvgDmgValue")

        self.minSkillDmg = GetObject("minSkillDmgValue")
        self.maxSkillDmg = GetObject("maxSkillDmgValue")

        self.saveBtn = GetObject("saveBtn")

        self.board.SetCloseEvent(self.Hide)

        self.saveBtn.SetEvent(self.SaveConfig)

    def SaveConfig(self):
        appInst().GetNet().SendShopSearchSettingsSavePacket()

    def Show(self):
        ui.ScriptWindow.Show(self)
        self.SetCenterPosition()
        self.SetTop()
        self.SetFocus()


class LocationTextLine(ui.TextLine):
    def __init__(self):
        super(LocationTextLine, self).__init__()

        self.tooltipInfo = ui.TextToolTip()
        self.tooltipInfo.Hide()

    def SetToolTipText(self, text):
        self.tooltipInfo.SetText(text)

    def OnMouseOverIn(self):
        self.tooltipInfo.Show()

    def OnMouseOverOut(self):
        self.tooltipInfo.Hide()


class PrivateShopSearchDialog(ui.ScriptWindow):
    class SearchResultItem(ui.ExpandedImageBox):

        def __init__(self):
            ui.ExpandedImageBox.__init__(self)
            self.tooltipItem = None
            startX = 13
            yPos = 3

            self.count = ui.TextLine()
            self.count.SetParent(self)
            self.count.SetVerticalAlignCenter()
            self.count.SetPosition(startX + 10, 0)
            self.count.Show()

            self.ItemSlotBase = ui.MakeImageBox(self, "", 70, 0)
            self.ItemSlotBase.SetParent(self)
            self.ItemSlotBase.SetVerticalAlignCenter()
            self.ItemSlotBase.SetOnMouseOverInEvent(self.__OverInItem)
            self.ItemSlotBase.SetOnMouseOverOutEvent(self.__OverOutItem)
            self.ItemSlotBase.Show()

            self.itemImage = ui.MakeImageBox(self.ItemSlotBase, "", 2, 0)
            self.itemImage.SetOnMouseOverInEvent(self.__OverInItem)
            self.itemImage.SetOnMouseOverOutEvent(self.__OverOutItem)
            self.itemImage.SetVerticalAlignCenter()
            self.itemImage.SetTop()
            self.itemImage.Show()

            self.whisperBtn = ui.MakeButton(
                self,
                startX + 270 - 27,
                5,
                "Whisper",
                "",
                "d:/ymir work/ui/game/shopsearch/message_icon.tga",
                "d:/ymir work/ui/game/shopsearch/message_icon_over.tga",
                "d:/ymir work/ui/game/shopsearch/message_icon.tga",
            )
            self.whisperBtn.SetVerticalAlignCenter()
            self.whisperBtn.SetEvent(self.WhisperPlayer)
            self.whisperBtn.Show()

            self.seller = ui.LimitedTextLine()
            self.seller.SetParent(self)
            self.seller.SetVerticalAlignCenter()
            self.seller.SetPosition(startX + 270, 0)
            self.seller.SetMax(9)
            self.seller.Show()

            self.price = ui.TextLine()
            self.price.SetParent(self)
            self.price.SetVerticalAlignCenter()
            self.price.SetPosition(startX + 100, 0)
            self.price.Show()

            self.buyButton = ui.MakeButton(
                self,
                startX + 350,
                5,
                "Buy",
                "d:/ymir work/ui/game/shopsearch/",
                "btn.png",
                "btn2.png",
                "btn3.png",
            )
            self.buyButton.SetVerticalAlignCenter()
            self.buyButton.SetEvent(self.BuyItem)
            self.buyButton.SetText(uiScriptLocale.PRIVATESHOPSEARCH_BUY)
            self.buyButton.Show()

            self.alarmImage = LocationTextLine()
            self.alarmImage.SetParent(self)
            self.alarmImage.RemoveFlag("not_pick")
            self.alarmImage.SetVerticalAlignCenter()
            self.alarmImage.SetPosition(startX + 420, 5)
            self.alarmImage.SetFontName("Font Awesome 5 Free:14")
            # self.alarmImage.SetText("Unknown")
            self.alarmImage.Show()

            self.isLoad = True

            self.data = None
            self.proto = None

            self.itemID = 0
            self.shopVid = 0
            self.shopOwner = ""
            self.metinSlot = []
            self.attrSlot = []
            self.effectID = 0
            self.interface = None

            self.SetSize(560, 32 + 27)

        def SetItemToolTip(self, tooltipItem):
            self.tooltipItem = tooltipItem

        def BindInterface(self, interface):
            self.interface = interface

        def Aquire(self, data):
            self.data = proxy(data)
            self.proto = itemManager().GetProto(data.data.info.vnum)

            self.itemID = data.data.info.id
            self.shopVid = data.ownerVid
            self.shopOwner = data.ownerName
            self.metinSlot = data.data.info.sockets
            self.attrSlot = data.data.info.attrs
            self.effectID = 0

            if self.proto:
                self.ItemSlotBase.LoadImage("d:/ymir work/ui/game/shopsearch/slot_32x%d.tga" % (self.proto.GetSize() * 32))
                self.itemImage.LoadImage(self.proto.GetIconImageFileName())

            self.SetPrice(
                emoji_config.ReplaceEmoticons(
                    "(gold){}".format(localeInfo.DottedNumber(data.data.llPrice))
                )
            )
            self.SetSeller(self.shopOwner)
            self.SetCount("{} x".format(data.data.info.count))
            self.alarmImage.SetText("{}, CH{}".format(
                    localeInfo.GetMapNameByIndex(data.mapIndex), data.channel
                )[:12] + "...")

            self.alarmImage.SetToolTipText(
                "{}, CH{}".format(
                    localeInfo.GetMapNameByIndex(data.mapIndex), data.channel
                )
            )
            self.LoadImage("d:/ymir work/ui/game/shopsearch/field_{}_black.png".format(self.proto.GetSize()))
            self.SetSize(560, 32 * self.proto.GetSize() + 27)
            self.Show()
            return self

        def Release(self):
            self.itemImage.LoadImage("")
            self.SetPrice("")
            self.data = None
            self.proto = None
            self.itemID = 0
            self.shopVid = 0
            self.shopOwner = ""
            self.metinSlot = []
            self.attrSlot = []
            self.effectID = 0
            self.Hide()

        def __OverOutItem(self):
            if None != self.tooltipItem:
                self.tooltipItem.HideToolTip()
                return False

            return True

        def __OverInItem(self):
            if self.tooltipItem is None:
                return False

            self.tooltipItem.ClearToolTip()
            itemData = self.data.data.info
            if not itemData:
                return False

            attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

            self.tooltipItem.AddItemData(
                itemData.vnum,
                itemData.sockets,
                attrSlot,
                player.INVENTORY,
                -1,
                sealDate=itemData.sealDate,
                changeLookVnum=itemData.transVnum,
                shopPrice=self.data.data.llPrice,
            )

            return True

        def WhisperPlayer(self):
            if self.interface:
                self.interface.OpenWhisperDialog(self.shopOwner)

        def BuyItem(self):
            appInst.instance().GetNet().SendPrivateShopSearchBuyItem(self.itemID)

        def SetCount(self, count):
            self.count.SetText(count)

        def SetPrice(self, price):
            self.price.SetText(price)

        def SetSeller(self, seller):
            self.seller.SetText(seller)

        def GetSeller(self):
            return self.seller.GetText()

        def SetMetinSlot(self, metinSlot):
            self.metinSlot = metinSlot

        def SetAttrSlot(self, attrSLot):
            self.attrSlot = attrSLot

        def SetShopVid(self, vid):
            self.shopVid = vid

        def GetShopVid(self):
            return self.shopVid

        def SetItemVnum(self, vnum):
            self.itemVnum = vnum

        def GetShopItemPos(self):
            return self.shopItemPos

        def SetShopItemPos(self, itemPos):
            self.shopItemPos = itemPos

    def __init__(self):
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.selectedItemIndex = -1
        self.board = None
        self.minLevel = None
        self.maxLevel = None
        self.minRefine = None
        self.maxRefine = None
        self.itemNameSearch = None
        self.currentItemCat = item.ITEM_TYPE_WEAPON
        self.currentSubItemCat = item.WEAPON_SWORD
        self.currentJob = 0
        self.itemDataList = []
        self.Categories = []
        self.pageButtons = []

        self.currentPage = 1
        self.pageCount = 1
        self.perPage = 15
        self.itemCount = 0
        self.selectedItemVnum = (-1, -1)
        self.tooltipItem = None

        self.interface = None
        self.shopSearchResults = []
        self.settingsDialog = PrivateShopSearchSettingsDialog()

        for i in xrange(25):
            self.shopSearchResults.append(PrivateShopSearchDialog.SearchResultItem())

        self.LoadWindow()

        # self.jobChoose.SelectItem(0)

    def SetItemToolTip(self, tooltipItem):
        self.tooltipItem = proxy(tooltipItem)

    def BindInterfaceClass(self, interface):
        self.interface = proxy(interface)

    def ToggleSettings(self):
        if self.settingsDialog.IsShow():
            self.settingsDialog.Hide()
        else:
            self.settingsDialog.Show()

    def LoadWindow(self):
        pyScrLoader = ui.PythonScriptLoader()
        pyScrLoader.LoadScriptFile(self, "UIScript/PrivateShopSearchDialog.py")

        GetObject = self.GetChild
        self.board = GetObject("board")

        self.pageButtons.append(GetObject("page1_button"))
        self.pageButtons.append(GetObject("page2_button"))
        self.pageButtons.append(GetObject("page3_button"))
        self.pageButtons.append(GetObject("page4_button"))
        self.pageButtons.append(GetObject("page5_button"))

        self.pageButtons[0].Show()
        self.pageButtons[1].Hide()
        self.pageButtons[2].Hide()
        self.pageButtons[3].Hide()
        self.pageButtons[4].Hide()
        self.pageButtons[0].Down()
        self.pageButtons[0].Disable()

        self.searchButton = GetObject("SearchButton")
        self.searchButton.SetEvent(self.StartSearch)

        self.buyButton = self.GetOptionalChild("ConfigButton")
        if self.buyButton:
            self.buyButton.SetEvent(self.ToggleSettings)

        self.nextButton = GetObject("next_button")
        self.lastButton = GetObject("last_next_button")
        self.prevButton = GetObject("prev_button")
        self.firstButton = GetObject("first_prev_button")

        self.nextButton.SetEvent(self.NextPage)
        self.prevButton.SetEvent(self.PrevPage)
        self.firstButton.SetEvent(self.FirstPage)
        self.lastButton.SetEvent(self.LastPage)

        self.board.SetCloseEvent(self.__OnCloseButtonClick)

        self.resultScrollWindow = self.GetChild("ResultItemsScroll")
        self.resultItemContainer = self.GetChild("ResultItemsContent")

        try:
            self.Categories = []

            self.Categories.append(
                [["Alle", "d:/ymir work/ui/game/shopsearch/miniicons/all.png"], []]
            )
            self.Categories.append(
                [
                    [
                        localeInfo.PRIVATESHOPSEARCH_WEAPON,
                        "d:/ymir work/ui/game/shopsearch/miniicons/weapon.png",
                    ],
                    [
                        [
                            localeInfo.PRIVATESHOPSEARCH_SWORD,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_SWORD),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_TWOSWORD,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_TWO_HANDED),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DAGGER,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_DAGGER),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_BOW,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_BOW),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_ARROW,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_ARROW),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_QUIVER,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_QUIVER),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_BELL,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_BELL),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_FAN,
                            (item.ITEM_TYPE_WEAPON, item.WEAPON_FAN),
                        ],
                    ],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.PRIVATESHOPSEARCH_ARMOR,
                        "d:/ymir work/ui/game/shopsearch/miniicons/armor.png",
                    ],
                    [
                        [
                            localeInfo.PRIVATESHOPSEARCH_BODY,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_BODY),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_HEAD,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_HEAD),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_SHIELD,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_SHIELD),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_WRIST,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_WRIST),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_FOOTS,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_FOOTS),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_NECK,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_NECK),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_EAR,
                            (item.ITEM_TYPE_ARMOR, item.ARMOR_EAR),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_BELT"),
                            (item.ITEM_TYPE_BELT, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_TALISMAN"),
                            (item.ITEM_TYPE_TALISMAN, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_RING"),
                            (item.ITEM_TYPE_RING, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_TOGGLE_AFFECT"),
                            (item.ITEM_TYPE_TOGGLE, item.TOGGLE_AFFECT),
                        ],
                    ],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.PRIVATESHOPSEARCH_COSTUME,
                        "d:/ymir work/ui/game/shopsearch/miniicons/costumes.png",
                    ],
                    [
                        [
                            localeInfo.PRIVATESHOPSEARCH_COSTUMEBODY,
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_BODY),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_COSTUMEHAIR,
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_HAIR),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_COSTUME_ACCE,
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_ACCE),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_COSTUME_WEAPON,
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_WEAPON),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_COSTUME_WEAPON_EFFECT"),
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_WEAPON_EFFECT),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_COSTUME_ARMOR_EFFECT"),
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_BODY_EFFECT),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_COSTUME_WING"),
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_WING_EFFECT),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_COSTUME_TITLE"),
                            (item.ITEM_TYPE_COSTUME, item.COSTUME_TYPE_RANK),
                        ],
                    ],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.Get("PRIVATESHOPSEARCH_PETS_N_MOUNT"),
                        "d:/ymir work/ui/game/shopsearch/miniicons/pets.png",
                    ],
                    [
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_LEVEL_PET"),
                            (item.ITEM_TYPE_TOGGLE, item.TOGGLE_LEVEL_PET),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_LEVEL_PET_FOOD"),
                            (item.ITEM_TYPE_USE, item.USE_LEVEL_PET_FOOD),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_NORMAL_PET"),
                            (item.ITEM_TYPE_TOGGLE, item.TOGGLE_PET),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_MOUNT"),
                            (item.ITEM_TYPE_TOGGLE, item.TOGGLE_MOUNT),
                        ],
                    ],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.PRIVATESHOPSEARCH_DS,
                        "d:/ymir work/ui/game/shopsearch/miniicons/dragon.png",
                    ],
                    [
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_WHITE,
                            (item.ITEM_TYPE_DS, item.DS_WHITE),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_RED,
                            (item.ITEM_TYPE_DS, item.DS_RED),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_GREEN,
                            (item.ITEM_TYPE_DS, item.DS_GREEN),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_BLUE,
                            (item.ITEM_TYPE_DS, item.DS_BLUE),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_YELLOW,
                            (item.ITEM_TYPE_DS, item.DS_YELLOW),
                        ],
                        [
                            localeInfo.PRIVATESHOPSEARCH_DS_BLACK,
                            (item.ITEM_TYPE_DS, item.DS_BLACK),
                        ],
                    ],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.Get("PRIVATESHOPSEARCH_GIFTBOX"),
                        "d:/ymir work/ui/game/shopsearch/miniicons/items.png",
                    ],
                    [],
                ]
            )

            self.Categories.append(
                [
                    [
                        localeInfo.Get("PRIVATESHOPSEARCH_USEFUL_ITEMS"),
                        "d:/ymir work/ui/game/shopsearch/miniicons/consumables.png",
                    ],
                    [
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_QUEST_ITEMS"),
                            (item.ITEM_TYPE_QUEST, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_USABLE_ITEMS"),
                            (item.ITEM_TYPE_USE, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_TIME_ENHANCEMENT"),
                            (item.ITEM_TYPE_USE, item.USE_ENHANCE_TIME),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_SKILLBOOKS"),
                            (item.ITEM_TYPE_SKILLBOOK, -1),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_ACESSORY"),
                            (item.ITEM_TYPE_USE, item.USE_PUT_INTO_ACCESSORY_SOCKET),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_FISHING_BAIT"),
                            (item.ITEM_TYPE_USE, item.USE_BAIT),
                        ],
                        [
                            localeInfo.Get("PRIVATESHOPSEARCH_FISH"),
                            (item.ITEM_TYPE_FISH, -1),
                        ],
                    ],
                ]
            )

            self.itemChooseByName = ui.ComboBoxAutoComplete(
                self, "d:/ymir work/ui/game/shopsearch/suchfeld.png", 20, 58
            )
            self.itemChooseByName.InsertItem(0, "-")
            self.itemChooseByName.SetCurrentItem("")

            self.itemChooseByName.SetAutoCompleteEvent(self.OnChangeItemName)
            self.itemChooseByName.SetEvent(self.OnSelectItemName)

            self.itemChooseByName.Show()

            self.scrollWindow = ui.ScrollWindow()
            self.scrollWindow.SetSize(188, 410)
            self.scrollWindow.SetPosition(0, 0)
            self.scrollWindow.SetHorizontalAlignLeft()
            self.scrollWindow.SetVerticalAlignTop()

            from ui_modern_controls import AutoGrowingVerticalContainerEx

            self.categoryContainer = AutoGrowingVerticalContainerEx()
            self.categoryContainer.SetWidth(178)
            for idx, cat in enumerate(self.Categories):
                toggleableWindow = ToggleAbleTitledWindow(
                    "d:/ymir work/ui/game/shopsearch/sobutton.png", cat[0][1]
                )
                toggleableWindow.SetParent(self.categoryContainer)
                toggleableWindow.SetWidth(178)
                toggleableWindow.SetPosition(0, 0)
                toggleableWindow.SetTitle(cat[0][0])
                toggleableWindow.SetOnToggleEvent(Event(self.OnToggleWindowToggle, idx))
                toggleableWindow.SetCloseEvent(Event(self.OnToggleWindowClose, idx))
                toggleableWindow.SetOpenEvent(Event(self.OnToggleWindowOpen, idx))
                toggleableWindow.SetToggleHideFilename(
                    "d:/ymir work/ui/game/windows/messenger_list_close.sub"
                )
                toggleableWindow.SetToggleShowFilename(
                    "d:/ymir work/ui/game/windows/messenger_list_open.sub"
                )
                self.categoryContainer.AppendItem(toggleableWindow)
                toggleableWindow.Close()
                toggleableWindow.Show()

            self.applies = []
            for idx, cat in enumerate(self.Categories):
                for info in cat[1]:
                    applyItem = ui.MakeButtonWithText(
                        self,
                        info[0],
                        0,
                        0,
                        "d:/ymir work/ui/game/shopsearch/",
                        "subcat_0.png",
                        "subcat_1.png",
                        "subcat_2.png",
                    )
                    applyItem.SetEvent(Event(self.ChooseCategory, info[1]))
                    imageWidth = applyItem.GetButtonImageWidth()
                    imageHeight = applyItem.GetButtonImageHeight()
                    applyItem.SetButtonScale(
                        180 * 1.0 / imageWidth, 24 * 1.0 / imageHeight
                    )
                    self.categoryContainer.GetElementByIndex(idx).AppendToToggleContent(
                        applyItem
                    )
                    self.applies.append(applyItem)

            self.categoryContainer.Show()

            self.categoryContainer.RecalculateHeight()

            self.scrollWindow.SetContentWindow(self.categoryContainer)
            self.scrollWindow.AutoFitWidth()
            self.scrollWindow.SetPosition(0, 0)

            self.scrollWindow.SetParent(self.board)
            self.scrollWindow.SetPosition(10, 96)
            self.scrollWindow.Show()
        except Exception as e:
            logging.exception(e)

        self.Children.append(self.itemChooseByName)

    def ChooseCategory(self, index):
        appInst.instance().GetNet().SendPrivateShopSearchOpenCategory(index)

    def OnToggleWindowClose(self, index):
        pass

    def OnToggleWindowOpen(self, index):
        appInst.instance().GetNet().SendPrivateShopSearchOpenFilter(index)

    def OnToggleWindowToggle(self, myIdx):

        for item in self.categoryContainer.GetItems():
            item.SetOpenCloseEventStatus(False)
            item.Close()
            item.SetOpenCloseEventStatus(True)

        self.categoryContainer.GetElementByIndex(myIdx).Open()
        self.categoryContainer.RecalculateHeight()

    def Destroy(self):
        self.ClearDictionary()
        self.resultItemContainer.ClearItems()
        self.titleBar = None

    def Open(self, type):
        self.RefreshMe()
        self.Show()
        self.SetCenterPosition()

        self.categoryContainer.GetElementByIndex(0).Open()

    def RefreshMe(self):
        background.DeletePrivateShopPos()
        self.itemDataList[:] = []
        self.resultItemContainer.ClearItems()

        self.itemCount = shopSearchInst().GetItemDataCount()

        for x in xrange(shopSearchInst().GetItemDataCount()):
            self.itemDataList.append(shopSearchInst().GetItemData(x))

        self.pageCount = shopSearchInst().GetPageInfo().pageCount
        self.paginationPageCount = shopSearchInst().GetPageInfo().pageCount
        self.currentPage = shopSearchInst().GetPageInfo().page
        self.RefreshList()

    def RefreshList(self):
        background.DeletePrivateShopPos()
        self.selectedItemIndex = -1
        self.RefreshPaginationButtons()
        for item in self.shopSearchResults:
            item.Release()
        self.resultItemContainer.ClearItems()
        self.resultItemContainer.SetWidth(560)

        currentPageDict = self.itemDataList

        basePos = 0
        for x, data in enumerate(currentPageDict):
            resultItem = self.shopSearchResults[x].Aquire(data)
            resultItem.SetParent(self.resultItemContainer)
            resultItem.SetPosition(0, basePos + 22)
            resultItem.SetItemToolTip(self.tooltipItem)
            resultItem.BindInterface(self.interface)
            self.resultItemContainer.AppendItem(resultItem)
            resultItem.Show()
            resultItem.SetTop()
            resultItem.UpdateRect()

            basePos += resultItem.GetHeight()

        self.resultItemContainer.UpdateRect()

    def RefreshPaginationButtons(self):
        self.currentPaginationPage = int(math.ceil(float(self.currentPage) / 5.0))
        self.shownPages = min(
            self.pageCount - (5 * (self.currentPaginationPage - 1)), 5
        )

        for x in xrange(5):
            currentPage = x + ((self.currentPaginationPage - 1) * 5) + 1
            self.pageButtons[x].SetUp()
            self.pageButtons[x].SetText("%d" % currentPage)
            self.pageButtons[x].SetEvent(Event(self.GotoPage, currentPage))

        map(ui.Button.Hide, self.pageButtons)
        map(ui.Button.Enable, self.pageButtons)

        for x in xrange(self.shownPages):
            self.pageButtons[x].Show()

        self.pageButtons[
            (self.currentPage - ((self.currentPaginationPage - 1) * 5)) - 1
        ].Down()
        self.pageButtons[
            (self.currentPage - ((self.currentPaginationPage - 1) * 5)) - 1
        ].Disable()

    def GotoPage(self, page):
        if page > shopSearchInst().GetPageInfo().pageCount or page < 1:
            return

        appInst.instance().GetNet().SendPrivateShopSearchOpenPage(page)

    def FirstPage(self):
        appInst.instance().GetNet().SendPrivateShopSearchOpenPage(1)

    def LastPage(self):
        appInst.instance().GetNet().SendPrivateShopSearchOpenPage(
            shopSearchInst().GetPageInfo().pageCount
        )

    def NextPage(self):
        page = shopSearchInst().GetPageInfo().page + 1
        if page > shopSearchInst().GetPageInfo().pageCount or page < 1:
            return
        appInst.instance().GetNet().SendPrivateShopSearchOpenPage(page)

    def PrevPage(self):
        page = shopSearchInst().GetPageInfo().page - 1
        if page > shopSearchInst().GetPageInfo().pageCount or page < 1:
            return
        appInst.instance().GetNet().SendPrivateShopSearchOpenPage(page)

    def RefreshRequest(self):
        self.StartSearch()
        self.RefreshList()

    def StartSearch(self):
        background.DeletePrivateShopPos()
        ItemVnum = self.selectedItemVnum

        appInst.instance().GetNet().SendPrivateShopSearchInfoSearch(ItemVnum)
        ##appInst.instance().GetNet().SendChatPacket("/shop_search %d %d %d %d %d %d %d %d %d" % (Race, ItemCat, SubCat, MinLevel, MaxLevel, MinRefine, MaxRefine, MinGold, MaxGold ))

    def BuySelectedItem(self):
        if self.selectedItemIndex == -1:
            return
        # shopSeller = self.resultItemContainer.GetElementByIndex(self.selectedItemIndex).GetSeller()
        # shopItemPos = self.resultItemContainer.GetElementByIndex(self.selectedItemIndex).GetShopItemPos()
        ##appInst.instance().GetNet().SendChatPacket("/shop_buy %d %d" % (shopVid, shopItemPos))
        # appInst.instance().GetNet().SendPrivateShopSearchBuyItem(shopSeller, shopItemPos)
        self.StartSearch()

    def OnSearchResultItemSelect(self, index):
        background.DeletePrivateShopPos()
        self.selectedItemIndex = index
        # background.CreatePrivateShopPos(shopVid)

    def OnChangeItemName(self):
        if self.itemChooseByName.GetInput() == "":
            self.selectedItemVnum = (-1, -1)
            return True

        import item

        data = item.GetAutoCompleEntries(self.itemChooseByName.GetInput())
        if not data:
            self.itemChooseByName.CloseListBox()
            return False

        mobNames = {}
        mobs = itemManager().GetLevelPetMobs()
        for mob in mobs:
            proto = npcManager().GetMonsterName(mob[1])
            if proto:
                mobNames[proto] = mob

        mobNames = {
            key: value
            for key, value in mobNames.items()
            if self.itemChooseByName.GetInput() in key
        }

        endResult = {}
        for key, value in mobNames.items():
            p = itemManager().GetProto(value[0])
            if p:
                endResult["{} {}".format(key, p.GetName())] = value
        self.itemChooseByName.ClearItem()
        self.itemChooseByName.InsertItem(0, "/")
        for item in data:
            try:
                self.itemChooseByName.InsertItem((item[0], 0), str(item[1]))
            except:
                pass

        for key, value in endResult.items():
            try:
                self.itemChooseByName.InsertItem(value, key)
            except:
                pass

        self.itemChooseByName.OpenListBox()
        return True

    # TODO: Get rid of this hack and do it properly
    def OnOpenItemChooseListBox(self):
        self.itemChooseByName.Hide()
        self.minLevel.Hide()
        self.maxLevel.Hide()
        self.minRefine.Hide()
        self.maxRefine.Hide()
        # self.levelImg.Hide()
        # self.refineImg.Hide()

    def OnCloseItemChooseListBox(self):
        self.itemChooseByName.Show()
        self.minLevel.Hide()
        self.maxLevel.Hide()
        self.minRefine.Hide()
        self.maxRefine.Hide()
        # self.levelImg.Show()
        # self.refineImg.Show()

    def OnSelectItemName(self, index):
        self.selectedItemVnum = index
        return True

    def Close(self):
        background.DeletePrivateShopPos()
        self.Hide()
        self.settingsDialog.Hide()

    def SetItemToolTip(self, itemTooltip):
        self.tooltipItem = itemTooltip
        self.tooltipItem.BindInterface(self.interface)

    def __ShowToolTip(self, slotIndex):
        if self.tooltipItem:
            self.tooltipItem.SetAcceWindowItem(slotIndex)

    def OnTop(self):
        if self.itemChooseByName:
            self.itemChooseByName.CloseListBox()

    def OverInItem(self, slotIndex):
        slotIndex = slotIndex
        self.__ShowToolTip(slotIndex)

    def OverOutItem(self):
        if self.tooltipItem:
            self.tooltipItem.HideToolTip()

    def Clear(self):
        self.Refresh()

    def Refresh(self):
        pass

    def __OnCloseButtonClick(self):
        self.Close()

    def OnKeyDown(self, key):
        if key == app.VK_ESCAPE:
            self.Close()
            return True
