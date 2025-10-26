#include "AsyncSQL.h"
#include <mysql.h>

#include <cstdlib>
#include <cstring>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <errmsg.h>
#include <mysqld_error.h>
#include <SpdLog.hpp>

namespace
{

bool ShouldRetry(uint32_t errorCode)
{
    switch (errorCode)
    {
    case CR_SOCKET_CREATE_ERROR:
    case CR_CONNECTION_ERROR:
    case CR_IPSOCK_ERROR:
    case CR_UNKNOWN_HOST:
    case CR_SERVER_GONE_ERROR:
    case CR_CONN_HOST_ERROR:
    case CR_SERVER_LOST:
    case ER_NOT_KEYFILE:
    case ER_CRASHED_ON_USAGE:
    case ER_CANT_OPEN_FILE:
    case ER_HOST_NOT_PRIVILEGED:
    case ER_HOST_IS_BLOCKED:
    case ER_PASSWORD_NOT_ALLOWED:
    case ER_PASSWORD_NO_MATCH:
    case ER_CANT_CREATE_THREAD:
    case ER_INVALID_USE_OF_NULL:
        return true;

    default:
        return false;
    }
}
} // namespace

SQLResult::SQLResult() : pSQLResult(nullptr), uiNumRows(0), uiAffectedRows(0), uiInsertID(0)
{
}

SQLResult::~SQLResult()
{
    if (pSQLResult)
    {
        mysql_free_result(pSQLResult);
        pSQLResult = nullptr;
    }
}

SQLMsg::SQLMsg() : m_pkSQL(nullptr), iID(0), uiResultPos(0), pvUserData(nullptr), bReturn(false), uiSQLErrno(0)
{
}

SQLMsg::~SQLMsg()
{
    for (const auto &p : vec_pkResult)
        delete p;
}

void SQLMsg::Store()
{
    do
    {
        auto* pRes = new SQLResult;
        pRes->pSQLResult = mysql_store_result(m_pkSQL);
        pRes->uiInsertID = mysql_insert_id(m_pkSQL);
        pRes->uiAffectedRows = mysql_affected_rows(m_pkSQL);

        if (pRes->pSQLResult)
        {
            // fetch row
	        MYSQL_ROW row = 0;
	        while ((row = mysql_fetch_row(pRes->pSQLResult)))
	        {
		        unsigned long* lengths = mysql_fetch_lengths(pRes->pSQLResult);
		        if (lengths == 0 || row == 0)
		        {
			        continue;
		        }

	            std::vector<std::string> r;
		        // Grab the full value and add it to the vector.
		        for (unsigned int i = 0; i < mysql_num_fields(pRes->pSQLResult); i++)
		        {
			        char* temp = new char[lengths[i] + 1];
			        memcpy(temp, row[i], lengths[i]);
			        temp[lengths[i]] = '\0';
			        r.emplace_back(temp);
			        delete [] temp;
		        }

                pRes->rows.push_back(r);
	        }

            mysql_data_seek(pRes->pSQLResult, 0);
            pRes->uiNumRows = mysql_num_rows(pRes->pSQLResult);

        }

        if (!pRes->pSQLResult)
            pRes->uiNumRows = 0;

        vec_pkResult.push_back(pRes);
    } while (!mysql_next_result(m_pkSQL));
}

SQLResult *SQLMsg::Get()
{
    if (uiResultPos >= vec_pkResult.size())
        return nullptr;

    return vec_pkResult[uiResultPos];
}

bool SQLMsg::Next()
{
    if (uiResultPos + 1 >= vec_pkResult.size())
        return false;

    ++uiResultPos;
    return true;
}

CAsyncSQL::CAsyncSQL() : m_connected(false), m_iMsgCount(0), m_iPort(0), m_bEnd(false), m_iQueryFinished(0)
{
    if (0 == mysql_init(&m_hDB))
        SPDLOG_ERROR("mysql_init failed");
}

