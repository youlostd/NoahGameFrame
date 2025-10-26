#include "mining.h"
#include "base/GroupTextTree.hpp"
#include "base/GroupTextTreeUtil.hpp"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "game/AffectConstants.hpp"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "skill.h"

#include <storm/StringFlags.hpp>

namespace mining
{
enum {
    MAX_ORE = 19,
    MAX_FRACTION_COUNT = 9,
    ORE_COUNT_FOR_REFINE = 100,
};

struct SInfo {
    uint32_t dwLoadVnum;
    uint32_t dwRawOreVnum;
    uint32_t dwRefineVnum;
};

SInfo info[MAX_ORE] = {
    {20047, 50601, 50621}, {20048, 50602, 50622}, {20049, 50603, 50623},
    {20050, 50604, 50624}, {20051, 50605, 50625}, {20052, 50606, 50626},
    {20053, 50607, 50627}, {20054, 50608, 50628}, {20055, 50609, 50629},
    {20056, 50610, 50630}, {20057, 50611, 50631}, {20058, 50612, 50632},
    {20059, 50613, 50633}, {30301, 50614, 50634}, {30302, 50615, 50635},
    {30303, 50616, 50636}, {30304, 50617, 50637}, {30305, 50618, 50638},
    {30306, 50619, 50639},

};

struct MiningExtraDropInfo {
    ItemVnum vnum;
    uint8_t chance;
};

using PickaxeVnum = ItemVnum;

std::unordered_map<PickaxeVnum, std::unique_ptr<CSpecialItemGroup>>
    miningExtraDropInfo;

int fraction_info[MAX_FRACTION_COUNT][3] = {
    {20, 1, 10}, {30, 11, 20}, {20, 21, 30}, {15, 31, 40}, {5, 41, 50},
    {4, 51, 60}, {3, 61, 70},  {2, 71, 80},  {1, 81, 90},
};

int PickGradeAddPct[10] = {3, 5, 8, 11, 15, 20, 26, 32, 40, 50};

int SkillLevelAddPct[SKILL_MAX_LEVEL + 1] = {
    0,  1,  1,  1, 1, //  1 - 4
    2,  2,  2,  2,    //  5 - 8
    3,  3,  3,  3,    //  9 - 12
    4,  4,  4,  4,    // 13 - 16
    5,  5,  5,  5,    // 17 - 20
    6,  6,  6,  6,    // 21 - 24
    7,  7,  7,  7,    // 25 - 28
    8,  8,  8,  8,    // 29 - 32
    9,  9,  9,  9,    // 33 - 36
    10, 10, 10,       // 37 - 39
    11,               // 40
};

uint32_t GetRawOreFromLoad(uint32_t dwLoadVnum)
{
    for (int i = 0; i < MAX_ORE; ++i) {
        if (info[i].dwLoadVnum == dwLoadVnum)
            return info[i].dwRawOreVnum;
    }
    return 0;
}

uint32_t GetRefineFromRawOre(uint32_t dwRawOreVnum)
{
    for (int i = 0; i < MAX_ORE; ++i) {
        if (info[i].dwRawOreVnum == dwRawOreVnum)
            return info[i].dwRefineVnum;
    }
    return 0;
}

int GetFractionCount()
{
    int r = Random::get(25, 30);

    return r;
}

void OreDrop(CHARACTER* ch, uint32_t dwLoadVnum)
{
    uint32_t dwRawOreVnum = GetRawOreFromLoad(dwLoadVnum);

    int iFractionCount = GetFractionCount();

    if (iFractionCount == 0) {
        SPDLOG_ERROR("Wrong ore fraction count");
        return;
    }

    CItem* item =
        ITEM_MANAGER::instance().CreateItem(dwRawOreVnum, GetFractionCount());

    if (!item) {
        SPDLOG_ERROR("cannot create item vnum %d", dwRawOreVnum);
        return;
    }

    PIXEL_POSITION pos;
    pos.x = ch->GetX() + Random::get(-200, 200);
    pos.y = ch->GetY() + Random::get(-200, 200);

    item->AddToGround(ch->GetMapIndex(), pos);
    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
        item->StartDestroyEvent(gConfig.itemGroundTimeLong);
        item->SetOwnership(ch, gConfig.itemOwnershipTimeLong);
    } else {
        item->StartDestroyEvent(gConfig.itemGroundTime);
        item->SetOwnership(ch, gConfig.itemOwnershipTime);
    }
    CItem* pick = ch->GetWear(WEAR_WEAPON);
    if (pick) {

        if (auto it = miningExtraDropInfo.find(pick->GetVnum());
            it != miningExtraDropInfo.end()) {

            const auto* pGroup = it->second.get();
            std::vector<int> idxes;
            int n = pGroup->GetMultiIndex(idxes);

            bool bSuccess = false;

            for (int i = 0; i < n; i++) {
                bSuccess = false;
                int idx = idxes[i];
                uint32_t dwVnum = pGroup->GetVnum(idx);
                SocketValue dwCount = pGroup->GetCount(idx);
                int iRarePct = pGroup->GetRarePct(idx);
                CItem* item_get = nullptr;
                switch (dwVnum) {
                    case CSpecialItemGroup::GOLD:
                        ch->ChangeGold(static_cast<Gold>(dwCount));
                        LogManager::instance().CharLog(ch, dwCount,
                                                       "TREASURE_GOLD", "");

                        bSuccess = true;
                        break;
                    case CSpecialItemGroup::EXP: {
                        ch->PointChange(POINT_EXP, dwCount);
                        LogManager::instance().CharLog(ch, dwCount,
                                                       "TREASURE_EXP", "");
                        bSuccess = true;
                    } break;

                    case CSpecialItemGroup::MOB: {
                        SPDLOG_INFO("CSpecialItemGroup::MOB %d", dwCount);
                        int x = ch->GetX() + Random::get(-500, 500);
                        int y = ch->GetY() + Random::get(-500, 500);

                        CHARACTER* ch = g_pCharManager->SpawnMob(
                            dwCount, ch->GetMapIndex(), x, y, 0, true, -1);
                        if (ch)
                            ch->SetAggressive();
                        bSuccess = true;
                    } break;
                    case CSpecialItemGroup::SLOW: {
                        SPDLOG_INFO("CSpecialItemGroup::SLOW %d",
                                    -(int)dwCount);
                        ch->AddAffect(AFFECT_SLOW, POINT_MOV_SPEED,
                                      -(int)dwCount, 300, 0, true);
                        bSuccess = true;
                    } break;
                    case CSpecialItemGroup::DRAIN_HP: {
                        int64_t iDropHP = ch->GetMaxHP() * dwCount / 100;
                        SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                        iDropHP = std::min(iDropHP, ch->GetHP() - 1);
                        SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                        ch->PointChange(POINT_HP, -iDropHP);
                        bSuccess = true;
                    } break;
                    case CSpecialItemGroup::POISON: {
                        ch->AttackedByPoison(nullptr);
                        bSuccess = true;
                    } break;

                    case CSpecialItemGroup::MOB_GROUP: {
                        int sx = ch->GetX() - Random::get(300, 500);
                        int sy = ch->GetY() - Random::get(300, 500);
                        int ex = ch->GetX() + Random::get(300, 500);
                        int ey = ch->GetY() + Random::get(300, 500);
                        g_pCharManager->SpawnGroup(dwCount, ch->GetMapIndex(),
                                                   sx, sy, ex, ey, nullptr,
                                                   true);

                        bSuccess = true;
                    } break;
                    case CSpecialItemGroup::POLY_MARBLE: {
                        item_get =
                            ITEM_MANAGER::instance().CreateItem(70104, 1);

                        if (item_get) {
                            item_get->SetSocket(0, dwCount, false);
                            item_get = ch->AutoGiveItem(item_get, true);
                            if (item_get)
                                bSuccess = true;
                        }
                    } break;
                    default: {
                        item_get = ch->AutoGiveItem(dwVnum, dwCount, iRarePct);

                        if (item_get) {
                            bSuccess = true;
                        }
                    } break;
                }

                if (bSuccess) {

                } else {
                    return;
                }
            }
        }
    }

