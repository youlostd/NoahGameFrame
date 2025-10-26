#ifndef METIN2_SERVER_DB_MONEYLOG_H
#define METIN2_SERVER_DB_MONEYLOG_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <map>
#include <game/Types.hpp>
#include <base/Singleton.hpp>

class CMoneyLog : public singleton<CMoneyLog>
{
    public:
	CMoneyLog();
	virtual ~CMoneyLog();

	void Save();
	void AddLog(uint8_t bType, uint32_t dwVnum, Gold iGold);

    private:
	std::map<uint32_t, Gold> m_MoneyLogContainer[MONEY_LOG_TYPE_MAX_NUM];
};

#ifdef ENABLE_GEM_SYSTEM
class CGemLog : public singleton < CGemLog >
{
public:
	CGemLog();
	virtual ~CGemLog();

	void Save();
	void AddLog(uint8_t bType, int gem);
private:
	std::map<uint32_t, int> m_GemLogContainer[GEM_LOG_TYPE_MAX_NUM];
};
#endif

#endif /* METIN2_SERVER_DB_MONEYLOG_H */
