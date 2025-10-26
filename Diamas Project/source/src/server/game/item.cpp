#include "item.h"
#include "DbCacheSocket.hpp"
#include "DragonSoul.h"
#include "ItemUtils.h"
#include "TextTagUtil.hpp"
#include "buff_on_attributes.h"
#include "char.h"
#include "config.h"
#include "desc.h"
#include "item_addon.h"
#include "item_manager.h"
#include "locale_service.h"
#include "log.h"
#include "marriage.h"
#include "pvp.h"
#include "sectree_manager.h"
#include "skill.h"
#include "utils.h"
#include <boost/signals2/connection.hpp>
#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>

#include "PacketUtils.hpp"

CItem::CItem(ItemVnum vnum)
    : CEntity(ENTITY_ITEM)
    , m_vnum(vnum)
    , m_pkDestroyEvent(nullptr)
    , m_pkExpireEvent(nullptr)
    , m_pkUniqueExpireEvent(nullptr)
    , m_pkTimerBasedOnWearExpireEvent(nullptr)
    , m_pkRealTimeExpireEvent(nullptr)
    , m_pkAccessorySocketExpireEvent(nullptr)
    , m_pkOwnershipEvent(nullptr)
    , m_selfSwitchEvent(nullptr)
{
}

CItem::~CItem()
{
    event_cancel(&m_pkDestroyEvent);
    event_cancel(&m_pkOwnershipEvent);
    event_cancel(&m_pkUniqueExpireEvent);
    event_cancel(&m_pkTimerBasedOnWearExpireEvent);
    event_cancel(&m_pkRealTimeExpireEvent);
    event_cancel(&m_pkAccessorySocketExpireEvent);
    event_cancel(&m_selfSwitchEvent);

    CEntity::Destroy();

    if (GetSectree())
        GetSectree()->RemoveEntity(this);
}

EVENTFUNC(item_destroy_event)
{
    auto info = static_cast<item_event_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("item_destroy_event> <Factor> Null pointer");
        return 0;
    }

    auto pkItem = info->item;
    if (pkItem->GetOwner())
        SPDLOG_ERROR("item_destroy_event: Owner exist. (item {0} owner {1})",
                     pkItem->GetVnum(), pkItem->GetOwner()->GetName());
    
    pkItem->SetDestroyEvent(nullptr);
    M2_DESTROY_ITEM(pkItem);
    return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
    m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int iSec)
{
    if (m_pkDestroyEvent)
        return;

    auto info = AllocEventInfo<item_event_info>();
    info->item = this;

    SetDestroyEvent(
        event_create(item_destroy_event, info, THECORE_SECS_TO_PASSES(iSec)));
}

uint32_t CItem::GetRefinedVnum() const
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwRefinedVnum : 0;
}

void CItem::SetSkipSave(bool b)
{
    m_bSkipSave = b;
}

bool CItem::GetSkipPacket() const
{
    return m_bSkipPacket;
}

void CItem::SetSkipPacket(bool b)
{
    m_bSkipPacket = b;
}

bool CItem::GetSkipSave() const
{
    return m_bSkipSave;
}

uint32_t CItem::GetOwnerPid() const
{
    return m_dwOwnershipPID;
}

bool CItem::WasMine() const
{
    return m_wasMine;
}

void CItem::WasMine(bool v)
{
    m_wasMine = v;
}

uint32_t CItem::GetLastOwnerPID() const
{
    return m_dwLastOwnerPID;
}

uint16_t CItem::GetRefineSet() const
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->wRefineSet : 0;
}

void CItem::EncodeInsertPacket(CEntity* ent)
{
    auto d = ent->GetDesc();
    if (!d)
        return;

    const auto& [x, y, z] = GetXYZ();

    GcPacketItemGroundAdd p = {x, y, z, m_dwVID, GetVnum()};

    d->Send(HEADER_GC_ITEM_GROUND_ADD, p);

    if (!m_pkOwnershipEvent)
        return;

    auto info = dynamic_cast<item_event_info*>(m_pkOwnershipEvent->info);

    if (info == nullptr) {
        SPDLOG_ERROR("CItem::EncodeInsertPacket> <Factor> Null pointer");
        return;
    }

    TPacketGCItemOwnership pOwner;
    pOwner.dwVID = m_dwVID;
    pOwner.szName = info->szOwnerName;

    d->Send(HEADER_GC_ITEM_OWNERSHIP, pOwner);
}

void CItem::EncodeRemovePacket(CEntity* ent)
{
    auto d = ent->GetDesc();
    if (!d)
        return;

    TPacketGCItemGroundDel pack = {};
    pack.dwVID = m_dwVID;
    d->Send(HEADER_GC_ITEM_GROUND_DEL, pack);

    SPDLOG_TRACE("Item::EncodeRemovePacket {0} to {1}", GetVnum(),
                 ((CHARACTER*)ent)->GetName());
}

void CItem::RemoveFlag(long bit)
{
    REMOVE_BIT(m_lFlag, bit);
}

uint32_t CItem::GetWearFlag() const
{
    const auto*  proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwWearFlags : 0;
}

uint64_t CItem::GetAntiFlag() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwAntiFlags : 0;
}

uint32_t CItem::GetImmuneFlag() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwImmuneFlag : 0;
}

void CItem::SetVID(uint32_t vid)
{
    m_dwVID = vid;
}

uint32_t CItem::GetVID()
{
    return m_dwVID;
}

void CItem::AddFlag(long bit)
{
    SET_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
    if (!m_pOwner || !m_pOwner->GetDesc())
        return;

    if (m_bSkipPacket)
        return;

    GcItemSetPacket pack;
    pack.pos = GetPosition();
    pack.data.id = GetID();

    pack.data.vnum = GetVnum();
    pack.data.count = m_dwCount;

    for (auto i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
        pack.data.sockets[i] = m_alSockets[i];

    memcpy(pack.data.attrs, GetAttributes(), sizeof(pack.data.attrs));
    pack.data.nSealDate = m_sealDate;
    pack.data.transVnum = m_transmutationVnum;
    pack.data.highlighted =
        !m_wasMine && (GetPosition().window_type == INVENTORY ||
                       GetPosition().window_type == DRAGON_SOUL_INVENTORY);

    m_pOwner->GetDesc()->Send(HEADER_GC_ITEM_SET, pack);
}

CountType CItem::GetCount() const
{
    if (GetItemType() == ITEM_ELK)
        return std::min<CountType>(m_dwCount,
                                   std::numeric_limits<CountType>::max());

    return std::min<CountType>(m_dwCount, GetItemMaxCount(this));
}

uint32_t CItem::GetVnum() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return m_dwMaskVnum ? m_dwMaskVnum : proto->dwVnum;
}

uint32_t CItem::GetTransmutationVnum() const
{
    return m_transmutationVnum;
}

void CItem::SetTransmutationVnum(uint32_t transVnum)
{
    m_transmutationVnum = transVnum;
    Save();
}

uint32_t CItem::GetOriginalVnum() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwVnum : 0;
}

uint8_t CItem::GetItemType() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->bType : 0;
}

uint8_t CItem::GetSubType() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->bSubType : 0;
}

uint8_t CItem::GetLimitType(uint32_t idx) const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->aLimits[idx].bType : 0;
}

long CItem::GetLimitValue(uint32_t idx) const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->aLimits[idx].value : 0;
}

int32_t CItem::GetLevel() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? (proto->aLimits[0].bType == LIMIT_LEVEL
                        ? proto->aLimits[0].value
                        : 0)
                 : 0;
}

bool CItem::IsSocketModifyingItem() const
{
    return GetItemType() == ITEM_USE &&
           (GetSubType() == USE_PUT_INTO_BELT_SOCKET ||
            GetSubType() == USE_PUT_INTO_RING_SOCKET ||
            GetSubType() == USE_PUT_INTO_ACCESSORY_SOCKET ||
            GetSubType() == USE_ADD_ACCESSORY_SOCKET ||
            GetSubType() == USE_CLEAN_SOCKET ||
            GetSubType() == USE_MAKE_ACCESSORY_SOCKET_PERM);
}

bool CItem::IsWeapon() const
{
    return GetItemType() == ITEM_WEAPON;
}

bool CItem::IsSword() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_SWORD;
}

bool CItem::IsDagger() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_DAGGER;
}

bool CItem::IsBow() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_BOW;
}

bool CItem::IsTwoHandSword() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_TWO_HANDED;
}

bool CItem::IsBell() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_BELL;
}

bool CItem::IsFan() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_FAN;
}

