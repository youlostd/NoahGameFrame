#ifndef METIN2_CLIENT_EFFECTLIB_UTIL_HPP
#define METIN2_CLIENT_EFFECTLIB_UTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Type.h"

#include <vector>
#include <algorithm>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if (p) { delete [] p; p = NULL; }
#endif

template <typename T>
T BlendSingleValue(float time,
                   const CTimeEvent<T> &low,
                   const CTimeEvent<T> &high)
{
    const float timeDiff = high.m_fTime - low.m_fTime;
    const float perc = (time - low.m_fTime) / timeDiff;

    const T valueDiff = high.m_Value - low.m_Value;
    return static_cast<T>(low.m_Value + perc * valueDiff);
}

inline Vector3 BlendSingleValue(float time, const TEffectPosition &low, const TEffectPosition &high)
{
    const float timeDiff = high.m_fTime - low.m_fTime;
    const float perc = (time - low.m_fTime) / timeDiff;

    if (low.m_iMovingType == MOVING_TYPE_DIRECT)
        return XMVectorAdd(low.m_Value, XMVectorScale((high.m_Value - low.m_Value), perc));

    if (low.m_iMovingType == MOVING_TYPE_BEZIER_CURVE)
    {
        const float invPerc = 1.0f - perc;

        return low.m_Value * invPerc * invPerc +
               (low.m_Value + low.m_vecControlPoint) * invPerc * perc * 2.0f +
               high.m_Value * perc * perc;
    }

    // Unknown moving type - impossible(?)
    return Vector3();
}

inline DWORDCOLOR BlendSingleValue(float time, const TTimeEventTypeColor &low, const TTimeEventTypeColor &high)
{
    const float timeDiff = high.m_fTime - low.m_fTime;
    const float perc = (time - low.m_fTime) / timeDiff;

    return low.m_Value * (1.0f - perc) + high.m_Value * perc;
}

template <typename T>
auto GetTimeEventBlendValue(float time,
                            const std::vector<T> &vec) -> typename T::value_type
{
    if (vec.empty())
        return typename T::value_type();

    // Single element is easy...
    if (vec.begin() + 1 == vec.end())
        return vec.front().m_Value;

    // All elements are greater than |time| - pick the smallest
    if (time < vec.front().m_fTime)
        return vec.front().m_Value;

    // All elements are smaller than |time| - pick the greatest
    if (time > vec.back().m_fTime)
        return vec.back().m_Value;

    // The two checks above make sure that result doesn't contain vec.end()
    // (We could simply check for vec.end() ourself, but above code lets us
    // skip std::equal_range() altogether, making it faster.)
    auto result = std::equal_range(vec.begin(), vec.end(), time);

    // We have one or more equal elements - pick the first
    if (result.first != result.second)
        return result.first->m_Value;

    // We need first to point to an element smaller than |time|
    // (Note that decrementing first is safe here, we already accounted for
    // vec.begin() being greater-or-equal to |time|.)
    --result.first;
    return BlendSingleValue(time, *result.first, *result.second);
}

template <typename T>
auto GetTimeEventBlendValue(double time,
                            const std::vector<T> &vec) -> typename T::value_type
{
    if (vec.empty())
        return typename T::value_type();

    // Single element is easy...
    if (vec.begin() + 1 == vec.end())
        return vec.front().m_Value;

    // All elements are greater than |time| - pick the smallest
    if (time < vec.front().m_fTime)
        return vec.front().m_Value;

    // All elements are smaller than |time| - pick the greatest
    if (time > vec.back().m_fTime)
        return vec.back().m_Value;

    // The two checks above make sure that result doesn't contain vec.end()
    // (We could simply check for vec.end() ourself, but above code lets us
    // skip std::equal_range() altogether, making it faster.)
    auto result = std::equal_range(vec.begin(), vec.end(), time);

    // We have one or more equal elements - pick the first
    if (result.first != result.second)
        return result.first->m_Value;

    // We need first to point to an element smaller than |time|
    // (Note that decrementing first is safe here, we already accounted for
    // vec.begin() being greater-or-equal to |time|.)
    --result.first;
    return BlendSingleValue(time, *result.first, *result.second);
}

template <typename T>
auto InsertItemTimeEvent(std::vector<CTimeEvent<T>> *pTable, double fTime, T fValue) -> void
{
    auto itor = std::lower_bound(pTable->begin(), pTable->end(), fTime);

    CTimeEvent<T> TimeEvent;
    TimeEvent.m_fTime = fTime;
    TimeEvent.m_Value = fValue;

    pTable->insert(itor, TimeEvent);
}

Color BlendColor(const Color &a, const Color &b);

#endif
