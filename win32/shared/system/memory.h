#pragma once

#include <memory.h>
#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"

struct memory_arena
{
    u32 Size;
    u32 Used;
    u8 *BaseAddress;
};

inline void *PushOntoMemoryArena(memory_arena *MemoryArena, u32 PushSize, bool Zeroed)
{
    Assert(MemoryArena->Used + PushSize < MemoryArena->Size);
    void *Result = MemoryArena->BaseAddress + MemoryArena->Used;
    MemoryArena->Used += PushSize;
    if (Zeroed)
    {
        memset(Result, 0, PushSize);
    }
    return Result;
}