bool CItem::IsArrow() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_ARROW;
}

bool CItem::IsMountSpear() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_MOUNT_SPEAR;
}

bool CItem::IsClaw() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_CLAW;
}

bool CItem::IsQuiver() const
{
    return GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_NUM_TYPES;
}

bool CItem::IsArmor() const
{
    return GetItemType() == ITEM_ARMOR;
}

bool CItem::IsArmorBody() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_BODY;
}

bool CItem::IsHelmet() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_HEAD;
}

bool CItem::IsShield() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_SHIELD;
}

bool CItem::IsWrist() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_WRIST;
}

bool CItem::IsShoe() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_FOOTS;
}

bool CItem::IsNecklace() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_NECK;
}

bool CItem::IsEarRing() const
{
    return GetItemType() == ITEM_ARMOR && GetSubType() == ARMOR_EAR;
}

bool CItem::IsRing() const
{
    return GetItemType() == ITEM_RING;
}

bool CItem::IsCostume() const
{
    return GetItemType() == ITEM_COSTUME;
}

bool CItem::IsCostumeMount() const
{
    return GetItemType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT;
}

bool CItem::IsCostumeHair() const
{
    return GetItemType() == ITEM_COSTUME && GetSubType() == COSTUME_HAIR;
}

bool CItem::IsCostumeBody() const
{
    return GetItemType() == ITEM_COSTUME && GetSubType() == COSTUME_BODY;
}

bool CItem::IsCostumeAcce() const
{
    return GetItemType() == ITEM_COSTUME && GetSubType() == COSTUME_ACCE;
}

bool CItem::IsCostumeWeapon() const
{
    return GetItemType() == ITEM_COSTUME && GetSubType() == COSTUME_WEAPON;
}

bool CItem::IsOldHair() const
{
    return GetVnum() >= 74001 && GetVnum() <= 75620;
}

bool CItem::IsCostumeModifyItem() const
{
    return GetItemType() == ITEM_USE &&
           (GetSubType() == USE_CHANGE_COSTUME_ATTR ||
            GetSubType() == USE_RESET_COSTUME_ATTR);
}

bool CItem::IsBelt() const
{
    return GetItemType() == ITEM_BELT;
}

bool CItem::SetCount(CountType count)
{
    if (GetItemType() == ITEM_ELK) {
        m_dwCount = std::min<CountType>(count, gConfig.maxGold);
    } else {
        m_dwCount = std::min<CountType>(count, GetItemMaxCount(this));
    }

    if (count == 0 && m_pOwner) {
        ITEM_MANAGER::instance().RemoveItem(this, "COUNT_ZERO");
        return false;
    }

    UpdatePacket();
    Save();
    return true;
}

void CItem::RemoveFromCharacter()
{
    switch (m_itemPos.window_type) {
        case SAFEBOX:
        case MALL:
            m_pOwner = nullptr;
            return;
        default:
            break;
    }

    if (m_pOwner)
        m_pOwner->SetItem(m_itemPos, nullptr);
}

bool CItem::AddToCharacter(CHARACTER* ch, const TItemPos& Cell)
{
    STORM_ASSERT(Cell.window_type != SAFEBOX && Cell.window_type != MALL,
                 "Use the corresponding CSafebox methods");

    if (ch->GetDesc())
        m_wasMine = GetLastOwnerPID() == ch->GetPlayerID();

    ch->SetItem(Cell, this);
    return true;
}

CItem* CItem::RemoveFromGround()
{
    if (GetSectree()) {
        SetOwnership(nullptr);

        GetSectree()->RemoveEntity(this);

        ViewCleanup();

        Save();
    }

    return (this);
}

bool CItem::AddToGround(long lMapIndex, const PIXEL_POSITION& pos,
                        bool skipOwnerCheck)
{
    if (0 == lMapIndex) {
        SPDLOG_ERROR("wrong map index argument: {0}", lMapIndex);
        return false;
    }

    if (GetSectree()) {
        SPDLOG_ERROR("sectree already assigned");
        return false;
    }

    if (!skipOwnerCheck && m_pOwner) {
        SPDLOG_ERROR("owner pointer not null");
        return false;
    }

    auto* tree = SECTREE_MANAGER::instance().Get(lMapIndex, pos.x, pos.y);
    if (!tree) {
        SPDLOG_ERROR("cannot find sectree by {0}x{1}", pos.x, pos.y);
        return false;
    }

    m_itemPos.window_type = GROUND;

    SetXYZ(pos.x, pos.y, pos.z);

    tree->InsertEntity(this);

    UpdateSectree();
    Save();
    return true;
}

bool CItem::DistanceValid(CHARACTER* ch)
{
    if (!GetSectree())
        return false;

    int iDist = DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY());

    if (iDist > 300)
        return false;

    return true;
}

boost::signals2::connection CItem::AddOnDestroy(const OnDestroy::slot_type& s)
{
    return m_onDestroy.connect(s);
}

bool CanHaveAccessories(uint32_t type, uint32_t subType)
{
    switch (type) {
        case ITEM_WEAPON: {
            switch (subType) {
                case WEAPON_ARROW:
                case WEAPON_QUIVER:
                    return false;

                default:
                    return true;
            }
        }

        case ITEM_ARMOR:
            return true;

        default:
            return false;
    }
}

void CItem::ModifyParts(bool bAdd)
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return;

    Part part = {GetVnum(), GetTransmutationVnum(), 0, 0};
    uint8_t partIdx = PART_MAX_NUM;

    switch (proto->bType) {
        case ITEM_PICK:
        case ITEM_WEAPON:
        case ITEM_ROD: {
            switch (GetSubType()) {
                case WEAPON_ARROW:
                case WEAPON_QUIVER: {
                    if (m_itemPos.cell != WEAR_ARROW)
                        break;

                    partIdx = PART_ARROW;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(PART_ARROW);
                } break;
                default: {
                    if (m_pOwner->GetWear(WEAR_COSTUME_WEAPON))
                        break;

                    if (m_itemPos.cell != WEAR_WEAPON)
                        break;

                    partIdx = PART_WEAPON;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(PART_WEAPON);
                } break;
            }

            break;
        }
        case ITEM_ARMOR: {
            if (m_pOwner->GetWear(WEAR_COSTUME_BODY))
                break;

            if (GetPosition().cell != WEAR_BODY)
                break;

            partIdx = PART_MAIN;

            if (!bAdd)
                part = m_pOwner->GetOriginalPart(PART_MAIN);

            break;
        }

        case ITEM_COSTUME: {
            switch (GetSubType()) {
                case COSTUME_BODY: {
                    partIdx = PART_MAIN;

                    // Default vnum/appearance is fine
                    if (bAdd) {
                        const auto* armor = m_pOwner->GetWear(WEAR_BODY);
                        if (armor)
                            part = {armor->GetVnum(),
                                    armor->GetTransmutationVnum(), GetVnum(),
                                    GetTransmutationVnum()};
                        else
                            part = {0, 0, GetVnum(), GetTransmutationVnum()};
                        break;
                    }

                    const auto* armor = m_pOwner->GetWear(WEAR_BODY);
                    if (armor)
                        part = {armor->GetVnum(), armor->GetTransmutationVnum(),
                                0, 0};
                    else
                        part = m_pOwner->GetOriginalPart(partIdx);
                    break;
                }

                case COSTUME_HAIR: {
                    partIdx = PART_HAIR;

                    if (bAdd) {
                        // Only send value3 instead of vnum if it is set,
                        // otherwise it prevents using default appearance
                        if (GetValue(3)) {
                            const auto itemTable =
                                ITEM_MANAGER::instance().GetTable(
                                    GetTransmutationVnum());
                            if (itemTable)
                                part.costume_appearance =
                                    itemTable->alValues[3];

                            part.costume = GetValue(3);
                            part.vnum =
                                m_pOwner->GetOriginalPart(PART_HAIR).vnum;
                            part.appearance =
                                m_pOwner->GetOriginalPart(PART_HAIR).appearance;
                        }
                    } else {
                        part = m_pOwner->GetOriginalPart(PART_HAIR);
                    }
                    break;
                }

                case COSTUME_RANK: {
                    partIdx = PART_RANK;

                    if (bAdd) {
                        if (GetValue(0)) {
                            const auto* itemTable =
                                ITEM_MANAGER::instance().GetTable(
                                    GetTransmutationVnum());
                            if (itemTable)
                                part.appearance = itemTable->alValues[0];

                            part.vnum = GetValue(0);
                        }
                    } else {
                        part = m_pOwner->GetOriginalPart(PART_RANK);
                    }
                    break;
                }

                case COSTUME_WEAPON: {
                    partIdx = PART_WEAPON;

                    // Default vnum/appearance is fine
                    if (bAdd) {
                        const auto* weapon = m_pOwner->GetWear(WEAR_WEAPON);
                        if (weapon)
                            part = {weapon->GetVnum(),
                                    weapon->GetTransmutationVnum(), GetVnum(),
                                    GetTransmutationVnum()};
                        else
                            part = {0, 0, GetVnum(), GetTransmutationVnum()};
                        break;
                    }

                    const auto weapon = m_pOwner->GetWear(WEAR_WEAPON);
                    if (weapon)
                        part = {weapon->GetVnum(),
                                weapon->GetTransmutationVnum(), 0, 0};
                    else
                        part = m_pOwner->GetOriginalPart(partIdx);
                    break;
                }

                case COSTUME_ACCE: {
                    partIdx = PART_ACCE;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(PART_ACCE);

                    break;
                }

                case COSTUME_BODY_EFFECT: {
                    partIdx = PART_BODY_EFFECT;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(partIdx);

                    break;
                }

                case COSTUME_WING_EFFECT: {
                    partIdx = PART_WING_EFFECT;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(partIdx);

                    break;
                }

                case COSTUME_WEAPON_EFFECT: {
                    partIdx = PART_WEAPON_EFFECT;

                    if (!bAdd)
                        part = m_pOwner->GetOriginalPart(partIdx);

                    break;
                }
                default:
                    break;
            }
        } break;
        default:
            break;
    }

    if (PART_MAX_NUM != partIdx) {
        m_pOwner->SetPart(partIdx, part);
        m_pOwner->UpdatePacket();
    }
}

