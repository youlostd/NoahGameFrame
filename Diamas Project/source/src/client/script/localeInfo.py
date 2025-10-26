# -*- coding: utf-8 -*-


import app
from pack import OpenVfsFile
from pygame.background import bgInst
import logging
APP_TITLE = "Diamas - Vs'lik Düello Sunucusu"

GUILD_HEADQUARTER = "Main Building"
GUILD_FACILITY = "Facility"
GUILD_OBJECT = "Object"
GUILD_MEMBER_COUNT_INFINITY = "INFINITY"

CHARACTER_NAME_MAX_LEN = 18  # For inputs

TOOLTIP_DRAGON_SOUL_DECK3 = "???"
DO_YOU_REMOVE_ITEM_FROM_SHOP = "Do you want to remove %s from your shop?"

LOGIN_FAILURE_WEB_BLOCK = "BLOCK_LOGIN(WEB)"
LOGIN_FAILURE_BLOCK_LOGIN = "BLOCK_LOGIN"
CHANNEL_NOTIFY_FULL = "CHANNEL_NOTIFY_FULL"

GUILD_BUILDING_LIST_TXT = app.GetLocalePath() + "/GuildBuildingList.txt"

GUILD_MARK_MIN_LEVEL = "3"
GUILD_MARK_NOT_ENOUGH_LEVEL = "You need at least a level 3 Guild to do this"

ERROR_MARK_UPLOAD_NEED_RECONNECT = "UploadMark: Reconnect to game"
ERROR_MARK_CHECK_NEED_RECONNECT = "CheckMark: Reconnect to game"

VIRTUAL_KEY_ALPHABET_LOWERS = r"[1234567890]/qwertyuiop\=asdfghjkl;`'zxcvbnm.,"
VIRTUAL_KEY_ALPHABET_UPPERS = r'{1234567890}?QWERTYUIOP|+ASDFGHJKL:~"ZXCVBNM<>'
VIRTUAL_KEY_SYMBOLS = '!@#$%^&*()_+|{}:"<>?~'
VIRTUAL_KEY_NUMBERS = "1234567890-=\[];',./`"
VIRTUAL_KEY_SYMBOLS_BR = '!@#$%^&*()_+|{}:"<>?~áàãâéèêíìóòôõúùç'

WORLD_BOSS_INFO_NAME = "Name"
WORLD_BOSS_INFO_SPAWN_LOCATIONS = "Spawn Locations"
WORLD_BOSS_INFO_SPAWN_TIME = "Spawn Time"
WORLD_BOSS_DAILY_AT = "Daily at"
WORLD_BOSS_AT = "at"
WORLD_BOSS_SPAWNS_IN = "Spawns ins {}"

__IS_ARABIC = "locale/ae" == app.GetLocalePath()

def IsARABIC():
    global __IS_ARABIC
    return __IS_ARABIC

def mapping(**kwargs): return kwargs

def SNA(text):
    def f(x):
        return text

    return f


def SA(text):
    def f(x):
        return text % x

    return f


def SAA(text):
    def f(x1, x2):
        return text % (x1, x2)

    return f


def SAN(text):
    def f(x1, x2):
        return text % x1

    return f

def LoadLocaleFile(filename, localeDict):
    funcDict = {"SA": SA, "SNA": SNA, "SAA": SAA, "SAN": SAN}

    lineIndex = 1
    for line in OpenVfsFile(filename):
        tokens = [l.strip() for l in line.split("\t")]
        if len(tokens) == 1:
            localeDict[tokens[0]] = ""
        elif len(tokens) == 2:
            localeDict[tokens[0]] = tokens[1]
        elif len(tokens) >= 3:
            type = tokens[2].strip()
            if type:
                localeDict[tokens[0]] = funcDict[type](tokens[1])
            else:
                localeDict[tokens[0]] = tokens[1]
        else:
            raise RuntimeError("Unknown TokenSize")

        lineIndex += 1

LoadLocaleFile("locale/de/locale_game.txt", locals())
LoadLocaleFile("%s/locale_game.txt" % app.GetLocalePath(), locals())

