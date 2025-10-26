#include "dungeon.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "party.h"
#include "questmanager.h"
#include "regen.h"
#include "sectree_manager.h"
#include "start_position.h"
#include <game/AffectConstants.hpp>

#include "utils.h"

namespace
{
struct FWarpToPosition {
    int32_t lMapIndex;
    int32_t x;
    int32_t y;

    FWarpToPosition(int32_t lMapIndex, int32_t x, int32_t y)
        : lMapIndex(lMapIndex)
        , x(x)
        , y(y)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* ch = (CHARACTER*)ent;
#ifdef __FAKE_PC__
        if (!ch->IsPC() && !ch->FakePC_IsSupporter())
            return;
#else
        if (!ch->IsPC())
            return;
#endif

        if (ch->GetMapIndex() == lMapIndex) {
            ch->Show(lMapIndex, x, y, 0);
            ch->Stop();
        }
#ifdef __FAKE_PC__
        else if (!ch->FakePC_Check())
#else
        else
#endif
        {
            ch->WarpSet(lMapIndex, x, y);
        }
    }
};

struct FWarpToPositionForce {
    int32_t lMapIndex;
    int32_t x;
    int32_t y;

    FWarpToPositionForce(int32_t lMapIndex, int32_t x, int32_t y)
        : lMapIndex(lMapIndex)
        , x(x)
        , y(y)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* ch = (CHARACTER*)ent;
        if (!ch->IsPC())
            return;

        ch->WarpSet(lMapIndex, x, y);
    }
};

struct FWarpToDungeon {
    FWarpToDungeon(int32_t lMapIndex, CDungeon* d)
        : m_lMapIndex(lMapIndex)
        , m_pkDungeon(d)
    {
        SECTREE_MAP* pkSectreeMap =
            SECTREE_MANAGER::instance().GetMap(lMapIndex);
        m_x = pkSectreeMap->m_setting.posSpawn.x;
        m_y = pkSectreeMap->m_setting.posSpawn.y;
    }

    void operator()(CHARACTER* ch)
    {
        ch->SaveExitLocation();
        ch->WarpSet(m_lMapIndex, m_x, m_y);
    }

    int32_t m_lMapIndex;
    int32_t m_x;
    int32_t m_y;
    CDungeon* m_pkDungeon;
};

struct FKillSectree {
    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            if (!ch->IsPC() && !ch->IsPet() && !ch->IsToggleMount())
                ch->Dead(nullptr, true);
        }
    }
};

struct FPurgeSectree {
    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            if (!ch->IsPC() && !ch->IsPet() && !ch->IsToggleMount()) {
                ch->SetPosition(POS_DEAD);
                M2_DESTROY_CHARACTER(ch);
            }
        } else if (ent->IsType(ENTITY_ITEM)) {
            CItem* item = (CItem*)ent;
            M2_DESTROY_ITEM(item);
        } else {
            SPDLOG_ERROR("unknown entity type %d is in dungeon",
                         ent->GetEntityType());
        }
    }
};

struct FCountMonster {
    int n;

    FCountMonster()
        : n(0){};

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            if (!ch->IsPC())
                n++;
        }
    }
};

struct FExitDungeon {
    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;

            if (ch->IsPC())
                ch->ExitToSavedLocation();
        }
    }
};

struct FChat {
    FChat(int32_t chatType, const char* psz)
        : m_psz(psz)
        , m_chatType(chatType)
    {
    }

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            ch->ChatPacket(m_chatType, "%s", m_psz);
        }
    }

    const char* m_psz;
    int32_t m_chatType;
};

struct FChatLocalized {
    FChatLocalized(int32_t chatType, const char* psz)
        : m_psz(psz)
        , m_chatType(chatType)
    {
    }

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            SendI18nChatPacket(ch, m_chatType, m_psz);
        }
    }

    const char* m_psz;
    int32_t m_chatType;
};

struct FNotice {
    FNotice(const char* psz)
        : m_psz(psz)
    {
    }

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            SendSpecialI18nChatPacket(ch, CHAT_TYPE_NOTICE, m_psz);
        }
    }

    const char* m_psz;
};

struct FExitDungeonToStartPosition {
    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;

            if (ch->IsPC()) {
                PIXEL_POSITION posWarp;

                // 현재 맵 인덱스를 넣는 것이 아니라 시작하는 맵 인덱스를
                // 넣는다.
                if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
                        g_start_position[ch->GetEmpire()][2], ch->GetEmpire(),
                        posWarp))
                    ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()), posWarp.x,
                                posWarp.y);
                else
                    ch->ExitToSavedLocation();
            }
        }
    }
};

struct FNearPosition {
    int32_t x;
    int32_t y;
    int dist;
    bool ret;