void CItem::ModifyPointsAcce(bool bAdd)
{
    const auto vnum = m_alSockets[1];
    if (vnum == 0)
        return;

    const auto f = m_alSockets[0];

    const auto* p = ITEM_MANAGER::instance().GetTable(vnum);
    if (!p)
        return;

    for (const auto& apply : p->aApplies) {
        if (apply.bType == APPLY_NONE || apply.lValue <= 0)
            continue;

        ApplyValue value = std::max<ApplyValue>(1, apply.lValue * f / 100);

        m_pOwner->ApplyPoint(apply.lValue, bAdd ? value : -value);
    }
}

void CItem::ModifyPoints(bool bAdd, CHARACTER* ch)
{
    if (!ch)
        ch = m_pOwner;

    if (!ch)
        return;

    auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return;

    int accessoryGrade = 0;
    int accessoryMinGrade = 0;
    // 무기와 갑옷만 소켓을 적용시킨다.
    if (!IsAccessoryForSocket()) {
        if (CanHaveAccessories(proto->bType, proto->bSubType)) {
            // 소켓이 속성강화에 사용되는 경우 적용하지 않는다 (ARMOR_WRIST
            // ARMOR_NECK ARMOR_EAR)
            for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
                uint32_t dwVnum;

                if ((dwVnum = GetSocket(i)) <= 2)
                    continue;

                const auto* p = ITEM_MANAGER::instance().GetTable(dwVnum);

                if (!p) {
                    SPDLOG_ERROR("cannot find table by vnum %u", dwVnum);
                    continue;
                }

                if (ITEM_METIN == p->bType) {
                    // m_pOwner->ApplyPoint(p->alValues[0], bAdd ?
                    // p->alValues[1] : -p->alValues[1]);
                    for (auto aApplie : p->aApplies) {
                        if (aApplie.bType == APPLY_NONE)
                            continue;

                        if (aApplie.bType == APPLY_SKILL)
                            ch->ApplyPoint(aApplie.bType,
                                           bAdd ? aApplie.lValue
                                                : int(aApplie.lValue) ^
                                                      0x00800000);
                        else
                            ch->ApplyPoint(aApplie.bType,
                                           bAdd ? aApplie.lValue
                                                : -aApplie.lValue);
                    }
                }
            }
        }
    } else {
        accessoryGrade = std::min<int>(GetAccessorySocketGrade(),
                                       ITEM_ACCESSORY_SOCKET_MAX_NUM);
        accessoryMinGrade = std::min<int>(GetAccessorySocketMinGrade(),
                                          ITEM_ACCESSORY_SOCKET_MAX_NUM);
    }

    constexpr bool isApplyDebuff = false;
    /*if (IsTier4Map(m_pOwner->GetMapIndex()))
        isApplyDebuff = IS_SET(GetFlag(), ITEM_FLAG_TIER_4) == 0 &&
    (GetItemType() == ITEM_ARMOR || GetItemType() == ITEM_WEAPON); else
    isApplyDebuff = IS_SET(GetFlag(), ITEM_FLAG_TIER_4) && (GetItemType() ==
    ITEM_ARMOR || GetItemType() == ITEM_WEAPON);
        */

    if (isApplyDebuff) {
        auto vnum = ITEM_MANAGER::instance().GetItemDebuffVnum(GetVnum());
        if (vnum) {
            const auto p = ITEM_MANAGER::instance().GetTable(vnum);
            if (!p)
                return;

            for (const auto& apply : p->aApplies) {
                if (apply.bType == APPLY_NONE)
                    continue;

                if (apply.bType == APPLY_SKILL)
                    ch->ApplyPoint(apply.lValue,
                                   bAdd ? apply.lValue
                                        : int(apply.lValue) ^ 0x00800000);
                else
                    ch->ApplyPoint(apply.lValue,
                                   bAdd ? apply.lValue : -apply.lValue);
            }
        } else {
            for (const auto& apply : proto->aApplies) {
                if (apply.bType == APPLY_NONE)
                    continue;

                int32_t value;

                if (ch->GetMapIndex() != 507)
                    value = apply.lValue * 0.45;
                else
                    value = apply.lValue;

                if (apply.bType == APPLY_SKILL) {
                    ch->ApplyPoint(apply.bType,
                                   bAdd ? value : value ^ 0x00800000);
                } else {
                    if (0 != accessoryGrade)
                        value += std::max<int>(
                            accessoryGrade,
                            value *
                                aiAccessorySocketEffectivePct[accessoryGrade] /
                                100);

                    ch->ApplyPoint(apply.bType, bAdd ? value : -value);
                }
            }
        }
    } else {
        for (const auto& apply : proto->aApplies) {
            if (apply.bType == APPLY_NONE)
                continue;

            ApplyValue value = apply.lValue;

            if (0 != accessoryGrade) {
                if (accessoryGrade == accessoryMinGrade) {
                    value += std::max<int>(
                        accessoryGrade,
                        value *
                            aiAccessorySocketPermEffectivePct[accessoryGrade] /
                            100);
                } else {
                    value += std::max<int>(
                        accessoryGrade,
                        value * aiAccessorySocketEffectivePct[accessoryGrade] /
                            100);
                }
            }

            ch->ApplyPoint(apply.bType, bAdd ? value : -value);
        }
    }

    if (ITEM_COSTUME == GetItemType() && COSTUME_ACCE == GetSubType()) {
        ModifyPointsAcce(bAdd);
    }

    for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i) {
        if (GetAttributeType(i)) {
            const TPlayerItemAttribute& ia = GetAttribute(i);

            float attrVal = ia.sValue;
            if (ITEM_COSTUME == GetItemType() && COSTUME_ACCE == GetSubType()) {
                attrVal = long(float(ia.sValue) / 100.0f * float(GetSocket(0)));
                if (attrVal < 0)
                    attrVal = 1;
            }

            ch->ApplyPoint(ia.bType, bAdd ? attrVal : -attrVal);
        }
    }

    if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
        ch->ShowAlignment(!bAdd);

    ModifyParts(bAdd);

    switch (proto->bType) {
        case ITEM_UNIQUE: {
            if (0 == GetSIGVnum())
                break;

            const CSpecialItemGroup* pItemGroup =
                ITEM_MANAGER::instance().GetSpecialItemGroup(GetSIGVnum());
            if (nullptr == pItemGroup)
                break;

            uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());

            const CSpecialAttrGroup* pAttrGroup =
                ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
            if (nullptr == pAttrGroup)
                break;

            for (auto m_vecAttr : pAttrGroup->m_vecAttrs)
                ch->ApplyPoint(m_vecAttr.apply_type,
                               bAdd ? m_vecAttr.apply_value
                                    : -m_vecAttr.apply_value);

            break;
        }
        default:
            break;
    }
}

