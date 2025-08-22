#pragma once

struct random_series_lane
{
    u32_lane State;
};

inline u32_lane
XORShift32Lane(random_series_lane *Series)
{
    u32_lane Result = Series->State;
    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Series->State = Result;
    return Result;
}

inline f32_lane
RandomUnilateralLane(random_series_lane *Series)
{
    f32_lane Result = F32LaneFromU32Lane(XORShift32Lane(Series) >> 1) / (f32)(UINT32_MAX >> 1);
    return Result;
}

inline f32_lane
RandomBilateralLane(random_series_lane *Series)
{
    f32_lane Result = -1.0f + 2.0f * RandomUnilateralLane(Series);
    return Result;
}