#ifndef METIN2_TOOL_MAKEMOTIONPROTO_MOTIONLIST_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_MOTIONLIST_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <unordered_map>

METIN2_BEGIN_NS

class MotionList
{
	public:
		bool Load(const storm::StringRef& filename);

		const std::vector<storm::String>* GetMotions(uint32_t key) const;
		const storm::String* GetMotion(uint32_t key) const;

	private:
		std::unordered_map<
			uint32_t,
			std::vector<storm::String>
		> m_motionFiles;
};

METIN2_END_NS

#endif // METIN2_TOOL_MAKEMOTIONPROTO_MOTIONLIST_HPP