    DBManager::instance().SendMoneyLog(MONEY_LOG_DROP, item->GetVnum(),
                                       item->GetCount());
}

int GetOrePct(CHARACTER* ch)
{
    int defaultPct = 20;
    int iSkillLevel = ch->GetSkillLevel(SKILL_MINING);

    CItem* pick = ch->GetWear(WEAR_WEAPON);

    if (!pick || pick->GetItemType() != ITEM_PICK)
        return 0;

    return 100;
}

EVENTINFO(mining_event_info)
{
    uint32_t pid;
    uint32_t vid_load;

    mining_event_info()
        : pid(0)
        , vid_load(0)
    {
    }
};

// REFINE_PICK
bool Pick_Check(CItem& item)
{
    if (item.GetItemType() != ITEM_PICK)
        return false;

    return true;
}

int Pick_GetMaxExp(CItem& pick)
{
    return pick.GetValue(2);
}

int Pick_GetCurExp(CItem& pick)
{
    return pick.GetSocket(0);
}

void Pick_IncCurExp(CItem& pick)
{
    int cur = Pick_GetCurExp(pick);
    pick.SetSocket(0, cur + 1);
}

void Pick_MaxCurExp(CItem& pick)
{
    int max = Pick_GetMaxExp(pick);
    pick.SetSocket(0, max);
}

