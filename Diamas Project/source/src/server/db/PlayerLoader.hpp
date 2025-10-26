#ifndef METIN2_SERVER_DB_PLAYERLOADER_HPP
#define METIN2_SERVER_DB_PLAYERLOADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include <vector>
#include "Persistence.hpp" // for SkillPair
#include <game/DbPackets.hpp>
#include <game/DungeonInfoTypes.hpp>
class CClientManager;
class CachedPlayer;

/// Stateful player loader
///
/// @note PlayerLoader deletes itself
class PlayerLoader
{
public:
	PlayerLoader(CClientManager& owner,
	             uint32_t clientId, uint32_t peerId,
	             uint32_t aid, uint32_t pid);
	~PlayerLoader();

	void Fail();

	void SetTable(const TPlayerTable& table);
	void SetTitle(const TPlayerTitle& skills);
	void SetSkills(std::vector<SkillPair> skills);
	void SetQuickslots(std::vector<QuickslotPair> quickslots);
	void SetItems(std::vector<TPlayerItem> items);
	void SetQuests(std::vector<TQuestTable> quests);
	void SetSwitchbotData(std::vector<SwitchBotSlotData> slotData);
	void SetAffects(std::vector<AffectData> affects);
#ifdef ENABLE_BATTLE_PASS
	void SetBattlePassMissions(std::vector<TPlayerBattlePassMission> missions);
    void SetHuntingMissions(std::vector<TPlayerHuntingMission> missions);
#endif
	void SetBlocked(std::vector<ElementLoadBlockedPC> blocked);
	void SetDungeonInfos(std::vector<TPlayerDungeonInfo> infos);

	uint32_t GetAid() const
	{
		return m_aid;
	}

	uint32_t GetPid() const
	{
		return m_pid;
	}

	uint32_t GetClientId() const
	{
		return m_clientId;
	}

	CachedPlayer& GetPlayer()
	{
		return m_cachedPlayer;
	}

private:
	void Flush();
	void FlushSuccess();
	void FlushFailure();

	/// Merge separately loaded fields into cached player table.
	void FinalizePlayerTable();

	CClientManager& m_owner;
	uint32_t m_clientId; // Client Id
	uint32_t m_peerId; // Game Id
	uint32_t m_aid;
	uint32_t m_pid;
	CachedPlayer& m_cachedPlayer;
	bool m_failed;

	TPlayerTitle m_title;
	std::vector<SkillPair> m_skills;
	std::vector<QuickslotPair> m_quickslots;
	std::vector<TPlayerItem> m_items;
	std::vector<TQuestTable> m_quests;
	std::vector<AffectData> m_affects;
	std::vector<SwitchBotSlotData> m_switchBotSlotData;
	std::vector<ElementLoadBlockedPC> m_blocked;
#ifdef ENABLE_BATTLE_PASS
	std::vector<TPlayerBattlePassMission> m_battlePassMissions;
#endif
	std::vector<TPlayerHuntingMission> m_huntingMissions;
	std::vector<TPlayerDungeonInfo> m_dungeonInfos;

	bool m_hasPlayer = false;
	bool m_hasSkills = false;
	bool m_hasQuickslots = false;
	bool m_hasItems = false;
	bool m_hasQuests = false;
	bool m_hasAffects = false;
	bool m_hasTitle = false;
	bool m_hasBlocked = false;
	bool m_hasSwitchBotSlotData = false;
#ifdef ENABLE_BATTLE_PASS
	bool m_hasBattlePassMissions = false;
#endif
	bool m_hasHuntingMissions = false;
	bool m_hasDungeonInfos = false;

};


#endif