all = ["locale", "error"]

FN_GM_MARK = "%s/effect/gm_logos/gm.mse" % app.GetLocalePath()
LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()


def CutMoneyString(sourceText, startIndex, endIndex, insertingText, backText):
    sourceLength = len(sourceText)

    if sourceLength < startIndex:
        return backText

    text = sourceText[max(0, sourceLength - endIndex):sourceLength - startIndex]

    if not text:
        return backText

    if int(text) <= 0:
        return backText

    text = str(int(text))

    if backText:
        backText = " " + backText

    return text + insertingText + backText


if app.ENABLE_GEM_SYSTEM:
    def SecondToHMGolbal(time):
        if time < 60:
            return "00:01"

        minute = int((time / 60) % 60)
        hour = int((time / 60) / 60)

        text = ""

        if hour > 0:
            if hour >= 10:
                text += str(hour) + ":"
            else:
                text += "0" + str(hour) + ":"
        else:
            text += "00:"

        if minute > 0:
            if minute >= 10:
                text += str(minute)
            else:
                text += "0" + str(minute)

        return text


def SecondToDHMS(time):
    if time < 60:
        return "%.2f %s" % (time, SECOND)

    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60) % 24
    day = int(int((time / 60) / 60) / 24)

    text = ""

    if day > 0:
        text += str(day) + DAY

    if hour > 0:
        text += " "
        text += str(hour) + HOUR

    if minute > 0:
        text += " "
        text += str(minute) + MINUTE

    if second > 0:
        text += " "
        text += str(second) + SECOND

    return text


def SecondToDHM(time):
    if time > 60 * 60 * 24 * 365 * 20:
        return NEVER_EXPIRES_DURATION

    if time < 60:
        if IsARABIC():
            return "%.2f %s" % (time, SECOND)
        else:
            return "0" + MINUTE

    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60) % 24
    day = int(int((time / 60) / 60) / 24)

    text = ""

    if day > 0:
        text += str(day) + DAY
        text += " "

    if hour > 0:
        text += str(hour) + HOUR
        text += " "

    if minute > 0:
        text += str(minute) + MINUTE

    return text


def SecondToHM(time):
    if time < 60:
        if IsARABIC():
            return "%.2f %s" % (time, SECOND)
        else:
            return "0" + MINUTE

    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60)

    text = ""

    if hour > 0:
        text += str(hour) + HOUR
        if hour > 0:
            text += " "

    if minute > 0:
        text += str(minute) + MINUTE

    return text


def SecondToHMS(time):
    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60)

    text = ""

    if hour > 0:
        text += str(hour) + HOUR
        if hour > 0:
            text += " "

    if minute > 0:
        text += str(minute) + MINUTE

    if second > 0:
        text += str(second) + SECOND

    return text


def SecondToMS(time):
    second = int(time % 60)
    minute = int((time / 60) % 60)

    text = ""

    if minute > 0:
        text += str(minute) + MINUTE

    if second > 0:
        text += str(second) + SECOND

    return text


if app.ENABLE_GROWTH_PET_SYSTEM:
    def SecondToDay(time):
        if time < 60:
            return "1" + DAY

        second = int(time % 60)
        minute = int((time / 60) % 60)
        hour = int((time / 60) / 60) % 24
        day = int(int((time / 60) / 60) / 24)

        if day < 1:
            day = 1

        if day > 9999:
            day = 9999

        text = str(day) + DAY
        return text

if app.ENABLE_GROWTH_PET_SYSTEM:
    def SecondToDayNumber(time):
        if time < 60:
            return 1

        second = int(time % 60)
        minute = int((time / 60) % 60)
        hour = int((time / 60) / 60) % 24
        day = int(int((time / 60) / 60) / 24)

        if day < 1:
            day = 1

        if day > 9999:
            day = 9999

        return day

if app.ENABLE_GROWTH_PET_SYSTEM:
    def SecondToH(time):
        hour = int((time / 60) / 60)

        text = ""
        hour = max(0, hour)
        text += str(hour)

        return text

