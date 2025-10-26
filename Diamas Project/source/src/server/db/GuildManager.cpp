#include "GuildManager.h"
#include "ClientManager.h"
#include "Config.h"
#include "Main.h"
#include "QID.h"
#include <cmath>

const int GUILD_RANK_MAX_NUM = 20;

uint32_t GetGuildWarWaitStartDuration()
{
    // const int GUILD_WAR_WAIT_START_DURATION = 60;
    // const int GUILD_WAR_WAIT_START_DURATION = 5;
    return 10;
}

uint32_t GetGuildWarReserveSeconds()
{
    // const int GUILD_WAR_RESERVE_SECONDS = 180;
    // const int GUILD_WAR_RESERVE_SECONDS = 10;

    return 5;
}

namespace
{

struct FSendPeerWar {
    FSendPeerWar(uint8_t bType, uint8_t bWar, uint32_t GID1, uint32_t GID2)
    {
        if (Random::get(0, 1))
            std::swap(GID1, GID2);

        memset(&p, 0, sizeof(TPacketGuildWar));

        p.bWar = bWar;
        p.bType = bType;
        p.dwGuildFrom = GID1;
        p.dwGuildTo = GID2;
    }

    void operator()(CPeer* peer)
    {
        if (peer->IsAuth())
            return;

        peer->EncodeHeader(HEADER_DG_GUILD_WAR, 0, sizeof(TPacketGuildWar));
        peer->Encode(&p, sizeof(TPacketGuildWar));
    }

    TPacketGuildWar p;
};

struct FSendGuildWarScore {
    FSendGuildWarScore(uint32_t guild_gain, uint32_t dwOppGID, int iScore,
                       int iBetScore)
    {
        pck.dwGuildGainPoint = guild_gain;
        pck.dwGuildOpponent = dwOppGID;
        pck.lScore = iScore;
        pck.lBetScore = iBetScore;
    }

    void operator()(CPeer* peer)
    {
        if (peer->IsAuth())
            return;

        peer->EncodeHeader(HEADER_DG_GUILD_WAR_SCORE, 0, sizeof(pck));
        peer->Encode(&pck, sizeof(pck));
    }

    TPacketGuildWarScore pck;
};

} // namespace

CGuildManager::CGuildManager(const Config& config)
    : m_config(config)
{
}

CGuildManager::~CGuildManager()
{
    while (!m_pqOnWar.empty()) {
        if (!m_pqOnWar.top().second->bEnd)
            delete m_pqOnWar.top().second;

        m_pqOnWar.pop();
    }
}

TGuild& CGuildManager::TouchGuild(uint32_t GID)
{
    auto it = m_map_kGuild.find(GID);
    if (it != m_map_kGuild.end())
        return it->second;

    TGuild info;
    m_map_kGuild.emplace(GID, info);
    return m_map_kGuild[GID];
}

void CGuildManager::ParseResult(SQLResult* pRes)
{
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(pRes->pSQLResult))) {
        uint32_t GID = strtoul(row[0], NULL, 10);

        TGuild& r_info = TouchGuild(GID);

        strlcpy(r_info.szName, row[1], sizeof(r_info.szName));
        str_to_number(r_info.ladder_point, row[2]);
        str_to_number(r_info.win, row[3]);
        str_to_number(r_info.draw, row[4]);
        str_to_number(r_info.loss, row[5]);
        str_to_number(r_info.gold, row[6]);
        str_to_number(r_info.level, row[7]);

        SPDLOG_DEBUG("GuildWar: {0} ladder {1} win {2} draw {3} loss {4}",
                     r_info.szName, r_info.ladder_point, r_info.win,
                     r_info.draw, r_info.loss);
    }
}

void CGuildManager::Initialize()
{
    std::unique_ptr<SQLMsg> pmsg(
        CDBManager::instance().DirectQuery("SELECT id, name, ladder_point, "
                                           "win, draw, loss, gold, level FROM "
                                           "guild"));

    if (pmsg->Get()->uiNumRows)
        ParseResult(pmsg->Get());

    if (!polyPower.Analyze(m_config.guildPowerPoly.c_str()))
        spdlog::error("cannot set power poly: {0}", m_config.guildPowerPoly);
    else
        SPDLOG_INFO("POWER_POLY: {0}", m_config.guildPowerPoly);

    if (!polyHandicap.Analyze(m_config.guildHandicapPoly.c_str()))
        spdlog::error("cannot set handicap poly: {0}",
                      m_config.guildHandicapPoly);
    else
        SPDLOG_INFO("HANDICAP_POLY: {0}", m_config.guildHandicapPoly);

    QueryRanking();
}

void CGuildManager::Load(uint32_t dwGuildID)
{
    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(
        fmt::format("SELECT id, name, ladder_point, win, draw, loss, gold, "
                    "level FROM guild WHERE id={}",
                    dwGuildID)));

    if (pmsg->Get()->uiNumRows)
        ParseResult(pmsg->Get());
}

void CGuildManager::QueryRanking()
{
    CDBManager::instance().ReturnQuery("SELECT id,name,ladder_point FROM guild "
                                       "ORDER BY ladder_point DESC LIMIT 20",
                                       QID_GUILD_RANKING, 0, 0);
}

int CGuildManager::GetRanking(uint32_t dwGID)
{
    auto it = map_kLadderPointRankingByGID.find(dwGID);
    if (it == map_kLadderPointRankingByGID.end())
        return GUILD_RANK_MAX_NUM;

    return std::clamp(it->second, 0, GUILD_RANK_MAX_NUM);
}

void CGuildManager::ResultRanking(MYSQL_RES* pRes)
{
    if (!pRes)
        return;

    int iLastLadderPoint = -1;
    int iRank = 0;

    map_kLadderPointRankingByGID.clear();

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(pRes))) {
        uint32_t dwGID = 0;
        str_to_number(dwGID, row[0]);
        int iLadderPoint = 0;
        str_to_number(iLadderPoint, row[2]);

        if (iLadderPoint != iLastLadderPoint)
            ++iRank;

        SPDLOG_INFO("GUILD_RANK: {0} {1} {2}", row[1], iRank, iLadderPoint);

        map_kLadderPointRankingByGID.emplace(dwGID, iRank);
    }
}

