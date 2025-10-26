#ifndef METIN2_SERVER_GAME_REFINE_H
#define METIN2_SERVER_GAME_REFINE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "constants.h"
#include <base/Singleton.hpp>

enum
{
    BLACKSMITH_MOB = 20016,
    // 확률 개량
    ALCHEMIST_MOB = 20001,
    // 100% 개량 성공 

    BLACKSMITH_WEAPON_MOB = 20044,
    BLACKSMITH_ARMOR_MOB = 20045,
    BLACKSMITH_ACCESSORY_MOB = 20046,

    DEVILTOWER_BLACKSMITH_WEAPON_MOB = 20074,
    DEVILTOWER_BLACKSMITH_ARMOR_MOB = 20075,
    DEVILTOWER_BLACKSMITH_ACCESSORY_MOB = 20076,

    BLACKSMITH2_MOB = 20091,
};

class CRefineManager : public singleton<CRefineManager>
{
    typedef std::map<uint32_t, RefineProto> TRefineRecipeMap;
public:
    CRefineManager();
    virtual ~CRefineManager();

    bool Initialize();
    const RefineProto *GetRefineRecipe(uint32_t id);

private:
    TRefineRecipeMap m_map_RefineRecipe;
};

#endif /* METIN2_SERVER_GAME_REFINE_H */
