#pragma once

#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

struct memory_arena
{
    size_t Size;
    size_t Used;
    u8 *BaseAddress;
};

inline void *PushOntoMemoryArena(memory_arena *MemoryArena, size_t PushSize, bool Zeroed)
{
    Assert((MemoryArena->Used + PushSize) < MemoryArena->Size);
    void *Result = MemoryArena->BaseAddress + MemoryArena->Used;
    MemoryArena->Used += PushSize;
    if (Zeroed)
    {
        memset(Result, 0, PushSize);
    }
    return Result;
}