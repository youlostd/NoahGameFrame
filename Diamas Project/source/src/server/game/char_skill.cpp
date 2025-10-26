#include "char.h"
#include "char_manager.h"

#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>
#include <game/MotionTypes.hpp>

#include "config.h"

#include "desc.h"
#include "desc_manager.h"

#include "GBattle.h"
#include "guild.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "main.h"
#include "party.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "utils.h"
#include "vector.h"

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#include "ItemUtils.h"
#include "motion.h"
#include "war_map.h"

static const uint32_t s_adwSubSkillVnums[] = {
    SKILL_LEADERSHIP,
    SKILL_COMBO,
    SKILL_MINING,
    SKILL_LANGUAGE1,
    SKILL_LANGUAGE2,
    SKILL_LANGUAGE3,
    SKILL_POLYMORPH,
    SKILL_HORSE,
    SKILL_HORSE_SUMMON,
    SKILL_HORSE_WILDATTACK,
    SKILL_HORSE_CHARGE,
    SKILL_HORSE_ESCAPE,
    SKILL_HORSE_WILDATTACK_RANGE,
    SKILL_ADD_HP,
    SKILL_RESIST_PENETRATE,
    SKILL_ADD_MONSTER,
};

struct FPartyPIDCollector {
    std::vector<uint32_t> vecPIDs;

    FPartyPIDCollector() {}

    void operator()(CHARACTER* ch) { vecPIDs.push_back(ch->GetPlayerID()); }
};

struct FPartyMOBCollector {
    std::vector<uint32_t> vecPIDs;

    FPartyMOBCollector() {}

    void operator()(CHARACTER* ch) { vecPIDs.push_back(ch->GetVID()); }
};

bool GetMovablePositionOnLine(SECTREE_MANAGER& sm, uint32_t mapIndex,
                              int32_t sX, int32_t sY, int32_t& eX, int32_t& eY,
                              uint32_t step)
{
    const auto steps = DISTANCE_APPROX(eX - sX, eY - sY) / step;
    for (uint32_t i = 0; i != steps; ++i) {
        const int32_t dX = sX + (eX - sX) * i / steps;
        const int32_t dY = sY + (eY - sY) * i / steps;

        if (sm.IsMovablePosition(mapIndex, dX, dY)) {
            eX = dX;
            eY = dY;
            return true;
        }
    }

    return false;
}

time_t CHARACTER::GetSkillNextReadTime(uint32_t dwVnum) const
{
    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("Skill vnum overflow overflow (Vnum: {})", dwVnum);
        return 0;
    }

    return m_pSkillLevels ? m_pSkillLevels[dwVnum].tNextRead : 0;
}

void CHARACTER::SetSkillNextReadTime(uint32_t dwVnum, time_t time)
{
    if (m_pSkillLevels && dwVnum < SKILL_MAX_NUM)
        m_pSkillLevels[dwVnum].tNextRead = time;
}

bool TSkillUseInfo::HitOnce(uint32_t dwVnum)
{
    // 쓰지도않았으면 때리지도 못한다.
    if (!bUsed)
        return false;
    const auto now = get_dword_time();
    SPDLOG_INFO("__HitOnce NextUse {0} current {1} count {2} scount {3}",
                dwNextSkillUsableTime, now, iHitCount, iSplashCount);

    if (dwNextSkillUsableTime && dwNextSkillUsableTime < now &&
        dwVnum != SKILL_MUYEONG && dwVnum != SKILL_HORSE_WILDATTACK) {
     //   SPDLOG_ERROR("__HitOnce can't hit");
        return false;
    }
    /*
    if (iHitCount == -1) {
        SPDLOG_ERROR("__HitOnce OK {0} {1} {2}", dwNextSkillUsableTime, now,
                     iHitCount);
        return true;
    }

    if (iHitCount) {
        SPDLOG_ERROR("__HitOnce OK {0} {1} {2}", dwNextSkillUsableTime, now,
                     iHitCount);
        iHitCount--;
        return true;
    }

    SPDLOG_WARN("Wrong hit count iHitCount {} iMaxHitCount {}", iHitCount,
                iMaxHitCount);

    return false;
    */
    return true;
}

bool TSkillUseInfo::IsCooltimeOver() const
{
    if (bUsed && dwNextSkillUsableTime > get_dword_time())
        return false;

    return true;
}

bool TSkillUseInfo::UseSkill(bool aisGrandMaster, uint32_t vid,
                             uint32_t dwCooltime, int splashcount, int hitcount,
                             int range)
{
    this->isGrandMaster = aisGrandMaster;
    const auto now = get_dword_time();
    const int64_t delta = dwNextSkillUsableTime - now;
    // 아직 쿨타임이 끝나지 않았다.
    if (bUsed && delta > 1000) {
        SPDLOG_INFO("cooltime is not over delta {0}",
                    dwNextSkillUsableTime - now);
        iHitCount = 0;
        return false;
    }

    bUsed = true;

    if (dwCooltime)
        dwNextSkillUsableTime = now + dwCooltime;
    else
        dwNextSkillUsableTime = 0;

    iRange = range;
    iMaxHitCount = iHitCount = hitcount;

    SPDLOG_DEBUG("UseSkill NextUse {0}  current {1} cooltime {2} hitcount "
                 "{3}/{4}",
                 dwNextSkillUsableTime, now, dwCooltime, iHitCount,
                 iMaxHitCount);

    dwVID = vid;
    iSplashCount = splashcount;
    return true;
}

int CHARACTER::GetChainLightningMaxCount() const
{
    return aiChainLightningCountBySkillLevel[std::min<int>(
        SKILL_MAX_LEVEL, GetSkillLevel(SKILL_CHAIN))];
}

void CHARACTER::SetAffectedEunhyung()
{
    m_dwAffectedEunhyungLevel = GetSkillPower(SKILL_EUNHYUNG);
}

void CHARACTER::SetSkillGroup(uint8_t bSkillGroup)
{
    if (bSkillGroup > 2)
        return;

    if (GetLevel() < 5)
        return;

    m_points.skill_group = bSkillGroup;

    TPacketGCChangeSkillGroup p;
    p.skill_group = m_points.skill_group;

    GetDesc()->Send(HEADER_GC_SKILL_GROUP, p);
}

void CHARACTER::ChangeSkillGroup()
{
    const auto oldLevel141 = GetSkillLevel(141);
    const auto oldLevel143 = GetSkillLevel(143);

    struct SkillGroupSkillIndices {
        uint32_t j1;
        uint32_t j2;
    };

    std::unordered_multimap<uint32_t, SkillGroupSkillIndices> skills = {
        {JOB_WARRIOR, {1, 16}},   {JOB_WARRIOR, {2, 17}},
        {JOB_WARRIOR, {3, 18}},   {JOB_WARRIOR, {4, 19}},
        {JOB_WARRIOR, {5, 20}},

        {JOB_ASSASSIN, {31, 46}}, {JOB_ASSASSIN, {32, 47}},
        {JOB_ASSASSIN, {33, 48}}, {JOB_ASSASSIN, {34, 49}},
        {JOB_ASSASSIN, {35, 50}},

        {JOB_SURA, {61, 76}},     {JOB_SURA, {62, 77}},
        {JOB_SURA, {63, 78}},     {JOB_SURA, {64, 79}},
        {JOB_SURA, {65, 80}},     {JOB_SURA, {66, 81}},

        {JOB_SHAMAN, {91, 106}},  {JOB_SHAMAN, {92, 107}},
        {JOB_SHAMAN, {93, 108}},  {JOB_SHAMAN, {94, 109}},
        {JOB_SHAMAN, {95, 110}},  {JOB_SHAMAN, {96, 111}},
    };

    auto skillCount = skills.count(GetJob());
    uint8_t count = 0;
    {
        auto range = skills.equal_range(GetJob());
        for (auto it = range.first; it != range.second; ++it) {

            if (GetSkillGroup() == 1) {
                if (GetSkillLevel(it->second.j1) == SKILL_MAX_LEVEL)
                    ++count;
            } else {
                if (GetSkillLevel(it->second.j2) == SKILL_MAX_LEVEL)
                    ++count;
            }
        }
    }

    auto targetSkillGroup = GetSkillGroup() == 1 ? 2 : 1;

    ClearSkill();

    SetSkillGroup(targetSkillGroup);

    if (skillCount == count) {
        auto range = skills.equal_range(GetJob());
        for (auto it = range.first; it != range.second; ++it) {
            SetSkillLevel(targetSkillGroup == 1 ? it->second.j1 : it->second.j2,
                          SKILL_MAX_LEVEL);
        }
    }

    SetSkillLevel(141, oldLevel141);
    SetSkillLevel(143, oldLevel143);

    SetHorseLevel(21);
    SetSkillLevel(137, 20);
    SetSkillLevel(138, 20);
    SetSkillLevel(139, 20);
    SetSkillLevel(140, 20);
    SetSkillLevel(131, 10);
    SetSkillLevel(126, 20);
    SetSkillLevel(127, 20);
    SetSkillLevel(128, 20);
    SetSkillLevel(121, 40);
    SetSkillLevel(124, 40);
    SetSkillLevel(129, 40);
    SetSkillLevel(122, 2);

    RemoveGoodAffect();

    SkillLevelPacket();
    ComputePoints();
    ComputeMountPoints();

    UpdatePacket();
}

int CHARACTER::ComputeCooltime(int time)
{
    return CalculateDuration(GetPoint(POINT_CASTING_SPEED), time);
}

void CHARACTER::SkillLevelPacket()
{
    if (!GetDesc())
        return;

    TPacketGCSkillLevel pack;
    std::memcpy(&pack.skills, m_pSkillLevels.get(),
                sizeof(TPlayerSkill) * SKILL_MAX_NUM);
    GetDesc()->Send(HEADER_GC_SKILL_LEVEL, pack);
}

void CHARACTER::SetSkillColor(uint32_t vnum, uint32_t color)
{
    if (!m_pSkillLevels)
        return;

    if (vnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("vnum overflow (vnum {0})", vnum);
        return;
    }

    m_pSkillLevels[vnum].color = color;
}

uint32_t CHARACTER::GetSkillColor(uint32_t vnum) const
{
    if (!m_pSkillLevels)
        return 0;

    if (vnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("vnum overflow (vnum {0})", vnum);
        return 0;
    }

    return m_pSkillLevels[vnum].color;
}

void CHARACTER::SetSkillLevel(uint32_t dwVnum, uint8_t bLev)
{
    if (nullptr == m_pSkillLevels)
        return;

    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("vnum overflow (vnum %u)", dwVnum);
        return;
    }

    if (dwVnum == SKILL_COMBO && bLev == 0)
        m_comboLevel = 0;

    m_pSkillLevels[dwVnum].bLevel = std::min<uint8_t>(SKILL_MAX_LEVEL, bLev);
    m_pSkillLevels[dwVnum].bMasterType = GetMasterTypeFromLevel(bLev);
}

bool CHARACTER::IsLearnableSkill(uint32_t dwSkillVnum) const
{
    const CSkillProto* pkSkill = CSkillManager::instance().Get(dwSkillVnum);

    if (!pkSkill)
        return false;

    if (GetSkillLevel(dwSkillVnum) >= SKILL_MAX_LEVEL)
        return false;

    if (pkSkill->dwType == SKILL_TYPE_SECONDARY ||
        pkSkill->dwType == SKILL_TYPE_SKILLTREE) {
        if (GetSkillLevel(dwSkillVnum) >= pkSkill->bMaxLevel)
            return false;

        return true;
    }

    if (pkSkill->dwType == SKILL_TYPE_HORSE) {
        if (dwSkillVnum == SKILL_HORSE_WILDATTACK_RANGE &&
            GetJob() != JOB_ASSASSIN)
            return false;

        return true;
    }

    if (GetSkillGroup() == 0)
        return false;

    if (SKILL_TYPE_WOLFMAN == pkSkill->dwType && JOB_WOLFMAN == GetJob())
        return true;

    if (pkSkill->dwType - 1 == GetJob())
        return true;

#ifdef ENABLE_678TH_SKILL
    if (SKILL_TYPE_COMMON == pkSkill->dwType) {
        if (SKILL_7_A_ANTI_TANHWAN_2015 <= dwSkillVnum &&
            dwSkillVnum <= SKILL_7_D_ANTI_SALPOONG_2015) {
            for (int i = 0; i < 9; i++) {
                if (unsigned(SKILL_7_A_ANTI_TANHWAN_2015 + i) != dwSkillVnum) {
                    if (0 != GetSkillLevel(SKILL_7_A_ANTI_TANHWAN_2015 + i)) {
                        return false;
                    }
                }
            }

            return true;
        }

        if (SKILL_8_A_POWER_TANHWAN_2015 <= dwSkillVnum &&
            dwSkillVnum <= SKILL_8_D_POWER_SALPOONG_2015) {
            for (int i = 0; i < 9; i++) {
                if (unsigned(SKILL_8_A_POWER_TANHWAN_2015 + i) != dwSkillVnum) {
                    if (0 != GetSkillLevel(SKILL_8_A_POWER_TANHWAN_2015 + i)) {
                        return false;
                    }
                }
            }

            return true;
        }
    }
#endif

    if (pkSkill->dwType == SKILL_TYPE_SKILLTREE) {
        return CheckSkillRequirements(pkSkill);
    }

    return false;
}

// ADD_GRANDMASTER_SKILL
bool CHARACTER::LearnLegendaryGrandMasterSkill(uint32_t dwSkillVnum)
{
    CSkillProto* pkSk = CSkillManager::instance().Get(dwSkillVnum);

    if (!pkSk)
        return false;

    if (!IsLearnableSkill(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "수련할 수 없는 스킬입니다.");
        return false;
    }

    // bType이 0이면 처음부터 책으로 수련 가능
    if (pkSk->dwType == SKILL_TYPE_SECONDARY) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "그랜드 마스터 수련을 할 수 없는 스킬입니다.");
        return false;
    }

    if (GetSkillMasterType(dwSkillVnum) != SKILL_PERFECT_MASTER) {
        if (GetSkillMasterType(dwSkillVnum) > SKILL_PERFECT_MASTER)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "퍼펙트 마스터된 스킬입니다. 더 이상 수련 할 수 "
                               "없습니다.");
        else
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 스킬은 아직 그랜드 마스터 수련을 할 경지에 "
                               "이르지 않았습니다.");
        return false;
    }

    std::string strTrainSkill;
    {
        std::ostringstream os;
        os << "training_grandmaster_skill.skill" << dwSkillVnum;
        strTrainSkill = os.str();
    }

    // 여기서 확률을 계산합니다.
    uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);

    int iTotalReadCount = GetQuestFlag(strTrainSkill) + 1;
    SetQuestFlag(strTrainSkill, iTotalReadCount);

    SkillLevelUp(dwSkillVnum, SKILL_UP_BY_QUEST);

    if (bLastLevel == GetSkillLevel(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? "
                           "젠장!");
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "수련이 실패로 끝났습니다. 다시 도전해주시기 "
                           "바랍니다.");
        LogManager::instance().CharLog(this, dwSkillVnum, "LM_READ_FAIL", "");
        return false;
    }

    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                       "몸에서 뭔가 힘이 터져 나오는 기분이야!");
    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                       "뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!");
    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                       "더 높은 경지의 수련을 성공적으로 끝내셨습니다.");
    LogManager::instance().CharLog(this, dwSkillVnum, "LM_READ_SUCCESS", "");
    return true;
}

