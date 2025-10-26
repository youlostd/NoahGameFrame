#include "stdafx.h"
#include "AffectsHolderUtil.hpp"


boost::iterator_range<AffectTypeIterator> GetAffectTypes(const AffectsHolder& c)
{
	return {AffectTypeIterator(c.begin(), c.end()),
		    AffectTypeIterator(c.end(), c.end())};
}

boost::iterator_range<AffectsHolder::const_iterator> GetAffectsByType(const AffectsHolder& holder,
                                                                      uint32_t type)
{
	const auto p = std::equal_range(holder.begin(), holder.end(), type,
	                                CompareAffectTypes());
	return {p.first, p.second};
}