if app.ENABLE_BATTLE_FIELD:
    def SecondToColonTypeHM(time):
        if time < 60:
            return "00:00"

        minute = int((time / 60) % 60)
        hour = int((time / 60) / 60)

        return "%02d:%02d" % (hour, minute)

if app.ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM or app.ENABLE_12ZI:
    def SecondToColonTypeHMS(time):
        if time < 60:
            return "00:00:%02d" % (time)

        second = int(time % 60)
        minute = int((time / 60) % 60)
        hour = int((time / 60) / 60)

        return "%02d:%02d:%02d" % (hour, minute, second)


    def SecondToColonTypeMS(time):

        second = int(time % 60)
        minute = int((time / 60) % 60)

        return "%02d:%02d" % (minute, second)


OPTION_PVPMODE_MESSAGE_DICT = {
    0: PVP_MODE_NORMAL,
    1: PVP_MODE_REVENGE,
    2: PVP_MODE_KILL,
    3: PVP_MODE_PROTECT,
    4: PVP_MODE_GUILD,
}

error = mapping(
    CREATE_WINDOW=GAME_INIT_ERROR_MAIN_WINDOW,
    CREATE_CURSOR=GAME_INIT_ERROR_CURSOR,
    CREATE_NETWORK=GAME_INIT_ERROR_NETWORK,
    CREATE_ITEM_PROTO=GAME_INIT_ERROR_ITEM_PROTO,
    CREATE_MOB_PROTO=GAME_INIT_ERROR_MOB_PROTO,
    CREATE_NO_DIRECTX=GAME_INIT_ERROR_DIRECTX,
    CREATE_DEVICE=GAME_INIT_ERROR_GRAPHICS_NOT_EXIST,
    CREATE_NO_APPROPRIATE_DEVICE=GAME_INIT_ERROR_GRAPHICS_BAD_PERFORMANCE,
    CREATE_FORMAT=GAME_INIT_ERROR_GRAPHICS_NOT_SUPPORT_32BIT,
    NO_ERROR=""
)

GUILDWAR_NORMAL_DESCLIST = [GUILD_WAR_USE_NORMAL_MAP, GUILD_WAR_LIMIT_30MIN, GUILD_WAR_WIN_CHECK_SCORE]
GUILDWAR_WARP_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_WIPE_OUT_GUILD, GUILD_WAR_REWARD_POTION]
GUILDWAR_CTF_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_TAKE_AWAY_FLAG1, GUILD_WAR_WIN_TAKE_AWAY_FLAG2,
                         GUILD_WAR_REWARD_POTION]

SKILL_TITLE = [
    [JOB_WARRIOR0, SKILL_GROUP_WARRIOR_1, SKILL_GROUP_WARRIOR_2, ],
    [JOB_ASSASSIN0, SKILL_GROUP_ASSASSIN_1, SKILL_GROUP_ASSASSIN_2, ],
    [JOB_SURA0, SKILL_GROUP_SURA_1, SKILL_GROUP_SURA_2, ],
    [JOB_SHAMAN0, SKILL_GROUP_SHAMAN_1, SKILL_GROUP_SHAMAN_2, ],
    [JOB_WOLFMAN0, SKILL_GROUP_WOLFMAN_1, SKILL_GROUP_WOLFMAN_2, ],
]

RACE_TITLE = [
    JOB_WARRIOR, JOB_ASSASSIN, JOB_SURA, JOB_SHAMAN, JOB_WOLFMAN
]

if app.ENABLE_WOLFMAN_CHARACTER:
    JOBINFO_TITLE = [
        [JOB_WARRIOR0, JOB_WARRIOR1, JOB_WARRIOR2, ],
        [JOB_ASSASSIN0, JOB_ASSASSIN1, JOB_ASSASSIN2, ],
        [JOB_SURA0, JOB_SURA1, JOB_SURA2, ],
        [JOB_SHAMAN0, JOB_SHAMAN1, JOB_SHAMAN2, ],
        [JOB_WOLFMAN0, JOB_WOLFMAN1, JOB_WOLFMAN1, ],
    ]