bool CItem::IsEquipable() const
{
    switch (this->GetItemType()) {
        case ITEM_COSTUME:
        case ITEM_ARMOR:
        case ITEM_WEAPON:
        case ITEM_ROD:
        case ITEM_PICK:
        case ITEM_UNIQUE:
        case ITEM_DS:
        case ITEM_SPECIAL_DS:
        case ITEM_TALISMAN:
        case ITEM_RING:
        case ITEM_BELT:
            return true;
        default:
            break;
    }

    return false;
}

long CItem::GetValue(uint32_t idx) const
{
    assert(idx < ITEM_VALUES_MAX_NUM);
    return GetProto()->alValues[idx];
}

long CItem::GetApply(uint32_t idx) const
{
    assert(idx < ITEM_APPLY_MAX_NUM);
    return GetProto()->aApplies[idx].lValue;
}

uint8_t CItem::GetWindow() const
{
    return m_itemPos.window_type;
}

uint16_t CItem::GetCell() const
{
    return m_itemPos.cell;
}

TItemPos CItem::GetPosition() const
{
    return m_itemPos;
}

void CItem::SetPosition(const TItemPos& pos)
{
    m_itemPos = pos;
    Save();
}

CHARACTER* CItem::GetOwner() const
{
    return m_pOwner;
}

void CItem::SetOwner(CHARACTER* ch)
{
    if (ch) {
        if (m_pOwner)
            m_dwLastOwnerPID = m_pOwner->GetPlayerID();
        else
            m_dwLastOwnerPID = ch->GetPlayerID();
        event_cancel(&m_pkDestroyEvent);
    }

    m_pOwner = ch;

    Save();
}

uint16_t CItem::GetCell()
{
    return m_itemPos.cell;
}

CHARACTER* CItem::GetOwner()
{
    return m_pOwner;
}

int CItem::GetBelongingItemID()
{
    return static_cast<int>(GetSocket(0));
}

bool CItem::IsEquipped() const
{
    return m_itemPos.window_type == EQUIPMENT;
}

bool CItem::HaveOwnership() const
{
    return (m_pkOwnershipEvent ? true : false);
}

bool CItem::IsExchanging() const
{
    return m_bExchanging;
}

void CItem::SetExchanging(bool bOn)
{
    m_bExchanging = bOn;
}

void CItem::Save()
{
    if (m_bSkipSave)
        return;

    ITEM_MANAGER::instance().DelayedSave(this);
}

void CItem::SetID(uint32_t id)
{
    m_dwID = id;
}

uint32_t CItem::GetID() const
{
    return m_dwID;
}

bool CItem::CreateSocket(uint8_t bSlot, uint8_t bGold)
{
    assert(bSlot < ITEM_SOCKET_MAX_NUM);

    if (m_alSockets[bSlot] != 0) {
        SPDLOG_ERROR("Item::CreateSocket : socket already exist {} {}",
                     GetName(), bSlot);
        return false;
    }

    if (bGold)
        m_alSockets[bSlot] = 2;
    else
        m_alSockets[bSlot] = 1;

    UpdatePacket();

    Save();
    return true;
}

const SocketValue* CItem::GetSockets()
{
    return &m_alSockets[0];
}

SocketValue CItem::GetSocket(int i) const
{
    return m_alSockets[i];
}

const TPlayerItemAttribute* CItem::GetAttributes() const
{
    return m_aAttr;
}

const TPlayerItemAttribute& CItem::GetAttribute(int i) const
{
    return m_aAttr[i];
}

uint8_t CItem::GetAttributeType(int i) const
{
    return m_aAttr[i].bType;
}

ApplyValue CItem::GetAttributeValue(int i) const
{
    return m_aAttr[i].sValue;
}

void CItem::SetSockets(const SocketValue* c_al)
{
    std::memcpy(m_alSockets, c_al, sizeof(m_alSockets));
    Save();
}

void CItem::HandleToggleMountSocketState(SocketValue v)
{
    assert(GetItemType() == ITEM_TOGGLE && GetSubType() == TOGGLE_MOUNT);

    if (v) {
        uint32_t mountVnum;
        if (m_transmutationVnum) {
            const auto* table =
                ITEM_MANAGER::instance().GetTable(m_transmutationVnum);
            if (table) {
                mountVnum = table->alValues[ITEM_VALUE_MOUNT_VNUM];
            } else {
                mountVnum = GetValue(ITEM_VALUE_MOUNT_VNUM);
            }
        } else {
            mountVnum = GetValue(ITEM_VALUE_MOUNT_VNUM);
        }
        m_pOwner->MountVnum(mountVnum);
        m_pOwner->DestroyToggleMount();
    } else {
        if (!m_pOwner->IsDead() && !m_pOwner->IsStun()) {
            m_pOwner->MountVnum(0);
        } else {
            m_pOwner->SetMountVnum(0);
            m_pOwner->UpdatePacket();
        }

        m_pOwner->CreateToggleMount(this);
    }
}

void CItem::SetSocket(int i, SocketValue v, bool bLog)
{
    assert(i < ITEM_SOCKET_MAX_NUM);

    if (GetItemType() == ITEM_TOGGLE && GetSubType() == TOGGLE_MOUNT &&
        i == ITEM_SOCKET_TOGGLE_RIDING) {
        if (bLog)
            ModifyPoints(v);
        HandleToggleMountSocketState(v);
    }

    m_alSockets[i] = v;
    UpdatePacket();
    Save();
}

const TItemTable* CItem::GetProto() const
{
    return ITEM_MANAGER::instance().GetTable(m_vnum);
}

Gold CItem::GetShopBuyPrice() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwShopBuyPrice : std::numeric_limits<int64_t>::max();
}

const char* CItem::GetName() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    return proto ? proto->szLocaleName : nullptr;
}

const char* CItem::GetBaseName() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    return proto ? proto->szName : nullptr;
}

uint8_t CItem::GetSize() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->bSize : 0;
}

uint64_t CItem::GetFlag() const
{
    const auto* proto = ITEM_MANAGER::instance().GetTable(m_vnum);

    return proto ? proto->dwFlags : 0;
}

bool CItem::IsSlowQuery()
{
    return (GetOwner() && IS_SET(GetFlag(), ITEM_FLAG_SLOW_QUERY));
}

bool CItem::IsOwnership(CHARACTER* ch) const
{
    if (!m_pkOwnershipEvent)
        return true;

    return m_dwOwnershipPID == ch->GetPlayerID();
}

EVENTFUNC(ownership_event)
{
    item_event_info* info = static_cast<item_event_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("ownership_event> <Factor> Null pointer");
        return 0;
    }

    info->item->SetOwnership(nullptr);

    return 0;
}

void CItem::SetOwnership(CHARACTER* ch, int iSec)
{
    if (iSec == 1243)
        iSec = gConfig.itemOwnershipTime;

    if (!ch) {
        if (m_pkOwnershipEvent) {
            event_cancel(&m_pkOwnershipEvent);
            m_dwOwnershipPID = 0;

            TPacketGCItemOwnership p;
            p.dwVID = m_dwVID;
            p.szName[0] = '\0';
            PacketAround(m_map_view, this, HEADER_GC_ITEM_OWNERSHIP, p);
        }

        return;
    }

    if (m_pkOwnershipEvent)
        return;

    if (iSec <= 10)
        iSec = 30;

    m_dwOwnershipPID = ch->GetPlayerID();
    m_dwLastOwnerPID = ch->GetPlayerID();

    item_event_info* info = AllocEventInfo<item_event_info>();
    storm::CopyStringSafe(info->szOwnerName, ch->GetName());
    info->item = this;

    m_pkOwnershipEvent =
        event_create(ownership_event, info, THECORE_SECS_TO_PASSES(iSec));

    TPacketGCItemOwnership p;
    p.dwVID = m_dwVID;
    p.szName = ch->GetName();
    PacketAround(m_map_view, this, HEADER_GC_ITEM_OWNERSHIP, p);
}

int CItem::GetSocketCount() const
{
    for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++) {
        if (GetSocket(i) == 0)
            return i;
    }
    return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
    int count = GetSocketCount();
    if (count == ITEM_SOCKET_MAX_NUM)
        return false;
    m_alSockets[count] = 1;
    return true;
}

void CItem::AlterToSocketItem(int iSocketCount)
{
    if (iSocketCount >= ITEM_SOCKET_MAX_NUM) {
        SPDLOG_INFO("Invalid Socket Count {}, set to maximum",
                    ITEM_SOCKET_MAX_NUM);
        iSocketCount = ITEM_SOCKET_MAX_NUM;
    }

    for (int i = 0; i < iSocketCount; ++i)
        SetSocket(i, 1);
}

