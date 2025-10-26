#include "char.h"
#include "DbCacheSocket.hpp"

#include <game/GamePacket.hpp>

#include "DragonSoul.h"

#include "item.h"
#include "desc.h"

void CHARACTER::UpdateSwitchbotSlotAttribute(uint32_t slotIndex, uint32_t tabIndex, uint32_t attrIndex, TItemApply attr)
{
    if (slotIndex >= SWITCHBOT_SLOT_COUNT || tabIndex >= SWITCHBOT_ALT_COUNT || attrIndex > ITEM_ATTRIBUTE_MAX_NUM)
        return;

    if (const auto it = m_switchbotData.find(slotIndex); it == m_switchbotData.end())
    {
        SwitchBotSlotData switchbotData = {};
        switchbotData.slot = slotIndex;

        m_switchbotData.emplace(slotIndex, switchbotData);
    }

    m_switchbotData[slotIndex].attr[tabIndex][attrIndex] = attr;

    GcSwitchbotSlotDataPacket p{};
    std::memcpy(&p.data, &m_switchbotData[slotIndex], sizeof(m_switchbotData[slotIndex]));
    GetDesc()->Send(HEADER_GC_SWITCHBOT_DATA, p);

    SaveSwitchbotData();
}

void CHARACTER::SaveSwitchbotData()
{
    TPacketGDAddSwitchbotData p;
    for (const auto &data : m_switchbotData)
    {
        p.pid = GetPlayerID();
        p.elem = data.second;
        db_clientdesc->DBPacket(HEADER_GD_ADD_SWITCHBOT_DATA, 0, &p, sizeof(p));
    }
}

void CHARACTER::LoadSwitchbotData(uint32_t count, const SwitchBotSlotData *data)
{
    SPDLOG_DEBUG("LOAD_SWITCHBOT_DATA: {0} count {1}", GetName(), count);

    for (uint32_t i = 0; i < count; ++i, ++data)
    {
        if (data->slot >= SWITCHBOT_SLOT_COUNT)
        {
            SPDLOG_ERROR("invalid switchbot data {} slot {}",
                         GetName(), data->slot);
            continue;
        }

        m_switchbotData.emplace(data->slot, *data);
    }

    GcSwitchbotSlotDataPacket p{};
    for (auto switchbotData : m_switchbotData)
    {
        auto item = GetSwitchbotItem(switchbotData.first);
        if (item) { if (switchbotData.second.status == SWITCHBOT_STATUS_ACTIVE) { item->StartSelfSwitchEvent(); } }
        else
        {
            if (switchbotData.second.status == SWITCHBOT_STATUS_DONE)
            {
                switchbotData.second.status = SWITCHBOT_STATUS_INACTIVE;
            }
        }

        // Packet
        std::memcpy(&p.data, &switchbotData.second, sizeof(switchbotData.second));
        GetDesc()->Send(HEADER_GC_SWITCHBOT_DATA, p);
    }
}

void CHARACTER::ActivateSwitchbotSlot(uint32_t slotIndex)
{
    if (slotIndex >= SWITCHBOT_SLOT_COUNT)
        return;

    if (const auto it = m_switchbotData.find(slotIndex); it == m_switchbotData.end())
    {
        SwitchBotSlotData switchbotData = {};
        switchbotData.slot = slotIndex;

        m_switchbotData.emplace(slotIndex, switchbotData);
    }

    if (m_switchbotData[slotIndex].status != SWITCHBOT_STATUS_INACTIVE && m_switchbotData[slotIndex].status !=
        SWITCHBOT_STATUS_DONE)
        return;

    auto* item = GetSwitchbotItem(slotIndex);
    if (item)
        item->StartSelfSwitchEvent();

    m_switchbotData[slotIndex].status = SWITCHBOT_STATUS_ACTIVE;

    GcSwitchbotSlotDataPacket p{};
    std::memcpy(&p.data, &m_switchbotData[slotIndex], sizeof(m_switchbotData[slotIndex]));
    GetDesc()->Send(HEADER_GC_SWITCHBOT_DATA, p);

    SaveSwitchbotData();
}

std::optional<SwitchBotSlotData> CHARACTER::GetSwitchbotSlotData(uint32_t slotIndex)
{
    if (const auto it = m_switchbotData.find(slotIndex); it == m_switchbotData.end())
        return std::nullopt;

    return m_switchbotData[slotIndex];
}

void CHARACTER::DeactivateSwitchbotSlot(uint32_t slotIndex)
{
    if (slotIndex >= SWITCHBOT_SLOT_COUNT)
        return;

    if (const auto it = m_switchbotData.find(slotIndex); it == m_switchbotData.end())
    {
        SwitchBotSlotData switchbotData = {};
        switchbotData.slot = slotIndex;

        m_switchbotData.emplace(slotIndex, switchbotData);
    }

    if (m_switchbotData[slotIndex].status != SWITCHBOT_STATUS_ACTIVE)
        return;

    m_switchbotData[slotIndex].status = SWITCHBOT_STATUS_INACTIVE;

    auto* item = GetSwitchbotItem(slotIndex);
    if (item)
        item->StopSelfSwitchEvent();

    GcSwitchbotSlotDataPacket p{};
    std::memcpy(&p.data, &m_switchbotData[slotIndex], sizeof(m_switchbotData[slotIndex]));
    GetDesc()->Send(HEADER_GC_SWITCHBOT_DATA, p);

    SaveSwitchbotData();
}

void CHARACTER::ClearSwitchbotData() { m_switchbotData.clear(); }

void CHARACTER::OnSwitchbotDone(uint16_t slotIndex)
{
    SendI18nSystemWhisperPacket(this, "Efsun Atýlan Slot %d baþarýyla geldi.", slotIndex + 1);

    if (const auto it = m_switchbotData.find(slotIndex); it == m_switchbotData.end())
    {
        SwitchBotSlotData switchbotData = {};
        switchbotData.slot = slotIndex;

        m_switchbotData.emplace(slotIndex, switchbotData);
    }

    if (m_switchbotData[slotIndex].status != SWITCHBOT_STATUS_ACTIVE)
        return;

    m_switchbotData[slotIndex].status = SWITCHBOT_STATUS_DONE;

    auto* item = GetSwitchbotItem(slotIndex);
    if (item)
        item->StopSelfSwitchEvent();

    GcSwitchbotSlotDataPacket p{};
    std::memcpy(&p.data, &m_switchbotData[slotIndex], sizeof(m_switchbotData[slotIndex]));
    GetDesc()->Send(HEADER_GC_SWITCHBOT_DATA, p);

    SaveSwitchbotData();
}
