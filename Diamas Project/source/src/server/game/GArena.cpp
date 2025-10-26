#include <fstream>
#include "constants.h"
#include "config.h"
#include <game/GamePacket.hpp>
#include "desc.h"
#include "GBufferManager.h"
#include "start_position.h"
#include "questmanager.h"
#include "char.h"
#include "char_manager.h"
#include "GArena.h"
#include "main.h"

#include <absl/strings/str_split.h>
#include <absl/strings/str_join.h>
#include "war_map.h"

CArena::CArena(uint32_t map, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
    : m_mapIndex(map)
{
    m_StartPointA.x = startA_X;
    m_StartPointA.y = startA_Y;
    m_StartPointA.z = 0;

    m_StartPointB.x = startB_X;
    m_StartPointB.y = startB_Y;
    m_StartPointB.z = 0;

    m_ObserverPoint.x = (startA_X + startB_X) / 2;
    m_ObserverPoint.y = (startA_Y + startB_Y) / 2;
    m_ObserverPoint.z = 0;

    m_pEvent = nullptr;
    m_pTimeOutEvent = nullptr;

    Clear();
}

bool CArenaManager::Initialize()
{
    std::ifstream inf("data/arena_forbidden_items.txt");

    if (!inf.is_open())
    {
        SPDLOG_ERROR("ArenaManager cannot open 'arena_forbidden_items.txt'.");
        return false;
    }

    std::string ln;
    while (getline(inf, ln))
    {
        if (ln.empty())
            continue;

        if (absl::StrContains(ln, "~"))
        {
            std::vector<absl::string_view> v = absl::StrSplit(ln, '~'); // Can also use ","
            if (v.size() < 2)
                continue;

            uint32_t from;
            if (!absl::SimpleAtoi(v[0], &from))
                continue;

            uint32_t to;
            if (!absl::SimpleAtoi(v[1], &to))
                continue;

            if (from > to)
                to = from;

            for (int i = from; i <= to; ++i) { m_listForbidden.push_back(i); }
        }
        else
        {
            uint32_t iVnum = atoi(ln.c_str());
            m_listForbidden.push_back(iVnum);
        }
    }

    return true;
}

void CArena::Clear()
{
    m_dwPIDA = 0;
    m_dwPIDB = 0;

    if (m_pEvent != nullptr) { event_cancel(&m_pEvent); }

    if (m_pTimeOutEvent != nullptr) { event_cancel(&m_pTimeOutEvent); }

    m_dwSetCount = 0;
    m_dwSetPointOfA = 0;
    m_dwSetPointOfB = 0;
}

bool CArenaManager::AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X,
                             uint16_t startB_Y)
{
    CArenaMap *pArenaMap = nullptr;
    auto iter = m_mapArenaMap.find(mapIdx);

    if (iter == m_mapArenaMap.end())
    {
        pArenaMap = new CArenaMap;
        m_mapArenaMap.insert(std::make_pair(mapIdx, pArenaMap));
    }
    else { pArenaMap = iter->second; }

    if (pArenaMap->AddArena(mapIdx, startA_X, startA_Y, startB_X, startB_Y) == false)
    {
        SPDLOG_INFO("CArenaManager::AddArena - AddMap Error MapID: %d", mapIdx);
        return false;
    }

    return true;
}

bool CArenaMap::AddArena(uint32_t mapIdx, uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        if (!(*iter)->CheckArea(startA_X, startA_Y, startB_X, startB_Y))
        {
            SPDLOG_INFO("CArenaMap::AddArena - Same Start Position set. stA(%d, %d) stB(%d, %d)", startA_X, startA_Y,
                        startB_X, startB_Y);
            return false;
        }
    }

    m_dwMapIndex = mapIdx;

    CArena *pArena = new CArena(mapIdx, startA_X, startA_Y, startB_X, startB_Y);
    m_listArena.push_back(pArena);

    return true;
}

void CArenaManager::Destroy()
{
    auto iter = m_mapArenaMap.begin();

    for (; iter != m_mapArenaMap.end(); ++iter)
    {
        CArenaMap *pArenaMap = iter->second;
        pArenaMap->Destroy();

        delete (pArenaMap);
    }
    m_mapArenaMap.clear();
}

void CArenaMap::Destroy()
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;
        pArena->EndDuel();

        delete (pArena);
    }
    m_listArena.clear();
}

bool CArena::CheckArea(uint16_t startA_X, uint16_t startA_Y, uint16_t startB_X, uint16_t startB_Y)
{
    if (m_StartPointA.x == startA_X && m_StartPointA.y == startA_Y &&
        m_StartPointB.x == startB_X && m_StartPointB.y == startB_Y)
        return false;
    return true;
}

