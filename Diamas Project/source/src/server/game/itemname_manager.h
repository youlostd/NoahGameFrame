#ifndef METIN2_SERVER_GAME_ITEMNAME_MANAGER_H
#define METIN2_SERVER_GAME_ITEMNAME_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>
class CItem;

class CItemNameManager : public singleton<CItemNameManager>
{
private :
    std::map<uint32_t, std::string> m_mapItemNames;

    void BroadCastItemName(uint32_t dwItemId, const char *szName);

public :
    CItemNameManager();

    const char *GetItemName(uint32_t dwItemId);

    void UpdateItemName(uint32_t dwItemId, const char *szName, bool broadcast = false);

    void Validate(CItem *pChar);
};

#endif /* METIN2_SERVER_GAME_ITEMNAME_MANAGER_H */
