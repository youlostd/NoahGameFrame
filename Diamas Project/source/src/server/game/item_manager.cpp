#include "item_manager.h"
#include "DbCacheSocket.hpp"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "item.h"
#include "log.h"
#include "priv_manager.h"
#include "questmanager.h"
#include "safebox.h"
#include "skill.h"

#include "utils.h"

#include "DragonSoul.h"
#include "ItemUtils.h"
#include "desc.h"
#include "exchange.h"
#include "mob_manager.h"
#include "shop.h"
#include <base/ClientDb.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include <storm/io/TextFileLoader.hpp>


#include "GRefineManager.h"

ITEM_MANAGER::ITEM_MANAGER()
    : m_iTopOfTable(0), m_dwVIDCount(0), m_ItemIDRange(kNullRange), m_ItemIDSpareRange(kNullRange)
{
    // ctor
}

ITEM_MANAGER::~ITEM_MANAGER()
{
    Destroy();
}

void ITEM_MANAGER::Destroy()
{
    for (auto & [fst, snd] : m_VIDMap)
    {
        delete (snd);
    }
    m_VIDMap.clear();
}

void ITEM_MANAGER::GracefulShutdown()
{
    auto it = m_set_pkItemForDelayedSave.begin();

    while (it != m_set_pkItemForDelayedSave.end())
        SaveSingleItem(*(it++));

    m_set_pkItemForDelayedSave.clear();
}

