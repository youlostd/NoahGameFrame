#include "war_map.h"
#include "GBufferManager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "desc.h"
#include "desc_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "item.h"
#include "sectree_manager.h"
#include <game/AffectConstants.hpp>

#include "locale_service.h"
#ifdef ENABLE_NEW_GUILD_WAR
#include "questmanager.h"
#endif
#include <boost/algorithm/string/replace.hpp>
#include <game/GamePacket.hpp>

EVENTINFO(war_map_info)
{
    int iStep;
    CWarMap* pWarMap;

    war_map_info()
        : iStep(0)
        , pWarMap(nullptr)
    {
    }
};

EVENTFUNC(war_begin_event)
{
    auto info = static_cast<war_map_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("war_begin_event> <Factor> Null pointer");
        return 0;
    }

    CWarMap* pMap = info->pWarMap;
    pMap->CheckWarEnd();
    return THECORE_SECS_TO_PASSES(10);
}

#ifdef ENABLE_GUILD_WAR_BEGIN
EVENTFUNC(war_begin_protection_event)
{
    war_map_info* info = dynamic_cast<war_map_info*>(event->info);

    if (info == NULL) {
        SPDLOG_ERROR("war_begin_protection_event> <Factor> Null pointer");
        return 0;
    }

    CWarMap* pMap = info->pWarMap;

    if (pMap->GetBeginProtectionStatus() == false) {
        DWORD dwElapsedTime = get_dword_time() - pMap->GetStartTime();
        if (dwElapsedTime >= 120000) {
            pMap->SetBeginProtectionStatus(true);
            pMap->Notice("Savaş başladı!");
            SECTREE_MANAGER::instance().PurgeNPCsInMap(pMap->GetMapIndex());
            return 0;
        } 
        else {
            char szNoticeText[512];
            snprintf(szNoticeText, sizeof(szNoticeText),
                     "Savaş %u saniye içinde başlayacak.",
                     (120 - (dwElapsedTime / 1000)));
            pMap->Notice(szNoticeText);
        }
        if ((dwElapsedTime / 1000) >= 100)
            return THECORE_SECS_TO_PASSES(1);
    }

    return THECORE_SECS_TO_PASSES(10);
}
#endif

EVENTFUNC(war_end_event)
{
    auto info = static_cast<war_map_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("war_end_event> <Factor> Null pointer");
        return 0;
    }

    CWarMap* pMap = info->pWarMap;

    if (info->iStep == 0) {
        ++info->iStep;
        pMap->ExitAll();
        return THECORE_SECS_TO_PASSES(5);
    } else {
        pMap->SetEndEvent(nullptr);
        CWarMapManager::instance().DestroyWarMap(pMap);
        return 0;
    }
}

EVENTFUNC(war_timeout_event)
{
    auto info = static_cast<war_map_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("war_timeout_event> <Factor> Null pointer");
        return 0;
    }

    CWarMap* pMap = info->pWarMap;
    pMap->Timeout();
    return 0;
}

void CWarMap::STeamData::Initialize()
{
    dwID = 0;
    pkGuild = nullptr;
    iMemberCount = 0;
    iUsePotionPrice = 0;
    iScore = 0;
    pkChrFlag = nullptr;
    pkChrFlagBase = nullptr;

    set_pidJoiner.clear();
}

CWarMap::CWarMap(int32_t lMapIndex, const TGuildWarInfo& r_info,
                 TWarMapInfo* pkWarMapInfo, uint32_t dwGuildID1,
                 uint32_t dwGuildID2)
{
    m_kMapInfo = *pkWarMapInfo;
    m_kMapInfo.lMapIndex = lMapIndex;

    memcpy(&m_WarInfo, &r_info, sizeof(TGuildWarInfo));

    m_TeamData[0].Initialize();
    m_TeamData[0].dwID = dwGuildID1;
    m_TeamData[0].pkGuild = CGuildManager::instance().TouchGuild(dwGuildID1);

    m_TeamData[1].Initialize();
    m_TeamData[1].dwID = dwGuildID2;
    m_TeamData[1].pkGuild = CGuildManager::instance().TouchGuild(dwGuildID2);
    m_iObserverCount = 0;

    war_map_info* info = AllocEventInfo<war_map_info>();
    info->pWarMap = this;

    SetBeginEvent(
        event_create(war_begin_event, info, THECORE_SECS_TO_PASSES(60)));
    m_pkEndEvent = nullptr;
    m_pkTimeoutEvent = nullptr;
    m_pkResetFlagEvent = nullptr;
    m_bTimeout = false;
    m_dwStartTime = get_dword_time();
    m_bEnded = false;
#ifdef ENABLE_GUILD_WAR_BEGIN
    m_bBeginProtectionCompleted = false;
#endif
    if (GetType() == WAR_MAP_TYPE_FLAG) {
        AddFlagBase(0);
        AddFlagBase(1);
        AddFlag(0);
        AddFlag(1);
    }
#ifdef ENABLE_GUILD_WAR_BEGIN
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 127 * 100, 102 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 120 * 100, 109 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 115 * 100, 114 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 109 * 100, 120 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 103 * 100, 126 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 99 * 100, 130 * 100 + 0, 2, false, 225, true);

	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 156 * 100, 129 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 151 * 100, 134 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 146 * 100, 139 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 141 * 100, 144 * 100 + 0, 2, false, 225, true);
	CHARACTER_MANAGER::instance().SpawnMob(20387, GetMapIndex(), 134 * 100, 151 * 100 + 0, 2, false, 225, true);