void CArenaManager::SendArenaMapListTo(CHARACTER *pChar)
{
    auto iter = m_mapArenaMap.begin();

    for (; iter != m_mapArenaMap.end(); ++iter)
    {
        CArenaMap *pArena = iter->second;
        pArena->SendArenaMapListTo(pChar, (iter->first));
    }
}

void CArenaMap::SendArenaMapListTo(CHARACTER *pChar, uint32_t mapIdx)
{
    if (pChar == nullptr)
        return;

    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        pChar->ChatPacket(CHAT_TYPE_INFO, "ArenaMapInfo Map: %d stA(%d, %d) stB(%d, %d)", mapIdx,
                          ((CArena *)(*iter))->GetStartPointA().x, ((CArena *)(*iter))->GetStartPointA().y,
                          ((CArena *)(*iter))->GetStartPointB().x, ((CArena *)(*iter))->GetStartPointB().y);
    }
}

bool CArenaManager::StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint,
                              int nMinute, uint32_t map)
{
    if (pCharFrom == nullptr || pCharTo == nullptr)
        return false;
    if (map)
    {
        auto it = m_mapArenaMap.find(map);
        if (it != m_mapArenaMap.end()) { return it->second->StartDuel(pCharFrom, pCharTo, nSetPoint, nMinute); }
    }
    else
    {
        for (auto iter = m_mapArenaMap.begin(); iter != m_mapArenaMap.end(); iter++)
        {
            return iter->second->StartDuel(pCharFrom, pCharTo, nSetPoint, nMinute);
        }
    }

    return false;
}

bool CArenaMap::StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint, int nMinute)
{
    for (auto it = m_listArena.begin(); it != m_listArena.end(); it++)
    {
        CArena *pArena = *it;
        if (pArena->IsEmpty()) { return pArena->StartDuel(pCharFrom, pCharTo, nSetPoint, nMinute); }
    }

    return false;
}

EVENTINFO(TArenaEventInfo)
{
    CArena *pArena;
    uint8_t state;

    TArenaEventInfo()
        : pArena(nullptr)
          , state(0)
    {
    }
};