void CItem::AlterToMagicItem(int iSecondPct /*= 0*/, int iThirdPct /*= 0 */)
{

    int idx = GetAttributeSetIndex();

    if (idx < 0)
        return;

    //      Appeariance Second Third
    // Weapon 50        20     5
    // Armor  30        10     2
    // Acc    20        10     1

    switch (GetItemType()) {
        case ITEM_WEAPON:
            iSecondPct = 20;
            iThirdPct = 5;
            break;
        case ITEM_COSTUME:
            iSecondPct = 30;
            iThirdPct = 20;
            break;
        case ITEM_ARMOR:
            if (GetSubType() == ARMOR_BODY) {
                iSecondPct = 10;
                iThirdPct = 2;
            } else {
                iSecondPct = 10;
                iThirdPct = 1;
            }
            break;

        default:
            break;
    }

    // 100% 확률로 좋은 속성 하나
    PutAttribute(aiItemMagicAttributePercentHigh);

    if (Random::get(1, 100) <= iSecondPct)
        PutAttribute(aiItemMagicAttributePercentLow);

    if (Random::get(1, 100) <= iThirdPct)
        PutAttribute(aiItemMagicAttributePercentLow);
}

uint32_t CItem::GetRefineFromVnum()
{
    return ITEM_MANAGER::instance().GetRefineFromVnum(GetVnum());
}

int CItem::GetRefineLevel()
{
    const char* name = GetBaseName();
    char* p = const_cast<char*>(strrchr(name, '+'));

    if (!p)
        return 0;

    int rtn = 0;
    str_to_number(rtn, p + 1);

    const char* locale_name = GetName();
    p = const_cast<char*>(strrchr(locale_name, '+'));

    if (p) {
        int locale_rtn = 0;
        str_to_number(locale_rtn, p + 1);
        if (locale_rtn != rtn) {
            SPDLOG_ERROR("refine_level_based_on_NAME(%d) is not equal to "
                         "refine_level_based_on_LOCALE_NAME(%d).",
                         rtn, locale_rtn);
        }
    }

    return rtn;
}

bool CItem::IsPolymorphItem()
{
    return GetItemType() == ITEM_POLYMORPH;
}

EVENTFUNC(unique_expire_event)
{
    item_event_info* info = static_cast<item_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("unique_expire_event> <Factor> Null pointer");
        return 0;
    }

    CItem* pkItem = info->item;

    if (pkItem->GetValue(2) == 0) {
        if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= 1) {
            SPDLOG_INFO("UNIQUE_ITEM: expire %s %u", pkItem->GetName(),
                        pkItem->GetID());
            pkItem->SetUniqueExpireEvent(nullptr);
            ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
            return 0;
        } else {
            pkItem->SetSocket(
                ITEM_SOCKET_UNIQUE_REMAIN_TIME,
                pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - 1);
            return THECORE_SECS_TO_PASSES(60);
        }
    } else {
        time_t cur = get_global_time();

        if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= cur) {
            pkItem->SetUniqueExpireEvent(nullptr);
            ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
            return 0;
        } else {
            // 게임 내에 시간제 아이템들이 빠릿빠릿하게 사라지지 않는 버그가
            // 있어 수정 by rtsummit
            if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur < 600)
                return THECORE_SECS_TO_PASSES(
                    pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur);
            else
                return THECORE_SECS_TO_PASSES(600);
        }
    }
}

// 시간 후불제
// timer를 시작할 때에 시간 차감하는 것이 아니라,
// timer가 발화할 때에 timer가 동작한 시간 만큼 시간 차감을 한다.
EVENTFUNC(timer_based_on_wear_expire_event)
{
    item_event_info* info = static_cast<item_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("expire_event <Factor> Null pointer");
        return 0;
    }

    auto pkItem = info->item;
    auto remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) -
                       processing_time / THECORE_SECS_TO_PASSES(1);
    if (remain_time <= 0) {
        SPDLOG_INFO("ITEM EXPIRED : expired %s %u", pkItem->GetName(),
                    pkItem->GetID());
        pkItem->SetTimerBasedOnWearExpireEvent(nullptr);
        pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

        // 일단 timer based on wear 용혼석은 시간 다 되었다고 없애지 않는다.
        if (pkItem->IsDragonSoul()) {
            DSManager::instance().DeactivateDragonSoul(pkItem);
        } else {
            ITEM_MANAGER::instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_"
                                                        "EXPIRE");
        }
        return 0;
    }
    pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
    return THECORE_SECS_TO_PASSES(std::min<SocketValue>(60, remain_time));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
    m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
    m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(switch_event)
{
    rmt_ScopedCPUSample(SwitchBotEvent, 0);

    const item_vid_event_info* info =
        reinterpret_cast<const item_vid_event_info*>(event->info);
    if (nullptr == info)
        return 0;

    CItem* item = ITEM_MANAGER::instance().FindByVID(info->item_vid);
    if (nullptr == item)
        return 0;

    auto* owner = item->GetOwner();
    if (!owner)
        return 0;

    if (!item->GetPosition().IsSwitchbotPosition()) {
        return 0;
    }
    const auto slotIndex = item->GetPosition().cell;

    if(!gConfig.enableSwitchbot) {
        SendI18nSystemWhisperPacket(owner, "The switchbot was temporarily disabled.");
        owner->OnSwitchbotDone(slotIndex);
        return 0;
    }

    if (SwitchbotCheckItem(item)) {
        owner->OnSwitchbotDone(slotIndex);
        return 0;
    }

    auto switchCount = owner && owner->FindAffect(AFFECT_PREMIUM_SWITCHBOT) ? 2 : 1;

    for (int i = 0; i < switchCount; ++i) {
      auto* switchItem =
            owner->GetSwitcherByAttributeIndex(item->GetAttributeSetIndex());
        if (!switchItem) {
            SendI18nSystemWhisperPacket(owner,
                                        "Switchbot Slot %d stopped out of "
                                        "bonus changers.",
                                        slotIndex + 1);
            owner->DeactivateSwitchbotSlot(slotIndex);
            return 0;
        }

        if (item->GetAttributeCount() == 0) {
            SendI18nSystemWhisperPacket(owner,
                                        "Switchbot Slot %d stopped no bonus to "
                                        "change.",
                                        slotIndex + 1);
            owner->DeactivateSwitchbotSlot(slotIndex);
            return 0;
        }

        if (switchItem->GetSubType() != USE_CHANGE_ATTRIBUTE_PERM)
            switchItem->SetCount(switchItem->GetCount() - 1);

        item->ChangeAttribute();

        if (SwitchbotIsRareConfigured(owner, item->GetPosition().cell)) {
            auto* switchItemRare = owner->GetSwitcherByAttributeIndex(
                item->GetAttributeSetIndex(), true);
            if (!switchItemRare) {
                SendI18nSystemWhisperPacket(owner,
                                            "Switchbot Slot %d stopped out of "
                                            "bonus changers.",
                                            slotIndex + 1);
                owner->DeactivateSwitchbotSlot(slotIndex);
                return 0;
            }

            if (item->GetRareAttrCount() == 0) {
                SendI18nSystemWhisperPacket(owner,
                                            "Switchbot Slot %d stopped rare "
                                            "bonus configured but no bonus to "
                                            "change.",
                                            slotIndex + 1);
                owner->DeactivateSwitchbotSlot(slotIndex);
            }

            if (item->ChangeRareAttribute())
                switchItemRare->SetCount(switchItem->GetCount() - 1);
        }

        item->UpdatePacket();
        if (SwitchbotCheckItem(item)) {
            owner->OnSwitchbotDone(slotIndex);
            return 0;
        }
    }


    auto time = THECORE_SECS_TO_PASSES(0.11);
    if (owner && owner->FindAffect(AFFECT_PREMIUM_SWITCHBOT)) {
        time = THECORE_SECS_TO_PASSES(0.06);
    }
    return time;
}

void CItem::StartSelfSwitchEvent()
{
    if (m_selfSwitchEvent)
        return;

    item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
    info->item_vid = GetVID();

    Lock(true);

    auto time = THECORE_MSECS_TO_PASSES(0.3);
    if (m_pOwner && m_pOwner->FindAffect(AFFECT_PREMIUM_SWITCHBOT)) {
        time = THECORE_MSECS_TO_PASSES(0.15);
    }

    m_selfSwitchEvent = event_create(switch_event, info, time);
}

