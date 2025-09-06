#include <stdarg.h>
#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\vga\vga.h"
#include "i686-elf\shared\memory\memory.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\cpu\gdt.h"
#include "i686-elf\shared\cpu\idt.h"
#include "i686-elf\os\kernel\linker.h"
#include "i686-elf\os\kernel\tests.h"
#include "i686-elf\os\kernel\descriptor_tables.h"

u8 FreeStore[KiloBytes(64)];
print_context GlobalPrintContext;

void __attribute__((section(".entry"))) Start()
{
    MemoryZero(&__bss_start, &__bss_end - &__bss_start);

    InitializeRamGDT();
    GlobalGDTDescriptor.Limit = sizeof(GlobalRamGDT) - 1;
    GlobalGDTDescriptor.Entry = (gdt_entry *)GlobalRamGDT;
    LoadGDT(&GlobalGDTDescriptor, 0x08, 0x10);

    InitializeRamIDT();
    EnableAllISRs();
    GlobalIDTDescriptor.Limit = sizeof(GlobalRamIDT) - 1;
    GlobalIDTDescriptor.FirstEntry = (idt_entry *)GlobalRamIDT;
    LoadIDT(&GlobalIDTDescriptor);

    ClearScreen();
    PrintString(&GlobalPrintContext, "\nHello from the kernel!\n\n");
    TestInterrupts();

    while (1) {};
}