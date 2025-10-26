#ifndef METIN2_SERVER_DB_MAIN_H
#define METIN2_SERVER_DB_MAIN_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_iItemPriceListTableCacheFlushSeconds;


int	Start();
void End();
const char * GetPlayerDBName();
const char * GetAccountDBName();

#endif /* METIN2_SERVER_DB_MAIN_H */
