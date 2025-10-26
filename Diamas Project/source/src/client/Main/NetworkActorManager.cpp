#include "StdAfx.h"
#include "NetworkActorManager.h"
#include "PythonCharacterManager.h"
#include "PythonItem.h"

#include "PythonPlayer.h"
#include "../eterBase/Timer.h"
#include "PythonBackground.h"

extern int CHAR_STAGE_VIEW_BOUND;

void SNetworkActorData::UpdatePosition()
{
    uint32_t dwClientCurTime = ELTimer_GetMSec();
    uint32_t dwElapsedTime = dwClientCurTime - m_dwClientSrcTime;

    if (dwElapsedTime < m_dwDuration)
    {
        float fRate = float(dwElapsedTime) / float(m_dwDuration);
        m_lCurX = LONG((m_lDstX - m_lSrcX) * fRate + m_lSrcX);
        m_lCurY = LONG((m_lDstY - m_lSrcY) * fRate + m_lSrcY);
    }
    else
    {
        m_lCurX = m_lDstX;
        m_lCurY = m_lDstY;
    }
}

void SNetworkActorData::SetDstPosition(uint32_t dwServerTime, LONG lDstX, LONG lDstY, uint32_t dwDuration)
{
    m_lSrcX = m_lCurX;
    m_lSrcY = m_lCurY;
    m_lDstX = lDstX;
    m_lDstY = lDstY;

    m_dwDuration = dwDuration;
    m_dwServerSrcTime = dwServerTime;
    m_dwClientSrcTime = ELTimer_GetMSec();
}

void SNetworkActorData::SetPosition(LONG lPosX, LONG lPosY)
{
    m_lDstX = m_lSrcX = m_lCurX = lPosX;
    m_lDstY = m_lSrcY = m_lCurY = lPosY;
}

// NETWORK_ACTOR_DATA_COPY
SNetworkActorData::SNetworkActorData(const SNetworkActorData &src)
{
    __copy__(src);
}

void SNetworkActorData::operator=(const SNetworkActorData &src)
{
    __copy__(src);
}

void SNetworkActorData::__copy__(const SNetworkActorData &src)
{
    m_bType = src.m_bType;
    m_dwVID = src.m_dwVID;
    m_dwStateFlags = src.m_dwStateFlags;
    m_dwEmpireID = src.m_dwEmpireID;
    m_dwRace = src.m_dwRace;
    m_dwMovSpd = src.m_dwMovSpd;
    m_dwAtkSpd = src.m_dwAtkSpd;
    m_fRot = src.m_fRot;
    m_lCurX = src.m_lCurX;
    m_lCurY = src.m_lCurY;
    m_lSrcX = src.m_lSrcX;
    m_lSrcY = src.m_lSrcY;
    m_lDstX = src.m_lDstX;
    m_lDstY = src.m_lDstY;

    m_dwServerSrcTime = src.m_dwServerSrcTime;
    m_dwClientSrcTime = src.m_dwClientSrcTime;
    m_dwDuration = src.m_dwDuration;

    std::memcpy(m_parts, src.m_parts, sizeof(m_parts));

    m_syncOwnerVid = src.m_syncOwnerVid;

    m_sAlignment = src.m_sAlignment;
    m_scale = src.m_scale;
    m_byPKMode = src.m_byPKMode;
    m_dwMountVnum = src.m_dwMountVnum;

    m_ownerVid = src.m_ownerVid;

    m_dwGuildID = src.m_dwGuildID;
    m_langCode = src.m_langCode;
    m_pt = src.m_pt;

    m_dwLevel = src.m_dwLevel;
    m_stName = src.m_stName;
#if defined(WJ_SHOW_MOB_INFO)
    m_dwAIFlag = src.m_dwAIFlag;
#endif
    m_isGuildLeader = src.m_isGuildLeader;
    m_isGuildGeneral = src.m_isGuildGeneral;
#ifdef ENABLE_PLAYTIME_ICON
    m_dwPlayTime = src.m_dwPlayTime;
#endif
}

// END_OF_NETWORK_ACTOR_DATA_COPY

