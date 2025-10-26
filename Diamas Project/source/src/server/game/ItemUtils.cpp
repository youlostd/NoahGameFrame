#include "ItemUtils.h"
#include "CharUtil.hpp"
#include "GArena.h"
#include "PetSystem.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "marriage.h"
#include "motion.h"
#include "skill.h"
#include "war_map.h"
#include <game/AffectConstants.hpp>
#include <game/DbPackets.hpp>
#include "questmanager.h"
#include <variant>

//귀환부, 귀환기억부, 결혼반지
bool IsSummonItem(int vnum)
{
    switch (vnum) {
        case 22000:
        case 22010:
        case ITEM_MARRIAGE_RING:
            return true;
    }

    return false;
}

static bool IS_MONKEY_DUNGEON(int map_index)
{
    switch (map_index) {
        case 5:
        case 25:
        case 45:
        case 108:
        case 109:
            return true;
    }

    return false;
}

bool IsSummonableZone(int map_index)
{
    // 몽키던전
    if (IS_MONKEY_DUNGEON(map_index))
        return false;
    // 성

    switch (map_index) {
        case 66:  // 사귀타워
        case 71:  // 거미 던전 2층
        case 72:  // 천의 동굴
        case 73:  // 천의 동굴 2층
        case 193: // 거미 던전 2-1층
#if 0
	case 184: // 천의 동굴(신수)
	case 185: // 천의 동굴 2층(신수)
	case 186: // 천의 동굴(천조)
	case 187: // 천의 동굴 2층(천조)
	case 188: // 천의 동굴(진노)
	case 189: // 천의 동굴 2층(진노)
#endif
                  //		case 206 : // 아귀동굴
        case 216: // 아귀동굴
        case 217: // 거미 던전 3층
        case 208: // 천의 동굴 (용방)
        case 209: // Earth Dragon

        case 113:
            return false;
    }

    // 모든 private 맵으론 워프 불가능
    if (map_index > 10000)
        return false;

    return true;
}

bool ConflictsWithEquipped(CHARACTER* ch, const CItem* item,
                           const CItem* except,
                           std::initializer_list<int16_t> cells)
{
    bool conflicted = false;
    for (auto cell : cells) {
        const auto target = ch->GetWear(cell);
        if (!target)
            continue;

        if (except == target)
            continue;

        conflicted = conflicted || target->IsSameSpecialGroup(item);
    }

    return conflicted;
}

int16_t FindEquipmentCellAux(const CItem* item, int iCandidateCell = -1)
{
    if (item->GetItemType() == ITEM_DS ||
        item->GetItemType() == ITEM_SPECIAL_DS) {
        if (iCandidateCell < 0) {
            return DRAGON_SOUL_EQUIP_SLOT_START + item->GetSubType();
        } else {
            for (int i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++) {
                if (DRAGON_SOUL_EQUIP_SLOT_START + i * DS_SLOT_MAX +
                        item->GetSubType() ==
                    iCandidateCell)
                    return iCandidateCell;
            }

            return -1;
        }
    }

    switch (item->GetItemType()) {
        case ITEM_WEAPON: {
            switch (item->GetSubType()) {
                case WEAPON_ARROW:
                case WEAPON_QUIVER:
                    return WEAR_ARROW;

                default:
                    return WEAR_WEAPON;
            }
        }

        case ITEM_ARMOR: {
            switch (item->GetSubType()) {
                case ARMOR_BODY:
                    return WEAR_BODY;
                case ARMOR_HEAD:
                    return WEAR_HEAD;
                case ARMOR_SHIELD:
                    return WEAR_SHIELD;
                case ARMOR_WRIST:
                    return WEAR_WRIST;
                case ARMOR_FOOTS:
                    return WEAR_FOOTS;
                case ARMOR_NECK:
                    return WEAR_NECK;
                case ARMOR_EAR:
                    return WEAR_EAR;
                default:
                    break;
            }
        }
        case ITEM_TALISMAN: {
            switch (item->GetSubType()) {
                case TALISMAN_1:
                    return WEAR_TALISMAN_1;
                case TALISMAN_2:
                    return WEAR_TALISMAN_2;
                case TALISMAN_3:
                    return WEAR_TALISMAN_3;
                case TALISMAN_4:
                    return WEAR_TALISMAN_4;
                case TALISMAN_5:
                    return WEAR_TALISMAN_5;
                case TALISMAN_6:
                    return WEAR_TALISMAN_6;
                case TALISMAN_7:
                    return WEAR_TALISMAN_7;
                default:
                    break;
            }
        }
        case ITEM_RING: {
            switch (item->GetSubType()) {
                case RING_1:
                    return WEAR_RING1;
                case RING_2:
                    return WEAR_RING2;
                case RING_3:
                    return WEAR_RING3;
                case RING_4:
                    return WEAR_RING4;
                case RING_5:
                    return WEAR_RING5;
                case RING_6:
                    return WEAR_RING6;
                case RING_7:
                    return WEAR_RING7;
                default:
                    break;
            }
        }

        case ITEM_COSTUME: {
            switch (item->GetSubType()) {
                case COSTUME_BODY:
                    return WEAR_COSTUME_BODY;
                case COSTUME_HAIR:
                    return WEAR_COSTUME_HAIR;
                case COSTUME_MOUNT:
                    return WEAR_COSTUME_MOUNT;
                case COSTUME_ACCE:
                    return WEAR_COSTUME_ACCE;
                case COSTUME_WEAPON:
                    return WEAR_COSTUME_WEAPON;
                case COSTUME_WEAPON_EFFECT:
                    return WEAR_COSTUME_WEAPON_EFFECT;
                case COSTUME_BODY_EFFECT:
                    return WEAR_COSTUME_BODY_EFFECT;
                case COSTUME_WING_EFFECT:
                    return WEAR_COSTUME_WING_EFFECT;
                case COSTUME_RANK:
                    return WEAR_COSTUME_RANK;
                default:
                    break;
            }
        }

        case ITEM_BELT:
            return WEAR_BELT;
        case ITEM_ROD:
        case ITEM_PICK:
            return WEAR_WEAPON;
        default:
            break;
    }

    return -1;
}