bool Pick_Refinable(CItem& item)
{
    if (Pick_GetCurExp(item) < Pick_GetMaxExp(item))
        return false;

    return true;
}

bool Pick_IsPracticeSuccess(CItem& pick)
{
    return (Random::get<int>(1, pick.GetValue(1)) == 1);
}

bool Pick_IsRefineSuccess(CItem& pick)
{
    return (Random::get(1, 100) <= pick.GetValue(3));
}

int RealRefinePick(CHARACTER* ch, CItem* item)
{
    if (!ch || !item)
        return 2;

    LogManager& rkLogMgr = LogManager::instance();
    ITEM_MANAGER& rkItemMgr = ITEM_MANAGER::instance();

    if (!Pick_Check(*item)) {
        SPDLOG_ERROR("REFINE_PICK_HACK pid(%u) item(%s:%d) type(%d)",
                     ch->GetPlayerID(), item->GetName(), item->GetID(),
                     item->GetItemType());
        rkLogMgr.RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(),
                           -1, 1, "PICK_HACK");
        return 2;
    }

    CItem& rkOldPick = *item;

    if (!Pick_Refinable(rkOldPick))
        return 2;

    int iAdv = rkOldPick.GetValue(0) / 10;

    if (rkOldPick.IsEquipped() == true)
        return 2;

    if (Pick_IsRefineSuccess(rkOldPick)) {
        rkLogMgr.RefineLog(ch->GetPlayerID(), rkOldPick.GetName(),
                           rkOldPick.GetID(), iAdv, 1, "PICK");

        CItem* pkNewPick =
            ITEM_MANAGER::instance().CreateItem(rkOldPick.GetRefinedVnum(), 1);
        if (pkNewPick) {
            uint16_t wCell = rkOldPick.GetCell();
            rkItemMgr.RemoveItem(item, "REMOVE (REFINE PICK)");
            pkNewPick->AddToCharacter(ch, TItemPos(INVENTORY, wCell));
            LogManager::instance().ItemLog(ch, pkNewPick, "REFINE PICK SUCCESS",
                                           pkNewPick->GetName());
            return 1;
        }

        return 2;
    } else {
        rkLogMgr.RefineLog(ch->GetPlayerID(), rkOldPick.GetName(),
                           rkOldPick.GetID(), iAdv, 0, "PICK");

        CItem* pkNewPick =
            ITEM_MANAGER::instance().CreateItem(rkOldPick.GetValue(4), 1);

        if (pkNewPick) {
            uint16_t wCell = rkOldPick.GetCell();
            rkItemMgr.RemoveItem(item, "REMOVE (REFINE PICK)");
            pkNewPick->AddToCharacter(ch, TItemPos(INVENTORY, wCell));
            rkLogMgr.ItemLog(ch, pkNewPick, "REFINE PICK FAIL",
                             pkNewPick->GetName());
            return 0;
        }

        return 2;
    }
}

