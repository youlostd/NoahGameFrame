#include "constants.h"
#include "utils.h"
#include "item.h"
#include "item_addon.h"

extern int minAddonFKS;
extern int maxAddonFKS;
extern int minAddonDSS;
extern int maxAddonDSS;

CItemAddonManager::CItemAddonManager()
{
}

CItemAddonManager::~CItemAddonManager()
{
}

void CItemAddonManager::ApplyAddonTo(int iAddonType, CItem *pItem)
{
    if (!pItem)
    {
        SPDLOG_ERROR("ITEM pointer null");
        return;
    }

    auto result = Random::get<std::normal_distribution<>>(0.0f, 5.0f);

    int iSkillBonus = std::clamp(static_cast<int>(result + 0.5f), -30, 30);
    int iNormalHitBonus = 0;
    if (abs(iSkillBonus) <= 20)
        iNormalHitBonus = static_cast<int32_t>(-2.0f * iSkillBonus + std::abs(
                                                   Random::get(-8.0f, 8.0f) + Random::get(-8.0f, 8.0f)) + Random::get(
                                                   1.0f, 4.0f));
    else
        iNormalHitBonus = static_cast<int32_t>(-2.0f * iSkillBonus + Random::get(1.0f, 5.0f));

    pItem->RemoveAttributeType(APPLY_SKILL_DAMAGE_BONUS);
    pItem->RemoveAttributeType(APPLY_NORMAL_HIT_DAMAGE_BONUS);

    pItem->AddAttribute(APPLY_NORMAL_HIT_DAMAGE_BONUS, iNormalHitBonus);
    pItem->AddAttribute(APPLY_SKILL_DAMAGE_BONUS, iSkillBonus);
}
