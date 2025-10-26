#ifndef METIN2_CLIENT_RUN_CHARACTEREFFECTREGISTRY_HPP
#define METIN2_CLIENT_RUN_CHARACTEREFFECTREGISTRY_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <cstdint>

struct CharacterEffectInfo
{
    // The effect data id.
    uint32_t id;

    // Name of the bone to attach to.
    std::string attachingBoneName;
};

class CharacterEffectRegistry
{
public:
    void RegisterEffect(uint32_t effect,
                        const std::string &attachingBone,
                        const char *filename);

    void RegisterAffect(uint32_t affect,
                        const std::string &attachingBone,
                        const char *filename);

    const CharacterEffectInfo *FindEffect(uint32_t effect) const;
    const CharacterEffectInfo *FindAffect(uint32_t affect) const;

private:
    std::unordered_map<uint32_t, CharacterEffectInfo> m_effects;
    std::unordered_map<uint32_t, CharacterEffectInfo> m_affects;
};

extern CharacterEffectRegistry *gCharacterEffectRegistry;

#endif
