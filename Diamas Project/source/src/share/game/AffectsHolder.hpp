#ifndef METIN2_GAME_AFFECTSHOLDER_HPP
#define METIN2_GAME_AFFECTSHOLDER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/DbPackets.hpp>


struct CompareAffectTypes
{
	bool operator()(uint32_t a, const AffectData& b) const
	{
		return a < b.type;
	}

	bool operator()(const AffectData& a, uint32_t b) const
	{
		return a.type < b;
	}

	bool operator()(const AffectData& a, const AffectData& b) const
	{
		return a.type < b.type;
	}
};

typedef std::vector<AffectData> AffectsHolder;

void InsertAffect(AffectsHolder& holder, const AffectData& affect);
void EraseAffect(AffectsHolder& holder, const AffectData& affect);

AffectData* SearchAffect(AffectsHolder& holder, uint32_t type,
	uint32_t pointType = POINT_NONE);

const AffectData* SearchAffect(const AffectsHolder& holder, uint32_t type,
	uint32_t pointType = POINT_NONE);



#endif
