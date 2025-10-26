# -*- coding: utf-8 -*-
# -*- coding: utf-8 -*-
import app
from pack import OpenVfsFile

AUTOBAN_QUIZ_ANSWER = "ANSWER"
AUTOBAN_QUIZ_REFRESH = "REFRESH"
AUTOBAN_QUIZ_REST_TIME = "REST_TIME"

OPTION_SHADOW = "SHADOW"

## ---------- NEW TRANSLATIONS ---------- ##
DRAGONSOUL_TAP_TITLE_1 = "DRAGONSOUL_TAP_TITLE_1"
DRAGONSOUL_TAP_TITLE_2 = "DRAGONSOUL_TAP_TITLE_2"
DRAGONSOUL_TAP_TITLE_3 = "DRAGONSOUL_TAP_TITLE_3"
DRAGONSOUL_TAP_TITLE_4 = "DRAGONSOUL_TAP_TITLE_4"
DRAGONSOUL_TAP_TITLE_5 = "DRAGONSOUL_TAP_TITLE_5"
DRAGONSOUL_TAP_TITLE_6 = "DRAGONSOUL_TAP_TITLE_6"
DRAGONSOUL_PAGE_BUTTON_1 = "DRAGONSOUL_PAGE_BUTTON_1"
DRAGONSOUL_PAGE_BUTTON_2 = "DRAGONSOUL_PAGE_BUTTON_2"
DRAGONSOUL_PAGE_BUTTON_3 = "DRAGONSOUL_PAGE_BUTTON_3"
DRAGONSOUL_PAGE_BUTTON_4 = "DRAGONSOUL_PAGE_BUTTON_4"
DRAGONSOUL_PAGE_BUTTON_5 = "DRAGONSOUL_PAGE_BUTTON_5"
DRAGONSOUL_PAGE_BUTTON_6 = "DRAGONSOUL_PAGE_BUTTON_6"
DRAGONSOUL_TITLE = "DRAGONSOUL_TITLE"
DRAGONSOUL_ACTIVATE = "DRAGONSOUL_ACTIVATE"
DRAGONSOUL_REFINE_WINDOW_TITLE = "DRAGONSOUL_REFINE_WINDOW_TITLE"
GRADE = "GRADE"
STEP = "STEP"
STRENGTH = "STRENGTH"
DO_REFINE = "DO_REFINE"
TASKBAR_DRAGON_SOUL = "TASKBAR_DRAGON_SOUL"
## ----------------- END -----------------##
UI_DEF_FONT_NUNITO = "Nunito Sans:16b"
UI_DEF_FONT_NUNITO_ITALIC = "Nunito Sans:16i"
UI_DEF_FONT_NUNITO_BOLD = "Nunito Sans:16b"

CODEPAGE = str(app.GetDefaultCodePage())

UserLang = "germany"


# CUBE_TITLE = "Cube Window"


def LoadLangFile(filename, localeDict):
    kLanguageDirectory = "lang/%s/" % UserLang
    kLangFileToLoad = "%s%s" % (kLanguageDirectory, filename)

    for line in open(kLangFileToLoad, "r"):
        tokens = line.strip().split("\t")

        if len(tokens) >= 2:
            localeDict[tokens[0]] = tokens[1]
        else:
            pass


def LoadLocaleFile(srcFileName, localeDict):
    localeDict["CUBE_INFO_TITLE"] = "Recipe"
    localeDict["CUBE_REQUIRE_MATERIAL"] = "Requirements"
    localeDict["CUBE_REQUIRE_MATERIAL_OR"] = "or"

    for line in OpenVfsFile(srcFileName):
        tokens = line[:-1].split("\t")
        if len(tokens) >= 2:
            localeDict[tokens[0]] = tokens[1].strip()
        else:
            pass


name = app.GetLocalePath()

LOCALE_UISCRIPT_PATH = "%s/ui/" % name
LOGIN_PATH = "%s/ui/login/" % name
EMPIRE_PATH = "locale/common/ui/empire/"
SELECT_PATH = "locale/common/ui/select/"
WINDOWS_PATH = "%s/ui/windows/" % name
MAPNAME_PATH = "%s/ui/mapname/" % name

JOBDESC_WARRIOR_PATH = "%s/jobdesc_warrior.txt" % name
JOBDESC_ASSASSIN_PATH = "%s/jobdesc_assassin.txt" % name
JOBDESC_SURA_PATH = "%s/jobdesc_sura.txt" % name
JOBDESC_SHAMAN_PATH = "%s/jobdesc_shaman.txt" % name
JOBDESC_WARRIOR_PATH_OLD = "%s/jobdesc_warrior_old.txt" % name
JOBDESC_ASSASSIN_PATH_OLD = "%s/jobdesc_assassin_old.txt" % name
JOBDESC_SURA_PATH_OLD = "%s/jobdesc_sura_old.txt" % name
JOBDESC_SHAMAN_PATH_OLD = "%s/jobdesc_shaman_old.txt" % name
JOBDESC_WOLFMAN_PATH = "%s/jobdesc_wolfman.txt" % name
JOBDESC_WOLFMAN_PATH_OLD = "%s/jobdesc_wolfmam.txt" % name
EMPIREDESC_A = "%s/empiredesc_a.txt" % name
EMPIREDESC_B = "%s/empiredesc_b.txt" % name
EMPIREDESC_C = "%s/empiredesc_c.txt" % name

LOCALE_INTERFACE_FILE_NAME = "%s/locale_interface.txt" % name
LOCALE_INTERFACE_BASE_FILE_NAME = "locale/de/locale_interface.txt"

LoadLocaleFile(LOCALE_INTERFACE_BASE_FILE_NAME, locals())
LoadLocaleFile(LOCALE_INTERFACE_FILE_NAME, locals())
