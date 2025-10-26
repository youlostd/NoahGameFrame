#include <game/DbPackets.hpp>
#include "item.h"
#include "char.h"
#include "buff_on_attributes.h"
#include <algorithm>

CBuffOnAttributes::CBuffOnAttributes(CHARACTER *pOwner, uint8_t point_type,
                                     std::vector<uint8_t> *p_vec_buff_wear_targets)
    : m_pBuffOwner(pOwner), m_bPointType(point_type), m_p_vec_buff_wear_targets(p_vec_buff_wear_targets)
{
    Initialize();
}

void CBuffOnAttributes::Initialize()
{
    m_bBuffValue = 0;
    m_map_additional_attrs.clear();
}

void CBuffOnAttributes::RemoveBuffFromItem(CItem *pItem)
{
    if (0 == m_bBuffValue)
        return;
    if (nullptr != pItem)
    {
        if (pItem->GetCell() < INVENTORY_MAX_NUM)
            return;
        std::vector<uint8_t>::iterator it = find(m_p_vec_buff_wear_targets->begin(), m_p_vec_buff_wear_targets->end(),
                                                 pItem->GetCell() - INVENTORY_MAX_NUM);
        if (m_p_vec_buff_wear_targets->end() == it)
            return;

        int m = pItem->GetAttributeCount();
        for (int j = 0; j < m; j++)
        {
            TPlayerItemAttribute attr = pItem->GetAttribute(j);
            TMapAttr::iterator it = m_map_additional_attrs.find(attr.bType);
            // m_map_additional_attrs에서 해당 attribute type에 대한 값을 제거하고,
            // 변경된 값의 (m_bBuffValue)%만큼의 버프 효과 감소
            if (it != m_map_additional_attrs.end())
            {
                ApplyValue &sum_of_attr_value = it->second;
                ApplyValue old_value = sum_of_attr_value * m_bBuffValue / 100;
                ApplyValue new_value = (sum_of_attr_value - attr.sValue) * m_bBuffValue / 100;
                m_pBuffOwner->ApplyPoint(attr.bType, new_value - old_value);
#ifdef __FAKE_PC__
                m_pBuffOwner->FakePC_Owner_ApplyPoint(attr.bType, new_value - old_value);
#endif
                sum_of_attr_value -= attr.sValue;
            }
            else
            {
                SPDLOG_ERROR("Buff ERROR(type {0}). This item({1}) attr_type({2}) was not in buff pool", m_bPointType,
                             pItem->GetVnum(), attr.bType);
                return;
            }
        }
    }
}

void CBuffOnAttributes::AddBuffFromItem(CItem *pItem)
{
    if (0 == m_bBuffValue)
        return;
    if (nullptr != pItem)
    {
        if (pItem->GetCell() < INVENTORY_MAX_NUM)
            return;
        std::vector<uint8_t>::iterator it = find(m_p_vec_buff_wear_targets->begin(), m_p_vec_buff_wear_targets->end(),
                                                 pItem->GetCell() - INVENTORY_MAX_NUM);
        if (m_p_vec_buff_wear_targets->end() == it)
            return;

        int m = pItem->GetAttributeCount();
        for (int j = 0; j < m; j++)
        {
            auto attr = pItem->GetAttribute(j);
            auto it = m_map_additional_attrs.find(attr.bType);

            // m_map_additional_attrs에서 해당 attribute type에 대한 값이 없다면 추가.
            // 추가된 값의 (m_bBuffValue)%만큼의 버프 효과 추가
            if (it == m_map_additional_attrs.end())
            {
                m_pBuffOwner->ApplyPoint(attr.bType, attr.sValue * m_bBuffValue / 100);
#ifdef __FAKE_PC__
                m_pBuffOwner->FakePC_Owner_ApplyPoint(attr.bType, attr.sValue * m_bBuffValue / 100);
#endif
                m_map_additional_attrs.insert(TMapAttr::value_type(attr.bType, attr.sValue));
            }
                // m_map_additional_attrs에서 해당 attribute type에 대한 값이 있다면, 그 값을 증가시키고,
                // 변경된 값의 (m_bBuffValue)%만큼의 버프 효과 추가
            else
            {
                ApplyValue &sum_of_attr_value = it->second;
                int old_value = sum_of_attr_value * m_bBuffValue / 100;
                int new_value = (sum_of_attr_value + attr.sValue) * m_bBuffValue / 100;
                m_pBuffOwner->ApplyPoint(attr.bType, new_value - old_value);
#ifdef __FAKE_PC__
                m_pBuffOwner->FakePC_Owner_ApplyPoint(attr.bType, new_value - old_value);
#endif
                sum_of_attr_value += attr.sValue;
            }
        }
    }
}

