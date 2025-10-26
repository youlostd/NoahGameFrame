#include "guild_manager.h"

#include "char.h"
#include "ChatUtil.hpp"
#include "config.h"
#include "db.h"
#include "DbCacheSocket.hpp"
#include "desc.h"
#include "GBufferManager.h"
#include "guild.h"
#include "locale_service.h"
#include "MarkManager.h"
#include "questmanager.h"
#include "war_map.h"

#include <game/Constants.hpp>
#include <game/DbPackets.hpp>
#include <game/GamePacket.hpp>


CGuildManager::CGuildManager()
{
}

CGuildManager::~CGuildManager()
{
    for (auto iter = m_mapGuild.begin(); iter != m_mapGuild.end(); ++iter)
        delete iter->second;

    m_mapGuild.clear();
}

int CGuildManager::GetDisbandDelay()
{
    return quest::CQuestManager::instance().GetEventFlag("guild_disband_delay") * 86400;
}

int CGuildManager::GetWithdrawDelay()
{
    return quest::CQuestManager::instance().GetEventFlag("guild_withdraw_delay") * 86400;
}

uint32_t CGuildManager::CreateGuild(TGuildCreateParameter &gcp)
{
    if (!gcp.master)
        return 0;

    if (!check_name(gcp.name))
    {
        SendI18nChatPacket(gcp.master, CHAT_TYPE_INFO, "<길드> 길드 이름이 적합하지 않습니다.");
        return 0;
    }

    std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM guild WHERE name = '{}'",
                                                                   gcp.name));

    if (pmsg->Get()->uiNumRows > 0)
    {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

        if (!(row[0] && row[0][0] == '0'))
        {
            SendI18nChatPacket(gcp.master, CHAT_TYPE_INFO, "<길드> 이미 같은 이름의 길드가 있습니다.");
            return 0;
        }
    }
    else
    {
        SendI18nChatPacket(gcp.master, CHAT_TYPE_INFO, "<길드> 길드를 생성할 수 없습니다.");
        return 0;
    }

    // new CGuild(gcp) queries guild tables and tell dbcache to notice other game servers.
    // other game server calls CGuildManager::LoadGuild to load guild.
    CGuild *pg = new CGuild(gcp);
    m_mapGuild.emplace(pg->GetID(), pg);
    return pg->GetID();
}

void CGuildManager::Unlink(uint32_t pid) { m_map_pkGuildByPID.erase(pid); }

CGuild *CGuildManager::GetLinkedGuild(uint32_t pid)
{
    TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

    if (it == m_map_pkGuildByPID.end())
        return nullptr;

    return it->second;
}

void CGuildManager::Link(uint32_t pid, CGuild *guild)
{
    if (m_map_pkGuildByPID.find(pid) == m_map_pkGuildByPID.end())
        m_map_pkGuildByPID.emplace(pid, guild);
}

void CGuildManager::P2PLogoutMember(uint32_t pid)
{
    TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

    if (it != m_map_pkGuildByPID.end()) { it->second->P2PLogoutMember(pid); }
}

void CGuildManager::P2PLoginMember(uint32_t pid)
{
    TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

    if (it != m_map_pkGuildByPID.end()) { it->second->P2PLoginMember(pid); }
}

void CGuildManager::LoginMember(CHARACTER *ch)
{
    TGuildMap::iterator it = m_map_pkGuildByPID.find(ch->GetPlayerID());

    if (it != m_map_pkGuildByPID.end()) { it->second->LoginMember(ch); }
}

CGuild *CGuildManager::TouchGuild(uint32_t guild_id)
{
    TGuildMap::iterator it = m_mapGuild.find(guild_id);

    if (it == m_mapGuild.end())
    {
        m_mapGuild.emplace(guild_id, new CGuild(guild_id));
        it = m_mapGuild.find(guild_id);

        g_pCharManager->for_each_pc(FGuildNameSender(guild_id, it->second->GetName()));
    }

    return it->second;
}

CGuild *CGuildManager::FindGuild(uint32_t guild_id)
{
    TGuildMap::iterator it = m_mapGuild.find(guild_id);
    if (it == m_mapGuild.end())
        return nullptr;

    return it->second;
}

