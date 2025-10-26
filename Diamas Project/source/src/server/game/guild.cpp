#include "guild.h"
#include "DbCacheSocket.hpp"
#include "GBufferManager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "guild_manager.h"
#include "utils.h"
#include <game/AffectConstants.hpp>

#include "building.h"
#include "desc.h"
#include "desc_manager.h"
#include "gm.h"
#include "locale_service.h"
#include "log.h"
#include "main.h"
#include "messenger_manager.h"
#include "questmanager.h"
#include "war_map.h"
#include <game/MasterPackets.hpp>

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

TPacketGCGuildSubMember SGuildMember::PacketData() const
{
    TPacketGCGuildSubMember d;
    d.pid = pid;
    d.byGrade = grade;
    d.byIsGeneral = is_general;
    d.byJob = job;
    d.byLevel = level;
    d.dwOffer = offer_exp;
    d.name = name;
    return d;
}

SGuildMember::SGuildMember(CHARACTER* ch, uint8_t grade, uint32_t offer_exp)
    : pid(ch->GetPlayerID())
    , grade(grade)
    , is_general(0)
    , job(ch->GetJob())
    , level(ch->GetLevel())
    , offer_exp(offer_exp)
    , _dummy(0)
    , name(ch->GetName())
{
}

SGuildMember::SGuildMember(uint32_t pid, uint8_t grade, uint8_t is_general,
                           uint8_t job, uint8_t level, uint32_t offer_exp,
                           const char* name)
    : pid(pid)
    , grade(grade)
    , is_general(is_general)
    , job(job)
    , level(level)
    , offer_exp(offer_exp)
    , _dummy(0)
    , name(name)
{
}

CGuild::CGuild(TGuildCreateParameter& cp)
{
    Initialize();

#ifdef ENABLE_GUILD_STORAGE
    m_pkStorage = new CGuildStorage(this);

#endif

    m_general_count = 0;

    m_iMemberCountBonus = 0;

    m_data.name = cp.name;
    m_data.master_pid = cp.master->GetPlayerID();
    const auto* locale = GetLocale(cp.master);

    m_data.grade_array[0].grade_name = LC_TEXT_LC("길드장", locale);

#ifdef ENABLE_GUILD_STORAGE
    m_data.grade_array[0].auth_flag =
        GUILD_AUTH_ADD_MEMBER | GUILD_AUTH_REMOVE_MEMBER | GUILD_AUTH_NOTICE |
        GUILD_AUTH_USE_SKILL | GUILD_AUTH_PUT_GOLD | GUILD_AUTH_TAKE_GOLD |
        GUILD_AUTH_USE_GS | GUILD_AUTH_GS1_PUTITEM | GUILD_AUTH_GS1_TAKEITEM |
        GUILD_AUTH_GS2_PUTITEM | GUILD_AUTH_GS2_TAKEITEM |
        GUILD_AUTH_GS3_PUTITEM | GUILD_AUTH_GS3_TAKEITEM |
        GUILD_AUTH_GS4_PUTITEM | GUILD_AUTH_GS4_TAKEITEM |
        GUILD_AUTH_GS5_PUTITEM | GUILD_AUTH_GS5_TAKEITEM |
        GUILD_AUTH_GS6_PUTITEM | GUILD_AUTH_GS6_TAKEITEM |
        GUILD_AUTH_GS7_PUTITEM | GUILD_AUTH_GS7_TAKEITEM |
        GUILD_AUTH_GS8_PUTITEM | GUILD_AUTH_GS8_TAKEITEM |
        GUILD_AUTH_GS9_PUTITEM | GUILD_AUTH_GS9_TAKEITEM |
        GUILD_AUTH_GS10_PUTITEM | GUILD_AUTH_GS10_TAKEITEM | GUILD_AUTH_LOG |
        GUILD_AUTH_MANAGE;
#else
    m_data.grade_array[0].auth_flag = GUILD_AUTH_ADD_MEMBER |
                                      GUILD_AUTH_REMOVE_MEMBER |
                                      GUILD_AUTH_NOTICE | GUILD_AUTH_USE_SKILL;

#endif

    for (int i = 1; i < GUILD_GRADE_COUNT; ++i) {
        m_data.grade_array[i].grade_name = LC_TEXT_LC("길드원", locale);
        m_data.grade_array[i].auth_flag = 0;
    }
    // <MartPwnS> Start with 1000 Elo instead of 0 [27.07.2014]
    std::unique_ptr<SQLMsg> pmsg(
        DBManager::instance().DirectQuery("INSERT INTO guild(name, master, sp, "
                                          "level, exp, skill_point, "
                                          "ladder_point) "
                                          "VALUES('{}', {}, 1000, 20, 0, 0, "
                                          "1000)",
                                          m_data.name, m_data.master_pid));
    
    // TODO if error occur?
    m_data.guild_id = (uint32_t)pmsg->Get()->uiInsertID;

    for (int i = 0; i < GUILD_GRADE_COUNT; ++i) {
        DBManager::instance().Query("INSERT INTO guild_grade VALUES({}, {}, "
                                    "'{}', {})",
                                    m_data.guild_id, i + 1,
                                    m_data.grade_array[i].grade_name,
                                    m_data.grade_array[i].auth_flag);
    }

    ComputeGuildPoints();
    m_data.skill_point = 19;
    m_data.power = m_data.max_power;
    m_data.ladder_point = 19000;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_CREATE, 0, &m_data.guild_id,
                            sizeof(uint32_t));

    TPacketGuildSkillUpdate guild_skill;
    guild_skill.guild_id = m_data.guild_id;
    guild_skill.amount = 0;
    guild_skill.skill_point = 19;
    memset(guild_skill.skill_levels, 0, GUILD_SKILL_COUNT);

    db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill,
                            sizeof(guild_skill));

    g_pCharManager->for_each_pc(FGuildNameSender(GetID(), GetName()));
    RequestAddMember(cp.master, GUILD_LEADER_GRADE);
}

void CGuild::Initialize()
{
    m_data = {};
    m_data.level = 20;

    for (int i = 0; i < GUILD_SKILL_COUNT; ++i)
        abSkillUsable[i] = true;

    m_iMemberCountBonus = 0;
}

CGuild::~CGuild() {}

void CGuild::RequestAddMember(CHARACTER* ch, int grade)
{
    if (ch->GetGuild())
        return;

    TPacketGDGuildAddMember gd;

    if (m_member.find(ch->GetPlayerID()) != m_member.end()) {
        SPDLOG_ERROR("Already a member in guild {0}[{1}]", ch->GetName(),
                     ch->GetPlayerID());
        return;
    }

    gd.dwPID = ch->GetPlayerID();
    gd.dwGuild = GetID();
    gd.bGrade = grade;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_ADD_MEMBER, 0, &gd,
                            sizeof(TPacketGDGuildAddMember));
}

void CGuild::AddMember(TPacketDGGuildMember* p)
{
    TGuildMemberContainer::iterator it;

    if ((it = m_member.find(p->dwPID)) == m_member.end())
        m_member.emplace(
            p->dwPID, TGuildMember(p->dwPID, p->bGrade, p->isGeneral, p->bJob,
                                   p->bLevel, p->dwOffer, p->szName));
    else {
        TGuildMember& r_gm = it->second;
        r_gm.pid = p->dwPID;
        r_gm.grade = p->bGrade;
        r_gm.job = p->bJob;
        r_gm.offer_exp = p->dwOffer;
        r_gm.is_general = p->isGeneral;
    }

    CGuildManager::instance().Link(p->dwPID, this);

    SendListOneToAll(p->dwPID);

    CHARACTER* ch = g_pCharManager->FindByPID(p->dwPID);

    SPDLOG_INFO("GUILD: AddMember PID {0}, grade {1}, job {2}, level {3}, "
                "offer {4}, name {5} ptr {6}",
                p->dwPID, p->bGrade, p->bJob, p->bLevel, p->dwOffer, p->szName,
                (void*)ch);

    if (ch) {
        ch->SetQuestFlag("meleylair_zone.next_entry",
                         get_global_time() + (60 * 60 * 3));
        LoginMember(ch);
    } else
        P2PLoginMember(p->dwPID);
}

bool CGuild::RequestRemoveMember(uint32_t pid)
{
    TGuildMemberContainer::iterator it;

    if ((it = m_member.find(pid)) == m_member.end())
        return false;

    if (it->second.grade == GUILD_LEADER_GRADE)
        return false;

    TPacketGuild gd_guild;

    gd_guild.dwGuild = GetID();
    gd_guild.dwInfo = pid;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_REMOVE_MEMBER, 0, &gd_guild,
                            sizeof(TPacketGuild));
    return true;
}

