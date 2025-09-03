#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\storage\disk\disk.h"
#include "sources\i686-elf\libraries\strings\print.h"
#include "sources\i686-elf\libraries\memory\memory.h"
#include "sources\i686-elf\libraries\memory\arena_allocator.h"
#include "sources\i686-elf\libraries\storage\fat12\fat12.h"
#include "sources\i686-elf\libraries\storage\fat12\get.h"
#include "sources\i686-elf\libraries\storage\fat12\set.h"
#include "sources\i686-elf\libraries\storage\file_io\file_io.h"

void FileIoInitialize
(
    u16 DriveNumber,
    void *TransientMemoryAddress,
    u32 TransientMemorySize,
    print_context *PrintContext,
    file_io_context *Context
)
{
    GetDiskDriveParameters(&Context->DiskParameters, DriveNumber);

    InitializeMemoryArena
    (
        &Context->TransientMemoryArena,
        TransientMemorySize,
        TransientMemoryAddress
    );

    InitializeFat12RamDisk(&Context->DiskParameters, &Context->Fat12RamDisk);

    Context->OpenFilesCount = 0;
    for (u16 Index = 0; Index < FILE_IO_MAX_OPEN_FILES; Index++)
    {
        Context->OpenFiles[Index].IsOpen = FALSE;
    }

    Context->PrintContext = PrintContext;
}

i16 FileOpen(file_io_context *Context, char *FilePath)
{
    if (Context->OpenFilesCount >= FILE_IO_MAX_OPEN_FILES)
    {
        return FILE_IO_INVALID_HANDLE;
    }

    i16 FileHandle = FILE_IO_INVALID_HANDLE;

    for (u16 Index = 0; Index < FILE_IO_MAX_OPEN_FILES; Index++)
    {
        if (!Context->OpenFiles[Index].IsOpen)
        {
            FileHandle = Index;
        }
    }

    if (FileHandle == FILE_IO_INVALID_HANDLE)
    {
        return FILE_IO_INVALID_HANDLE;
    }

    file_io_file *OpenedFile = &Context->OpenFiles[FileHandle];
    directory_entry *FileDirectoryEntry = GetDirectoryEntryOfFileByPath
    (
        &Context->Fat12RamDisk,
        &Context->TransientMemoryArena,
        &Context->DiskParameters,
        FilePath
    );

    if (!FileDirectoryEntry)
    {
        return FILE_IO_INVALID_HANDLE;
    }

    Context->OpenFilesCount++;
    OpenedFile->Handle = FileHandle;
    if
    (
        (FileDirectoryEntry->FileAttributes & FAT12_FILE_ATTRIBUTE_DIRECTORY) ==
        FAT12_FILE_ATTRIBUTE_DIRECTORY
    )
    {
        OpenedFile->IsDirectory = TRUE;
    }
    else
    {
        OpenedFile->IsDirectory = FALSE;
    }
    OpenedFile->IsOpen = TRUE;
    OpenedFile->Position = 0;
    OpenedFile->Size = FileDirectoryEntry->FileSize;
    OpenedFile->FirstCluster = FileDirectoryEntry->ClusterNumberLowWord;
    OpenedFile->LoadedCluster = OpenedFile->FirstCluster;

    GetDiskSectorFromFatClusterNumber
    (
        &Context->DiskParameters,
        (sector *)OpenedFile->Buffer,
        OpenedFile->FirstCluster
    );

    FreeMemoryArena(&Context->TransientMemoryArena);

    return FileHandle;
}