else:
    JOBINFO_TITLE = [
        [JOB_WARRIOR0, JOB_WARRIOR1, JOB_WARRIOR2, ],
        [JOB_ASSASSIN0, JOB_ASSASSIN1, JOB_ASSASSIN2, ],
        [JOB_SURA0, JOB_SURA1, JOB_SURA2, ],
        [JOB_SHAMAN0, JOB_SHAMAN1, JOB_SHAMAN2, ],
    ]

WHISPER_ERROR = {
    1: CANNOT_WHISPER_NOT_LOGON,
    2: CANNOT_WHISPER_DEST_REFUSE,
    3: CANNOT_WHISPER_SELF_REFUSE,
}

NOTIFY_MESSAGE = {
    "CANNOT_EQUIP_SHOP": CANNOT_EQUIP_IN_SHOP,
    "CANNOT_EQUIP_EXCHANGE": CANNOT_EQUIP_IN_EXCHANGE,
}

ATTACK_ERROR_TAIL_DICT = {
    "IN_SAFE": CANNOT_ATTACK_SELF_IN_SAFE,
    "DEST_IN_SAFE": CANNOT_ATTACK_DEST_IN_SAFE,
}

SHOT_ERROR_TAIL_DICT = {
    "EMPTY_ARROW": CANNOT_SHOOT_EMPTY_ARROW,
    "IN_SAFE": CANNOT_SHOOT_SELF_IN_SAFE,
    "DEST_IN_SAFE": CANNOT_SHOOT_DEST_IN_SAFE,
}

USE_SKILL_ERROR_TAIL_DICT = {
    "IN_SAFE": CANNOT_SKILL_SELF_IN_SAFE,
    "NEED_TARGET": CANNOT_SKILL_NEED_TARGET,
    "NEED_EMPTY_BOTTLE": CANNOT_SKILL_NEED_EMPTY_BOTTLE,
    "NEED_POISON_BOTTLE": CANNOT_SKILL_NEED_POISON_BOTTLE,
    "REMOVE_FISHING_ROD": CANNOT_SKILL_REMOVE_FISHING_ROD,
    "NOT_YET_LEARN": CANNOT_SKILL_NOT_YET_LEARN,
    "NOT_MATCHABLE_WEAPON": CANNOT_SKILL_NOT_MATCHABLE_WEAPON,
    "WAIT_COOLTIME": CANNOT_SKILL_WAIT_COOLTIME,
    "NOT_ENOUGH_HP": CANNOT_SKILL_NOT_ENOUGH_HP,
    "NOT_ENOUGH_SP": CANNOT_SKILL_NOT_ENOUGH_SP,
    "CANNOT_USE_SELF": CANNOT_SKILL_USE_SELF,
    "ONLY_FOR_ALLIANCE": CANNOT_SKILL_ONLY_FOR_ALLIANCE,
    "CANNOT_ATTACK_ENEMY_IN_SAFE_AREA": CANNOT_SKILL_DEST_IN_SAFE,
    "CANNOT_APPROACH": CANNOT_SKILL_APPROACH,
    "CANNOT_ATTACK": CANNOT_SKILL_ATTACK,
    "ONLY_FOR_CORPSE": CANNOT_SKILL_ONLY_FOR_CORPSE,
    "EQUIP_FISHING_ROD": CANNOT_SKILL_EQUIP_FISHING_ROD,
    "NOT_HORSE_SKILL": CANNOT_SKILL_NOT_HORSE_SKILL,
    "HAVE_TO_RIDE": CANNOT_SKILL_HAVE_TO_RIDE,
    "BLOCKED": CANNOT_SKILL_BLOCKED,
}

LEVEL_LIST = ["", HORSE_LEVEL1, HORSE_LEVEL2, HORSE_LEVEL3]

HEALTH_LIST = [
    HORSE_HEALTH0,
    HORSE_HEALTH1,
    HORSE_HEALTH2,
    HORSE_HEALTH3,
]