void CHEAT_MAX_PICK(CHARACTER* ch, CItem* item)
{
    if (!item)
        return;

    if (!Pick_Check(*item))
        return;

    CItem& pick = *item;
    Pick_MaxCurExp(pick);

    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                       "곡괭이 수련도가 최대(%d)가 되었습니다.",
                       Pick_GetCurExp(pick));
}

void PracticePick(CHARACTER* ch, CItem* item)
{
    if (!item)
        return;

    if (!Pick_Check(*item))
        return;

    CItem& pick = *item;
    uint32_t refinedVnum = pick.GetRefinedVnum();
    if (refinedVnum <= 0 || refinedVnum == pick.GetVnum())
        // We dont want to increase points on the last pick (it would display
        // 0/0 anyway)
        return;

    if (Pick_IsPracticeSuccess(pick)) {
        if (Pick_Refinable(pick)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "곡괭이가 최대 수련도에 도달하였습니다.");
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "나무꾼를 통해 다음 레벨의 곡괭이로 업그레이드 "
                               "할 수 있습니다.");
        } else {
            Pick_IncCurExp(pick);

            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "곡괭이의 수련도가 증가하였습니다! (%d/%d)",
                               Pick_GetCurExp(pick), Pick_GetMaxExp(pick));

            if (Pick_Refinable(pick)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "곡괭이가 최대 수련도에 도달하였습니다.");
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "나무꾼를 통해 다음 레벨의 곡괭이로 "
                                   "업그레이드 할 수 있습니다.");
            }
        }
    }
}

// END_OF_REFINE_PICK

EVENTFUNC(mining_event)
{
    mining_event_info* info = static_cast<mining_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("mining_event_info> <Factor> Null pointer");
        return 0;
    }

    CHARACTER* ch = g_pCharManager->FindByPID(info->pid);
    CHARACTER* load = g_pCharManager->Find(info->vid_load);

    if (!ch)
        return 0;

    ch->mining_take();

    CItem* pick = ch->GetWear(WEAR_WEAPON);

    // REFINE_PICK
    if (!pick || !Pick_Check(*pick)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "곡괭이를 들고 있지 않아서 캘 수 없습니다.");
        return 0;
    }
    // END_OF_REFINE_PICK

    if (!load) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "더이상 캐낼 수 없습니다.");
        return 0;
    }

    int iPct = GetOrePct(ch);

    if (Random::get(1, 100) <= iPct) {
        OreDrop(ch, load->GetRaceNum());
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "채광에 성공하였습니다.");
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "채광에 실패하였습니다.");
    }

    PracticePick(ch, pick);

    return 0;
}

LPEVENT CreateMiningEvent(CHARACTER* ch, CHARACTER* load, int count)
{
    mining_event_info* info = AllocEventInfo<mining_event_info>();
    info->pid = ch->GetPlayerID();
    info->vid_load = load->GetVID();

    return event_create(mining_event, info, THECORE_SECS_TO_PASSES(2 * count));
}

bool OreRefine(CHARACTER* ch, CHARACTER* npc, CItem* item, Gold cost, int pct,
               CItem* metinstone_item)
{
    if (!ch || !npc)
        return false;

    if (item->GetOwner() != ch) {
        SPDLOG_ERROR("wrong owner");
        return false;
    }

    if (item->GetCount() < ORE_COUNT_FOR_REFINE) {
        SPDLOG_ERROR("not enough count");
        return false;
    }

    uint32_t dwRefinedVnum = GetRefineFromRawOre(item->GetVnum());

    if (dwRefinedVnum == 0)
        return false;

    ch->SetRefineNPC(npc);
    item->SetCount(item->GetCount() - ORE_COUNT_FOR_REFINE);
    Gold refineFee = ch->ComputeRefineFee(cost, 1);

    if (ch->GetGold() < refineFee)
        return false;

    ch->PayRefineFee(refineFee);

    if (metinstone_item)
        ITEM_MANAGER::instance().RemoveItem(metinstone_item, "REMOVE (MELT)");

    if (Random::get(1, 100) <= pct) {
        ch->AutoGiveItem(dwRefinedVnum, 1);
        return true;
    }

    return false;
}

