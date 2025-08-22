#pragma once

#define FILE_IO_MAX_OPEN_FILES 10
#define FILE_IO_INVALID_HANDLE -1

typedef struct
{
    u32 Position;
    u32 Size;

    i16 Handle;
    u16 FirstCluster;
    u16 LoadedCluster;

    b8 IsDirectory;
    b8 IsOpen;

    u8 Buffer[FAT12_SECTOR_SIZE];
} file_io_file;

typedef struct
{
    fat12_ram_disk Fat12RamDisk;
    disk_parameters DiskParameters;
    memory_arena TransientMemoryArena;
    print_context *PrintContext;

    file_io_file OpenFiles[FILE_IO_MAX_OPEN_FILES];
    u8 OpenFilesCount;
} file_io_context;

void FileIoInitialize
(
    u16 DriveNumber,
    void *TransientMemoryAddress,
    u32 TransientMemorySize,
    print_context *PrintContext,
    file_io_context *Context
);
i16 FileOpen(file_io_context *Context, char *FilePath);
void FileRead(file_io_context *Context, i16 FileHandle, u32 ByteCount, u8 *DataOut);
void FileWrite(file_io_context *Context, i16 FileHandle, u32 ByteCount, u8 *DataIn);
void FileSeek(file_io_context *Context, i16 FileHandle, u32 NewSeekPosition);
u32 FileGetPosition(file_io_context *Context, i16 FileHandle);
void FileClose(file_io_context *Context, i16 FileHandle);
void ListDirectory(file_io_context *Context, char *DirectoryFilePath);