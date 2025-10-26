#include "constants.h"
#include "gm.h"
#include "desc.h"
#include "messenger_manager.h"
#include "GBufferManager.h"
#include "DbCacheSocket.hpp"
#include "log.h"
#include "config.h"

#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#include "desc_manager.h"
#include <game/MasterPackets.hpp>
#include <base/Crc32.hpp>

void MessengerManager::P2PLogin(MessengerManager::keyA account) { Login(account); }

void MessengerManager::P2PLogout(MessengerManager::keyA account) { Logout(account); }

void MessengerManager::Login(MessengerManager::keyA account)
{
    if (m_set_loginAccount.find(account) != m_set_loginAccount.end())
        return;

    DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadList, this, std::placeholders::_1),
                                    "SELECT account, companion FROM messenger_list WHERE account='%s'",
                                    account.c_str());

    m_set_loginAccount.insert(account);

    SendGMList(account);

    auto it = std::find_if(m_gameMasters.begin(), m_gameMasters.end(), [&account](const GameMaster &g)
    {
        return g.name == account;
    });
    if (it != m_gameMasters.end()) { for (auto &loginAccount : m_set_loginAccount) { SendGMLogin(loginAccount, *it); } }
}

void MessengerManager::LoadList(SQLMsg *msg)
{
    if (nullptr == msg)
        return;

    if (nullptr == msg->Get())
        return;

    if (msg->Get()->uiNumRows == 0)
        return;

    std::string account;

    SPDLOG_TRACE("Messenger::LoadList");

    for (uint i = 0; i < msg->Get()->uiNumRows; ++i)
    {
        MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

        if (row[0] && row[1])
        {
            if (account.length() == 0)
                account = row[0];

            m_Relation[row[0]].insert(row[1]);
            m_InverseRelation[row[1]].insert(row[0]);
        }
    }

    SendList(account);

    for (auto key : m_InverseRelation[account])
        SendLogin(key, account);
}

void MessengerManager::SendGMLogin(keyA account, const GameMaster &gm)
{
    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (!d)
        return;

    if (!d->GetCharacter())
        return;

    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_GM_LOGIN;
    pack.name = gm.name;
    pack.lang = gm.lang;
    d->Send(HEADER_GC_MESSENGER, pack);

}

void MessengerManager::SendGMLogout(keyA account, const GameMaster &gm)
{
    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (!d)
        return;

    if (!d->GetCharacter())
        return;

    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_GM_LOGOUT;
    pack.name = gm.name;
    pack.lang = gm.lang;
    d->Send(HEADER_GC_MESSENGER, pack);
}