void CItem::StopSelfSwitchEvent()
{
    if (!m_selfSwitchEvent)
        return;

    Lock(false);
    event_cancel(&m_selfSwitchEvent);

    ITEM_MANAGER::instance().SaveSingleItem(this);
}

EVENTFUNC(real_time_expire_event)
{
    const item_vid_event_info* info =
        reinterpret_cast<const item_vid_event_info*>(event->info);
    if (nullptr == info)
        return 0;

    CItem* item = ITEM_MANAGER::instance().FindByVID(info->item_vid);
    if (nullptr == item)
        return 0;

    const time_t current = get_global_time();
    if (current > item->GetSocket(0)) {
        ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");

        return 0;
    }

    return THECORE_SECS_TO_PASSES(1);
}

void CItem::StartRealTimeExpireEvent()
{
    if (m_pkRealTimeExpireEvent)
        return;

    if (FindLimit(LIMIT_REAL_TIME) ||
        FindLimit(LIMIT_REAL_TIME_START_FIRST_USE)) {
        item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
        info->item_vid = GetVID();

        m_pkRealTimeExpireEvent = event_create(real_time_expire_event, info,
                                               THECORE_SECS_TO_PASSES(1));

        SPDLOG_INFO("REAL_TIME_EXPIRE: StartRealTimeExpireEvent");
    }
}

bool CItem::IsRealTimeItem()
{
    return FindLimit(LIMIT_REAL_TIME);
}

bool CItem::IsLimitTimeItem() const
{
    return FindLimit(LIMIT_REAL_TIME) ||
           FindLimit(LIMIT_REAL_TIME_START_FIRST_USE) ||
           FindLimit(LIMIT_TIMER_BASED_ON_WEAR);
}

/**
 * Check if the item's consuming timer has already began counting.
 * @return bool Whether it started or not.
 */
bool CItem::IsUsedTimeItem()
{
    return m_pkRealTimeExpireEvent != nullptr;
}

void CItem::StartUniqueExpireEvent()
{
    if (GetItemType() != ITEM_UNIQUE)
        return;

    if (m_pkUniqueExpireEvent)
        return;

    //기간제 아이템일 경우 시간제 아이템은 동작하지 않는다
    if (IsLimitTimeItem())
        return;

    // HARD CODING
    if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
        m_pOwner->ShowAlignment(false);

    SocketValue iSec = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);

    if (iSec == 0)
        iSec = 60;
    else
        iSec = std::min<int>(iSec, 60);

    SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

    item_event_info* info = AllocEventInfo<item_event_info>();
    info->item = this;

    SetUniqueExpireEvent(
        event_create(unique_expire_event, info, THECORE_SECS_TO_PASSES(iSec)));
}

// 시간 후불제
// timer_based_on_wear_expire_event 설명 참조
void CItem::StartTimerBasedOnWearExpireEvent()
{
    if (m_pkTimerBasedOnWearExpireEvent)
        return;

    if (IsRealTimeItem())
        return;

    if (!FindLimit(LIMIT_TIMER_BASED_ON_WEAR))
        return;

    SocketValue iSec = GetSocket(0);

    if (0 != iSec) {
        iSec %= 60;
        if (0 == iSec)
            iSec = 60;
    }

    item_event_info* info = AllocEventInfo<item_event_info>();
    info->item = this;

    SetTimerBasedOnWearExpireEvent(event_create(
        timer_based_on_wear_expire_event, info, THECORE_SECS_TO_PASSES(iSec)));
}

void CItem::StopUniqueExpireEvent()
{
    if (!m_pkUniqueExpireEvent)
        return;

    if (GetValue(2) !=
        0) // 게임시간제 이외의 아이템은 UniqueExpireEvent를 중단할 수 없다.
        return;

    // HARD CODING
    if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
        m_pOwner->ShowAlignment(true);

    SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME,
              event_time(m_pkUniqueExpireEvent) / THECORE_SECS_TO_PASSES(1));
    event_cancel(&m_pkUniqueExpireEvent);

    ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
    if (!m_pkTimerBasedOnWearExpireEvent)
        return;

    auto remain_time = GetSocket(ITEM_SOCKET_REMAIN_SEC) -
                       event_processing_time(m_pkTimerBasedOnWearExpireEvent) /
                           THECORE_SECS_TO_PASSES(1);

    SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
    event_cancel(&m_pkTimerBasedOnWearExpireEvent);

    ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int iAddonType)
{
    CItemAddonManager::instance().ApplyAddonTo(iAddonType, this);
}

int CItem::GetSpecialGroup() const
{
    return ITEM_MANAGER::instance().GetSpecialGroupFromItem(GetVnum());
}

//
// 악세서리 소켓 처리.
//
bool CItem::IsAccessoryForSocket()
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return false;
    return (proto->bType == ITEM_ARMOR &&
            (proto->bSubType == ARMOR_WRIST || proto->bSubType == ARMOR_NECK ||
             proto->bSubType == ARMOR_EAR)) ||
           (proto->bType == ITEM_BELT);
    // 2013년 2월 새로 추가된 '벨트' 아이템의 경우 기획팀에서 악세서리 소켓
    // 시스템을 그대로 이용하자고 함.
}

