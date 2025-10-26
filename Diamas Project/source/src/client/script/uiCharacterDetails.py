# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import logging

import app
import item
import wndMgr
from pygame.player import playerInst

import localeInfo
import ui
import uiToolTip
from ui_event import MakeEvent
from ui_modern_controls import ToggleAbleTitledWindow


class ApplyItem(ui.Window):
    def __init__(self):
        ui.Window.__init__(self)
        self.SetSize(200, 22)
        self.pointType = 0
        self.applyTooltipText = ""
        self.applyOverEvent = None
        self.applyOutEvent = None

        self.applyName = ui.MakeButton(self, 0, 0, "", "", "", "", "")
        self.applyName.SetSize(160, 17)

        self.applyValue = ui.MakeButton(self, 0, 0, "", "", "", "", "")
        self.applyValue.SetHorizontalAlignRight()
        self.applyValue.SetSize(40, 17)

        self.applyValue.SetText("0")

    def SetApplyName(self, name):
        self.applyName.SetTextAlignLeft(name)

    def SetApplyTooltipText(self, name):
        self.applyTooltipText = name

    def SetApplyValue(self, val):
        self.pointType = int(val)

    def SetApplyOverEvent(self, event):
        self.applyOverEvent = MakeEvent(event)

    def SetApplyOverOutEvent(self, event):
        self.applyOutEvent = MakeEvent(event)

    def OnMouseOverIn(self):
        if self.applyOverEvent:
            self.applyOverEvent(self.applyTooltipText)

    def OnMouseOverOut(self):
        if self.applyOutEvent:
            self.applyOutEvent()

    def Refresh(self):
        self.applyValue.SetText(
            localeInfo.NumberToString(playerInst().GetPoint(self.pointType))
        )