#endif
}

CWarMap::~CWarMap()
{
    event_cancel(&m_pkBeginEvent);
#ifdef ENABLE_GUILD_WAR_BEGIN
    event_cancel(&m_pkBeginProtectionEvent);
#endif
    event_cancel(&m_pkEndEvent);
    event_cancel(&m_pkTimeoutEvent);
    event_cancel(&m_pkResetFlagEvent);

    SPDLOG_TRACE("WarMap::~WarMap : map index {0}", GetMapIndex());

    auto it = m_set_pkChr.begin();
    while (it != m_set_pkChr.end()) {
        CHARACTER* ch = *(it++);

        if (ch->GetDesc()) {
            SPDLOG_TRACE("WarMap::~WarMap : disconnecting {0}", ch->GetName());
            DESC_MANAGER::instance().DestroyDesc(ch->GetDesc());
        }
    }

    m_set_pkChr.clear();
}

void CWarMap::SetBeginEvent(LPEVENT pkEv)
{
    if (m_pkBeginEvent != nullptr) {
        event_cancel(&m_pkBeginEvent);
    }
    if (pkEv != nullptr) {
        m_pkBeginEvent = pkEv;
    }
}

#ifdef ENABLE_GUILD_WAR_BEGIN
void CWarMap::SetBeginProtectionEvent(LPEVENT pkEv)
{
    if (m_pkBeginProtectionEvent != NULL) {
        event_cancel(&m_pkBeginProtectionEvent);
    }
    if (pkEv != NULL) {
        m_pkBeginProtectionEvent = pkEv;
    }
}
#endif

void CWarMap::SetEndEvent(LPEVENT pkEv)
{
    if (m_pkEndEvent != nullptr) {
        event_cancel(&m_pkEndEvent);
    }
    if (pkEv != nullptr) {
        m_pkEndEvent = pkEv;
    }
}

void CWarMap::SetTimeoutEvent(LPEVENT pkEv)
{
    if (m_pkTimeoutEvent != nullptr) {
        event_cancel(&m_pkTimeoutEvent);
    }
    if (pkEv != nullptr) {
        m_pkTimeoutEvent = pkEv;
    }
}

void CWarMap::SetResetFlagEvent(LPEVENT pkEv)
{
    if (m_pkResetFlagEvent != nullptr) {
        event_cancel(&m_pkResetFlagEvent);
    }
    if (pkEv != nullptr) {
        m_pkResetFlagEvent = pkEv;
    }
}

bool CWarMap::GetTeamIndex(uint32_t dwGuildID, uint8_t& bIdx)
{
    if (m_TeamData[0].dwID == dwGuildID) {
        bIdx = 0;
        return true;
    } else if (m_TeamData[1].dwID == dwGuildID) {
        bIdx = 1;
        return true;
    }

    return false;
}

uint32_t CWarMap::GetGuildID(uint8_t bIdx)
{
    assert(bIdx < 2);
    return m_TeamData[bIdx].dwID;
}

#ifdef ENABLE_NEW_GUILD_WAR
int CWarMap::GetCurrentPlayer(BYTE bIdx)
{
    assert(bIdx < 2);
    return m_TeamData[bIdx].iMemberCount;
}
#endif

CGuild* CWarMap::GetGuild(uint8_t bIdx)
{
    return m_TeamData[bIdx].pkGuild;
}

int32_t CWarMap::GetMapIndex()
{
    return m_kMapInfo.lMapIndex;
}

uint8_t CWarMap::GetType()
{
    return m_kMapInfo.bType;
}

uint32_t CWarMap::GetGuildOpponent(CHARACTER* ch)
{
    if (ch->GetGuild()) {
        uint32_t gid = ch->GetGuild()->GetID();
        uint8_t idx;

        if (GetTeamIndex(gid, idx))
            return m_TeamData[!idx].dwID;
    }
    return 0;
}

uint32_t CWarMap::GetWinnerGuild()
{
    uint32_t win_gid = 0;

    if (m_TeamData[1].iScore > m_TeamData[0].iScore) {
        win_gid = m_TeamData[1].dwID;
    } else if (m_TeamData[0].iScore > m_TeamData[1].iScore) {
        win_gid = m_TeamData[0].dwID;
    }

    return (win_gid);
}

void CWarMap::UsePotion(CHARACTER* ch, CItem* item)
{
    if (m_pkEndEvent)
        return;

    if (ch->IsObserverMode())
        return;

    if (!ch->GetGuild())
        return;

    if (!item->GetProto())
        return;

    auto price = item->GetProto()->dwShopBuyPrice;
    uint32_t gid = ch->GetGuild()->GetID();

    if (gid == m_TeamData[0].dwID)
        m_TeamData[0].iUsePotionPrice += price;
    else if (gid == m_TeamData[1].dwID)
        m_TeamData[1].iUsePotionPrice += price;
}