SNetworkActorData::SNetworkActorData()
{
    SetPosition(0, 0);

    m_bType = 0;
    m_dwVID = 0;
    m_dwStateFlags = 0;
    m_dwRace = 0;
    m_dwMovSpd = 0;
    m_dwAtkSpd = 0;
    m_fRot = 0.0f;
    std::memset(m_parts, 0, sizeof(m_parts));

    m_dwEmpireID = 0;

    m_syncOwnerVid = 0;

    m_dwDuration = 0;
    m_dwClientSrcTime = 0;
    m_dwServerSrcTime = 0;

    m_sAlignment = 0;
    m_scale = 100;
    m_byPKMode = 0;
    m_dwMountVnum = 0;

    m_ownerVid = 0;

    m_stName = "";
    m_isGuildLeader = 0;
    m_isGuildGeneral = 0;
#ifdef ENABLE_PLAYTIME_ICON
    m_dwPlayTime = 0;
#endif
}

////////////////////////////////////////////////////////////////////////////////

CNetworkActorManager::CNetworkActorManager()
    : m_lMainPosX(0),
      m_lMainPosY(0)
{
    m_dwMainVID = 0;
}

CNetworkActorManager::~CNetworkActorManager()
{
}

void CNetworkActorManager::Destroy()
{
    m_kNetActorDict.clear();

    m_dwMainVID = 0;
    m_lMainPosX = 0;
    m_lMainPosY = 0;
}

void CNetworkActorManager::SetMainActorVID(uint32_t dwVID)
{
    m_dwMainVID = dwVID;
    m_lMainPosX = 0;
    m_lMainPosY = 0;

    m_kNetActorDict.clear();
}

void CNetworkActorManager::Update()
{
    __OLD_Update();
}

void CNetworkActorManager::__OLD_Update()
{
    __UpdateMainActor();

    CPythonCharacterManager &rkChrMgr = __GetCharacterManager();

    for (auto &p : m_kNetActorDict)
    {
        auto &rkNetActorData = p.second;
        rkNetActorData.UpdatePosition();

        CInstanceBase *pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
        if (!pkInstFind)
        {
            if (__IsVisibleActor(rkNetActorData))
                __AppendCharacterManagerActor(rkNetActorData);
        }
    }
}

CInstanceBase *CNetworkActorManager::__FindActor(SNetworkActorData &rkNetActorData, LONG lDstX, LONG lDstY)
{
    CPythonCharacterManager &rkChrMgr = __GetCharacterManager();
    CInstanceBase *pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
    if (!pkInstFind)
    {
        if (__IsVisiblePos(lDstX, lDstY))
            return __AppendCharacterManagerActor(rkNetActorData);

        return NULL;
    }

    return pkInstFind;
}

CInstanceBase *CNetworkActorManager::__FindActor(SNetworkActorData &rkNetActorData)
{
    CPythonCharacterManager &rkChrMgr = __GetCharacterManager();
    CInstanceBase *pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
    if (!pkInstFind)
    {
        if (__IsVisibleActor(rkNetActorData))
            return __AppendCharacterManagerActor(rkNetActorData);

        return NULL;
    }

    return pkInstFind;
}

void CNetworkActorManager::__RemoveAllGroundItems()
{
    CPythonItem &rkItemMgr = CPythonItem::Instance();
    rkItemMgr.DeleteAllItems();
}

void CNetworkActorManager::__RemoveAllActors()
{
    m_kNetActorDict.clear();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    rkChrMgr.DeleteAllInstances();
}

void CNetworkActorManager::__RemoveDynamicActors()
{
    CPythonCharacterManager &chrMgr = CPythonCharacterManager::Instance();

    std::vector<uint32_t> dwCharacterVIDList;
    for (auto i = chrMgr.CharacterInstanceBegin(); i != chrMgr.CharacterInstanceEnd(); ++i)
        dwCharacterVIDList.push_back((*i)->GetVirtualID());

    for (auto vid : dwCharacterVIDList)
    {
        auto ch = chrMgr.GetInstancePtr(vid);
        if (!ch) // This shouldn't happen - we just collected them
            continue;

        CActorInstance *actorInstance = ch->GetGraphicThingInstancePtr();
        if (actorInstance->IsPC() || actorInstance->IsNPC() || actorInstance->IsEnemy())
        {
            chrMgr.DeleteInstance(vid);
            m_kNetActorDict.erase(vid);
        }
    }

    chrMgr.DestroyDeadInstanceList();
}

void CNetworkActorManager::__UpdateMainActor()
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (!pkInstMain)
        return;

    TPixelPosition kPPosMain;
    pkInstMain->NEW_GetPixelPosition(&kPPosMain);

    m_lMainPosX = LONG(kPPosMain.x);
    m_lMainPosY = LONG(kPPosMain.y);
}

