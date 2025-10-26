#include "cube.h"
#include "log.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "battle_pass.h"
#include "questmanager.h"

#include <base/ClientDb.hpp>
#include <game/CubeTypes.hpp>

#define RETURN_IF_CUBE_IS_NOT_OPENED(ch) if (!(ch)->IsCubeOpen()) return

/*--------------------------------------------------------*/
/*                   GLOBAL VARIABLES                     */
/*--------------------------------------------------------*/
static std::unordered_map<GenericVnum, CubeProto> s_cube_proto;
static bool s_isInitializedCubeMaterialInformation = false;

// Å¥ºê¸¦ µ¹·ÈÀ»¶§ ³ª¿À´Â ¾ÆÀÌÅÛÀÇ Á¾·ù¸¦ °áÁ¤ÇÔ

Gold GetGoldValue(const CubeProto &recipe)
{
    const auto craftEvent = quest::CQuestManager::instance().GetEventFlag("craft_event");
    if (craftEvent) { return recipe.cost - (recipe.cost / 100 * std::clamp(craftEvent, 1, 25)); }
    return recipe.cost;
}

// Å¥ºêÃ¢ ¿­±â
void Cube_open(CHARACTER *ch)
{
    if (nullptr == ch)
        return;

    if (ch->IsCubeOpen())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이미 제조창이 열려있습니다.");
        return;
    }
    if (ch->GetExchange() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "다른 거래중(창고,교환,상점)에는 사용할 수 없습니다.");
        return;
    }

    ch->SetCubeOpen(true);
    ch->ChatPacket(CHAT_TYPE_COMMAND, "cube open");
}

// Å¥ºê Äµ½½
void Cube_close(CHARACTER *ch)
{
    RETURN_IF_CUBE_IS_NOT_OPENED(ch);
    ch->SetCubeOpen(false);
    ch->ChatPacket(CHAT_TYPE_COMMAND, "cube close");
}

bool Cube_init()
{
    std::vector<CubeProto> vc;
    if (!LoadClientDbFromFile<CubeProto>("data/cube_proto", vc))
        return false;

    s_cube_proto.clear();

    for (const auto &c : vc) { s_cube_proto.emplace(c.vnum, c); }

    return true;
}

// return new item
bool Cube_make(CHARACTER *ch, uint32_t cubeVnum)
{
    if (!(ch)->IsCubeOpen())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "제조창이 열려있지 않습니다");
        return false;
    }

    const auto emptyInven = ch->GetEmptyInventoryCount(3);
    if (emptyInven < 1)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You need at least 3 free inventory slots to craft items.");
        return false;
    }

    const auto it = s_cube_proto.find(cubeVnum);
    if (it == s_cube_proto.end())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Receipe could not be found.");
        return false;
    }

    const auto &recipe = it->second;
    if (ch->GetGold() < recipe.cost)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "돈이 부족하거나 아이템이 제자리에 없습니다."); // 이 텍스트는 이미 널리 쓰이는거라 추가번역 필요 없음
        return false;
    }

    const auto &reward_value = recipe.reward;
#ifndef ENABLE_CUBE_SELECT_REMOVE_ITEMS_ON_FAILURE
	cube_proto->remove_material(ch);
