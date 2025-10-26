# -*- coding: utf-8 -*-

import grp

CHAT_RGB_TALK = (1.0, 1.0, 1.0)
CHAT_RGB_INFO = (1.0, 200.0 / 255.0, 200.0/ 255.0)
CHAT_RGB_NOTICE = (1.0, 230.0/ 255.0, 186.0 / 255.0)
CHAT_RGB_PARTY = (0, 1.0, 228.0/ 255.0)
CHAT_RGB_GUILD = (253.0 / 255.0, 1.0, 124.0 / 255.0)
# CHAT_RGB_GUILD = (231, 215, 255)
CHAT_RGB_COMMAND = (167.0 / 255.0, 1.0, 212/ 255)
CHAT_RGB_SHOUT = (167.0 / 255.0, 1.0, 212.0 / 255.0)
CHAT_RGB_WHISPER = (74.0 / 255.0, 225.0 / 255.0, 74.0/ 255.0)
CHAT_RGB_TEAM = (1.0, 1.0, 0.0)

CHR_NAME_RGB_MOB = (235, 22, 9)
CHR_NAME_RGB_STONE = (255, 255, 255)
CHR_NAME_RGB_NPC = (122, 231, 93)
CHR_NAME_RGB_PC = (255, 215, 76)
CHR_NAME_RGB_BUFFBOT = (215, 215, 76)
CHR_NAME_RGB_PK = (180, 100, 0)
CHR_NAME_RGB_PVP = (238, 54, 223)
CHR_NAME_RGB_PARTY = (128, 192, 255)
CHR_NAME_RGB_WARP = (136, 218, 241)
CHR_NAME_RGB_WAYPOINT = (255, 255, 255)

CHR_NAME_RGB_EMPIRE_MOB = (235, 22, 9)
CHR_NAME_RGB_EMPIRE_NPC = (122, 231, 93)
CHR_NAME_RGB_EMPIRE_PC_A = (157, 0, 0)
CHR_NAME_RGB_EMPIRE_PC_B = (222, 160, 47)
CHR_NAME_RGB_EMPIRE_PC_C = (23, 30, 138)
CHR_NAME_RGB_DEAD = (0, 0, 0)

