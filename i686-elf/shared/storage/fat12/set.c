#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\storage\disk\disk.h"
#include "sources\i686-elf\libraries\strings\print.h"
#include "sources\i686-elf\libraries\strings\strings.h"
#include "sources\i686-elf\libraries\memory\memory.h"
#include "sources\i686-elf\libraries\memory\arena_allocator.h"
#include "sources\i686-elf\libraries\strings\path_handling.h"
#include "sources\i686-elf\libraries\storage\fat12\fat12.h"
#include "sources\i686-elf\libraries\storage\fat12\get.h"

void InitializeFat12RamDisk
(
    disk_parameters *DiskParameters,
    fat12_ram_disk *RamDisk
)
{
    ReadDiskSectors(DiskParameters, 0, 1, &RamDisk->BootSectorHeader);
    ReadDiskSectors(DiskParameters, 1, FAT12_SECTORS_IN_FAT, &RamDisk->Fat);
    ReadDiskSectors
    (
        DiskParameters,
        1 + (2 * FAT12_SECTORS_IN_FAT),
        FAT12_SECTORS_IN_ROOT_DIRECTORY,
        &RamDisk->RootDirectory
    );
}

void SetFatEntry(fat12_ram_disk *Disk, u32 ClusterNumber, u16 FatEntry)
{
    u32 StartingByteIndex = ClusterNumber * 3 / 2;

    if ((ClusterNumber % 2) == 0)
    {
        Disk->Fat.Bytes[StartingByteIndex] = (u8)FatEntry;
        Disk->Fat.Bytes[StartingByteIndex + 1] &= (u8)0xF0;
        Disk->Fat.Bytes[StartingByteIndex + 1] |= (u8)((FatEntry >> 8) & 0x000F);
    }
    else
    {
        Disk->Fat.Bytes[StartingByteIndex] &= (u8)0x0F;
        Disk->Fat.Bytes[StartingByteIndex] |= (u8)((FatEntry & 0x000F) << 4);
        Disk->Fat.Bytes[StartingByteIndex + 1] = (u8)((FatEntry >> 4) & 0x00FF);
    }
}

u16 AllocateDiskClusters
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    void *Memory,
    u32 Size
)
{
    u16 FirstAllocatedClusterNumber = 0;

    u32 ClustersNeeded = (Size + FAT12_SECTOR_SIZE - 1) / FAT12_SECTOR_SIZE;
    if (ClustersNeeded > CalculateFreeClusterNumbers(Disk))
    {
        return 0;
    }

    u32 SizeLeft = Size;
    char *ReadPointer = (char *)Memory;
    u16 PreviousClusterNumber = 0;

    for (u32 ClusterIndex = 0; ClusterIndex < ClustersNeeded; ClusterIndex++)
    {
        u16 ClusterNumber = GetFirstFreeClusterNumber(Disk);

        SetFatEntry(Disk, ClusterNumber, FAT12_FAT_ENTRY_RESERVED_CLUSTER);

        if (PreviousClusterNumber)
        {
            SetFatEntry(Disk, PreviousClusterNumber, ClusterNumber);
        }
        else
        {
            FirstAllocatedClusterNumber = ClusterNumber;
        }

        u16 SectorIndex = TranslateFatClusterNumberToSectorIndex(ClusterNumber);

        u32 BytesToCopy = FAT12_SECTOR_SIZE;
        u32 BytesToZero = 0;

        if (SizeLeft < BytesToCopy)
        {
            BytesToCopy = SizeLeft;
            BytesToZero = FAT12_SECTOR_SIZE - SizeLeft;
        }

        sector *RamSector = PushStruct(MemoryArena, sector);
        ReadDiskSectors(DiskParameters, SectorIndex, 1, RamSector->Bytes);

        if (ReadPointer)
        {
            MemoryCopy(RamSector->Bytes, ReadPointer, BytesToCopy);
            ReadPointer += BytesToCopy;
            MemoryZero(RamSector->Bytes + BytesToCopy, BytesToZero);
        }
        else
        {
            MemoryZero(RamSector->Bytes, BytesToCopy + BytesToZero);
        }

        WriteDiskSectors(DiskParameters, SectorIndex, 1, RamSector->Bytes);

        SizeLeft -= BytesToCopy;

        if (SizeLeft == 0)
        {
            SetFatEntry(Disk, ClusterNumber, FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_END);
        }

        PreviousClusterNumber = ClusterNumber;
    }

    return FirstAllocatedClusterNumber;
}

b8 AllocateFileToDirectoryEntry
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *DirectoryEntry,
    char *FileName,
    char *Extension,
    void *Memory,
    u32 Size
)
{
    u16 ClusterNumber = AllocateDiskClusters(Disk, MemoryArena, DiskParameters, Memory, Size);
    if (ClusterNumber)
    {
        MemoryZero(DirectoryEntry, sizeof(directory_entry));

        MemoryCopy(DirectoryEntry->FileName, FileName, 8);
        MemoryCopy(DirectoryEntry->FileExtension, Extension, 3);

        DirectoryEntry->FileAttributes = FAT12_FILE_ATTRIBUTE_NORMAL;
        DirectoryEntry->FileSize = Size;
        DirectoryEntry->ClusterNumberLowWord = ClusterNumber;
        return TRUE;
    }

    return FALSE;
}

