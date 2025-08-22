#pragma once

typedef struct
{
    u64 LimitLowPart: 16;
    u64 BaseLowPart: 24;
    u64 Access: 8;
    u64 LimitHighPart: 4;
    u64 Flags: 4;
    u64 BaseHighPart: 8;
} __attribute__((packed)) gdt_entry;

typedef struct
{
    u16 Limit;
    gdt_entry *Entry;
} __attribute__((packed)) gdt_descriptor;

typedef enum
{
    GDTAFO_ACCESSED_BIT = 0,
    GDTAFO_READ_WRITE_BIT = 1,
    GDTAFO_DIRECTION_CONFORMING_BIT = 2,
    GDTAFO_EXECUTABLE_BIT = 3,
    GDTAFO_TYPE_BIT = 4,
    GDTAFO_DESCRIPTOR_PRIVILIGE_LEVEL_BIT0 = 5,
    GDTAFO_DESCRIPTOR_PRIVILIGE_LEVEL_BIT1 = 6,
    GDTAFO_PRESENT_BIT = 7,
} gdt_access_field_offsets;

typedef enum
{
    GDTFFO_RESERVED_BIT = 0,
    GDTFFO_LONG_MODE_BIT = 1,
    GDTFFO_DB_BIT = 2,
    GDTFFO_GRANULARITY_BIT = 3,
} gdt_flags_field_offsets;

gdt_entry CreateGDTEntry(u32 Base, u32 Limit, u8 AccessField, u8 FlagsField);
void __attribute__((cdecl)) LoadGDT(gdt_descriptor *GDTDescriptor, u16 CodeSegmentOffset, u16 DataSegmentOffset);