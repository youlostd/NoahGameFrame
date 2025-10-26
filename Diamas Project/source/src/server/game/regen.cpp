#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "regen.h"
#include "mob_manager.h"
#include "dungeon.h"
#include <thecore/utils.hpp>
#include "ChatUtil.hpp"

namespace
{
bool CheckRegen(std::string filename, uint32_t line, const regen &r)
{
    switch (r.type)
    {
    case REGEN_TYPE_MOB:
    case REGEN_TYPE_ANYWHERE: {
        if (CMobManager::instance().Get(r.vnum))
            break;

        SPDLOG_ERROR("{0}:{1}: Mob {2} is invalid", filename.c_str(), line, r.vnum);
        return false;
    }

    case REGEN_TYPE_GROUP: {
        if (CMobManager::instance().GetGroup(r.vnum))
            break;

        SPDLOG_ERROR("{0}:{1}: Group {2} is invalid", filename.c_str(), line, r.vnum);
        return false;
    }

    case REGEN_TYPE_GROUP_GROUP: {
        if (CMobManager::instance().GetGroupFromGroupGroup(r.vnum))
            break;

        SPDLOG_ERROR("{0}:{1}: Group Group {2} is invalid", filename.c_str(), line, r.vnum);
        return false;
    }
    }

    const auto map = SECTREE_MANAGER::instance().GetMap(r.lMapIndex);
    if (!map)
    {
        SPDLOG_ERROR("{0}:{1}: Map {2} is invalid", filename.c_str(), line, r.lMapIndex);
        return false;
    }

    if (!map->IsRegionValid(r.sx, r.sy, r.ex, r.ey))
    {
        SPDLOG_WARN(
            "{0}:{1}: Spawn area includes invalid positions",
            filename.c_str(), line);
        return false;
    }

    if (map->HasAttrInRegion(r.sx, r.sy, r.ex, r.ey, ATTR_BANPK | ATTR_BLOCK))
    {
        SPDLOG_INFO(
            "{0}:{1}: Spawn area includes positions marked as BANPK/BLOCK",
            filename.c_str(), line);
    }

    return true;
}
}

std::list<std::unique_ptr<regen>> regen_list;
std::list<std::unique_ptr<regen_exception>> regen_exception_list;

typedef struct SMapDataContainer
{
    char szBaseName[256];
} TMapDataContainer;

using mbMapDataCType = std::map<uint32_t, TMapDataContainer *>;
mbMapDataCType mbMapDataContainer;

enum ERegenModes
{
    MODE_TYPE = 0,
    MODE_SX,
    MODE_SY,
    MODE_EX,
    MODE_EY,
    MODE_Z_SECTION,
    MODE_DIRECTION,
    MODE_REGEN_TIME,
    MODE_REGEN_PERCENT,
    MODE_MAX_COUNT,
    MODE_VNUM
};

static bool get_word(FILE *fp, char *buf) // ¿öµå´ÜÀ§·Î ¹Þ´Â´Ù.
{
    int i = 0;
    int c;

    int semicolon_mode = 0;

    while ((c = fgetc(fp)) != EOF)
    {
        if (i == 0)
        {
            if (c == '"')
            {
                semicolon_mode = 1;
                continue;
            }

            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                continue;
        }

        if (semicolon_mode)
        {
            if (c == '"')
            {
                buf[i] = '\0';
                return true;
            }

            buf[i++] = c;
        }
        else
        {
            if ((c == ' ' || c == '\t' || c == '\n' || c == '\r'))
            {
                buf[i] = '\0';
                return true;
            }

            buf[i++] = c;
        }

        if (i == 2 && buf[0] == '/' && buf[1] == '/')
        {
            buf[i] = '\0';
            return true;
        }
    }

    buf[i] = '\0';
    return (i != 0);
}

static void next_line(FILE *fp)
{
    int c;

    while ((c = fgetc(fp)) != EOF)
        if (c == '\n')
            return;
}