EVENTFUNC(ready_to_start_event)
{
    if (event == nullptr)
        return 0;

    if (event->info == nullptr)
        return 0;

    TArenaEventInfo *info = static_cast<TArenaEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("ready_to_start_event> <Factor> Null pointer");
        return 0;
    }

    CArena *pArena = info->pArena;

    if (pArena == nullptr)
    {
        SPDLOG_ERROR("ARENA: Arena start event info is null.");
        return 0;
    }

    CHARACTER *chA = pArena->GetPlayerA();
    CHARACTER *chB = pArena->GetPlayerB();

    if (chA == nullptr || chB == nullptr)
    {
        SPDLOG_ERROR("ARENA: Player err in event func ready_start_event");

        if (chA != nullptr)
        {
            SendI18nChatPacket(chA, CHAT_TYPE_INFO, "대련 상대가 사라져 대련을 종료합니다.");
            SPDLOG_INFO("ARENA: Oppernent is disappered. MyPID(%d) OppPID(%d)", pArena->GetPlayerAPID(),
                        pArena->GetPlayerBPID());
        }

        if (chB != nullptr)
        {
            SendI18nChatPacket(chB, CHAT_TYPE_INFO, "대련 상대가 사라져 대련을 종료합니다.");
            SPDLOG_INFO("ARENA: Oppernent is disappered. MyPID(%d) OppPID(%d)", pArena->GetPlayerBPID(),
                        pArena->GetPlayerAPID());
        }

        pArena->SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("대련 상대가 사라져 대련을 종료합니다."));

        pArena->EndDuel();
        return 0;
    }

    switch (info->state)
    {
    case 0: {
        chA->SetArena(pArena);
        chB->SetArena(pArena);

        int count = quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count");

        if (count > 10000)
        {
            SendI18nChatPacket(chA, CHAT_TYPE_INFO, "물약 제한이 없습니다.");
            SendI18nChatPacket(chB, CHAT_TYPE_INFO, "물약 제한이 없습니다.");
        }
        else
        {
            chA->SetPotionLimit(count);
            chB->SetPotionLimit(count);

            SendI18nChatPacket(chA, CHAT_TYPE_INFO, "물약을 %d 개 까지 사용 가능합니다.", chA->GetPotionLimit());
            SendI18nChatPacket(chB, CHAT_TYPE_INFO, "물약을 %d 개 까지 사용 가능합니다.", chB->GetPotionLimit());
        }
        SendI18nChatPacket(chA, CHAT_TYPE_INFO, "10초뒤 대련이 시작됩니다.");
        SendI18nChatPacket(chB, CHAT_TYPE_INFO, "10초뒤 대련이 시작됩니다.");
        pArena->SendChatPacketToObserver(CHAT_TYPE_INFO, ("10초뒤 대련이 시작됩니다."));

        info->state++;
        return THECORE_SECS_TO_PASSES(10);
    }
    break;

    case 1: {
        SendI18nChatPacket(chA, CHAT_TYPE_INFO, "대련이 시작되었습니다.");
        SendI18nChatPacket(chB, CHAT_TYPE_INFO, "대련이 시작되었습니다.");
        pArena->SendChatPacketToObserver(CHAT_TYPE_INFO, ("대련이 시작되었습니다."));

        TPacketGCDuelStart duelStart;
        duelStart.participants.push_back((uint32_t)chB->GetVID());
        duelStart.participants.push_back((uint32_t)chA->GetVID());

        chB->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);

        return 0;
    }
    break;

    case 2: {
        pArena->EndDuel();
        return 0;
    }
    break;

    case 3: {
        chA->Show(chA->GetMapIndex(), pArena->GetStartPointA().x * 100, pArena->GetStartPointA().y * 100);
        chB->Show(chB->GetMapIndex(), pArena->GetStartPointB().x * 100, pArena->GetStartPointB().y * 100);

        chA->GetDesc()->SetPhase(PHASE_GAME);
        chA->StartRecoveryEvent();
        chA->SetPosition(POS_STANDING);
        chA->PointChange(POINT_HP, chA->GetMaxHP() - chA->GetHP());
        chA->PointChange(POINT_SP, chA->GetMaxSP() - chA->GetSP());
        chA->ViewReencode();

        chB->GetDesc()->SetPhase(PHASE_GAME);
        chB->StartRecoveryEvent();
        chB->SetPosition(POS_STANDING);
        chB->PointChange(POINT_HP, chB->GetMaxHP() - chB->GetHP());
        chB->PointChange(POINT_SP, chB->GetMaxSP() - chB->GetSP());
        chB->ViewReencode();


        uint32_t dwOppList[8]; // 최대 파티원 8명 이므로..
        TPacketGCDuelStart duelStart;
        duelStart.participants.emplace_back((uint32_t)chB->GetVID());
        chA->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);

        duelStart.participants.clear();
        duelStart.participants.emplace_back((uint32_t)chA->GetVID());
        chB->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);

        SendI18nChatPacket(chA, CHAT_TYPE_INFO, "대련이 시작되었습니다.");
        SendI18nChatPacket(chB, CHAT_TYPE_INFO, "대련이 시작되었습니다.");
        pArena->SendChatPacketToObserver(CHAT_TYPE_INFO, ("대련이 시작되었습니다."));

        pArena->ClearEvent();

        return 0;
    }
    break;

    default: {
        SendI18nChatPacket(chA, CHAT_TYPE_INFO, "대련장 문제로 인하여 대련을 종료합니다.");
        SendI18nChatPacket(chB, CHAT_TYPE_INFO, "대련장 문제로 인하여 대련을 종료합니다.");
        pArena->SendChatPacketToObserver(CHAT_TYPE_INFO, ("대련장 문제로 인하여 대련을 종료합니다."));

        SPDLOG_INFO("ARENA: Something wrong in event func. info->state(%d)", info->state);

        pArena->EndDuel();

        return 0;
    }
    }
}

