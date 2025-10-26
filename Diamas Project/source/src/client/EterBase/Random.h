#ifndef __INC_ETERBASE_RANDOM_H__
#define __INC_ETERBASE_RANDOM_H__
#include <cstdint>

extern void				srandom(uint32_t seed);
extern uint32_t	random();

#include <random>
#include <utility>
#include <type_traits>

#include <boost/utility/enable_if.hpp>
std::mt19937& GetRand();

// Return value is between [lo, hi]
template <typename T>
typename boost::enable_if<std::is_integral<T>, T>::type GetRandom(T lo, T hi)
{
	// TODO(tim): Fix callers
	if (lo == hi)
		return lo;

	if (hi < lo)
		std::swap(lo, hi);

	std::uniform_int_distribution<T> d(lo, hi);
	return d(GetRand());
}

// Return value *should be* between [lo, hi)
template <typename T>
typename boost::enable_if<std::is_floating_point<T>, T>::type GetRandom(T lo, T hi)
{
	// TODO(tim): Fix callers
	if (lo == hi)
		return lo;

	if (hi < lo)
		std::swap(lo, hi);

	std::uniform_real_distribution<T> d(lo, hi);
	return d(GetRand());
}

// std::pair<I, I> overloads


template <typename T>
T GetRandom(const std::pair<T, T>& lohi)
{
	return GetRandom<T>(lohi.first, lohi.second);
}

#endif