int16_t FindEquipmentCell(CHARACTER* ch, const CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_UNIQUE: {
            auto target = ch->GetWear(WEAR_UNIQUE1);
            if (target && !target->IsSameSpecialGroup(item))
                return WEAR_UNIQUE2;
            else
                return WEAR_UNIQUE1;
        }
    }

    return FindEquipmentCellAux(item);
}

bool IsEquipmentCell(const CItem* item, uint8_t cell)
{
    switch (item->GetItemType()) {
        case ITEM_UNIQUE:
            return cell == WEAR_UNIQUE1 || cell == WEAR_UNIQUE2;
    }

    const auto foundCell = FindEquipmentCellAux(item, cell);
    if (foundCell == -1)
        return false;

    return foundCell == cell;
}

bool CanRemove(CHARACTER* ch, const CItem* item)
{
    switch (item->GetWindow()) {
        case EQUIPMENT: {
            if (!CanUnequipItem(ch, item))
                return false;
        }

        case SWITCHBOT: {
            if (item->isLocked())
                return false;
        }
    }

    return true;
}

bool CanAdd(CHARACTER* ch, const CItem* item, const TItemPos& pos,
            const CItem* except, bool skipTimeChecks)
{
    switch (pos.window_type) {
        case DRAGON_SOUL_INVENTORY: {
            return ch->IsEmptyItemGridDS(pos, item->GetSize(),
                                         except ? except->GetCell() : -1);
        }

        case INVENTORY: {
            return ch->IsEmptyItemGrid(pos, item->GetSize(), except);
        }

        case SWITCHBOT: {
            return ch->GetSwitchbotItem(pos.cell) == nullptr;
        }

        case EQUIPMENT: {
            if (!IsEquipmentCell(item, pos.cell))
                return false;

            return CanEquipItem(ch, item, except, skipTimeChecks);
        }
    }

    return false;
}

bool CanModifyItem(const CItem* item)
{
    if (item->IsExchanging())
        return false;

    if (item->isLocked())
        return false;

    return true;
}

bool CanStack(const CItem* from, const CItem* to)
{
    if (from == to)
        return false;

    if (from->GetVnum() != to->GetVnum())
        return false;

    if (!to->IsStackable())
        return false;

    // ignore socket5 (offline shop item price)
    for (auto i = 0; i < ITEM_SOCKET_MAX_NUM - 1; ++i)
        if (from->GetSocket(i) != to->GetSocket(i))
            return false;

    return true;
}

CountType Stack(CItem* from, CItem* to, CountType count)
{
    STORM_ASSERT(CanStack(from, to), "Sanity");
    STORM_ASSERT(count < from->GetCount(), "Sanity");

    if (count == 0)
        count = from->GetCount();

    count = std::min(GetItemMaxCount(to) - to->GetCount(), count);

    // Well, don't mutate anything in that case
    if (count == 0)
        return 0;

    from->SetCount(from->GetCount() - count);
    to->SetCount(to->GetCount() + count);
    return count;
}

CountType GetItemMaxCount(std::variant<const CItem*, const uint32_t> v)
{
    uint32_t type;

    if (std::holds_alternative<const CItem*>(v)) {
        type = std::get<const CItem*>(v)->GetItemType();
    } else {
        const auto vnum = std::get<const uint32_t>(v);
        const auto* p = ITEM_MANAGER::instance().GetTable(vnum);
        if (!p)
            return ITEM_MAX_COUNT;

        type = p->bType;
    }

    switch (type) {
        case ITEM_GIFTBOX:
            return 500000;
        case ITEM_MATERIAL:
            return 500000;
        case ITEM_ELK:
            return std::numeric_limits<CountType>::max();
        case ITEM_METIN:
            return 500000;
        case ITEM_FISH:
            return 500000;
        case ITEM_POLYMORPH:
            return 5000;
        case ITEM_BLEND:
            return 10000;
        case ITEM_SPECIAL:
            return 500000;
        case ITEM_RESOURCE:
            return 500000;
        case ITEM_USE:
            return 500000;

        default:
            break;
    }

    return ITEM_MAX_COUNT;
}

