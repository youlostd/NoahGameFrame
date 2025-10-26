# -*- coding: utf-8 -*-

import logging

import app
import chrmgr
import item
import player
import skill
from pygame.app import appInst

import localeInfo
import ui
import uiToolTip
from ui_event import Event

INFINITE_AFFECT_DURATION = 0x1FFFFFFF
from ui_event import MakeEvent
from uiofflineshop import OfflineShopManager
from _weakref import proxy

class AffectImage(ui.ExpandedImageBox):
    def __init__(self):
        ui.ExpandedImageBox.__init__(self)

        self.toolTip = uiToolTip.ItemToolTip(100)
        self.toolTip.CheckUsability(False)
        self.toolTip.HideToolTip()

    def OnMouseOverIn(self):
        self.toolTip.ShowToolTip()
        return True

    def OnMouseOverOut(self):
        self.toolTip.HideToolTip()
        return True


class PolyImage(AffectImage):
    def __init__(self, affectType, image, duration, description):
        AffectImage.__init__(self)

        if duration != INFINITE_AFFECT_DURATION and 0 < duration < 60 * 60 * 24 * 356 * 10:
            self.endTime = app.GetGlobalTimeStamp() + duration
        else:
            self.endTime = 0

        self.affectType = affectType
        self.polymorphQuestionDialog = None

        self.description = description
        self.LoadImage(image)
        self.SetScale(0.7, 0.7)
        self.Update()

    def GetType(self):
        return self.affectType

    def OnMouseLeftButtonUp(self):
        self.OnPolymorphQuestionDialog()

    def OnPolymorphQuestionDialog(self):
        import uiCommon
        self.polymorphQuestionDialog = uiCommon.QuestionDialog()
        self.polymorphQuestionDialog.SetText(localeInfo.POLYMORPH_AFFECT_REMOVE_QUESTION)
        self.polymorphQuestionDialog.SetWidth(370)
        self.polymorphQuestionDialog.SetAcceptEvent(Event(self.OnClosePolymorphQuestionDialog, True))
        self.polymorphQuestionDialog.SetCancelEvent(Event(self.OnClosePolymorphQuestionDialog, False))
        self.polymorphQuestionDialog.Open()

    def OnClosePolymorphQuestionDialog(self, answer):
        if not self.polymorphQuestionDialog:
            return False

        self.polymorphQuestionDialog.Close()
        self.polymorphQuestionDialog = None

        if not answer:
            return False

        appInst.instance().GetNet().SendChatPacket("/remove_polymorph")
        return True

    def Update(self):
        if not self.description:
            return

        self.toolTip.ClearToolTip()
        self.toolTip.AppendTextLine(self.description)

        if self.endTime != 0:
            now = app.GetGlobalTimeStamp()
            leftTime = self.endTime - now
            self.toolTip.AppendTextLine("{}: {}".format(localeInfo.LEFT_TIME, localeInfo.SecondToDHMS(leftTime)))

class LovePointImage(AffectImage):
    FILE_PATH = "d:/ymir work/ui/pattern/LovePoint/"
    FILE_DICT = {
        # TODO: Use .sub images!
        0: FILE_PATH + "01.dds",
        1: FILE_PATH + "02.dds",
        2: FILE_PATH + "02.dds",
        3: FILE_PATH + "03.dds",
        4: FILE_PATH + "04.dds",
        5: FILE_PATH + "05.dds",
    }

    def __init__(self):
        AffectImage.__init__(self)

        self.loverName = ""
        self.lovePoint = 0

    def SetLoverInfo(self, name, lovePoint):
        self.loverName = name
        self.lovePoint = lovePoint
        self.__Refresh()

    def OnUpdateLovePoint(self, lovePoint):
        self.lovePoint = lovePoint
        self.__Refresh()

    def __Refresh(self):
        self.lovePoint = max(0, self.lovePoint)
        self.lovePoint = min(100, self.lovePoint)

        if 0 == self.lovePoint:
            loveGrade = 0
        else:
            loveGrade = self.lovePoint / 25 + 1

        fileName = self.FILE_DICT.get(loveGrade, self.FILE_PATH + "00.dds")

        try:
            self.LoadImage(fileName)
        except RuntimeError:
            print("Loading failed for lovePoint=%d %s" % (self.lovePoint, fileName))

        # This is necessary because LoadImage() destroys the old
        # CGraphicExpandedImageInstance object...
        self.SetScale(0.7, 0.7)

        self.toolTip.ClearToolTip()
        self.toolTip.SetTitle(self.loverName)
        self.toolTip.AppendTextLine(localeInfo.AFFECT_LOVE_POINT.format(self.lovePoint))