static bool read_line(std::string filename, uint32_t &line,
                      FILE *fp, regen *regen)
{
    char szTmp[256];

    int mode = MODE_TYPE;
    int tmpTime;
    uint32_t i;

    while (get_word(fp, szTmp))
    {
        if (!strncmp(szTmp, "//", 2))
        {
            next_line(fp);
            ++line;
            continue;
        }

        switch (mode)
        {
        case MODE_TYPE:
            if (szTmp[0] == 'm')
                regen->type = REGEN_TYPE_MOB;
            else if (szTmp[0] == 'g')
            {
                regen->type = REGEN_TYPE_GROUP;

                if (szTmp[1] == 'a')
                    regen->is_aggressive = true;
            }
            else if (szTmp[0] == 'e')
                regen->type = REGEN_TYPE_EXCEPTION;
            else if (szTmp[0] == 'r')
                regen->type = REGEN_TYPE_GROUP_GROUP;
            else if (szTmp[0] == 's')
                regen->type = REGEN_TYPE_ANYWHERE;
            else
            {
                SPDLOG_ERROR("%s:%d: unknown regen type %s",
                             filename.c_str(), line, szTmp[0]);
                return false;
            }

            ++mode;
            break;

        case MODE_SX:
            str_to_number(regen->sx, szTmp);
            ++mode;
            break;

        case MODE_SY:
            str_to_number(regen->sy, szTmp);
            ++mode;
            break;

        case MODE_EX: {
            int iX = 0;
            str_to_number(iX, szTmp);

            regen->sx -= iX;
            regen->ex = regen->sx + iX * 2;

            regen->sx *= 100;
            regen->ex *= 100;

            ++mode;
        }
        break;

        case MODE_EY: {
            int iY = 0;
            str_to_number(iY, szTmp);

            regen->sy -= iY;
            regen->ey = regen->sy + iY * 2;

            regen->sy *= 100;
            regen->ey *= 100;

            ++mode;
        }
        break;

        case MODE_Z_SECTION:
            str_to_number(regen->z_section, szTmp);

            // 익셉션 이면 나가주자.
            if (regen->type == REGEN_TYPE_EXCEPTION)
                return true;

            ++mode;
            break;

        case MODE_DIRECTION:
            str_to_number(regen->direction, szTmp);
            ++mode;
            break;

        case MODE_REGEN_TIME:
            regen->time = 0;
            tmpTime = 0;

            for (i = 0; i < strlen(szTmp); ++i)
            {
                switch (szTmp[i])
                {
                case 'h':
                    regen->time += tmpTime * 3600;
                    tmpTime = 0;
                    break;

                case 'm':
                    regen->time += tmpTime * 60;
                    tmpTime = 0;
                    break;

                case 's':
                    regen->time += tmpTime;
                    tmpTime = 0;
                    break;

                default:
                    if (szTmp[i] >= '0' && szTmp[i] <= '9')
                    {
                        tmpTime *= 10;
                        tmpTime += (szTmp[i] - '0');
                    }
                }
            }

            ++mode;
            break;

        case MODE_REGEN_PERCENT:
            ++mode;
            break;

        case MODE_MAX_COUNT:
            regen->count = 0;
            str_to_number(regen->max_count, szTmp);
            ++mode;
            break;

        case MODE_VNUM:
            str_to_number(regen->vnum, szTmp);
            ++mode;
            return true;
        }
    }

    return false;
}

bool is_regen_exception(long x, long y)
{
    for(const auto& exc : regen_exception_list)
    {
        if (exc->sx <= x && exc->sy <= y)
            if (exc->ex >= x && exc->ey >= y)
                return true;
    }

    return false;
}

static void regen_spawn_dungeon(regen *regen, CDungeon *pDungeon, bool bOnce)
{
    uint32_t num;
    uint32_t i;

    num = (regen->max_count - regen->count);

    if (!num)
        return;

    for (i = 0; i < num; ++i)
    {
        CHARACTER *ch = nullptr;

        if (regen->type == REGEN_TYPE_ANYWHERE)
        {
            ch = g_pCharManager->SpawnMobRandomPosition(regen->vnum, regen->lMapIndex);

            if (ch)
            {
                ++regen->count;
                ch->SetDungeon(pDungeon);
            }
        }
        else if (regen->sx == regen->ex && regen->sy == regen->ey)
        {
            ch = g_pCharManager->SpawnMob(regen->vnum,
                                          regen->lMapIndex,
                                          regen->sx,
                                          regen->sy,
                                          regen->z_section,
                                          false,
                                          regen->direction == 0 ? Random::get(0, 7) * 45 : (regen->direction - 1) * 45);

            if (ch)
            {
                ++regen->count;
                ch->SetDungeon(pDungeon);
            }
        }
        else
        {
            if (regen->type == REGEN_TYPE_MOB)
            {
                ch = g_pCharManager->SpawnMobRange(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                                   regen->ey, true);

                if (ch)
                {
                    ++regen->count;
                    ch->SetDungeon(pDungeon);
                }
            }
            else if (regen->type == REGEN_TYPE_GROUP)
            {
                if (g_pCharManager->SpawnGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                               regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pDungeon))
                    ++regen->count;
            }
            else if (regen->type == REGEN_TYPE_GROUP_GROUP)
            {
                if (g_pCharManager->SpawnGroupGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                                    regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pDungeon))
                    ++regen->count;
            }
        }

        if (ch && !bOnce)
            ch->SetRegen(regen);
    }
}

