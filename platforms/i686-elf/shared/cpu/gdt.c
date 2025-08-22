#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\cpu\gdt.h"

gdt_entry CreateGDTEntry(u32 Base, u32 Limit, u8 AccessField, u8 FlagsField)
{
    gdt_entry Result = {};
    Result.LimitLowPart = Limit & 0x0000FFFF;
    Result.BaseLowPart = Base & 0x00FFFFFF;
    Result.Access = AccessField;
    Result.LimitHighPart = (Limit & 0x00FF0000) >> 24;
    Result.Flags = FlagsField;
    Result.BaseHighPart = (Base & 0xFF000000) >> 24;
    return Result;
}