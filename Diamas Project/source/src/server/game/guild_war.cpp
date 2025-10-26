#include "guild.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "char.h"
#include "GBufferManager.h"
#include "war_map.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "guild_manager.h"
#include "desc_manager.h"
#include "desc.h"

#include "DbCacheSocket.hpp"

#include <game/GamePacket.hpp>
#include <game/MasterPackets.hpp>

//
// Packet
enum
{
    GUILD_WAR_WAIT_START_DURATION = 10 * 60
};

const char *__GetWarType(int n)
{
    switch (n)
    {
    case 0:
        return "Field";
    case 1:
        return "Arena";
    case 2:
        return "Flag";
    default:
        return "Unknown";
    }
}

//
// Packet
//
#ifdef ENABLE_NEW_GUILD_WAR
void CGuild::GuildWarPacket(uint32_t dwOppGID, uint8_t bWarType,
                            uint8_t bWarState)
{

    CGuild* opp_g = CGuildManager::instance().FindGuild(GetID());
    CGuild* g = CGuildManager::instance().FindGuild(dwOppGID);
    int skor =
        quest::CQuestManager::instance().GetEventFlagBR("skor", g->GetName());
    BYTE minseviye = quest::CQuestManager::instance().GetEventFlagBR(
        "minseviye", g->GetName());
    BYTE maxplayer = quest::CQuestManager::instance().GetEventFlagBR(
        "maxplayer", g->GetName());
    BYTE binekkullanimi = quest::CQuestManager::instance().GetEventFlagBR(
        "binekkullanimi", g->GetName());

    BYTE pet =
        quest::CQuestManager::instance().GetEventFlagBR("pet", g->GetName());
    BYTE buffi =
        quest::CQuestManager::instance().GetEventFlagBR("buffi", g->GetName());
    BYTE savasci = quest::CQuestManager::instance().GetEventFlagBR(
        "savasci", g->GetName());
    BYTE ninja =
        quest::CQuestManager::instance().GetEventFlagBR("ninja", g->GetName());
    BYTE sura =
        quest::CQuestManager::instance().GetEventFlagBR("sura", g->GetName());
    BYTE shaman =
        quest::CQuestManager::instance().GetEventFlagBR("shaman", g->GetName());

    TPacketGCGuildWar pack2;

    pack2.dwGuildSelf = GetID();
    pack2.dwGuildOpp = dwOppGID;
    pack2.bWarState = bWarState;
    pack2.bType = bWarType;
    pack2.bScore = skor;
    pack2.bMinLevel = minseviye;
    pack2.bMaxPlayer = maxplayer;
    pack2.bBinekkullanimi = binekkullanimi;
    pack2.savasci = savasci;
    pack2.ninja = ninja;
    pack2.sura = sura;
    pack2.shaman = shaman;

    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_WAR;
    p.guildWar = pack2;

    for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
    CHARACTER* ch = *it;

    if (bWarState == GUILD_WAR_ON_WAR)
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전중에는 사냥에 "
                                               "따른 이익이 없습니다."));

    DESC* d = ch->GetDesc();

    if (d) {
        ch->SendGuildName(dwOppGID);

        d->Send(HEADER_GC_GUILD, p);
    }
    }

    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "skor", opp_g->GetName(), skor);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "minseviye", opp_g->GetName(), minseviye);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "maxplayer", opp_g->GetName(), maxplayer);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "binekkullanimi", opp_g->GetName(), binekkullanimi);
    quest::CQuestManager::instance().RequestSetEventFlagBR("skor", g->GetName(),
                                                           skor);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "minseviye", g->GetName(), minseviye);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "maxplayer", g->GetName(), maxplayer);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "binekkullanimi", g->GetName(), binekkullanimi);

    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "savasci", opp_g->GetName(), savasci);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "ninja", opp_g->GetName(), ninja);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "sura", opp_g->GetName(), sura);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "shaman", opp_g->GetName(), shaman);

    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "savasci", g->GetName(), savasci);
    quest::CQuestManager::instance().RequestSetEventFlagBR("ninja",
                                                           g->GetName(), ninja);
    quest::CQuestManager::instance().RequestSetEventFlagBR("sura", g->GetName(),
                                                           sura);
    quest::CQuestManager::instance().RequestSetEventFlagBR(
        "shaman", g->GetName(), shaman);
}
#else
void CGuild::GuildWarPacket(uint32_t dwOppGID, uint8_t bWarType,
                            uint8_t bWarState)
{
    TPacketGCGuildWar pack2;
    pack2.dwGuildSelf = GetID();
    pack2.dwGuildOpp = dwOppGID;
    pack2.bWarState = bWarState;
    pack2.bType = bWarType;
    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_WAR;

    p.guildWar = pack2;

    for (auto ch : m_memberOnline) {
    if (gConfig.testServer)
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("[TESTSERVER] Receive guild war type {}",
                                   __GetWarType(bWarType)));

    if (bWarState == GUILD_WAR_ON_WAR)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드전중에는 사냥에 따른 이익이 없습니다.");

    DESC* d = ch->GetDesc();

    if (d) {
        ch->SendGuildName(dwOppGID);

        d->Send(HEADER_GC_GUILD, p);
    }
    }
}
#endif

