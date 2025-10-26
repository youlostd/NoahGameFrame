#include "PlayerLoader.hpp"

#include "ClientManager.h"
#include "Persistence.hpp"
#include <utility>

PlayerLoader::PlayerLoader(CClientManager& owner, uint32_t clientId,
                           uint32_t peerId, uint32_t aid, uint32_t pid)
    : m_owner(owner)
    , m_clientId(clientId)
    , m_peerId(peerId)
    , m_aid(aid)
    , m_pid(pid)
    , m_cachedPlayer(owner.GetPlayerCache().ForceGet(pid))
    , m_failed(false)
    , m_title() {
    m_cachedPlayer.Lock();
}

PlayerLoader::~PlayerLoader()
{
    // If we failed, there's no one holding on to that player anymore.
    if (m_failed) {
        m_cachedPlayer.Unlock();

        // Remove the half-loaded player from our cache again...
        m_owner.GetPlayerCache().Remove(m_pid);
    }
}

void PlayerLoader::Fail()
{
    m_failed = true;
}

void PlayerLoader::SetTable(const TPlayerTable& table)
{
    m_cachedPlayer.SetTable(table, false);

    m_hasPlayer = true;
    Flush();
}

void PlayerLoader::SetTitle(const TPlayerTitle& skills)
{
    m_title = skills;
    m_hasTitle = true;
    Flush();
}

void PlayerLoader::SetSkills(std::vector<SkillPair> skills)
{
    m_skills = std::move(skills);
    m_hasSkills = true;
    Flush();
}

void PlayerLoader::SetQuickslots(std::vector<QuickslotPair> quickslots)
{
    m_quickslots = std::move(quickslots);
    m_hasQuickslots = true;
    Flush();
}

void PlayerLoader::SetItems(std::vector<TPlayerItem> items)
{
    m_items = std::move(items);
    m_hasItems = true;
    Flush();
}

void PlayerLoader::SetQuests(std::vector<TQuestTable> quests)
{
    m_quests = std::move(quests);
    m_hasQuests = true;
    Flush();
}

void PlayerLoader::SetSwitchbotData(std::vector<SwitchBotSlotData> slotData)
{
    m_switchBotSlotData = std::move(slotData);
    m_hasSwitchBotSlotData = true;
    Flush();
}

void PlayerLoader::SetAffects(std::vector<AffectData> affects)
{
    m_affects = std::move(affects);
    m_hasAffects = true;
    Flush();
}

#ifdef ENABLE_BATTLE_PASS
void PlayerLoader::SetBattlePassMissions(
    std::vector<TPlayerBattlePassMission> missions)
{
    m_battlePassMissions = std::move(missions);
    m_hasBattlePassMissions = true;
    Flush();
}
#endif

void PlayerLoader::SetHuntingMissions(
    std::vector<TPlayerHuntingMission> missions)
{
    m_huntingMissions = std::move(missions);
    m_hasHuntingMissions = true;
    Flush();
}

void PlayerLoader::SetBlocked(std::vector<ElementLoadBlockedPC> blocked)
{
    m_blocked = std::move(blocked);
    m_hasBlocked = true;
    Flush();
}

void PlayerLoader::SetDungeonInfos(std::vector<TPlayerDungeonInfo> infos)
{
    m_dungeonInfos = std::move(infos);
    m_hasDungeonInfos = true;
    Flush();
}

void PlayerLoader::Flush()
{
    // Check whether we have everything yet
    if (!m_hasPlayer || !m_hasSkills || !m_hasQuickslots || !m_hasItems ||
        !m_hasQuests || !m_hasAffects || !m_hasTitle || !m_hasBlocked
#ifdef ENABLE_BATTLE_PASS
        || !m_hasBattlePassMissions
#endif
        || !m_hasHuntingMissions || !m_hasDungeonInfos) {

        return;
    }

    // If we end up here, we have served our purpose and all outstanding
    // requests have either finished or failed.
    std::unique_ptr<PlayerLoader> scoped(this);

    FinalizePlayerTable();

    if (m_failed)
        FlushFailure();
    else
        FlushSuccess();
}

void PlayerLoader::FlushSuccess()
{
    const auto size =
        sizeof(TPlayerTable) + sizeof(uint32_t) +
        m_items.size() * sizeof(TPlayerItem) + sizeof(uint32_t) +
        m_quests.size() * sizeof(TQuestTable) + sizeof(uint32_t) +
        m_affects.size() * sizeof(AffectData) + sizeof(uint32_t) +
        m_switchBotSlotData.size() * sizeof(SwitchBotSlotData) +
        sizeof(uint32_t) + m_blocked.size() * sizeof(ElementLoadBlockedPC) +
        sizeof(uint32_t) +
        m_battlePassMissions.size() * sizeof(TPlayerBattlePassMission) +
        sizeof(uint32_t) +
        m_huntingMissions.size() * sizeof(TPlayerHuntingMission) +
        sizeof(uint32_t) + m_dungeonInfos.size() * sizeof(TPlayerDungeonInfo);

    auto peer = m_owner.GetPeer(m_peerId);
    if (!peer)
        return;

    SPDLOG_TRACE("Loaded player {0}: items {1}, quests {2}, affects {3}, "
                  "blocked {4}",
                  m_pid, m_items.size(), m_quests.size(), m_affects.size(),
                  m_blocked.size());

    peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, m_clientId, size);

    peer->Encode(&m_cachedPlayer.GetTable(), sizeof(TPlayerTable));

    peer->EncodeDWORD(m_items.size());
    peer->Encode(m_items.data(), sizeof(TPlayerItem) * m_items.size());

    peer->EncodeDWORD(m_quests.size());
    peer->Encode(m_quests.data(), sizeof(TQuestTable) * m_quests.size());

    peer->EncodeDWORD(m_affects.size());
    peer->Encode(m_affects.data(), sizeof(AffectData) * m_affects.size());

    peer->EncodeDWORD(m_switchBotSlotData.size());
    peer->Encode(m_switchBotSlotData.data(),
                 sizeof(SwitchBotSlotData) * m_switchBotSlotData.size());

    peer->EncodeDWORD(m_blocked.size());
    peer->Encode(m_blocked.data(),
                 sizeof(ElementLoadBlockedPC) * m_blocked.size());

#ifdef ENABLE_BATTLE_PASS
    peer->EncodeDWORD(m_battlePassMissions.size());
    peer->Encode(m_battlePassMissions.data(), sizeof(TPlayerBattlePassMission) *
                                                  m_battlePassMissions.size());
#endif

    peer->EncodeDWORD(m_huntingMissions.size());
    peer->Encode(m_huntingMissions.data(),
                 sizeof(TPlayerHuntingMission) * m_huntingMissions.size());

    peer->EncodeDWORD(m_dungeonInfos.size());
    peer->Encode(m_dungeonInfos.data(),
                 sizeof(TPlayerDungeonInfo) * m_dungeonInfos.size());
}

void PlayerLoader::FlushFailure()
{
    auto peer = m_owner.GetPeer(m_peerId);
    if (!peer)
        return;

    peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, m_clientId, 0);
}

void PlayerLoader::FinalizePlayerTable()
{
    auto table = m_cachedPlayer.GetTable();

    for (const auto& p : m_skills)
        table.skills[p.first] = p.second;

    for (const auto& p : m_quickslots)
        table.quickslot[p.first] = p.second;

    table.title = m_title;

    m_cachedPlayer.SetTable(table, false);
}