EVENTFUNC(duel_time_out)
{
    if (event == nullptr)
        return 0;
    if (event->info == nullptr)
        return 0;

    TArenaEventInfo *info = static_cast<TArenaEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("duel_time_out> <Factor> Null pointer");
        return 0;
    }

    CArena *pArena = info->pArena;

    if (pArena == nullptr)
    {
        SPDLOG_ERROR("ARENA: Time out event error");
        return 0;
    }

    CHARACTER *chA = pArena->GetPlayerA();
    CHARACTER *chB = pArena->GetPlayerB();

    if (chA == nullptr || chB == nullptr)
    {
        if (chA != nullptr)
        {
            SendI18nChatPacket(chA, CHAT_TYPE_INFO, "대련 상대가 사라져 대련을 종료합니다.");
            SPDLOG_INFO("ARENA: Oppernent is disappered. MyPID(%d) OppPID(%d)", pArena->GetPlayerAPID(),
                        pArena->GetPlayerBPID());
        }

        if (chB != nullptr)
        {
            SendI18nChatPacket(chB, CHAT_TYPE_INFO, "대련 상대가 사라져 대련을 종료합니다.");
            SPDLOG_INFO("ARENA: Oppernent is disappered. MyPID(%d) OppPID(%d)", pArena->GetPlayerBPID(),
                        pArena->GetPlayerAPID());
        }

        pArena->SendChatPacketToObserver(CHAT_TYPE_INFO, ("대련 상대가 사라져 대련을 종료합니다."));

        pArena->EndDuel();
        return 0;
    }
    else
    {
        switch (info->state)
        {
        case 0:
            {
                pArena->SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("대련 시간 초과로 대련을 중단합니다."));
                pArena->SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("10초뒤 마을로 이동합니다."));

                SendI18nChatPacket(chA, CHAT_TYPE_NOTICE, "대련 시간 초과로 대련을 중단합니다.");
                SendI18nChatPacket(chA, CHAT_TYPE_NOTICE, "10초뒤 마을로 이동합니다.");

                SendI18nChatPacket(chB, CHAT_TYPE_NOTICE, "대련 시간 초과로 대련을 중단합니다.");
                SendI18nChatPacket(chB, CHAT_TYPE_NOTICE, "10초뒤 마을로 이동합니다.");

                TPacketGCDuelStart duelStart;

                chA->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);
                chA->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);

                info->state++;

                SPDLOG_INFO("ARENA: Because of time over, duel is end. PIDA(%d) vs PIDB(%d)", pArena->GetPlayerAPID(),
                            pArena->GetPlayerBPID());

                return THECORE_SECS_TO_PASSES(10);
            }


        case 1:
            pArena->EndDuel();
            break;
        }
    }

    return 0;
}

bool CArena::StartDuel(CHARACTER *pCharFrom, CHARACTER *pCharTo, int nSetPoint, int nMinute)
{
    this->m_dwPIDA = pCharFrom->GetPlayerID();
    this->m_dwPIDB = pCharTo->GetPlayerID();
    this->m_dwSetCount = nSetPoint;

    pCharFrom->WarpSet(m_mapIndex, GetStartPointA().x * 100, GetStartPointA().y * 100);
    pCharTo->WarpSet(m_mapIndex, GetStartPointB().x * 100, GetStartPointB().y * 100);

    if (m_pEvent != nullptr) { event_cancel(&m_pEvent); }

    TArenaEventInfo *info = AllocEventInfo<TArenaEventInfo>();

    info->pArena = this;
    info->state = 0;

    m_pEvent = event_create(ready_to_start_event, info, THECORE_SECS_TO_PASSES(10));

    if (m_pTimeOutEvent != nullptr) { event_cancel(&m_pTimeOutEvent); }

    info = AllocEventInfo<TArenaEventInfo>();

    info->pArena = this;
    info->state = 0;

    m_pTimeOutEvent = event_create(duel_time_out, info, THECORE_SECS_TO_PASSES(nMinute*60));

    pCharFrom->PointChange(POINT_HP, pCharFrom->GetMaxHP() - pCharFrom->GetHP());
    pCharFrom->PointChange(POINT_SP, pCharFrom->GetMaxSP() - pCharFrom->GetSP());

    pCharTo->PointChange(POINT_HP, pCharTo->GetMaxHP() - pCharTo->GetHP());
    pCharTo->PointChange(POINT_SP, pCharTo->GetMaxSP() - pCharTo->GetSP());

    SPDLOG_INFO("ARENA: Start Duel with PID_A(%d) vs PID_B(%d)", GetPlayerAPID(), GetPlayerBPID());
    return true;
}

void CArenaManager::EndAllDuel()
{
    auto iter = m_mapArenaMap.begin();

    for (; iter != m_mapArenaMap.end(); ++iter)
    {
        CArenaMap *pArenaMap = iter->second;
        if (pArenaMap != nullptr)
            pArenaMap->EndAllDuel();
    }

    return;
}

void CArenaMap::EndAllDuel()
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;
        if (pArena != nullptr)
            pArena->EndDuel();
    }
}