void FileRead(file_io_context *Context, i16 FileHandle, u32 ByteCount, u8 *DataOut)
{
    file_io_file *File = &Context->OpenFiles[FileHandle];

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        u32 OffsetInSector = File->Position % FAT12_SECTOR_SIZE;
        if ((OffsetInSector + ByteCount) < FAT12_SECTOR_SIZE)
        {
            MemoryCopy(DataOut, &File->Buffer[OffsetInSector], ByteCount);
            File->Position += ByteCount;
            return;
        }
        else
        {
            u32 BytesToRead = FAT12_SECTOR_SIZE - OffsetInSector;
            MemoryCopy(DataOut, &File->Buffer[OffsetInSector], BytesToRead);

            File->Position += BytesToRead;
            ByteCount -= BytesToRead;
            DataOut += BytesToRead;

            u16 NextCluster = GetFatEntryFromClusterNumber(&Context->Fat12RamDisk, File->LoadedCluster);
            if (IsFatEntryEndOfFile(NextCluster))
            {
                return;
            }
            else
            {
                GetDiskSectorFromFatClusterNumber
                (
                    &Context->DiskParameters,
                    (sector *)File->Buffer,
                    NextCluster
                );
                File->LoadedCluster = NextCluster;
            }
        }
    }
}

void FileWrite(file_io_context *Context, i16 FileHandle, u32 ByteCount, u8 *DataIn)
{
    file_io_file *File = &Context->OpenFiles[FileHandle];

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        u32 OffsetInSector = File->Position % FAT12_SECTOR_SIZE;
        if ((OffsetInSector + ByteCount) < FAT12_SECTOR_SIZE)
        {
            MemoryCopy(&File->Buffer[OffsetInSector], DataIn, ByteCount);
            WriteDiskSectors
            (
                &Context->DiskParameters,
                TranslateFatClusterNumberToSectorIndex(File->LoadedCluster),
                1,
                File->Buffer
            );
            File->Position += ByteCount;
            return;
        }
        else
        {
            u32 BytesToWrite = FAT12_SECTOR_SIZE - OffsetInSector;
            MemoryCopy(&File->Buffer[OffsetInSector], DataIn, BytesToWrite);
            WriteDiskSectors
            (
                &Context->DiskParameters,
                TranslateFatClusterNumberToSectorIndex(File->LoadedCluster),
                1,
                File->Buffer
            );

            File->Position += BytesToWrite;
            ByteCount -= BytesToWrite;
            DataIn += BytesToWrite;

            u16 NextCluster = GetFatEntryFromClusterNumber(&Context->Fat12RamDisk, File->LoadedCluster);
            if (IsFatEntryEndOfFile(NextCluster))
            {
                return;
            }
            else
            {
                GetDiskSectorFromFatClusterNumber
                (
                    &Context->DiskParameters,
                    (sector *)File->Buffer,
                    NextCluster
                );
                File->LoadedCluster = NextCluster;
            }
        }
    }
}

void FileSeek(file_io_context *Context, i16 FileHandle, u32 NewSeekPosition)
{
    file_io_file *File = &Context->OpenFiles[FileHandle];

    if (NewSeekPosition < File->Size)
    {
        u32 SectorIndex = NewSeekPosition / FAT12_SECTOR_SIZE;
        u16 CurrentCluster = File->FirstCluster;

        while (SectorIndex)
        {
            CurrentCluster = GetFatEntryFromClusterNumber(&Context->Fat12RamDisk, CurrentCluster);
            SectorIndex--;
        }

        GetDiskSectorFromFatClusterNumber
        (
            &Context->DiskParameters,
            (sector *)File->Buffer,
            CurrentCluster
        );

        File->LoadedCluster = CurrentCluster;
        File->Position = NewSeekPosition;
    }
}

u32 FileGetPosition(file_io_context *Context, i16 FileHandle)
{
    file_io_file *File = &Context->OpenFiles[FileHandle];
    return File->Position;
}

void FileClose(file_io_context *Context, i16 FileHandle)
{
    file_io_file *File = &Context->OpenFiles[FileHandle];
    File->IsOpen = FALSE;
}

void ListDirectory(file_io_context *Context, char *DirectoryFilePath)
{
    Fat12ListDirectory
    (
        &Context->Fat12RamDisk,
        &Context->TransientMemoryArena,
        &Context->DiskParameters,
        Context->PrintContext,
        DirectoryFilePath
    );
    FreeMemoryArena(&Context->TransientMemoryArena);
}