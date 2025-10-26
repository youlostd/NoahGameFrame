#ifndef __INC_METIN_II_ASYNCSQL_H__
#define __INC_METIN_II_ASYNCSQL_H__

#include <Config.hpp>

#include <thecore/stdafx.h>

#include <string>
#include <queue>
#include <vector>
#include <map>
#include <unordered_map>
#include <mysql.h>
#include <errmsg.h>
#include <mysqld_error.h>

#include <thread>
#include <mutex>

#include <thecore/semaphore.hpp>

#include <SpdLog.hpp>

#define ASQL_QUERY_MAX_LEN 8192

//Safe strlcpy method for mysql returning a nullptr.
inline void m_strlcpy(char* dst, char* src, size_t size)
{
	if (src == nullptr) { //if we got a nullptr set to \0 termination char
		strlcpy(dst, "\0", 1);
		return;
	}


	strlcpy(dst, src, size);
}
struct stringhash       
{
	size_t operator () (const std::string & str) const
	{
		const unsigned char * s = (const unsigned char*) str.c_str();
		const unsigned char * end = s + str.size();
		size_t h = 0;

		while (s < end)
		{
			h *= 16777619;
			h ^= *(s++);
		}

		return h;
	}
};

struct SQLResult
{
	SQLResult();
	~SQLResult();

	MYSQL_RES* pSQLResult;
	uint64_t uiNumRows;
	uint64_t uiAffectedRows;
	uint64_t uiInsertID;
	std::vector<std::vector<std::string>> rows;
	std::unordered_map<std::string, int32_t, stringhash> fieldToIndex;
};

struct SQLMsg
{
	SQLMsg();
	~SQLMsg();

	void Store();

	SQLResult* Get();
	bool Next();


	MYSQL* m_pkSQL;
	int iID;
	std::string stQuery;

	std::vector<SQLResult*> vec_pkResult; // result 벡터
	unsigned int uiResultPos; // 현재 result 위치

	void* pvUserData;
	bool bReturn;

	unsigned int uiSQLErrno;
};

class CAsyncSQL
{
public:
	CAsyncSQL();
	~CAsyncSQL();

	void Quit();

	bool Setup(const char* c_pszHost, const char* c_pszUser,
		const char* c_pszPassword,
		const char* c_pszDB, const char* c_pszLocale,
		bool bNoThread = false, int iPort = 0);

	bool Setup(CAsyncSQL* sql, bool bNoThread = false);

	void SetLocale(const std::string& stLocale);

	bool Connect();

	void AsyncQuery(const std::string& c_pszQuery);
	void ReturnQuery(const std::string& c_pszQuery, void* pvUserData);
	SQLMsg* DirectQuery(const std::string& c_pszQuery);

	uint32_t CountQuery() const;
	uint32_t CountResult() const;

	void PushResult(SQLMsg* p);
	bool PopResult(SQLMsg** pp);

	void ChildLoop();

	MYSQL* GetSQLHandle();

	int CountQueryFinished() const;
	void ResetQueryFinished();

	size_t EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize);

protected:
	void PushQuery(SQLMsg* p);

	std::queue<SQLMsg*> PopPendingQueries();

	MYSQL m_hDB;

	std::string m_stHost;
	std::string m_stUser;
	std::string m_stPassword;
	std::string m_stDB;
	std::string m_stLocale;
	bool m_connected;

	int	m_iMsgCount;
	int m_iPort;

	std::queue<SQLMsg*> m_queue_query;
	std::queue<SQLMsg*> m_queue_result;

	volatile bool m_bEnd;

	std::thread m_asyncThread;
	std::mutex m_queryMutex;
	std::mutex m_resultMutex;

	CSemaphore m_sem;

	int	m_iQueryFinished;
};

typedef CAsyncSQL CAsyncSQL2;
#endif