    FNearPosition(int32_t x, int32_t y, int d)
        : x(x)
        , y(y)
        , dist(d)
        , ret(true)
    {
    }

    void operator()(CEntity* ent)
    {
        if (ret == false)
            return;

        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;

            if (ch->IsPC()) {
                if (DISTANCE_APPROX(ch->GetX() - x * 100,
                                    ch->GetY() - y * 100) > dist * 100)
                    ret = false;
            }
        }
    }
};
} // namespace

CDungeon::CDungeon(IdType id, int32_t lOriginalMapIndex, int32_t lMapIndex)
    : m_id(id)
    , m_lOrigMapIndex(lOriginalMapIndex)
    , m_lMapIndex(lMapIndex)
    , m_map_Area(SECTREE_MANAGER::instance().GetDungeonArea(lOriginalMapIndex))
    , regen_id_(0)
{
}

CDungeon::~CDungeon()
{
    assert(m_players.empty() && "Sanity check");

    // The unique map might still be filled with non-owned characters on other
    // maps at this point.

    ClearRegen();
}

void CDungeon::AddBossVnum(uint32_t dwVnum)
{
    m_bossDamage.insert_or_assign(dwVnum, 0);
}

uint32_t CDungeon::IsBossVnum(uint32_t vnum)
{
    return m_bossDamage.find(vnum) != m_bossDamage.end();
}

void CDungeon::SetMaxDmgOnBoss(uint32_t dwVnum, uint32_t dwDmg)
{
    m_bossDamage.insert_or_assign(dwVnum, dwDmg);
}

uint32_t CDungeon::GetMaxDmgOnBoss(uint32_t vnum)
{
    if (auto it = m_bossDamage.find(vnum); it != m_bossDamage.end())
        return it->second;

    return 0;
}

void CDungeon::Notice(const char* msg)
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FNotice f(msg);
    pMap->for_each(f);
}

void CDungeon::ClearMissionMessage()
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FChat f(CHAT_TYPE_MISSION, "");
    pMap->for_each(f);
}

void CDungeon::SetMissionMessage(const char* msg)
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FChatLocalized f(CHAT_TYPE_MISSION, msg);
    pMap->for_each(f);
}

void CDungeon::SetMissionSubMessage(const char* msg)
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FChat f(CHAT_TYPE_SUB_MISSION, msg);
    pMap->for_each(f);
}

void CDungeon::Join(CHARACTER* ch)
{
    if (SECTREE_MANAGER::instance().GetMap(m_lMapIndex) == nullptr) {
        SPDLOG_ERROR("CDungeon: SECTREE_MAP not found for %d", m_lMapIndex);
        return;
    }

    FWarpToDungeon(m_lMapIndex, this)(ch);
}

void CDungeon::JoinParty(CParty* pParty)
{
    if (!SECTREE_MANAGER::instance().GetMap(m_lMapIndex)) {
        SPDLOG_ERROR("CDungeon: SECTREE_MAP not found for %d", m_lMapIndex);
        return;
    }

    FWarpToDungeon f(m_lMapIndex, this);
    pParty->ForEachOnlineMember(f);
}

EVENTINFO(dungeon_id_info)
{
    CDungeon::IdType dungeon_id;

    dungeon_id_info()
        : dungeon_id(0)
    {
    }
};

EVENTINFO(dungeon_jump_info)
{
    CDungeon::IdType dungeon_id;
    uint32_t x;
    uint32_t y;

    dungeon_jump_info()
        : dungeon_id(0), x(0), y(0)
    {
    }
};

void CDungeon::JumpAll(int32_t lFromMapIndex, int x, int y)
{
    x *= 100;
    y *= 100;

    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(lFromMapIndex);

    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", lFromMapIndex);
        return;
    }

    FWarpToPosition f(m_lMapIndex, x, y);

    // <Factor> SECTREE::for_each -> SECTREE::for_each_entity
    pMap->for_each(f);
}

void CDungeon::WarpAll(int32_t lFromMapIndex, int x, int y)
{
    x *= 100;
    y *= 100;

    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(lFromMapIndex);

    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", lFromMapIndex);
        return;
    }

    FWarpToPositionForce f(m_lMapIndex, x, y);

    // <Factor> SECTREE::for_each -> SECTREE::for_each_entity
    pMap->for_each(f);
}

void CDungeon::JumpParty(CParty* pParty, int32_t lFromMapIndex, int x, int y)
{
    x *= 100;
    y *= 100;

    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(lFromMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", lFromMapIndex);
        return;
    }

    FWarpToPosition f(m_lMapIndex, x, y);
    pParty->ForEachOnMapMember(f, lFromMapIndex);
}

void CDungeon::ExitAll()
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FExitDungeon f;
    pMap->for_each(f);
}

void CDungeon::ExitAllToStartPosition()
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    FExitDungeonToStartPosition f;
    pMap->for_each(f);
}

