#pragma once

void MemorySet(void *Destination, u8 Value, u32 Size);
void MemoryZero(void *Destination, u32 Size);
void MemoryCopy(void *Destination, void *Source, u32 Size);
i16 MemoryCompare(void *Source1, void *Source2, u32 Size);
u32 AlignPointer(u32 Pointer, u32 Alignment);