bool ITEM_MANAGER::ReloadItemProto()
{
    std::vector<TItemTable> v;
    if (!LoadClientDbFromFile("data/item_proto_server", v))
        return false;

    m_itemProto.clear();
    m_itemNames.clear();
    m_map_ItemRefineFrom.clear();

    for (const auto &item : v)
    {
        if (item.dwVnumRange)
        {
            for (int i = item.dwVnum; i <= item.dwVnum + item.dwVnumRange; ++i)
            {
                TItemTable t = item;
                t.dwVnum = i;
                m_itemProto.emplace(i, t);
            }
        }
        else
        {
            m_itemProto.emplace(item.dwVnum, item);
        }
    }

    for (auto &it : m_itemProto)
    {
        if (it.second.bType == ITEM_QUEST ||
            IS_SET(it.second.dwFlags, ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
            quest::CQuestManager::instance().RegisterNPCVnum(it.second.dwVnum);

        if (it.second.dwRefinedVnum)
        {
            auto fit = m_map_ItemRefineFrom.find(it.second.dwRefinedVnum);
            if (fit != m_map_ItemRefineFrom.end())
            {
                SPDLOG_WARN("Item {0} has multiple refined from items {1} {2}", it.second.dwRefinedVnum, it.first,
                            fit->second);
            }

            m_map_ItemRefineFrom.emplace(it.second.dwRefinedVnum, it.first);
        }
    }
    return true;
}

bool ITEM_MANAGER::Initialize()
{
    std::vector<TItemTable> v;
    if (!LoadClientDbFromFile("data/item_proto_server", v))
        return false;

    DSManager::instance().ClearDragonSoulTable();

    m_itemProto.clear();
    m_itemNames.clear();
    for (auto &dropCat : m_commonDropItem)
        dropCat.clear();
    m_map_dwEtcItemDropProb.clear();
    m_map_pkDropItemGroup.clear();
    m_map_pkSpecialItemGroup.clear();
    m_map_pkQuestItemGroup.clear();
    m_map_pkSpecialAttrGroup.clear();
    m_map_pkMobItemGroup.clear();
    m_map_pkLevelItemGroup.clear();
    m_map_pkDropInfoByVNum.clear();
    m_map_petEvolutionFromToVnum.clear();
    m_enhanceExceptionVnums.clear();
    m_map_new_to_ori.clear();
    m_map_ori_to_debuffed.clear();

    for (const auto &item : v)
    {
        if (item.dwVnumRange)
        {
            for (int i = item.dwVnum; i <= item.dwVnum + item.dwVnumRange; ++i)
            {
                TItemTable t = item;
                t.dwVnum = i;
                m_itemProto.emplace(i, t);
            }
        }
        else
        {
            m_itemProto.emplace(item.dwVnum, item);
        }
    }

    if (!LoadNames(GetLocaleService().GetDefaultLocale().path + "/item_desc.txt"))
        return false;

    if (!ReadCommonDropItemFile("data/common_drop_item.txt"))
        return false;

    if (!ReadEtcDropItemFile("data/etc_drop_item.txt"))
        return false;

    if (!ReadAddSocketBlock("data/socket_add_block.txt", false))
        return false;

    if (!ReadSpecialDropItemFile("data/special_item_group.txt"))
        return false;

    if (!ReadMonsterDropItemGroup("data/mob_drop_item.txt"))
        return false;

    std::vector<ItemAttrProto> va;
    if (!LoadClientDbFromFile<ItemAttrProto>("data/item_attr_proto_server", va))
        return false;

    for (auto &item : va)
        m_itemAttr.emplace(item.apply, item);

    va.clear();

    if (!LoadClientDbFromFile<ItemAttrProto>("data/item_attr_rare_proto_server", va))
        return false;

    for (auto &item : va)
        m_itemAttrRare.emplace(item.apply, item);

    const char *szItemVnumMaskTableFileName = "data/ori_to_new_table.txt";
    const char *szDragonSoulTableFileName = "data/dragon_soul_table.txt";

    if (!ReadDoubleItemAttrAllow("data/item_attr_double_allow.txt"))
    {
        SPDLOG_ERROR("cannot load ReadDoubleItemAttrAllow: "
                     "data/item_attr_double_allow.txt");
        return false;
    }

    if (!ReadEnhanceTimeExeption("data/enhance_time_exception.txt"))
    {
        SPDLOG_ERROR("cannot load EnhanceTimeExeption: data/enhance_time_exception.txt");
        return false;
    }

    if (!ReadAttackPetDropData("data/attack_pet_drop.txt"))
    {
        SPDLOG_ERROR("cannot load ReadAttackPetDropData: data/attack_pet_drop.txt");
        return false;
    }

    if (!ReadLevelPetData("data/togggle_level_pet.txt"))
    {
        SPDLOG_ERROR("cannot load ReadLevelPetData: data/attack_pet_drop.txt");
        return false;
    }

    if (!ReadPetItemEvolutionTable("data/pet_item_evolution.txt", false))
    {
        SPDLOG_ERROR("cannot load PetItemEvolutionTable: data/pet_item_evolution.txt");
        return false;
    }

    if (!ReadItemVnumMaskTable(szItemVnumMaskTableFileName))
    {
        SPDLOG_TRACE("Could not open MaskItemTable");
    }

    if (!ReadItemDebuffTable(" data/item_debuff.txt"))
    {
        SPDLOG_TRACE("Could not open ItemDebuffTable");
    }

    if (!DSManager::instance().ReadDragonSoulTableFile(szDragonSoulTableFileName))
    {
        SPDLOG_ERROR("cannot load DragonSoulTable: {0}", szDragonSoulTableFileName);
    }

    // END_OF_LOCALE_SERVICE

    for (auto &it : m_itemProto)
    {
        if (it.second.bType == ITEM_QUEST ||
            IS_SET(it.second.dwFlags, ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
            quest::CQuestManager::instance().RegisterNPCVnum(it.second.dwVnum);

        if (it.second.dwRefinedVnum)
        {
            auto fit = m_map_ItemRefineFrom.find(it.second.dwRefinedVnum);
            if (fit != m_map_ItemRefineFrom.end())
            {
                SPDLOG_WARN("Item {0} has multiple refined from items {1} {2}", it.second.dwRefinedVnum, it.first,
                            fit->second);
            }

            m_map_ItemRefineFrom.emplace(it.second.dwRefinedVnum, it.first);
        }
    }

    SPDLOG_TRACE("Item proto loading");

    m_isInitialized = true;
    return true;
}

CItem *ITEM_MANAGER::CreateItem(uint32_t vnum, CountType count, uint32_t id, bool bTryMagic, int iRarePct,
                                bool bSkipSave)
{
    if (0 == vnum)
        return nullptr;

    uint32_t dwMaskVnum = 0;

    if (GetMaskVnum(vnum))
    {
        dwMaskVnum = GetMaskVnum(vnum);
    }

    const TItemTable *table = GetTable(vnum);

    if (!table)
        return nullptr;

    if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
    {
        CItem *item = m_map_pkItemByID[id];
        CHARACTER *owner = item->GetOwner();
        SPDLOG_ERROR("ITEM_ID_DUP: {} {} owner {:p}", id, item->GetName(), fmt::ptr(owner));
        return nullptr;
    }

    const bool bIsNewItem = (0 == id);
    //아이템 하나 할당하고
    auto* item = new CItem(table->dwVnum);

    if (item->GetItemType() == ITEM_ELK)
    {
        item->SetSkipSave(true);
    }
    else if (!bIsNewItem)
    {
        item->SetID(id);
        item->SetSkipSave(true);
    }
    else
    {
        item->SetID(GetNewID());
    }

    item->SetMaskVnum(dwMaskVnum);

    if (item->GetItemType() == ITEM_ELK)
    {
        // Money does not need any treatment
    }
    else if (item->IsStackable())
    {
        // For items that can be combined
        count = std::clamp<CountType>(count, 1, GetItemMaxCount(item));
        if (bTryMagic && count <= 1 && IS_SET(item->GetFlag(), ITEM_FLAG_MAKECOUNT))
            count = item->GetValue(1);
    }
    else
    {
        count = 1;
    }

    item->SetVID(++m_dwVIDCount);

    if (!bSkipSave)
        m_VIDMap.emplace(item->GetVID(), item);

    if (item->GetID() != 0 && !bSkipSave)
        m_map_pkItemByID.emplace(item->GetID(), item);

    if (!item->SetCount(count))
        return nullptr;

    item->SetSkipSave(false);

    if (bIsNewItem)
    {
        if (item->IsCostumeAcce())
        {
            bTryMagic = false;
        }

        events::Item::OnCreate(item);
        item->SetSealDate(0);

        if (table->sAddonType)
            item->ApplyAddon(table->sAddonType);

        if (bTryMagic)
        {
            if (iRarePct == -1)
                iRarePct = table->bAlterToMagicItemPct;

            if (Random::get(1, 100) <= iRarePct)
                item->AlterToMagicItem();
        }

        if (table->bGainSocketPct && !item->IsCostumeAcce())
            item->AlterToSocketItem(table->bGainSocketPct);

        // 50300 == 기술 수련서
        if (vnum == 50300 || vnum == ITEM_SKILLFORGET_VNUM)
        {
            uint32_t dwSkillVnum;

            do
            {
#ifdef ENABLE_WOLFMAN
                dwSkillVnum = Random::get(1, 175);
                if (dwSkillVnum > 111)
                    dwSkillVnum = Random::get(170, 175);
                else
#endif
                    dwSkillVnum = Random::get(1, 111);

                if (nullptr != CSkillManager::instance().Get(dwSkillVnum))
                    break;
            } while (true);

            item->SetSocket(0, dwSkillVnum);
        }
    }

    if (item->GetItemType() == ITEM_QUEST)
    {
        for (auto it = m_map_pkQuestItemGroup.begin(); it != m_map_pkQuestItemGroup.end(); it++)
        {
            if (it->second->m_bType == CSpecialItemGroup::QUEST && it->second->Contains(vnum))
            {
                item->SetSIGVnum(it->first);
            }
        }
    }
    else if (item->GetItemType() == ITEM_UNIQUE)
    {
        for (auto it = m_map_pkSpecialItemGroup.begin(); it != m_map_pkSpecialItemGroup.end(); it++)
        {
            if (it->second->m_bType == CSpecialItemGroup::SPECIAL && it->second->Contains(vnum))
            {
                item->SetSIGVnum(it->first);
            }
        }
    }
    else if (item->GetItemType() == ITEM_GACHA)
        item->SetSocket(0, item->GetLimitValue(1) * count);

    // 새로 생성되는 용혼석 처리.
    if (item->IsDragonSoul() && 0 == id)
    {
        DSManager::instance().DragonSoulItemInitialize(item);
    }
    return item;
}

void ITEM_MANAGER::DelayedSave(CItem *item)
{
    if (item->GetID() != 0)
        m_set_pkItemForDelayedSave.insert(item);
}

void ITEM_MANAGER::FlushDelayedSave(CItem *item)
{
    auto it = m_set_pkItemForDelayedSave.find(item);
    if (it == m_set_pkItemForDelayedSave.end())
        return;

    m_set_pkItemForDelayedSave.erase(it);
    SaveSingleItem(item);
}

void ITEM_MANAGER::SaveSingleItem(CItem *item)
{
    if (!item)
        return;

    if (!item->GetOwner())
    {
        uint32_t dwID = item->GetID();
        uint32_t dwOwnerID = item->GetLastOwnerPID();

        db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
        db_clientdesc->Packet(&dwID, sizeof(uint32_t));
        db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));

        SPDLOG_TRACE("ITEM_DELETE {}:{}", item->GetName(), dwID);
        return;
    }
    SPDLOG_TRACE("ITEM_MANAGER::SaveSingleItem this {:p} item {:p} vid {} owner {:p} ownerPid {}", (void *)this,
                (void *)item, item->GetVID(), (void *)item->GetOwner(), item->GetOwnerPid());

    // SPDLOG_TRACE( "ITEM_SAVE {}:{} in {} window {}", item->GetName(), item->GetID(), item->GetOwner()->GetName(),
    // item->GetWindow());

    TPlayerItem t{};

    t.id = item->GetID();
    t.window = item->GetWindow();
    t.pos = item->GetCell();
    t.data.count = item->GetCount();
    t.data.vnum = item->GetOriginalVnum();
    t.data.transVnum = item->GetTransmutationVnum();
    t.data.nSealDate = item->GetSealDate();
    t.data.id = item->GetID();

    switch (t.window)
    {
#ifdef ENABLE_GUILD_STORAGE
    case GUILDSTORAGE: {
        if (item->GetOwner()->GetGuild() != NULL)
            t.owner = item->GetOwner()->GetGuild()->GetID();
    }
    break;
#endif

    case SAFEBOX:
    case MALL:
        t.owner = item->GetOwner()->GetDesc()->GetAid();
        break;
    default:
        t.owner = item->GetOwner()->GetPlayerID();
        break;
    }
    t.is_gm_owner = item->IsGMOwner();
    t.is_blocked = item->IsBlocked();

    t.price = item->GetPrivateShopPrice();

    std::memcpy(t.data.sockets, item->GetSockets(), sizeof(t.data.sockets));
    std::memcpy(t.data.attrs, item->GetAttributes(), sizeof(t.data.attrs));

    db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
    db_clientdesc->Packet(&t, sizeof(TPlayerItem));
}

void ITEM_MANAGER::Update()
{
    if (!m_isInitialized)
        return;

    for (auto item : m_set_pkItemForDelayedSave)
    {
        SaveSingleItem(item);
    }

    m_set_pkItemForDelayedSave.clear();
}

void ITEM_MANAGER::RemoveItem(CItem *item, const char *c_pszReason)
{
    auto *o = item->GetOwner();

    if (o)
    {
        const auto szHint = fmt::format("{} {} ", item->GetName(), item->GetCount());
        LogManager::instance().ItemLog(o, item, c_pszReason ? c_pszReason : "REMOVE", szHint.c_str());

        item->m_onDestroy();

        if (item->GetWindow() == MALL || item->GetWindow() == SAFEBOX)
        {
            auto *pSafebox = item->GetWindow() == MALL ? o->GetMall() : o->GetSafebox();
            if (pSafebox)
                pSafebox->Remove(item->GetCell());
        }
        else
        {
            events::Item::OnRemove(o, item);
            item->RemoveFromCharacter();
        }

        // Clear from quests if set
        auto *pPc = quest::CQuestManager::instance().GetPCForce(o->GetPlayerID());
        if (pPc && pPc->IsRunning())
        {
            auto *questState = pPc->GetRunningQuestState();
            if (questState && questState->item != 0 && Find(questState->item) == item)
                questState->item = 0;
        }
    }

    M2_DESTROY_ITEM(item);
}

void ITEM_MANAGER::DestroyItem(CItem *item)
{
    if (!item)
        return;

    rmt_ScopedCPUSample(ItemDestroy, 0);

    if (item->GetSectree())
        item->RemoveFromGround();

    if (item->GetOwner())
    {
        if (g_pCharManager->Find(item->GetOwner()->GetPlayerID()) != nullptr)
        {
            SPDLOG_CRITICAL("DestroyItem ({}): GetOwner {} {}!", item->GetID(), item->GetName(),
                         item->GetOwner()->GetName());
            item->RemoveFromCharacter();
        }
        else
        {
            SPDLOG_CRITICAL("Item ({}): WTH! Invalid item owner. owner pointer : {:p}", item->GetID(),
                         fmt::ptr(item->GetOwner()));
        }
    }

    m_set_pkItemForDelayedSave.erase(item);

    uint32_t dwID = item->GetID();

    if (!item->GetSkipSave() && dwID)
    {
        uint32_t dwOwnerID = item->GetLastOwnerPID();

        db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
        db_clientdesc->Packet(&dwID, sizeof(uint32_t));
        db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));
    }

    if (dwID)
        m_map_pkItemByID.erase(dwID);

    m_VIDMap.erase(item->GetVID());
    delete item;
}

