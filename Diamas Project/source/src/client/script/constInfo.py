# -*- coding: utf-8 -*-

import app
import chrmgr
import item
import player

DROP_INFO = {}
LAST_SENTENCE_STACK = []
LAST_SENTENCE_POS = 0
CONSOLE_ENABLE = 1
ALREADY_NOTIFY_LIST = (
    [],
    [],
    []
)

REAL_LOGOUT_LIST = []

IS_AUTO_REFINE = False
AUTO_REFINE_TYPE = 0
AUTO_REFINE_DATA = {
    "ITEM": [-1, -1],
    "NPC": [0, -1, -1, 0]
}

PVPMODE_ENABLE = 1
PVPMODE_ACCELKEY_ENABLE = 1
PVPMODE_ACCELKEY_DELAY = 0.5
FOG_LEVEL0 = 3800.0
FOG_LEVEL1 = 12600.0
FOG_LEVEL2 = 20800.0
FOG_LEVEL = FOG_LEVEL0
FOG_LEVEL_LIST = [FOG_LEVEL0, FOG_LEVEL1, FOG_LEVEL2]

NEW_QUEST_TIMER_PLAY_SOUND = True			# notification sound
NEW_QUEST_TIMER_LAST_SOUND = 0 				# dont change, its for time measuring


CHRNAME_COLOR_INDEX = 0

ENVIRONMENT_NIGHT = "d:/ymir work/environment/moonlight04.msenv"

# constant
HIGH_PRICE = 500000
MIDDLE_PRICE = 50000
ERROR_METIN_STONE = 28960
USE_SKILL_EFFECT_UPGRADE_ENABLE = 1
VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = 1
SEQUENCE_PACKET_ENABLE = 1
KEEP_ACCOUNT_CONNETION_ENABLE = 1
MINIMAP_POSITIONINFO_ENABLE = 1
USE_ITEM_WEAPON_TABLE_ATTACK_BONUS = 0
ADD_DEF_BONUS_ENABLE = 0
PVPMODE_PROTECTED_LEVEL = 15
TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE = 10

isItemDropQuestionDialog = 0
GUILD_WAR_TYPE_SELECT_ENABLE = 1
WHEEL_TO_SCROLL_MIN = 5
WHEEL_TO_SCROLL_MAX = 40
WHEEL_TO_SCROLL_DEFAULT = 13

WHEEL_VALUE = WHEEL_TO_SCROLL_DEFAULT
WHEEL_LOADED = True


def WHEEL_LOAD():
    global WHEEL_VALUE, WHEEL_LOADED
    WHEEL_LOADED = True


def WHEEL_SET_VALUE(value):
    global  WHEEL_VALUE
    if int(value) < WHEEL_TO_SCROLL_MIN:
        value = WHEEL_TO_SCROLL_MIN
    elif int(value) > WHEEL_TO_SCROLL_MAX:
        value = WHEEL_TO_SCROLL_MAX

    WHEEL_VALUE = int(value)


def WHEEL_TO_SCROLL(wheel):
    if False == WHEEL_LOADED:
        WHEEL_LOAD()
    return -wheel * (WHEEL_VALUE / WHEEL_TO_SCROLL_MIN)


def WHEEL_TO_SCROLL_SLOW(wheel):
    if False == WHEEL_LOADED:
        WHEEL_LOAD()
    return -wheel * max(1, WHEEL_VALUE / WHEEL_TO_SCROLL_DEFAULT)


def WHEEL_TO_SCROLL_PX(wheel):
    if False == WHEEL_LOADED:
        WHEEL_LOAD()
    return -wheel * WHEEL_VALUE


def GET_ITEM_QUESTION_DIALOG_STATUS():
    global isItemDropQuestionDialog
    return isItemDropQuestionDialog


def SET_ITEM_QUESTION_DIALOG_STATUS(flag):
    global isItemDropQuestionDialog
    isItemDropQuestionDialog = flag


########################

def SET_DEFAULT_FOG_LEVEL():
    global FOG_LEVEL
    app.SetMinFog(FOG_LEVEL)


def SET_FOG_LEVEL_INDEX(index):
    global FOG_LEVEL
    global FOG_LEVEL_LIST
    try:
        FOG_LEVEL = FOG_LEVEL_LIST[index]
    except IndexError:
        FOG_LEVEL = FOG_LEVEL_LIST[0]
    app.SetMinFog(FOG_LEVEL)


def SET_DEFAULT_CHRNAME_COLOR():
    global CHRNAME_COLOR_INDEX
    chrmgr.SetEmpireNameMode(CHRNAME_COLOR_INDEX)


def SET_CHRNAME_COLOR_INDEX(index):
    global CHRNAME_COLOR_INDEX
    CHRNAME_COLOR_INDEX = index
    chrmgr.SetEmpireNameMode(index)


def GET_CHRNAME_COLOR_INDEX():
    global CHRNAME_COLOR_INDEX
    return CHRNAME_COLOR_INDEX


def SET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD(index):
    global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
    VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = index


def GET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD():
    global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
    return VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD


def SET_DEFAULT_USE_ITEM_WEAPON_TABLE_ATTACK_BONUS():
    global USE_ITEM_WEAPON_TABLE_ATTACK_BONUS
    player.SetWeaponAttackBonusFlag(USE_ITEM_WEAPON_TABLE_ATTACK_BONUS)


def SET_DEFAULT_USE_SKILL_EFFECT_ENABLE():
    global USE_SKILL_EFFECT_UPGRADE_ENABLE
    app.SetSkillEffectUpgradeEnable(USE_SKILL_EFFECT_UPGRADE_ENABLE)


def SET_TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE():
    global TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE
    app.SetTwoHandedWeaponAttSpeedDecreaseValue(TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE)


########################
ACCESSORY_MATERIAL_LIST = [50623, 50624, 50625, 50626, 50627, 50628, 50629, 50630, 50631, 50632, 50633, 50639, 50640]

JewelAccessoryInfos = [
    # jewel        wrist    neck    ear
    [50639, 9750, 9740, 9760, ],
    [50640, 14160, 16160, 17160],
]


def GET_ACCESSORY_MATERIAL_VNUM(vnum, subType):
    ret = vnum
    item_base = (vnum / 10) * 10
    for info in JewelAccessoryInfos:
        if item.ARMOR_WRIST == subType:
            if info[1] == item_base:
                return info[0]
        elif item.ARMOR_NECK == subType:
            if info[2] == item_base:
                return info[0]
        elif item.ARMOR_EAR == subType:
            if info[3] == item_base:
                return info[0]

    if item.ARMOR_WRIST == subType:
        WRIST_ITEM_VNUM_BASE = 14000
        ret -= WRIST_ITEM_VNUM_BASE
    elif item.ARMOR_NECK == subType:
        NECK_ITEM_VNUM_BASE = 16000
        ret -= NECK_ITEM_VNUM_BASE
    elif item.ARMOR_EAR == subType:
        EAR_ITEM_VNUM_BASE = 17000
        ret -= EAR_ITEM_VNUM_BASE

    type = ret / 20

    if type < 0 or type >= len(ACCESSORY_MATERIAL_LIST):
        type = (ret - 170) / 20
        if type < 0 or type >= len(ACCESSORY_MATERIAL_LIST):
            return 0

    return ACCESSORY_MATERIAL_LIST[type]


if app.ENABLE_GROWTH_PET_SYSTEM:
    def IS_PET_ITEM(itemVnum):
        if itemVnum == 0:
            return 0

        item.SelectItem(itemVnum)
        itemType = item.GetItemType()

        if itemType == item.ITEM_TYPE_PET:
            return 1

        return 0
