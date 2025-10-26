#include "StdAfx.h"
#include "PythonPlayerEventHandler.h"
#include "PythonApplication.h"
#include "PythonItem.h"
#include "../EterPythonLib/PythonSlotWindow.h"
#include "../EterBase/Timer.h"

#include <fmt/printf.h>
#include <optional>
#include <utility>

const uint32_t POINT_MAGIC_NUMBER = 0xe73ac1da;

void CPythonPlayer::SPlayerStatus::SetPoint(UINT ePoint, PointValue lPoint)
{
    m_alPoint[ePoint] = lPoint;
}

void CPythonPlayer::SPlayerStatus::SetGold(Gold gold)
{
    m_ullGold = gold;
}

Gold CPythonPlayer::SPlayerStatus::GetGold()
{
    return m_ullGold;
}

#ifdef ENABLE_GEM_SYSTEM
void CPythonPlayer::SPlayerStatus::SetGem(int gem)
{
	m_gem = gem;
}

int CPythonPlayer::SPlayerStatus::GetGem()
{
	return m_gem;
}
#endif

PointValue CPythonPlayer::SPlayerStatus::GetPoint(UINT ePoint) const
{
    return m_alPoint[ePoint];
}

CPythonPlayer::Windows::Windows()
    : inventory(INVENTORY_PAGE_HEIGHT, INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_COUNT)
      , equipment(1, WEAR_MAX_NUM)
      , switchbot(1, SWITCHBOT_SLOT_COUNT)
      , belt(BELT_INVENTORY_WIDTH, BELT_INVENTORY_HEIGHT)
{
    // ctor
}

void CPythonPlayer::Windows::Clear()
{
    inventory.Clear();
    equipment.Clear();
    belt.Clear();
}

const PagedGrid<ClientItemData> *CPythonPlayer::Windows::Get(uint8_t type) const
{
    switch (type)
    {
    case INVENTORY:
        return &inventory;
    case EQUIPMENT:
        return &equipment;
    case SWITCHBOT:
        return &switchbot;
    case BELT_INVENTORY:
        return &belt;
    }

    return nullptr;
}

PagedGrid<ClientItemData> *CPythonPlayer::Windows::Get(uint8_t type)
{
    switch (type)
    {
    case INVENTORY:
        return &inventory;
    case EQUIPMENT:
        return &equipment;
    case SWITCHBOT:
        return &switchbot;
    case BELT_INVENTORY:
        return &belt;
    }

    return nullptr;
}

void CPythonPlayer::SetSwitchbotSlotData(SwitchBotSlotData d)
{
    // TODO: Check switch bot slot count
    m_switchbotSlotData[d.slot] = d;

    PyCallClassMemberFunc(m_ppyGameWindow, "RefreshSwitchbot");
}

void CPythonPlayer::SetSwitchbotSlotAttribute(uint32_t slot, uint32_t alternative, uint32_t attrIndex, TItemApply apply)
{
    // TODO: Check switch bot slot count

    if (!m_switchbotSlotData[slot].has_value())
    {
        m_switchbotSlotData[slot] = {};
    }

    m_switchbotSlotData[slot]->attr[alternative][attrIndex] = apply;

    PyCallClassMemberFunc(m_ppyGameWindow, "RefreshSwitchbot");
}

void CPythonPlayer::SetSwitchbotSlotStatus(uint32_t slot, uint8_t status)
{
    // TODO: Check switch bot slot count

    if (!m_switchbotSlotData[slot].has_value())
    {
        m_switchbotSlotData[slot] = {};
    }

    m_switchbotSlotData[slot]->status = status;

    PyCallClassMemberFunc(m_ppyGameWindow, "RefreshSwitchbot");
}

std::optional<SwitchBotSlotData> CPythonPlayer::GetSwitchbotSlotData(uint32_t slot)
{
    return m_switchbotSlotData[slot];
}


std::optional<CInstanceBase *> CPythonPlayer::__GetPickedActorPtrNew()
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstPicked = rkChrMgr.OLD_GetPickedInstancePtr();
    if (!pkInstPicked)
        return std::nullopt;

    return pkInstPicked;
}



bool CPythonPlayer::__GetPickedActorPtr(CInstanceBase **ppkInstPicked)
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstPicked = rkChrMgr.OLD_GetPickedInstancePtr();
    if (!pkInstPicked)
        return false;

    *ppkInstPicked = pkInstPicked;
    return true;
}

bool CPythonPlayer::__GetPickedActorID(uint32_t *pdwActorID)
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    return rkChrMgr.OLD_GetPickedInstanceVID(pdwActorID);
}

bool CPythonPlayer::__GetPickedItemID(uint32_t *pdwItemID)
{
    CPythonItem &rkItemMgr = CPythonItem::Instance();
    return rkItemMgr.GetPickedItemID(pdwItemID);
}

bool CPythonPlayer::__GetPickedGroundPos(TPixelPosition *pkPPosPicked)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();

    TPixelPosition kPPosPicked;
    if (rkBG.GetPickingPoint(pkPPosPicked))
    {
        pkPPosPicked->y = -pkPPosPicked->y;
        return true;
    }

    return false;
}

void CPythonPlayer::NEW_GetMainActorPosition(TPixelPosition *pkPPosActor) const
{
    if (auto pInstance = NEW_GetMainActorPtr(); pInstance)
    {
        pInstance->NEW_GetPixelPosition(pkPPosActor);
    }
    else
    {
        CPythonApplication::AppInst().GetCenterPosition(pkPPosActor);
    }
}

bool CPythonPlayer::RegisterEffect(uint32_t dwEID, const char *c_szFileName, bool isCache)
{
    if (dwEID >= EFFECT_NUM)
        return false;

    CEffectManager &rkEftMgr = CEffectManager::Instance();
    return rkEftMgr.RegisterEffect(c_szFileName, &m_adwEffect[dwEID], isCache);
}

void CPythonPlayer::NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst)
{
    if (dwEID >= EFFECT_NUM)
        return;

    auto &mgr = CEffectManager::Instance();
    auto index = mgr.CreateEffect(m_adwEffect[dwEID]);

    Matrix matWorld = Matrix::CreateTranslation(kPPosDst.x, -kPPosDst.y, kPPosDst.z);

    auto* effect = mgr.GetEffectInstance(index);
    if (effect)
        effect->SetGlobalMatrix(matWorld);
}

CInstanceBase *CPythonPlayer::NEW_FindActorPtr(uint32_t dwVID) const
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    return rkChrMgr.GetInstancePtr(dwVID);
}

CInstanceBase *CPythonPlayer::NEW_GetMainActorPtr() const
{
    return NEW_FindActorPtr(m_dwMainCharacterIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayer::Update()
{
    NEW_RefreshMouseWalkingDirection();
    CPythonPlayerEventHandler &rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
    rkPlayerEventHandler.FlushVictimList();

    if (m_isDestPosition)
    {
        CInstanceBase *pInstance = NEW_GetMainActorPtr();
        if (pInstance)
        {
            TPixelPosition PixelPosition;
            pInstance->NEW_GetPixelPosition(&PixelPosition);

            if (abs(int(PixelPosition.x) - m_ixDestPos) + abs(int(PixelPosition.y) - m_iyDestPos) < 10000)
            {
                m_isDestPosition = FALSE;
            }
            else
            {
                if (DX::StepTimer::instance().GetTotalMillieSeconds() - m_iLastAlarmTime > 20000)
                {
                    AlarmHaveToGo();
                }
            }
        }
    }

    __Update_AutoAttack();
    __Update_NotifyGuildAreaEvent();
}

bool CPythonPlayer::__IsUsingChargeSkill()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    if (__CheckDashAffect(*pkInstMain))
        return true;

    if (MODE_USE_SKILL != m_eReservedMode)
        return false;

    TSkillInstance &rkSkillInst = m_Skill[m_dwSkillSlotIndexReserved];

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
        return false;

    return pSkillData->IsChargeSkill();
}

void CPythonPlayer::__Update_AutoAttack()
{
    if (0 == m_dwAutoAttackTargetVID)
        return;

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    // źȯ�� ���� �޷����� ���߿��� ��ŵ
    if (__IsUsingChargeSkill())
        return;

    CInstanceBase *pkInstVictim = NEW_FindActorPtr(m_dwAutoAttackTargetVID);
    if (!pkInstVictim)
    {
        __ClearAutoAttackTargetActorID();
    }
    else
    {
        if (!pkInstMain->IsTargetableInstance(*pkInstVictim))
        {
            __ClearAutoAttackTargetActorID();
        }
        else if (pkInstMain->IsMountingHorse() && !pkInstMain->CanAttackHorseLevel())
        {
            __ClearAutoAttackTargetActorID();
        }
        else if (pkInstMain->IsAttackableInstance(*pkInstVictim))
        {
            __ReserveClickActor(m_dwAutoAttackTargetVID);
        }
    }
}

void CPythonPlayer::__Update_NotifyGuildAreaEvent()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (pkInstMain)
    {
        TPixelPosition kPixelPosition;
        pkInstMain->NEW_GetPixelPosition(&kPixelPosition);

        uint32_t dwAreaID = CPythonMiniMap::Instance().GetGuildAreaID(
            ULONG(kPixelPosition.x), ULONG(kPixelPosition.y));

        if (dwAreaID != m_inGuildAreaID)
        {
            if (0xffffffff != dwAreaID)
            {
                m_ppyGameWindow.attr("BINARY_Guild_EnterGuildArea")(dwAreaID);
            }
            else
            {
                m_ppyGameWindow.attr("BINARY_Guild_ExitGuildArea")(dwAreaID);
            }

            m_inGuildAreaID = dwAreaID;
        }
    }
}