static void regen_spawn(regen *regen, bool bOnce)
{
    uint32_t num;
    uint32_t i;

    num = (regen->max_count - regen->count);

    if (!num)
        return;

    for (i = 0; i < num; ++i)
    {
        CHARACTER *ch = nullptr;

        if (regen->type == REGEN_TYPE_ANYWHERE)
        {
            ch = g_pCharManager->SpawnMobRandomPosition(regen->vnum, regen->lMapIndex);

            if (ch)
                ++regen->count;
        }
        else if (regen->sx == regen->ex && regen->sy == regen->ey)
        {
            ch = g_pCharManager->SpawnMob(regen->vnum,
                                          regen->lMapIndex,
                                          regen->sx,
                                          regen->sy,
                                          regen->z_section,
                                          false,
                                          regen->direction == 0 ? Random::get(0, 7) * 45 : (regen->direction - 1) * 45);

            if (ch)
                ++regen->count;
        }
        else
        {
            if (regen->type == REGEN_TYPE_MOB)
            {
                ch = g_pCharManager->SpawnMobRange(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                                   regen->ey, regen->is_aggressive, regen->is_aggressive);
                if (ch)
                    ++regen->count;
            }
            else if (regen->type == REGEN_TYPE_GROUP)
            {
                if (g_pCharManager->SpawnGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                               regen->ey, bOnce ? nullptr : regen, regen->is_aggressive))
                    ++regen->count;
            }
            else if (regen->type == REGEN_TYPE_GROUP_GROUP)
            {
                if (g_pCharManager->SpawnGroupGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex,
                                                    regen->ey, bOnce ? nullptr : regen, regen->is_aggressive))
                    ++regen->count;
            }
        }

        if (ch && !bOnce) { ch->SetRegen(regen); }
    }
}

EVENTFUNC(dungeon_regen_event)
{
    dungeon_regen_event_info *info = static_cast<dungeon_regen_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("dungeon_regen_event> <Factor> Null pointer");
        return 0;
    }

    CDungeon *pDungeon = CDungeonManager::instance().Find<CDungeon>(info->dungeon_id);
    if (pDungeon == nullptr) { return 0; }

    regen *regen = info->regen;
    if (regen->time == 0) { regen->event = nullptr; }

    regen_spawn_dungeon(regen, pDungeon, false);
    return THECORE_SECS_TO_PASSES(regen->time);
}