bool ActivateToggleItem(CHARACTER* ch, CItem* item)
{
    // Checks before activation:
    switch (item->GetSubType()) {
        case TOGGLE_AUTO_RECOVERY_HP: {
            if (!ch->IsNextAutoPotionPulseTp()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You can not use this so fast.");
                return false;
            }

            if (ch->GetArena()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You can not use this in the arena.");
                return false;
            }

            break;
        }

        case TOGGLE_AUTO_RECOVERY_SP: {
            if (!ch->IsNextAutoPotionPulseMp()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You can not use this so fast.");
                return false;
            }

            if (ch->GetArena()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You can not use this in the arena.");
                return false;
            }

            break;
        }

        case TOGGLE_LEVEL_PET: {
            if (ch->GetLevel() < item->GetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "Your pet no longer listens to you. Your "
                                   "level is too low.");
                return false;
            }

            if (ch->IsLevelPetStatusWindowOpen()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "Your cannot spawn your pet while having "
                                   "the status window open.");
                return false;
            }
            break;
        }
    }

    if (item->GetCount() > 1) {
        int pos = ch->GetEmptyInventory(item);
        if (-1 == pos) {
            ch->ChatPacket(CHAT_TYPE_INFO, "Not enough space in your "
                                           "inventory");
            return false;
        }

        item->SetCount(item->GetCount() - 1);

        const auto item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum());
        item2->AddToCharacter(ch, TItemPos(INVENTORY, pos));

        item = item2;
    }

    item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, true);

    item->Lock(true);

    if (item->FindLimit(LIMIT_TIMER_BASED_ON_WEAR))
        item->StartTimerBasedOnWearExpireEvent();

    switch (item->GetSubType()) {
        case TOGGLE_AUTO_RECOVERY_HP: {
            ch->UpdateAutoPotionPulseTp();
            ch->StartAffectEvent();
            ch->UpdateAffect();
        } break;
        case TOGGLE_AUTO_RECOVERY_SP: {
            ch->UpdateAutoPotionPulseMp();
            ch->StartAffectEvent();
            ch->UpdateAffect();
            break;
        }

        case TOGGLE_MOUNT: {
            if (!item->GetSocket(ITEM_SOCKET_TOGGLE_RIDING))
                ch->CreateToggleMount(item);

            break;
        }

        case TOGGLE_PET: {
            item->ModifyPoints(true);
            auto vnum = item->GetValue(ITEM_VALUE_PET_VNUM);
            if (item->GetTransmutationVnum()) {
                const auto tbl = ITEM_MANAGER::instance().GetTable(
                    item->GetTransmutationVnum());
                if (tbl) {
                    vnum = tbl->alValues[ITEM_VALUE_PET_VNUM];
                }
            }

            ch->GetPetSystem()->SummonItem(vnum, item->GetID(), false);
            break;
        }

        case TOGGLE_LEVEL_PET: {
            item->ModifyPoints(true);
            auto vnum = item->GetTransmutationVnum();

            ch->GetPetSystem()->SummonItem(vnum, item->GetID(), false);
            break;
        }

        case TOGGLE_ANTI_EXP: {
            ch->SetInstantFlag(INSTANT_FLAG_REFUSE_EXP);
            break;
        }

        case TOGGLE_AFFECT: {
            item->ModifyPoints(true);
            break;
        }
    }

    ch->CheckMaximumPoints();

    return true;
}

void DeactivateToggleItem(CHARACTER* ch, CItem* item)
{
    switch (item->GetSubType()) {
        case TOGGLE_MOUNT: {
            if (item->GetSocket(ITEM_SOCKET_TOGGLE_RIDING)) {
                item->SetSocket(ITEM_SOCKET_TOGGLE_RIDING, 0, true);
            }
            ch->DestroyToggleMount();
            break;
        }
        case TOGGLE_LEVEL_PET:
        case TOGGLE_PET: {
            ch->GetPetSystem()->UnsummonItem(item->GetID());
            item->ModifyPoints(false);

            break;
        }

        case TOGGLE_ANTI_EXP: {
            ch->ClearInstantFlag(INSTANT_FLAG_REFUSE_EXP);
            item->ModifyPoints(false);

            break;
        }

        case TOGGLE_AFFECT: {
            item->ModifyPoints(false);

            break;
        }
    }

    if (item->FindLimit(LIMIT_TIMER_BASED_ON_WEAR))
        item->StopTimerBasedOnWearExpireEvent();

    item->Lock(false);
    item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);

    ch->CheckMaximumPoints();
}

namespace events::Item
{
namespace Toggle
{
void OnCreate(CItem* item)
{
    switch (item->GetSubType()) {
        case TOGGLE_AUTO_RECOVERY_HP:
        case TOGGLE_AUTO_RECOVERY_SP:
            item->SetSocket(ITEM_SOCKET_AUTORECOVERY_FULL,
                            item->GetValue(ITEM_VALUE_AUTORECOVERY_AMOUNT),
                            false);
            break;
        case TOGGLE_LEVEL_PET:
            if (item->GetAttributeCount() < 2) {
                auto a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
                while (item->HasAttr(a.type)) {
                    a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
                }

                item->SetForceAttribute(0, a.type, a.value);
                a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
                while (item->HasAttr(a.type)) {
                    a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
                }
                item->SetForceAttribute(1, a.type, a.value);
            }
            item->SetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL, LEVEL_PET_START_LEVEL);
            item->SetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT,
                            ITEM_MANAGER::instance().GetPetNeededExpByLevel(
                                LEVEL_PET_START_LEVEL));
            item->SetSocket(ITEM_SOCKET_LEVEL_STAT_POINT,
                            LEVEL_PET_STAT_POINT_PER_LEVEL);
            break;
    }
}

