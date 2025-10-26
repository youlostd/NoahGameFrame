#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "NetworkActorManager.h"
#include "PythonBackground.h"

#include "PythonApplication.h"
#include "../gamelib/ActorInstance.h"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/enum.hpp>


bool CPythonNetworkStream::__CanActMainInstance()
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (!pkInstMain)
        return false;

    return pkInstMain->CanAct();
}

void CPythonNetworkStream::__ClearNetworkActorManager()
{
    m_rokNetActorMgr->Destroy();
}

//테이블에서 이름이 "." 인 것들
//차후에 서버에서 보내주지 않게 되면 없어질 함수..(서버님께 꼭!!협박; )
bool IsInvisibleRace(uint16_t raceNum)
{
    switch (raceNum)
    {
    case 20025:
    case 20038:
    case 20039:
        return true;
    default:
        return false;
    }
}

static SNetworkActorData s_kNetActorData = {};

bool CPythonNetworkStream::RecvCharacterAppendPacket(const TPacketGCCharacterAdd& chrAddPacket)
{
    SNetworkActorData kNetActorData = {};
    kNetActorData.m_bType = chrAddPacket.bType;
    kNetActorData.m_dwMovSpd = chrAddPacket.bMovingSpeed;
    kNetActorData.m_dwAtkSpd = chrAddPacket.bAttackSpeed;
    kNetActorData.m_dwRace = chrAddPacket.wRaceNum;

    kNetActorData.m_dwStateFlags = chrAddPacket.bStateFlag;
    kNetActorData.m_dwVID = chrAddPacket.dwVID;
    kNetActorData.m_fRot = chrAddPacket.angle;
    kNetActorData.rotPosition[0] = chrAddPacket.xRot;
    kNetActorData.rotPosition[1] = chrAddPacket.yRot;

    kNetActorData.SetPosition(chrAddPacket.x, chrAddPacket.y);

    kNetActorData.m_sAlignment = 0; /*chrAddPacket.sAlignment*/
#ifdef ENABLE_PLAYTIME_ICON
    kNetActorData.m_dwPlayTime = 0;
#endif
    kNetActorData.m_byPKMode = 0;   /*chrAddPacket.bPKMode*/
    kNetActorData.m_dwGuildID = 0;  /*chrAddPacket.dwGuild*/
    kNetActorData.m_dwEmpireID = 0; /*chrAddPacket.bEmpire*/

    kNetActorData.m_scale = chrAddPacket.scale;

    kNetActorData.m_ownerVid = 0;

    kNetActorData.m_dwMountVnum = 0;

#if defined(WJ_SHOW_MOB_INFO)
    kNetActorData.m_dwLevel = chrAddPacket.dwLevel;
    kNetActorData.m_dwAIFlag = chrAddPacket.dwAIFlag;
#else
	kNetActorData.m_dwLevel = 0; // 몬스터 레벨 표시 안함
#endif

    kNetActorData.m_dwGuildID = chrAddPacket.guildID; //Guild building flags bugfix!

    if (kNetActorData.m_bType != CActorInstance::TYPE_PC &&
        kNetActorData.m_bType != CActorInstance::TYPE_NPC &&
        kNetActorData.m_bType != CActorInstance::TYPE_PET &&
        kNetActorData.m_bType != CActorInstance::TYPE_ATTACK_PET &&
        kNetActorData.m_bType != CActorInstance::TYPE_MOUNT &&
        kNetActorData.m_bType != CActorInstance::TYPE_SHOP &&
        kNetActorData.m_bType != CActorInstance::TYPE_BUFFBOT)
    {
        NpcManager &rkNonPlayer = NpcManager::Instance();

        auto optName = rkNonPlayer.GetName(kNetActorData.m_dwRace);
        if (optName)
            kNetActorData.m_stName = optName.value();

        __RecvCharacterAppendPacket(&kNetActorData);
    }
    else
    {
        s_kNetActorData = kNetActorData;
    }

    if (kNetActorData.m_bType == CActorInstance::TYPE_BUILDING)
    {
        if (!CPythonGuild::instance().IsGuildEnable())
            return true;

        auto &info = CPythonGuild::instance().GetGuildInfoRef();
        if (info.dwGuildID != kNetActorData.m_dwGuildID)
            return true;

        if (CPythonGuild::instance().HasObject(kNetActorData.m_dwRace))
        {
            GuildObject building = {};
            building.vid = kNetActorData.m_dwVID;
            building.vnum = kNetActorData.m_dwRace;
            building.x = kNetActorData.m_lCurX;
            building.y = kNetActorData.m_lCurY;
            building.zRot = kNetActorData.m_fRot;
            building.xRot = 0.0f;
            building.yRot = 0.0f;

            CPythonGuild::instance().AddBuilding(kNetActorData.m_dwVID, building);
        }
    }

    return true;
}

