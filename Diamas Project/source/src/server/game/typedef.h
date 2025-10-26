#ifndef METIN2_SERVER_GAME_TYPEDEF_H
#define METIN2_SERVER_GAME_TYPEDEF_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

typedef struct SPixelPosition2
{
    int32_t x, y;
} TPixelPosition2D;

typedef struct pixel_position_s
{
    int32_t x, y, z;
} PIXEL_POSITION, TPixelPosition3D;

typedef struct SPixelRect
{
    TPixelPosition2D min;
    TPixelPosition2D max;

    bool Contains(TPixelPosition2D Point) const;
    bool Contains(TPixelPosition3D Point) const;
} TPixelRect;

BOOST_FORCEINLINE bool SPixelRect::Contains(TPixelPosition2D p) const
{
    return p.x >= min.x && p.x < max.x && p.y >= min.y && p.y < max.y;
}

BOOST_FORCEINLINE bool SPixelRect::Contains(TPixelPosition3D p) const
{
    return p.x >= min.x && p.x < max.x && p.y >= min.y && p.y < max.y;
}

enum EEntityTypes
{
    ENTITY_CHARACTER,
    ENTITY_ITEM,
    ENTITY_OBJECT,
};
#endif /* METIN2_SERVER_GAME_TYPEDEF_H */
