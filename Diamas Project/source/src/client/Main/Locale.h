#ifndef METIN2_CLIENT_MAIN_LOCALE_H
#define METIN2_CLIENT_MAIN_LOCALE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

unsigned LocaleService_GetCodePage();
const char *LocaleService_GetName();
const char *LocaleService_GetLocaleName();
const char *LocaleService_GetLocalePath();
uint8_t LocaleService_GetLocaleID();

void LocaleService_ForceSetLocale(const char *name, const char *localePath);
void LocaleService_LoadConfig(const char *fileName);
unsigned LocaleService_GetNeededGuildExp(int level);

#endif /* METIN2_CLIENT_MAIN_LOCALE_H */