void CItem::SetAccessorySocketGrade(int iGrade)
{
    if (iGrade < GetAccessorySocketMinGrade())
        SetSocket(0, std::clamp(GetAccessorySocketMinGrade(), 0,
                                GetAccessorySocketMaxGrade()));
    else
        SetSocket(0, std::clamp(iGrade, 0, GetAccessorySocketMaxGrade()));

    int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

    // if (gConfig.testServer)
    //	iDownTime /= 60;

    SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int iMaxGrade)
{
    SetSocket(1, std::clamp<int>(iMaxGrade, 0, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketMinGrade(int iMinGrade)
{
    SetSocket(3, std::clamp<int>(iMinGrade, 0, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketDownGradeTime(uint32_t time)
{
    SetSocket(2, time);

    if (gConfig.testServer && GetOwner())
        SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO,
                           "%s에서 소켓 빠질때까지 남은 시간 %d",
                           TextTag::itemname(GetVnum()).c_str(), time);
}

void degrade(CItem* item)
{
    if (!item)
        return;

    item->SetAccessorySocketExpireEvent(nullptr);
    item->AccessorySocketDegrade();
};

EVENTFUNC(accessory_socket_expire_event)
{
    item_vid_event_info* info = static_cast<item_vid_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("accessory_socket_expire_event> <Factor> Null pointer");
        return 0;
    }

    CItem* item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

    if (item->GetAccessorySocketDownGradeTime() <= 1)
        return 0;

    CHARACTER* owner = item->GetOwner();
    auto now = get_dword_time();
    // Should expire if: Player was attacked in the last 60s, player attacked in
    // the last 60s.
    bool shouldExpire = !owner || now - owner->GetLastAttackTime() < 60000 ||
                        now - owner->GetLastHitReceivedTime() < 60000;

    int iTime = item->GetAccessorySocketDownGradeTime() - 60;

    if (iTime <= 1) {
        degrade(item);
        return 0;
    }

    if (shouldExpire)
        item->SetAccessorySocketDownGradeTime(iTime);

    if (iTime > 60)
        return THECORE_SECS_TO_PASSES(60);

    return THECORE_SECS_TO_PASSES(iTime);
}

void CItem::StartAccessorySocketExpireEvent()
{
    if (!IsAccessoryForSocket())
        return;

    if (m_pkAccessorySocketExpireEvent)
        return;

    if (GetAccessorySocketMaxGrade() == 0)
        return;

    if (GetAccessorySocketGrade() == 0)
        return;

    int iSec = GetAccessorySocketDownGradeTime();
    SetAccessorySocketExpireEvent(nullptr);

    if (iSec <= 1)
        iSec = 5;
    else
        iSec = std::min<int>(iSec, 60);

    item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
    info->item_vid = GetVID();

    SetAccessorySocketExpireEvent(event_create(
        accessory_socket_expire_event, info, THECORE_SECS_TO_PASSES(iSec)));
}

void CItem::StopAccessorySocketExpireEvent()
{
    if (!m_pkAccessorySocketExpireEvent)
        return;

    if (!IsAccessoryForSocket())
        return;

    int new_time = GetAccessorySocketDownGradeTime() -
                   (60 - event_time(m_pkAccessorySocketExpireEvent) /
                             THECORE_SECS_TO_PASSES(1));

    event_cancel(&m_pkAccessorySocketExpireEvent);

    if (new_time <= 1) {
        AccessorySocketDegrade();
    } else {
        SetAccessorySocketDownGradeTime(new_time);
    }
}

bool CItem::IsRideItem()
{
    return ITEM_TOGGLE == GetItemType() && TOGGLE_MOUNT == GetSubType();
}

void CItem::SetMaskVnum(uint32_t vnum)
{
    m_dwMaskVnum = vnum;
}

uint32_t CItem::GetMaskVnum() const
{
    return m_dwMaskVnum;
}

bool CItem::IsMaskedItem() const
{
    return m_dwMaskVnum != 0;
}

long CItem::GetSealDate() const
{
    return m_sealDate;
}

void CItem::SetSealDate(long sealDate)
{
    m_sealDate = sealDate;
}

bool CItem::IsSealed() const
{
    return m_sealDate == -1 || m_sealDate > time(nullptr);
}

bool CItem::IsUnlimitedSealDate() const
{
    return m_sealDate == -1;
}

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
    m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
    if (GetAccessorySocketGrade() > 0) {
        CHARACTER* ch = GetOwner();

        if (ch &&
            !(GetAccessorySocketGrade() - 1 < GetAccessorySocketMinGrade())) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "%s에 박혀있던 보석이 사라집니다.",
                               TextTag::itemname(GetVnum()).c_str());
        }

        ModifyPoints(false);
        SetAccessorySocketGrade(GetAccessorySocketGrade() - 1);
        ModifyPoints(true);

        int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

        if (gConfig.testServer)
            iDownTime /= 60;

        SetAccessorySocketDownGradeTime(iDownTime);

        if (iDownTime)
            StartAccessorySocketExpireEvent();
    }
}

// ring에 item을 박을 수 있는지 여부를 체크해서 리턴
static bool CanPutIntoRing(CItem* ring, CItem* item)
{
    // const uint32_t vnum = item->GetVnum();
    return false;
}

static bool CanPutIntoBelt(CItem* belt, CItem* item)
{
    if (item->GetSubType() != USE_PUT_INTO_BELT_SOCKET)
        return false;

    if (item->GetValue(0) != belt->GetValue(0))
        return false;

    if (belt->GetValue(1) == 1)
        return false;

    return true;
}

bool CItem::CanPutInto(CItem* item)
{
    if (item->GetItemType() == ITEM_BELT)
        return CanPutIntoBelt(item, this);

    else if (item->GetItemType() == ITEM_RING)
        return CanPutIntoRing(item, this);

    else if (item->GetItemType() != ITEM_ARMOR)
        return false;

    uint32_t vnum = item->GetVnum();

    struct JewelAccessoryInfo {
        uint32_t jewel;
        uint32_t wrist;
        uint32_t neck;
        uint32_t ear;
    };
    const static JewelAccessoryInfo infos[] = {
        {50639, 9750, 9740, 9760},
        {50634, 14420, 16220, 17220},
        {50640, 14160, 16160, 17160},
    };

    uint32_t item_type = (item->GetVnum() / 10) * 10;
    for (auto info : infos) {
        switch (item->GetSubType()) {
            case ARMOR_WRIST:
                if (info.wrist == item_type) {
                    return info.jewel == GetVnum();
                }
                break;
            case ARMOR_NECK:
                if (info.neck == item_type) {
                    return info.jewel == GetVnum();
                }
                break;
            case ARMOR_EAR:
                if (info.ear == item_type) {
                    return info.jewel == GetVnum();
                }
                break;
            default:
                break;
        }
    }
    if (item->GetSubType() == ARMOR_WRIST)
        vnum -= 14000;
    else if (item->GetSubType() == ARMOR_NECK)
        vnum -= 16000;
    else if (item->GetSubType() == ARMOR_EAR)
        vnum -= 17000;
    else
        return false;

    uint32_t type = vnum / 20;

    if (type > 11) {
        type = (vnum - 170) / 20;

        if (50623 + type != GetVnum())
            return false;
        else
            return true;
    } else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219) {
        if (50625 != GetVnum())
            return false;
        else
            return true;
    } else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239) {
        if (50626 != GetVnum())
            return false;
        else
            return true;
    }

    return 50623 + type == GetVnum();
}

bool CItem::CheckItemUseLevel(int nLevel)
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return false;

    for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i) {
        if (proto->aLimits[i].bType == LIMIT_LEVEL) {
            if (proto->aLimits[i].value > nLevel)
                return false;
            else
                return true;
        }
    }
    return true;
}

long CItem::FindApplyValue(uint8_t bApplyType)
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return 0;

    for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i) {
        if (proto->aApplies[i].bType == bApplyType)
            return proto->aApplies[i].lValue;
    }

    return 0;
}

bool CItem::IsStackable() const
{
    return IS_SET(GetFlag(), ITEM_FLAG_STACKABLE) &&
           !IS_SET(GetAntiFlag(), ITEM_ANTIFLAG_STACK);
}

void CItem::CopySocketTo(CItem* pItem)
{
    for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
        pItem->m_alSockets[i] = m_alSockets[i];
    pItem->Save();
}

int CItem::GetAccessorySocketGrade()
{
    return std::clamp<int>(GetSocket(0), 0, GetAccessorySocketMaxGrade());
}

int CItem::GetAccessorySocketMaxGrade()
{
    return std::clamp<int>(GetSocket(1), 0, ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int CItem::GetAccessorySocketMinGrade()
{
    return std::clamp<int>(GetSocket(3), 0, ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int CItem::GetAccessorySocketDownGradeTime()
{
    return std::clamp<int>(
        GetSocket(2), 0,
        aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

int CItem::GetLevelLimit()
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return 0;
    for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i) {
        if (proto->aLimits[i].bType == LIMIT_LEVEL) {
            return proto->aLimits[i].value;
        }
    }
    return 0;
}

bool CItem::IsDragonSoul() const
{
    return GetItemType() == ITEM_DS;
}

int CItem::GiveMoreTime_Per(float fPercent)
{
    if (IsDragonSoul()) {
        auto duration = DSManager::instance().GetDuration(this);
        int remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
        int given_time = (int)(fPercent * duration / 100);
        if (remain_sec == duration)
            return false;
        if ((given_time + remain_sec) >= duration) {
            SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
            return duration - remain_sec;
        } else {
            SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
            return given_time;
        }
    }
    // 우선 용혼석에 관해서만 하도록 한다.
    else
        return 0;
}

int CItem::GiveMoreTime_Fix(uint32_t dwTime)
{
    if (IsDragonSoul()) {
        const uint32_t duration = DSManager::instance().GetDuration(this);
        const uint32_t remainSec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
        if (remainSec == duration)
            return false;
        if ((dwTime + remainSec) >= duration) {
            SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
            return duration - remainSec;
        } else {
            SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remainSec);
            return dwTime;
        }
    }
    // 우선 용혼석에 관해서만 하도록 한다.
    else
        return 0;
}

int CItem::GetDuration() const
{
    auto limit = FindLimit(LIMIT_REAL_TIME_START_FIRST_USE);
    if (limit)
        return limit->value;

    limit = FindLimit(LIMIT_REAL_TIME);
    if (limit)
        return limit->value;

    limit = FindLimit(LIMIT_TIMER_BASED_ON_WEAR);
    if (limit)
        return limit->value;

    return -1;
}

bool CItem::IsSameSpecialGroup(const CItem* item) const
{
    // 서로 VNUM이 같다면 같은 그룹인 것으로 간주
    if (this->GetVnum() == item->GetVnum())
        return true;

    if (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()))
        return true;

    return false;
}

// 28/03/14 - Think - Fix immune bug
uint32_t CItem::GetRealImmuneFlag()
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return 0;

    uint32_t dwImmuneFlag = proto->dwImmuneFlag;
    for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i) {
        if (GetAttributeType(i)) {
            const TPlayerItemAttribute& ia = GetAttribute(i);

            if (ia.bType == APPLY_IMMUNE_STUN &&
                !IS_SET(dwImmuneFlag, IMMUNE_STUN))
                SET_BIT(dwImmuneFlag, IMMUNE_STUN);
            else if (ia.bType == APPLY_IMMUNE_FALL &&
                     !IS_SET(dwImmuneFlag, IMMUNE_FALL))
                SET_BIT(dwImmuneFlag, IMMUNE_FALL);
            else if (ia.bType == APPLY_IMMUNE_SLOW &&
                     !IS_SET(dwImmuneFlag, IMMUNE_SLOW))
                SET_BIT(dwImmuneFlag, IMMUNE_SLOW);
        }
    }

    return dwImmuneFlag;
}