bool CGuild::RemoveMember(uint32_t pid)
{
    SPDLOG_INFO("Receive Guild P2P RemoveMember");
    TGuildMemberContainer::iterator it;

    if ((it = m_member.find(pid)) == m_member.end())
        return false;

    if (it->second.grade == GUILD_LEADER_GRADE)
        return false;

    if (it->second.is_general)
        m_general_count--;

    m_member.erase(it);
    SendOnlineRemoveOnePacket(pid);

    CGuildManager::instance().Unlink(pid);

    CHARACTER* ch = g_pCharManager->FindByPID(pid);

    if (ch) {
        m_memberOnline.erase(ch);
        ch->SetGuild(nullptr);
    }

    return true;
}

void CGuild::P2PLoginMember(uint32_t pid)
{
    if (m_member.find(pid) == m_member.end()) {
        SPDLOG_ERROR("GUILD [{0}] is not a memeber of guild.", pid);
        return;
    }

    m_memberP2POnline.insert(pid);

    // Login event occur + Send List
    TGuildMemberOnlineContainer::iterator it;

    for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
        SendLoginPacket(*it, pid);
}

void CGuild::LoginMember(CHARACTER* ch)
{
    if (m_member.find(ch->GetPlayerID()) == m_member.end()) {
        SPDLOG_ERROR("GUILD {0}[{1}] is not a memeber of guild.", ch->GetName(),
                     ch->GetPlayerID());
        return;
    }

    ch->SetGuild(this);

    // Login event occur + Send List
    TGuildMemberOnlineContainer::iterator it;

    for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
        SendLoginPacket(*it, ch);

    m_memberOnline.insert(ch);

    SendAllGradePacket(ch);
    SendGuildInfoPacket(ch);
    SendListPacket(ch);
    SendSkillInfoPacket(ch);
    SendEnemyGuild(ch);

    // GuildUpdateAffect(ch);
}

void CGuild::P2PLogoutMember(uint32_t pid)
{
    if (m_member.find(pid) == m_member.end()) {
        SPDLOG_ERROR("GUILD [%d] is not a member of guild.", pid);
        return;
    }

    m_memberP2POnline.erase(pid);

    // Logout event occur
    TGuildMemberOnlineContainer::iterator it;
    for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
        SendLogoutPacket(*it, pid);
    }
}

void CGuild::LogoutMember(CHARACTER* ch)
{
    if (m_member.find(ch->GetPlayerID()) == m_member.end()) {
        SPDLOG_ERROR("GUILD %s[%d] is not a member of guild.", ch->GetName(),
                     ch->GetPlayerID());
        return;
    }

    // GuildRemoveAffect(ch);

    // ch->SetGuild(NULL);
    m_memberOnline.erase(ch);

    // Logout event occur
    TGuildMemberOnlineContainer::iterator it;
    for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
        SendLogoutPacket(*it, ch);
    }
}

void CGuild::SendOnlineRemoveOnePacket(uint32_t pid)
{
    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_REMOVE;
    pack.pid = pid;

    TGuildMemberOnlineContainer::iterator it;

    for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
        DESC* d = (*it)->GetDesc();

        if (d)
            d->Send(HEADER_GC_GUILD, pack);
    }
}

void CGuild::SendAllGradePacket(CHARACTER* ch)
{
    DESC* d = ch->GetDesc();
    if (!d)
        return;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_GRADE;

    GuildGradeMap m;

    for (int i = 0; i < GUILD_GRADE_COUNT; i++) {
        uint8_t j = i + 1;
        m.emplace(j, m_data.grade_array[i]);
    }

    pack.grades = m;
    pack.opts = {};

    d->Send(HEADER_GC_GUILD, pack);
}

void CGuild::SendListOneToAll(CHARACTER* ch)
{
    SendListOneToAll(ch->GetPlayerID());
}

void CGuild::SendListOneToAll(uint32_t pid)
{
    auto cit = m_member.find(pid);
    if (cit == m_member.end())
        return;

    std::vector<TPacketGCGuildSubMember> v;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_LIST;
    v.emplace_back(cit->second.PacketData());

    for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
        DESC* d = (*it)->GetDesc();
        if (!d)
            continue;

        d->Send(HEADER_GC_GUILD, pack);
    }
}

void CGuild::SendListPacket(CHARACTER* ch)
{
    DESC* d = ch->GetDesc();
    if (!d)
        return;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_LIST;

    std::vector<TPacketGCGuildSubMember> data;
    data.reserve(m_member.size());

    for (const auto& [key, member] : m_member)
        data.push_back(member.PacketData());

    pack.submembers = data;
    d->Send(HEADER_GC_GUILD, pack);

    for (auto it : m_memberOnline) {
        SendLoginPacket(ch, it);
    }

    for (auto it : m_memberP2POnline) {
        SendLoginPacket(ch, it);
    }
}

void CGuild::SendLoginPacket(CHARACTER* ch, CHARACTER* chLogin)
{
    SendLoginPacket(ch, chLogin->GetPlayerID());
}

void CGuild::SendLoginPacket(CHARACTER* ch, uint32_t pid)
{
    /*
   Login Packet
   header 4
   pid 4
 */
    if (!ch->GetDesc())
        return;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_LOGIN;
    pack.pid = pid;

    ch->GetDesc()->Send(HEADER_GC_GUILD, pack);
}

void CGuild::SendLogoutPacket(CHARACTER* ch, CHARACTER* chLogout)
{
    SendLogoutPacket(ch, chLogout->GetPlayerID());
}

void CGuild::SendLogoutPacket(CHARACTER* ch, uint32_t pid)
{
    /*
       Logout Packet
       header 4
       pid 4
     */
    if (!ch->GetDesc())
        return;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_LOGOUT;
    pack.pid = pid;
    ch->GetDesc()->Send(HEADER_GC_GUILD, pack);
}

void CGuild::LoadGuildMemberData(SQLMsg* pmsg)
{
    if (pmsg->Get()->uiNumRows == 0)
        return;

    m_general_count = 0;

    m_member.clear();

    for (uint i = 0; i < pmsg->Get()->uiNumRows; ++i) {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

        uint32_t pid = strtoul(row[0], (char**)nullptr, 10);
        uint8_t grade = (uint8_t)strtoul(row[1], (char**)nullptr, 10);
        uint8_t is_general = 0;

        if (row[2] && *row[2] == '1')
            is_general = 1;

        uint32_t offer = strtoul(row[3], (char**)nullptr, 10);
        uint8_t level = (uint8_t)strtoul(row[4], (char**)nullptr, 10);
        uint8_t job = (uint8_t)strtoul(row[5], (char**)nullptr, 10);
        char* name = row[6];

        if (is_general)
            m_general_count++;

        m_member.emplace(
            pid, TGuildMember(pid, grade, is_general, job, level, offer, name));
        CGuildManager::instance().Link(pid, this);
    }

    SendGuildDataUpdateToAllMember();
}

void CGuild::LoadGuildGradeData(SQLMsg* pmsg)
{
    /*
    // 15개 아닐 가능성 존재
    if (pmsg->Get()->iNumRows != 15)
    {
        SPDLOG_ERROR("Query failed: getting guild grade data. GuildID(%d)",
    GetID()); return;
    }
    */
    for (int i = 1; i < GUILD_GRADE_COUNT; ++i) {
        m_data.grade_array[i].grade_name = LC_TEXT("길드원");
        m_data.grade_array[i].auth_flag = 0;
    }

    for (const auto& row : pmsg->Get()->rows) {
        uint8_t grade = 0;
        storm::ParseNumber(row[0], grade);

        std::string name = row[1];

        uint64_t auth;
        storm::ParseNumber(row[2], auth);

        if (grade >= 1 && grade <= 15) {
            SPDLOG_TRACE("GuildGradeLoad {0} auth {1}", name, auth);
            m_data.grade_array[grade - 1].grade_name = name;
            m_data.grade_array[grade - 1].auth_flag = auth;
        }
    }

    DBManager::instance().FuncQuery(
        std::bind(&CGuild::LoadGuildMemberData, this, std::placeholders::_1),
        "SELECT pid, grade, is_general, offer, level, job, name FROM "
        "guild_member, player WHERE guild_id = %u and pid = id",
        m_data.guild_id);
}