void CBuffOnAttributes::ChangeBuffValue(uint8_t bNewValue)
{
    if (0 == m_bBuffValue)
        On(bNewValue);
    else if (0 == bNewValue)
        Off();
    else
    {
        // 기존에, m_map_additional_attrs의 값의 (m_bBuffValue)%만큼이 버프로 들어가 있었으므로,
        // (bNewValue)%만큼으로 값을 변경함.
        for (TMapAttr::iterator it = m_map_additional_attrs.begin(); it != m_map_additional_attrs.end(); ++it)
        {
            ApplyValue &sum_of_attr_value = it->second;
            //int old_value = sum_of_attr_value * m_bBuffValue / 100;
            //int new_value = sum_of_attr_value * bNewValue / 100;

            m_pBuffOwner->ApplyPoint(it->first, -sum_of_attr_value * m_bBuffValue / 100);
#ifdef __FAKE_PC__
            m_pBuffOwner->FakePC_Owner_ApplyPoint(it->first, -sum_of_attr_value * m_bBuffValue / 100);
#endif
        }
        m_bBuffValue = bNewValue;
    }
}

bool CBuffOnAttributes::On(uint8_t bValue)
{
    if (0 != m_bBuffValue || 0 == bValue)
        return false;

    int n = m_p_vec_buff_wear_targets->size();
    m_map_additional_attrs.clear();
    for (int i = 0; i < n; i++)
    {
        CItem *pItem = m_pBuffOwner->GetWear(m_p_vec_buff_wear_targets->at(i));
        if (nullptr != pItem)
        {
            int m = pItem->GetAttributeCount();
            for (int j = 0; j < m; j++)
            {
                TPlayerItemAttribute attr = pItem->GetAttribute(j);
                TMapAttr::iterator it = m_map_additional_attrs.find(attr.bType);
                if (it != m_map_additional_attrs.end()) { it->second += attr.sValue; }
                else { m_map_additional_attrs.insert(TMapAttr::value_type(attr.bType, attr.sValue)); }
            }
        }
    }

    for (TMapAttr::iterator it = m_map_additional_attrs.begin(); it != m_map_additional_attrs.end(); ++it)
    {
        m_pBuffOwner->ApplyPoint(it->first, static_cast<ApplyValue>(it->second * bValue / 100));
#ifdef __FAKE_PC__
        m_pBuffOwner->FakePC_Owner_ApplyPoint(it->first, static_cast<ApplyValue>(it->second * bValue / 100));
#endif
    }

    m_bBuffValue = bValue;

    return true;
}

void CBuffOnAttributes::Off()
{
    if (0 == m_bBuffValue)
        return;

    for (TMapAttr::iterator it = m_map_additional_attrs.begin(); it != m_map_additional_attrs.end(); ++it)
    {
        m_pBuffOwner->ApplyPoint(it->first, static_cast<ApplyValue>(-it->second * m_bBuffValue / 100));
#ifdef __FAKE_PC__
        m_pBuffOwner->FakePC_Owner_ApplyPoint(it->first, static_cast<ApplyValue>(-it->second * m_bBuffValue / 100));
#endif
    }
    Initialize();
}