TITLE_RGB_GOOD_35 = (172, 4, 4)  ## NoName
TITLE_RGB_GOOD_34 = (172, 4, 4)  ## NoName
TITLE_RGB_GOOD_33 = (172, 4, 4)  ## NoName
TITLE_RGB_GOOD_32 = (172, 4, 4)  ## La
TITLE_RGB_GOOD_31 = (0, 255, 0)  ## Puschel<3
TITLE_RGB_GOOD_30 = (210, 0, 255)  ## Göttin
TITLE_RGB_GOOD_29 = (255, 0, 255)  ## Prinzessin
TITLE_RGB_GOOD_28 = (255, 255, 0)  ## V.I.P
TITLE_RGB_GOOD_27 = (255, 0, 0)  ## Team
TITLE_RGB_GOOD_26 = (161, 12, 230)  ## Legendär
TITLE_RGB_GOOD_25 = (0, 120, 255)  ## Gottesgleich
TITLE_RGB_GOOD_24 = (0, 120, 255)  ## Göttlich
TITLE_RGB_GOOD_23 = (0, 150, 255)  ## Halbmensch
TITLE_RGB_GOOD_22 = (0, 180, 255)  ## Heilig
TITLE_RGB_GOOD_21 = (0, 200, 255)  ## Gesegnet
TITLE_RGB_GOOD_20 = (0, 220, 255)  ## Monarch
TITLE_RGB_GOOD_19 = (0, 240, 255)  ## Baron
TITLE_RGB_GOOD_18 = (0, 255, 255)  ## Kaiser
TITLE_RGB_GOOD_17 = (0, 255, 230)  ## König
TITLE_RGB_GOOD_16 = (0, 255, 210)  ## Königlich
TITLE_RGB_GOOD_15 = (0, 255, 180)  ## Großherzog
TITLE_RGB_GOOD_14 = (0, 255, 150)  ## Erzherzog
TITLE_RGB_GOOD_13 = (0, 255, 120)  ## Herzog
TITLE_RGB_GOOD_12 = (0, 255, 90)  ## Freiherr
TITLE_RGB_GOOD_11 = (0, 255, 60)  ## Graf
TITLE_RGB_GOOD_10 = (0, 255, 30)  ## Adlig
TITLE_RGB_GOOD_9 = (0, 255, 0)  ## Marschall
TITLE_RGB_GOOD_8 = (20, 255, 0)  ## Ritterlich
TITLE_RGB_GOOD_7 = (50, 255, 0)  ## Ritter
TITLE_RGB_GOOD_6 = (80, 255, 0)  ## Zenturio
TITLE_RGB_GOOD_5 = (110, 255, 0)  ## Knappe
TITLE_RGB_GOOD_4 = (140, 255, 0)  ## Edel
TITLE_RGB_GOOD_3 = (170, 255, 0)  ## Gut
TITLE_RGB_GOOD_2 = (200, 255, 0)  ## Gutmütig
TITLE_RGB_GOOD_1 = (230, 255, 0)  ## Freundlich
TITLE_RGB_NORMAL = (255, 255, 255)  ## Neutral
TITLE_RGB_EVIL_1 = (255, 180, 0)  ## Aggressiv
TITLE_RGB_EVIL_2 = (255, 180, 0)  ## Arglistig
TITLE_RGB_EVIL_3 = (255, 180, 0)  ## Scharmlos
TITLE_RGB_EVIL_4 = (255, 170, 0)  ## Bauer
TITLE_RGB_EVIL_5 = (255, 160, 0)  ## Reizbar
TITLE_RGB_EVIL_6 = (255, 150, 0)  ## Provokant
TITLE_RGB_EVIL_7 = (255, 145, 0)  ## Gehässig
TITLE_RGB_EVIL_8 = (255, 140, 63)  ## Furios
TITLE_RGB_EVIL_9 = (255, 135, 63)  ## Rücksichtslos
TITLE_RGB_EVIL_10 = (255, 130, 36)  ## Furchtlos
TITLE_RGB_EVIL_11 = (255, 125, 36)  ## Erbarmungslos
TITLE_RGB_EVIL_12 = (255, 120, 36)  ## Streitsüchtig
TITLE_RGB_EVIL_13 = (255, 115, 33)  ## Feindselig
TITLE_RGB_EVIL_14 = (255, 110, 33)  ## Bösartig
TITLE_RGB_EVIL_15 = (255, 105, 33)  ## Böse
TITLE_RGB_EVIL_16 = (255, 100, 29)  ## Grausam
TITLE_RGB_EVIL_17 = (255, 90, 29)  ## Schänder
TITLE_RGB_EVIL_18 = (255, 80, 29)  ## Verdammt
TITLE_RGB_EVIL_19 = (255, 70, 0)  ## Verflucht
TITLE_RGB_EVIL_20 = (255, 60, 0)  ## Gnadenlos
TITLE_RGB_EVIL_21 = (255, 50, 0)  ## Zerstörer
TITLE_RGB_EVIL_22 = (255, 40, 0)  ## Blutrünstig
TITLE_RGB_EVIL_23 = (255, 30, 0)  ## Verbannt
TITLE_RGB_EVIL_24 = (255, 20, 0)  ## Blutgott
TITLE_RGB_EVIL_25 = (255, 10, 0)  ## Satan
TITLE_RGB_EVIL_26 = (255, 0, 0)  ## Gameforge4D

WHISPER_CHAT = None  # default color
WHISPER_GM = 0xffffa200
WHISPER_SYSTEM = 0xffffc8c8

WHISPER_COLOR = {
    WHISPER_TYPE_NORMAL: WHISPER_CHAT,
    WHISPER_TYPE_GM: WHISPER_GM,
    WHISPER_TYPE_SYSTEM: WHISPER_SYSTEM
}

UI_COLOR_SELECTED = grp.GenerateColor(1.0, 1.0, 1.0, 0.1)
UI_BACKGROUND_COLOR = grp.GenerateColor(0.0, 0.0, 0.0, 1.0)
UI_BACKGROUND_COLOR_ALPHA = grp.GenerateColor(0.1, 0.1, 0.1, 0.3)
UI_DARK_COLOR = grp.GenerateColor(0.2, 0.2, 0.2, 1.0)
UI_BRIGHT_COLOR = grp.GenerateColor(0.7, 0.7, 0.7, 1.0)
UI_SELECT_COLOR = grp.GenerateColor(0.0, 0.0, 0.5, 0.3)

UI_WHITE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.5)
UI_HALF_WHITE_COLOR = grp.GenerateColor(1.0, 1.0, 1.0, 0.2)

UI_GREEN_ALPHA_STRONG = grp.GenerateColor(30 / 255.0,
                    215 / 255.0,
                    35 / 255.0, 0.2)

UI_GREEN_ALPHA_LIGHT = grp.GenerateColor(30 / 255.0,
                    215 / 255.0,
                    35 / 255.0, 0.1)

UI_RED_ALPHA_STRONG = grp.GenerateColor(215 / 255.0,
                    30 / 255.0,
                    30 / 255.0, 0.4)

UI_RED_ALPHA_LIGHT = grp.GenerateColor(30 / 255.0,
                    30 / 255.0,
                    30 / 255.0, 0.35)



def TextTag(color=None):
    if not color:
        return "|r".encode('utf-8')
    return "|c{}".encode('utf-8').format(format(color, 'x')[2:].rjust(8, "f"))

def Colorize(text, color):
    return "{}{}{}".format(TextTag(color), text, TextTag(None))