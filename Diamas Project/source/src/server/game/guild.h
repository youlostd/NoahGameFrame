#ifndef INC_GUILD_H
#define INC_GUILD_H

#include "skill.h"
#include "event.h"
#include <mysql/AsyncSQL.h>

#include <game/Constants.hpp>
#include <game/Types.hpp>
#include <game/DbPackets.hpp>

#include "desc.h"

#include "char.h"

struct SGuildMaster
{
    uint32_t pid;
};


typedef struct SGuildMember
{
    uint32_t pid;  // player 테이블의 id; primary key
    uint8_t grade; // 길드상의 플레이어의 계급 1 to 15 (1이 짱)
    uint8_t is_general;
    uint8_t job;
    uint8_t level;
    uint32_t offer_exp; // 공헌한 경험치
    uint8_t _dummy;

    std::string name;
    TPacketGCGuildSubMember PacketData() const;
    SGuildMember(CHARACTER *ch, uint8_t grade, uint32_t offer_exp);
    SGuildMember(uint32_t pid, uint8_t grade, uint8_t is_general, uint8_t job, uint8_t level, uint32_t offer_exp,
                 const char *name);
} TGuildMember;


typedef struct SGuildData
{
    uint32_t guild_id;
    uint32_t master_pid;
    uint64_t exp;
    uint8_t level;
    std::string name;

    TGuildGrade grade_array[GUILD_GRADE_COUNT];

    uint8_t skill_point;
    uint8_t abySkill[GUILD_SKILL_COUNT];

    int power;
    int max_power;

    int ladder_point;

    int win;
    int draw;
    int loss;

    int gold;
#ifdef __DUNGEON_FOR_GUILD__
    uint8_t dungeon_ch;
    long dungeon_map;
    uint32_t dungeon_cooldown;
#endif
} TGuildData;

struct TGuildCreateParameter
{
    CHARACTER *master;
    char name[GUILD_NAME_MAX_LEN + 1];
};

typedef struct SGuildWar
{
    uint32_t war_start_time;
    uint32_t score;
    uint32_t state;
    uint8_t type;
    uint32_t map_index;

    SGuildWar(uint8_t type)
        : war_start_time(0),
          score(0),
          state(GUILD_WAR_RECV_DECLARE),
          type(type),
          map_index(0)
    {
    }

    bool IsWarBegin() const { return state == GUILD_WAR_ON_WAR; }
} TGuildWar;


struct FGuildNameSender
{
    
    FGuildNameSender(uint32_t id, const char *guild_name)
    {
        GuildIdAndName gn{id, guild_name};
        n = gn;
    }

    void operator()(CHARACTER *ch)
    {
        DESC *d = ch->GetDesc();

        if (d)
        {
            p.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
            p.name = n;
            d->Send(HEADER_GC_GUILD, p);
        }
    }
    GuildIdAndName n;
    TPacketGCGuild p;
};


class CGuild
{
public:
    CGuild(TGuildCreateParameter &cp);

    explicit CGuild(uint32_t guild_id) { Load(guild_id); }

    ~CGuild();

    uint32_t GetID() const { return m_data.guild_id; }

    const char *GetName() const { return m_data.name.c_str(); }

    int GetSP() const { return m_data.power; }

    int GetMaxSP() { return m_data.max_power; }

    uint32_t GetMasterPID() const { return m_data.master_pid; }

    CHARACTER *GetMasterCharacter();

    uint8_t GetLevel() const { return m_data.level; }

    void Reset() { m_data.power = m_data.max_power; }

    void RequestDisband(uint32_t pid);
    void Disband();

    void RequestAddMember(CHARACTER *ch, int grade = 15);
    void AddMember(TPacketDGGuildMember *p);

    bool RequestRemoveMember(uint32_t pid);
    bool RemoveMember(uint32_t pid);

    void LoginMember(CHARACTER *ch);
    void P2PLoginMember(uint32_t pid);