void OnLoad(CHARACTER* ch, CItem* item)
{
    // Don't do anything if we're not active.
    if (!item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
        return;

    auto mapIndex = ch->GetMapIndex();
    if (mapIndex >= 10000)
        mapIndex /= 10000;

    // If our toggle item has an unique group, check if we already activated
    // another item of the same group.
    if (item->GetSubType() == TOGGLE_PET) {
        const auto isAttackPet = item->GetValue(ITEM_VALUE_PET_IS_ATTACKING);

        const auto group = item->GetValue(1);
        if (-1 != group &&
            FindToggleItem(ch, true, item->GetSubType(), group, item)) {
            item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);
            return;
        }
    } else {
        const auto group = item->GetValue(ITEM_VALUE_TOGGLE_GROUP);
        if (-1 != group &&
            FindToggleItem(ch, true, item->GetSubType(), group, item)) {
            item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);
            return;
        }
    }

    if (item->GetSubType() == TOGGLE_LEVEL_PET) {
        if (!IsLevelPetUsableZone(ch->GetMapIndex())) {
            item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);
            return;
        }
    }

    if (item->GetSubType() == TOGGLE_PET) {
        if (!IsPetUsableZone(ch->GetMapIndex())) {
            item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);
            return;
        }
    }

    if (item->GetSubType() == TOGGLE_MOUNT) {
        if (!IsMountableZone(ch->GetMapIndex(), false)) {
            item->SetSocket(ITEM_SOCKET_TOGGLE_ACTIVE, false);
            item->SetSocket(ITEM_SOCKET_TOGGLE_RIDING, false, false);
            ch->DestroyToggleMount();
            return;
        } else {
            if (item->GetSubType() == TOGGLE_MOUNT) {
                if (item->GetSocket(ITEM_SOCKET_TOGGLE_RIDING))
                    item->ModifyPoints(true);
                item->HandleToggleMountSocketState(
                    item->GetSocket(ITEM_SOCKET_TOGGLE_RIDING));
            }
        }
    }

    ActivateToggleItem(ch, item);
}

void OnRemove(CHARACTER* ch, CItem* item)
{
    // Don't do anything if we're not active.
    if (!item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
        return;

    DeactivateToggleItem(ch, item);
}

bool OnUse(CHARACTER* ch, CItem* item)
{
    if (item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE)) {
        DeactivateToggleItem(ch, item);
        return true;
    }

    if (item->GetSubType() == TOGGLE_PET) {
        if (!IsPetUsableZone(ch->GetMapIndex())) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot use this item on this map.");
            return false;
        }
    }

    if (item->GetSubType() == TOGGLE_LEVEL_PET) {
        if (!IsLevelPetUsableZone(ch->GetMapIndex())) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot use this item on this map.");
            return false;
        }
    }

    if (item->GetSubType() == TOGGLE_MOUNT) {
        if (!IsMountableZone(ch->GetMapIndex(), false)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot use this item on this map.");
            return false;
        }
    }

    // If our toggle item has an unique group, check if we already activated
    // another item of the same group.
    if (item->GetSubType() == TOGGLE_PET) {
        const auto isAttackPet = item->GetValue(ITEM_VALUE_PET_IS_ATTACKING);

        const auto group = item->GetValue(1);
        if (-1 != group &&
            FindToggleItem(ch, true, item->GetSubType(), group, item)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot activate two items of this kind.");
            return false;
        }
    } else {
        const auto group = item->GetValue(ITEM_VALUE_TOGGLE_GROUP);
        if (-1 != group &&
            FindToggleItem(ch, true, item->GetSubType(), group, item)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot activate two items of this kind.");
            return false;
        }
    }

    if (!item->IsNextToggleUsePulse()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You cannot activate and deactivate this so "
                           "quickly");
        return false;
    }

    item->UpdateToggleUsePulse();

    return ActivateToggleItem(ch, item);
}
} // namespace Toggle

void OnCreate(CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_TOGGLE:
            Toggle::OnCreate(item);
            break;
    }

    item->OnCreate();
}

void OnLoad(CHARACTER* ch, CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_TOGGLE:
            Toggle::OnLoad(ch, item);
            break;
    }

    item->OnLoad();
}

void OnRemove(CHARACTER* ch, CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_TOGGLE:
            Toggle::OnRemove(ch, item);
            break;
    }
}

bool OnUse(CHARACTER* ch, CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_TOGGLE:
            return Toggle::OnUse(ch, item);
    }

    return true;
}

bool IsEpicWeaponVnum(ItemVnum vnum)
{
    return vnum == 740149 || vnum == 733149 || vnum == 731119 ||
           vnum == 732159 || vnum == 740159 || vnum == 747149 || vnum == 755099;
}