void CArena::EndDuel()
{
    if (m_pEvent != nullptr) { event_cancel(&m_pEvent); }
    if (m_pTimeOutEvent != nullptr) { event_cancel(&m_pTimeOutEvent); }

    CHARACTER *playerA = GetPlayerA();
    CHARACTER *playerB = GetPlayerB();

    if (playerA != nullptr)
    {
        playerA->SetPKMode(PK_MODE_PEACE);
        playerA->StartRecoveryEvent();
        playerA->SetPosition(POS_STANDING);
        playerA->PointChange(POINT_HP, playerA->GetMaxHP() - playerA->GetHP());
        playerA->PointChange(POINT_SP, playerA->GetMaxSP() - playerA->GetSP());

        playerA->SetArena(nullptr);

        playerA->WarpSet(ARENA_RETURN_POINT_MAP(playerA->GetEmpire()), ARENA_RETURN_POINT_X(playerA->GetEmpire()),
                         ARENA_RETURN_POINT_Y(playerA->GetEmpire()));
    }

    if (playerB != nullptr)
    {
        playerB->SetPKMode(PK_MODE_PEACE);
        playerB->StartRecoveryEvent();
        playerB->SetPosition(POS_STANDING);
        playerB->PointChange(POINT_HP, playerB->GetMaxHP() - playerB->GetHP());
        playerB->PointChange(POINT_SP, playerB->GetMaxSP() - playerB->GetSP());

        playerB->SetArena(nullptr);

        playerB->WarpSet(ARENA_RETURN_POINT_MAP(playerB->GetEmpire()), ARENA_RETURN_POINT_X(playerB->GetEmpire()),
                         ARENA_RETURN_POINT_Y(playerB->GetEmpire()));
    }

    auto iter = m_mapObserver.begin();

    for (; iter != m_mapObserver.end(); ++iter)
    {
        CHARACTER *pChar = g_pCharManager->FindByPID(iter->first);
        if (pChar != nullptr)
        {
            pChar->WarpSet(ARENA_RETURN_POINT_MAP(pChar->GetEmpire()), ARENA_RETURN_POINT_X(pChar->GetEmpire()),
                           ARENA_RETURN_POINT_Y(pChar->GetEmpire()));
        }
    }

    m_mapObserver.clear();

    SPDLOG_INFO("ARENA: End Duel PID_A({}) vs PID_B({})", GetPlayerAPID(), GetPlayerBPID());

    Clear();
}

void CArenaManager::GetDuelList(lua_State *L)
{
    auto iter = m_mapArenaMap.begin();

    int index = 1;
    lua_newtable(L);

    for (; iter != m_mapArenaMap.end(); ++iter)
    {
        CArenaMap *pArenaMap = iter->second;
        if (pArenaMap != nullptr)
            index = pArenaMap->GetDuelList(L, index);
    }
}

int CArenaMap::GetDuelList(lua_State *L, int index)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;

        if (pArena == nullptr)
            continue;

        if (pArena->IsEmpty() == false)
        {
            CHARACTER *chA = pArena->GetPlayerA();
            CHARACTER *chB = pArena->GetPlayerB();

            if (chA != nullptr && chB != nullptr)
            {
                lua_newtable(L);

                lua_pushstring(L, chA->GetName().c_str());
                lua_rawseti(L, -2, 1);

                lua_pushstring(L, chB->GetName().c_str());
                lua_rawseti(L, -2, 2);

                lua_pushnumber(L, m_dwMapIndex);
                lua_rawseti(L, -2, 3);

                lua_pushnumber(L, pArena->GetObserverPoint().x);
                lua_rawseti(L, -2, 4);

                lua_pushnumber(L, pArena->GetObserverPoint().y);
                lua_rawseti(L, -2, 5);

                lua_rawseti(L, -2, index++);
            }
        }
    }

    return index;
}

bool CArenaManager::CanAttack(CHARACTER *pCharAttacker, CHARACTER *pCharVictim)
{
    if (pCharAttacker == nullptr || pCharVictim == nullptr)
        return false;

    if (pCharAttacker == pCharVictim)
        return false;

    long mapIndex = pCharAttacker->GetMapIndex();
    if (mapIndex != pCharVictim->GetMapIndex())
        return false;

    auto iter = m_mapArenaMap.find(mapIndex);

    if (iter == m_mapArenaMap.end())
        return false;

    CArenaMap *pArenaMap = (CArenaMap *)(iter->second);
    return pArenaMap->CanAttack(pCharAttacker, pCharVictim);
}

bool CArenaMap::CanAttack(CHARACTER *pCharAttacker, CHARACTER *pCharVictim)
{
    if (pCharAttacker == nullptr || pCharVictim == nullptr)
        return false;

    uint32_t dwPIDA = pCharAttacker->GetPlayerID();
    uint32_t dwPIDB = pCharVictim->GetPlayerID();

    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;
        if (pArena->CanAttack(dwPIDA, dwPIDB) == true) { return true; }
    }
    return false;
}

bool CArena::CanAttack(uint32_t dwPIDA, uint32_t dwPIDB)
{
    // 1:1 전용 다대다 할 경우 수정 필요
    if (m_dwPIDA == dwPIDA && m_dwPIDB == dwPIDB)
        return true;
    if (m_dwPIDA == dwPIDB && m_dwPIDB == dwPIDA)
        return true;

    return false;
}

