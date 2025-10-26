#include "threeway_war.h"

#include <game/length.h>
#include <game/DbPackets.hpp>
#include <game/GamePacket.hpp>

#include "char.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "config.h"

#include "ChatUtil.hpp"

EVENTINFO(regen_mob_event_info)
{
    uint32_t dwMapIndex;

    regen_mob_event_info()
        : dwMapIndex(0)
    {
    }
};

EVENTFUNC(regen_mob_event)
{
    regen_mob_event_info *info =
        static_cast<regen_mob_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("regen_mob_event> <Factor> Null pointer");
        return 0;
    }

    int iMapIndex = info->dwMapIndex;

    char filename[128];
    std::string szFilename(GetSungziMapPath());

    int choice =
        quest::CQuestManager::instance().GetEventFlag("threeway_war_choice");
    if (0 == choice)
        sprintf(filename, "%sregen00.txt", GetSungziMapPath());
    else { sprintf(filename, "%sregen00_%d.txt", GetSungziMapPath(), choice); }

    SECTREE_MAP *pkMap = SECTREE_MANAGER::instance().GetMap(iMapIndex);
    if (nullptr != pkMap)
    {
        if (0 != choice)
        {
            if (regen_load_in_file(filename, iMapIndex))
            {
                sprintf(filename, "%sregen00.txt", GetSungziMapPath());
                regen_load_in_file(filename, iMapIndex);
            }
        }
    }

    return 0;
}

CThreeWayWar::CThreeWayWar()
    : RegenFlag_(0) { std::memset(KillScore_, 0, sizeof(KillScore_)); }

CThreeWayWar::~CThreeWayWar()
{
    RegisterUserMap_.clear();
    ReviveTokenMap_.clear();
}

bool CThreeWayWar::Initialize()
{
    if (!LoadSetting("data/forkedmapindex.txt"))
        return false;

    return true;
}

int CThreeWayWar::GetKillScore(uint8_t empire) const
{
    if (empire <= 0 || empire >= EMPIRE_MAX_NUM)
    {
        SPDLOG_ERROR("ThreeWayWar::GetKillScore Wrong Empire variable");
        return 0;
    }

    return KillScore_[empire - 1];
}

void CThreeWayWar::SetKillScore(uint8_t empire, int count)
{
    if (empire <= 0 || empire >= EMPIRE_MAX_NUM)
    {
        SPDLOG_ERROR("ThreeWayWar::SetKillScore Wrong Empire variable");
        return;
    }

    KillScore_[empire - 1] = count;
}

void CThreeWayWar::SetReviveTokenForPlayer(uint32_t PlayerID, int count)
{
    if (0 == PlayerID)
        return;

    ReviveTokenMap_[PlayerID] = count;
}

int CThreeWayWar::GetReviveTokenForPlayer(uint32_t PlayerID)
{
    if (0 == PlayerID)
        return 0;

    return ReviveTokenMap_[PlayerID];
}

void CThreeWayWar::DecreaseReviveTokenForPlayer(uint32_t PlayerID)
{
    if (0 == PlayerID)
        return;

    ReviveTokenMap_[PlayerID] = ReviveTokenMap_[PlayerID] - 1;
}

const ForkedPassMapInfo &CThreeWayWar::GetEventPassMapInfo() const
{
    const size_t idx =
        quest::CQuestManager::instance().GetEventFlag("threeway_war_pass_idx");
    return PassInfoMap_[idx];
}

const ForkedSungziMapInfo &CThreeWayWar::GetEventSungZiMapInfo() const
{
    const size_t idx = quest::CQuestManager::instance().GetEventFlag(
        "threeway_war_sungzi_idx");
    return SungZiInfoMap_[idx];
}

bool CThreeWayWar::IsThreeWayWarMapIndex(int iMapIndex) const
{
    return MapIndexSet_.find(iMapIndex) != MapIndexSet_.end();
}

bool CThreeWayWar::IsSungZiMapIndex(int iMapIndex) const
{
    auto it = SungZiInfoMap_.begin();
    for (; it != SungZiInfoMap_.end(); ++it) { if (iMapIndex == it->m_iForkedSung) { return true; } }

    return false;
}

