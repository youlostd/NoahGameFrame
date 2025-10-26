#ifndef METIN2_SERVER_GAME_GM_H
#define METIN2_SERVER_GAME_GM_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef __NEW_GAMEMASTER_CONFIG__
namespace GM
{
void init(uint32_t *adwAdminConfig);
void insert(const tAdminInfo &c_rInfo);
void remove(const char *name);
uint8_t get_level(const char *name, const char *account = nullptr, bool ignore_test_server = false);
void clear();

bool check_allow(unsigned char bGMLevel, uint32_t dwCheckFlag);
bool check_account_allow(const std::string &stAccountName, uint32_t dwCheckFlag);
}
#else
extern void gm_insert(const char * name, uint8_t level);
extern uint8_t gm_get_level(const char * name, const char * host, const char * account);
extern uint8_t gm_get_level_by_name(const char * name);
extern void gm_host_insert(const char * host);
extern void gm_new_clear();
extern void gm_new_insert( const tAdminInfo & c_rInfo );
extern void gm_new_host_inert( const char * host );
extern std::vector<std::string> get_gm_list();
extern bool gm_verify_key(const char * name, const char * account, const char * key);
#endif

#endif /* METIN2_SERVER_GAME_GM_H */