class HorseImage(AffectImage):
    FILE_PATH = "d:/ymir work/ui/pattern/HorseState/"

    FILE_DICT = {
        # TODO: Use .sub images!
        00: FILE_PATH + "00.dds",
        01: FILE_PATH + "00.dds",
        02: FILE_PATH + "00.dds",
        03: FILE_PATH + "00.dds",
        10: FILE_PATH + "10.dds",
        11: FILE_PATH + "11.dds",
        12: FILE_PATH + "12.dds",
        13: FILE_PATH + "13.dds",
        20: FILE_PATH + "20.dds",
        21: FILE_PATH + "21.dds",
        22: FILE_PATH + "22.dds",
        23: FILE_PATH + "23.dds",
        30: FILE_PATH + "30.dds",
        31: FILE_PATH + "31.dds",
        32: FILE_PATH + "32.dds",
        33: FILE_PATH + "33.dds",
    }

    def __GetHorseGrade(self, level):
        if 0 == level:
            return 0

        return (level - 1) / 10 + 1

    def SetState(self, level, health, battery):
        self.toolTip.ClearToolTip()

        if level == 0:
            return

        grade = self.__GetHorseGrade(level)

        try:
            self.toolTip.AppendTextLine(localeInfo.LEVEL_LIST[grade])
        except IndexError:
            return

        try:
            healthName = localeInfo.HEALTH_LIST[health]
            if healthName:
                self.toolTip.AppendTextLine(healthName)
        except IndexError:
            return

        if health > 0 and battery == 0:
            self.toolTip.AppendTextLine(localeInfo.NEEFD_REST)

        try:
            fileName = self.FILE_DICT[health * 10 + battery]
        except KeyError:
            return

        try:
            self.LoadImage(fileName)
        except:
            return

        # [tim] This is necessary because LoadImage() destroys the old
        # CGraphicExpandedImageInstance object...
        self.SetScale(0.7, 0.7)


class DropStatusImage(AffectImage):
    FILE_PATH = "d:/ymir work/ui/public/button_refresh_"

    FILE_DICT = {
        0: FILE_PATH + "03.sub",
        1: FILE_PATH + "01.sub",
    }


    def SetState(self, state):
        self.toolTip.ClearToolTip()

        self.toolTip.AppendTextLine(localeInfo.Get("TOOLTIP_DROP_STATE").format(localeInfo.Get("DROP_STATE_ENABLED") if state else localeInfo.Get("DROP_STATE_DISABLED")))

        try:
            fileName = self.FILE_DICT[state]
        except KeyError:
            return

        try:
            self.LoadImage(fileName)
        except Exception as e:
            return

        self.SetScale(1.0, 1.0)

class ExpEventImage(AffectImage):
    FILE_PATH = "d:/ymir work/ui/exp_"

    FILE_DICT = {
        0: FILE_PATH + "bonus.tga",
        1: FILE_PATH + "bonus_hot_time.tga",
    }

    def __init__(self):
        AffectImage.__init__(self)

        self.timeOut = None
        self.end_time = 0

    def SetState(self, state, exp_bonus, end_time):
        self.toolTip.ClearToolTip()
        self.end_time = end_time
        self.toolTip.AppendTextLine(localeInfo.Get("TOOLTIP_EXP_BONUS").format(exp_bonus))
        self.timeOut = self.toolTip.AppendTextLine(localeInfo.SecondToDHMS(self.end_time - app.GetGlobalTimeStamp()))

        try:
            fileName = self.FILE_DICT[state]
        except KeyError:
            return

        try:
            self.LoadImage(fileName)
        except Exception as e:
            return

        self.SetScale(1.0, 1.0)

    def Update(self):
        self.timeOut.SetText(localeInfo.SecondToDHMS(self.end_time - app.GetGlobalTimeStamp()))

