#ifndef METIN2_SERVER_GAME_BUFF_ON_ATTRIBUTES_H
#define METIN2_SERVER_GAME_BUFF_ON_ATTRIBUTES_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CHARACTER;
class CItem;

class CBuffOnAttributes
{
public:
    CBuffOnAttributes(CHARACTER *pOwner, uint8_t m_point_type, std::vector<uint8_t> *vec_buff_targets);

    // 장착 중 이면서, m_p_vec_buff_wear_targets에 해당하는 아이템인 경우, 해당 아이템으로 인해 붙은 효과를 제거.
    void RemoveBuffFromItem(CItem *pItem);
    // m_p_vec_buff_wear_targets에 해당하는 아이템인 경우, 해당 아이템의 attribute에 대한 효과 추가.
    void AddBuffFromItem(CItem *pItem);
    // m_bBuffValue를 바꾸고, 버프의 값도 바꿈.
    void ChangeBuffValue(uint8_t bNewValue);

    // m_p_vec_buff_wear_targets에 해당하는 모든 아이템의 attribute를 type별로 합산하고,
    // 그 attribute들의 (m_bBuffValue)% 만큼을 버프로 줌.
    bool On(uint8_t bValue);
    // 버프 제거 후, 초기화
    void Off();

    void Initialize();
private:
    CHARACTER *m_pBuffOwner;
    uint8_t m_bPointType;
    uint8_t m_bBuffValue;
    std::vector<uint8_t> *m_p_vec_buff_wear_targets;

    using TMapAttr = std::map<uint8_t, ApplyValue>;
    TMapAttr m_map_additional_attrs;
};

#endif /* METIN2_SERVER_GAME_BUFF_ON_ATTRIBUTES_H */