b8 AllocateDirectoryToDirectoryEntry
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *DirectoryEntry,
    char *DirectoryName
)
{
    u16 ClusterNumber = AllocateDiskClusters(Disk, MemoryArena, DiskParameters, 0, FAT12_SECTOR_SIZE);
    if (ClusterNumber)
    {
        MemoryZero(DirectoryEntry, sizeof(directory_entry));

        MemoryCopy(DirectoryEntry->FileName, DirectoryName, 8);
        DirectoryEntry->FileAttributes = FAT12_FILE_ATTRIBUTE_DIRECTORY;
        DirectoryEntry->FileSize = 0;
        DirectoryEntry->ClusterNumberLowWord = ClusterNumber;
        return TRUE;
    }

    return FALSE;
}

directory_entry *
AddFileToRootDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FileName,
    char *Extension,
    void *Memory,
    u32 Size
)
{
    u16 FoundSectorLBA = 0;
    sector *FoundSector = NULL;
    directory_entry *FoundDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < FAT12_SECTORS_IN_ROOT_DIRECTORY; SectorIndex++)
    {
        FoundSectorLBA = 1 + (2 * FAT12_SECTORS_IN_FAT) + SectorIndex;
        FoundSector = &Disk->RootDirectory.Sectors[SectorIndex];
        FoundDirectoryEntry = GetFirstFreeDirectoryEntryInSector(FoundSector);

        if (FoundDirectoryEntry)
        {
            break;
        }
    }

    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b8 Result = AllocateFileToDirectoryEntry
    (
        Disk,
        MemoryArena,
        DiskParameters,
        FoundDirectoryEntry,
        FileName,
        Extension,
        Memory,
        Size
    );

    if (Result)
    {
        WriteDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *
AddDirectoryToRootDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *DirectoryName
)
{
    u16 FoundSectorLBA = 0;
    sector *FoundSector = NULL;
    directory_entry *FoundDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < FAT12_SECTORS_IN_ROOT_DIRECTORY; SectorIndex++)
    {
        FoundSectorLBA = 1 + (2 * FAT12_SECTORS_IN_FAT) + SectorIndex;
        FoundSector = &Disk->RootDirectory.Sectors[SectorIndex];
        FoundDirectoryEntry = GetFirstFreeDirectoryEntryInSector(FoundSector);

        if (FoundDirectoryEntry)
        {
            break;
        }
    }

    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b8 Result = AllocateDirectoryToDirectoryEntry
    (
        Disk,
        MemoryArena,
        DiskParameters,
        FoundDirectoryEntry,
        DirectoryName
    );

    if (Result)
    {
        WriteDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *
AddFileToDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *ParentDirectory,
    char *FileName,
    char *Extension,
    void *Memory,
    u32 Size
)
{
    u16 FoundSectorLBA = 0;
    sector *FoundSector = NULL;
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = ParentDirectory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        FoundSectorLBA = TranslateFatClusterNumberToSectorIndex(CurrentClusterNumber);
        FoundSector = PushStruct(MemoryArena, sector);
        ReadDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        FoundDirectoryEntry = GetFirstFreeDirectoryEntryInSector(FoundSector);

        if (FoundDirectoryEntry)
        {
            break;
        }
        else
        {
            if (IsFatEntryEndOfFile(CurrentFatEntry))
            {
                // TODO: extend the directory by allocating more sectors to it instead of aborting
                break;
            }
            else
            {
                CurrentClusterNumber = CurrentFatEntry;
                CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);
            }
        }
    }

    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b8 Result = AllocateFileToDirectoryEntry
    (
        Disk,
        MemoryArena,
        DiskParameters,
        FoundDirectoryEntry,
        FileName,
        Extension,
        Memory,
        Size
    );

    if (Result)
    {
        WriteDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *
AddDirectoryToDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *ParentDirectory,
    char *DirectoryName
)
{
    u16 FoundSectorLBA = 0;
    sector *FoundSector = NULL;
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = ParentDirectory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        FoundSectorLBA = TranslateFatClusterNumberToSectorIndex(CurrentClusterNumber);
        FoundSector = PushStruct(MemoryArena, sector);
        ReadDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        FoundDirectoryEntry = GetFirstFreeDirectoryEntryInSector(FoundSector);

        if (FoundDirectoryEntry)
        {
            break;
        }
        else
        {
            if (IsFatEntryEndOfFile(CurrentFatEntry))
            {
                // TODO: extend the directory by allocating more sectors to it instead of aborting
                break;
            }
            else
            {
                CurrentClusterNumber = CurrentFatEntry;
                CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);
            }
        }
    }

    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b8 Result = AllocateDirectoryToDirectoryEntry
    (
        Disk,
        MemoryArena,
        DiskParameters,
        FoundDirectoryEntry,
        DirectoryName
    );

    if (Result)
    {
        WriteDiskSectors(DiskParameters, FoundSectorLBA, 1, (u8 *)FoundSector);
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *
Fat12AddFileByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FullFilePath,
    void *Memory,
    u32 Size
)
{
    if (StringLength(FullFilePath) == 1)
    {
        return NULL;
    }

    file_path_node *CurrentPathNode = CreateFilePathSegmentList(FullFilePath, MemoryArena);

    char LocalFileName[8];
    char LocalFileExtension[3];
    MemoryZero(LocalFileName, 8);
    MemoryZero(LocalFileExtension, 3);

    GetFileNameAndExtensionFromString
    (
        CurrentPathNode->FileName, LocalFileName, 8, LocalFileExtension, 3
    );

    if (!CurrentPathNode->ChildNode)
    {
        directory_entry *FileDirectoryEntry = GetDirectoryEntryOfFileInRootDirectory
        (
            Disk,
            LocalFileName,
            LocalFileExtension
        );

        if (FileDirectoryEntry)
        {
            return NULL;
        }
        else
        {
            FileDirectoryEntry = AddFileToRootDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                LocalFileName,
                LocalFileExtension,
                Memory,
                Size
            );
            return FileDirectoryEntry;
        }
    }

    directory_entry *CurrentDirectoryEntry =
        GetDirectoryEntryOfDirectoryInRootDirectory(Disk, LocalFileName);
    CurrentPathNode = CurrentPathNode->ChildNode;

    while (CurrentPathNode)
    {

        MemoryZero(LocalFileName, ArrayCount(LocalFileName));
        MemoryZero(LocalFileExtension, ArrayCount(LocalFileExtension));

        GetFileNameAndExtensionFromString
        (
            CurrentPathNode->FileName, LocalFileName, 8, LocalFileExtension, 3
        );

        if (!CurrentPathNode->ChildNode)
        {
            directory_entry *FileDirectoryEntry = GetDirectoryEntryOfFileInDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                CurrentDirectoryEntry,
                LocalFileName,
                LocalFileExtension
            );

            if (FileDirectoryEntry)
            {
                return NULL;
            }
            else
            {
                FileDirectoryEntry = AddFileToDirectory
                (
                    Disk,
                    MemoryArena,
                    DiskParameters,
                    CurrentDirectoryEntry,
                    LocalFileName,
                    LocalFileExtension,
                    Memory,
                    Size
                );
                return FileDirectoryEntry;
            }
        }
        else
        {
            CurrentDirectoryEntry = GetDirectoryEntryOfDirectoryInDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                CurrentDirectoryEntry,
                LocalFileName
            );
            CurrentPathNode = CurrentPathNode->ChildNode;
        }
    }

    return NULL;
}

