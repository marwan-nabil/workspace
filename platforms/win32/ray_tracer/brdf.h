#pragma once

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