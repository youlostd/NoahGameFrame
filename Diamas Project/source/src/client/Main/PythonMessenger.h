#ifndef METIN2_CLIENT_MAIN_PYTHONMESSENGER_H
#define METIN2_CLIENT_MAIN_PYTHONMESSENGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CPythonMessenger : public CSingleton<CPythonMessenger>
{
public:
    typedef std::set<std::string> TFriendNameMap;
#ifdef ENABLE_MESSENGER_BLOCK
		typedef std::set<std::string> TBlockNameMap;
#endif
    typedef std::map<std::string, uint8_t> TGuildMemberStateMap;
    typedef std::set<std::string> TGamemasterNameMap;

    enum EMessengerGroupIndex
    {
        MESSENGER_GRUOP_INDEX_FRIEND,
        MESSENGER_GRUOP_INDEX_GUILD,
        MESSENGER_GRUOP_INDEX_GM,
    };

public:
    CPythonMessenger();
    virtual ~CPythonMessenger();

    void Destroy();

    // Friend
    void RemoveFriend(const char *c_szKey);
    void OnFriendLogin(const char *c_szKey);
    void OnFriendLogout(const char *c_szKey);

    void SetMobile(const char *c_szKey, uint8_t byState);
    bool IsFriendByKey(const char *c_szKey);
    bool IsFriendByName(const char *c_szName);

    // Team
    bool IsTeamByName(const char *c_szName);
    void OnGamemasterLogin(const char *c_szKey, uint32_t lang);
    void OnGamemasterLogout(const char *c_szKey, uint32_t lang);

    // Guild
    void AppendGuildMember(const char *c_szName);
    void RemoveGuildMember(const char *c_szName);
    void RemoveAllGuildMember();
    void LoginGuildMember(const char *c_szName);
    void LogoutGuildMember(const char *c_szName);
    void RefreshGuildMember();

    void SetMessengerHandler(PyObject *poHandler);

    // Block
    void AppendBlock(const BlockedPC &pc);
    void RemoveBlock(const std::string &pc);
protected:
    TFriendNameMap m_FriendNameMap;
    TGuildMemberStateMap m_GuildMemberStateMap;
    TGamemasterNameMap m_GamemasterNameMap;
    std::unordered_map<std::string, uint8_t> m_blocked;
private:
    PyObject *m_poMessengerHandler;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONMESSENGER_H */
