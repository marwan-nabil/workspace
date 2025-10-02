#pragma once

#include <math.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

inline i32
RoundF32ToI32(f32 Real)
{
    i32 Result = (i32)roundf(Real);
    return Result;
}

inline u32
RoundF32ToU32(f32 Real)
{
    u32 Result = (u32)roundf(Real);
    return Result;
}

inline i32
FloorF32ToI32(f32 Real)
{
    i32 Result = (i32)floorf(Real);
    return Result;
}

inline i32
CeilingF32ToI32(f32 Real)
{
    i32 Result = (i32)ceilf(Real);
    return Result;
}

inline i32
TruncateF32ToI32(f32 Real)
{
    i32 Result = (i32)(Real);
    return Result;
}

inline u32
TruncateF32ToU32(f32 Real)
{
    u32 Result = (u32)(Real);
    return Result;
}

inline u32
SafeTruncateUint64ToUint32(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    return (u32)Value;
}

inline u32
StaticCastF32ToU32(f32 Value)
{
    u32 Result = *(u32 *)&Value;
    return Result;
}

inline f32
StaticCastU32ToF32(u32 Value)
{
    f32 Result = *(f32 *)&Value;
    return Result;
}