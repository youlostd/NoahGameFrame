#include "pvp.h"
#include <game/GamePacket.hpp>
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "ChatUtil.hpp"
#include "config.h"
#include "sectree_manager.h"
#include "item.h"

#include "locale.hpp"
#include <base/Crc32.hpp>


CPVP::CPVP(uint32_t dwPID1, uint32_t dwPID2)
{
    if (dwPID1 > dwPID2)
    {
        m_players[0].dwPID = dwPID1;
        m_players[1].dwPID = dwPID2;
        m_players[0].bAgree = true;
    }
    else
    {
        m_players[0].dwPID = dwPID2;
        m_players[1].dwPID = dwPID1;
        m_players[1].bAgree = true;
    }

    uint32_t adwID[2];
    adwID[0] = m_players[0].dwPID;
    adwID[1] = m_players[1].dwPID;
    m_dwCRC = ComputeCrc32(0, &adwID, 8);
    m_bRevenge = false;

    SetLastFightTime();
}

CPVP::CPVP(CPVP &k)
{
    m_players[0] = k.m_players[0];
    m_players[1] = k.m_players[1];

    m_dwCRC = k.m_dwCRC;
    m_bRevenge = k.m_bRevenge;

    SetLastFightTime();
}

CPVP::~CPVP()
{
}

void CPVP::Packet(bool bDelete)
{
    if (!m_players[0].dwVID || !m_players[1].dwVID)
    {
        if (bDelete)
            SPDLOG_ERROR("PvP: Null vid when removing {} {}", m_players[0].dwVID, m_players[0].dwVID);

        return;
    }

    TPacketGCPVP p;

    if (bDelete)
    {
        p.bMode = PVP_MODE_NONE;
        p.dwVIDSrc = m_players[0].dwVID;
        p.dwVIDDst = m_players[1].dwVID;
    }
    else if (IsFight())
    {
        p.bMode = PVP_MODE_FIGHT;
        p.dwVIDSrc = m_players[0].dwVID;
        p.dwVIDDst = m_players[1].dwVID;
    }
    else
    {
        p.bMode = m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

        if (m_players[0].bAgree)
        {
            p.dwVIDSrc = m_players[0].dwVID;
            p.dwVIDDst = m_players[1].dwVID;
        }
        else
        {
            p.dwVIDSrc = m_players[1].dwVID;
            p.dwVIDDst = m_players[0].dwVID;
        }
    }

    BroadcastPacket(DESC_MANAGER::instance().GetClientSet(), HEADER_GC_PVP, p);
}

bool CPVP::Agree(uint32_t dwPID)
{
    m_players[m_players[0].dwPID != dwPID ? 1 : 0].bAgree = true;

    if (IsFight())
    {
        Packet();
        return true;
    }

    return false;
}

bool CPVP::IsFight() { return (m_players[0].bAgree == m_players[1].bAgree) && m_players[0].bAgree; }

void CPVP::Win(uint32_t dwPID)
{
    int iSlot = m_players[0].dwPID != dwPID ? 1 : 0;

    m_bRevenge = true;

    m_players[iSlot].bAgree = true; // 자동으로 동의
    m_players[!iSlot].bCanRevenge = true;
    m_players[!iSlot].bAgree = false;

    Packet();

    auto* winner = g_pCharManager->Find( m_players[iSlot].dwVID);
    auto* looser = g_pCharManager->Find( m_players[!iSlot].dwVID);
    if(!winner || !looser)
        return;

    const Locale *locale;
    if (looser)
        locale = GetLocale(looser);
    else
        locale = &GetLocaleService().GetDefaultLocale();

    SendI18nChatPacket(looser, CHAT_TYPE_INFO, "<Düello Sonucu> %s kazandı! Seni öldürdüğündeki Kalan Canı: %d / Toplam Canı: %d ", winner->GetName(), winner->GetHP(), winner->GetMaxHP());
}

bool CPVP::CanRevenge(uint32_t dwPID) { return m_players[m_players[0].dwPID != dwPID ? 1 : 0].bCanRevenge; }

void CPVP::SetVID(uint32_t dwPID, uint32_t dwVID)
{
    if (m_players[0].dwPID == dwPID)
        m_players[0].dwVID = dwVID;
    else
        m_players[1].dwVID = dwVID;
}

CPVPManager::CPVPManager()
{
}

CPVPManager::~CPVPManager()
{
}