void CGuild::SendEnemyGuild(CHARACTER *ch)
{
    DESC *d = ch->GetDesc();

    if (!d)
        return;

    TPacketGCGuild pack;
    TPacketGCGuildWar pack2;
    pack.subheader = GUILD_SUBHEADER_GC_WAR;
    pack2.dwGuildSelf = GetID();

    TPacketGCGuild pScore;
    pScore.subheader = GUILD_SUBHEADER_GC_WAR_SCORE;

    for (auto it = m_EnemyGuild.begin(); it != m_EnemyGuild.end(); ++it)
    {
        ch->SendGuildName(it->first);

        pack2.dwGuildOpp = it->first;
        pack2.bType = it->second.type;
        pack2.bWarState = it->second.state;

        pack.guildWar = pack2;


        d->Send(HEADER_GC_GUILD, pack);

        if (it->second.state == GUILD_WAR_ON_WAR)
        {
            int32_t lScore;

            lScore = GetWarScoreAgainstTo(pack2.dwGuildOpp);


            TPacketGuildWarPoint s;
            s.dwOpponentGuildID = pack2.dwGuildSelf;
            s.dwGainGuildID = pack2.dwGuildOpp;
            s.lPoint = lScore;
            pScore.guildWarPoint = s;

            d->Send(HEADER_GC_GUILD, pScore);

            lScore = CGuildManager::instance().TouchGuild(pack2.dwGuildOpp)->GetWarScoreAgainstTo(pack2.dwGuildSelf);

            TPacketGuildWarPoint s2;
            s2.dwOpponentGuildID = pack2.dwGuildOpp;
            s2.dwGainGuildID = pack2.dwGuildSelf;
            s2.lPoint = lScore;
            pScore.guildWarPoint = s2;

            d->Send(HEADER_GC_GUILD, pScore);

        }
    }
}

//
// War Login
//
int CGuild::GetGuildWarState(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return GUILD_WAR_NONE;

    auto it = m_EnemyGuild.find(dwOppGID);
    return (it != m_EnemyGuild.end()) ? (it->second.state) : GUILD_WAR_NONE;
}

int CGuild::GetGuildWarType(uint32_t dwOppGID)
{
    auto git = m_EnemyGuild.find(dwOppGID);

    if (git == m_EnemyGuild.end())
        return GUILD_WAR_TYPE_FIELD;

    return git->second.type;
}

uint32_t CGuild::GetGuildWarMapIndex(uint32_t dwOppGID)
{
    auto git = m_EnemyGuild.find(dwOppGID);

    if (git == m_EnemyGuild.end())
        return 0;

    return git->second.map_index;
}

