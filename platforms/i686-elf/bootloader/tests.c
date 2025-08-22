#include <stdarg.h>
#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\strings\print.h"
#include "sources\i686-elf\libraries\vga\vga.h"
#include "sources\i686-elf\libraries\cpu\timing.h"
#include "sources\i686-elf\libraries\memory\memory.h"
#include "sources\i686-elf\libraries\strings\strings.h"
#include "sources\i686-elf\libraries\storage\disk\disk.h"
#include "sources\i686-elf\libraries\storage\fat12\fat12.h"
#include "sources\i686-elf\libraries\memory\arena_allocator.h"
#include "sources\i686-elf\libraries\strings\path_handling.h"
#include "sources\i686-elf\libraries\storage\fat12\get.h"
#include "sources\i686-elf\libraries\storage\fat12\set.h"
#include "sources\i686-elf\libraries\storage\file_io\file_io.h"

void TestVGA(print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 10);

    PrintString(PrintContext, "\r\n============ VGA driver tests ============== \r\n");
    for (u32 Y = 0; Y < VGA_SCREEN_HEIGHT; Y++)
    {
        for (u32 X = 0; X < VGA_SCREEN_WIDTH; X++)
        {
            u8 Color = X + Y;
            char Character = X + Y;
            PrintCharacterColored(Character, Color, X, Y);
            SpinlockWait(100);
        }
    }

    ScrollBack(2);
}

void TestIO(print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n============ IO library tests ============== \r\n");
}

void StringTests(print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);

    char *FarString = "far string";
    char *FarString2 = "aaaaaaa";

    PrintString(PrintContext, "\r\n============ formatted print tests ============== \r\n");
    PrintFormatted
    (
        PrintContext,
        "Formatted %% %c %s %s\r\n",
        'a', "string", FarString
    );
    PrintFormatted
    (
        PrintContext,
        "Formatted %d %i %x %p %o %hd %hi %hhu %hhd\r\n",
        1234, -5678, 0xdead, 0xbeef, 012345, (i16)27,
        (i16)-42, (u8)20, (i8)-10
    );
    PrintFormatted
    (
        PrintContext,
        "Formatted %d %x %lld %llx\r\n",
        -100000, 0xdeadbeef, 10200300400ll, 0xdeadbeeffeebdaed
    );

    PrintString(PrintContext, "\r\n=========== memory utils tests ================= \r\n");
    PrintFormatted(PrintContext, "Test MemoryZero Before: %s\r\n", FarString);
    MemoryZero(FarString, StringLength(FarString));
    PrintFormatted(PrintContext, "Test MemoryZero After: %s\r\n", FarString);
    MemoryCopy(FarString, FarString2, StringLength(FarString2));
    PrintFormatted(PrintContext, "Test MemoryCopy After: %s\r\n", FarString);

    PrintString(PrintContext, "\r\n============ other string tests ============== \r\n");
    char LocalPathBuffer[1024];
    MemoryZero(LocalPathBuffer, ArrayCount(LocalPathBuffer));

    StringConcatenate(LocalPathBuffer, ArrayCount(LocalPathBuffer), "test ");
    StringConcatenate(LocalPathBuffer, ArrayCount(LocalPathBuffer), "string");
    PrintFormatted(PrintContext, "Test string after concatenation: %s\r\n", LocalPathBuffer);
}

