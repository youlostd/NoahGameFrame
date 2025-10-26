#ifndef METIN2_SERVER_GAME_VECTOR_H
#define METIN2_SERVER_GAME_VECTOR_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/math/constants/constants.hpp>

typedef struct SVector
{
    float x;
    float y;
    float z;
} VECTOR;

extern float GetDegreeFromPosition(float x, float y);
extern float GetDegreeFromPositionXY(int32_t sx, int32_t sy, int32_t ex, int32_t ey);
extern void GetDeltaByDegree(float fDegree, float fDistance, float *x, float *y);
extern float GetDegreeDelta(float iDegree, float iDegree2);

void RotateRegion(int32_t &sx, int32_t &sy,
                  int32_t &ex, int32_t &ey,
                  float xRot, float yRot, float zRot);

#endif /* METIN2_SERVER_GAME_VECTOR_H */