void CGuild::LoadGuildData(SQLMsg* pmsg)
{
    if (pmsg->Get()->uiNumRows == 0) {
        SPDLOG_ERROR("Query failed: getting guild data {}",
                     pmsg->stQuery.c_str());
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
    m_data.master_pid = strtoul(row[0], (char**)NULL, 10);
    m_data.level =
        std::min<Level>(strtoul(row[1], nullptr, 10), GUILD_MAX_LEVEL);
    m_data.exp = strtoul(row[2], nullptr, 10);
    m_data.name = row[3];

    m_data.skill_point = (uint8_t)strtoul(row[4], nullptr, 10);
    memset(m_data.abySkill, 0, sizeof(uint8_t) * GUILD_SKILL_COUNT);

    m_data.power = std::max<int>(0, strtoul(row[5], nullptr, 10));

    str_to_number(m_data.ladder_point, row[6]);

    if (m_data.ladder_point < 0)
        m_data.ladder_point = 0;

    storm::ParseNumber(row[7], m_data.win);
    storm::ParseNumber(row[8], m_data.draw);
    storm::ParseNumber(row[9], m_data.loss);
    storm::ParseNumber(row[10], m_data.gold);

#ifdef __DUNGEON_FOR_GUILD__
    storm::ParseNumber(row[11], m_data.dungeon_ch);
    storm::ParseNumber(row[12], m_data.dungeon_map);
    storm::ParseNumber(row[13], m_data.dungeon_cooldown);
#endif

    ComputeGuildPoints();

    DBManager::instance().FuncQuery(
        [this](SQLMsg* pmsg) { this->LoadGuildSkills(pmsg); },
        "SELECT skill_vnum, skill_level FROM guild_skill WHERE guild_id = %u",
        m_data.guild_id);

    DBManager::instance().FuncQuery(
        [this](SQLMsg* pmsg) { this->LoadGuildGradeData(pmsg); },
        "SELECT grade, name, auth+0 FROM guild_grade WHERE guild_id = %u",
        m_data.guild_id);
}

void CGuild::LoadGuildSkills(SQLMsg* pmsg)
{
    for (const auto& row : pmsg->Get()->rows) {
        uint16_t skillVnum = 0;
        storm::ParseNumber(row[0], skillVnum);
        uint8_t skillLevel = 0;
        storm::ParseNumber(row[1], skillLevel);
        uint32_t dwRealVnum = skillVnum - GUILD_SKILL_START;

        m_data.abySkill[dwRealVnum] = skillLevel;
    }
}

void CGuild::Load(uint32_t guild_id, int memberBonus /*= 0*/)
{
    Initialize();

    m_data.guild_id = guild_id;
    m_iMemberCountBonus = memberBonus;

    DBManager::instance().FuncQuery(
        [this](SQLMsg* pmsg) { this->LoadGuildData(pmsg); },
        "SELECT master, level, exp, name, skill_point, sp, "
        "ladder_point, win, draw, loss, gold, dungeon_ch, dungeon_map, "
        "dungeon_cooldown FROM guild WHERE id = %u",
        m_data.guild_id);

    SPDLOG_TRACE("GUILD: loading guild id {} {}", m_data.name, guild_id);
}

void CGuild::SaveLevel()
{
    DBManager::instance().Query("UPDATE guild SET level={}, exp={}, "
                                "skill_point={} WHERE id = {}",
                                m_data.level, m_data.exp, m_data.skill_point,
                                m_data.guild_id);
}

void CGuild::SendDBSkillUpdate(int amount)
{
    TPacketGuildSkillUpdate guild_skill;
    guild_skill.guild_id = m_data.guild_id;
    guild_skill.amount = amount;
    guild_skill.skill_point = m_data.skill_point;
    std::memcpy(guild_skill.skill_levels, m_data.abySkill,
                sizeof(uint8_t) * GUILD_SKILL_COUNT);

    db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill,
                            sizeof(guild_skill));
}

void CGuild::SaveSkill()
{
    DBManager::instance().Query("UPDATE guild SET sp = {}, skill_point={} "
                                " WHERE id = {};",
                                m_data.power, m_data.skill_point,
                                m_data.guild_id);

    for (int i = 0; i < GUILD_SKILL_COUNT; ++i) {
        if(m_data.abySkill[i] == 0) {
          DBManager::instance().Query("DELETE FROM guild_skill WHERE guild_id = {} AND"
                                      " skill_vnum={}",
                                      m_data.guild_id, GUILD_SKILL_START + i);
        } else {
          DBManager::instance().Query("REPLACE INTO guild_skill SET guild_id = {},"
                                      " skill_vnum={}, skill_level={};",
                                      m_data.guild_id, GUILD_SKILL_START + i,
                                      static_cast<int>(m_data.abySkill[i]));
        }

    }
}

const TGuildMember* CGuild::GetMember(uint32_t pid) const
{
    auto it = m_member.find(pid);
    if (it == m_member.end())
        return nullptr;

    return &it->second;
}

uint32_t CGuild::GetMemberPID(const std::string& strName)
{
    for (auto iter = m_member.begin(); iter != m_member.end(); iter++) {
        if (iter->second.name == strName)
            return iter->first;
    }

    return 0;
}

void CGuild::__P2PUpdateGrade(SQLMsg* pmsg)
{
    if (pmsg->Get()->uiNumRows) {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

        int grade = 0;
        const char* name = row[1];
        uint64_t auth = 0;

        str_to_number(grade, row[0]);
        str_to_number(auth, row[2]);

        if (grade <= 0)
            return;

        grade--;

        // 등급 명칭이 현재와 다르다면 업데이트
        if (m_data.grade_array[grade].grade_name != name) {
            m_data.grade_array[grade].grade_name = name;

            TPacketGCGuild pack;
            pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

            TOneGradeNamePacket pack2;
            pack2.grade = grade + 1;
            pack2.grade_name = name;
            pack.oneGrade = pack2;

            for (TGuildMemberOnlineContainer::iterator it =
                     m_memberOnline.begin();
                 it != m_memberOnline.end(); ++it) {
                DESC* d = (*it)->GetDesc();

                if (d)
                    d->Send(HEADER_GC_GUILD, pack);
            }
        }

        if (m_data.grade_array[grade].auth_flag != auth) {
            m_data.grade_array[grade].auth_flag = auth;

            TPacketGCGuild pack;
            pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

            TOneGradeAuthPacket pack2;
            pack2.grade = grade + 1;
            pack2.auth = auth;
            pack.oneGradeAuth = pack2;

            for (TGuildMemberOnlineContainer::iterator it =
                     m_memberOnline.begin();
                 it != m_memberOnline.end(); ++it) {
                DESC* d = (*it)->GetDesc();
                if (d) {
                    d->Send(HEADER_GC_GUILD, pack);
                }
            }
        }
    }
}

void CGuild::P2PChangeGrade(uint8_t grade)
{
    DBManager::instance().FuncQuery(
        [this](SQLMsg* pmsg) { this->__P2PUpdateGrade(pmsg); },
        fmt::format("SELECT grade, name, auth+0 FROM guild_grade WHERE "
                    "guild_id = {} and grade = {}",
                    m_data.guild_id, grade)
            .c_str());
}

namespace
{
struct FSendChangeGrade {
    uint8_t grade;
    TPacketGuild p;

    FSendChangeGrade(uint32_t guild_id, uint8_t grade)
        : grade(grade)
    {
        p.dwGuild = guild_id;
        p.dwInfo = grade;
    }

    void operator()()
    {
        db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_GRADE, 0, &p, sizeof(p));
    }
};
} // namespace

void CGuild::ChangeGradeName(uint8_t grade, const char* grade_name)
{
    if (grade == 1)
        return;

    if (grade < 1 || grade > 15) {
        SPDLOG_ERROR("Wrong guild grade value {0}", grade);
        return;
    }

    if (strlen(grade_name) > GUILD_NAME_MAX_LEN)
        return;

    if (!*grade_name)
        return;

    char text[GUILD_NAME_MAX_LEN * 2 + 1];

    DBManager::instance().EscapeString(text, sizeof(text), grade_name,
                                       strlen(grade_name));
    DBManager::instance().FuncAfterQuery(FSendChangeGrade(GetID(), grade),
                                         "UPDATE guild_grade SET name = '%s' "
                                         "where guild_id = %u and grade = %d",
                                         text, m_data.guild_id, grade);

    grade--;
    m_data.grade_array[grade].grade_name = grade_name;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

    TOneGradeNamePacket pack2;
    pack2.grade = grade + 1;
    pack2.grade_name = grade_name;
    pack.oneGrade = pack2;

    TEMP_BUFFER buf;
    buf.write(&pack, sizeof(pack));
    buf.write(&pack2, sizeof(pack2));

    for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin();
         it != m_memberOnline.end(); ++it) {
        DESC* d = (*it)->GetDesc();

        if (d)
            d->Send(HEADER_GC_GUILD, pack);
    }
}

void CGuild::ChangeGradeAuth(uint8_t grade, uint64_t auth)
{
    if (grade == 1)
        return;

    if (grade < 1 || grade > 15) {
        SPDLOG_ERROR("Wrong guild grade value {0}", grade);
        return;
    }

    DBManager::instance().FuncAfterQuery(FSendChangeGrade(GetID(), grade),
                                         "UPDATE guild_grade SET auth = %llu "
                                         "where guild_id = %u and grade = %d",
                                         auth, m_data.guild_id, grade);

    grade--;

    m_data.grade_array[grade].auth_flag = auth;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

    TOneGradeAuthPacket pack2;
    pack2.grade = grade + 1;
    pack2.auth = auth;
    pack.oneGradeAuth = pack2;

    for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin();
         it != m_memberOnline.end(); ++it) {
        DESC* d = (*it)->GetDesc();

        if (d)
            d->Send(HEADER_GC_GUILD, pack);
    }
}

