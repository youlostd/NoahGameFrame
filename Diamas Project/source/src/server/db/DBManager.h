#ifndef METIN2_SERVER_DB_DBMANAGER_H
#define METIN2_SERVER_DB_DBMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#define ASYNC_SQL_USE_STORM
#include <mysql/AsyncSQL.h>
#include "Config.h"
#include <base/Singleton.hpp>

#define SQL_SAFE_LENGTH(size)	(size * 2 + 1)
#define QUERY_SAFE_LENGTH(size)	(1024 + SQL_SAFE_LENGTH(size))

struct CQueryInfo
{
	int	iType;
	uint32_t dwIdent;
	void* pvData;
};

enum eSQL_SLOT
{
	SQL_PLAYER,
	SQL_ACCOUNT,
	SQL_COMMON,
	SQL_LOG,
	SQL_MAX_NUM,
};

class CDBManager : public singleton<CDBManager>
{
public:
	CDBManager();
	virtual ~CDBManager();

	void Quit();

	bool Connect(uint32_t slot, const DbConfig& config);

	void ReturnQuery(const std::string & c_pszQuery, int iType,
		uint32_t dwIdent, void * pvData,
		uint32_t slot = SQL_PLAYER);

	void AsyncQuery(const std::string& c_pszQuery, uint32_t slot = SQL_PLAYER);
	SQLMsg* DirectQuery(const std::string& c_pszQuery, uint32_t slot = SQL_PLAYER);

	SQLMsg* PopResult();
	SQLMsg* PopResult(uint32_t slot);

	uint32_t EscapeString(void* to, const void* from,
		uint32_t length, uint32_t slot = SQL_PLAYER);

	uint32_t CountReturnQuery(uint32_t slot) const;
	uint32_t CountReturnResult(uint32_t slot) const;
	uint32_t CountReturnQueryFinished(uint32_t slot) const;

	uint32_t CountAsyncQuery(uint32_t slot) const;
	uint32_t CountAsyncResult(uint32_t slot) const;
	uint32_t CountAsyncQueryFinished(uint32_t slot) const;

	void ResetCounter();

private:
	CAsyncSQL m_mainSQL[SQL_MAX_NUM];
	CAsyncSQL m_directSQL[SQL_MAX_NUM];
	CAsyncSQL m_asyncSQL[SQL_MAX_NUM];
};


#endif /* METIN2_SERVER_DB_DBMANAGER_H */
