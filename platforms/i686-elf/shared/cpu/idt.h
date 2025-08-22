#pragma once

typedef struct
{
    u64 EntryAddressLow: 16;
    u64 SegmentSelector: 16;
    u64 Reserved: 8;
    u64 Flags: 8;
    u64 EntryAddressHigh: 16;
} __attribute__((packed)) idt_entry;

typedef struct
{
    u16 Limit;
    idt_entry *FirstEntry;
} __attribute__((packed)) idt_descriptor;

typedef enum
{
    IDTFFO_GATE_TYPE_BIT0 = 0,
    IDTFFO_GATE_TYPE_BIT1 = 1,
    IDTFFO_GATE_TYPE_BIT2 = 2,
    IDTFFO_GATE_TYPE_BIT3 = 3,
    IDTFFO_ZEROED_BIT = 4,
    IDTFFO_PRIVILIGE_LEVEL_BIT0 = 5,
    IDTFFO_PRIVILIGE_LEVEL_BIT1 = 6,
    IDTFFO_PRESENT_BIT = 7,
} idt_flags_field_offsets;

#define IDT_GATE_TYPE_TASK 0x5
#define IDT_GATE_TYPE_16BIT_INTERRUPT 0x6
#define IDT_GATE_TYPE_16BIT_TRAP 0x7
#define IDT_GATE_TYPE_32BIT_INTERRUPT 0xE
#define IDT_GATE_TYPE_32BIT_TRAP 0xF

#define IDT_PRIVILIGE_LEVEL_RING0 (0 << IDTFFO_PRIVILIGE_LEVEL_BIT0)
#define IDT_PRIVILIGE_LEVEL_RING1 (1 << IDTFFO_PRIVILIGE_LEVEL_BIT0)
#define IDT_PRIVILIGE_LEVEL_RING2 (2 << IDTFFO_PRIVILIGE_LEVEL_BIT0)
#define IDT_PRIVILIGE_LEVEL_RING3 (3 << IDTFFO_PRIVILIGE_LEVEL_BIT0)

#define IDT_PRESENT 0x80

idt_entry CreateIDTEntry(u32 EntryAddress, u16 SegmentSelector, u8 Flags);
void EnableInterruptGate(idt_entry *IDT, u8 InterruptNumber);
void DisableInterruptGate(idt_entry *IDT, u8 InterruptNumber);
void __attribute__((cdecl)) LoadIDT(idt_descriptor *IDTDescriptor);