void CGuild::SendGuildInfoPacket(CHARACTER* ch)
{
    DESC* d = ch->GetDesc();

    if (!d)
        return;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_INFO;

    TPacketGCGuildInfo pack_sub;
    pack_sub.member_count = GetMemberCount();
    pack_sub.max_member_count = GetMaxMemberCount();
    pack_sub.guild_id = m_data.guild_id;
    pack_sub.master_pid = m_data.master_pid;
    pack_sub.exp = m_data.exp;
    pack_sub.level = m_data.level;
    pack_sub.name = m_data.name;
    pack_sub.gold = m_data.gold;
    pack_sub.hasLand = HasLand();

    pack.info = pack_sub;
    SPDLOG_INFO("GMC guild_name {0}", m_data.name);
    SPDLOG_INFO("GMC master {0}", m_data.master_pid);

    d->Send(HEADER_GC_GUILD, pack);
}

bool CGuild::OfferExp(CHARACTER* ch, PointValue amount)
{
    TGuildMemberContainer::iterator cit = m_member.find(ch->GetPlayerID());

    if (cit == m_member.end())
        return false;

    if (m_data.exp + amount < m_data.exp)
        return false;

    if (ch->GetExp() < amount) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 제공하고자 하는 경험치가 남은 경험치보다 "
                           "많습니다.");
        return false;
    }

    if (ch->GetExp() - amount > ch->GetExp()) {
        SPDLOG_ERROR("Wrong guild offer amount {0} by {1}[{2}]", amount,
                     ch->GetName(), ch->GetPlayerID());
        return false;
    }

    ch->PointChange(POINT_EXP, -amount);

    TPacketGuildExpUpdate guild_exp;
    guild_exp.guild_id = GetID();
    guild_exp.amount = amount / 200;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_EXP_UPDATE, 0, &guild_exp,
                            sizeof(guild_exp));
    GuildPointChange(POINT_EXP, amount / 200, true);

    cit->second.offer_exp += amount / 200;

    SendListOneToAll(cit->second.pid);

    SaveMember(ch->GetPlayerID());

    TPacketGuildChangeMemberData gd_guild;

    gd_guild.guild_id = GetID();
    gd_guild.pid = ch->GetPlayerID();
    gd_guild.offer = cit->second.offer_exp;
    gd_guild.level = ch->GetLevel();
    gd_guild.grade = cit->second.grade;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild,
                            sizeof(gd_guild));
    return true;
}

void CGuild::Disband()
{
    SPDLOG_INFO("GUILD: Disband {0}:{1}", GetName(), GetID());

    // building::CLand* pLand =
    // building::CManager::instance().FindLandByGuild(GetID()); if (pLand)
    // pLand->SetOwner(0);

    for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin();
         it != m_memberOnline.end(); ++it) {
        CHARACTER* ch = *it;
        ch->SetGuild(nullptr);
        SendOnlineRemoveOnePacket(ch->GetPlayerID());
        ch->SetQuestFlag("guild_manage.new_withdraw_time", get_global_time());
    }

    for (TGuildMemberContainer::iterator it = m_member.begin();
         it != m_member.end(); ++it) {
        CGuildManager::instance().Unlink(it->first);
    }
}

void CGuild::RequestDisband(uint32_t pid)
{
    if (m_data.master_pid != pid)
        return;

    TPacketGuild gd_guild;
    gd_guild.dwGuild = GetID();
    gd_guild.dwInfo = 0;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_DISBAND, 0, &gd_guild,
                            sizeof(TPacketGuild));

    // LAND_CLEAR
    building::CManager::instance().ClearLandByGuildID(GetID());
    // END_LAND_CLEAR
}

bool CGuild::HasGradeAuth(int grade, uint64_t auth_flag) const
{
    return IS_SET(m_data.grade_array[grade - 1].auth_flag, auth_flag);
}

void CGuild::AddComment(CHARACTER* ch, const std::string& str)
{
    if (str.length() > GUILD_COMMENT_MAX_LEN ||
        str.length() == 0) // Added string null verification
        return;

    // Compare last pulse with current pulse and notify the player
    if (m_guildPostCommentPulse != 0 &&
        m_guildPostCommentPulse > thecore_pulse()) {
        int deltaInSeconds =
            ((m_guildPostCommentPulse / THECORE_SECS_TO_PASSES(1)) -
             (thecore_pulse() / THECORE_SECS_TO_PASSES(1)));
        int minutes = deltaInSeconds / 60;
        int seconds = (deltaInSeconds - (minutes * 60));

        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You can post a new comment in: %02d minutes and "
                           "%02d seconds!",
                           minutes, seconds);
        return;
    }

    char text[GUILD_COMMENT_MAX_LEN * 2 + 1];
    DBManager::instance().EscapeString(text, sizeof(text), str.c_str(),
                                       str.length());
    const uint32_t playerId = ch->GetPlayerID();

    auto handler = [this, &playerId]() { RefreshCommentForce(playerId); };
    DBManager::instance().FuncAfterQuery(handler,
                                         "INSERT INTO guild_comment(guild_id, "
                                         "name, notice, content, time) "
                                         "VALUES(%u, '%s', %d, '%s', NOW())",
                                         m_data.guild_id, ch->GetName().c_str(),
                                         (str[0] == '!') ? 1 : 0, text);

    // Set comment pulse to 10 minutes
    m_guildPostCommentPulse = thecore_pulse() + THECORE_SECS_TO_PASSES(10 * 60);
}

void CGuild::DeleteComment(CHARACTER* ch, uint32_t comment_id)
{
    SQLMsg* pmsg;

    if (GetMember(ch->GetPlayerID())->grade == GUILD_LEADER_GRADE)
        pmsg = DBManager::instance().DirectQuery("DELETE FROM guild_comment "
                                                 "WHERE id = {} AND guild_id = "
                                                 "{}",
                                                 comment_id, m_data.guild_id);
    else
        pmsg = DBManager::instance().DirectQuery("DELETE FROM guild_comment "
                                                 "WHERE id = {} AND guild_id = "
                                                 "{} AND name = '{}'",
                                                 comment_id, m_data.guild_id,
                                                 ch->GetName());

    if (pmsg->Get()->uiAffectedRows == 0 ||
        pmsg->Get()->uiAffectedRows == (uint32_t)-1)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 삭제할 수 없는 글입니다.");
    else
        RefreshCommentForce(ch->GetPlayerID());

    delete (pmsg);
}

void CGuild::RefreshComment(CHARACTER* ch)
{
    RefreshCommentForce(ch->GetPlayerID());
}

void CGuild::RefreshCommentForce(uint32_t player_id)
{
    CHARACTER* ch = g_pCharManager->FindByPID(player_id);
    if (ch == nullptr) {
        return;
    }

    std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(
        "SELECT id, name, content FROM guild_comment WHERE guild_id = {} ORDER "
        "BY notice DESC, id DESC LIMIT {}",
        m_data.guild_id, GUILD_COMMENT_MAX_COUNT));

    uint8_t count = pmsg->Get()->uiNumRows;

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_COMMENTS;

    std::vector<GuildComment> comments;
    comments.reserve(count);

    DESC* d = ch->GetDesc();

    if (!d)
        return;

    for (uint32_t i = 0; i < pmsg->Get()->uiNumRows; i++) {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
        uint32_t id = strtoul(row[0], nullptr, 10);

        comments.emplace_back(GuildComment{id, row[1], row[2]});
    }
    pack.comments = comments;
    d->Send(HEADER_GC_GUILD, pack);
}

bool CGuild::ChangeMemberGeneral(uint32_t pid, uint8_t is_general)
{
    if (is_general && GetGeneralCount() >= GetMaxGeneralCount())
        return false;

    TGuildMemberContainer::iterator it = m_member.find(pid);
    if (it == m_member.end()) {
        return true;
    }

    is_general = is_general ? 1 : 0;

    if (it->second.is_general == is_general)
        return true;

    if (is_general)
        ++m_general_count;
    else
        --m_general_count;

    it->second.is_general = is_general;

    TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL;

    GuildGeneralUpdate p2;
    p2.pid = pid;
    p2.isGeneral = is_general;
    pack.updateGeneral = p2;

    while (itOnline != m_memberOnline.end()) {
        DESC* d = (*(itOnline++))->GetDesc();

        if (!d)
            continue;

        d->Send(HEADER_GC_GUILD, pack);
    }

    SaveMember(pid);
    return true;
}