// END_OF_ADD_GRANDMASTER_SKILL

// ADD_GRANDMASTER_SKILL
bool CHARACTER::LearnGrandMasterSkill(uint32_t dwSkillVnum)
{
    CSkillProto* pkSk = CSkillManager::instance().Get(dwSkillVnum);

    if (!pkSk)
        return false;

    if (!IsLearnableSkill(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "수련할 수 없는 스킬입니다.");
        return false;
    }

    // bType이 0이면 처음부터 책으로 수련 가능
    if (pkSk->dwType == SKILL_TYPE_SECONDARY) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "그랜드 마스터 수련을 할 수 없는 스킬입니다.");
        return false;
    }

    if (GetSkillMasterType(dwSkillVnum) >= SKILL_PERFECT_MASTER) {
        return LearnLegendaryGrandMasterSkill(dwSkillVnum);
    }

    if (GetSkillMasterType(dwSkillVnum) != SKILL_GRAND_MASTER) {
        if (GetSkillMasterType(dwSkillVnum) > SKILL_GRAND_MASTER)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "퍼펙트 마스터된 스킬입니다. 더 이상 수련 할 수 "
                               "없습니다.");
        else
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 스킬은 아직 그랜드 마스터 수련을 할 경지에 "
                               "이르지 않았습니다.");
        return false;
    }

    std::string strTrainSkill;
    {
        std::ostringstream os;
        os << "training_grandmaster_skill.skill" << dwSkillVnum;
        strTrainSkill = os.str();
    }

    // 여기서 확률을 계산합니다.
    uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);

    int idx = std::min(9, GetSkillLevel(dwSkillVnum) - 30);

    SPDLOG_INFO("LearnGrandMasterSkill %s table idx %d value %d", GetName(),
                idx, aiGrandMasterSkillBookCountForLevelUp[idx]);

    int iTotalReadCount = GetQuestFlag(strTrainSkill) + 1;
    SetQuestFlag(strTrainSkill, iTotalReadCount);

    int iMinReadCount = aiGrandMasterSkillBookMinCount[idx];
    int iMaxReadCount = aiGrandMasterSkillBookMaxCount[idx];

    int iBookCount = aiGrandMasterSkillBookCountForLevelUp[idx];

    if (FindAffect(AFFECT_SKILL_BOOK_BONUS) || gConfig.enableAlwaysBook) {
        if (iBookCount & 1)
            iBookCount = iBookCount / 2 + 1;
        else
            iBookCount = iBookCount / 2;

        RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
    }

    int n = Random::get(1, iBookCount);
    SPDLOG_INFO("Number(%d)", n);

    uint32_t nextTime = get_global_time() + gConfig.skillBookDelay;

    SPDLOG_INFO("GrandMaster SkillBookCount min %d cur %d max %d "
                "(next_time=%d)",
                iMinReadCount, iTotalReadCount, iMaxReadCount, nextTime);

    bool bSuccess = n == 2;

    if (iTotalReadCount < iMinReadCount)
        bSuccess = false;
    if (iTotalReadCount > iMaxReadCount)
        bSuccess = true;

    if (bSuccess) {
        SkillLevelUp(dwSkillVnum, SKILL_UP_BY_QUEST);
    }

    SetSkillNextReadTime(dwSkillVnum, nextTime);

    if (bLastLevel == GetSkillLevel(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? "
                           "젠장!");
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "수련이 실패로 끝났습니다. 다시 도전해주시기 "
                           "바랍니다.");
        LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_FAIL", "");
        return false;
    }

    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                       "몸에서 뭔가 힘이 터져 나오는 기분이야!");
    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                       "뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!");
    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                       "더 높은 경지의 수련을 성공적으로 끝내셨습니다.");
    LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_SUCCESS", "");
    return true;
}

// END_OF_ADD_GRANDMASTER_SKILL

bool CHARACTER::LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb)
{
    const CSkillProto* pkSk = CSkillManager::instance().Get(dwSkillVnum);

    if (!pkSk)
        return false;

    if (!IsLearnableSkill(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "수련할 수 없는 스킬입니다.");
        return false;
    }

    uint32_t need_exp = 0;

    // bType이 0이면 처음부터 책으로 수련 가능
    if (pkSk->dwType != SKILL_TYPE_SECONDARY) {
        if (GetSkillMasterType(dwSkillVnum) != SKILL_MASTER) {
            if (GetSkillMasterType(dwSkillVnum) > SKILL_MASTER)
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "이 스킬은 책으로 더이상 수련할 수 "
                                   "없습니다.");
            else
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "이 스킬은 아직 책으로 수련할 경지에 이르지 "
                                   "않았습니다.");
            return false;
        }
    }

    if (get_global_time() < GetSkillNextReadTime(dwSkillVnum)) {
        if (!quest::CQuestManager::instance().GetEventFlag("no_read_delay")) {
            if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY)) {
                // 주안술서 사용중에는 시간 제한 무시
                RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "주안술서를 통해 주화입마에서 "
                                   "빠져나왔습니다.");
            } else {
                SkillLearnWaitMoreTimeMessage(
                    GetSkillNextReadTime(dwSkillVnum) - get_global_time());
                return false;
            }
        }
    }

    // 여기서 확률을 계산합니다.
    uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);

    if (bProb != 0) {
        // SKILL_BOOK_BONUS
        if (gConfig.enableAlwaysBook == true) {
            bProb = 100;
        } else if (FindAffect(AFFECT_SKILL_BOOK_BONUS)) {
            bProb += bProb / 2;
            RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
        }
        // END_OF_SKILL_BOOK_BONUS

        SPDLOG_INFO("LearnSkillByBook Pct %u prob %d", dwSkillVnum, bProb);

        if (Random::get(1, 100) <= bProb) {
            if (gConfig.testServer)
                SPDLOG_INFO("LearnSkillByBook %u SUCC", dwSkillVnum);

            SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
        } else {
            if (gConfig.testServer)
                SPDLOG_INFO("LearnSkillByBook %u FAIL", dwSkillVnum);
        }
    } else {
        int need_bookcount = GetSkillLevel(dwSkillVnum) - 20;

        if (gConfig.disableSkillBookStepLeveling)
            need_bookcount = 0;

        quest::CQuestManager& q = quest::CQuestManager::instance();
        quest::PC* pPC = q.GetPC(GetPlayerID());

        if (pPC) {
            const auto flag =
                fmt::format("traning_master_skill.{}.read_count", dwSkillVnum);

            int read_count = pPC->GetFlag(flag);
            int percent = 65;

            if (FindAffect(AFFECT_SKILL_BOOK_BONUS) ||
                gConfig.enableAlwaysBook) {
                percent = 0;
                RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
            }

            if (Random::get(1, 100) > percent) {
                // 책읽기에 성공
                if (read_count >= need_bookcount) {
                    SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
                    pPC->SetFlag(flag, 0);

                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "책으로 더 높은 경지의 수련을 "
                                       "성공적으로 끝내셨습니다.");
                    LogManager::instance().CharLog(this, dwSkillVnum,
                                                   "READ_SUCCESS", "");
                    return true;
                } else {
                    pPC->SetFlag(flag, read_count + 1);

                    switch (Random::get(1, 3)) {
                        case 1:
                            SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                               "어느정도 이 기술에 대해 이해가 "
                                               "되었지만 조금 부족한듯 한데..");
                            break;

                        case 2:
                            SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                               "드디어 끝이 보이는 건가...  이 "
                                               "기술은 이해하기가 너무 "
                                               "힘들어..");
                            break;

                        case 3:
                        default:
                            SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                               "열심히 하는 배움을 가지는 "
                                               "것만이 기술을 배울수 있는 "
                                               "유일한 길이다..");
                            break;
                    }

                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "%d 권을 더 읽어야 수련을 완료 할 수 "
                                       "있습니다.",
                                       need_bookcount - read_count);
                    return true;
                }
            }
        } else {
            // 사용자의 퀘스트 정보 로드 실패
        }
    }

    if (bLastLevel != GetSkillLevel(dwSkillVnum)) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "몸에서 뭔가 힘이 터져 나오는 기분이야!");
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!");
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "책으로 더 높은 경지의 수련을 성공적으로 "
                           "끝내셨습니다.");
        LogManager::instance().CharLog(this, dwSkillVnum, "READ_SUCCESS", "");
    } else {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? "
                           "젠장!");
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "수련이 실패로 끝났습니다. 다시 도전해주시기 "
                           "바랍니다.");
        LogManager::instance().CharLog(this, dwSkillVnum, "READ_FAIL", "");
    }

    return true;
}

bool CHARACTER::SkillLevelDown(uint32_t dwVnum)
{
    if (nullptr == m_pSkillLevels)
        return false;

    if (gConfig.disableSkills)
        return false;

    if (IsPolymorphed())
        return false;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk) {
        SPDLOG_ERROR("There is no such skill by number %u", dwVnum);
        return false;
    }

    if (!IsLearnableSkill(dwVnum))
        return false;

    if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
        return false;

    if (!GetSkillGroup())
        return false;

    if (pkSk->dwVnum >= SKILL_MAX_NUM)
        return false;

    if (m_pSkillLevels[pkSk->dwVnum].bLevel == 0)
        return false;

    int idx = POINT_SKILL;
    switch (pkSk->dwType) {
        case SKILL_TYPE_SECONDARY:
            idx = POINT_SUB_SKILL;
            break;
        case SKILL_TYPE_WARRIOR:
        case SKILL_TYPE_ASSASSIN:
        case SKILL_TYPE_SURA:
        case SKILL_TYPE_SHAMAN:
        case SKILL_TYPE_COMMON:
        case SKILL_TYPE_WOLFMAN:
            idx = POINT_SKILL;
            break;
        case SKILL_TYPE_HORSE:
            idx = POINT_HORSE_SKILL;
            break;
        case SKILL_TYPE_SKILLTREE:
            idx = POINT_SKILLTREE_POINTS;
            break;
        default:
            SPDLOG_ERROR("Invalid Skill type {} for Skill {}", pkSk->dwType,
                         pkSk->dwVnum);
            return false;
    }

    PointChange(idx, +1);
    SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel - 1);

    Save();

    ComputePoints();
    ComputeMountPoints();

    SkillLevelPacket();
    return true;
}

bool CHARACTER::CheckSkillRequirements(const CSkillProto* pkSk) const
{
    for (auto& requirement : pkSk->requirements) {
        if (GetSkillLevel(requirement.skillVnum) < requirement.skillLevel)
            return false;
    }
    return true;
}

void CHARACTER::SkillLevelUp(uint32_t dwVnum, uint8_t bMethod)
{
    if (nullptr == m_pSkillLevels)
        return;

    if (gConfig.disableSkills)
        return;

    if (IsPolymorphed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "둔갑 중에는 능력을 올릴 수 없습니다.");
        return;
    }

    if (SKILL_7_A_ANTI_TANHWAN <= dwVnum && dwVnum <= SKILL_8_D_ANTI_BYEURAK) {
        if (0 == GetSkillLevel(dwVnum))
            return;
    }

#ifdef ENABLE_678TH_SKILL
    if (SKILL_7_A_ANTI_TANHWAN_2015 <= dwVnum &&
        dwVnum <= SKILL_7_D_ANTI_SALPOONG_2015) {
        if (0 == GetSkillLevel(dwVnum))
            return;
    }

    if (SKILL_8_A_POWER_TANHWAN_2015 <= dwVnum &&
        dwVnum <= SKILL_8_D_POWER_SALPOONG_2015) {
        if (0 == GetSkillLevel(dwVnum))
            return;
    }
#endif

    auto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk) {
        SPDLOG_ERROR("There is no such skill by number (vnum {})", dwVnum);
        return;
    }

    if (pkSk->dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("Skill Vnum overflow (vnum {})", dwVnum);
        return;
    }

    if (!IsLearnableSkill(dwVnum))
        return;

    // 그랜드 마스터는 퀘스트로만 수행가능
    if (pkSk->dwType != SKILL_TYPE_SECONDARY &&
        pkSk->dwType != SKILL_TYPE_SKILLTREE) {
        switch (GetSkillMasterType(pkSk->dwVnum)) {

            case SKILL_GRAND_MASTER:
            case SKILL_PERFECT_MASTER:
                if (bMethod != SKILL_UP_BY_QUEST)
                    return;
                break;

            case SKILL_LEGENDARY_MASTER:
                return;
        }
    }

    if (bMethod == SKILL_UP_BY_POINT) {
        if (pkSk->dwType != SKILL_TYPE_SKILLTREE) {
            // 마스터가 아닌 상태에서만 수련가능
            if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
                return;
        }

        if (IS_SET(pkSk->dwFlag, SKILL_FLAG_DISABLE_BY_POINT_UP))
            return;
    } else if (bMethod == SKILL_UP_BY_BOOK) {
        if (pkSk->dwType !=
            SKILL_TYPE_SECONDARY) // 직업에 속하지 않았거나 포인트로 올릴수 없는
                                  // 스킬은 처음부터 책으로 배울 수 있다.
            if (GetSkillMasterType(pkSk->dwVnum) != SKILL_MASTER)
                return;
    }

    if (GetLevel() < pkSk->bLevelLimit)
        return;

    if (!CheckSkillRequirements(pkSk))
        return;

    if (!GetSkillGroup())
        return;

    if (bMethod == SKILL_UP_BY_POINT) {
        int idx;

        switch (pkSk->dwType) {
            case SKILL_TYPE_SECONDARY:
                idx = POINT_SUB_SKILL;
                break;

            case SKILL_TYPE_WARRIOR:
            case SKILL_TYPE_ASSASSIN:
            case SKILL_TYPE_SURA:
            case SKILL_TYPE_SHAMAN:
            case SKILL_TYPE_WOLFMAN:
            case SKILL_TYPE_COMMON:
                idx = POINT_SKILL;
                break;

            case SKILL_TYPE_HORSE:
                idx = POINT_HORSE_SKILL;
                break;
            case SKILL_TYPE_SKILLTREE:
                idx = POINT_SKILLTREE_POINTS;
                break;
            default:
                SPDLOG_ERROR("Wrong skill type %d skill vnum %d", pkSk->dwType,
                             pkSk->dwVnum);
                return;
        }
        const float k = 1.0f * GetSkillPower(pkSk->dwVnum) / 100.0f;

        pkSk->levelStep.SetVar("k", k /*bSkillLevel*/);

        if (GetPoint(idx) < int32_t(pkSk->levelStep.Eval()))
            return;

        PointChange(idx, -int32_t(pkSk->levelStep.Eval()));
    }

    int SkillPointBefore = GetSkillLevel(pkSk->dwVnum);
    SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel + 1);

    if (pkSk->dwType != SKILL_TYPE_SECONDARY &&
        pkSk->dwType != SKILL_TYPE_SKILLTREE) {
        // 갑자기 그레이드 업하는 코딩
        switch (GetSkillMasterType(pkSk->dwVnum)) {
            case SKILL_NORMAL:
                if (GetSkillLevel(pkSk->dwVnum) >= 17) {
                    if (GetQuestFlag("reset_scroll.force_to_master_skill") >
                            0 ||
                        gConfig.disableRandomMasterSkills) {
                        SetSkillLevel(pkSk->dwVnum, 20);
                        SetQuestFlag("reset_scroll.force_to_master_skill", 0);
                    } else {
                        if (Random::get(
                                1, 21 - std::min(20, GetSkillLevel(
                                                         pkSk->dwVnum))) == 1)
                            SetSkillLevel(pkSk->dwVnum, 20);
                    }
                }
                break;

            case SKILL_MASTER:
                if (GetSkillLevel(pkSk->dwVnum) >= 30) {
                    if (Random::get(
                            1, 31 - std::min(30,
                                             GetSkillLevel(pkSk->dwVnum))) == 1)
                        SetSkillLevel(pkSk->dwVnum, 30);
                }
                break;

            case SKILL_GRAND_MASTER:
                if (GetSkillLevel(pkSk->dwVnum) >= 40) {
                    SetSkillLevel(pkSk->dwVnum, 40);
                }
                break;
            case SKILL_PERFECT_MASTER:
                if (GetSkillLevel(pkSk->dwVnum) >= 50) {
                    SetSkillLevel(pkSk->dwVnum, 50);
                }
                break;
        }
    }

    auto szSkillUp = fmt::sprintf(
        "SkillUp: %s %u %d %d[Before:%d] type %u", GetName(), pkSk->dwVnum,
        m_pSkillLevels[pkSk->dwVnum].bMasterType,
        m_pSkillLevels[pkSk->dwVnum].bLevel, SkillPointBefore, pkSk->dwType);

    SPDLOG_DEBUG("{}", szSkillUp);

    LogManager::instance().CharLog(this, pkSk->dwVnum, "SKILLUP",
                                   szSkillUp.c_str());
    Save();

    if ((pkSk->dwVnum == SKILL_ADD_HP || pkSk->dwVnum == SKILL_ADD_MONSTER ||
         pkSk->dwVnum == SKILL_RESIST_PENETRATE) ||
        (pkSk->dwVnum >= SKILL_7_A_ANTI_TANHWAN_2015 &&
         pkSk->dwVnum <= SKILL_8_D_POWER_SALPOONG_2015) ||
        pkSk->dwType == SKILL_TYPE_SKILLTREE) {
        ComputePoints();
        ComputeMountPoints();
    }

    SkillLevelPacket();
}