int CWarMap::STeamData::GetAccumulatedJoinerCount()
{
    return set_pidJoiner.size();
}

int CWarMap::STeamData::GetCurJointerCount()
{
    return iMemberCount;
}

void CWarMap::STeamData::AppendMember(CHARACTER* ch)
{
    set_pidJoiner.insert(ch->GetPlayerID());
    ++iMemberCount;
}

void CWarMap::STeamData::RemoveMember(CHARACTER* ch)
{
    // set_pidJoiner 는 누적 인원을 계산하기 때문에 제거하지 않는다
    --iMemberCount;
}

struct FSendUserCount {
    char buf1[30];
    char buf2[128];

    FSendUserCount(uint32_t g1, int g1_count, uint32_t g2, int g2_count,
                   int observer)
    {
        std::snprintf(buf1, sizeof(buf1), "ObserverCount %d", observer);
        std::snprintf(buf2, sizeof(buf2), "WarUC %u %d %u %d %d", g1, g1_count,
                      g2, g2_count, observer);
    }

    void operator()(CHARACTER* ch)
    {
        ch->ChatPacket(CHAT_TYPE_COMMAND, buf1);
        ch->ChatPacket(CHAT_TYPE_COMMAND, buf2);
    }
};

void CWarMap::UpdateUserCount()
{
    FSendUserCount f(
        m_TeamData[0].dwID, m_TeamData[0].GetAccumulatedJoinerCount(),
        m_TeamData[1].dwID, m_TeamData[1].GetAccumulatedJoinerCount(),
        m_iObserverCount);

    std::for_each(m_set_pkChr.begin(), m_set_pkChr.end(), f);
}

CWarMap::TMemberStats* CWarMap::RegisterMemberStats(CHARACTER* ch)
{
    uint32_t dwPlayerID = ch->GetPlayerID();

    CWarMap::TMemberStats* pStats = new CWarMap::TMemberStats;
    pStats->strPlayerName = ch->GetName();
    pStats->byLevel = ch->GetLevel();

    CGuild* pGuild = ch->GetGuild();

    if (pGuild)
        pStats->dwGuildId = pGuild->GetID();

    boost::algorithm::replace_all(pStats->strPlayerName, " ", "_");
    map_MemberStats.insert(
        CWarMap::TMemberStatsMap::value_type(dwPlayerID, pStats));

    return pStats;
}

CWarMap::TMemberStats* CWarMap::GetMemberStats(CHARACTER* ch)
{
    if (!ch)
        return nullptr;
    CWarMap::TMemberStats* pStats;

    uint32_t dwPlayerID = ch->GetPlayerID();
    CWarMap::TMemberStatsMap::iterator entry = map_MemberStats.find(dwPlayerID);

    if (entry == map_MemberStats.end())
        pStats = RegisterMemberStats(ch);
    else
        pStats = entry->second;

    return pStats;
}

namespace
{
struct FScoreBoard {
    FScoreBoard(CWarMap::TMemberStats* pStats)
        : m_pStats(pStats)
    {
    }

    void operator()(CHARACTER* ch)
    {
        if (!ch || !m_pStats)
            return;

        ch->ChatPacket(CHAT_TYPE_COMMAND, "warboard update|%u|%s|%u|%u|%d|%llu",
                       m_pStats->dwGuildId, m_pStats->strPlayerName.c_str(),
                       m_pStats->dwKills, m_pStats->dwDeaths, m_pStats->byLevel,
                       m_pStats->ullDamage);
    }

    CWarMap::TMemberStats* m_pStats;
};
} // namespace

void CWarMap::SendStats(CWarMap::TMemberStats* pStats)
{
    FScoreBoard f(pStats);
    std::for_each(m_set_pkChr.begin(), m_set_pkChr.end(), f);
}

void CWarMap::SendStatusBoard(CHARACTER* ch)
{
    if (!ch)
        return;

    for (CWarMap::TMemberStatsMap::iterator it = map_MemberStats.begin();
         it != map_MemberStats.end(); ++it) {
        CWarMap::TMemberStats* pStats = it->second;

        if (!pStats)
            continue;

        ch->ChatPacket(CHAT_TYPE_COMMAND, "warboard update|%u|%s|%u|%u|%d|%llu",
                       pStats->dwGuildId, pStats->strPlayerName.c_str(),
                       pStats->dwKills, pStats->dwDeaths, pStats->byLevel,
                       pStats->ullDamage);
    }
}

