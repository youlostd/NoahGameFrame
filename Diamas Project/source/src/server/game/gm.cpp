#include "constants.h"
#include "gm.h"
#include "config.h"
#include "locale_service.h"
#ifdef __NEW_GAMEMASTER_CONFIG__
#include "utils.h"
#include "questmanager.h"

namespace GM
{
uint32_t g_adwAdminConfig[GM_MAX_NUM];

//ADMIN_MANAGER
typedef struct
{
    tAdminInfo Info;
} tGM;

std::map<std::string, tGM> g_map_GM;

void init(uint32_t *adwAdminConfig) { memcpy(&g_adwAdminConfig[0], adwAdminConfig, sizeof(g_adwAdminConfig)); }

void insert(const tAdminInfo &rAdminInfo)
{
    SPDLOG_INFO("InsertGMList(account:{}, player:{}, auth:{})",
                rAdminInfo.m_szAccount,
                rAdminInfo.m_szName,
                rAdminInfo.m_Authority);

    tGM t;

    std::memcpy(&t.Info, &rAdminInfo, sizeof(rAdminInfo));

    g_map_GM[rAdminInfo.m_szName] = t;
}

void remove(const char *szName) { g_map_GM.erase(szName); }

uint8_t get_level(const char *name, const char *account, bool ignore_test_server)
{
    if (!ignore_test_server && gConfig.testServer)
        return GM_IMPLEMENTOR;

    std::map<std::string, tGM>::iterator it = g_map_GM.find(name);

    if (g_map_GM.end() == it)
        return GM_PLAYER;

    if (account)
    {
        if (strcasecmp(it->second.Info.m_szAccount, account) != 0 && strcmp(it->second.Info.m_szAccount, "[ALL]") != 0)
        {
            SPDLOG_ERROR("GM::get_level: account compare failed [real account %s need account %s]", account,
                         it->second.Info.m_szAccount);
            return GM_PLAYER;
        }
    }

    SPDLOG_INFO("GM::GET_LEVEL : FIND ACCOUNT");
    return it->second.Info.m_Authority;
}

void clear() { g_map_GM.clear(); }

bool check_allow(unsigned char bGMLevel, uint32_t dwCheckFlag)
{
    SPDLOG_INFO("g_adwAdminConfig[bGMLevel] = {0}", g_adwAdminConfig[bGMLevel]);
    return IS_SET(g_adwAdminConfig[bGMLevel], dwCheckFlag);
}

bool check_account_allow(const std::string &stAccountName, uint32_t dwCheckFlag)
{
    std::map<std::string, tGM>::iterator it = g_map_GM.begin();

    bool bHasGM = false;
    bool bCheck = false;
    while (it != g_map_GM.end() && !bCheck)
    {
        if (!stAccountName.compare(it->second.Info.m_szAccount))
        {
            bHasGM = true;

            unsigned char bGMLevel = it->second.Info.m_Authority;
            bCheck = check_allow(bGMLevel, dwCheckFlag);
        }

        ++it;
    }

    return !bHasGM || bCheck;
}
}

#else

//ADMIN_MANAGER
typedef struct 
{
	tAdminInfo Info;		
	std::set<std::string> *pset_Host;	
} tGM;

std::set<std::string> g_set_Host;
std::map<std::string, tGM> g_map_GM;

void gm_new_clear()
{
	g_set_Host.clear();
	g_map_GM.clear();
}

void gm_new_insert( const tAdminInfo &rAdminInfo )
{	
	SPDLOG_INFO( "InsertGMList(account:%s, player:%s, contact_ip:%s, server_ip:%s, auth:%d)", 
			rAdminInfo.m_szAccount,
			rAdminInfo.m_szName,
			rAdminInfo.m_szContactIP,
			rAdminInfo.m_szServerIP,
			rAdminInfo.m_Authority);

	tGM t;

	if ( rAdminInfo.m_szContactIP[0] == '\0' )
	{
		t.pset_Host = &g_set_Host;
		SPDLOG_INFO( "GM Use ContactIP" );
	}
	else
	{
		t.pset_Host = NULL;
		SPDLOG_INFO( "GM Use Default Host List" );
	}

	std::memcpy ( &t.Info, &rAdminInfo, sizeof ( rAdminInfo ) );

	g_map_GM[rAdminInfo.m_szName] = t;

}

void gm_new_host_inert( const char * host )
{
	g_set_Host.insert( host );
	SPDLOG_INFO( "InsertGMHost(ip:%s)", host );
}

uint8_t gm_new_get_level( const char * name, const char * host, const char* account)
{
	if ( gConfig.testServer ) return GM_IMPLEMENTOR;

	std::map<std::string, tGM >::iterator it = g_map_GM.find(name);

	if (g_map_GM.end() == it)
		return GM_PLAYER;

    if (account)
    {
		if ( strcmp ( it->second.Info.m_szAccount, account  ) != 0 )
		{
			SPDLOG_INFO( "GM_NEW_GET_LEVEL : BAD ACCOUNT [ACCOUNT:{}/{}", it->second.Info.m_szAccount, account);
			return GM_PLAYER;
		}
	}

	SPDLOG_INFO( "GM_NEW_GET_LEVEL : FIND ACCOUNT");
	return it->second.Info.m_Authority;

	return GM_PLAYER;
}

std::vector<std::string> get_gm_list()
{
	std::map<std::string, tGM >::iterator it;
	std::vector<std::string> gm_list;

	for (it = g_map_GM.begin(); it != g_map_GM.end(); ++it)
	{
		gm_list.push_back(it->second.Info.m_szName); // only push player name back
	}				

	return gm_list;
}
	
//END_ADMIN_MANAGER
uint8_t gm_get_level(const char * name, const char * host, const char* account)
{
	return gm_new_get_level( name, host, account );
}


uint8_t gm_get_level_by_name(const char * name)
{
	std::map<std::string, tGM >::iterator it = g_map_GM.find(name);

	if (g_map_GM.end() == it)
		return GM_PLAYER;

	return it->second.Info.m_Authority;
}
#endif