CGuild *CGuildManager::FindGuildByName(const std::string &guild_name)
{
    for (auto it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it)
    {
        if (it->second->GetName() == guild_name)
            return it->second;
    }
    return nullptr;
}

void CGuildManager::Initialize()
{
    SPDLOG_INFO("Initializing Guild");

    if (gConfig.authServer)
    {
        SPDLOG_INFO("	No need for auth server");
        return;
    }

    std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery("SELECT id FROM guild"));

    for (const auto& row : pmsg->Get()->rows)
    {
        uint32_t guild_id = 0;
        storm::ParseNumber(row[0], guild_id);

        DBManager::instance().Query("UPDATE guild SET dungeon_ch = 0, dungeon_map = 0 WHERE id = {};", guild_id);
        LoadGuild(guild_id);
    }

    CGuildMarkManager &rkMarkMgr = CGuildMarkManager::instance();

    rkMarkMgr.SetMarkPathPrefix("mark");

    rkMarkMgr.LoadMarkIndex();
    rkMarkMgr.LoadMarkImages();
    rkMarkMgr.LoadSymbol(GUILD_SYMBOL_FILENAME);
}

void CGuildManager::LoadGuild(uint32_t guild_id)
{
    TGuildMap::iterator it = m_mapGuild.find(guild_id);

    if (it == m_mapGuild.end()) { m_mapGuild.emplace(guild_id, new CGuild(guild_id)); }
    else
    {
        //it->second->Load(guild_id);
    }
}

void CGuildManager::DisbandGuild(uint32_t guild_id)
{
    TGuildMap::iterator it = m_mapGuild.find(guild_id);

    if (it == m_mapGuild.end())
        return;

    it->second->Disband();

    delete it->second;
    m_mapGuild.erase(it);

    CGuildMarkManager::instance().DeleteMark(guild_id);
}

void CGuildManager::SkillRecharge()
{
    for (TGuildMap::iterator it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it) { it->second->SkillRecharge(); }
}

int CGuildManager::GetRank(CGuild *g)
{
    int point = g->GetLadderPoint();
    int rank = 1;

    for (auto it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it)
    {
        CGuild *pg = it->second;

        if (pg->GetLadderPoint() > point)
            rank++;
    }

    return rank;
}

std::string CGuildManager::GetHighRankString(uint32_t dwMyGuild)
{
    using namespace std;
    vector<CGuild *> v;
    std::string buffer;

    for (auto it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it)
    {
        if (it->second)
            v.push_back(it->second);
    }

    std::sort(v.begin(), v.end(), []
          (CGuild *g1, CGuild *g2)
              {
                  if (g1->GetLadderPoint() < g2->GetLadderPoint())
                      return true;
                  if (g1->GetLadderPoint() > g2->GetLadderPoint())
                      return false;
                  if (g1->GetGuildWarWinCount() < g2->GetGuildWarWinCount())
                      return true;
                  if (g1->GetGuildWarWinCount() > g2->GetGuildWarWinCount())
                      return false;
                  if (g1->GetGuildWarLossCount() < g2->GetGuildWarLossCount())
                      return true;
                  if (g1->GetGuildWarLossCount() > g2->GetGuildWarLossCount())
                      return false;
                  int c = strcmp(g1->GetName(), g2->GetName());
                  if (c > 0)
                      return true;
                  return false;
              });
    int n = v.size();
    int len = 0, len2;

    for (int i = 0; i < 8; ++i)
    {
        if (n - i - 1 < 0)
            break;

        CGuild *g = v[n - i - 1];

        if (!g)
            continue;

        if (g->GetLadderPoint() <= 0)
            break;

        if (dwMyGuild == g->GetID()) { buffer += "[COLOR r;255|g;255|b;0]"; }

        if (i) { buffer += "[ENTER]"; }

        buffer += fmt::sprintf("%3d | %-22s | %-8d | %4d | %4d | %4d",
                               GetRank(g),
                               g->GetName(),
                               g->GetLadderPoint(),
                               g->GetGuildWarWinCount(),
                               g->GetGuildWarDrawCount(),
                               g->GetGuildWarLossCount());

        if (g->GetID() == dwMyGuild) { buffer += "[/COLOR]"; }
    }

    return buffer;
}

