#pragma once

/******************************************/
/*         arithmetic operations          */
/******************************************/
inline f32_lane
operator+(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = A.m32_f32[0] + B.m32_f32[0];
    return Result;
}

inline f32_lane
operator-(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = A.m32_f32[0] - B.m32_f32[0];
    return Result;
}

inline f32_lane
operator*(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = A.m32_f32[0] * B.m32_f32[0];
    return Result;
}

inline f32_lane
operator/(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = A.m32_f32[0] / B.m32_f32[0];
    return Result;
}

/******************************************/
/*          bitwise operations            */
/******************************************/
inline f32_lane
operator&(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = StaticCastU32ToF32
    (
        StaticCastF32ToU32(A.m32_f32[0]) &
        StaticCastF32ToU32(B.m32_f32[0])
    );
    return Result;
}

inline f32_lane
AndNot(f32_lane A, f32_lane B)
{
    // (~A) & B
    f32_lane Result = StaticCastU32LaneToF32Lane
    (
        (~StaticCastF32LaneToU32Lane(A)) &
        StaticCastF32LaneToU32Lane(B)
    );
    return Result;
}

inline f32_lane
operator|(f32_lane A, f32_lane B)
{
    f32_lane Result = StaticCastU32LaneToF32Lane
    (
        StaticCastF32LaneToU32Lane(A) |
        StaticCastF32LaneToU32Lane(B)
    );
    return Result;
}

inline f32_lane
operator^(f32_lane A, f32_lane B)
{
    f32_lane Result = StaticCastU32LaneToF32Lane
    (
        StaticCastF32LaneToU32Lane(A) ^
        StaticCastF32LaneToU32Lane(B)
    );
    return Result;
}

/******************************************/
/*         comparison operations          */
/******************************************/
inline u32_lane
operator<(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] < B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

inline u32_lane
operator<=(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] <= B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

inline u32_lane
operator>(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] > B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

inline u32_lane
operator>=(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] >= B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

inline u32_lane
operator==(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] == B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

inline u32_lane
operator!=(f32_lane A, f32_lane B)
{
    u32_lane Result;
    if (A.m32_f32[0] != B.m32_f32[0])
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    else
    {
        Result.m32i_i32[0] = 0;
    }
    return Result;
}

/******************************************/
/*                gathers                 */
/******************************************/
inline f32_lane
GatherF32Implementation(void *Base, u32 Stride, u32_lane Indices)
{
    u32 *IndexPointer = (u32 *)&Indices;
    f32_lane Result = F32LaneFromF32
    (
        *(f32 *)((u8 *)Base + IndexPointer[0] * Stride)
    );
    return Result;
}

/******************************************/
/*             Common Functions           */
/******************************************/
inline f32_lane
SquareRoot(f32_lane A)
{
    f32_lane Result;
    Result.m32_f32[0] = SquareRoot(A.m32_f32[0]);
    return Result;
}

inline f32
HorizontalAdd(f32_lane WideValue)
{
    f32 NarrowValue = WideValue.m32_f32[0];
    return NarrowValue;
}

inline f32_lane
Max(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = Max(A.m32_f32[0], B.m32_f32[0]);
    return Result;
}

inline f32_lane
Min(f32_lane A, f32_lane B)
{
    f32_lane Result;
    Result.m32_f32[0] = Min(A.m32_f32[0], B.m32_f32[0]);
    return Result;
}

inline f32_lane
Power(f32_lane A, f32 Exponent)
{
    f32_lane Result;
    Result.m32_f32[0] = Power(A.m32_f32[0], Exponent);
    return Result;
}

inline f32_lane
Power(f32_lane A, f32_lane Exponent)
{
    f32_lane Result = Power(A, Exponent.m32_f32[0]);
    return Result;
}