void CPythonPlayer::SetMainCharacterIndex(int iIndex)
{
    m_dwMainCharacterIndex = iIndex;

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (pkInstMain)
    {
        CPythonPlayerEventHandler &rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
        pkInstMain->SetEventHandler(&rkPlayerEventHandler);
    }
}

uint32_t CPythonPlayer::GetMainCharacterIndex()
{
    return m_dwMainCharacterIndex;
}

bool CPythonPlayer::IsMainCharacterIndex(uint32_t dwIndex)
{
    return (m_dwMainCharacterIndex == dwIndex);
}

uint32_t CPythonPlayer::GetGuildID()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return 0xffffffff;

    return pkInstMain->GetGuildID();
}

void CPythonPlayer::SetRace(uint32_t dwRace)
{
    m_dwRace = dwRace;
}

uint32_t CPythonPlayer::GetRace()
{
    return m_dwRace;
}

uint32_t CPythonPlayer::__GetRaceStat()
{
    switch (GetRace())
    {
    case MAIN_RACE_WARRIOR_M:
    case MAIN_RACE_WARRIOR_W:
    case MAIN_RACE_WOLFMAN_M:
    case MAIN_RACE_SURA_M:
    case MAIN_RACE_SURA_W:
        return GetStatus(POINT_ST);
    case MAIN_RACE_ASSASSIN_M:
    case MAIN_RACE_ASSASSIN_W:
        return GetStatus(POINT_DX);
    case MAIN_RACE_SHAMAN_M:
    case MAIN_RACE_SHAMAN_W:
        return GetStatus(POINT_IQ);
    }
    return GetStatus(POINT_ST);
}

uint32_t CPythonPlayer::__GetLevelAtk()
{
    return 2 * GetStatus(POINT_LEVEL);
}

uint32_t CPythonPlayer::__GetStatAtk()
{
    return (4 * GetStatus(POINT_ST) + 2 * __GetRaceStat()) / 3;
}

uint32_t CPythonPlayer::__GetWeaponAtk(uint32_t dwWeaponPower)
{
    return 2 * dwWeaponPower;
}

uint32_t CPythonPlayer::__GetTotalAtk(uint32_t dwWeaponPower, uint32_t dwRefineBonus)
{
    uint32_t dwLvAtk = __GetLevelAtk();
    uint32_t dwStAtk = __GetStatAtk();

    /////

    uint32_t dwWepAtk;
    uint32_t dwTotalAtk;

    int hr = __GetHitRate();
    dwWepAtk = __GetWeaponAtk(dwWeaponPower + dwRefineBonus);
    dwTotalAtk = dwLvAtk + (dwStAtk + dwWepAtk) * hr / 100;

    return dwTotalAtk;
}

uint32_t CPythonPlayer::__GetHitRate()
{
    int src = (GetStatus(POINT_DX) * 4 + GetStatus(POINT_LEVEL) * 2) / 6;

    return 100;
}

uint32_t CPythonPlayer::__GetEvadeRate()
{
    return 30 * (2 * GetStatus(POINT_DX) + 5) / (GetStatus(POINT_DX) + 95);
}

int32_t CalcAttackRating(const CPythonPlayer::SPlayerStatus &status)
{
    const auto attackerDx = status.GetPoint(POINT_DX);
    const auto attackerLv = status.GetPoint(POINT_LEVEL);
    const auto arSrc = std::min<int32_t>(90, (attackerDx * 4 + attackerLv * 2) / 6);

    return 1.0f;
}

PointValue CalcMeleeDamage(const CPythonPlayer::SPlayerStatus &status, PointValue dam, PointValue damBonus)
{
    PointValue attack = 0;

    // level must be ignored when multiply by fAR, so subtract it before calculation.
    attack = status.GetPoint(POINT_ATT_GRADE) + dam - status.GetPoint(POINT_LEVEL) * 2;

    attack = attack * CalcAttackRating(status) / 100;
    attack += status.GetPoint(POINT_LEVEL) * 2; // and add again

    attack += damBonus * 2.0;

    attack += status.GetPoint(POINT_PARTY_ATTACKER_BONUS);
    return attack;
}

void CPythonPlayer::UpdateBattleStatus()
{
    DamageValue damBonus = 0;
    DamageValue meleeMin = 0, meleeMax = 0, magicMin = 0, magicMax = 0;

    const auto* weapon = GetItemData({EQUIPMENT, WEAR_WEAPON});
    if (weapon)
    {
        const auto* proto = CItemManager::Instance().GetProto(weapon->vnum);
        if (proto && proto->GetType() == ITEM_WEAPON)
        {
            magicMin = proto->GetValue(1);
            magicMax = proto->GetValue(2);
            meleeMin = proto->GetValue(3);
            meleeMax = proto->GetValue(4);
            damBonus += proto->GetValue(5);
        }
    }

    m_playerStatus.SetPoint(POINT_HIT_RATE, CalcAttackRating(m_playerStatus));
    m_playerStatus.SetPoint(POINT_MIN_WEP, meleeMin + damBonus);
    m_playerStatus.SetPoint(POINT_MAX_WEP, meleeMax + damBonus);
    m_playerStatus.SetPoint(POINT_MIN_MAGIC_WEP, magicMin + damBonus);
    m_playerStatus.SetPoint(POINT_MAX_MAGIC_WEP, magicMax + damBonus);
    m_playerStatus.SetPoint(POINT_WEAPON_MIN, CalcMeleeDamage(m_playerStatus, meleeMin, damBonus));
    m_playerStatus.SetPoint(POINT_WEAPON_MAX, CalcMeleeDamage(m_playerStatus, meleeMax, damBonus));

    PyCallClassMemberFunc(m_ppyGameWindow, "RefreshStatus");

}

void CPythonPlayer::SetStatus(uint32_t dwType, PointValue lValue)
{
    if (dwType >= POINT_MAX_NUM)
    {
        assert(!" CPythonPlayer::SetStatus - Strange Status Type!");
        SPDLOG_DEBUG("CPythonPlayer::SetStatus - Set Status Type Error\n");
        return;
    }

    if (dwType == POINT_LEVEL)
    {
        CInstanceBase *pkPlayer = NEW_GetMainActorPtr();

        if (pkPlayer && pkPlayer->GetLevel() != lValue)
            pkPlayer->UpdateTextTailLevel(lValue);
    }

    m_playerStatus.SetPoint(dwType, lValue);

    switch (dwType)
    {
    case POINT_LEVEL:
    case POINT_ST:
    case POINT_DX:
    case POINT_IQ:
    case POINT_ATT_GRADE:
    case POINT_PARTY_ATTACKER_BONUS:
        UpdateBattleStatus();
        break;
    }
}

PointValue CPythonPlayer::GetStatus(uint32_t dwType) const
{
    if (dwType >= POINT_MAX_NUM)
    {
        assert(!" CPythonPlayer::GetStatus - Strange Status Type!");
        SPDLOG_DEBUG("CPythonPlayer::GetStatus - Get Status Type Error\n");
        return 0;
    }

    auto value = m_playerStatus.GetPoint(dwType);
    if (POINT_ATT_SPEED == dwType)
    {
        CInstanceBase *pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
        if (pInstance && (WEAPON_TWO_HANDED == pInstance->GetWeaponType()))
        {
            value -= 10;
        }
    }

    return value;
}

void CPythonPlayer::SetTitlePreview(std::string title, uint32_t color)
{
    CInstanceBase *pkPlayer = NEW_GetMainActorPtr();

    if (pkPlayer)
        pkPlayer->SetTitlePreview(title, color);
}

void CPythonPlayer::ChangeBlockMode(uint32_t flagToXor)
{
    m_playerStatus.blockMode ^= flagToXor;
    gPythonNetworkStream->SendBlockModePacket(m_playerStatus.blockMode);
}

const char *CPythonPlayer::GetName() const
{
    return m_stName.c_str();
}

void CPythonPlayer::SetName(const char *name)
{
    m_stName = name;
}

void CPythonPlayer::NotifyDeletingCharacterInstance(uint32_t dwVID)
{
    if (m_dwMainCharacterIndex == dwVID)
        m_dwMainCharacterIndex = 0;
}

void CPythonPlayer::NotifyCharacterDead(uint32_t dwVID)
{
    if (__IsSameTargetVID(dwVID))
    {
        __ClearTarget();
    }
}

