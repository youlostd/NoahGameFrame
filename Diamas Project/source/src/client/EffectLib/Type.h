#ifndef METIN2_CLIENT_EFFECTLIB_TYPE_H
#define METIN2_CLIENT_EFFECTLIB_TYPE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/TextFileLoader.h"

enum EEffectType
{
    EFFECT_TYPE_PARTICLE = 1,
    EFFECT_TYPE_ANIMATION_TEXTURE = 2,
    EFFECT_TYPE_MESH = 3,
    EFFECT_TYPE_SIMPLE_LIGHT = 4,
};

enum EMeshBillBoardType
{
    MESH_BILLBOARD_TYPE_NONE,

    MESH_BILLBOARD_TYPE_ALL,
    MESH_BILLBOARD_TYPE_Y,

    MESH_BILLBOARD_TYPE_MOVE
};

enum EBillBoardType
{
    BILLBOARD_TYPE_NONE,

    BILLBOARD_TYPE_ALL,
    BILLBOARD_TYPE_Y,

    BILLBOARD_TYPE_LIE,
    // 바닥에 누은 형상

    BILLBOARD_TYPE_2FACE,
    BILLBOARD_TYPE_3FACE,
};

enum EMovingType
{
    MOVING_TYPE_DIRECT,
    MOVING_TYPE_BEZIER_CURVE,
};

template <typename T>
struct CTimeEvent
{
    typedef T value_type;

    double m_fTime;
    T m_Value;
};

template <typename T>
bool operator<(const CTimeEvent<T> &lhs, const CTimeEvent<T> &rhs)
{
    return lhs.m_fTime < rhs.m_fTime;
}

template <typename T>
bool operator<(const CTimeEvent<T> &lhs, const float &rhs)
{
    return lhs.m_fTime < rhs;
}

template <typename T>
bool operator<(const float &lhs, const CTimeEvent<T> &rhs)
{
    return lhs < rhs.m_fTime;
}

typedef struct SEffectPosition : CTimeEvent<Vector3>
{
    // For Bezier Curve
    int m_iMovingType;
    Vector3 m_vecControlPoint;
} TEffectPosition;

inline uint32_t MultiplyColor(uint32_t color, float mul)
{
    static const uint32_t AG_MASK = 0xff00ff00;
    static const uint32_t RB_MASK = 0x00ff00ff;

    const uint32_t idx = static_cast<uint32_t>(mul * 256);
    return ((((color & AG_MASK) >> 8) * idx) & AG_MASK) +
           ((((color & RB_MASK) * idx) >> 8) & RB_MASK);
}

struct DWORDCOLOR
{
    uint32_t color;

    DWORDCOLOR(uint32_t color = 0)
        : color(color)
    {
        // ctor
    }

    DWORDCOLOR(const DWORDCOLOR &) = default;

    DWORDCOLOR &operator*=(float f)
    {
        color = MultiplyColor(color, f);
        return *this;
    }

    DWORDCOLOR &operator+=(const DWORDCOLOR &r)
    {
        color += r.color;
        return *this;
    }

    DWORDCOLOR &operator-=(const DWORDCOLOR &r)
    {
        color -= r.color;
        return *this;
    }

    operator uint32_t() const
    {
        return color;
    }
};

inline DWORDCOLOR operator*(DWORDCOLOR dc, float f)
{
    return MultiplyColor(dc, f);
}

inline DWORDCOLOR operator*(float f, DWORDCOLOR dc)
{
    return MultiplyColor(dc, f);
}

inline DWORDCOLOR operator+(DWORDCOLOR a, DWORDCOLOR b)
{
    return a.color + b.color;
}

inline DWORDCOLOR operator-(DWORDCOLOR a, DWORDCOLOR b)
{
    return a.color - b.color;
}

typedef CTimeEvent<char> TTimeEventTypeCharacter;
typedef CTimeEvent<short> TTimeEventTypeShort;
typedef CTimeEvent<float> TTimeEventTypeFloat;
typedef CTimeEvent<double> TTimeEventTypeDouble;
typedef CTimeEvent<uint16_t> TTimeEventTypeWord;
typedef CTimeEvent<uint32_t> TTimeEventTypeDoubleWord;
typedef CTimeEvent<DWORDCOLOR> TTimeEventTypeColor;
typedef CTimeEvent<Vector2> TTimeEventTypeVector2;
typedef CTimeEvent<Vector3> TTimeEventTypeVector3;

typedef std::vector<float> TTimeEventTable;
typedef std::vector<TEffectPosition> TTimeEventTablePosition;
typedef std::vector<TTimeEventTypeCharacter> TTimeEventTableCharacter;
typedef std::vector<TTimeEventTypeShort> TTimeEventTableShort;
typedef std::vector<TTimeEventTypeFloat> TTimeEventTableFloat;
typedef std::vector<TTimeEventTypeDouble> TTimeEventTableDouble;
typedef std::vector<TTimeEventTypeWord> TTimeEventTableWord;
typedef std::vector<TTimeEventTypeDoubleWord> TTimeEventTableDoubleWord;
typedef std::vector<TTimeEventTypeColor> TTimeEventTableColor;
typedef std::vector<TTimeEventTypeVector2> TTimeEventTableVector2;
typedef std::vector<TTimeEventTypeVector3> TTimeEventTableVector3;

bool GetTokenTimeEventFloat(CTextFileLoader &loader,
                            const char *key,
                            TTimeEventTableFloat *table);

bool GetTokenTimeEventDouble(CTextFileLoader &loader,
                             const char *key,
                             TTimeEventTableDouble *table);
#endif /* METIN2_CLIENT_EFFECTLIB_TYPE_H */