void CPVPManager::Insert(CHARACTER *pkChr, CHARACTER *pkVictim)
{
    if (pkChr->IsDead() || pkVictim->IsDead())
        return;

    CPVP kPVP(pkChr->GetPlayerID(), pkVictim->GetPlayerID());


    if (auto* pvp = Find(kPVP.m_dwCRC); pvp)
    {
        // 복수할 수 있으면 바로 싸움!
        if (pvp->Agree(pkChr->GetPlayerID()))
        {
            SendI18nChatPacket(pkVictim, CHAT_TYPE_INFO, "%s님과의 대결 시작!", pkChr->GetName());
            SendI18nChatPacket(pkChr, CHAT_TYPE_INFO, "%s님과의 대결 시작!", pkVictim->GetName());
        }
        return;
    }

    auto pkPVP = std::make_unique<CPVP>(kPVP);

    pkPVP->SetVID(pkChr->GetPlayerID(), pkChr->GetVID());
    pkPVP->SetVID(pkVictim->GetPlayerID(), pkVictim->GetVID());
    auto* rawPtr = pkPVP.get();
    m_map_pkPVP.emplace(pkPVP->m_dwCRC, std::move(pkPVP));

    m_map_pkPVPSetByID[pkChr->GetPlayerID()].insert(rawPtr);
    m_map_pkPVPSetByID[pkVictim->GetPlayerID()].insert(rawPtr);

    rawPtr->Packet();

    const Locale *locale;
    if (pkVictim)
        locale = GetLocale(pkVictim);
    else
        locale = &GetLocaleService().GetDefaultLocale();

    std::string msg = fmt::sprintf(LC_TEXT_LC("%s님이 대결신청을 했습니다. 승낙하려면 대결동의를 하세요.", locale), pkChr->GetName());

    SendI18nChatPacket(pkVictim, CHAT_TYPE_INFO, "%s님이 대결신청을 했습니다. 승낙하려면 대결동의를 하세요.", pkChr->GetName());
    SendI18nChatPacket(pkChr, CHAT_TYPE_INFO, "%s에게 대결신청을 했습니다.", pkVictim->GetName());

    // NOTIFY_PVP_MESSAGE
    DESC *pkVictimDesc = pkVictim->GetDesc();
    if (pkVictimDesc)
    {
        TPacketGCWhisper pack{};
        pack.bType = WHISPER_TYPE_SYSTEM;
        pack.szNameFrom = pkChr->GetName();
        pack.message = msg;

        pkVictimDesc->Send(HEADER_GC_WHISPER, pack);
    }
    // END_OF_NOTIFY_PVP_MESSAGE
}

void CPVPManager::ConnectEx(CHARACTER *pkChr, bool bDisconnect)
{
    auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

    if (it == m_map_pkPVPSetByID.end())
        return;

    uint32_t dwVID = bDisconnect ? 0 : pkChr->GetVID();

    auto it2 = it->second.begin();

    while (it2 != it->second.end())
    {
        CPVP *pkPVP = *it2++;
        pkPVP->SetVID(pkChr->GetPlayerID(), dwVID);
    }
}

void CPVPManager::Connect(CHARACTER *pkChr) { ConnectEx(pkChr, false); }

void CPVPManager::Disconnect(CHARACTER *pkChr)
{
    //ConnectEx(pkChr, true);
}

void CPVPManager::GiveUp(CHARACTER *pkChr, uint32_t dwKillerPID) // This method is calling from no where yet.
{
    auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

    if (it == m_map_pkPVPSetByID.end())
        return;

    SPDLOG_TRACE("PVPManager::Dead {}", pkChr->GetPlayerID());
    auto it2 = it->second.begin();

    while (it2 != it->second.end())
    {
        CPVP *pkPVP = *it2++;

        uint32_t dwCompanionPID;

        if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
            dwCompanionPID = pkPVP->m_players[1].dwPID;
        else
            dwCompanionPID = pkPVP->m_players[0].dwPID;

        if (dwCompanionPID != dwKillerPID)
            continue;

        pkPVP->SetVID(pkChr->GetPlayerID(), 0);

        m_map_pkPVPSetByID.erase(dwCompanionPID);

        it->second.erase(pkPVP);

        if (it->second.empty())
            m_map_pkPVPSetByID.erase(it);

        pkPVP->Packet(true);

        m_map_pkPVP.erase(pkPVP->m_dwCRC);
        break;
    }
}

bool CPVPManager::IsDuelingInstance(CHARACTER *pkChr)
{
    const auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

    if (it == m_map_pkPVPSetByID.end())
        return false;

    return true;
}