void CHARACTER::ComputeSkillPoints()
{
    if (gConfig.disableSkills)
        return;
}

void CHARACTER::ResetSkill()
{
    if (nullptr == m_pSkillLevels)
        return;

    // 보조 스킬은 리셋시키지 않는다
    std::unordered_map<uint32_t, TPlayerSkill> vec;
    std::unordered_map<uint32_t, TPlayerSkill> ps;

    for (const auto& skillVnum : s_adwSubSkillVnums) {
        if (skillVnum >= SKILL_MAX_NUM)
            continue;

        vec.emplace(skillVnum, m_pSkillLevels[skillVnum]);
    }

    for (int i = 180; i <= 203; ++i) {
        ps.emplace(i, m_pSkillLevels[i]);
    }

    m_pSkillLevels.reset(new TPlayerSkill[SKILL_MAX_NUM]());

    for (const auto& entry : vec)
        m_pSkillLevels[entry.first] = entry.second;

    for (const auto& entry : ps)
        m_pSkillLevels[entry.first] = entry.second;

    ComputePoints();
    ComputeMountPoints();
    SkillLevelPacket();
    Save();
}

void CHARACTER::ComputeMountPoints()
{
    auto* item = FindToggleItem(this, true, TOGGLE_MOUNT);
    if (item && item->GetSocket(ITEM_SOCKET_TOGGLE_RIDING)) {
        item->ModifyPoints(true);
    }
}

void CHARACTER::ComputePassiveSkill(uint32_t dwVnum)
{
    if (gConfig.disableSkills)
        return;

    if (GetSkillLevel(dwVnum) == 0)
        return;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);
    if (pkSk->dwType != SKILL_TYPE_SKILLTREE &&
        pkSk->dwType != SKILL_TYPE_SECONDARY)
        return;

    if (pkSk->dwVnum == SKILL_ADD_HP || pkSk->dwVnum == SKILL_ADD_MONSTER)
        return;

    if (pkSk->bPointOn == POINT_NONE)
        return;

    pkSk->SetPointVar("k", 1.0f * GetSkillPower(dwVnum) * 1.0f / 100.0f);

    PointChange(pkSk->bPointOn, pkSk->kPointPoly.Eval());

    if (pkSk->bPointOn2 == POINT_NONE)
        return;

    PointChange(pkSk->bPointOn2, pkSk->kPointPoly2.Eval());

    if (pkSk->bPointOn3 == POINT_NONE)
        return;

    PointChange(pkSk->bPointOn3, pkSk->kPointPoly3.Eval());
}

struct FFindNearVictim {
    FFindNearVictim(CHARACTER* center, CHARACTER* attacker,
                    const CHARACTER_SET& excepts_set = empty_set_)
        : m_pkChrCenter(center)
        , m_pkChrNextTarget(nullptr)
        , m_pkChrAttacker(attacker)
        , m_count(0)
        , m_excepts_set(excepts_set)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        if (!m_excepts_set.empty()) {
            if (m_excepts_set.find(pkChr) != m_excepts_set.end())
                return;
        }

        if (m_pkChrCenter == pkChr)
            return;

        if (!battle_is_attackable(m_pkChrAttacker, pkChr)) {
            return;
        }

        int dist = DISTANCE_APPROX(m_pkChrCenter->GetX() - pkChr->GetX(),
                                   m_pkChrCenter->GetY() - pkChr->GetY());
        if (dist < 1000) {
            ++m_count;

            if ((m_count == 1) || Random::get(1, m_count) == 1)
                m_pkChrNextTarget = pkChr;
        }
    }

    CHARACTER* GetVictim() { return m_pkChrNextTarget; }

    CHARACTER* m_pkChrCenter;
    CHARACTER* m_pkChrNextTarget;
    CHARACTER* m_pkChrAttacker;
    int m_count;
    const CHARACTER_SET& m_excepts_set;

    static CHARACTER_SET empty_set_;
};

CHARACTER_SET FFindNearVictim::empty_set_;

EVENTINFO(chain_lightning_event_info)
{
    uint32_t dwVictim;
    uint32_t dwChr;

    chain_lightning_event_info()
        : dwVictim(0)
        , dwChr(0)
    {
    }
};

EVENTFUNC(ChainLightningEvent)
{
    chain_lightning_event_info* info =
        static_cast<chain_lightning_event_info*>(event->info);

    CHARACTER* pkChrVictim = g_pCharManager->Find(info->dwVictim);
    CHARACTER* pkChr = g_pCharManager->Find(info->dwChr);
    CHARACTER* pkTarget = nullptr;

    if (!pkChr || !pkChrVictim) {
        SPDLOG_TRACE("use chainlighting, but no character");
        return 0;
    }

    SPDLOG_TRACE("chainlighting event {0}", pkChr->GetName());

    if (pkChrVictim->GetParty()) // 파티 먼저
    {
        pkTarget = pkChrVictim->GetParty()->GetNextOwnership(
            nullptr, pkChrVictim->GetX(), pkChrVictim->GetY(),
            pkChrVictim->GetMapIndex());
        if (pkTarget == pkChrVictim || !Random::get(0, 2) ||
            pkChr->GetChainLightingExcept().find(pkTarget) !=
                pkChr->GetChainLightingExcept().end())
            pkTarget = nullptr;
    }

    if (!pkTarget) {
        // 1. Find Next victim
        FFindNearVictim f(pkChrVictim, pkChr, pkChr->GetChainLightingExcept());

        if (pkChrVictim->GetSectree()) {
            pkChrVictim->ForEachSeen(f);
            // 2. If exist, compute it again
            pkTarget = f.GetVictim();
        }
    }

    if (pkTarget) {
        pkChrVictim->CreateFly(FLY_CHAIN_LIGHTNING, pkTarget);
        pkChr->ComputeSkill(SKILL_CHAIN, pkTarget);
        pkChr->AddChainLightningExcept(pkTarget);
    } else {
        SPDLOG_WARN("{0} use chainlighting, but find victim failed near {1}",
                    pkChr->GetName(), pkChrVictim->GetName());
    }

    return 0;
}

void SetPolyVarForAttack(CHARACTER* ch, CSkillProto* pkSk, CItem* pkWeapon,
                         CItem* pAcce)
{
    if (ch->IsPC()) {
        if (pkWeapon && pkWeapon->GetItemType() == ITEM_WEAPON) {
            int iWep =
                Random::get(pkWeapon->GetValue(3), pkWeapon->GetValue(4));
            iWep += pkWeapon->GetValue(5);

            int iMtk =
                Random::get(pkWeapon->GetValue(1), pkWeapon->GetValue(2));
            iMtk += pkWeapon->GetValue(5);
            iMtk += ch->GetPoint(POINT_MAGIC_ATT_GRADE_BONUS);

            if (pAcce && pAcce->GetItemType() == ITEM_COSTUME &&
                pAcce->GetSubType() == COSTUME_ACCE) {
                const auto p =
                    ITEM_MANAGER::instance().GetTable(pAcce->GetSocket(1));

                if (p) {
                    long drainPct = pAcce->GetSocket(0);
                    if (drainPct != 0) {
                        iWep += (int)(float(Random::get(p->alValues[3],
                                                        p->alValues[4])) /
                                      100.0f * float(drainPct));
                        iWep += (int)(float(p->alValues[5]) / 100.0f *
                                      float(drainPct));

                        iMtk += (int)(float(Random::get(p->alValues[1],
                                                        p->alValues[2])) /
                                      100.0f * float(drainPct));
                        iMtk += (int)(float(p->alValues[5]) / 100.0f *
                                      float(drainPct));
                    }
                }
            }

            pkSk->SetPointVar("wep", iWep);
            pkSk->SetPointVar("mtk", iMtk);
            pkSk->SetPointVar("mwep", iMtk);
        } else {
            pkSk->SetPointVar("wep", 0);
            pkSk->SetPointVar("mtk", 0);
            pkSk->SetPointVar("mwep", 0);
        }
    } else {
        int iWep = Random::get(ch->GetMobDamageMin(), ch->GetMobDamageMax());
        pkSk->SetPointVar("wep", iWep);
        pkSk->SetPointVar("mwep", iWep);
        pkSk->SetPointVar("mtk", iWep);
    }
}

struct FuncSplashDamage {
    FuncSplashDamage(int x, int y, CSkillProto* pkSk, CHARACTER* pkChr,
                     int iAmount, int iAG, int iMaxHit, CItem* pkWeapon,
                     CItem* pAcce, bool bDisableCooltime, TSkillUseInfo* pInfo,
                     uint8_t bUseSkillPower)
        : m_x(x)
        , m_y(y)
        , m_pkSk(pkSk)
        , m_pkChr(pkChr)
        , m_iAmount(iAmount)
        , m_iAG(iAG)
        , m_iCount(0)
        , m_iMaxHit(iMaxHit)
        , m_pkWeapon(pkWeapon)
        , m_pAcce(pAcce)
        , m_bDisableCooltime(bDisableCooltime)
        , m_pInfo(pInfo)
        , m_bUseSkillPower(bUseSkillPower)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER)) {
            // if (m_pkSk->dwVnum == SKILL_CHAIN) SPDLOG_INFO( "CHAIN target not
            // character %s", m_pkChr->GetName());
            return;
        }

        CHARACTER* pkChrVictim = (CHARACTER*)ent;

        if (pkChrVictim->IsPC()) {
            if (gConfig.IsPvPDisabledMap(pkChrVictim->GetMapIndex()))
                return;
        }

        if (pkChrVictim->IsPet() || pkChrVictim->IsToggleMount())
            return;

        if (DISTANCE_APPROX(m_x - pkChrVictim->GetX(),
                            m_y - pkChrVictim->GetY()) > m_pkSk->iSplashRange) {
            if (gConfig.testServer)
                SPDLOG_INFO("XXX target too far %s", m_pkChr->GetName());
            return;
        }

#ifdef __FAKE_PC__
        if (m_pkChr->FakePC_Check() &&
            !IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SPLASH)) {
            const float fMaxRotationDif = 30.0f;

            float fRealRotation = m_pkChr->GetRotation();
            float fHitRotation = GetDegreeFromPositionXY(
                m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(),
                pkChrVictim->GetY());

            if (fRealRotation > 180.0f)
                fRealRotation = 360.0f - fRealRotation;
            if (fHitRotation > 180.0f)
                fHitRotation = 360.0f - fHitRotation;

            float fDif = abs(fRealRotation - fHitRotation);
            if (fDif > fMaxRotationDif) {
                return;
            }
        }