void CWarMap::IncMember(CHARACTER* ch)
{
    if (!ch->IsPC())
        return;

    SPDLOG_TRACE("WarMap::IncMember");
    uint32_t gid = 0;

    if (ch->GetGuild())
        gid = ch->GetGuild()->GetID();

    bool isWarMember = ch->GetQuestFlag("war.is_war_member") > 0 ? true : false;
    if (isWarMember && gid != m_TeamData[0].dwID && gid != m_TeamData[1].dwID) {
        ch->SetQuestFlag("war.is_war_member", 0);
        isWarMember = false;
    }

    ch->ChatPacket(CHAT_TYPE_COMMAND, "warboard versus|%u|%s|%u|%s",
                   m_TeamData[0].dwID, m_TeamData[0].pkGuild->GetName(),
                   m_TeamData[1].dwID, m_TeamData[1].pkGuild->GetName());

    if (isWarMember) {
#ifdef ENABLE_GUILD_WAR_BEGIN
        war_map_info* info = AllocEventInfo<war_map_info>();
        info->pWarMap = this;
        if (m_set_pkChr.empty()) {
            m_dwFirstLogonTime = get_dword_time();
            SetBeginProtectionEvent(event_create(
                war_begin_protection_event, info, THECORE_SECS_TO_PASSES(10)));
        }
#endif
        if (gid == m_TeamData[0].dwID) {
            m_TeamData[0].AppendMember(ch);
        } else if (gid == m_TeamData[1].dwID) {
            m_TeamData[1].AppendMember(ch);
        }

        event_cancel(&m_pkTimeoutEvent);

        SPDLOG_TRACE("WarMap +m {0}(cur:{1}, acc:{2}) vs {3}(cur:{4}, acc:{5})",
                     m_TeamData[0].dwID, m_TeamData[0].GetCurJointerCount(),
                     m_TeamData[0].GetAccumulatedJoinerCount(),
                     m_TeamData[1].dwID, m_TeamData[1].GetCurJointerCount(),
                     m_TeamData[1].GetAccumulatedJoinerCount());
    } else {
        ++m_iObserverCount;
        SPDLOG_TRACE("WarMap +o {0}", m_iObserverCount);
        ch->SetObserverMode(true);
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "관전 모드로 길드전에 참가하셨습니다.");
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "자신을 선택하시면 밖으로 나갈 수 있는 <관람 종료> "
                           "버튼이 나옵니다.");
    }

    UpdateUserCount();

    m_set_pkChr.insert(ch);

    if (isWarMember) {
        CWarMap::TMemberStats* pStats = RegisterMemberStats(ch);
        SendStats(pStats);
    }

    SendStatusBoard(ch);

    DESC* d = ch->GetDesc();
    ch->ChatPacket(CHAT_TYPE_COMMAND, "warboard toggle|1");
    SendWarPacket(d);
    SendScorePacket(0, d);
    SendScorePacket(1, d);
}

void CWarMap::DecMember(CHARACTER* ch)
{
    if (!ch->IsPC())
        return;

    SPDLOG_TRACE("WarMap::DecMember");
    uint32_t gid = 0;

    if (ch->GetGuild())
        gid = ch->GetGuild()->GetID();

    if (!ch->IsObserverMode()) {
        if (gid == m_TeamData[0].dwID)
            m_TeamData[0].RemoveMember(ch);
        else if (gid == m_TeamData[1].dwID)
            m_TeamData[1].RemoveMember(ch);

        if (m_kMapInfo.bType == WAR_MAP_TYPE_FLAG) {
            auto aff = ch->FindAffect(AFFECT_WAR_FLAG_1);
            if (!aff)
                aff = ch->FindAffect(AFFECT_WAR_FLAG_2);
            if (!aff)
                aff = ch->FindAffect(AFFECT_WAR_FLAG_3);

            if (aff) {
                uint8_t idx;
                if (GetTeamIndex(aff->pointValue, idx))
                    AddFlag(idx, ch->GetX(), ch->GetY());

                ch->RemoveAffect(*aff);
            }
        }

        SPDLOG_TRACE("WarMap -m {0}(cur:{1}, acc:{2}) vs {3}(cur:{4}, acc:{5})",
                     m_TeamData[0].dwID, m_TeamData[0].GetCurJointerCount(),
                     m_TeamData[0].GetAccumulatedJoinerCount(),
                     m_TeamData[1].dwID, m_TeamData[1].GetCurJointerCount(),
                     m_TeamData[1].GetAccumulatedJoinerCount());

        CheckWarEnd();
        ch->RemoveAffect(GUILD_SKILL_BLOOD);
        ch->SetQuestFlag("war.is_war_member", 0);
    } else {
        --m_iObserverCount;

        SPDLOG_TRACE("WarMap -o {0}", m_iObserverCount);
        ch->SetObserverMode(false);
    }

    UpdateUserCount();

    m_set_pkChr.erase(ch);
}

#ifdef ENABLE_GUILD_WAR_BEGIN
DWORD CWarMap::GetStartTime()
{
    return m_dwFirstLogonTime;
}

bool CWarMap::GetBeginProtectionStatus()
{
    return m_bBeginProtectionCompleted;
}

void CWarMap::SetBeginProtectionStatus(bool bNewStat)
{
    m_bBeginProtectionCompleted = bNewStat;
}
#endif

struct FExitGuildWar {
    void operator()(CHARACTER* ch)
    {
        if (ch->IsPC()) {
            ch->ExitToSavedLocation();
        }
    }
};