bool CPVPManager::Dead(CHARACTER* pkChr, uint32_t dwKillerPID,
                       CHARACTER* pkKiller)
{
    auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

    if (it == m_map_pkPVPSetByID.end())
        return false;

    bool found = false;

    SPDLOG_TRACE("PVPManager::Dead {}", pkChr->GetPlayerID());
    auto it2 = it->second.begin();

    while (it2 != it->second.end()) {
        CPVP* pkPVP = *it2++;

        uint32_t dwCompanionPID;

        if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
            dwCompanionPID = pkPVP->m_players[1].dwPID;
        else
            dwCompanionPID = pkPVP->m_players[0].dwPID;

        if (dwCompanionPID == dwKillerPID) {
            if (pkPVP->IsFight()) {
                pkPVP->SetLastFightTime();
                pkPVP->Win(dwKillerPID);
                if (pkKiller != NULL)
                    pkKiller->RemoveBadAffectAfterDuel();
                found = true;
                break;
            } else if (get_dword_time() - pkPVP->GetLastFightTime() <= 15000) {
                found = true;
                break;
            }
        }
    }

    return found;
}

bool CPVPManager::CanAttack(CHARACTER *attacker, CHARACTER *victim)
{
    switch (victim->GetCharType())
    {
    case CHAR_TYPE_NPC:
    case CHAR_TYPE_WARP:
    case CHAR_TYPE_DOOR:
    case CHAR_TYPE_BUILDING:
    case CHAR_TYPE_GOTO:
    case CHAR_TYPE_SHOP:
    case CHAR_TYPE_PET:
    case CHAR_TYPE_MOUNT:
    case CHAR_TYPE_GROWTH_PET:
    case CHAR_TYPE_BUFFBOT:
    case CHAR_TYPE_ATTACK_PET:
        return false;
    default: break;
    }

    if (attacker == victim)
        return false;

    if (victim->IsNPC() && attacker->IsNPC() && (!attacker->IsGuardNPC() && !attacker->IsPet()))
        return false;

    if (victim->IsPet())
        return false;

    if (victim->IsPetPawn())
        return false;

    if (victim->IsBuffBot())
        return false;

    if (victim->IsToggleMount())
        return false;

#if defined(WJ_COMBAT_ZONE)
	if (CCombatZoneManager::Instance().IsCombatZoneMap(attacker->GetMapIndex()) || CCombatZoneManager::Instance().IsCombatZoneMap(victim->GetMapIndex()))
		return true;
#endif

#ifdef __FAKE_PC__
    if (attacker->FakePC_Check() && victim->FakePC_Check())
    {
        if (attacker->FakePC_IsSupporter() == victim->FakePC_IsSupporter() && attacker->GetPVPTeam() == victim->
            GetPVPTeam())
            return false;
    }

    if ((attacker->FakePC_IsSupporter() && !attacker->FakePC_CanAttack()) || (
            victim->FakePC_IsSupporter() && !victim->FakePC_CanAttack()))
        return false;

    if ((((attacker->FakePC_IsSupporter() && victim->IsPC()) || (attacker->IsPC() && victim->FakePC_IsSupporter())) &&
         attacker->GetPVPTeam() == victim->GetPVPTeam()) ||
        (attacker->FakePC_Check() && !attacker->FakePC_IsSupporter() && !victim->IsPC()) ||
        (!attacker->IsPC() && victim->FakePC_Check() && !victim->FakePC_IsSupporter()))
        return false;
#endif

    if (true == attacker->IsHorseRiding())
    {
        if (attacker->GetHorseLevel() > 0 && 1 == attacker->GetHorseGrade())
            return false;
    }

    if (victim->IsNPC() || attacker->IsNPC())
        return true;

    {
        const auto bMapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(attacker->GetMapIndex());

        if ((attacker->GetPKMode() == PK_MODE_PROTECT && attacker->GetEmpire() == bMapEmpire) ||
            (victim->GetPKMode() == PK_MODE_PROTECT && victim->GetEmpire() == bMapEmpire)) { return false; }
    }

    if (gConfig.IsPvPDisabledMap(victim->GetMapIndex()))
        return false;

    if (gConfig.disablePvP)
        return false;

    if (victim->GetParty() && victim->GetParty() == attacker->GetParty())
        return false;

    CPVP kPVP(attacker->GetPlayerID(), victim->GetPlayerID());
    CPVP *pkPVP = Find(kPVP.m_dwCRC);

    if (pkPVP && pkPVP->IsFight())
    {
        pkPVP->SetLastFightTime();
        return true;
    }

    const auto mapIndex = attacker->GetMapIndex();
    if (gConfig.IsDuelOnlyMap(mapIndex))
        return false;

    if (victim->IsKillerMode())
        return true;

    if (attacker->GetEmpire() != victim->GetEmpire())
        return true;

    switch (attacker->GetPKMode())
    {
    case PK_MODE_PEACE:
    case PK_MODE_REVENGE:
        // Cannot attack same guild
        if (victim->GetGuild() && victim->GetGuild() == attacker->GetGuild())
            break;

        if (attacker->GetPKMode() == PK_MODE_REVENGE)
        {
            if (attacker->GetAlignment() < 0 && victim->GetAlignment() >= 0)
            {
                attacker->SetKillerMode(true);
                return true;
            }
            if (attacker->GetAlignment() >= 0 && victim->GetAlignment() < 0)
                return true;
        }
        break;

    case PK_MODE_GUILD:
        // Same implementation from PK_MODE_FREE except for attacking same guild
        if (!attacker->GetGuild() || (victim->GetGuild() != attacker->GetGuild()))
        {
            if (victim->GetAlignment() >= 0 || (attacker->GetAlignment() < 0 && victim->GetAlignment() < 0))
                attacker->SetKillerMode(true);

            return true;
        }
        break;

    case PK_MODE_FREE:
        if ((victim->GetAlignment() >= 0) || (attacker->GetAlignment() < 0 && victim->GetAlignment() < 0))
            attacker->SetKillerMode(true);

        return true;
    default: break;
    }

    return false;
}