void MessengerManager::Logout(MessengerManager::keyA account)
{
    if (m_set_loginAccount.find(account) == m_set_loginAccount.end())
        return;

    m_set_loginAccount.erase(account);
    for (auto it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
    {
        SendLogout(*it, account);
    }

    auto it2 = m_Relation.begin();

    while (it2 != m_Relation.end())
    {
        it2->second.erase(account);
        ++it2;
    }

    m_Relation.erase(account);

    auto it = std::find_if(m_gameMasters.begin(), m_gameMasters.end(), [&account](const GameMaster &g)
    {
        return g.name == account;
    });

    if (it != m_gameMasters.end())
    {
        for (auto &loginAccount : m_set_loginAccount) { SendGMLogout(loginAccount, *it); }
    }
}

void MessengerManager::LoadGMList(SQLMsg *msg)
{
    if (nullptr == msg || nullptr == msg->Get() || msg->Get()->uiNumRows == 0)
        return;

    SPDLOG_INFO("Messenger::LoadGMList");

    for (const auto& row : msg->Get()->rows)
    {
        if (!row[0].empty())
        {
            uint32_t lang = 0;
            storm::ParseNumber(row[1], lang);

            m_gameMasters.push_back({lang, row[0]});
        }
    }

    for (auto &loginAccount : m_set_loginAccount)
        SendGMList(loginAccount);
}

void MessengerManager::SendGMList(keyA account)
{
    CHARACTER *ch = g_pCharManager->FindPC(account);

    if (!ch)
        return;

    DESC *d = ch->GetDesc();

    // if (!d or m_BlockRelation.find(account) == m_BlockRelation.end() or m_BlockRelation[account].empty())
    if (!d)
        return;

    if (m_gameMasters.empty())
        return;

    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_GM_LIST;

    TPacketGCMessengerList list;
    std::vector<decltype(list)> v; // 128k

    for (auto &gm : m_gameMasters)
    {
        if (m_set_loginAccount.find(gm.name) != m_set_loginAccount.end())
        {
            list.connected = 1;
            list.name = gm.name;
            list.lang = gm.lang;
        }
        else
        {
            list.connected = 0;
            list.name = gm.name;
            list.lang = gm.lang;
        }
        v.emplace_back(list);
    }

    pack.list = v;
    d->Send(HEADER_GC_MESSENGER, pack);
}

void MessengerManager::RequestToAdd(CHARACTER *ch, CHARACTER *target)
{
    if (!ch->IsPC() || !target->IsPC())
        return;

    if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "상대방이 친구 추가를 받을 수 없는 상태입니다.");
        return;
    }

    if (quest::CQuestManager::instance().GetPCForce(target->GetPlayerID())->IsRunning())
        return;

    if (ch->IsGM() && !target->IsGM() && !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_ADD_PLAYER_AS_FRIEND))
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You are not allowed to have players as friends");
        return;
    }

    if (!ch->IsGM() && target->IsGM() && !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_ADD_GM_AS_FRIEND))
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You are not allowed to have game masters as friends");
        return;
    }

    uint32_t dw1 = ComputeCrc32(0, ch->GetName().data(), ch->GetName().size());
    uint32_t dw2 = ComputeCrc32(0, target->GetName().data(), target->GetName().size());

    auto buf = fmt::format("{}:{}", dw1, dw2);
    uint32_t dwComplex = ComputeCrc32(0, buf.data(), buf.size());

    m_set_requestToAdd.insert(dwComplex);

    ch->UpdateFriendshipRequestPulse();

    target->ChatPacket(CHAT_TYPE_COMMAND, "messenger_auth %s", ch->GetName());
}

void MessengerManager::RequestToAdd(keyA account, keyA companion)
{
    uint32_t dw2 = ComputeCrc32(0, companion.c_str(), companion.length());
    uint32_t dw1 = ComputeCrc32(0, account.c_str(), account.length());

    auto buf = fmt::format("{}:{}", dw1, dw2);
    uint32_t dwComplex = ComputeCrc32(0, buf.c_str(), buf.length());
    SPDLOG_INFO("RequestToAdd {}:{} = {}", companion, account, dwComplex);
    m_set_requestToAdd.insert(dwComplex);
}

void MessengerManager::AuthToAdd(MessengerManager::keyA account, MessengerManager::keyA companion, bool bDeny)
{
    uint32_t dw1 = ComputeCrc32(0, companion.c_str(), companion.length());
    uint32_t dw2 = ComputeCrc32(0, account.c_str(), account.length());

    auto buf = fmt::format("{}:{}", dw1, dw2);
    uint32_t dwComplex = ComputeCrc32(0, buf.c_str(), buf.length());
    SPDLOG_INFO("AuthToAdd {}:{} = {}", companion, account, dwComplex);

    if (m_set_requestToAdd.find(dwComplex) == m_set_requestToAdd.end())
    {
        SPDLOG_INFO("MessengerManager::AuthToAdd : request not exist {} -> {}", companion.c_str(), account.c_str());
        return;
    }

    m_set_requestToAdd.erase(dwComplex);

    if (!bDeny)
    {
        AddToList(companion, account);
        AddToList(account, companion);
    }
}

void MessengerManager::__AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    m_Relation[account].insert(companion);
    m_InverseRelation[companion].insert(account);

    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (d) { SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<메신져> %s 님을 친구로 추가하였습니다.", companion.c_str()); }

    const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(companion);
    CHARACTER *tch = g_pCharManager->FindPC(companion);

    if (op || tch) { SendLogin(account, companion); }
    else { SendLogout(account, companion); }
}

void MessengerManager::AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    if (companion.empty())
        return;

    if (m_Relation[account].find(companion) != m_Relation[account].end())
        return;

    SPDLOG_INFO("Messenger Add {} {}", account.c_str(), companion.c_str());
    DBManager::instance().Query("INSERT INTO messenger_list VALUES ('{}', '{}')",
                                account.c_str(), companion.c_str());

    __AddToList(account, companion);

    GmMessengerAddPacket p;
    p.name = account;
    p.companion = companion;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmMessengerAdd, p);
}

