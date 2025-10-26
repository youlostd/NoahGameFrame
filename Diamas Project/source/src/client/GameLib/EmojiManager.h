#ifndef METIN2_CLIENT_GAMELIB_EMOJIMANAGER_H
#define METIN2_CLIENT_GAMELIB_EMOJIMANAGER_H

#include <Config.hpp>
#include <optional>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

using EmoticonMap = std::unordered_map<std::string, std::string>;

class EmojiManager : public CSingleton<EmojiManager>
{
public:
    void Register(std::string code, std::string emoji);
    std::optional<std::string> GetFilePath(const std::string &code);
private:
    EmoticonMap m_emoji;
};

#endif /* METIN2_CLIENT_GAMELIB_EMOJIMANAGER_H */