void CPythonPlayer::NotifyCharacterUpdate(uint32_t dwVID)
{
    if (__IsSameTargetVID(dwVID))
    {
        const auto pMainInstance = NEW_GetMainActorPtr(), pTargetInstance = CPythonCharacterManager::Instance().
                       GetInstancePtr(dwVID);
        if (pMainInstance && pTargetInstance)
        {
            if (!pMainInstance->IsTargetableInstance(*pTargetInstance))
            {
                __ClearTarget();
                PyCallClassMemberFunc(m_ppyGameWindow, "CloseTargetBoard");
            }
            else
            {
                PyCallClassMemberFunc(m_ppyGameWindow.ptr(), "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
            }
        }
    }
}

void CPythonPlayer::NotifyDeadMainCharacter()
{
    __ClearAutoAttackTargetActorID();
    __ClearReservedAction();
}

void CPythonPlayer::NotifyChangePKMode() const
{
    PyCallClassMemberFunc(m_ppyGameWindow, "OnChangePKMode");
}

void CPythonPlayer::MoveItemData(TItemPos SrcCell, TItemPos DstCell)
{
    if (!SrcCell.IsValidItemPosition() || !DstCell.IsValidItemPosition())
        return;

    const auto srcItemData = GetItemData(SrcCell);
    const auto dstItemData = GetItemData(DstCell);

    if (!srcItemData || !dstItemData)
        return;

    //ClientItemData src_item(*srcItemData);
    //ClientItemData dst_item(*dstItemData);

    SetItemData(DstCell, *srcItemData);
    SetItemData(SrcCell, *dstItemData);
}

const ClientItemData *CPythonPlayer::GetItemData(TItemPos Cell) const
{
    if (!Cell.IsValidItemPosition())
        return nullptr;

    if (Cell.window_type == DRAGON_SOUL_INVENTORY)
    {
        return &m_playerStatus.aDSItem[Cell.cell];
    }

    auto w = m_windows.Get(Cell.window_type);
    if (!w || Cell.cell >= w->GetSize())
        return nullptr;

    return &w->GetPos(Cell.cell);
}

py::bytes CPythonPlayer::GetItemLink(TItemPos Cell) const
{
    const auto* pPlayerItem = GetItemData(Cell);
    if (!pPlayerItem)
        return "";

    const auto* pItemData = CItemManager::Instance().GetProto(pPlayerItem->vnum);

    std::string outString;

    if (pPlayerItem && pItemData)
    {
        std::string itemLink;
        bool isAttr = pPlayerItem->attrs[0].sValue != 0;

        itemLink += fmt::format("item:{:x}:{:x}",
                                 pPlayerItem->vnum, pPlayerItem->id);

        CItemManager::instance().RegisterHyperlinkItem(pPlayerItem->id, *pPlayerItem);

        if (isAttr)
        {
            outString = fmt::sprintf("|cffffc700|H%s|h |h|r", itemLink);
        }
        else
        {
            outString = fmt::sprintf("|cfff1e6c0|H%s|h |h|r", itemLink);
        }
    }

    return outString;
}

void CPythonPlayer::SetItemData(TItemPos Cell, const ClientItemData &data)
{
    if (!Cell.IsValidItemPosition())
        return;

    if (data.vnum != 0)
    {
        auto pItemData = CItemManager::Instance().GetProto(data.vnum);
        if (!pItemData)
        {
            SPDLOG_ERROR(
                "CPythonPlayer::SetItemData(window_type : {0}, dwSlotIndex={1}, itemIndex={2}) - Failed to item data\n",
                Cell.window_type, Cell.cell, data.vnum);
            return;
        }
    }

    if(!Engine::GetSettings().IsUsingItemHighlight())
        const_cast<ClientItemData&>(data).highlighted = false;

    if (Cell.window_type == DRAGON_SOUL_INVENTORY)
    {
        m_playerStatus.aDSItem[Cell.cell] = data;
        return;
    }

    auto w = m_windows.Get(Cell.window_type);
    if (!w || Cell.cell >= w->GetSize())
    {
        SPDLOG_ERROR("SetItemData: Invalid window/cell {0} {1}",
                      Cell.window_type, Cell.cell);
        return;
    }



    w->ClearPos(Cell.cell);
    w->PutPos(data, Cell.cell);

    //if (Cell.window_type == SWITCHBOT)
    //{
    //    PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_SetSwitchbotItem", Cell.cell);
    //}

    if (Cell.cell == WEAR_WEAPON && Cell.window_type == EQUIPMENT)
        UpdateBattleStatus();
}

uint32_t CPythonPlayer::GetItemTypeBySlot(TItemPos Cell)
{
    if (!Cell.IsValidItemPosition())
    {
        return 0;
    }

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return 0;

    const auto proto = CItemManager::Instance().GetProto(itemData->vnum);
    if (!proto)
        return 0;

    return proto->GetType();
}

uint32_t CPythonPlayer::GetItemSubTypeBySlot(TItemPos Cell)
{
    if (!Cell.IsValidItemPosition())
        return 0;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return 0;

    const auto proto = CItemManager::Instance().GetProto(itemData->vnum);
    if (!proto)
        return 0;

    return proto->GetSubType();
}

uint32_t CPythonPlayer::GetItemIndex(TItemPos Cell) const
{
    if (!Cell.IsValidItemPosition())
        return 0;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return 0;

    return itemData->vnum;
}

uint32_t CPythonPlayer::GetItemValue(TItemPos Cell, uint32_t index)
{
    if (!Cell.IsValidItemPosition())
        return 0;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return 0;

    const auto proto = CItemManager::instance().GetProto(itemData->vnum);
    if (!proto)
        return 0;

    return proto->GetValue(index);
}

uint32_t CPythonPlayer::GetItemChangeLookVnum(TItemPos Cell) const
{
    if (!Cell.IsValidItemPosition())
        return 0;

    if (const auto itemData = GetItemData(Cell); itemData)
        return itemData->transVnum;

    return 0;
}

uint32_t CPythonPlayer::GetItemFlags(TItemPos Cell)
{
    if (!Cell.IsValidItemPosition())
        return 0;

    const auto pItem = GetItemData(Cell);
    if (!pItem)
        return 0;

    if (const auto proto = CItemManager::instance().GetProto(pItem->vnum); proto)
        return proto->GetFlags();

    return 0;
}

uint32_t CPythonPlayer::GetItemAntiFlags(TItemPos Cell) const
{
    if (!Cell.IsValidItemPosition())
    {
        return 0;
    }

    const auto pItem = GetItemData(Cell);
    if (!pItem)
        return 0;

    if (const auto proto = CItemManager::instance().GetProto(pItem->vnum); proto)
        return proto->GetAntiFlags();

    return 0;
}

CountType CPythonPlayer::GetItemCount(TItemPos Cell) const
{
    if (!Cell.IsValidItemPosition())
        return 0;

    const auto itemData = GetItemData(Cell);
    if (itemData)
        return itemData->count;

    return 0;
}

int32_t CPythonPlayer::GetEmptyInventory(uint8_t size, uint8_t type, int32_t except)
{
    switch (type)
    {
    case ITEM_MATERIAL:
        return m_windows.inventory.FindBlankBetweenPages(size, NORMAL_INVENTORY_MAX_PAGE,
                                                         MATERIAL_INVENTORY_MAX_PAGE, except).value_or(-1);
    case ITEM_COSTUME:
        return m_windows.inventory.FindBlankBetweenPages(size, MATERIAL_INVENTORY_MAX_PAGE,
                                                         COSTUME_INVENTORY_MAX_PAGE, except).value_or(-1);
    default:
        return m_windows.inventory.FindBlankBetweenPages(size, NORMAL_INVENTORY_MIN_PAGE, NORMAL_INVENTORY_MAX_PAGE,
                                                         except).
                         value_or(-1);
    }
}

CountType CPythonPlayer::GetItemCountByVnum(uint32_t dwVnum)
{
    CountType dwCount = 0;

    for (int i = 0; i < m_windows.inventory.GetSize(); ++i)
    {
        const auto &data = m_windows.inventory.GetPos(i);
        if (data.vnum == dwVnum)
            dwCount += data.count;
    }

    return dwCount;
}

SocketValue CPythonPlayer::GetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex)
{
    if (!Cell.IsValidItemPosition())
        return 0;

    if (dwMetinSocketIndex >= ITEM_SOCKET_MAX_NUM)
        return 0;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return 0;

    return itemData->sockets[dwMetinSocketIndex];
}

TPlayerItemAttribute CPythonPlayer::GetItemAttributeNew(TItemPos Cell, uint32_t idx) const
{
    if (!Cell.IsValidItemPosition())
        return TPlayerItemAttribute{APPLY_NONE, 0};

    if (idx >= ITEM_ATTRIBUTE_MAX_NUM)
        return TPlayerItemAttribute{APPLY_NONE, 0};

    auto itemData = GetItemData(Cell);
    if (!itemData)
        return TPlayerItemAttribute{APPLY_NONE, 0};

    return itemData->attrs[idx];
}

void CPythonPlayer::GetItemAttribute(TItemPos Cell, uint32_t dwAttrSlotIndex, ApplyType *pbyType,
                                     ApplyValue *psValue) const
{
    *pbyType = 0;
    *psValue = 0;

    if (!Cell.IsValidItemPosition())
        return;

    if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_MAX_NUM)
        return;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return;

    *pbyType = itemData->attrs[dwAttrSlotIndex].bType;
    *psValue = itemData->attrs[dwAttrSlotIndex].sValue;
}

std::optional<std::vector<TPlayerItemAttribute>> CPythonPlayer::GetItemAttributes(const TItemPos &Cell) const
{
    if (!Cell.IsValidItemPosition())
        return std::nullopt;

    const auto itemData = GetItemData(Cell);
    if (!itemData)
        return std::nullopt;

    std::vector<TPlayerItemAttribute> attributes;

    for (auto &attr : itemData->attrs)
        attributes.push_back(attr);

    return attributes;
}

void CPythonPlayer::SetItemCount(TItemPos Cell, CountType byCount)
{
    if (!Cell.IsValidItemPosition())
        return;

    (const_cast<ClientItemData *>(GetItemData(Cell)))->count = byCount;

    PyCallClassMemberFunc(m_ppyGameWindow.ptr(), "RefreshInventory", Py_BuildValue("()"));
}

void CPythonPlayer::SetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber)
{
    if (!Cell.IsValidItemPosition())
        return;
    if (dwMetinSocketIndex >= ITEM_SOCKET_MAX_NUM)
        return;

    (const_cast<ClientItemData *>(GetItemData(Cell)))->sockets[dwMetinSocketIndex] = dwMetinNumber;
}

void CPythonPlayer::SetItemAttribute(TItemPos Cell, uint32_t dwAttrIndex, uint8_t byType, short sValue)
{
    if (!Cell.IsValidItemPosition())
        return;
    if (dwAttrIndex >= ITEM_ATTRIBUTE_MAX_NUM)
        return;

    (const_cast<ClientItemData *>(GetItemData(Cell)))->attrs[dwAttrIndex].bType = byType;
    (const_cast<ClientItemData *>(GetItemData(Cell)))->attrs[dwAttrIndex].sValue = sValue;
}