std::string CGuildManager::GetAroundRankString(uint32_t dwMyGuild)
{
    using namespace std;
    std::string buffer;

    vector<CGuild *> v;

    for (auto it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it)
    {
        if (it->second)
            v.push_back(it->second);
    }

    std::sort(v.begin(), v.end(), []
          (CGuild *g1, CGuild *g2)
              {
                  if (g1->GetLadderPoint() < g2->GetLadderPoint())
                      return true;
                  if (g1->GetLadderPoint() > g2->GetLadderPoint())
                      return false;
                  if (g1->GetGuildWarWinCount() < g2->GetGuildWarWinCount())
                      return true;
                  if (g1->GetGuildWarWinCount() > g2->GetGuildWarWinCount())
                      return false;
                  if (g1->GetGuildWarLossCount() < g2->GetGuildWarLossCount())
                      return true;
                  if (g1->GetGuildWarLossCount() > g2->GetGuildWarLossCount())
                      return false;
                  int c = strcmp(g1->GetName(), g2->GetName());
                  if (c > 0)
                      return true;
                  return false;
              });
    int n = v.size();
    int idx;

    for (idx = 0; idx < (int)v.size(); ++idx)
        if (v[idx]->GetID() == dwMyGuild)
            break;

    int len = 0, len2;
    int count = 0;

    for (int i = -3; i <= 3; ++i)
    {
        if (idx - i < 0)
            continue;

        if (idx - i >= n)
            continue;

        CGuild *g = v[idx - i];

        if (!g)
            continue;

        if (dwMyGuild == g->GetID()) { buffer += "[COLOR r;255|g;255|b;0]"; }

        if (count) { buffer += "[ENTER]"; }

        buffer += fmt::sprintf("%3d | %-22s | %-8d | %4d | %4d | %4d",
                               GetRank(g),
                               g->GetName(),
                               g->GetLadderPoint(),
                               g->GetGuildWarWinCount(),
                               g->GetGuildWarDrawCount(),
                               g->GetGuildWarLossCount());

        ++count;

        if (g->GetID() == dwMyGuild) { buffer += "[/COLOR]"; }
    }
    return buffer;
}

/////////////////////////////////////////////////////////////////////
// Guild War
/////////////////////////////////////////////////////////////////////
void CGuildManager::RequestCancelWar(uint32_t guild_id1, uint32_t guild_id2)
{
    SPDLOG_INFO("RequestCancelWar {0} {1}", guild_id1, guild_id2);

    TPacketGuildWar p;
    p.bWar = GUILD_WAR_CANCEL;
    p.dwGuildFrom = guild_id1;
    p.dwGuildTo = guild_id2;
    p.isRanked = 0;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
}

void CGuildManager::RequestEndWar(uint32_t guild_id1, uint32_t guild_id2)
{
    SPDLOG_INFO("RequestEndWar {0} {1}", guild_id1, guild_id2);

    TPacketGuildWar p;
    p.bWar = GUILD_WAR_END;
    p.dwGuildFrom = guild_id1;
    p.dwGuildTo = guild_id2;
    p.isRanked = 0;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
}

void CGuildManager::RequestWarOver(uint32_t dwGuild1, uint32_t dwGuild2, uint32_t dwGuildWinner, int32_t lReward)
{
    CGuild *g1 = TouchGuild(dwGuild1);
    CGuild *g2 = TouchGuild(dwGuild2);

    if (g1->GetGuildWarState(g2->GetID()) != GUILD_WAR_ON_WAR)
    {
        SPDLOG_INFO("RequestWarOver : both guild was not in war {0} {1}", dwGuild1, dwGuild2);
        RequestEndWar(dwGuild1, dwGuild2);
        return;
    }

    TPacketGuildWar p;
    p.isRanked = 0;
    p.bWar = GUILD_WAR_OVER;
    p.bType = dwGuildWinner == 0 ? 1 : 0; // bType == 1 means draw for this packet.

    if (dwGuildWinner == 0)
    {
        p.dwGuildFrom = dwGuild1;
        p.dwGuildTo = dwGuild2;
    }
    else
    {
        p.dwGuildFrom = dwGuildWinner;
        p.dwGuildTo = dwGuildWinner == dwGuild1 ? dwGuild2 : dwGuild1;
    }

    db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
    SPDLOG_INFO("RequestWarOver : winner {0} loser {1} draw {2}", p.dwGuildFrom, p.dwGuildTo, p.bType);
}