class SimpleAffectImage(AffectImage):
    def __init__(self, affectType, subType, image, duration, description, scale=True):
        AffectImage.__init__(self)

        if subType == 0:
            if duration != INFINITE_AFFECT_DURATION and 0 < duration < 60 * 60 * 24 * 356 * 10:
                self.endTime = app.GetGlobalTimeStamp() + duration
            else:
                self.endTime = 0
        elif subType == 1:
            self.endTime = duration

        self.removeAffectQuestionDialog = None

        self.affectType = affectType

        self.description = description
        self.LoadImage(image)
        if scale:
            self.SetScale(0.7, 0.7)
        self.Update()

    def GetType(self):
        return self.affectType

    def OnMouseLeftButtonUp(self):
        self.OnyRemoveAffectQuestionDialog()

    def OnyRemoveAffectQuestionDialog(self):
        import uiCommon
        self.removeAffectQuestionDialog = uiCommon.QuestionDialog()
        self.removeAffectQuestionDialog.SetText(localeInfo.AFFECT_REMOVE_QUESTION)
        self.removeAffectQuestionDialog.SetWidth(370)
        self.removeAffectQuestionDialog.SetAcceptEvent(Event(self.OnCloseRemoveAffectDialog, True))
        self.removeAffectQuestionDialog.SetCancelEvent(Event(self.OnCloseRemoveAffectDialog, False))
        self.removeAffectQuestionDialog.Open()

    def OnCloseRemoveAffectDialog(self, answer):
        if not self.removeAffectQuestionDialog:
            return False

        self.removeAffectQuestionDialog.Close()
        self.removeAffectQuestionDialog = None

        if not answer:
            return False

        appInst.instance().GetNet().SendChatPacket("/remove_affect " + str(self.affectType))
        return True

    def Update(self):
        if not self.description:
            return

        self.toolTip.ClearToolTip()
        self.toolTip.AppendTextLine(self.description)

        if self.endTime != 0:
            now = app.GetGlobalTimeStamp()
            leftTime = self.endTime - now
            self.toolTip.AppendTextLine("{}: {}".format(localeInfo.LEFT_TIME, localeInfo.SecondToDHMS(leftTime)))





class ItemImage(AffectImage):
    def __init__(self, cell):
        AffectImage.__init__(self)

        self.cell = cell

    def SetImage(self, filename):
        self.LoadImage(filename)
        self.SetScale(0.7, 0.7)

    def SetCell(self, cell):
        self.cell = cell
        self.__Refresh()

    def __Refresh(self):
        self.toolTip.ClearToolTip()
        self.toolTip.SetInventoryItem(self.cell)
        self.toolTip.HideToolTip()

    def Update(self):
        return