void CDungeon::Purge()
{
    SECTREE_MAP* pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkMap == nullptr) {
        SPDLOG_ERROR("CDungeon: SECTREE_MAP not found for %d", m_lMapIndex);
        return;
    }

    FPurgeSectree f;
    pkMap->for_each(f);
}

void CDungeon::KillAll()
{
    SECTREE_MAP* pkMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkMap == nullptr) {
        SPDLOG_ERROR("CDungeon: SECTREE_MAP not found for %d", m_lMapIndex);
        return;
    }

    FKillSectree f;
    pkMap->for_each(f);
}

void CDungeon::Spawn(uint32_t vnum, const char* pos)
{
    if (!m_map_Area) {
        SPDLOG_ERROR("Can't use area feature without any areas loaded");
        return;
    }

    auto it = m_map_Area->find(pos);
    if (it == m_map_Area->end()) {
        SPDLOG_ERROR("Wrong position string : %d", pos);
        return;
    }

    const TAreaInfo& ai = it->second;
    int dir = ai.dir;
    if (dir == -1)
        dir = Random::get(0, 359);

    int dx = Random::get(ai.sx, ai.ex);
    int dy = Random::get(ai.sy, ai.ey);

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    CHARACTER* ch =
        g_pCharManager->SpawnMob(vnum, m_lMapIndex, dx, dy, 0, false, dir);
    if (ch)
        ch->SetDungeon(this);
}

CHARACTER* CDungeon::SpawnMob(uint32_t vnum, int x, int y, int dir)
{
    SPDLOG_INFO("CDungeon::SpawnMob %d %d %d", vnum, x, y);

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return nullptr;
    }

    CHARACTER* ch =
        g_pCharManager->SpawnMob(vnum, m_lMapIndex, x * 100, y * 100, 0, false,
                                 dir == 0 ? -1 : (dir - 1) * 45);

    if (ch) {
        ch->SetDungeon(this);
        SPDLOG_INFO("CDungeon::SpawnMob name %s", ch->GetName());
    }

    return ch;
}

CHARACTER* CDungeon::SpawnMob_ac_dir(uint32_t vnum, int x, int y, int dir)
{
    SPDLOG_INFO("CDungeon::SpawnMob %d %d %d", vnum, x, y);

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return nullptr;
    }

    CHARACTER* ch = g_pCharManager->SpawnMob(vnum, m_lMapIndex, x * 100,
                                             y * 100, 0, false, dir);

    if (ch) {
        ch->SetDungeon(this);
        SPDLOG_INFO("CDungeon::SpawnMob name %s", ch->GetName());
    }

    return ch;
}

void CDungeon::SpawnNameMob(uint32_t vnum, int x, int y, const char* name)
{
    CHARACTER* ch =
        g_pCharManager->SpawnMob(vnum, m_lMapIndex, x, y, 0, false, -1);
    if (ch) {
        ch->SetName(name);
        ch->SetDungeon(this);
    }
}

void CDungeon::SpawnGotoMob(int32_t lFromX, int32_t lFromY, int32_t lToX,
                            int32_t lToY)
{
    const int MOB_GOTO_VNUM = 20039;

    SPDLOG_INFO("SpawnGotoMob {0} {1} to {2} {3}", lFromX, lFromY, lToX, lToY);

    lFromX = lFromX * 100;
    lFromY = lFromY * 100;

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    CHARACTER* ch = g_pCharManager->SpawnMob(MOB_GOTO_VNUM, m_lMapIndex, lFromX,
                                             lFromY, 0, false, -1);

    if (ch) {
        char buf[30 + 1];
        std::snprintf(buf, sizeof(buf), ". %d %d", lToX, lToY);

        ch->SetName(buf);
        ch->SetDungeon(this);
    }
}

CHARACTER* CDungeon::SpawnGroup(uint32_t vnum, int32_t x, int32_t y,
                                float radius, bool bAggressive, int count)
{
    int iRadius = (int)radius;

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return nullptr;
    }

    int sx = x - iRadius;
    int sy = y - iRadius;
    int ex = sx + iRadius;
    int ey = sy + iRadius;

    CHARACTER* ch = nullptr;

    while (count--) {
        CHARACTER* chLeader = g_pCharManager->SpawnGroup(
            vnum, m_lMapIndex, sx, sy, ex, ey, nullptr, bAggressive, this);
        if (chLeader && !ch)
            ch = chLeader;
    }

    return ch;
}