extern bool IsWall(unsigned race);

bool CNetworkActorManager::__IsVisibleActor(const SNetworkActorData &c_rkNetActorData)
{
    if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
        return true;

    if (__IsVisiblePos(c_rkNetActorData.m_lCurX, c_rkNetActorData.m_lCurY))
        return true;

    if (IsWall(c_rkNetActorData.m_dwRace))
        return true;

    return false;
}

bool CNetworkActorManager::__IsVisiblePos(LONG lPosX, LONG lPosY)
{
    LONG dx = lPosX - m_lMainPosX;
    LONG dy = lPosY - m_lMainPosY;
    LONG len = (LONG)sqrt(double(dx * dx + dy * dy));

    if (len < CHAR_STAGE_VIEW_BOUND) // 거리제한 cm
        return true;

    return false;
}

bool CNetworkActorManager::__IsMainActorVID(uint32_t dwVID)
{
    if (dwVID == m_dwMainVID)
        return true;

    return false;
}

CPythonCharacterManager &CNetworkActorManager::__GetCharacterManager()
{
    return CPythonCharacterManager::Instance();
}

void CNetworkActorManager::__RemoveCharacterManagerActor(SNetworkActorData &rkNetActorData)
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

    if (__IsMainActorVID(rkNetActorData.m_dwVID))
        rkChrMgr.DeleteInstance(rkNetActorData.m_dwVID);
    else
        rkChrMgr.DeleteInstanceByFade(rkNetActorData.m_dwVID);
}

CInstanceBase *CNetworkActorManager::__AppendCharacterManagerActor(SNetworkActorData &rkNetActorData)
{
    uint32_t dwVID = rkNetActorData.m_dwVID;

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

    CInstanceBase::SCreateData kCreateData;
    kCreateData.m_bType = rkNetActorData.m_bType;
    kCreateData.m_dwLevel = rkNetActorData.m_dwLevel;
#if defined(WJ_SHOW_MOB_INFO)
    kCreateData.m_dwAIFlag = rkNetActorData.m_dwAIFlag;
#endif
    kCreateData.m_dwGuildID = rkNetActorData.m_dwGuildID;
    kCreateData.m_dwEmpireID = rkNetActorData.m_dwEmpireID;
    kCreateData.m_dwVID = rkNetActorData.m_dwVID;
    kCreateData.m_dwMountVnum = rkNetActorData.m_dwMountVnum;
    kCreateData.m_dwRace = rkNetActorData.m_dwRace;
    kCreateData.m_fRot = rkNetActorData.m_fRot;
    kCreateData.m_lPosX = rkNetActorData.m_lCurX;
    kCreateData.m_lPosY = rkNetActorData.m_lCurY;
    kCreateData.m_stName = rkNetActorData.m_stName;
    kCreateData.m_dwStateFlags = rkNetActorData.m_dwStateFlags;
    kCreateData.m_dwMovSpd = rkNetActorData.m_dwMovSpd;
    kCreateData.m_dwAtkSpd = rkNetActorData.m_dwAtkSpd;
    kCreateData.m_sAlignment = rkNetActorData.m_sAlignment;
    kCreateData.m_byPKMode = rkNetActorData.m_byPKMode;
    kCreateData.m_ownerVid = rkNetActorData.m_ownerVid;

    std::memcpy(kCreateData.m_parts, rkNetActorData.m_parts, sizeof(rkNetActorData.m_parts));

    kCreateData.m_isGuildLeader = rkNetActorData.m_isGuildLeader;
    kCreateData.m_isGuildGeneral = rkNetActorData.m_isGuildGeneral;
#ifdef ENABLE_PLAYTIME_ICON
    kCreateData.m_dwPlayTime = rkNetActorData.m_dwPlayTime;
#endif
    kCreateData.m_scale = rkNetActorData.m_scale;
    kCreateData.m_langCode = rkNetActorData.m_langCode;
    kCreateData.m_pt = rkNetActorData.m_pt;

    kCreateData.m_isMain = __IsMainActorVID(dwVID);

    if (rkChrMgr.GetInstancePtr(dwVID))
        rkChrMgr.DeleteInstance(dwVID);

    CInstanceBase *pNewInstance = rkChrMgr.CreateInstance(kCreateData);
    if (!pNewInstance)
        return NULL;

    if (kCreateData.m_isMain)
    {
        auto &rkPlayer = CPythonPlayer::Instance();
        rkPlayer.SetMainCharacterIndex(dwVID);

        m_lMainPosX = rkNetActorData.m_lCurX;
        m_lMainPosY = rkNetActorData.m_lCurY;
    }

    uint32_t dwClientCurTime = ELTimer_GetMSec();
    uint32_t dwElapsedTime = dwClientCurTime - rkNetActorData.m_dwClientSrcTime;

    if (dwElapsedTime < rkNetActorData.m_dwDuration)
    {
        TPixelPosition kPPosDst;
        kPPosDst.x = float(rkNetActorData.m_lDstX);
        kPPosDst.y = float(rkNetActorData.m_lDstY);
        kPPosDst.z = 0;
        pNewInstance->PushTCPState(rkNetActorData.m_dwServerSrcTime + dwElapsedTime, kPPosDst, rkNetActorData.m_fRot,
                                   CInstanceBase::FUNC_MOVE, 0, 0);
    }
    return pNewInstance;
}