void OnEquip(CHARACTER* ch, CItem* item)
{
    const auto vnum = item->GetVnum();

    if (IsEpicWeaponVnum(vnum)) {
        ch->AddAffect(AFFECT_EPIC, 0, 0, 60 * 60 * 24 * 356, 0, true, false);
    }

    if (ITEM_UNIQUE == item->GetItemType() && 0 != item->GetSIGVnum()) {
        auto& mgr = ITEM_MANAGER::instance();

        const auto pGroup = mgr.GetSpecialItemGroup(item->GetSIGVnum());
        if (pGroup) {
            const auto pAttrGroup =
                mgr.GetSpecialAttrGroup(pGroup->GetAttrVnum(vnum));
            if (pAttrGroup)
                ch->SpecificEffectPacket(pAttrGroup->m_stEffectFileName);
        }
    } else if (item->GetItemType() == ITEM_COSTUME &&
               item->GetSubType() == COSTUME_ACCE) {
        ch->EffectPacket(SE_ACCE_BACK);
        ch->EffectPacket(SE_ACCE_EQUIP);
    }

    item->OnEquip();
    ch->UpdateEquipPulse();
}

bool OnUnequip(CHARACTER* ch, CItem* item)
{
    const auto vnum = item->GetVnum();
    if (IsEpicWeaponVnum(vnum)) {
        ch->RemoveAffect(AFFECT_EPIC);
    }
    item->OnUnequip();
    return true;
}
} // namespace events::Item

void ProcessAutoRecoveryItem(CHARACTER* ch, CItem* item)
{
    if (ch->IsDead() || ch->IsStun())
        return;

    static const uint32_t kStunSkills[] = {AFFECT_STUN,    SKILL_TANHWAN,
                                           SKILL_GEOMPUNG, SKILL_BYEURAK,
                                           SKILL_GIGUNG,   AFFECT_NO_RECOVERY};
    for (auto skill : kStunSkills)
        if (ch->FindAffect(skill))
            return;

    if (ch->GetArena()) {
        DeactivateToggleItem(ch, item);
        return;
    }

    const int32_t amountUsed = 0;
    const int32_t amountFull = item->GetSocket(ITEM_SOCKET_AUTORECOVERY_FULL);
    const int32_t avail = amountFull - amountUsed;

    int64_t amount = 0;

    if (TOGGLE_AUTO_RECOVERY_HP == item->GetSubType())
        amount =
            ch->GetMaxHP() - (ch->GetHP() + ch->GetPoint(POINT_HP_RECOVERY));
    else if (TOGGLE_AUTO_RECOVERY_SP == item->GetSubType())
        amount =
            ch->GetMaxSP() - (ch->GetSP() + ch->GetPoint(POINT_SP_RECOVERY));

    if (amount <= 0)
        return;

    if (avail > amount) {
        const int pct_of_used = amountUsed * 100 / amountFull;
        const int pct_of_will_used = (amountUsed + amount) * 100 / amountFull;

        bool log = false;
        if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
            log = true;

    } else {
        amount = avail;
    }

    if (TOGGLE_AUTO_RECOVERY_HP == item->GetSubType()) {
        ch->PointChange(POINT_HP_RECOVERY, amount);
        ch->EffectPacket(SE_AUTO_HPUP);
    } else if (TOGGLE_AUTO_RECOVERY_SP == item->GetSubType()) {
        ch->PointChange(POINT_SP_RECOVERY, amount);
        ch->EffectPacket(SE_AUTO_SPUP);
    }

    if (amount == avail) {
        // RemoveItem() will deactivate |item|
        ITEM_MANAGER::instance().RemoveItem(item);
    }
}

CItem* FindToggleItem(CHARACTER* ch, bool active, int32_t subType,
                      int32_t group, CItem* except)
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
        const auto item = ch->GetInventoryItem(i);
        if (!item || item == except || item->GetItemType() != ITEM_TOGGLE)
            continue;

        if (subType != -1 && item->GetSubType() != subType)
            continue;

        if (active !=
            static_cast<bool>(item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE)))
            continue;

        if (subType == TOGGLE_PET) {
            if (group != -1 && group != item->GetValue(1))
                continue;
        } else {
            if (group != -1 && group != item->GetValue(ITEM_VALUE_TOGGLE_GROUP))
                continue;
        }

        return item;
    }

    return nullptr;
}

bool CheckItemSex(CHARACTER* ch, const CItem* item)
{
    const auto sex = GetSexByRace(ch->GetRaceNum());

    // 남자 금지
    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE)) {
        if (SEX_MALE == sex)
            return false;
    }

    // 여자금지
    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE)) {
        if (SEX_FEMALE == sex)
            return false;
    }

    return true;
}

bool CheckItemEmpire(CHARACTER* ch, const CItem* item)
{
    const auto empire = ch->GetEmpire();

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_EMPIRE_A)) {
        if (EMPIRE_A == empire)
            return false;
    }

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_EMPIRE_B)) {
        if (EMPIRE_B == empire)
            return false;
    }

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_EMPIRE_C)) {
        if (EMPIRE_C == empire)
            return false;
    }

    return true;
}

