#ifndef METIN2_CLIENT_MAIN_NETWORKACTORMANAGER_H
#define METIN2_CLIENT_MAIN_NETWORKACTORMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "InstanceBase.h"
#include <game/Constants.hpp>

struct SNetworkActorData
{
    std::string m_stName;

    uint32_t rotPosition[2]{};

    uint8_t m_bType = 0;
    uint32_t m_dwVID = 0;
    uint32_t m_dwStateFlags = 0;
    uint32_t m_dwEmpireID = 0;
    uint32_t m_dwRace = 0;
    uint32_t m_dwMovSpd = 0;
    uint32_t m_dwAtkSpd = 0;
    FLOAT m_fRot = 0.0f;
    LONG m_lCurX = 0;
    LONG m_lCurY = 0;
    LONG m_lSrcX = 0;
    LONG m_lSrcY = 0;
    LONG m_lDstX = 0;
    LONG m_lDstY = 0;

    uint32_t m_dwServerSrcTime = 0;
    uint32_t m_dwClientSrcTime = 0;
    uint32_t m_dwDuration = 0;

    Part m_parts[PART_MAX_NUM]{};

    uint32_t m_syncOwnerVid = 0;
    uint32_t m_ownerVid = 0;

    PlayerAlignment m_sAlignment;
    uint8_t m_byPKMode;
    uint32_t m_dwMountVnum;
#ifdef ENABLE_PLAYTIME_ICON
    DWORD m_dwPlayTime;
#endif
    uint32_t m_dwGuildID;
    uint32_t m_dwLevel;
#if defined(WJ_SHOW_MOB_INFO)
    uint32_t m_dwAIFlag;
#endif
    uint8_t m_langCode;

    uint8_t m_isGuildLeader;
    uint8_t m_isGuildGeneral;
    int16_t m_PvPTeam = -1;

    uint16_t m_scale{};

    TPlayerTitle m_pt = {};

    SNetworkActorData();

    void SetDstPosition(uint32_t dwServerTime, LONG lDstX, LONG lDstY, uint32_t dwDuration);
    void SetPosition(LONG lPosX, LONG lPosY);
    void UpdatePosition();

    // NETWORK_ACTOR_DATA_COPY
    SNetworkActorData(const SNetworkActorData &src);
    void operator=(const SNetworkActorData &src);
    void __copy__(const SNetworkActorData &src);
    // END_OF_NETWORK_ACTOR_DATA_COPY
};

struct SNetworkUpdateActorData
{
    uint32_t m_dwVID;
    uint32_t m_dwGuildID;
    Part m_parts[PART_MAX_NUM];
    uint32_t m_dwMovSpd;
    uint32_t m_dwAtkSpd;
    PlayerAlignment m_sAlignment;
    uint32_t m_dwLevel;
    uint8_t m_byPKMode;
#ifdef ENABLE_PLAYTIME_ICON
    DWORD m_dwPlayTime;
#endif
    uint32_t m_dwStateFlags; // ���� Create ���� ���̴� ������
    uint16_t m_scale;
    int16_t m_pvpTeam = -1;
	uint8_t m_comboLevel = 0;

    TPlayerTitle m_pt;
};

struct SNetworkMoveActorData
{
    uint32_t m_dwVID = 0;
    uint32_t m_dwTime = 0;
    LONG m_lPosX = 0;
    LONG m_lPosY = 0;
    float m_fRot = 0;
    uint32_t m_dwFunc = 0;
    uint32_t m_dwArg = 0;
    uint32_t m_dwDuration = 0;
    uint32_t m_skillColor = 0;
    uint8_t m_isMovingSkill = 0;
    uint8_t m_loopCount = 0;
};

class CPythonCharacterManager;
class CInstanceBase;

class CNetworkActorManager
{
public:
    CNetworkActorManager();
    virtual ~CNetworkActorManager();

    void Destroy();

    void SetMainActorVID(uint32_t dwVID);

    void RemoveActor(uint32_t dwVID);
    void AppendActor(const SNetworkActorData &c_rkNetActorData);
    void UpdateActor(const SNetworkUpdateActorData &c_rkNetUpdateActorData);
    void MoveActor(const SNetworkMoveActorData &c_rkNetMoveActorData);

    void SyncActor(uint32_t dwVID, LONG lPosX, LONG lPosY);
    void SetActorOwner(uint32_t dwOwnerVID, uint32_t dwVictimVID);
    SNetworkActorData *__FindActorData(uint32_t dwVID);

    void Update();

protected:
    void __OLD_Update();

    void __UpdateMainActor();

    bool __IsVisiblePos(LONG lPosX, LONG lPosY);
    bool __IsVisibleActor(const SNetworkActorData &c_rkNetActorData);
    bool __IsMainActorVID(uint32_t dwVID);

    void __RemoveAllGroundItems();
    void __RemoveAllActors();
    void __RemoveDynamicActors();
    void __RemoveCharacterManagerActor(SNetworkActorData &rkNetActorData);

    CInstanceBase *__AppendCharacterManagerActor(SNetworkActorData &rkNetActorData);
    CInstanceBase *__FindActor(SNetworkActorData &rkNetActorData);
    CInstanceBase *__FindActor(SNetworkActorData &rkNetActorData, LONG lDstX, LONG lDstY);

    CPythonCharacterManager &__GetCharacterManager();

protected:
    uint32_t m_dwMainVID;

    LONG m_lMainPosX;
    LONG m_lMainPosY;

    std::unordered_map<uint32_t, SNetworkActorData> m_kNetActorDict;
};
#endif /* METIN2_CLIENT_MAIN_NETWORKACTORMANAGER_H */