bool ITEM_MANAGER::RemoveMapItem(uint32_t id)
{
    const auto it = m_map_pkItemByID.find(id);
    if (it == m_map_pkItemByID.end())
        return false;
    m_map_pkItemByID.erase(it);
    return true;
}

CItem *ITEM_MANAGER::Find(uint32_t id)
{
    const auto it = m_map_pkItemByID.find(id);
    if (it == m_map_pkItemByID.end())
        return nullptr;
    return it->second;
}

CItem *ITEM_MANAGER::FindByVID(uint32_t vid)
{
    auto it = m_VIDMap.find(vid);

    if (it == m_VIDMap.end())
        return nullptr;

    return (it->second);
}

const TItemTable *ITEM_MANAGER::GetTable(uint32_t vnum)
{
    const auto it = m_itemProto.find(vnum);
    if (it != m_itemProto.end())
        return &it->second;

    return nullptr;
}

bool ITEM_MANAGER::GetVnum(const char *c_pszName, uint32_t &r_dwVnum)
{
    auto f = [c_pszName](const NameMap::value_type &p) -> bool { return boost::starts_with(p.second, c_pszName); };

    auto it = std::find_if(m_itemNames.begin(), m_itemNames.end(), f);
    if (it != m_itemNames.end())
    {
        r_dwVnum = it->first;
        return true;
    }

    return false;
}

const char *ITEM_MANAGER::GetName(uint32_t vnum) const
{
    auto it = m_itemNames.find(vnum);
    if (it != m_itemNames.end())
        return it->second.c_str();

    return "";
}

bool ITEM_MANAGER::GetVnumByOriginalName(const char *c_pszName, uint32_t &r_dwVnum)
{
    const auto f = [c_pszName](const ProtoMap::value_type &p) -> bool { return !strcmp(p.second.szName, c_pszName); };

    const auto it = std::find_if(m_itemProto.cbegin(), m_itemProto.cend(), f);
    if (it != m_itemProto.end())
    {
        r_dwVnum = it->first;
        return true;
    }

    return false;
}


// 20050503.ipkn.
// iMinimum 보다 작으면 iDefault 세팅 (단, iMinimum은 0보다 커야함)
// 1, 0 식으로 ON/OFF 되는 방식을 지원하기 위해 존재
int GetDropPerKillPct(int iMinimum, int iDefault, int iDeltaPercent, const char *c_pszFlag)
{
    int iVal = 0;

    if ((iVal = quest::CQuestManager::instance().GetEventFlag(c_pszFlag)))
    {
        if (!gConfig.testServer)
        {
            if (iVal < iMinimum)
                iVal = iDefault;

            if (iVal < 0)
                iVal = iDefault;
        }
    }

    if (iVal == 0)
        return 0;

    // 기본 세팅일때 (iDeltaPercent=100)
    // 40000 iVal 마리당 하나 느낌을 주기 위한 상수임
    return (40000 * iDeltaPercent / iVal);
}

