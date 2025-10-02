#pragma once

#include <intrin.h>
#include "portable\shared\base_types.h"

struct bitscan_result
{
    b32 Found;
    u32 Index;
};

inline bitscan_result
FindLeastSignificantSetBit(u32 Value)
{
    bitscan_result Result = {};
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
    return Result;
}

inline u32
RotateLeft(u32 Value, i32 Rotation)
{
    u32 Result = _rotl(Value, Rotation);
    return Result;
}

inline u32
RotateRight(u32 Value, i32 Rotation)
{
    u32 Result = _rotr(Value, Rotation);
    return Result;
}