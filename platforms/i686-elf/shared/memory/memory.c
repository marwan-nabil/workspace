#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"

void MemorySet(void *Destination, u8 Value, u32 Size)
{
    for (u16 Index = 0; Index < Size; Index++)
    {
        ((u8 *)Destination)[Index] = Value;
    }
}

void MemoryZero(void *Destination, u32 Size)
{
    MemorySet(Destination, 0, Size);
}

void MemoryCopy(void *Destination, void *Source, u32 Size)
{
    u32 Index = 0;
    for (; Index < Size; Index++)
    {
        ((u8 *)Destination)[Index] = ((u8 *)Source)[Index];
    }
}

i16 MemoryCompare(void *Source1, void *Source2, u32 Size)
{
    for (u16 Index = 0; Index < Size; Index++)
    {
        if
        (
            ((u8 *)Source1)[Index] !=
            ((u8 *)Source2)[Index]
        )
        {
            return 1;
        }
    }

    return 0;
}

u32 AlignPointer(u32 Pointer, u32 Alignment)
{
    if (Alignment == 0)
    {
        return Pointer;
    }

    u32 Remainder = Pointer % Alignment;
    if (Remainder > 0)
    {
        return Pointer + (Alignment - Remainder);
    }
    else
    {
        return Pointer;
    }
}