bool IsVeinOfOre(uint32_t vnum)
{
    for (int i = 0; i < MAX_ORE; i++) {
        if (info[i].dwLoadVnum == vnum)
            return true;
    }
    return false;
}

#ifdef ENABLE_GEM_SYSTEM
bool GemRefine(CHARACTER* ch, CHARACTER* npc, CItem* item, int cost,
               CItem* metinstone_item)
{
    if (!ch || !npc)
        return false;

    if (item->GetOwner() != ch) {
        SPDLOG_ERROR("wrong owner");
        return false;
    }

    if (item->GetCount() < 10) {
        SPDLOG_ERROR("not enough count");
        return false;
    }

    ch->SetRefineNPC(npc);
    item->SetCount(item->GetCount() - 10);

    if (ch->GetGold() < cost)
        return false;

    ch->ChangeGold(-cost);

    if (metinstone_item)
        ITEM_MANAGER::instance().RemoveItem(metinstone_item, "REMOVE "
                                                             "(GLIMERSTONE)");

    if (Random::get(1, 100) <= 30) {
        ch->ChangeGem(1);
        return true;
    }

    return false;
}
#endif

bool LoadMiningExtraDrops(const std::string& filename)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(filename)) {
        return false;
    }

    for (const auto& p : reader.GetChildren()) {
        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        auto grp = static_cast<GroupTextGroup*>(node);

        uint32_t pickVnum;
        if (!GetGroupProperty(grp, "PickVnum", pickVnum)) {
            SPDLOG_ERROR("LoadMiningExtraDrops : Syntax error {0} : no "
                         "PickVnum, node {1}",
                         filename, p.first);
            return false;
        }

        auto items = grp->GetList("Items");
        if (items) {
            auto pkGroup = std::make_unique<CSpecialItemGroup>(
                pickVnum, CSpecialItemGroup::NORMAL);

            std::vector<storm::StringRef> tokens;
            int32_t lineNum = 1;
            for (const auto& line : items->GetLines()) {
                tokens.clear();
                storm::ExtractArguments(line, tokens);

                if (tokens.size() < 3) {
                    SPDLOG_WARN("ReadSpecialDropItemFile Ignoring line {} in "
                                "Group {} invalid token count atleast 3 tokens "
                                "required",
                                lineNum, p.first);
                    continue;
                }

                uint32_t dwVnum = 0;

                std::string name(tokens[0].data(), tokens[0].size());

                if (!ITEM_MANAGER::instance().GetVnumByOriginalName(
                        name.c_str(), dwVnum)) {
                    dwVnum = CSpecialItemGroup::NONE;
                    if (!name.empty()) {

                        if (!storm::ParseNumber(name, dwVnum)) {
                            SPDLOG_ERROR("Group {0}: Invalid vnum {1}",
                                         grp->GetName(), name);
                            return false;
                        }
                        if (!ITEM_MANAGER::instance().GetTable(dwVnum)) {
                            SPDLOG_ERROR("Group {0}: Item {1} does not exist",
                                         grp->GetName(), name);
                            return false;
                        }
                    }
                }

                SocketValue iCount = 0;
                storm::ParseNumber(tokens[1], iCount);

                int iProb = 0;
                storm::ParseNumber(tokens[2], iProb);

                int iRarePct = 0;
                if (tokens.size() > 3) {
                    storm::ParseNumber(tokens[3], iRarePct);
                }

                pkGroup->AddItem(dwVnum, iCount, iProb, iRarePct);
            }

            if (pkGroup)
                miningExtraDropInfo.emplace(pickVnum, std::move(pkGroup));
        }
    }
    return true;
}

} // namespace mining
