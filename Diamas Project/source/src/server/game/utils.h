#ifndef METIN2_SERVER_GAME_UTILS_H
#define METIN2_SERVER_GAME_UTILS_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "typedef.h"

#ifndef IS_SET
#define IS_SET(flag, bit) ((flag) & (bit))
#endif

#ifndef SET_BIT
#define SET_BIT(var, bit) ((var) |= (bit))
#endif

#ifndef REMOVE_BIT
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(var, bit) ((var) = (var) ^ (bit))
#endif

inline float DISTANCE_SQRT(long dx, long dy) { return ::sqrt((float)dx * dx + (float)dy * dy); }

inline bool SIIsNumber(const char *str)
{
    int i;

    if (!str || !*str)
        return 0;

    for (i = 0; str[i]; ++i)
    {
        if (!isdigit(str[i]))
            return false;
    }

    return true;
}

inline bool SIIsNumber(const char *str, int Len)
{
    int i;

    if (!str || !*str)
        return 0;

    for (i = 0; str[i] && i < Len; ++i)
    {
        if (!isdigit(str[i]))
            return false;
    }

    return true;
}

inline float DISTANCE(int64_t x, int64_t y, int64_t dx, int64_t dy)
{
    return std::sqrt(std::abs((dx - x) * (dx - x)) + std::abs((dy - y) * (dy - y)));
}

inline float DISTANCE(const PIXEL_POSITION &rpos1, const PIXEL_POSITION &rpos2)
{
    return DISTANCE(rpos1.x, rpos1.y, rpos2.x, rpos2.y);
}

inline int DISTANCE_APPROX(int dx, int dy)
{
    int min, max;

    if (dx < 0)
        dx = -dx;

    if (dy < 0)
        dy = -dy;

    if (dx < dy)
    {
        min = dx;
        max = dy;
    }
    else
    {
        min = dy;
        max = dx;
    }

    // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
    return (((max << 8) + (max << 3) - (max << 4) - (max << 1) + (min << 7) - (min << 5) + (min << 3) - (min << 1)) >>
            8);
}

#ifndef _WIN32
inline uint16_t MAKEWORD(uint8_t a, uint8_t b)
{
    return static_cast<uint16_t>(a) | (static_cast<uint16_t>(b) << 8);
}
#endif

void set_global_time(time_t t);
time_t get_global_time();

void skip_spaces(char **string);

const char *one_argument(const char *argument, char *first_arg, size_t first_size);
const char *two_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg,
                          size_t second_size);
const char *three_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg,
                            size_t second_size, char *third_arg, size_t third_size);

const char *first_cmd(const char *argument, char *first_arg, size_t first_arg_size, size_t *first_arg_len_result);

extern void split_argument(const char *argument, std::vector<std::string> & vecArgs);

BOOST_FORCEINLINE int32_t CalculateDuration(uint32_t iSpd, int32_t iDur)
{
    int i = 100 - iSpd;

    if (i > 0)
        i = 100 + i;
    else if (i < 0)
        i = 10000 / (100 - i);
    else
        i = 100;

    return iDur * i / 100;
}

int parse_time_str(const char *str);

bool WildCaseCmp(const char *w, const char *s);

size_t str_lower(const char *src, char *dest, size_t dest_size);

/// Copy characters.
///
/// This function copies at most @c (size - 1) characters from
/// @c src to @c dst. It stops earlier if it encounters a NUL in
/// @c src.
///
/// @param dst The destination string.
///
/// @param src The source string.
///
/// @param size The maximum length of @c dst.
///
/// @return A bool denoting whether the string had to be truncated.
/// @c true means the string was successfully copied, while @c false
/// means the string got truncated.
bool CopyStringSafe(char *dst, const std::string &src, uint32_t size);

template <std::size_t N> BOOST_FORCEINLINE bool CopyStringSafe(char (&dst)[N], const std::string &src)
{
    return CopyStringSafe(dst, src, N);
}

namespace gtl
{
template <typename ContainerT, typename PredicateT> void erase_if(ContainerT &items, const PredicateT &predicate)
{
    for (auto it = items.begin(); it != items.end();)
    {
        if (predicate(*it))
            it = items.erase(it);
        else
            ++it;
    }
};
} // namespace gtl

// Taken from: http://jguegant.github.io/blogs/tech/performing-try-emplace.html
template <class Factory> struct lazy_convert_construct
{
    using result_type = std::invoke_result_t<const Factory &>; // Use some traits to check what would be the return type
    // of the lambda if called.

    constexpr lazy_convert_construct(Factory &&factory)
        : factory_(std::move(factory)) // Let's store the factory for a latter usage.
    {
    }

    //                                     ↓ Respect the same nowthrow properties as the lambda factory.
    constexpr operator result_type() const noexcept(std::is_nothrow_invocable_v<const Factory &>)
    //        ^ enable       ^ the type this struct can be converted to
    //          conversion
    {
        return factory_(); // Delegate the conversion job to the lambda factory.
    }

    Factory factory_;
};

int GetSecondsTillNextMonth();

#endif /* METIN2_SERVER_GAME_UTILS_H */