void CGuildManager::DeclareWar(uint32_t guild_id1, uint32_t guild_id2, uint8_t bType)
{
    if (guild_id1 == guild_id2)
        return;

    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (!g1 || !g2)
        return;

    if (g1->DeclareWar(guild_id2, bType, GUILD_WAR_SEND_DECLARE) &&
        g2->DeclareWar(guild_id1, bType, GUILD_WAR_RECV_DECLARE))
    {
        g1->MasterChatLocalized(fmt::format("%s 길드가 %s 길드에 선전포고를 하였습니다!#{};{}", TouchGuild(guild_id1)->GetName(),
                                            TouchGuild(guild_id2)->GetName()));
        g2->MasterChatLocalized(fmt::format("%s 길드가 %s 길드에 선전포고를 하였습니다!#{};{}", TouchGuild(guild_id1)->GetName(),
                                            TouchGuild(guild_id2)->GetName()));
    }
}

void CGuildManager::RefuseWar(uint32_t guild_id1, uint32_t guild_id2)
{
    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (g1 && g2)
    {
        if (g2->GetMasterCharacter())
            SendI18nChatPacket(g2->GetMasterCharacter(), CHAT_TYPE_INFO, "<길드> %s 길드가 길드전을 거부하였습니다.", g1->GetName());
    }

    if (g1 != nullptr)
        g1->RefuseWar(guild_id2);

    if (g2 != nullptr && g1 != nullptr)
        g2->RefuseWar(g1->GetID());
}

void CGuildManager::WaitStartWar(uint32_t guild_id1, uint32_t guild_id2)
{
    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (!g1 || !g2)
    {
        SPDLOG_INFO(
            "GuildWar: CGuildManager::WaitStartWar({0},{1}) - something wrong in arg. there is no guild like that.",
            guild_id1, guild_id2);
        return;
    }

    if (g1->WaitStartWar(guild_id2) || g2->WaitStartWar(guild_id1))
    {
        SendI18nNotice("%s 길드와 %s 길드가 잠시 후 전쟁을 시작합니다!", g1->GetName(), g2->GetName());
    }
}

struct FSendWarList
{
    FSendWarList(uint8_t subheader, uint32_t guild_id1, uint32_t guild_id2)
    {
        GuildWarData gwd;
        gwd.gid1 = guild_id1;
        gwd.gid2 = guild_id2;
        std::vector<GuildWarData> v;
        v.push_back(gwd);
        p.subheader = subheader;
        p.guildWars = v;
    }

    void operator()(CHARACTER *ch)
    {
        DESC *d = ch->GetDesc();

        if (d)
        {
            d->Send(HEADER_GC_GUILD, p);
        }
    }

    TPacketGCGuild p{};
};

void CGuildManager::StartWar(uint32_t guild_id1, uint32_t guild_id2)
{
    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (!g1 || !g2)
        return;

    if (!g1->CheckStartWar(guild_id2) || !g2->CheckStartWar(guild_id1))
        return;

    g1->StartWar(guild_id2);
    g2->StartWar(guild_id1);

    SendI18nNotice("%s 길드와 %s 길드가 전쟁을 시작하였습니다!", g1->GetName(), g2->GetName());

    if (guild_id1 > guild_id2)
        std::swap(guild_id1, guild_id2);

    const auto f = FSendWarList(GUILD_SUBHEADER_GC_GUILD_WAR_LIST, guild_id1, guild_id2);
    g_pCharManager->for_each_pc(f);
    m_GuildWar.emplace(guild_id1, guild_id2);
}

void SendGuildWarOverNotice(CGuild *g1, CGuild *g2, bool bDraw)
{
    if (g1 && g2)
    {
        std::string formatString;

        if (bDraw) { formatString = "%s 길드와 %s 길드 사이의 전쟁이 무승부로 끝났습니다.";; }
        else
        {
            if (g1->GetWarScoreAgainstTo(g2->GetID()) > g2->GetWarScoreAgainstTo(g1->GetID()))
            {
                formatString = "%s 길드가 %s 길드와의 전쟁에서 승리 했습니다.";
            }
            else { formatString = "%s 길드가 %s 길드와의 전쟁에서 승리 했습니다."; }
        }

        SendI18nNotice(formatString.c_str(), g1->GetName(), g2->GetName());
    }
}