bool CPythonNetworkStream::RecvCharacterAdditionalInfo(const TPacketGCCharacterAdditionalInfo& chrInfoPacket)
{
    SNetworkActorData kNetActorData = s_kNetActorData;
    if (IsInvisibleRace(kNetActorData.m_dwRace))
        return true;

    if (kNetActorData.m_dwVID == chrInfoPacket.dwVID)
    {
        kNetActorData.m_stName = chrInfoPacket.name;

        if (kNetActorData.m_bType != CActorInstance::TYPE_PC && kNetActorData.m_bType != CActorInstance::TYPE_SHOP)
        {
            auto optName = NpcManager::Instance().GetName(kNetActorData.m_dwRace);
            if (optName)
            {
                kNetActorData.m_stName = optName.value();
            }
        }

        kNetActorData.m_ownerVid = chrInfoPacket.ownerVid;
        kNetActorData.m_dwLevel = chrInfoPacket.dwLevel;
        kNetActorData.m_scale = chrInfoPacket.scale;
        kNetActorData.m_sAlignment = chrInfoPacket.sAlignment;
        kNetActorData.m_byPKMode = chrInfoPacket.bPKMode;
        kNetActorData.m_dwGuildID = chrInfoPacket.dwGuildID;
        kNetActorData.m_dwEmpireID = chrInfoPacket.bEmpire;
        kNetActorData.m_dwMountVnum = chrInfoPacket.dwMountVnum;
        kNetActorData.m_isGuildLeader = chrInfoPacket.isGuildLeader;
        kNetActorData.m_isGuildGeneral = chrInfoPacket.isGuildGeneral;
#ifdef ENABLE_PLAYTIME_ICON
        kNetActorData.m_dwPlayTime = chrInfoPacket.dwPlayTime;
#endif
        kNetActorData.m_langCode = chrInfoPacket.lang;
        kNetActorData.m_PvPTeam = chrInfoPacket.pvpTeam;

        kNetActorData.m_pt = chrInfoPacket.pt;

        std::memcpy(kNetActorData.m_parts, chrInfoPacket.adwPart, sizeof(chrInfoPacket.adwPart));

        __RecvCharacterAppendPacket(&kNetActorData);
    }
    else
    {
       // SPDLOG_ERROR("TPacketGCCharacterAdditionalInfo name={0} vid={1} race={2} Error",
       //               chrInfoPacket.name, chrInfoPacket.dwVID, kNetActorData.m_dwRace);
    }
    return true;
}

bool CPythonNetworkStream::RecvCharacterUpdatePacket(const TPacketGCCharacterUpdate& chrUpdatePacket)
{
    SNetworkUpdateActorData kNetUpdateActorData{};
    kNetUpdateActorData.m_dwGuildID = chrUpdatePacket.dwGuildID;
    kNetUpdateActorData.m_dwMovSpd = chrUpdatePacket.bMovingSpeed;
    kNetUpdateActorData.m_dwAtkSpd = chrUpdatePacket.bAttackSpeed;
    kNetUpdateActorData.m_scale = chrUpdatePacket.scale;
    kNetUpdateActorData.m_dwVID = chrUpdatePacket.dwVID;
    kNetUpdateActorData.m_sAlignment = chrUpdatePacket.sAlignment;
    kNetUpdateActorData.m_byPKMode = chrUpdatePacket.bPKMode;
    kNetUpdateActorData.m_dwStateFlags = chrUpdatePacket.bStateFlag;
    kNetUpdateActorData.m_dwLevel = chrUpdatePacket.dwLevel;
    kNetUpdateActorData.m_pt = chrUpdatePacket.pt;
#ifdef ENABLE_PLAYTIME_ICON
    kNetUpdateActorData.m_dwPlayTime = chrUpdatePacket.dwPlayTime;
#endif
    kNetUpdateActorData.m_pvpTeam = chrUpdatePacket.pvpTeam;
    kNetUpdateActorData.m_comboLevel = chrUpdatePacket.comboLevel;
	
    std::memcpy(kNetUpdateActorData.m_parts, chrUpdatePacket.adwPart, sizeof(chrUpdatePacket.adwPart));

    __RecvCharacterUpdatePacket(&kNetUpdateActorData);

    return true;
}

