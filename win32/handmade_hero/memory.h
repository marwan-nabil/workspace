#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"

#define PushStruct(Arena, DataType) (DataType *)PushOntoMemoryArena((Arena), sizeof(DataType))
#define PushArray(Arena, Count, DataType) (DataType *)PushOntoMemoryArena((Arena), (Count) * sizeof(DataType))
#define ZeroStruct(Struct) memset(&(Struct), 0, sizeof(Struct))

struct memory_arena
{
    size_t Size;
    size_t Used;
    u8 *BaseAddress;
};

inline void
InitializeMemoryArena(memory_arena *Arena, size_t Size, void *BaseAddress)
{
    Arena->Size = Size;
    Arena->Used = 0;
    Arena->BaseAddress = (u8 *)BaseAddress;
}

inline void *
PushOntoMemoryArena(memory_arena *Arena, size_t PushSize)
{
    Assert((Arena->Used + PushSize) <= Arena->Size);

    void *Result = Arena->BaseAddress + Arena->Used;
    Arena->Used += PushSize;
    return Result;
}