void MessengerManager::__RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    m_Relation[account].erase(companion);
    m_InverseRelation[companion].erase(account);

    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (d)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<메신져> %s 님을 메신저에서 삭제하였습니다.", companion.c_str());

    auto tch = g_pCharManager->FindPC(companion.c_str());
    if (tch && tch->GetDesc())
    {
        TPacketGCMessenger p;
        p.subheader = MESSENGER_SUBHEADER_GC_REMOVE_FRIEND;
        p.name = account;
        tch->GetDesc()->Send(HEADER_GC_MESSENGER, p);
    }
}

void MessengerManager::RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    if (companion.empty())
        return;

    if (!IsInList(account, companion))
        return;

    SPDLOG_TRACE("Messenger Remove {} {}", account.c_str(), companion.c_str());
    DBManager::instance().Query("DELETE FROM messenger_list WHERE account='{}' AND companion = '{}'",
                                account.c_str(), companion.c_str());

    __RemoveFromList(account, companion);

    GmMessengerDelPacket p;
    p.name = account;
    p.companion = companion;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmMessengerDel, p);
}

void MessengerManager::RemoveAllList(keyA account)
{
    auto company(m_Relation[account]);

    /* SQL Data 삭제 */
    DBManager::instance().Query("DELETE FROM messenger_list WHERE account='{}' OR companion='{}'",
                                account.c_str(), account.c_str());

    /* 내가 가지고있는 리스트 삭제 */
    for (const auto &iter : company) { this->RemoveFromList(account, iter); }

    /* 복사한 데이타 삭제 */
    for (auto iter = company.begin();
         iter != company.end();
    ) { company.erase(iter++); }

    company.clear();
}

bool MessengerManager::IsInList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    if (m_Relation.find(account) == m_Relation.end())
        return false;

    if (m_Relation[account].empty())
        return false;

    return m_Relation[account].find(companion) != m_Relation[account].end();
}

void MessengerManager::Initialize()
{
    DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadGMList, this, std::placeholders::_1),
                                    fmt::format("SELECT mName,mLang+0 FROM {}.gmlist WHERE mDisplayInFriendlist=1",
                                                gConfig.commonDb.name).c_str());
}

void MessengerManager::ReloadGameMasters()
{
    m_gameMasters.clear();

    DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadGMList, this, std::placeholders::_1),
                                    fmt::format("SELECT mName,mLang+0 FROM {}.gmlist WHERE mDisplayInFriendlist=1",
                                                gConfig.commonDb.name).c_str());
}

void MessengerManager::SendList(MessengerManager::keyA account)
{
    const auto ch = g_pCharManager->FindPC(account);

    if (!ch)
        return;

    DESC *d = ch->GetDesc();

    if (!d)
        return;

    if (m_Relation.find(account) == m_Relation.end())
        return;

    if (m_Relation[account].empty())
        return;

    TPacketGCMessenger pack;

    pack.subheader = MESSENGER_SUBHEADER_GC_LIST;

    TPacketGCMessengerList packList;

    TEMP_BUFFER buf(128 * 1024); // 128k
    std::vector<TPacketGCMessengerList> list;
    auto it = m_Relation[account].begin();
    const auto eit = m_Relation[account].end();

    while (it != eit)
    {
        if (m_set_loginAccount.find(*it) != m_set_loginAccount.end())
        {
            packList.connected = 1;
        }
        else
        {
            packList.connected = 0;
        }

        packList.name = *(it);
        list.emplace_back(packList);
        ++it;
    }

    pack.list = list;
    d->Send(HEADER_GC_MESSENGER, pack);
}

void MessengerManager::SendLogin(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (!d)
        return;

    if (!d->GetCharacter())
        return;


    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_LOGIN;
    pack.state = 1;
    pack.name = companion;
    d->Send(HEADER_GC_MESSENGER, pack);

}

void MessengerManager::SendLogout(MessengerManager::keyA account, MessengerManager::keyA companion)
{
    if (companion.empty())
        return;

    CHARACTER *ch = g_pCharManager->FindPC(account);
    DESC *d = ch ? ch->GetDesc() : nullptr;

    if (!d)
        return;

    uint8_t bLen = companion.size();

    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_LOGOUT;
    pack.state = false;
    pack.name = companion;
    d->Send(HEADER_GC_MESSENGER, pack);

}