void CWarMap::ExitAll()
{
    FExitGuildWar f;
    std::for_each(m_set_pkChr.begin(), m_set_pkChr.end(), f);
}

void CWarMap::CheckWarEnd()
{
    if (m_bEnded)
        return;

    if (m_TeamData[0].iMemberCount == 0 || m_TeamData[1].iMemberCount == 0) {
        if (m_bTimeout)
            return;

        if (m_pkTimeoutEvent)
            return;

        Notice("길드전에 참가한 상대방 길드원이 아무도 없습니다.");
        Notice("1분 이내에 아무도 접속하지 않으면 길드전이 자동 종료됩니다.");

        SPDLOG_TRACE("CheckWarEnd: Timeout begin {0} vs {1}",
                     m_TeamData[0].dwID, m_TeamData[1].dwID);

        war_map_info* info = AllocEventInfo<war_map_info>();
        info->pWarMap = this;

        SetTimeoutEvent(
            event_create(war_timeout_event, info, THECORE_SECS_TO_PASSES(60)));
    } else
        CheckScore();
}

int CWarMap::GetRewardGold(uint8_t bWinnerIdx)
{
    int iRewardGold = m_WarInfo.iWarPrice;
    iRewardGold += (m_TeamData[bWinnerIdx].iUsePotionPrice *
                    m_WarInfo.iWinnerPotionRewardPctToWinner) /
                   100;
    iRewardGold += (m_TeamData[bWinnerIdx ? 0 : 1].iUsePotionPrice *
                    m_WarInfo.iLoserPotionRewardPctToWinner) /
                   100;
    return iRewardGold;
}

void CWarMap::Draw()
{
    CGuildManager::instance().RequestWarOver(m_TeamData[0].dwID,
                                             m_TeamData[1].dwID, 0, 0);
}

void CWarMap::Timeout()
{
    SetTimeoutEvent(nullptr);

    if (m_bTimeout)
        return;

    if (m_bEnded)
        return;

    uint32_t dwWinner = 0;
    uint32_t dwLoser = 0;
    int iRewardGold = 0;

    if (get_dword_time() - m_dwStartTime < 60000 * 5) {
        Notice("길드전이 일찍 종료되어 무승부로 판정 되었습니다. (5분이 지나지 "
               "않음)");
        dwWinner = 0;
        dwLoser = 0;
    } else {
        int iWinnerIdx = -1;

        if (m_TeamData[0].iMemberCount == 0)
            iWinnerIdx = 1;
        else if (m_TeamData[1].iMemberCount == 0)
            iWinnerIdx = 0;

        if (iWinnerIdx == -1) {
            dwWinner = GetWinnerGuild();

            if (dwWinner == m_TeamData[0].dwID) {
                iRewardGold = GetRewardGold(0);
                dwLoser = m_TeamData[1].dwID;
            } else if (dwWinner == m_TeamData[1].dwID) {
                iRewardGold = GetRewardGold(1);
                dwLoser = m_TeamData[0].dwID;
            }

            SPDLOG_ERROR("WarMap: member count is not zero, guild1 {0} {1} "
                         "guild2 {2} {3}, winner {4}",
                         m_TeamData[0].dwID, m_TeamData[0].iMemberCount,
                         m_TeamData[1].dwID, m_TeamData[1].iMemberCount,
                         dwWinner);
        } else {
            dwWinner = m_TeamData[iWinnerIdx].dwID;
            dwLoser = m_TeamData[iWinnerIdx == 0 ? 1 : 0].dwID;

            iRewardGold = GetRewardGold(iWinnerIdx);
        }
    }

    SPDLOG_TRACE("WarMap: Timeout {0} {1} winner {2} loser {3} reward {4} map "
                 "{5}",
                 m_TeamData[0].dwID, m_TeamData[1].dwID, dwWinner, dwLoser,
                 iRewardGold, m_kMapInfo.lMapIndex);

    if (dwWinner)
        CGuildManager::instance().RequestWarOver(dwWinner, dwLoser, dwWinner,
                                                 iRewardGold);
    else
        CGuildManager::instance().RequestWarOver(
            m_TeamData[0].dwID, m_TeamData[1].dwID, dwWinner, iRewardGold);

    m_bTimeout = true;
}

void CWarMap::SendWarPacket(DESC* d)
{
    TPacketGCGuild pack;
    TPacketGCGuildWar pack2;
    pack.subheader = GUILD_SUBHEADER_GC_WAR;

    pack2.dwGuildSelf = m_TeamData[0].dwID;
    pack2.dwGuildOpp = m_TeamData[1].dwID;
    pack2.bType = CGuildManager::instance()
                      .TouchGuild(m_TeamData[0].dwID)
                      ->GetGuildWarType(m_TeamData[1].dwID);
    pack2.bWarState = CGuildManager::instance()
                          .TouchGuild(m_TeamData[0].dwID)
                          ->GetGuildWarState(m_TeamData[1].dwID);
    pack.guildWar = pack2;

    d->Send(HEADER_GC_GUILD, pack);
}