bool ITEM_MANAGER::GetDropPct(CHARACTER *pkChr, CHARACTER *pkKiller, int &iDeltaPercent, int &iRandRange)
{
    if (nullptr == pkChr || nullptr == pkKiller)
        return false;

    int iLevel = pkKiller->GetLevel();
    iDeltaPercent = 100;

#ifndef L3_EAZY
    if (!pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS)
        iDeltaPercent = PERCENT_LVDELTA_BOSS(pkKiller->GetLevel(), pkChr->GetLevel());
    else
        iDeltaPercent = PERCENT_LVDELTA(pkKiller->GetLevel(), pkChr->GetLevel());
#endif

    uint8_t bRank = pkChr->GetMobRank();

    if (1 == Random::get(1, 50000))
        iDeltaPercent += 1000;
    else if (1 == Random::get(1, 10000))
        iDeltaPercent += 500;

    SPDLOG_TRACE("CreateDropItem for level: %d rank: %u pct: %d", iLevel, bRank, iDeltaPercent);
    iDeltaPercent = iDeltaPercent * g_pCharManager->GetMobItemRate(pkKiller) / 100;

    if (pkKiller->GetPoint(POINT_MALL_ITEMBONUS) > 0)
        iDeltaPercent += iDeltaPercent * pkKiller->GetPoint(POINT_MALL_ITEMBONUS) / 100;

    // ADD_PREMIUM
    if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||  // mall
        pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM)) // any kind of equipped gloves
        iDeltaPercent += iDeltaPercent;
    // END_OF_ADD_PREMIUM

    int bonus = 0;
    if (pkKiller->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_ITEM) && pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
    {
        // irremovable gloves + mall
        bonus = 100;
    }
    else if (pkKiller->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_ITEM) ||
             (pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM) &&
              pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0))
    {
        // irremovable gloves OR removable gloves + mall
        bonus = 50;
    }

    const auto itemDropBonus = std::clamp<PointValue>(pkKiller->GetPoint(POINT_ITEM_DROP_BONUS), 0, 100);

    iRandRange = 4000000;
    iRandRange =
        iRandRange * 100 / (100 + CPrivManager::instance().GetPriv(pkKiller, PRIV_ITEM_DROP) + bonus + itemDropBonus);
    return true;
}

bool ITEM_MANAGER::CreateDropItem(CHARACTER *pkChr, CHARACTER *pkKiller, std::vector<CItem *> &vec_item)
{


    // Think 19/03/14 - Drop debug system in syslog
    bool drop_debug = (gConfig.testServer && quest::CQuestManager::instance().GetEventFlag("drop_test"));
    // [more later]

    bool isDoubleCountPriv = CPrivManager::instance().GetPriv(pkKiller, PRIV_DOUBLE_DROP_COUNT);

    int32_t dropCount = isDoubleCountPriv ? 2 : 1;

    int iLevel = pkKiller->GetLevel();
    uint32_t race = pkChr->GetRaceNum();

    int iDeltaPercent, iRandRange;
    if (!GetDropPct(pkChr, pkKiller, iDeltaPercent, iRandRange))
        return false;

    uint8_t bRank = pkChr->GetMobRank();
    CItem *item = nullptr;

    // Common Drop Items
    for (const auto &info : m_commonDropItem[bRank])
    {
        if (iLevel < info.m_iLevelStart || iLevel > info.m_iLevelEnd)
            continue;

        int iPercent = (info.m_iPercent * iDeltaPercent) / 100;
        // SPDLOG_TRACE(fmt::format("CreateDropItem {0} ~ {1} {2}({3})", info.m_iLevelStart, info.m_iLevelEnd,
        // info.m_dwVnum, iPercent, info.m_iPercent).c_str());

        if (iPercent >= Random::get(1, iRandRange))
        {
            auto table = GetTable(info.m_dwVnum);
            if (!table)
                continue;

            for (int i = 0; i < dropCount; ++i)
            {
                item = nullptr;

                if (table->bType == ITEM_POLYMORPH)
                {
                    if (info.m_dwVnum == pkChr->GetPolymorphItemVnum())
                    {
                        item = CreateItem(info.m_dwVnum, info.m_count, 0, true);

                        if (item)
                            item->SetSocket(0, pkChr->GetRaceNum());
                    }
                }
                else
                    item = CreateItem(info.m_dwVnum, info.m_count, 0, true);
            }

            if (item)
                vec_item.push_back(item);
        }
    }

    if (drop_debug)
    {
        SPDLOG_TRACE("====== Drops ====== ##");
        SPDLOG_TRACE("\t iDeltaPercent: %d ##", iDeltaPercent);
        SPDLOG_TRACE("\t iRandRange: %d ##", iRandRange);
    }

    // Attack Pet Drop
    for (const auto &info : m_attackPetItemDropInfos)
    {
        if (info.m_mobVnum != race)
            continue;

        int iPercent = (info.m_dropPct * iDeltaPercent) / 100;

        if (iPercent >= Random::get(1, iRandRange))
        {
            auto table = GetTable(info.m_itemVnum);
            if (!table)
                continue;

            for (int i = 0; i < 1; ++i)
            {
                item = CreateItem(info.m_itemVnum, 1, 0, true);

                if (item)
                {
                    SPDLOG_TRACE("CreateAttackPetDropItem {0}: {1} {2} {3} {5}", info.m_mobVnum, info.m_itemVnum,
                                 info.m_damagePlus, info.m_scalePlus, info.m_dropPct);

                    item->SetSocket(0, info.m_mobVnum);
                    item->SetSocket(1, info.m_scalePlus);
                    item->SetSocket(2, info.m_damagePlus);
                    vec_item.push_back(item);
                }
            }
        }
    }

    // Level Pet Drop
    for (const auto &info : m_levelPetItemDropInfos)
    {
        if (info.m_mobVnum != race)
            continue;

        int iPercent = (info.m_dropPct * iDeltaPercent) / 100;

        if (iPercent >= Random::get(1, iRandRange))
        {
            auto table = GetTable(info.m_itemVnum);
            if (!table)
                continue;

            for (int i = 0; i < 1; ++i)
            {
                item = CreateItem(info.m_itemVnum, 1, 0, true);

                if (item)
                {
                    SPDLOG_TRACE("CreateLevelPetDropItem {}: {} {}", info.m_mobVnum, info.m_itemVnum, info.m_dropPct);

                    item->SetTransmutationVnum(info.m_mobVnum);
                    vec_item.push_back(item);
                }
            }
        }
    }

    // Drop Item Group
    {
        if (drop_debug)
            SPDLOG_TRACE("> Drop item group ##");

        auto it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());

        if (it != m_map_pkDropItemGroup.end())
        {
            if (it->second)
            {
                auto v = it->second->GetVector();

                for (uint32_t i = 0; i < v.size(); ++i)
                {
                    int iPercent = (v[i].dwPct * iDeltaPercent) / 100;
                    int ciRand = Random::get(1, iRandRange);

                    if (drop_debug)
                    {
                        const TItemTable *table = GetTable(v[i].dwVnum);
                        float realPercent = (float)iPercent / iRandRange * 100;

                        SPDLOG_TRACE("\t {}: {}%. {}(iPct: {}, Rnd: {}) ##", table->szLocaleName, realPercent,
                                    (iPercent >= ciRand) ? "DROP! " : "", iPercent, ciRand);
                    }

                    if (iPercent >= ciRand)
                    {
                        for (int j = 0; j < dropCount; ++j)
                        {
                            item = CreateItem(v[i].dwVnum, v[i].iCount, 0, true);

                            if (item)
                            {
                                if (item->GetItemType() == ITEM_POLYMORPH)
                                {
                                    if (item->GetVnum() == pkChr->GetPolymorphItemVnum())
                                    {
                                        item->SetSocket(0, pkChr->GetRaceNum());
                                    }
                                }

                                vec_item.push_back(item);
                            }
                        }
                    }
                }
            }
        }
    }

    // MobDropItem Group
    {
        if (drop_debug)
            SPDLOG_TRACE("> Mob item group ##");

        const auto it = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());
        if (it != m_map_pkMobItemGroup.end())
        {
            auto pGroup = it->second.get();

            // MOB_DROP_ITEM_BUG_FIX
            // 20050805.myevan.MobDropItem 에 아이템이 없을 경우 CMobItemGroup::GetOne() 접근시 문제 발생 수정
            if (pGroup && !pGroup->IsEmpty())
            {
                int iPercent = 40000 * iDeltaPercent / pGroup->GetKillPerDrop();
                int ciRand = Random::get(1, iRandRange);

                if (drop_debug)
                {
                    float realPercent = (float)iPercent / iRandRange * 100;

                    SPDLOG_TRACE("\t All items: {0}. {1}(iPct: {2}, Rnd: {3}) ##", realPercent,
                                (iPercent >= ciRand) ? "DROP ONE! " : "", iPercent, ciRand);
                }

                if (iPercent >= ciRand)
                {
                    const CMobItemGroup::SMobItemGroupInfo &info = pGroup->GetOne();
                    for (int j = 0; j < dropCount; ++j)
                    {
                        item = CreateItem(info.dwItemVnum, info.iCount, 0, true, info.iRarePct);

                        if (item && drop_debug)
                            SPDLOG_TRACE("\t\t Dropped {0} ##", item->GetName());

                        if (item)
                            vec_item.push_back(item);
                    }
                }
            }
            // END_OF_MOB_DROP_ITEM_BUG_FIX
        }
    }

    // Level Item Group
    {
        if (drop_debug)
            SPDLOG_TRACE("> Level item group ##");

        auto it = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());

        if (it != m_map_pkLevelItemGroup.end())
        {
            if (it->second->GetLevelLimit() <= (uint32_t)iLevel)
            {
                decltype(it->second->GetVector()) v = it->second->GetVector();

                for (uint32_t i = 0; i < v.size(); i++)
                {
                    uint32_t dropChance = Random::get(1, 1000000 /*iRandRange*/);
                    if (drop_debug)
                    {
                        const TItemTable *table = GetTable(v[i].dwVNum);
                        float realPercent = (float)v[i].dwPct / 1000000 * 100;

                        SPDLOG_TRACE("\t %s: %f%%. %s (chance: %lu, rnd: %lu) ##", table->szLocaleName, realPercent,
                                    (v[i].dwPct >= dropChance) ? "DROP! " : "", v[i].dwVNum, dropChance);
                    }

                    if (v[i].dwPct >= dropChance)
                    {
                        uint32_t dwVnum = v[i].dwVNum;
                        for (int j = 0; j < dropCount; ++j)
                        {
                            item = CreateItem(dwVnum, v[i].iCount, 0, true);
                            if (item)
                                vec_item.push_back(item);
                        }
                    }
                }
            }
        }
    }

    // 잡템
    if (pkChr->GetMobDropItemVnum())
    {
        if (drop_debug)
            SPDLOG_TRACE("> Etc item group ##");

        auto it = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());

        if (it != m_map_dwEtcItemDropProb.end())
        {
            int iPercent = (it->second * iDeltaPercent) / 100;
            int ciRand = Random::get(1, iRandRange);

            if (drop_debug)
            {
                float realPercent = (float)iPercent / iRandRange * 100;

                SPDLOG_TRACE("\t All items: %f%%. %s(iPct: %d, Rnd: %d) ##", realPercent,
                            (iPercent >= ciRand) ? "DROP ONE! " : "", iPercent, ciRand);
            }

            if (iPercent >= ciRand)
            {
                for (int j = 0; j < dropCount; ++j)
                {
                    item = CreateItem(pkChr->GetMobDropItemVnum(), 1, 0, true);
                    if (item && drop_debug)
                        SPDLOG_TRACE("\t\t Dropped %s ##", item->GetName());

                    if (item)
                        vec_item.push_back(item);
                }
            }
        }
    }