bool CheckItemLimits(CHARACTER* ch, const CItem* item, bool showMessages)
{
    const auto* itemTable = item->GetProto();

    for (const auto& limit : itemTable->aLimits) {
        switch (limit.bType) {
            case LIMIT_LEVEL:
                if (ch->GetLevel() < limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;
            case LIMIT_MAX_LEVEL:
                if (ch->GetLevel() > limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;

            case LIMIT_STR:
                if (ch->GetPoint(POINT_ST) < limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;

            case LIMIT_INT:
                if (ch->GetPoint(POINT_IQ) < limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;

            case LIMIT_DEX:
                if (ch->GetPoint(POINT_DX) < limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;

            case LIMIT_CON:
                if (ch->GetPoint(POINT_HT) < limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "레벨이 낮아 착용할 수 없습니다.");
                    return false;
                }
                break;
            case LIMIT_MAP:
                if (ch->GetMapIndex() != limit.value &&
                    ch->GetMapIndex() / 10000 != limit.value) {
                    if (showMessages)
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "You cannot use this item in this "
                                           "area.");
                    return false;
                }
                break;
            default:
                break;
        }
    }

    return true;
}

bool CheckItemRace(CHARACTER* ch, const CItem* item)
{
    switch (ch->GetJob()) {
        case JOB_WARRIOR:
            if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
                return false;
            break;

        case JOB_ASSASSIN:
            if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
                return false;
            break;

        case JOB_SHAMAN:
            if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
                return false;
            break;

        case JOB_SURA:
            if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
                return false;
            break;

        case JOB_WOLFMAN:
            if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
                return false;
            break;
        default:
            break;
    }
    return true;
}

bool CanEquipItem(CHARACTER* ch, const CItem* item, const CItem* except,
                  bool skipTimeChecks, bool showMessages)
{
    if (!item->IsEquipable())
        return false;

    if (auto questPc = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
        questPc->GetFlag("pvp.block_item") == 1)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Arenadayken esya degistiremezsin!");
         return false;
    }
       

    const auto itemType = item->GetItemType();
    const auto itemSubType = item->GetSubType();

    if (!CheckItemRace(ch, item))
        return false;

    if (ch->IsAcceWindowOpen())
        return false;

    if (item->GetPosition().IsSwitchbotPosition())
        return false;

    auto mapIndex = ch->GetMapIndex();
    if (mapIndex >= 10000)
        mapIndex /= 10000;

    if (CArenaManager::instance().IsLimitedItem(mapIndex, item->GetVnum())) {
        if (showMessages)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot equip this item on this map");
        return false;
    }

    if (!CheckItemLimits(ch, item, showMessages))
        return false;

    if (itemType == ITEM_ARMOR && itemSubType == ARMOR_BODY && ch->IsRiding() &&
        (item->GetVnum() >= 11901 && item->GetVnum() <= 11904)) {
        if (showMessages)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can not wear a robe while wearing a "
                               "horse.");
        return false;
    }

    if (ch->IsPolymorphed()) {
        if (showMessages)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can not change the equipment you are "
                               "wearing while you are polymorphed.");
        return false;
    }

    if (!CheckItemSex(ch, item)) {
        if (showMessages)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "This item can not be used because the gender "
                               "does not match.");
        return false;
    }

    const auto now = get_dword_time();

    if (itemType == ITEM_WEAPON &&
        (itemSubType == WEAPON_BOW || itemSubType == WEAPON_DAGGER))
        skipTimeChecks = true;

    if (!skipTimeChecks) {
        if (now - ch->GetLastPlayerHitReceivedTime() <= 1000 ||
            now - ch->GetLastSkillTime() <= 1500 ||
            now - ch->GetLastAttackTime() <= 1000) {
            if (showMessages)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You can only change your equipment while "
                                   "you are still.");
            return false;
        }

        /*
            const auto now = get_dword_time();
            if (now - ch->GetLastAttackTime() <= 1500 ||
                now - ch->GetLastSkillTime() <= 1500 ||
                now - ch->GetLastHitReceivedTime() <= 1500) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You can wear it only
           when you are still."); return false;
            }
        */
        if (itemType == ITEM_COSTUME && itemSubType == COSTUME_ACCE &&
            !ch->IsNextEquipPulse()) {
            if (showMessages)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You cannot change your equipment this "
                                   "fast.");
            return false;
        }
    }

    if (itemType == ITEM_UNIQUE) {
        if (ConflictsWithEquipped(ch, item, except,
                                  {WEAR_UNIQUE1, WEAR_UNIQUE2})) {
            if (showMessages)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "같은 종류의 유니크 아이템 두 개를 동시에 "
                                   "장착할 수 없습니다.");
            return false;
        }

        auto& marriage = marriage::CManager::instance();
        if (marriage.IsMarriageUniqueItem(item->GetVnum()) &&
            !marriage.IsMarried(ch->GetPlayerID())) {
            if (showMessages)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "결혼하지 않은 상태에서 예물을 착용할 수 "
                                   "없습니다.");
            return false;
        }
    }

    //
    // Weapon costume/weapon incompatibility checks
    //
    // |ITEM_VALUE_COSTUME_WEAPON_TYPE| must match the weapon's sub type
    //
    if (!skipTimeChecks) {
        const auto* weapon = ch->GetWear(WEAR_WEAPON);
        const auto* bodyCostume = ch->GetWear(WEAR_COSTUME_BODY);
        const auto* body = ch->GetWear(WEAR_BODY);

        const auto* weaponCostume = ch->GetWear(WEAR_COSTUME_WEAPON);

        if (itemType == ITEM_COSTUME && itemSubType == COSTUME_WEAPON) {
            if (!weapon) {
                if (showMessages)
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot equip a costume without a "
                                       "weapon");
                return false; // Cannot equip a costume without a weapon
            }

            if (item->GetValue(ITEM_VALUE_COSTUME_WEAPON_TYPE) !=
                weapon->GetSubType()) {
                if (showMessages)
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot equip a costume not matching "
                                       "the weapon type.");
                return false; // Cannot equip a costume not matching the weapon
                              // type
            }
        }

        if (itemType == ITEM_WEAPON &&
            (itemSubType != WEAPON_ARROW && itemSubType != WEAPON_QUIVER) &&
            weaponCostume) {
            if (weaponCostume->GetValue(ITEM_VALUE_COSTUME_WEAPON_TYPE) !=
                itemSubType)
                return false;
        }

        if (itemType == ITEM_COSTUME && itemSubType == COSTUME_BODY_EFFECT) {
            if (!bodyCostume && !body)
                return false;
        }
        if (itemType == ITEM_COSTUME && itemSubType == COSTUME_WEAPON_EFFECT) {
            if (!weapon && !weaponCostume)
                return false;
        }
    }

    // Block sashes on all maps excluding the listed maps
    if (item->GetItemType() == ITEM_COSTUME &&
        item->GetSubType() == COSTUME_ACCE) {
        if (gConfig.HasAllowSashMaps()) {
            return gConfig.IsAllowSashMap(mapIndex);
        }

        if (gConfig.HasDisallowSashMaps()) {
            return !gConfig.IsDisallowSashMap(mapIndex);
        }
    }

    return true;
}