bool CArenaManager::OnDead(CHARACTER *pCharKiller, CHARACTER *pCharVictim)
{
    if (pCharKiller == nullptr || pCharVictim == nullptr)
        return false;

    long mapIndex = pCharKiller->GetMapIndex();
    if (mapIndex != pCharVictim->GetMapIndex())
        return false;

    auto iter = m_mapArenaMap.find(mapIndex);
    if (iter == m_mapArenaMap.end())
        return false;

    CArenaMap *pArenaMap = (CArenaMap *)(iter->second);
    return pArenaMap->OnDead(pCharKiller, pCharVictim);
}

bool CArenaMap::OnDead(CHARACTER *pCharKiller, CHARACTER *pCharVictim)
{
    uint32_t dwPIDA = pCharKiller->GetPlayerID();
    uint32_t dwPIDB = pCharVictim->GetPlayerID();

    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;

        if (pArena->IsMember(dwPIDA) == true && pArena->IsMember(dwPIDB) == true)
        {
            pArena->OnDead(dwPIDA, dwPIDB);
            return true;
        }
    }
    return false;
}

bool CArena::OnDead(uint32_t dwPIDA, uint32_t dwPIDB)
{
    bool restart = false;

    CHARACTER *pCharA = GetPlayerA();
    CHARACTER *pCharB = GetPlayerB();

    if (pCharA == nullptr && pCharB == nullptr)
    {
        // 둘다 접속이 끊어졌다 ?!
        SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("대련자 문제로 인하여 대련을 중단합니다."));
        restart = false;
    }
    else if (pCharA == nullptr && pCharB != nullptr)
    {
        SendI18nChatPacket(pCharB, CHAT_TYPE_NOTICE, "상대방 캐릭터의 문제로 인하여 대련을 종료합니다.");
        SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("대련자 문제로 인하여 대련을 종료합니다."));
        restart = false;
    }
    else if (pCharA != nullptr && pCharB == nullptr)
    {
        SendI18nChatPacket(pCharA, CHAT_TYPE_NOTICE, "상대방 캐릭터의 문제로 인하여 대련을 종료합니다.");
        SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("대련자 문제로 인하여 대련을 종료합니다."));
        restart = false;
    }
    else if (pCharA != nullptr && pCharB != nullptr)
    {
        if (m_dwPIDA == dwPIDA)
        {
            m_dwSetPointOfA++;

            if (m_dwSetPointOfA >= m_dwSetCount)
            {
                SendI18nChatPacket(pCharA, CHAT_TYPE_NOTICE, "%s 님이 대련에서 승리하였습니다.", pCharA->GetName());
                SendI18nChatPacket(pCharB, CHAT_TYPE_NOTICE, "%s 님이 대련에서 승리하였습니다.", pCharA->GetName());
                SendChatPacketToObserver(CHAT_TYPE_NOTICE, ("%s 님이 대련에서 승리하였습니다."), pCharA->GetName());

                SPDLOG_INFO("ARENA: Duel is end. Winner %s(%d) Loser %s(%d)",
                            pCharA->GetName(), GetPlayerAPID(), pCharB->GetName(), GetPlayerBPID());
            }
            else
            {
                restart = true;
                SendI18nChatPacket(pCharA, CHAT_TYPE_INFO, "%s 님이 승리하였습니다.", pCharA->GetName().c_str());
                pCharA->ChatPacket(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName(), m_dwSetPointOfA,
                                   m_dwSetPointOfB, pCharB->GetName().c_str());

                SendI18nChatPacket(pCharB, CHAT_TYPE_INFO, "%s 님이 승리하였습니다.", pCharA->GetName().c_str());
                pCharB->ChatPacket(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName().c_str(), m_dwSetPointOfA,
                                   m_dwSetPointOfB, pCharB->GetName().c_str());

                SendChatPacketToObserver(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName().c_str(), m_dwSetPointOfA,
                                         m_dwSetPointOfB, pCharB->GetName().c_str());

                SPDLOG_INFO("ARENA: %s(%d) won a round vs %s(%d)",
                            pCharA->GetName(), GetPlayerAPID(), pCharB->GetName(), GetPlayerBPID());
            }
        }
        else if (m_dwPIDB == dwPIDA)
        {
            m_dwSetPointOfB++;
            if (m_dwSetPointOfB >= m_dwSetCount)
            {
                SendI18nChatPacket(pCharA, CHAT_TYPE_NOTICE, "%s 님이 대련에서 승리하였습니다.", pCharB->GetName());
                SendI18nChatPacket(pCharB, CHAT_TYPE_NOTICE, "%s 님이 대련에서 승리하였습니다.", pCharB->GetName());

                SPDLOG_INFO("ARENA: Duel is end. Winner(%d) Loser(%d)", GetPlayerBPID(), GetPlayerAPID());
            }
            else
            {
                restart = true;
                SendI18nChatPacket(pCharA, CHAT_TYPE_INFO, "%s 님이 승리하였습니다.", pCharB->GetName());
                pCharA->ChatPacket(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName().c_str(), m_dwSetPointOfA,
                                   m_dwSetPointOfB, pCharB->GetName().c_str());

                SendI18nChatPacket(pCharB, CHAT_TYPE_INFO, "%s 님이 승리하였습니다.", pCharB->GetName());
                pCharB->ChatPacket(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName().c_str(), m_dwSetPointOfA,
                                   m_dwSetPointOfB, pCharB->GetName().c_str());

                SendChatPacketToObserver(CHAT_TYPE_NOTICE, "%s %d : %d %s", pCharA->GetName().c_str(), m_dwSetPointOfA,
                                         m_dwSetPointOfB, pCharB->GetName().c_str());

                SPDLOG_INFO("ARENA : PID(%d) won a round. Opp(%d)", GetPlayerBPID(), GetPlayerAPID());
            }
        }
        else
        {
            // wtf
            SPDLOG_INFO("ARENA : OnDead Error (%d, %d) (%d, %d)", m_dwPIDA, m_dwPIDB, dwPIDA, dwPIDB);
        }

        int potion = quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count");
        pCharA->SetPotionLimit(potion);
        pCharB->SetPotionLimit(potion);
    }
    else
    {
        // 오면 안된다 ?!
    }

    if (restart == false)
    {
        if (pCharA != nullptr)
            SendI18nChatPacket(pCharA, CHAT_TYPE_INFO, "10초뒤 마을로 되돌아갑니다.");

        if (pCharB != nullptr)
            SendI18nChatPacket(pCharB, CHAT_TYPE_INFO, "10초뒤 마을로 되돌아갑니다.");

        SendChatPacketToObserver(CHAT_TYPE_INFO, ("10초뒤 마을로 되돌아갑니다."));

        if (m_pEvent != nullptr) { event_cancel(&m_pEvent); }

        TArenaEventInfo *info = AllocEventInfo<TArenaEventInfo>();

        info->pArena = this;
        info->state = 2;

        m_pEvent = event_create(ready_to_start_event, info, THECORE_SECS_TO_PASSES(10));
    }
    else
    {
        if (pCharA != nullptr)
            SendI18nChatPacket(pCharA, CHAT_TYPE_INFO, "10초뒤 다음 판을 시작합니다.");

        if (pCharB != nullptr)
            SendI18nChatPacket(pCharB, CHAT_TYPE_INFO, "10초뒤 다음 판을 시작합니다.");

        SendChatPacketToObserver(CHAT_TYPE_INFO, ("10초뒤 다음 판을 시작합니다."));

        if (m_pEvent != nullptr) { event_cancel(&m_pEvent); }

        TArenaEventInfo *info = AllocEventInfo<TArenaEventInfo>();

        info->pArena = this;
        info->state = 3;

        m_pEvent = event_create(ready_to_start_event, info, THECORE_SECS_TO_PASSES(10));
    }

    return true;
}