    void LogoutMember(CHARACTER *ch);
    void P2PLogoutMember(uint32_t pid);

    void ChangeMemberGrade(uint32_t pid, uint8_t grade);
    bool OfferExp(CHARACTER *ch, PointValue amount);
    void LevelChange(uint32_t pid, uint8_t level);
    void ChangeMemberData(uint32_t pid, uint32_t offer, uint8_t level, uint8_t grade);

    void ChangeGradeName(uint8_t grade, const char *grade_name);
    void ChangeGradeAuth(uint8_t grade, uint64_t auth);
    void P2PChangeGrade(uint8_t grade);

    bool ChangeMemberGeneral(uint32_t pid, uint8_t is_general);

    bool ChangeMasterTo(uint32_t dwPID);

    template<typename T>
    void Packet(PacketId id, const T& p);

    void SendOnlineRemoveOnePacket(uint32_t pid);
    void SendAllGradePacket(CHARACTER *ch);
    void SendListPacket(CHARACTER *ch);
    void SendListOneToAll(uint32_t pid);
    void SendListOneToAll(CHARACTER *ch);
    void SendLoginPacket(CHARACTER *ch, CHARACTER *chLogin);
    void SendLogoutPacket(CHARACTER *ch, CHARACTER *chLogout);
    void SendLoginPacket(CHARACTER *ch, uint32_t pid);
    void SendLogoutPacket(CHARACTER *ch, uint32_t pid);
    void SendGuildInfoPacket(CHARACTER *ch);
    void SendGuildDataUpdateToAllMember();

    void Load(uint32_t guild_id, int memberBonus = 0);
    void SaveLevel();
    void SaveSkill();
    void SaveMember(uint32_t pid);

    int GetMaxMemberCount();

    int GetMemberCount() { return m_member.size(); }

    int GetTotalLevel() const;

    // GUILD_MEMBER_COUNT_BONUS
    void SetMemberCountBonus(int iBonus);

    int GetMemberCountBonus() { return m_iMemberCountBonus; }

    void BroadcastMemberCountBonus();
    // END_OF_GUILD_MEMBER_COUNT_BONUS

    int GetMaxGeneralCount() const { return 1 /*+ GetSkillLevel(GUILD_SKILL_DEUNGYONG)/3*/; }

    int GetGeneralCount() const { return m_general_count; }

    const TGuildMember *GetMember(uint32_t pid) const;
    uint32_t GetMemberPID(const std::string &strName);

    bool HasGradeAuth(int grade, uint64_t auth_flag) const;

    void AddComment(CHARACTER *ch, const std::string &str);
    void DeleteComment(CHARACTER *ch, uint32_t comment_id);

    void RefreshComment(CHARACTER *ch);
    void RefreshCommentForce(uint32_t player_id);

    int GetSkillLevel(uint32_t vnum);
    void SkillLevelUp(uint32_t dwVnum);
    void UseSkill(uint32_t dwVnum, CHARACTER *ch, uint32_t pid);

    void SendSkillInfoPacket(CHARACTER *ch) const;
    void ComputeGuildPoints();

    void GuildPointChange(uint8_t type, PointValue amount, bool save = false);

    //void		GuildUpdateAffect(CHARACTER* ch);
    //void		GuildRemoveAffect(CHARACTER* ch);

    void UpdateSkill(uint8_t grade, uint8_t *skill_levels);
    void SendDBSkillUpdate(int amount = 0);

    void SkillRecharge();
    bool ChargeSP(CHARACTER *ch, int iSP);

    void Chat(const char *c_pszText);
    void LocalizedChat(const std::string &text);
    void Chat(const char *c_pszText, const char *issuer);
    void MasterChat(const char *c_pszText, const char *issuer);
    void MasterChatLocalized(const std::string &c_pszText);

    void SkillUsableChange(uint32_t dwSkillVnum, bool bUsable);
    void AdvanceLevel(int iLevel);