USE_SKILL_ERROR_CHAT_DICT = {
    "NEED_EMPTY_BOTTLE": SKILL_NEED_EMPTY_BOTTLE,
    "NEED_POISON_BOTTLE": SKILL_NEED_POISON_BOTTLE,
    "ONLY_FOR_GUILD_WAR": SKILL_ONLY_FOR_GUILD_WAR,
}

SHOP_ERROR_DICT = {
    "NOT_ENOUGH_MONEY": SHOP_NOT_ENOUGH_MONEY,
    "NOT_ENOUGH_CASH": SHOP_NOT_ENOUGH_CASH,
    "SOLDOUT": SHOP_SOLDOUT,
    "INVENTORY_FULL": SHOP_INVENTORY_FULL,
    "INVALID_POS": SHOP_INVALID_POS,
    "NOT_ENOUGH_MONEY_EX": SHOP_NOT_ENOUGH_MONEY_EX,
}

STAT_MINUS_DESCRIPTION = {
    "HTH-": STAT_MINUS_CON,
    "INT-": STAT_MINUS_INT,
    "STR-": STAT_MINUS_STR,
    "DEX-": STAT_MINUS_DEX,
}

RACE_FLAG_TO_NAME = {
    1 << 0: TARGET_INFO_RACE_ANIMAL,
    1 << 1: TARGET_INFO_RACE_UNDEAD,
    1 << 2: TARGET_INFO_RACE_DEVIL,
    1 << 3: TARGET_INFO_RACE_HUMAN,
    1 << 4: TARGET_INFO_RACE_ORC,
    1 << 5: TARGET_INFO_RACE_MILGYO,
    1 << 6: TARGET_INFO_RACE_INSECT,
    1 << 7: TARGET_INFO_RACE_FIRE,
    1 << 8: TARGET_INFO_RACE_ICE,
    1 << 9: TARGET_INFO_RACE_DESERT,
    1 << 10: TARGET_INFO_RACE_TREE,
}

SUB_RACE_FLAG_TO_NAME = {
    1 << 11: TARGET_INFO_RACE_ELEC,
    1 << 12: TARGET_INFO_RACE_FIRE,
    1 << 13: TARGET_INFO_RACE_ICE,
    1 << 14: TARGET_INFO_RACE_WIND,
    1 << 15: TARGET_INFO_RACE_EARTH,
    1 << 16: TARGET_INFO_RACE_DARK,
}

import nonplayer

GRADES_TO_NAME = {
    nonplayer.PAWN: TARGET_LEVEL_PAWN,
    nonplayer.S_PAWN: TARGET_LEVEL_S_PAWN,
    nonplayer.KNIGHT: TARGET_LEVEL_KNIGHT,
    nonplayer.S_KNIGHT: TARGET_LEVEL_S_KNIGHT,
    nonplayer.BOSS: TARGET_LEVEL_BOSS,
    nonplayer.KING: TARGET_LEVEL_KING,
}

MODE_NAME_LIST = (PVP_OPTION_NORMAL, PVP_OPTION_REVENGE, PVP_OPTION_KILL, PVP_OPTION_PROTECT,)
TITLE_NAME_LIST = (
PVP_LEVEL0, PVP_LEVEL1, PVP_LEVEL2, PVP_LEVEL3, PVP_LEVEL4, PVP_LEVEL5, PVP_LEVEL6, PVP_LEVEL7, PVP_LEVEL8, PVP_LEVEL9,
PVP_LEVEL10, PVP_LEVEL11, PVP_LEVEL12, PVP_LEVEL13, PVP_LEVEL14, PVP_LEVEL15, PVP_LEVEL16, PVP_LEVEL17, PVP_LEVEL18,
PVP_LEVEL19, PVP_LEVEL20, PVP_LEVEL21, PVP_LEVEL22, PVP_LEVEL23, PVP_LEVEL24, PVP_LEVEL25, PVP_LEVEL26, PVP_LEVEL27,
PVP_LEVEL28, PVP_LEVEL29, PVP_LEVEL30, PVP_LEVEL31, PVP_LEVEL32, PVP_LEVEL33, PVP_LEVEL34, PVP_LEVEL35, PVP_LEVEL36,
PVP_LEVEL37, PVP_LEVEL38, PVP_LEVEL39, PVP_LEVEL40, PVP_LEVEL41, PVP_LEVEL42, PVP_LEVEL43, PVP_LEVEL44, PVP_LEVEL45,
PVP_LEVEL46, PVP_LEVEL47, PVP_LEVEL48, PVP_LEVEL49, PVP_LEVEL50, PVP_LEVEL51, PVP_LEVEL52, PVP_LEVEL53, PVP_LEVEL54,
PVP_LEVEL55, PVP_LEVEL56, PVP_LEVEL57, PVP_LEVEL58, PVP_LEVEL59, PVP_LEVEL60, PVP_LEVEL61,)