void CNetworkActorManager::AppendActor(const SNetworkActorData &c_rkNetActorData)
{
    // TODO: Check whether we can remove this!
    if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
    {
        //__RemoveAllActors();
        __RemoveDynamicActors();
        __RemoveAllGroundItems();
    }

    SNetworkActorData &rkNetActorData = m_kNetActorDict[c_rkNetActorData.m_dwVID];
    rkNetActorData = c_rkNetActorData;

    if (__IsVisibleActor(rkNetActorData))
    {
        if (!__AppendCharacterManagerActor(rkNetActorData))
            m_kNetActorDict.erase(c_rkNetActorData.m_dwVID);
    }
}

void CNetworkActorManager::RemoveActor(uint32_t dwVID)
{
    auto f = m_kNetActorDict.find(dwVID);
    if (m_kNetActorDict.end() == f)
    {
#ifdef _DEBUG
        SPDLOG_ERROR("CNetworkActorManager::RemoveActor(dwVID={0}) - NOT EXIST VID", dwVID);
#endif
        return;
    }

    SNetworkActorData &rkNetActorData = f->second;
    __RemoveCharacterManagerActor(rkNetActorData);

    m_kNetActorDict.erase(f);
}

void CNetworkActorManager::UpdateActor(const SNetworkUpdateActorData &c_rkNetUpdateActorData)
{
    auto f = m_kNetActorDict.find(c_rkNetUpdateActorData.m_dwVID);
    if (m_kNetActorDict.end() == f)
    {
#ifdef _DEBUG
        SPDLOG_ERROR("CNetworkActorManager::UpdateActor(dwVID={0}) - NOT EXIST VID", c_rkNetUpdateActorData.m_dwVID);
#endif
        return;
    }

    SNetworkActorData &rkNetActorData = f->second;

    CInstanceBase *pkInstFind = __FindActor(rkNetActorData);
    if (pkInstFind)
    {
        CPythonCharacterManager::Instance().SetPVPTeam(pkInstFind->GetVirtualID(), c_rkNetUpdateActorData.m_pvpTeam);

        pkInstFind->SetLevel(c_rkNetUpdateActorData.m_dwLevel);
        pkInstFind->ChangeArmor(c_rkNetUpdateActorData.m_parts[PART_MAIN]);
        pkInstFind->ChangeWeapon(c_rkNetUpdateActorData.m_parts[PART_WEAPON]);
        pkInstFind->ChangeHair(c_rkNetUpdateActorData.m_parts[PART_HAIR]);
        pkInstFind->ChangeAcce(c_rkNetUpdateActorData.m_parts[PART_ACCE]);
        pkInstFind->SetArrow(c_rkNetUpdateActorData.m_parts[PART_ARROW]);
        pkInstFind->SetPrefixNum(c_rkNetUpdateActorData.m_parts[PART_RANK]);

        pkInstFind->ChangeWeaponEffect(c_rkNetUpdateActorData.m_parts[PART_WEAPON_EFFECT]);
        pkInstFind->ChangeArmorEffect(c_rkNetUpdateActorData.m_parts[PART_BODY_EFFECT]);
        pkInstFind->ChangeWingEffect(c_rkNetUpdateActorData.m_parts[PART_WING_EFFECT]);
        pkInstFind->SetTitle(c_rkNetUpdateActorData.m_pt.title, c_rkNetUpdateActorData.m_pt.color);

        pkInstFind->ChangeScale(c_rkNetUpdateActorData.m_scale);

        pkInstFind->ChangeGuild(c_rkNetUpdateActorData.m_dwGuildID);
        pkInstFind->SetMoveSpeed(c_rkNetUpdateActorData.m_dwMovSpd);
        pkInstFind->SetAttackSpeed(c_rkNetUpdateActorData.m_dwAtkSpd);
        pkInstFind->SetAlignment(c_rkNetUpdateActorData.m_sAlignment);
        pkInstFind->SetPKMode(c_rkNetUpdateActorData.m_byPKMode);
        pkInstFind->SetStateFlags(c_rkNetUpdateActorData.m_dwStateFlags);
#ifdef ENABLE_PLAYTIME_ICON
        pkInstFind->SetPlayTime(c_rkNetUpdateActorData.m_dwPlayTime);
#endif
    }

    rkNetActorData.m_dwGuildID = c_rkNetUpdateActorData.m_dwGuildID;
    rkNetActorData.m_dwMovSpd = c_rkNetUpdateActorData.m_dwMovSpd;
    rkNetActorData.m_dwAtkSpd = c_rkNetUpdateActorData.m_dwAtkSpd;
    std::memcpy(rkNetActorData.m_parts, c_rkNetUpdateActorData.m_parts, sizeof(c_rkNetUpdateActorData.m_parts));
    rkNetActorData.m_PvPTeam = c_rkNetUpdateActorData.m_pvpTeam;

    rkNetActorData.m_scale = c_rkNetUpdateActorData.m_scale;
    rkNetActorData.m_sAlignment = c_rkNetUpdateActorData.m_sAlignment;
    rkNetActorData.m_byPKMode = c_rkNetUpdateActorData.m_byPKMode;
#ifdef ENABLE_PLAYTIME_ICON
    rkNetActorData.m_dwPlayTime = c_rkNetUpdateActorData.m_dwPlayTime;
#endif
}