void CDungeon::SpawnMoveGroup(uint32_t vnum, const char* pos_from,
                              const char* pos_to, int count)
{
    if (!m_map_Area) {
        SPDLOG_ERROR("Can't use area feature without any areas loaded");
        return;
    }

    auto it_to = m_map_Area->find(pos_to);
    if (it_to == m_map_Area->end()) {
        SPDLOG_ERROR("Wrong position string : %s", pos_to);
        return;
    }

    auto it_from = m_map_Area->find(pos_from);
    if (it_from == m_map_Area->end()) {
        SPDLOG_ERROR("Wrong position string : %s", pos_from);
        return;
    }

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (pkSectreeMap == nullptr) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return;
    }

    const TAreaInfo& ai = it_from->second;
    const TAreaInfo& ai_to = it_to->second;

    int dir = ai.dir;
    if (dir == -1)
        dir = Random::get(0, 359);

    while (count--) {
        int tx = Random::get(ai_to.sx, ai_to.ex);
        int ty = Random::get(ai_to.sy, ai_to.ey);
        g_pCharManager->SpawnMoveGroup(vnum, m_lMapIndex, ai.sx, ai.sy, ai.ex,
                                       ai.ey, tx, ty, nullptr, true);
    }
}

void CDungeon::SpawnRegen(const char* filename, bool bOnce)
{
    if (!filename) {
        SPDLOG_ERROR("CDungeon::SpawnRegen(filename={}, bOnce={}) - "
                     "m_lMapIndex[{}] file does not exist",
                     filename, bOnce, m_lMapIndex);
        return;
    }

    SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pkSectreeMap) {
        SPDLOG_ERROR("Cannot spawn regen (filename={}, bOnce={}) - "
                     "m_lMapIndex[{}] no map",
                     filename, bOnce, m_lMapIndex);
        return;
    }
    regen_do(filename, m_lMapIndex, this, bOnce);
}

void CDungeon::AddRegen(regen* r)
{
    r->id = regen_id_++;
    m_regen.push_back(r);
}

void CDungeon::ClearRegen()
{
    for (auto it = m_regen.begin(); it != m_regen.end(); ++it) {
        regen* r = *it;

        // TODO: Clear regen for mobs here, so we can remove the regen id!

        event_cancel(&r->event);
        delete r;
    }

    m_regen.clear();
}

bool CDungeon::IsValidRegen(regen* r, size_t regen_id)
{
    auto it = std::find(m_regen.begin(), m_regen.end(), r);
    if (it == m_regen.end())
        return false;

    return (*it)->id == regen_id;
}

void CDungeon::Completed()
{
    m_completed = true;
    CDungeonManager::instance().RemovePlayerInfoDungeon(m_lMapIndex);
}

/*virtual*/
void CDungeon::OnEnter(CHARACTER* ch)
{
    if (ch->IsPC())
        m_players.insert(ch);
}

/*virtual*/
void CDungeon::OnLeave(CHARACTER* ch)
{
	if (!m_completed) {
        if (m_bSkipSaveWarpOnce) {
            m_bSkipSaveWarpOnce = false;
        } else {
            CDungeonManager::instance().SetPlayerInfo(ch);
        }
    }

    if (ch->IsPC())
        m_players.erase(ch);
}

void CDungeon::Rejoin(CHARACTER* ch)
{
    SkipPlayerSaveDungeonOnce();
    ch->WarpSet(m_lMapIndex, m_rejoinX, m_rejoinY);
    CDungeonManager::instance().RemovePlayerInfo(ch->GetPlayerID());
}

//
// QuestDungeon
//

QuestDungeon::QuestDungeon(IdType id, int32_t originalMapIndex,
                           int32_t mapIndex)
    : CDungeon(id, originalMapIndex, mapIndex)
    , m_iMobKill(0)
    , m_iStoneKill(0)
    , m_bUsePotion(false)
    , m_bUseRevive(false)
    , m_iMonsterCount(0)
    , m_bExitAllAtEliminate(false)
    , m_bWarpAtEliminate(false)
    , m_bEventAtEliminate(false)
    , m_iWarpDelay(0)
    , m_lWarpMapIndex(0)
    , m_lWarpX(0)
    , m_lWarpY(0)
    , m_lExitWarpMapIndex(0)
    , m_lExitWarpX(0)
    , m_lExitWarpY(0)
{
    // ctor
    m_dungeonScriptIndex = quest::CQuestManager::instance().LoadDungeonScript(
        std::to_string(originalMapIndex));
}

QuestDungeon::~QuestDungeon()
{
    event_cancel(&deadEvent);
    event_cancel(&exit_all_event_);
    event_cancel(&jump_to_event_);
    event_cancel(&jump_all_event_);
}

void QuestDungeon::SetFlag(const std::string& name, int value)
{
    auto it = m_map_Flag.find(name);
    if (it != m_map_Flag.end())
        it->second = value;
    else
        m_map_Flag.emplace(name, value);

    // quest::CQuestManager::instance().DungeonFlagChangeEvent(m_dungeonScriptIndex,
    // this, name);
}

