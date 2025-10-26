#include "db.h"
#include "DbCacheSocket.hpp"
#include "char.h"
#include "char_manager.h"

DBManager::DBManager()
    : m_bIsConnect(false)
{
}

DBManager::~DBManager()
{
}

bool DBManager::Connect(const char *host, const int port, const char *user, const char *pwd, const char *db)
{
    if (m_sql.Setup(host, user, pwd, db, "utf8", false, port))
        m_bIsConnect = true;

    if (!m_sql_direct.Setup(host, user, pwd, db, "utf8", true, port))
        SPDLOG_ERROR("cannot open direct sql connection to host %s", host);

    return m_bIsConnect;
}

bool DBManager::IsConnected() { return m_bIsConnect; }

SQLMsg *DBManager::PopResult()
{
    SQLMsg *p;

    if (m_sql.PopResult(&p))
        return p;

    return nullptr;
}

void DBManager::Process()
{
    rmt_ScopedCPUSample(DBProcess, 0);

    SQLMsg *pMsg = nullptr;

    while ((pMsg = PopResult()))
    {
        if (nullptr != pMsg->pvUserData)
        {
            switch (reinterpret_cast<CQueryInfo *>(pMsg->pvUserData)->iQueryType)
            {
            case QUERY_TYPE_RETURN:
                AnalyzeReturnQuery(pMsg);
                break;

            case QUERY_TYPE_FUNCTION: {
                CFuncQueryInfo *qi = reinterpret_cast<CFuncQueryInfo *>(pMsg->pvUserData);
                qi->f(pMsg);
                delete (qi);
            }
            break;

            case QUERY_TYPE_AFTER_FUNCTION: {
                CFuncAfterQueryInfo *qi = reinterpret_cast<CFuncAfterQueryInfo *>(pMsg->pvUserData);
                qi->f();
                delete (qi);
            }
            break;
            }
        }

        delete pMsg;
    }
}

void DBManager::AnalyzeReturnQuery(SQLMsg *pMsg)
{
    CReturnQueryInfo *qi = (CReturnQueryInfo *)pMsg->pvUserData;

    switch (qi->iType)
    {
    case QID_HIGHSCORE_REGISTER: {
        THighscoreRegisterQueryInfo *info = (THighscoreRegisterQueryInfo *)qi->pvData;
        bool bQuery = true;

        if (pMsg->Get()->uiNumRows)
        {
            MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

            if (row && row[0])
            {
                int iCur = 0;
                str_to_number(iCur, row[0]);

                if ((info->bOrder && iCur >= info->iValue) || (!info->bOrder && iCur <= info->iValue))
                    bQuery = false;
            }
        }

        if (bQuery)
            Query("REPLACE INTO highscore VALUES('{}', {}, {})", info->szBoard, info->dwPID, info->iValue);

        delete (info);
    }
    break;

    case QID_HIGHSCORE_SHOW: {
    }
    break;

        // BLOCK_CHAT
    case QID_BLOCK_CHAT_LIST: {
        CHARACTER *ch = g_pCharManager->FindByPID(qi->dwIdent);

        if (ch == nullptr)
            break;
        if (pMsg->Get()->uiNumRows)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
            {
                ch->ChatPacket(CHAT_TYPE_INFO, "%s %s sec", row[0], row[1]);
            }
        }
        else { ch->ChatPacket(CHAT_TYPE_INFO, "No one currently blocked."); }
    }
    break;
        // END_OF_BLOCK_CHAT

    default:
        SPDLOG_ERROR("FATAL ERROR!!! Unhandled return query id %d", qi->iType);
        break;
    }

    delete (qi);
}

void DBManager::SendMoneyLog(uint8_t type, uint32_t vnum, Gold gold)
{
    if (!gold)
        return;
    TPacketMoneyLog p;
    p.type = type;
    p.vnum = vnum;
    p.gold = gold;
    db_clientdesc->DBPacket(HEADER_GD_MONEY_LOG, 0, &p, sizeof(p));
}

#ifdef ENABLE_GEM_SYSTEM
void DBManager::SendGemLog(uint8_t type, int gem)
{
    if (!gem)
        return;

    TPacketGemLog p;
    p.type = type;
    p.gem = gem;
    db_clientdesc->DBPacket(HEADER_GD_GEM_LOG, 0, &p, sizeof(p));
}
#endif

size_t DBManager::EscapeString(char *dst, size_t dstSize, const char *src, size_t srcSize)
{
    return m_sql_direct.EscapeString(dst, dstSize, src, srcSize);
}
