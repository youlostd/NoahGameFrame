#include "StdAfx.h"
#include "EmojiManager.h"

void EmojiManager::Register(std::string code, std::string emoji)
{
    m_emoji.emplace(code, emoji);
}

std::optional<std::string> EmojiManager::GetFilePath(const std::string &code)
{
    const auto it = m_emoji.find(code);
    if (it == m_emoji.end())
        return std::nullopt;

    return it->second;
}
