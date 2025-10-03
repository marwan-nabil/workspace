#pragma once

#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

struct linear_allocator
{
    size_t Size;
    size_t Used;
    u8 *BaseAddress;
};

inline void *PushOntoMemoryArena(linear_allocator *Allocator, size_t PushSize, b8 Zeroed)
{
    Assert((Allocator->Used + PushSize) < Allocator->Size);
    void *Result = Allocator->BaseAddress + Allocator->Used;
    Allocator->Used += PushSize;
    if (Zeroed)
    {
        memset(Result, 0, PushSize);
    }
    return Result;
}