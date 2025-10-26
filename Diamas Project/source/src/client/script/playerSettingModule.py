# -*- coding: utf-8 -*-


import app
import chr
import chrmgr
import player
from pack import OpenVfsFile

import localeInfo
import uiMyShopDecoration

MALE = 1
FEMALE = 0

JOB_WARRIOR = 0
JOB_ASSASSIN = 1
JOB_SURA = 2
JOB_SHAMAN = 3
JOB_WOLFMAN = 4

JOB_NAME = {
    JOB_WARRIOR: localeInfo.JOB_WARRIOR,
    JOB_ASSASSIN: localeInfo.JOB_ASSASSIN,
    JOB_SURA: localeInfo.JOB_SURA,
    JOB_SHAMAN: localeInfo.JOB_SHAMAN,
    JOB_WOLFMAN: localeInfo.JOB_WOLFMAN,
}

RACE_WARRIOR_M = 0
RACE_ASSASSIN_W = 1
RACE_SURA_M = 2
RACE_SHAMAN_W = 3
RACE_WARRIOR_W = 4
RACE_ASSASSIN_M = 5
RACE_SURA_W = 6
RACE_SHAMAN_M = 7
RACE_WOLFMAN_M = 8
RACE_WOLFMAN_W = 9


def GetOtherRaceByOwnSex(job):
    race = player.GetRace()
    sex = chr.RaceToSex(race)

    if sex == MALE:
        if job == JOB_WARRIOR:
            return RACE_WARRIOR_M
        elif job == JOB_ASSASSIN:
            return RACE_ASSASSIN_M
        elif job == JOB_SHAMAN:
            return RACE_SHAMAN_M
        elif job == JOB_SURA:
            return RACE_SURA_M
    else:
        if job == JOB_WARRIOR:
            return RACE_WARRIOR_W
        elif job == JOB_ASSASSIN:
            return RACE_ASSASSIN_W
        elif job == JOB_SHAMAN:
            return RACE_SHAMAN_W
        elif job == JOB_SURA:
            return RACE_SURA_W


PASSIVE_GUILD_SKILL_INDEX_LIST = (151,)
ACTIVE_GUILD_SKILL_INDEX_LIST = (
    152,
    153,
    154,
    155,
    156,
    157,
)


def __LoadShopDeco():
    for line in OpenVfsFile("%s/shop_deco.txt" % app.GetLocalePath()):
        if line[0] == "#":
            continue
        if line == "\r\n":
            continue

        tokens = line.strip().split("\t")

        type = int(tokens[0])

        if type == uiMyShopDecoration.MyShopDecoration.MODE_MODEL_VIEW:
            uiMyShopDecoration.DECO_SHOP_MODEL_LIST.append((tokens[1], int(tokens[2])))
        elif type == uiMyShopDecoration.MyShopDecoration.MODE_TITLE_VIEW:
            uiMyShopDecoration.DECO_SHOP_TITLE_LIST.append(
                (tokens[1], tokens[2], tokens[3])
            )


def __LoadNamePrefix():
    for line in OpenVfsFile("%s/name_prefix.txt" % app.GetLocalePath()):
        if line[0] == "#":
            continue
        if line == "\r\n":
            continue

        tokens = line.strip().split("\t")

        type = int(tokens[0])
        chrmgr.RegisterNamePrefix(type, tokens[1])


loadGameDataDict = {
    "SHOPDECO": __LoadShopDeco,
    "NAMEPREFIX": __LoadNamePrefix,
}


def LoadGameData(name):
    global loadGameDataDict

    load = loadGameDataDict.get(name, 0)
    if load:
        loadGameDataDict[name] = 0
        load()