bool CGuild::CanStartWar(uint8_t bGuildWarType) // 타입에 따라 다른 조건이 생길 수도 있음
{
    if (quest::CQuestManager::instance().GetEventFlag("guild_test"))
        return true;

    if (bGuildWarType >= GUILD_WAR_TYPE_MAX_NUM)
        return false;

    if (bGuildWarType == GUILD_WAR_TYPE_FIELD)
        return false;
   

    return GetMemberCount() >= GUILD_WAR_MIN_MEMBER_COUNT;
}

bool CGuild::UnderWar(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return false;

    auto it = m_EnemyGuild.find(dwOppGID);
    return (it != m_EnemyGuild.end()) && (it->second.IsWarBegin());
}

uint32_t CGuild::UnderAnyWar(uint8_t bType)
{
    for (auto it = m_EnemyGuild.begin(); it != m_EnemyGuild.end(); ++it)
    {
        if (bType < GUILD_WAR_TYPE_MAX_NUM)
            if (it->second.type != bType)
                continue;

        if (it->second.IsWarBegin())
            return it->first;
    }

    return 0;
}

void CGuild::SetWarScoreAgainstTo(uint32_t dwOppGID, int iScore)
{
    uint32_t dwSelfGID = GetID();

    SPDLOG_INFO("GuildWarScore Set {0} from {1} {2}", dwSelfGID, dwOppGID, iScore);
    auto it = m_EnemyGuild.find(dwOppGID);

    if (it != m_EnemyGuild.end())
    {
        it->second.score = iScore;

        if (it->second.type != GUILD_WAR_TYPE_FIELD)
        {
            CGuild *gOpp = CGuildManager::instance().TouchGuild(dwOppGID);
            CWarMap *pMap = CWarMapManager::instance().Find(it->second.map_index);

            if (pMap)
                pMap->UpdateScore(dwSelfGID, iScore, dwOppGID, gOpp->GetWarScoreAgainstTo(dwSelfGID));
        }
        else
        {
            TPacketGCGuild p;
            p.subheader = GUILD_SUBHEADER_GC_WAR_SCORE;
            TPacketGuildWarPoint point;
            
            point.dwOpponentGuildID = dwSelfGID;
            point.dwGainGuildID = dwOppGID;
            point.lPoint = iScore;
            p.guildWarPoint = point;

            Packet(HEADER_GC_GUILD, p);

            CGuild *gOpp = CGuildManager::instance().TouchGuild(dwOppGID);

            if (gOpp)
                gOpp->Packet(HEADER_GC_GUILD, p);
        }
    }
}

int CGuild::GetWarScoreAgainstTo(uint32_t dwOppGID)
{
    auto it = m_EnemyGuild.find(dwOppGID);

    if (it != m_EnemyGuild.end())
    {
        SPDLOG_INFO("GuildWarScore Get {0} from {1} {2}", GetID(), dwOppGID, it->second.score);
        return it->second.score;
    }

    SPDLOG_INFO("GuildWarScore Get {0} from {1} No data", GetID(), dwOppGID);
    return 0;
}

uint32_t CGuild::GetWarStartTime(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return 0;

    auto it = m_EnemyGuild.find(dwOppGID);

    if (it == m_EnemyGuild.end())
        return 0;

    return it->second.war_start_time;
}

const TGuildWarInfo &GuildWar_GetTypeInfo(unsigned type) { return KOR_aGuildWarInfo[type]; }

unsigned GuildWar_GetTypeMapIndex(unsigned type) { return GuildWar_GetTypeInfo(type).lMapIndex; }

bool GuildWar_IsWarMap(unsigned type)
{
    if (type == GUILD_WAR_TYPE_FIELD)
        return true;

    unsigned mapIndex = GuildWar_GetTypeMapIndex(type);

    return mapIndex != 0;
    // return SECTREE_MANAGER::instance().GetMapRegion(mapIndex) != nullptr;
}

void CGuild::NotifyGuildMaster(const char *msg)
{
    CHARACTER *ch = GetMasterCharacter();
    if (ch)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, msg);
}