#endif

        if (!battle_is_attackable(m_pkChr, pkChrVictim)) {
            if (gConfig.testServer)
                SPDLOG_INFO("XXX target not attackable %s", m_pkChr->GetName());
            return;
        }

        if (m_pkChr->IsPC())
            // 길드 스킬은 쿨타임 처리를 하지 않는다.
            if (!(m_pkSk->dwVnum >= GUILD_SKILL_START &&
                  m_pkSk->dwVnum <= GUILD_SKILL_END))
                if (!m_bDisableCooltime && m_pInfo &&
                    !m_pInfo->HitOnce(m_pkSk->dwVnum) &&
                    m_pkSk->dwVnum != SKILL_MUYEONG) {
                    if (gConfig.testServer)
                        SPDLOG_INFO("check guild skill %s", m_pkChr->GetName());
                    return;
                }

        ++m_iCount;

        PointValue iDam;

        ////////////////////////////////////////////////////////////////////////////////
        float k = 1.0f * static_cast<float>(m_bUseSkillPower) * 1.0f / 100.0f;
        // m_pkSk->kPointPoly2.SetVar("k", 1.0 * m_bUseSkillPower *
        // m_pkSk->bMaxLevel / 100);
        m_pkSk->SetPointVar("k", k);
        m_pkSk->SetPointVar("lv", m_pkChr->GetLevel());
        m_pkSk->SetPointVar("iq", m_pkChr->GetPoint(POINT_IQ));
        m_pkSk->SetPointVar("str", m_pkChr->GetPoint(POINT_ST));
        m_pkSk->SetPointVar("dex", m_pkChr->GetPoint(POINT_DX));
        m_pkSk->SetPointVar("con", m_pkChr->GetPoint(POINT_HT));
        m_pkSk->SetPointVar("def", m_pkChr->GetPoint(POINT_DEF_GRADE));
        m_pkSk->SetPointVar("odef",
                            m_pkChr->GetPoint(POINT_DEF_GRADE) -
                                m_pkChr->GetPoint(POINT_DEF_GRADE_BONUS));
        m_pkSk->SetPointVar("horse_level", m_pkChr->GetHorseLevel());
        m_pkSk->SetPointVar("vdef", pkChrVictim->GetPoint(POINT_DEF_GRADE));
        m_pkSk->SetPointVar("vodef",
                            pkChrVictim->GetPoint(POINT_DEF_GRADE) -
                                pkChrVictim->GetPoint(POINT_DEF_GRADE_BONUS));

        // int iPenetratePct = (int)(1 + k*4);
        bool bIgnoreDefense = false;

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_PENETRATE)) {
            PointValue iPenetratePct = m_pkSk->kPointPoly2.Eval();

            if (Random::get(1, 100) <= iPenetratePct)
                bIgnoreDefense = true;
        }

        bool bIgnoreTargetRating = false;

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_IGNORE_TARGET_RATING)) {
            PointValue iPct = m_pkSk->kPointPoly2.Eval();

            if (Random::get(1, 100) <= iPct)
                bIgnoreTargetRating = true;
        }

        m_pkSk->SetPointVar(
            "ar", CalcAttackRating(m_pkChr, pkChrVictim, bIgnoreTargetRating));

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
            m_pkSk->SetPointVar("atk",
                                CalcMeleeDamage(m_pkChr, pkChrVictim, true,
                                                bIgnoreTargetRating));
        else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE)) {
            CItem* pkBow;
            CItem* pkArrow;

            if (1 == m_pkChr->GetArrowAndBow(&pkBow, &pkArrow, 1))
                m_pkSk->SetPointVar("atk",
                                    CalcArrowDamage(m_pkChr, pkChrVictim, pkBow,
                                                    pkArrow, true));
            else
                m_pkSk->SetPointVar("atk", 0);
        }

        if (m_pkSk->bPointOn == POINT_MOV_SPEED)
            m_pkSk->kPointPoly.SetVar(
                "maxv", pkChrVictim->GetLimitPoint(POINT_MOV_SPEED));

        m_pkSk->SetPointVar("maxhp",
                            static_cast<PointValue>(pkChrVictim->GetMaxHP()));
        m_pkSk->SetPointVar("maxsp", pkChrVictim->GetMaxSP());

        m_pkSk->SetPointVar("chain", m_pkChr->GetChainLightningIndex());
        m_pkChr->IncChainLightningIndex();

        bool bUnderEunhyung =
            m_pkChr->GetAffectedEunhyung(); // 이건 왜 여기서 하지??

        m_pkSk->SetPointVar("ek", m_pkChr->GetAffectedEunhyung() * 1. / 100);
        // m_pkChr->ClearAffectedEunhyung();
        SetPolyVarForAttack(m_pkChr, m_pkSk, m_pkWeapon, m_pAcce);

        PointValue iAmount = 0;

        if (m_pkChr->GetUsedSkillMasterType(m_pkSk->dwVnum) >=
            SKILL_GRAND_MASTER) {
            iAmount = m_pkSk->kMasterBonusPoly.Eval();
        } else {
            iAmount = m_pkSk->kPointPoly.Eval();
        }

        if (gConfig.testServer && iAmount == 0.0 &&
            m_pkSk->bPointOn != POINT_NONE) {
            m_pkChr->ChatPacket(CHAT_TYPE_INFO, "Has no effect. Check the "
                                                "skill formula");
        }
        ////////////////////////////////////////////////////////////////////////////////
        iAmount = -iAmount;

        const auto weapon = m_pkChr->GetWear(WEAR_WEAPON);

        switch (m_pkSk->dwVnum) {
            case SKILL_AMSEOP: {
                float adjust = 1.0f;

                if (bUnderEunhyung)
                    adjust += 0.5f;

                if (weapon && weapon->GetSubType() == WEAPON_DAGGER)
                    adjust += 0.5f;

                iAmount = (iAmount * adjust);
                break;
            }

            case SKILL_GUNGSIN: {
                float adjust = 1.0;
                if (weapon && weapon->GetSubType() == WEAPON_DAGGER)
                    adjust = 1.4f;

                iAmount = (iAmount * adjust);
                break;
            }

            case SKILL_CHAYEOL:
            case SKILL_SALPOONG:
            case SKILL_GONGDAB: {
                float adjust = 1.0;

                if (weapon && weapon->GetSubType() == WEAPON_CLAW)
                    adjust = 1.35f;

                iAmount = (iAmount * adjust);
                break;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // SPDLOG_INFO( "name: %s skill: %s amount %d to %s",
        // m_pkChr->GetName(), m_pkSk->szName, iAmount, pkChrVictim->GetName());

        iDam = CalcBattleDamage(iAmount, m_pkChr->GetLevel(),
                                pkChrVictim->GetLevel());

        if (m_pkChr->IsPC() &&
            m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum].GetMainTargetVID() !=
                (uint32_t)pkChrVictim->GetVID()) {
            // 데미지 감소
            iDam = (iDam * m_pkSk->kSplashAroundDamageAdjustPoly.Eval());
        }

        // TODO 스킬에 따른 데미지 타입 기록해야한다.
        EDamageType dt = DAMAGE_TYPE_NONE;

        switch (m_pkSk->bSkillAttrType) {
            case SKILL_ATTR_TYPE_NORMAL:
                break;

            case SKILL_ATTR_TYPE_MELEE: {
                dt = DAMAGE_TYPE_MELEE;

                CItem* pkWeapon = m_pkChr->GetWear(WEAR_WEAPON);

                if (pkWeapon)
                    switch (pkWeapon->GetSubType()) {
                        case WEAPON_SWORD:
                            iDam = iDam *
                                   (100.0 -
                                    pkChrVictim->GetPoint(POINT_RESIST_SWORD)) /
                                   100.0;
                            break;

                        case WEAPON_TWO_HANDED:
                            iDam = iDam *
                                   (100.0 - pkChrVictim->GetPoint(
                                                POINT_RESIST_TWOHAND)) /
                                   100.0;
                            // 양손검 페널티 10%
                            // iDam = iDam * 95 / 100;

                            break;

                        case WEAPON_DAGGER:
                            iDam = iDam *
                                   (100.0 - pkChrVictim->GetPoint(
                                                POINT_RESIST_DAGGER)) /
                                   100.0;
                            break;

                        case WEAPON_BELL:
                            iDam = iDam *
                                   (100.0 -
                                    pkChrVictim->GetPoint(POINT_RESIST_BELL)) /
                                   100;
                            break;

                        case WEAPON_FAN:
                            iDam = iDam *
                                   (100.0 -
                                    pkChrVictim->GetPoint(POINT_RESIST_FAN)) /
                                   100.0;
                            break;

                        case WEAPON_CLAW:
                            iDam = iDam *
                                   (100.0 -
                                    pkChrVictim->GetPoint(POINT_RESIST_CLAW)) /
                                   100.0;
                            break;
                        case WEAPON_BOUQUET:
                            iDam = 0;
                            break;
                    }

                if (!bIgnoreDefense)
                    iDam -= pkChrVictim->GetPoint(POINT_DEF_GRADE);
            } break;

            case SKILL_ATTR_TYPE_RANGE:
                dt = DAMAGE_TYPE_RANGE;
                // 으아아아악
                // 예전에 적용안했던 버그가 있어서 방어력 계산을 다시하면 유저가
                // 난리남 iDam -= pkChrVictim->GetPoint(POINT_DEF_GRADE);
                iDam = iDam *
                       (100.0 - pkChrVictim->GetPoint(POINT_RESIST_BOW)) /
                       100.0;
                break;

            case SKILL_ATTR_TYPE_MAGIC:
                dt = DAMAGE_TYPE_MAGIC;
                iDam = ApplyAttackBonus(m_pkChr, pkChrVictim, iDam);
                iDam =
                    ApplyResistBonus(m_pkChr, pkChrVictim, iDam); // 으아아아악
                // 예전에 적용안했던 버그가 있어서 방어력 계산을 다시하면 유저가
                // 난리남 iDam -= pkChrVictim->GetPoint(POINT_MAGIC_DEF_GRADE);
                iDam = iDam *
                       (100.0 - pkChrVictim->GetPoint(POINT_RESIST_MAGIC)) /
                       100.0;
                break;

            default:
                SPDLOG_ERROR("Unknown skill attr type %u vnum %u",
                             m_pkSk->bSkillAttrType, m_pkSk->dwVnum);
                break;
        }

        //
        // 20091109 독일 스킬 속성 요청 작업
        // 기존 스킬 테이블에 SKILL_FLAG_WIND, SKILL_FLAG_ELEC,
        // SKILL_FLAG_FIRE를 가진 스킬이 전혀 없었으므로 몬스터의 RESIST_WIND,
        // RESIST_ELEC, RESIST_FIRE도 사용되지 않고 있었다.
        //
        // PvP와 PvE밸런스 분리를 위해 의도적으로 NPC만 적용하도록 했으며 기존
        // 밸런스와 차이점을 느끼지 못하기 위해 mob_proto의 RESIST_MAGIC을
        // RESIST_WIND, RESIST_ELEC, RESIST_FIRE로 복사하였다.
        //
        if (pkChrVictim->IsNPC()) {
            if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_WIND)) {
                iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_WIND)) /
                       100;
            }

            if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_ELEC)) {
                iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_ELEC)) /
                       100;
            }

            if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE)) {
                iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_FIRE)) /
                       100;
            }
        }

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_COMPUTE_MAGIC_DAMAGE))
            dt = DAMAGE_TYPE_MAGIC;

        if (pkChrVictim->CanBeginFight())
            pkChrVictim->BeginFight(m_pkChr);

        if (m_pkSk->dwVnum == SKILL_CHAIN)
            SPDLOG_INFO("{0} CHAIN INDEX {1} DAM {2} DT {3}",
                        m_pkChr->GetName(),
                        m_pkChr->GetChainLightningIndex() - 1, iDam, dt);

#ifdef ENABLE_678TH_SKILL
        {
            uint8_t AntiSkillID = 0;
            switch (m_pkSk->dwVnum) {
                case SKILL_PALBANG:
                    AntiSkillID = SKILL_7_A_ANTI_TANHWAN_2015;
                    break;
                case SKILL_AMSEOP:
                    AntiSkillID = SKILL_7_B_ANTI_AMSEOP_2015;
                    break;
                case SKILL_SWAERYUNG:
                    AntiSkillID = SKILL_7_C_ANTI_SWAERYUNG_2015;
                    break;
                case SKILL_YONGBI:
                    AntiSkillID = SKILL_7_D_ANTI_YONGBI_2015;
                    break;
                case SKILL_GIGONGCHAM:
                    AntiSkillID = SKILL_7_A_ANTI_GIGONGCHAM_2015;
                    break;
                case SKILL_HWAJO:
                    AntiSkillID = SKILL_7_B_ANTI_YEONSA_2015;
                    break;
                case SKILL_MARYUNG:
                    AntiSkillID = SKILL_7_C_ANTI_MAHWAN_2015;
                    break;
                case SKILL_BYEURAK:
                    AntiSkillID = SKILL_7_D_ANTI_BYEURAK_2015;
                    break;
                case SKILL_SALPOONG:
                    AntiSkillID = SKILL_7_D_ANTI_SALPOONG_2015;
                    break;
            }

            if (0 != AntiSkillID) {
                uint8_t AntiSkillLevel =
                    pkChrVictim->GetSkillLevel(AntiSkillID);

                if (0 != AntiSkillLevel) {
                    CSkillProto* pkSk =
                        CSkillManager::instance().Get(AntiSkillID);
                    if (!pkSk) {
                        SPDLOG_ERROR("There is no anti skill(%d) in skill "
                                     "proto",
                                     AntiSkillID);
                    } else {
                        pkSk->SetPointVar(
                            "k", 1.0f *
                                     pkChrVictim->GetSkillPower(AntiSkillID) *
                                     1.0 / 100);

                        double ResistAmount = pkSk->kPointPoly.Eval();

                        SPDLOG_INFO("ANTI_SKILL_GUARD: Resist(%lf) Orig(%d) "
                                    "Reduce(%d)",
                                    ResistAmount, iDam,
                                    int(iDam * (ResistAmount / 100.0)));

                        iDam -= iDam * (ResistAmount / 100.0);
                    }
                }
            }

            uint8_t PowerSkillID = 0;
            switch (m_pkSk->dwVnum) {
                case SKILL_PALBANG:
                    PowerSkillID = SKILL_8_A_POWER_TANHWAN_2015;
                    break;
                case SKILL_AMSEOP:
                    PowerSkillID = SKILL_8_B_POWER_AMSEOP_2015;
                    break;
                case SKILL_SWAERYUNG:
                    PowerSkillID = SKILL_8_C_POWER_SWAERYUNG_2015;
                    break;
                case SKILL_YONGBI:
                    PowerSkillID = SKILL_8_D_POWER_YONGBI_2015;
                    break;
                case SKILL_GIGONGCHAM:
                    PowerSkillID = SKILL_8_A_POWER_GIGONGCHAM_2015;
                    break;
                case SKILL_HWAJO:
                    PowerSkillID = SKILL_8_B_POWER_YEONSA_2015;
                    break;
                case SKILL_MARYUNG:
                    PowerSkillID = SKILL_8_C_POWER_MAHWAN_2015;
                    break;
                case SKILL_BYEURAK:
                    PowerSkillID = SKILL_8_D_POWER_BYEURAK_2015;
                    break;
                case SKILL_SALPOONG:
                    PowerSkillID = SKILL_8_D_POWER_SALPOONG_2015;
                    break;
            }

            if (0 != PowerSkillID) {
                uint8_t PowerSkillLevel = m_pkChr->GetSkillLevel(PowerSkillID);

                if (0 != PowerSkillLevel) {
                    CSkillProto* pkSk =
                        CSkillManager::instance().Get(PowerSkillID);
                    if (!pkSk) {
                        SPDLOG_ERROR("There is no power skill(%d) in skill "
                                     "proto",
                                     PowerSkillID);
                    } else {
                        pkSk->SetPointVar(
                            "k", 1.0f * m_pkChr->GetSkillPower(PowerSkillID) *
                                     1.0 / 100);

                        double PowerAmount = pkSk->kPointPoly.Eval();

                        SPDLOG_INFO("POWER_SKILL: Percentage(%lf) Orig(%d) "
                                    "Bonus(%d)",
                                    PowerAmount, iDam,
                                    int(iDam * (PowerAmount / 100.0)));

                        iDam += iDam * (PowerAmount / 100.0);
                    }
                }
            }
        }