directory_entry *
Fat12AddDirectoryByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *DirectoryPath
)
{
    if (StringLength(DirectoryPath) == 1)
    {
        return NULL;
    }

    file_path_node *CurrentPathNode = CreateFilePathSegmentList(DirectoryPath, MemoryArena);

    if (!CurrentPathNode->ChildNode)
    {
        char LocalDirectoryName[8];
        MemoryZero(LocalDirectoryName, 8);

        FillFixedSizeStringBuffer
        (
            LocalDirectoryName,
            ArrayCount(LocalDirectoryName),
            CurrentPathNode->FileName
        );

        directory_entry *DirectoryDirectoryEntry = GetDirectoryEntryOfDirectoryInRootDirectory
        (
            Disk,
            LocalDirectoryName
        );

        if (DirectoryDirectoryEntry)
        {
            return NULL;
        }
        else
        {
            directory_entry *DirectoryDirectoryEntry = AddDirectoryToRootDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                LocalDirectoryName
            );
            return DirectoryDirectoryEntry;
        }
    }

    directory_entry *CurrentDirectoryEntry =
        GetDirectoryEntryOfDirectoryInRootDirectory(Disk, CurrentPathNode->FileName);
    CurrentPathNode = CurrentPathNode->ChildNode;

    while (CurrentPathNode)
    {
        if (!CurrentPathNode->ChildNode)
        {
            char LocalDirectoryName[8];
            MemoryZero(LocalDirectoryName, 8);

            FillFixedSizeStringBuffer
            (
                LocalDirectoryName,
                ArrayCount(LocalDirectoryName),
                CurrentPathNode->FileName
            );

            directory_entry *DirectoryDirectoryEntry = GetDirectoryEntryOfDirectoryInDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                CurrentDirectoryEntry,
                LocalDirectoryName
            );

            if (DirectoryDirectoryEntry)
            {
                return NULL;
            }
            else
            {
                DirectoryDirectoryEntry = AddDirectoryToDirectory
                (
                    Disk,
                    MemoryArena,
                    DiskParameters,
                    CurrentDirectoryEntry,
                    LocalDirectoryName
                );
                return DirectoryDirectoryEntry;
            }
        }
        else
        {
            CurrentDirectoryEntry = GetDirectoryEntryOfDirectoryInDirectory
            (
                Disk,
                MemoryArena,
                DiskParameters,
                CurrentDirectoryEntry,
                CurrentPathNode->FileName
            );
            CurrentPathNode = CurrentPathNode->ChildNode;
        }
    }

    return NULL;
}