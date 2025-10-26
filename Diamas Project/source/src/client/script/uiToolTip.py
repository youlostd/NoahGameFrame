# -*- coding: utf-8 -*-


import logging
from _weakref import proxy

import app
import background
import chr
import exchange
import grp
import grpText
import item
import nonplayer
import player
import safebox
import shop
import skill
import wndMgr
from pygame.app import appInst
from pygame.exchange import exchangeInst
from pygame.item import MakeItemPosition, itemManager
from pygame.player import playerInst
from pygame.safebox import safeboxInst
from pygame.shop import shopInst

import constInfo
import localeInfo
import ui
import uiPrivateShopBuilder
from emoji_config import ReplaceEmoticons
from playerSettingModule import GetOtherRaceByOwnSex, JOB_SHAMAN, JOB_ASSASSIN

WARP_SCROLLS = [22011, 22000, 22010]

DESC_DEFAULT_MAX_COLS = 26
DESC_WESTERN_MAX_COLS = 35
DESC_WESTERN_MAX_WIDTH = 220


def chop(n):
    return round(n - 0.5, 1)


def SplitDescription(desc, limit):
    total_tokens = desc.split()
    line_tokens = []
    line_len = 0
    lines = []
    for token in total_tokens:
        if "|" in token:
            sep_pos = token.find("|")
            line_tokens.append(token[:sep_pos])

            lines.append(" ".join(line_tokens))
            line_len = len(token) - (sep_pos + 1)
            line_tokens = [token[sep_pos + 1 :]]
        elif app.WJ_MULTI_TEXTLINE and "\\n" in token:
            sep_pos = token.find("\\n")
            line_tokens.append(token[:sep_pos])

            lines.append(" ".join(line_tokens))
            line_len = len(token) - (sep_pos + 2)
            line_tokens = [token[sep_pos + 2 :]]
        else:
            line_len += len(token)
            if len(line_tokens) + line_len > limit:
                lines.append(" ".join(line_tokens))
                line_len = len(token)
                line_tokens = [token]
            else:
                line_tokens.append(token)

    if line_tokens:
        lines.append(" ".join(line_tokens))

    return lines


class ToolTip(ui.ThinBoardOld):
    TOOL_TIP_WIDTH = 190
    TOOL_TIP_HEIGHT = 10

    TEXT_LINE_HEIGHT = 17

    TITLE_COLOR = grp.GenerateColor(0.9490, 0.9058, 0.7568, 1.0)
    SPECIAL_TITLE_COLOR = grp.GenerateColor(1.0, 0.7843, 0.0, 1.0)
    NORMAL_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
    FONT_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
    PRICE_COLOR = 0xFFFFB96D

    HIGH_PRICE_COLOR = SPECIAL_TITLE_COLOR
    MIDDLE_PRICE_COLOR = grp.GenerateColor(0.85, 0.85, 0.85, 1.0)
    LOW_PRICE_COLOR = grp.GenerateColor(0.7, 0.7, 0.7, 1.0)

    ENABLE_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
    DISABLE_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)

    NEGATIVE_COLOR = grp.GenerateColor(0.9, 0.4745, 0.4627, 1.0)
    POSITIVE_COLOR = grp.GenerateColor(0.5411, 0.7254, 0.5568, 1.0)
    SPECIAL_POSITIVE_COLOR = grp.GenerateColor(0.6911, 0.8754, 0.7068, 1.0)
    SPECIAL_POSITIVE_COLOR2 = grp.GenerateColor(0.8824, 0.9804, 0.8824, 1.0)
    TEXTLINE_2ND_COLOR_DEFAULT = grp.GenerateColor(1.0, 1.0, 0.6078, 1.0)

    CONDITION_COLOR = 0xFFBEB47D
    CAN_LEVEL_UP_COLOR = 0xFF8EC292
    CANNOT_LEVEL_UP_COLOR = DISABLE_COLOR
    NEED_SKILL_POINT_COLOR = 0xFF9A9CDB

    if app.ENABLE_CHANGE_LOOK_SYSTEM:
        CHANGELOOK_TITLE_COLOR = 0xFF8BBDFF
        CHANGELOOK_ITEMNAME_COLOR = 0xFFBCE55C

    def __init__(self, width=TOOL_TIP_WIDTH, isPickable=False):
        super(ToolTip, self).__init__("TOP_MOST")
        self.SetWindowName(self.__class__.__name__)
        if isPickable:
            pass
        else:
            self.AddFlag("not_pick")

        self.AddFlag("float")

        self.followFlag = True
        self.modernFollow = False
        self.toolTipWidth = width
        self.toolTipHeight = 12
        self.childrenList = []

        self.xPos = -1
        self.yPos = -1

        self.itemIndex = 0

        self.defFontName = localeInfo.UI_DEF_FONT

    def ClearToolTip(self):
        self.toolTipHeight = 12
        self.childrenList = []

    def SetFollow(self, flag):
        self.followFlag = flag

    def SetDefaultFontName(self, fontName):
        self.defFontName = fontName

    def AppendSpace(self, size):
        self.toolTipHeight += size
        self.ResizeToolTip()

    def AppendHorizontalLine(self):
        for i in xrange(2):
            horizontalLine = ui.Line()
            horizontalLine.SetParent(self)
            horizontalLine.SetPosition(0, self.toolTipHeight + 3 + i)
            horizontalLine.SetHorizontalAlignCenter()
            horizontalLine.SetSize(150, 0)
            horizontalLine.Show()

            if 0 == i:
                horizontalLine.SetColor(0xFF555555)
            else:
                horizontalLine.SetColor(0xFF000000)

            self.childrenList.append(horizontalLine)

        self.toolTipHeight += 11
        self.ResizeToolTip()

    if app.NEW_SELECT_CHARACTER:

        def SetThinBoardSize(self, width, height=12):
            self.toolTipWidth = width
            self.toolTipHeight = height

    def AppendTextLine(self, text, color=FONT_COLOR, centerAlign=True):
        textLine = ui.TextLine()
        textLine.SetParent(self)
        textLine.SetFontName(self.defFontName)
        textLine.SetPackedFontColor(color)
        textLine.SetText(text)
        textLine.SetOutline()
        textLine.SetPosition(0, self.toolTipHeight)
        if centerAlign:
            textLine.SetHorizontalAlignCenter()
        textLine.Show()

        self.childrenList.append(textLine)

        textWidth = textLine.GetWidth() + 40
        textHeight = textLine.GetHeight() + 5

        if self.toolTipWidth < textWidth:
            self.toolTipWidth = textWidth

        self.toolTipHeight += max(textHeight, self.TEXT_LINE_HEIGHT)
        self.ResizeToolTip()
        return textLine

    def AppendTwoColorTextLine(
        self, text, color, text2, color2=TEXTLINE_2ND_COLOR_DEFAULT, centerAlign=True
    ):
        textLine = ui.TextLine()
        textLine.SetParent(self)
        textLine.SetFontName(self.defFontName)
        textLine.SetPackedFontColor(color)
        textLine.SetText(text)
        textLine.SetOutline()
        textLine.SetPosition(0, self.toolTipHeight)
        if centerAlign:
            textLine.SetHorizontalAlignCenter()
        textLine.Show()

        textLine2 = ui.TextLine()
        textLine2.SetParent(textLine)
        textLine2.SetFontName(self.defFontName)
        textLine2.SetPackedFontColor(color2)
        textLine2.SetText(text2)
        textLine2.SetOutline()
        textLine2.SetPosition(-textLine2.GetWidth() - 2, 0)
        textLine2.SetHorizontalAlignRight()
        textLine2.Show()
        textLine.SetPosition(-textLine2.GetWidth(), self.toolTipHeight)

        self.childrenList.append(textLine)
        self.childrenList.append(textLine2)

        textWidth = textLine.GetWidth() + textLine2.GetWidth() + 30 + 40
        textHeight = textLine.GetHeight() + 5

        if self.toolTipWidth < textWidth:
            self.toolTipWidth = textWidth

        self.toolTipHeight += max(textHeight, self.TEXT_LINE_HEIGHT)
        self.ResizeToolTip()
        return textLine

    def AppendRenderTarget(
        self, data, height=290, width=400, fov=15.0, targetZ=0.0, hasRotation=True
    ):
        renderTarget = ui.ItemRenderTarget()
        renderTarget.SetParent(self)
        renderTarget.SetHorizontalAlignCenter()
        renderTarget.SetPosition(0, self.toolTipHeight)
        renderTarget.SetSize(height, width)
        renderTarget.SetRenderTarget(0)
        renderTarget.SetData(data)
        renderTarget.SetFov(fov)
        renderTarget.SetTargetZ(targetZ)
        renderTarget.EnableRotation(hasRotation)
        renderTarget.Show()

        self.childrenList.append(renderTarget)

        textWidth = renderTarget.GetWidth() + 40
        textHeight = renderTarget.GetHeight() + 5

        if self.toolTipWidth < textWidth:
            self.toolTipWidth = textWidth

        self.toolTipHeight += textHeight
        self.ResizeToolTip()
        return renderTarget

    def AppendImage(self, image, centerAlign=True, yChange=0):
        imageBox = ui.ImageBox()
        imageBox.SetParent(self)
        imageBox.LoadImage(image)
        imageBox.Show()

        if centerAlign:
            imageBox.SetPosition(
                (self.toolTipWidth - imageBox.GetWidth()) / 2,
                self.toolTipHeight + yChange,
            )

        else:
            imageBox.SetPosition(10, self.toolTipHeight + yChange)

        self.childrenList.append(imageBox)

        self.toolTipHeight += imageBox.GetHeight() + 2 + yChange
        self.ResizeToolTip()

        return imageBox

    def AppendDescription(self, desc, limit, color=FONT_COLOR):
        self.__AppendDescription_WesternLanguage(desc, color)

    def __AppendDescription_EasternLanguage(
        self, description, characterLimitation, color=FONT_COLOR
    ):
        length = len(description)
        if 0 == length:
            return

        self.AppendSpace(5)

        for line in grpText.GetLines(description, characterLimitation):
            self.AppendTextLine(line, color)

    def __AppendDescription_WesternLanguage(self, desc, color=FONT_COLOR):
        lines = SplitDescription(desc, DESC_WESTERN_MAX_COLS)
        if not lines:
            return

        self.AppendSpace(5)

        for line in lines:
            self.AppendTextLine(line, color)

    def ResizeToolTipText(self, x, y):
        self.SetSize(x, y)

    def ResizeToolTip(self):
        self.SetSize(self.toolTipWidth, self.TOOL_TIP_HEIGHT + self.toolTipHeight)

    def SetTitle(self, name):
        self.AppendTextLine(name, self.TITLE_COLOR)

    def GetLimitTextLineColor(self, curValue, limitValue):
        if curValue < limitValue:
            return self.DISABLE_COLOR

        return self.ENABLE_COLOR

    def GetChangeTextLineColor(self, value, isSpecial=False):
        if float(value) > 0.0:
            if isSpecial:
                return self.SPECIAL_POSITIVE_COLOR
            else:
                return self.POSITIVE_COLOR

        if 0.0 == float(value):
            return self.NORMAL_COLOR

        return self.NEGATIVE_COLOR

    def SetToolTipPosition(self, x=-1, y=-1):
        self.xPos = x
        self.yPos = y

    def RectSize(self, width, height):
        self.toolTipHeight = int(height)
        self.toolTipWidth = int(width)
        self.ResizeToolTip()
        self.UpdateRect()

    def ShowToolTip(self):
        self.SetTop()
        self.Show()
        active = wndMgr.GetFocus()
        if not isinstance(active, ui.EditLine):
            self.SetFocus()

        self.OnUpdate()

    def HideToolTip(self):
        self.Hide()

    def OnUpdate(self):

        if not self.followFlag:
            return

        x = 0
        y = 0
        width = self.GetWidth()
        height = self.toolTipHeight

        if -1 == self.xPos and -1 == self.yPos:

            (mouseX, mouseY) = wndMgr.GetMousePosition()

            if mouseY < wndMgr.GetScreenHeight() - 300:
                y = mouseY + 40
            else:
                y = mouseY - height - 30

            x = mouseX - width / 2

        else:

            x = self.xPos - width / 2
            y = self.yPos - height

        x = max(x, 0)
        y = max(y, 0)
        x = min(x + width / 2, wndMgr.GetScreenWidth() - width / 2) - width / 2
        y = min(y + self.GetHeight(), wndMgr.GetScreenHeight()) - self.GetHeight()

        parentWindow = self.GetParentProxy()
        if parentWindow:
            (gx, gy) = parentWindow.GetGlobalPosition()
            x -= width
            x -= gx
            y -= gy

        self.SetPosition(x, y)