void CGuild::ChangeMemberGrade(uint32_t pid, uint8_t grade)
{
    if (grade == 1)
        return;

    TGuildMemberContainer::iterator it = m_member.find(pid);

    if (it == m_member.end())
        return;

    it->second.grade = grade;

    TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

    TPacketGCGuild pack;
    pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE;
    GuildUpdateMemberGrade p2;
    p2.pid = pid;
    p2.grade = grade;
    pack.updateMemberGrade = p2;

    while (itOnline != m_memberOnline.end()) {
        DESC* d = (*(itOnline++))->GetDesc();

        if (!d)
            continue;

        d->Send(HEADER_GC_GUILD, pack);
    }

    SaveMember(pid);

    TPacketGuildChangeMemberData gd_guild;

    gd_guild.guild_id = GetID();
    gd_guild.pid = pid;
    gd_guild.offer = it->second.offer_exp;
    gd_guild.level = it->second.level;
    gd_guild.grade = grade;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild,
                            sizeof(gd_guild));
}

void CGuild::SkillLevelUp(uint32_t dwVnum)
{
    uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

    if (dwRealVnum >= GUILD_SKILL_COUNT)
        return;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk) {
        SPDLOG_ERROR("There is no such guild skill by number {0}", dwVnum);
        return;
    }

    if (m_data.abySkill[dwRealVnum] >= pkSk->bMaxLevel)
        return;

    if (m_data.skill_point <= 0)
        return;

    m_data.skill_point--;
    m_data.abySkill[dwRealVnum] += 1;

    ComputeGuildPoints();
    SaveSkill();
    SendDBSkillUpdate();

    /*switch (dwVnum)
      {
      case GUILD_SKILL_GAHO:
      {
      TGuildMemberOnlineContainer::iterator it;

      for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
      (*it)->PointChange(POINT_DEF_GRADE, 1);
      }
      break;
      case GUILD_SKILL_HIM:
      {
      TGuildMemberOnlineContainer::iterator it;

      for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
      (*it)->PointChange(POINT_ATT_GRADE, 1);
      }
      break;
      }*/

    for_each(
        m_memberOnline.begin(), m_memberOnline.end(),
        std::bind(&CGuild::SendSkillInfoPacket, *this, std::placeholders::_1));

    SPDLOG_INFO("Guild SkillUp: {0} {1} level {2} type {3}", GetName(),
                pkSk->dwVnum, m_data.abySkill[dwRealVnum], pkSk->dwType);
}

void CGuild::UseSkill(uint32_t dwVnum, CHARACTER* ch, uint32_t pid)
{
    CHARACTER* victim;

    if (!GetMember(ch->GetPlayerID()) ||
        !HasGradeAuth(GetMember(ch->GetPlayerID())->grade,
                      GUILD_AUTH_USE_SKILL))
        return;

    SPDLOG_TRACE("GUILD_USE_SKILL : cname(%s), skill(%d)",
                 ch ? ch->GetName() : "", dwVnum);

    uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

    if (!ch->CanMove())
        return;

    if (dwRealVnum >= GUILD_SKILL_COUNT)
        return;

    if (!CWarMapManager::instance().IsWarMap(ch->GetMapIndex())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<Guild> Skills can only be used on guild war "
                           "maps.");
        return;
    }

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk) {
        SPDLOG_ERROR("There is no such guild skill by number {0}", dwVnum);
        return;
    }

    if (m_data.abySkill[dwRealVnum] == 0)
        return;

    if ((pkSk->dwFlag & SKILL_FLAG_SELFONLY)) {
        // 이미 걸려 있으므로 사용하지 않음.
        if (ch->FindAffect(pkSk->dwVnum))
            return;

        victim = ch;
    }

    if (ch->FindAffect(AFFECT_REVIVE_INVISIBLE))
        ch->RemoveAffect(AFFECT_REVIVE_INVISIBLE);

    if (ch->FindAffect(SKILL_EUNHYUNG))
        ch->RemoveAffect(SKILL_EUNHYUNG);

    double k = 1.0 * m_data.abySkill[dwRealVnum] / 1.0;
    pkSk->kSPCostPoly.SetVar("k", k);
    int iNeededSP = (int)pkSk->kSPCostPoly.Eval();

    if (GetSP() < iNeededSP) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 용신력이 부족합니다. (%d, %d)", GetSP(),
                           iNeededSP);
        return;
    }

    pkSk->kCooldownPoly.SetVar("k", k);
    int iCooltime = (int)pkSk->kCooldownPoly.Eval();

    if (!abSkillUsable[dwRealVnum]) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 쿨타임이 끝나지 않아 길드 스킬을 사용할 수 "
                           "없습니다.");
        return;
    }

    {
        TPacketGuildUseSkill p;
        p.dwGuild = GetID();
        p.dwSkillVnum = pkSk->dwVnum;
        p.dwCooltime = iCooltime;
        db_clientdesc->DBPacket(HEADER_GD_GUILD_USE_SKILL, 0, &p, sizeof(p));
    }
    abSkillUsable[dwRealVnum] = false;
    // abSkillUsed[dwRealVnum] = true;
    // adwSkillNextUseTime[dwRealVnum] = get_dword_time() + iCooltime * 1000;

    // PointChange(POINT_SP, -iNeededSP);
    // GuildPointChange(POINT_SP, -iNeededSP);

    if (gConfig.testServer)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> %d 스킬을 사용함 (%d, %d) to %u", dwVnum,
                           GetSP(), iNeededSP, pid);

    switch (dwVnum) {
        case GUILD_SKILL_TELEPORT: {
            // 현재 서버에 있는 사람을 먼저 시도.
            SendDBSkillUpdate(-iNeededSP);
            if ((victim = (g_pCharManager->FindByPID(pid)))) {
                ch->WarpSet(victim->GetMapIndex(), victim->GetX(),
                            victim->GetY());
                break;
            }

            if (m_memberP2POnline.find(pid) == m_memberP2POnline.end()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 상대가 온라인 상태가 아닙니다.");
                break;
            }

            // 다른 서버에 로그인된 사람이 있음 -> 메시지 보내 좌표를 받아오자
            // 1. A.pid, B.pid 를 뿌림
            // 2. B.pid를 가진 서버가 뿌린서버에게 A.pid, 좌표 를 보냄
            // 3. 워프
            const auto op =
                DESC_MANAGER::instance().GetOnlinePlayers().Get(pid);
            if (!op)
                break;

            if (op->channel != gConfig.channel) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 상대가 %d 채널에 있습니다. (현재 "
                                   "채널 %d)",
                                   op->channel, gConfig.channel);
                break;
            }

            GmCharacterTransferPacket p;
            p.sourcePid = ch->GetPlayerID();
            p.targetPid = pid;
            DESC_MANAGER::instance().GetMasterSocket()->Send(
                kGmCharacterTransfer, p);

            if (gConfig.testServer)
                ch->ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for "
                                                "guild teleport");

            break;
        }

            /*case GUILD_SKILL_ACCEL:
              ch->RemoveAffect(dwVnum);
              ch->AddAffect(dwVnum, POINT_MOV_SPEED,
              m_data.abySkill[dwRealVnum]*3, pkSk->dwAffectFlag,
              (int)pkSk->kDurationPoly.Eval(), 0, false); ch->AddAffect(dwVnum,
              POINT_ATT_SPEED, m_data.abySkill[dwRealVnum]*3,
              pkSk->dwAffectFlag, (int)pkSk->kDurationPoly.Eval(), 0, false);
              break;*/

        default: {
            /*if (ch->GetPlayerID() != GetMasterPID())
              {
              SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<길드> 길드장만 길드
              스킬을 사용할 수 있습니다."); return;
              }*/

            if (!UnderAnyWar()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드 스킬은 길드전 중에만 사용할 수 "
                                   "있습니다.");
                return;
            }

            SendDBSkillUpdate(-iNeededSP);
            CSkillProto *pkSk = CSkillManager::instance().Get(dwVnum);
            if(!pkSk)
                return;

            pkSk->SetPointVar("k", 1.0f * (100.0f * GetSkillLevel(dwVnum) / 7.0f) * 1.0f / 100.0f);

            for (auto member : m_memberOnline) {
                member->RemoveAffect(dwVnum);
                pkSk->SetPointVar("maxhp", member->GetMaxHP());
                member->AddAffect(dwVnum, pkSk->bPointOn, pkSk->kPointPoly.Eval(), pkSk->kDurationPoly.Eval(), 0, true, false);
            }
        } break;
            /*if (!victim)
              return;

              ch->ComputeSkill(dwVnum, victim, m_data.abySkill[dwRealVnum]);*/
    }
}

void CGuild::SendSkillInfoPacket(CHARACTER* ch) const
{
    DESC* d = ch->GetDesc();
    if (!d)
        return;

    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_SKILL_INFO;

    GcGuildSkillInfo info;

    info.skillPoints = m_data.skill_point;
    std::memcpy(info.skillLevels, m_data.abySkill, GUILD_SKILL_COUNT);
    info.power = m_data.power;
    info.maxPower = m_data.max_power;
    p.skillInfo = info;

    d->Send(HEADER_GC_GUILD, p);
}