void DiskDriverTests(u8 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n=========== disk driver tests ================= \r\n");

    disk_parameters DiskParameters;
    GetDiskDriveParameters(&DiskParameters, BootDriveNumber);

    PrintFormatted(PrintContext, "Basic Disk Drive parameters:\r\n");
    PrintFormatted(PrintContext, "ID: %hhd\r\n", DiskParameters.Id);
    PrintFormatted(PrintContext, "Type: %hhd\r\n", DiskParameters.Type);
    PrintFormatted(PrintContext, "Cylinders: %hd\r\n", DiskParameters.Cylinders);
    PrintFormatted(PrintContext, "Heads: %hd\r\n", DiskParameters.Heads);
    PrintFormatted(PrintContext, "Sectors: %hd\r\n", DiskParameters.Sectors);
    PrintFormatted(PrintContext, "\r\n");

    PrintFormatted(PrintContext, "LBA to CHS translation examples:\r\n");
    u16 LBA, Cylinder, Head, Sector;
    for (u16 Index = 0; Index < 3; Index++)
    {
        LBA = Index;
        TranslateLbaToChs(&DiskParameters, LBA, &Cylinder, &Head, &Sector);
        PrintFormatted(PrintContext, "LBA: %hd    CHS: %hd  %hd  %hd\r\n", LBA, Cylinder, Head, Sector);
        SpinlockWait(100);
    }
    PrintFormatted(PrintContext, "\r\n");

    PrintFormatted(PrintContext, "reading from disk:\r\n");
    ReadDiskSectors(&DiskParameters, 0, 1, FreeMemoryArea);

    char OEMName[9];
    MemoryZero(OEMName, ArrayCount(OEMName));
    MemoryCopy
    (
        OEMName,
        &((boot_sector *)FreeMemoryArea)->OEMName,
        ArrayCount(OEMName) - 1
    );
    PrintFormatted(PrintContext, "BootSector OEM name: %s\r\n", OEMName);

    u16 BootSignature = 0;
    MemoryCopy
    (
        &BootSignature,
        &((boot_sector *)FreeMemoryArea)->BootSectorSignature,
        sizeof(u16)
    );
    PrintFormatted(PrintContext, "BootSector boot signature: %hx\r\n", BootSignature);
    PrintFormatted(PrintContext, "\r\n");

    PrintFormatted(PrintContext, "modifying OEM name then writing to disk.\r\n");
    ((boot_sector *)FreeMemoryArea)->OEMName[0] = 'K';
    WriteDiskSectors(&DiskParameters, 0, 1, FreeMemoryArea);

    PrintFormatted(PrintContext, "reading the bootsector back from disk.\r\n");
    ReadDiskSectors(&DiskParameters, 0, 1, FreeMemoryArea);

    MemoryZero(OEMName, ArrayCount(OEMName));
    MemoryCopy
    (
        OEMName,
        &((boot_sector *)FreeMemoryArea)->OEMName,
        ArrayCount(OEMName) - 1
    );
    PrintFormatted(PrintContext, "modified BootSector OEM name: %s\r\n", OEMName);
}

void AllocatorTests(void *FreeMemoryAddress, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n=========== memory allocator tests ================= \r\n");

    memory_arena LocalMemoryArena;
    InitializeMemoryArena
    (
        &LocalMemoryArena,
        KiloBytes(10),
        FreeMemoryAddress
    );

    typedef struct
    {
        u32 A;
        u32 B;
        u32 C;
    } my_struct;

    my_struct *MyStructInstance = PushStruct(&LocalMemoryArena, my_struct);
    my_struct *MyStructInstance2 = PushStruct(&LocalMemoryArena, my_struct);
    my_struct *MyStructInstance3 = PushStruct(&LocalMemoryArena, my_struct);
    PrintFormatted(PrintContext, "Pointer of first object allocated:  0x%x\r\n", (u32)MyStructInstance);
    PrintFormatted(PrintContext, "Pointer of second object allocated: 0x%x\r\n", (u32)MyStructInstance2);
    PrintFormatted(PrintContext, "Pointer of third object allocated:  0x%x\r\n", (u32)MyStructInstance3);
}

void PathHandlingTests(void *FreeMemoryArea, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n=========== path handling tests ================= \r\n");

    memory_arena LocalMemoryArena;
    InitializeMemoryArena
    (
        &LocalMemoryArena,
        KiloBytes(10),
        FreeMemoryArea
    );

    file_path_node *PathListHead = CreateFilePathSegmentList
    (
        "\\aaa\\bbb\\ccc",
        &LocalMemoryArena
    );
    PrintFormatted
    (
        PrintContext,
        "reconstructed path: %ls %ls %ls\r\n",
        PathListHead->FileName,
        PathListHead->ChildNode->FileName,
        PathListHead->ChildNode->ChildNode->FileName
    );

    char *LocalString = "\\xxx\\yyy\\zzz";
    PrintFormatted(PrintContext, "local string before trimming: %ls\r\n", LocalString);
    RemoveLastSegmentFromPath(LocalString);
    PrintFormatted(PrintContext, "local string after trimming: %ls\r\n", LocalString);
}