class ItemToolTip(ToolTip):
    CHARACTER_NAMES = (
        "(warrior_m)",
        "(assassin_w)",
        "(sura_m)",
        "(shaman_w)",
    )
    if app.ENABLE_WOLFMAN_CHARACTER:
        CHARACTER_NAMES += ("(wolfman_m)",)

    CHARACTER_COUNT = len(CHARACTER_NAMES)
    ANTI_FLAG_NAMES = (
        localeInfo.TOOLTIP_ANTIFLAG_DROP,  # Drop
        localeInfo.TOOLTIP_ANTIFLAG_SELL,  # Sell
        localeInfo.TOOLTIP_ANTIFLAG_GIVE,  # Trade
        localeInfo.TOOLTIP_ANTIFLAG_PKDROP,  # PvP Drop
        localeInfo.TOOLTIP_ANTIFLAG_MYSHOP,  # Sell private shop
        localeInfo.TOOLTIP_ANTIFLAG_SAFEBOX,  # Store
        localeInfo.TOOLTIP_ANTIFLAG_CHANGE_ATTRIBUTE,  # Change attributes
        localeInfo.TOOLTIP_ANTIFLAG_DESTROY,  # Destroy
        localeInfo.TOOLTIP_ANTIFLAG_CHANGELOOK,  # Changelook
    )

    ANTI_FLAG_COUNT = len(ANTI_FLAG_NAMES)
    WEAR_NAMES = (
        localeInfo.TOOLTIP_ARMOR,
        localeInfo.TOOLTIP_HELMET,
        localeInfo.TOOLTIP_SHOES,
        localeInfo.TOOLTIP_WRISTLET,
        localeInfo.TOOLTIP_WEAPON,
        localeInfo.TOOLTIP_NECK,
        localeInfo.TOOLTIP_EAR,
        localeInfo.TOOLTIP_UNIQUE,
        localeInfo.TOOLTIP_SHIELD,
        localeInfo.TOOLTIP_ARROW,
    )
    WEAR_COUNT = len(WEAR_NAMES)

    AFFECT_DICT = {
        item.APPLY_MAX_HP: localeInfo.TOOLTIP_MAX_HP,
        item.APPLY_MAX_SP: localeInfo.TOOLTIP_MAX_SP,
        item.APPLY_CON: localeInfo.TOOLTIP_CON,
        item.APPLY_INT: localeInfo.TOOLTIP_INT,
        item.APPLY_STR: localeInfo.TOOLTIP_STR,
        item.APPLY_DEX: localeInfo.TOOLTIP_DEX,
        item.APPLY_ATT_SPEED: localeInfo.TOOLTIP_ATT_SPEED,
        item.APPLY_MOV_SPEED: localeInfo.TOOLTIP_MOV_SPEED,
        item.APPLY_CAST_SPEED: localeInfo.TOOLTIP_CAST_SPEED,
        item.APPLY_HP_REGEN: localeInfo.TOOLTIP_HP_REGEN,
        item.APPLY_SP_REGEN: localeInfo.TOOLTIP_SP_REGEN,
        item.APPLY_POISON_PCT: localeInfo.TOOLTIP_APPLY_POISON_PCT,
        item.APPLY_STUN_PCT: localeInfo.TOOLTIP_APPLY_STUN_PCT,
        item.APPLY_SLOW_PCT: localeInfo.TOOLTIP_APPLY_SLOW_PCT,
        item.APPLY_CRITICAL_PCT: localeInfo.TOOLTIP_APPLY_CRITICAL_PCT,
        item.APPLY_PENETRATE_PCT: localeInfo.TOOLTIP_APPLY_PENETRATE_PCT,
        item.APPLY_ATTBONUS_WARRIOR: localeInfo.TOOLTIP_APPLY_ATTBONUS_WARRIOR,
        item.APPLY_ATTBONUS_ASSASSIN: localeInfo.TOOLTIP_APPLY_ATTBONUS_ASSASSIN,
        item.APPLY_ATTBONUS_SURA: localeInfo.TOOLTIP_APPLY_ATTBONUS_SURA,
        item.APPLY_ATTBONUS_SHAMAN: localeInfo.TOOLTIP_APPLY_ATTBONUS_SHAMAN,
        item.APPLY_ATTBONUS_MONSTER: localeInfo.TOOLTIP_APPLY_ATTBONUS_MONSTER,
        item.APPLY_ATTBONUS_HUMAN: localeInfo.TOOLTIP_APPLY_ATTBONUS_HUMAN,
        item.APPLY_ATTBONUS_ANIMAL: localeInfo.TOOLTIP_APPLY_ATTBONUS_ANIMAL,
        item.APPLY_ATTBONUS_ORC: localeInfo.TOOLTIP_APPLY_ATTBONUS_ORC,
        item.APPLY_ATTBONUS_MILGYO: localeInfo.TOOLTIP_APPLY_ATTBONUS_MILGYO,
        item.APPLY_ATTBONUS_UNDEAD: localeInfo.TOOLTIP_APPLY_ATTBONUS_UNDEAD,
        item.APPLY_ATTBONUS_DEVIL: localeInfo.TOOLTIP_APPLY_ATTBONUS_DEVIL,
        item.APPLY_ATTBONUS_INSECT: localeInfo.TOOLTIP_APPLY_ATTBONUS_ARACHNID,
        item.APPLY_STEAL_HP: localeInfo.TOOLTIP_APPLY_STEAL_HP,
        item.APPLY_STEAL_SP: localeInfo.TOOLTIP_APPLY_STEAL_SP,
        item.APPLY_MANA_BURN_PCT: localeInfo.TOOLTIP_APPLY_MANA_BURN_PCT,
        item.APPLY_DAMAGE_SP_RECOVER: localeInfo.TOOLTIP_APPLY_DAMAGE_SP_RECOVER,
        item.APPLY_BLOCK: localeInfo.TOOLTIP_APPLY_BLOCK,
        item.APPLY_DODGE: localeInfo.TOOLTIP_APPLY_DODGE,
        item.APPLY_RESIST_SWORD: localeInfo.TOOLTIP_APPLY_RESIST_SWORD,
        item.APPLY_RESIST_TWOHAND: localeInfo.TOOLTIP_APPLY_RESIST_TWOHAND,
        item.APPLY_RESIST_DAGGER: localeInfo.TOOLTIP_APPLY_RESIST_DAGGER,
        item.APPLY_RESIST_BELL: localeInfo.TOOLTIP_APPLY_RESIST_BELL,
        item.APPLY_RESIST_FAN: localeInfo.TOOLTIP_APPLY_RESIST_FAN,
        item.APPLY_RESIST_BOW: localeInfo.TOOLTIP_RESIST_BOW,
        item.APPLY_RESIST_FIRE: localeInfo.TOOLTIP_RESIST_FIRE,
        item.APPLY_RESIST_ELEC: localeInfo.TOOLTIP_RESIST_ELEC,
        item.APPLY_RESIST_MAGIC: localeInfo.TOOLTIP_RESIST_MAGIC,
        item.APPLY_RESIST_WIND: localeInfo.TOOLTIP_APPLY_RESIST_WIND,
        item.APPLY_REFLECT_MELEE: localeInfo.TOOLTIP_APPLY_REFLECT_MELEE,
        item.APPLY_REFLECT_CURSE: localeInfo.TOOLTIP_APPLY_REFLECT_CURSE,
        item.APPLY_POISON_REDUCE: localeInfo.TOOLTIP_APPLY_POISON_REDUCE,
        item.APPLY_KILL_SP_RECOVER: localeInfo.TOOLTIP_APPLY_KILL_SP_RECOVER,
        item.APPLY_EXP_DOUBLE_BONUS: localeInfo.TOOLTIP_APPLY_EXP_DOUBLE_BONUS,
        item.APPLY_GOLD_DOUBLE_BONUS: localeInfo.TOOLTIP_APPLY_GOLD_DOUBLE_BONUS,
        item.APPLY_ITEM_DROP_BONUS: localeInfo.TOOLTIP_APPLY_ITEM_DROP_BONUS,
        item.APPLY_POTION_BONUS: localeInfo.TOOLTIP_APPLY_POTION_BONUS,
        item.APPLY_KILL_HP_RECOVER: localeInfo.TOOLTIP_APPLY_KILL_HP_RECOVER,
        item.APPLY_IMMUNE_STUN: localeInfo.TOOLTIP_APPLY_IMMUNE_STUN,
        item.APPLY_IMMUNE_SLOW: localeInfo.TOOLTIP_APPLY_IMMUNE_SLOW,
        item.APPLY_IMMUNE_FALL: localeInfo.TOOLTIP_APPLY_IMMUNE_FALL,
        item.APPLY_BOW_DISTANCE: localeInfo.TOOLTIP_BOW_DISTANCE,
        item.APPLY_DEF_GRADE_BONUS: localeInfo.TOOLTIP_DEF_GRADE,
        item.APPLY_ATT_GRADE_BONUS: localeInfo.TOOLTIP_ATT_GRADE,
        item.APPLY_MAGIC_ATT_GRADE: localeInfo.TOOLTIP_MAGIC_ATT_GRADE,
        item.APPLY_MAGIC_DEF_GRADE: localeInfo.TOOLTIP_MAGIC_DEF_GRADE,
        item.APPLY_MAX_STAMINA: localeInfo.TOOLTIP_MAX_STAMINA,
        item.APPLY_MALL_ATTBONUS: localeInfo.TOOLTIP_MALL_ATTBONUS,
        item.APPLY_MALL_DEFBONUS: localeInfo.TOOLTIP_MALL_DEFBONUS,
        item.APPLY_MALL_EXPBONUS: localeInfo.TOOLTIP_MALL_EXPBONUS,
        item.APPLY_MALL_ITEMBONUS: localeInfo.TOOLTIP_MALL_ITEMBONUS,
        item.APPLY_MALL_GOLDBONUS: localeInfo.TOOLTIP_MALL_GOLDBONUS,
        item.APPLY_SKILL_DAMAGE_BONUS: localeInfo.TOOLTIP_SKILL_DAMAGE_BONUS,
        item.APPLY_NORMAL_HIT_DAMAGE_BONUS: localeInfo.TOOLTIP_NORMAL_HIT_DAMAGE_BONUS,
        item.APPLY_SKILL_DEFEND_BONUS: localeInfo.TOOLTIP_SKILL_DEFEND_BONUS,
        item.APPLY_NORMAL_HIT_DEFEND_BONUS: localeInfo.TOOLTIP_NORMAL_HIT_DEFEND_BONUS,
        item.APPLY_PC_BANG_EXP_BONUS: localeInfo.TOOLTIP_MALL_EXPBONUS_P_STATIC,
        item.APPLY_PC_BANG_DROP_BONUS: localeInfo.TOOLTIP_MALL_ITEMBONUS_P_STATIC,
        item.APPLY_RESIST_WARRIOR: localeInfo.TOOLTIP_APPLY_RESIST_WARRIOR,
        item.APPLY_RESIST_ASSASSIN: localeInfo.TOOLTIP_APPLY_RESIST_ASSASSIN,
        item.APPLY_RESIST_SURA: localeInfo.TOOLTIP_APPLY_RESIST_SURA,
        item.APPLY_RESIST_SHAMAN: localeInfo.TOOLTIP_APPLY_RESIST_SHAMAN,
        item.APPLY_MAX_HP_PCT: localeInfo.TOOLTIP_APPLY_MAX_HP_PCT,
        item.APPLY_MAX_SP_PCT: localeInfo.TOOLTIP_APPLY_MAX_SP_PCT,
        item.APPLY_ENERGY: localeInfo.TOOLTIP_ENERGY,
        item.APPLY_COSTUME_ATTR_BONUS: localeInfo.TOOLTIP_COSTUME_ATTR_BONUS,
        item.APPLY_MAGIC_ATTBONUS_PER: localeInfo.TOOLTIP_MAGIC_ATTBONUS_PER,
        item.APPLY_MELEE_MAGIC_ATTBONUS_PER: localeInfo.TOOLTIP_MELEE_MAGIC_ATTBONUS_PER,
        item.APPLY_RESIST_ICE: localeInfo.TOOLTIP_RESIST_ICE,
        item.APPLY_RESIST_EARTH: localeInfo.TOOLTIP_RESIST_EARTH,
        item.APPLY_RESIST_DARK: localeInfo.TOOLTIP_RESIST_DARK,
        item.APPLY_ANTI_CRITICAL_PCT: localeInfo.TOOLTIP_ANTI_CRITICAL_PCT,
        item.APPLY_ANTI_PENETRATE_PCT: localeInfo.TOOLTIP_ANTI_PENETRATE_PCT,
        item.APPLY_ACCEDRAIN_RATE: localeInfo.TOOLTIP_APPLY_ACCEDRAIN_RATE,
        item.APPLY_BLEEDING_PCT: localeInfo.TOOLTIP_APPLY_BLEEDING_PCT,
        item.APPLY_BLEEDING_REDUCE: localeInfo.TOOLTIP_APPLY_BLEEDING_REDUCE,
        item.APPLY_ATTBONUS_WOLFMAN: localeInfo.TOOLTIP_APPLY_ATTBONUS_WOLFMAN,
        item.APPLY_RESIST_WOLFMAN: localeInfo.TOOLTIP_APPLY_RESIST_WOLFMAN,
        item.APPLY_RESIST_HUMAN: localeInfo.TOOLTIP_APPLY_RESIST_HUMAN,
        item.APPLY_RESIST_CLAW: localeInfo.TOOLTIP_APPLY_RESIST_CLAW,
        item.APPLY_ATTBONUS_METIN: localeInfo.TOOLTIP_APPLY_ATTBONUS_METIN,
        item.APPLY_ATTBONUS_TRENT: localeInfo.TOOLTIP_APPLY_ATTBONUS_TRENT,
        item.APPLY_ATTBONUS_BOSS: localeInfo.TOOLTIP_APPLY_ATTBONUS_BOSS,
        item.APPLY_ATTBONUS_SHADOW: localeInfo.Get("TOOLTIP_APPLY_ATTBONUS_SHADOW"),
    }

    ATTRIBUTE_NEED_WIDTH = {
        23: 230,
        24: 230,
        25: 230,
        26: 220,
        27: 210,
        35: 210,
        36: 210,
        37: 210,
        38: 210,
        39: 210,
        40: 210,
        41: 210,
        42: 220,
        43: 230,
        45: 230,
    }

    if app.ENABLE_WOLFMAN_CHARACTER:
        ANTI_FLAG_DICT = {
            0: item.ITEM_ANTIFLAG_WARRIOR,
            1: item.ITEM_ANTIFLAG_ASSASSIN,
            2: item.ITEM_ANTIFLAG_SURA,
            3: item.ITEM_ANTIFLAG_SHAMAN,
            4: item.ITEM_ANTIFLAG_WOLFMAN,
        }
    else:
        ANTI_FLAG_DICT = {
            0: item.ITEM_ANTIFLAG_WARRIOR,
            1: item.ITEM_ANTIFLAG_ASSASSIN,
            2: item.ITEM_ANTIFLAG_SURA,
            3: item.ITEM_ANTIFLAG_SHAMAN,
        }

    FONT_COLOR = grp.GenerateColor(0.7607, 0.7607, 0.7607, 1.0)
    CHANGELOOK_TITLE_COLOR = 0xFF8BBDFF
    CHANGELOOK_ITEMNAME_COLOR = 0xFFBCE55C

    def __init__(self, *args, **kwargs):
        super(ItemToolTip, self).__init__(*args, **kwargs)
        self.SetWindowName(self.__class__.__name__)
        self.itemVnum = 0
        self.isShopItem = False
        self.isSwitchbotItem = False
        self.slotIndex = -1
        self.updateEndTime = app.GetGlobalTimeStamp() + 50
        self.attributeSlots = []
        # ??? ??? ??? ? ?? ???? ??? ? ?? ?????? ??? Disable Color? ?? (?? ??? ???? ??? ?? ? ??? ???)
        self.mallLastTime = None
        self.mallEndTime = 1
        self.checkUsability = True
        self.renderTarget = None
        self.levelPet = None
        self.interface = None
        self.attributeSlots = []
        self.metinSlot = None

    def ClearToolTip(self):
        self.toolTipHeight = 12
        self.childrenList = []
        self.attributeSlots = []
        self.mallLastTime = None
        self.mallEndTime = 1
        self.checkUsability = True
        self.renderTarget = None
        self.levelPet = None
        self.interface = None
        self.itemVnum = 0
        self.isShopItem = False
        self.isSwitchbotItem = False
        self.slotIndex = -1
        self.metinSlot = None

    def BindInterface(self, interface):
        self.interface = interface

    def CheckUsability(self, enable):
        self.checkUsability = enable

    def CanEquip(self, ignoreLevelCheck=False, ignoreSex=False):

        race = player.GetRace()
        job = chr.RaceToJob(race)

        if job not in self.ANTI_FLAG_DICT:
            return False

        if item.IsAntiFlag(self.ANTI_FLAG_DICT[job]):
            return False

        sex = chr.RaceToSex(race)

        MALE = 0
        FEMALE = 1

        if not ignoreSex:
            if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE) and sex == MALE:
                return False

            if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE) and sex == FEMALE:
                return False

        if not ignoreLevelCheck:
            i = 0
            for i in xrange(item.LIMIT_MAX_NUM):
                try:
                    limitType, limitValue = item.GetLimit(i)
                except TypeError:
                    break
                i += 1
                if item.LIMIT_LEVEL == limitType:
                    if playerInst().GetPoint(player.LEVEL) < limitValue:
                        return False
                if item.LIMIT_MAX_LEVEL == limitType:
                    if playerInst().GetPoint(player.LEVEL) > limitValue:
                        return False
                if item.LIMIT_MAP == limitType:
                    if background.GetCurrentMapIndex() != limitValue:
                        return False
                """
                elif item.LIMIT_STR == limitType:
                    if playerInst().GetPoint(player.ST) < limitValue:
                        return False
                elif item.LIMIT_DEX == limitType:
                    if playerInst().GetPoint(player.DX) < limitValue:
                        return False
                elif item.LIMIT_INT == limitType:
                    if playerInst().GetPoint(player.IQ) < limitValue:
                        return False
                elif item.LIMIT_CON == limitType:
                    if playerInst().GetPoint(player.HT) < limitValue:
                        return False
                """
        if (
            item.GetItemType() == item.ITEM_TYPE_TOGGLE
            and item.GetItemSubType() == item.TOGGLE_LEVEL_PET
        ):
            if self.metinSlot is not None and isinstance(self.metinSlot, list):
                petLevel = self.metinSlot[0]
                if petLevel > playerInst().GetPoint(player.LEVEL):
                    return False

        return True

    if app.ENABLE_ACCE_COSTUME_SYSTEM:

        def AppendTextLineAcce(self, text, color=FONT_COLOR, centerAlign=True):
            return ToolTip.AppendTextLine(self, text, color, centerAlign)

    def AppendTextLine(self, text, color=FONT_COLOR, centerAlign=True):
        if self.checkUsability and not self.CanEquip():
            color = self.DISABLE_COLOR

        return ToolTip.AppendTextLine(self, text, color, centerAlign)

    def AppendRenderTarget(
        self, data, height=190, width=200, fov=10.0, targetZ=0.0, hasRotation=True
    ):
        return ToolTip.AppendRenderTarget(
            self, data, height, width, fov, targetZ, hasRotation
        )

    def ClearToolTip(self):
        self.windowType = 0
        self.mallLastTime = None
        self.renderTarget = None
        self.mallEndTime = 1
        self.isShopItem = False

        self.toolTipWidth = self.TOOL_TIP_WIDTH
        self.toolTipHeight = 12
        self.childrenList = []

    if app.ENABLE_CHANGE_LOOK_SYSTEM:

        def AppendChangeLookInfoItemVnum(self, changelookvnum):
            if not changelookvnum == 0:
                self.AppendSpace(5)
                self.AppendTextLine(
                    "[ " + localeInfo.CHANGE_LOOK_TITLE + " ]",
                    self.CHANGELOOK_TITLE_COLOR,
                )
                itemName = item.GetItemNameByVnum(changelookvnum)

                if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                    if item.GetItemSubType() == item.COSTUME_TYPE_BODY:
                        malefemale = ""
                        if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
                            malefemale = localeInfo.FOR_FEMALE

                        if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
                            malefemale = localeInfo.FOR_MALE
                        itemName += " ( " + malefemale + " )"

                textLine = self.AppendTextLine(
                    itemName, self.CHANGELOOK_ITEMNAME_COLOR, True
                )

    def AppendChangeLookInfoPrivateShopWIndow(self, slotIndex):
        changelookvnum = shop.GetPrivateShopItemChangeLookVnum(slotIndex)
        if not changelookvnum == 0:
            self.AppendSpace(5)
            self.AppendTextLine(
                "[ " + localeInfo.CHANGE_LOOK_TITLE + " ]", self.CHANGELOOK_TITLE_COLOR
            )
            itemName = item.GetItemNameByVnum(changelookvnum)

            if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                if item.GetItemSubType() == item.COSTUME_TYPE_BODY:
                    malefemale = ""
                    if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
                        malefemale = localeInfo.FOR_FEMALE

                    if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
                        malefemale = localeInfo.FOR_MALE
                    itemName += " ( " + malefemale + " )"

            textLine = self.AppendTextLine(
                itemName, self.CHANGELOOK_ITEMNAME_COLOR, True
            )

    def AppendChangeLookInfoShopWIndow(self, slotIndex):
        changelookvnum = shop.GetItemChangeLookVnum(slotIndex)
        if not changelookvnum == 0:
            self.AppendSpace(5)
            self.AppendTextLine(
                "[ " + localeInfo.CHANGE_LOOK_TITLE + " ]", self.CHANGELOOK_TITLE_COLOR
            )
            itemName = item.GetItemNameByVnum(changelookvnum)

            if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                if item.GetItemSubType() == item.COSTUME_TYPE_BODY:
                    malefemale = ""
                    if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
                        malefemale = localeInfo.FOR_FEMALE

                    if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
                        malefemale = localeInfo.FOR_MALE
                    itemName += " ( " + malefemale + " )"

            textLine = self.AppendTextLine(
                itemName, self.CHANGELOOK_ITEMNAME_COLOR, True
            )

    ###
    #  TODO: As soon as we use ItemPosition everywhere we can handle alot more
    #        of tooltip logic in this class
    #        basically I want a "SetItem" function that just takes the ItemPosition
    ###
    def SetInventoryItem(
        self, slotIndex, window_type=player.INVENTORY, isSwitchbotItem=False
    ):
        itemData = playerInst().GetItemData(MakeItemPosition(window_type, slotIndex))
        if not itemData:
            return

        if not itemData.vnum:
            return

        self.isSwitchbotItem = isSwitchbotItem

        self.ClearToolTip()

        if shop.IsOpen():
            if not shop.IsPrivateShop():
                item.SelectItem(itemData.vnum)
                self.AppendSellingPrice(
                    player.GetISellItemPrice(window_type, slotIndex)
                )

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum,
            itemData.sockets,
            attrSlot,
            window_type,
            slotIndex,
            changeLookVnum=itemData.transVnum,
        )

    def UpdateAttributes(self, window_type, slotIndex):
        itemData = playerInst().GetItemData(MakeItemPosition(window_type, slotIndex))
        if not itemData:
            return

        if not itemData.vnum:
            return

        for i, attr in enumerate(self.attributeSlots):
            if 0.0 == itemData.attrs[i].value:
                continue

            affectString = self.__GetAffectString(
                itemData.attrs[i].type, itemData.attrs[i].value
            )
            if affectString:
                affectColor = self.__GetAttributeColor(i, itemData.attrs[i].value)
                attr.SetText(affectString)
                attr.SetPackedFontColor(affectColor)

                textWidth = attr.GetWidth() + 40

                if self.toolTipWidth < textWidth:
                    self.toolTipWidth = textWidth
                self.ResizeToolTip()

    if app.ENABLE_CHANGE_LOOK_SYSTEM:

        def SetChangeLookWindowItem(self, slotIndex):
            inventoryslotindex = playerInst().GetChangeLookItemInvenSlot(slotIndex)
            if inventoryslotindex.cell == player.ITEM_SLOT_COUNT:
                return
            itemVnum = playerInst().GetItemIndex(inventoryslotindex)
            if 0 == itemVnum:
                return
            self.ClearToolTip()
            metinSlot = [
                playerInst().GetItemMetinSocket(inventoryslotindex, i)
                for i in xrange(player.METIN_SOCKET_MAX_NUM)
            ]
            attrSlot = [
                player.GetItemAttribute(
                    inventoryslotindex.windowType, inventoryslotindex.cell, i
                )
                for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)
            ]
            self.AddItemData(
                itemVnum,
                metinSlot,
                attrSlot,
                inventoryslotindex.windowType,
                inventoryslotindex.cell,
            )

    def SetResulItemAttrMove(
        self, baseSlotIndex, materialSlotIndex, window_type=player.INVENTORY
    ):
        baseItemVnum = player.GetItemIndex(window_type, baseSlotIndex)

        if 0 == baseItemVnum:
            return

        materialItemVnum = player.GetItemIndex(window_type, materialSlotIndex)

        if 0 == materialItemVnum:
            return

        self.ClearToolTip()

        metinSlot = [
            player.GetItemMetinSocket(window_type, baseSlotIndex, i)
            for i in xrange(player.METIN_SOCKET_MAX_NUM)
        ]
        attrSlot = [
            player.GetItemAttribute(window_type, materialSlotIndex, i)
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)
        ]

        self.AddItemData(baseItemVnum, metinSlot, attrSlot)

    def SetOfflineShopItem(self, item):
        self.ClearToolTip()
        self.AddItemData(
            item.VNUM, item.SOCKETS, item.ATTRIBUTES, changeLookVnum=item.TRANS_VNUM
        )
        self.AppendPrice(item.PRICE)

    def SetShopItem(self, slotIndex):
        itemData = shopInst().GetItemData(slotIndex)

        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()
        self.isShopItem = True

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum, itemData.sockets, attrSlot, changeLookVnum=itemData.transVnum
        )
        if app.ENABLE_CHANGE_LOOK_SYSTEM:
            self.AppendChangeLookInfoShopWIndow(slotIndex)

        self.AppendSpace(5)
        self.AppendTextLine(
            ReplaceEmoticons(localeInfo.Get("TOOLTIP_KEY_TO_BUY_MULTIPLE_FROM_SHOP")),
            grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
        )

        self.AppendPrice(itemData.price)

        # if app.ENABLE_12ZI:
        #    if shop.IsLimitedItemShop():
        #        count = shop.GetLimitedCount(itemVnum)
        #        if count != 0:
        #            purchaseCount = shop.GetLimitedPurchaseCount(itemVnum)
        #            self.AppendLimitedCount(count, purchaseCount)

    def SetShopItemBySecondaryCoin(self, slotIndex, coinVnum=0):
        itemVnum = shop.GetItemID(slotIndex)
        if 0 == itemVnum:
            return

        price = shop.GetItemPrice(slotIndex)
        self.ClearToolTip()
        self.isShopItem = True

        metinSlot = []
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            metinSlot.append(long(shop.GetItemMetinSocket(slotIndex, i)))
        attrSlot = []
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attrSlot.append(shop.GetItemAttribute(slotIndex, i))

        self.AddItemData(itemVnum, metinSlot, attrSlot)

        self.AppendSpace(5)
        self.AppendTextLine(
            ReplaceEmoticons(localeInfo.Get("TOOLTIP_KEY_TO_BUY_MULTIPLE_FROM_SHOP")),
            grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
        )

        if coinVnum != 0:
            if item.SelectItem(coinVnum):
                self.AppendPriceBySecondaryCoin(
                    price, "|Eitem:{}|e {}".format(coinVnum, item.GetItemName())
                )
        else:
            self.AppendPriceBySecondaryCoin(price, localeInfo.TOOLTIP_TEXT1)

    def SetExchangeOwnerItem(self, slotIndex):
        itemData = exchangeInst().GetItemDataFromSelf(slotIndex)
        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum, itemData.sockets, attrSlot, changeLookVnum=itemData.transVnum
        )

        # if app.ENABLE_CHANGE_LOOK_SYSTEM:
        #    self.AppendChangeLookInfoExchangeWIndow(0, slotIndex)

    if app.ENABLE_CHANGE_LOOK_SYSTEM:

        def AppendChangeLookInfoExchangeWIndow(self, type, slotIndex):
            if type == 0:
                changelookvnum = exchange.GetChangeLookVnumFromSelf(slotIndex)
            elif type == 1:
                changelookvnum = exchange.GetChangeLookVnumFromTarget(slotIndex)
            if not changelookvnum == 0:
                self.AppendSpace(5)
                self.AppendTextLine(
                    "[ " + localeInfo.CHANGE_LOOK_TITLE + " ]",
                    self.CHANGELOOK_TITLE_COLOR,
                )
                itemName = item.GetItemNameByVnum(changelookvnum)

                if item.GetItemType() == item.ITEM_TYPE_COSTUME:
                    if item.GetItemSubType() == item.COSTUME_TYPE_BODY:
                        malefemale = ""
                        if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
                            malefemale = localeInfo.FOR_FEMALE

                        if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
                            malefemale = localeInfo.FOR_MALE
                        itemName += " ( " + malefemale + " )"

                textLine = self.AppendTextLine(
                    itemName, self.CHANGELOOK_ITEMNAME_COLOR, True
                )

    def SetExchangeTargetItem(self, slotIndex):
        itemData = exchangeInst().GetItemDataFromTarget(slotIndex)
        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum, itemData.sockets, attrSlot, changeLookVnum=itemData.transVnum
        )

        # if app.ENABLE_CHANGE_LOOK_SYSTEM:
        #    self.AppendChangeLookInfoExchangeWIndow(1, slotIndex)

    def SetPrivateShopBuilderItem(self, invenType, invenPos, privateShopSlotIndex):
        itemData = playerInst().GetItemData(MakeItemPosition(invenType, invenPos))
        if not itemData:
            return

        if not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum,
            itemData.sockets,
            attrSlot,
            invenType,
            invenPos,
            changeLookVnum=itemData.transVnum,
            shopPrice=shop.GetPrivateShopItemPrice(invenType, invenPos),
        )

    def SetAcceWindowItem(self, slotIndex):
        itemData = playerInst().GetAcceItemData(slotIndex)
        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]
        self.AddItemData(
            itemData.vnum,
            itemData.sockets,
            attrSlot,
            sealDate=itemData.sealDate,
            changeLookVnum=itemData.transVnum,
        )

    def SetSafeBoxItem(self, slotIndex):
        itemData = safeboxInst().GetItemData(slotIndex)
        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum,
            itemData.sockets,
            attrSlot,
            player.MALL,
            slotIndex,
            sealDate=itemData.sealDate,
            changeLookVnum=itemData.transVnum,
        )

    def SetMallItem(self, slotIndex):
        itemData = safeboxInst().GetMallItemData(slotIndex)
        if not itemData or not itemData.vnum:
            return

        self.ClearToolTip()

        attrSlot = [(attr.type, attr.value) for attr in itemData.attrs]

        self.AddItemData(
            itemData.vnum,
            itemData.sockets,
            attrSlot,
            player.MALL,
            slotIndex,
            sealDate=itemData.sealDate,
            changeLookVnum=itemData.transVnum,
        )

    def SetItemToolTip(self, itemVnum):
        self.ClearToolTip()

        metinSlot = [0 for i in xrange(player.METIN_SOCKET_MAX_NUM)]
        attrSlot = [(0, 0) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]

        self.AddItemData(itemVnum, metinSlot, attrSlot)

    def __AppendAttackSpeedInfo(self, item):
        atkSpd = item.GetValue(0)

        if atkSpd < 80:
            stSpd = localeInfo.TOOLTIP_ITEM_VERY_FAST
        elif atkSpd <= 95:
            stSpd = localeInfo.TOOLTIP_ITEM_FAST
        elif atkSpd <= 105:
            stSpd = localeInfo.TOOLTIP_ITEM_NORMAL
        elif atkSpd <= 120:
            stSpd = localeInfo.TOOLTIP_ITEM_SLOW
        else:
            stSpd = localeInfo.TOOLTIP_ITEM_VERY_SLOW

        self.AppendTextLine(
            localeInfo.TOOLTIP_ITEM_ATT_SPEED.format(stSpd), self.NORMAL_COLOR
        )

    def __AppendAttackGradeInfo(self):
        atkGrade = item.GetValue(1)
        self.AppendTextLine(
            localeInfo.TOOLTIP_ITEM_ATT_GRADE.format(atkGrade),
            self.GetChangeTextLineColor(atkGrade),
        )

    def __AppendAttackPowerInfo(self):
        minPower = item.GetValue(3)
        maxPower = item.GetValue(4)
        addPower = item.GetValue(5)

        if maxPower > minPower:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_ATT_POWER.format(
                    minPower + addPower, maxPower + addPower
                ),
                self.POSITIVE_COLOR,
            )
        else:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_ATT_POWER_ONE_ARG.format(minPower + addPower),
                self.POSITIVE_COLOR,
            )

    def __AppendMagicAttackInfo(self):
        minMagicAttackPower = item.GetValue(1)
        maxMagicAttackPower = item.GetValue(2)
        addPower = item.GetValue(5)

        if minMagicAttackPower > 0 or maxMagicAttackPower > 0:
            if maxMagicAttackPower > minMagicAttackPower:
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER.format(
                        minMagicAttackPower + addPower, maxMagicAttackPower + addPower
                    ),
                    self.POSITIVE_COLOR,
                )
            else:
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER_ONE_ARG.format(
                        minMagicAttackPower + addPower
                    ),
                    self.POSITIVE_COLOR,
                )

    def __AppendAttackPowerInfoAcce(self, inDrainPct, itemInDrainVnum):
        item.SelectItem(itemInDrainVnum)
        minPower = self.drainedPercent(item.GetValue(3), inDrainPct)
        maxPower = self.drainedPercent(item.GetValue(4), inDrainPct)
        addPower = self.drainedPercent(item.GetValue(5), inDrainPct)

        if maxPower > minPower:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_ATT_POWER.format(
                    minPower + addPower, maxPower + addPower
                ),
                self.POSITIVE_COLOR,
            )
        else:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_ATT_POWER_ONE_ARG.format(minPower + addPower),
                self.POSITIVE_COLOR,
            )

    def __AppendMagicAttackInfoAcce(self, inDrainPct, itemInDrainVnum):
        item.SelectItem(itemInDrainVnum)
        minMagicAttackPower = self.drainedPercent(item.GetValue(1), inDrainPct)
        maxMagicAttackPower = self.drainedPercent(item.GetValue(2), inDrainPct)
        addPower = self.drainedPercent(item.GetValue(5), inDrainPct)

        if minMagicAttackPower > 0 or maxMagicAttackPower > 0:
            if maxMagicAttackPower > minMagicAttackPower:
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER.format(
                        minMagicAttackPower + addPower, maxMagicAttackPower + addPower
                    ),
                    self.POSITIVE_COLOR,
                )
            else:
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER_ONE_ARG.format(
                        minMagicAttackPower + addPower
                    ),
                    self.POSITIVE_COLOR,
                )

    def __AppendArmorDefenseInfo(self):
        ## 방어력
        defGrade = item.GetValue(1)
        defBonus = item.GetValue(5) * 2

        if defGrade > 0:
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_DEF_GRADE.format(defGrade + defBonus),
                self.GetChangeTextLineColor(defGrade),
            )

    def __AppendMagicDefenceInfo(self):
        magicDefencePower = item.GetValue(0)

        if magicDefencePower > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_MAGIC_DEF_POWER.format(magicDefencePower),
                self.GetChangeTextLineColor(magicDefencePower),
            )

    def __AppendMagicDefenceInfoAcce(self, drainInPercent, itemInDrainVnum):
        item.SelectItem(itemInDrainVnum)
        magicDefencePower = self.drainedPercent(item.GetValue(0), drainInPercent)

        if magicDefencePower > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_MAGIC_DEF_POWER.format(magicDefencePower),
                self.GetChangeTextLineColor(magicDefencePower),
            )

    def __AppendAttributeInformationAcce(
        self, itemVnum, attrSlot, slotIndex, window_type, metinSlot
    ):
        if 0 != attrSlot:

            (affectTypeAcce, affectValueAcce) = item.GetAffect(0)

            if item.GetRefinedVnum() == 0:
                ## Àü¼³µî±Þ Èí¼öÀ² ¾Ë¾Æ¿À±â.
                if window_type == player.INVENTORY:
                    socketInDrainValue = player.GetItemMetinSocket(slotIndex, 0)
                    if not metinSlot[1] == 0 and socketInDrainValue == 0:
                        socketInDrainValue = metinSlot[1]

                elif window_type == player.ACCEREFINE:
                    socketInDrainValue = player.GetAcceItemMetinSocket(slotIndex, 0)
                elif window_type == player.SAFEBOX:
                    socketInDrainValue = safebox.GetItemMetinSocket(slotIndex, 0)

                drainlate = socketInDrainValue / 100.0
            else:
                drainlate = affectValueAcce / 100.0

            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                type = attrSlot[i][0]
                value = attrSlot[i][1]

                if 0 == value:
                    continue

                value = max(((value) * drainlate), 1)
                affectString = self.__GetAffectString(type, value)

                if affectString:
                    affectColor = self.__GetAttributeColor(i, value)
                    self.AppendTextLine(affectString, affectColor)

    def __AppendAttributeInformation(self, attrSlot):
        if attrSlot is not None and 0 != attrSlot:
            self.attributeSlots = []
            for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                attrType = attrSlot[i][0]
                value = float(attrSlot[i][1])

                if 0.0 == value:
                    continue

                affectString = self.__GetAffectString(attrType, value)

                if affectString:
                    affectColor = self.__GetAttributeColor(i, value)
                    self.attributeSlots.append(
                        proxy(self.AppendTextLine(affectString, affectColor))
                    )

    def __AppendDragonSoulAttributeInformation(self, attrSlot, dsType=0, grade=0):
        if 0 != attrSlot:
            logging.debug("test %d", grade)
            if grade != 0:
                for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                    attrType = attrSlot[i][0]
                    value = attrSlot[i][1]

                    if 0 == value:
                        continue

                    affectString = self.__GetAffectString(attrType, value)
                    if affectString:
                        affectColor = self.__GetAttributeColor(i, value)

                        setValue = item.GetDSSetValue(
                            int(dsType), int(i), int(attrType), int(grade)
                        )
                        if setValue > 0.0:
                            if affectString.find("%") == -1:
                                self.AppendTwoColorTextLine(
                                    affectString,
                                    affectColor,
                                    " (+{:n})".format(setValue),
                                )
                            else:
                                self.AppendTwoColorTextLine(
                                    affectString,
                                    affectColor,
                                    " (+{:n}%)".format(setValue),
                                )
                        else:
                            self.AppendTextLine(affectString, affectColor)
            else:
                for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
                    attrType = attrSlot[i][0]
                    value = attrSlot[i][1]

                    if 0 == value:
                        continue

                    affectString = self.__GetAffectString(attrType, value)
                    if affectString:
                        affectColor = self.__GetAttributeColor(i, value)
                        self.AppendTextLine(affectString, affectColor)

    def __GetAttributeColor(self, index, value):
        if value > 0.0:
            if index >= 5:
                return self.SPECIAL_POSITIVE_COLOR2
            else:
                return self.SPECIAL_POSITIVE_COLOR
        elif value == 0.0:
            return self.NORMAL_COLOR
        else:
            return self.NEGATIVE_COLOR

    def __IsPolymorphItem(self, itemVnum):
        item.SelectItem(itemVnum)
        return item.GetItemType() == item.ITEM_TYPE_POLYMORPH

    def __IsTogglePetItem(self, itemVnum):
        item.SelectItem(itemVnum)
        return (
            item.GetItemType() == item.ITEM_TYPE_TOGGLE
            and item.GetItemSubType() == item.TOGGLE_PET
        )

    def __IsToggleLevelPetItem(self, itemVnum):
        item.SelectItem(itemVnum)
        return (
            item.GetItemType() == item.ITEM_TYPE_TOGGLE
            and item.GetItemSubType() == item.TOGGLE_LEVEL_PET
        )

    def _GetItemName(self):
        return item.GetItemName()

    def __SetPolymorphItemTitle(self, monsterVnum):
        if monsterVnum:
            itemName = nonplayer.GetMonsterName(monsterVnum)
            itemName += " "
            itemName += self._GetItemName()
        else:
            itemName = self._GetItemName()

        self.SetTitle(itemName)

    def __SetNormalItemTitle(self):
        self.SetTitle(self._GetItemName())

    def __SetSpecialItemTitle(self):
        self.AppendTextLine(self._GetItemName(), self.SPECIAL_TITLE_COLOR)

    def __SetItemTitle(self, itemVnum, metinSlot, attrSlot, changeLookVnum=None):
        if self.__IsPolymorphItem(itemVnum):
            self.__SetPolymorphItemTitle(metinSlot[0])
        elif self.__IsTogglePetItem(itemVnum):
            self.__SetPolymorphItemTitle(metinSlot[0])
        elif self.__IsToggleLevelPetItem(itemVnum) and changeLookVnum:
            self.__SetPolymorphItemTitle(changeLookVnum)
        else:
            if self.__IsAttr(attrSlot):
                self.__SetSpecialItemTitle()
                return

            self.__SetNormalItemTitle()

    def __IsAttr(self, attrSlot):
        if not attrSlot:
            return False

        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attrType = attrSlot[i][0]

            if 0 != attrType:
                return True

        return False

    def AddRefineItemData(self, itemVnum, metinSlot, attrSlot=None):
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            metinSlotData = metinSlot[i]

            if self.GetMetinItemIndex(metinSlotData) == constInfo.ERROR_METIN_STONE:
                metinSlot[i] = player.METIN_SOCKET_TYPE_SILVER

        self.AddItemData(itemVnum, metinSlot, attrSlot)

    def AddItemData_Offline(self, itemVnum, itemDesc, itemSummary, metinSlot, attrSlot):
        self.__AdjustMaxWidth(attrSlot, itemDesc)
        self.__SetItemTitle(itemVnum, metinSlot, attrSlot)

        if self.__IsHair(itemVnum):
            self.__AppendHairIcon(itemVnum)

        ### Description ###
        self.AppendDescription(itemDesc, 26)
        self.AppendDescription(itemSummary, 26, self.CONDITION_COLOR)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def AddHyperLinkPetItemData(self, itemVnum, metinSlot, attrSlot, pet_info):

            pet_level = int(pet_info[0], 16)
            pet_birthday = int(pet_info[1], 16)
            pet_evol_level = int(pet_info[2], 16)
            pet_hp = float(pet_info[3])
            pet_def = float(pet_info[4])
            pet_sp = float(pet_info[5])
            pet_life_time = int(pet_info[6], 16)
            pet_life_time_max = int(pet_info[7], 16)
            pet_skill_count = int(pet_info[8], 16)
            pet_item_vnum = int(pet_info[9], 16)

            pet_skill_vnum = [0] * player.PET_SKILL_COUNT_MAX
            pet_skill_level = [0] * player.PET_SKILL_COUNT_MAX

            skill_index = 10
            for index in range(player.PET_SKILL_COUNT_MAX):
                pet_skill_vnum[index] = int(pet_info[skill_index], 16)
                skill_index = skill_index + 1
                pet_skill_level[index] = int(pet_info[skill_index], 16)
                skill_index = skill_index + 1

            self.itemVnum = itemVnum
            item.SelectItem(itemVnum)
            itemType = item.GetItemType()
            itemSubType = item.GetItemSubType()
            itemDesc = item.GetItemDescription()
            itemSummary = item.GetItemSummary()

            ## Æê ¾ÆÀÌÅÛ¸¸
            if item.ITEM_TYPE_PET != itemType:
                return

            if itemSubType not in [item.PET_UPBRINGING, item.PET_BAG]:
                return

            ## ÀÌ¸§
            if item.PET_BAG == itemSubType:
                self.itemVnum = pet_item_vnum
                item.SelectItem(pet_item_vnum)
                self.AppendTextLine(
                    item.GetItemName() + "(" + localeInfo.PET_TOOLTIP_TRADABLE + ")",
                    self.TITLE_COLOR,
                )
                self.itemVnum = itemVnum
                item.SelectItem(itemVnum)
            elif item.PET_UPBRINGING == itemSubType:
                self.AppendTextLine(item.GetItemName(), self.TITLE_COLOR)

            ## ¼³¸í
            if item.PET_UPBRINGING == itemSubType:
                self.AppendDescription(itemDesc, 26)
                self.AppendDescription(itemSummary, 26, self.CONDITION_COLOR)

            ## Æê Á¤º¸
            ## ³ªÀÌ
            self.AppendSpace(5)
            if item.PET_UPBRINGING == itemSubType:
                pet_birthday = max(0, app.GetGlobalTimeStamp() - pet_birthday)
            elif item.PET_BAG == itemSubType:
                cur_time = app.GetGlobalTimeStamp()
                pet_birthday = max(
                    0, cur_time - pet_birthday + (cur_time - metinSlot[1])
                )

            self.AppendTextLine(
                localeInfo.PET_TOOLTIP_LEVEL
                + " "
                + str(pet_level)
                + " ("
                + localeInfo.SecondToDay(pet_birthday)
                + ")"
            )

            ## ÀÏ¹ÝÁøÈ­, Æ¯¼öÁøÈ­, ½ºÅ³ °³¼ö
            self.AppendSpace(5)
            if pet_skill_count:
                self.AppendTextLine(
                    self.__GetEvolName(pet_evol_level)
                    + "("
                    + str(pet_skill_count)
                    + ")"
                )
            else:
                self.AppendTextLine(self.__GetEvolName(pet_evol_level))

            ## ´É·ÂÄ¡
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.PET_TOOLTIP_HP + " +" + str("%0.1f" % pet_hp) + "%",
                self.SPECIAL_POSITIVE_COLOR,
            )
            self.AppendTextLine(
                localeInfo.PET_TOOLTIP_DEF + " +" + str("%0.1f" % pet_def) + "%",
                self.SPECIAL_POSITIVE_COLOR,
            )
            self.AppendTextLine(
                localeInfo.PET_TOOLTIP_SP + " +" + str("%0.1f" % pet_sp) + "%",
                self.SPECIAL_POSITIVE_COLOR,
            )

            ## ½ºÅ³
            for index in range(player.PET_SKILL_COUNT_MAX):

                if pet_skill_vnum[index]:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill_vnum[index])
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level[index]
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )

            ## ¼ö¸í
            for i in xrange(item.LIMIT_MAX_NUM):
                (limitType, limitValue) = item.GetLimit(i)
                if item.LIMIT_REAL_TIME == limitType:
                    self.AppendPetItemLastTime(metinSlot[0])
                if item.LIMIT_REAL_TIME_START_FIRST_USE == limitType:
                    self.AppendPetItemLastTime(metinSlot[0])

            self.ShowToolTip()

    def drainedPercent(self, value, percent):
        return int(float(value) / float(100) * float(percent))

    def AddItemData(
        self,
        itemVnum,
        metinSlot,
        attrSlot=None,
        window_type=player.INVENTORY,
        slotIndex=-1,
        shopPrice=0,
        showIcon=0,
        showRefinedVnum=False,
        sealDate=None,
        changeLookVnum=None,
    ):
        self.slotIndex = slotIndex
        self.itemVnum = itemVnum
        self.renderTarget = None
        self.levelPet = None
        self.metinSlot = metinSlot
        itemData = itemManager().GetProto(itemVnum)

        item.SelectItem(itemVnum)
        if not itemData:
            return
        itemType = itemData.GetType()
        itemSubType = itemData.GetSubType()
        itemRefineVnum = itemData.GetRefinedVnum()

        if shopPrice != 0 and (
            50026 == itemVnum or 50300 == itemVnum or 70037 == itemVnum
        ):
            self.AppendSellingPrice(
                shopPrice
            )  # Shop Price bugfix <martpwns> 03.08.2014

        if 50026 == itemVnum:
            if 0 != metinSlot:
                name = self._GetItemName()
                if metinSlot[0] > 0:
                    name += " "
                    name += localeInfo.NumberToMoneyString(metinSlot[0])
                self.SetTitle(name)
                self.ShowToolTip()
            return

        ### Skill Book ###
        elif 50300 == itemVnum:
            if 0 != metinSlot:
                self.__SetSkillBookToolTip(
                    metinSlot[0], localeInfo.TOOLTIP_SKILLBOOK_NAME, 1
                )
                self.ShowToolTip()
            return
        elif 70037 == itemVnum:
            if 0 != metinSlot:
                self.__SetSkillBookToolTip(
                    metinSlot[0], localeInfo.TOOLTIP_SKILL_FORGET_BOOK_NAME, 0
                )
                self.AppendDescription(item.GetItemDescription(), 26)
                self.AppendDescription(item.GetItemSummary(), 26, self.CONDITION_COLOR)
                self.ShowToolTip()
            return

        ###########################################################################################

        itemDesc = item.GetItemDescription()
        itemSummary = item.GetItemSummary()

        self.__AdjustMaxWidth(attrSlot, itemDesc)

        if shopPrice != 0:
            self.AppendSellingPrice(shopPrice)

        if player.IsGameMaster() or app.IsEnableTestServerFlag():
            self.AppendTextLine("itemVnum -> {} ".format(itemVnum))

        self.__SetItemTitle(itemVnum, metinSlot, attrSlot, changeLookVnum)

        if app.ENABLE_GROWTH_PET_SYSTEM:
            if not (item.ITEM_TYPE_PET == itemType and item.PET_BAG == itemSubType):
                self.__AppendTimeLimitInfo(metinSlot)
        else:
            self.__AppendTimeLimitInfo(metinSlot)

        if app.ENABLE_GROWTH_PET_SYSTEM and item.ITEM_TYPE_PET == itemType:
            if item.PET_UPBRINGING == itemSubType:

                pet_id = metinSlot[2]
                if pet_id:
                    (
                        pet_level,
                        evol_level,
                        birthday,
                        pet_nick,
                        pet_hp,
                        pet_def,
                        pet_sp,
                        evol_name,
                    ) = player.GetPetItem(pet_id)

                    self.ClearToolTip()
                    self.AppendTextLine(pet_nick, self.TITLE_COLOR)

        if showIcon:
            self.__AppendItemIcon(itemVnum)
        ## Hair Preview Image ##
        if (
            item.ITEM_TYPE_COSTUME == itemType
            and itemSubType == item.COSTUME_TYPE_HAIR
            and self.CanEquip(True, False)
        ):
            item.SelectItem(itemVnum)

            hairIndex = item.GetValue(3)
            self.renderTarget = {
                "charType": chr.INSTANCE_TYPE_PLAYER,
                "race": player.GetRace(),
                "hair": hairIndex,
                "weapon": 0,
            }

        if sealDate is None:
            self.__AppendSealInformation(
                window_type, slotIndex
            )  ## cyh itemseal 2013 11 11
        else:
            self.__AppendSealInformation(itemSealDate=sealDate)
        ## Description ##
        self.AppendDescription(itemDesc, 26)
        self.AppendDescription(itemSummary, 26, self.CONDITION_COLOR)

        ## Weapon ##
        if item.ITEM_TYPE_WEAPON == itemType:
            if item.WEAPON_QUIVER == itemSubType:
                self.__AppendLimitInformation()
                self.__AppendWearableInformation()
                self.AppendAntiflagInformation()
            else:

                self.__AppendLimitInformation()

                if item.WEAPON_FAN == itemSubType:
                    self.__AppendMagicAttackInfo()
                    self.__AppendAttackPowerInfo()

                else:
                    self.__AppendAttackPowerInfo()
                    self.__AppendMagicAttackInfo()

                self.__AppendAffectInformation()
                self.__AppendAttributeInformation(attrSlot)

                if app.ENABLE_CHANGE_LOOK_SYSTEM:
                    self.AppendChangeLookInformation(changeLookVnum)

                self.__AppendWearableInformation()
                self.AppendAntiflagInformation()
                self.__AppendMetinSlotInfo(metinSlot)
                item.SelectItem(itemVnum)

                renderVnum = itemVnum
                if changeLookVnum is not None and changeLookVnum > 0:
                    renderVnum = changeLookVnum

                if self.CanEquip(True, False):
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "weapon": renderVnum,
                    }
                elif (
                    itemSubType == item.WEAPON_DAGGER or itemSubType == item.WEAPON_BOW
                ):
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": GetOtherRaceByOwnSex(JOB_ASSASSIN),
                        "weapon": renderVnum,
                        "armor": 0,
                    }
                elif itemSubType == item.WEAPON_FAN or itemSubType == item.WEAPON_BELL:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": GetOtherRaceByOwnSex(JOB_SHAMAN),
                        "weapon": renderVnum,
                        "armor": 0,
                    }

        ## Armor ##
        elif item.ITEM_TYPE_ARMOR == itemType:
            self.__AppendLimitInformation()
            self.AppendSpace(5)
            self.__AppendArmorDefenseInfo()
            self.__AppendMagicDefenceInfo()
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                self.AppendChangeLookInformation(changeLookVnum)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()

            if itemSubType in (item.ARMOR_WRIST, item.ARMOR_NECK, item.ARMOR_EAR):
                self.__AppendAccessoryMetinSlotInfo(
                    metinSlot,
                    constInfo.GET_ACCESSORY_MATERIAL_VNUM(itemVnum, itemSubType),
                )
            else:
                self.__AppendMetinSlotInfo(metinSlot)

            if itemSubType == item.ARMOR_BODY:
                renderVnum = itemVnum
                if changeLookVnum is not None and changeLookVnum > 0:
                    renderVnum = changeLookVnum

                if self.CanEquip(True, False):
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "armor": renderVnum,
                    }

        elif (
            item.ITEM_TYPE_COSTUME == itemType and item.COSTUME_TYPE_ACCE == itemSubType
        ):
            self.__AppendLimitInformation()
            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                self.AppendChangeLookInformation(changeLookVnum)
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()
            if self.CanEquip(True, False):
                renderVnum = itemVnum
                if changeLookVnum is not None and changeLookVnum > 0:
                    renderVnum = changeLookVnum

                self.renderTarget = {
                    "charType": chr.INSTANCE_TYPE_PLAYER,
                    "race": player.GetRace(),
                    "acce": renderVnum,
                }

        ## Ring Slot Item (Not UNIQUE) ##
        elif item.ITEM_TYPE_RING == itemType:
            self.__AppendLimitInformation()
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()
        elif item.ITEM_TYPE_TALISMAN == itemType:
            self.__AppendLimitInformation()
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()
        ## Belt Item ##
        elif item.ITEM_TYPE_BELT == itemType:
            self.__AppendLimitInformation()
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()
            self.__AppendAccessoryMetinSlotInfo(metinSlot, 18900)

        ## Costume ##
        elif item.ITEM_TYPE_COSTUME == itemType:
            self.__AppendLimitInformation()

            if app.ENABLE_CHANGE_LOOK_SYSTEM:
                self.AppendChangeLookInformation(changeLookVnum)

            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendWearableInformation()
            self.AppendAntiflagInformation()
            if self.CanEquip(True, False):
                renderVnum = itemVnum
                if changeLookVnum is not None and changeLookVnum > 0:
                    renderVnum = changeLookVnum

                if itemSubType == item.COSTUME_TYPE_BODY:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "armor": renderVnum,
                    }
                elif itemSubType == item.COSTUME_TYPE_WEAPON:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "weapon": renderVnum,
                    }
                elif itemSubType == item.COSTUME_TYPE_WEAPON_EFFECT:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "weapon_effect": long(renderVnum),
                    }
                elif itemSubType == item.COSTUME_TYPE_BODY_EFFECT:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "body_effect": long(renderVnum),
                    }
                elif itemSubType == item.COSTUME_TYPE_WING_EFFECT:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_PLAYER,
                        "race": player.GetRace(),
                        "wing_effect": long(renderVnum),
                    }

        ## Rod ##
        elif item.ITEM_TYPE_ROD == itemType:

            if metinSlot:
                curLevel = item.GetValue(0) / 10
                curEXP = metinSlot[0]
                maxEXP = item.GetValue(2)
                self.__AppendLimitInformation()
                self.__AppendRodInformation(curLevel, curEXP, maxEXP)
                self.AppendAntiflagInformation()

        ## Pick ##
        elif item.ITEM_TYPE_PICK == itemType:

            if 0 != metinSlot and itemVnum != 29111:  # 29111: mining relic
                curLevel = item.GetValue(0) / 10
                curEXP = metinSlot[0]
                maxEXP = item.GetValue(2)
                self.__AppendLimitInformation()
                self.__AppendPickInformation(curLevel, curEXP, maxEXP)
                self.AppendAntiflagInformation()

        ## Lottery ##
        elif item.ITEM_TYPE_LOTTERY == itemType:
            if 0 != metinSlot:
                ticketNumber = int(metinSlot[0])
                stepNumber = int(metinSlot[1])

                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_LOTTERY_STEP_NUMBER.format(stepNumber),
                    self.NORMAL_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.TOOLTIP_LOTTO_NUMBER.format(ticketNumber),
                    self.NORMAL_COLOR,
                )

        ### Metin ###
        elif item.ITEM_TYPE_METIN == itemType:
            self.__AppendAffectInformation()
            self.AppendMetinWearInformation()
            self.AppendSpace(5)
            self.AppendAntiflagInformation()

        ### Fish ###
        elif item.ITEM_TYPE_FISH == itemType:
            if 0 != metinSlot:
                self.__AppendFishInfo(metinSlot[0])

        ## item.ITEM_TYPE_BLEND
        elif item.ITEM_TYPE_BLEND == itemType:
            self.__AppendLimitInformation()

            if metinSlot:
                affectType = metinSlot[0]
                affectValue = metinSlot[1]
                affectText = self.__GetAffectString(affectType, affectValue)

                self.AppendSpace(5)

                if affectText:
                    self.AppendTextLine(affectText, self.NORMAL_COLOR)

                time = metinSlot[2]
                if time > 0:
                    minute = time / 60
                    second = time % 60
                    timeString = localeInfo.TOOLTIP_POTION_TIME

                    if minute > 0:
                        timeString += str(minute) + localeInfo.TOOLTIP_POTION_MIN
                    if second > 0:
                        timeString += " " + str(second) + localeInfo.TOOLTIP_POTION_SEC

                    self.AppendTextLine(timeString)
                else:
                    self.AppendTextLine(localeInfo.BLEND_POTION_NO_TIME)
            else:
                self.AppendTextLine(localeInfo.Get("BLEND_POTION_NO_INFO"))

            self.AppendAntiflagInformation()

        elif item.ITEM_TYPE_UNIQUE == itemType:
            self.__AppendLimitInformation()
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            if metinSlot:
                bHasRealtimeFlag = 0

                for i in xrange(item.LIMIT_MAX_NUM):
                    (limitType, limitValue) = item.GetLimit(i)

                    if (
                        item.LIMIT_REAL_TIME == limitType
                        or item.LIMIT_TIMER_BASED_ON_WEAR == limitType
                        or item.LIMIT_REAL_TIME_START_FIRST_USE == limitType
                    ):
                        bHasRealtimeFlag = 1

                if not bHasRealtimeFlag:
                    time = metinSlot[3 - 1]

                    if 1 == item.GetValue(2):
                        self.__AppendMallItemLastTime(time)
                    else:
                        self.__AppendRemainingTime(time * 60)
            self.AppendAntiflagInformation()

        ### Use ###
        elif item.ITEM_TYPE_USE == itemType:
            self.__AppendLimitInformation()

            if item.USE_POTION == itemSubType or item.USE_POTION_NODELAY == itemSubType:
                self.__AppendPotionInformation()

            elif item.USE_ABILITY_UP == itemSubType:
                self.__AppendAbilityPotionInformation()

            if 27989 == itemVnum or 76006 == itemVnum:
                if 0 != metinSlot:
                    useCount = int(metinSlot[0])

                    self.AppendSpace(5)
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_REST_USABLE_COUNT.format(6 - useCount),
                        self.NORMAL_COLOR,
                    )

            elif 50004 == itemVnum:
                if 0 != metinSlot:
                    useCount = int(metinSlot[0])

                    self.AppendSpace(5)
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_REST_USABLE_COUNT.format(10 - useCount),
                        self.NORMAL_COLOR,
                    )

            ## Warp scrolls
            elif itemVnum in WARP_SCROLLS:
                if 0 != metinSlot:
                    xPos = int(metinSlot[0])
                    yPos = int(metinSlot[1])
                    index = int(metinSlot[2])

                    if xPos != 0 and yPos != 0:
                        mapName = background.GetMapInfo(index)

                        localeMapName = localeInfo.GetMapNameByIndex(index)

                        self.AppendSpace(5)

                        if localeMapName:
                            self.AppendTextLine(
                                localeInfo.TOOLTIP_MEMORIZED_POSITION.format(
                                    localeMapName, int(xPos) / 100, int(yPos) / 100
                                ),
                                self.NORMAL_COLOR,
                            )
                        else:
                            self.AppendTextLine(
                                localeInfo.TOOLTIP_MEMORIZED_POSITION_ERROR.format(
                                    int(xPos) / 100, int(yPos) / 100
                                ),
                                self.NORMAL_COLOR,
                            )
                            logging.debug(
                                "NOT_EXIST_IN_MINIMAP_ZONE_NAME_DICT: %s", mapName
                            )

            elif item.USE_TIME_CHARGE_PER == itemSubType:
                if metinSlot[2]:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_TIME_CHARGER_PER.format(metinSlot[2])
                    )
                else:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_TIME_CHARGER_PER.format(item.GetValue(0))
                    )

            elif item.USE_TIME_CHARGE_FIX == itemSubType:
                if metinSlot[2]:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_TIME_CHARGER_FIX.format(metinSlot[2])
                    )
                else:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_TIME_CHARGER_FIX.format(item.GetValue(0))
                    )
            self.AppendAntiflagInformation()

        elif item.ITEM_TYPE_QUEST == itemType:
            self.__AppendAffectInformation()
            self.__AppendAttributeInformation(attrSlot)
            self.__AppendLimitInformation()
            self.AppendAntiflagInformation()

        elif item.ITEM_TYPE_DS == itemType:

            self.AppendTextLine(self.__DragonSoulInfoString(itemVnum))
            if (
                window_type == player.EQUIPMENT
                and self.interface
                and self.interface.wndDragonSoul
            ):
                self.__AppendDragonSoulAttributeInformation(
                    attrSlot,
                    itemVnum / 10000,
                    self.interface.wndDragonSoul.GetDSSetGrade(),
                )
            else:
                self.__AppendDragonSoulAttributeInformation(attrSlot)

        elif app.ENABLE_GROWTH_PET_SYSTEM and item.ITEM_TYPE_PET == itemType:

            if item.PET_EGG == itemSubType:
                if window_type == player.INVENTORY:
                    self.__AppendPetEggItemInformation(metinSlot)
                else:
                    self.__AppendPetEggItemInformation(metinSlot, True)

            elif item.PET_UPBRINGING == itemSubType:
                self.__AppendUpBringingPetItemInfomation(metinSlot)

            elif item.PET_BAG == itemSubType:
                self.__AppendPetBagItemInfomation(metinSlot)
        elif item.ITEM_TYPE_GIFTBOX == itemType:
            self.__AppendLimitInformation()
            self.AppendAntiflagInformation()

        elif item.ITEM_TYPE_TOGGLE == itemType:
            if not itemSubType == item.TOGGLE_LEVEL_PET:
                self.AppendChangeLookInformation(changeLookVnum)
            self.__AppendLimitInformation()
            self.__AppendAffectInformation()
            self.AppendAntiflagInformation()

            if itemSubType == item.TOGGLE_LEVEL_PET:
                self.__AppendLevelPetInformation(metinSlot)
                self.AppendSpace(5)
                self.__AppendAttributeInformation(attrSlot)
                renderVnum = 0
                if changeLookVnum is not None and changeLookVnum > 0:
                    renderVnum = changeLookVnum
                if renderVnum > 0:
                    self.renderTarget = {
                        "charType": chr.INSTANCE_TYPE_NPC,
                        "race": renderVnum,
                    }

            self.AppendSpace(5)

            isActivated = int(metinSlot[3])
            if 0 != isActivated:
                self.AppendTextLine(
                    "(%s)" % localeInfo.TOOLTIP_AUTO_POTION_USING,
                    self.SPECIAL_POSITIVE_COLOR,
                )
                self.AppendSpace(5)

            ## ????
            if itemSubType in (
                item.TOGGLE_AUTO_RECOVERY_HP,
                item.TOGGLE_AUTO_RECOVERY_SP,
            ):
                if 0 != metinSlot:
                    ## 0: ???, 1: ???, 2: ??
                    usedAmount = float(metinSlot[1])
                    totalAmount = float(metinSlot[2])

                    if 0 == totalAmount:
                        totalAmount = 1

                    rest = 100.0 - ((usedAmount / totalAmount) * 100.0)
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_AUTO_POTION_REST.format(rest),
                        self.POSITIVE_COLOR,
                    )

            if self.CanEquip(True, False):
                if itemSubType == item.TOGGLE_MOUNT:
                    item.SelectItem(itemVnum)
                    mountVnum = item.GetValue(0)
                    if mountVnum > 0:
                        renderVnum = mountVnum
                        if changeLookVnum is not None and changeLookVnum > 0:
                            renderVnum = changeLookVnum

                        self.renderTarget = {
                            "charType": chr.INSTANCE_TYPE_PLAYER,
                            "race": player.GetRace(),
                            "mountVnum": renderVnum,
                        }
                elif itemSubType == item.TOGGLE_PET:
                    item.SelectItem(itemVnum)
                    isAttackingPet = item.GetValue(4)
                    mobVnum = item.GetValue(0)

                    if isAttackingPet:
                        mobVnum = int(metinSlot[0])

                    if mobVnum > 0:
                        renderVnum = mobVnum
                        if changeLookVnum is not None and changeLookVnum > 0:
                            renderVnum = changeLookVnum

                        self.renderTarget = {
                            "charType": chr.INSTANCE_TYPE_NPC,
                            "race": renderVnum,
                        }
                elif itemSubType == item.TOGGLE_LEVEL_PET:
                    if window_type == player.INVENTORY:
                        if not isActivated:
                            self.levelPet = slotIndex
                            self.AppendSpace(5)
                            self.AppendTextLine(
                                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT2),
                                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
                            )
                            self.AppendSpace(5)
                        else:
                            self.AppendSpace(5)
                            self.AppendTextLine(
                                ReplaceEmoticons(
                                    localeInfo.Get(
                                        "TOOLTIP_DEACTIVATE_TO_OPEN_PET_STATUS"
                                    )
                                ),
                                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
                            )
                            self.AppendSpace(5)

        if (
            self.renderTarget
            and not self.IsShowingExchange()
            and not self.IsShowingSafeBox()
        ):
            self.AppendSpace(5)
            self.AppendSpace(5)
            self.AppendTextLine(
                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT3),
                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
            )
            self.AppendSpace(5)

        if (
            itemData
            and itemData.IsFlag(item.ITEM_FLAG_STACKABLE)
            and not itemData.IsAntiFlag(item.ITEM_ANTIFLAG_STACK)
        ):
            self.AppendSpace(5)
            self.AppendTextLine(
                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT4),
                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
            )
            self.AppendSpace(5)

        if item.ITEM_TYPE_TOGGLE == itemType and window_type == player.INVENTORY:
            self.AppendSpace(5)
            isActivated = int(metinSlot[3])
            if isActivated:
                self.AppendTextLine(
                    ReplaceEmoticons(localeInfo.TOOLTIP_TEXT5),
                    grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
                )
            else:
                self.AppendTextLine(
                    ReplaceEmoticons(localeInfo.TOOLTIP_TEXT6),
                    grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
                )
            self.AppendSpace(5)

        if item.ITEM_TYPE_GIFTBOX == itemType:
            self.AppendSpace(5)

            self.AppendTextLine(
                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT7),
                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
            )
            self.AppendSpace(5)

        if window_type == player.SAFEBOX:
            self.AppendSpace(5)
            self.AppendTextLine(
                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT8),
                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
            )
            self.AppendSpace(5)

        if safebox.IsOpen() and window_type == player.INVENTORY:
            self.AppendSpace(5)
            self.AppendTextLine(
                ReplaceEmoticons(localeInfo.TOOLTIP_TEXT9),
                grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
            )
            self.AppendSpace(5)

        # self.AppendSpace(5)
        # self.AppendTextLine(
            # ReplaceEmoticons(localeInfo.TOOLTIP_TEXT10),
            # grp.GenerateColor(1.0, 0.64, 0.39, 1.0),
        # )
        self.AppendSpace(5)

        if showRefinedVnum and itemRefineVnum:
            refinedData = itemManager().GetProto(itemRefineVnum)
            if refinedData:
                self.AppendTextLine(localeInfo.Get("TOOLTIP_REFINED_TO"))
                self.AppendTextLine(refinedData.GetName())  # TODO LOCALE_GAME
                self.AppendSpace(5)

        self.ShowToolTip()

    def IsShowingSafeBox(self):
        return (
            self.interface
            and self.interface.wndSafebox
            and self.interface.wndSafebox.IsShow()
        )

    def IsShowingExchange(self):
        return (
            self.interface
            and self.interface.dlgExchange
            and self.interface.dlgExchange.IsShow()
        )

    def __AppendLevelPetInformation(self, metinSlot):
        petLevel = metinSlot[0]
        needCount = metinSlot[1]
        statPoints = metinSlot[2]

        self.AppendTextLine(localeInfo.PET_TOOLTIP_LEVEL + " " + str(petLevel))

        if petLevel == 150:
            return

        self.AppendTextLine(localeInfo.TOOLTIP_TEXT11.format(str(needCount)))
        self.AppendTextLine(localeInfo.TOOLTIP_TEXT12.format(str(statPoints)))

        foodNeeded = ""
        foods = itemManager().GetLevelPetDisplayFoods()

        for food in foods:
            if food.min <= petLevel <= food.max:
                proto = itemManager().GetProto(food.vnum)
                if proto:
                    foodNeeded = proto.GetName()

        self.AppendTextLine(
            localeInfo.TOOLTIP_LEVEL_PET_NEEDED_FOOD.format(str(foodNeeded))
        )

    if app.ENABLE_GROWTH_PET_SYSTEM:

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

        def __AppendPetBagItemInfomation(self, metinSlot):
            pet_id = metinSlot[2]
            if pet_id:
                self.ClearToolTip()
                growhPetVNum = player.GetPetItemVNumInBag(pet_id)
                if 0 == growhPetVNum:
                    return

                # Æê °¡¹æ ÀÌ¸§ ´Ð³×ÀÓÀ¸·Î Ãâ·Â½Ã
                (
                    pet_level,
                    evol_level,
                    birthday,
                    pet_nick,
                    pet_hp,
                    pet_def,
                    pet_sp,
                    evol_name,
                ) = player.GetPetItem(pet_id)
                self.AppendTextLine(
                    pet_nick + "(" + localeInfo.PET_TOOLTIP_TRADABLE + ")",
                    self.TITLE_COLOR,
                )

                # Æê °¡¹æ ÀÌ¸§ Ãâ·Â½Ã
                # item.SelectItem(growhPetVNum)
                # self.AppendTextLine( item.GetItemName() + "(" + localeInfo.PET_TOOLTIP_TRADABLE + ")", self.TITLE_COLOR )

                item.SelectItem(self.itemVnum)

                ## skill Á¤º¸
                (
                    skill_count,
                    pet_skill1,
                    pet_skill_level1,
                    pet_skill_cool1,
                    pet_skill2,
                    pet_skill_level2,
                    pet_skill_cool2,
                    pet_skill3,
                    pet_skill_level3,
                    pet_skill_cool3,
                ) = player.GetPetSkill(pet_id)

                ## Lv XXX ( DDDDÀÏ)
                (
                    pet_level,
                    evol_level,
                    birthday,
                    pet_nick,
                    pet_hp,
                    pet_def,
                    pet_sp,
                    evol_name,
                ) = player.GetPetItem(pet_id)
                self.AppendSpace(5)
                cur_time = app.GetGlobalTimeStamp()
                birthSec = max(0, cur_time - birthday + (cur_time - metinSlot[1]))
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_LEVEL
                    + " "
                    + str(pet_level)
                    + " ("
                    + localeInfo.SecondToDay(birthSec)
                    + ")"
                )

                ## ÀÏ¹ÝÁøÈ­, Æ¯¼öÁøÈ­, ½ºÅ³ °³¼ö
                self.AppendSpace(5)
                if skill_count:
                    self.AppendTextLine(
                        self.__GetEvolName(evol_level) + "(" + str(skill_count) + ")"
                    )
                else:
                    self.AppendTextLine(self.__GetEvolName(evol_level))

                ## ´É·ÂÄ¡
                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_HP + " +" + str(pet_hp) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_DEF + " +" + str(pet_def) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_SP + " +" + str(pet_sp) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )

                ## ½ºÅ³
                if pet_skill1:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill1)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level1
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )
                if pet_skill2:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill2)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level2
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )
                if pet_skill3:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill3)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level3
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )

                ## ±â°£
                for i in xrange(item.LIMIT_MAX_NUM):
                    (limitType, limitValue) = item.GetLimit(i)
                    if item.LIMIT_REAL_TIME_START_FIRST_USE == limitType:
                        self.AppendPetItemLastTime(metinSlot[0])

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __AppendPetEggItemInformation(self, metinSlot, isFeedWindow=False):

            for i in xrange(item.LIMIT_MAX_NUM):
                (limitType, limitValue) = item.GetLimit(i)
                if item.LIMIT_REAL_TIME == limitType:
                    self.AppendPetItemLastTime(metinSlot[0])

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def __AppendUpBringingPetItemInfomation(self, metinSlot):
            pet_id = metinSlot[2]
            if pet_id:
                ## skill Á¤º¸
                (
                    skill_count,
                    pet_skill1,
                    pet_skill_level1,
                    pet_skill_cool1,
                    pet_skill2,
                    pet_skill_level2,
                    pet_skill_cool2,
                    pet_skill3,
                    pet_skill_level3,
                    pet_skill_cool3,
                ) = player.GetPetSkill(pet_id)

                ## Lv XXX ( DDDDÀÏ)
                (
                    pet_level,
                    evol_level,
                    birthday,
                    pet_nick,
                    pet_hp,
                    pet_def,
                    pet_sp,
                    evol_name,
                ) = player.GetPetItem(pet_id)
                self.AppendSpace(5)
                birthSec = max(0, app.GetGlobalTimeStamp() - birthday)
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_LEVEL
                    + " "
                    + str(pet_level)
                    + " ("
                    + localeInfo.SecondToDay(birthSec)
                    + ")"
                )

                ## ÀÏ¹ÝÁøÈ­, Æ¯¼öÁøÈ­, ½ºÅ³ °³¼ö
                self.AppendSpace(5)
                if skill_count:
                    self.AppendTextLine(
                        self.__GetEvolName(evol_level) + "(" + str(skill_count) + ")"
                    )
                else:
                    self.AppendTextLine(self.__GetEvolName(evol_level))

                ## ´É·ÂÄ¡
                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_HP + " +" + str(pet_hp) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_DEF + " +" + str(pet_def) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.PET_TOOLTIP_SP + " +" + str(pet_sp) + "%",
                    self.SPECIAL_POSITIVE_COLOR,
                )

                ## ½ºÅ³
                if pet_skill1:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill1)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level1
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )
                if pet_skill2:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill2)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level2
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )
                if pet_skill3:
                    self.AppendSpace(5)
                    (
                        pet_skill_name,
                        pet_skill_desc,
                        pet_skill_use_type,
                        pet_skill_cool_time,
                    ) = skill.GetPetSkillInfo(pet_skill3)
                    self.AppendTextLine(
                        localeInfo.PET_TOOLTUP_SKILL.format(
                            pet_skill_name, pet_skill_level3
                        ),
                        self.SPECIAL_POSITIVE_COLOR,
                    )

                ## ¼ö¸í
                for i in xrange(item.LIMIT_MAX_NUM):
                    (limitType, limitValue) = item.GetLimit(i)
                    if item.LIMIT_REAL_TIME == limitType:
                        self.AppendPetItemLastTime(metinSlot[0])

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def AppendPetItemLastTime(self, endTime):
            self.AppendSpace(5)

            leftSec = endTime - app.GetGlobalTimeStamp()

            if leftSec >= 0:
                self.AppendTextLine(
                    localeInfo.LEFT_TIME + " : " + localeInfo.SecondToDHM(leftSec),
                    self.NORMAL_COLOR,
                )
            else:
                self.AppendTextLine(
                    localeInfo.LEFT_TIME + " : " + localeInfo.PET_TOOLTIP_LIFETIME_OVER,
                    self.DISABLE_COLOR,
                )

    def __DragonSoulInfoString(self, dwVnum):
        step = (dwVnum / 100) % 10
        refine = (dwVnum / 10) % 10
        if 0 == step:
            return (
                localeInfo.DRAGON_SOUL_STEP_LEVEL1
                + " "
                + localeInfo.DRAGON_SOUL_STRENGTH.format(refine)
            )
        elif 1 == step:
            return (
                localeInfo.DRAGON_SOUL_STEP_LEVEL2
                + " "
                + localeInfo.DRAGON_SOUL_STRENGTH.format(refine)
            )
        elif 2 == step:
            return (
                localeInfo.DRAGON_SOUL_STEP_LEVEL3
                + " "
                + localeInfo.DRAGON_SOUL_STRENGTH.format(refine)
            )
        elif 3 == step:
            return (
                localeInfo.DRAGON_SOUL_STEP_LEVEL4
                + " "
                + localeInfo.DRAGON_SOUL_STRENGTH.format(refine)
            )
        elif 4 == step:
            return (
                localeInfo.DRAGON_SOUL_STEP_LEVEL5
                + " "
                + localeInfo.DRAGON_SOUL_STRENGTH.format(refine)
            )
        else:
            return ""

    ## ?????
    def __IsHair(self, itemVnum):
        return (
            self.__IsOldHair(itemVnum)
            or self.__IsNewHair(itemVnum)
            or self.__IsNewHair2(itemVnum)
            or self.__IsNewHair3(itemVnum)
            or self.__IsCostumeHair(itemVnum)
            or self.__IsHairColor(itemVnum)
        )

    def __IsOldHair(self, itemVnum):
        return itemVnum > 73000 and itemVnum < 74000

    def __IsHairColor(self, itemVnum):
        return itemVnum >= 70201 and itemVnum <= 70206

    def __IsNewHair(self, itemVnum):
        return itemVnum > 74000 and itemVnum < 75000

    def __IsNewHair2(self, itemVnum):
        return itemVnum > 75000 and itemVnum < 76000

    def __IsNewHair3(self, itemVnum):
        return (
            (74012 < itemVnum and itemVnum < 74022)
            or (74262 < itemVnum and itemVnum < 74272)
            or (74512 < itemVnum and itemVnum < 74522)
            or (74762 < itemVnum and itemVnum < 74772)
            or (45000 < itemVnum and itemVnum < 47000)
        )

    def __IsCostumeHair(self, itemVnum):
        item.SelectItem(itemVnum)
        return (
            item.ITEM_TYPE_COSTUME == item.GetItemType()
            and item.COSTUME_TYPE_HAIR == item.GetItemSubType()
        )

    def __AppendHairIcon(self, itemVnum):
        itemImage = ui.ImageBox()
        itemImage.SetParent(self)
        itemImage.Show()
        try:
            if self.__IsOldHair(itemVnum):
                itemImage.LoadImage("d:/ymir work/item/quest/" + str(itemVnum) + ".tga")
            elif self.__IsNewHair3(itemVnum):
                itemImage.LoadImage("icon/hair/%d.sub" % itemVnum)
            elif self.__IsHairColor(itemVnum):
                itemImage.LoadImage("icon/hair/%d.sub" % itemVnum)
            elif self.__IsNewHair(itemVnum):
                if itemVnum > 74520 and itemVnum < 74751:
                    itemImage.LoadImage("icon/hair/%d.sub" % itemVnum)
                else:
                    itemImage.LoadImage(
                        "d:/ymir work/item/quest/" + str(itemVnum - 1000) + ".tga"
                    )
            elif self.__IsNewHair2(itemVnum):
                itemImage.LoadImage("icon/hair/%d.sub" % itemVnum)
            elif self.__IsCostumeHair(itemVnum):
                itemImage.LoadImage("icon/hair/%d.sub" % (itemVnum - 100000))
        except RuntimeError:
            del itemImage
            return
        itemImage.SetPosition(itemImage.GetWidth() / 2, self.toolTipHeight)
        self.toolTipHeight += itemImage.GetHeight()
        # self.toolTipWidth += itemImage.GetWidth()/2
        self.childrenList.append(itemImage)
        self.ResizeToolTip()

    def __AppendItemIcon(self, itemVnum):
        itemImage = ui.ImageBox()
        itemImage.SetParent(self)
        itemImage.Show()

        item.SelectItem(itemVnum)

        ## Image
        try:
            itemImage.LoadImage(item.GetIconImageFileName())
        except:
            logging.exception(
                "Failed to load item icon for %d: %s",
                itemVnum,
                item.GetIconImageFileName(),
            )

        itemImage.SetHorizontalAlignCenter()
        itemImage.SetPosition(0, self.toolTipHeight)
        self.toolTipHeight += itemImage.GetHeight()
        self.childrenList.append(itemImage)
        self.ResizeToolTip()

    def __AdjustMaxWidth(self, attrSlot, desc):
        if not attrSlot:
            return

        newToolTipWidth = self.toolTipWidth
        newToolTipWidth = max(self.__AdjustAttrMaxWidth(attrSlot), newToolTipWidth)
        newToolTipWidth = max(self.__AdjustDescMaxWidth(desc), newToolTipWidth)

        if newToolTipWidth > self.toolTipWidth:
            self.toolTipWidth = newToolTipWidth
            self.ResizeToolTip()

    def __AdjustAttrMaxWidth(self, attrSlot):
        if not attrSlot:
            return self.toolTipWidth

        maxWidth = self.toolTipWidth
        for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM):
            attrType = attrSlot[i][0]
            value = attrSlot[i][1]

            if attrType in self.ATTRIBUTE_NEED_WIDTH:
                if value > 0:
                    maxWidth = max(self.ATTRIBUTE_NEED_WIDTH[attrType], maxWidth)

        return maxWidth

    def __AdjustDescMaxWidth(self, desc):
        if len(desc) < DESC_DEFAULT_MAX_COLS:
            return self.toolTipWidth

        return DESC_WESTERN_MAX_WIDTH

    def __SetSkillBookToolTip(self, skillIndex, bookName, skillGrade):
        skillName = skill.GetSkillName(skillIndex)

        if not skillName:
            return

        itemName = skillName + " " + bookName
        self.SetTitle(itemName)

    def __AppendPickInformation(self, curLevel, curEXP, maxEXP):
        self.AppendSpace(5)

        if maxEXP == 0:  # Max refine state
            self.AppendTextLine(localeInfo.TOOLTIP_PICK_MAX, self.NORMAL_COLOR)
            return

        self.AppendTextLine(
            localeInfo.TOOLTIP_PICK_LEVEL.format(curLevel), self.NORMAL_COLOR
        )
        self.AppendTextLine(
            localeInfo.TOOLTIP_PICK_EXP.format(curEXP, maxEXP), self.NORMAL_COLOR
        )

        if curEXP == maxEXP:
            self.AppendSpace(5)
            self.AppendTextLine(localeInfo.TOOLTIP_PICK_UPGRADE1, self.NORMAL_COLOR)
            self.AppendTextLine(localeInfo.TOOLTIP_PICK_UPGRADE2, self.NORMAL_COLOR)
            self.AppendTextLine(localeInfo.TOOLTIP_PICK_UPGRADE3, self.NORMAL_COLOR)

    def __AppendRodInformation(self, curLevel, curEXP, maxEXP):
        self.AppendSpace(5)
        self.AppendTextLine(
            localeInfo.TOOLTIP_FISHINGROD_LEVEL.format(curLevel), self.NORMAL_COLOR
        )
        self.AppendTextLine(
            localeInfo.TOOLTIP_FISHINGROD_EXP.format(curEXP, maxEXP), self.NORMAL_COLOR
        )

        if curEXP == maxEXP:
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.TOOLTIP_FISHINGROD_UPGRADE1, self.NORMAL_COLOR
            )
            self.AppendTextLine(
                localeInfo.TOOLTIP_FISHINGROD_UPGRADE2, self.NORMAL_COLOR
            )
            self.AppendTextLine(
                localeInfo.TOOLTIP_FISHINGROD_UPGRADE3, self.NORMAL_COLOR
            )

    def __AppendLimitInformation(self):
        appendSpace = False

        for i in xrange(item.LIMIT_MAX_NUM):

            (limitType, limitValue) = item.GetLimit(i)

            if limitValue > 0:
                if False == appendSpace:
                    self.AppendSpace(5)
                    appendSpace = True
            else:
                continue

            if item.LIMIT_LEVEL == limitType:
                color = self.GetLimitTextLineColor(
                    playerInst().GetPoint(player.LEVEL), limitValue
                )
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_LIMIT_LEVEL.format(limitValue), color
                )

            if item.LIMIT_MAX_LEVEL == limitType:
                color = self.GetLimitTextLineColor(
                    playerInst().GetPoint(player.LEVEL), limitValue
                )
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_LIMIT_MAX_LEVEL.format(limitValue), color
                )

            if item.LIMIT_MAP == limitType:
                color = (
                    self.ENABLE_COLOR
                    if background.GetCurrentMapIndex() == limitValue
                    else self.DISABLE_COLOR
                )
                mapName = localeInfo.GetMapNameByIndex(limitValue)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_ITEM_LIMIT_MAP.format(mapName), color
                )
            """
            elif item.LIMIT_STR == limitType:
                color = self.GetLimitTextLineColor(playerInst().GetPoint(player.ST), limitValue)
                self.AppendTextLine(localeInfo.TOOLTIP_ITEM_LIMIT_STR.format(limitValue), color)
            elif item.LIMIT_DEX == limitType:
                color = self.GetLimitTextLineColor(playerInst().GetPoint(player.DX), limitValue)
                self.AppendTextLine(localeInfo.TOOLTIP_ITEM_LIMIT_DEX.format(limitValue), color)
            elif item.LIMIT_INT == limitType:
                color = self.GetLimitTextLineColor(playerInst().GetPoint(player.IQ), limitValue)
                self.AppendTextLine(localeInfo.TOOLTIP_ITEM_LIMIT_INT.format(limitValue), color)
            elif item.LIMIT_CON == limitType:
                color = self.GetLimitTextLineColor(playerInst().GetPoint(player.HT), limitValue)
                self.AppendTextLine(localeInfo.TOOLTIP_ITEM_LIMIT_CON.format(limitValue), color)
            """

        if (
            item.GetItemType() == item.ITEM_TYPE_TOGGLE
            and item.GetItemSubType() == item.TOGGLE_LEVEL_PET
        ):
            if self.metinSlot is not None and isinstance(self.metinSlot, list):
                petLevel = self.metinSlot[0]
                if petLevel > playerInst().GetPoint(player.LEVEL):
                    color = self.DISABLE_COLOR
                    self.AppendTextLine(
                        localeInfo.Get("TOOLTIP_LEVEL_PET_LEVEL_HIGHER_THAN_PLAYER"),
                        color,
                    )

    ## cyh itemseal 2013 11 11
    def __AppendSealInformation(
        self, window_type=None, slotIndex=None, itemSealDate=None
    ):
        if window_type is not None and slotIndex is not None:
            itemSealDate = player.GetItemSealDate(window_type, slotIndex)
            if itemSealDate == item.GetUnlimitedSealDate():
                self.AppendTextLine(localeInfo.TOOLTIP_SEALED, self.NEGATIVE_COLOR)
            elif (
                itemSealDate > item.GetDefaultSealDate()
                and itemSealDate > app.GetGlobalTimeStamp()
            ):
                hours, minutes = player.GetItemUnSealLeftTime(window_type, slotIndex)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_UNSEAL_LEFT_TIME.format(hours, minutes),
                    self.NEGATIVE_COLOR,
                )
        elif itemSealDate is not None:
            if itemSealDate == item.GetUnlimitedSealDate():
                self.AppendTextLine(localeInfo.TOOLTIP_SEALED, self.NEGATIVE_COLOR)
            elif (
                itemSealDate > item.GetDefaultSealDate()
                and itemSealDate > app.GetGlobalTimeStamp()
            ):
                hours, minutes = (
                    (itemSealDate - app.GetGlobalTimeStamp()) / 3600,
                    (itemSealDate - app.GetGlobalTimeStamp() % 3600 / 60),
                )
                self.AppendTextLine(
                    localeInfo.TOOLTIP_UNSEAL_LEFT_TIME.format(hours, minutes),
                    self.NEGATIVE_COLOR,
                )

    if app.ENABLE_CHANGED_ATTR:

        def GetAffectString(self, affectType, affectValue):
            if 0 == affectType:
                return None

            if 0 == affectValue:
                return None

            try:
                return self.AFFECT_DICT[affectType].format(
                    localeInfo.DottedNumber(affectValue)
                )
            except TypeError:
                return "UNKNOWN_VALUE[%s] %s" % (affectType, affectValue)
            except KeyError:
                return "UNKNOWN_TYPE[%s] %s" % (affectType, affectValue)

    def __GetAffectString(self, affectType, affectValue):
        if 0 == affectType:
            return None

        if 0 == affectValue:
            return None

        try:
            return self.AFFECT_DICT[affectType].format(
                localeInfo.DottedNumber(affectValue)
            )
        except TypeError:
            return self.AFFECT_DICT[affectType]
        except KeyError:
            return "UNKNOWN_TYPE[%s] %s" % (affectType, affectValue)

    def __AppendAffectInformation(self):
        for i in xrange(item.ITEM_APPLY_MAX_NUM):

            (affectType, affectValue) = item.GetAffect(i)

            affectString = self.__GetAffectString(affectType, affectValue)
            if affectString:
                self.AppendTextLine(
                    affectString, self.GetChangeTextLineColor(affectValue)
                )

    if app.ENABLE_CHANGE_LOOK_SYSTEM:

        def AppendChangeLookInformation(self, changeLookVnum):
            if changeLookVnum:
                proto = itemManager().GetProto(changeLookVnum)
                if not proto:
                    return
                self.AppendSpace(5)
                self.AppendTextLine(
                    "[ " + localeInfo.CHANGE_LOOK_TITLE + " ]",
                    self.CHANGELOOK_TITLE_COLOR,
                )

                itemName = proto.GetName()

                if proto.GetType() == item.ITEM_TYPE_COSTUME:
                    if proto.GetSubType() == item.COSTUME_TYPE_BODY:
                        malefemale = ""
                        if proto.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
                            malefemale = localeInfo.FOR_FEMALE

                        if proto.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
                            malefemale = localeInfo.FOR_MALE
                        itemName += " ( " + malefemale + " )"

                self.AppendTextLine(itemName, self.CHANGELOOK_ITEMNAME_COLOR, True)

    def __AppendAcceItemAffectInformation(
        self, oriitemVnum, window_type, slotIndex, metinSlot
    ):

        (affectTypeAcce, affectValueAcce) = item.GetAffect(0)
        socketInDrainValue = 0

        if item.GetRefinedVnum() == 0:
            ## Àü¼³µî±Þ Èí¼öÀ² ¾Ë¾Æ¿À±â.
            if window_type == player.INVENTORY:
                socketInDrainValue = player.GetItemMetinSocket(slotIndex, 0)
                if not metinSlot[1] == 0 and socketInDrainValue == 0:
                    socketInDrainValue = metinSlot[1]
            elif window_type == player.ACCEREFINE:
                socketInDrainValue = player.GetAcceItemMetinSocket(slotIndex, 0)
            elif window_type == player.SAFEBOX:
                socketInDrainValue = safebox.GetItemMetinSocket(slotIndex, 0)

            drainlate = socketInDrainValue / 100.0
        else:
            drainlate = affectValueAcce / 100.0
            socketInDrainValue = affectValueAcce

        if socketInDrainValue == 0:
            return

        ## Á¤º¸ »Ì¾Æ¿Ã ¾ÆÀÌÅÛ ¾Ë¾Æ¿À±â.
        if window_type == player.INVENTORY:
            socketInDrainItemVnum = player.GetItemMetinSocket(slotIndex, 1)
            if not metinSlot[0] == 0 and socketInDrainItemVnum == 0:
                socketInDrainItemVnum = metinSlot[1]
        elif window_type == player.ACCEREFINE:
            socketInDrainItemVnum = player.GetAcceItemMetinSocket(slotIndex, 1)
        elif window_type == player.SAFEBOX:
            socketInDrainItemVnum = safebox.GetItemMetinSocket(slotIndex, 1)

        ## Èí¼öÇÑ ¾Ç¼¼¼­¸® ¾ÆÀÌÅÛ¸¸ º¸¿©ÁÖ±âÀ§ÇØ °É·¯ÁØ´Ù.
        if socketInDrainItemVnum == 0:
            return

        item.SelectItem(socketInDrainItemVnum)

        itemtype = item.GetItemType()

        if itemtype == item.ITEM_TYPE_WEAPON:
            ## Ãß°¡ µ¥¹ÌÁö
            addPower = item.GetValue(5)

            ## ¹«±â. Attact
            if item.GetValue(3) >= 1 and item.GetValue(4) >= 1:
                minPower = max(((item.GetValue(3) + addPower) * drainlate), 1)
                maxPower = max(((item.GetValue(4) + addPower) * drainlate), 1)

                if maxPower > minPower:
                    self.AppendTextLineAcce(
                        localeInfo.TOOLTIP_ITEM_ATT_POWER.format(minPower, maxPower),
                        self.POSITIVE_COLOR,
                    )
                else:
                    self.AppendTextLineAcce(
                        localeInfo.TOOLTIP_ITEM_ATT_POWER_ONE_ARG.format(minPower),
                        self.POSITIVE_COLOR,
                    )

            ## ¹«±â. Magic
            if item.GetValue(1) >= 1 or item.GetValue(2) >= 1:
                minMagicAttackPower = max(
                    ((item.GetValue(1) + addPower) * drainlate), 1
                )
                maxMagicAttackPower = max(
                    ((item.GetValue(2) + addPower) * drainlate), 1
                )

                if maxMagicAttackPower > minMagicAttackPower:
                    self.AppendTextLineAcce(
                        localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER.format(
                            minMagicAttackPower, maxMagicAttackPower
                        ),
                        self.POSITIVE_COLOR,
                    )
                else:
                    self.AppendTextLineAcce(
                        localeInfo.TOOLTIP_ITEM_MAGIC_ATT_POWER_ONE_ARG.format(
                            minMagicAttackPower
                        ),
                        self.POSITIVE_COLOR,
                    )

        elif itemtype == item.ITEM_TYPE_ARMOR:

            ## ¹æ¾î·Â
            defBonus = item.GetValue(5) * 2  ## ¹æ¾î·Â Ç¥½Ã Àß¸ø µÇ´Â ¹®Á¦¸¦ ¼öÁ¤
            if item.GetValue(1) >= 1:
                defGrade = max(((item.GetValue(1) + defBonus) * drainlate), 1)
                if defGrade > 0:
                    self.AppendSpace(5)
                    self.AppendTextLineAcce(
                        localeInfo.TOOLTIP_ITEM_DEF_GRADE.format(defGrade),
                        self.GetChangeTextLineColor(defGrade),
                    )

        ## Itemtable ¿¡ ÀÖ´Â ±âº» ¼Ó¼º Àû¿ë.
        ## ¾Ç¼¼¼­¸®¿¡´Â - °© ex ÀÌµ¿¼Óµµ Àû¿ë ¾ÈÇÔ.
        for i in xrange(item.ITEM_APPLY_MAX_NUM):
            (affectType, affectValue) = item.GetAffect(i)
            if affectValue > 0:
                affectValue = max((affectValue * drainlate), 1)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    self.AppendTextLineAcce(
                        affectString, self.GetChangeTextLineColor(affectValue)
                    )

        item.SelectItem(oriitemVnum)

    def __AppendAffectInformationAcce(self, slotIndex, window_type, metinSlot):
        for i in xrange(item.ITEM_APPLY_MAX_NUM):
            (affectType, affectValue) = item.GetAffect(i)

            if item.GetRefinedVnum() == 0:
                ## Àü¼³µî±Þ Èí¼öÀ² ¾Ë¾Æ¿À±â.
                if window_type == player.INVENTORY:
                    socketInDrainValue = player.GetItemMetinSocket(slotIndex, 0)
                    if not metinSlot[1] == 0 and socketInDrainValue == 0:
                        socketInDrainValue = metinSlot[1]
                elif window_type == player.ACCEREFINE:
                    socketInDrainValue = player.GetAcceItemMetinSocket(slotIndex, 0)
                elif window_type == player.SAFEBOX:
                    socketInDrainValue = safebox.GetItemMetinSocket(slotIndex, 0)

                affectString = self.__GetAffectString(affectType, socketInDrainValue)
            else:
                affectString = self.__GetAffectString(affectType, affectValue)

            if affectString:
                if affectType == item.APPLY_ACCEDRAIN_RATE:
                    self.AppendTextLine(affectString, self.CONDITION_COLOR)
            else:
                if app.ENABLE_ACCE_SECOND_COSTUME_SYSTEM:
                    if affectType == item.APPLY_ACCEDRAIN_RATE:
                        if (
                            item.GetRefinedVnum() == 0
                            and slotIndex == 2
                            and socketInDrainValue == 0
                        ):
                            socketInDrainValue = player.GetAcceItemMetinSocket(0, 0)
                            ## ¸ÞÀÎ¿¡ µî·ÏµÈ ¾ÆÀÌÅÛÀÌ Àü¼³ÀÏ¶§¸¸.
                            if socketInDrainValue == 0:
                                return
                            plusdrainlate = 5
                            if socketInDrainValue >= 21:
                                plusdrainlate = plusdrainlate - (
                                    socketInDrainValue - 20
                                )
                            affectString = localeInfo.TOOLTIP_APPLY_ACCEDRAIN_RATE(
                                socketInDrainValue
                            ) + " ~ %d%%" % (socketInDrainValue + plusdrainlate)
                            self.AppendTextLine(affectString, self.CONDITION_COLOR)
                else:
                    pass

    def AppendAntiflagInformation(self):

        antiFlagList = (
            item.IsAntiFlag(item.ITEM_ANTIFLAG_DROP),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_SELL),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_GIVE),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_PKDROP),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_MYSHOP),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_SAFEBOX),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_CHANGE_ATTRIBUTE),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_DESTROY),
            item.IsAntiFlag(item.ITEM_ANTIFLAG_CHANGELOOK),
        )

        antiFlagNames = ""
        flagCount = 0
        for i in xrange(self.ANTI_FLAG_COUNT):

            name = self.ANTI_FLAG_NAMES[i]
            flag = antiFlagList[i]

            if flag:
                if flagCount > 0:
                    antiFlagNames += " "
                flagCount = flagCount + 1
                antiFlagNames += name

        if flagCount > 0:
            self.AppendSpace(5)
            self.AppendTextLine(localeInfo.NOT_POSSIBLE, self.NEGATIVE_COLOR)
            textLine = self.AppendTextLine(antiFlagNames, self.NEGATIVE_COLOR)

            self.ResizeToolTip()

    def __AppendWearableInformation(self):

        self.AppendSpace(5)
        self.AppendTextLine(localeInfo.TOOLTIP_ITEM_WEARABLE_JOB, self.NORMAL_COLOR)

        if app.ENABLE_WOLFMAN_CHARACTER:
            flagList = (
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_WARRIOR),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_ASSASSIN),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_SURA),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_SHAMAN),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_WOLFMAN),
            )
        else:
            flagList = (
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_WARRIOR),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_ASSASSIN),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_SURA),
                not item.IsAntiFlag(item.ITEM_ANTIFLAG_SHAMAN),
            )

        characterNames = ""
        self.AppendSpace(2)
        for i in xrange(self.CHARACTER_COUNT):

            name = self.CHARACTER_NAMES[i]
            flag = flagList[i]

            if flag:
                characterNames += " "
                characterNames += ReplaceEmoticons(name)

        textLine = self.AppendTextLine(characterNames, self.NORMAL_COLOR, True)
        # self.AppendSpace(10)

        if item.IsAntiFlag(item.ITEM_ANTIFLAG_MALE):
            textLine = self.AppendTextLine(
                localeInfo.FOR_FEMALE, self.NORMAL_COLOR, True
            )

        if item.IsAntiFlag(item.ITEM_ANTIFLAG_FEMALE):
            textLine = self.AppendTextLine(localeInfo.FOR_MALE, self.NORMAL_COLOR, True)

    def __AppendPotionInformation(self):
        self.AppendSpace(5)

        healHP = item.GetValue(0)
        healSP = item.GetValue(1)
        healStatus = item.GetValue(2)
        healPercentageHP = item.GetValue(3)
        healPercentageSP = item.GetValue(4)

        if healHP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_HP_POINT.format(healHP),
                self.GetChangeTextLineColor(healHP),
            )
        if healSP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_SP_POINT.format(healSP),
                self.GetChangeTextLineColor(healSP),
            )
        if healStatus != 0:
            self.AppendTextLine(localeInfo.TOOLTIP_POTION_CURE)
        if healPercentageHP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_HP_PERCENT.format(healPercentageHP),
                self.GetChangeTextLineColor(healPercentageHP),
            )
        if healPercentageSP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_SP_PERCENT.format(healPercentageSP),
                self.GetChangeTextLineColor(healPercentageSP),
            )

    def __AppendAbilityPotionInformation(self):

        self.AppendSpace(5)

        abilityType = item.GetValue(0)
        time = item.GetValue(1)
        point = item.GetValue(2)

        if abilityType == item.APPLY_ATT_SPEED:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_ATTACK_SPEED.format(point),
                self.GetChangeTextLineColor(point),
            )
        elif abilityType == item.APPLY_MOV_SPEED:
            self.AppendTextLine(
                localeInfo.TOOLTIP_POTION_PLUS_MOVING_SPEED.format(point),
                self.GetChangeTextLineColor(point),
            )

        if time > 0:
            minute = time / 60
            second = time % 60
            timeString = localeInfo.TOOLTIP_POTION_TIME

            if minute > 0:
                timeString += str(minute) + localeInfo.TOOLTIP_POTION_MIN
            if second > 0:
                timeString += " " + str(second) + localeInfo.TOOLTIP_POTION_SEC

            self.AppendTextLine(timeString)

    def GetPriceColor(self, price):
        if price >= constInfo.HIGH_PRICE:
            return self.HIGH_PRICE_COLOR
        if price >= constInfo.MIDDLE_PRICE:
            return self.MIDDLE_PRICE_COLOR
        else:
            return self.LOW_PRICE_COLOR

    def AppendPrice(self, price):
        self.AppendSpace(5)
        if price < 1:
            self.AppendTextLine(localeInfo.NPC_0YANG, self.POSITIVE_COLOR)
        else:
            self.AppendTextLine(localeInfo.TOOLTIP_BUYPRICE.format(localeInfo.NumberToMoneyString(price)),
                                self.GetPriceColor(price))



    def AppendPriceBySecondaryCoin(self, price, coinTypeString):
        self.AppendSpace(5)
        self.AppendTextLine(
            localeInfo.TOOLTIP_BUYPRICE.format(
                localeInfo.NumberToSecondaryCoinString(price, coinTypeString)
            ),
            self.GetPriceColor(price),
        )

    def AppendSellingPrice(self, price):
        if uiPrivateShopBuilder.IsBuildingPrivateShop():
            self.AppendTextLine(
                localeInfo.TOOLTIP_SELLPRICE.format(
                    localeInfo.NumberToMoneyString(price)
                ),
                self.GetPriceColor(price),
            )
            self.AppendSpace(5)
        elif item.IsAntiFlag(item.ITEM_ANTIFLAG_SELL):
            self.AppendTextLine(localeInfo.TOOLTIP_ANTI_SELL, self.DISABLE_COLOR)
            self.AppendSpace(5)
        else:
            self.AppendTextLine(
                localeInfo.TOOLTIP_SELLPRICE.format(
                    localeInfo.NumberToMoneyString(price)
                ),
                self.GetPriceColor(price),
            )
            self.AppendSpace(5)

    def AppendMetinInformation(self):
        affectType, affectValue = item.GetAffect(0)

        affectString = self.__GetAffectString(affectType, affectValue)

        if affectString:
            self.AppendSpace(5)
            self.AppendTextLine(affectString, self.GetChangeTextLineColor(affectValue))

    def AppendMetinWearInformation(self):

        self.AppendSpace(5)
        self.AppendTextLine(localeInfo.TOOLTIP_SOCKET_REFINABLE_ITEM, self.NORMAL_COLOR)

        flagList = (
            item.IsWearableFlag(item.WEARABLE_BODY),
            item.IsWearableFlag(item.WEARABLE_HEAD),
            item.IsWearableFlag(item.WEARABLE_FOOTS),
            item.IsWearableFlag(item.WEARABLE_WRIST),
            item.IsWearableFlag(item.WEARABLE_WEAPON),
            item.IsWearableFlag(item.WEARABLE_NECK),
            item.IsWearableFlag(item.WEARABLE_EAR),
            item.IsWearableFlag(item.WEARABLE_UNIQUE),
            item.IsWearableFlag(item.WEARABLE_SHIELD),
            item.IsWearableFlag(item.WEARABLE_ARROW),
        )

        wearNames = ""
        for i in xrange(self.WEAR_COUNT):

            name = self.WEAR_NAMES[i]
            flag = flagList[i]

            if flag:
                wearNames += "  "
                wearNames += name

        textLine = ui.TextLine()
        textLine.SetParent(self)
        textLine.SetFontName(self.defFontName)
        textLine.SetPosition(0, self.toolTipHeight)
        textLine.SetHorizontalAlignCenter()
        textLine.SetPackedFontColor(self.NORMAL_COLOR)
        textLine.SetText(wearNames)
        textLine.Show()
        self.childrenList.append(textLine)

        self.toolTipHeight += self.TEXT_LINE_HEIGHT
        self.ResizeToolTip()

    def GetMetinSocketType(self, number):
        if player.METIN_SOCKET_TYPE_NONE == number:
            return player.METIN_SOCKET_TYPE_NONE
        elif player.METIN_SOCKET_TYPE_SILVER == number:
            return player.METIN_SOCKET_TYPE_SILVER
        elif player.METIN_SOCKET_TYPE_GOLD == number:
            return player.METIN_SOCKET_TYPE_GOLD
        else:
            if not item.SelectItem(number):
                return player.METIN_SOCKET_TYPE_NONE
            else:
                if item.METIN_NORMAL == item.GetItemSubType():
                    return player.METIN_SOCKET_TYPE_SILVER
                elif item.METIN_GOLD == item.GetItemSubType():
                    return player.METIN_SOCKET_TYPE_GOLD
                elif "USE_PUT_INTO_ACCESSORY_SOCKET" == item.GetUseType(number):
                    return player.METIN_SOCKET_TYPE_SILVER
                elif "USE_PUT_INTO_RING_SOCKET" == item.GetUseType(number):
                    return player.METIN_SOCKET_TYPE_SILVER
                elif "USE_PUT_INTO_BELT_SOCKET" == item.GetUseType(number):
                    return player.METIN_SOCKET_TYPE_SILVER

        return player.METIN_SOCKET_TYPE_NONE

    def GetMetinItemIndex(self, number):
        if long(player.METIN_SOCKET_TYPE_SILVER) == long(number):
            return 0
        if long(player.METIN_SOCKET_TYPE_GOLD) == long(number):
            return 0

        return long(number)

    def __AppendAccessoryMetinSlotInfo(self, metinSlot, mtrlVnum):
        ACCESSORY_SOCKET_MAX_SIZE = 3

        cur = min(metinSlot[0], ACCESSORY_SOCKET_MAX_SIZE)
        end = min(metinSlot[1], ACCESSORY_SOCKET_MAX_SIZE)

        affectType1, affectValue1 = item.GetAffect(0)
        affectList1 = [
            0,
            max(1, affectValue1 * 10 / 100),
            max(2, affectValue1 * 20 / 100),
            max(3, affectValue1 * 40 / 100),
        ]
        affectList1Perm = [
            0,
            max(1, affectValue1 * 15 / 100),
            max(2, affectValue1 * 30 / 100),
            max(3, affectValue1 * 60 / 100),
        ]

        affectType2, affectValue2 = item.GetAffect(1)
        affectList2 = [
            0,
            max(1, affectValue2 * 10 / 100),
            max(2, affectValue2 * 20 / 100),
            max(3, affectValue2 * 40 / 100),
        ]
        affectList2Perm = [
            0,
            max(1, affectValue2 * 15 / 100),
            max(2, affectValue2 * 30 / 100),
            max(3, affectValue2 * 60 / 100),
        ]

        affectType3, affectValue3 = item.GetAffect(2)
        affectList3 = [
            0,
            max(1, affectValue3 * 10 / 100),
            max(2, affectValue3 * 20 / 100),
            max(3, affectValue3 * 40 / 100),
        ]
        affectList3Perm = [
            0,
            max(1, affectValue3 * 15 / 100),
            max(2, affectValue3 * 30 / 100),
            max(3, affectValue3 * 60 / 100),
        ]

        affectType4, affectValue4 = item.GetAffect(3)
        affectList4 = [
            0,
            max(1, affectValue4 * 10 / 100),
            max(2, affectValue4 * 20 / 100),
            max(3, affectValue4 * 40 / 100),
        ]
        affectList4Perm = [
            0,
            max(1, affectValue4 * 15 / 100),
            max(2, affectValue4 * 30 / 100),
            max(3, affectValue4 * 60 / 100),
        ]

        affectType5, affectValue5 = item.GetAffect(4)
        affectList5 = [
            0,
            max(1, affectValue5 * 10 / 100),
            max(2, affectValue5 * 20 / 100),
            max(3, affectValue5 * 40 / 100),
        ]
        affectList5Perm = [
            0,
            max(1, affectValue5 * 15 / 100),
            max(2, affectValue5 * 30 / 100),
            max(3, affectValue5 * 60 / 100),
        ]

        affectType6, affectValue6 = item.GetAffect(5)
        affectList6 = [
            0,
            max(1, affectValue6 * 10 / 100),
            max(2, affectValue6 * 20 / 100),
            max(3, affectValue6 * 40 / 100),
        ]
        affectList6Perm = [
            0,
            max(1, affectValue6 * 15 / 100),
            max(2, affectValue6 * 30 / 100),
            max(3, affectValue6 * 60 / 100),
        ]

        mtrlPos = 0
        mtrlList = [mtrlVnum] * cur + [player.METIN_SOCKET_TYPE_SILVER] * (end - cur)
        for i, mtrl in enumerate(mtrlList):
            leftTime = 0
            isPerm = False

            if cur == mtrlPos + 1:
                leftTime = metinSlot[2]

            if metinSlot[3] > i:
                leftTime = 60 * 60 * 24 * 365 * 50
                isPerm = True

            affectString1 = self.__GetAffectString(
                affectType1,
                (
                    affectList1[mtrlPos + 1] - affectList1[mtrlPos]
                    if not isPerm
                    else affectList1Perm[mtrlPos + 1] - affectList1Perm[mtrlPos]
                ),
            )
            affectString2 = self.__GetAffectString(
                affectType2,
                (
                    affectList2[mtrlPos + 1] - affectList2[mtrlPos]
                    if not isPerm
                    else affectList2Perm[mtrlPos + 1] - affectList2Perm[mtrlPos]
                ),
            )
            affectString3 = self.__GetAffectString(
                affectType3,
                (
                    affectList3[mtrlPos + 1] - affectList3[mtrlPos]
                    if not isPerm
                    else affectList3Perm[mtrlPos + 1] - affectList3Perm[mtrlPos]
                ),
            )
            affectString4 = self.__GetAffectString(
                affectType4,
                (
                    affectList4[mtrlPos + 1] - affectList4[mtrlPos]
                    if not isPerm
                    else affectList4Perm[mtrlPos + 1] - affectList4Perm[mtrlPos]
                ),
            )
            affectString5 = self.__GetAffectString(
                affectType5,
                (
                    affectList5[mtrlPos + 1] - affectList5[mtrlPos]
                    if not isPerm
                    else affectList5Perm[mtrlPos + 1] - affectList5Perm[mtrlPos]
                ),
            )
            affectString6 = self.__GetAffectString(
                affectType6,
                (
                    affectList6[mtrlPos + 1] - affectList6[mtrlPos]
                    if not isPerm
                    else affectList6Perm[mtrlPos + 1] - affectList6Perm[mtrlPos]
                ),
            )

            self.__AppendMetinSlotInfo_AppendMetinSocketData(
                mtrlPos,
                mtrl,
                affectString1,
                affectString2,
                affectString3,
                affectString4,
                affectString5,
                affectString6,
                leftTime,
            )
            mtrlPos += 1

    def __AppendMetinSlotInfo(self, metinSlot):
        if self.__AppendMetinSlotInfo_IsEmptySlotList(metinSlot):
            return

        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            self.__AppendMetinSlotInfo_AppendMetinSocketData(i, long(metinSlot[i]))

    def __AppendMetinSlotInfo_IsEmptySlotList(self, metinSlot):
        if 0 == metinSlot:
            return 1

        if len(metinSlot) == 0:
            return 1

        count = 0
        for i in xrange(player.METIN_SOCKET_MAX_NUM):
            metinSlotData = long(metinSlot[i])
            if 0 != self.GetMetinSocketType(metinSlotData):
                count = count + 1
                if 0 != self.GetMetinItemIndex(metinSlotData):
                    return 0

        if count > 3:
            return 0

        return 1

    def __AppendMetinSlotInfo_AppendMetinSocketData(
        self,
        index,
        metinSlotData,
        custumAffectString="",
        custumAffectString2="",
        custumAffectString3="",
        custumAffectString4="",
        custumAffectString5="",
        custumAffectString6="",
        leftTime=0,
    ):
        slotType = self.GetMetinSocketType(metinSlotData)
        itemIndex = self.GetMetinItemIndex(metinSlotData)

        if 0 == slotType:
            return

        self.AppendSpace(5)

        slotImage = ui.ImageBox()
        slotImage.SetParent(self)
        slotImage.Show()

        ## Name
        nameTextLine = ui.TextLine()
        nameTextLine.SetParent(self)
        nameTextLine.SetFontName(self.defFontName)
        nameTextLine.SetPackedFontColor(self.NORMAL_COLOR)
        nameTextLine.SetOutline()
        nameTextLine.Show()

        self.childrenList.append(nameTextLine)

        if player.METIN_SOCKET_TYPE_SILVER == slotType:
            slotImage.LoadImage("d:/ymir work/ui/game/windows/metin_slot_silver.sub")
        elif player.METIN_SOCKET_TYPE_GOLD == slotType:
            slotImage.LoadImage("d:/ymir work/ui/game/windows/metin_slot_gold.sub")

        self.childrenList.append(slotImage)

        if localeInfo.IsARABIC():
            slotImage.SetPosition(
                self.toolTipWidth - slotImage.GetWidth() - 9, self.toolTipHeight - 1
            )
            nameTextLine.SetPosition(self.toolTipWidth - 50, self.toolTipHeight + 2)
        else:
            slotImage.SetPosition(9, self.toolTipHeight - 1)
            nameTextLine.SetPosition(50, self.toolTipHeight + 2)

        metinImage = ui.ImageBox()
        metinImage.SetParent(self)
        metinImage.Show()
        self.childrenList.append(metinImage)

        if itemIndex:

            item.SelectItem(itemIndex)

            ## Image
            try:
                metinImage.LoadImage(item.GetIconImageFileName())
            except:
                logging.exception(
                    "Failed to load item icon for %d: %s",
                    itemIndex,
                    item.GetIconImageFileName(),
                )

            nameTextLine.SetText(self._GetItemName())

            ## Affect
            affectTextLine = ui.TextLine()
            affectTextLine.SetParent(self)
            affectTextLine.SetFontName(self.defFontName)
            affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
            affectTextLine.SetOutline()
            affectTextLine.Show()

            if localeInfo.IsARABIC():
                metinImage.SetPosition(
                    self.toolTipWidth - metinImage.GetWidth() - 10, self.toolTipHeight
                )
                affectTextLine.SetPosition(
                    self.toolTipWidth - 50, self.toolTipHeight + 16 + 2
                )
            else:
                metinImage.SetPosition(10, self.toolTipHeight)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2)

            if custumAffectString:
                affectTextLine.SetText(custumAffectString)
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(0)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine.SetText(affectString)
            else:
                affectTextLine.SetText(localeInfo.TOOLTIP_APPLY_NOAFFECT)

            self.childrenList.append(affectTextLine)

            if custumAffectString2:
                affectTextLine = ui.TextLine()
                affectTextLine.SetParent(self)
                affectTextLine.SetFontName(self.defFontName)
                affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                affectTextLine.SetOutline()
                affectTextLine.Show()
                affectTextLine.SetText(custumAffectString2)
                self.childrenList.append(affectTextLine)
                self.toolTipHeight += 16 + 2
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(1)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine = ui.TextLine()
                    affectTextLine.SetParent(self)
                    affectTextLine.SetFontName(self.defFontName)
                    affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                    affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                    affectTextLine.SetOutline()
                    affectTextLine.Show()
                    affectTextLine.SetText(affectString)
                    self.childrenList.append(affectTextLine)
                    self.toolTipHeight += 16 + 2

            if custumAffectString3:
                affectTextLine = ui.TextLine()
                affectTextLine.SetParent(self)
                affectTextLine.SetFontName(self.defFontName)
                affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                affectTextLine.SetOutline()
                affectTextLine.Show()
                affectTextLine.SetText(custumAffectString3)
                self.childrenList.append(affectTextLine)
                self.toolTipHeight += 16 + 2
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(2)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine = ui.TextLine()
                    affectTextLine.SetParent(self)
                    affectTextLine.SetFontName(self.defFontName)
                    affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                    affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                    affectTextLine.SetOutline()
                    affectTextLine.Show()
                    affectTextLine.SetText(affectString)
                    self.childrenList.append(affectTextLine)
                    self.toolTipHeight += 16 + 2

            if custumAffectString4:
                affectTextLine = ui.TextLine()
                affectTextLine.SetParent(self)
                affectTextLine.SetFontName(self.defFontName)
                affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                affectTextLine.SetOutline()
                affectTextLine.Show()
                affectTextLine.SetText(custumAffectString4)
                self.childrenList.append(affectTextLine)
                self.toolTipHeight += 16 + 2
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(3)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine = ui.TextLine()
                    affectTextLine.SetParent(self)
                    affectTextLine.SetFontName(self.defFontName)
                    affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                    affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                    affectTextLine.SetOutline()
                    affectTextLine.Show()
                    affectTextLine.SetText(affectString)
                    self.childrenList.append(affectTextLine)
                    self.toolTipHeight += 16 + 2

            if custumAffectString5:
                affectTextLine = ui.TextLine()
                affectTextLine.SetParent(self)
                affectTextLine.SetFontName(self.defFontName)
                affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                affectTextLine.SetOutline()
                affectTextLine.Show()
                affectTextLine.SetText(custumAffectString5)
                self.childrenList.append(affectTextLine)
                self.toolTipHeight += 16 + 2
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(4)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine = ui.TextLine()
                    affectTextLine.SetParent(self)
                    affectTextLine.SetFontName(self.defFontName)
                    affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                    affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                    affectTextLine.SetOutline()
                    affectTextLine.Show()
                    affectTextLine.SetText(affectString)
                    self.childrenList.append(affectTextLine)
                    self.toolTipHeight += 16 + 2

            if custumAffectString6:
                affectTextLine = ui.TextLine()
                affectTextLine.SetParent(self)
                affectTextLine.SetFontName(self.defFontName)
                affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                affectTextLine.SetOutline()
                affectTextLine.Show()
                affectTextLine.SetText(custumAffectString6)
                self.childrenList.append(affectTextLine)
                self.toolTipHeight += 16 + 2
            elif itemIndex != constInfo.ERROR_METIN_STONE:
                affectType, affectValue = item.GetAffect(5)
                affectString = self.__GetAffectString(affectType, affectValue)
                if affectString:
                    affectTextLine = ui.TextLine()
                    affectTextLine.SetParent(self)
                    affectTextLine.SetFontName(self.defFontName)
                    affectTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                    affectTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                    affectTextLine.SetOutline()
                    affectTextLine.Show()
                    affectTextLine.SetText(affectString)
                    self.childrenList.append(affectTextLine)
                    self.toolTipHeight += 16 + 2

            if 0 != leftTime:
                timeText = (
                    localeInfo.LEFT_TIME + " : " + localeInfo.SecondToDHM(leftTime)
                )

                timeTextLine = ui.TextLine()
                timeTextLine.SetParent(self)
                timeTextLine.SetFontName(self.defFontName)
                timeTextLine.SetPackedFontColor(self.POSITIVE_COLOR)
                timeTextLine.SetPosition(50, self.toolTipHeight + 16 + 2 + 16 + 2)
                timeTextLine.SetOutline()
                timeTextLine.Show()
                timeTextLine.SetText(timeText)
                self.childrenList.append(timeTextLine)
                self.toolTipHeight += 16 + 2

        else:
            nameTextLine.SetText(localeInfo.TOOLTIP_SOCKET_EMPTY)

        self.toolTipHeight += 35
        self.ResizeToolTip()

    def __AppendFishInfo(self, size):
        if size > 0:
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.TOOLTIP_FISH_LEN.format(float(size) / 100.0),
                self.NORMAL_COLOR,
            )

    # Item expiration time
    def __AppendRemainingTime(self, seconds):
        self.AppendSpace(5)
        return self.AppendTextLine(
            localeInfo.LEFT_TIME + ": " + localeInfo.SecondToDHM(seconds),
            self.NORMAL_COLOR,
        )

    def __AppendMallItemLastTime(self, endTime):
        leftSec = max(0, endTime - app.GetGlobalTimeStamp())
        self.mallEndTime = endTime
        self.mallLastTime = self.__AppendRemainingTime(leftSec)

    def __AppendTimerBasedOnWearLastTime(self, metinSlot, limitValue):
        rem = metinSlot[0]
        if 0 == rem:
            rem = limitValue
        self.__AppendRemainingTime(rem)

    def __AppendRealTimeStartFirstUseLastTime(self, metinSlot, limitValue):
        useCount = metinSlot[1]
        remTime = metinSlot[0]

        # 한 번이라도 사용했다면 Socket0에 종료 시간(2012년 3월 1일 13시 01분 같은..) 이 박혀있음.
        # 사용하지 않았다면 Socket0에 이용가능시간(이를테면 600 같은 값. 초단위)이 들어있을 수 있고, 0이라면 Limit Value에 있는 이용가능시간을 사용한다.
        if 0 == useCount:
            if 0 == remTime:
                remTime = limitValue

            self.__AppendRemainingTime(remTime)
        else:
            self.__AppendMallItemLastTime(remTime)

    def __AppendRealTimeLastTime(self, metinSlot, limitValue):
        rem = metinSlot[0]
        if rem != 0:
            self.__AppendMallItemLastTime(rem)
        else:
            self.__AppendRemainingTime(limitValue)

    def __AppendTimeLimitInfo(self, metinSlot):
        for i in xrange(item.LIMIT_MAX_NUM):
            (limitType, limitValue) = item.GetLimit(i)

            if item.LIMIT_REAL_TIME == limitType:
                self.__AppendRealTimeLastTime(metinSlot, limitValue)
            elif item.LIMIT_REAL_TIME_START_FIRST_USE == limitType:
                self.__AppendRealTimeStartFirstUseLastTime(metinSlot, limitValue)
            elif item.LIMIT_TIMER_BASED_ON_WEAR == limitType:
                self.__AppendTimerBasedOnWearLastTime(metinSlot, limitValue)

    def OnKeyDown(self, key):
        if not self.IsRendering():
            return False

        if key == app.VK_CONTROL and self.renderTarget:
            if self.interface:
                if not self.IsShowingExchange() and not self.IsShowingSafeBox():
                    self.interface.OpenPreview(self.renderTarget)
            return True

        active = wndMgr.GetFocus()
        # if key == app.VK_MENU and not isinstance(active, ui.EditLine):
            # if self.interface:
                # self.interface.OpenItemWiki(self.itemVnum)
            # return True

        if key == app.VK_X and self.levelPet is not None:
            if self.interface:
                appInst.instance().GetNet().SendOpenLevelPet(
                    MakeItemPosition(player.INVENTORY, self.levelPet)
                )
                # self.interface.OpenLevelPetWindow(self.levelPet)
            return True
        return False

    def OnUpdate(self):
        if ToolTip:
            ToolTip.OnUpdate(self)

        if self.IsShow() and self.isSwitchbotItem and self.slotIndex != -1:
            self.UpdateAttributes(player.SWITCHBOT, self.slotIndex)
            self.updateEndTime = app.GetGlobalTimeStamp() + 50

        if self.mallLastTime:
            leftSec = max(0, self.mallEndTime - app.GetGlobalTimeStamp())
            self.mallLastTime.SetText(
                localeInfo.LEFT_TIME + " : " + localeInfo.SecondToDHMS(leftSec)
            )


