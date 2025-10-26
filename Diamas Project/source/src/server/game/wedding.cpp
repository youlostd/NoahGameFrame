#include "DbCacheSocket.hpp"
#include "desc_manager.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "config.h"
#include "char.h"
#include "wedding.h"
#include "regen.h"

namespace marriage
{
EVENTINFO(wedding_map_info)
{
    WeddingMap *pWeddingMap;
    int iStep;

    wedding_map_info()
        : pWeddingMap(nullptr)
          , iStep(0)
    {
    }
};

EVENTFUNC(wedding_end_event)
{
    auto info = static_cast<wedding_map_info *>(event->info);
    if (!info)
        return 0;

    auto pMap = info->pWeddingMap;

    if (info->iStep == 0)
    {
        ++info->iStep;
        pMap->WarpAll();
        return THECORE_SECS_TO_PASSES(15);
    }

    WeddingManager::instance().DestroyWeddingMap(pMap);

    return 0;
}

// Map instance
WeddingMap::WeddingMap(uint32_t dwMapIndex, uint32_t dwPID1, uint32_t dwPID2)
    : m_dwMapIndex(dwMapIndex),
      m_pEndEvent(nullptr),
      m_isDark(false),
      m_isSnow(false),
      m_isMusic(false),
      dwPID1(dwPID1),
      dwPID2(dwPID2)
{
}

WeddingMap::~WeddingMap() { event_cancel(&m_pEndEvent); }

void WeddingMap::SetEnded()
{
    if (m_pEndEvent)
        return;

    auto info = AllocEventInfo<wedding_map_info>();

    info->pWeddingMap = this;

    m_pEndEvent = event_create(wedding_end_event, info, THECORE_SECS_TO_PASSES(5));

    Notice("WEDDING_ENDING_SOON");
    Notice("WEDDING_ENDING_LEAVE");
}

void WeddingMap::Notice(const char *psz)
{
    for_each(m_set_pkChr.begin(), m_set_pkChr.end(), [psz](CHARACTER *ch)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE, psz);
    });
}

void WeddingMap::WarpAll()
{
    for_each(m_set_pkChr.begin(), m_set_pkChr.end(), [](CHARACTER *ch)
    {
        if (ch->IsPC())
        {
            // ExitToSavedLocation calls WarpSet. In this function
            // Sectree is NULL Later on from SectreeManager
            // This character could not be found, so handled separately in DestroyAll below
            ch->ExitToSavedLocation();
        }
    });
}

void WeddingMap::DestroyAll()
{
    for (auto &ch : m_set_pkChr)
    {
        if (ch->IsToggleMount())
            continue;

        if (auto desc = ch->GetDesc(); desc)
            DESC_MANAGER::instance().DestroyDesc(desc);
        else
            M2_DESTROY_CHARACTER(ch);
    }
}

void WeddingMap::IncMember(CHARACTER *ch)
{
    if (IsMember(ch))
        return;

    m_set_pkChr.insert(ch);

    SendLocalEvent(ch);
}

void WeddingMap::DecMember(CHARACTER *ch)
{
    if (!IsMember(ch))
        return;

    m_set_pkChr.erase(ch);
}

bool WeddingMap::IsMember(CHARACTER *ch)
{
    if (m_set_pkChr.empty())
        return false;

    return m_set_pkChr.find(ch) != m_set_pkChr.end();
}

void WeddingMap::ShoutInMap(uint8_t type, const char *msg)
{
    for (auto it = m_set_pkChr.begin(); it != m_set_pkChr.end(); ++it)
    {
        CHARACTER *ch = *it;
        ch->ChatPacket(CHAT_TYPE_COMMAND, msg);
    }
}

void WeddingMap::SetMusic(bool bSet, const char *musicFileName)
{
    if (m_isMusic != bSet)
    {
        m_isMusic = bSet;
        m_stMusicFileName = musicFileName;

        char szCommand[256];
        if (m_isMusic)
        {
            ShoutInMap(CHAT_TYPE_COMMAND,
                       __BuildCommandPlayMusic(szCommand, sizeof(szCommand), 1, m_stMusicFileName.c_str()));
        }
        else { ShoutInMap(CHAT_TYPE_COMMAND, __BuildCommandPlayMusic(szCommand, sizeof(szCommand), 0, "default")); }
    }
}

void WeddingMap::SetDark(bool bSet)
{
    if (m_isDark != bSet)
    {
        m_isDark = bSet;

        if (m_isDark)
            ShoutInMap(CHAT_TYPE_COMMAND, "DayMode dark");
        else
            ShoutInMap(CHAT_TYPE_COMMAND, "DayMode light");
    }
}