void CGuildManager::Update()
{
    ProcessReserveWar(); // 예약 전쟁 처리

    time_t now = CClientManager::instance().GetCurrentTime();

    if (!m_pqOnWar.empty()) {
        // UNKNOWN_GUILD_MANAGE_UPDATE_LOG
        /*
           SPDLOG_INFO( "GuildManager::Update size {0} now {1} top {2}, {3}({4})
           vs {5}({6})", m_WarMap.size(), now, m_pqOnWar.top().first,
           m_map_kGuild[m_pqOnWar.top().second->GID[0]].szName,
           m_pqOnWar.top().second->GID[0],
           m_map_kGuild[m_pqOnWar.top().second->GID[1]].szName,
           m_pqOnWar.top().second->GID[1]);
           */
        // END_OF_UNKNOWN_GUILD_MANAGE_UPDATE_LOG

        while (!m_pqOnWar.empty() &&
               (m_pqOnWar.top().first <= now ||
                (m_pqOnWar.top().second && m_pqOnWar.top().second->bEnd))) {
            TGuildWarPQElement* e = m_pqOnWar.top().second;

            m_pqOnWar.pop();

            if (e) {
                if (!e->bEnd)
                    WarEnd(e->GID[0], e->GID[1], false);

                delete e;
            }
        }
    }

    // GUILD_SKILL_COOLTIME_BUG_FIX
    while (!m_pqSkill.empty() && m_pqSkill.top().first <= now) {
        const TGuildSkillUsed& s = m_pqSkill.top().second;
        CClientManager::instance().SendGuildSkillUsable(s.GID, s.dwSkillVnum,
                                                        true);
        m_pqSkill.pop();
    }
    // END_OF_GUILD_SKILL_COOLTIME_BUG_FIX

    while (!m_pqWaitStart.empty() && m_pqWaitStart.top().first <= now) {
        const TGuildWaitStartInfo& ws = m_pqWaitStart.top().second;
        m_pqWaitStart.pop();

        StartWar(ws.bType, ws.GID[0], ws.GID[1],
                 ws.pkReserve); // insert new element to m_WarMap and m_pqOnWar

        if (ws.lInitialScore) {
            UpdateScore(ws.GID[0], ws.GID[1], ws.lInitialScore, 0);
            UpdateScore(ws.GID[1], ws.GID[0], ws.lInitialScore, 0);
        }

        TPacketGuildWar p;

        p.bType = ws.bType;
        p.bWar = GUILD_WAR_ON_WAR;
        p.dwGuildFrom = ws.GID[0];
        p.dwGuildTo = ws.GID[1];

        CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_WAR, &p,
                                                 sizeof(p));
        SPDLOG_INFO("GuildWar: GUILD sending start of wait start war {0} {1}",
                    ws.GID[0], ws.GID[1]);
    }
}

#define for_all(cont, it)                                                      \
    for (auto it = (cont).begin(); it != (cont).end(); ++it)

void CGuildManager::OnSetup(CPeer* peer)
{
    for_all(m_WarMap, it_cont) for_all(it_cont->second, it)
    {
        uint32_t g1 = it_cont->first;
        uint32_t g2 = it->first;
        TGuildWarPQElement* p = it->second.pElement;

        if (!p || p->bEnd)
            continue;

        FSendPeerWar(p->bType, GUILD_WAR_ON_WAR, g1, g2)(peer);
        FSendGuildWarScore(p->GID[0], p->GID[1], p->iScore[0], p->iBetScore[0]);
        FSendGuildWarScore(p->GID[1], p->GID[0], p->iScore[1], p->iBetScore[1]);
    }

    for_all(m_DeclareMap, it)
    {
        FSendPeerWar(it->bType, GUILD_WAR_SEND_DECLARE, it->dwGuildID[0],
                     it->dwGuildID[1])(peer);
    }

    for_all(m_map_kWarReserve, it)
    {
        it->second->OnSetup(peer);
    }
}

void CGuildManager::GuildWarWin(uint32_t GID)
{
    auto it = m_map_kGuild.find(GID);

    if (it == m_map_kGuild.end())
        return;

    ++it->second.win;

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET win={} "
                                                  "WHERE id={}",
                                                  it->second.win, GID));
}

void CGuildManager::GuildWarLose(uint32_t GID)
{
    auto it = m_map_kGuild.find(GID);

    if (it == m_map_kGuild.end())
        return;

    ++it->second.loss;

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET loss={} "
                                                  "WHERE id={}",
                                                  it->second.loss, GID));
}

void CGuildManager::GuildWarDraw(uint32_t GID)
{
    auto it = m_map_kGuild.find(GID);

    if (it == m_map_kGuild.end())
        return;

    ++it->second.draw;

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET draw={} "
                                                  "WHERE id={}",
                                                  it->second.draw, GID));
}

bool CGuildManager::IsHalfWinLadderPoint(uint32_t dwGuildWinner,
                                         uint32_t dwGuildLoser)
{
    uint32_t GID1 = dwGuildWinner;
    uint32_t GID2 = dwGuildLoser;

    if (GID1 > GID2)
        std::swap(GID1, GID2);

    auto it = m_mapGuildWarEndTime[GID1].find(GID2);
    if (it != m_mapGuildWarEndTime[GID1].end() &&
        it->second + GUILD_WAR_LADDER_HALF_PENALTY_TIME >
            CClientManager::instance().GetCurrentTime())
        return true;

    return false;
}

void CGuildManager::ProcessDraw(uint32_t dwGuildID1, uint32_t dwGuildID2)
{
    SPDLOG_INFO("GuildWar: \tThe war between {0} and {1} is ended in draw",
                dwGuildID1, dwGuildID2);

    GuildWarDraw(dwGuildID1);
    GuildWarDraw(dwGuildID2);
    ChangeLadderPoint(dwGuildID1, 0);
    ChangeLadderPoint(dwGuildID2, 0);

    QueryRanking();
}

void CGuildManager::ProcessWinLose(uint32_t dwGuildWinner,
                                   uint32_t dwGuildLoser)
{
    GuildWarWin(dwGuildWinner);
    GuildWarLose(dwGuildLoser);
    SPDLOG_INFO("GuildWar: \tWinner : {0} Loser : {1}", dwGuildWinner,
                dwGuildLoser);

    int iPoint = GetLadderPoint(dwGuildLoser);
    int gain = (int)(iPoint * 0.05);
    int loss = (int)(iPoint * 0.07);

    if (IsHalfWinLadderPoint(dwGuildWinner, dwGuildLoser))
        gain /= 2;

    SPDLOG_INFO("GuildWar: \tgain : {0} loss : {1}", gain, loss);

    ChangeLadderPoint(dwGuildWinner, gain);
    ChangeLadderPoint(dwGuildLoser, -loss);

    QueryRanking();
}