void CGuild::ComputeGuildPoints()
{
    m_data.max_power =
        GUILD_BASE_POWER + (m_data.level - 1) * GUILD_POWER_PER_LEVEL;

    m_data.power = std::clamp(m_data.power, 0, m_data.max_power);
}

int CGuild::GetSkillLevel(uint32_t vnum)
{
    uint32_t dwRealVnum = vnum - GUILD_SKILL_START;

    if (dwRealVnum >= GUILD_SKILL_COUNT)
        return 0;

    return m_data.abySkill[dwRealVnum];
}

/*void CGuild::GuildUpdateAffect(CHARACTER* ch)
  {
  if (GetSkillLevel(GUILD_SKILL_GAHO))
  ch->PointChange(POINT_DEF_GRADE, GetSkillLevel(GUILD_SKILL_GAHO));

  if (GetSkillLevel(GUILD_SKILL_HIM))
  ch->PointChange(POINT_ATT_GRADE, GetSkillLevel(GUILD_SKILL_HIM));
  }*/

/*void CGuild::GuildRemoveAffect(CHARACTER* ch)
  {
  if (GetSkillLevel(GUILD_SKILL_GAHO))
  ch->PointChange(POINT_DEF_GRADE, -(int) GetSkillLevel(GUILD_SKILL_GAHO));

  if (GetSkillLevel(GUILD_SKILL_HIM))
  ch->PointChange(POINT_ATT_GRADE, -(int) GetSkillLevel(GUILD_SKILL_HIM));
  }*/

void CGuild::UpdateSkill(uint8_t skill_point, uint8_t* skill_levels)
{
    // int iDefMoreBonus = 0;
    // int iAttMoreBonus = 0;

    m_data.skill_point = skill_point;
    /*if (skill_levels[GUILD_SKILL_GAHO -
      GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_GAHO))
      {
      iDefMoreBonus = skill_levels[GUILD_SKILL_GAHO -
      GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_GAHO);
      }
      if (skill_levels[GUILD_SKILL_HIM -
      GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_HIM))
      {
      iAttMoreBonus = skill_levels[GUILD_SKILL_HIM  -
      GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_HIM);
      }

      if (iDefMoreBonus || iAttMoreBonus)
      {
      for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it
      != m_memberOnline.end(); ++it)
      {
      (*it)->PointChange(POINT_ATT_GRADE, iAttMoreBonus);
      (*it)->PointChange(POINT_DEF_GRADE, iDefMoreBonus);
      }
      }*/

    std::memcpy(m_data.abySkill, skill_levels,
                sizeof(uint8_t) * GUILD_SKILL_COUNT);
    ComputeGuildPoints();
}

void CGuild::GuildPointChange(uint8_t type, PointValue amount, bool save)
{
    switch (type) {
        case POINT_SP:
            m_data.power += amount;

            m_data.power = std::clamp(m_data.power, 0, m_data.max_power);

            if (save) {
                SaveSkill();
            }

            for (auto& member : m_memberOnline) {
                SendSkillInfoPacket(member);
            }
            break;

        case POINT_EXP:
            if (m_data.level == GUILD_MAX_LEVEL)
                break;

            if (amount < 0) {
                m_data.exp -= std::min<uint32_t>(m_data.exp, -amount);
            } else {
                m_data.exp += amount;

                while (m_data.exp >= GetGuildLevelExp(m_data.level)) {
                    m_data.exp -= GetGuildLevelExp(m_data.level);

                    ++m_data.level;
                    ++m_data.skill_point;

                    ComputeGuildPoints();
                    GuildPointChange(POINT_SP, m_data.max_power - m_data.power);

                    if (save)
                        ChangeLadderPoint(GUILD_LADDER_POINT_PER_LEVEL);

                    // NOTIFY_GUILD_EXP_CHANGE
                    for (auto& member : m_memberOnline) {
                        SendSkillInfoPacket(member);
                    }
                    // END_OF_NOTIFY_GUILD_EXP_CHANGE

                    if (m_data.level == GUILD_MAX_LEVEL) {
                        m_data.exp = 0;
                        break;
                    }
                }
            }

            TPacketGCGuild pack;
            pack.subheader = GUILD_SUBHEADER_GC_CHANGE_EXP;

            GuildStatusUpdate s;
            s.lv = m_data.level;
            s.exp = m_data.exp;
            pack.status = s;

            for (auto it = m_memberOnline.begin(); it != m_memberOnline.end();
                 ++it) {
                DESC* d = (*it)->GetDesc();

                if (d)
                    d->Send(HEADER_GC_GUILD, pack);
            }

            if (save)
                SaveLevel();

            break;
    }
}

void CGuild::SkillRecharge()
{
    // GuildPointChange(POINT_SP, m_data.max_power / 2);
    // GuildPointChange(POINT_SP, 10);
}

void CGuild::SaveMember(uint32_t pid)
{
    TGuildMemberContainer::iterator it = m_member.find(pid);

    if (it == m_member.end())
        return;

    DBManager::instance().Query("UPDATE guild_member SET grade = {}, offer = "
                                "{}, is_general = {} WHERE pid = {} and "
                                "guild_id = {}",
                                it->second.grade, it->second.offer_exp,
                                it->second.is_general, pid, m_data.guild_id);
}

void CGuild::LevelChange(uint32_t pid, uint8_t level)
{
    TGuildMemberContainer::iterator cit = m_member.find(pid);

    if (cit == m_member.end())
        return;

    cit->second.level = level;

    TPacketGuildChangeMemberData gd_guild;

    gd_guild.guild_id = GetID();
    gd_guild.pid = pid;
    gd_guild.offer = cit->second.offer_exp;
    gd_guild.grade = cit->second.grade;
    gd_guild.level = level;

    db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild,
                            sizeof(gd_guild));

    SendListOneToAll(pid);
}

void CGuild::ChangeMemberData(uint32_t pid, uint32_t offer, uint8_t level,
                              uint8_t grade)
{
    auto cit = m_member.find(pid);

    if (cit == m_member.end())
        return;

    cit->second.offer_exp = offer;
    cit->second.level = level;
    cit->second.grade = grade;
    SendListOneToAll(pid);
}

namespace
{
struct FGuildChat {
    const char* c_pszText;

    FGuildChat(const char* c_pszText)
        : c_pszText(c_pszText)
    {
    }

    explicit FGuildChat(const char* c_pszText, const char* issuer)
        : c_pszText(c_pszText)
    {
    }

    void operator()(CHARACTER* ch)
    {
        ch->ChatPacket(CHAT_TYPE_GUILD, "%s", c_pszText);
    }
};
} // namespace

void CGuild::MasterChat(const char* c_pszText, const char* issuer)
{
    std::for_each(m_memberOnline.begin(), m_memberOnline.end(),
                  [c_pszText](CHARACTER* ch) {
                      SendChatPacket(ch, CHAT_TYPE_GUILD, c_pszText);
                  });
}

void CGuild::MasterChatLocalized(const std::string& c_pszText)
{
    std::for_each(m_memberOnline.begin(), m_memberOnline.end(),
                  [c_pszText](CHARACTER* ch) {
                      SendSpecialI18nChatPacket(ch, CHAT_TYPE_GUILD, c_pszText);
                  });
}

void CGuild::Chat(const char* c_pszText, const char* issuer)
{
    std::for_each(m_memberOnline.begin(), m_memberOnline.end(),
                  FGuildChat(c_pszText, issuer));

    GmGuildChatPacket p;
    p.gid = GetID();
    p.message = c_pszText;
    p.type = GmGuildChatPacket::kGuildChat;

    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmGuildChat, p);
}

void CGuild::Chat(const char* c_pszText)
{
    std::for_each(m_memberOnline.begin(), m_memberOnline.end(),
                  FGuildChat(c_pszText));

    GmGuildChatPacket p;
    p.gid = GetID();
    p.message = c_pszText;
    p.type = GmGuildChatPacket::kGuildChat;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmGuildChat, p);
}

void CGuild::LocalizedChat(const std::string& text)
{
    std::for_each(m_memberOnline.begin(), m_memberOnline.end(),
                  [text](CHARACTER* ch) {
                      SendSpecialI18nChatPacket(ch, CHAT_TYPE_GUILD, text);
                  });

    GmGuildChatPacket p;
    p.gid = GetID();
    p.message = text;
    p.type = GmGuildChatPacket::kGuildChatLocalized;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmGuildChat, p);
}

CHARACTER* CGuild::GetMasterCharacter()
{
    return g_pCharManager->FindByPID(GetMasterPID());
}

int CGuild::GetTotalLevel() const
{
    int total = 0;

    for (auto it = m_member.begin(); it != m_member.end(); ++it) {
        total += it->second.level;
    }

    return total;
}