#endif

        if (!pkChrVictim->Damage(m_pkChr, iDam, dt, m_pkSk->dwVnum) &&
            !pkChrVictim->IsStun()) {
            if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_REMOVE_GOOD_AFFECT)) {
                PointValue iAmount2 = m_pkSk->kPointPoly2.Eval();
                PointValue iDur2 = m_pkSk->kDurationPoly2.Eval();
#ifndef SHELIA_BUILD
                iDur2 += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                int chance = iAmount2;

                if (m_pkChr && m_pkChr->GetLevel() < pkChrVictim->GetLevel()) {
                    int delta = pkChrVictim->GetLevel() - m_pkChr->GetLevel();
                    if (delta > 10)
                        chance = 0;
                }
                if (gConfig.IsSuraRemoveGoodDisabledMap(
                        m_pkChr->GetMapIndex())) {
                    chance = 0;
                }

                if (Random::get(1, 100) <= chance) {
                    pkChrVictim->RemoveGoodAffect();
                    pkChrVictim->AddAffect(AFFECT_PREVENT_GOOD, POINT_NONE, 0,
                                           iDur2, 0, true);
                }
            }

            if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW | SKILL_FLAG_STUN |
                                           SKILL_FLAG_FIRE_CONT |
                                           SKILL_FLAG_POISON |
                                           SKILL_FLAG_BLEEDING)) {
                PointValue iPct = m_pkSk->kPointPoly2.Eval();
                PointValue iDur = m_pkSk->kDurationPoly2.Eval();
#ifndef SHELIA_BUILD
                iDur += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif

                if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_STUN)) {
                    SkillAttackAffect(pkChrVictim, iPct, IMMUNE_STUN,
                                      AFFECT_STUN, POINT_NONE, 0, iDur,
                                      m_pkSk->name.c_str());
                } else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW)) {
                    SkillAttackAffect(pkChrVictim, iPct, IMMUNE_SLOW,
                                      AFFECT_SLOW, POINT_MOV_SPEED, -30, iDur,
                                      m_pkSk->name.c_str());
                } else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE_CONT)) {
                    m_pkSk->SetDurationVar("k",
                                           1.0 * m_bUseSkillPower * 1.0 / 100);
                    m_pkSk->SetDurationVar("iq", m_pkChr->GetPoint(POINT_IQ));

                    iDur = (PointValue)m_pkSk->kDurationPoly2.Eval();
#ifndef SHELIA_BUILD
                    iDur += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                    if (Random::get(1, 100) <= iDur) {
                        pkChrVictim->AttackedByFire(m_pkChr, iPct, 5);
                    }
                } else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_POISON)) {
                    if (Random::get(1, 100) <= iPct)
                        pkChrVictim->AttackedByPoison(m_pkChr);
                }
            }

            if (IS_SET(m_pkSk->dwFlag,
                       SKILL_FLAG_CRUSH | SKILL_FLAG_CRUSH_LONG) &&
                !IS_SET(pkChrVictim->GetAIFlag(), AIFLAG_NOMOVE) &&
                !IS_SET(pkChrVictim->GetAIFlag(), AIFLAG_NOPUSH)) {
                float fCrushSlidingLength = 200;

                if (m_pkChr->IsNPC())
                    fCrushSlidingLength = 400;

                if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_CRUSH_LONG))
                    fCrushSlidingLength *= 2;

                float fx, fy;
                float degree = GetDegreeFromPositionXY(
                    m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(),
                    pkChrVictim->GetY());

                if (m_pkSk->dwVnum == SKILL_HORSE_WILDATTACK) {
                    degree -= m_pkChr->GetRotation();
                    degree = fmod(degree, 360.0f) - 180.0f;

                    if (degree > 0)
                        degree = m_pkChr->GetRotation() + 90.0f;
                    else
                        degree = m_pkChr->GetRotation() - 90.0f;
                }

                GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
                SPDLOG_INFO("CRUSH! {0} -> {1} ({2} {3}) -> ({4} {5})",
                            m_pkChr->GetName(), pkChrVictim->GetName(),
                            pkChrVictim->GetX(), pkChrVictim->GetY(),
                            (int32_t)(pkChrVictim->GetX() + fx),
                            (int32_t)(pkChrVictim->GetY() + fy));
                int32_t tx = (int32_t)(pkChrVictim->GetX() + fx);
                int32_t ty = (int32_t)(pkChrVictim->GetY() + fy);

                if (gConfig.IsDuelOnlyMap(m_pkChr->GetMapIndex())) {
                    pkChrVictim->Sync(tx, ty);
                    pkChrVictim->Stop();
                    pkChrVictim->mining_cancel();
                    // pkChrVictim->SetLastSyncTime(std::chrono::system_clock::now());
                    pkChrVictim->SyncPacket();

                } else {
                    if (m_pkSk->dwVnum == SKILL_GIGONGCHAM ||
                        m_pkSk->dwVnum == SKILL_DAEJINGAK ||
                        m_pkSk->dwVnum == SKILL_DAEJINGAK) {
                        pkChrVictim->Sync(tx, ty);
                        pkChrVictim->Stop();
                        pkChrVictim->mining_cancel();
                        // pkChrVictim->SetLastSyncTime(std::chrono::system_clock::now());
                        pkChrVictim->SyncPacket();
                    } else {
                        pkChrVictim->Sync(tx, ty);
                        pkChrVictim->Goto(tx, ty);
                    }
                }

              if (m_pkChr->IsPC() && m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum]
                                               .GetMainTargetVID() ==
                                           (uint32_t)pkChrVictim->GetVID()) {
                    if (!gConfig.IsDuelOnlyMap(m_pkChr->GetMapIndex())) {

                        if (m_pkSk->dwVnum != 16 && m_pkSk->dwVnum != 18) {
                            SkillAttackAffect(pkChrVictim, 1000, IMMUNE_STUN,
                                              AFFECT_STUN, POINT_NONE, 0, 4,
                                              m_pkSk->name.c_str());
                        }
                    } else {
                        SkillAttackAffect(pkChrVictim, 1000, IMMUNE_STUN,
                                          AFFECT_STUN, POINT_NONE, 0, 4,
                                          m_pkSk->name.c_str());
                    }
                } else {
                    if (m_pkSk->dwVnum != 16 && m_pkSk->dwVnum != 18 && !gConfig.IsDuelOnlyMap(m_pkChr->GetMapIndex())) {
                        pkChrVictim->SyncPacket();
                    }
                }
            }
        }

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_HP_ABSORB)) {
            PointValue iPct = m_pkSk->kPointPoly2.Eval();
            m_pkChr->PointChange(POINT_HP, iDam * iPct / 100);
        }

        if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SP_ABSORB)) {
            PointValue iPct = m_pkSk->kPointPoly2.Eval();
            m_pkChr->PointChange(POINT_SP, iDam * iPct / 100);
        }

        if (m_pkSk->dwVnum == SKILL_CHAIN &&
            m_pkChr->GetChainLightningIndex() <
                m_pkChr->GetChainLightningMaxCount()) {
            chain_lightning_event_info* info =
                AllocEventInfo<chain_lightning_event_info>();

            info->dwVictim = pkChrVictim->GetVID();
            info->dwChr = m_pkChr->GetVID();

            event_create(ChainLightningEvent, info,
                         THECORE_SECS_TO_PASSES(1) / 5);
        }
        if (gConfig.testServer)
            SPDLOG_INFO("FuncSplashDamage End :{0} ", m_pkChr->GetName());
    }

    int m_x;
    int m_y;
    CSkillProto* m_pkSk;
    CHARACTER* m_pkChr;
    int m_iAmount;
    int m_iAG;
    int m_iCount;
    int m_iMaxHit;
    CItem* m_pkWeapon;
    CItem* m_pAcce;
    bool m_bDisableCooltime;
    TSkillUseInfo* m_pInfo;
    uint8_t m_bUseSkillPower;
};

struct FuncSplashAffect {
    FuncSplashAffect(CHARACTER* ch, int x, int y, int iDist, uint32_t dwVnum,
                     uint8_t bPointOn, int iAmount, int iDuration, int iSPCost,
                     bool bOverride, int iMaxHit)
    {
        m_x = x;
        m_y = y;
        m_iDist = iDist;
        m_dwVnum = dwVnum;
        m_bPointOn = bPointOn;
        m_iAmount = iAmount;
        m_iDuration = iDuration;
        m_iSPCost = iSPCost;
        m_bOverride = bOverride;
        m_pkChrAttacker = ch;
        m_iMaxHit = iMaxHit;
        m_iCount = 0;
    }

    void operator()(CEntity* ent)
    {
        if (m_iMaxHit && m_iMaxHit <= m_iCount)
            return;

        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* pkChr = (CHARACTER*)ent;

            if (gConfig.testServer)
                SPDLOG_INFO("FuncSplashAffect step 1 : name:%s vnum:%d iDur:%d",
                            pkChr->GetName(), m_dwVnum, m_iDuration);
            if (DISTANCE_APPROX(m_x - pkChr->GetX(), m_y - pkChr->GetY()) <
                m_iDist) {
                if (gConfig.testServer)
                    SPDLOG_INFO("FuncSplashAffect step 2 : name:%s vnum:%d "
                                "iDur:%d",
                                pkChr->GetName(), m_dwVnum, m_iDuration);
                if (m_dwVnum == SKILL_TUSOK)
                    if (pkChr->CanBeginFight())
                        pkChr->BeginFight(m_pkChrAttacker);

                if (pkChr->IsPC() && m_dwVnum == SKILL_TUSOK)
                    pkChr->AddAffect(m_dwVnum, m_bPointOn, m_iAmount,
                                     m_iDuration / 3, m_iSPCost, m_bOverride,
                                     false,
                                     m_pkChrAttacker->GetSkillColor(m_dwVnum));
                else
                    pkChr->AddAffect(m_dwVnum, m_bPointOn, m_iAmount,
                                     m_iDuration, m_iSPCost, m_bOverride, false,
                                     m_pkChrAttacker->GetSkillColor(m_dwVnum));

                m_iCount++;
            }
        }
    }

    CHARACTER* m_pkChrAttacker;
    int m_x;
    int m_y;
    int m_iDist;
    uint32_t m_dwVnum;
    uint8_t m_bPointOn;
    PointValue m_iAmount;
    int m_iDuration;
    int m_iSPCost;
    bool m_bOverride;
    int m_iMaxHit;
    int m_iCount;
};

EVENTINFO(skill_gwihwan_info)
{
    uint32_t pid;
    uint8_t bsklv;

    skill_gwihwan_info()
        : pid(0)
        , bsklv(0)
    {
    }
};

EVENTFUNC(skill_gwihwan_event)
{
    skill_gwihwan_info* info = static_cast<skill_gwihwan_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("skill_gwihwan_event> <Factor> Null pointer");
        return 0;
    }

    uint32_t pid = info->pid;
    uint8_t sklv = info->bsklv;
    CHARACTER* ch = g_pCharManager->FindByPID(pid);

    if (!ch)
        return 0;

    int percent = 20 * sklv - 1;

    if (Random::get(1, 100) <= percent) {
        PIXEL_POSITION pos;

        // 성공
        if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
                ch->GetMapIndex(), ch->GetEmpire(), pos)) {
            SPDLOG_WARN("Recall: %s %d %d -> %d %d", ch->GetName(), ch->GetX(),
                        ch->GetY(), pos.x, pos.y);
            ch->WarpSet(ch->GetMapIndex(), pos.x, pos.y);
        } else {
            SPDLOG_ERROR("CHARACTER::UseItem : cannot find spawn position "
                         "(name %s, %d x %d)",
                         ch->GetName(), ch->GetX(), ch->GetY());
            ch->GoHome();
        }
    } else {
        //실패
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "귀환에 실패하였습니다.");
    }
    return 0;
}

int CHARACTER::ComputeSkillAtPosition(uint32_t dwVnum,
                                      const PIXEL_POSITION& posTarget,
                                      uint8_t bSkillLevel)
{
    if (GetMountVnum())
        return BATTLE_NONE;

    if (IsPolymorphed())
        return BATTLE_NONE;

    if (gConfig.disableSkills)
        return BATTLE_NONE;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk)
        return BATTLE_NONE;

    if (gConfig.testServer) {
        SPDLOG_INFO("ComputeSkillAtPosition %s vnum %d x %d y %d level %d",
                    GetName(), dwVnum, posTarget.x, posTarget.y, bSkillLevel);
    }

    // 나에게 쓰는 스킬은 내 위치를 쓴다.
    // if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
    //	posTarget = GetXYZ();

    // 스플래쉬가 아닌 스킬은 주위이면 이상하다
    if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
        return BATTLE_NONE;

    if (0 == bSkillLevel) {
        if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0) {
            return BATTLE_NONE;
        }
    }

    const float k = 1.0 * GetSkillPower(pkSk->dwVnum, bSkillLevel) * 1.0 / 100;

    pkSk->SetPointVar("k", k);
    pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE)) {
        pkSk->SetPointVar("atk", CalcMeleeDamage(this, this, true, false));
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE)) {
        pkSk->SetPointVar("atk", CalcMagicDamage(this, this));
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE)) {
        CItem* pkBow;
        CItem* pkArrow;
        if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1)) {
            pkSk->SetPointVar(
                "atk", CalcArrowDamage(this, this, pkBow, pkArrow, true));
        } else {
            pkSk->SetPointVar("atk", 0);
        }
    }

    if (pkSk->bPointOn == POINT_MOV_SPEED) {
        pkSk->SetPointVar("maxv", this->GetLimitPoint(POINT_MOV_SPEED));
    }

    pkSk->SetPointVar("lv", GetLevel());
    pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
    pkSk->SetPointVar("str", GetPoint(POINT_ST));
    pkSk->SetPointVar("dex", GetPoint(POINT_DX));
    pkSk->SetPointVar("con", GetPoint(POINT_HT));
    pkSk->SetPointVar("maxhp", static_cast<PointValue>(GetMaxHP()));
    pkSk->SetPointVar("maxsp", this->GetMaxSP());
    pkSk->SetPointVar("chain", 0);
    pkSk->SetPointVar("ar", CalcAttackRating(this, this));
    pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
    pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) -
                                  GetPoint(POINT_DEF_GRADE_BONUS));
    pkSk->SetPointVar("horse_level", GetHorseLevel());

    if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
        OnMove(true);

    CItem* pkWeapon = GetWear(WEAR_WEAPON);
    CItem* pkAcce = GetWear(WEAR_COSTUME_ACCE);

    SetPolyVarForAttack(this, pkSk, pkWeapon, pkAcce);

    pkSk->SetDurationVar("k", k /*bSkillLevel*/);

    PointValue iAmount = pkSk->kPointPoly.Eval();
    PointValue iAmount2 = pkSk->kPointPoly2.Eval();

    // ADD_GRANDMASTER_SKILL
    PointValue iAmount3 = pkSk->kPointPoly3.Eval();

    if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER) {
        /*
           if (iAmount >= 0)
           iAmount +=  m_pkSk->kMasterBonusPoly.Eval();
           else
           iAmount -=  m_pkSk->kMasterBonusPoly.Eval();
         */
        iAmount = pkSk->kMasterBonusPoly.Eval();
    }

    if (gConfig.testServer && iAmount == 0.0 && pkSk->bPointOn != POINT_NONE) {
        ChatPacket(CHAT_TYPE_INFO, "Has no effect. Check the skill formula");
    }

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT)) {
        if (Random::get(1, 100) <= iAmount2) {
            RemoveBadAffect();
        }
    }
    // END_OF_ADD_GRANDMASTER_SKILL

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE |
                                 SKILL_FLAG_USE_MAGIC_DAMAGE)) {
        //
        // 공격 스킬일 경우
        //
        bool bAdded = false;

        if (pkSk->bPointOn == POINT_HP && iAmount < 0) {
            int iAG = 0;

            FuncSplashDamage f(posTarget.x, posTarget.y, pkSk, this, iAmount,
                               iAG, pkSk->lMaxHit, pkWeapon, pkAcce,
                               m_bDisableCooltime,
                               IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr,
                               GetSkillPower(dwVnum, bSkillLevel));

#ifdef __FAKE_PC__
            if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH) ||
                (FakePC_Check() && pkSk->iSplashRange > 0))