void CPythonNetworkStream::__RecvCharacterAppendPacket(SNetworkActorData *pkNetActorData)
{
    // NOTE : 카메라가 땅에 묻히는 문제의 해결을 위해 메인 캐릭터가 지형에 올려지기
    //        전에 맵을 업데이트 해 높이를 구할 수 있도록 해놓아야 합니다.
    //        단, 게임이 들어갈때가 아닌 이미 캐릭터가 추가 된 이후에만 합니다.
    //        헌데 이동인데 왜 Move로 안하고 Append로 하는지..? - [levites]
    auto &rkPlayer = CPythonPlayer::Instance();
    if (rkPlayer.IsMainCharacterIndex(pkNetActorData->m_dwVID))
    {
        rkPlayer.SetRace(pkNetActorData->m_dwRace);

        if (rkPlayer.NEW_GetMainActorPtr())
        {
            CPythonBackground::Instance().Update(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY, 0.0f);
            CPythonCharacterManager::Instance().Update();

            // NOTE : 사귀 타워일 경우 GOTO 로 이동시에도 맵 이름을 출력하도록 처리
            {
                std::string strMapName = CPythonBackground::Instance().GetWarpMapName();
                if (strMapName == "metin2_map_deviltower1")
                    __ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
            }
        }
        else
        {
            __ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
        }
    }

    m_rokNetActorMgr->AppendActor(*pkNetActorData);

    if (GetMainActorVID() == pkNetActorData->m_dwVID)
    {
        rkPlayer.SetTarget(0);
        if (m_bComboSkillFlag)
            rkPlayer.SetComboSkillFlag(m_bComboSkillFlag);

        __SetGuildID(pkNetActorData->m_dwGuildID);
        //CPythonApplication::Instance().SkipRenderBuffering(10000);
        CPythonPlayer::Instance().SetPVPTeam(pkNetActorData->m_PvPTeam);
    }
    else
        CPythonCharacterManager::Instance().SetPVPTeam(pkNetActorData->m_dwVID, pkNetActorData->m_PvPTeam);
}

void CPythonNetworkStream::__RecvCharacterUpdatePacket(SNetworkUpdateActorData *pkNetUpdateActorData)
{
    m_rokNetActorMgr->UpdateActor(*pkNetUpdateActorData);

    auto &rkPlayer = CPythonPlayer::Instance();
    if (rkPlayer.IsMainCharacterIndex(pkNetUpdateActorData->m_dwVID))
    {
        __SetGuildID(pkNetUpdateActorData->m_dwGuildID);
        rkPlayer.UpdateBattleStatus();
    	rkPlayer.SetComboSkillFlag(pkNetUpdateActorData->m_comboLevel > 0);

        __RefreshStatus();
        __RefreshAlignmentWindow();
        __RefreshEquipmentWindow();
        __RefreshInventoryWindow();

        memset(m_players[m_selectedCharacterSlot].parts, 0, sizeof(m_players[m_selectedCharacterSlot].parts));
        memcpy(m_players[m_selectedCharacterSlot].parts, pkNetUpdateActorData->m_parts, sizeof(m_players[m_selectedCharacterSlot].parts));

    }
    else
    {
        rkPlayer.NotifyCharacterUpdate(pkNetUpdateActorData->m_dwVID);
    }
}

bool CPythonNetworkStream::RecvCharacterDeletePacket(const TPacketGCCharacterDelete& chrDelPacket)
{

    CInstanceBase *pkInstFind = CPythonCharacterManager::instance().GetInstancePtr(chrDelPacket.id);

    if (pkInstFind && pkInstFind->GetInstanceType() == CActorInstance::TYPE_BUILDING)
    {
        if (CPythonGuild::instance().HasObject(pkInstFind->GetRace()))
            CPythonGuild::instance().RemoveBuilding(chrDelPacket.id);
    }
    m_rokNetActorMgr->RemoveActor(chrDelPacket.id);

    // 캐릭터가 사라질때 개인 상점도 없애줍니다.
    // Key Check 를 하기때문에 없어도 상관은 없습니다.
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_PrivateShop_Disappear",
                          chrDelPacket.id
        );

    return true;
}