void CWarMap::SendScorePacket(uint8_t bIdx, DESC* d)
{
    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_WAR_SCORE;

    TPacketGuildWarPoint p2;
    p2.dwGainGuildID = m_TeamData[bIdx].dwID;
    p2.dwOpponentGuildID = m_TeamData[bIdx ? 0 : 1].dwID;
    p2.lPoint = m_TeamData[bIdx].iScore;

    p.guildWarPoint = p2;

    if (d)
        d->Send(HEADER_GC_GUILD, p);
    else
        Packet(HEADER_GC_GUILD, p);
}

void CWarMap::UpdateScore(uint32_t g1, int score1, uint32_t g2, int score2)
{
    uint8_t idx;

    if (GetTeamIndex(g1, idx)) {
        if (m_TeamData[idx].iScore != score1) {
            m_TeamData[idx].iScore = score1;
            SendScorePacket(idx);
        }
    }

    if (GetTeamIndex(g2, idx)) {
        if (m_TeamData[idx].iScore != score2) {
            m_TeamData[idx].iScore = score2;
            SendScorePacket(idx);
        }
    }

    CheckScore();
}

bool CWarMap::CheckScore()
{
    if (m_bEnded)
        return true;

    // 30초 이후 부터 확인한다.
    if (get_dword_time() - m_dwStartTime < 30000)
        return false;

    // 점수가 같으면 체크하지 않는다.
    if (m_TeamData[0].iScore == m_TeamData[1].iScore)
        return false;

#ifdef ENABLE_NEW_GUILD_WAR
    CGuild* opp_g = CGuildManager::instance().FindGuild(m_TeamData[0].dwID);
    int iEndScore = quest::CQuestManager::instance().GetEventFlagBR(
        "skor", opp_g->GetName());
#else
    int iEndScore = m_WarInfo.iEndScore;
#endif

    uint32_t dwWinner;
    uint32_t dwLoser;

    if (m_TeamData[0].iScore >= iEndScore) {
        dwWinner = m_TeamData[0].dwID;
        dwLoser = m_TeamData[1].dwID;
    } else if (m_TeamData[1].iScore >= iEndScore) {
        dwWinner = m_TeamData[1].dwID;
        dwLoser = m_TeamData[0].dwID;
    } else
        return false;

    int iRewardGold = 0;

    if (dwWinner == m_TeamData[0].dwID)
        iRewardGold = GetRewardGold(0);
    else if (dwWinner == m_TeamData[1].dwID)
        iRewardGold = GetRewardGold(1);

    SPDLOG_TRACE("WarMap::CheckScore end score {0} guild1 {1} score guild2 {2} "
                 "{3} score {4} winner {5} reward {6}",
                 iEndScore, m_TeamData[0].dwID, m_TeamData[0].iScore,
                 m_TeamData[1].dwID, m_TeamData[1].iScore, dwWinner,
                 iRewardGold);

    CGuildManager::instance().RequestWarOver(dwWinner, dwLoser, dwWinner,
                                             iRewardGold);
    return true;
}

bool CWarMap::SetEnded()
{
    SPDLOG_TRACE("WarMap::SetEnded {0}", m_kMapInfo.lMapIndex);

    if (m_pkEndEvent)
        return false;

    if (m_TeamData[0].pkChrFlag) {
        M2_DESTROY_CHARACTER(m_TeamData[0].pkChrFlag);
        m_TeamData[0].pkChrFlag = nullptr;
    }

    if (m_TeamData[0].pkChrFlagBase) {
        M2_DESTROY_CHARACTER(m_TeamData[0].pkChrFlagBase);
        m_TeamData[0].pkChrFlagBase = nullptr;
    }

    if (m_TeamData[1].pkChrFlag) {
        M2_DESTROY_CHARACTER(m_TeamData[1].pkChrFlag);
        m_TeamData[1].pkChrFlag = nullptr;
    }

    if (m_TeamData[1].pkChrFlagBase) {
        M2_DESTROY_CHARACTER(m_TeamData[1].pkChrFlagBase);
        m_TeamData[1].pkChrFlagBase = nullptr;
    }

    event_cancel(&m_pkResetFlagEvent);
    m_bEnded = true;

    war_map_info* info = AllocEventInfo<war_map_info>();

    info->pWarMap = this;
    info->iStep = 0;
    SetEndEvent(event_create(war_end_event, info, THECORE_SECS_TO_PASSES(10)));
    return true;
}