//
// War State Relative
//
//
// A Declare -> B Refuse
//           -> B Declare -> StartWar -> EndWar
//
// A Declare -> B Refuse
//           -> B Declare -> WaitStart -> Fail
//                                     -> StartWar -> EndWar
//
void CGuild::RequestDeclareWar(uint32_t dwOppGID, uint8_t type)
{
    if (dwOppGID == GetID())
    {
        SPDLOG_INFO("GuildWar.DeclareWar.DECLARE_WAR_SELF id({0} -> {1}), type({2})", GetID(), dwOppGID, type);
        return;
    }

    if (type >= GUILD_WAR_TYPE_MAX_NUM)
    {
        SPDLOG_INFO("GuildWar.DeclareWar.UNKNOWN_WAR_TYPE id({0} -> {1}), type({2})", GetID(), dwOppGID, type);
        return;
    }

    auto it = m_EnemyGuild.find(dwOppGID);
    if (it == m_EnemyGuild.end())
    {
        if (!GuildWar_IsWarMap(type))
        {
            SPDLOG_ERROR("GuildWar.DeclareWar.NOT_EXIST_MAP id({0} -> {1}), type({2}), map({3})", GetID(), dwOppGID,
                         type, GuildWar_GetTypeMapIndex(type));

            NotifyGuildMaster("전쟁 서버가 열려있지 않아 길드전을 시작할 수 없습니다.");
            return;
        }

        // 패킷 보내기 to another server
        TPacketGuildWar p;
        p.bType = type;
        p.bWar = GUILD_WAR_SEND_DECLARE;
        p.dwGuildFrom = GetID();
        p.dwGuildTo = dwOppGID;
        p.isRanked = 0;

        db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
        SPDLOG_INFO("GuildWar.DeclareWar id({0} -> {1}), type({2})", GetID(), dwOppGID, type);
        return;
    }

    switch (it->second.state)
    {
    case GUILD_WAR_RECV_DECLARE: {
        const unsigned saved_type = it->second.type;

        if (saved_type == GUILD_WAR_TYPE_FIELD)
        {
            // 선전포고 한것을 받아들였다.
            TPacketGuildWar p;
            p.bType = saved_type;
            p.bWar = GUILD_WAR_ON_WAR;
            p.dwGuildFrom = GetID();
            p.dwGuildTo = dwOppGID;
            p.isRanked = 0;

            db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
            SPDLOG_INFO("GuildWar.AcceptWar id({0} -> {1}), type({2})", GetID(), dwOppGID, saved_type);
            return;
        }

        if (!GuildWar_IsWarMap(saved_type))
        {
            SPDLOG_ERROR("GuildWar.AcceptWar.NOT_EXIST_MAP id({0} -> {1}), type({2}), map({3})", GetID(), dwOppGID,
                         type, GuildWar_GetTypeMapIndex(type));

            NotifyGuildMaster("전쟁 서버가 열려있지 않아 길드전을 시작할 수 없습니다.");
            return;
        }

        const TGuildWarInfo &guildWarInfo = GuildWar_GetTypeInfo(saved_type);

        TPacketGuildWar p;
        p.bType = saved_type;
        p.bWar = GUILD_WAR_WAIT_START;
        p.dwGuildFrom = GetID();
        p.dwGuildTo = dwOppGID;
        p.lInitialScore = guildWarInfo.iInitialScore;
        p.isRanked = 0;

        db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));

        SPDLOG_INFO("GuildWar.WaitStartSendToDB id({0} vs {1}), type({2}), bet({3}), map_index({4})", GetID(),
                    dwOppGID, saved_type, guildWarInfo.iWarPrice, guildWarInfo.lMapIndex);
    }
    break;
    case GUILD_WAR_SEND_DECLARE: { NotifyGuildMaster("이미 선전포고 중인 길드입니다."); }
    break;
    default:
        SPDLOG_ERROR("GuildWar.DeclareWar.UNKNOWN_STATE[{0}]: id({1} vs {2}), type({3}), guild({4}:{5})",
                     it->second.state, GetID(), dwOppGID, type, GetName(), GetID());
        break;
    }
}

