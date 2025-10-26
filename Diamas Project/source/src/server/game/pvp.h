#ifndef METIN2_SERVER_GAME_PVP_H
#define METIN2_SERVER_GAME_PVP_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>

class DESC;
class CItem;
class CHARACTER;

// CPVP에는 uint32_t 아이디 두개를 받아서 m_dwCRC를 만들어서 가지고 있는다.
// CPVPManager에서 이렇게 만든 CRC를 통해 검색한다.
class CPVP
{
public:
    friend class CPVPManager;

    typedef struct _player
    {
        uint32_t dwPID;
        uint32_t dwVID;
        bool bAgree;
        bool bCanRevenge;

        _player()
            : dwPID(0), dwVID(0), bAgree(false), bCanRevenge(false)
        {
        }
    } TPlayer;

    CPVP(uint32_t dwID1, uint32_t dwID2);
    CPVP(CPVP &v);
    ~CPVP();

    void Win(uint32_t dwPID);        // dwPID가 이겼다!
    bool CanRevenge(uint32_t dwPID); // dwPID가 복수할 수 있어?
    bool IsFight();
    bool Agree(uint32_t dwPID);

    void SetVID(uint32_t dwPID, uint32_t dwVID);
    void Packet(bool bDelete = false);

    void SetLastFightTime() { m_dwLastFightTime = get_dword_time(); }

    uint32_t GetLastFightTime() const { return m_dwLastFightTime; }

    uint32_t GetCRC() { return m_dwCRC; }

protected:
    TPlayer m_players[2];
    uint32_t m_dwCRC;
    bool m_bRevenge;

    uint32_t m_dwLastFightTime;
};

class CPVPManager : public singleton<CPVPManager>
{
    typedef std::map<uint32_t, std::unordered_set<CPVP*>> CPVPSetMap;

public:
    CPVPManager();
    virtual ~CPVPManager();

    void Insert(CHARACTER* pkChr, CHARACTER* pkVictim);
    bool CanAttack(CHARACTER* pkChr, CHARACTER* pkVictim);
    bool Dead(CHARACTER* pkChr, uint32_t dwKillerPID, CHARACTER* pkKiller = nullptr);
    void GiveUp(CHARACTER* pkChr, uint32_t dwKillerPID);
    void Connect(CHARACTER* pkChr);
    void Disconnect(CHARACTER* pkChr);
    bool IsDuelingInstance(CHARACTER* pkChr);
    void SendList(DESC* d);
    void Delete(CPVP* pkPVP);
    std::unordered_set<CPVP*> GetMyDuels(CHARACTER* pkChr);

    void Process();

public:
    CPVP* Find(uint32_t dwCRC);

protected:
    void ConnectEx(CHARACTER* pkChr, bool bDisconnect);

    std::map<uint32_t, std::unique_ptr<CPVP>> m_map_pkPVP;
    CPVPSetMap m_map_pkPVPSetByID;
};

#endif /* METIN2_SERVER_GAME_PVP_H */
