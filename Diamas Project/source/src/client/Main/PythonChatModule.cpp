#include "StdAfx.h"
#include "PythonChat.h"
#include "PythonItem.h"
#include "../gamelib/ItemManager.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"

PyObject *chatAppendChat(PyObject *poSelf, PyObject *poArgs)
{
    int iType;
    if (!PyTuple_GetInteger(poArgs, 0, &iType))
        return Py_BadArgument();

    std::string szChat;
    if (!PyTuple_GetString(poArgs, 1, szChat))
        return Py_BadArgument();

    PyCallClassMemberFunc(gPythonNetworkStream->GetPhaseWindow(CPythonNetworkStream::PHASE_WINDOW_GAME),
                          "BINARY_AppendChat", iType, py::bytes(szChat));

    Py_RETURN_NONE;
}

PyObject *chatIgnoreCharacter(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BadArgument();

    CPythonChat::Instance().IgnoreCharacter(szName.c_str());

    Py_RETURN_NONE;
}

PyObject *chatIsIgnoreCharacter(PyObject *poSelf, PyObject *poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BadArgument();

    CPythonChat::Instance().IsIgnoreCharacter(szName.c_str());

    Py_RETURN_NONE;
}

PyObject *chatGetLinkFromHyperlink(PyObject *poSelf, PyObject *poArgs)
{
    std::string stHyperlink;

    if (!PyTuple_GetString(poArgs, 0, stHyperlink))
        return Py_BuildException();

    std::vector<std::string> results;

    split_string(stHyperlink, ":", results, false);

    // item:vnum:flag:socket0:socket1:socket2:socket3:socket4:socket5
    if (0 == results[0].compare("item"))
    {
        if (results.size() < 6)
            return Py_BuildValue("s", "");

        CItemData *pItemData = NULL;

        if (CItemManager::Instance().GetItemDataPointer(htoi(results[1].c_str()), &pItemData))
        {
            char buf[1024] = {0};
            char itemlink[256];
            int len;
            bool isAttr = false;

            len = snprintf(itemlink, sizeof(itemlink), "item:%x:%x:%lld:%lld:%lld:%lld:%lld:%lld",
                           htoi(results[1].c_str()),
                           htoi(results[2].c_str()),
                           atoll(results[3].c_str()),
                           atoll(results[4].c_str()),
                           atoll(results[5].c_str()),
                           atoll(results[6].c_str()),
                           atoll(results[7].c_str()),
                           atoll(results[8].c_str()));

            if (results.size() >= 8)
            {
                for (int i = 9; i < results.size(); i += 2)
                {
                    len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%lld",
                                    htoi(results[i].c_str()),
                                    atoll(results[i + 1].c_str()));
                    isAttr = true;
                }
            }

            if (isAttr)
                //"item:¹øÈ£:ÇÃ·¡±×:¼ÒÄÏ0:¼ÒÄÏ1:¼ÒÄÏ2"
                snprintf(buf, sizeof(buf), "|cffffc700|H%s|h|h|r", itemlink);
            else
                snprintf(buf, sizeof(buf), "|cfff1e6c0|H%s|h|h|r", itemlink);

            return Py_BuildValue("s", buf);
        }
    }

    return Py_BuildValue("s", "");
}

extern "C" void initchat()
{
    static PyMethodDef s_methods[] =
    {
        {"AppendChat", chatAppendChat, METH_VARARGS},
        // Ignore
        {"IgnoreCharacter", chatIgnoreCharacter, METH_VARARGS},
        {"IsIgnoreCharacter", chatIsIgnoreCharacter, METH_VARARGS},

        // Link
        {"GetLinkFromHyperlink", chatGetLinkFromHyperlink, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("chat", s_methods);

    PyModule_AddIntConstant(poModule, "CHAT_SET_CHAT_WINDOW", 0);
    PyModule_AddIntConstant(poModule, "CHAT_SET_LOG_WINDOW", 1);

    PyObject *builtins = PyImport_ImportModule("__builtin__");

    PyModule_AddIntMacro(builtins, CHAT_TYPE_TALKING);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_INFO);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_NOTICE);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_PARTY);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_GUILD);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_COMMAND);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_SHOUT);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_WHISPER);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_BIG_NOTICE);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_DICE_INFO);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_BIG_CONTROL);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_MISSION);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_SUB_MISSION);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_TEAM);
    PyModule_AddIntMacro(builtins, CHAT_TYPE_EMPIRE);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_NORMAL);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_NOT_EXIST);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_TARGET_BLOCKED);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_SENDER_BLOCKED);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_ERROR);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_GM);
    PyModule_AddIntMacro(builtins, WHISPER_TYPE_SYSTEM);

    Py_DECREF(builtins);
}