TITLE_NAME_LIST_FEMALE = (
PVP_LEVEL0_FEMALE, PVP_LEVEL1_FEMALE, PVP_LEVEL2_FEMALE, PVP_LEVEL3_FEMALE, PVP_LEVEL4_FEMALE, PVP_LEVEL5_FEMALE, PVP_LEVEL6_FEMALE, PVP_LEVEL7_FEMALE, PVP_LEVEL8_FEMALE, PVP_LEVEL9_FEMALE,
PVP_LEVEL10_FEMALE, PVP_LEVEL11_FEMALE, PVP_LEVEL12_FEMALE, PVP_LEVEL13_FEMALE, PVP_LEVEL14_FEMALE, PVP_LEVEL15_FEMALE, PVP_LEVEL16_FEMALE, PVP_LEVEL17_FEMALE, PVP_LEVEL18_FEMALE,
PVP_LEVEL19_FEMALE, PVP_LEVEL20_FEMALE, PVP_LEVEL21_FEMALE, PVP_LEVEL22_FEMALE, PVP_LEVEL23_FEMALE, PVP_LEVEL24_FEMALE, PVP_LEVEL25_FEMALE, PVP_LEVEL26_FEMALE, PVP_LEVEL27_FEMALE,
PVP_LEVEL28_FEMALE, PVP_LEVEL29_FEMALE, PVP_LEVEL30_FEMALE, PVP_LEVEL31_FEMALE, PVP_LEVEL32_FEMALE, PVP_LEVEL33_FEMALE, PVP_LEVEL34_FEMALE, PVP_LEVEL35_FEMALE, PVP_LEVEL36_FEMALE,
PVP_LEVEL37_FEMALE, PVP_LEVEL38_FEMALE, PVP_LEVEL39_FEMALE, PVP_LEVEL40_FEMALE, PVP_LEVEL41_FEMALE, PVP_LEVEL42_FEMALE, PVP_LEVEL43_FEMALE, PVP_LEVEL44_FEMALE, PVP_LEVEL45_FEMALE,
PVP_LEVEL46_FEMALE, PVP_LEVEL47_FEMALE, PVP_LEVEL48_FEMALE, PVP_LEVEL49_FEMALE, PVP_LEVEL50_FEMALE, PVP_LEVEL51_FEMALE, PVP_LEVEL52_FEMALE, PVP_LEVEL53_FEMALE, PVP_LEVEL54_FEMALE,
PVP_LEVEL55_FEMALE, PVP_LEVEL56_FEMALE, PVP_LEVEL57_FEMALE, PVP_LEVEL58_FEMALE, PVP_LEVEL59_FEMALE, PVP_LEVEL60_FEMALE, PVP_LEVEL61_FEMALE,)
# Note: Simply appending an 'i' only works,
# if the font already has a size spec (e.g. :12)
if UI_DEF_FONT and UI_DEF_FONT.find(":") == -1:
    UI_DEF_FONT += "Roboto:14s"

UI_DEF_FONT_ITALIC = "Verdana:11i"
UI_DEF_FONT_BOLD = "Roboto:14s"
UI_DEF_FONT_NUNITO = "Nunito Sans:12"
UI_DEF_FONT_NUNITO_ITALIC = "Nunito Sans:12i"
UI_DEF_FONT_NUNITO_BOLD = "Nunito Sans:12b"
UI_DEF_FONT_NUNITO_BOLD_BIG = "Nunito Sans:14b"