bool regen_do(const char *filename, long lMapIndex, CDungeon *pDungeon, bool bOnce)
{
    if (gConfig.disableRegen)
        return true;

    if (lMapIndex >= 114 && lMapIndex <= 117)
        return true;

    regen *regen = nullptr;
    FILE *fp = fopen(filename, "rt");

    if (nullptr == fp)
    {
        SPDLOG_ERROR("SYSTEM: regen_do: %s: file not found", filename);
        return false;
    }

    for (uint32_t line = 1; ; ++line)
    {
        REGEN tmp = {};
        tmp.lMapIndex = lMapIndex;

        if (!read_line(filename, line, fp, &tmp))
            break;

        if (!CheckRegen(filename, line, tmp))
            continue;

        if (tmp.type == REGEN_TYPE_MOB ||
            tmp.type == REGEN_TYPE_GROUP ||
            tmp.type == REGEN_TYPE_GROUP_GROUP ||
            tmp.type == REGEN_TYPE_ANYWHERE)
        {
            if (!bOnce)
                regen = new REGEN(tmp);
            else
                regen = &tmp;

            if (pDungeon)
                regen->is_aggressive = true;

            regen->lMapIndex = lMapIndex;
            regen->count = 0;

            if (regen->sx > regen->ex)
                std::swap(regen->sx, regen->ex);

            if (regen->sy > regen->ey)
                std::swap(regen->sy, regen->ey);

            if (!bOnce && pDungeon != nullptr)
            {
                dungeon_regen_event_info *info = AllocEventInfo<dungeon_regen_event_info>();

                info->regen = regen;
                info->dungeon_id = pDungeon->GetId();

                regen->event = event_create(dungeon_regen_event, info,
                                            THECORE_SECS_TO_PASSES(Random::get(0, 16)) + THECORE_SECS_TO_PASSES(regen->
                                                time));

                pDungeon->AddRegen(regen);
                // regen_id should be determined at this point,
                // before the call to CHARACTER::SetRegen()
            }

            // 처음엔 무조건 리젠 해준다.
            regen_spawn_dungeon(regen, pDungeon, bOnce);
        }
    }

    fclose(fp);
    return true;
}

bool regen_load_in_file(const char *filename, long lMapIndex)
{
    if (gConfig.disableRegen)
        return true;

    regen *regen = nullptr;
    FILE *fp = fopen(filename, "rt");

    if (nullptr == fp)
    {
        SPDLOG_ERROR("SYSTEM: regen_do: %s: file not found", filename);
        return false;
    }

    for (uint32_t line = 1; ; ++line)
    {
        REGEN tmp = {};
        tmp.lMapIndex = lMapIndex;

        if (!read_line(filename, line, fp, &tmp))
            break;

        if (!CheckRegen(filename, line, tmp))
            continue;

        if (tmp.type == REGEN_TYPE_MOB ||
            tmp.type == REGEN_TYPE_GROUP ||
            tmp.type == REGEN_TYPE_GROUP_GROUP ||
            tmp.type == REGEN_TYPE_ANYWHERE)
        {
            regen = &tmp;

            regen->is_aggressive = true;

            regen->lMapIndex = lMapIndex;
            regen->count = 0;

            if (regen->sx > regen->ex)
                std::swap(regen->sx, regen->ex);

            if (regen->sy > regen->ey)
                std::swap(regen->sy, regen->ey);

            // 처음엔 무조건 리젠 해준다.
            regen_spawn(regen, true);
        }
    }

    fclose(fp);
    return true;
}

EVENTFUNC(regen_event)
{
    regen_event_info *info = static_cast<regen_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("regen_event> <Factor> Null pointer");
        return 0;
    }

    regen *regen = info->regen;
    if (!is_valid_regen(regen))
        return 0;

    if (regen->time == 0)
        regen->event = nullptr;

    regen_spawn(regen, false);
    SPDLOG_DEBUG("REGEN_SPAWN: regen_event vnum {} time {}", regen->vnum, regen->time);

    return THECORE_SECS_TO_PASSES(regen->time);
}

