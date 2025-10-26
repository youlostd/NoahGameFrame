#ifndef METIN2_SERVER_GAME_HORSENAME_MANAGER_H
#define METIN2_SERVER_GAME_HORSENAME_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>
class CHARACTER;

class CHorseNameManager : public singleton<CHorseNameManager>
{
private :
    std::map<uint32_t, std::string> m_mapHorseNames;

    void BroadcastHorseName(uint32_t dwPlayerID, const char *szHorseName);

public :
    CHorseNameManager();

    const char *GetHorseName(uint32_t dwPlayerID);

    void UpdateHorseName(uint32_t dwPlayerID, const char *szHorseName, bool broadcast = false);

    void Validate(CHARACTER *pChar);
};

#endif /* METIN2_SERVER_GAME_HORSENAME_MANAGER_H */