app.SetDefaultFontName(UI_DEF_FONT, UI_DEF_FONT_ITALIC, UI_DEF_FONT_BOLD)


def GetLetterImageName():
    return "d:/ymir work/ui/gui/common/scroll_close.tga"


def GetLetterOpenImageName():
    return "d:/ymir work/ui/gui/common/scroll_open.tga"


def GetLetterCloseImageName():
    return "d:/ymir work/ui/gui/common/scroll_close.tga"


def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
    if sellItemCount > 1:
        return DO_YOU_SELL_ITEM2.format(sellItemName, sellItemCount, NumberToMoneyString(sellItemPrice))
    else:
        return DO_YOU_SELL_ITEM1.format(sellItemName, NumberToMoneyString(sellItemPrice))


def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice):
    if buyItemCount > 1:
        return DO_YOU_BUY_ITEM2.format(buyItemName, buyItemCount, buyItemPrice)
    else:
        return DO_YOU_BUY_ITEM1.format(buyItemName, buyItemPrice)


def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
    return REFINE_FAILURE_CAN_NOT_ATTACH_STR.format(attachedItemName)


def REFINE_FAILURE_NO_SOCKET(attachedItemName):
    return REFINE_FAILURE_NO_SOCKET0.format(attachedItemName)


def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName):
    return REFINE_FAILURE_NO_GOLD_SOCKET.format(attachedItemName)


def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount):
    if dropItemCount > 1:
        return HOW_MANY_ITEM_DO_YOU_DROP2.format(dropItemName, dropItemCount)
    else:
        return HOW_MANY_ITEM_DO_YOU_DROP1.format(dropItemName)


def FISHING_NOTIFY(isFish, fishName):
    if isFish:
        return FISHING_NOTIFY1.format(fishName)
    else:
        return FISHING_NOTIFY2.format(fishName)


def FISHING_SUCCESS(isFish, fishName):
    if isFish:
        return FISHING_SUCCESS1.format(fishName)
    else:
        return FISHING_SUCCESS2.format(fishName)


def NumberToMoneyString(n):
    if app.GetLocaleName() == "de":
        if float(n).is_integer():
            return '{:,} {}'.format(int(n), MONETARY_UNIT0).replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f} {}'.format(n, MONETARY_UNIT0).replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if float(n).is_integer():
            return '{:,} {}'.format(int(n), MONETARY_UNIT0)
        else:
            return '{:,.2f} {}'.format(n, MONETARY_UNIT0)

def NumberToCoinsString(n):
    if app.GetLocaleName() == "de":
        if float(n).is_integer():
            return '{:,} {}'.format(int(n), "Coins").replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f} {}'.format(n, "Coins").replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if float(n).is_integer():
            return '{:,} {}'.format(int(n), "Coins")
        else:
            return '{:,.2f} {}'.format(n, "Coins")

def MoneyFormat(n):
    if app.GetLocaleName() == "de":
        if float(n).is_integer():
            return '{:,}'.format(int(n)).replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f}'.format(n).replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if float(n).is_integer():
            return '{:,}'.format(int(n))
        else:
            return '{:,.2f}'.format(n)


def DottedNumber(n):
    n = float(n)
    if app.GetLocaleName() == "de":
        if n.is_integer():
            return '{:,}'.format(int(n)).replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f}'.format(n).replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if n.is_integer():
            return '{:,}'.format(int(n))
        else:
            return '{:,.2f}'.format(n)


def NumberToSecondaryCoinString(n, coinTypeString):
    n = float(n)
    if app.GetLocaleName() == "de":
        if n.is_integer():
            return '{:,} {}'.format(int(n), coinTypeString).replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f} {}'.format(n, coinTypeString).replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if n.is_integer():
            return '{:,} {}'.format(int(n), coinTypeString)
        else:
            return '{:,.2f} {}'.format(n, coinTypeString)