class HyperlinkItemToolTip(ItemToolTip):
    def __init__(self):
        ItemToolTip.__init__(self, isPickable=True)
        self.SetWindowName(self.__class__.__name__)
        self.RemoveFlag("not_pick")

    def SetHyperlinkItem(self, tokens):
        def hexDecode(n):
            return int(n, 16)

        itemVnum, itemId = list(map(hexDecode, tokens[1:3]))
        hyperLinkItem = itemManager().GetHyperlinkItemData(itemId)
        if not hyperLinkItem:
            self.ClearToolTip()
            self.AppendTextLine("Not loaded")
            # self.AppendSpace(40)
            ItemToolTip.OnUpdate(self)
            return

        attrSlot = [(attr.type, attr.value) for attr in hyperLinkItem.attrs]

        self.ClearToolTip()
        self.AddItemData(
            itemVnum,
            hyperLinkItem.sockets,
            attrSlot,
            sealDate=hyperLinkItem.sealDate,
            changeLookVnum=hyperLinkItem.transVnum,
        )

        ItemToolTip.OnUpdate(self)

    if app.ENABLE_GROWTH_PET_SYSTEM:

        def SetHyperlinkPetItem(self, tokens):

            defaultTokenCount = 3 + player.METIN_SOCKET_MAX_NUM
            petTokenCount = defaultTokenCount + 10 + (player.PET_SKILL_COUNT_MAX * 2)

            if tokens and len(tokens) == petTokenCount:
                head, vnum, flag = tokens[:3]
                itemVnum = int(vnum, 16)
                metinSlot = [int(metin, 16) for metin in tokens[3:6]]

                attrSlot = [(0, 0)] * player.ATTRIBUTE_SLOT_MAX_NUM

                self.ClearToolTip()
                self.AddHyperLinkPetItemData(itemVnum, metinSlot, attrSlot, tokens[6:])
                ItemToolTip.OnUpdate(self)

    def OnUpdate(self):
        pass

    def OnMouseLeftButtonDown(self):
        self.Hide()