void CThreeWayWar::RandomEventMapSet()
{
    const auto pass_idx = Random::get<uint32_t>(0, PassInfoMap_.size() - 1);
    const auto sung_idx = Random::get<uint32_t>(0, SungZiInfoMap_.size() - 1);

    quest::CQuestManager::instance().RequestSetEventFlag(
        "threeway_war_sungzi_idx", sung_idx);
    quest::CQuestManager::instance().RequestSetEventFlag(
        "threeway_war_pass_idx", pass_idx);
}

bool CThreeWayWar::IsRegisteredUser(uint32_t PlayerID) const
{
    auto iter = RegisterUserMap_.find(PlayerID);
    if (iter == RegisterUserMap_.end())
        return false;

    return true;
}

void CThreeWayWar::RegisterUser(uint32_t PlayerID)
{
    if (0 == PlayerID)
        return;

    RegisterUserMap_.insert(std::make_pair(PlayerID, PlayerID));
}

int GetKillValue(int level)
{
    int iMinLevelFor1Point, iMaxLevelFor1Point;
    int iMinLevelFor2Point, iMaxLevelFor2Point;
    int iMinLevelFor3Point, iMaxLevelFor3Point;

    iMinLevelFor1Point = 50;
    iMaxLevelFor1Point = 59;
    iMinLevelFor2Point = 60;
    iMaxLevelFor2Point = 69;
    iMinLevelFor3Point = 70;
    iMaxLevelFor3Point = 99;

    if (iMinLevelFor1Point <= level && level <= iMaxLevelFor1Point) { return 1; }
    else if (iMinLevelFor2Point <= level && level <= iMaxLevelFor2Point) { return 2; }
    else if (iMinLevelFor3Point <= level && level <= iMaxLevelFor3Point) { return 3; }

    return 0;
}