// End of fix immune bug

void CItem::SetSIGVnum(uint32_t dwSIG)
{
    m_dwSIGVnum = dwSIG;
}

uint32_t CItem::GetSIGVnum() const
{
    return m_dwSIGVnum;
}

bool CItem::IsGMOwner() const
{
    return m_bIsGMOwner == GM_OWNER_GM;
}

void CItem::SetGMOwner(bool bGMOwner)
{
    m_bIsGMOwner = bGMOwner ? GM_OWNER_GM : GM_OWNER_PLAYER;
}

bool CItem::IsBlocked() const
{
    return m_bIsBlocked;
}

void CItem::SetBlocked(bool bBlocked)
{
    m_bIsBlocked = bBlocked;
    Save();
}

SocketValue CItem::GetPrivateShopPrice() const
{
    return m_privateShopPrice;
}

void CItem::SetPrivateShopPrice(SocketValue val)
{
    m_privateShopPrice = val;
}

std::string CItem::GetHyperlink()
{
    std::string attrString;

    auto hasAttribute = false;

    for (int i = 0; i < GetNormAttrNum(); ++i) {
        if (GetAttributeType(i) != 0) {
            attrString += fmt::sprintf(":%x:%d", GetAttributeType(i),
                                       GetAttributeValue(i));
            hasAttribute = true;
        }
    }

    auto hyperLink = fmt::sprintf(
        "|%s|Hitem:%x:%x:%lld:%lld:%lld:%lld:%lld:%lld%s|h[%s]|h|r",
        hasAttribute ? "cffffc700" : "cfff1e6c0", GetVnum(), GetFlag(),
        GetSocket(0), GetSocket(1), GetSocket(2), GetSocket(3), GetSocket(4),
        GetSocket(5), attrString.c_str(), GetName());

    return hyperLink;
}

bool CItem::IsSealAble()
{
    if (GetItemType() == ITEM_WEAPON)
        return GetSubType() != WEAPON_ARROW && GetSubType() != WEAPON_QUIVER;

    return GetItemType() == ITEM_ARMOR || GetItemType() == ITEM_BELT;
}

const TItemApply* CItem::FindApply(uint8_t type) const
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return nullptr;

    for (const auto& apply : proto->aApplies) {
        if (apply.bType == type)
            return &apply;
    }

    return nullptr;
}

const TItemLimit* CItem::FindLimit(uint8_t type) const
{
    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return nullptr;

    for (const auto& limit : proto->aLimits) {
        if (limit.bType == type)
            return &limit;
    }

    return nullptr;
}

void CItem::OnCreate()
{
    if (GetItemType() == ITEM_GACHA)
        SetSocket(0, GetLimitValue(1));

    if (GetItemType() == ITEM_UNIQUE) {
        // Unique items record the time remaining on the socket at creation
        // time.
        if (GetValue(ITEM_VALUE_UNIQUE_IS_REMAIN_MIN) == 0) {
            SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME,
                      GetValue(ITEM_VALUE_UNIQUE_EXPIRE)); // Game Time Unique
        } else {
            SetSocket(
                ITEM_SOCKET_UNIQUE_REMAIN_TIME,
                get_global_time() +
                    GetValue(ITEM_VALUE_UNIQUE_EXPIRE)); // Real-time unique

            StartUniqueExpireEvent();
        }
    }

    /*if (ITEM_COSTUME == GetItemType() && COSTUME_ACCE == GetSubType()) {
        if (GetRefinedVnum() == 0)
            SetSocket(0, Random::get(GetApply(0) - 10, GetApply(0)));
        else
            SetSocket(0, GetApply(0));

        SetSocket(1, 0);
    }*/

    auto limit = FindLimit(LIMIT_REAL_TIME);
    if (limit) {
        if (limit->value)
            SetSocket(ITEM_SOCKET_REALTIME_EXPIRE,
                      time(nullptr) + limit->value);
        else
            SetSocket(ITEM_SOCKET_REALTIME_EXPIRE,
                      time(nullptr) + 60 * 60 * 24 * 7);

        StartRealTimeExpireEvent();
    }

    limit = FindLimit(LIMIT_TIMER_BASED_ON_WEAR);
    if (limit) {
        // Start the timer if the item is already worn, and set the available
        // time for the new item. ( If you pay by item mall, Socket0 value must
        // be set before entering this logic.
        if (IsEquipped()) {
            if (IsDragonSoul()) {
                DSManager::instance().ActivateDragonSoul(this);
#ifdef ENABLE_DS_SET
                GetOwner()->DragonSoul_HandleSetBonus();
#endif
            } else {
                StartTimerBasedOnWearExpireEvent();
            }
        } else {
            int32_t duration = GetSocket(ITEM_SOCKET_WEAR_REMAIN_SEC);
            if (0 == duration)
                duration = limit->value;

            if (0 == duration)
                duration =
                    60 * 60 *
                    10; // If there is no information, it defaults to 10 hours.

            SetSocket(ITEM_SOCKET_WEAR_REMAIN_SEC, duration);
        }
    }

    if (GetItemType() == ITEM_WEAPON && GetSubType() == WEAPON_QUIVER) {
        SetSocket(2, GetValue(2), false);
    }
}

void CItem::OnLoad()
{
    // If you have used an item at least once, then the time is subtracted even
    // if you are not using it
    const auto limit = FindLimit(LIMIT_REAL_TIME_START_FIRST_USE);
    if (limit) {
        // The number of times the item is used is recorded in Socket1, so once
        // used items start the timer.
        if (0 != GetSocket(ITEM_SOCKET_REALTIME_USE_COUNT)) {
            StartRealTimeExpireEvent();
        } else if (IsEquipped()) {
            int32_t duration = limit->value;

            if (0 == duration)
                duration = 60 * 60 * 24 * 7;

            SetSocket(ITEM_SOCKET_REALTIME_EXPIRE, time(nullptr) + duration);
            StartRealTimeExpireEvent();
        }
    }

    if (FindLimit(LIMIT_REAL_TIME))
        StartRealTimeExpireEvent();

    auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
    if (!proto)
        return;
    // Attributes should be attached with a 100% probability, but if not, attach
    // them. ...............
    if (100 == proto->bAlterToMagicItemPct && 0 == GetAttributeCount())
        AlterToMagicItem();
}

void CItem::OnUse()
{
    // Handle the way the time is deducted without using it since the first use
    // of the item.
    const auto limit = FindLimit(LIMIT_REAL_TIME_START_FIRST_USE);
    if (limit) {
        // Whether or not the item is used once is determined by Socket1.
        // (Record usage count on Socket1)
        const auto useCount = GetSocket(ITEM_SOCKET_REALTIME_USE_COUNT);
        if (0 == useCount) {
            // Limit value is used as the default value, but Socket0 should use
            // the value. (In seconds)
            int32_t duration = GetSocket(ITEM_SOCKET_REALTIME_EXPIRE);

            if (0 == duration)
                duration = limit->value;

            if (0 == duration)
                duration = 60 * 60 * 24 * 7;

            SetSocket(ITEM_SOCKET_REALTIME_EXPIRE, time(nullptr) + duration);
            StartRealTimeExpireEvent();
        }

        SetSocket(ITEM_SOCKET_REALTIME_USE_COUNT, useCount + 1);
    }
}

void CItem::OnEquip()
{
    if (IsDragonSoul()) {
        OnUse();
        return; // DSManager::instance().ActivateDragonSoul(this);
    }

    StartUniqueExpireEvent();

    if (FindLimit(LIMIT_TIMER_BASED_ON_WEAR))
        StartTimerBasedOnWearExpireEvent();

    // ACCESSORY_REFINE
    StartAccessorySocketExpireEvent();
    // END_OF_ACCESSORY_REFINE

    // Equipping an item means using it.
    OnUse();
}

void CItem::OnUnequip()
{
    StopUniqueExpireEvent();

    if (FindLimit(LIMIT_TIMER_BASED_ON_WEAR))
        StopTimerBasedOnWearExpireEvent();

    // ACCESSORY_REFINE
    StopAccessorySocketExpireEvent();
    // END_OF_ACCESSORY_REFINE
}

void CItem::SetLastOwnerPid(uint32_t lastOwnerPID)
{
    m_dwLastOwnerPID = lastOwnerPID;
}

void CItem::UpdateToggleUsePulse()
{
    m_nextTogglePulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(4));
}

bool CItem::IsNextToggleUsePulse() const
{
    return (m_nextTogglePulse == 0 || (m_nextTogglePulse < thecore_pulse()));
}