void CGuildManager::RemoveWar(uint32_t GID1, uint32_t GID2)
{
    SPDLOG_INFO("GuildWar: RemoveWar({0}, {1})", GID1, GID2);

    if (GID1 > GID2)
        std::swap(GID2, GID1);

    auto it = m_WarMap[GID1].find(GID2);
    if (it == m_WarMap[GID1].end()) {
        if (m_WarMap[GID1].empty())
            m_WarMap.erase(GID1);

        return;
    }

    if (it->second.pElement)
        it->second.pElement->bEnd = true;

    m_mapGuildWarEndTime[GID1][GID2] =
        CClientManager::instance().GetCurrentTime();

    m_WarMap[GID1].erase(it);

    if (m_WarMap[GID1].empty())
        m_WarMap.erase(GID1);
}

//
// 길드전 비정상 종료 및 필드전 종료
//
void CGuildManager::WarEnd(uint32_t GID1, uint32_t GID2, bool bForceDraw)
{
    if (GID1 > GID2)
        std::swap(GID2, GID1);

    SPDLOG_INFO("GuildWar: WarEnd {0} {1}", GID1, GID2);

    auto itWarMap = m_WarMap[GID1].find(GID2);
    if (itWarMap == m_WarMap[GID1].end()) {
        spdlog::error("GuildWar: war not exist or already ended. [1]");
        return;
    }

    TGuildWarInfo gwi = itWarMap->second;
    TGuildWarPQElement* pData = gwi.pElement;

    if (!pData || pData->bEnd) {
        spdlog::error("GuildWar: war not exist or already ended. [2]");
        return;
    }

    uint32_t win_guild = pData->GID[0];
    uint32_t lose_guild = pData->GID[1];

    bool bDraw = false;

    if (!bForceDraw) // 강제 무승부가 아닐 경우에는 점수를 체크한다.
    {
        if (pData->iScore[0] > pData->iScore[1]) {
            win_guild = pData->GID[0];
            lose_guild = pData->GID[1];
        } else if (pData->iScore[1] > pData->iScore[0]) {
            win_guild = pData->GID[1];
            lose_guild = pData->GID[0];
        } else
            bDraw = true;
    } else // 강제 무승부일 경우에는 무조건 무승부
        bDraw = true;

    if (bDraw)
        ProcessDraw(win_guild, lose_guild);
    else
        ProcessWinLose(win_guild, lose_guild);

    // DB 서버에서 자체적으로 끝낼 때도 있기 때문에 따로 패킷을 보내줘야 한다.
    CClientManager::instance().for_each_peer(
        FSendPeerWar(0, GUILD_WAR_END, GID1, GID2));

    RemoveWar(GID1, GID2);
}

//
// 길드전 정상 종료
//
void CGuildManager::RecvWarOver(uint32_t dwGuildWinner, uint32_t dwGuildLoser,
                                bool bDraw, int32_t lWarPrice)
{
    SPDLOG_INFO("GuildWar: RecvWarOver : winner {0} vs {1} draw? {2} war_price "
                "{3}",
                dwGuildWinner, dwGuildLoser, bDraw ? 1 : 0, lWarPrice);

    uint32_t GID1 = dwGuildWinner;
    uint32_t GID2 = dwGuildLoser;

    if (GID1 > GID2)
        std::swap(GID1, GID2);

    auto it = m_WarMap[GID1].find(GID2);
    if (it == m_WarMap[GID1].end())
        return;

    TGuildWarInfo& gw = it->second;

    // Award
    if (bDraw) {

        ProcessDraw(dwGuildWinner, dwGuildLoser);
    } else {
        ProcessWinLose(dwGuildWinner, dwGuildLoser);
    }

    if (gw.pkReserve) {
        if (bDraw || !gw.pElement)
            gw.pkReserve->Draw();
        else if (gw.pElement->bType == GUILD_WAR_TYPE_BATTLE)
            gw.pkReserve->End(gw.pElement->iBetScore[0],
                              gw.pElement->iBetScore[1]);
    }

    RemoveWar(GID1, GID2);
}

void CGuildManager::RecvWarEnd(uint32_t GID1, uint32_t GID2)
{
    SPDLOG_INFO("GuildWar: RecvWarEnded : {0} vs {1}", GID1, GID2);
    WarEnd(GID1, GID2, true); // 무조건 비정상 종료 시켜야 한다.
}

void CGuildManager::StartWar(uint8_t bType, uint32_t GID1, uint32_t GID2,
                             CGuildWarReserve* pkReserve)
{
    SPDLOG_INFO("GuildWar: StartWar({0},{1},{2})", bType, GID1, GID2);

    if (GID1 > GID2)
        std::swap(GID1, GID2);

    TGuildWarInfo& gw = m_WarMap[GID1][GID2]; // map insert

    if (bType == GUILD_WAR_TYPE_FIELD)
        gw.tEndTime =
            CClientManager::instance().GetCurrentTime() + GUILD_WAR_DURATION;
    else
        gw.tEndTime = CClientManager::instance().GetCurrentTime() + 172800;

    gw.pElement = new TGuildWarPQElement(bType, GID1, GID2);
    gw.pkReserve = pkReserve;

    m_pqOnWar.push(std::make_pair(gw.tEndTime, gw.pElement));
}

void CGuildManager::UpdateScore(uint32_t dwGainGID, uint32_t dwOppGID,
                                int iScoreDelta, int iBetScoreDelta)
{
    uint32_t GID1 = dwGainGID;
    uint32_t GID2 = dwOppGID;

    if (GID1 > GID2)
        std::swap(GID1, GID2);

    auto it = m_WarMap[GID1].find(GID2);
    if (it != m_WarMap[GID1].end()) {
        TGuildWarPQElement* p = it->second.pElement;

        if (!p || p->bEnd) {
            spdlog::error("GuildWar: war not exist or already ended.");
            return;
        }

        int iNewScore = 0;
        int iNewBetScore = 0;

        if (p->GID[0] == dwGainGID) {
            p->iScore[0] += iScoreDelta;
            p->iBetScore[0] += iBetScoreDelta;

            iNewScore = p->iScore[0];
            iNewBetScore = p->iBetScore[0];
        } else {
            p->iScore[1] += iScoreDelta;
            p->iBetScore[1] += iBetScoreDelta;

            iNewScore = p->iScore[1];
            iNewBetScore = p->iBetScore[1];
        }

        SPDLOG_INFO("GuildWar: SendGuildWarScore guild {0} wartype {1} "
                    "score_delta {2} betscore_delta {3} result {4}, {5}",
                    dwGainGID, p->bType, iScoreDelta, iBetScoreDelta, iNewScore,
                    iNewBetScore);

        CClientManager::instance().for_each_peer(
            FSendGuildWarScore(dwGainGID, dwOppGID, iNewScore, iNewBetScore));
    }
}

