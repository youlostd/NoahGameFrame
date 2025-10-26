#ifndef METIN2_TOOL_MAKEMOTIONPROTO_RACE_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_RACE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Motion.hpp"

#include <storm/String.hpp>

#include <unordered_map>

METIN2_BEGIN_NS

class Race
{
	public:
		typedef std::unordered_map<
			uint32_t,
			std::vector<Motion>
		> MotionMap;

		Race(uint32_t vnum);

		uint32_t GetVnum() const;

		const storm::String& GetModelPath() const;

		MotionMap& GetMotionMap();
		const MotionMap& GetMotionMap() const;

		const std::vector<Motion>* GetMotions(uint32_t key) const;
		const Motion* GetMotion(uint32_t key) const;

		bool LoadModelScript(const storm::StringRef& filename);

		void AddMotion(uint32_t key, const Motion& motion);

	private:
		uint32_t m_vnum;
		storm::String m_modelPath;
		MotionMap m_motions;
};

BOOST_FORCEINLINE uint32_t Race::GetVnum() const
{ return m_vnum; }

BOOST_FORCEINLINE const storm::String& Race::GetModelPath() const
{ return m_modelPath; }

BOOST_FORCEINLINE Race::MotionMap& Race::GetMotionMap()
{ return m_motions; }

BOOST_FORCEINLINE const Race::MotionMap& Race::GetMotionMap() const
{ return m_motions; }

METIN2_END_NS

#endif // METIN2_TOOL_MAKEMOTIONPROTO_RACE_HPP