// cyh itemseal 2013 11 11
bool CPythonPlayer::SealItem(TItemPos Cell)
{
    const ClientItemData *pData = GetItemData(Cell);
    if (!pData)
    {
        SPDLOG_ERROR("FAILED  CPythonPlayer::SealItem()  Cell({0}, {1}) item is null", Cell.window_type, Cell.cell);
        return false;
    }

    (const_cast<ClientItemData *>(pData))->nSealDate = E_SEAL_DATE_UNLIMITED_TIMESTAMP;

    return true;
}

bool CPythonPlayer::UnSealItem(TItemPos Cell)
{
    const ClientItemData *pData = GetItemData(Cell);
    if (!pData)
    {
        SPDLOG_ERROR("FAILED  CPythonPlayer::SealItem()  Cell({0}, {1}) item is null", Cell.window_type, Cell.cell);
        return false;
    }

    time_t curTime;
    time(&curTime);

    curTime += (24 * 3600);

    (const_cast<ClientItemData *>(pData))->nSealDate = curTime;

    return true;
}

int CPythonPlayer::GetQuickPage()
{
    return m_playerStatus.lQuickPageIndex;
}

void CPythonPlayer::SetQuickPage(int nQuickPageIndex)
{
    if (nQuickPageIndex < 0)
        m_playerStatus.lQuickPageIndex = QUICKSLOT_MAX_LINE + nQuickPageIndex;
    else if (nQuickPageIndex >= QUICKSLOT_MAX_LINE)
        m_playerStatus.lQuickPageIndex = nQuickPageIndex % QUICKSLOT_MAX_LINE;
    else
        m_playerStatus.lQuickPageIndex = nQuickPageIndex;

    PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory");
}

uint32_t CPythonPlayer::LocalQuickSlotIndexToGlobalQuickSlotIndex(uint32_t dwLocalSlotIndex)
{
    return m_playerStatus.lQuickPageIndex * QUICKSLOT_MAX_COUNT_PER_LINE + dwLocalSlotIndex;
}

void CPythonPlayer::GetGlobalQuickSlotData(uint32_t dwGlobalSlotIndex, uint32_t *pdwWndType, uint32_t *pdwWndItemPos)
{
    TQuickslot &rkQuickSlot = __RefGlobalQuickSlot(dwGlobalSlotIndex);
    *pdwWndType = rkQuickSlot.type;
    *pdwWndItemPos = rkQuickSlot.pos;
}

void CPythonPlayer::GetLocalQuickSlotData(uint32_t dwSlotPos, uint32_t *pdwWndType, uint32_t *pdwWndItemPos)
{
    TQuickslot &rkQuickSlot = __RefLocalQuickSlot(dwSlotPos);
    *pdwWndType = rkQuickSlot.type;
    *pdwWndItemPos = rkQuickSlot.pos;
}

TQuickslot CPythonPlayer::GetLocalQuickSlot(int SlotIndex)
{
    return GetGlobalQuickSlot(LocalQuickSlotIndexToGlobalQuickSlotIndex(SlotIndex));
}

TQuickslot CPythonPlayer::GetGlobalQuickSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= QUICKSLOT_MAX_NUM)
    {
        TQuickslot s_kQuickSlot;
        s_kQuickSlot.type = 0;
        s_kQuickSlot.pos = 0;
        return s_kQuickSlot;
    }

    return m_playerStatus.aQuickSlot[SlotIndex];
}

TQuickslot &CPythonPlayer::__RefLocalQuickSlot(int SlotIndex)
{
    return __RefGlobalQuickSlot(LocalQuickSlotIndexToGlobalQuickSlotIndex(SlotIndex));
}

TQuickslot &CPythonPlayer::__RefGlobalQuickSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= QUICKSLOT_MAX_NUM)
    {
        static TQuickslot s_kQuickSlot;
        s_kQuickSlot.type = 0;
        s_kQuickSlot.pos = 0;
        return s_kQuickSlot;
    }

    return m_playerStatus.aQuickSlot[SlotIndex];
}

void CPythonPlayer::RemoveQuickSlotByValue(int iType, int iPosition)
{
    for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
    {
        if (iType == m_playerStatus.aQuickSlot[i].type)
            if (iPosition == m_playerStatus.aQuickSlot[i].pos)
                gPythonNetworkStream->SendQuickSlotDelPacket(i);
    }
}

bool CPythonPlayer::IsItem(TItemPos Cell)
{
    if (!Cell.IsValidItemPosition())
        return 0;

    return 0 != GetItemData(Cell)->vnum;
}

void CPythonPlayer::RequestMoveGlobalQuickSlotToLocalQuickSlot(uint32_t dwGlobalSrcSlotIndex,
                                                               uint32_t dwLocalDstSlotIndex)
{
    //uint32_t dwGlobalSrcSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSrcSlotIndex);
    uint32_t dwGlobalDstSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalDstSlotIndex);

    gPythonNetworkStream->SendQuickSlotMovePacket((uint8_t)dwGlobalSrcSlotIndex, (uint8_t)dwGlobalDstSlotIndex);
}

void CPythonPlayer::RequestAddLocalQuickSlot(uint32_t dwLocalSlotIndex, uint32_t dwWndType, uint32_t dwWndItemPos)
{
    if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
        return;

    uint32_t dwGlobalSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

    gPythonNetworkStream->SendQuickSlotAddPacket((uint8_t)dwGlobalSlotIndex, (uint8_t)dwWndType, (uint8_t)dwWndItemPos);
}

void CPythonPlayer::RequestAddToEmptyLocalQuickSlot(uint32_t dwWndType, uint32_t dwWndItemPos)
{
    for (int i = 0; i < QUICKSLOT_MAX_COUNT_PER_LINE; ++i)
    {
        TQuickslot &rkQuickSlot = __RefLocalQuickSlot(i);

        if (0 == rkQuickSlot.type)
        {
            uint32_t dwGlobalQuickSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(i);
            gPythonNetworkStream->SendQuickSlotAddPacket((uint8_t)dwGlobalQuickSlotIndex, (uint8_t)dwWndType,
                                                         (uint8_t)dwWndItemPos);
            return;
        }
    }
}

void CPythonPlayer::RequestDeleteGlobalQuickSlot(uint32_t dwGlobalSlotIndex)
{
    if (dwGlobalSlotIndex >= QUICKSLOT_MAX_COUNT)
        return;

    gPythonNetworkStream->SendQuickSlotDelPacket((uint8_t)dwGlobalSlotIndex);
}

void CPythonPlayer::RequestUseLocalQuickSlot(uint32_t dwLocalSlotIndex)
{
    if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
        return;

    uint32_t dwRegisteredType;
    uint32_t dwRegisteredItemPos;
    GetLocalQuickSlotData(dwLocalSlotIndex, &dwRegisteredType, &dwRegisteredItemPos);

    switch (dwRegisteredType)
    {
    case SLOT_TYPE_INVENTORY: {
        gPythonNetworkStream->SendItemUsePacket(TItemPos(INVENTORY, dwRegisteredItemPos));
        break;
    }
    case SLOT_TYPE_SKILL: {
        ClickSkillSlot(dwRegisteredItemPos);
        break;
    }
    case SLOT_TYPE_EMOTION: {
        m_ppyGameWindow.attr("BINARY_ActEmotion")(dwRegisteredItemPos);
        break;
    }
    }
}

void CPythonPlayer::AddQuickSlot(int QuickSlotIndex, char IconType, uint16_t IconPosition)
{
    if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
        return;

    m_playerStatus.aQuickSlot[QuickSlotIndex].type = IconType;
    m_playerStatus.aQuickSlot[QuickSlotIndex].pos = IconPosition;
}

void CPythonPlayer::DeleteQuickSlot(int QuickSlotIndex)
{
    if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
        return;

    m_playerStatus.aQuickSlot[QuickSlotIndex].type = 0;
    m_playerStatus.aQuickSlot[QuickSlotIndex].pos = 0;
}

void CPythonPlayer::MoveQuickSlot(int Source, int Target)
{
    if (Source < 0 || Source >= QUICKSLOT_MAX_NUM)
        return;

    if (Target < 0 || Target >= QUICKSLOT_MAX_NUM)
        return;

    TQuickslot &rkSrcSlot = __RefGlobalQuickSlot(Source);
    TQuickslot &rkDstSlot = __RefGlobalQuickSlot(Target);

    std::swap(rkSrcSlot, rkDstSlot);
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
bool CPythonPlayer::IsBeltInventorySlot(TItemPos Cell)
{
    return Cell.IsBeltInventoryPosition();
}
#endif

bool CPythonPlayer::IsInventorySlot(TItemPos Cell)
{
    return !Cell.IsEquipPosition() && Cell.IsValidItemPosition();
}

bool CPythonPlayer::IsEquipmentSlot(TItemPos Cell)
{
    return Cell.IsEquipPosition();
}

bool CPythonPlayer::IsEquipItemInSlot(TItemPos Cell)
{
    if (!Cell.IsEquipPosition())
    {
        return false;
    }

    const ClientItemData *pData = GetItemData(Cell);

    if (NULL == pData)
    {
        return false;
    }

    uint32_t dwItemIndex = pData->vnum;

    CItemManager::Instance().SelectItemData(dwItemIndex);
    CItemData *pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
    {
        SPDLOG_ERROR("Failed to find ItemData - CPythonPlayer::IsEquipItem(window_type={0}, iSlotindex={1})\n",
                      Cell.window_type, Cell.cell);
        return false;
    }

    return pItemData->IsEquipment() ? true : false;
}

void CPythonPlayer::SetSkill(uint32_t dwSlotIndex, uint32_t dwSkillIndex)
{
    m_Skill[dwSlotIndex].dwIndex = dwSkillIndex;
    m_skillSlotDict[dwSkillIndex] = dwSlotIndex;
}

int CPythonPlayer::GetSkillIndex(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].dwIndex;
}