void CGuildManager::AddDeclare(uint8_t bType, uint32_t guild_from,
                               uint32_t guild_to)
{
    TGuildDeclareInfo di(bType, guild_from, guild_to);

    if (m_DeclareMap.find(di) == m_DeclareMap.end())
        m_DeclareMap.insert(di);

    SPDLOG_INFO("GuildWar: AddDeclare(Type:{0},from:{1},to:{2})", bType,
                guild_from, guild_to);
}

void CGuildManager::RemoveDeclare(uint32_t guild_from, uint32_t guild_to)
{
    auto it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_from, guild_to));
    if (it != m_DeclareMap.end())
        m_DeclareMap.erase(it);

    it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_to, guild_from));
    if (it != m_DeclareMap.end())
        m_DeclareMap.erase(it);

    SPDLOG_INFO("GuildWar: RemoveDeclare(from:{0},to:{1})", guild_from,
                guild_to);
}

bool CGuildManager::TakeBetPrice(uint32_t dwGuildTo, uint32_t dwGuildFrom,
                                 int32_t lWarPrice)
{
    auto it_from = m_map_kGuild.find(dwGuildFrom);
    auto it_to = m_map_kGuild.find(dwGuildTo);

    if (it_from == m_map_kGuild.end() || it_to == m_map_kGuild.end()) {
        SPDLOG_INFO("TakeBetPrice: guild not exist {0} {1}", dwGuildFrom,
                    dwGuildTo);
        return false;
    }

    if (it_from->second.gold < lWarPrice || it_to->second.gold < lWarPrice) {
        SPDLOG_INFO("TakeBetPrice: not enough gold {0} {1} to {2} {3}",
                    dwGuildFrom, it_from->second.gold, dwGuildTo,
                    it_to->second.gold);
        return false;
    }

    it_from->second.gold -= lWarPrice;
    it_to->second.gold -= lWarPrice;

    MoneyChange(dwGuildFrom, it_from->second.gold);
    MoneyChange(dwGuildTo, it_to->second.gold);
    return true;
}

bool CGuildManager::WaitStart(TPacketGuildWar* p)
{
    uint32_t dwCurTime = CClientManager::instance().GetCurrentTime();

    TGuildWaitStartInfo info(p->bType, p->dwGuildFrom, p->dwGuildTo,
                             p->isRanked, p->lInitialScore, NULL);
    m_pqWaitStart.emplace(dwCurTime + GetGuildWarWaitStartDuration(), info);

    spdlog::trace("GuildWar: WaitStart g1 {} g2 {} price {} start at {}",
                  p->dwGuildFrom, p->dwGuildTo, p->isRanked,
                  dwCurTime + GetGuildWarWaitStartDuration());

    return true;
}

int CGuildManager::GetLadderPoint(uint32_t GID)
{
    auto it = m_map_kGuild.find(GID);

    if (it == m_map_kGuild.end())
        return 0;

    return it->second.ladder_point;
}

void CGuildManager::ChangeLadderPoint(uint32_t GID, int change)
{
    auto it = m_map_kGuild.find(GID);

    if (it == m_map_kGuild.end())
        return;

    TGuild& r = it->second;

    r.ladder_point += change;

    if (r.ladder_point < 0)
        r.ladder_point = 0;

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET "
                                                  "ladder_point={} WHERE id={}",
                                                  r.ladder_point, GID));

    SPDLOG_INFO("GuildManager::ChangeLadderPoint {0} {1}", GID, r.ladder_point);

    // Packet 보내기
    TPacketGuildLadder p;

    p.dwGuild = GID;
    p.lLadderPoint = r.ladder_point;
    p.lWin = r.win;
    p.lDraw = r.draw;
    p.lLoss = r.loss;

    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_LADDER, &p,
                                             sizeof(TPacketGuildLadder));
}

void CGuildManager::UseSkill(uint32_t GID, uint32_t dwSkillVnum,
                             uint32_t dwCooltime)
{
    // GUILD_SKILL_COOLTIME_BUG_FIX
    SPDLOG_INFO("UseSkill(gid={0}, skill={1}) CoolTime({2}:{3})", GID,
                dwSkillVnum, dwCooltime,
                CClientManager::instance().GetCurrentTime() + dwCooltime);
    m_pqSkill.push(
        std::make_pair(CClientManager::instance().GetCurrentTime() + dwCooltime,
                       TGuildSkillUsed(GID, dwSkillVnum)));
    // END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

void CGuildManager::MoneyChange(uint32_t dwGuild, uint32_t dwGold)
{
    SPDLOG_INFO("GuildManager::MoneyChange {0} {1}", dwGuild, dwGold);

    TPacketDGGuildMoneyChange p;
    p.dwGuild = dwGuild;
    p.iTotalGold = dwGold;
    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_MONEY_CHANGE, &p,
                                             sizeof(p));

    CDBManager::instance().AsyncQuery(
        fmt::format("UPDATE guild SET gold={} WHERE id = {}", dwGold, dwGuild));
}

void CGuildManager::DepositMoney(uint32_t dwGuild, int32_t iGold)
{
    if (iGold <= 0)
        return;

    auto it = m_map_kGuild.find(dwGuild);

    if (it == m_map_kGuild.end()) {
        spdlog::error("No guild by id {0}", dwGuild);
        return;
    }

    it->second.gold += iGold;
    SPDLOG_INFO("GUILD: {0} Deposit {1} Total {2}", dwGuild, iGold,
                it->second.gold);

    MoneyChange(dwGuild, it->second.gold);
}

void CGuildManager::WithdrawMoney(CPeer* peer, uint32_t dwGuild, int32_t iGold)
{
    auto it = m_map_kGuild.find(dwGuild);

    if (it == m_map_kGuild.end()) {
        spdlog::error("No guild by id {0}", dwGuild);
        return;
    }

    // 돈이있으니 출금하고 올려준다
    if (it->second.gold >= iGold) {
        it->second.gold -= iGold;
        SPDLOG_INFO("GUILD: {0} Withdraw {1} Total {2}", dwGuild, iGold,
                    it->second.gold);

        TPacketDGGuildMoneyWithdraw p;
        p.dwGuild = dwGuild;
        p.iChangeGold = iGold;

        peer->EncodeHeader(HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE, 0,
                           sizeof(TPacketDGGuildMoneyWithdraw));
        peer->Encode(&p, sizeof(TPacketDGGuildMoneyWithdraw));
    }
}