CAsyncSQL::~CAsyncSQL()
{
    Quit();

    // if (!m_stHost.empty()) {
    // SPDLOG_INFO( "Disconnecting from %s@%s:%s", m_stUser.c_str(),
    //        m_stHost.c_str(), m_stDB.c_str());
    //}

    mysql_close(&m_hDB);
}

bool CAsyncSQL::Connect()
{

    if (!mysql_real_connect(&m_hDB, m_stHost.c_str(), m_stUser.c_str(), m_stPassword.c_str(), m_stDB.c_str(), m_iPort,
                            NULL, CLIENT_MULTI_STATEMENTS))
    {
        spdlog::error("mysql_real_connect to {0}@{1}:{2}: {3}", m_stUser, m_stHost, m_stDB, mysql_error(&m_hDB));

        return false;
    }

    my_bool reconnect = true;
    if (0 != mysql_options(&m_hDB, MYSQL_OPT_RECONNECT, &reconnect))
    {
        spdlog::error("mysql_option: {0}", mysql_error(&m_hDB));

        return false;
    }

    if (mysql_set_character_set(&m_hDB, m_stLocale.c_str()))
    {
        spdlog::error("Failed to set locale {0} with errno {1} {2}", m_stLocale, mysql_errno(&m_hDB),
                      mysql_error(&m_hDB));
    }

    SPDLOG_INFO("Connected to MySQL server {0}:{1} as {2}", m_stHost, m_stDB, m_stUser);

    m_connected = true;
    return true;
}

void CAsyncSQL::Quit()
{
    m_bEnd = true;
    m_sem.Release();

    if (m_asyncThread.joinable())
        m_asyncThread.join();
}

bool CAsyncSQL::Setup(CAsyncSQL *sql, bool bNoThread)
{
    return Setup(sql->m_stHost.c_str(), sql->m_stUser.c_str(), sql->m_stPassword.c_str(), sql->m_stDB.c_str(),
                 sql->m_stLocale.c_str(), bNoThread, sql->m_iPort);
}

bool CAsyncSQL::Setup(const char *c_pszHost, const char *c_pszUser, const char *c_pszPassword, const char *c_pszDB,
                      const char *c_pszLocale, bool bNoThread, int iPort)
{
    m_stHost = c_pszHost;
    m_stUser = c_pszUser;
    m_stPassword = c_pszPassword;
    m_stDB = c_pszDB;
    m_iPort = iPort;

    if (c_pszLocale)
        m_stLocale = c_pszLocale;

    if (!bNoThread)
    {
        auto f = [this]() {
            mysql_thread_init();

            if (Connect())
                ChildLoop();

            mysql_thread_end();
        };

        assert(!m_asyncThread.joinable() && "Thread must not be joinable, "
                                            "otherwise terminate() is called");

        std::thread thr(f);
        std::swap(m_asyncThread, thr);

        return true;
    }
    else
    {
        return Connect();
    }
}

void CAsyncSQL::SetLocale(const std::string &stLocale)
{
    assert(!stLocale.empty() && "Empty locale");

    m_stLocale = stLocale;
    if (m_connected && mysql_set_character_set(&m_hDB, m_stLocale.c_str()))
    {
        SPDLOG_ERROR("Failed to set locale {0} with errno {1} {2}", m_stLocale, mysql_errno(&m_hDB),
                      mysql_error(&m_hDB));
    }
}

SQLMsg *CAsyncSQL::DirectQuery(const std::string &c_pszQuery)
{
    assert(!m_asyncThread.joinable() && "DirectQuery not allowed");
    assert(m_connected && "We were never connected");

    auto* p = new SQLMsg;
    p->m_pkSQL = &m_hDB;
    p->iID = ++m_iMsgCount;
    p->stQuery = c_pszQuery;

    if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
    {
        p->uiSQLErrno = mysql_errno(&m_hDB);
    }

    p->Store();
    return p;
}