std::optional<uint32_t> CPythonPlayer::GetSkillSlotIndexNew(uint32_t dwSkillIndex)
{
    auto f = m_skillSlotDict.find(dwSkillIndex);
    if (m_skillSlotDict.end() == f)
    {
        return std::nullopt;
    }
    return f->second;
}

bool CPythonPlayer::GetSkillSlotIndex(uint32_t dwSkillIndex, uint32_t *pdwSlotIndex)
{
    auto f = m_skillSlotDict.find(dwSkillIndex);
    if (m_skillSlotDict.end() == f)
    {
        return false;
    }

    *pdwSlotIndex = f->second;

    return true;
}

int CPythonPlayer::GetSkillGrade(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].iGrade;
}

int CPythonPlayer::GetSkillDisplayLevel(uint32_t dwSlotIndex)
{
    return GetRelativeSkillLevel(m_Skill[dwSlotIndex].iGrade, m_Skill[dwSlotIndex].iLevel);
}

int CPythonPlayer::GetSkillLevel(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].iLevel;
}

uint32_t CPythonPlayer::GetSkillColor(uint32_t slotIndex)
{
    return m_Skill[slotIndex].color;
}

float CPythonPlayer::GetSkillCurrentEfficientPercentage(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].fcurEfficientPercentage;
}

float CPythonPlayer::GetSkillNextEfficientPercentage(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].fnextEfficientPercentage;
}

void CPythonPlayer::SetSkillLevel(uint32_t dwSlotIndex, uint32_t dwSkillLevel)
{
    assert(!"CPythonPlayer::SetSkillLevel - ������� �ʴ� �Լ�");

    m_Skill[dwSlotIndex].iGrade = -1;
    m_Skill[dwSlotIndex].iLevel = dwSkillLevel;
}

void CPythonPlayer::SetSkillInfo(uint32_t vnum, uint32_t grade, uint32_t level, uint32_t color)
{
    uint32_t dwSlotIndex;
    if (!GetSkillSlotIndex(vnum, &dwSlotIndex))
        return;

    auto &slot = m_Skill[dwSlotIndex];

    if (slot.iGrade != static_cast<int>(grade))
    {
        if (m_ppyGameWindow && !py::isinstance<py::none>(m_ppyGameWindow))
            m_ppyGameWindow.attr("SkillGradeChange")(dwSlotIndex, slot.iGrade, grade);
    }

    slot.iGrade = grade;
    slot.iLevel = level;
    slot.color = color;

    if (level > SKILL_MAX_LEVEL)
    {
        slot.fcurEfficientPercentage = 0.0f;
        slot.fnextEfficientPercentage = 0.0f;

        SPDLOG_ERROR("Skill level {0} > {1} for {2}/{3}",
		          level, SKILL_MAX_LEVEL, vnum, dwSlotIndex);
        return;
    }

    slot.fcurEfficientPercentage = GetSkillPowerByLevel(level) / 100.0f;
    slot.fnextEfficientPercentage = GetSkillPowerByLevel(level + 1) / 100.0f;
}

void CPythonPlayer::SetSkillCoolTime(uint32_t dwSkillIndex)
{
    uint32_t dwSlotIndex;
    if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
    {
        SPDLOG_DEBUG("CPythonPlayer::SetSkillCoolTime(dwSkillIndex={0}) - FIND SLOT ERROR", dwSkillIndex);
        return;
    }


    m_Skill[dwSlotIndex].isCoolTime = true;
}

void CPythonPlayer::EndSkillCoolTime(uint32_t dwSkillIndex)
{
    uint32_t dwSlotIndex;
    if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
    {
        SPDLOG_DEBUG("CPythonPlayer::EndSkillCoolTime(dwSkillIndex={0}) - FIND SLOT ERROR", dwSkillIndex);
        return;
    }


    m_Skill[dwSlotIndex].isCoolTime = false;
}

double CPythonPlayer::GetSkillCoolTime(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].fCoolTime;
}

double CPythonPlayer::GetSkillElapsedCoolTime(uint32_t dwSlotIndex)
{
    return DX::StepTimer::Instance().GetTotalSeconds() - m_Skill[dwSlotIndex].fLastUsedTime;
}

void CPythonPlayer::ActivateSkillSlot(uint32_t dwSlotIndex)
{

    m_Skill[dwSlotIndex].bActive = TRUE;
    PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSkillSlot", dwSlotIndex);
}

void CPythonPlayer::DeactivateSkillSlot(uint32_t dwSlotIndex)
{
    m_Skill[dwSlotIndex].bActive = FALSE;
    PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSkillSlot", dwSlotIndex);
}

bool CPythonPlayer::IsSkillCoolTime(uint32_t dwSlotIndex)
{
    if (!__CheckRestSkillCoolTime(dwSlotIndex))
        return FALSE;

    return TRUE;
}

bool CPythonPlayer::IsSkillActive(uint32_t dwSlotIndex)
{
    return m_Skill[dwSlotIndex].bActive;
}

bool CPythonPlayer::IsToggleSkill(uint32_t dwSlotIndex)
{

    uint32_t dwSkillIndex = m_Skill[dwSlotIndex].dwIndex;

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
        return FALSE;

    return pSkillData->IsToggleSkill();
}

void CPythonPlayer::SetPlayTime(uint32_t dwPlayTime)
{
    m_dwPlayTime = dwPlayTime;
}

uint32_t CPythonPlayer::GetPlayTime()
{
    return m_dwPlayTime;
}

void CPythonPlayer::SendClickItemPacket(uint32_t dwIID)
{
    if (IsObserverMode() && !NEW_GetMainActorPtr()->IsGameMaster())
        return;

    const char *c_szOwnerName;
    if (!CPythonItem::Instance().GetOwnership(dwIID, &c_szOwnerName))
        return;

    if (strlen(c_szOwnerName) > 0)
        if (0 != strcmp(c_szOwnerName, GetName()))
        {
            CItemData *pItemData;
            if (!CItemManager::Instance().GetItemDataPointer(
                CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
            {
                SPDLOG_DEBUG("CPythonPlayer::SendClickItemPacket(dwIID={0}) : Non-exist item.", dwIID);
                return;
            }
            if (!IsPartyMemberByName(c_szOwnerName) || pItemData->IsAntiFlag(ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE))
            {
                //PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotPickItem", Py_BuildValue("()"));
                return;
            }
        }

    gPythonNetworkStream->SendItemPickUpPacket(dwIID);
}

void CPythonPlayer::__SendClickActorPacket(CInstanceBase &rkInstVictim)
{
    // ���� Ÿ�� ������ ĳ�� �Ϳ� ���� ���� ó��
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (pkInstMain && pkInstMain->IsHoldingPickAxe() && pkInstMain->IsMountingHorse() && rkInstVictim.IsResource())
    {
        m_ppyGameWindow.attr("OnCannotMining")();
        return;
    }

    static uint32_t s_dwNextTCPTime = 0;

    uint32_t dwCurTime = ELTimer_GetMSec();

    if (dwCurTime >= s_dwNextTCPTime)
    {
        s_dwNextTCPTime = dwCurTime + 1000;

        uint32_t dwVictimVID = rkInstVictim.GetVirtualID();
        gPythonNetworkStream->SendOnClickPacket(dwVictimVID);
    }
}

void CPythonPlayer::ActEmotion(uint32_t dwEmotionID)
{
    CInstanceBase *pkInstTarget = __GetAliveTargetInstancePtr();
    if (!pkInstTarget)
    {
        PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", GetMainCharacterIndex(), "NEED_TARGET");
        return;
    }

    gPythonNetworkStream->SendChatPacket(_getf("/kiss %s", pkInstTarget->GetNameString()));
}

void CPythonPlayer::StartEmotionProcess()
{
    __ClearReservedAction();
    __ClearAutoAttackTargetActorID();

    m_bisProcessingEmotion = TRUE;
}

void CPythonPlayer::EndEmotionProcess()
{
    m_bisProcessingEmotion = FALSE;
}

bool CPythonPlayer::__IsProcessingEmotion()
{
    return m_bisProcessingEmotion;
}

// Dungeon
void CPythonPlayer::SetDungeonDestinationPosition(int ix, int iy)
{
    m_isDestPosition = TRUE;
    m_ixDestPos = ix;
    m_iyDestPos = iy;

    AlarmHaveToGo();
}

void CPythonPlayer::AlarmHaveToGo()
{
    m_iLastAlarmTime = DX::StepTimer::instance().GetTotalMillieSeconds();

    /////

    CInstanceBase *pInstance = NEW_GetMainActorPtr();
    if (!pInstance)
        return;

    TPixelPosition PixelPosition;
    pInstance->NEW_GetPixelPosition(&PixelPosition);

    float fAngle = GetDegreeFromPosition2(PixelPosition.x, PixelPosition.y, float(m_ixDestPos), float(m_iyDestPos));
    fAngle = fmod(540.0f - fAngle, 360.0f);

    auto &mgr = CEffectManager::Instance();
    mgr.RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
    auto index = mgr.CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");

    Matrix mat = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(fAngle));
    mat._41 = PixelPosition.x;
    mat._42 = -PixelPosition.y;
    mat._43 = PixelPosition.z;

    auto* effect = mgr.GetEffectInstance(index);
    if (effect)
        effect->SetGlobalMatrix(mat);
}

// Party
void CPythonPlayer::ExitParty()
{
    m_PartyMemberMap.clear();

    CPythonCharacterManager::Instance().RefreshAllPCTextTail();
}

void CPythonPlayer::AppendPartyMember(uint32_t dwPID, const char *c_szName)
{
    m_PartyMemberMap.insert(std::make_pair(dwPID, TPartyMemberInfo(dwPID, c_szName)));
}

void CPythonPlayer::LinkPartyMember(uint32_t dwPID, uint32_t dwVID)
{
    TPartyMemberInfo *pPartyMemberInfo;
    if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        SPDLOG_ERROR(" CPythonPlayer::LinkPartyMember(dwPID={0}, dwVID={1}) - Failed to find party member", dwPID,
                      dwVID);
        return;
    }

    pPartyMemberInfo->dwVID = dwVID;

    CInstanceBase *pInstance = NEW_FindActorPtr(dwVID);
    if (pInstance)
        pInstance->RefreshTextTail();
}