void CGuildManager::WithdrawMoneyReply(uint32_t dwGuild, uint8_t bGiveSuccess,
                                       int32_t iGold)
{
    auto it = m_map_kGuild.find(dwGuild);

    if (it == m_map_kGuild.end())
        return;

    SPDLOG_INFO("GuildManager::WithdrawMoneyReply : guild {0} success {1} gold "
                "{2}",
                dwGuild, bGiveSuccess, iGold);

    if (!bGiveSuccess)
        it->second.gold += iGold;
    else
        MoneyChange(dwGuild, it->second.gold);
}

//
// 예약 길드전(관전자가 배팅할 수 있다)
//
const int c_aiScoreByLevel[GUILD_MAX_LEVEL + 1] = {
    500, // level 0 = 500 probably error
    500, // 1
    1000,  2000,  3000,  4000,  6000,  8000,  10000, 12000,
    15000, // 10
    18000, 21000, 24000, 28000, 32000, 36000, 40000, 45000, 50000, 55000,
};

const int c_aiScoreByRanking[GUILD_RANK_MAX_NUM + 1] = {
    0,
    55000, // 1위
    50000, 45000, 40000, 36000, 32000, 28000, 24000, 21000,
    18000, // 10위
    15000, 12000, 10000, 8000,  6000,  4000,  3000,  2000,  1000,
    500 // 20위
};

void CGuildManager::BootReserveWar()
{
    const char* c_apszQuery[2] = {"SELECT id, guild1, guild2, "
                                  "UNIX_TIMESTAMP(time), type, warprice, "
                                  "initscore, bet_from, bet_to, power1, "
                                  "power2, handicap FROM guild_war_reservation "
                                  "WHERE started=1 AND winner=-1",
                                  "SELECT id, guild1, guild2, "
                                  "UNIX_TIMESTAMP(time), type, warprice, "
                                  "initscore, bet_from, bet_to, power1, "
                                  "power2, handicap FROM guild_war_reservation "
                                  "WHERE started=0"};

    for (int i = 0; i < 2; ++i) {
        std::unique_ptr<SQLMsg> pmsg(
            CDBManager::instance().DirectQuery(c_apszQuery[i]));

        if (pmsg->Get()->uiNumRows == 0)
            continue;

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(pmsg->Get()->pSQLResult))) {
            int col = 0;

            TGuildWarReserve t;

            str_to_number(t.dwID, row[col++]);
            str_to_number(t.dwGuildFrom, row[col++]);
            str_to_number(t.dwGuildTo, row[col++]);
            str_to_number(t.dwTime, row[col++]);
            str_to_number(t.bType, row[col++]);
            str_to_number(t.lWarPrice, row[col++]);
            str_to_number(t.lInitialScore, row[col++]);
            str_to_number(t.dwBetFrom, row[col++]);
            str_to_number(t.dwBetTo, row[col++]);
            str_to_number(t.lPowerFrom, row[col++]);
            str_to_number(t.lPowerTo, row[col++]);
            str_to_number(t.lHandicap, row[col++]);
            t.bStarted = 0;

            CGuildWarReserve* pkReserve = new CGuildWarReserve(t);

            std::string msg =
                fmt::format("GuildWar: BootReserveWar : step {} id {} GID1 {} "
                            "GID2 {}",
                            i, t.dwID, t.dwGuildFrom, t.dwGuildTo);

            // i == 0 이면 길드전 도중 DB가 튕긴 것이므로 무승부 처리한다.
            // 또는, 5분 이하 남은 예약 길드전도 무승부 처리한다. (각자의
            // 배팅액을 돌려준다)
            // if (i == 0 || (int) t.dwTime -
            // CClientManager::instance().GetCurrentTime() < 60 * 5)
            if (i == 0 ||
                (int)t.dwTime - CClientManager::instance().GetCurrentTime() <
                    0) {
                if (i == 0)
                    SPDLOG_INFO("{0} : DB was shutdowned while war is being.",
                                msg);
                else
                    SPDLOG_INFO("{0} : left time lower than 5 minutes, will be "
                                "canceled",
                                msg);

                pkReserve->Draw();
                delete pkReserve;
            } else {
                SPDLOG_INFO("{0} : OK", msg);
                m_map_kWarReserve.emplace(t.dwID, pkReserve);
            }
        }
    }
}

int GetAverageGuildMemberLevel(uint32_t dwGID)
{
    std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(
        fmt::format("SELECT AVG(level) FROM guild_member, player AS p WHERE "
                    "guild_id={} AND guild_member.pid=p.id",
                    dwGID)));

    MYSQL_ROW row;
    row = mysql_fetch_row(msg->Get()->pSQLResult);

    int nAverageLevel = 0;
    str_to_number(nAverageLevel, row[0]);
    return nAverageLevel;
}

int GetGuildMemberCount(uint32_t dwGID)
{
    std::unique_ptr<SQLMsg> msg(CDBManager::instance().DirectQuery(fmt::format(
        "SELECT COUNT(*) FROM guild_member WHERE guild_id={}", dwGID)));

    MYSQL_ROW row;
    row = mysql_fetch_row(msg->Get()->pSQLResult);

    uint32_t dwCount = 0;
    str_to_number(dwCount, row[0]);
    return dwCount;
}

