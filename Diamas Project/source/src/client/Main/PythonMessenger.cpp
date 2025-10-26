#include "stdafx.h"
#include "PythonMessenger.h"
#include "PythonPlayer.h"

void CPythonMessenger::RemoveFriend(const char *c_szKey)
{
    m_FriendNameMap.erase(c_szKey);

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogin(const char *c_szKey/*, const char * c_szName*/)
{
    m_FriendNameMap.insert(c_szKey);

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogout(const char *c_szKey)
{
    m_FriendNameMap.insert(c_szKey);

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnGamemasterLogin(const char *c_szKey, uint32_t lang)
{
    m_GamemasterNameMap.insert(c_szKey);

    if (m_poMessengerHandler)
    {
        std::string langStr;
        GetGMLanguageFlagsString(langStr, lang);

        PyCallClassMemberFunc(m_poMessengerHandler, "OnTeamLogin",
                              Py_BuildValue("(iss)", MESSENGER_GRUOP_INDEX_GM, c_szKey, langStr.c_str()));

    }
}

void CPythonMessenger::OnGamemasterLogout(const char *c_szKey, uint32_t lang)
{
    m_GamemasterNameMap.insert(c_szKey);

    if (m_poMessengerHandler)
    {
        std::string langStr;
        GetGMLanguageFlagsString(langStr, lang);
        PyCallClassMemberFunc(m_poMessengerHandler, "OnTeamLogout",
                              Py_BuildValue("(iss)", MESSENGER_GRUOP_INDEX_GM, c_szKey, langStr.c_str()));
    }
}

void CPythonMessenger::SetMobile(const char *c_szKey, uint8_t byState)
{
    m_FriendNameMap.insert(c_szKey);

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnMobile",
                              Py_BuildValue("(isi)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey, byState));
}

bool CPythonMessenger::IsFriendByKey(const char *c_szKey)
{
    return m_FriendNameMap.end() != m_FriendNameMap.find(c_szKey);
}

bool CPythonMessenger::IsFriendByName(const char *c_szName)
{
    return IsFriendByKey(c_szName);
}

bool CPythonMessenger::IsTeamByName(const char *c_szName)
{
    return m_GamemasterNameMap.end() != m_GamemasterNameMap.find(c_szName);
}

void CPythonMessenger::AppendGuildMember(const char *c_szName)
{
    if (m_GuildMemberStateMap.end() != m_GuildMemberStateMap.find(c_szName))
        return;

    LogoutGuildMember(c_szName);
}

void CPythonMessenger::RemoveGuildMember(const char *c_szName)
{
    m_GuildMemberStateMap.erase(c_szName);

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RemoveAllGuildMember()
{
    m_GuildMemberStateMap.clear();

    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveAllList",
                              Py_BuildValue("(i)", MESSENGER_GRUOP_INDEX_GUILD));
}

void CPythonMessenger::LoginGuildMember(const char *c_szName)
{
    m_GuildMemberStateMap[c_szName] = 1;
    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::LogoutGuildMember(const char *c_szName)
{
    m_GuildMemberStateMap[c_szName] = 0;
    if (m_poMessengerHandler)
        PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout",
                              Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RefreshGuildMember()
{
    for (TGuildMemberStateMap::iterator itor = m_GuildMemberStateMap.begin(); itor != m_GuildMemberStateMap.end(); ++
         itor)
    {
        if (itor->second)
            PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin",
                                  Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor->first).c_str()));
        else
            PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout",
                                  Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor->first).c_str()));
    }
}

void CPythonMessenger::Destroy()
{
    m_FriendNameMap.clear();
    m_GuildMemberStateMap.clear();
    m_GamemasterNameMap.clear();
}

void CPythonMessenger::SetMessengerHandler(PyObject *poHandler)
{
    m_poMessengerHandler = poHandler;
}

void CPythonMessenger::AppendBlock(const BlockedPC &pc)
{
    auto it = m_blocked.find(pc.name);
    if (it != m_blocked.end())
        it->second = pc.mode;
    else
        m_blocked.insert(std::make_pair(pc.name, pc.mode));

    PyCallClassMemberFunc(m_poMessengerHandler, "OnAppendBlock",
                          Py_BuildValue("(si)", pc.name, pc.mode));
}

void CPythonMessenger::RemoveBlock(const std::string &name)
{
    const auto it = m_blocked.find(name);
    if (it == m_blocked.end())
        return;

    PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveBlock",
                          Py_BuildValue("(s)", it->first.c_str()));

    m_blocked.erase(it);
}

CPythonMessenger::CPythonMessenger()
    : m_poMessengerHandler(NULL)
{
}

CPythonMessenger::~CPythonMessenger()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *messengerRemoveFriend(PyObject *poSelf, PyObject *poArgs)
{
    std::string szKey;
    if (!PyTuple_GetString(poArgs, 0, szKey))
        return Py_BuildException();

    CPythonMessenger::Instance().RemoveFriend(szKey.c_str());
    Py_RETURN_NONE;
}

PyObject *messengerIsFriendByName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonMessenger::Instance().IsFriendByName(szName.c_str()));
}

PyObject *messengerIsTeamByName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonMessenger::Instance().IsTeamByName(szName.c_str()));
}

PyObject *messengerDestroy(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMessenger::Instance().Destroy();
    Py_RETURN_NONE;
}

PyObject *messengerRefreshGuildMember(PyObject *poSelf, PyObject *poArgs)
{
    CPythonMessenger::Instance().RefreshGuildMember();
    Py_RETURN_NONE;
}

PyObject *messengerSetMessengerHandler(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poEventHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
        return Py_BuildException();

    CPythonMessenger::Instance().SetMessengerHandler(poEventHandler);
    Py_RETURN_NONE;
}

PyObject *messengerRemoveBlock(PyObject *poSelf, PyObject *poArgs)
{
    std::string name;
    if (!PyTuple_GetString(poArgs, 0, name))
        return Py_BadArgument();

    CPythonMessenger::Instance().RemoveBlock(name);
    Py_RETURN_NONE;
}

extern "C" void initmessenger()
{
    static PyMethodDef s_methods[] =
    {
        {"RemoveFriend", messengerRemoveFriend, METH_VARARGS},
        {"IsFriendByName", messengerIsFriendByName, METH_VARARGS},
        {"IsTeamByName", messengerIsTeamByName, METH_VARARGS},
        {"Destroy", messengerDestroy, METH_VARARGS},
        {"RefreshGuildMember", messengerRefreshGuildMember, METH_VARARGS},
        {"SetMessengerHandler", messengerSetMessengerHandler, METH_VARARGS},
        {"RemoveBlock", messengerRemoveBlock, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    Py_InitModule("messenger", s_methods);
}