void CThreeWayWar::onDead(CHARACTER *pChar, CHARACTER *pkKiller)
{
    if (false == pChar->IsPC())
        return;

    if (GM_PLAYER != pChar->GetGMLevel() && false == gConfig.testServer)
        return;

    if (-1 == GetRegenFlag())
        return;

    DecreaseReviveTokenForPlayer(pChar->GetPlayerID());

    if (false == IsSungZiMapIndex(pChar->GetMapIndex()))
        return;

    if (nullptr == pkKiller || true != pkKiller->IsPC())
        return;

    // 같은 제국은 계산하지 않음
    if (pChar->GetEmpire() == pkKiller->GetEmpire())
        return;

    int nKillScore = GetKillScore(pkKiller->GetEmpire());

    // 제국 킬 스코어가 -1일경우는 탈락국가이기때문에 점수 체크를 하면 안된다.
    if (nKillScore >= 0)
    {
        nKillScore += GetKillValue(pChar->GetLevel());
        SetKillScore(pkKiller->GetEmpire(), nKillScore);
    }

    if (nKillScore != 0 && (gConfig.testServer || (nKillScore % 5) == 0))
    {
        char szBuf[64 + 1];

        std::snprintf(szBuf, sizeof(szBuf),
                      LC_TEXT("현재 스코어 신수국:%d 천조국:%d 진노국:%d"),
                      GetKillScore(1), GetKillScore(2), GetKillScore(3));

        SendNoticeMap(szBuf, GetSungziMapIndex(), false);
    }

    const int nVictoryScore = quest::CQuestManager::instance().GetEventFlag(
        "threeway_war_kill_count");

    if (0 == GetRegenFlag())
    {
        int nEliminatedEmpireCount = 0;
        uint8_t bLoseEmpire = 0;

        for (int n = 1; n < 4; ++n)
        {
            if (nVictoryScore > GetKillScore(n))
            {
                ++nEliminatedEmpireCount;
                bLoseEmpire = n;
            }
        }

        if (1 != nEliminatedEmpireCount)
            return;

        //----------------------
        //카운트 초기화
        //----------------------
        SetKillScore(1, 0);
        SetKillScore(2, 0);
        SetKillScore(3, 0);
        SetKillScore(bLoseEmpire, -1);

        quest::warp_all_to_map_my_empire_event_info *info;

        //----------------------
        //탈락국가 퇴장 시키기 : 성지에서
        //----------------------
        info = AllocEventInfo<quest::warp_all_to_map_my_empire_event_info>();

        info->m_lMapIndexFrom = GetSungziMapIndex();
        info->m_lMapIndexTo = EMPIRE_START_MAP(bLoseEmpire);
        info->m_x = EMPIRE_START_X(bLoseEmpire);
        info->m_y = EMPIRE_START_Y(bLoseEmpire);
        info->m_bEmpire = bLoseEmpire;

        event_create(quest::warp_all_to_map_my_empire_event, info,
                     THECORE_SECS_TO_PASSES(10));

        //----------------------
        //탈락국가 퇴장 시키기 : 통로에서
        //----------------------
        info = AllocEventInfo<quest::warp_all_to_map_my_empire_event_info>();

        info->m_lMapIndexFrom = GetPassMapIndex(bLoseEmpire);
        info->m_lMapIndexTo = EMPIRE_START_MAP(bLoseEmpire);
        info->m_x = EMPIRE_START_X(bLoseEmpire);
        info->m_y = EMPIRE_START_Y(bLoseEmpire);
        info->m_bEmpire = bLoseEmpire;

        event_create(quest::warp_all_to_map_my_empire_event, info,
                     THECORE_SECS_TO_PASSES(10));

        //----------------------
        //성지에 팅기는 국가에 대한 이야기를 마왕이 함!
        //----------------------
        const std::string Nation(EMPIRE_NAME(bLoseEmpire));
        const std::string Script(
            LC_TEXT("성지의 마왕: 너희 ") + Nation +
            LC_TEXT("녀석들은 이곳 성지에 있을 자격을 잃었다. 모두 성지에서 "
                "물러나거라~~[ENTER][ENTER] 10초 후에 모두 마을로 이동하게 "
                "됩니다. ") +
            "[ENTER][DONE]");

        g_pCharManager->SendScriptToMap(pChar->GetMapIndex(),
                                        Script);

        //----------------------
        // 공지 한방 날려줌.
        //----------------------
        BroadcastNotice(fmt::format("삼거리 전투에서 %s 국가가 가장먼저 탈락을 하였습니다#{}", Nation).c_str());
        LogManager::instance().CharLog(0, 0, 0, 0, "THREEWAY", fmt::format("First Step: {} exclusion", Nation).c_str(),
                                       nullptr);

        //----------------------
        // 몹을 리젠한다.
        //----------------------
        regen_mob_event_info *regen_info =
            AllocEventInfo<regen_mob_event_info>();

        regen_info->dwMapIndex = pChar->GetMapIndex();

        event_create(regen_mob_event, regen_info, THECORE_SECS_TO_PASSES(10));

        SetRegenFlag(1);
    }
    else if (1 == GetRegenFlag())
    {
        int nVictoryEmpireIndex = 0;

        for (int n = 1; n < 4; ++n)
        {
            nKillScore = GetKillScore(n);

            if (nKillScore == -1)
                continue;

            if (nVictoryScore <= nKillScore)
            {
                nVictoryEmpireIndex = n;
                break;
            }
        }

        if (0 == nVictoryEmpireIndex)
            return;

        for (int n = 1; n < 4; ++n)
        {
            if (n != nVictoryEmpireIndex)
            {
                uint8_t bLoseEmpire = n;
                quest::warp_all_to_map_my_empire_event_info *info;

                //----------------------
                //탈락국가 퇴장 시키기 : 성지에서
                //----------------------
                info = AllocEventInfo<
                    quest::warp_all_to_map_my_empire_event_info>();

                info->m_lMapIndexFrom = GetSungziMapIndex();
                info->m_lMapIndexTo = EMPIRE_START_MAP(bLoseEmpire);
                info->m_x = EMPIRE_START_X(bLoseEmpire);
                info->m_y = EMPIRE_START_Y(bLoseEmpire);
                info->m_bEmpire = bLoseEmpire;

                event_create(quest::warp_all_to_map_my_empire_event, info,
                             THECORE_SECS_TO_PASSES(5));

                //----------------------
                //탈락국가 퇴장 시키기 : 통로에서
                //----------------------
                info = AllocEventInfo<
                    quest::warp_all_to_map_my_empire_event_info>();

                info->m_lMapIndexFrom = GetPassMapIndex(bLoseEmpire);
                info->m_lMapIndexTo = EMPIRE_START_MAP(bLoseEmpire);
                info->m_x = EMPIRE_START_X(bLoseEmpire);
                info->m_y = EMPIRE_START_Y(bLoseEmpire);
                info->m_bEmpire = bLoseEmpire;

                event_create(quest::warp_all_to_map_my_empire_event, info,
                             THECORE_SECS_TO_PASSES(5));
            }
        }

        //------------------------------
        // 최종 스코어 표시
        //------------------------------
        {
            char szBuf[64 + 1];
            std::snprintf(szBuf, sizeof(szBuf),
                          LC_TEXT("현재 스코어 신수국:%d 천조국:%d 진노국:%d"),
                          GetKillScore(1), GetKillScore(2), GetKillScore(3));

            SendNoticeMap(szBuf, GetSungziMapIndex(), false);
        }

        // 메세지를 띄워준다.
        SECTREE_MAP *pSecMap =
            SECTREE_MANAGER::instance().GetMap(pChar->GetMapIndex());
        if (nullptr != pSecMap)
        {
            const std::string EmpireName(EMPIRE_NAME(nVictoryEmpireIndex));
            const std::string Script(
                EmpireName +
                LC_TEXT(". 너희가 성지의 수호자를 잡게 된다면 너희는 성지의 "
                    "주인이 된다.[ENTER][ENTER] ") +
                "[ENTER][DONE]");

            quest::FSendScriptToEmpire fSend;
            fSend.bEmpire = nVictoryEmpireIndex;

            fSend.p.skin = 1;
            fSend.p.script = Script;

            pSecMap->for_each(fSend);

            char szBuf[512];
            std::snprintf(szBuf, sizeof(szBuf), "Second Step: %s remain",
                          EMPIRE_NAME(nVictoryEmpireIndex));
            LogManager::instance().CharLog(0, 0, 0, 0, "THREEWAY", szBuf, nullptr);
        }

        //------------------------------
        // 마지막 보상 : 진구미호 소환
        //-----------------------------
        for (int n = 0; n < quest::CQuestManager::instance().GetEventFlag(
                            "threeway_war_boss_count");)
        {
            int x = pChar->GetX();
            int y = pChar->GetY();

            x = Random::get(0, 1) ? x - Random::get(200, 1000) : x + Random::get(200, 1000);
            y = Random::get(0, 1) ? y - Random::get(200, 1000) : y + Random::get(200, 1000);

            if (x < 0)
                x = pChar->GetX();

            if (y < 0)
                y = pChar->GetY();

            CHARACTER *ch = g_pCharManager->SpawnMob(
                GetEventSungZiMapInfo().m_iBossMobVnum, pChar->GetMapIndex(), x,
                y, 0, false);

            if (nullptr != ch)
            {
                ch->SetAggressive();
                ++n;
            }
        }

        SetRegenFlag(-1);
    }
}