bool CPythonNetworkStream::RecvCharacterMovePacket(const TPacketGCMove& p)
{

    SNetworkMoveActorData kNetMoveActorData;
    kNetMoveActorData.m_dwArg = p.bArg;
    kNetMoveActorData.m_dwFunc = p.bFunc;
    kNetMoveActorData.m_dwTime = p.dwTime;
    kNetMoveActorData.m_dwVID = p.dwVID;
    kNetMoveActorData.m_fRot = p.bRot * 5.0f;
    kNetMoveActorData.m_lPosX = p.lX;
    kNetMoveActorData.m_lPosY = p.lY;
    kNetMoveActorData.m_dwDuration = p.dwDuration;
    kNetMoveActorData.m_skillColor = p.color;
    kNetMoveActorData.m_isMovingSkill = p.isMovingSkill;
    kNetMoveActorData.m_loopCount = p.loopCount;

    SPDLOG_DEBUG("Recv Movement packet for vid {}", p.dwVID);

    m_rokNetActorMgr->MoveActor(kNetMoveActorData);

    return true;
}

bool CPythonNetworkStream::RecvOwnerShipPacket(const TPacketGCOwnership& p)
{
    m_rokNetActorMgr->SetActorOwner(p.dwOwnerVID, p.dwVictimVID);

    return true;
}

bool CPythonNetworkStream::RecvSyncPositionPacket(const TPacketGCSyncPosition& p)
{

    for(const auto& elem : p.elems) {
        m_rokNetActorMgr->SyncActor(elem.dwVID, elem.lX, elem.lY);
    }

    return true;
}

bool CPythonNetworkStream::RecvNpcUseSkill(const TPacketGCNPCUseSkill& packet)
{

    CInstanceBase *pkInstClone = CPythonCharacterManager::Instance().GetInstancePtr(packet.dwVid);
    uint32_t dwSkillIndex = packet.dwVnum;

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
        return false;

  uint32_t value = 0;
    if (packet.dwLevel <= 17)
        value = 0;
    else if (packet.dwLevel <= 30 && packet.dwLevel > 17)
        value = 1;
    else if (packet.dwLevel > 30 && packet.dwLevel <= 39)
        value = 2;
    else if (packet.dwLevel >= 40)
        value = 3;

    uint32_t dwMotionIndex = pSkillData->GetSkillMotionIndex(value);
    if (!pkInstClone->NEW_UseSkill(dwSkillIndex, dwMotionIndex, 1, false, 0))
    {
        SPDLOG_DEBUG("CPythonPlayer::UseGuildSkill(%d) - pkInstMain->NEW_UseSkill - ERROR", dwSkillIndex);
    }

    return true;
}

bool CPythonNetworkStream::RecvSkillMotion(const GcSkillMotionPacket& p)
{
    long lX = p.x, lY = p.y;

    // Get Skill-Data
    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(p.skillVnum, &pSkillData))
    {
        return false;
    }

    if (pSkillData->IsNoMotion())
        return true;

    DWORD dwMotionIndex = pSkillData->GetSkillMotionIndex(p.skillGrade);
    DWORD dwLoopCount = pSkillData->GetMotionLoopCount(GetSkillPowerByLevel(p.skillLevel) / 100.0f);


    SNetworkMoveActorData kNetMoveActorData;
    kNetMoveActorData.m_dwArg = dwLoopCount | (dwMotionIndex << 8);
    kNetMoveActorData.m_dwFunc = CInstanceBase::FUNC_SKILL;
    kNetMoveActorData.m_dwTime = p.time;
    kNetMoveActorData.m_dwVID = p.vid;
    kNetMoveActorData.m_fRot = p.rotation * 5.0f;
    kNetMoveActorData.m_lPosX = lX;
    kNetMoveActorData.m_lPosY = lY;
    kNetMoveActorData.m_dwDuration = 0;
    kNetMoveActorData.m_loopCount = dwLoopCount;
    kNetMoveActorData.m_isMovingSkill = pSkillData->IsMovingSkill();


    // TraceError("MoveActor");
    m_rokNetActorMgr->MoveActor(kNetMoveActorData);
    // TraceError("MoveActor END");

    return true;
}