def NumberToString(n):
    n = float(n)

    if app.GetLocaleName() == "de":
        if n.is_integer():
            return '{:,}'.format(int(n)).replace('.', '_').replace(',', '.').replace('_', '.')
        else:
            return '{:,.2f}'.format(n).replace('.', '_').replace(',', '.').replace('_', '.')
    else:
        if n.is_integer():
            return '{:,}'.format(int(n))
        else:
            return '{:,.2f}'.format(n)


def GetSkillGroupName(job, skillGroup):
    if skillGroup == 0:
        return SKILLGROUP_NAME_NONE

    skillGroupNames = [
        [SKILLGROUP_NAME_WARRIOR_1, SKILLGROUP_NAME_WARRIOR_2],
        [SKILLGROUP_NAME_ASSASSIN_1, SKILLGROUP_NAME_ASSASSIN_2],
        [SKILLGROUP_NAME_SURA_1, SKILLGROUP_NAME_SURA_2],
        [SKILLGROUP_NAME_SHAMAN_1, SKILLGROUP_NAME_SHAMAN_2],
    ]

    return skillGroupNames[job][skillGroup - 1]


def FormatWhisperLine(name, message):
    return "{0}: {1}".format(name, message)


def NumberToRoman(input):
    """
    Convert an integer to Roman numerals.

    Examples:
    >>> NumberToRoman(0)
    Traceback (most recent call last):
    ValueError: Argument must be between 1 and 3999

    >>> NumberToRoman(-1)
    Traceback (most recent call last):
    ValueError: Argument must be between 1 and 3999

    >>> NumberToRoman(1.5)
    Traceback (most recent call last):
    TypeError: expected integer, got <type 'float'>

    >>> for i in range(1, 21): print NumberToRoman(i)
    ...
    I
    II
    III
    IV
    V
    VI
    VII
    VIII
    IX
    X
    XI
    XII
    XIII
    XIV
    XV
    XVI
    XVII
    XVIII
    XIX
    XX
    >>> print NumberToRoman(2000)
    MM
    >>> print NumberToRoman(1999)
    MCMXCIX
    """
    if type(input) != type(1):
        raise TypeError, "expected integer, got %s" % type(input)
    if not 0 < input < 4000:
        raise ValueError, "Argument must be between 1 and 3999"
    ints = (1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1)
    nums = ('M', 'CM', 'D', 'CD', 'C', 'XC', 'L', 'XL', 'X', 'IX', 'V', 'IV', 'I')
    result = ""
    for i in range(len(ints)):
        count = int(input / ints[i])
        result += nums[i] * count
        input -= ints[i] * count
    return result


def GetFormattedTimeString(time):
    days, remainder = divmod(time, 86400)
    hours, remainder = divmod(remainder, 3600)
    minutes, seconds = divmod(remainder, 60)
    return "%dGün %dSaat %02dDakika %02dSaniye" % (days, hours, minutes, seconds)


def GetMapNameByIndex(mapIndex):
    try:
        mapInfo = bgInst().GetMapInfo(mapIndex)
        if mapInfo:
            return globals().get(mapInfo.mapTranslation, "Unknown")
    except Exception as e:
        logging.exception(e)
        return "UNKNOWN"
    return "UNKNOWN"

def MinuteToHM(time):
    minute = int(time % 60)
    hour = int((time / 60) % 60)

    text = ""

    if hour > 0:
        text += str(hour) + HOUR
        text += " "

    if minute > 0:
        text += str(minute) + MINUTE

    return text

def SecondToHMSG(time):
    if time < 60:
        return "00:00:%02d" % (time)

    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60)

    return "%02d:%02d:%02d" % (hour, minute, second)

def SecondToNiceTime(time):
    second = int(time % 60)
    minute = int((time / 60) % 60)
    hour = int((time / 60) / 60) % 24

    return "%02d:%02d:%02d" % (hour, minute, second)

def GetVariableValue(varName):
    for name, value in globals().items():
        if str(varName) == name:
            return str(value)

def GetVariableName(varValue):
    for name, value in globals().items():
        if str(varValue) == value:
            return str(name)

def Get(name):
    return globals().get(name, str(name))
