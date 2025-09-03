/******************************************/
/*                  floats                */
/******************************************/
inline f32_lane
F32LaneFromF32(f32 Value)
{
    f32_lane Result = _mm_set1_ps(Value);
    return Result;
}

inline f32_lane
F32LaneFromF32(f32 A, f32 B, f32 C, f32 D)
{
    f32_lane Result = _mm_set_ps(A, B, C, D);
    return Result;
}

inline f32_lane
F32LaneFromU32(u32 Value)
{
    f32_lane Result = _mm_set1_ps((f32)Value);
    return Result;
}

inline f32_lane
F32LaneFromU32Lane(u32_lane Value)
{
    f32_lane Result = _mm_cvtepi32_ps(Value);
    return Result;
}

inline f32_lane
StaticCastU32LaneToF32Lane(u32_lane Value)
{
    f32_lane Result = _mm_castsi128_ps(Value);
    return Result;
}

inline f32
F32FromF32Lane(f32_lane Value, u32 Index)
{
    Assert(Index < 4);
    f32 Result = ((f32 *)&Value)[Index];
    return Result;
}

/******************************************/
/*                  integers              */
/******************************************/
inline u32_lane
U32LaneFromU32(u32 Value)
{
    u32_lane Result = _mm_set1_epi32(Value);
    return Result;
}

inline u32_lane
U32LaneFromU32(u32 A, u32 B, u32 C, u32 D)
{
    u32_lane Result = _mm_set_epi32(A, B, C, D);
    return Result;
}

inline u32
U32FromU32Lane(u32_lane Value, u32 Index)
{
    Assert(Index < 4);
    u32 Result = ((u32 *)&Value)[Index];
    return Result;
}

inline u32_lane
StaticCastF32LaneToU32Lane(f32_lane Value)
{
    u32_lane Result = _mm_castps_si128(Value);
    return Result;
}

inline u32_lane
MaskFromBoolean(u32_lane Value)
{
    u32_lane Result = _mm_xor_si128
    (
        _mm_cmpeq_epi32(Value, U32LaneFromU32(0)),
        _mm_set1_epi32(0xFFFFFFFF)
    );
    return Result;
}