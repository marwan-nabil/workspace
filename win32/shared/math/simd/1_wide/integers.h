#pragma once

/******************************************/
/*          arithmetic operations         */
/******************************************/
inline u32_lane
operator+(u32_lane A, u32_lane B)
{
    u32_lane Result;
    Result.m32i_i32[0] = A.m32i_i32[0] + B.m32i_i32[0];
    return Result;
}

inline u32_lane
operator-(u32_lane A, u32_lane B)
{
    u32_lane Result;
    Result.m32i_i32[0] = A.m32i_i32[0] - B.m32i_i32[0];
    return Result;
}

/******************************************/
/*             bitwise operations         */
/******************************************/
inline u32_lane
operator&(u32_lane A, u32_lane B)
{
    u32_lane Result;
    Result.m32i_i32[0] = A.m32i_i32[0] & B.m32i_i32[0];
    return Result;
}

inline u32_lane
AndNot(u32_lane A, u32_lane B)
{
    // (~A) & B
    u32_lane Result;
    Result.m32i_i32[0] = (~A.m32i_i32[0]) & B.m32i_i32[0];
    return Result;
}

inline u32_lane
operator|(u32_lane A, u32_lane B)
{
    u32_lane Result;
    Result.m32i_i32[0] = A.m32i_i32[0] | B.m32i_i32[0];
    return Result;
}

inline u32_lane
operator^(u32_lane A, u32_lane B)
{
    u32_lane Result;
    Result.m32i_i32[0] = A.m32i_i32[0] ^ B.m32i_i32[0];
    return Result;
}

inline u32_lane
operator~(u32_lane A)
{
    u32_lane Result;
    Result.m32i_i32[0] = ~A.m32i_i32[0];
    return Result;
}

inline u32_lane
operator<<(u32_lane Value, u32 Shift)
{
    u32_lane Result;
    Result.m32i_i32[0] = Value.m32i_i32[0] << Shift;
    return Result;
}

inline u32_lane
operator>>(u32_lane Value, u32 Shift)
{
    u32_lane Result;
    Result.m32i_i32[0] = Value.m32i_i32[0] >> Shift;
    return Result;
}

/******************************************/
/*           comparison operations        */
/******************************************/
inline u32_lane
operator<(u32_lane A, u32_lane B)
{
    u32_lane Result;
    if (A.m32i_i32[0] < B.m32i_i32[0])
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
operator>(u32_lane A, u32_lane B)
{
    u32_lane Result;
    if (A.m32i_i32[0] > B.m32i_i32[0])
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
operator==(u32_lane A, u32_lane B)
{
    u32_lane Result;
    if (A.m32i_i32[0] == B.m32i_i32[0])
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
operator!=(u32_lane A, u32_lane B)
{
    u32_lane Result;
    if (A.m32i_i32[0] == B.m32i_i32[0])
    {
        Result.m32i_i32[0] = 0;
    }
    else
    {
        Result.m32i_i32[0] = 0xFFFFFFFF;
    }
    return Result;
}

/******************************************/
/*             other operations           */
/******************************************/
inline b32
MaskIsAllZeroes(u32_lane Mask)
{
    if (Mask.m32i_i32[0] == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

inline u64
HorizontalAdd(u32_lane WideValue)
{
    return (u64)WideValue.m32i_i32[0];
}