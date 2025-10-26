#include "config.h"
#include "xmas_event.h"
#include "desc.h"
#include "desc_manager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#include "utils.h"
#include "GBattle.h"
#include "GBufferManager.h"
#include "item.h"
#include "item_manager.h"
#include "DbCacheSocket.hpp"

#include "db.h"

namespace xmas
{
void ProcessEventFlag(const std::string &name, int prev_value, int value)
{
    if (name == "xmas_snow" || name == "xmas_boom" || name == "xmas_song" || name == "xmas_tree")
    {
        // 뿌려준다
        const DESC_MANAGER::DESC_SET &c_ref_set = DESC_MANAGER::instance().GetClientSet();

        for (auto it = c_ref_set.begin(); it != c_ref_set.end(); ++it)
        {
            CHARACTER *ch = (*it)->GetCharacter();

            if (!ch)
                continue;

            ch->ChatPacket(CHAT_TYPE_COMMAND, "%s %d", name.c_str(), value);
        }

        if (name == "xmas_boom")
        {
            if (value && !prev_value) { SpawnEventHelper(true); }
            else if (!value && prev_value) { SpawnEventHelper(false); }
        }
        else if (name == "xmas_tree")
        {
            if (value > 0 && prev_value == 0)
            {
                const auto chars = g_pCharManager->GetCharactersByRaceNum(MOB_XMAS_TREE_VNUM);
                if (chars.empty())
                    g_pCharManager->SpawnMob(MOB_XMAS_TREE_VNUM, 61, 76500 + 358400, 60900 + 153600, 0, false, -1);
            }
            else if (prev_value > 0 && value == 0)
            {
                const auto chars = g_pCharManager->GetCharactersByRaceNum(MOB_XMAS_TREE_VNUM);
                for (auto ch : chars)
                    M2_DESTROY_CHARACTER(ch);
            }
        }
    }
    else if (name == "xmas_santa")
    {
        switch (value)
        {
        case 0: {
            const auto chars = g_pCharManager->GetCharactersByRaceNum(MOB_SANTA_VNUM);
            for (auto ch : chars)
                M2_DESTROY_CHARACTER(ch);
            break;
        }

        case 1:
            if (gConfig.IsHostingMap(61))
            {
                quest::CQuestManager::instance().RequestSetEventFlag("xmas_santa", 2);
                g_pCharManager->SpawnMobRandomPosition(MOB_SANTA_VNUM, 61);
            }
            break;

        case 2:
            break;
        }
    }
}

EVENTINFO(spawn_santa_info)
{
    long lMapIndex;

    spawn_santa_info()
        : lMapIndex(0)
    {
    }
};

EVENTFUNC(spawn_santa_event)
{
    spawn_santa_info *info = static_cast<spawn_santa_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("spawn_santa_event> <Factor> Null pointer");
        return 0;
    }

    long lMapIndex = info->lMapIndex;

    if (quest::CQuestManager::instance().GetEventFlag("xmas_santa") == 0)
        return 0;

    const auto chars = g_pCharManager->GetCharactersByRaceNum(MOB_SANTA_VNUM);
    if (!chars.empty())
        return 0;

    if (g_pCharManager->SpawnMobRandomPosition(xmas::MOB_SANTA_VNUM, lMapIndex))
    {
        SPDLOG_TRACE("santa comes to town!");
        return 0;
    }

    return THECORE_SECS_TO_PASSES(5);
}

void SpawnSanta(long lMapIndex, int iTimeGapSec)
{
    if (gConfig.testServer) { iTimeGapSec /= 60; }

    SPDLOG_TRACE("santa respawn time = %d", iTimeGapSec);
    spawn_santa_info *info = AllocEventInfo<spawn_santa_info>();

    info->lMapIndex = lMapIndex;

    event_create(spawn_santa_event, info, THECORE_SECS_TO_PASSES(iTimeGapSec));
}

void SpawnEventHelper(bool spawn)
{
    if (spawn == true)
    {
        // 없으면 만들어준다
        struct SNPCSellFireworkPosition
        {
            long lMapIndex;
            int x;
            int y;
        } positions[] = {
                {1, 615, 618},
                {3, 500, 625},
                {21, 598, 665},
                {23, 476, 360},
                {41, 318, 629},
                {43, 478, 375},
                {0, 0, 0},
            };

        SNPCSellFireworkPosition *p = positions;
        while (p->lMapIndex)
        {
            if (gConfig.IsHostingMap(p->lMapIndex))
            {
                g_pCharManager->SpawnMob(
                    MOB_XMAS_FIRWORK_SELLER_VNUM, p->lMapIndex, p->x * 100, p->y * 100, 0, false, -1);
            }
            p++;
        }
    }
    else
    {
        const auto chars = g_pCharManager->GetCharactersByRaceNum(MOB_XMAS_FIRWORK_SELLER_VNUM);
        for (auto ch : chars)
            M2_DESTROY_CHARACTER(ch);
    }
}

bool IsSeason() { return quest::CQuestManager::instance().GetEventFlag("xmas_event_2014") == 1; }

bool IsSnowmanForPlayer(CHARACTER *ch, uint32_t raceNum)
{
    if (raceNum != 20024) //20024 = Harang
        return false;

    return (ch && ch->IsPC() && ch->GetQuestFlag("xmas_event_2014_snowman.harang_transform") == 1);
}
}