class SkillToolTip(ToolTip):
    POINT_NAME_DICT = {
        player.LEVEL: localeInfo.SKILL_TOOLTIP_LEVEL,
        player.IQ: localeInfo.SKILL_TOOLTIP_INT,
    }

    SKILL_TOOL_TIP_WIDTH = 200
    PARTY_SKILL_TOOL_TIP_WIDTH = 340

    PARTY_SKILL_EXPERIENCE_AFFECT_LIST = (
        (
            2,
            2,
            10,
        ),
        (
            8,
            3,
            20,
        ),
        (
            14,
            4,
            30,
        ),
        (
            22,
            5,
            45,
        ),
        (
            28,
            6,
            60,
        ),
        (
            34,
            7,
            80,
        ),
        (
            38,
            8,
            100,
        ),
    )

    PARTY_SKILL_PLUS_GRADE_AFFECT_LIST = (
        (
            4,
            2,
            1,
            0,
        ),
        (
            10,
            3,
            2,
            0,
        ),
        (
            16,
            4,
            2,
            1,
        ),
        (
            24,
            5,
            2,
            2,
        ),
    )

    PARTY_SKILL_ATTACKER_AFFECT_LIST = (
        (
            36,
            3,
        ),
        (
            26,
            1,
        ),
        (
            32,
            2,
        ),
    )

    SKILL_GRADE_NAME = {
        player.SKILL_GRADE_MASTER: localeInfo.SKILL_GRADE_NAME_MASTER,
        player.SKILL_GRADE_GRAND_MASTER: localeInfo.SKILL_GRADE_NAME_GRAND_MASTER,
        player.SKILL_GRADE_PERFECT_MASTER: localeInfo.SKILL_GRADE_NAME_PERFECT_MASTER,
        player.SKILL_GRADE_LEGENDARY_MASTER: localeInfo.SKILL_GRADE_NAME_LEGENDARY_MASTER,
    }

    AFFECT_NAME_DICT = {
        "HP": localeInfo.TOOLTIP_SKILL_AFFECT_ATT_POWER,
        "ATT_GRADE": localeInfo.TOOLTIP_SKILL_AFFECT_ATT_GRADE,
        "DEF_GRADE": localeInfo.TOOLTIP_SKILL_AFFECT_DEF_GRADE,
        "ATT_SPEED": localeInfo.TOOLTIP_SKILL_AFFECT_ATT_SPEED,
        "MOV_SPEED": localeInfo.TOOLTIP_SKILL_AFFECT_MOV_SPEED,
        "DODGE": localeInfo.TOOLTIP_SKILL_AFFECT_DODGE,
        "RESIST_NORMAL": localeInfo.TOOLTIP_SKILL_AFFECT_RESIST_NORMAL,
        "REFLECT_MELEE": localeInfo.TOOLTIP_SKILL_AFFECT_REFLECT_MELEE,
    }
    AFFECT_APPEND_TEXT_DICT = {
        "DODGE": "%",
        "RESIST_NORMAL": "%",
        "REFLECT_MELEE": "%",
    }

    def __init__(self):
        ToolTip.__init__(self, self.SKILL_TOOL_TIP_WIDTH)
        self.SetWindowName(self.__class__.__name__)

    def SetSkill(self, skillIndex, skillLevel=-1):

        if 0 == skillIndex:
            return

        if skill.SKILL_TYPE_GUILD == skill.GetSkillType(skillIndex):
            if self.SKILL_TOOL_TIP_WIDTH != self.toolTipWidth:
                self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
                self.ResizeToolTip()

            self.AppendDefaultData(skillIndex)
            self.AppendSkillConditionData(skillIndex)
            self.AppendGuildSkillData(skillIndex, skillLevel)

        else:
            if self.SKILL_TOOL_TIP_WIDTH != self.toolTipWidth:
                self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
                self.ResizeToolTip()

            slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)
            skillGrade = playerInst().GetSkillGrade(slotIndex)
            skillLevel = playerInst().GetSkillDisplayLevel(slotIndex)
            skillCurrentPercentage = playerInst().GetSkillCurrentEfficientPercentage(
                slotIndex
            )
            skillNextPercentage = playerInst().GetSkillNextEfficientPercentage(
                slotIndex
            )

            self.AppendDefaultData(skillIndex)
            self.AppendSkillConditionData(skillIndex)
            self.AppendSkillDataNew(
                slotIndex,
                skillIndex,
                skillGrade,
                skillLevel,
                skillCurrentPercentage,
                skillNextPercentage,
            )
            self.AppendSkillRequirement(skillIndex, skillLevel)
            self.AppendSkillLevelStep(skillIndex, skillCurrentPercentage, skillLevel)

        self.ShowToolTip()

    def AppendSkillLevelStep(self, skillIndex, skillCurrentPercentage, skillLevel=0):
        levelStep = skill.GetSkillLevelStep(skillIndex, skillCurrentPercentage)
        if levelStep > 1:
            skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)
            if skillMaxLevel != 1 and skillLevel >= skillMaxLevel:
                return

            self.AppendHorizontalLine()
            self.AppendTextLine(localeInfo.TOOLTIP_TEXT13.format(levelStep))
            self.AppendSpace(5)

    def SetSkillNew(self, slotIndex, skillIndex, skillGrade, skillLevel):

        if 0 == skillIndex:
            return

        if player.SKILL_INDEX_TONGSOL == skillIndex:

            slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)
            skillLevel = playerInst().GetSkillDisplayLevel(slotIndex)

            self.AppendDefaultData(skillIndex)
            self.AppendPartySkillData(skillGrade, skillLevel)

        elif player.SKILL_INDEX_RIDING == skillIndex:

            slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)
            self.AppendSupportSkillDefaultData(skillIndex, skillGrade, skillLevel, 30)

        elif player.SKILL_INDEX_SUMMON == skillIndex:

            maxLevel = 10

            self.ClearToolTip()
            self.__SetSkillTitle(skillIndex, skillGrade)

            ## Description
            description = skill.GetSkillDescription(skillIndex)
            self.AppendDescription(description, 25)

            if skillLevel == 10:
                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_SKILL_LEVEL_MASTER.format(skillLevel),
                    self.NORMAL_COLOR,
                )
                self.AppendTextLine(
                    localeInfo.SKILL_SUMMON_DESCRIPTION.format(skillLevel * 10),
                    self.NORMAL_COLOR,
                )

            else:
                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_SKILL_LEVEL.format(skillLevel), self.NORMAL_COLOR
                )
                self.__AppendSummonDescription(skillLevel, self.NORMAL_COLOR)

                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_SKILL_LEVEL.format(skillLevel + 1),
                    self.NEGATIVE_COLOR,
                )
                self.__AppendSummonDescription(skillLevel + 1, self.NEGATIVE_COLOR)

        elif skill.SKILL_TYPE_GUILD == skill.GetSkillType(skillIndex):

            if self.SKILL_TOOL_TIP_WIDTH != self.toolTipWidth:
                self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
                self.ResizeToolTip()

            self.AppendDefaultData(skillIndex)
            self.AppendSkillConditionData(skillIndex)
            self.AppendGuildSkillData(skillIndex, skillLevel)

        else:

            if self.SKILL_TOOL_TIP_WIDTH != self.toolTipWidth:
                self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
                self.ResizeToolTip()

            slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)
            if slotIndex:
                skillCurrentPercentage = (
                    playerInst().GetSkillCurrentEfficientPercentage(slotIndex)
                )
                skillNextPercentage = playerInst().GetSkillNextEfficientPercentage(
                    slotIndex
                )

                self.AppendDefaultData(skillIndex, skillGrade)
                self.AppendSkillConditionData(skillIndex)
                self.AppendSkillDataNew(
                    slotIndex,
                    skillIndex,
                    skillGrade,
                    skillLevel,
                    skillCurrentPercentage,
                    skillNextPercentage,
                )
                self.AppendSkillRequirement(skillIndex, skillLevel)
                self.AppendSkillLevelStep(skillIndex, skillCurrentPercentage)

        self.ShowToolTip()
        # self.AppendSpace(10)

    def __SetSkillTitle(self, skillIndex, skillGrade):
        if player.IsGameMaster() or app.IsEnableTestServerFlag():
            self.AppendTextLine(" skillIndex -> {}".format(skillIndex))

        self.SetTitle(skill.GetSkillName(skillIndex, skillGrade))
        self.__AppendSkillGradeName(skillIndex, skillGrade)

    def __AppendSkillGradeName(self, skillIndex, skillGrade):
        if skillGrade in self.SKILL_GRADE_NAME:
            self.AppendSpace(5)
            self.AppendTextLine(
                self.SKILL_GRADE_NAME[skillGrade].format(
                    skill.GetSkillName(skillIndex, 0)
                ),
                self.CAN_LEVEL_UP_COLOR,
            )

    def SetSkillOnlyName(self, slotIndex, skillIndex, skillGrade):
        if 0 == skillIndex:
            return

        slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)

        self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
        self.ResizeToolTip()

        self.ClearToolTip()
        self.__SetSkillTitle(skillIndex, skillGrade)
        self.AppendDefaultData(skillIndex, skillGrade)
        self.AppendSkillConditionData(skillIndex)
        self.ShowToolTip()

    def AppendDefaultData(self, skillIndex, skillGrade=0):
        self.ClearToolTip()
        self.__SetSkillTitle(skillIndex, skillGrade)

        ## Level Limit
        levelLimit = skill.GetSkillLevelLimit(skillIndex)
        if levelLimit > 0:

            color = self.NORMAL_COLOR
            if playerInst().GetPoint(player.LEVEL) < levelLimit:
                color = self.NEGATIVE_COLOR

            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.TOOLTIP_ITEM_LIMIT_LEVEL.format(levelLimit), color
            )

        ## Description
        description = skill.GetSkillDescription(skillIndex)
        self.AppendDescription(description, 25)

    def AppendSupportSkillDefaultData(
        self, skillIndex, skillGrade, skillLevel, maxLevel
    ):
        self.ClearToolTip()
        self.__SetSkillTitle(skillIndex, skillGrade)

        ## Description
        description = skill.GetSkillDescription(skillIndex)
        self.AppendDescription(description, 25)

        if 1 == skillGrade:
            skillLevel += 19
        elif 2 == skillGrade:
            skillLevel += 29
        elif 3 == skillGrade:
            skillLevel = 40

        self.AppendSpace(5)
        self.AppendTextLine(
            localeInfo.TOOLTIP_SKILL_LEVEL_WITH_MAX.format(skillLevel, maxLevel),
            self.NORMAL_COLOR,
        )

    def AppendSkillConditionData(self, skillIndex):
        conditionDataCount = skill.GetSkillConditionDescriptionCount(skillIndex)
        if conditionDataCount > 0:
            self.AppendSpace(5)
            for i in xrange(conditionDataCount):
                self.AppendTextLine(
                    skill.GetSkillConditionDescription(skillIndex, i),
                    self.CONDITION_COLOR,
                )

    def AppendGuildSkillData(self, skillIndex, skillLevel):
        skillMaxLevel = 7
        skillCurrentPercentage = float(skillLevel) / float(skillMaxLevel)
        skillNextPercentage = float(skillLevel + 1) / float(skillMaxLevel)

        ## Current Level
        if skillLevel > 0:
            if self.HasSkillLevelDescription(skillIndex, skillLevel):
                self.AppendSpace(5)

                if skillLevel == skillMaxLevel:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_LEVEL_MASTER.format(skillLevel),
                        self.NORMAL_COLOR,
                    )
                else:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_LEVEL.format(skillLevel),
                        self.NORMAL_COLOR,
                    )

                for i in xrange(skill.GetSkillAffectDescriptionCount(skillIndex)):
                    self.AppendTextLine(
                        skill.GetSkillAffectDescription(
                            skillIndex, i, skillCurrentPercentage, skillLevel
                        ),
                        self.ENABLE_COLOR,
                    )

                ## Cooltime
                coolTime = skill.GetSkillCoolTime(skillIndex, skillCurrentPercentage)

                if coolTime > 0:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_COOL_TIME + str(coolTime),
                        self.ENABLE_COLOR,
                    )

                ## SP
                needGSP = skill.GetSkillNeedSP(skillIndex, skillCurrentPercentage)

                if needGSP > 0:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_NEED_GSP.format(needGSP), self.ENABLE_COLOR
                    )

        ## Next Level
        if skillLevel < skillMaxLevel:
            if self.HasSkillLevelDescription(skillIndex, skillLevel + 1):
                self.AppendSpace(5)
                self.AppendTextLine(
                    localeInfo.TOOLTIP_NEXT_SKILL_LEVEL_1.format(
                        skillLevel + 1, skillMaxLevel
                    ),
                    self.DISABLE_COLOR,
                )

                for i in xrange(skill.GetSkillAffectDescriptionCount(skillIndex)):
                    self.AppendTextLine(
                        skill.GetSkillAffectDescription(
                            skillIndex, i, skillNextPercentage, skillLevel
                        ),
                        self.DISABLE_COLOR,
                    )

                ## Cooltime
                coolTime = skill.GetSkillCoolTime(skillIndex, skillNextPercentage)

                if coolTime > 0:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_COOL_TIME + str(coolTime),
                        self.DISABLE_COLOR,
                    )

                ## SP
                needGSP = skill.GetSkillNeedSP(skillIndex, skillNextPercentage)

                if needGSP > 0:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_NEED_GSP.format(needGSP), self.DISABLE_COLOR
                    )

    def AppendSkillDataNew(
        self,
        slotIndex,
        skillIndex,
        skillGrade,
        skillLevel,
        skillCurrentPercentage,
        skillNextPercentage,
    ):

        self.skillMaxLevelStartDict = {
            0: 17,
            1: 7,
            2: 10,
        }
        self.skillMaxLevelEndDict = {
            0: 20,
            1: 10,
            2: 10,
        }
        self.skillMaxLevelStartDict.update(
            {
                3: 10,
            }
        )
        self.skillMaxLevelEndDict.update(
            {
                3: 10,
            }
        )

        skillLevelUpPoint = 1
        realSkillGrade = playerInst().GetSkillGrade(slotIndex)
        realSkillLevel = playerInst().GetSkillLevel(slotIndex)
        skillMaxLevelStart = self.skillMaxLevelStartDict.get(realSkillGrade, 15)
        skillMaxLevelEnd = self.skillMaxLevelEndDict.get(realSkillGrade, 20)

        ## Current Level
        if skillLevel > 0:
            if self.HasSkillLevelDescription(skillIndex, skillLevel):
                self.AppendSpace(5)
                skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)
                if skillMaxLevel != 1:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_LEVEL.format(realSkillLevel),
                        self.NORMAL_COLOR,
                    )
                elif skillGrade == skill.SKILL_GRADE_COUNT:
                    pass
                elif skillLevel == skillMaxLevelEnd:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_LEVEL_MASTER.format(skillLevel),
                        self.NORMAL_COLOR,
                    )
                else:
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_SKILL_LEVEL.format(skillLevel),
                        self.NORMAL_COLOR,
                    )
                self.AppendSkillLevelDescriptionNew(
                    skillIndex,
                    skillCurrentPercentage,
                    skillLevel,
                    skillGrade,
                    self.ENABLE_COLOR,
                )

        ## Next Level
        if skillGrade != skill.SKILL_GRADE_COUNT:
            skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)
            if skillMaxLevel != 1 and realSkillLevel >= skillMaxLevel:
                return
            if skillLevel < skillMaxLevelEnd or (
                skillMaxLevel != 1 and realSkillLevel < skillMaxLevel
            ):
                if self.HasSkillLevelDescription(
                    skillIndex, skillLevel + skillLevelUpPoint
                ):
                    self.AppendSpace(5)

                    if skillIndex == 141 or skillIndex == 143:
                        self.AppendTextLine(
                            localeInfo.TOOLTIP_NEXT_SKILL_LEVEL_3.format(
                                skillLevel + 1
                            ),
                            self.DISABLE_COLOR,
                        )
                    elif skillIndex in range(180, 203):
                        self.AppendTextLine(
                            localeInfo.TOOLTIP_NEXT_SKILL_LEVEL_1.format(
                                realSkillLevel + 1, skillMaxLevel
                            ),
                            self.DISABLE_COLOR,
                        )
                    else:
                        self.AppendTextLine(
                            localeInfo.TOOLTIP_NEXT_SKILL_LEVEL_1.format(
                                skillLevel + 1, skillMaxLevelEnd
                            ),
                            self.DISABLE_COLOR,
                        )
                    self.AppendSkillLevelDescriptionNew(
                        skillIndex,
                        skillNextPercentage,
                        skillLevel,
                        skillGrade,
                        self.DISABLE_COLOR,
                    )

    def AppendSkillLevelDescriptionNew(
        self, skillIndex, skillPercentage, realSkillLevel, skillGrade, color
    ):
        affectDataCount = skill.GetNewAffectDataCount(skillIndex)

        if affectDataCount > 0:
            for i in xrange(affectDataCount):
                type, minValue, maxValue = skill.GetNewAffectData(
                    skillIndex, i, skillPercentage, 0
                )

                if type not in self.AFFECT_NAME_DICT:
                    continue

                minValue = int(minValue)
                maxValue = int(maxValue)
                affectText = self.AFFECT_NAME_DICT[type]

                if "HP" == type:
                    if minValue < 0 and maxValue < 0:
                        minValue *= -1
                        maxValue *= -1

                    else:
                        affectText = localeInfo.TOOLTIP_SKILL_AFFECT_HEAL

                affectText += str(minValue)
                if minValue != maxValue:
                    affectText += " - " + str(maxValue)
                affectText += self.AFFECT_APPEND_TEXT_DICT.get(type, "")

                self.AppendTextLine(affectText, color)
        else:
            for i in xrange(skill.GetSkillAffectDescriptionCount(skillIndex)):
                self.AppendTextLine(
                    skill.GetSkillAffectDescription(
                        skillIndex, i, skillPercentage, realSkillLevel
                    ),
                    color,
                )

        ## Duration
        duration = skill.GetDuration(skillIndex, skillPercentage)

        if duration > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_SKILL_DURATION.format(duration), color
            )

        ## Cooltime
        coolTime = skill.GetSkillCoolTime(skillIndex, skillPercentage)

        if coolTime > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_SKILL_COOL_TIME + str(coolTime), color
            )

        ## SP
        needSP = skill.GetSkillNeedSP(skillIndex, skillPercentage)

        if needSP != 0:
            continuationSP = skill.GetSkillContinuationSP(skillIndex, skillPercentage)

            if skill.IsUseHPSkill(skillIndex):
                self.AppendNeedHP(needSP, continuationSP, color)
            else:
                self.AppendNeedSP(needSP, continuationSP, color)

    def AppendSkillRequirement(self, skillIndex, skillLevel):

        skillMaxLevel = skill.GetSkillMaxLevel(skillIndex)

        if skillLevel >= skillMaxLevel:
            return

        isAppendHorizontalLine = False

        ## Requirement
        if skill.IsSkillRequirement(skillIndex):

            if not isAppendHorizontalLine:
                isAppendHorizontalLine = True
                self.AppendHorizontalLine()

            requirementData = skill.GetSkillRequirementData(skillIndex)
            for index, requireSkill in enumerate(requirementData):
                color = self.CANNOT_LEVEL_UP_COLOR
                if skill.CheckRequirementSueccess(skillIndex, index):
                    color = self.CAN_LEVEL_UP_COLOR
                self.AppendTextLine(
                    localeInfo.TOOLTIP_REQUIREMENT_SKILL_LEVEL.format(
                        skill.GetSkillName(requireSkill[0]), requireSkill[1]
                    ),
                    color,
                )

        ## Require Stat
        requireStatCount = skill.GetSkillRequireStatCount(skillIndex)
        if requireStatCount > 0:

            for i in xrange(requireStatCount):
                type, level = skill.GetSkillRequireStatData(skillIndex, i)
                if type in self.POINT_NAME_DICT:

                    if not isAppendHorizontalLine:
                        isAppendHorizontalLine = True
                        self.AppendHorizontalLine()

                    name = self.POINT_NAME_DICT[type]
                    color = self.CANNOT_LEVEL_UP_COLOR
                    if playerInst().GetPoint(type) >= level:
                        color = self.CAN_LEVEL_UP_COLOR
                    self.AppendTextLine(
                        localeInfo.TOOLTIP_REQUIREMENT_STAT_LEVEL.format(name, level),
                        color,
                    )

    def HasSkillLevelDescription(self, skillIndex, skillLevel):
        if skill.GetSkillAffectDescriptionCount(skillIndex) > 0:
            return True
        if skill.GetSkillCoolTime(skillIndex, skillLevel) > 0:
            return True
        if skill.GetSkillNeedSP(skillIndex, skillLevel) > 0:
            return True

        return False

    def AppendMasterAffectDescription(self, index, desc, color):
        self.AppendTextLine(desc, color)

    def AppendNextAffectDescription(self, index, desc):
        self.AppendTextLine(desc, self.DISABLE_COLOR)

    def AppendNeedHP(self, needSP, continuationSP, color):

        self.AppendTextLine(localeInfo.TOOLTIP_NEED_HP.format(needSP), color)

        if continuationSP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_NEED_HP_PER_SEC.format(continuationSP), color
            )

    def AppendNeedSP(self, needSP, continuationSP, color):

        if -1 == needSP:
            self.AppendTextLine(localeInfo.TOOLTIP_NEED_ALL_SP, color)

        else:
            self.AppendTextLine(localeInfo.TOOLTIP_NEED_SP.format(needSP), color)

        if continuationSP > 0:
            self.AppendTextLine(
                localeInfo.TOOLTIP_NEED_SP_PER_SEC.format(continuationSP), color
            )

    def AppendPartySkillData(self, skillGrade, skillLevel):
        if 1 == skillGrade:
            skillLevel += 19
        elif 2 == skillGrade:
            skillLevel += 29
        elif 3 == skillGrade:
            skillLevel = 40

        if skillLevel <= 0:
            return

        skillIndex = player.SKILL_INDEX_TONGSOL
        slotIndex = playerInst().GetSkillSlotIndexNew(skillIndex)
        skillPower = playerInst().GetSkillCurrentEfficientPercentage(slotIndex)
        k = skillPower
        self.AppendSpace(5)
        self.AppendTextLine(
            localeInfo.TOOLTIP_PARTY_SKILL_LEVEL.format(skillLevel), self.NORMAL_COLOR
        )

        if skillLevel >= 10:
            self.AppendTextLine(
                localeInfo.PARTY_SKILL_ATTACKER.format(chop(10 + 60 * k))
            )

        if skillLevel >= 20:
            self.AppendTextLine(
                localeInfo.PARTY_SKILL_BERSERKER.format(chop(1 + 5 * k))
            )
            self.AppendTextLine(
                localeInfo.PARTY_SKILL_TANKER.format(chop(50 + 1450 * k))
            )

        if skillLevel >= 25:
            self.AppendTextLine(localeInfo.PARTY_SKILL_BUFFER.format(chop(5 + 45 * k)))

        if skillLevel >= 35:
            self.AppendTextLine(
                localeInfo.PARTY_SKILL_SKILL_MASTER.format(chop(25 + 600 * k))
            )

        if skillLevel >= 40:
            self.AppendTextLine(
                localeInfo.PARTY_SKILL_DEFENDER.format(chop(5 + 30 * k))
            )

    def __AppendSummonDescription(self, skillLevel, color):
        if skillLevel > 1:
            self.AppendTextLine(
                localeInfo.SKILL_SUMMON_DESCRIPTION.format(skillLevel * 10), color
            )
        elif 1 == skillLevel:
            self.AppendTextLine(
                localeInfo.DRAGON_SOUL_STRENGTH.format(skillLevel), color
            )
        elif 0 == skillLevel:
            self.AppendTextLine(
                localeInfo.DRAGON_SOUL_STRENGTH.format(skillLevel), color
            )