bool CGuild::DeclareWar(uint32_t dwOppGID, uint8_t type, uint8_t state)
{
    if (m_EnemyGuild.find(dwOppGID) != m_EnemyGuild.end())
        return false;

    TGuildWar gw(type);
    gw.state = state;

    m_EnemyGuild.emplace(dwOppGID, gw);

    GuildWarPacket(dwOppGID, type, state);
    return true;
}

bool CGuild::CheckStartWar(uint32_t dwOppGID)
{
    auto it = m_EnemyGuild.find(dwOppGID);

    if (it == m_EnemyGuild.end())
        return false;

    TGuildWar &gw(it->second);

    if (gw.state == GUILD_WAR_ON_WAR)
        return false;

    return true;
}

void CGuild::StartWar(uint32_t dwOppGID)
{
    auto it = m_EnemyGuild.find(dwOppGID);

    if (it == m_EnemyGuild.end())
        return;

    TGuildWar &gw(it->second);

    if (gw.state == GUILD_WAR_ON_WAR)
        return;

    gw.state = GUILD_WAR_ON_WAR;
    gw.war_start_time = get_global_time();

    GuildWarPacket(dwOppGID, gw.type, GUILD_WAR_ON_WAR);

    if (gw.type != GUILD_WAR_TYPE_FIELD)
        GuildWarEntryAsk(dwOppGID);
}

bool CGuild::WaitStartWar(uint32_t dwOppGID)
{
    //자기자신이면
    if (dwOppGID == GetID())
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.DECLARE_WAR_SELF id({} -> {})", GetID(), dwOppGID);
        return false;
    }

    //상대방 길드 TGuildWar 를 얻어온다.
    auto it = m_EnemyGuild.find(dwOppGID);
    if (it == m_EnemyGuild.end())
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.UNKNOWN_ENEMY id({} -> {})", GetID(), dwOppGID);
        return false;
    }

    //레퍼런스에 등록하고
    TGuildWar &gw(it->second);

    if (gw.state == GUILD_WAR_WAIT_START)
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.UNKNOWN_STATE id({} -> {}), state({})", GetID(), dwOppGID, gw.state);
        return false;
    }

    //상태를 저장한다.
    gw.state = GUILD_WAR_WAIT_START;

    //상대편의 길드 클래스 포인터를 얻어오고
    CGuild *g = CGuildManager::instance().FindGuild(dwOppGID);
    if (!g)
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.NOT_EXIST_GUILD id({} -> {})", GetID(), dwOppGID);
        return false;
    }

    // GUILDWAR_INFO
    const TGuildWarInfo &rkGuildWarInfo = GuildWar_GetTypeInfo(gw.type);
    // END_OF_GUILDWAR_INFO

    // 필드형이면 맵생성 안함
    if (gw.type == GUILD_WAR_TYPE_FIELD)
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.FIELD_TYPE id({} -> {})", GetID(), dwOppGID);
        return true;
    }

    // 전쟁 서버 인지 확인
    SPDLOG_TRACE("GuildWar.WaitStartWar.CheckWarServer id({} -> {}), type({}), map({})", GetID(), dwOppGID, gw.type,
                 rkGuildWarInfo.lMapIndex);

    if (!gConfig.IsHostingMap(rkGuildWarInfo.lMapIndex))
    {
        SPDLOG_TRACE("GuildWar.WaitStartWar.SKIP_WAR_MAP id({} -> {})", GetID(), dwOppGID);
        return true;
    }

    uint32_t id1 = GetID();
    uint32_t id2 = dwOppGID;

    if (id1 > id2)
        std::swap(id1, id2);

    //워프 맵을 생성
    uint32_t lMapIndex = CWarMapManager::instance().CreateWarMap(rkGuildWarInfo, id1, id2);
    if (!lMapIndex)
    {
        SPDLOG_ERROR("GuildWar.WaitStartWar.CREATE_WARMAP_ERROR id({0} vs {1}), type({2}), map({3})", id1, id2,
                     gw.type, rkGuildWarInfo.lMapIndex);
        CGuildManager::instance().RequestEndWar(GetID(), dwOppGID);
        return false;
    }

    SPDLOG_INFO("GuildWar.WaitStartWar.CreateMap id({0} vs {1}), type({2}), map({3}) -> map_inst({4})", id1, id2,
                gw.type, rkGuildWarInfo.lMapIndex, lMapIndex);

    //길드전 정보에 맵인덱스를 세팅
    gw.map_index = lMapIndex;

    //양쪽에 등록(?)
    SetGuildWarMapIndex(dwOppGID, lMapIndex);
    g->SetGuildWarMapIndex(GetID(), lMapIndex);

    SPDLOG_INFO("Send Master Server packet: gid1 {}  gid2 {} mapIndex {}", id1, id2, lMapIndex);
    ///////////////////////////////////////////////////////
    GmGuildWarZoneInfoPacket p;
    p.gid1 = id1;
    p.gid2 = id2;
    p.mapIndex = lMapIndex;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmGuildWarZoneInfo, p);
    ///////////////////////////////////////////////////////

    return true;
}