#ifndef DISABLE_METIN_STONE_DROP
    if (pkChr->IsStone())
    {
        if (drop_debug)
            SPDLOG_TRACE("> METIN! Drop stones ##");

        if (pkChr->GetDropMetinStoneVnum())
        {
            int iPercent = (pkChr->GetDropMetinStonePct() * iDeltaPercent) * 400;
            int ciRand = Random::get(1, iRandRange);

            if (drop_debug)
            {
                float realPercent = (float)iPercent / iRandRange * 100;

                SPDLOG_TRACE("\t Chance: %f%%. %s(iPct: %d, Rnd: %d) ##", realPercent,
                            (iPercent >= ciRand) ? "DROP ONE! " : "", iPercent, ciRand);
            }

            if (iPercent >= ciRand)
            {
                item = CreateItem(pkChr->GetDropMetinStoneVnum(), 1, 0, true);

                if (item && drop_debug)
                    SPDLOG_TRACE("\t\t Dropped %s ##", item->GetName());

                if (item)
                    vec_item.push_back(item);
            }
        }
    }
#endif

    if (pkKiller->IsHorseRiding() &&
        GetDropPerKillPct(1000, 1000000, iDeltaPercent, "horse_skill_book_drop") >= Random::get(1, iRandRange))
    {
        SPDLOG_TRACE("EVENT HORSE_SKILL_BOOK_DROP");
        for (int j = 0; j < dropCount; ++j)
        {
            if ((item = CreateItem(ITEM_HORSE_SKILL_TRAIN_BOOK, 1, 0, true)))
                vec_item.push_back(item);
        }
    }

    //
    // 스페셜 드롭 아이템
    //
    CreateQuestDropItem(pkChr, pkKiller, vec_item, iDeltaPercent, iRandRange);

    for (auto it = vec_item.begin(); it != vec_item.end(); ++it)
    {
        CItem *item = *it;
        DBManager::instance().SendMoneyLog(MONEY_LOG_DROP, item->GetVnum(), item->GetCount());
    }

    return vec_item.size();
}

// ADD_GRANDMASTER_SKILL
int GetThreeSkillLevelAdjust(int level)
{
    if (level < 40)
        return 32;
    if (level < 45)
        return 16;
    if (level < 50)
        return 8;
    if (level < 55)
        return 4;
    if (level < 60)
        return 2;
    return 1;
}