class AutoPotionImage(ItemImage):
    FILE_PATH_HP = "d:/ymir work/ui/pattern/auto_hpgauge/"
    FILE_PATH_SP = "d:/ymir work/ui/pattern/auto_spgauge/"

    def __init__(self, subType, cell):
        ItemImage.__init__(self, cell)

        self.subType = subType
        self.oldGrade = -1

    def Update(self):
        itemVnum = player.GetItemIndex(player.INVENTORY, self.cell)
        if itemVnum == 0:
            return

        item.SelectItem(itemVnum)
        metinSocket = [player.GetItemMetinSocket(self.cell, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]

        totalAmount = metinSocket[2]
        usedAmount = metinSocket[1]
        currentAmount = totalAmount - usedAmount

        # We have no real info yet - skip
        if 0 == totalAmount:
            return

        amountPercent = 100 * currentAmount / totalAmount

        if self.subType == item.TOGGLE_AUTO_RECOVERY_HP:
            path = self.FILE_PATH_HP
        else:
            path = self.FILE_PATH_SP

        grade = self.__GetGradeFromPercent(amountPercent)
        if self.oldGrade != grade:
            fileName = "%s%.2d.dds" % (path, grade)

            try:
                self.SetImage(fileName)
            except RuntimeError:
                logging.debug("Failed to load auto-potion image %s" % fileName)

            self.oldGrade = grade

        ItemImage.Update(self)

    @staticmethod
    def __GetGradeFromPercent(percent):
        if percent > 80:
            return 5
        if percent > 60:
            return 4
        if percent > 40:
            return 3
        if percent > 20:
            return 2

        return 1


if app.ENABLE_GROWTH_PET_SYSTEM:
    # GROWTH PET IMAGE
    class GrowthPetImage(ui.ExpandedImageBox):

        def __init__(self):
            ui.ExpandedImageBox.__init__(self)

            self.toolTipText = None
            self.description = None

        def SetToolTipText(self, text, x=0, y=-19):

            if not self.toolTipText:
                textLine = ui.TextLine()
                textLine.SetParent(self)
                textLine.SetSize(0, 0)
                textLine.SetOutline()
                textLine.Hide()
                self.toolTipText = textLine

            self.toolTipText.SetText(text)
            w, h = (self.toolTipText.GetWidth(), self.toolTipText.GetHeight())
            if localeInfo.IsARABIC():
                self.toolTipText.SetPosition(w + 20, y)
            else:
                self.toolTipText.SetPosition(max(0, x + self.GetWidth() / 2 - w / 2), y)

        def SetDescription(self, description):
            self.description = description

        def OnMouseOverIn(self):
            if self.toolTipText:
                self.toolTipText.Show()
                return True
            return False

        def OnMouseOverOut(self):
            if self.toolTipText:
                self.toolTipText.Hide()
                return True
            return False

            # END OF GROWTH PET IMAGE


class AffectShower(ui.Window):
    MALL_DESC_IDX_START = 1000
    IMAGE_STEP = 25
    AFFECT_MAX_NUM = 32

    AFFECT_DATA_DICT = {
        chrmgr.AFFECT_MOV_SPEED: (
            localeInfo.SKILL_INC_MOVSPD, "d:/ymir work/ui/skill/common/affect/Increase_Move_Speed.sub"),
        chrmgr.AFFECT_ATT_SPEED: (
            localeInfo.SKILL_INC_ATKSPD, "d:/ymir work/ui/skill/common/affect/Increase_Attack_Speed.sub"),

        chrmgr.AFFECT_POISON: (localeInfo.SKILL_TOXICDIE, "d:/ymir work/ui/skill/common/affect/poison.sub"),
        chrmgr.AFFECT_FIRE: (localeInfo.SKILL_FIRE, "d:/ymir work/ui/skill/sura/hwayeom_03.sub",),
        chrmgr.AFFECT_SLOW: (localeInfo.SKILL_SLOW, "d:/ymir work/ui/skill/common/affect/slow.sub"),
        chrmgr.AFFECT_STUN: (localeInfo.SKILL_STUN, "d:/ymir work/ui/skill/common/affect/stun.sub"),

        chrmgr.AFFECT_EXP_BONUS: (
            localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),

        chrmgr.AFFECT_ITEM_BONUS: (
            localeInfo.TOOLTIP_MALL_ITEMBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
        chrmgr.AFFECT_SAFEBOX: (localeInfo.TOOLTIP_MALL_SAFEBOX, "d:/ymir work/ui/skill/common/affect/safebox.sub",),
        chrmgr.AFFECT_AUTOLOOT: (localeInfo.TOOLTIP_MALL_AUTOLOOT, "d:/ymir work/ui/skill/common/affect/autoloot.sub",),
        chrmgr.AFFECT_FISH_MIND: (
            localeInfo.TOOLTIP_MALL_FISH_MIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub",),
        chrmgr.AFFECT_MARRIAGE_FAST: (
            localeInfo.TOOLTIP_MALL_MARRIAGE_FAST, "d:/ymir work/ui/skill/common/affect/marriage_fast.sub",),
        chrmgr.AFFECT_GOLD_BONUS: (
            localeInfo.TOOLTIP_MALL_GOLDBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),

        chrmgr.AFFECT_VOTE_BONUS2: (
            localeInfo.Get("TOOLTIP_AFFECT_VOTE_BONUS"), "d:/ymir work/ui/votebuff.png",),

        chrmgr.AFFECT_NO_DEATH_PENALTY: (
            localeInfo.TOOLTIP_APPLY_NO_DEATH_PENALTY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
        chrmgr.AFFECT_SKILL_BOOK_BONUS: (
            localeInfo.TOOLTIP_APPLY_SKILL_BOOK_BONUS, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
        chrmgr.AFFECT_SKILL_NO_BOOK_DELAY: (
            localeInfo.TOOLTIP_APPLY_SKILL_BOOK_NO_DELAY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),

        chrmgr.AFFECT_FISH_MIND_PILL: (localeInfo.SKILL_FISHMIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub"),
        chrmgr.AFFECT_CHINA_FIREWORK: (
            localeInfo.SKILL_POWERFUL_STRIKE, "d:/ymir work/ui/skill/common/affect/powerfulstrike.sub",),

        # Skills

        # assassin

        chrmgr.SKILL_GYEONGGONG: (localeInfo.SKILL_GYEONGGONG, "d:/ymir work/ui/skill/assassin/gyeonggong_03.sub",),
        chrmgr.SKILL_EUNHYUNG: (localeInfo.SKILL_EUNHYEONG, "d:/ymir work/ui/skill/assassin/eunhyeong_03.sub",),

        # shaman
        chrmgr.SKILL_JEUNGRYEOK: (localeInfo.SKILL_JEUNGRYEOK, "d:/ymir work/ui/skill/shaman/jeungryeok_03.sub",),
        chrmgr.SKILL_GICHEON: (localeInfo.SKILL_GICHEON, "d:/ymir work/ui/skill/shaman/gicheon_03.sub",),
        chrmgr.SKILL_KWAESOK: (localeInfo.SKILL_KWAESOK, "d:/ymir work/ui/skill/shaman/kwaesok_03.sub",),
        chrmgr.SKILL_REFLECT: (localeInfo.SKILL_BOHO, "d:/ymir work/ui/skill/shaman/boho_03.sub",),
        chrmgr.SKILL_HOSIN: (localeInfo.SKILL_HOSIN, "d:/ymir work/ui/skill/shaman/hosin_03.sub",),

        # sura
        chrmgr.SKILL_PABEOB: (localeInfo.SKILL_PABEOP, "d:/ymir work/ui/skill/sura/pabeop_03.sub",),
        chrmgr.AFFECT_PREVENT_GOOD: (localeInfo.SKILL_PABEOP, "d:/ymir work/ui/skill/sura/pabeop_03.sub",),
        chrmgr.SKILL_MUYEONG: (localeInfo.SKILL_MUYEONG, "d:/ymir work/ui/skill/sura/muyeong_03.sub",),
        chrmgr.SKILL_MANASHIELD: (localeInfo.SKILL_HEUKSIN, "d:/ymir work/ui/skill/sura/heuksin_03.sub",),
        chrmgr.SKILL_JUMAGAP: (localeInfo.SKILL_JUMAGAP, "d:/ymir work/ui/skill/sura/jumagap_03.sub"),
        chrmgr.SKILL_GWIGEOM: (localeInfo.SKILL_GWIGEOM, "d:/ymir work/ui/skill/sura/gwigeom_03.sub",),
        chrmgr.SKILL_TERROR: (localeInfo.SKILL_GONGPO, "d:/ymir work/ui/skill/sura/gongpo_03.sub",),

        # warrior
        chrmgr.SKILL_JEONGWI: (localeInfo.SKILL_JEONGWI, "d:/ymir work/ui/skill/warrior/jeongwi_03.sub",),
        chrmgr.SKILL_GEOMKYUNG: (localeInfo.SKILL_GEOMGYEONG, "d:/ymir work/ui/skill/warrior/geomgyeong_03.sub",),
        chrmgr.SKILL_CHUNKEON: (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),
        # TODO
        # chr.AFF_FALLEN_CHEONGEUN : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),

        # wolfman
        chrmgr.SKILL_JEOKRANG: (localeInfo.SKILL_GWIGEOM, "d:/ymir work/ui/skill/wolfman/red_possession_03.sub",),
        chrmgr.SKILL_CHEONGRANG: (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/wolfman/blue_possession_03.sub",),

        # 자동물약 hp, sp
        # AFFECT_AUTO_HP_RECOVERY: (localeInfo.TOOLTIP_AUTO_POTION_REST, "<auto>"),
        # AFFECT_AUTO_SP_RECOVERY: (localeInfo.TOOLTIP_AUTO_POTION_REST, "<auto>"),

        MALL_DESC_IDX_START + player.POINT_MALL_ATTBONUS: (
            localeInfo.TOOLTIP_MALL_ATTBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/att_bonus.sub",),
        MALL_DESC_IDX_START + player.POINT_MALL_DEFBONUS: (
            localeInfo.TOOLTIP_MALL_DEFBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/def_bonus.sub",),
        MALL_DESC_IDX_START + player.POINT_MALL_EXPBONUS: (
            localeInfo.TOOLTIP_MALL_EXPBONUS, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),
        MALL_DESC_IDX_START + player.POINT_MALL_ITEMBONUS: (
            localeInfo.TOOLTIP_MALL_ITEMBONUS, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
        MALL_DESC_IDX_START + player.POINT_MALL_GOLDBONUS: (
            localeInfo.TOOLTIP_MALL_GOLDBONUS, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),
        MALL_DESC_IDX_START + player.POINT_CRITICAL_PCT: (
            localeInfo.TOOLTIP_APPLY_CRITICAL_PCT, "d:/ymir work/ui/skill/common/affect/critical.sub"),
        MALL_DESC_IDX_START + player.POINT_PENETRATE_PCT: (
            localeInfo.TOOLTIP_APPLY_PENETRATE_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
        MALL_DESC_IDX_START + player.POINT_MAX_HP_PCT: (
            localeInfo.TOOLTIP_MAX_HP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
        MALL_DESC_IDX_START + player.POINT_MAX_SP_PCT: (
            localeInfo.TOOLTIP_MAX_SP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),

        MALL_DESC_IDX_START + player.POINT_PC_BANG_EXP_BONUS: (
            localeInfo.TOOLTIP_MALL_EXPBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/EXP_Bonus_p_on.sub",),
        MALL_DESC_IDX_START + player.POINT_PC_BANG_DROP_BONUS: (
            localeInfo.TOOLTIP_MALL_ITEMBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/Item_Bonus_p_on.sub",),
    }
    if app.ENABLE_DRAGON_SOUL_SYSTEM:
        # 용혼석 천, 지 덱.
        AFFECT_DATA_DICT[chrmgr.AFFECT_DRAGON_SOUL_DECK_0] = (
            localeInfo.TOOLTIP_DRAGON_SOUL_DECK1, "d:/ymir work/ui/dragonsoul/buff_ds_sky1.tga")
        AFFECT_DATA_DICT[chrmgr.AFFECT_DRAGON_SOUL_DECK_1] = (
            localeInfo.TOOLTIP_DRAGON_SOUL_DECK2, "d:/ymir work/ui/dragonsoul/buff_ds_land1.tga")

    if app.ENABLE_BATTLE_PASS:
        AFFECT_DATA_DICT[chrmgr.AFFECT_BATTLE_PASS] = (
        localeInfo.Get("TOOLTIP_AFFECT_BATTLE_PASS"), "d:/ymir work/ui/skill/common/affect/researcher_elixir.sub")

    AFFECT_DATA_DICT[chrmgr.AFFECT_PREMIUM_SWITCHBOT] = (
        localeInfo.Get("TOOLTIP_AFFECT_PREMIUM_SWITCHBOT"), "icon/item/71084.tga", True)

    AFFECT_DATA_DICT[chrmgr.AFFECT_DS_SET] = (localeInfo.TOOLTIP_DS_SET,
                                                  "d:/ymir work/ui/skill/common/affect/ds_set_bonus.sub")

    if app.ENABLE_AFFECT_POLYMORPH_REMOVE:
        AFFECT_DATA_DICT[chrmgr.AFFECT_POLYMORPH] = (
            localeInfo.POLYMORPH_AFFECT_TOOLTIP, "d:/ymir work/ui/polymorph_marble_icon.png")

    TOGGLE_DATA = {
        item.TOGGLE_MOUNT: "d:/ymir work/ui/skill/common/affect/toggle_mount.tga",
        item.TOGGLE_PET: "d:/ymir work/ui/skill/common/affect/toggle_pet.tga",
        item.TOGGLE_ANTI_EXP: "d:/ymir work/ui/skill/common/affect/exp_bonus_p_on.sub",
        item.TOGGLE_LEVEL_PET: "d:/ymir work/ui/skill/common/affect/toggle_pet.tga",
    }

    def __init__(self):
        ui.Window.__init__(self)

        self.affectImageDict = {}
        self.toggleImageDict = {}
        self.toolTipItem = None

        self.horseImage = None
        self.lovePointImage = None
        self.dropStatus = None
        self.expBonusImage = None

        self.SetPosition(10, 10)
        self.Show()

        if app.ENABLE_GROWTH_PET_SYSTEM:
            self.petSkillaffectImageDict = {}

        self.TOGGLE_CALLBACK = {
            item.TOGGLE_MOUNT: MakeEvent(self.ToggleCallback_Default),
            item.TOGGLE_PET: MakeEvent(self.ToggleCallback_Default),
            item.TOGGLE_ANTI_EXP: MakeEvent(self.ToggleCallback_Default),
            item.TOGGLE_AFFECT: MakeEvent(self.ToggleCallback_ItemIcon),
            item.TOGGLE_AUTO_RECOVERY_HP: MakeEvent(self.ToggleCallback_AutoPotion),
            item.TOGGLE_AUTO_RECOVERY_SP: MakeEvent(self.ToggleCallback_AutoPotion),
            item.TOGGLE_LEVEL_PET: MakeEvent(self.ToggleCallback_Default),
        }

    def SetItemToolTip(self, toolTipItem):
        self.toolTipItem = proxy(toolTipItem)

    def ToggleCallback_Default(self, cell):
        image = ItemImage(cell)
        image.SetCell(cell)
        image.SetImage(self.TOGGLE_DATA[item.GetItemSubType()])
        return image

    def ToggleCallback_ItemIcon(self, cell):
        image = ItemImage(cell)
        image.SetCell(cell)
        image.SetImage(item.GetIconImageFileName())
        return image

    def ToggleCallback_AutoPotion(self, cell):
        image = AutoPotionImage(item.GetItemSubType(), cell)
        image.SetCell(cell)
        return image

    def ClearAllAffects(self):
        self.horseImage = None
        self.dropStatus = None
        self.expBonusImage = None
        self.lovePointImage = None
        self.affectImageDict = {}
        self.toggleImageDict = {}

        if app.ENABLE_GROWTH_PET_SYSTEM:
            self.petSkillaffectImageDict = {}

        self.__ArrangeImageList()

    def BINARY_NEW_AddAffect(self, affectType, subType, pointIdx, value, duration):

        if affectType == chrmgr.AFFECT_MALL:
            affect = self.MALL_DESC_IDX_START + pointIdx
        else:
            affect = affectType

        if affect in self.affectImageDict:
            return

        ## 용신의 가호, 선인의 교훈은 Duration 을 0 으로 설정한다.
        if affect == chrmgr.AFFECT_NO_DEATH_PENALTY or \
                affect == chrmgr.AFFECT_SKILL_BOOK_BONUS or \
                affect == chrmgr.AFFECT_AUTO_SP_RECOVERY or \
                affect == chrmgr.AFFECT_AUTO_HP_RECOVERY or \
                affect == chrmgr.AFFECT_SKILL_NO_BOOK_DELAY:
            duration = 0

        if pointIdx == player.POINT_MALL_ITEMBONUS or \
                pointIdx == player.POINT_MALL_GOLDBONUS:
            value = 1 + float(value) / 100.0

        if affect not in self.AFFECT_DATA_DICT:
            return

        affectData = self.AFFECT_DATA_DICT[affect]

        # TODO: Get rid of these wrappers...
        if callable(affectData[0]):
            description = affectData[0](float(value))
        else:
            try:
                description = affectData[0] % value
            except TypeError:
                description = affectData[0]
            except ValueError:
                description = affectData[0]

        scale = True
        if affect == chrmgr.AFFECT_DRAGON_SOUL_DECK_0 or affect == chrmgr.AFFECT_DRAGON_SOUL_DECK_1:
            scale = False

        if affect == chrmgr.AFFECT_POLYMORPH:
            image = PolyImage(affect, affectData[1], duration, description)
        else:
            image = SimpleAffectImage(affect, subType, affectData[1], duration, description, scale)
        image.SetParent(self)
        image.Show()

        self.affectImageDict[affect] = image
        self.__ArrangeImageList()

    def BINARY_NEW_RemoveAffect(self, affectType, pointIdx):
        if affectType == chrmgr.AFFECT_MALL:
            affect = self.MALL_DESC_IDX_START + pointIdx
        else:
            affect = affectType

        try:
            del self.affectImageDict[affect]
        except KeyError:
            print("Failed to find affect %d" % affectType)

        self.__ArrangeImageList()

    def SetLoverInfo(self, name, lovePoint):
        image = LovePointImage()
        image.SetParent(self)
        image.SetLoverInfo(name, lovePoint)
        self.lovePointImage = image
        self.__ArrangeImageList()

    def ShowLoverState(self):
        if self.lovePointImage:
            self.lovePointImage.Show()
            self.__ArrangeImageList()

    def HideLoverState(self):
        if self.lovePointImage:
            self.lovePointImage.Hide()
            self.__ArrangeImageList()

    def ClearLoverState(self):
        self.lovePointImage = None
        self.__ArrangeImageList()

    def OnUpdateLovePoint(self, lovePoint):
        if self.lovePointImage:
            self.lovePointImage.OnUpdateLovePoint(lovePoint)

    def SetHorseState(self, level, health, battery):
        if level == 0:
            self.horseImage = None
            self.__ArrangeImageList()
        else:
            image = HorseImage()
            image.SetParent(self)
            image.SetState(level, health, battery)
            image.Show()

            self.horseImage = image
            self.__ArrangeImageList()

    def SetDropState(self, state):
        if not self.dropStatus:
            image = DropStatusImage()
            image.SetParent(self)
            image.SetState(state)
            image.Show()

            self.dropStatus = image
            self.__ArrangeImageList()
            return

        self.dropStatus.SetState(state)

    def SetExpBonus(self, state, exp_bonus, end_time):
        if exp_bonus == 0:
            self.expBonusImage = None
            self.__ArrangeImageList()
        elif not self.expBonusImage:
            image = ExpEventImage()
            image.SetParent(self)
            image.SetState(state, exp_bonus, end_time)
            image.Show()

            self.expBonusImage = image
            self.__ArrangeImageList()
            return
        else:
            if self.expBonusImage:
                self.expBonusImage.SetState(state, exp_bonus, end_time)

    def RefreshInventory(self):

        self.toggleImageDict.clear()

        for cell in xrange(player.INVENTORY_SLOT_COUNT):
            itemVnum = player.GetItemIndex(player.INVENTORY, cell)
            if itemVnum == 0:
                self.__TryDeleteToggleItem(cell)
                continue

            item.SelectItem(itemVnum)

            if item.GetItemType() != item.ITEM_TYPE_TOGGLE:
                self.__TryDeleteToggleItem(cell)
                continue

            metinSocket = [player.GetItemMetinSocket(cell, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]

            if 0 != metinSocket[3]:
                # We already have an image...
                # i.e. nothing to do.
                if cell in self.toggleImageDict:
                    continue

                image = self.TOGGLE_CALLBACK[item.GetItemSubType()](cell)
                image.SetParent(self)
                image.Show()

                self.toggleImageDict[cell] = image
            else:
                self.__TryDeleteToggleItem(cell)

        self.__ArrangeImageList()

    def OnUpdate(self):
        for image in self.affectImageDict.values():
            try:
                image.Update()
            except Exception as e:
                print("Error during affect-image update")

        if self.expBonusImage:
            self.expBonusImage.Update()

        for image in self.toggleImageDict.values():
            try:
                image.Update()
            except Exception as e:
                print("Error during item-image update")

    def __ArrangeImageList(self):

        width = len(self.affectImageDict) + len(self.toggleImageDict)
        if app.ENABLE_GROWTH_PET_SYSTEM:
            width += len(self.petSkillaffectImageDict)

        offlineShopInfoIcon = OfflineShopManager.GetInstance().GetInfoIcon()
        if offlineShopInfoIcon:
            offlineShopInfoIcon.SetParent(self)
            offlineShopInfoIcon.Show()
            width += 1

        if self.lovePointImage:
            width += 1
        if self.horseImage:
            width += 1

        if self.dropStatus:
            width += 1
        if self.expBonusImage:
            width += 1

        self.SetSize(width * self.IMAGE_STEP, 26)

        xPos = 0

        if app.ENABLE_GROWTH_PET_SYSTEM:
            tempDict = {}
            for value in self.petSkillaffectImageDict.values():
                tempDict[value[0]] = value[1]

            for value in range(1, 4):
                if value in tempDict:
                    tempDict[value].SetPosition(xPos, 0)
                    xPos += self.IMAGE_STEP

        if self.lovePointImage:
            if self.lovePointImage.IsShow():
                self.lovePointImage.SetPosition(xPos, 0)
                xPos += self.IMAGE_STEP

        if self.horseImage:
            self.horseImage.SetPosition(xPos, 0)
            xPos += self.IMAGE_STEP

        if self.dropStatus:
            self.dropStatus.SetPosition(xPos, 0)
            xPos += self.IMAGE_STEP

        if self.expBonusImage:
            self.expBonusImage.SetPosition(xPos, 0)
            xPos += self.IMAGE_STEP

        for image in self.affectImageDict.values():
            image.SetPosition(xPos, 0)
            xPos += self.IMAGE_STEP

        for image in self.toggleImageDict.values():
            if image:
                image.SetPosition(xPos, 0)
                xPos += self.IMAGE_STEP

        if offlineShopInfoIcon and offlineShopInfoIcon.IsShow():
            offlineShopInfoIcon.SetPosition(xPos, 0)
            offlineShopInfoIcon.SetTop()
            xPos += self.IMAGE_STEP

    if app.ENABLE_GROWTH_PET_SYSTEM:
        def SetPetSkillAffect(self, index, affect):

            if self.__AppendPetSkillAffect(index, affect):
                self.__ArrangeImageList()

    if app.ENABLE_GROWTH_PET_SYSTEM:
        def ClearPetSkillAffect(self):
            self.petSkillaffectImageDict.clear()
            self.__ArrangeImageList()

    if app.ENABLE_GROWTH_PET_SYSTEM:
        def __AppendPetSkillAffect(self, index, affect):

            if affect in self.petSkillaffectImageDict:
                return False

            filename = skill.GetPetSkillIconPath(affect)
            if "" == filename:
                return False

            (pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time) = skill.GetPetSkillInfo(affect)

            image = GrowthPetImage()
            image.SetParent(self)

            try:
                image.LoadImage(filename)
            except Exception as e:
                return False

            image.SetToolTipText(pet_skill_name, 0, 40)

            image.SetDescription(pet_skill_desc)

            image.SetScale(0.7, 0.7)

            image.Show()

            self.petSkillaffectImageDict[affect] = [index, image]

            return True

    def __TryDeleteToggleItem(self, cell):
        # We don't have an image for this item...
        # i.e. nothing to do
        if cell not in self.toggleImageDict:
            return

        self.toggleImageDict[cell] = None
        del self.toggleImageDict[cell]