    // Guild Money
    void RequestDepositMoney(CHARACTER *ch, Gold iGold);
    void RequestWithdrawMoney(CHARACTER *ch, Gold iGold);

    void RecvMoneyChange(Gold iGold);
    void RecvWithdrawMoneyGive(Gold iChangeGold); // bGive==1 이면 길드장에게 주는 걸 시도하고 성공실패를 디비에게 보낸다

    int GetGuildMoney() const { return m_data.gold; }

    // War general
    void GuildWarPacket(uint32_t guild_id, uint8_t bWarType, uint8_t bWarState);
    void SendEnemyGuild(CHARACTER *ch);

    int GetGuildWarState(uint32_t guild_id);
    bool CanStartWar(uint8_t bGuildWarType);
    uint32_t GetWarStartTime(uint32_t guild_id);
    bool UnderWar(uint32_t guild_id); // 전쟁중인가?
    uint32_t UnderAnyWar(uint8_t bType = GUILD_WAR_TYPE_MAX_NUM);

    // War map relative
    void SetGuildWarMapIndex(uint32_t dwGuildID, int32_t lMapIndex);
    int GetGuildWarType(uint32_t dwGuildOpponent);
    uint32_t GetGuildWarMapIndex(uint32_t dwGuildOpponent);

    // War entry question
    void GuildWarEntryAsk(uint32_t guild_opp);
    void GuildWarEntryAccept(uint32_t guild_opp, CHARACTER *ch);

    // War state relative
    void NotifyGuildMaster(const char *msg);
    void RequestDeclareWar(uint32_t guild_id, uint8_t type);
    void RequestRefuseWar(uint32_t guild_id);

    bool DeclareWar(uint32_t guild_id, uint8_t type, uint8_t state);
    void RefuseWar(uint32_t guild_id);
    bool WaitStartWar(uint32_t guild_id);
    bool CheckStartWar(uint32_t guild_id); // check if StartWar method fails (call it before StartWar)
    void StartWar(uint32_t guild_id);
    void EndWar(uint32_t guild_id);
    void ReserveWar(uint32_t guild_id, uint8_t type);

    // War points relative
    void SetWarScoreAgainstTo(uint32_t guild_opponent, int newpoint);
    int GetWarScoreAgainstTo(uint32_t guild_opponent);

    int GetLadderPoint() const { return m_data.ladder_point; }

    void SetLadderPoint(int point);

    void SetWarData(int iWin, int iDraw, int iLoss) { m_data.win = iWin, m_data.draw = iDraw, m_data.loss = iLoss; }

    void ChangeLadderPoint(int iChange);

    int GetGuildWarWinCount() const { return m_data.win; }

    int GetGuildWarDrawCount() const { return m_data.draw; }

    int GetGuildWarLossCount() const { return m_data.loss; }

    bool HasLand();

    // GUILD_JOIN_BUG_FIX
    /// character 에게 길드가입 초대를 한다.
    /**
     * @param	pchInviter 초대한 character.
     * @param	pchInvitee 초대할 character.
     *
     * 초대하거나 받을수 없는 상태라면 해당하는 채팅 메세지를 전송한다.
     */
    void Invite(CHARACTER *pchInviter, CHARACTER *pchInvitee);

    /// 길드초대에 대한 상대 character 의 수락을 처리한다.
    /**
     * @param	pchInvitee 초대받은 character
     *
     * 길드에 가입가능한 상태가 아니라면 해당하는 채팅 메세지를 전송한다.
     */
    void InviteAccept(CHARACTER *pchInvitee);

    /// 길드초대에 대한 상대 character 의 거부를 처리한다.
    /**
     * @param	dwPID 초대받은 character 의 PID
     */
    void InviteDeny(uint32_t dwPID);
    // END_OF_GUILD_JOIN_BUG_FIX

#ifdef __DUNGEON_FOR_GUILD__
public:
    uint8_t GetDungeonCH() const { return m_data.dungeon_ch; }

