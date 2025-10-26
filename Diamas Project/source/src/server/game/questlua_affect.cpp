#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include <game/AffectConstants.hpp>
#include "db.h"

namespace quest
{
//
// "affect" Lua functions
//
int affect_add(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager &q = CQuestManager::instance();

    uint8_t applyOn = (uint8_t)lua_tonumber(L, 1);

    CHARACTER *ch = q.GetCurrentCharacterPtr();

    if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
    {
        SPDLOG_ERROR("apply is out of range : %d", applyOn);
        return 0;
    }

    if (ch->FindAffect(AFFECT_QUEST_START_IDX + q.GetCurrentPC()->GetCurrentQuestIndex(), applyOn))
        // 퀘스트로 인해 같은 곳에 효과가 걸려있으면 스킵
        return 0;

    long value = (long)lua_tonumber(L, 2);
    long duration = (long)lua_tonumber(L, 3);

    ch->AddAffect(AFFECT_QUEST_START_IDX + q.GetCurrentPC()->GetCurrentQuestIndex(), GetApplyPoint(applyOn), value,
                  duration, 0, false);

    return 0;
}

int affect_remove(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    int iType;

    if (lua_isnumber(L, 1))
    {
        iType = (int)lua_tonumber(L, 1);

        if (iType == 0)
            iType = q.GetCurrentPC()->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;
    }
    else
        iType = q.GetCurrentPC()->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;

    q.GetCurrentCharacterPtr()->RemoveAffect(iType);
    return 0;
}

int affect_remove_bad(lua_State *L) // 나쁜 효과를 없앰
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    ch->RemoveBadAffect();
    return 0;
}

int affect_remove_good(lua_State *L) // 좋은 효과를 없앰
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    ch->RemoveGoodAffect();
    return 0;
}

// 현재 캐릭터가 AFFECT_TYPE affect를 갖고있으면 bApplyOn 값을 반환하고 없으면 nil을 반환하는 함수.
// usage :	applyOn = affect.get_apply(AFFECT_TYPE) 
int affect_get_apply_on(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    uint32_t affectType = (uint32_t)lua_tonumber(L, 1);

    const auto pkAff = ch->FindAffect(affectType);

    if (pkAff)
        lua_pushnumber(L, pkAff->pointType);
    else
        lua_pushnil(L);

    return 1;
}

int affect_add_collect(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager &q = CQuestManager::instance();

    uint8_t applyOn = (uint8_t)lua_tonumber(L, 1);

    CHARACTER *ch = q.GetCurrentCharacterPtr();

    if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
    {
        SPDLOG_ERROR("apply is out of range : %d", applyOn);
        return 0;
    }

    long value = (long)lua_tonumber(L, 2);
    long duration = (long)lua_tonumber(L, 3);

    ch->AddAffect(AFFECT_COLLECT, GetApplyPoint(applyOn), value,
                  duration, 0, false);
    return 0;
}

int affect_add_collect_point(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager &q = CQuestManager::instance();

    uint8_t point_type = (uint8_t)lua_tonumber(L, 1);

    CHARACTER *ch = q.GetCurrentCharacterPtr();

    if (point_type >= POINT_MAX_NUM || point_type < 1)
    {
        SPDLOG_ERROR("point is out of range : %d", point_type);
        return 0;
    }

    long value = (long)lua_tonumber(L, 2);
    long duration = (long)lua_tonumber(L, 3);

    ch->AddAffect(AFFECT_COLLECT, point_type, value,
                  duration, 0, false);
    return 0;
}

int affect_remove_collect(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch != nullptr)
    {
        uint8_t bApply = (uint8_t)lua_tonumber(L, 1);

        if (bApply >= MAX_APPLY_NUM)
            return 0;

        bApply = GetApplyPoint(bApply);
        int32_t value = (int32_t)lua_tonumber(L, 2);

        for (const auto &affect : ch->GetAffectContainer())
        {
            if (affect.type == AFFECT_COLLECT)
            {
                if (affect.pointType == bApply && affect.pointValue == value)
                {
                    ch->RemoveAffect(affect);
                    break;
                }
            }
        }
    }

    return 0;
}

int affect_remove_all_collect(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch != nullptr) { ch->RemoveAffect(AFFECT_COLLECT); }

    return 0;
}

void RegisterAffectFunctionTable()
{
    luaL_reg affect_functions[] =
    {
        {"add", affect_add},
        {"remove", affect_remove},
        {"remove_bad", affect_remove_bad},
        {"remove_good", affect_remove_good},
        {"add_collect", affect_add_collect},
        {"add_collect_point", affect_add_collect_point},
        {"remove_collect", affect_remove_collect},
        {"remove_all_collect", affect_remove_all_collect},
        {"get_apply_on", affect_get_apply_on},

        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("affect", affect_functions);
}
};