#endif

    auto* table = ITEM_MANAGER::instance().GetTable(reward_value.vnum);
    if(!table) {
      return false;
    }

    if(auto val = ch->GetEmptyInventory(table->bSize, table->bType); val == -1) {
      return false;
    }

    std::vector<CItem *> matTransfer;
    for (const auto &recMat : recipe.items)
    {
        if (recMat.transferBonus)
        {
            const auto item = ch->FindSpecifyItem(recMat.vnum);
            if (!item)
            {
                SendSpecialI18nChatPacket(
                    ch, CHAT_TYPE_INFO,
                    fmt::format("Missing %s to create the recipe you have chosen#{}", TextTag::itemname(recMat.vnum)));
                return false;
            }

            matTransfer.push_back(item);
            
        }
        auto count = ch->CountSpecifyItem(recMat.vnum);
        if (count < recMat.count)
        {
            SendSpecialI18nChatPacket(ch, CHAT_TYPE_INFO,
                                      fmt::format("Missing %s %s to create the recipe you have chosen#{};{}",
                                                  recMat.count - count, TextTag::itemname(recMat.vnum)));
            return false;
        }
    }

    if (Random::get(1, 100) <= recipe.probability)
    {
        auto *newItem = ch->AutoGiveItem(reward_value.vnum, reward_value.count, -1, false);
        if (!newItem)
        {
            SPDLOG_ERROR("Cube failed to make: %d could not create item.", reward_value.vnum);
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Sorry I failed to create the recipe you wanted me to make");
            ch->ChatPacket(CHAT_TYPE_COMMAND, "cube fail");
            LogManager::instance().CubeLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(),
                                           reward_value.vnum, 0, 0, false);
            return false;
        }

        ch->ChatPacket(CHAT_TYPE_COMMAND, "cube success %d %d", reward_value.vnum, reward_value.count);

#ifdef ENABLE_CUBE_TRANSFER_BONUSES
        for (const auto &recMat : recipe.items)
        {
            for (const auto &mat : matTransfer)
            {
                if (mat->GetVnum() == recMat.vnum)
                {
                    mat->CopyAttributeTo(newItem);
                    mat->CopySocketTo(newItem);
                }
            }
        }
#endif
        newItem->UpdatePacket();
        ITEM_MANAGER::instance().SaveSingleItem(newItem);

        if (0 < recipe.cost)
        {
#ifdef ENABLE_BATTLE_PASS
            uint8_t bBattlePassId = ch->GetBattlePassId();
            if (bBattlePassId)
            {
                uint32_t dwYangCount, dwNotUsed;
                if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, SPENT_YANG, &dwNotUsed, &dwYangCount)
                )
                {
                    if (ch->GetMissionProgress(SPENT_YANG, bBattlePassId) < dwYangCount)
                        ch->UpdateMissionProgress(SPENT_YANG, bBattlePassId, recipe.cost, dwYangCount);
                }
            }
#endif
            ch->ChangeGold(-recipe.cost);
        }

#ifdef ENABLE_CUBE_SELECT_REMOVE_ITEMS_ON_FAILURE
        for (const auto &recMat : recipe.items)
        {
            ch->RemoveSpecifyItem(recMat.vnum, recMat.count);

        }
#endif

      
 #ifdef ENABLE_BATTLE_PASS
    uint8_t bBattlePassId =  ch->GetBattlePassId();
    if (bBattlePassId)
    {
        uint32_t dwItemVnum, dwUseCount;
        if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, CRAFT_ITEM, &dwItemVnum, &dwUseCount))
        {
            if (dwItemVnum == reward_value.vnum && ch->GetMissionProgress(CRAFT_ITEM, bBattlePassId) < dwUseCount)
                 ch->UpdateMissionProgress(CRAFT_ITEM, bBattlePassId, 1, dwUseCount);
        }
    }
#endif

        LogManager::instance().CubeLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(),
                                       reward_value.vnum, newItem->GetID(), reward_value.count, 1);

        return true;
    }

#ifdef ENABLE_CUBE_SELECT_REMOVE_ITEMS_ON_FAILURE
    for (const auto &recMat : recipe.items)
    {
        if (recMat.removeAtFailure)
        {
            ch->RemoveSpecifyItem(recMat.vnum, recMat.count);
        }
    }
#endif

    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "제조에 실패하였습니다."); // 2012.11.12 새로 추가된 메세지 (locale_string.txt 에 추가해야 함)
    ch->ChatPacket(CHAT_TYPE_COMMAND, "cube fail");
    LogManager::instance().CubeLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(),
                                   reward_value.vnum, 0, 0, 0);
    return false;
}