bool CGuildManager::ReserveWar(TPacketGuildWar* p)
{
    uint32_t GID1 = p->dwGuildFrom;
    uint32_t GID2 = p->dwGuildTo;

    if (GID1 > GID2)
        std::swap(GID1, GID2);

    TGuildWarReserve t;
    memset(&t, 0, sizeof(TGuildWarReserve));

    t.dwGuildFrom = GID1;
    t.dwGuildTo = GID2;
    t.dwTime = CClientManager::instance().GetCurrentTime() +
               GetGuildWarReserveSeconds();
    t.bType = p->bType;
    t.lWarPrice = 0;
    t.lInitialScore = p->lInitialScore;

    int lvp, rkp, alv, mc;

    // 파워 계산
    TGuild& k1 = TouchGuild(GID1);

    lvp = c_aiScoreByLevel[std::min<int>(GUILD_MAX_LEVEL, k1.level)];
    rkp = c_aiScoreByRanking[GetRanking(GID1)];
    alv = GetAverageGuildMemberLevel(GID1);
    mc = GetGuildMemberCount(GID1);

    polyPower.SetVar("lvp", lvp);
    polyPower.SetVar("rkp", rkp);
    polyPower.SetVar("alv", alv);
    polyPower.SetVar("mc", mc);

    t.lPowerFrom = (int32_t)polyPower.Eval();
    SPDLOG_INFO("GuildWar: {0} lvp {1} rkp {2} alv {3} mc {4} power {5}", GID1,
                lvp, rkp, alv, mc, t.lPowerFrom);

    // 파워 계산
    TGuild& k2 = TouchGuild(GID2);

    lvp = c_aiScoreByLevel[std::min<int>(GUILD_MAX_LEVEL, k2.level)];
    rkp = c_aiScoreByRanking[GetRanking(GID2)];
    alv = GetAverageGuildMemberLevel(GID2);
    mc = GetGuildMemberCount(GID2);

    polyPower.SetVar("lvp", lvp);
    polyPower.SetVar("rkp", rkp);
    polyPower.SetVar("alv", alv);
    polyPower.SetVar("mc", mc);

    t.lPowerTo = (int32_t)polyPower.Eval();
    SPDLOG_INFO("GuildWar: {0} lvp {1} rkp {2} alv {3} mc {4} power {5}", GID2,
                lvp, rkp, alv, mc, t.lPowerTo);

    // 핸디캡 계산
    if (t.lPowerTo > t.lPowerFrom) {
        polyHandicap.SetVar("pA", t.lPowerTo);
        polyHandicap.SetVar("pB", t.lPowerFrom);
    } else {
        polyHandicap.SetVar("pA", t.lPowerFrom);
        polyHandicap.SetVar("pB", t.lPowerTo);
    }

    t.lHandicap = (int32_t)polyHandicap.Eval();
    SPDLOG_INFO("GuildWar: handicap {0}", t.lHandicap);

    // 쿼리
    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(
        fmt::format("INSERT INTO guild_war_reservation (guild1, guild2, time, "
                    "type, warprice, initscore, power1, power2, handicap) "
                    "VALUES({}, {}, DATE_ADD(NOW(), INTERVAL 180 SECOND), {}, "
                    "{}, {}, {}, {}, {})",
                    GID1, GID2, p->bType, p->isRanked, p->lInitialScore,
                    t.lPowerFrom, t.lPowerTo, t.lHandicap)));

    /* if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiInsertID == 0 ||
                 pmsg->Get()->uiAffectedRows == (uint32_t)-1)
        {
            spdlog::error( "GuildWar: Cannot insert row");
            return false;
        }
        */
    t.dwID = pmsg->Get()->uiInsertID;

    m_map_kWarReserve.emplace(t.dwID, new CGuildWarReserve(t));

    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_WAR_RESERVE_ADD,
                                             &t, sizeof(TGuildWarReserve));
    return true;
}

void CGuildManager::ProcessReserveWar()
{
    uint32_t dwCurTime = CClientManager::instance().GetCurrentTime();

    auto it = m_map_kWarReserve.begin();
    while (it != m_map_kWarReserve.end()) {
        auto it2 = it++;

        CGuildWarReserve* pk = it2->second;
        TGuildWarReserve& r = pk->GetDataRef();

        if (!r.bStarted && r.dwTime - 1800 <= dwCurTime) // 30분 전부터 알린다.
        {
            int iMin = (int)ceil((int)(r.dwTime - dwCurTime) / 60.0);

            TGuild& r_1 = m_map_kGuild[r.dwGuildFrom];
            TGuild& r_2 = m_map_kGuild[r.dwGuildTo];

            SPDLOG_INFO("GuildWar: started GID1 {0} GID2 {1} {2} time {3} min "
                        "{4}",
                        r.dwGuildFrom, r.dwGuildTo, r.bStarted,
                        dwCurTime - r.dwTime, iMin);

            if (iMin <= 0) {
                CDBManager::instance().AsyncQuery(fmt::sprintf("UPDATE "
                                                               "guild_war_"
                                                               "reservation "
                                                               "SET started=1 "
                                                               "WHERE id=%u",
                                                               r.dwID));

                CClientManager::instance().ForwardPacket(
                    HEADER_DG_GUILD_WAR_RESERVE_DEL, &r.dwID, sizeof(uint32_t));

                r.bStarted = true;

                TGuildWaitStartInfo info(r.bType, r.dwGuildFrom, r.dwGuildTo,
                                         r.lWarPrice, r.lInitialScore, pk);
                m_pqWaitStart.emplace(
                    dwCurTime + GetGuildWarWaitStartDuration(), info);

                TPacketGuildWar pck;

                pck.bType = r.bType;
                pck.bWar = GUILD_WAR_WAIT_START;
                pck.dwGuildFrom = r.dwGuildFrom;
                pck.dwGuildTo = r.dwGuildTo;
                pck.isRanked = r.lWarPrice;
                pck.lInitialScore = r.lInitialScore;

                CClientManager::instance().ForwardPacket(
                    HEADER_DG_GUILD_WAR, &pck, sizeof(TPacketGuildWar));
                // m_map_kWarReserve.erase(it2);
            } else {
                // if (iMin != pk->GetLastNoticeMin())
                //{
                //	pk->SetLastNoticeMin(iMin);
                //
                //	if (!g_stLocale.compare("euckr"))
                //		CClientManager::instance().SendNotice("%s 길드와 %s 길드의
                //전쟁이 약 %d분 후 시작 됩니다!", r_1.szName, r_2.szName,
                //iMin); 	else if (!g_stLocale.compare("gb2312"))
                //		CClientManager::instance().SendNotice("%s 곤삔뵨 %s
                //곤삔돨곤삔濫轢쉥瞳 %d롸爐빈역迦!", r_1.szName, r_2.szName,
                //iMin);
                // }
            }
        }
    }
}

