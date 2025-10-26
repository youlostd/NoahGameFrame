#ifndef METIN2_SERVER_DB_PRIVMANAGER_H
#define METIN2_SERVER_DB_PRIVMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include "Peer.h"
#include <queue>
#include <utility>
#include <game/Constants.hpp>
#include <base/Singleton.hpp>

struct TPrivEmpireData
{
    uint8_t type;
    int value;
    bool bRemoved;
    uint8_t empire;

	// ADD_EMPIRE_PRIV_TIME
    time_t end_time_sec;

    TPrivEmpireData(uint8_t type, int value, uint8_t empire, time_t end_time_sec)
	: type(type), value(value), bRemoved(false), empire(empire), end_time_sec(end_time_sec)
    {}
	// END_OF_ADD_EMPIRE_PRIV_TIME
};

/**
 * @version 05/06/08	Bang2ni - ���ӽð� �߰�
 */
struct TPrivGuildData
{
    uint8_t type;
    int value;
    bool bRemoved;
    uint32_t guild_id;

	// ADD_GUILD_PRIV_TIME
    time_t end_time_sec;	///< ���ӽð�

    TPrivGuildData(uint8_t type, int value, uint32_t guild_id, time_t _end_time_sec)
	: type(type), value(value), bRemoved(false), guild_id(guild_id), end_time_sec(_end_time_sec )
    {}
	// END_OF_ADD_GUILD_PRIV_TIME
};

struct TPrivCharData
{
	bool bRemoved;
	uint32_t pid;
	uint8_t type;
    int value;
    TPrivCharData(uint8_t type, int value, uint32_t pid)
	: type(type), value(value), bRemoved(false), pid(pid)
    {}
};

/**
 * @version 05/06/08	Bang2ni - Guild privilege ���� �Լ� ���� �ð� �߰�
 */
class CPrivManager : public singleton<CPrivManager>
{
    public: 
	CPrivManager();
	virtual ~CPrivManager();

	// ADD_GUILD_PRIV_TIME
	void AddGuildPriv(uint32_t guild_id, uint8_t type, int value, uint32_t time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME

	// ADD_EMPIRE_PRIV_TIME	
	void AddEmpirePriv(uint8_t empire, uint8_t type, int value, uint32_t time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME
	
	void AddCharPriv(uint32_t pid, uint8_t type, int value);

	void Update();

	void SendPrivOnSetup(CPeer* peer);

    private:

	// ADD_GUILD_PRIV_TIME
	void SendChangeGuildPriv(uint32_t guild_id, uint8_t type, int value, uint32_t end_time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME

	// ADD_EMPIRE_PRIV_TIME	
	void SendChangeEmpirePriv(uint8_t empire, uint8_t type, int value, uint32_t end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME

	void SendChangeCharPriv(uint32_t pid, uint8_t type, int value);

	typedef std::pair<std::chrono::system_clock::time_point, std::shared_ptr<TPrivCharData>> stPairChar;
	typedef std::pair<std::chrono::system_clock::time_point, std::shared_ptr<TPrivGuildData>> stPairGuild;
	typedef std::pair<std::chrono::system_clock::time_point, std::shared_ptr<TPrivEmpireData>> stPairEmpire;

	std::priority_queue<stPairChar, std::vector<stPairChar>, std::greater<stPairChar> >
	    m_pqPrivChar;
	std::priority_queue<stPairGuild, std::vector<stPairGuild>, std::greater<stPairGuild> > 
	    m_pqPrivGuild;
	std::priority_queue<stPairEmpire, std::vector<stPairEmpire>, std::greater<stPairEmpire> >
	    m_pqPrivEmpire;

	std::array<std::shared_ptr<TPrivEmpireData>, EMPIRE_MAX_NUM>  m_aaPrivEmpire[MAX_PRIV_NUM];
	std::map<uint32_t, std::shared_ptr<TPrivGuildData>> m_aPrivGuild[MAX_PRIV_NUM];
	std::map<uint32_t, std::shared_ptr<TPrivCharData>> m_aPrivChar[MAX_PRIV_NUM];
};

#endif /* METIN2_SERVER_DB_PRIVMANAGER_H */