void CWarMap::OnKill(CHARACTER* killer, CHARACTER* ch)
{
    if (m_bEnded)
        return;

    uint32_t dwKillerGuild = 0;
    uint32_t dwDeadGuild = 0;

    if (killer->GetGuild())
        dwKillerGuild = killer->GetGuild()->GetID();

    if (ch->GetGuild())
        dwDeadGuild = ch->GetGuild()->GetID();

    uint8_t idx;

    SPDLOG_TRACE("WarMap::OnKill {0} {1}", dwKillerGuild, dwDeadGuild);

    if (!GetTeamIndex(dwKillerGuild, idx))
        return;

    if (!GetTeamIndex(dwDeadGuild, idx))
        return;

    if (killer) {
        TMemberStats* pStats = GetMemberStats(killer);
        pStats->dwKills++;
        SendStats(pStats);
    }

    if (ch) {
        TMemberStats* pStats = GetMemberStats(ch);
        pStats->dwDeaths++;
        SendStats(pStats);
    }

    ++m_killCount;

    switch (m_kMapInfo.bType) {
        case WAR_MAP_TYPE_NORMAL:
            if (m_killCount == 1) {
                m_firstBloodPid = killer->GetPlayerID();
                Notice(fmt::format("{} ilk kanı döktü.", killer->GetName())
                           .c_str());
            }

            SendGuildWarScore(dwKillerGuild, dwDeadGuild, 1, ch->GetLevel());
            break;

        case WAR_MAP_TYPE_FLAG: {
            auto aff = ch->FindAffect(AFFECT_WAR_FLAG_1);
            if (!aff)
                aff = ch->FindAffect(AFFECT_WAR_FLAG_2);
            if (!aff)
                aff = ch->FindAffect(AFFECT_WAR_FLAG_3);

            if (aff) {
                if (GetTeamIndex(aff->pointValue, idx))
                    AddFlag(idx, ch->GetX(), ch->GetY());

                ch->RemoveAffect(*aff);
            }
        } break;

        default:
            SPDLOG_ERROR("unknown war map type {0} index {1}", m_kMapInfo.bType,
                         m_kMapInfo.lMapIndex);
            break;
    }
}

void CWarMap::AddFlagBase(uint8_t bIdx, uint32_t x, uint32_t y)
{
    if (m_bEnded)
        return;

    assert(bIdx < 2);

    TeamData& r = m_TeamData[bIdx];

    if (r.pkChrFlagBase)
        return;

    if (x == 0) {
        x = m_kMapInfo.posStart[bIdx].x;
        y = m_kMapInfo.posStart[bIdx].y;
    }

    r.pkChrFlagBase = g_pCharManager->SpawnMob(warmap::WAR_FLAG_BASE_VNUM,
                                               m_kMapInfo.lMapIndex, x, y, 0);

    if (r.pkChrFlagBase) {
        r.pkChrFlagBase->SetPoint(POINT_STAT, r.dwID);
        r.pkChrFlagBase->SetWarMap(this);
    }
}

void CWarMap::AddFlag(uint8_t bIdx, uint32_t x, uint32_t y)
{
    if (m_bEnded)
        return;

    assert(bIdx < 2);

    TeamData& r = m_TeamData[bIdx];

    if (r.pkChrFlag)
        return;

    if (x == 0) {
        x = m_kMapInfo.posStart[bIdx].x;
        y = m_kMapInfo.posStart[bIdx].y;
    }

    r.pkChrFlag = g_pCharManager->SpawnMob(bIdx == 0 ? warmap::WAR_FLAG_VNUM0
                                                     : warmap::WAR_FLAG_VNUM1,
                                           m_kMapInfo.lMapIndex, x, y, 0);
    if (r.pkChrFlag) {
        r.pkChrFlag->SetPoint(POINT_STAT, r.dwID);
        r.pkChrFlag->SetWarMap(this);
    }
}

void CWarMap::RemoveFlag(uint8_t bIdx)
{
    assert(bIdx < 2);

    TeamData& r = m_TeamData[bIdx];

    if (!r.pkChrFlag)
        return;

    SPDLOG_TRACE("WarMap::RemoveFlag {0} {1:p}", bIdx, (void*)r.pkChrFlag);

    r.pkChrFlag->Dead(nullptr, true);
    r.pkChrFlag = nullptr;
}

bool CWarMap::IsFlagOnBase(uint8_t bIdx)
{
    assert(bIdx < 2);

    TeamData& r = m_TeamData[bIdx];

    if (!r.pkChrFlag)
        return false;

    const PIXEL_POSITION& pos = r.pkChrFlag->GetXYZ();

    if (pos.x == m_kMapInfo.posStart[bIdx].x &&
        pos.y == m_kMapInfo.posStart[bIdx].y)
        return true;

    return false;
}

EVENTFUNC(war_reset_flag_event)
{
    auto info = static_cast<war_map_info*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("war_reset_flag_event> <Factor> Null pointer");
        return 0;
    }

    CWarMap* pMap = info->pWarMap;

    pMap->AddFlag(0);
    pMap->AddFlag(1);

    pMap->SetResetFlagEvent(nullptr);
    return 0;
}

struct FRemoveFlagAffect {
    void operator()(CHARACTER* ch)
    {
        ch->RemoveAffect(AFFECT_WAR_FLAG_1);
        ch->RemoveAffect(AFFECT_WAR_FLAG_2);
        ch->RemoveAffect(AFFECT_WAR_FLAG_3);
    }
};