void WeddingMap::SetSnow(bool bSet)
{
    if (m_isSnow != bSet)
    {
        m_isSnow = bSet;

        if (m_isSnow)
            ShoutInMap(CHAT_TYPE_COMMAND, "xmas_snow 1");
        else
            ShoutInMap(CHAT_TYPE_COMMAND, "xmas_snow 0");
    }
}

bool WeddingMap::IsPlayingMusic() const { return m_isMusic; }

void WeddingMap::SendLocalEvent(CHARACTER *ch)
{
    char szCommand[256];

    if (m_isDark)
        ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode dark");
    if (m_isSnow)
        ch->ChatPacket(CHAT_TYPE_COMMAND, "xmas_snow 1");
    if (m_isMusic)
        ch->ChatPacket(CHAT_TYPE_COMMAND,
                       __BuildCommandPlayMusic(szCommand, sizeof(szCommand), 1, m_stMusicFileName.c_str()));
}

const char *WeddingMap::__BuildCommandPlayMusic(char *szCommand, size_t nCmdLen, uint8_t bSet,
                                                const char *c_szMusicFileName)
{
    if (nCmdLen < 1)
    {
        szCommand[0] = '\0';
        return "PlayMusic 0 CommandLengthError";
    }

    std::snprintf(szCommand, nCmdLen, "PlayMusic %d %s", bSet, c_szMusicFileName);
    return szCommand;
}

bool WeddingManager::IsWeddingMap(uint32_t dwMapIndex)
{
    return (dwMapIndex == WEDDING_MAP_INDEX || dwMapIndex / 10000 == WEDDING_MAP_INDEX);
}

std::optional<WeddingMapPtr> WeddingManager::Find(uint32_t dwMapIndex)
{
    const auto it = m_mapWedding.find(dwMapIndex);
    if (it == m_mapWedding.end())
        return std::nullopt;

    return it->second;
}

uint32_t WeddingManager::__CreateWeddingMap(uint32_t dwPID1, uint32_t dwPID2)
{
    SECTREE_MANAGER &rkSecTreeMgr = SECTREE_MANAGER::instance();

    uint32_t dwMapIndex = rkSecTreeMgr.CreatePrivateMap(WEDDING_MAP_INDEX);
    if (!dwMapIndex)
    {
        SPDLOG_ERROR("CreateWeddingMap(pid1=%d, pid2=%d) / CreatePrivateMap(%d) FAILED", dwPID1, dwPID2,
                     WEDDING_MAP_INDEX);
        return 0;
    }

    m_mapWedding.emplace(dwMapIndex, std::make_shared<WeddingMap>(dwMapIndex, dwPID1, dwPID2));

    const char *filename = "data/map/metin2_map_wedding_01/npc.txt";
    if (!regen_do(filename, dwMapIndex, nullptr, true))
    {
        SPDLOG_ERROR("CreateWeddingMap(pid1={}, pid2={}) / regen_do(fileName={}, mapIndex={}) FAILED",
                     dwPID1, dwPID2,
                     filename,
                     dwMapIndex);
    }

    return dwMapIndex;
}

void WeddingManager::DestroyWeddingMap(WeddingMap *pMap)
{
    SECTREE_MANAGER::instance().for_each(pMap->GetMapIndex(), [](CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            auto tch = reinterpret_cast<CHARACTER *>(ent);
            if (tch && tch->GetDesc()) { tch->GoHome(); }
        }
    });

    pMap->DestroyAll();
    SECTREE_MANAGER::instance().DestroyPrivateMap(pMap->GetMapIndex());
    auto it = m_mapWedding.find(pMap->GetMapIndex());
    if (it != m_mapWedding.end())
    {
        it->second.reset();
        m_mapWedding.erase(it);
    }
}

bool WeddingManager::End(uint32_t dwMapIndex)
{
    auto it = m_mapWedding.find(dwMapIndex);

    if (it == m_mapWedding.end())
        return false;

    it->second->SetEnded();
    return true;
}

void WeddingManager::Request(uint32_t dwPID1, uint32_t dwPID2)
{
    if (gConfig.IsHostingMap(WEDDING_MAP_INDEX))
    {
        uint32_t dwMapIndex = __CreateWeddingMap(dwPID1, dwPID2);

        if (!dwMapIndex)
        {
            SPDLOG_ERROR("cannot create wedding map for {}, {}", dwPID1, dwPID2);
            return;
        }

        TPacketWeddingReady p;
        p.dwPID1 = dwPID1;
        p.dwPID2 = dwPID2;
        p.dwMapIndex = dwMapIndex;

        db_clientdesc->DBPacket(HEADER_GD_WEDDING_READY, 0, &p, sizeof(p));
    }
}
}