int QuestDungeon::GetFlag(const std::string& name)
{
    auto it = m_map_Flag.find(name);
    if (it != m_map_Flag.end())
        return it->second;
    else
        return 0;
}

EVENTFUNC(dungeon_dead_event)
{
    auto* info = static_cast<dungeon_id_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("dungeon_dead_event> <Factor> Null pointer");
        return 0;
    }

    auto* pDungeon =
        CDungeonManager::instance().Find<QuestDungeon>(info->dungeon_id);
    if (pDungeon == nullptr)
        return 0;

    pDungeon->deadEvent = nullptr;

    CDungeonManager::instance().Destroy(info->dungeon_id);

    return 0;
}

void QuestDungeon::IncMonster()
{
    ++m_iMonsterCount;
}

void QuestDungeon::DecMonster()
{
    --m_iMonsterCount;
    CheckEliminated();
}

void QuestDungeon::IncKillCount(CHARACTER* pkKiller, CHARACTER* pkVictim)
{
    if (pkVictim->IsStone())
        m_iStoneKill++;
    else
        m_iMobKill++;
}

int QuestDungeon::GetKillMobCount()
{
    return m_iMobKill;
}

int QuestDungeon::GetKillStoneCount()
{
    return m_iStoneKill;
}

int QuestDungeon::CountRealMonster()
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lOrigMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lOrigMapIndex);
        return 0;
    }

    FCountMonster f;

    // <Factor> SECTREE::for_each -> SECTREE::for_each_entity
    pMap->for_each(f);
    return f.n;
}

EVENTFUNC(dungeon_jump_to_event)
{
    auto* info = static_cast<dungeon_id_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("dungeon_jump_to_event> <Factor> Null pointer");
        return 0;
    }

    auto* pDungeon =
        CDungeonManager::instance().Find<QuestDungeon>(info->dungeon_id);
    pDungeon->jump_to_event_ = nullptr;

    if (pDungeon)
        pDungeon->JumpToEliminateLocation();
    else
        SPDLOG_ERROR("cannot find dungeon with map index %d", info->dungeon_id);

    return 0;
}

EVENTFUNC(dungeon_exit_all_event)
{
    auto* info = static_cast<dungeon_id_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("dungeon_exit_all_event> <Factor> Null pointer");
        return 0;
    }

    auto* pDungeon =
        CDungeonManager::instance().Find<QuestDungeon>(info->dungeon_id);
    pDungeon->exit_all_event_ = nullptr;

    if (pDungeon)
        pDungeon->ExitAll();

    return 0;
}

EVENTFUNC(dungeon_jump_all_event)
{
    auto* info = static_cast<dungeon_jump_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("dungeon_exit_all_event> <Factor> Null pointer");
        return 0;
    }

    auto* pDungeon =
        CDungeonManager::instance().Find<QuestDungeon>(info->dungeon_id);
    pDungeon->jump_all_event_ = nullptr;

    if (pDungeon)
        pDungeon->JumpAll(pDungeon->GetMapIndex(), info->x, info->y);

    return 0;
}

void QuestDungeon::JumpAllDelayed(int x, int y, int delay)
{
    dungeon_jump_info* info = AllocEventInfo<dungeon_jump_info>();
    info->dungeon_id = m_id;
    info->x = x;
    info->y = y;

    event_cancel(&jump_all_event_);
    jump_all_event_ = event_create(dungeon_jump_all_event, info,
                                   THECORE_SECS_TO_PASSES(delay));
}

void QuestDungeon::CheckEliminated()
{
    if (m_iMonsterCount > 0)
        return;

    if (m_bExitAllAtEliminate) {
        SPDLOG_TRACE("CheckEliminated: exit");
        m_bExitAllAtEliminate = false;

        if (m_iWarpDelay) {
            dungeon_id_info* info = AllocEventInfo<dungeon_id_info>();
            info->dungeon_id = m_id;

            event_cancel(&exit_all_event_);
            exit_all_event_ =
                event_create(dungeon_exit_all_event, info,
                             THECORE_SECS_TO_PASSES(m_iWarpDelay));
        } else {
            ExitAll();
        }
    } else if (m_bWarpAtEliminate) {
        SPDLOG_TRACE("CheckEliminated: warp");
        m_bWarpAtEliminate = false;

        if (m_iWarpDelay) {
            dungeon_id_info* info = AllocEventInfo<dungeon_id_info>();
            info->dungeon_id = m_id;

            event_cancel(&jump_to_event_);
            jump_to_event_ = event_create(dungeon_jump_to_event, info,
                                          THECORE_SECS_TO_PASSES(m_iWarpDelay));
        } else {
            JumpToEliminateLocation();
        }
    } else if (m_bEventAtEliminate) {
        SPDLOG_TRACE("CheckEliminated: event");
        m_bEventAtEliminate = false;
        quest::CQuestManager::instance().DungeonEliminatedEvent(
            m_dungeonScriptIndex, this);
    } else {
        SPDLOG_TRACE("CheckEliminated: none");
    }
}

