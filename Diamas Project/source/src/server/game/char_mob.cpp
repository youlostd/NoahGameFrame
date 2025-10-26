#include "char.h"
#include "item.h"
#include "mining.h"
#include "mob_manager.h"
#include "party.h"
#include "vector.h"
#include "war_map.h"

bool CHARACTER::IsBerserk() const
{
    if (m_pkMobInst != nullptr)
        return m_pkMobInst->m_IsBerserk;

    return false;
}

bool CHARACTER::SetMobPosLastAttacked(PIXEL_POSITION &pos)
{
    if (m_pkMobInst != nullptr)
    {
        m_pkMobInst->m_posLastAttacked = pos;
        return true;
    }

    return false;
}

void CHARACTER::SetBerserk(bool mode)
{
    if (m_pkMobInst != nullptr)
        m_pkMobInst->m_IsBerserk = mode;
}

bool CHARACTER::IsGodSpeed() const
{
    if (m_pkMobInst != nullptr) { return m_pkMobInst->m_IsGodSpeed; }
    else { return false; }
}

void CHARACTER::SetGodSpeed(bool mode)
{
    if (m_pkMobInst != nullptr)
    {
        m_pkMobInst->m_IsGodSpeed = mode;

        if (mode == true) { SetPoint(POINT_ATT_SPEED, 250); }
        else { SetPoint(POINT_ATT_SPEED, m_pkMobData->sAttackSpeed); }
    }
}

bool CHARACTER::IsDeathBlow() const
{
    if (GetHPPct() > 50) //Only deathblow when low on health
        return false;

    return (Random::get(1, 100) <= m_pkMobData->bDeathBlowPoint);
}

bool CHARACTER::IsRevive() const
{
    if (m_pkMobInst != nullptr) { return m_pkMobInst->m_IsRevive; }

    return false;
}

void CHARACTER::SetRevive(bool mode) { if (m_pkMobInst != nullptr) { m_pkMobInst->m_IsRevive = mode; } }

bool CHARACTER::IsAggressive() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE); }

void CHARACTER::SetAggressive()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE);
}

bool CHARACTER::IsCoward() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_COWARD); }

void CHARACTER::SetCoward()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_COWARD);
}

bool CHARACTER::IsBerserker() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_BERSERK); }

bool CHARACTER::IsStoneSkinner() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_STONESKIN); }

bool CHARACTER::IsGodSpeeder() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_GODSPEED); }

bool CHARACTER::IsDeathBlower() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_DEATHBLOW); }

bool CHARACTER::IsReviver() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_REVIVE); }

void CHARACTER::CowardEscape()
{
    int iDist[4] = {500, 1000, 3000, 5000};

    for (int iDistIdx = 2; iDistIdx >= 0; --iDistIdx)
        for (int iTryCount = 0; iTryCount < 8; ++iTryCount)
        {
            SetRotation((float)Random::get(0, 359)); // 방향은 랜덤으로 설정

            float fx, fy;
            float fDist = (float)Random::get(iDist[iDistIdx], iDist[iDistIdx + 1]);

            GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

            bool bIsWayBlocked = false;
            for (int j = 1; j <= 100; ++j)
            {
                if (!SECTREE_MANAGER::instance().IsMovablePosition(GetMapIndex(), GetX() + (int)fx * j / 100,
                                                                   GetY() + (int)fy * j / 100))
                {
                    bIsWayBlocked = true;
                    break;
                }
            }

            if (bIsWayBlocked)
                continue;

            m_dwStateDuration = THECORE_SECS_TO_PASSES(1);

            int iDestX = GetX() + (int)fx;
            int iDestY = GetY() + (int)fy;

            Goto(iDestX, iDestY);

            SPDLOG_INFO("WAEGU move to {0} {1} (far)", iDestX, iDestY);
            return;
        }
}

void CHARACTER::SetNoAttackShinsu()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU);
}

bool CHARACTER::IsNoAttackShinsu() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU); }

void CHARACTER::SetNoAttackChunjo()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO);
}

bool CHARACTER::IsNoAttackChunjo() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO); }

void CHARACTER::SetNoAttackJinno()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO);
}

bool CHARACTER::IsNoAttackJinno() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO); }

void CHARACTER::SetAttackMob()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

bool CHARACTER::IsAttackMob() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB); }

void CHARACTER::SetPetPawn()
{
    SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

bool CHARACTER::IsPetPawn() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB); }

const TMobTable &CHARACTER::GetMobTable() const { return *m_pkMobData; }

bool CHARACTER::IsRaceFlag(uint32_t dwBit) const
{
    return m_pkMobData ? IS_SET(m_pkMobData->dwRaceFlag, dwBit) : false;
}

uint32_t CHARACTER::GetMobDamageMin() const { return m_pkMobData->dwDamageRange[0]; }

uint32_t CHARACTER::GetMobDamageMax() const { return m_pkMobData->dwDamageRange[1]; }

float CHARACTER::GetMobDamageMultiply() const
{
    float fDamMultiply = GetMobTable().fDamMultiply;

    if (IsBerserk())
        fDamMultiply = fDamMultiply * 2.0f; // BALANCE: 광폭화 시 두배

    return fDamMultiply;
}

uint32_t CHARACTER::GetMobDropItemVnum() const
{
    if (m_pkMobData == nullptr)
        return 0;

    return m_pkMobData->dwDropItemVnum;
}

bool CHARACTER::IsSummonMonster() const { return GetSummonVnum() != 0; }