struct FDestroyAllEntity
{
    void operator()(CEntity *ent)
    {
        if (true == ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = static_cast<CHARACTER *>(ent);

            if (false == ch->IsPC()) { ch->Dead(); }
        }
    }
};

void CThreeWayWar::RemoveAllMonstersInThreeWay() const
{
    auto iter = MapIndexSet_.begin();
    while (iter != MapIndexSet_.end())
    {
        SECTREE_MAP *pSecMap = SECTREE_MANAGER::instance().GetMap(*iter);
        if (nullptr != pSecMap)
        {
            FDestroyAllEntity f;

            pSecMap->for_each(f);
        }

        ++iter;
    }
}

bool CThreeWayWar::LoadSetting(const char *szFileName)
{
    FILE *pf = fopen(szFileName, "r");
    if (nullptr == pf)
    {
        SPDLOG_ERROR("[INIT_FORKED] Do not open file ({0})", szFileName);
        return false;
    }

    char szLine[256];
    char szSungziName[128];
    char szPassName[3][128];

    while (nullptr != fgets(szLine, 256, pf))
    {
        if (0 == strncmp(szLine, "sungzi:", 7))
        {
            struct ForkedSungziMapInfo sungziinfo;

            sscanf(szLine + 7, "%d %d %d %d %d %d %d %s %d",
                   &sungziinfo.m_iForkedSung,
                   &sungziinfo.m_iForkedSungziStartPosition[0][0],
                   &sungziinfo.m_iForkedSungziStartPosition[0][1],
                   &sungziinfo.m_iForkedSungziStartPosition[1][0],
                   &sungziinfo.m_iForkedSungziStartPosition[1][1],
                   &sungziinfo.m_iForkedSungziStartPosition[2][0],
                   &sungziinfo.m_iForkedSungziStartPosition[2][1], szSungziName,
                   &sungziinfo.m_iBossMobVnum);

            sungziinfo.m_stMapName = static_cast<std::string>(szSungziName);

            SungZiInfoMap_.push_back(sungziinfo);

            MapIndexSet_.insert(sungziinfo.m_iForkedSung);
        }
        else if (0 == strncmp(szLine, "pass:", 5))
        {
            struct ForkedPassMapInfo passinfo;

            sscanf(szLine + 5, "%d %d %d %s %d %d %d %s %d %d %d %s",
                   &passinfo.m_iForkedPass[0],
                   &passinfo.m_iForkedPassStartPosition[0][0],
                   &passinfo.m_iForkedPassStartPosition[0][1], szPassName[0],
                   &passinfo.m_iForkedPass[1],
                   &passinfo.m_iForkedPassStartPosition[1][0],
                   &passinfo.m_iForkedPassStartPosition[1][1], szPassName[1],
                   &passinfo.m_iForkedPass[2],
                   &passinfo.m_iForkedPassStartPosition[2][0],
                   &passinfo.m_iForkedPassStartPosition[2][1], szPassName[2]);

            passinfo.m_stMapName[0] = static_cast<std::string>(szPassName[0]);
            passinfo.m_stMapName[1] = static_cast<std::string>(szPassName[1]);
            passinfo.m_stMapName[2] = static_cast<std::string>(szPassName[2]);

            PassInfoMap_.push_back(passinfo);

            MapIndexSet_.insert(passinfo.m_iForkedPass[0]);
            MapIndexSet_.insert(passinfo.m_iForkedPass[1]);
            MapIndexSet_.insert(passinfo.m_iForkedPass[2]);
        }
    }

    fclose(pf);
    return true;
}

