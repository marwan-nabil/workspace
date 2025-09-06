#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\memory\arena_allocator.h"

void InitializeMemoryArena(memory_arena *Arena, u32 Size, void *BaseAddress)
{
    Arena->Size = Size;
    Arena->Used = 0;
    Arena->BaseAddress = (u8 *)BaseAddress;
}

void FreeMemoryArena(memory_arena *Arena)
{
    Arena->Used = 0;
}

void PrintMemoryArenaUsage(memory_arena *Arena, print_context *PrintContext)
{
    PrintFormatted(PrintContext, "Arena usage: %ld bytes.\r\n", Arena->Used);
}

void *PushOntoMemoryArena(memory_arena *Arena, u32 PushSize)
{
    void *Result = Arena->BaseAddress + Arena->Used;
    Arena->Used += PushSize;
    return Result;
}