bool CHARACTER::CanSummonMonster()
{
    if (!IsSummonMonster())
        return false;

    if (!GetParty())
        CPartyManager::instance().CreateParty(this);

    const auto leader = GetParty()->GetLeaderCharacter();
    if (leader && leader->IsPet()) { return false; }

    return true;
}

uint32_t CHARACTER::GetSummonVnum() const { return m_pkMobData ? m_pkMobData->dwSummonVnum : 0; }

uint32_t CHARACTER::GetPolymorphItemVnum() const { return m_pkMobData ? m_pkMobData->dwPolymorphItemVnum : 0; }

uint32_t CHARACTER::GetMonsterDrainSPPoint() const { return m_pkMobData ? m_pkMobData->dwDrainSP : 0; }

uint8_t CHARACTER::GetMobRank() const
{
    if (!m_pkMobData)
        return MOB_RANK_KNIGHT; // PC일 경우 KNIGHT급

    return m_pkMobData->bRank;
}

uint16_t CHARACTER::GetMobScale() const
{
    if (!m_pkMobData)
        return 100;

    return m_pkMobData->wScale;
}

uint32_t CHARACTER::GetMobVnum() const
{
    if (!m_pkMobData)
        return 0;
    return m_pkMobData->dwVnum;
}

uint16_t CHARACTER::GetMobAttackRange() const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
    {
        CItem *pkWeapon = GetWear(WEAR_WEAPON);
        if (pkWeapon && pkWeapon->GetItemType() == WEAPON_BOW)
            return 600;
        else
            return 250;
    }
#endif

    if (GetForceMonsterAttackRange() != 0)
        return GetForceMonsterAttackRange();

    switch (GetMobBattleType())
    {
    case BATTLE_TYPE_RANGE:
    case BATTLE_TYPE_MAGIC:
        return m_pkMobData->wAttackRange + GetPoint(POINT_BOW_DISTANCE);
    default:
        return m_pkMobData->wAttackRange;
    }
}

uint8_t CHARACTER::GetMobBattleType() const
{
    if (!m_pkMobData)
        return BATTLE_TYPE_MELEE;

    return (m_pkMobData->bBattleType);
}

void CHARACTER::SetProto(const TMobTable *pkMob)
{
    m_pkMobData = pkMob;

    m_pkMobInst.reset(new CMobInstance);

    m_bPKMode = PK_MODE_FREE;

	if(!pkMob)
        return;

	const auto *t = m_pkMobData;

    m_bCharType = static_cast<ECharType>(t->bType);

    SetLevel(t->bLevel);
    SetEmpire(t->bEmpire);

    SetExp(t->dwExp);
    m_points.st = t->bStr;
    m_points.dx = t->bDex;
    m_points.ht = t->bCon;
    m_points.iq = t->bInt;

    ComputePoints();

    SetHP(GetMaxHP());
    SetSP(GetMaxSP());

    ////////////////////
    m_pointsInstant.dwAIFlag = t->dwAIFlag;
    SetImmuneFlag(t->dwImmuneFlag);

	SetScale(t->wScale);

    AssignTriggers(t);

    ApplyMobAttribute(t);

    if (IsStone()) { DetermineDropMetinStone(); }

    if (IsWarp() || IsGoto()) { StartWarpNPCEvent(); }

    g_pCharManager->RegisterRaceNumMap(this);

    // CHAOS DUNGEON
    if (GetRaceNum() == 20415)
    {
        //Portal for Dawn map
        m_dwPlayStartTime = get_dword_time() + 120 * 1000;
    }

    // XXX CTF GuildWar hardcoding
    if (warmap::IsWarFlag(GetRaceNum()))
    {
        m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
        m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
        m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
    }

    if (warmap::IsWarFlagBase(GetRaceNum()))
    {
        m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
        m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
        m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
    }

    if (m_bCharType == CHAR_TYPE_HORSE ||
        GetRaceNum() == 20101 ||
        GetRaceNum() == 20102 ||
        GetRaceNum() == 20103 ||
        GetRaceNum() == 20104 ||
        GetRaceNum() == 20105 ||
        GetRaceNum() == 20106 ||
        GetRaceNum() == 20107 ||
        GetRaceNum() == 20108 ||
        GetRaceNum() == 20109
    ) { SetStateHorse(); }

    if (mining::IsVeinOfOre(GetRaceNum()) || m_pkMobData->despawnSeconds)
    {
        SPDLOG_ERROR("DESPAWNSECONDS {0}\t{1}", m_pkMobData->dwVnum, m_pkMobData->despawnSeconds);
        uint32_t timeOut = m_pkMobData->despawnSeconds;
        if (mining::IsVeinOfOre(GetRaceNum())) { timeOut = Random::get(7 * 60, 15 * 60); }
        auto *info = AllocEventInfo<char_event_info>();

        info->ch = this;

        m_despawnEvent = event_create(despawn_event, info, THECORE_SECS_TO_PASSES(timeOut));
    }

    if (IsGoto() || IsWarp()) { SetName(GetProtoName()); }
}

void CHARACTER::SetLastAttacked(uint32_t dwTime)
{
    assert(m_pkMobInst != NULL);

    m_pkMobInst->m_dwLastAttackedTime = dwTime;
    m_pkMobInst->m_posLastAttacked = GetXYZ();
}

void CHARACTER::ApplyMobAttribute(const TMobTable *table)
{
    for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
    {
        if (table->cEnchants[i] != 0)
            ApplyPoint(aiMobEnchantApplyIdx[i], table->cEnchants[i]);
    }

    for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
    {
        if (table->cResists[i] != 0)
            ApplyPoint(aiMobResistsApplyIdx[i], table->cResists[i]);
    }
}