bool CanUnequipItem(CHARACTER* ch, const CItem* item, const TItemPos& swapCell)
{
    const int iWearCell = FindEquipmentCell(ch, item);

    
    if (auto questPc =
            quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
        questPc->GetFlag("pvp.block_item") == 1) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Arenadayken esya degistiremezsin!");
        return false;
    }

    if (iWearCell == WEAR_WEAPON) {
        const auto costumeWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
        if (costumeWeapon && !ch->UnequipItem(costumeWeapon)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot unequip the weapon. Not enough "
                               "space for your costume weapon.");
            return false;
        }

        const auto costumeWeaponEffect =
            ch->GetWear(WEAR_COSTUME_WEAPON_EFFECT);
        if (costumeWeaponEffect && !ch->UnequipItem(costumeWeaponEffect)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot unequip the weapon. Not enough "
                               "space for your weapon effect");
            return false;
        }
    }

    if (iWearCell == WEAR_COSTUME_BODY) {
        const auto body = ch->GetWear(WEAR_BODY);
        if (!body) {
            const auto costumeBodyEffect =
                ch->GetWear(WEAR_COSTUME_BODY_EFFECT);
            if (costumeBodyEffect && !ch->UnequipItem(costumeBodyEffect)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You cannot unequip the armor. Not enough "
                                   "space for your armor effect");
                return false;
            }
        }
    }

    if (iWearCell == WEAR_BODY) {
        const auto costumeBodyEffect = ch->GetWear(WEAR_COSTUME_BODY_EFFECT);
        if (costumeBodyEffect && !ch->UnequipItem(costumeBodyEffect)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You cannot unequip the armor. Not enough space "
                               "for your armor effect");
            return false;
        }
    }

    if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
        return false;

    const auto itemType = item->GetItemType();
    const auto itemSubType = item->GetSubType();

    const auto now = get_dword_time();

    auto mapIndex = ch->GetMapIndex();

    /*
            if (now - ch->GetLastAttackTime() <= 1500 ||
            now - ch->GetLastSkillTime() <= 1500 ||
            now - ch->GetLastHitReceivedTime() <= 1500) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                "You can wear it only when you are still.");
            return false;
        }
     */

    if (ch->GetJob() == JOB_ASSASSIN && ch->GetSkillGroup() == 2 &&
        item->GetItemType() == ITEM_WEAPON) {
        if (now - ch->GetLastSkillTime() <= 1500) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can only change your equipment while you "
                               "are still.");
            return false;
        }
    } else {
        if (now - ch->GetLastPlayerHitReceivedTime() <= 1000 ||
            now - ch->GetLastSkillTime() <= 1500 ||
            now - ch->GetLastAttackTime() <= 1000) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can only change your equipment while you "
                               "are still.");
            return false;
        }
    }

    // ¾ÆÀÌÅÛ unequip½Ã ÀÎº¥Åä¸®·Î ¿Å±æ ¶§ ºó ÀÚ¸®°¡ ÀÖ´Â Áö È®ÀÎ
    {
        int pos = -1;

        if (item->IsDragonSoul())
            pos = ch->GetEmptyDragonSoulInventory(item);
        else
            pos = ch->GetEmptyInventory(item);

        // Not enough space, but we are swapping, so let's figure out if I can
        // unequip after the theorical swap - [Think]
        if (pos == -1 && !item->IsDragonSoul() && swapCell != NPOS) {
            CItem* destItem = ch->GetItem(swapCell);

            if (destItem) {
                if (destItem->GetSize() - item->GetSize() >=
                    0) // Target is equal/larger than after the change
                    return true;

                if (ch->IsEmptyItemGrid(
                        TItemPos(INVENTORY, destItem->GetCell()),
                        item->GetSize(), destItem))
                    return true;
            }
        }

        if (-1 == pos) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "소지품에 빈 공간이 없습니다.");
            return false;
        }
    }

    return true;
}

