#ifndef METIN2_SERVER_GAME_DB_H
#define METIN2_SERVER_GAME_DB_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <mysql/AsyncSQL.h>
#include <game/Types.hpp>
#include <base/Singleton.hpp>

enum
{
    QUERY_TYPE_RETURN = 1,
    QUERY_TYPE_FUNCTION = 2,
    QUERY_TYPE_AFTER_FUNCTION = 3,
};

enum
{
    QID_HIGHSCORE_REGISTER,
    QID_HIGHSCORE_SHOW,

    // BLOCK_CHAT
    QID_BLOCK_CHAT_LIST,
    // END_OF_BLOCK_CHAT
    f
};

class CQueryInfo
{
public:
    int iQueryType;
};

class CReturnQueryInfo : public CQueryInfo
{
public:
    int iType;
    uint32_t dwIdent;
    void *pvData;
};

class CFuncQueryInfo : public CQueryInfo
{
public:
    std::function<void(SQLMsg *)> f;
};

class CFuncAfterQueryInfo : public CQueryInfo
{
public:
    std::function<void(void)> f;
};

class CLoginData;

class DBManager : public singleton<DBManager>
{
public:
    DBManager();
    virtual ~DBManager();

    bool IsConnected();

    bool Connect(const char *host, const int port, const char *user, const char *pwd, const char *db);

    template <typename ...Args>
    void Query(const std::string &query, const Args & ... args);

    template <typename ...Args>
    SQLMsg *DirectQuery(std::string_view format, Args ... args);

    template <typename... Args>
    void ReturnQuery(int iType, uint32_t dwIdent, void *pvData, const std::string &c_pszFormat, const Args & ... args);

    void Process();
    void AnalyzeReturnQuery(SQLMsg *pmsg);

    void SendMoneyLog(uint8_t type, uint32_t vnum, Gold gold);
#ifdef ENABLE_GEM_SYSTEM
		void SendGemLog(uint8_t type, int gem);
#endif

    uint32_t CountQuery() { return m_sql.CountQuery(); }

    uint32_t CountQueryResult() { return m_sql.CountResult(); }

    void ResetQueryResult() { m_sql.ResetQueryFinished(); }

    template <class Functor>
    void FuncQuery(Functor f, const char *c_pszFormat, ...); // 결과를 f인자로 호출함 (SQLMsg *) 알아서 해제됨
    template <class Functor>
    void FuncAfterQuery(Functor f, const char *c_pszFormat, ...); // 끝나고 나면 f가 호출됨 void			f(void) 형태

    size_t EscapeString(char *dst, size_t dstSize, const char *src, size_t srcSize);

private:
    SQLMsg *PopResult();

    CAsyncSQL m_sql;
    CAsyncSQL m_sql_direct;
    bool m_bIsConnect;
};

template <typename... Args>
void DBManager::Query(const std::string &query, const Args & ... args)
{
    m_sql.AsyncQuery(fmt::format(query, args...));
}

template <typename... Args>
void DBManager::ReturnQuery(int iType, uint32_t dwIdent, void *pvData, const std::string &c_pszFormat,
                            const Args & ... args)
{
    CReturnQueryInfo *p = new CReturnQueryInfo;

    p->iQueryType = QUERY_TYPE_RETURN;
    p->iType = iType;
    p->dwIdent = dwIdent;
    p->pvData = pvData;

    m_sql.ReturnQuery(fmt::format(c_pszFormat, args...), p);
}

template <typename ...Args>
inline SQLMsg *DBManager::DirectQuery(std::string_view format, Args ...args)
{
    return m_sql_direct.DirectQuery(fmt::format(format, args...));
}

template <class Functor> void DBManager::FuncQuery(Functor f, const char *c_pszFormat, ...)
{
    char szQuery[4096];
    va_list args;

    va_start(args, c_pszFormat);
    vsnprintf(szQuery, 4096, c_pszFormat, args);
    va_end(args);

    CFuncQueryInfo *p = new CFuncQueryInfo;

    p->iQueryType = QUERY_TYPE_FUNCTION;
    p->f = f;

    m_sql.ReturnQuery(szQuery, p);
}

template <class Functor> void DBManager::FuncAfterQuery(Functor f, const char *c_pszFormat, ...)
{
    char szQuery[4096];
    va_list args;

    va_start(args, c_pszFormat);
    vsnprintf(szQuery, 4096, c_pszFormat, args);
    va_end(args);

    CFuncAfterQueryInfo *p = new CFuncAfterQueryInfo;

    p->iQueryType = QUERY_TYPE_AFTER_FUNCTION;
    p->f = f;

    m_sql.ReturnQuery(szQuery, p);
}

////////////////////////////////////////////////////////////////
typedef struct SHighscoreRegisterQueryInfo
{
    char szBoard[20 + 1];
    bool bOrder;
    uint32_t dwPID;
    int iValue;
} THighscoreRegisterQueryInfo;

#endif /* METIN2_SERVER_GAME_DB_H */
