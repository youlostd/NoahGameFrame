#include "stdafx.h"
#include "CharacterEffectRegistry.hpp"
#include "../EffectLib/EffectManager.h"

CharacterEffectRegistry *gCharacterEffectRegistry = nullptr;

void CharacterEffectRegistry::RegisterEffect(uint32_t effect,
                                             const std::string &attachingBone,
                                             const char *filename)
{
    CharacterEffectInfo info;
    if (!CEffectManager::Instance().RegisterEffect(filename, &info.id))
    {
        SPDLOG_ERROR("Failed to register effect {0}: {1}",
                      effect, filename);
        return;
    }

    info.attachingBoneName = attachingBone;
    m_effects.emplace(effect, std::move(info));
}

void CharacterEffectRegistry::RegisterAffect(uint32_t affect,
                                             const std::string &attachingBone,
                                             const char *filename)
{
    CharacterEffectInfo info;
    if (!CEffectManager::Instance().RegisterEffect(filename, &info.id))
    {
        SPDLOG_ERROR("Failed to register effect {0}: {1}",
                      affect, filename);
        return;
    }

    info.attachingBoneName = attachingBone;
    m_affects.emplace(affect, std::move(info));
}

const CharacterEffectInfo *CharacterEffectRegistry::FindEffect(uint32_t effect) const
{
    const auto it = m_effects.find(effect);
    if (it != m_effects.end())
        return &it->second;

    return nullptr;
}

const CharacterEffectInfo *CharacterEffectRegistry::FindAffect(uint32_t affect) const
{
    const auto it = m_affects.find(affect);
    if (it != m_affects.end())
        return &it->second;

    return nullptr;
}