void CGuild::RequestRefuseWar(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return;

    auto it = m_EnemyGuild.find(dwOppGID);

    if (it != m_EnemyGuild.end() && it->second.state == GUILD_WAR_RECV_DECLARE)
    {
        // 선전포고를 거절했다.
        TPacketGuildWar p;
        p.bWar = GUILD_WAR_REFUSE;
        p.dwGuildFrom = GetID();
        p.dwGuildTo = dwOppGID;
        p.isRanked = 0;

        db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
    }
}

void CGuild::RefuseWar(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return;

    auto it = m_EnemyGuild.find(dwOppGID);

    if (it != m_EnemyGuild.end() &&
        (it->second.state == GUILD_WAR_SEND_DECLARE || it->second.state == GUILD_WAR_RECV_DECLARE))
    {
        uint8_t type = it->second.type;
        m_EnemyGuild.erase(dwOppGID);

        GuildWarPacket(dwOppGID, type, GUILD_WAR_END);
    }
}

void CGuild::ReserveWar(uint32_t dwOppGID, uint8_t type)
{
    if (dwOppGID == GetID())
        return;

    auto it = m_EnemyGuild.find(dwOppGID);

    if (it == m_EnemyGuild.end())
    {
        TGuildWar gw(type);
        gw.state = GUILD_WAR_RESERVE;
        m_EnemyGuild.emplace(dwOppGID, gw);
    }
    else
        it->second.state = GUILD_WAR_RESERVE;

    SPDLOG_INFO("Guild::ReserveWar {0}", dwOppGID);
}

void CGuild::EndWar(uint32_t dwOppGID)
{
    if (dwOppGID == GetID())
        return;

    auto it = m_EnemyGuild.find(dwOppGID);

    if (it != m_EnemyGuild.end())
    {
        CWarMap *pMap = CWarMapManager::instance().Find(it->second.map_index);

        if (pMap)
            pMap->SetEnded();

        GuildWarPacket(dwOppGID, it->second.type, GUILD_WAR_END);
        m_EnemyGuild.erase(it);

        if (!UnderAnyWar())
        {
            for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
            {
                CHARACTER *ch = *it;
                ch->RemoveAffect(GUILD_SKILL_BLOOD);
                ch->RemoveAffect(GUILD_SKILL_BLESS);
                ch->RemoveAffect(GUILD_SKILL_SEONGHWI);
                ch->RemoveAffect(GUILD_SKILL_ACCEL);
                ch->RemoveAffect(GUILD_SKILL_BUNNO);
                ch->RemoveAffect(GUILD_SKILL_JUMUN);

                ch->RemoveBadAffect();
            }
        }
    }
}

void CGuild::SetGuildWarMapIndex(uint32_t dwOppGID, int32_t lMapIndex)
{
    auto it = m_EnemyGuild.find(dwOppGID);

    if (it == m_EnemyGuild.end())
        return;

    it->second.map_index = lMapIndex;
    SPDLOG_INFO("GuildWar.SetGuildWarMapIndex id({0} -> {1}), map({2})", GetID(), dwOppGID, lMapIndex);
}

