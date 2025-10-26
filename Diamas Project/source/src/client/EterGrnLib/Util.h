#ifndef METIN2_CLIENT_ETERGRNLIB_UTIL_HPP
#define METIN2_CLIENT_ETERGRNLIB_UTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrannyLibrary.hpp"

namespace GrannyTimer
{
static granny_system_clock BootClock;
static granny_system_clock LastClock;
static granny_system_clock CurrentClock;
static granny_real32 SecondsElapsed;

static void InitiateClocks()
{
    GrannyGetSystemSeconds(&BootClock);
    GrannyGetSystemSeconds(&LastClock);
}

static void UpdateClock()
{
    GrannyGetSystemSeconds(&CurrentClock);

    SecondsElapsed = GrannyGetSecondsElapsed(
        &LastClock, &CurrentClock);
    LastClock = CurrentClock;
};

static granny_real32 GetElapsedSeconds()
{
    return SecondsElapsed;
}

static granny_real32 TotalSecondsElapsed()
{
    return GrannyGetSecondsElapsed(
        &BootClock, &CurrentClock);
}
}

struct granny_pngbt333322_vertex
{
    granny_real32 Position[3];
    granny_real32 Normal[3];
    granny_real32 Tangent[3];
    granny_real32 Binormal[3];
    granny_real32 UV0[2];
    granny_real32 UV1[2];
};

struct granny_pwngbt3433322_vertex
{
    granny_real32 Position[3];
    granny_uint8 BoneWeights[4];
    granny_uint8 BoneIndices[4];
    granny_real32 Normal[3];
    granny_real32 Tangent[3];
    granny_real32 Binormal[3];
    granny_real32 UV0[2];
    granny_real32 UV1[2];
};

extern granny_data_type_definition GrannyPNGBT333322VertexType[7];
extern granny_data_type_definition GrannyPWNGBT3433322VertexType[9];



std::string GetGrannyTypeString(const granny_data_type_definition *type);

// mode- Source model, which will be scanned for meshes
// type - Output vertex type. Might differ from source mesh vertex type.
// fvf - Flexible vertex format (D3D)
bool FindBestRigidVertexFormat(const granny_model *model,
                               const granny_data_type_definition *&type,
                               bool& isMultiTexCoord);

#endif