#else
            if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
#endif
            {
                ForEachSeen(f);
            } else {
                f(this);
            }
        } else {
            // if (dwVnum == SKILL_CHAIN) SPDLOG_INFO( "CHAIN skill no damage %d
            // %s", iAmount, GetName());
            PointValue iDur = (PointValue)pkSk->kDurationPoly.Eval();

            if (IsPC())
                if (!(dwVnum >= GUILD_SKILL_START &&
                      dwVnum <= GUILD_SKILL_END)) // 길드 스킬은 쿨타임 처리를
                                                  // 하지 않는다.
                    if (!m_bDisableCooltime &&
                        !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) &&
                        dwVnum != SKILL_MUYEONG) {

                        // if (dwVnum == SKILL_CHAIN) SPDLOG_INFO( "CHAIN skill
                        // cannot hit %s", GetName());
                        return BATTLE_NONE;
                    }

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH)) {
                    AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, iDur, 0,
                              true, false, GetSkillColor(dwVnum));
                } else {
                    if (GetSectree()) {
                        FuncSplashAffect f(this, posTarget.x, posTarget.y,
                                           pkSk->iSplashRange, pkSk->dwVnum,
                                           pkSk->bPointOn, iAmount, iDur, 0,
                                           true, pkSk->lMaxHit);
                        ForEachSeen(f);
                    }
                }
                bAdded = true;
            }
        }

        if (pkSk->bPointOn2 != POINT_NONE) {
            PointValue iDur = (PointValue)pkSk->kDurationPoly2.Eval();

            SPDLOG_WARN("try second %u %d %d", pkSk->dwVnum, pkSk->bPointOn2,
                        iDur);

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH)) {
                    AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, iDur, 0,
                              !bAdded, false, GetSkillColor(dwVnum));
                } else {
                    if (GetSectree()) {
                        FuncSplashAffect f(this, posTarget.x, posTarget.y,
                                           pkSk->iSplashRange, pkSk->dwVnum,
                                           pkSk->bPointOn2, iAmount2, iDur, 0,
                                           !bAdded, pkSk->lMaxHit);
                        ForEachSeen(f);
                    }
                }
                bAdded = true;
            } else {
                PointChange(pkSk->bPointOn2, iAmount2);
            }
        }

        // ADD_GRANDMASTER_SKILL
        if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER &&
            pkSk->bPointOn3 != POINT_NONE) {
            PointValue iDur = (PointValue)pkSk->kDurationPoly3.Eval();

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH)) {
                    AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3, iDur, 0,
                              !bAdded, false, GetSkillColor(dwVnum));
                } else {
                    if (GetSectree()) {
                        FuncSplashAffect f(this, posTarget.x, posTarget.y,
                                           pkSk->iSplashRange, pkSk->dwVnum,
                                           pkSk->bPointOn3, iAmount3, iDur, 0,
                                           !bAdded, pkSk->lMaxHit);
                        ForEachSeen(f);
                    }
                }
            } else {
                PointChange(pkSk->bPointOn3, iAmount3);
            }
        }
        // END_OF_ADD_GRANDMASTER_SKILL

        return BATTLE_DAMAGE;
    } else {
        bool bAdded = false;
        PointValue iDur = (PointValue)pkSk->kDurationPoly.Eval();

        if (iDur > 0) {
#ifndef SHELIA_BUILD
            iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
            // AffectFlag가 없거나, toggle 하는 것이 아니라면..
            pkSk->kDurationSPCostPoly.SetVar("k", k /*bSkillLevel*/);

            AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, iDur,
                      (int32_t)pkSk->kDurationSPCostPoly.Eval(), !bAdded, false,
                      GetSkillColor(dwVnum));

            bAdded = true;
        } else {
            PointChange(pkSk->bPointOn, iAmount);
        }

        if (pkSk->bPointOn2 != POINT_NONE) {
            PointValue iDur = (PointValue)pkSk->kDurationPoly2.Eval();

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, iDur, 0,
                          !bAdded, false, GetSkillColor(dwVnum));
                bAdded = true;
            } else {
                PointChange(pkSk->bPointOn2, iAmount2);
            }
        }

        // ADD_GRANDMASTER_SKILL
        if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER &&
            pkSk->bPointOn3 != POINT_NONE) {
            PointValue iDur = (PointValue)pkSk->kDurationPoly3.Eval();

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3, iDur, 0,
                          !bAdded, false, GetSkillColor(dwVnum));
            } else {
                PointChange(pkSk->bPointOn3, iAmount3);
            }
        }
        // END_OF_ADD_GRANDMASTER_SKILL

        return BATTLE_NONE;
    }
}

// bSkillLevel 인자가 0이 아닐 경우에는 m_abSkillLevels를 사용하지 않고 강제로
// bSkillLevel로 계산한다.
int CHARACTER::ComputeSkill(uint32_t dwVnum, CHARACTER* pkVictim,
                            uint8_t bSkillLevel)
{
    const bool bCanUseHorseSkill = CanUseHorseSkill();

    // 말을 타고있지만 스킬은 사용할 수 없는 상태라면 return
    if (!bCanUseHorseSkill && IsRiding())
        return BATTLE_NONE;

    if (IsPolymorphed())
        return BATTLE_NONE;

    if (gConfig.disableSkills)
        return BATTLE_NONE;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

    if (!pkSk)
        return BATTLE_NONE;

    if (bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE)
        return BATTLE_NONE;

    if (!bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE)
        return BATTLE_NONE;

    // 상대방에게 쓰는 것이 아니면 나에게 써야 한다.
    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
        pkVictim = this;

    if (!pkVictim) {
        if (gConfig.testServer)
            SPDLOG_INFO("ComputeSkill: %s Victim == null, skill %d", GetName(),
                        dwVnum);

        return BATTLE_NONE;
    }

    if (pkSk->dwTargetRange &&
        DISTANCE_SQRT(GetX() - pkVictim->GetX(), GetY() - pkVictim->GetY()) >=
            pkSk->dwTargetRange) {
        if (gConfig.testServer)
            SPDLOG_INFO("ComputeSkill: Victim too far, skill {} : {} to {} "
                        "(distance {} limit {})",
                        dwVnum, GetName(), pkVictim->GetName(),
                        (int32_t)DISTANCE_SQRT(GetX() - pkVictim->GetX(),
                                               GetY() - pkVictim->GetY()),
                        pkSk->dwTargetRange);

        return BATTLE_NONE;
    }

    if (0 == bSkillLevel) {
        if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0) {
            if (gConfig.testServer)
                SPDLOG_INFO("ComputeSkill : name:%s vnum:%d  skillLevelBySkill "
                            ": %d ",
                            GetName(), pkSk->dwVnum, bSkillLevel);
            return BATTLE_NONE;
        }
    }

    if (pkVictim->FindAffect(AFFECT_PREVENT_GOOD) &&
        pkVictim->IsGoodAffect(dwVnum))
        return BATTLE_NONE;

    const auto k = 1.0 * GetSkillPower(pkSk->dwVnum, bSkillLevel) * 1.0 / 100.0;

    pkSk->SetPointVar("k", k);
    pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

    if (pkSk->dwType == SKILL_TYPE_HORSE) {
        CItem* pkBow;
        CItem* pkArrow;
        if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1)) {
            pkSk->SetPointVar(
                "atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
        } else {
            pkSk->SetPointVar("atk",
                              CalcMeleeDamage(this, pkVictim, true, false));
        }
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE)) {
        pkSk->SetPointVar("atk", CalcMeleeDamage(this, pkVictim, true, false));
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE)) {
        pkSk->SetPointVar("atk", CalcMagicDamage(this, pkVictim));
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE)) {
        CItem* pkBow;
        CItem* pkArrow;
        if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1)) {
            pkSk->SetPointVar(
                "atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
        } else {
            pkSk->SetPointVar("atk", 0);
        }
    }

    if (pkSk->bPointOn == POINT_MOV_SPEED) {
        pkSk->SetPointVar("maxv", pkVictim->GetLimitPoint(POINT_MOV_SPEED));
    }

    pkSk->SetPointVar("lv", GetLevel());
    pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
    pkSk->SetPointVar("str", GetPoint(POINT_ST));
    pkSk->SetPointVar("dex", GetPoint(POINT_DX));
    pkSk->SetPointVar("con", GetPoint(POINT_HT));
    pkSk->SetPointVar("maxhp", static_cast<PointValue>(pkVictim->GetMaxHP()));
    pkSk->SetPointVar("maxsp", pkVictim->GetMaxSP());
    pkSk->SetPointVar("chain", 0);
    pkSk->SetPointVar("ar", CalcAttackRating(this, pkVictim));
    pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
    pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) -
                                  GetPoint(POINT_DEF_GRADE_BONUS));
    pkSk->SetPointVar("horse_level", GetHorseLevel());
    pkSk->SetPointVar("vdef", pkVictim->GetPoint(POINT_DEF_GRADE));
    pkSk->SetPointVar("vodef", pkVictim->GetPoint(POINT_DEF_GRADE) -
                                   pkVictim->GetPoint(POINT_DEF_GRADE_BONUS));

    if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
        OnMove(true);

    CItem* pkWeapon = GetWear(WEAR_WEAPON);
    CItem* pkAcce = GetWear(WEAR_COSTUME_ACCE);

    SetPolyVarForAttack(this, pkSk, pkWeapon, pkAcce);

    pkSk->kDurationPoly.SetVar("k", k /*bSkillLevel*/);
    pkSk->kDurationPoly2.SetVar("k", k /*bSkillLevel*/);

    PointValue iAmount = pkSk->kPointPoly.Eval();
    PointValue iAmount2 = pkSk->kPointPoly2.Eval();
    PointValue iAmount3 = pkSk->kPointPoly3.Eval();

    if (gConfig.testServer && IsPC())
        SPDLOG_INFO("iAmount: %d %d %d , atk:%f skLevel:%f k:%f "
                    "GetSkillPower(%d) MaxLevel:%d Per:%f",
                    iAmount, iAmount2, iAmount3, pkSk->kPointPoly.GetVar("atk"),
                    pkSk->kPointPoly.GetVar("k"), k,
                    GetSkillPower(pkSk->dwVnum, bSkillLevel), 1.0, 1.0 / 100);

    // ADD_GRANDMASTER_SKILL
    if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER) {
        iAmount = pkSk->kMasterBonusPoly.Eval();
    }

    if (gConfig.testServer && iAmount == 0.0 && pkSk->bPointOn != POINT_NONE) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "Has no effect. Check the skill formula");
    }
    // END_OF_ADD_GRANDMASTER_SKILL

    // SPDLOG_INFO( "XXX SKILL Calc %d Amount %d", dwVnum, iAmount);

    // REMOVE_BAD_AFFECT_BUG_FIX
    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT)) {
        if (Random::get(1, 100) <= iAmount2) {
            pkVictim->RemoveBadAffect();
        }
    }
    // END_OF_REMOVE_BAD_AFFECT_BUG_FIX

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE |
                                 SKILL_FLAG_USE_MAGIC_DAMAGE) &&
        !(pkSk->dwVnum == SKILL_MUYEONG && pkVictim == this) &&
        !(pkSk->IsChargeSkill() && pkVictim == this)) {
        bool bAdded = false;

        if (pkSk->bPointOn == POINT_HP && iAmount < 0) {
            int critBonus = 0;

            switch (pkSk->dwVnum) {
                case SKILL_CHAYEOL:
                    critBonus = 20;
                    break;
                case SKILL_PASWAE:
                    critBonus = 20;
                    break;
            }

            if (critBonus != 0)
                PointChange(POINT_CRITICAL_PCT, critBonus);

            int iAG = 0;
            FuncSplashDamage f(pkVictim->GetX(), pkVictim->GetY(), pkSk, this,
                               iAmount, iAG, pkSk->lMaxHit, pkWeapon, pkAcce,
                               m_bDisableCooltime,
                               IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr,
                               GetSkillPower(dwVnum, bSkillLevel));

            if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH)) {
                pkVictim->ForEachSeen(f);
            } else {
                f(pkVictim);
            }

            if (critBonus != 0)
                PointChange(POINT_CRITICAL_PCT, -critBonus);
        } else {
            pkSk->kDurationPoly.SetVar("k", k /*bSkillLevel*/);
            PointValue iDur = (PointValue)pkSk->kDurationPoly.Eval();

            if (IsPC())
                if (!(dwVnum >= GUILD_SKILL_START &&
                      dwVnum <= GUILD_SKILL_END)) // 길드 스킬은 쿨타임 처리를
                                                  // 하지 않는다.
                    if (!m_bDisableCooltime &&
                        !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) &&
                        dwVnum != SKILL_MUYEONG) {

                        return BATTLE_NONE;
                    }

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
                    pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount,
                                        iDur, 0, true, false,
                                        GetSkillColor(dwVnum));
                else {
                    if (pkVictim->GetSectree()) {
                        FuncSplashAffect f(
                            this, pkVictim->GetX(), pkVictim->GetY(),
                            pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn,
                            iAmount, iDur, 0, true, pkSk->lMaxHit);
                        pkVictim->ForEachSeen(f);
                    }
                }
                bAdded = true;
            }
        }

        // stun skills for fake pc
#ifdef __FAKE_PC__
        if (FakePC_Check()) {
            const uint32_t stunSkills[] = {SKILL_TANHWAN, SKILL_GEOMPUNG,
                                           SKILL_BYEURAK, SKILL_GIGUNG};

            for (size_t i = 0; i < sizeof(stunSkills) / sizeof(uint32_t); ++i) {
                if (stunSkills[i] == pkSk->dwVnum) {
                    if (pkVictim->GetPoint(POINT_IMMUNE_STUN) == 0)
                        SkillAttackAffect(pkVictim, 1000, IMMUNE_STUN,
                                          AFFECT_STUN, POINT_NONE, 0, 2,
                                          pkSk->name.c_str());
                    break;
                }
            }
        }