bool CanUseItem(CHARACTER* ch, const CItem* item)
{
    return CheckItemLimits(ch, item, false) && CheckItemRace(ch, item);
}

bool CanTradeRestrictedItem(CItem* item, CHARACTER* player)
{
    if (!player->IsRestrictedAccount())
        return true;

    return IS_SET(item->GetFlag(), ITEM_FLAG_MONEY_ITEM);
}

bool CanTradeRestrictedItem(const TItemTable* item, CHARACTER* player)
{
    if (!player->IsRestrictedAccount())
        return true;

    return IS_SET(item->dwFlags, ITEM_FLAG_MONEY_ITEM);
}

bool CanModifyCostumeAttr(CItem* item, CItem* itemTo)
{
    if (item->GetItemType() != ITEM_USE)
        return false;

    if (item->GetSubType() != USE_CHANGE_COSTUME_ATTR &&
        item->GetSubType() != USE_RESET_COSTUME_ATTR)
        return false;

    switch (itemTo->GetSubType()) {
        case COSTUME_BODY:
        case COSTUME_HAIR:
        case COSTUME_WEAPON:
            return true;

        default:
            break;
    }
    return false;
}

bool CanModifyEquipAttr(CItem* item, CItem* itemTo)
{
    if (item->GetItemType() != ITEM_USE)
        return false;

    if (itemTo->IsSealed())
        return false;

    switch (item->GetSubType()) {
        case USE_RESET_COSTUME_ATTR:
        case USE_CHANGE_COSTUME_ATTR:
            return false;
        default:
            break;
    }

    return true;
}

bool CanModifyItemAttr(CItem* item, CItem* itemTo, CHARACTER* ch)
{
    if (!itemTo)
        return false;

    if (itemTo->IsEquipped())
        return false;

    if (itemTo->IsExchanging())
        return false;

    if (itemTo->IsSealed()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "ITEM_SEALED_CANNOT_MODIFY");
        return false;
    }

#ifdef ENABLE_ANTI_CHANGE_ATTR
    if (IS_SET(itemTo->GetAntiFlag(), ITEM_ANTIFLAG_CHANGE_ATTRIBUTE)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "CHANGE_ATTR_CANNOT_CHANGE_THIS_ITEM");
        return false;
    }
#endif

    if (itemTo->GetAttributeSetIndex() == -1 &&
        !item->IsSocketModifyingItem()) {
        return false;
    }

    switch (itemTo->GetItemType()) {
        case ITEM_COSTUME:
            return CanModifyCostumeAttr(item, itemTo);
        case ITEM_WEAPON:
        case ITEM_ARMOR:
            return CanModifyEquipAttr(item, itemTo);
        default:
            break;
    }

    return true;
}

bool IsOpenPvPSash(const CItem* sash)
{
    return sash->GetVnum() >= 88000 && sash->GetVnum() <= 88169;
}

uint8_t GetRefineLevel(const TItemTable* proto)
{
    std::string name = proto->szName;

    auto pos = name.find('+');
    if (pos != std::string::npos) {
        uint32_t level = 0;
        storm::ParseNumber(name.substr(pos + 1).c_str(), level);
        return level;
    }

    return 0;
}

bool SwitchbotIsRareConfigured(CHARACTER* ch, uint32_t slotIndex)
{
    auto slotData = ch->GetSwitchbotSlotData(slotIndex);
    if (!slotData) {
        return false;
    }

    return slotData.value().AltRareConfigured(0) ||
           slotData.value().AltRareConfigured(1) ||
           slotData.value().AltRareConfigured(2);
}

bool SwitchbotCheckItem(CItem* item)
{
    if (!item)
        return false;

    auto ch = item->GetOwner();

    if (!ch)
        return false;

    bool checked = false;
    auto slotData = ch->GetSwitchbotSlotData(item->GetPosition().cell);
    if (!slotData) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "No attributes set in slot %d of the switchbot. "
                           "Done",
                           item->GetPosition().cell);
        return true;
    }

    uint32_t altIndex = 0;
    for (const auto& alternative : slotData.value().attr) {
        if (!slotData.value().AltConfigured(altIndex)) {
            continue;
        }

        ++altIndex;

        uint8_t configuredAttrCount = 0;
        uint8_t correctAttrCount = 0;

        for (const auto& destAttr : alternative) {
            if (!destAttr.bType || destAttr.lValue == 0.0f) {
                continue;
            }

            ++configuredAttrCount;

            for (uint8_t attrIdx = 0; attrIdx < ITEM_ATTRIBUTE_MAX_NUM;
                 ++attrIdx) {
                const TPlayerItemAttribute& curAttr =
                    item->GetAttribute(attrIdx);

                if (curAttr.bType != destAttr.bType ||
                    curAttr.sValue < destAttr.lValue) {
                    continue;
                }

                ++correctAttrCount;
                break;
            }
        }

        checked = true;

        if (configuredAttrCount == correctAttrCount) {
            return true;
        }
    }

    if (!checked) {
        return true;
    }

    return false;
}