void CPythonPlayer::UnlinkPartyMember(uint32_t dwPID)
{
    TPartyMemberInfo *pPartyMemberInfo;
    if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        SPDLOG_ERROR(" CPythonPlayer::UnlinkPartyMember(dwPID={0}) - Failed to find party member", dwPID);
        return;
    }

    pPartyMemberInfo->dwVID = 0;
}

void CPythonPlayer::UpdatePartyMemberInfo(uint32_t dwPID, uint8_t byState, uint8_t byHPPercentage, uint8_t byIsLeader, uint8_t byRace)
{
    TPartyMemberInfo *pPartyMemberInfo;
    if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        SPDLOG_ERROR(
            " CPythonPlayer::UpdatePartyMemberInfo(dwPID={0}, byState={1}, byHPPercentage={2}) - Failed to find character",
            dwPID, byState, byHPPercentage);
        return;
    }

    pPartyMemberInfo->byState = byState;
    pPartyMemberInfo->byHPPercentage = byHPPercentage;
    pPartyMemberInfo->byIsLeader = byIsLeader;
    pPartyMemberInfo->byRace = byRace;

}

void CPythonPlayer::UpdatePartyMemberAffect(uint32_t dwPID, uint8_t byAffectSlotIndex, short sAffectNumber)
{
    if (byAffectSlotIndex >= 7)
    {
        SPDLOG_ERROR(
            " CPythonPlayer::UpdatePartyMemberAffect(dwPID={0}, byAffectSlotIndex={1}, sAffectNumber={2}) - Strange affect slot index",
            dwPID, byAffectSlotIndex, sAffectNumber);
        return;
    }

    TPartyMemberInfo *pPartyMemberInfo;
    if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        SPDLOG_ERROR(
            " CPythonPlayer::UpdatePartyMemberAffect(dwPID={0}, byAffectSlotIndex={1}, sAffectNumber={2}) - Failed to find character",
            dwPID, byAffectSlotIndex, sAffectNumber);
        return;
    }

    pPartyMemberInfo->sAffects[byAffectSlotIndex] = sAffectNumber;
}

void CPythonPlayer::RemovePartyMember(uint32_t dwPID)
{
    uint32_t dwVID = 0;
    TPartyMemberInfo *pPartyMemberInfo;
    if (GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        dwVID = pPartyMemberInfo->dwVID;
    }

    m_PartyMemberMap.erase(dwPID);

    if (dwVID > 0)
    {
        CInstanceBase *pInstance = NEW_FindActorPtr(dwVID);
        if (pInstance)
            pInstance->RefreshTextTail();
    }
}

bool CPythonPlayer::IsPartyMemberByVID(uint32_t dwVID)
{
    auto itor = m_PartyMemberMap.begin();
    for (; itor != m_PartyMemberMap.end(); ++itor)
    {
        TPartyMemberInfo &rPartyMemberInfo = itor->second;
        if (dwVID == rPartyMemberInfo.dwVID)
            return true;
    }

    return false;
}

bool CPythonPlayer::IsPartyMemberByName(const char *c_szName)
{
    for (const auto &p : m_PartyMemberMap)
    {
        const auto &rPartyMemberInfo = p.second;
        if (0 == rPartyMemberInfo.strName.compare(c_szName))
            return true;
    }

    return false;
}

bool CPythonPlayer::GetPartyMemberPtr(uint32_t dwPID, TPartyMemberInfo **ppPartyMemberInfo)
{
    auto itor = m_PartyMemberMap.find(dwPID);

    if (m_PartyMemberMap.end() == itor)
        return false;

    *ppPartyMemberInfo = &(itor->second);

    return true;
}

bool CPythonPlayer::PartyMemberPIDToVID(uint32_t dwPID, uint32_t *pdwVID)
{
    auto itor = m_PartyMemberMap.find(dwPID);

    if (m_PartyMemberMap.end() == itor)
        return false;

    const TPartyMemberInfo &c_rPartyMemberInfo = itor->second;
    *pdwVID = c_rPartyMemberInfo.dwVID;

    return true;
}

bool CPythonPlayer::PartyMemberVIDToPID(uint32_t dwVID, uint32_t *pdwPID)
{
    for (const auto &p : m_PartyMemberMap)
    {
        const auto &rPartyMemberInfo = p.second;
        if (dwVID == rPartyMemberInfo.dwVID)
        {
            *pdwPID = rPartyMemberInfo.dwPID;
            return true;
        }
    }

    return false;
}

bool CPythonPlayer::IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2)
{
    return (IsPartyMemberByVID(dwVID1) && IsPartyMemberByVID(dwVID2));
}

// PVP
void CPythonPlayer::RememberChallengeInstance(uint32_t dwVID)
{
    m_RevengeInstanceSet.erase(dwVID);
    m_ChallengeInstanceSet.insert(dwVID);
}

void CPythonPlayer::RememberRevengeInstance(uint32_t dwVID)
{
    m_ChallengeInstanceSet.erase(dwVID);
    m_RevengeInstanceSet.insert(dwVID);
}

void CPythonPlayer::RememberCantFightInstance(uint32_t dwVID)
{
    m_CantFightInstanceSet.insert(dwVID);
}

void CPythonPlayer::ForgetInstance(uint32_t dwVID)
{
    m_ChallengeInstanceSet.erase(dwVID);
    m_RevengeInstanceSet.erase(dwVID);
    m_CantFightInstanceSet.erase(dwVID);
}

bool CPythonPlayer::IsChallengeInstance(uint32_t dwVID)
{
    return m_ChallengeInstanceSet.end() != m_ChallengeInstanceSet.find(dwVID);
}

bool CPythonPlayer::IsRevengeInstance(uint32_t dwVID)
{
    return m_RevengeInstanceSet.end() != m_RevengeInstanceSet.find(dwVID);
}

bool CPythonPlayer::IsCantFightInstance(uint32_t dwVID)
{
    return m_CantFightInstanceSet.end() != m_CantFightInstanceSet.find(dwVID);
}

void CPythonPlayer::OpenPrivateShop()
{
    m_isOpenPrivateShop = TRUE;
}

void CPythonPlayer::ClosePrivateShop()
{
    m_isOpenPrivateShop = FALSE;
}

bool CPythonPlayer::IsOpenPrivateShop()
{
    return m_isOpenPrivateShop;
}

void CPythonPlayer::SetObserverMode(bool isEnable)
{
    m_isObserverMode = isEnable;
}

bool CPythonPlayer::IsObserverMode()
{
    return m_isObserverMode;
}

bool CPythonPlayer::IsGameMaster()
{
    return NEW_GetMainActorPtr() ? NEW_GetMainActorPtr()->IsGameMaster() : false;
}

bool CPythonPlayer::IsFishing()
{
    return NEW_GetMainActorPtr() ? NEW_GetMainActorPtr()->IsFishing() : false;
}


void CPythonPlayer::SetFreeCameraMode(bool isEnable)
{
    m_isFreeCameraMode = isEnable;
}

bool CPythonPlayer::IsFreeCameraMode()
{
    return m_isFreeCameraMode;
}

bool CPythonPlayer::__ToggleCoolTime()
{
    m_sysIsCoolTime = 1 - m_sysIsCoolTime;
    return m_sysIsCoolTime;
}

bool CPythonPlayer::__ToggleLevelLimit()
{
    m_sysIsLevelLimit = 1 - m_sysIsLevelLimit;
    return m_sysIsLevelLimit;
}

uint32_t CPythonPlayer::GetPKMode()
{
    CInstanceBase *pInstance = NEW_GetMainActorPtr();
    if (!pInstance)
        return 0;

    return pInstance->GetPKMode();
}

void CPythonPlayer::SetGameWindow(py::object ppyObject)
{
    m_ppyGameWindow = std::move(ppyObject);
}

void CPythonPlayer::NEW_ClearSkillData(bool bAll)
{
    for (auto it = m_skillSlotDict.begin(); it != m_skillSlotDict.end();)
    {
        if (bAll || __GetSkillType(it->first) == CPythonSkill::SKILL_TYPE_ACTIVE)
            it = m_skillSlotDict.erase(it);
        else
            ++it;
    }


    PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_CheckGameButton");
}

void CPythonPlayer::ClearSkillDict()
{
    // ClearSkillDict
    m_skillSlotDict.clear();
    m_Skill.clear();
    // Game End - Player Data Reset
    m_isOpenPrivateShop = false;
    m_isObserverMode = false;
    m_isFreeCameraMode = false;

    m_bMobileFlag = FALSE;

    __ClearAutoAttackTargetActorID();
}