bool CGuildManager::EndWar(uint32_t guild_id1, uint32_t guild_id2)
{
    if (guild_id1 > guild_id2)
        std::swap(guild_id1, guild_id2);

    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    std::pair<uint32_t, uint32_t> k = std::make_pair(guild_id1, guild_id2);

    auto it = m_GuildWar.find(k);

    if (m_GuildWar.end() == it)
    {
        SPDLOG_INFO("EndWar({0},{1}) - EndWar request but guild is not in list", guild_id1, guild_id2);
        return false;
    }

    if (g1 && g2)
    {
        if (g1->GetGuildWarType(guild_id2) == GUILD_WAR_TYPE_FIELD)
        {
            SendGuildWarOverNotice(g1, g2, g1->GetWarScoreAgainstTo(guild_id2) == g2->GetWarScoreAgainstTo(guild_id1));
        }
    }
    else { return false; }

    if (g1)
        g1->EndWar(guild_id2);

    if (g2)
        g2->EndWar(guild_id1);

    m_GuildWarEndTime[k] = get_global_time();
    const auto f = FSendWarList(GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST, guild_id1, guild_id2);
    g_pCharManager->for_each_pc(f);
    m_GuildWar.erase(it);

    return true;
}

void CGuildManager::WarOver(uint32_t guild_id1, uint32_t guild_id2, bool bDraw)
{
    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (guild_id1 > guild_id2)
        std::swap(guild_id1, guild_id2);

    std::pair<uint32_t, uint32_t> k = std::make_pair(guild_id1, guild_id2);

    auto it = m_GuildWar.find(k);

    if (m_GuildWar.end() == it)
        return;

    SendGuildWarOverNotice(g1, g2, bDraw);

    EndWar(guild_id1, guild_id2);
}

void CGuildManager::CancelWar(uint32_t guild_id1, uint32_t guild_id2)
{
    if (!EndWar(guild_id1, guild_id2))
        return;

    CGuild *g1 = FindGuild(guild_id1);
    CGuild *g2 = FindGuild(guild_id2);

    if (g1)
    {
        CHARACTER *master1 = g1->GetMasterCharacter();

        if (master1)
            SendI18nChatPacket(master1, CHAT_TYPE_INFO, "<길드> 길드전이 취소 되었습니다.");
    }

    if (g2)
    {
        CHARACTER *master2 = g2->GetMasterCharacter();

        if (master2)
            SendI18nChatPacket(master2, CHAT_TYPE_INFO, "<길드> 길드전이 취소 되었습니다.");
    }

    if (g1 && g2) { SendI18nNotice("%s 길드와 %s 길드 사이의 전쟁이 취소되었습니다.", g1->GetName(), g2->GetName()); }
}

void CGuildManager::ReserveWar(uint32_t dwGuild1, uint32_t dwGuild2, uint8_t bType) // from DB
{
    SPDLOG_INFO("GuildManager::ReserveWar {0} {1}", dwGuild1, dwGuild2);

    CGuild *g1 = FindGuild(dwGuild1);
    CGuild *g2 = FindGuild(dwGuild2);

    if (!g1 || !g2)
        return;

    g1->ReserveWar(dwGuild2, bType);
    g2->ReserveWar(dwGuild1, bType);
}

void CGuildManager::ShowGuildWarList(CHARACTER *ch)
{
    for (auto it = m_GuildWar.begin(); it != m_GuildWar.end(); ++it)
    {
        CGuild *A = TouchGuild(it->first);
        CGuild *B = TouchGuild(it->second);

        if (A && B)
        {
            ch->ChatPacket(CHAT_TYPE_NOTICE, "%s[%d] vs %s[%d] time %u sec.",
                           A->GetName(), A->GetID(),
                           B->GetName(), B->GetID(),
                           get_global_time() - A->GetWarStartTime(B->GetID()));
        }
    }
}

void CGuildManager::SendGuildWar(CHARACTER *ch)
{
    if (!ch->GetDesc())
        return;

    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_GUILD_WAR_LIST;
    std::vector<GuildWarData> gwds;

    for (auto it = m_GuildWar.begin(); it != m_GuildWar.end(); ++it)
    {
        gwds.emplace_back(GuildWarData{it->first, it->second});

    }
    p.guildWars = gwds;

    ch->GetDesc()->Send(HEADER_GC_GUILD, p);
}