void QuestDungeon::SetExitAllAtEliminate(int32_t time)
{
    SPDLOG_INFO("SetExitAllAtEliminate: time {}", time);
    m_bExitAllAtEliminate = true;
    m_iWarpDelay = time;
}

void QuestDungeon::SetEventAtEliminate(const char* regen_file)
{
    m_bEventAtEliminate = true;
    if (!regen_file || !*regen_file)
        m_stRegenFile.clear();
    else
        m_stRegenFile = regen_file;

    SPDLOG_INFO("SetEventAtEliminate: regenfile {}", m_stRegenFile.c_str());
}

void QuestDungeon::SetDungeonExitLocation(uint32_t mapIndex, uint32_t x,
                                          uint32_t y)
{
    m_lExitWarpMapIndex = mapIndex;
    m_lExitWarpX = x;
    m_lExitWarpY = y;
}

void QuestDungeon::SetWarpAtEliminate(int32_t time, int32_t lMapIndex, int x,
                                      int y, const char* regen_file)
{
    m_bWarpAtEliminate = true;
    m_iWarpDelay = time;
    m_lWarpMapIndex = lMapIndex;
    m_lWarpX = x;
    m_lWarpY = y;

    if (!regen_file || !*regen_file)
        m_stRegenFile.clear();
    else
        m_stRegenFile = regen_file;

    SPDLOG_INFO("SetWarpAtEliminate: time {} map {} {}x{} regenfile {}", time,
                lMapIndex, x, y, m_stRegenFile.c_str());
}

void QuestDungeon::JumpToEliminateLocation()
{
    auto* pDungeon =
        CDungeonManager::instance().FindByMapIndex<CDungeon>(m_lWarpMapIndex);
    if (pDungeon) {
        pDungeon->JumpAll(m_lMapIndex, m_lWarpX, m_lWarpY);

        if (!m_stRegenFile.empty()) {
            pDungeon->SpawnRegen(m_stRegenFile.c_str());
            m_stRegenFile.clear();
        }
    } else {
        // 일반 맵으로 워프
        SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
        if (!pMap) {
            SPDLOG_ERROR("Cannot jump to eliminate location no map by index {}", m_lMapIndex);
            return;
        }

        FWarpToPosition f(m_lWarpMapIndex, m_lWarpX * 100, m_lWarpY * 100);

        // <Factor> SECTREE::for_each -> SECTREE::for_each_entity
        pMap->for_each(f);
    }
}

void QuestDungeon::SpawnMoveUnique(const char* key, uint32_t vnum,
                                   const char* pos_from, const char* pos_to)
{
    if (!m_map_Area) {
        SPDLOG_ERROR("Can't use area feature without any areas loaded");
        return;
    }

    auto it_to = m_map_Area->find(pos_to);
    if (it_to == m_map_Area->end()) {
        SPDLOG_ERROR("Undefined area '{}' when spawning unique (mapIdx {})", pos_to, m_lOrigMapIndex);
        return;
    }

    auto it_from = m_map_Area->find(pos_from);
    if (it_from == m_map_Area->end()) {
        SPDLOG_ERROR("Undefined area '{}' when spawning unique (mapIdx {})", pos_from, m_lOrigMapIndex);
        return;
    }

    const TAreaInfo& ai = it_from->second;
    const TAreaInfo& ai_to = it_to->second;
    int dir = ai.dir;
    if (dir == -1)
        dir = Random::get(0, 359);

    for (int i = 0; i < 100; i++) {
        int dx = Random::get(ai.sx, ai.ex);
        int dy = Random::get(ai.sy, ai.ey);
        int tx = Random::get(ai_to.sx, ai_to.ex);
        int ty = Random::get(ai_to.sy, ai_to.ey);

        CHARACTER* ch =
            g_pCharManager->SpawnMob(vnum, m_lMapIndex, dx, dy, 0, false, dir);
        if (ch) {
            m_map_UniqueMob.emplace(key, ch);
            ch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, 65535, 0, true);
            ch->SetDungeon(this);

            ch->Goto(tx, ty);
        } else {
            SPDLOG_ERROR("Dungeon: Cannot spawn move unique at {} {}", (ai.sx + ai.ex) >> 1,
                         (ai.sy + ai.ey) >> 1);
        }
    }
}