void CPythonPlayer::Clear()
{
    memset(&m_playerStatus, 0, sizeof(m_playerStatus));
    NEW_ClearSkillData(true);

    m_bisProcessingEmotion = FALSE;

    m_dwSendingTargetVID = 0;
    m_fTargetUpdateTime = 0.0f;

    // Test Code for Status Interface
    m_stName = "";
    m_dwMainCharacterIndex = 0;
    m_dwRace = 0;

    /////
    m_MovingCursorPosition = TPixelPosition(0, 0, 0);
    m_fMovingCursorSettingTime = 0.0f;

    m_eReservedMode = MODE_NONE;
    m_fReservedDelayTime = 0.0f;
    m_kPPosReserved = TPixelPosition(0, 0, 0);
    m_dwVIDReserved = 0;
    m_dwIIDReserved = 0;
    m_dwSkillSlotIndexReserved = 0;
    m_dwSkillRangeReserved = 0;

    m_isUp = false;
    m_isDown = false;
    m_isLeft = false;
    m_isRight = false;
    m_isSmtMov = false;
    m_isDirMov = false;
    m_isDirKey = false;
    m_isAtkKey = false;

    m_isCmrRot = true;
    m_fCmrRotSpd = 20.0f;

    m_playerStatus.Clear();
    m_windows.Clear();

    m_iComboOld = 0;

    m_dwVIDPicked = 0;
    m_dwIIDPicked = 0;

    m_dwcurSkillSlotIndex = uint32_t(-1);

    m_dwTargetVID = 0;
    m_dwTargetEndTime = 0;

    m_PartyMemberMap.clear();

    m_ChallengeInstanceSet.clear();
    m_RevengeInstanceSet.clear();

    m_isOpenPrivateShop = false;
    m_isObserverMode = false;
    m_isFreeCameraMode = false;

    m_inGuildAreaID = 0xffffffff;

    __ClearAutoAttackTargetActorID();

    m_ItemAcceInstanceVector.clear();
    m_ItemAcceInstanceVector.resize(ACCE_SLOT_MAX_NUM);

    for (uint32_t i = 0; i < m_ItemAcceInstanceVector.size(); ++i)
    {
        ClientItemData &rInstance = m_ItemAcceInstanceVector[i];
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
    m_acceRefineWindowIsOpen = false;
    m_acceRefineWindowType = 3;

    for (auto &slot : m_acceRefineActivedSlot)
        slot = -1;

    m_isChangeLookWindowOpen = false;
    m_ItemChangeLookInstanceVector.clear();
    m_ItemChangeLookInstanceVector.reserve(CHANGELOOK_SLOT_MAX);
    m_ItemChangeLookInstanceVector.resize(CHANGELOOK_SLOT_MAX);

    for (uint32_t i = 0; i < m_ItemChangeLookInstanceVector.size(); ++i)
    {
        ClientItemData &rInstance = m_ItemChangeLookInstanceVector[i];
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
    for (auto &slot : m_changeLookActivedSlot)
    {
        slot.window_type = INVENTORY;
        slot.cell = INVENTORY_MAX_NUM;
    }

    for (auto &slot : m_itemCombinationActivedItemSlot)
        slot = -1;
}

void CPythonPlayer::LeaveGamePhase()
{
    m_iPVPTeam = -1;
    SetComboSkillFlag(false);
    m_switchbotSlotData.fill(std::nullopt);
}

CPythonPlayer::CPythonPlayer(void)
{
    m_ppyGameWindow.release();

    m_sysIsCoolTime = TRUE;
    m_sysIsLevelLimit = TRUE;
    m_dwPlayTime = 0;

    m_aeMBFButton[MBT_LEFT] = CPythonPlayer::MBF_SMART;
    m_aeMBFButton[MBT_RIGHT] = CPythonPlayer::MBF_CAMERA;
    m_aeMBFButton[MBT_MIDDLE] = CPythonPlayer::MBF_CAMERA;

    memset(m_adwEffect, 0, sizeof(m_adwEffect));
    m_iPVPTeam = -1;

    m_isDestPosition = FALSE;
    m_ixDestPos = 0;
    m_iyDestPos = 0;
    m_iLastAlarmTime = 0;

    Clear();
}

CPythonPlayer::~CPythonPlayer(void)
{
}

void CPythonPlayer::SetGold(Gold value)
{
    m_playerStatus.SetGold(value);
}

Gold CPythonPlayer::GetGold()
{
    return m_playerStatus.GetGold();
}

#ifdef ENABLE_GEM_SYSTEM
void CPythonPlayer::SetGem(int value)
{
	m_playerStatus.SetGem(value);
}

int CPythonPlayer::GetGem()
{
	return m_playerStatus.GetGem();
}
#endif

void CPythonPlayer::SetAcceItemData(uint32_t dwSlotIndex, const ClientItemData &rItemInstance)
{
    if (dwSlotIndex >= m_ItemAcceInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonSafeBox::SetAcceItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    if (dwSlotIndex != 2)
    {
        m_ppyGameWindow.attr("ActivateSlot")(m_acceRefineActivedSlot[dwSlotIndex], UI::HILIGHTSLOT_ACCE);
    }

    m_ItemAcceInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonPlayer::DelAcceItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemAcceInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::DelAcceItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }
    ClientItemData &rInstance = m_ItemAcceInstanceVector[dwSlotIndex];

    CItemData *pItemData = CItemManager::instance().GetProto(rInstance.vnum);
    if (pItemData)
    {
        if (dwSlotIndex == ACCE_SLOT_LEFT)
            DelAcceItemData(ACCE_SLOT_RESULT);

        if (pItemData->GetType() == ITEM_WEAPON || pItemData->GetType() == ITEM_ARMOR)
            DelAcceItemData(ACCE_SLOT_RESULT);

        if (dwSlotIndex != ACCE_SLOT_RESULT)
        {
            m_ppyGameWindow.attr("DeactivateSlot")(m_acceRefineActivedSlot[dwSlotIndex], UI::HILIGHTSLOT_ACCE);
            m_acceRefineActivedSlot[dwSlotIndex] = INVENTORY_MAX_NUM;
        }
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
}

int CPythonPlayer::GetCurrentAcceSize()
{
    return m_ItemAcceInstanceVector.size();
}

bool CPythonPlayer::GetAcceSlotItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID)
{
    if (dwSlotIndex >= m_ItemAcceInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::GetAcceSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *pdwItemID = m_ItemAcceInstanceVector[dwSlotIndex].vnum;

    return TRUE;
}

bool CPythonPlayer::GetAcceItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance)
{
    if (dwSlotIndex >= m_ItemAcceInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::GetAcceItemDataPtr(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return FALSE;
    }

    *ppInstance = &m_ItemAcceInstanceVector[dwSlotIndex];

    return TRUE;
}

std::optional<ClientItemData> CPythonPlayer::GetAcceItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemAcceInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::GetAcceItemDataPtr(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return std::nullopt;
    }

    return m_ItemAcceInstanceVector[dwSlotIndex];
}
bool CPythonPlayer::IsEmtpyAcceWindow()
{
    for (auto &i : m_ItemAcceInstanceVector)
    {
        if (i.vnum)
            break;

        return false;
    }

    return true;
}

int CPythonPlayer::GetCurrentAcceItemCount()
{
    int itemCount = 0;
    for (auto &i : m_ItemAcceInstanceVector)
    {
        if (i.vnum)
            ++itemCount;
    }

    return itemCount;
}

void CPythonPlayer::SetAcceRefineWindowOpen(int type)
{
    m_acceRefineWindowType = type;
    m_acceRefineWindowIsOpen = m_acceRefineWindowIsOpen == 0;
}

void CPythonPlayer::SetActivedAcceSlot(int acceSlot, int itemPos)
{
    m_acceRefineActivedSlot[acceSlot] = itemPos;
}

int CPythonPlayer::FindActivedSlot(int acceSlot)
{
    for (int i = 0; i < 3; ++i)
    {
        if (m_acceRefineActivedSlot[i] == acceSlot)
            return i;
    }

    return 3;
}

int CPythonPlayer::FindUsingSlot(int acceSlot)
{
    if (acceSlot == 3)
        return -1;
    return m_acceRefineActivedSlot[acceSlot];
}

int CPythonPlayer::FineMoveAcceItemSlot()
{
    for (int i = 0; i < m_ItemAcceInstanceVector.size(); ++i)
    {
        if (!m_ItemAcceInstanceVector[i].vnum)
            return i;
    }
    return 3;
}

#ifdef ENABLE_MOVE_COSTUME_ATTR
void CPythonPlayer::SetItemCombinationWindowActivedItemSlot(int32_t index, int32_t value)
{
    m_itemCombinationActivedItemSlot[index] = value;
}

int32_t CPythonPlayer::GetConbWindowSlotByAttachedInvenSlot(int32_t value) const
{
    for (int32_t i = 0; i < COMB_WND_SLOT_MAX; ++i)
    {
        if (m_itemCombinationActivedItemSlot[i] == value)
            return i;
    }
    return 4;
}

int32_t CPythonPlayer::GetInvenSlotAttachedToConbWindowSlot(int32_t index) const
{
    if (index < 4)
        return m_itemCombinationActivedItemSlot[index];

    return -1;
}
#endif

void CPythonPlayer::SetChangeLookItemInvenSlot(int32_t index, const TItemPos &pos)
{
    m_changeLookActivedSlot[index] = pos;
}

const TItemPos &CPythonPlayer::GetChangeLookItemInvenSlot(int32_t index) const
{
    return m_changeLookActivedSlot[index];
}

void CPythonPlayer::SetChangeLookItemData(uint32_t dwSlotIndex, const ClientItemData &rItemInstance)
{
    if (dwSlotIndex >= m_ItemChangeLookInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::SetChangeLookItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }

    if (dwSlotIndex != CHANGELOOK_SLOT_MAX)
    {
        PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSlot", m_changeLookActivedSlot[dwSlotIndex],
                              (int)UI::HILIGHTSLOT_CHANGE_LOOK);
    }

    m_ItemChangeLookInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonPlayer::DelChangeLookItemData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemChangeLookInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::DelChangeLookItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return;
    }
    ClientItemData &rInstance = m_ItemChangeLookInstanceVector[dwSlotIndex];

    CItemData *pItemData = CItemManager::instance().GetProto(rInstance.vnum);
    if (pItemData)
    {
        if (dwSlotIndex != CHANGELOOK_SLOT_MAX)
        {
            PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", m_changeLookActivedSlot[dwSlotIndex],
                                  (int)UI::HILIGHTSLOT_CHANGE_LOOK);
            m_changeLookActivedSlot[dwSlotIndex].window_type = INVENTORY;
            m_changeLookActivedSlot[dwSlotIndex].cell = INVENTORY_MAX_NUM;
        }
        ZeroMemory(&rInstance, sizeof(rInstance));
    }
}

