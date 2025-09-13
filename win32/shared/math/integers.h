#pragma once

#include "win32\shared\base_types.h"

inline u32
Clamp(u32 Value, u32 Min, u32 Max)
{
    u32 Result = Value;
    if (Result < Min)
    {
        Result = Min;
    }
    else if (Result > Max)
    {
        Result = Max;
    }
    return Result;
}

inline u32
AbsoluteValue(i32 A)
{
    if (A < 0)
    {
        return -A;
    }
    else
    {
        return A;
    }
}

inline i32
Max(i32 A, i32 B)
{
    if (A > B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

inline i32
Min(i32 A, i32 B)
{
    if (A < B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

inline u32
Max(u32 A, u32 B)
{
    if (A > B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

inline u32
Min(u32 A, u32 B)
{
    if (A < B)
    {
        return A;
    }
    else
    {
        return B;
    }
}