bool CGuildManager::Bet(uint32_t dwID, const char* c_pszLogin, uint32_t dwGold,
                        uint32_t dwGuild)
{
    auto it = m_map_kWarReserve.find(dwID);

    char szQuery[1024];

    if (it == m_map_kWarReserve.end()) {
        SPDLOG_INFO("WAR_RESERVE: Bet: cannot find reserve war by id {0}",
                    dwID);
        CDBManager::instance().AsyncQuery(
            fmt::sprintf("INSERT INTO item_award (login, vnum, socket0, "
                         "given_time) VALUES('%s', %d, %u, NOW())",
                         c_pszLogin, ITEM_ELK_VNUM, dwGold));
        return false;
    }

    if (!it->second->Bet(c_pszLogin, dwGold, dwGuild)) {
        SPDLOG_INFO("WAR_RESERVE: Bet: cannot bet id {0}, login {1}, gold {2}, "
                    "guild {3}",
                    dwID, c_pszLogin, dwGold, dwGuild);
        CDBManager::instance().AsyncQuery(
            fmt::sprintf("INSERT INTO item_award (login, vnum, socket0, "
                         "given_time) VALUES('%s', %d, %u, NOW())",
                         c_pszLogin, ITEM_ELK_VNUM, dwGold));
        return false;
    }

    return true;
}

void CGuildManager::CancelWar(uint32_t GID1, uint32_t GID2)
{
    RemoveDeclare(GID1, GID2);
    RemoveWar(GID1, GID2);
}

bool CGuildManager::ChangeMaster(uint32_t dwGID, uint32_t dwFrom, uint32_t dwTo)
{
    auto iter = m_map_kGuild.find(dwGID);

    if (iter == m_map_kGuild.end())
        return false;

    std::unique_ptr<SQLMsg> q1(CDBManager::instance().DirectQuery(
        fmt::format("UPDATE guild SET master={} WHERE id={}", dwTo, dwGID)));
    std::unique_ptr<SQLMsg> q2(CDBManager::instance().DirectQuery(
        fmt::format("UPDATE guild_member SET grade=1 WHERE pid={}", dwTo)));
    std::unique_ptr<SQLMsg> q3(CDBManager::instance().DirectQuery(
        fmt::format("UPDATE guild_member SET grade=15 WHERE pid={}", dwFrom)));

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Guild War Reserve Class
//////////////////////////////////////////////////////////////////////////////////////////
CGuildWarReserve::CGuildWarReserve(const TGuildWarReserve& rTable)
{
    memcpy(&m_data, &rTable, sizeof(TGuildWarReserve));
    m_iLastNoticeMin = -1;

    Initialize();
}

void CGuildWarReserve::Initialize()
{
    std::unique_ptr<SQLMsg> msgbet(
        CDBManager::instance().DirectQuery(fmt::format("SELECT login, guild, "
                                                       "gold FROM "
                                                       "guild_war_bet WHERE "
                                                       "war_id={}",
                                                       m_data.dwID)));

    if (msgbet->Get()->uiNumRows) {
        MYSQL_RES* res = msgbet->Get()->pSQLResult;
        MYSQL_ROW row;

        char szLogin[LOGIN_MAX_LEN + 1];
        uint32_t dwGuild;
        uint32_t dwGold;

        while ((row = mysql_fetch_row(res))) {
            dwGuild = dwGold = 0;
            strlcpy(szLogin, row[0], sizeof(szLogin));
            str_to_number(dwGuild, row[1]);
            str_to_number(dwGold, row[2]);

            mapBet.emplace(szLogin, std::make_pair(dwGuild, dwGold));
        }
    }
}

void CGuildWarReserve::OnSetup(CPeer* peer)
{
    if (m_data.bStarted) // 이미 시작된 것은 보내지 않는다.
        return;

    FSendPeerWar(m_data.bType, GUILD_WAR_RESERVE, m_data.dwGuildFrom,
                 m_data.dwGuildTo)(peer);

    peer->EncodeHeader(HEADER_DG_GUILD_WAR_RESERVE_ADD, 0,
                       sizeof(TGuildWarReserve));
    peer->Encode(&m_data, sizeof(TGuildWarReserve));

    TPacketGDGuildWarBet pckBet;
    pckBet.dwWarID = m_data.dwID;

    auto it = mapBet.begin();
    while (it != mapBet.end()) {
        strlcpy(pckBet.szLogin, it->first.c_str(), sizeof(pckBet.szLogin));
        pckBet.dwGuild = it->second.first;
        pckBet.dwGold = it->second.second;

        peer->EncodeHeader(HEADER_DG_GUILD_WAR_BET, 0,
                           sizeof(TPacketGDGuildWarBet));
        peer->Encode(&pckBet, sizeof(TPacketGDGuildWarBet));

        ++it;
    }
}

bool CGuildWarReserve::Bet(const char* pszLogin, uint32_t dwGold,
                           uint32_t dwGuild)
{
    char szQuery[1024];

    if (m_data.dwGuildFrom != dwGuild && m_data.dwGuildTo != dwGuild) {
        SPDLOG_INFO("GuildWarReserve::Bet: invalid guild id");
        return false;
    }

    if (m_data.bStarted) {
        SPDLOG_INFO("GuildWarReserve::Bet: war is already started");
        return false;
    }

    if (mapBet.find(pszLogin) != mapBet.end()) {
        SPDLOG_INFO("GuildWarReserve::Bet: failed. already bet");
        return false;
    }

    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(
        fmt::format("INSERT INTO guild_war_bet (war_id, login, gold, guild) "
                    "VALUES({}, '{}', {}, {})",
                    m_data.dwID, pszLogin, dwGold, dwGuild)));

    if (pmsg->Get()->uiAffectedRows == 0 ||
        pmsg->Get()->uiAffectedRows == (uint32_t)-1) {
        SPDLOG_INFO("GuildWarReserve::Bet: failed. cannot insert row to "
                    "guild_war_bet table");
        return false;
    }

    if (m_data.dwGuildFrom == dwGuild)
        m_data.dwBetFrom += dwGold;
    else
        m_data.dwBetTo += dwGold;

    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_WAR_RESERVE_ADD,
                                             &m_data, sizeof(TGuildWarReserve));

    CDBManager::instance().AsyncQuery(
        fmt::sprintf("UPDATE guild_war_reservation SET bet_from=%u,bet_to=%u "
                     "WHERE id=%u",
                     m_data.dwBetFrom, m_data.dwBetTo, m_data.dwID));

    SPDLOG_INFO("GuildWarReserve::Bet: success. {0} {1} war_id {2} bet {3} : "
                "{4}",
                pszLogin, dwGuild, m_data.dwID, m_data.dwBetFrom,
                m_data.dwBetTo);
    mapBet.emplace(pszLogin, std::make_pair(dwGuild, dwGold));

    TPacketGDGuildWarBet pckBet;
    pckBet.dwWarID = m_data.dwID;
    strlcpy(pckBet.szLogin, pszLogin, sizeof(pckBet.szLogin));
    pckBet.dwGuild = dwGuild;
    pckBet.dwGold = dwGold;

    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_WAR_BET, &pckBet,
                                             sizeof(TPacketGDGuildWarBet));
    return true;
}

