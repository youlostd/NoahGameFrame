#include "StdAfx.h"
#include "Locale.h"
#include "PythonApplication.h"
#include "../eterBase/CRC32.h"
#include <pak/Vfs.hpp>
#include <windowsx.h>

const char *LSS_YMIR = "YMIR";
const char *LSS_JAPAN = "JAPAN";
const char *LSS_ENGLISH = "ENGLISH";
const char *LSS_HONGKONG = "HONGKONG";
const char *LSS_TAIWAN = "TAIWAN";
const char *LSS_NEWCIBN = "NEWCIBN";
const char *LSS_EUROPE = "EUROPE";
const char *LSS_GLOBAL = "GLOBAL";

static bool IS_CHEONMA = false;

#ifndef LSS_SECURITY_KEY
#define LSS_SECURITY_KEY	"testtesttesttest"
#endif

std::string __SECURITY_KEY_STRING__ = LSS_SECURITY_KEY;

char MULTI_LOCALE_SERVICE[256] = "GLOBAL";
char MULTI_LOCALE_PATH[256] = "locale/germany";
char MULTI_LOCALE_NAME[256] = "germany";
int MULTI_LOCALE_CODE = 1252;
int MULTI_LOCALE_REPORT_PORT = 10000;
int MULTI_LOCALE_ID = 0;

void LocaleService_LoadConfig(const char *fileName)
{
    FILE *fp = fopen(fileName, "rt");

    if (fp)
    {
        char line[256];
        char name[256];
        int code;
        int id;
        if (fgets(line, sizeof(line) - 1, fp))
        {
            line[sizeof(line) - 1] = '\0';
            sscanf(line, "%d %d %s", &id, &code, name);

            MULTI_LOCALE_REPORT_PORT = id;
            MULTI_LOCALE_CODE = code;
            strcpy(MULTI_LOCALE_NAME, name);
            sprintf(MULTI_LOCALE_PATH, "locale/%s", MULTI_LOCALE_NAME);
        }
        fclose(fp);
    }
}

unsigned LocaleService_GetNeededGuildExp(int level)
{
    static const int GUILD_LEVEL_MAX = 22;

    static uint32_t INTERNATIONAL_GUILDEXP_LIST[GUILD_LEVEL_MAX + 1] =
    {
        0,
        6000UL,
        18000UL,
        36000UL,
        64000UL,
        94000UL,
        130000UL,
        172000UL,
        220000UL,
        274000UL,
        334000UL, // 10
        400000UL,
        600000UL,
        840000UL,
        1120000UL,
        1440000UL,
        1800000UL,
        2600000UL,
        3200000UL,
        4000000UL,
 //       16800000UL, // 20
 //       48000000UL,
 //      58000000UL,
    };

    if (level < 0 || level >= GUILD_LEVEL_MAX)
        return 0;

    return INTERNATIONAL_GUILDEXP_LIST[level];
}

static const uint32_t kSkillPowerByLevel[50 + 1] = {
    0,                                                                           //
    5, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 50, // normal
    52, 54, 56, 58, 60, 63, 66, 69, 72, 82,                                      // master
    85, 88, 91, 94, 98, 102, 106, 110, 115,                                      // grand master
    125,                                                                         // perfect master
    125, 125, 125, 125, 125, 125, 125, 125, 125,
    125, // legendary master

};

#define _LSS_USE_LOCALE_CFG			1
#define _LSS_SERVICE_NAME			LSS_EUROPE

const char *LocaleService_GetName()
{
    return _LSS_SERVICE_NAME;
}

unsigned int LocaleService_GetCodePage()
{
    return MULTI_LOCALE_CODE;
}

const char *LocaleService_GetLocaleName()
{
    return MULTI_LOCALE_NAME;
}

uint8_t LocaleService_GetLocaleID()
{
    return MULTI_LOCALE_ID;
}

const char *LocaleService_GetLocalePath()
{
    return MULTI_LOCALE_PATH;
}

void LocaleService_ForceSetLocale(const char *name, const char *localePath)
{
    MULTI_LOCALE_ID = GetLanguageIDByName(name);
    strcpy(MULTI_LOCALE_NAME, name);
    strcpy(MULTI_LOCALE_PATH, localePath);
}

void LocaleService_SetLocale(const char *name, const char *localePath)
{
    CopyStringSafe(MULTI_LOCALE_NAME, name);
    CopyStringSafe(MULTI_LOCALE_PATH, localePath);
}