// END_OF_ADD_GRANDMASTER_SKILL

/*
 * TODO: Implement a generalized version of this best would be a file where we can go like
 *		Group MoonDrop
 *		{
 *			Flag "2006_drop"
 *			Min	100
 *			Max 2000
 *
 *			List Items
 *			{
 *				50037
 *			}
 *		}
 */
void ITEM_MANAGER::CreateQuestDropItem(CHARACTER *pkChr, CHARACTER *pkKiller, std::vector<CItem *> &vec_item,
                                       int iDeltaPercent, int iRandRange)
{
    CItem *item = nullptr;

    if (!pkChr || !pkKiller)
        return;

    bool isDoubleCountPriv = CPrivManager::instance().GetPriv(pkKiller, PRIV_DOUBLE_DROP_COUNT);

    int32_t dropCount = isDoubleCountPriv ? 2 : 1;

    SPDLOG_TRACE("CreateQuestDropItem victim({}), killer({})", pkChr->GetName(), pkKiller->GetName());

    /*
        Drop: Hexagonal Box
        Type: Static (Does not increase as you get drops)
    */
    if (GetDropPerKillPct(100, 2000, iDeltaPercent, "2006_drop") >= Random::get(1, iRandRange))
    {
        const uint32_t dwVnum = 50037;

        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drop: Chinese Fireworks
        Type: Static.
    */
    if (GetDropPerKillPct(/* minimum */ 100, /* default */ 1000, iDeltaPercent, "newyear_fire") >=
        Random::get(1, iRandRange))
    {
        const uint32_t dwVnum = 50107;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Chinese New Year pastes
        Drop:
            - 50016	Bean Paste
            - 50017	Sugar Paste
            - 50018	Fruit Paste
            - 50019	Sweet Rice Wrap
        Type: Static.
    */
    if (GetDropPerKillPct(100, 500, iDeltaPercent, "newyear_moon") >= Random::get(1, iRandRange))
    {
        const uint32_t wonso_items[6] = {
            50016, 50017, 50018, 50019, 50019, 50019,
        };
        const uint32_t dwVnum = wonso_items[Random::get(0, 5)];

        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drops:
            - 50024: Rose (For females)
            - 50025: Chocolate (For males)

        Type: Static.
    */
    if (GetDropPerKillPct(1, 2000, iDeltaPercent, "valentine_drop") >= Random::get(1, iRandRange))
    {
        const uint32_t valentine_items[2] = {50024, 50025};
        const uint32_t dwVnum = valentine_items[Random::get(0, 1)];

        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drops: Ice Cream
        Type: Static.
    */
    if (GetDropPerKillPct(100, 2000, iDeltaPercent, "icecream_drop") >= Random::get(1, iRandRange))
    {
        const uint32_t dwVnum = 50123;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Easter Event
        Drops: All easter eggs (19)
        Type: Static.
    */
    if (GetDropPerKillPct(100, 2000, iDeltaPercent, "easter_drop") >= Random::get(1, iRandRange))
    {
        const static uint32_t easter_item_base = 50160;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(easter_item_base + Random::get(0, 19), 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drops: Yellow Rose, Candy
        Type: Static
    */
    if (GetDropPerKillPct(100, 2000, iDeltaPercent, "whiteday_drop") >= Random::get(1, iRandRange))
    {
        const static uint32_t whiteday_items[2] = {ITEM_WHITEDAY_ROSE, ITEM_WHITEDAY_CANDY};
        uint32_t dwVnum = whiteday_items[Random::get(0, 1)];
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Puzzle Box Event

        Drops: Puzzle Box (Surprise!)
        Type: Static
    */
    const char *flag = pkKiller->GetLevel() >= 50 ? "kids_day_drop_high" : "kids_day_drop";
    if (GetDropPerKillPct(100, 1000, iDeltaPercent, flag) >= Random::get(1, iRandRange))
    {
        uint32_t dwVnum = 50034;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drops: Soul Stone
        Type: Static

        Drops higher as you get past levels 45, 50, 55, and 60
    */
    if (pkChr->GetLevel() >= 40 && pkChr->GetMobRank() >= MOB_RANK_BOSS &&
        GetDropPerKillPct(/* minimum */ 1, /* default */ 1000, iDeltaPercent, "three_skill_item") /
                GetThreeSkillLevelAdjust(pkChr->GetLevel()) >=
            Random::get(1, iRandRange))
    {
        const uint32_t dwVnum = 50513;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    // 2013 ramadan
    if (GetDropPerKillPct(1, 100, iDeltaPercent, "ramadan_drop") >= Random::get(1, iRandRange))
    {
        const static uint32_t ramadan_item = 30315;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(ramadan_item, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    // 2013 ramadan
    if (GetDropPerKillPct(1, 100, iDeltaPercent, "don_plate_drop") >= Random::get(1, iRandRange))
    {
        if ((pkChr->GetMobRank() >= MOB_RANK_BOSS || pkChr->GetMobRank() >= MOB_RANK_KING) && !pkChr->IsStone())
        {
            const static uint32_t blade_item = 30316;
            for (int j = 0; j < dropCount; ++j)
            {
                if (item = CreateItem(blade_item, 1, 0, true); item)
                    vec_item.push_back(item);
            }
        }
    }

    /*
        Drops: Seed (Insta potion)
        Type: Static
    */
    if (GetDropPerKillPct(100, 1000, iDeltaPercent, "dragon_boat_festival_drop") >= Random::get(1, iRandRange))
    {
        const uint32_t dwVnum = 50085;
        for (int j = 0; j < dropCount; ++j)
        {
            if (item = CreateItem(dwVnum, 1, 0, true); item)
                vec_item.push_back(item);
        }
    }

    /*
        Drops: Perpetual Iron.
        Type: Static.

        Only drops from level > 15 players and with higher chance from bosses.
    */
    if (pkKiller->GetLevel() >= 15 && quest::CQuestManager::instance().GetEventFlag("mars_drop"))
    {
        const uint32_t dwVnum = 70035;
        int iDropMultiply[MOB_RANK_MAX_NUM] = {
            50, 30, 5, 1, 0, 0,
        };

        if (iDropMultiply[pkChr->GetMobRank()] && GetDropPerKillPct(1000, 1500, iDeltaPercent, "mars_drop") >=
                                                      Random::get(1, iRandRange) * iDropMultiply[pkChr->GetMobRank()])
        {
            for (int j = 0; j < dropCount; ++j)
            {
                if (item = CreateItem(dwVnum, 1, 0, true); item)
                    vec_item.push_back(item);
            }
        }
    }
}

bool ITEM_MANAGER::LoadNames(const std::string &filename)
{
    std::string file;
    bsys::error_code ec;

    storm::ReadFileToString(filename, file, ec);
    if (ec)
    {
        SPDLOG_ERROR("Failed to load item-desc '{0}' with '{1}'", filename.c_str(), ec);
        return false;
    }

    std::vector<std::string> lines;
    storm::Tokenize(file, "\r\n", lines);

    std::vector<storm::String> args;

    int i = 0;
    for (const auto &line : lines)
    {
        args.clear();
        storm::Tokenize(boost::trim_copy(line), "\t", args);

        ++i;

        if (args.empty())
            continue;

        if (args.size() < 2)
        {
            SPDLOG_ERROR("item-desc '{0}' line {1} has only {2} tokens", filename, i, args.size());
            continue;
        }

        uint32_t vnum;
        if (!storm::ParseNumber(args[0], vnum))
        {
            SPDLOG_ERROR("item-desc '{0}' line {1} vnum is invalid", filename, i);
            return false;
        }

        m_itemNames[vnum] = args[1];
    }
    return true;
}

uint32_t ITEM_MANAGER::GetRefineFromVnum(uint32_t dwVnum)
{
    auto it = m_map_ItemRefineFrom.find(dwVnum);
    if (it != m_map_ItemRefineFrom.end())
        return it->second;
    return 0;
}

void ITEM_MANAGER::RegisterHyperlink(const unsigned id, const ClientItemData& d) {
    m_hyperLinkItems.insert_or_assign(id, d);

}

std::optional<ClientItemData> ITEM_MANAGER::GetHyperlinkItemData(uint32_t id) {
    if(m_hyperLinkItems.find(id) == m_hyperLinkItems.end())
        return std::nullopt;

    return m_hyperLinkItems[id];
}

const CSpecialItemGroup *ITEM_MANAGER::GetSpecialItemGroup(uint32_t dwVnum)
{
    auto it = m_map_pkSpecialItemGroup.find(dwVnum);
    if (it != m_map_pkSpecialItemGroup.end())
    {
        return it->second.get();
    }
    return nullptr;
}

const CSpecialAttrGroup *ITEM_MANAGER::GetSpecialAttrGroup(uint32_t dwVnum)
{
    auto it = m_map_pkSpecialAttrGroup.find(dwVnum);
    if (it != m_map_pkSpecialAttrGroup.end())
    {
        return it->second.get();
    }
    return nullptr;
}

uint32_t ITEM_MANAGER::GetItemDebuffVnum(uint32_t vnum)
{
    TMapDW2DW::iterator it = m_map_ori_to_debuffed.find(vnum);
    if (it != m_map_ori_to_debuffed.end())
    {
        return it->second;
    }
    else
        return 0;
}

uint32_t ITEM_MANAGER::GetMaskVnum(uint32_t dwVnum)
{
    TMapDW2DW::iterator it = m_map_new_to_ori.find(dwVnum);
    if (it != m_map_new_to_ori.end())
    {
        return it->second;
    }
    else
        return 0;
}

const ItemAttrProto &ITEM_MANAGER::GetRareItemAttr(uint32_t apply)
{
    return m_itemAttrRare[apply];
}

const ItemAttrProto &ITEM_MANAGER::GetItemAttr(uint32_t apply)
{
    return m_itemAttr[apply];
}

// pkNewItem으로 모든 속성과 소켓 값들을 목사하는 함수.
// 기존에 char_item.cpp 파일에 있던 로컬함수인 TransformRefineItem 그대로 복사함
void ITEM_MANAGER::CopyAllAttrTo(CItem *pkOldItem, CItem *pkNewItem)
{
    pkNewItem->SetGMOwner(pkOldItem->IsGMOwner());
    pkNewItem->SetBlocked(pkOldItem->IsBlocked());
    pkNewItem->SetSealDate(pkOldItem->GetSealDate());
    // ACCESSORY_REFINE
    if (pkOldItem->IsAccessoryForSocket())
    {
        if (IS_SET(pkOldItem->GetFlag(), ITEM_FLAG_TIER_4) || !(IS_SET(pkNewItem->GetFlag(), ITEM_FLAG_TIER_4)))
        {
            for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
            {
                pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
            }
        }
        // pkNewItem->StartAccessorySocketExpireEvent();
    }
    // END_OF_ACCESSORY_REFINE
    else
    {
        // 여기서 깨진석이 자동적으로 청소 됨
        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
        {
            if (!pkOldItem->GetSocket(i))
                break;
            else
                pkNewItem->SetSocket(i, 1);
        }

        // 소켓 설정
        int slot = 0;

        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
        {
            long socket = pkOldItem->GetSocket(i);
            const int ITEM_BROKEN_METIN_VNUM =
                28960; // 이건 뭐 똑같은 상수가 3군데나 있냐... 하나로 해놓지ㅠㅠㅠ 나는 패스 홍이 할꺼임
            if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
                pkNewItem->SetSocket(slot++, socket);
        }
    }

    // 매직 아이템 설정
    pkOldItem->CopyAttributeTo(pkNewItem);
}

void ITEM_MANAGER::InitializeDropInfo()
{
    for (auto& it : CMobManager::instance()) {
        const TMobTable &c_rMobTable = it.second;

        int iDeltaPercent = 100;
        int iRandRange = 4000000;

        TVecDropInfo pvecDropInfo; 

        uint8_t bRank = c_rMobTable.bRank;
        uint16_t wRace = c_rMobTable.dwVnum;

        for (auto it = m_commonDropItem[bRank].begin(); it != m_commonDropItem[bRank].end(); ++it)
        {
            const CItemDropInfo &c_rInfo = *it;

            SDropInfo kInfo;
            kInfo.dwVNum = c_rInfo.m_dwVnum;
            kInfo.iMinLevel = c_rInfo.m_iLevelStart;
            kInfo.iMaxLevel = c_rInfo.m_iLevelEnd;
            kInfo.count = c_rInfo.m_count;
                        float realPercent = (float)c_rInfo.m_iPercent / iRandRange * 100;

            kInfo.rarity = realPercent;

            pvecDropInfo.push_back(kInfo);
        }
        

        for (const auto &info : m_levelPetItemDropInfos)
        {
            if (info.m_mobVnum != wRace)
                continue;

            SDropInfo kInfo{};
            kInfo.dwVNum = info.m_itemVnum;
            kInfo.count = 1;
            float realPercent = (float)info.m_dropPct / iRandRange * 100;
            kInfo.rarity = realPercent;

            pvecDropInfo.push_back(kInfo);
        }

        for (const auto &info : m_attackPetItemDropInfos)
        {
            if (info.m_mobVnum != wRace)
                continue;

            SDropInfo kInfo{};
            kInfo.dwVNum = info.m_itemVnum;
            kInfo.count = 1;
            float realPercent = (float)info.m_dropPct / iRandRange * 100;

            kInfo.rarity = realPercent;

            pvecDropInfo.push_back(kInfo);
        }

        {
            auto it = m_map_pkDropItemGroup.find(wRace);

            if (it != m_map_pkDropItemGroup.end())
            {
                if (it->second)
                {
                    const auto &vecDrops = it->second->GetVector();
                    for (auto it2 = vecDrops.begin(); it2 != vecDrops.end(); ++it2)
                    {
                        int iPercent = (it2->dwPct * iDeltaPercent) / 100;
                        float realPercent = (float)iPercent / iRandRange * 100;

                        SDropInfo kInfo;
                        kInfo.dwVNum = it2->dwVnum;
                        kInfo.iMaxLevel = kInfo.iMaxLevel = 0;
                        kInfo.count = it2->iCount;
                        kInfo.rarity = realPercent;

                        pvecDropInfo.push_back(kInfo);
                    }
                }
            }
        }

        {
            auto it = m_map_pkMobItemGroup.find(wRace);

            if (it != m_map_pkMobItemGroup.end())
            {
                CMobItemGroup *pGroup = it->second.get();
                if (pGroup && !pGroup->IsEmpty())
                {
                    int iPercent = 40000 * iDeltaPercent / pGroup->GetKillPerDrop();

                    for (const auto &drop : pGroup->GetVector())
                    {
                        float realPercent = (float)iPercent / iRandRange * 100;

                        SDropInfo kInfo;
                        kInfo.dwVNum = drop.dwItemVnum;
                        kInfo.iMaxLevel = kInfo.iMaxLevel = 0;
                        kInfo.count = drop.iCount;
                        kInfo.rarity = realPercent;

                        pvecDropInfo.push_back(kInfo);
                    }
                }
            }
        }

        {
            auto it = m_map_pkLevelItemGroup.find(wRace);

            if (it != m_map_pkLevelItemGroup.end())
            {
                auto v = it->second->GetVector();

                for (uint32_t i = 0; i < v.size(); i++)
                {
                    float realPercent = (float)v[i].dwPct / 1000000 * 100;

                    SDropInfo kInfo;
                    kInfo.dwVNum = v[i].dwVNum;
                    kInfo.iMinLevel = 0;
                    kInfo.iMaxLevel = it->second->GetLevelLimit();
                    kInfo.count = v[i].iCount;
                    kInfo.rarity = realPercent;

                    pvecDropInfo.push_back(kInfo);
                }
            }
        }

        if (c_rMobTable.dwDropItemVnum != 0)
        {
            auto it = m_map_dwEtcItemDropProb.find(c_rMobTable.dwDropItemVnum);

            if (it != m_map_dwEtcItemDropProb.end())
            {
                int iPercent = (it->second * iDeltaPercent) / 100;
                float realPercent = (float)iPercent / iRandRange * 100;

                SDropInfo kInfo;
                kInfo.dwVNum = c_rMobTable.dwDropItemVnum;
                kInfo.iMinLevel = kInfo.iMaxLevel = 0;
                kInfo.count = 1;
                kInfo.rarity = realPercent;

                pvecDropInfo.push_back(kInfo);
            }
        }

        this->m_map_pkDropInfoByVNum.emplace(c_rMobTable.dwVnum, std::move(pvecDropInfo));
    }
}

std::optional<std::reference_wrapper<TVecDropInfo>> ITEM_MANAGER::FindDropInfo(uint32_t dwVNum)
{
    auto it = this->m_map_pkDropInfoByVNum.find(dwVNum);
    if (it == this->m_map_pkDropInfoByVNum.end())
    {
        return std::nullopt;
    }

    return std::ref(it->second);
}

uint32_t ITEM_MANAGER::GetPetItemEvolvedByVnum(uint32_t evolType, uint32_t vnum)
{
    const auto it = std::find_if(m_map_petEvolutionFromToVnum.begin(), m_map_petEvolutionFromToVnum.end(),
                                 [&](const TPetEvolInfo &a) { return a.evolType == evolType && a.fromVnum == vnum; });

    if (it == m_map_petEvolutionFromToVnum.end())
        return 0;

    return (*it).toVnum;
}

#ifdef INGAME_WIKI
void ITEM_MANAGER::BuildWikiInfoForVnum(uint32_t vnum)
{
    auto *tbl = GetTable(vnum);
    if (!tbl)
        return;

    auto newTable = std::make_unique<TWikiItemInfo>();

    newTable->bIsCommon = false;
    for (int mobRank = 0; mobRank < MOB_RANK_MAX_NUM && !newTable->bIsCommon; ++mobRank)
        for (auto it2 = m_commonDropItem[mobRank].begin();
             it2 != m_commonDropItem[mobRank].end() && !newTable->bIsCommon; ++it2)
            if (it2->m_dwVnum == vnum)
                newTable->bIsCommon = true;

    newTable->dwOrigin = 0;

    if ((tbl->bType == ITEM_WEAPON || tbl->bType == ITEM_ARMOR || tbl->bType == ITEM_BELT ||
         tbl->bType == ITEM_TALISMAN || tbl->bType == ITEM_RING || (tbl->bType == ITEM_TOGGLE && tbl->bSubType == TOGGLE_AFFECT)) &&
        vnum % 10 == 0 && tbl->dwRefinedVnum)
    {
        TWikiRefineInfo tempRef[10] = {};
        auto *tblTemp = tbl;
        for (int i = 0; i < 10; ++i)
        {
            auto &tmpRef = tempRef[i];
            if (!tblTemp)
            {
                continue;
            }

            const auto *refTbl = CRefineManager::instance().GetRefineRecipe(tblTemp->wRefineSet);
            if (!refTbl)
            {
                continue;
            }

            for (auto j = 0; j < refTbl->materials.size(); ++j)
                tmpRef.materials[j] = refTbl->materials[j];
            tmpRef.price = refTbl->cost;
            tmpRef.material_count = refTbl->materials.size();

            tblTemp = GetTable(tblTemp->dwRefinedVnum);
        }

        for (const auto &i : tempRef)
            newTable->pRefineData.push_back(i);
    }
    else if (tbl->bType == ITEM_GIFTBOX || (tbl->bType == ITEM_USE && tbl->bSubType == USE_SPECIAL))
    {
        CSpecialItemGroup *ptr = nullptr;
        auto it = m_map_pkSpecialItemGroup.find(vnum);
        if (it == m_map_pkSpecialItemGroup.end())
        {
            it = m_map_pkQuestItemGroup.find(vnum);
            if (it != m_map_pkQuestItemGroup.end())
                ptr = it->second.get();
        }
        else
            ptr = it->second.get();

        if (ptr && (!ptr->m_vecItems.empty()))
        {
            for (auto it2 : ptr->m_vecItems)
                if (it2.vnum >= ptr->GIVE_TYPE_MAX_NUM)
                {
                    newTable->pChestInfo.emplace_back(it2.vnum);
                }
        }
    }

    m_wikiInfoMap.emplace(vnum, std::move(newTable));
}

TWikiItemInfo *ITEM_MANAGER::GetItemWikiInfo(uint32_t vnum)
{
    if (const auto it = m_wikiInfoMap.find(vnum); it != m_wikiInfoMap.end())
        return it->second.get();

    BuildWikiInfoForVnum(vnum);

    if (const auto it = m_wikiInfoMap.find(vnum); it != m_wikiInfoMap.end())
        return it->second.get();

    return nullptr;
}
#endif
