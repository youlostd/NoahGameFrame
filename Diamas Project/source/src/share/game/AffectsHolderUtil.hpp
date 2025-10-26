#ifndef METIN2_GAME_AFFECTSHOLDERUTIL_HPP
#define METIN2_GAME_AFFECTSHOLDERUTIL_HPP

#pragma once

#include "AffectsHolder.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <algorithm>
#include <vector>



struct AffectTypeIterator : boost::iterator_facade<
	AffectTypeIterator, uint32_t, boost::forward_traversal_tag, uint32_t>
{
	AffectTypeIterator(AffectsHolder::const_iterator first,
	                   AffectsHolder::const_iterator last)
		: m_first(first)
		, m_last(last)
	{
		// ctor
	}

private:
	friend class boost::iterator_core_access;

	void increment()
	{
		if (m_first != m_last)
			m_first = std::upper_bound(m_first, m_last, *m_first,
			                           CompareAffectTypes());
	}

	bool equal(const AffectTypeIterator& other) const
	{
		return m_first == other.m_first;
	}

	uint32_t dereference() const { return m_first->type; }

	AffectsHolder::const_iterator m_first;
	AffectsHolder::const_iterator m_last;
};

boost::iterator_range<AffectTypeIterator> GetAffectTypes(const AffectsHolder& c);
boost::iterator_range<AffectsHolder::const_iterator> GetAffectsByType(
	const AffectsHolder& holder,
	uint32_t type);


#endif
