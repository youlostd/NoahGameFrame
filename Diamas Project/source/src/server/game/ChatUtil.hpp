#ifndef METIN2_SERVER_GAME_CHATUTIL_HPP
#define METIN2_SERVER_GAME_CHATUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "char.h"
#include "desc.h"
#include "CharUtil.hpp"
#include "desc_manager.h"

#include <algorithm>


void ConvertEmpireText(uint32_t dwEmpireID, char *szText, size_t len, int iPct);


template <typename ...Args>
void SendI18nNoticeMap(const char *c_pszBuf, int nMapIndex, bool bBigFont, Args ... args)
{
    const DESC_MANAGER::DESC_SET &c_ref_set = DESC_MANAGER::instance().GetClientSet();
    std::for_each(c_ref_set.begin(), c_ref_set.end(), [&, args...](DescPtr d)
    {
        if (d->GetCharacter() == nullptr)
            return;
        if (d->GetCharacter()->GetMapIndex() != nMapIndex)
            return;

        SendI18nChatPacket(d->GetCharacter(), bBigFont == true ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, c_pszBuf,
                           args...);
    });
}

template <typename ...Args>
void SendNoticeMap(const char *c_pszBuf, int nMapIndex, bool bBigFont, Args ... args)
{
    const DESC_MANAGER::DESC_SET &c_ref_set = DESC_MANAGER::instance().GetClientSet();
    std::for_each(c_ref_set.begin(), c_ref_set.end(), [&, args...](DescPtr d)
    {
        if (d->GetCharacter() == nullptr)
            return;
        if (d->GetCharacter()->GetMapIndex() != nMapIndex)
            return;

        d->GetCharacter()->ChatPacket(bBigFont == true ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, c_pszBuf, args...);
    });
}

template <typename ...Args>
void SendNotice(const char *c_pszBuf, Args ... args)
{
    const auto &descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&, args...](DescPtr d)
    {
        if (!d->GetCharacter())
            return;

        d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, c_pszBuf, args...);
    });
}

template <typename ...Args>
void SendCommandChat(const char *c_pszBuf, Args ... args)
{
    const auto &descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&, args...](DescPtr d)
    {
        if (!d->GetCharacter())
            return;

        d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, c_pszBuf, args...);
    });
}

void SendNoticeSpecial(const char *c_pszBuf);
void SendBigOXNoticeSpecial(const char *c_pszBuf);

template <typename ...Args>
void SendI18nNotice(const char *c_pszBuf, Args ... args)
{
    const auto &descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&, args...](DescPtr d)
    {
        if (!d->GetCharacter())
            return;

        SendI18nChatPacket(d->GetCharacter(), CHAT_TYPE_NOTICE, c_pszBuf, args...);
    });
}
void SendWhisperPacket(CHARACTER* target, uint8_t bType, std::string msg, uint8_t lc, uint32_t sourcePid, std::string sourceName);
void SendWhisperAll(const char *c_pszBuf);
void SendBigNotice(const char *c_pszBuf, int mapIndex);
void SendLog(const char *c_pszBuf);
void SendShout(const char *message, uint8_t empire, uint8_t lang);
void BroadcastShout(const char *message, uint8_t empire, uint8_t lang);
void BroadcastNotice(const char *c_pszBuf);
void BroadcastNoticeSpecial(const char *c_pszBuf);
void BroadcastWhisperAll(const char *c_pszBuf);
void BroadcastBigNotice(const char *c_pszBuf, int mapIndex);
#endif /* METIN2_SERVER_GAME_CHATUTIL_HPP */
