#ifndef METIN2_SERVER_GAME_ARENA_H
#define METIN2_SERVER_GAME_ARENA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <lua.h>
#include "char_manager.h"
#include <base/Singleton.hpp>

enum MEMBER_IDENTITY
{
    MEMBER_NO,
    MEMBER_DUELIST,
    MEMBER_OBSERVER,

    MEMBER_MAX
};

class CArena
{
    friend class CArenaMap;

private :
    uint32_t m_dwPIDA;
    uint32_t m_dwPIDB;

    LPEVENT m_pEvent;
    LPEVENT m_pTimeOutEvent;

    uint32_t m_mapIndex;
    PIXEL_POSITION m_StartPointA;
    PIXEL_POSITION m_StartPointB;
    PIXEL_POSITION m_ObserverPoint;

    uint32_t m_dwSetCount;
    uint32_t m_dwSetPointOfA;
    uint32_t m_dwSetPointOfB;

    std::map<uint32_t, CHARACTER *> m_mapObserver;

protected :
    CArena(uint32_t map, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y);

    bool StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint, int nMinute = 5);

    bool IsEmpty() const { return ((m_dwPIDA == 0) && (m_dwPIDB == 0)); }

    bool IsMember(uint32_t dwPID) const { return ((m_dwPIDA == dwPID) || (m_dwPIDB == dwPID)); }

    bool CheckArea(uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y);
    void Clear();

    bool CanAttack(uint32_t dwPIDA, uint32_t dwPIDB);
    bool OnDead(uint32_t dwPIDA, uint32_t dwPIDB);

    bool IsObserver(uint32_t pid);
    bool IsMyObserver(long ObserverX, long ObserverY);
    bool AddObserver(CHARACTER *pChar);
    bool RegisterObserverPtr(CHARACTER *pChar);

public :
    uint32_t GetPlayerAPID() { return m_dwPIDA; }

    uint32_t GetPlayerBPID() { return m_dwPIDB; }

    CHARACTER *GetPlayerA() { return g_pCharManager->FindByPID(m_dwPIDA); }

    CHARACTER *GetPlayerB() { return g_pCharManager->FindByPID(m_dwPIDB); }

    PIXEL_POSITION GetStartPointA() { return m_StartPointA; }

    PIXEL_POSITION GetStartPointB() { return m_StartPointB; }

    PIXEL_POSITION GetObserverPoint() { return m_ObserverPoint; }

    void EndDuel();

    void ClearEvent() { m_pEvent = nullptr; }

    void OnDisconnect(uint32_t pid);
    void RemoveObserver(uint32_t pid);

    void SendPacketToObserver(const void *c_pvData, int iSize);
    template <typename ...Args>
    void SendChatPacketToObserver(uint8_t type, const char *format, Args ... args);
};

template <typename ... Args> void CArena::SendChatPacketToObserver(uint8_t type, const char *format, Args ... args)
{
    for (auto &[id, ch] : m_mapObserver)
    {
        if (ch) { SendI18nChatPacket(ch, type, format, std::forward<Args>(args)...); }
    }
}

class CArenaMap
{
    friend class CArenaManager;

private :
    uint32_t m_dwMapIndex;
    std::list<CArena *> m_listArena;

protected :
    void Destroy();

    bool AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y);
    void SendArenaMapListTo(CHARACTER *pChar, uint32_t dwMapIndex);

    bool StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint, int nMinute = 5);
    void EndAllDuel();
    bool EndDuel(uint32_t pid);

    int GetDuelList(lua_State *L, int index);

    bool CanAttack(CHARACTER *pCharAttacker, CHARACTER *pCharVictim);
    bool OnDead(CHARACTER *pCharKiller, CHARACTER *pCharVictim);

    bool AddObserver(CHARACTER *pChar, long ObserverX, long ObserverY);
    bool RegisterObserverPtr(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY);

    MEMBER_IDENTITY IsMember(uint32_t PID);
};

class CArenaManager : public singleton<CArenaManager>
{
private :
    std::map<uint32_t, CArenaMap *> m_mapArenaMap;
    std::vector<int> m_listForbidden;

public :
    bool Initialize();
    void Destroy();

    bool StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint, int nMinute = 5, uint32_t map = 0);

    bool AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y);

    void SendArenaMapListTo(CHARACTER *pChar);

    void EndAllDuel();
    bool EndDuel(uint32_t pid);

    void GetDuelList(lua_State *L);

    bool CanAttack(CHARACTER *pCharAttacker, CHARACTER *pCharVictim);

    bool OnDead(CHARACTER *pCharKiller, CHARACTER *pCharVictim);

    bool AddObserver(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY);
    bool RegisterObserverPtr(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY);

    bool IsArenaMap(uint32_t dwMapIndex);
    MEMBER_IDENTITY IsMember(uint32_t dwMapIndex, uint32_t PID);

    bool IsLimitedItem(long lMapIndex, uint32_t dwVnum);
};
#endif /* METIN2_SERVER_GAME_ARENA_H */