class CharacterDetailsUI(ui.ScriptWindow):
    def __init__(self, parent):
        self.uiCharacterStatus = parent
        ui.ScriptWindow.__init__(self)
        self.AddFlag("animated_board")

        self.toolTip = uiToolTip.ToolTip()
        self.Categories = []
        self.AttackDefenceInfo = []
        self.applies = []

        self.__LoadScript()

    def __LoadScript(self):
        try:
            pyScrLoader = ui.PythonScriptLoader()
            pyScrLoader.LoadScriptFile(self, "UIScript/CharacterDetailsWindow.py")
        except:
            logging.exception("CharacterDetailsWindow.__LoadScript.LoadObject")
            return

        try:
            self.Width = 250
            getChild = self.GetChild
            getChild("TitleBar").CloseButtonHide()
            self.board = getChild("MainBoard")

            self.__Initialize()
        except:
            logging.exception("CharacterDetailsWindow.__LoadScript.BindObject")

    def __Initialize(self):
        self.tooltipInfo = uiToolTip.ItemToolTip(100)
        self.tooltipInfo.CheckUsability(False)
        self.tooltipInfo.HideToolTip()

        self.InitAttackDefenceInfo()
        self.InitElementalInfo()
        self.InitClassInfo()
        self.InitWeaponInfo()
        self.InitMiscInfo()
        self.InitSpecialInfo()

        self.scrollWindow = ui.ScrollWindow()
        self.scrollWindow.SetSize(200, 310)
        self.scrollWindow.SetPosition(0, 0)
        self.scrollWindow.SetHorizontalAlignLeft()
        self.scrollWindow.SetVerticalAlignTop()

        from ui_modern_controls import AutoGrowingVerticalContainerEx

        self.autoGrowingStuff = AutoGrowingVerticalContainerEx()
        self.autoGrowingStuff.SetWidth(200)

        for cat in self.Categories:
            toggleableWindow = ToggleAbleTitledWindow()
            toggleableWindow.SetParent(self.autoGrowingStuff)
            toggleableWindow.SetWidth(200)
            toggleableWindow.SetPosition(0, 0)
            toggleableWindow.SetTitle(cat[0])
            toggleableWindow.SetOnToggleEvent(self.OnToggleWindowToggle)
            toggleableWindow.SetToggleHideFilename(
                "d:/ymir work/ui/game/windows/messenger_list_close.sub"
            )
            toggleableWindow.SetToggleShowFilename(
                "d:/ymir work/ui/game/windows/messenger_list_open.sub"
            )
            self.autoGrowingStuff.AppendItem(toggleableWindow)
            toggleableWindow.Show()

        self.applies = []
        for idx, cat in enumerate(self.Categories):
            for info in cat[1]:
                applyItem = ApplyItem()
                applyItem.SetApplyName(info[0])
                applyItem.SetApplyTooltipText(info[1])
                applyItem.SetApplyValue(str(info[2]))
                applyItem.SetApplyOverEvent(self.OnApplyOverIn)
                applyItem.SetApplyOverOutEvent(self.OnApplyOverOut)
                self.autoGrowingStuff.GetElementByIndex(idx).AppendToToggleContent(
                    applyItem
                )
                self.applies.append(applyItem)

        self.autoGrowingStuff.Show()

        self.autoGrowingStuff.RecalculateHeight()

        self.scrollWindow.SetContentWindow(self.autoGrowingStuff)
        self.scrollWindow.AutoFitWidth()
        self.scrollWindow.SetPosition(0, 0)

        self.scrollWindow.SetParent(self.board)
        self.scrollWindow.SetPosition(20, 50)
        self.scrollWindow.Show()

        self.Children.append(self.scrollWindow)
        self.RefreshLabel()

    def OnApplyOverIn(self, text):
        self.tooltipInfo.ClearToolTip()
        self.tooltipInfo.AppendTextLine(text)
        self.tooltipInfo.ShowToolTip()

    def OnApplyOverOut(self):
        self.tooltipInfo.HideToolTip()

    def OnToggleWindowToggle(self):
        self.autoGrowingStuff.RecalculateHeight()

    def RefreshLabel(self):
        for apply in self.applies:
            apply.Refresh()

    def InitSpecialInfo(self):
        self.SpecialBonusInfo = []
        self.SpecialBonusInfo.append(
            [
                localeInfo.DETAILS_73,
                localeInfo.DETAILS_TOOLTIP_73,
                item.GetApplyPoint(item.APPLY_MALL_EXPBONUS),
            ]
        )
        self.SpecialBonusInfo.append(
            [
                localeInfo.DETAILS_74,
                localeInfo.DETAILS_TOOLTIP_74,
                item.GetApplyPoint(item.APPLY_MALL_ITEMBONUS),
            ]
        )
        self.SpecialBonusInfo.append(
            [
                localeInfo.DETAILS_75,
                localeInfo.DETAILS_TOOLTIP_75,
                item.GetApplyPoint(item.APPLY_MALL_GOLDBONUS),
            ]
        )
        self.Categories.append(["Diğer Bonuslar", self.SpecialBonusInfo])

    def InitMiscInfo(self):
        self.MiscBonusInfo = []
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_53,
                localeInfo.DETAILS_TOOLTIP_53,
                item.GetApplyPoint(item.APPLY_STUN_PCT),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_54,
                localeInfo.DETAILS_TOOLTIP_54,
                item.GetApplyPoint(item.APPLY_SLOW_PCT),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_55,
                localeInfo.DETAILS_TOOLTIP_55,
                item.GetApplyPoint(item.APPLY_POISON_PCT),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_56,
                localeInfo.DETAILS_TOOLTIP_56,
                item.GetApplyPoint(item.APPLY_POISON_REDUCE),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_57,
                localeInfo.DETAILS_TOOLTIP_57,
                item.GetApplyPoint(item.APPLY_BLEEDING_PCT),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_58,
                localeInfo.DETAILS_TOOLTIP_58,
                item.GetApplyPoint(item.APPLY_BLEEDING_REDUCE),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_59,
                localeInfo.DETAILS_TOOLTIP_59,
                item.GetApplyPoint(item.APPLY_STEAL_HP),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_60,
                localeInfo.DETAILS_TOOLTIP_60,
                item.GetApplyPoint(item.APPLY_STEAL_SP),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_61,
                localeInfo.DETAILS_TOOLTIP_61,
                item.GetApplyPoint(item.APPLY_HP_REGEN),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_62,
                localeInfo.DETAILS_TOOLTIP_62,
                item.GetApplyPoint(item.APPLY_SP_REGEN),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_63,
                localeInfo.DETAILS_TOOLTIP_63,
                item.GetApplyPoint(item.APPLY_BLOCK),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_64,
                localeInfo.DETAILS_TOOLTIP_65,
                item.GetApplyPoint(item.APPLY_DODGE),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_65,
                localeInfo.DETAILS_TOOLTIP_65,
                item.GetApplyPoint(item.APPLY_REFLECT_MELEE),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_66,
                localeInfo.DETAILS_TOOLTIP_66,
                item.GetApplyPoint(item.APPLY_KILL_HP_RECOVER),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_67,
                localeInfo.DETAILS_TOOLTIP_67,
                item.GetApplyPoint(item.APPLY_KILL_SP_RECOVER),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_68,
                localeInfo.DETAILS_TOOLTIP_68,
                item.GetApplyPoint(item.APPLY_EXP_DOUBLE_BONUS),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_69,
                localeInfo.DETAILS_TOOLTIP_69,
                item.GetApplyPoint(item.APPLY_GOLD_DOUBLE_BONUS),
            ]
        )
        self.MiscBonusInfo.append(
            [
                localeInfo.DETAILS_70,
                localeInfo.DETAILS_TOOLTIP_70,
                item.GetApplyPoint(item.APPLY_ITEM_DROP_BONUS),
            ]
        )
        self.Categories.append(["Çeşitli Bonuslar", self.MiscBonusInfo])

    def InitWeaponInfo(self):
        self.WeaponBonusInfo = []
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_46,
                localeInfo.DETAILS_TOOLTIP_46,
                item.GetApplyPoint(item.APPLY_RESIST_SWORD),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_47,
                localeInfo.DETAILS_TOOLTIP_47,
                item.GetApplyPoint(item.APPLY_RESIST_TWOHAND),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_48,
                localeInfo.DETAILS_TOOLTIP_48,
                item.GetApplyPoint(item.APPLY_RESIST_DAGGER),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_49,
                localeInfo.DETAILS_TOOLTIP_49,
                item.GetApplyPoint(item.APPLY_RESIST_CLAW),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_50,
                localeInfo.DETAILS_TOOLTIP_50,
                item.GetApplyPoint(item.APPLY_RESIST_BELL),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_51,
                localeInfo.DETAILS_TOOLTIP_51,
                item.GetApplyPoint(item.APPLY_RESIST_FAN),
            ]
        )
        self.WeaponBonusInfo.append(
            [
                localeInfo.DETAILS_52,
                localeInfo.DETAILS_TOOLTIP_52,
                item.GetApplyPoint(item.APPLY_RESIST_BOW),
            ]
        )
        if app.ENABLE_PENDANT:
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_SWORD,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_SWORD,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_SWORD),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_TWOHAND,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_TWOHAND,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_TWOHAND),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_DAGGER,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_DAGGER,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_DAGGER),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_BELL,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_BELL,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_BELL),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_FAN,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_FAN,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_FAN),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_BOW,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_BOW,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_BOW),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_ATTBONUS_CLAW,
                    localeInfo.DETAILS_TOOLTIP_ATTBONUS_CLAW,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_CLAW),
                ]
            )
            self.WeaponBonusInfo.append(
                [
                    localeInfo.DETAILS_RESIST_MOUNT_FALL,
                    localeInfo.DETAILS_TOOLTIP_RESIST_MOUNT_FALL,
                    item.GetApplyPoint(item.APPLY_RESIST_MOUNT_FALL),
                ]
            )
        self.Categories.append(["Silah Savunma Bonusları", self.WeaponBonusInfo])

    def InitClassInfo(self):
        self.ClassBonus = []
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_36,
                localeInfo.DETAILS_TOOLTIP_36,
                item.GetApplyPoint(item.APPLY_ATTBONUS_WARRIOR),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_37,
                localeInfo.DETAILS_TOOLTIP_37,
                item.GetApplyPoint(item.APPLY_ATTBONUS_ASSASSIN),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_38,
                localeInfo.DETAILS_TOOLTIP_38,
                item.GetApplyPoint(item.APPLY_ATTBONUS_SURA),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_39,
                localeInfo.DETAILS_TOOLTIP_39,
                item.GetApplyPoint(item.APPLY_ATTBONUS_SHAMAN),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_40,
                localeInfo.DETAILS_TOOLTIP_40,
                item.GetApplyPoint(item.APPLY_ATTBONUS_WOLFMAN),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_41,
                localeInfo.DETAILS_TOOLTIP_41,
                item.GetApplyPoint(item.APPLY_RESIST_WARRIOR),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_42,
                localeInfo.DETAILS_TOOLTIP_42,
                item.GetApplyPoint(item.APPLY_RESIST_ASSASSIN),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_43,
                localeInfo.DETAILS_TOOLTIP_43,
                item.GetApplyPoint(item.APPLY_RESIST_SURA),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_44,
                localeInfo.DETAILS_TOOLTIP_44,
                item.GetApplyPoint(item.APPLY_RESIST_SHAMAN),
            ]
        )
        self.ClassBonus.append(
            [
                localeInfo.DETAILS_45,
                localeInfo.DETAILS_TOOLTIP_45,
                item.GetApplyPoint(item.APPLY_RESIST_WOLFMAN),
            ]
        )
        self.Categories.append(["Karakter Bonusları", self.ClassBonus])

    def InitElementalInfo(self):
        self.ElementalInfo = []
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_76,
                localeInfo.DETAILS_TOOLTIP_76,
                item.GetApplyPoint(item.APPLY_RESIST_MAGIC),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_24,
                localeInfo.DETAILS_TOOLTIP_24,
                item.GetApplyPoint(item.APPLY_RESIST_ELEC),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_25,
                localeInfo.DETAILS_TOOLTIP_25,
                item.GetApplyPoint(item.APPLY_RESIST_ICE),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_26,
                localeInfo.DETAILS_TOOLTIP_26,
                item.GetApplyPoint(item.APPLY_RESIST_DARK),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_27,
                localeInfo.DETAILS_TOOLTIP_27,
                item.GetApplyPoint(item.APPLY_RESIST_FIRE),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_28,
                localeInfo.DETAILS_TOOLTIP_28,
                item.GetApplyPoint(item.APPLY_RESIST_WIND),
            ]
        )
        self.ElementalInfo.append(
            [
                localeInfo.DETAILS_29,
                localeInfo.DETAILS_TOOLTIP_29,
                item.GetApplyPoint(item.APPLY_RESIST_EARTH),
            ]
        )
        # self.InfoList.append(
        # [localeInfo.DETAILS_77, localeInfo.DETAILS_TOOLTIP_77, item.GetApplyPoint(item.APPLY_RESIST_MAGIC_REDUCTION)])
        if app.ENABLE_ELEMENT_ADD:
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_30,
                    localeInfo.DETAILS_TOOLTIP_30,
                    item.GetApplyPoint(item.APPLY_ENCHANT_ELECT),
                ]
            )
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_31,
                    localeInfo.DETAILS_TOOLTIP_31,
                    item.GetApplyPoint(item.APPLY_ENCHANT_ICE),
                ]
            )
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_32,
                    localeInfo.DETAILS_TOOLTIP_32,
                    item.GetApplyPoint(item.APPLY_ENCHANT_DARK),
                ]
            )
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_33,
                    localeInfo.DETAILS_TOOLTIP_33,
                    item.GetApplyPoint(item.APPLY_ENCHANT_FIRE),
                ]
            )
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_34,
                    localeInfo.DETAILS_TOOLTIP_34,
                    item.GetApplyPoint(item.APPLY_ENCHANT_WIND),
                ]
            )
            self.ElementalInfo.append(
                [
                    localeInfo.DETAILS_35,
                    localeInfo.DETAILS_TOOLTIP_35,
                    item.GetApplyPoint(item.APPLY_ENCHANT_EARTH),
                ]
            )
        self.Categories.append(["Direnç Bonusları", self.ElementalInfo])

    def InitAttackDefenceInfo(self):
        self.AttackDefenceInfo = []
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_1,
                localeInfo.DETAILS_TOOLTIP_1,
                item.GetApplyPoint(item.APPLY_ATTBONUS_HUMAN),
            ]
        )
        if app.ENABLE_PVP_BALANCE or app.ENABLE_PENDANT:
            self.AttackDefenceInfo.append(
                [
                    localeInfo.DETAILS_2,
                    localeInfo.DETAILS_TOOLTIP_2,
                    item.GetApplyPoint(item.APPLY_RESIST_HUMAN),
                ]
            )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_3,
                localeInfo.DETAILS_TOOLTIP_3,
                item.GetApplyPoint(item.APPLY_ATTBONUS_ORC),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_4,
                localeInfo.DETAILS_TOOLTIP_4,
                item.GetApplyPoint(item.APPLY_ATTBONUS_UNDEAD),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_5,
                localeInfo.DETAILS_TOOLTIP_5,
                item.GetApplyPoint(item.APPLY_ATTBONUS_MONSTER),
            ]
        )
        if app.ENABLE_ELEMENT_ADD:
            self.AttackDefenceInfo.append(
                [
                    localeInfo.DETAILS_6,
                    localeInfo.DETAILS_TOOLTIP_6,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_CZ),
                ]
            )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_7,
                localeInfo.DETAILS_TOOLTIP_7,
                item.GetApplyPoint(item.APPLY_ATTBONUS_ANIMAL),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_8,
                localeInfo.DETAILS_TOOLTIP_8,
                item.GetApplyPoint(item.APPLY_ATTBONUS_MILGYO),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_9,
                localeInfo.DETAILS_TOOLTIP_9,
                item.GetApplyPoint(item.APPLY_ATTBONUS_DEVIL),
            ]
        )
        if app.ENABLE_ELEMENT_ADD:
            self.AttackDefenceInfo.append(
                [
                    localeInfo.DETAILS_10,
                    localeInfo.DETAILS_TOOLTIP_10,
                    item.GetApplyPoint(item.APPLY_ATTBONUS_DESERT),
                ]
            )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_11,
                localeInfo.DETAILS_TOOLTIP_11,
                item.GetApplyPoint(item.APPLY_ATTBONUS_INSECT),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_78,
                localeInfo.DETAILS_TOOLTIP_78,
                item.GetApplyPoint(item.APPLY_ATTBONUS_TRENT),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_79,
                localeInfo.DETAILS_TOOLTIP_79,
                item.GetApplyPoint(item.APPLY_ATTBONUS_BOSS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_80,
                localeInfo.DETAILS_TOOLTIP_80,
                item.GetApplyPoint(item.APPLY_ATTBONUS_METIN),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_12,
                localeInfo.DETAILS_TOOLTIP_12,
                item.GetApplyPoint(item.APPLY_ATT_GRADE_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_13,
                localeInfo.DETAILS_TOOLTIP_13,
                item.GetApplyPoint(item.APPLY_DEF_GRADE_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_14,
                localeInfo.DETAILS_TOOLTIP_14,
                item.GetApplyPoint(item.APPLY_NORMAL_HIT_DAMAGE_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_15,
                localeInfo.DETAILS_TOOLTIP_15,
                item.GetApplyPoint(item.APPLY_NORMAL_HIT_DEFEND_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_16,
                localeInfo.DETAILS_TOOLTIP_16,
                item.GetApplyPoint(item.APPLY_SKILL_DAMAGE_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_17,
                localeInfo.DETAILS_TOOLTIP_17,
                item.GetApplyPoint(item.APPLY_SKILL_DEFEND_BONUS),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_18,
                localeInfo.DETAILS_TOOLTIP_18,
                item.GetApplyPoint(item.APPLY_MELEE_MAGIC_ATTBONUS_PER),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_19,
                localeInfo.DETAILS_TOOLTIP_19,
                item.GetApplyPoint(item.APPLY_MAGIC_ATTBONUS_PER),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_20,
                localeInfo.DETAILS_TOOLTIP_20,
                item.GetApplyPoint(item.APPLY_CRITICAL_PCT),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_21,
                localeInfo.DETAILS_TOOLTIP_21,
                item.GetApplyPoint(item.APPLY_PENETRATE_PCT),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_22,
                localeInfo.DETAILS_TOOLTIP_22,
                item.GetApplyPoint(item.APPLY_ANTI_CRITICAL_PCT),
            ]
        )
        self.AttackDefenceInfo.append(
            [
                localeInfo.DETAILS_23,
                localeInfo.DETAILS_TOOLTIP_23,
                item.GetApplyPoint(item.APPLY_ANTI_PENETRATE_PCT),
            ]
        )
        self.Categories.append(["Saldırı ve Savunma Bonusları", self.AttackDefenceInfo])

    def Show(self):
        ui.ScriptWindow.Show(self)
        self.SetTop()
        self.RefreshLabel()

    def Close(self):
        self.Hide()

    def AdjustPosition(self, x, y):
        self.SetPosition(x + self.Width, y)

    # idx, tooltip, argLen, pos_x, pos_y
    def __ButtonOverIn(self, i):
        (pos_x, pos_y) = wndMgr.GetMousePosition()
        self.toolTip.ClearToolTip()
        self.toolTip.SetThinBoardSize(11)
        self.toolTip.SetToolTipPosition(pos_x + 50, pos_y + 50)
        self.toolTip.AppendTextLine("", 4294967040)
        self.toolTip.Show()

    def __ButtonOverOut(self, idx):
        self.toolTip.Hide()
