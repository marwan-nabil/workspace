#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\math\vector3.h"

#if (SIMD_NUMBEROF_LANES == 1)
#   include "win32\shared\math\simd\1_wide\math.h"
#elif (SIMD_NUMBEROF_LANES == 4)
#   include "win32\shared\math\simd\4_wide\math.h"
#elif (SIMD_NUMBEROF_LANES == 8)
#   include "win32\shared\math\simd\8_wide\math.h"
#else
#   error "ERROR: brdf.h: SIMD_NUMBEROF_LANES is not defined, or not supported."
#endif

#include "win32\shared\math\simd\shared\math.h"

struct brdf_sample
{
    f32 ThetaIn;
    f32 PhiIn;
    f32 ThetaOut;
    f32 PhiOut;
    v3 Color;
};

struct brdf_table
{
    u32 SampleCount;
    brdf_sample *Samples;
};

struct material;

void LoadReflectionDataForMaterial(char *FileName, material *Material);
v3_lane GetMaterialReflectionColor
(
    material *Materials, u32_lane MaterialIndex, u32_lane LaneMask,
    v3_lane Tangent, v3_lane BiTangent, v3_lane Normal,
    v3_lane Incoming, v3_lane Outgoing
);