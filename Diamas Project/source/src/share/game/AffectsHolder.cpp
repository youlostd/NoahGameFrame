#include "stdafx.h"
#include "AffectsHolder.hpp"


void InsertAffect(AffectsHolder& holder, const AffectData& affect)
{
	const auto it = std::lower_bound(holder.begin(), holder.end(), affect);
	holder.insert(it, affect);
}

void EraseAffect(AffectsHolder& holder, const AffectData& affect)
{
	// Optimization for cases where |affect| lies inside |holder|
	const auto affectPtr = &affect;
	const auto holderPtr = holder.data();
	if (affectPtr >= holderPtr && affectPtr < holderPtr + holder.size()) {
		holder.erase(holder.begin() + (affectPtr - holderPtr));
		return;
	}

	// TODO(tim): use binary search
	const auto it = std::find(holder.begin(), holder.end(), affect);
	if (it != holder.end())
		holder.erase(it);
}

AffectData* SearchAffect(AffectsHolder& holder, uint32_t type, uint32_t pointType)
{
	// Construct a minimal temporary for comparision.
	AffectData tmp = {};
	tmp.type = type;
	tmp.pointType = pointType;

	if (pointType == POINT_NONE) {
		const auto it = std::lower_bound(holder.begin(), holder.end(), tmp,
			CompareAffectTypes());
		if (it == holder.end() || CompareAffectTypes()(tmp, *it))
			return nullptr;

		return &(*it);
	}
	else {
		const auto it = std::lower_bound(holder.begin(), holder.end(), tmp);
		if (it == holder.end() || tmp < *it)
			return nullptr;

		return &(*it);
	}
}

const AffectData* SearchAffect(const AffectsHolder& holder, uint32_t type, uint32_t pointType)
{
	return SearchAffect(const_cast<AffectsHolder&>(holder), type, pointType);
}