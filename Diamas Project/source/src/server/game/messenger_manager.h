#ifndef METIN2_SERVER_GAME_MESSENGER_MANAGER_H
#define METIN2_SERVER_GAME_MESSENGER_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "db.h"
#include <set>
#include <map>
#include <vector>

class MessengerManager : public singleton<MessengerManager>
{
public:
    typedef std::string keyT;

    struct GameMaster
    {
        uint32_t lang;
        std::string name;
    };

    typedef const std::string &keyA;

public:
    void P2PLogin(keyA account);
    void P2PLogout(keyA account);

    void Login(keyA account);
    void Logout(keyA account);

    void RequestToAdd(CHARACTER *ch, CHARACTER *target);
    void RequestToAdd(keyA account, keyA companion);

    void AuthToAdd(keyA account, keyA companion, bool bDeny);

    void __AddToList(keyA account, keyA companion); // 실제 m_Relation, m_InverseRelation 수정하는 메소드
    void AddToList(keyA account, keyA companion);

    void __RemoveFromList(keyA account, keyA companion); // 실제 m_Relation, m_InverseRelation 수정하는 메소드
    void RemoveFromList(keyA account, keyA companion);

    void RemoveAllList(keyA account);
    bool IsInList(MessengerManager::keyA account, MessengerManager::keyA companion);

    void Initialize();
    void ReloadGameMasters();

private:
    void SendList(keyA account);
    void SendLogin(keyA account, keyA companion);
    void SendLogout(keyA account, keyA companion);

    void LoadList(SQLMsg *pmsg);

    void SendGMLogin(keyA account, const GameMaster &gm);
    void SendGMLogout(keyA account, const GameMaster &gm);

    void LoadGMList(SQLMsg *msg);
    void SendGMList(keyA account);

    void Destroy();

    std::set<keyT> m_set_loginAccount;
    std::map<keyT, std::set<keyT>> m_Relation;
    std::map<keyT, std::set<keyT>> m_InverseRelation;
    std::set<uint32_t> m_set_requestToAdd;

    std::vector<GameMaster> m_gameMasters;
};

#endif /* METIN2_SERVER_GAME_MESSENGER_MANAGER_H */