void CNetworkActorManager::MoveActor(const SNetworkMoveActorData &c_rkNetMoveActorData)
{
    auto f = m_kNetActorDict.find(c_rkNetMoveActorData.m_dwVID);
    if (m_kNetActorDict.end() == f)
    {
        SPDLOG_DEBUG("CNetworkActorManager::MoveActor(dwVID=%d) - NOT EXIST VID", c_rkNetMoveActorData.m_dwVID);
        return;
    }

    SNetworkActorData &rkNetActorData = f->second;

    CInstanceBase *pkInstFind = __FindActor(rkNetActorData, c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY);
    if (pkInstFind)
    {
        TPixelPosition kPPosDst;
        kPPosDst.x = float(c_rkNetMoveActorData.m_lPosX);
        kPPosDst.y = float(c_rkNetMoveActorData.m_lPosY);
        kPPosDst.z = 0.0f;

        SPDLOG_DEBUG("PushTCPState: time {} func {} arg {}", c_rkNetMoveActorData.m_dwTime,
                     c_rkNetMoveActorData.m_dwFunc, c_rkNetMoveActorData.m_dwArg);
        pkInstFind->PushTCPState(c_rkNetMoveActorData, kPPosDst);
    }

    rkNetActorData.SetDstPosition(c_rkNetMoveActorData.m_dwTime,
                                  c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY,
                                  c_rkNetMoveActorData.m_dwDuration);
    rkNetActorData.m_fRot = c_rkNetMoveActorData.m_fRot;
}

void CNetworkActorManager::SyncActor(uint32_t dwVID, LONG lPosX, LONG lPosY)
{
    auto f = m_kNetActorDict.find(dwVID);
    if (m_kNetActorDict.end() == f)
    {
        return;
    }

    SNetworkActorData &rkNetActorData = f->second;

    CInstanceBase *pkInstFind = __FindActor(rkNetActorData);
    if (pkInstFind)
    {
        pkInstFind->NEW_SyncPixelPosition(lPosX, lPosY);
    }

    rkNetActorData.SetPosition(lPosX, lPosY);
}

void CNetworkActorManager::SetActorOwner(uint32_t syncOwnerVid, uint32_t dwVictimVID)
{
    auto f = m_kNetActorDict.find(dwVictimVID);
    if (m_kNetActorDict.end() == f)
        return;

    SNetworkActorData &rkNetActorData = f->second;
    rkNetActorData.m_syncOwnerVid = syncOwnerVid;

    auto pkInstFind = __FindActor(rkNetActorData);
    if (pkInstFind)
    {
        pkInstFind->SetSyncOwner(rkNetActorData.m_syncOwnerVid);
    }
}