#endif

        if (pkSk->bPointOn2 != POINT_NONE && !pkSk->IsChargeSkill()) {
            pkSk->kDurationPoly2.SetVar("k", k /*bSkillLevel*/);
            PointValue iDur = (PointValue)pkSk->kDurationPoly2.Eval();

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
                    pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2,
                                        iDur, 0, !bAdded, false,
                                        GetSkillColor(dwVnum));
                else {
                    if (pkVictim->GetSectree()) {
                        FuncSplashAffect f(
                            this, pkVictim->GetX(), pkVictim->GetY(),
                            pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn2,
                            iAmount2, iDur, 0, !bAdded, pkSk->lMaxHit);
                        pkVictim->ForEachSeen(f);
                    }
                }

                bAdded = true;
            } else {
                pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
            }
        }

        // ADD_GRANDMASTER_SKILL
        if (pkSk->bPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() &&
            GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER) {
            pkSk->kDurationPoly3.SetVar("k", k /*bSkillLevel*/);
            PointValue iDur = (PointValue)pkSk->kDurationPoly3.Eval();

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
                    pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3,
                                        iDur, 0, !bAdded, false,
                                        GetSkillColor(dwVnum));
                else {
                    if (pkVictim->GetSectree()) {
                        FuncSplashAffect f(
                            this, pkVictim->GetX(), pkVictim->GetY(),
                            pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn3,
                            iAmount3, iDur, 0, !bAdded, pkSk->lMaxHit);
                        pkVictim->ForEachSeen(f);
                    }
                }

                bAdded = true;
            } else {
                pkVictim->PointChange(pkSk->bPointOn3, iAmount3);
            }
        }
        // END_OF_ADD_GRANDMASTER_SKILL

        return BATTLE_DAMAGE;
    } else {
        if (dwVnum == SKILL_MUYEONG) {
            pkSk->kDurationPoly.SetVar("k", k /*bSkillLevel*/);
            pkSk->kDurationSPCostPoly.SetVar("k", k /*bSkillLevel*/);

            int iDur = (int32_t)pkSk->kDurationPoly.Eval();

            if (pkVictim == this)
                AddAffect(dwVnum, POINT_NONE, 0, iDur,
                          (int32_t)pkSk->kDurationSPCostPoly.Eval(), true,
                          false, GetSkillColor(dwVnum));

            return BATTLE_NONE;
        }

        bool bAdded = false;
        pkSk->kDurationPoly.SetVar("k", k /*bSkillLevel*/);
        PointValue iDur = (PointValue)pkSk->kDurationPoly.Eval();

        if (iDur > 0) {
#ifndef SHELIA_BUILD
            iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
            // AffectFlag가 없거나, toggle 하는 것이 아니라면..
            pkSk->kDurationSPCostPoly.SetVar("k", k /*bSkillLevel*/);

            if (pkSk->bPointOn2 != POINT_NONE) {
                pkVictim->RemoveAffect(pkSk->dwVnum);

                PointValue iDur2 = pkSk->kDurationPoly2.Eval();

                if (iDur2 > 0) {
#ifndef SHELIA_BUILD
                    iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                    if (gConfig.testServer)
                        SPDLOG_INFO("SKILL_AFFECT: {} {} Dur:{} To:{} "
                                    "Amount:{}",
                                    GetName(), pkSk->name, iDur2,
                                    pkSk->bPointOn2, iAmount2);

                    pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2,
                                        iDur2, 0, false, false,
                                        GetSkillColor(dwVnum));
                } else {
                    pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
                }

                pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, iDur,
                                    (int32_t)pkSk->kDurationSPCostPoly.Eval(),
                                    false, false, GetSkillColor(dwVnum));
            } else {
                if (gConfig.testServer)
                    SPDLOG_INFO("SKILL_AFFECT: {} {} Dur:{} To:{} Amount:{}",
                                GetName(), pkSk->name, iDur, pkSk->bPointOn,
                                iAmount);

                pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, iDur,
                                    (int32_t)pkSk->kDurationSPCostPoly.Eval(),
                                    // ADD_GRANDMASTER_SKILL
                                    !bAdded, false, GetSkillColor(dwVnum));
                // END_OF_ADD_GRANDMASTER_SKILL
            }

            bAdded = true;
        } else {
            if (!pkSk->IsChargeSkill())
                pkVictim->PointChange(pkSk->bPointOn, iAmount);

            if (pkSk->bPointOn2 != POINT_NONE) {
                pkVictim->RemoveAffect(pkSk->IsChargeSkill() ? AFFECT_DASH
                                                             : pkSk->dwVnum);

                int iDur2 = (int)pkSk->kDurationPoly2.Eval();

                if (iDur2 > 0) {
#ifndef SHELIA_BUILD
                    iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                    pkVictim->AddAffect(pkSk->IsChargeSkill() ? AFFECT_DASH
                                                              : pkSk->dwVnum,
                                        pkSk->bPointOn2, iAmount2, iDur2, 0,
                                        false, false, GetSkillColor(dwVnum));
                } else {
                    pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
                }
            }
        }

        // ADD_GRANDMASTER_SKILL
        if (pkSk->bPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() &&
            GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER) {
            pkSk->kDurationPoly3.SetVar("k", k /*bSkillLevel*/);
            PointValue iDur = (PointValue)pkSk->kDurationPoly3.Eval();

            SPDLOG_INFO("try third %u %d %d %d 1894", pkSk->dwVnum,
                        pkSk->bPointOn3, iDur, iAmount3);

            if (iDur > 0) {
#ifndef SHELIA_BUILD
                iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
                if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
                    pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3,
                                        iDur, 0, !bAdded, false,
                                        GetSkillColor(dwVnum));
                else {
                    if (pkVictim->GetSectree()) {
                        FuncSplashAffect f(
                            this, pkVictim->GetX(), pkVictim->GetY(),
                            pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn3,
                            iAmount3, iDur, 0, !bAdded, pkSk->lMaxHit);
                        pkVictim->ForEachSeen(f);
                    }
                }

                bAdded = true;
            } else {
                pkVictim->PointChange(pkSk->bPointOn3, iAmount3);
            }
        }
        // END_OF_ADD_GRANDMASTER_SKILL

        return BATTLE_NONE;
    }
}

struct FPartySkill {
    FPartySkill(CHARACTER* ch, uint32_t vnum, uint8_t skillLevel)
    {
        m_ch = ch;
        m_vnum = vnum;
        m_skillLevel = skillLevel;
    }

    void operator()(CHARACTER* ch)
    {
        m_ch->ComputeSkill(m_vnum, ch, m_skillLevel);
    }

    CHARACTER* m_ch;
    uint32_t m_vnum;
    uint8_t m_skillLevel;
};

bool CHARACTER::ComputePartySkill(unsigned vnum, uint8_t skillLevel)
{
    const auto party = GetParty();
    if (party) {
        if (party->GetMemberCount() == 1)
            return false;

        FPartySkill f(this, vnum, skillLevel);
        party->ForEachOnlineMember(f);
        return true;
    }

    return false;
}

bool CHARACTER::UseSkill(uint32_t dwVnum, CHARACTER* pkVictim,
                         bool bUseGrandMaster)
{
    if ((dwVnum == SKILL_GEOMKYUNG || dwVnum == SKILL_GWIGEOM) && !GetWear(WEAR_WEAPON))
    return false;

    if (!IsNextEquipSkillUsePulse()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot use skills for 2 seconds after changing "
                           "your equipment.");
        return false;
    }

    if (!CanUseSkill(dwVnum))
        return false;

    if (gConfig.disableSkills)
        return false;

    if (IsObserverMode())
        return false;

    if (!CanMove())
        return false;

    if (IsPolymorphed())
        return false;

    uint32_t dwCur = get_dword_time();

    const bool bCanUseHorseSkill = CanUseHorseSkill();

    if (dwVnum == SKILL_HORSE_SUMMON) {
        if (GetSkillLevel(dwVnum) == 0)
            return false;

        if (GetHorseLevel() <= 0)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "말이 없습니다. 마굿간 경비병을 찾아가세요.");
        else
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "말 소환 아이템을 사용하세요.");

        return true;
    }

    // 말을 타고있지만 스킬은 사용할 수 없는 상태라면 return false
    if (!bCanUseHorseSkill && IsRiding())
        return false;

    CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);
    SPDLOG_INFO("{}: USE_SKILL: {} pkVictim {}", GetName(), dwVnum,
                (void*)(pkVictim));

    if (!pkSk)
        return false;

    if (bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE)
        return BATTLE_NONE;

    if (!bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE)
        return BATTLE_NONE;

    if (GetSkillLevel(dwVnum) == 0)
        return false;

    if (pkVictim && IsPC() && this != pkVictim &&
        (!IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK) &&
         gConfig.IsDuelOnlyMap(GetMapIndex()))) {
        return false;
    }
    // NO_GRANDMASTER
    if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
        bUseGrandMaster = false;
    // END_OF_NO_GRANDMASTER

    // MINING
    if (GetWear(WEAR_WEAPON) &&
        (GetWear(WEAR_WEAPON)->GetItemType() == ITEM_ROD ||
         GetWear(WEAR_WEAPON)->GetItemType() == ITEM_PICK))
        return false;
    // END_OF_MINING

    auto& skillUseInfo = m_SkillUseInfo[dwVnum];
    skillUseInfo.hitLog.Clear();

    if (pkSk->IsChargeSkill()) {
        // Don't use |chargeAffect| after using ComputeSkill
        const auto chargeAffect = FindAffect(AFFECT_DASH);
        if (chargeAffect || (pkVictim && pkVictim != this)) {
            if (!pkVictim)
                return false;

            if (!chargeAffect) {
                if (!UseSkill(dwVnum, this))
                    return false;
            }

            skillUseInfo.SetMainTargetVID(pkVictim->GetVID());
            ComputeSkill(dwVnum, pkVictim);
            RemoveAffect(AFFECT_DASH);
            return true;
        }
    }

    if (dwVnum == SKILL_COMBO) {
        if (m_comboLevel)
            m_comboLevel = 0;
        else
            m_comboLevel = GetSkillLevel(SKILL_COMBO);

        UpdatePacket();
        return true;
    }

    // Toggle 할 때는 SP를 쓰지 않음 (SelfOnly로 구분)
    if ((pkSk->dwFlag & SKILL_FLAG_TOGGLE) && RemoveAffect(pkSk->dwVnum))
        return true;

    if (FindAffect(AFFECT_REVIVE_INVISIBLE))
        RemoveAffect(AFFECT_REVIVE_INVISIBLE);

    const float k = 1.0 * GetSkillPower(pkSk->dwVnum) * 1.0f / 100.0f;

    pkSk->SetPointVar("k", k);
    pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

    // 쿨타임 체크
    pkSk->kCooldownPoly.SetVar("k", k);
    int iCooltime = (int)pkSk->kCooldownPoly.Eval();
    int lMaxHit = pkSk->lMaxHit ? pkSk->lMaxHit : -1;

    pkSk->SetSPCostVar("k", k);

    if (dwVnum == SKILL_TERROR && skillUseInfo.bUsed &&
        skillUseInfo.dwNextSkillUsableTime > dwCur) {
        SPDLOG_INFO(" SKILL_TERROR's Cooltime is not delta over %u",
                    skillUseInfo.dwNextSkillUsableTime - dwCur);
        return false;
    }

    int iNeededSP = 0;

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_HP_AS_COST)) {
        pkSk->SetSPCostVar("maxhp", static_cast<PointValue>(GetMaxHP()));
        pkSk->SetSPCostVar("v", GetHP());
        iNeededSP = pkSk->kSPCostPoly.Eval();

        // ADD_GRANDMASTER_SKILL
        if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER &&
            bUseGrandMaster) {
            iNeededSP = pkSk->kGrandMasterAddSPCostPoly.Eval();
        }
        // END_OF_ADD_GRANDMASTER_SKILL

        if (GetHP() < iNeededSP)
            return false;

        PointChange(POINT_HP, -iNeededSP);
    } else {
        // SKILL_FOMULA_REFACTORING
        pkSk->SetSPCostVar("maxhp", static_cast<PointValue>(GetMaxHP()));
        pkSk->SetSPCostVar("maxv", GetMaxSP());
        pkSk->SetSPCostVar("v", GetSP());

        iNeededSP = (int)pkSk->kSPCostPoly.Eval();

        if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER &&
            bUseGrandMaster) {
            iNeededSP = (int)pkSk->kGrandMasterAddSPCostPoly.Eval();
        }
        // END_OF_SKILL_FOMULA_REFACTORING

        if (GetSP() < iNeededSP)
            return false;

        if (gConfig.testServer)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "%s SP소모: %d",
                               pkSk->name.c_str(), iNeededSP);

        PointChange(POINT_SP, -iNeededSP);
    }

    const bool isSelfCharging =
        pkSk->IsChargeSkill() && !FindAffect(AFFECT_DASH) && !pkVictim;
    if (pkSk->dwVnum == SKILL_MUYEONG || isSelfCharging) {
        pkVictim = this;
    }

    int iSplashCount = 1;

    if (!m_bDisableCooltime) {
        if (!skillUseInfo.UseSkill(
                bUseGrandMaster,
                (pkVictim && SKILL_HORSE_WILDATTACK != dwVnum)
                    ? pkVictim->GetVID()
                    : 0,
                ComputeCooltime(iCooltime * 1000), iSplashCount, lMaxHit)) {
            if (gConfig.testServer)
                ChatPacket(CHAT_TYPE_BIG_NOTICE, "cooltime not finished %s %d",
                           pkSk->name.c_str(), iCooltime);

            return false;
        }
    }

    if (dwVnum == SKILL_CHAIN) {
        ResetChainLightningIndex();
        AddChainLightningExcept(pkVictim);
    }

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY)) {
        ComputeSkill(dwVnum, this);
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY)) {
        if (!ComputePartySkill(dwVnum, 0))
            ComputeSkill(dwVnum, this);
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY_AND_ALL)) {
        const auto* party = GetParty();
        if (party && party->IsMember(pkVictim->GetPlayerID())) {
            if (!ComputePartySkill(dwVnum, 0)) {
                if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
                    ComputeSkill(dwVnum, this);
                else {
                    ComputeSkill(dwVnum, pkVictim);
                }
            }
        } else {
            if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
                ComputeSkill(dwVnum, this);
            else {
                ComputeSkill(dwVnum, pkVictim);
            }
        }
    }
#ifdef __FAKE_PC__
    else if (FakePC_Check()) {
        ComputeSkill(dwVnum, pkVictim);
    }
#endif
    else if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK)) {
        ComputeSkill(dwVnum, pkVictim);
    } else if (dwVnum == SKILL_BYEURAK) {
        ComputeSkill(dwVnum, pkVictim);
    } else if (dwVnum == SKILL_MUYEONG || pkSk->IsChargeSkill()) {
        ComputeSkill(dwVnum, pkVictim);
    }

    m_dwLastSkillTime = get_dword_time();
    m_dwLastSkillVnum = dwVnum;

    return true;
}

int CHARACTER::GetUsedSkillMasterType(uint32_t dwVnum)
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetUsedSkillMasterType(dwVnum);
#endif
    const TSkillUseInfo& rInfo = m_SkillUseInfo[dwVnum];

    if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
        return GetSkillMasterType(dwVnum);

    if (rInfo.isGrandMaster)
        return GetSkillMasterType(dwVnum);

    return std::min<int>(GetSkillMasterType(dwVnum), SKILL_MASTER);
}

int CHARACTER::GetSkillMasterType(uint32_t dwVnum) const
{
    if (!IsPC())
        return 0;

    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("%s skill vnum overflow %u", GetName(), dwVnum);
        return 0;
    }

    if (m_pSkillLevels)
        return m_pSkillLevels[dwVnum].bMasterType;

    return SKILL_NORMAL;
}

int CHARACTER::GetSkillPower(uint32_t dwVnum, uint8_t bLevel) const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetSkillPower(dwVnum, bLevel);
#endif

    if (dwVnum >= SKILL_LANGUAGE1 && dwVnum <= SKILL_LANGUAGE3 &&
        IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
        return 100;

    if (dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END) {
        if (GetGuild())
            return 100 * GetGuild()->GetSkillLevel(dwVnum) / 7 / 7;
        else
            return 0;
    }

    if (bLevel)
        return GetSkillPowerByLevel(bLevel);

    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("%s skill vnum overflow %d", GetName(), dwVnum);
        return 0;
    }

    return GetSkillPowerByLevel(GetSkillLevel(dwVnum));
}