bool CArenaManager::AddObserver(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY)
{
    auto iter = m_mapArenaMap.find(mapIdx);

    if (iter == m_mapArenaMap.end())
        return false;

    CArenaMap *pArenaMap = iter->second;
    return pArenaMap->AddObserver(pChar, ObserverX, ObserverY);
}

bool CArenaMap::AddObserver(CHARACTER *pChar, long ObserverX, long ObserverY)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;

        if (pArena->IsMyObserver(ObserverX, ObserverY) == true)
        {
            pChar->SetArena(pArena);
            return pArena->AddObserver(pChar);
        }
    }

    return false;
}

bool CArena::IsMyObserver(long ObserverX, long ObserverY)
{
    return ((ObserverX == m_ObserverPoint.x) && (ObserverY == m_ObserverPoint.y));
}

bool CArena::AddObserver(CHARACTER *pChar)
{
    uint32_t pid = pChar->GetPlayerID();

    m_mapObserver.insert(std::make_pair(pid, (CHARACTER *)nullptr));

    pChar->SaveExitLocation();
    pChar->WarpSet(m_mapIndex, m_ObserverPoint.x * 100, m_ObserverPoint.y * 100);

    return true;
}

bool CArenaManager::IsArenaMap(uint32_t dwMapIndex) { return m_mapArenaMap.find(dwMapIndex) != m_mapArenaMap.end(); }