//
// 무승부 처리: 대부분 승부가 나야 정상이지만, 서버 문제 등 특정 상황일 경우에는
//              무승부 처리가 있어야 한다.
//
void CGuildWarReserve::Draw()
{

    CDBManager::instance().AsyncQuery(fmt::sprintf("UPDATE "
                                                   "guild_war_reservation SET "
                                                   "started=1,winner=0 WHERE "
                                                   "id=%u",
                                                   m_data.dwID));

    if (mapBet.empty())
        return;

    SPDLOG_INFO("WAR_REWARD: Draw. war_id {0}", m_data.dwID);

    auto it = mapBet.begin();

    std::string query = "INSERT INTO item_award (login, vnum, socket0, "
                        "given_time) VALUES";
    while (1) {
        int iLen = 0;
        int iRow = 0;

        while (it != mapBet.end()) {
            if (iRow == 0)
                query +=
                    fmt::sprintf("('%s', %d, %u, NOW())", it->first.c_str(),
                                 ITEM_ELK_VNUM, it->second.second);
            else
                query +=
                    fmt::sprintf(",('%s', %d, %u, NOW())", it->first.c_str(),
                                 ITEM_ELK_VNUM, it->second.second);

            ++it;

            if (iLen > 384)
                break;

            ++iRow;
        }

        if (iRow > 0) {
            SPDLOG_DEBUG("WAR_REWARD: QUERY: {0}", query);
            CDBManager::instance().AsyncQuery(query);
        }

        if (it == mapBet.end())
            break;
    }
}

void CGuildWarReserve::End(int iScoreFrom, int iScoreTo)
{
    uint32_t dwWinner;

    SPDLOG_INFO("WAR_REWARD: End: From {0} {1} To {2} {3}", m_data.dwGuildFrom,
                iScoreFrom, m_data.dwGuildTo, iScoreTo);

    if (m_data.lPowerFrom > m_data.lPowerTo) {
        if (m_data.lHandicap > iScoreFrom - iScoreTo) {
            SPDLOG_INFO("WAR_REWARD: End: failed to overcome handicap, From is "
                        "strong but To won");
            dwWinner = m_data.dwGuildTo;
        } else {
            SPDLOG_INFO("WAR_REWARD: End: success to overcome handicap, From "
                        "win!");
            dwWinner = m_data.dwGuildFrom;
        }
    } else {
        if (m_data.lHandicap > iScoreTo - iScoreFrom) {
            SPDLOG_INFO("WAR_REWARD: End: failed to overcome handicap, To is "
                        "strong but From won");
            dwWinner = m_data.dwGuildFrom;
        } else {
            SPDLOG_INFO("WAR_REWARD: End: success to overcome handicap, To "
                        "win!");
            dwWinner = m_data.dwGuildTo;
        }
    }

    CDBManager::instance().AsyncQuery(
        fmt::sprintf("UPDATE guild_war_reservation SET "
                     "started=1,winner=%u,result1=%d,result2=%d WHERE id=%u",
                     dwWinner, iScoreFrom, iScoreTo, m_data.dwID));

    if (mapBet.empty())
        return;

    uint32_t dwTotalBet = m_data.dwBetFrom + m_data.dwBetTo;
    uint32_t dwWinnerBet = 0;

    if (dwWinner == m_data.dwGuildFrom)
        dwWinnerBet = m_data.dwBetFrom;
    else if (dwWinner == m_data.dwGuildTo)
        dwWinnerBet = m_data.dwBetTo;
    else {
        spdlog::error("WAR_REWARD: fatal error, winner does not exist!");
        return;
    }

    if (dwWinnerBet == 0) {
        spdlog::error("WAR_REWARD: total bet money on winner is zero");
        return;
    }

    SPDLOG_INFO("WAR_REWARD: End: Total bet: {0}, Winner bet: {1}", dwTotalBet,
                dwWinnerBet);

    auto it = mapBet.begin();
    std::string query = "INSERT INTO item_award (login, vnum, socket0, "
                        "given_time) VALUES";
    while (1) {
        int iLen = 0;
        int iRow = 0;

        while (it != mapBet.end()) {
            if (it->second.first != dwWinner) {
                ++it;
                continue;
            }

            double ratio = (double)it->second.second / dwWinnerBet;

            // 10% 세금 공제 후 분배
            SPDLOG_INFO("WAR_REWARD: {0} {1} ratio {2}", it->first,
                        it->second.second, ratio);

            uint32_t dwGold = (uint32_t)(dwTotalBet * ratio * 0.9);

            if (iRow == 0)
                query += fmt::sprintf("('%s', %d, %u, NOW())",
                                      it->first.c_str(), ITEM_ELK_VNUM, dwGold);
            else
                query += fmt::sprintf(",('%s', %d, %u, NOW())",
                                      it->first.c_str(), ITEM_ELK_VNUM, dwGold);

            ++it;

            if (iLen > 384)
                break;

            ++iRow;
        }

        if (iRow > 0) {
            SPDLOG_DEBUG("WAR_REWARD: query: {0}", query);
            CDBManager::instance().AsyncQuery(query);
        }

        if (it == mapBet.end())
            break;
    }
}

#ifdef __DUNGEON_FOR_GUILD__
void CGuildManager::Dungeon(uint32_t dwGuildID, uint8_t bChannel,
                            long lMapIndex)
{
    TPacketDGGuildDungeon sPacket;
    sPacket.dwGuildID = dwGuildID;
    sPacket.bChannel = bChannel;
    sPacket.lMapIndex = lMapIndex;
    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_DUNGEON, &sPacket,
                                             sizeof(sPacket));

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET dungeon_ch "
                                                  "= {}, dungeon_map = {} "
                                                  "WHERE id = {};",
                                                  bChannel, lMapIndex,
                                                  dwGuildID));
}

void CGuildManager::DungeonCooldown(uint32_t dwGuildID, uint32_t dwTime)
{
    TPacketDGGuildDungeonCD sPacket;
    sPacket.dwGuildID = dwGuildID;
    sPacket.dwTime = dwTime;
    CClientManager::instance().ForwardPacket(HEADER_DG_GUILD_DUNGEON_CD,
                                             &sPacket, sizeof(sPacket));

    CDBManager::instance().AsyncQuery(fmt::format("UPDATE guild SET "
                                                  "dungeon_cooldown = {} WHERE "
                                                  "id = {};",
                                                  dwTime, dwGuildID));
}
#endif