void QuestDungeon::SpawnUnique(const char* key, uint32_t vnum, const char* pos)
{
    if (!m_map_Area) {
        SPDLOG_ERROR("Can't use area feature without any areas loaded");
        return;
    }

    auto it = m_map_Area->find(pos);
    if (it == m_map_Area->end()) {
        SPDLOG_ERROR("Undefined area '{}' when spawning unique (mapIdx {})", pos, m_lOrigMapIndex);
        return;
    }

    const TAreaInfo& ai = it->second;
    int dir = ai.dir;
    if (dir == -1)
        dir = Random::get(0, 359);

    for (int i = 0; i < 100; i++) {
        int dx = Random::get(ai.sx, ai.ex);
        int dy = Random::get(ai.sy, ai.ey);

        CHARACTER* ch =
            g_pCharManager->SpawnMob(vnum, m_lMapIndex, dx, dy, 0, false, dir);

        if (ch) {
            m_map_UniqueMob.emplace(key, ch);
            ch->SetDungeon(this);
            ch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, 65535, 0, true);
            break;
        } else {
            SPDLOG_ERROR("Dungeon: Cannot spawn unique at {} {}", (ai.sx + ai.ex) >> 1,
                         (ai.sy + ai.ey) >> 1);
        }
    }
}

void QuestDungeon::SetUnique(const char* key, uint32_t vid)
{
    CHARACTER* ch = g_pCharManager->Find(vid);
    if (ch) {
        m_map_UniqueMob.emplace(key, ch);
        ch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, 65535, 0, true);
    }
}

void QuestDungeon::SetRejoinPosition(uint32_t x, uint32_t y)
{
    m_rejoinX = x;
    m_rejoinY = y;
}

void QuestDungeon::SpawnStoneDoor(const char* key, const char* pos)
{
    SpawnUnique(key, 13001, pos);
}

void QuestDungeon::SpawnWoodenDoor(const char* key, const char* pos)
{
    SpawnUnique(key, 13000, pos);
    UniqueSetMaxHP(key, 10000);
    UniqueSetHP(key, 10000);
    UniqueSetDefGrade(key, 300);
}

void QuestDungeon::KillUnique(const std::string& key)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return;
    }

    CHARACTER* ch = it->second;
    m_map_UniqueMob.erase(it);
    ch->Dead();
}

void QuestDungeon::PurgeUnique(const std::string& key)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return;
    }

    CHARACTER* ch = it->second;
    m_map_UniqueMob.erase(it);
    M2_DESTROY_CHARACTER(ch);
}

bool QuestDungeon::IsUniqueDead(const std::string& key)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return true;
    }

    return it->second->IsDead();
}

uint32_t QuestDungeon::GetUniqueVid(const std::string& key)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return 0;
    }

    CHARACTER* ch = it->second;
    return ch->GetVID();
}

PointValue QuestDungeon::GetUniqueHpPerc(const std::string& key)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return false;
    }
    return (100.f * it->second->GetHP()) / it->second->GetMaxHP();
}

void QuestDungeon::UniqueSetMaxHP(const std::string& key, int64_t iMaxHP)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return;
    }

    it->second->SetMaxHP(iMaxHP);
}

void QuestDungeon::UniqueSetHP(const std::string& key, int64_t iHP)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return;
    }

    it->second->SetHP(iHP);
}

void QuestDungeon::UniqueSetHPPerc(const std::string& key, int iHPPct)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Dungeon: Unknown Key or Dead: {0}", key.c_str());
        return;
    }

    it->second->SetHP(it->second->GetMaxHP() / 100.0f * iHPPct);
}

void QuestDungeon::UniqueSetDefGrade(const std::string& key, int iGrade)
{
    const auto it = m_map_UniqueMob.find(key);
    if (it == m_map_UniqueMob.end()) {
        SPDLOG_ERROR("Unknown Key : %s", key.c_str());
        return;
    }

    it->second->PointChange(POINT_DEF_GRADE,
                            iGrade - it->second->GetPoint(POINT_DEF_GRADE));
}

void QuestDungeon::SetDungeonStage(int stage)
{
    m_prevDungeonStage = m_dungeonStage;
    m_dungeonStage = stage;
    SPDLOG_ERROR("Setting dungeon stage {}", m_prevDungeonStage,
                 m_dungeonStage);
    quest::CQuestManager::instance().DungeonStageChange(m_dungeonScriptIndex,
                                                        this);
}

bool QuestDungeon::IsAllPCNearTo(int x, int y, int dist)
{
    auto* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", m_lMapIndex);
        return false;
    }

    FNearPosition f(x, y, dist);
    pMap->for_each(f);

    return f.ret;
}

void QuestDungeon::CreateItemGroup(const std::string& group_name,
                                   ItemGroup& item_group)
{
    m_map_ItemGroup.emplace(group_name, item_group);
}

const QuestDungeon::ItemGroup*
QuestDungeon::GetItemGroup(const std::string& group_name)
{
    const auto it = m_map_ItemGroup.find(group_name);
    if (it != m_map_ItemGroup.end())
        return &(it->second);
    else
        return nullptr;
}

