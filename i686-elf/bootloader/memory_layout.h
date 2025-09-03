#pragma once

// 0x00000000 - 0x000003FF: interrupt vector table [2,048 bytes]
// 0x00000400 - 0x000004FF: BIOS data area [256 bytes]
// 0x00000500 - 0x00010500: bootloader area [65,536 bytes]
//      0x00000500 - ... : code and data
//      ... - 0x0000FFF0: stack
// 0x00020000 - 0x00030000:
// 0x00030000 - 0x00080000: free memory
// 0x00080000 - 0x0009FFFF: Extended BIOS data area [131,072 bytes]
// 0x000A0000 - 0x000C7FFF: VGA Ram [163,840 bytes]
// 0x000C8000 - 0x000FFFFF: BIOS code [229,376 bytes]

#define MEMORY_LAYOUT_KERNEL_LOAD_ADDRESS ((void *)0x100000)
#define MEMORY_LAYOUT_KERNEL_LOAD_SIZE 0x00010000