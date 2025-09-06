#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\cpu\idt.h"

idt_entry CreateIDTEntry
(
    u32 EntryAddress,
    u16 SegmentSelector,
    u8 Flags
)
{
    idt_entry Result;
    Result.EntryAddressLow = (u16)EntryAddress;
    Result.SegmentSelector = SegmentSelector;
    Result.Flags = Flags;
    Result.EntryAddressHigh = (u16)(EntryAddress >> 16);
    return Result;
}

void EnableInterruptGate(idt_entry *IDT, u8 InterruptNumber)
{
    IDT[InterruptNumber].Flags |= (1 << IDTFFO_PRESENT_BIT);
}

void DisableInterruptGate(idt_entry *IDT, u8 InterruptNumber)
{
    IDT[InterruptNumber].Flags &= ~(1 << IDTFFO_PRESENT_BIT);
}