bool CGuild::ChargeSP(CHARACTER* ch, int iSP)
{
    Gold gold = iSP * 100;

    if (gold < iSP || ch->GetGold() < gold)
        return false;

    int iRemainSP = m_data.max_power - m_data.power;

    if (iSP > iRemainSP) {
        iSP = iRemainSP;
        gold = iSP * 100;
    }

    ch->ChangeGold(-gold);
    DBManager::instance().SendMoneyLog(MONEY_LOG_GUILD, 1, -gold);

    SendDBSkillUpdate(iSP);
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> %u의 용신력을 회복하였습니다.", iSP);
    }
    return true;
}

void CGuild::SkillUsableChange(uint32_t dwSkillVnum, bool bUsable)
{
    uint32_t dwRealVnum = dwSkillVnum - GUILD_SKILL_START;

    if (dwRealVnum >= GUILD_SKILL_COUNT)
        return;

    abSkillUsable[dwRealVnum] = bUsable;

    // GUILD_SKILL_COOLTIME_BUG_FIX
    SPDLOG_INFO("CGuild::SkillUsableChange(guild={0}, skill={1}, usable={2})",
                GetName(), dwSkillVnum, bUsable);
    // END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

// GUILD_MEMBER_COUNT_BONUS
void CGuild::SetMemberCountBonus(int iBonus)
{
    m_iMemberCountBonus = iBonus;
    SPDLOG_INFO("GUILD_IS_FULL_BUG : Bonus set to {0}(val:{1})", iBonus,
                m_iMemberCountBonus);
}

void CGuild::BroadcastMemberCountBonus()
{
    GmGuildMemberCountBonusPacket p;
    p.gid = GetID();
    p.bonus = m_iMemberCountBonus;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmGuildMemberCountBonus,
                                                     p);
}

int CGuild::GetMaxMemberCount()
{
    // GUILD_IS_FULL_BUG_FIX
    if (m_iMemberCountBonus < 0 || m_iMemberCountBonus > 18)
        m_iMemberCountBonus = 0;
    // END_GUILD_IS_FULL_BUG_FIX

    quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetMasterPID());
    if (pPC && pPC->GetFlag("guild.is_unlimit_member") == 1)
        return INT_MAX;

    return (32 + 2 * (m_data.level - 1) + m_iMemberCountBonus) * 2;
}

// END_OF_GUILD_MEMBER_COUNT_BONUS

void CGuild::AdvanceLevel(int iLevel)
{
    if (m_data.level == iLevel)
        return;

    m_data.level = std::min<int32_t>(GUILD_MAX_LEVEL, iLevel);
}

void CGuild::RequestDepositMoney(CHARACTER* ch, Gold iGold)
{
    if (!ch->CanDeposit() && ch->GetGuild() == this) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 잠시후에 이용해주십시오");
        return;
    }

    if (ch->GetGold() < iGold)
        return;

    ch->ChangeGold(-iGold);

    TPacketGDGuildMoney p;
    p.dwGuild = GetID();
    p.iGold = iGold;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_DEPOSIT_MONEY, 0, &p, sizeof(p));

    char buf[64 + 1];
    std::snprintf(buf, sizeof(buf), "%u %s", GetID(), GetName());
    LogManager::instance().CharLog(ch, iGold, "GUILD_DEPOSIT", buf);

    ch->UpdateDepositPulse();
    // SPDLOG_INFO( "GUILD: DEPOSIT %s:%u player %s[%u] gold %d", GetName(),
    // GetID(), ch->GetName(), ch->GetPlayerID(), iGold);
    DBManager::instance().DirectQuery("INSERT INTO "
                                      "log.guild_storage_log(guild_storage_log."
                                      "guild, guild_storage_log.member, "
                                      "guild_storage_log.action, "
                                      "guild_storage_log.time, "
                                      "guild_storage_log.itemvnum) VALUES "
                                      "('{}', '{}', '{}', NOW(), '{}');",
                                      GetID(), ch->GetPlayerID(), "DEPOSIT",
                                      iGold);
}

void CGuild::RequestWithdrawMoney(CHARACTER* ch, Gold iGold)
{
    if (!ch->CanDeposit()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 잠시후에 이용해주십시오");
        return;
    }

    if (ch->GetPlayerID() != GetMasterPID()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드 금고에선 길드장만 출금할 수 있습니다.");
        return;
    }

    if (m_data.gold < iGold) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 가지고 있는 돈이 부족합니다.");
        return;
    }

    TPacketGDGuildMoney p;
    p.dwGuild = GetID();
    p.iGold = iGold;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY, 0, &p, sizeof(p));

    ch->UpdateDepositPulse();
    DBManager::instance().DirectQuery("INSERT INTO "
                                      "log.guild_storage_log(guild_storage_log."
                                      "guild, guild_storage_log.member, "
                                      "guild_storage_log.action, "
                                      "guild_storage_log.time, "
                                      "guild_storage_log.itemvnum) VALUES "
                                      "('{}', '{}', '{}', NOW(), '{}');",
                                      GetID(), ch->GetPlayerID(), "WITHDRAW",
                                      iGold);
}

void CGuild::RecvMoneyChange(Gold iGold)
{
    m_data.gold = iGold;

    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_MONEY_CHANGE;
    p.pid = iGold;

    for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
        CHARACTER* ch = *it;
        DESC* d = ch->GetDesc();
        d->Send(HEADER_GC_GUILD, p);
    }
}

void CGuild::RecvWithdrawMoneyGive(Gold iChangeGold)
{
    CHARACTER* ch = GetMasterCharacter();

    if (ch) {
        ch->ChangeGold(iChangeGold);
        SPDLOG_INFO("GUILD: WITHDRAW {0}:{1} player {2}[{3}] gold {4}",
                    GetName(), GetID(), ch->GetName(), ch->GetPlayerID(),
                    iChangeGold);
    }

    TPacketGDGuildMoneyWithdrawGiveReply p;
    p.dwGuild = GetID();
    p.iChangeGold = iChangeGold;
    p.bGiveSuccess = ch ? 1 : 0;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY, 0, &p,
                            sizeof(p));
}

bool CGuild::HasLand()
{
    return building::CManager::instance().FindLandByGuild(GetID()) != nullptr;
}

// GUILD_JOIN_BUG_FIX
/// 길드 초대 event 정보
EVENTINFO(TInviteGuildEventInfo)
{
    uint32_t dwInviteePID; ///< 초대받은 character 의 PID
    uint32_t dwGuildID;    ///< 초대한 Guild 의 ID

    TInviteGuildEventInfo()
        : dwInviteePID(0)
        , dwGuildID(0)
    {
    }
};

/**
 * 길드 초대 event callback 함수.
 * event 가 발동하면 초대 거절로 처리한다.
 */
EVENTFUNC(GuildInviteEvent)
{
    auto pInfo = static_cast<TInviteGuildEventInfo*>(event->info);
    if (pInfo == nullptr) {
        SPDLOG_ERROR("GuildInviteEvent> <Factor> Null pointer");
        return 0;
    }

    CGuild* pGuild = CGuildManager::instance().FindGuild(pInfo->dwGuildID);
    if (pGuild) {
        SPDLOG_INFO("GuildInviteEvent {0}", pGuild->GetName());
        pGuild->InviteDeny(pInfo->dwInviteePID);
    }

    return 0;
}