bool regen_load(const char *filename, long lMapIndex)
{
    if (gConfig.disableRegen)
        return true;

    regen *regen = nullptr;
    FILE *fp = fopen(filename, "rt");

    if (nullptr == fp)
    {
        SPDLOG_ERROR("SYSTEM: regen_load: {}: file not found", filename);
        return false;
    }

    for (uint32_t line = 1; ; ++line)
    {
        REGEN tmp = {};
        tmp.lMapIndex = lMapIndex;

        if (!read_line(filename, line, fp, &tmp))
            break;

        if (!CheckRegen(filename, line, tmp))
            continue;

        if (tmp.type == REGEN_TYPE_MOB ||
            tmp.type == REGEN_TYPE_GROUP ||
            tmp.type == REGEN_TYPE_GROUP_GROUP ||
            tmp.type == REGEN_TYPE_ANYWHERE)
        {
            regen = new REGEN(tmp);

            regen_list.emplace_back(regen);


            regen->count = 0;

            if (regen->sx > regen->ex)
                std::swap(regen->sx, regen->ex);

            if (regen->sy > regen->ey)
                std::swap(regen->sy, regen->ey);

            if (regen->type == REGEN_TYPE_MOB)
            {
                const auto *p = CMobManager::instance().Get(regen->vnum);
                if (p->bType == CHAR_TYPE_NPC ||
                    p->bType == CHAR_TYPE_WARP ||
                    p->bType == CHAR_TYPE_GOTO)
                {
                    SECTREE_MANAGER::instance().InsertNPCPosition(lMapIndex, p->dwVnum,
                                                                  p->bType,
                                                                  p->szLocaleName,
                                                                  (regen->sx + regen->ex) / 2,
                                                                  (regen->sy + regen->ey) / 2);
                }
            }

            //NO_REGEN
            // Desc: 	regen.txt (외 리젠관련 텍스트 ) 에서 리젠 시간을 0으로 세팅할시
            // 			리젠을 하지 안한다.
            if (regen->time != 0)
            {
                // 처음엔 무조건 리젠 해준다.
                regen_spawn(regen, false);

                regen_event_info *info = AllocEventInfo<regen_event_info>();

                info->regen = regen;

                regen->event = event_create(regen_event, info,
                                            THECORE_SECS_TO_PASSES(Random::get(0, 16)) + THECORE_SECS_TO_PASSES(regen->
                                                time));
            }
            //END_NO_REGEN
        }
        else if (tmp.type == REGEN_TYPE_EXCEPTION)
        {
            auto exc = std::make_unique<REGEN_EXCEPTION>();

            exc->sx = tmp.sx;
            exc->sy = tmp.sy;
            exc->ex = tmp.ex;
            exc->ey = tmp.ey;
            exc->z_section = tmp.z_section;
            regen_exception_list.emplace_back(std::move(exc));
        }
    }

    fclose(fp);
    return true;
}

void regen_free(void)
{
    for(const auto& regen : regen_list) {
        event_cancel(&regen->event);
    }
    regen_list.clear();
    regen_exception_list.clear();
}

void regen_reset(int x, int y)
{
    for (const auto& regen : regen_list)
    {    
        if (!regen->event)
            continue;

        // 좌표가 있으면 좌표 내에 있는 리젠 리스트만 리젠 시킨다.
        if (x != 0 || y != 0)
        {
            if (x >= regen->sx && x <= regen->ex)
                if (y >= regen->sy && y <= regen->ey)
                    event_reset_time(regen->event, 1);
        }
        // 없으면 전부 리젠
        else
            event_reset_time(regen->event, 1);
        
    }
}

bool is_valid_regen(regen *currRegen)
{
    return std::find_if(regen_list.begin(), regen_list.end(), [&](std::unique_ptr<regen>& p) { return p.get() == currRegen;}) != regen_list.end();
}

void regen_free_map(long lMapIndex)
{
    auto itr = regen_list.cbegin();
    while (itr != regen_list.cend()) {
        if((*itr)->lMapIndex == lMapIndex) {
            event_cancel(&(*itr)->event);
            itr = regen_list.erase(itr);
        } else {
            ++itr;
        }
    }
}

void regen_reload(long lMapIndex)
{
    if (mbMapDataContainer.find(lMapIndex) == mbMapDataContainer.end())
        return;

    char szFilename[256];

    std::snprintf(szFilename, sizeof(szFilename), "%sregen.txt", mbMapDataContainer[lMapIndex]->szBaseName);
    regen_load(szFilename, lMapIndex);

    std::snprintf(szFilename, sizeof(szFilename), "%snpc.txt", mbMapDataContainer[lMapIndex]->szBaseName);
    regen_load(szFilename, lMapIndex);

    std::snprintf(szFilename, sizeof(szFilename), "%sboss.txt", mbMapDataContainer[lMapIndex]->szBaseName);
    regen_load(szFilename, lMapIndex);

    std::snprintf(szFilename, sizeof(szFilename), "%sstone.txt", mbMapDataContainer[lMapIndex]->szBaseName);
    regen_load(szFilename, lMapIndex);
}

void regen_register_map(const char *szBaseName, long lMapIndex)
{
    TMapDataContainer *container = new TMapDataContainer;
    std::memset(container->szBaseName, 0, sizeof(container->szBaseName));
#ifdef __FreeBSD__
	strlcpy(container->szBaseName, szBaseName, sizeof(container->szBaseName) - 1);
#else
    strncpy(container->szBaseName, szBaseName, sizeof(container->szBaseName) - 1);
#endif
    mbMapDataContainer[lMapIndex] = container;
}