//
// C functions
//

const char *GetSungziMapPath()
{
    static char s_szMapPath[128];

    std::snprintf(
        s_szMapPath, sizeof(s_szMapPath), "data/map/%s/",
        g_pThreeWayWar->GetEventSungZiMapInfo().m_stMapName.c_str());

    return s_szMapPath;
}

const char *GetPassMapPath(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
    {
        static char s_szMapPath[128];

        std::snprintf(s_szMapPath, sizeof(s_szMapPath), "data/map/%s/",
                      CThreeWayWar::instance()
                      .GetEventPassMapInfo()
                      .m_stMapName[bEmpire - 1]
                      .c_str());

        return s_szMapPath;
    }

    return nullptr;
}

int GetPassMapIndex(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
        return CThreeWayWar::instance()
               .GetEventPassMapInfo()
               .m_iForkedPass[bEmpire - 1];

    return 0;
}

int GetPassStartX(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
        return CThreeWayWar::instance()
               .GetEventPassMapInfo()
               .m_iForkedPassStartPosition[bEmpire - 1][0];

    return 0;
}

int GetPassStartY(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
        return CThreeWayWar::instance()
               .GetEventPassMapInfo()
               .m_iForkedPassStartPosition[bEmpire - 1][1];

    return 0;
}

int GetSungziMapIndex() { return g_pThreeWayWar->GetEventSungZiMapInfo().m_iForkedSung; }

int GetSungziStartX(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
        return CThreeWayWar::instance()
               .GetEventSungZiMapInfo()
               .m_iForkedSungziStartPosition[bEmpire - 1][0];

    return 0;
}

int GetSungziStartY(uint8_t bEmpire)
{
    if (bEmpire > 0 && bEmpire < EMPIRE_MAX_NUM)
        return CThreeWayWar::instance()
               .GetEventSungZiMapInfo()
               .m_iForkedSungziStartPosition[bEmpire - 1][1];

    return 0;
}