void CGuild::Invite(CHARACTER* pchInviter, CHARACTER* pchInvitee)
{
    if (quest::CQuestManager::instance()
            .GetPCForce(pchInviter->GetPlayerID())
            ->IsRunning() == true) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           "<길드> 상대방이 초대 신청을 받을 수 없는 "
                           "상태입니다.");
        return;
    }

    if (quest::CQuestManager::instance()
            .GetPCForce(pchInvitee->GetPlayerID())
            ->IsRunning() == true)
        return;

    if (pchInviter->IsGM() && !pchInvitee->IsGM() &&
        !GM::check_allow(pchInviter->GetGMLevel(),
                         GM_ALLOW_PLAYER_GUILD_INVITE)) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           "You are not allowed to invite players into a "
                           "guild");
        return;
    }

    if (!pchInviter->IsGM() && pchInvitee->IsGM() &&
        !GM::check_allow(pchInviter->GetGMLevel(), GM_ALLOW_GM_GUILD_INVITE)) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           "You are not allowed to invite game masters into a "
                           "guild");
        return;
    }

    if (pchInvitee->IsBlockMode(BLOCK_GUILD_INVITE, pchInviter)) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           ("<길드> 상대방이 길드 초대 거부 상태입니다."));
        return;
    } else if (!HasGradeAuth(GetMember(pchInviter->GetPlayerID())->grade,
                             GUILD_AUTH_ADD_MEMBER)) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           ("<길드> 길드원을 초대할 권한이 없습니다."));
        return;
    } else if (pchInvitee->GetEmpire() != pchInviter->GetEmpire()) {
        SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                           ("<길드> 다른 제국 사람을 길드에 초대할 수 "
                            "없습니다."));
        return;
    }

    GuildJoinErrCode errcode = VerifyGuildJoinableCondition(pchInvitee);
    switch (errcode) {
        case GERR_NONE:
            break;
        case GERR_WITHDRAWPENALTY:
            SendI18nChatPacket(
                pchInviter, CHAT_TYPE_INFO,
                "<길드> 탈퇴한 후 %d일이 지나지 않은 사람은 길드에 초대할 수 "
                "없습니다.",
                quest::CQuestManager::instance().GetEventFlag("guild_withdraw_"
                                                              "delay"));
            return;
        case GERR_COMMISSIONPENALTY:
            SendI18nChatPacket(
                pchInviter, CHAT_TYPE_INFO,
                "<길드> 길드를 해산한 지 %d일이 지나지 않은 사람은 길드에 "
                "초대할 수 없습니다.",
                quest::CQuestManager::instance().GetEventFlag("guild_disband_"
                                                              "delay"));
            return;
        case GERR_ALREADYJOIN:
            SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                               "<길드> 상대방이 이미 다른 길드에 "
                               "속해있습니다.");
            return;
        case GERR_GUILDISFULL:
            SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                               "<길드> 최대 길드원 수를 초과했습니다.");
            return;
        case GERR_GUILD_IS_IN_WAR:
            SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                               "<길드> 현재 길드가 전쟁 중 입니다.");
            return;
        case GERR_INVITE_LIMIT:
            SendI18nChatPacket(pchInviter, CHAT_TYPE_INFO,
                               "<길드> 현재 신규 가입 제한 상태 입니다.");
            return;

        default:
            SPDLOG_ERROR("ignore guild join error(%d)", errcode);
            return;
    }

    if (m_GuildInviteEventMap.end() !=
        m_GuildInviteEventMap.find(pchInvitee->GetPlayerID()))
        return;

    //
    // 이벤트 생성
    //
    TInviteGuildEventInfo* pInfo = AllocEventInfo<TInviteGuildEventInfo>();
    pInfo->dwInviteePID = pchInvitee->GetPlayerID();
    pInfo->dwGuildID = GetID();

    m_GuildInviteEventMap.emplace(
        pchInvitee->GetPlayerID(),
        event_create(GuildInviteEvent, pInfo, THECORE_SECS_TO_PASSES(10)));

    //
    // 초대 받는 character 에게 초대 패킷 전송
    //

    uint32_t gid = GetID();

    TPacketGCGuild p;
    p.subheader = GUILD_SUBHEADER_GC_GUILD_INVITE;

    GuildInvite p2;
    p2.gid = GetID();
    p2.name = GetName();
    p.guildInvite = p2;

    TEMP_BUFFER buf;
    buf.write(&p, sizeof(p));
    buf.write(&gid, sizeof(uint32_t));
    buf.write(GetName(), GUILD_NAME_MAX_LEN + 1);

    pchInvitee->GetDesc()->Send(HEADER_GC_GUILD, p);
}

void CGuild::InviteAccept(CHARACTER* pchInvitee)
{
    EventMap::iterator itFind =
        m_GuildInviteEventMap.find(pchInvitee->GetPlayerID());
    if (itFind == m_GuildInviteEventMap.end()) {
        SPDLOG_TRACE("GuildInviteAccept from not invited character(invite "
                     "guild: {}, invitee: {})",
                     GetName(), pchInvitee->GetName());
        return;
    }

    event_cancel(&itFind->second);
    m_GuildInviteEventMap.erase(itFind);

    GuildJoinErrCode errcode = VerifyGuildJoinableCondition(pchInvitee);
    switch (errcode) {
        case GERR_NONE:
            break;
        case GERR_WITHDRAWPENALTY:
            SendI18nChatPacket(
                pchInvitee, CHAT_TYPE_INFO,
                "<길드> 탈퇴한 후 %d일이 지나지 않은 사람은 길드에 초대할 수 "
                "없습니다.",
                quest::CQuestManager::instance().GetEventFlag("guild_withdraw_"
                                                              "delay"));
            return;
        case GERR_COMMISSIONPENALTY:
            SendI18nChatPacket(
                pchInvitee, CHAT_TYPE_INFO,
                "<길드> 길드를 해산한 지 %d일이 지나지 않은 사람은 길드에 "
                "초대할 수 없습니다.",
                quest::CQuestManager::instance().GetEventFlag("guild_disband_"
                                                              "delay"));
            return;
        case GERR_ALREADYJOIN:
            SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO,
                               "<길드> 상대방이 이미 다른 길드에 "
                               "속해있습니다.");
            return;
        case GERR_GUILDISFULL:
            SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO,
                               "<길드> 최대 길드원 수를 초과했습니다.");
            return;
        case GERR_GUILD_IS_IN_WAR:
            SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO,
                               ("<길드> 현재 길드가 전쟁 중 입니다."));
            return;
        case GERR_INVITE_LIMIT:
            SendI18nChatPacket(pchInvitee, CHAT_TYPE_INFO,
                               ("<길드> 현재 신규 가입 제한 상태 입니다."));
            return;

        default:
            SPDLOG_ERROR("ignore guild join error(%d)", errcode);
            return;
    }

    RequestAddMember(pchInvitee, 15);
}

void CGuild::InviteDeny(uint32_t dwPID)
{
    auto itFind = m_GuildInviteEventMap.find(dwPID);
    if (itFind == m_GuildInviteEventMap.end()) {
        return;
    }

    event_cancel(&itFind->second);
    m_GuildInviteEventMap.erase(itFind);
}

CGuild::GuildJoinErrCode
CGuild::VerifyGuildJoinableCondition(const CHARACTER* pchInvitee)
{
    if (get_global_time() - pchInvitee->GetQuestFlag("guild_manage.new_"
                                                     "withdraw_time") <
        CGuildManager::instance().GetWithdrawDelay())
        return GERR_WITHDRAWPENALTY;
    else if (get_global_time() - pchInvitee->GetQuestFlag("guild_manage.new_"
                                                          "disband_time") <
             CGuildManager::instance().GetDisbandDelay())
        return GERR_COMMISSIONPENALTY;
    else if (pchInvitee->GetGuild())
        return GERR_ALREADYJOIN;
    else if (GetMemberCount() >= GetMaxMemberCount()) {
        SPDLOG_DEBUG("GuildName = {0}, GetMemberCount() = {1}, "
                     "GetMaxMemberCount() = {2} (32 + MAX(level({3})-10, 0) * "
                     "2 + bonus({4})",
                     GetName(), GetMemberCount(), GetMaxMemberCount(),
                     m_data.level, m_iMemberCountBonus);
        return GERR_GUILDISFULL;
    } else if (UnderAnyWar() != 0) {
        return GERR_GUILD_IS_IN_WAR;
    }

    return GERR_NONE;
}

// END_OF_GUILD_JOIN_BUG_FIX

bool CGuild::ChangeMasterTo(uint32_t dwPID)
{
    if (GetMember(dwPID) == nullptr)
        return false;

    TPacketChangeGuildMaster p;
    p.dwGuildID = GetID();
    p.idFrom = GetMasterPID();
    p.idTo = dwPID;

    db_clientdesc->DBPacket(HEADER_GD_REQ_CHANGE_GUILD_MASTER, 0, &p,
                            sizeof(p));

    return true;
}

void CGuild::SendGuildDataUpdateToAllMember()
{
    for (const auto& ch : m_memberOnline) {
        SendGuildInfoPacket(ch);
        SendAllGradePacket(ch);
    }
}

#ifdef __DUNGEON_FOR_GUILD__
bool CGuild::RequestDungeon(uint8_t bChannel, long lMapIndex)
{
    TPacketGDGuildDungeon sPacket;
    sPacket.dwGuildID = GetID();
    sPacket.bChannel = bChannel;
    sPacket.lMapIndex = lMapIndex;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_DUNGEON, 0, &sPacket,
                            sizeof(sPacket));
    return true;
}

void CGuild::RecvDungeon(uint8_t bChannel, long lMapIndex)
{
    m_data.dungeon_ch = bChannel;
    m_data.dungeon_map = lMapIndex;
}

bool CGuild::SetDungeonCooldown(uint32_t dwTime)
{
    TPacketGDGuildDungeonCD sPacket;
    sPacket.dwGuildID = GetID();
    sPacket.dwTime = dwTime;
    db_clientdesc->DBPacket(HEADER_GD_GUILD_DUNGEON_CD, 0, &sPacket,
                            sizeof(sPacket));
    return true;
}

void CGuild::RecvDungeonCD(uint32_t dwTime)
{
    m_data.dungeon_cooldown = dwTime;
}
#endif
