#ifndef METIN2_SERVER_GAME_ITEM_ADDON_H
#define METIN2_SERVER_GAME_ITEM_ADDON_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CItemAddonManager : public singleton<CItemAddonManager>
{
public:
    CItemAddonManager();
    virtual ~CItemAddonManager();

    void ApplyAddonTo(int iAddonType, CItem *pItem);
};

#endif /* METIN2_SERVER_GAME_ITEM_ADDON_H */
