#include "stdafx.h"
#include "DBManager.h"
#include "ClientManager.h"
#include "Main.h"
#include <storm/ErrorReporting.hpp>
#include "Config.h"


extern std::string g_stLocale;

CDBManager::CDBManager()
{
}

CDBManager::~CDBManager()
{
}

void CDBManager::Quit()
{
	for (int i = 0; i < SQL_MAX_NUM; ++i)
	{
		m_mainSQL[i].Quit();
		m_asyncSQL[i].Quit();
		m_directSQL[i].Quit();
	}
}

SQLMsg* CDBManager::PopResult()
{
	SQLMsg* p;

	for (int i = 0; i < SQL_MAX_NUM; ++i)
		if (m_mainSQL[i].PopResult(&p))
			return p;

	return NULL;
}

SQLMsg* CDBManager::PopResult(uint32_t slot)
{
	SQLMsg * p;
	if (m_mainSQL[slot].PopResult(&p))
		return p;

	return NULL;
}

bool CDBManager::Connect(uint32_t slot, const DbConfig& config)
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");

	if (!m_directSQL[slot].Setup(config.addr.c_str(), config.user.c_str(),
	                             config.password.c_str(), config.name.c_str(),
	                             "utf8", true, config.port))
		return false;

	if (!m_mainSQL[slot].Setup(config.addr.c_str(), config.user.c_str(),
	                           config.password.c_str(), config.name.c_str(),
	                           "utf8", false, config.port))
		return false;

	if (!m_asyncSQL[slot].Setup(config.addr.c_str(), config.user.c_str(),
	                            config.password.c_str(), config.name.c_str(),
	                            "utf8", false, config.port))
		return false;

	return true;
}

SQLMsg * CDBManager::DirectQuery(const std::string&  c_pszQuery, uint32_t slot)
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_directSQL[slot].DirectQuery(c_pszQuery);
}

void CDBManager::ReturnQuery(const std::string& c_pszQuery, int iType,
	uint32_t dwIdent, void* udata, uint32_t slot)
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");

	CQueryInfo * p = new CQueryInfo;

	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = udata;

	m_mainSQL[slot].ReturnQuery(c_pszQuery.c_str(), p);
}

void CDBManager::AsyncQuery(const std::string& c_pszQuery, uint32_t slot)
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	m_asyncSQL[slot].AsyncQuery(c_pszQuery);
}

uint32_t CDBManager::EscapeString(void *to, const void *from, uint32_t length, uint32_t slot)
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return mysql_real_escape_string(m_directSQL[slot].GetSQLHandle(),
		static_cast<char*>(to),
		static_cast<const char*>(from),
		length);
}

uint32_t CDBManager::CountReturnQuery(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_mainSQL[slot].CountQuery();
}

uint32_t CDBManager::CountReturnResult(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_mainSQL[slot].CountResult();
}

uint32_t CDBManager::CountReturnQueryFinished(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_mainSQL[slot].CountQueryFinished();
}

uint32_t CDBManager::CountAsyncQuery(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_asyncSQL[slot].CountQuery();
}

uint32_t CDBManager::CountAsyncResult(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_asyncSQL[slot].CountResult();
}

uint32_t CDBManager::CountAsyncQueryFinished(uint32_t slot) const
{
	STORM_ASSERT(slot < SQL_MAX_NUM, "Out of bounds");
	return m_asyncSQL[slot].CountQueryFinished();
}

void CDBManager::ResetCounter()
{
	for (uint32_t i = 0; i < SQL_MAX_NUM; ++i) {
		m_mainSQL[i].ResetQueryFinished();
		m_asyncSQL[i].ResetQueryFinished();
	}
}
