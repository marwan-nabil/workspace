#include <stdarg.h>
#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\vga\vga.h"
#include "sources\i686-elf\libraries\storage\disk\disk.h"
#include "sources\i686-elf\libraries\strings\print.h"
#include "sources\i686-elf\libraries\cpu\timing.h"
#include "sources\i686-elf\libraries\storage\fat12\fat12.h"
#include "sources\i686-elf\libraries\memory\arena_allocator.h"
#include "sources\i686-elf\libraries\storage\file_io\file_io.h"
#include "sources\i686-elf\os\bootloader\memory_layout.h"
#include "sources\i686-elf\os\bootloader\tests.h"

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