void CAsyncSQL::AsyncQuery(const std::string &c_pszQuery)
{
    auto* p = new SQLMsg;
    p->m_pkSQL = &m_hDB;
    p->iID = ++m_iMsgCount;
    p->stQuery = c_pszQuery;

    PushQuery(p);
}

void CAsyncSQL::ReturnQuery(const std::string &c_pszQuery, void *pvUserData)
{
    auto* p = new SQLMsg;
    p->m_pkSQL = &m_hDB;
    p->iID = ++m_iMsgCount;
    p->stQuery = c_pszQuery;
    p->bReturn = true;
    p->pvUserData = pvUserData;

    PushQuery(p);
}

void CAsyncSQL::PushResult(SQLMsg *p)
{
    std::lock_guard<std::mutex> lock(m_resultMutex);
    m_queue_result.push(p);
}

bool CAsyncSQL::PopResult(SQLMsg **pp)
{
    std::lock_guard<std::mutex> lock(m_resultMutex);

    if (m_queue_result.empty())
        return false;

    *pp = m_queue_result.front();
    m_queue_result.pop();
    return true;
}

void CAsyncSQL::PushQuery(SQLMsg *p)
{
    {
        std::lock_guard<std::mutex> lock(m_queryMutex);
        m_queue_query.push(p);
    }

    m_sem.Release();
}

std::queue<SQLMsg *> CAsyncSQL::PopPendingQueries()
{
    std::lock_guard<std::mutex> lock(m_queryMutex);

    std::queue<SQLMsg *> queue;
    swap(queue, m_queue_query);

    return queue;
}

uint32_t CAsyncSQL::CountQuery() const
{
    // [noff] Potential race condition here
    // count only used for reporting => irrelevant?
    return m_queue_query.size();
}

uint32_t CAsyncSQL::CountResult() const
{
    // [noff] Potential race condition here
    // count only used for reporting => irrelevant?
    return m_queue_result.size();
}

void CAsyncSQL::ChildLoop()
{
    while (!m_bEnd)
    {
        
        m_sem.Wait();

        auto queue = PopPendingQueries();
        while (!queue.empty())
        {
            auto* p = queue.front();

            const auto startTime = get_dword_time();
            if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
            {
                p->uiSQLErrno = mysql_errno(&m_hDB);

                const bool retry = ShouldRetry(p->uiSQLErrno);

                if (retry)
                    continue;
            }

            const auto runTime = get_dword_time() - startTime;


            queue.pop();

            p->Store();

            if (p->bReturn)
                PushResult(p);
            else
                delete p;

            ++m_iQueryFinished;
        }
    }

    auto queue = PopPendingQueries();
    while (!queue.empty())
    {
        auto* p = queue.front();
        if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
        {
            p->uiSQLErrno = mysql_errno(&m_hDB);

            const bool retry = ShouldRetry(p->uiSQLErrno);

            if (retry)
                continue;
        }


        queue.pop();

        p->Store();

        if (p->bReturn)
            PushResult(p);
        else
            delete p;

        ++m_iQueryFinished;
    }
}

int CAsyncSQL::CountQueryFinished() const
{
    return m_iQueryFinished;
}

void CAsyncSQL::ResetQueryFinished()
{
    m_iQueryFinished = 0;
}

MYSQL *CAsyncSQL::GetSQLHandle()
{
    return &m_hDB;
}

size_t CAsyncSQL::EscapeString(char *dst, size_t dstSize, const char *src, size_t srcSize)
{
    if (0 == srcSize)
    {
        memset(dst, 0, dstSize);
        return 0;
    }

    if (0 == dstSize)
        return 0;

    if (dstSize < srcSize * 2 + 1)
    {
        char tmp[256];
        size_t tmpLen = sizeof(tmp) > srcSize ? srcSize : sizeof(tmp);
        strlcpy(tmp, src, tmpLen);

 
        dst[0] = '\0';
        return 0;
    }

    return mysql_real_escape_string(&m_hDB, dst, src, srcSize);
}
