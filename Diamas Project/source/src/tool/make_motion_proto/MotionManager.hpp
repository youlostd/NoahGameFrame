#ifndef METIN2_TOOL_MAKEMOTIONPROTO_MOTIONMANAGER_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_MOTIONMANAGER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrannyScene.hpp"

#include <storm/String.hpp>

#include <Eigen/Core>

#include <unordered_map>


class PathTranslator;
class MotionList;
class Motion;
class Race;

class MotionManager
{
	public:
		const std::list<Race>& GetRaces() const;

		bool Load(const storm::StringRef& npcList);

		void UpdateAll();

	private:
		bool LoadPcMotions();
		bool LoadFromNpcList(const storm::StringRef& filename);

		bool LoadNpcRace(uint32_t vnum, const storm::StringRef& msmPath);
		void LoadPcRaceMode(uint32_t mode, const storm::String& path,
		                    Race& race, bool allowAttack = true);

		void LoadMotion(const MotionList& list,
		                const storm::StringRef& path,
		                uint32_t mode, uint32_t index,
		                Race& race);

		void LoadMotion(const storm::StringRef& fileame,
		                uint32_t mode, uint32_t index,
		                Race& race);

		void UpdateMotions(Race& race);

		std::list<Race> m_races;
		GrannyScene m_scene;
};

BOOST_FORCEINLINE const std::list<Race>& MotionManager::GetRaces() const
{ return m_races; }


#endif // METIN2_TOOL_MAKEMOTIONPROTO_RACEMANAGER_HPP