void CGuild::GuildWarEntryAccept(uint32_t dwOppGID, CHARACTER *ch)
{
    auto git = m_EnemyGuild.find(dwOppGID);

    if (git == m_EnemyGuild.end())
        return;

    TGuildWar &gw(git->second);

    if (gw.type == GUILD_WAR_TYPE_FIELD)
        return;

    if (gw.state != GUILD_WAR_ON_WAR)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이미 전쟁이 끝났습니다.");
        return;
    }

    #ifdef ENABLE_NEW_GUILD_WAR
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "minseviye", ch->GetGuild()->GetName()) > ch->GetLevel()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Maalesef savasa girmek icin leveliniz yetersiz !");
        return;
    }

    if (quest::CQuestManager::instance().GetEventFlagBR(
            "suankiplayer", ch->GetGuild()->GetName()) >=
        quest::CQuestManager::instance().GetEventFlagBR(
            "maxplayer", ch->GetGuild()->GetName())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Maalesef savastaki oyuncu sayisi full durumda !");
        return;
    }

    if (quest::CQuestManager::instance().GetEventFlagBR(
            "savasci", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 0) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Bu savasa savasci karakterler katilamaz.");
        return;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "ninja", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 1) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Bu savasa ninja karakterler katilamaz.");
        return;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "sura", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 2) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Bu savasa sura karakterler katilamaz.");
        return;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "shaman", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 3) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Bu savasa shaman karakterler katilamaz.");
        return;
    }
#endif

    if (!gw.map_index)
        return;

    PIXEL_POSITION pos;

    if (!CWarMapManager::instance().GetStartPosition(gw.map_index, GetID() < dwOppGID ? 0 : 1, pos))
        return;

    quest::PC *pPC = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
    pPC->SetFlag("war.is_war_member", 1);

    ch->SaveExitLocation();
    ch->WarpSet(gw.map_index, pos.x, pos.y);
}

void CGuild::GuildWarEntryAsk(uint32_t dwOppGID)
{
    auto git = m_EnemyGuild.find(dwOppGID);
    if (git == m_EnemyGuild.end())
    {
        SPDLOG_ERROR("GuildWar.GuildWarEntryAsk.UNKNOWN_ENEMY({0})", dwOppGID);
        return;
    }

    TGuildWar &gw(git->second);

    SPDLOG_INFO("GuildWar.GuildWarEntryAsk id({0} vs {1}), map({2})", GetID(), dwOppGID, gw.map_index);
    if (!gw.map_index)
    {
        SPDLOG_ERROR("GuildWar.GuildWarEntryAsk.NOT_EXIST_MAP id({0} vs {1})", GetID(), dwOppGID);
        return;
    }

    PIXEL_POSITION pos;
    if (!CWarMapManager::instance().GetStartPosition(gw.map_index, GetID() < dwOppGID ? 0 : 1, pos))
    {
        SPDLOG_ERROR("GuildWar.GuildWarEntryAsk.START_POSITION_ERROR id({0} vs {1}), pos({2}, {3})", GetID(), dwOppGID,
                     pos.x, pos.y);
        return;
    }

    SPDLOG_INFO("GuildWar.GuildWarEntryAsk.OnlineMemberCount({0})", m_memberOnline.size());

    auto it = m_memberOnline.begin();

    while (it != m_memberOnline.end())
    {
        CHARACTER *ch = *it++;
        ch->ChatPacket(CHAT_TYPE_COMMAND, "guild_war %d", ch->GetGuild()->UnderAnyWar() != 0);
    }
}

//
// LADDER POINT
//
void CGuild::SetLadderPoint(int point)
{
    if (m_data.ladder_point != point)
    {
        for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
        {
            CHARACTER *ch = (*it);
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<길드> 래더 점수가 %d 점이 되었습니다", point);
        }
    }
    m_data.ladder_point = point;
}

void CGuild::ChangeLadderPoint(int iChange)
{
    TPacketGuildLadderPoint p;
    p.dwGuild = GetID();
    p.lChange = iChange;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_LADDER_POINT, 0, &p, sizeof(p));
}