void CWarMap::ResetFlag()
{
    if (m_kMapInfo.bType != WAR_MAP_TYPE_FLAG)
        return;

    if (m_pkResetFlagEvent)
        return;

    if (m_bEnded)
        return;

    FRemoveFlagAffect f;
    std::for_each(m_set_pkChr.begin(), m_set_pkChr.end(), f);

    RemoveFlag(0);
    RemoveFlag(1);

    war_map_info* info = AllocEventInfo<war_map_info>();

    info->pWarMap = this;
    info->iStep = 0;
    SetResetFlagEvent(
        event_create(war_reset_flag_event, info, THECORE_SECS_TO_PASSES(10)));
}

/////////////////////////////////////////////////////////////////////////////////
// WarMapManager
/////////////////////////////////////////////////////////////////////////////////
CWarMapManager::CWarMapManager()
{
    TWarMapInfo* k;

    k = new TWarMapInfo;
    k->bType = WAR_MAP_TYPE_NORMAL;

    k->lMapIndex = 110;
    k->posStart[0].x = 48 * 100;
    k->posStart[0].y = 52 * 100;
    k->posStart[1].x = 183 * 100;
    k->posStart[1].y = 206 * 100;
    k->posStart[2].x = 141 * 100;
    k->posStart[2].y = 117 * 100;

    m_map_kWarMapInfo.emplace(k->lMapIndex, k);

    k = new TWarMapInfo;
    k->bType = WAR_MAP_TYPE_FLAG;

    k->lMapIndex = 111;
    k->posStart[0].x = 68 * 100;
    k->posStart[0].y = 69 * 100;
    k->posStart[1].x = 171 * 100;
    k->posStart[1].y = 182 * 100;
    k->posStart[2].x = 122 * 100;
    k->posStart[2].y = 131 * 100;

    m_map_kWarMapInfo.emplace(k->lMapIndex, k);
}

CWarMapManager::~CWarMapManager()
{
    for (auto iter = m_map_kWarMapInfo.begin(); iter != m_map_kWarMapInfo.end();
         ++iter) {
        delete iter->second;
    }

    m_map_kWarMapInfo.clear();
}

bool CWarMapManager::IsWarMap(int32_t lMapIndex)
{
    return GetWarMapInfo(lMapIndex) ? true : false;
}

bool CWarMapManager::GetStartPosition(int32_t lMapIndex, uint8_t bIdx,
                                      PIXEL_POSITION& pos)
{
    assert(bIdx < 3);

    TWarMapInfo* pi = GetWarMapInfo(lMapIndex);

    if (!pi) {
        SPDLOG_TRACE("GetStartPosition FAILED [{0}] WarMapInfoSize({1})",
                     lMapIndex, m_map_kWarMapInfo.size());

        for (auto it = m_map_kWarMapInfo.begin(); it != m_map_kWarMapInfo.end();
             ++it) {
            PIXEL_POSITION& cur = it->second->posStart[bIdx];
            SPDLOG_TRACE("WarMap[{0}]=Pos({1}, {2})", it->first, cur.x, cur.y);
        }

        return false;
    }

    pos = pi->posStart[bIdx];
    return true;
}

int32_t CWarMapManager::CreateWarMap(const TGuildWarInfo& guildWarInfo,
                                     uint32_t dwGuildID1, uint32_t dwGuildID2)
{
    TWarMapInfo* pkInfo = GetWarMapInfo(guildWarInfo.lMapIndex);
    if (!pkInfo) {
        SPDLOG_ERROR("GuildWar.CreateWarMap.NOT_FOUND_MAPINFO[{0}]",
                     guildWarInfo.lMapIndex);
        return 0;
    }

    uint32_t lMapIndex =
        SECTREE_MANAGER::instance().CreatePrivateMap(guildWarInfo.lMapIndex);

    if (lMapIndex) {
        m_mapWarMap.emplace(lMapIndex,
                            new CWarMap(lMapIndex, guildWarInfo, pkInfo,
                                        dwGuildID1, dwGuildID2));
    }

    return lMapIndex;
}

TWarMapInfo* CWarMapManager::GetWarMapInfo(int32_t lMapIndex)
{
    if (lMapIndex >= 10000)
        lMapIndex /= 10000;

    auto it = m_map_kWarMapInfo.find(lMapIndex);
    if (m_map_kWarMapInfo.end() == it)
        return nullptr;

    return it->second;
}

void CWarMapManager::DestroyWarMap(CWarMap* pMap)
{
    int32_t mapIdx = pMap->GetMapIndex();

    SPDLOG_TRACE("WarMap::DestroyWarMap : {0}", mapIdx);
    auto& members = pMap->GetMembers();
    for (auto* member : members)
        member->SetWarMap(nullptr);

    m_mapWarMap.erase(pMap->GetMapIndex());
    delete pMap;

    SECTREE_MANAGER::instance().DestroyPrivateMap(mapIdx);
}

CWarMap* CWarMapManager::Find(int32_t lMapIndex)
{
    auto it = m_mapWarMap.find(lMapIndex);
    if (it == m_mapWarMap.end())
        return nullptr;

    return it->second;
}

void CWarMapManager::OnShutdown()
{
    auto it = m_mapWarMap.begin();
    while (it != m_mapWarMap.end())
        (it++)->second->Draw();
}