void Fat12Tests(u16 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n=========== fat12 tests ================= \r\n");

    disk_parameters DiskParameters;
    GetDiskDriveParameters(&DiskParameters, BootDriveNumber);

    memory_arena LocalMemoryArena;
    InitializeMemoryArena
    (
        &LocalMemoryArena,
        KiloBytes(10),
        FreeMemoryArea
    );

    fat12_ram_disk *RamDisk = PushStruct(&LocalMemoryArena, fat12_ram_disk);
    InitializeFat12RamDisk(&DiskParameters, RamDisk);

    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\"
    );
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\Dir0"
    );
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\Dir0\\Dir1"
    );
    directory_entry *FileHandle = GetDirectoryEntryOfFileByPath
    (
        RamDisk, &LocalMemoryArena, &DiskParameters, "\\Dir0\\Dir1\\sample.txt"
    );

    PrintFormatted(PrintContext, "\r\n");
    PrintFormatted(PrintContext, "name of opened file handle: %ls\r\n", FileHandle->FileName);

    PrintFormatted(PrintContext, "\r\n");
    PrintFormatted(PrintContext, "creating a new file in root directory...\r\n");
    Fat12AddFileByPath(RamDisk, &LocalMemoryArena, &DiskParameters, "\\test.txt", NULL, 30);
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\"
    );

    PrintFormatted(PrintContext, "\r\n");
    PrintFormatted(PrintContext, "create a new file in sub directory.\r\n");
    Fat12AddFileByPath(RamDisk, &LocalMemoryArena, &DiskParameters, "\\Dir0\\test.txt", NULL, 400);
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\Dir0"
    );

    PrintFormatted(PrintContext, "\r\n");
    PrintFormatted(PrintContext, "reload the disk to see if file system modifications are persistent.\r\n");
    InitializeFat12RamDisk(&DiskParameters, RamDisk);

    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\"
    );
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\Dir0"
    );
    Fat12ListDirectory
    (
        RamDisk,
        &LocalMemoryArena,
        &DiskParameters,
        PrintContext,
        "\\Dir0\\Dir1"
    );
}

void FileIoTests(u16 BootDriveNumber, void *FreeMemoryArea, print_context *PrintContext)
{
    ClearScreen();
    SetCursorPosition(PrintContext, 0, 0);
    PrintString(PrintContext, "\r\n=========== File IO tests ================= \r\n");

    memory_arena LocalMemoryArena;
    InitializeMemoryArena
    (
        &LocalMemoryArena,
        KiloBytes(64),
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
    ListDirectory(FileIoContext, "\\Dir0");
    ListDirectory(FileIoContext, "\\Dir0\\Dir1");

    i16 FileHandle = FileOpen(FileIoContext, "\\Dir0\\Dir1\\sample.txt");

    char LocalStringBuffer[30];
    MemoryZero(LocalStringBuffer, 30);

    u32 FileOffset = 0;

    FileSeek(FileIoContext, FileHandle, FileOffset);
    FileRead(FileIoContext, FileHandle, 29, (u8 *)LocalStringBuffer);
    PrintFormatted(PrintContext, "Data read from the file: %s\r\n", LocalStringBuffer);

    PrintFormatted(PrintContext, "overwriting the same data read before with A's ...\r\n");

    MemorySet(LocalStringBuffer, 'A', 29);
    FileSeek(FileIoContext, FileHandle, FileOffset);
    FileWrite(FileIoContext, FileHandle, 30, (u8 *)LocalStringBuffer);

    FileSeek(FileIoContext, FileHandle, FileOffset);
    FileRead(FileIoContext, FileHandle, 30, (u8 *)LocalStringBuffer);
    PrintFormatted(PrintContext, "Data read from the file: %s\r\n", LocalStringBuffer);
}