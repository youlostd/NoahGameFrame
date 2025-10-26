#ifndef METIN2_CLIENT_MAIN_PYTHONCHAT_H
#define METIN2_CLIENT_MAIN_PYTHONCHAT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CPythonChat : public CSingleton<CPythonChat>
{
public:
    enum EWhisperType
    {
        WHISPER_TYPE_CHAT = 0,
        WHISPER_TYPE_NOT_EXIST = 1,
        WHISPER_TYPE_TARGET_BLOCKED = 2,
        WHISPER_TYPE_SENDER_BLOCKED = 3,
        WHISPER_TYPE_ERROR = 4,
        WHISPER_TYPE_GM = 5,
        WHISPER_TYPE_SYSTEM = 0xFF
    };

    typedef std::unordered_set<std::string> TIgnoreCharacterSet;

    CPythonChat() = default;
    ~CPythonChat() = default;

    // Ignore
    void IgnoreCharacter(const char *c_szName);
    bool IsIgnoreCharacter(const char *c_szName);

protected:
    TIgnoreCharacterSet m_IgnoreCharacterSet;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONCHAT_H */
