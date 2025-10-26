#include "char.h"
#include "item.h"
#include "desc.h"
#include "DragonSoul.h"
#include "log.h"
#include <game/AffectConstants.hpp>

// 용혼석 초기화
// 용혼석 on/off는 Affect로 저장되기 때문에,
// 용혼석 Affect가 있다면 덱에 있는 용혼석을 activate해야한다.
// 또한 용혼석 사용 자격은 QuestFlag로 저장해 놓았기 때문에,
// 퀘스트 Flag에서 용혼석 사용 자격을 읽어온다.

// 캐릭터의 affect, quest가 load 되기 전에 DragonSoul_Initialize를 호출하면 안된다.
// affect가 가장 마지막에 로드되어 LoadAffect에서 호출함.
void CHARACTER::DragonSoul_Initialize()
{
    for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
    {
        CItem *pItem = GetWear(i);
        if (nullptr != pItem)
            pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 0);
    }

    if (FindAffect(AFFECT_DRAGON_SOUL_DECK_0)) { DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_0); }
    else if (FindAffect(AFFECT_DRAGON_SOUL_DECK_1)) { DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_1); }
}

int CHARACTER::DragonSoul_GetActiveDeck() const { return m_pointsInstant.iDragonSoulActiveDeck; }

bool CHARACTER::DragonSoul_IsDeckActivated() const { return m_pointsInstant.iDragonSoulActiveDeck >= 0; }

bool CHARACTER::DragonSoul_IsQualified() const
{
#ifndef NON_CHECK_DS_QUALIFIED
    return FindAffect(AFFECT_DRAGON_SOUL_QUALIFIED) != nullptr;
#else
    return GetLevel() >= gConfig.dragonSoulMinLevel;
#endif
}

void CHARACTER::DragonSoul_GiveQualification()
{
    if (nullptr == FindAffect(AFFECT_DRAGON_SOUL_QUALIFIED))
    {
        LogManager::instance().CharLog(this, 0, "DS_QUALIFIED", "");
    }
    AddAffect(AFFECT_DRAGON_SOUL_QUALIFIED, APPLY_NONE, 0, INFINITE_AFFECT_DURATION, 0, false, false);
    // SetQuestFlag("dragon_soul.is_qualified", 1);
    //// 자격있다면 POINT_DRAGON_SOUL_IS_QUALIFIED는 무조건 1
    // PointChange(POINT_DRAGON_SOUL_IS_QUALIFIED, 1 - GetPoint(POINT_DRAGON_SOUL_IS_QUALIFIED));
}

bool CHARACTER::DragonSoul_ActivateDeck(int deck_idx)
{
    if (deck_idx < DRAGON_SOUL_DECK_0 || deck_idx >= DRAGON_SOUL_DECK_MAX_NUM) { return false; }

    if (DragonSoul_GetActiveDeck() == deck_idx)
        return true;

    DragonSoul_DeactivateAll();

    if (!DragonSoul_IsQualified())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "용혼석 상자가 활성화되지 않았습니다.");
        return false;
    }

    AddAffect(AFFECT_DRAGON_SOUL_DECK_0 + deck_idx, APPLY_NONE, 0, INFINITE_AFFECT_DURATION, 0, false);

    if (deck_idx == DRAGON_SOUL_DECK_0)
        SpecificEffectPacket("d:\\ymir work\\effect\\etc\\dragonsoul\\dragonsoul_earth.mse");
    else
        SpecificEffectPacket("d:\\ymir work\\effect\\etc\\dragonsoul\\dragonsoul_sky.mse");

    m_pointsInstant.iDragonSoulActiveDeck = deck_idx;

    for (int i = DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * deck_idx;
         i < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * (deck_idx + 1); i++)
    {
        CItem *pItem = GetWear(i);
        if (nullptr != pItem)
            DSManager::instance().ActivateDragonSoul(pItem);
    }

#ifdef ENABLE_DS_SET
    DragonSoul_HandleSetBonus();
#endif
    return true;
}

void CHARACTER::DragonSoul_DeactivateAll()
{
    for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_RESERVED_SLOT_END; i++)
    {
        DSManager::instance().DeactivateDragonSoul(GetWear(i), true);
    }

#ifdef ENABLE_DS_SET
    DragonSoul_HandleSetBonus();
#endif

    m_pointsInstant.iDragonSoulActiveDeck = -1;
    RemoveAffect(AFFECT_DRAGON_SOUL_DECK_0);
    RemoveAffect(AFFECT_DRAGON_SOUL_DECK_1);

#ifdef ENABLE_DS_SET
    RemoveAffect(AFFECT_DS_SET);
#endif
}

#ifdef ENABLE_DS_SET
void CHARACTER::DragonSoul_HandleSetBonus()
{
    bool bAdd = true;
    uint8_t iSetGrade;;
    if (!DSManager::instance().GetDSSetGrade(this, iSetGrade))
    {
        auto pkAffect = FindAffect(AFFECT_DS_SET);
        if (!pkAffect) { return; }

        iSetGrade = static_cast<uint8_t>(pkAffect->pointValue);
        bAdd = false;
    }
    else { AddAffect(AFFECT_DS_SET, APPLY_NONE, iSetGrade, INFINITE_AFFECT_DURATION, 0, true); }

    const uint8_t iDeckIdx = DragonSoul_GetActiveDeck();
    const uint8_t iStartSlotIndex = DRAGON_SOUL_EQUIP_SLOT_START + (iDeckIdx * DS_SLOT_MAX);
    const uint8_t iEndSlotIndex = iStartSlotIndex + DS_SLOT_MAX;

    for (uint8_t iSlotIndex = iStartSlotIndex; iSlotIndex < iEndSlotIndex; ++iSlotIndex)
    {
        const auto pkItem = GetWear(iSlotIndex);
        if (!pkItem) { return; }

        for (uint8_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
        {
            if (pkItem->GetAttributeType(i))
            {
                const TPlayerItemAttribute &ia = pkItem->GetAttribute(i);
                ApplyValue iSetValue = DSManager::instance().GetDSSetValue(i, ia.bType, pkItem->GetVnum(), iSetGrade);

                ApplyPoint(ia.bType, bAdd ? iSetValue : -iSetValue);
            }
        }
    }
}
#endif

void CHARACTER::DragonSoul_CleanUp()
{
    for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
    {
        DSManager::instance().DeactivateDragonSoul(GetWear(i), true);
    }
}

bool CHARACTER::DragonSoul_RefineWindow_Open(bool v)
{
    if (m_pointsInstant.m_pDragonSoulRefineWindowOpener == false)
    {
        m_pointsInstant.m_pDragonSoulRefineWindowOpener = v;
    }

    TPacketGCDragonSoulRefine PDS;
    PDS.bSubType = DS_SUB_HEADER_OPEN;
    DESC *d = GetDesc();

    if (nullptr == d)
    {
        SPDLOG_ERROR("User(%s)'s DESC is NULL POINT.", GetName());
        return false;
    }

    d->Send(HEADER_GC_DRAGON_SOUL_REFINE, PDS);
    return true;
}

bool CHARACTER::DragonSoul_RefineWindow_Close()
{
    m_pointsInstant.m_pDragonSoulRefineWindowOpener = false;
    return true;
}

bool CHARACTER::DragonSoul_RefineWindow_CanRefine() { return true; }
