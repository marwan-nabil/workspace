#pragma once

struct random_series
{
    u32 State;
};

inline u32
XORShift32(random_series *Series)
{
    u32 Result = Series->State;
    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Series->State = Result;
    return Result;
}

inline f32
RandomUnilateral(random_series *Series)
{
    f32 Result = (f32)XORShift32(Series) / (f32)UINT32_MAX;
    return Result;
}

inline f32
RandomBilateral(random_series *Series)
{
    f32 Result = -1.0f + 2.0f * RandomUnilateral(Series);
    return Result;
}