CPVP *CPVPManager::Find(uint32_t dwCRC)
{
    const auto it = m_map_pkPVP.find(dwCRC);

    if (it == m_map_pkPVP.end())
        return nullptr;

    return it->second.get();
}

void CPVPManager::Delete(CPVP *pkPVP)
{
    auto it = m_map_pkPVP.find(pkPVP->m_dwCRC);

    if (it == m_map_pkPVP.end())
        return;

    m_map_pkPVPSetByID[pkPVP->m_players[0].dwPID].erase(pkPVP);
    m_map_pkPVPSetByID[pkPVP->m_players[1].dwPID].erase(pkPVP);
    m_map_pkPVP.erase(it);
}

void CPVPManager::SendList(DESC *d)
{
    auto it = m_map_pkPVP.begin();

    uint32_t dwVID = d->GetCharacter()->GetVID();

    TPacketGCPVP pack;

    while (it != m_map_pkPVP.end())
    {
        auto pkPVP = (it++)->second.get();

        if (!pkPVP->m_players[0].dwVID || !pkPVP->m_players[1].dwVID)
            continue;

        // VID가 둘다 있을 경우에만 보낸다.
        if (pkPVP->IsFight())
        {
            pack.bMode = PVP_MODE_FIGHT;
            pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
            pack.dwVIDDst = pkPVP->m_players[1].dwVID;
        }
        else
        {
            pack.bMode = pkPVP->m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

            if (pkPVP->m_players[0].bAgree)
            {
                pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
                pack.dwVIDDst = pkPVP->m_players[1].dwVID;
            }
            else
            {
                pack.dwVIDSrc = pkPVP->m_players[1].dwVID;
                pack.dwVIDDst = pkPVP->m_players[0].dwVID;
            }
        }

        d->Send(HEADER_GC_PVP, pack);

        SPDLOG_TRACE("PVPManager::SendList {} {}", pack.dwVIDSrc, pack.dwVIDDst);

        if (pkPVP->m_players[0].dwVID == dwVID)
        {
            auto ch = g_pCharManager->Find(pkPVP->m_players[1].dwVID);
            if (ch && ch->GetDesc())
            {
                auto* d0 = ch->GetDesc();
                d0->Send(HEADER_GC_PVP, pack);
            }
        }
        else if (pkPVP->m_players[1].dwVID == dwVID)
        {
            const auto* ch = g_pCharManager->Find(pkPVP->m_players[0].dwVID);
            if (ch && ch->GetDesc())
            {
                auto* d1 = ch->GetDesc();
                d1->Send(HEADER_GC_PVP, pack);
            }
        }
    }
}

void CPVPManager::Process()
{
    rmt_ScopedCPUSample(pvpManager, 0);

    auto it = m_map_pkPVP.begin();

    while (it != m_map_pkPVP.end())
    {
        auto pvp = (it++)->second.get();

        if (get_dword_time() - pvp->GetLastFightTime() > 600000) // 10분 이상 싸움이 없었으면
        {
            pvp->Packet(true);
            Delete(pvp);
        }
    }
}

std::unordered_set<CPVP *> CPVPManager::GetMyDuels(CHARACTER *pkChr)
{
    std::unordered_set<CPVP *> set;

    const auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());
    if (it != m_map_pkPVPSetByID.end())
        set = it->second;

    return set;
}
