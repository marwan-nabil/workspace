#pragma once

#define PushStruct(Arena, DataType) \
    (DataType *)PushOntoMemoryArena((Arena), sizeof(DataType))
#define PushArray(Arena, Count, DataType) \
    (DataType *)PushOntoMemoryArena((Arena), (Count) * sizeof(DataType))

typedef struct
{
    u32 Size;
    u32 Used;
    u8 *BaseAddress;
} memory_arena;

void InitializeMemoryArena(memory_arena *Arena, u32 Size, void *BaseAddress);
void FreeMemoryArena(memory_arena *Arena);
void PrintMemoryArenaUsage(memory_arena *Arena, print_context *PrintContext);
void *PushOntoMemoryArena(memory_arena *Arena, u32 PushSize);