std::optional<ClientItemData> CPythonPlayer::GetChangeLookData(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemChangeLookInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::GetChangeLookItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return std::nullopt;
    }

    return m_ItemChangeLookInstanceVector[dwSlotIndex];
}

std::optional<ClientItemData *> CPythonPlayer::GetChangeLookItemDataPtr(uint32_t dwSlotIndex)
{
    if (dwSlotIndex >= m_ItemChangeLookInstanceVector.size())
    {
        SPDLOG_ERROR("CPythonPlayer::GetChangeLookItemDataPtr(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
        return std::nullopt;
    }

    return &m_ItemChangeLookInstanceVector[dwSlotIndex];
}

int32_t CPythonPlayer::CanAttachMetin(uint32_t vnum, TItemPos targetCell) const
{
    if (targetCell.window_type == EQUIPMENT)
    {
        return ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT;
    }

    if (targetCell.window_type == SWITCHBOT)
    {
        return ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT;
    }

    CItemData *pMetinItemData = CItemManager::Instance().GetProto(vnum);
    if (!pMetinItemData)
    {
        return ATTACH_METIN_CANT;
    }

    uint32_t dwTargetItemIndex = GetItemIndex(targetCell);
    auto targetItemData = GetItemData(targetCell);
    if (targetItemData->nSealDate == -1 || targetItemData->nSealDate > CPythonApplication::AppInst().
        GetServerTimeStamp())
        return ATTACH_METIN_SEALED;

    CItemData *targetItemProto = CItemManager::Instance().GetProto(dwTargetItemIndex);
    if (!targetItemProto)
    {
        return ATTACH_METIN_CANT;
    }

    uint32_t dwMetinWearFlags = pMetinItemData->GetWearFlags();
    uint32_t dwTargetWearFlags = targetItemProto->GetWearFlags();
    if (0 == (dwMetinWearFlags & dwTargetWearFlags))
    {
        return ATTACH_METIN_NOT_MATCHABLE_ITEM;
    }
    if (ITEM_ROD == targetItemProto->GetType())
    {
        return ATTACH_METIN_CANT;
    }

    bool bNotExistGoldSocket = FALSE;

    int iSubType = pMetinItemData->GetSubType();
    for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
    {
        uint32_t dwSocketType = CPythonPlayer::Instance().GetItemMetinSocket(targetCell, i);
        if (METIN_NORMAL == iSubType)
        {
            if (CPythonPlayer::METIN_SOCKET_TYPE_SILVER == dwSocketType ||
                CPythonPlayer::METIN_SOCKET_TYPE_GOLD == dwSocketType)
            {
                return ATTACH_METIN_OK;
            }
        }
        else if (METIN_GOLD == iSubType)
        {
            if (CPythonPlayer::METIN_SOCKET_TYPE_GOLD == dwSocketType)
            {
                return ATTACH_METIN_OK;
            }
            else if (CPythonPlayer::METIN_SOCKET_TYPE_SILVER == dwSocketType)
            {
                bNotExistGoldSocket = TRUE;
            }
        }
    }

    if (bNotExistGoldSocket)
    {
        return ATTACH_METIN_NOT_EXIST_GOLD_SOCKET;
    }

    return ATTACH_METIN_NO_MATCHABLE_SOCKET;
}

int32_t CPythonPlayer::CanRefine(uint32_t vnum, TItemPos targetCell)
{
    if (targetCell.window_type == EQUIPMENT)
    {
        return REFINE_CANT_REFINE_METIN_TO_EQUIPMENT;
    }
    if (targetCell.window_type == SWITCHBOT)
    {
        return REFINE_CANT_REFINE_METIN_TO_EQUIPMENT;
    }
    // Scroll
    const auto pScrollItemData = CItemManager::Instance().GetProto(vnum);
    if (!pScrollItemData)
    {
        return REFINE_CANT;
    }

    const int iScrollType = pScrollItemData->GetType();
    const int iScrollSubType = pScrollItemData->GetSubType();

    if (iScrollType != ITEM_USE)
    {
        return REFINE_CANT;
    }

    if (iScrollSubType != USE_TUNING)
    {
        return REFINE_CANT;
    }

    // Target Item
    const int iTargetItemIndex = GetItemIndex(targetCell);
    const auto pTargetItemData = CItemManager::Instance().GetProto(iTargetItemIndex);

    if (!pTargetItemData)
    {
        return REFINE_CANT;
    }

    const int iTargetType = pTargetItemData->GetType();

    if (ITEM_ROD == iTargetType)
    {
        return REFINE_CANT_REFINE_ROD;
    }

    if (pTargetItemData->HasNextGrade())
    {
        return REFINE_OK;
    }

    return REFINE_NOT_NEXT_GRADE_ITEM;
}

int32_t CPythonPlayer::CanSealItem(uint32_t vnum, TItemPos targetCell) const
{
    if (vnum == 50263 || vnum == 50264)
    {
        CItemData *pItemData = CItemManager::Instance().GetProto(vnum);
        if (!pItemData)
        {
            return 0;
        }

        if (pItemData->GetType() == ITEM_USE)
        {
            uint32_t dwTargetIndex = CPythonPlayer::Instance().GetItemIndex(targetCell);
            CItemData *pTargetItemData = CItemManager::Instance().GetProto(dwTargetIndex);
            if (!pTargetItemData)
            {
                return 0;
            }

            if (pTargetItemData->IsSealAbleItem())
            {
                auto pPlayerItem = GetItemData(targetCell);
                if (!pPlayerItem)
                {
                    return 0;
                }

                if (vnum == 50263)
                {
                    if (pPlayerItem->nSealDate == 0)
                    {
                        return 1;
                    }
                }
                else
                {
                    if (vnum == 50264 && (pPlayerItem->nSealDate > 0 || pPlayerItem->nSealDate < 0))
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

int32_t CPythonPlayer::CanUnlockTreasure(uint32_t vnum, TItemPos targetCell) const
{
    // Key
    CItemData *pKeyItemData = CItemManager::Instance().GetProto(vnum);
    if (!pKeyItemData)
    {
        return 0;
    }
    int iKeyType = pKeyItemData->GetType();
    if (iKeyType != ITEM_TREASURE_KEY)
    {
        return 0;
    }

    // Target Item
    int iTargetItemIndex = GetItemIndex(targetCell);

    CItemData *pTargetItemData = CItemManager::Instance().GetProto(iTargetItemIndex);
    if (!pTargetItemData)
    {
        return 0;
    }
    int iTargetType = pTargetItemData->GetType();
    if (iTargetType != ITEM_TREASURE_BOX)
    {
        return 0;
    }

    return 1;
}

void CPythonPlayer::ShowPopUp(std::string message)
{
    if (m_ppyGameWindow && !py::isinstance<py::none>(m_ppyGameWindow))
        m_ppyGameWindow.attr("ShowPopUp")(py::bytes(message));
}

void CPythonPlayer::SetHuntingMission(uint32_t id,
                                      const TPlayerHuntingMission &mission) {
  m_huntingMissions.insert_or_assign(id, mission);
}

bool CPythonPlayer::CanAttachItemToCombSlotIndex(int32_t index, CItemData *itemData)
{
    if (index >= COMB_WND_SLOT_MAX)
        return false;

    if (!itemData)
        return false;

    const auto itemTable = itemData->GetTable();
    if (!itemTable)
        return false;

#ifdef ENABLE_MOVE_COSTUME_ATTR_BONUS_LIMIT
    if (itemTable->bAlterToMagicItemPct != 100)
        return false;
#endif

    if (itemData->GetType() != ITEM_COSTUME)
        return false;

    if(itemData->GetTable()->bAlterToMagicItemPct != 100) {
        return false;
    }

    switch (index)
    {
    case COMB_WND_SLOT_BASE: {
        TItemPos otherItemCell(INVENTORY, GetInvenSlotAttachedToConbWindowSlot(COMB_WND_SLOT_MATERIAL));
        if (!otherItemCell.IsValidItemPosition())
            return true;

        auto otherItemData = CItemManager::instance().GetProto(GetItemIndex(otherItemCell));
        if (!otherItemData)
            return false;

         if (itemData->GetSubType() == otherItemData->GetSubType())
            return true;
    }
    break;
    case COMB_WND_SLOT_MATERIAL: {
        TItemPos otherItemCell(INVENTORY, GetInvenSlotAttachedToConbWindowSlot(COMB_WND_SLOT_BASE));
        if (!otherItemCell.IsValidItemPosition())
            return true;

        auto otherItemData = CItemManager::instance().GetProto(GetItemIndex(otherItemCell));
        if (!otherItemData)
            return false;

        if (itemData->GetSubType() == otherItemData->GetSubType())
            return true;
    }
    break;
    default:
        break;
    }

    return false;
}

void CPythonPlayer::ClearHuntingMissions()
{
	m_huntingMissions.clear();
}

std::optional<TPlayerHuntingMission> CPythonPlayer::GetHuntingMission(uint32_t id)
{
    if (auto it = m_huntingMissions.find(id); it != m_huntingMissions.end())
        return it->second;
    return std::nullopt;
}

const CPythonPlayer::HuntingMissionMap &CPythonPlayer::GetHuntingMissions() {
  return m_huntingMissions;
}
