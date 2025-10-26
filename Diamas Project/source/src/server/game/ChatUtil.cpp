#include "ChatUtil.hpp"

#include <game/MasterPackets.hpp>

void SendWhisperAll(const char* c_pszBuf)
{
    TPacketGCWhisper pack{};
    pack.bType = WHISPER_TYPE_SYSTEM;
    pack.szNameFrom = "<[ Information ]>";
    pack.message = c_pszBuf;
    BroadcastPacket(DESC_MANAGER::instance().GetClientSet(), HEADER_GC_WHISPER,
                    pack);
}

void SendWhisperPacket(CHARACTER* target, uint8_t bType, std::string msg,
                       uint8_t lc, uint32_t sourcePid, std::string sourceName)
{
    TPacketGCWhisper pack;
    pack.bType = bType;
    pack.pid = sourcePid;
    pack.localeCode = lc;
    pack.szNameFrom = sourceName;
    pack.message = msg;
    target->GetDesc()->Send(HEADER_GC_WHISPER, pack);
}

void SendBigNotice(const char* c_pszBuf, int mapIndex)
{
    const auto& descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&](DescPtr d) {
        CHARACTER* ch = d->GetCharacter();
        if (!ch)
            return;

        if (mapIndex == -1 || ch->GetMapIndex() == mapIndex)
            SendSpecialI18nChatPacket(ch, CHAT_TYPE_BIG_NOTICE, c_pszBuf);
    });
}

void SendLog(const char* c_pszBuf)
{
    const auto& descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&](DescPtr d) {
        if (!d->GetCharacter())
            return;

        if (d->GetCharacter()->GetGMLevel() > GM_PLAYER)
            d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", c_pszBuf);
    });
}

void SendShout(const char* message, uint8_t empire, uint8_t lang)
{
    const auto& clients = DESC_MANAGER::instance().GetClientSet();
    std::for_each(clients.begin(), clients.end(), [&](const DescPtr& d) {
        if (!d->GetCharacter())
            return;
        if (d->GetCharacter()->IsChatFilter(lang))
            return;
        d->GetCharacter()->ChatPacket(CHAT_TYPE_SHOUT, "%s", message);
    });
}

void BroadcastShout(const char* message, uint8_t empire, uint8_t lang)
{
    GmShoutPacket p;
    p.empire = empire;
    p.message = message;
    p.lang = lang;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmShout, p);

    SendShout(message, empire, lang);
}

void BroadcastNotice(const char* message)
{
    GmNoticePacket p;
    p.message = message;
    p.type = NOTICE_TYPE_DEFAULT;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmNotice, p);

    SendNotice(message);
}

void BroadcastNoticeSpecial(const char* c_pszBuf)
{
    GmNoticePacket p;
    p.message = c_pszBuf;
    p.type = NOTICE_TYPE_FORMATTED;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmNotice, p);

    SendNoticeSpecial(c_pszBuf);
}

void BroadcastWhisperAll(const char* c_pszBuf)
{
    GmNoticePacket p;
    p.message = c_pszBuf;
    p.type = NOTICE_TYPE_WHISPER;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmNotice, p);

    SendWhisperAll(c_pszBuf);
}

void ConvertEmpireText(uint32_t dwEmpireID, char* szText, size_t len, int iPct)
{
    char alphanum[] = "0123456789"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz";

    if (dwEmpireID < 1 || dwEmpireID > 3 || len == 0)
        return;

    if (iPct >= 100)
        return;

    for (size_t i = 0; i < len && szText[i] != '\0'; ++i) {
        if (Random::get(1, 100) > iPct && szText[i] != ' ')
            szText[i] = alphanum[rand() % sizeof(alphanum) - 1];
    }
}

void SendNoticeSpecial(const char* c_pszBuf)
{
    const auto& descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&](DescPtr d) {
        if (!d->GetCharacter())
            return;
        SendSpecialI18nChatPacket(d->GetCharacter(), CHAT_TYPE_NOTICE,
                                  c_pszBuf);
    });
}

void SendBigOXNoticeSpecial(const char* c_pszBuf)
{
    const auto& descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&](DescPtr d) {
        if (!d->GetCharacter())
            return;

        if (d->GetCharacter()->GetMapIndex() == 113)
            SendSpecialI18nOXPacket(d->GetCharacter(), CHAT_TYPE_BIG_NOTICE,
                                    c_pszBuf);
    });
}

// If mapIndex == -1 <-> all maps.
// Otherwise, just that map.
void BroadcastBigNotice(const char* c_pszBuf, int mapIndex)
{
    if (mapIndex == -1) // Send the notice through map & cores
    {
        GmNoticePacket p;
        p.message = c_pszBuf;
        p.type = NOTICE_TYPE_BIG;
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmNotice, p);
    }

    SendBigNotice(c_pszBuf, mapIndex);
}
