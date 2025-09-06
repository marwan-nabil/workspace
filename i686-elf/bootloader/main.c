#include <stdarg.h>
#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\vga\vga.h"
#include "i686-elf\shared\storage\disk\disk.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\cpu\timing.h"
#include "i686-elf\shared\storage\fat12\fat12.h"
#include "i686-elf\shared\memory\arena_allocator.h"
#include "i686-elf\shared\storage\file_io\file_io.h"
#include "i686-elf\os\bootloader\memory_layout.h"
#include "i686-elf\os\bootloader\tests.h"

u8 FreeStore[KiloBytes(64)];
print_context GlobalPrintContext;
u8 *KernelLoadBuffer = MEMORY_LAYOUT_KERNEL_LOAD_ADDRESS;

void LoadKernel(u32 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n============ loading the kernel ============== \r\n");

    disk_parameters DiskParameters;
    GetDiskDriveParameters(&DiskParameters, BootDriveNumber);

    memory_arena LocalMemoryArena;
    InitializeMemoryArena
    (
        &LocalMemoryArena,
        KiloBytes(10),
        FreeMemoryArea
    );

    file_io_context *FileIoContext = PushStruct(&LocalMemoryArena, file_io_context);
    FileIoInitialize
    (
        BootDriveNumber,
        PushArray(&LocalMemoryArena, KiloBytes(16), u8),
        KiloBytes(16),
        PrintContext,
        FileIoContext
    );

    ListDirectory(FileIoContext, "\\");

    i16 FileHandle = FileOpen(FileIoContext, "\\kernel  .bin");
    FileRead(FileIoContext, FileHandle, MEMORY_LAYOUT_KERNEL_LOAD_SIZE, KernelLoadBuffer);
    FileClose(FileIoContext, FileHandle);

    SpinlockWait(400);

    typedef void (*kernel_start_function)();
    kernel_start_function KernelStart = (kernel_start_function)KernelLoadBuffer;
    KernelStart();
}

void __attribute__((cdecl)) cstart(u32 BootDriveNumber)
{
    // TestVGA(&GlobalPrintContext);
    // TestIO(&GlobalPrintContext);
    // StringTests(&GlobalPrintContext);
    // DiskDriverTests(BootDriveNumber, FreeStore, &GlobalPrintContext);
    // AllocatorTests(FreeStore, &GlobalPrintContext);
    // PathHandlingTests(FreeStore, &GlobalPrintContext);
    // Fat12Tests(BootDriveNumber, FreeStore, &GlobalPrintContext);
    FileIoTests(BootDriveNumber, FreeStore, &GlobalPrintContext);
    // LoadKernel(BootDriveNumber, &FreeStore, &GlobalPrintContext);

    while (1) {};
}