int CHARACTER::GetSkillLevel(uint32_t dwVnum) const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetSkillLevel(dwVnum);
#endif

    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("%s skill vnum overflow %u", GetName(), dwVnum);
        SPDLOG_INFO("%s skill vnum overflow %u", GetName(), dwVnum);
        return 0;
    }

    return std::min<int>(SKILL_MAX_LEVEL,
                         m_pSkillLevels ? m_pSkillLevels[dwVnum].bLevel : 0);
}

EVENTFUNC(skill_muyoung_event)
{
    char_event_info* info = static_cast<char_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("skill_muyoung_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER* ch = info->ch;

    if (ch == nullptr) {
        // <Factor>
        return 0;
    }

    if (!ch->FindAffect(SKILL_MUYEONG)) {
        ch->StopMuyeongEvent();
        return 0;
    }

    // 1. Find Victim
    FFindNearVictim f(ch, ch);
    if (ch->GetSectree()) {
        ch->ForEachSeen(f);
        // 2. Shoot!
        if (f.GetVictim()) {
            ch->CreateFly(FLY_SKILL_MUYEONG, f.GetVictim());
            ch->ComputeSkill(SKILL_MUYEONG, f.GetVictim());
        }
    }

    return THECORE_SECS_TO_PASSES(3);
}

void CHARACTER::StartMuyeongEvent()
{
    if (m_pkMuyeongEvent)
        return;

    char_event_info* info = AllocEventInfo<char_event_info>();

    info->ch = this;
    m_pkMuyeongEvent =
        event_create(skill_muyoung_event, info, THECORE_SECS_TO_PASSES(1));
}

void CHARACTER::StopMuyeongEvent()
{
    event_cancel(&m_pkMuyeongEvent);
}

void CHARACTER::SkillLearnWaitMoreTimeMessage(uint32_t ms)
{
    // const char* str = "";
    //
    if (ms < 3 * 60)
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "몸 속이 뜨겁군. 하지만 아주 편안해. 이대로 기를 "
                           "안정시키자.");
    else if (ms < 5 * 60)
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "그래, 천천히. 좀더 천천히, 그러나 막힘 없이 "
                           "빠르게!");
    else if (ms < 10 * 60) // 10분
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "그래, 이 느낌이야. 체내에 기가 아주 충만해.");
    else if (ms < 30 * 60) // 30분
    {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "다 읽었다! 이제 비급에 적혀있는 대로 전신에 기를 "
                           "돌리기만 하면,");
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "그것으로 수련은 끝난 거야!");
    } else if (ms < 1 * 3600) // 1시간
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "이제 책의 마지막 장이야! 수련의 끝이 눈에 보이고 "
                           "있어!");
    else if (ms < 2 * 3600) // 2시간
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "얼마 안 남았어! 조금만 더!");
    else if (ms < 3 * 3600)
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "좋았어! 조금만 더 읽으면 끝이다!");
    else if (ms < 6 * 3600) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "책장도 이제 얼마 남지 않았군.");
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           "뭔가 몸 안에 힘이 생기는 기분인 걸.");
    } else if (ms < 12 * 3600) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           ("이제 좀 슬슬 가닥이 잡히는 것 같은데."));
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           ("좋아, 이 기세로 계속 나간다!"));
    } else if (ms < 18 * 3600) {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           ("아니 어떻게 된 게 종일 읽어도 머리에 안 "
                            "들어오냐."));
        SendI18nChatPacket(this, CHAT_TYPE_TALKING, ("공부하기 싫어지네."));
    } else // if (ms < 2 * 86400)
    {
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           ("생각만큼 읽기가 쉽지가 않군. 이해도 어렵고 내용도 "
                            "난해해."));
        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                           ("이래서야 공부가 안된다구."));
    }
    /*
       str = "30%";
       else if (ms < 3 * 86400)
       str = "10%";
       else if (ms < 4 * 86400)
       str = "5%";
       else
       str = "0%";*/

    // ChatPacket(CHAT_TYPE_TALKING, "%s", str);
}

void CHARACTER::DisableCooltime()
{
    m_bDisableCooltime = true;
}

bool CHARACTER::HasMobSkill() const
{
    if (!m_pkMobData)
        return false;

#ifndef INSTINCT_BUILD
    if (IsPet())
        return false;
#endif

    for (size_t i = 0; i < MOB_SKILL_MAX_NUM; ++i)
        if (m_pkMobData->Skills[i].dwVnum)
            return true;

    return false;
}

size_t CHARACTER::CountMobSkill() const
{
    if (!m_pkMobData)
        return 0;

    size_t c = 0;

    for (size_t i = 0; i < MOB_SKILL_MAX_NUM; ++i)
        if (m_pkMobData->Skills[i].dwVnum)
            ++c;

    return c;
}

const TMobSkillLevel* CHARACTER::GetMobSkill(unsigned int idx) const
{
    if (idx >= MOB_SKILL_MAX_NUM)
        return nullptr;

    if (!m_pkMobData)
        return nullptr;

    if (0 == m_pkMobData->Skills[idx].dwVnum)
        return nullptr;

    return &m_pkMobData->Skills[idx];
}

bool CHARACTER::CanUseMobSkill(unsigned int idx) const
{
    const auto* pInfo = GetMobSkill(idx);

    if (!pInfo)
        return false;

    if (IsPet())
        return false;

    if (m_adwMobSkillCooltime[idx] > get_dword_time())
        return false;

    if (Random::get(0, 1))
        return false;

    return true;
}

EVENTINFO(mob_skill_event_info)
{
    DynamicCharacterPtr ch;
    PIXEL_POSITION pos;
    uint32_t vnum;
    int index;
    uint8_t level;

    mob_skill_event_info()
        : ch()
        , pos()
        , vnum(0)
        , index(0)
        , level(0)
    {
    }
};

EVENTFUNC(mob_skill_hit_event)
{
    mob_skill_event_info* info =
        static_cast<mob_skill_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("mob_skill_event_info> <Factor> Null pointer");
        return 0;
    }

    // <Factor>
    CHARACTER* ch = info->ch;
    if (ch == nullptr) {
        return 0;
    }

    // CHAOS DUNGEON
    CSkillProto* pkSk = CSkillManager::instance().Get(info->vnum);

    if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && !ch->GetParty()) {
        ch->ComputeSkill(info->vnum, ch, info->level);
        ch->EffectPacket(SE_HEAL);
    } else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && ch->GetParty()) {
        FPartyMOBCollector f;
        ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());
        for (std::vector<uint32_t>::iterator it = f.vecPIDs.begin();
             it != f.vecPIDs.end(); it++) {
            CHARACTER* ch = g_pCharManager->Find(*it);
            if (ch) {
                ch->ComputeSkill(info->vnum, ch, info->level);
                ch->EffectPacket(SE_HEAL);
            }
        }
    } else {
        ch->ComputeSkillAtPosition(info->vnum, info->pos, info->level);
    }
    ch->m_mapMobSkillEvent.erase(info->index);

    return 0;
}

bool CHARACTER::UseMobSkill(unsigned int idx)
{
    if (IsPC())
        return false;

    const auto* pInfo = GetMobSkill(idx);
    if (!pInfo)
        return false;

    CSkillProto* pkSk = CSkillManager::instance().Get(pInfo->dwVnum);
    if (!pkSk)
        return false;

    const float k =
        1.0 * GetSkillPower(pkSk->dwVnum, pInfo->bLevel) * 1.0 / 100.0;

    pkSk->kCooldownPoly.SetVar("k", k);
    PointValue iCooltime = (pkSk->kCooldownPoly.Eval() * 1000);

    m_adwMobSkillCooltime[idx] = get_dword_time() + ceil(iCooltime);

    SPDLOG_INFO("USE_MOB_SKILL: {} idx {} vnum {} cooltime {}", GetName(), idx,
                pInfo->dwVnum, iCooltime);

    const auto key = MakeMotionKey(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + idx);
    const auto* motion = GetMotionManager().Get(GetRaceNum(), key);

    if (!motion || motion->events.empty()) {
        SPDLOG_ERROR("No skill hit data for mob {} index {}", GetName(), idx);
        return false;
    }

    for (auto i = 0; i < motion->events.size(); i++) {
        PIXEL_POSITION pos = GetXYZ();
        const auto& ev = motion->events[i];

        if (ev.position.y()) {
            float fx, fy;
            GetDeltaByDegree(GetRotation(), -ev.position.y(), &fx, &fy);
            pos.x += static_cast<uint32_t>(fx);
            pos.y += static_cast<uint32_t>(fy);
        }

        if (ev.startTime) {
            mob_skill_event_info* info = AllocEventInfo<mob_skill_event_info>();

            info->ch = this;
            info->pos = pos;
            info->level = pInfo->bLevel;
            info->vnum = pInfo->dwVnum;
            info->index = i;

            // <Factor> Cancel existing event first
            auto it = m_mapMobSkillEvent.find(i);
            if (it != m_mapMobSkillEvent.end()) {
                LPEVENT existing = it->second;
                event_cancel(&existing);
                m_mapMobSkillEvent.erase(it);
            }

            const auto time = static_cast<uint32_t>(100 + ev.startTime);
            auto newEv = event_create(mob_skill_hit_event, info,
                                      THECORE_MSECS_TO_PASSES(time));

            m_mapMobSkillEvent.emplace(i, newEv);
        } else {
            ComputeSkillAtPosition(pInfo->dwVnum, pos, pInfo->bLevel);
        }
    }

    return true;
}

void CHARACTER::ResetMobSkillCooltime()
{
    memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
}

void CHARACTER::ClearSkill()
{
    PointChange(POINT_SKILL, 4 + (GetLevel() - 5) - GetPoint(POINT_SKILL));

    ResetSkill();
}

void CHARACTER::ClearSubSkill()
{
    PointChange(POINT_SUB_SKILL,
                GetLevel() < 10 ? 0
                                : (GetLevel() - 9) - GetPoint(POINT_SUB_SKILL));

    if (m_pSkillLevels == nullptr) {
        SPDLOG_ERROR("m_pSkillLevels nil (name: {})", GetName());
        return;
    }

    TPlayerSkill CleanSkill = {};

    size_t count = sizeof(s_adwSubSkillVnums) / sizeof(s_adwSubSkillVnums[0]);

    for (size_t i = 0; i < count; ++i) {
        if (s_adwSubSkillVnums[i] >= SKILL_MAX_NUM)
            continue;

        m_pSkillLevels[s_adwSubSkillVnums[i]] = CleanSkill;
    }

    ComputePoints();
    ComputeMountPoints();
    SkillLevelPacket();
}

bool CHARACTER::ResetOneSkill(uint32_t dwVnum)
{
    if (nullptr == m_pSkillLevels) {
        SPDLOG_ERROR("{}: m_pSkillLevels nil (vnum {})", GetName(), dwVnum);
        return false;
    }

    if (dwVnum >= SKILL_MAX_NUM) {
        SPDLOG_ERROR("{}: Skill vnum overflow (vnum {})", GetName(), dwVnum);
        return false;
    }

    uint8_t level = m_pSkillLevels[dwVnum].bLevel;

    m_pSkillLevels[dwVnum].bLevel = 0;
    m_pSkillLevels[dwVnum].bMasterType = 0;
    m_pSkillLevels[dwVnum].tNextRead = 0;

    if (level > 17)
        level = 17;

    PointChange(POINT_SKILL, level);

    LogManager::instance().CharLog(this, dwVnum, "ONE_SKILL_RESET_BY_SCROLL",
                                   "");

    ComputePoints();
    ComputeMountPoints();
    SkillLevelPacket();

    return true;
}

bool CHARACTER::CanUseSkill(uint32_t dwSkillVnum) const
{
    if (0 == dwSkillVnum)
        return false;

    if (0 < GetSkillGroup()) {
        const int SKILL_COUNT = 6;
        static const uint32_t SkillList[JOB_MAX_NUM][2][SKILL_COUNT] = {
            {{1, 2, 3, 4, 5, 6}, {16, 17, 18, 19, 20, 21}},
            {{31, 32, 33, 34, 35, 36}, {46, 47, 48, 49, 50, 51}},
            {{61, 62, 63, 64, 65, 66}, {76, 77, 78, 79, 80, 81}},
            {{91, 92, 93, 94, 95, 96}, {106, 107, 108, 109, 110, 111}},
            {{170, 171, 172, 173, 174, 175}, {16, 17, 18, 19, 20, 21}},
        };

        const uint32_t* pSkill = SkillList[GetJob()][GetSkillGroup() - 1];

        for (int i = 0; i < SKILL_COUNT; ++i) {
            if (pSkill[i] == dwSkillVnum)
                return true;
        }
    }

    switch (dwSkillVnum) {
        case SKILL_HORSE_WILDATTACK:
        case SKILL_HORSE_CHARGE:
        case SKILL_HORSE_ESCAPE:
        case SKILL_HORSE_WILDATTACK_RANGE: {
            if (gConfig.IsBlockedHorseSkillMap(GetMapIndex()))
                return false;

            return true;
        }
    }

    switch (dwSkillVnum) {
        case 121:
        case 122:
        case 124:
        case 126:
        case 127:
        case 128:
        case 129:
        case 130:
        case 131:
        case 151:
        case 152:
        case 153:
        case 154:
        case 155:
        case 156:
        case 157:
        case 158:
        case 159:
            return true;
    }

    // SKILLTREE
    if (dwSkillVnum >= 180 && dwSkillVnum <= 202)
        return true;

    return false;
}

bool CHARACTER::CheckSkillHitCount(uint8_t SkillID, uint32_t TargetVID)
{
    auto iter = m_SkillUseInfo.find(SkillID);
    if (iter == m_SkillUseInfo.end()) {
        SPDLOG_INFO("SkillHack: Skill({0}) is not in container", SkillID);
        return false;
    }

    auto& rSkillUseInfo = iter->second;
    if (!rSkillUseInfo.bUsed) {
        SPDLOG_INFO("SkillHack: not used skill({0})", SkillID);
        return false;
    }

    switch (SkillID) {
        case SKILL_YONGKWON:
        case SKILL_HWAYEOMPOK:
        case SKILL_DAEJINGAK:
        case SKILL_PAERYONG:
            SPDLOG_INFO("SkillHack: cannot use attack packet for skill({})",
                        SkillID);
            return false;
    }

    uint32_t MaxAttackCountPerTarget = 1;

    switch (SkillID) {
        case SKILL_SAMYEON:
        case SKILL_CHARYUN:
        case SKILL_CHAYEOL:
            MaxAttackCountPerTarget = 3;
            break;

        case SKILL_HORSE_WILDATTACK_RANGE:
            MaxAttackCountPerTarget = 5;
            break;

        case SKILL_YEONSA:
            MaxAttackCountPerTarget = 7;
            break;

        case SKILL_HORSE_ESCAPE:
            MaxAttackCountPerTarget = 10;
            break;
    }

    const auto count = rSkillUseInfo.hitLog.Hit(TargetVID);
    if (count >= MaxAttackCountPerTarget) {
        SPDLOG_INFO("Too Many hits from SkillID({0}) count({1})", SkillID,
                    count);
        return false;
    }

    return true;
}