/*virtual*/
void QuestDungeon::OnEnter(CHARACTER* ch)
{
    CDungeon::OnEnter(ch);

    if (ch->IsPC())
        event_cancel(&deadEvent);
    else if (ch->IsMonster() || ch->IsStone())
        IncMonster();
}

/*virtual*/
void QuestDungeon::OnLeave(CHARACTER* ch)
{
    CDungeon::OnLeave(ch);

    if (!ch->IsPC()) {
        auto it = m_map_UniqueMob.begin();
        while (it != m_map_UniqueMob.end()) {
            if (it->second == ch) {
                m_map_UniqueMob.erase(it);
                break;
            }
            ++it;
        }
    }

    if (ch->IsMonster() || ch->IsStone())
        DecMonster();

    if (m_players.empty() && !deadEvent) {
        auto* info = AllocEventInfo<dungeon_id_info>();
        const auto when = THECORE_SECS_TO_PASSES(!m_completed ? 300 : 10);
        info->dungeon_id = m_id;
        deadEvent = event_create(dungeon_dead_event, info, when);
    }
}

/*virtual*/
void QuestDungeon::OnKill(CHARACTER* killer, CHARACTER* victim)
{
    if (!victim->IsPC() && killer->IsPC())
        IncKillCount(killer, victim);

    if (!victim->IsPC()) {
        auto it = m_map_UniqueMob.begin();
        while (it != m_map_UniqueMob.end()) {
            if (it->second == victim) {
                m_map_UniqueMob.erase(it);
                break;
            }
            ++it;
        }
    }
}

void QuestDungeon::ExitAllDungeon()
{
    SECTREE_MAP* pMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);
    if (!pMap) {
        SPDLOG_ERROR("Dungeon: Cannot find map by index {}", m_lMapIndex);
        return;
    }

    if (!m_lExitWarpMapIndex) {
        SPDLOG_ERROR("Dungeon: Exiting with no exit location set with map index {}", m_lMapIndex);
        return;
    }

    pMap->for_each([this](CEntity* ent) {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;

            if (ch->IsPC())
                ch->WarpSet(m_lExitWarpMapIndex, m_lExitWarpX * 100,
                            m_lExitWarpY * 100);
        }
    });
}

//
// CDungeonManager
//

CDungeonManager::CDungeonManager()
    : next_id_(0)
{
}

CDungeonManager::~CDungeonManager() {}

void CDungeonManager::Destroy(CDungeon::IdType dungeon_id)
{
    SPDLOG_INFO("DUNGEON destroy : map index {}", dungeon_id);

    auto pDungeon = Find<CDungeon>(dungeon_id);
    if (pDungeon == nullptr)
        return;

	RemovePlayerInfoDungeon(pDungeon->m_lMapIndex);
    m_dungeonsByMap.erase(pDungeon->m_lMapIndex);

    quest::CQuestManager::instance().CancelServerTimers(pDungeon->m_lMapIndex);
    SECTREE_MANAGER::instance().DestroyPrivateMap(pDungeon->m_lMapIndex);

    // Dungeon is deleted after this call
    m_dungeons.erase(dungeon_id);
}

void CDungeonManager::SetPlayerInfo(DWORD pid, uint32_t mapIndex)
{
    RemovePlayerInfo(pid);
    m_map_pkDungeonPlayerInfo.emplace(pid, mapIndex);
}

void CDungeonManager::SetPlayerInfo(CHARACTER* ch)
{
    SetPlayerInfo(ch->GetPlayerID(), ch->GetMapIndex());
}

void CDungeonManager::RemovePlayerInfoDungeon(uint32_t map_idx)
{
    if (m_map_pkDungeonPlayerInfo.empty())
        return;

    for (auto it = m_map_pkDungeonPlayerInfo.begin(); it != m_map_pkDungeonPlayerInfo.end();)
    {
        if ((*it).second == map_idx)
            it = m_map_pkDungeonPlayerInfo.erase(it);
        else
            ++it;
    }
}

void CDungeonManager::RemovePlayerInfo(uint32_t pid)
{
    auto it = m_map_pkDungeonPlayerInfo.find(pid);
    if (it != m_map_pkDungeonPlayerInfo.end()) {
        m_map_pkDungeonPlayerInfo.erase(it);
    }
}

uint32_t CDungeonManager::GetPlayerInfo(uint32_t pid)
{
    auto it = m_map_pkDungeonPlayerInfo.find(pid);
    if (it != m_map_pkDungeonPlayerInfo.end()) {
        return it->second;
    }

    return 0;
}

uint32_t CDungeonManager::CreateAux(uint32_t mapIndex)
{
    return SECTREE_MANAGER::instance().CreatePrivateMap(mapIndex);
}