    long GetDungeonMapIndex() const { return m_data.dungeon_map; }

    bool RequestDungeon(uint8_t bChannel, long lMapIndex);
    void RecvDungeon(uint8_t bChannel, long lMapIndex);

    uint32_t GetDungeonCooldown() const { return m_data.dungeon_cooldown; }

    bool SetDungeonCooldown(uint32_t dwTime);
    void RecvDungeonCD(uint32_t dwTime);
#endif

#ifdef ENABLE_GUILD_STORAGE
	private:
		CGuildStorage * m_pkStorage;
	public:
		CGuildStorage* GetStorage() { return m_pkStorage; };
		void OpenGuildStorage(CHARACTER* pkOpener);
#endif
private:

    void Initialize();

    TGuildData m_data{};
    int m_general_count;

    // GUILD_MEMBER_COUNT_BONUS
    int m_iMemberCountBonus;
    // END_OF_GUILD_MEMBER_COUNT_BONUS

    typedef std::unordered_map<uint32_t, TGuildMember> TGuildMemberContainer;
    TGuildMemberContainer m_member;

    typedef std::unordered_set<CHARACTER *> TGuildMemberOnlineContainer;
    TGuildMemberOnlineContainer m_memberOnline;

    typedef std::unordered_set<uint32_t> TGuildMemberP2POnlineContainer;
    TGuildMemberP2POnlineContainer m_memberP2POnline;

    void LoadGuildData(SQLMsg *pmsg);
    void LoadGuildSkills(SQLMsg *pmsg);
    void LoadGuildGradeData(SQLMsg *pmsg);
    void LoadGuildMemberData(SQLMsg *pmsg);

    void __P2PUpdateGrade(SQLMsg *pmsg);

    typedef std::unordered_map<uint32_t, TGuildWar> TEnemyGuildContainer;
    TEnemyGuildContainer m_EnemyGuild;

    std::unordered_map<uint32_t, uint32_t> m_mapGuildWarEndTime;

    bool abSkillUsable[GUILD_SKILL_COUNT]{};

    // GUILD_JOIN_BUG_FIX
    /// 길드 가입을 할 수 없을 경우의 에러코드.
    enum GuildJoinErrCode
    {
        GERR_NONE = 0,
        ///< 처리성공
        GERR_WITHDRAWPENALTY,
        ///< 탈퇴후 가입가능한 시간이 지나지 않음
        GERR_COMMISSIONPENALTY,
        ///< 해산후 가입가능한 시간이 지나지 않음
        GERR_ALREADYJOIN,
        ///< 길드가입 대상 캐릭터가 이미 길드에 가입해 있음
        GERR_GUILDISFULL,
        ///< 길드인원 제한 초과
        GERR_GUILD_IS_IN_WAR,
        ///< 길드가 현재 전쟁중
        GERR_INVITE_LIMIT,
        ///< 길드원 가입 제한 상태
        GERR_MAX ///< Error code 최고치. 이 앞에 Error code 를 추가한다.
    };

    /// 길드에 가입 가능한 조건을 검사한다.
    /**
     * @param [in]	pchInvitee 초대받는 character
     * @return	GuildJoinErrCode
     */
    GuildJoinErrCode VerifyGuildJoinableCondition(const CHARACTER *pchInvitee);

    typedef robin_hood::unordered_map<uint32_t, LPEVENT> EventMap;
    EventMap m_GuildInviteEventMap; ///< 길드 초청 Event map. key: 초대받은 캐릭터의 PID
    // END_OF_GUILD_JOIN_BUG_FIX
protected:
    int m_guildPostCommentPulse = 0;
};

template <typename T>
void CGuild::Packet(PacketId id, const T& p) {

    for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
    {
        DESC *d = (*it)->GetDesc();

        if (d)
            d->Send(id, p);
    }

}

#endif
