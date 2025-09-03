#pragma once

#define UINT32_MAX 2147483647

inline u32 Max(u32 A, u32 B)
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

inline u32 Min(u32 A, u32 B)
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