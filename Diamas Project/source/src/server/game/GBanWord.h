#ifndef METIN2_SERVER_GAME_BANWORD_H
#define METIN2_SERVER_GAME_BANWORD_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <game/DbPackets.hpp>
#include <base/Singleton.hpp>

class CBanwordManager : public singleton<CBanwordManager>
{
public:
    bool Initialize(TBanwordTable * p, uint16_t wSize);
    bool Find(const char * c_pszString);
    bool CheckString(const char * c_pszString, uint32_t _len);
    bool CheckString(const std::string & string);
    void ConvertString(char * c_pszString, uint32_t _len);
    void ConvertString(std::string& string);

protected:
    typedef std::unordered_map<std::string, bool> TBanwordHashmap;
    TBanwordHashmap m_hashmap_words;
};

#endif /* METIN2_SERVER_GAME_BANWORD_H */