void SendGuildWarScore(uint32_t dwGuild, uint32_t dwGuildOpp, int iDelta, int iBetScoreDelta)
{
    TPacketGuildWarScore p;

    p.dwGuildGainPoint = dwGuild;
    p.dwGuildOpponent = dwGuildOpp;
    p.lScore = iDelta;
    p.lBetScore = iBetScoreDelta;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR_SCORE, 0, &p, sizeof(TPacketGuildWarScore));
    SPDLOG_INFO("SendGuildWarScore {0} {1} {2}", dwGuild, dwGuildOpp, iDelta);
}

void CGuildManager::Kill(CHARACTER *killer, CHARACTER *victim)
{
    if (!killer->IsPC())
        return;

    if (!victim->IsPC())
        return;

    if (killer->GetWarMap())
    {
        killer->GetWarMap()->OnKill(killer, victim);
        return;
    }

    CGuild *gAttack = killer->GetGuild();
    CGuild *gDefend = victim->GetGuild();

    if (!gAttack || !gDefend)
        return;

    if (gAttack->GetGuildWarType(gDefend->GetID()) != GUILD_WAR_TYPE_FIELD)
        return;

    if (!gAttack->UnderWar(gDefend->GetID()))
        return;

    SendGuildWarScore(gAttack->GetID(), gDefend->GetID(), victim->GetLevel());
}

void CGuildManager::StopAllGuildWar()
{
    for (auto it = m_GuildWar.begin(); it != m_GuildWar.end(); ++it)
    {
        CGuild *g = CGuildManager::instance().TouchGuild(it->first);
        CGuild *pg = CGuildManager::instance().TouchGuild(it->second);
        g->EndWar(it->second);
        pg->EndWar(it->first);
    }

    m_GuildWar.clear();
}

void CGuildManager::ReserveWarAdd(TGuildWarReserve *p)
{
    auto it = m_map_kReserveWar.find(p->dwID);

    CGuildWarReserveForGame *pkReserve;

    if (it != m_map_kReserveWar.end())
        pkReserve = it->second;
    else
    {
        pkReserve = new CGuildWarReserveForGame;

        m_map_kReserveWar.emplace(p->dwID, pkReserve);
        m_vec_kReserveWar.push_back(pkReserve);
    }

    memcpy(&pkReserve->data, p, sizeof(TGuildWarReserve));

    SPDLOG_INFO("ReserveWarAdd {0} gid1 {1} gid2 {2}",
                pkReserve->data.dwID, p->dwGuildFrom, p->dwGuildTo);
}

bool CGuildManager::IsBet(uint32_t dwID, const char *c_pszLogin)
{
    auto it = m_map_kReserveWar.find(dwID);

    if (it == m_map_kReserveWar.end())
        return true;

    return it->second->mapBet.end() != it->second->mapBet.find(c_pszLogin);
}

void CGuildManager::ReserveWarDelete(uint32_t dwID)
{
    SPDLOG_INFO("ReserveWarDelete {0}", dwID);
    auto it = m_map_kReserveWar.find(dwID);

    if (it == m_map_kReserveWar.end())
        return;

    auto it_vec = m_vec_kReserveWar.begin();

    while (it_vec != m_vec_kReserveWar.end())
    {
        if (*it_vec == it->second)
        {
            it_vec = m_vec_kReserveWar.erase(it_vec);
            break;
        }
        else
            ++it_vec;
    }

    delete it->second;
    m_map_kReserveWar.erase(it);
}

std::vector<CGuildWarReserveForGame *> &CGuildManager::GetReserveWarRef() { return m_vec_kReserveWar; }

//
// End of Guild War
//

void CGuildManager::ChangeMaster(uint32_t dwGID)
{
    auto iter = m_mapGuild.find(dwGID);

    if (iter == m_mapGuild.end())
        return;

    CGuild *guild = iter->second;

    if (!guild)
        return;

    //BugFix for Guilds losing member limits after a load triggered! <MartPwnS | 16.01.2015> #17
    guild->Load(dwGID, guild->GetMemberCountBonus());
    //BugFix end

    // Information update loop
    guild->SendGuildDataUpdateToAllMember();
}