MEMBER_IDENTITY CArenaManager::IsMember(uint32_t dwMapIndex, uint32_t PID)
{
    auto iter = m_mapArenaMap.find(dwMapIndex);

    if (iter != m_mapArenaMap.end())
    {
        CArenaMap *pArenaMap = iter->second;
        return pArenaMap->IsMember(PID);
    }

    return MEMBER_NO;
}

MEMBER_IDENTITY CArenaMap::IsMember(uint32_t PID)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;

        if (pArena->IsObserver(PID) == true)
            return MEMBER_OBSERVER;
        if (pArena->IsMember(PID) == true)
            return MEMBER_DUELIST;
    }
    return MEMBER_NO;
}

bool CArena::IsObserver(uint32_t PID)
{
    auto iter = m_mapObserver.find(PID);

    return iter != m_mapObserver.end();
}

void CArena::OnDisconnect(uint32_t pid)
{
    if (m_dwPIDA == pid)
    {
        if (GetPlayerB() != nullptr)
            SendI18nChatPacket(GetPlayerB(), CHAT_TYPE_INFO, "상대방 캐릭터가 접속을 종료하여 대련을 중지합니다.");

        SPDLOG_INFO("ARENA : Duel is end because of Opp(%d) is disconnect. MyPID(%d)", GetPlayerAPID(),
                    GetPlayerBPID());
        EndDuel();
    }
    else if (m_dwPIDB == pid)
    {
        if (GetPlayerA() != nullptr)
            SendI18nChatPacket(GetPlayerA(), CHAT_TYPE_INFO, "상대방 캐릭터가 접속을 종료하여 대련을 중지합니다.");

        SPDLOG_INFO("ARENA : Duel is end because of Opp(%d) is disconnect. MyPID(%d)", GetPlayerBPID(),
                    GetPlayerAPID());
        EndDuel();
    }
}

void CArena::RemoveObserver(uint32_t pid)
{
    auto iter = m_mapObserver.find(pid);

    if (iter != m_mapObserver.end()) { m_mapObserver.erase(iter); }
}

void CArena::SendPacketToObserver(const void *c_pvData, int iSize)
{
    /*
    auto iter = m_mapObserver.begin();

    for (; iter != m_mapObserver.end(); ++iter)
    {
        CHARACTER* pChar = iter->second;

        if (pChar != NULL)
        {
            if (pChar->GetDesc() != NULL)
            {
                pChar->GetDesc()->Packet(c_pvData, iSize);
            }
        }
    }
    */
}

bool CArenaManager::EndDuel(uint32_t pid)
{
    auto iter = m_mapArenaMap.begin();

    for (; iter != m_mapArenaMap.end(); ++iter)
    {
        CArenaMap *pArenaMap = iter->second;
        if (pArenaMap->EndDuel(pid) == true)
            return true;
    }
    return false;
}

bool CArenaMap::EndDuel(uint32_t pid)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;
        if (pArena->IsMember(pid) == true)
        {
            pArena->EndDuel();
            return true;
        }
    }
    return false;
}

bool CArenaManager::RegisterObserverPtr(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY)
{
    if (pChar == nullptr)
        return false;

    auto iter = m_mapArenaMap.find(mapIdx);

    if (iter == m_mapArenaMap.end())
    {
        SPDLOG_INFO("ARENA : Cannot find ArenaMap. %d %ld %ld", mapIdx, ObserverX, ObserverY);
        return false;
    }

    CArenaMap *pArenaMap = iter->second;
    return pArenaMap->RegisterObserverPtr(pChar, mapIdx, ObserverX, ObserverY);
}

bool CArenaMap::RegisterObserverPtr(CHARACTER *pChar, uint32_t mapIdx, long ObserverX, long ObserverY)
{
    auto iter = m_listArena.begin();

    for (; iter != m_listArena.end(); ++iter)
    {
        CArena *pArena = *iter;

        if (pArena->IsMyObserver(ObserverX, ObserverY) == true) { return pArena->RegisterObserverPtr(pChar); }
    }

    return false;
}

bool CArena::RegisterObserverPtr(CHARACTER *pChar)
{
    uint32_t pid = pChar->GetPlayerID();
    auto iter = m_mapObserver.find(pid);

    if (iter == m_mapObserver.end())
    {
        SPDLOG_INFO("ARENA : not in ob list");
        return false;
    }

    m_mapObserver[pid] = pChar;
    return true;
}

bool CArenaManager::IsLimitedItem(long mapIndex, uint32_t dwVnum) { return false; }