if app.ENABLE_GROWTH_PET_SYSTEM:

    class PetSkillToolTip(ToolTip):

        PET_SKILL_TOOL_TIP_WIDTH = 255

        PET_SKILL_APPLY_DATA_DICT = {
            chr.PET_SKILL_AFFECT_JIJOONG_WARRIOR: localeInfo.TOOLTIP_APPLY_RESIST_WARRIOR,  # %d%%
            chr.PET_SKILL_AFFECT_JIJOONG_SURA: localeInfo.TOOLTIP_APPLY_RESIST_SURA,  # %d%%
            chr.PET_SKILL_AFFECT_JIJOONG_ASSASSIN: localeInfo.TOOLTIP_APPLY_RESIST_ASSASSIN,  # %d%%
            chr.PET_SKILL_AFFECT_JIJOONG_SHAMAN: localeInfo.TOOLTIP_APPLY_RESIST_SHAMAN,  # %d%%
            chr.PET_SKILL_AFFECT_JIJOONG_WOLFMAN: localeInfo.TOOLTIP_APPLY_RESIST_WOLFMAN,  # %d%%
            chr.PET_SKILL_AFFECT_PACHEON: localeInfo.TOOLTIP_MELEE_MAGIC_ATTBONUS_PER,  # %d%%
            chr.PET_SKILL_AFFECT_CHEONRYEONG: localeInfo.TOOLTIP_RESIST_MAGIC_REDUCTION,  # %d%%
            chr.PET_SKILL_AFFECT_BANYA: localeInfo.TOOLTIP_CAST_SPEED,  # %d%%
            chr.PET_SKILL_AFFECT_CHOEHOENBIMU: localeInfo.TOOLTIP_APPLY_PENETRATE_PCT,  # %d%%
            chr.PET_SKILL_AFFECT_HEAL: localeInfo.PET_SKILL_TOOLTIP_HEAL,  # %d%%, %d
            chr.PET_SKILL_AFFECT_STEALHP: localeInfo.TOOLTIP_APPLY_STEAL_HP,  # %d%%
            chr.PET_SKILL_AFFECT_STEALMP: localeInfo.TOOLTIP_APPLY_STEAL_SP,  # %d%%
            chr.PET_SKILL_AFFECT_BLOCK: localeInfo.TOOLTIP_APPLY_BLOCK,  # %d%%
            chr.PET_SKILL_AFFECT_REFLECT_MELEE: localeInfo.TOOLTIP_APPLY_REFLECT_MELEE,  # %d%%
            chr.PET_SKILL_AFFECT_GOLD_DROP: localeInfo.TOOLTIP_MALL_GOLDBONUS,  # %.1f%%
            chr.PET_SKILL_AFFECT_BOW_DISTANCE: localeInfo.TOOLTIP_BOW_DISTANCE,  # %dm
            chr.PET_SKILL_AFFECT_INVINCIBILITY: localeInfo.PET_SKILL_TOOLTIP_INVINVIBILITY,  # %d%%, %.1f
            chr.PET_SKILL_AFFECT_REMOVAL: localeInfo.PET_SKILL_TOOLTIP_REMOVAL,  # %d%%
        }

        def __init__(self):
            ToolTip.__init__(self, self.PET_SKILL_TOOL_TIP_WIDTH)

        def SetPetSkill(self, pet_id, slot, index):

            if 0 == pet_id:
                return

            self.ClearToolTip()

            (
                pet_skill_vnum,
                pet_skill_level,
                formula1,
                formula2,
                next_formula1,
                next_formula2,
                bonus_value,
            ) = player.GetPetSkillByIndex(pet_id, slot)

            if 0 == pet_skill_vnum:
                return

            (
                pet_skill_name,
                pet_skill_desc,
                pet_skill_use_type,
                pet_skill_cool_time,
            ) = skill.GetPetSkillInfo(pet_skill_vnum)

            # 1  ½ºÅ³¸í
            self.SetTitle(pet_skill_name)
            # 2  ½ºÅ³¼³¸í
            self.AppendDescription(pet_skill_desc, 30)
            # 3  ÇöÀç·¹º§
            self.AppendSpace(5)
            if bonus_value:
                self.AppendTextLine(
                    (localeInfo.DRAGON_SOUL_STRENGTH.format(pet_skill_level))
                    + " "
                    + localeInfo.PET_TOOLTIP_SKILL_BONUS_VALUE,
                    self.NORMAL_COLOR,
                )
            else:
                self.AppendTextLine(
                    localeInfo.DRAGON_SOUL_STRENGTH.format(pet_skill_level),
                    self.NORMAL_COLOR,
                )

            ## ÇöÀç ½ºÅ³ ¼öÄ¡ ³»¿ë
            if skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                self.__AppendPassiveSkill(
                    pet_skill_vnum, int(formula2), self.NORMAL_COLOR
                )

            elif skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                self.__AppendAutoSkill(
                    pet_skill_vnum, formula1, formula2, self.NORMAL_COLOR
                )
                # 6  Áö¼Ó½Ã°£ : ÆÐ½Ãºê ½ºÅ³ÀÏ °æ¿ì ¾øÀ½
                self.__AppendRemainsTime()
                # 7  ÄðÅ¸ÀÓ     : ÆÐ½Ãºê ½ºÅ³ÀÏ °æ¿ì ¾øÀ½
                self.__AppendCoolTime(pet_skill_cool_time, self.NORMAL_COLOR)

            ## ´ÙÀ½ ·¹º§ÀÌ Á¸ÀçÇÑ´Ù¸é...
            nextSkillLevel = pet_skill_level + 1
            maxSkillLevel = player.PET_GROWTH_SKILL_LEVEL_MAX
            if nextSkillLevel <= maxSkillLevel:

                ## ¶óÀÎ »ý¼º ---------------------------
                self.AppendHorizontalLine()
                # 8  ´ÙÀ½·¹º§ : 00 (ÃÖ´ë 20)
                self.__AppendNextLevel(nextSkillLevel, maxSkillLevel)

                ## ´ÙÀ½ ½ºÅ³ ¼öÄ¡ ³»¿ë
                if skill.PET_SKILL_USE_TYPE_PASSIVE == pet_skill_use_type:
                    self.__AppendPassiveSkill(
                        pet_skill_vnum, int(next_formula2), self.NEGATIVE_COLOR
                    )

                elif skill.PET_SKILL_USE_TYPE_AUTO == pet_skill_use_type:
                    self.__AppendAutoSkill(
                        pet_skill_vnum,
                        next_formula1,
                        next_formula2,
                        self.NEGATIVE_COLOR,
                    )
                    # 6  Áö¼Ó½Ã°£ : ÆÐ½Ãºê ½ºÅ³ÀÏ °æ¿ì ¾øÀ½
                    self.__AppendRemainsTime()
                    # 7  ÄðÅ¸ÀÓ     : ÆÐ½Ãºê ½ºÅ³ÀÏ °æ¿ì ¾øÀ½
                    self.__AppendCoolTime(pet_skill_cool_time, self.NEGATIVE_COLOR)

            self.ResizeToolTip()
            self.ShowToolTip()

        ## ¼³¸í ¿¹¿Ü »óÈ² Ã³¸®
        def __PassiveSkillExceptionDecsriptionValueChange(self, pet_skill_vnum, value):

            if chr.PET_SKILL_AFFECT_GOLD_DROP == pet_skill_vnum:
                value = value / 10.0

            return value

        ## ÆÐ½Ãºê ½ºÅ³ ¼³¸í
        def __AppendPassiveSkill(self, pet_skill_vnum, value, color):

            if 0 == pet_skill_vnum:
                return

            try:
                text = self.PET_SKILL_APPLY_DATA_DICT[pet_skill_vnum]
            except KeyError:
                return

            value = self.__PassiveSkillExceptionDecsriptionValueChange(
                pet_skill_vnum, value
            )

            self.AppendSpace(5)
            self.AppendTextLine(text(value), color)

        ## ÀÚµ¿ ½ºÅ³ ¼³¸í
        def __AppendAutoSkill(self, pet_skill_vnum, value1, value2, color):

            if 0 == pet_skill_vnum:
                return

            try:
                text = self.PET_SKILL_APPLY_DATA_DICT[pet_skill_vnum]
            except KeyError:
                return

            self.AppendSpace(5)
            self.AppendTextLine(text(value1, value2), color)

        ## Áö¼Ó½Ã°£ Ç¥½Ã
        def __AppendRemainsTime(self):
            return

        ## ÄðÅ¸ÀÓ Ç¥½Ã
        def __AppendCoolTime(self, pet_skill_cool_time, color):
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.DRAGON_SOUL_STRENGTH.format(pet_skill_cool_time), color
            )
            return

        def __AppendNextLevel(self, curLevel, maxLevel):
            self.AppendSpace(5)
            self.AppendTextLine(
                localeInfo.PET_TOOLTIP_SKILL_NEXT_LEVEL.format(curLevel, maxLevel),
                self.NEGATIVE_COLOR,
            )
