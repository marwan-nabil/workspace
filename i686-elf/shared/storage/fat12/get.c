#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\storage\disk\disk.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\strings\strings.h"
#include "i686-elf\shared\memory\memory.h"
#include "i686-elf\shared\memory\arena_allocator.h"
#include "i686-elf\shared\strings\path_handling.h"
#include "i686-elf\shared\storage\fat12\fat12.h"

b8 IsFatEntryEndOfFile(u16 FatEntry)
{
    if
    (
        (FatEntry >= FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_START) &&
        (FatEntry <= FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_END)
    )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

u16 TranslateFatClusterNumberToSectorIndex(u16 ClusterNumber)
{
    u16 PhysicalSectorIndex = FAT12_DATA_AREA_START_SECTOR + ClusterNumber - 2;
    return PhysicalSectorIndex;
}

u16 TranslateSectorIndexToFatClusterNumber(u16 SectorIndex)
{
    u16 ClusterNumber = SectorIndex + 2 - FAT12_DATA_AREA_START_SECTOR;
    return ClusterNumber;
}

void GetDiskSectorFromFatClusterNumber
(
    disk_parameters *DiskParameters,
    sector *SectorLocation,
    u16 ClusterNumber
)
{
    u16 LBA = TranslateFatClusterNumberToSectorIndex(ClusterNumber);
    ReadDiskSectors(DiskParameters, LBA, 1, (u8 *)SectorLocation);
}

u16 GetFatEntryFromClusterNumber(fat12_ram_disk *Disk, u32 ClusterNumber)
{
    u16 Result = 0;
    u32 StartingByteIndex = ClusterNumber * 3 / 2;

    if ((ClusterNumber % 2) == 0)
    {
        // Result = (*(u16 *)&Disk->Fat.Bytes[StartingByteIndex]) & 0x0FFF;
        Result = (u16)Disk->Fat.Bytes[StartingByteIndex];
        Result |= ((u16)Disk->Fat.Bytes[StartingByteIndex + 1] & 0x000F) << 8;
    }
    else
    {
        Result = (u16)(Disk->Fat.Bytes[StartingByteIndex] & 0xF0) >> 4;
        Result |= ((u16)Disk->Fat.Bytes[StartingByteIndex + 1]) << 4;
    }

    return Result;
}

u16 GetFirstFreeClusterNumber(fat12_ram_disk *Disk)
{
    for (u16 ClusterNumber = 2; ClusterNumber < FAT12_FAT_ENTRIES_IN_FAT; ClusterNumber++)
    {
        u16 FatEntry = GetFatEntryFromClusterNumber(Disk, ClusterNumber);
        if (FatEntry == FAT12_FAT_ENTRY_FREE_CLUSTER)
        {
            return ClusterNumber;
        }
    }
    return 0;
}

u32 CalculateFreeClusterNumbers(fat12_ram_disk *Disk)
{
    u32 Result = 0;

    for (u16 ClusterNumber = 2; ClusterNumber < FAT12_FAT_ENTRIES_IN_FAT; ClusterNumber++)
    {
        u16 FatEntry = GetFatEntryFromClusterNumber(Disk, ClusterNumber);
        if (FatEntry == FAT12_FAT_ENTRY_FREE_CLUSTER)
        {
            Result++;
        }
    }

    return Result;
}

directory_entry *
GetFirstFreeDirectoryEntryInSector(sector *Sector)
{
    for
    (
        u32 DirectoryEntryIndex = 0;
        DirectoryEntryIndex < FAT12_DIRECTORY_ENTRIES_IN_SECTOR;
        DirectoryEntryIndex++
    )
    {
        directory_entry *DirectoryEntry = &Sector->DirectoryEntries[DirectoryEntryIndex];
        if (DirectoryEntry->FileName[0] == FAT12_FILENAME_EMPTY_SLOT)
        {
            return DirectoryEntry;
        }
    }

    return NULL;
}

directory_entry *
GetDirectoryEntryOfFileInSector(sector *Sector, char *FileName, char *Extension)
{
    for
    (
        u32 DirectoryEntryIndex = 0;
        DirectoryEntryIndex < ArrayCount(Sector->DirectoryEntries);
        DirectoryEntryIndex++
    )
    {
        directory_entry *DirectoryEntry = &Sector->DirectoryEntries[DirectoryEntryIndex];
        if
        (
            (MemoryCompare(DirectoryEntry->FileName, FileName, 8) == 0) &&
            (MemoryCompare(DirectoryEntry->FileExtension, Extension, 3) == 0)
        )
        {
            return DirectoryEntry;
        }
    }

    return NULL;
}

directory_entry *
GetDirectoryEntryOfDirectoryInSector(sector *Sector, char *DirectoryName)
{
    for
    (
        u32 DirectoryEntryIndex = 0;
        DirectoryEntryIndex < FAT12_DIRECTORY_ENTRIES_IN_SECTOR;
        DirectoryEntryIndex++
    )
    {
        directory_entry *DirectoryEntry = &Sector->DirectoryEntries[DirectoryEntryIndex];

        if
        (
            StringCompare
            (
                DirectoryEntry->FileName,
                DirectoryName,
                StringLength(DirectoryName)
            )
            ==
            0
        )
        {
            return DirectoryEntry;
        }
    }

    return NULL;
}

directory_entry *
GetDirectoryEntryOfFileInDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *Directory,
    char *FileName,
    char *Extension
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = Directory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        sector *PhysicalSector = PushStruct(MemoryArena, sector);
        GetDiskSectorFromFatClusterNumber
        (
            DiskParameters,
            PhysicalSector,
            CurrentClusterNumber
        );
        FoundDirectoryEntry = GetDirectoryEntryOfFileInSector(PhysicalSector, FileName, Extension);

        if (FoundDirectoryEntry)
        {
            break;
        }
        else
        {
            if (IsFatEntryEndOfFile(CurrentFatEntry))
            {
                break;
            }
            else
            {
                CurrentClusterNumber = CurrentFatEntry;
                CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);
            }
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfDirectoryInDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *Directory,
    char *DirectoryName
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = Directory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        sector *PhysicalSector = PushStruct(MemoryArena, sector);
        GetDiskSectorFromFatClusterNumber
        (
            DiskParameters,
            PhysicalSector,
            CurrentClusterNumber
        );
        FoundDirectoryEntry = GetDirectoryEntryOfDirectoryInSector(PhysicalSector, DirectoryName);

        if (FoundDirectoryEntry)
        {
            break;
        }
        else
        {
            if (IsFatEntryEndOfFile(CurrentFatEntry))
            {
                break;
            }
            else
            {
                CurrentClusterNumber = CurrentFatEntry;
                CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);
            }
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfFileInRootDirectory
(
    fat12_ram_disk *Disk,
    char *FileName,
    char *Extension
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < ArrayCount(Disk->RootDirectory.Sectors); SectorIndex++)
    {
        FoundDirectoryEntry =
            GetDirectoryEntryOfFileInSector(&Disk->RootDirectory.Sectors[SectorIndex], FileName, Extension);

        if (FoundDirectoryEntry)
        {
            break;
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfDirectoryInRootDirectory
(
    fat12_ram_disk *Disk,
    char *DirectoryName
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < FAT12_SECTORS_IN_ROOT_DIRECTORY; SectorIndex++)
    {
        FoundDirectoryEntry =
            GetDirectoryEntryOfDirectoryInSector(&Disk->RootDirectory.Sectors[SectorIndex], DirectoryName);

        if (FoundDirectoryEntry)
        {
            break;
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfFileByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FullFilePath
)
{
    if (StringLength(FullFilePath) == 1)
    {
        return NULL;
    }

    file_path_node *FirstNode = CreateFilePathSegmentList(FullFilePath, MemoryArena);
    if (!FirstNode)
    {
        return NULL;
    }

    file_path_node *CurrentNode = FirstNode;

    char LocalFileName[8];
    char LocalFileExtension[3];
    MemoryZero(LocalFileName, 8);
    MemoryZero(LocalFileExtension, 3);

    GetFileNameAndExtensionFromString
    (
        CurrentNode->FileName,
        (char *)LocalFileName, 8,
        (char *)LocalFileExtension, 3
    );

    directory_entry *CurrentEntry = GetDirectoryEntryOfFileInRootDirectory
    (
        Disk,
        (char *)LocalFileName,
        (char *)LocalFileExtension
    );

    if (CurrentEntry && !CurrentNode->ChildNode)
    {
        return CurrentEntry;
    }

    CurrentNode = CurrentNode->ChildNode;

    while (CurrentNode)
    {
        MemoryZero(LocalFileName, ArrayCount(LocalFileName));
        MemoryZero(LocalFileExtension, ArrayCount(LocalFileExtension));

        GetFileNameAndExtensionFromString
        (
            CurrentNode->FileName,
            (char *)LocalFileName, 8,
            (char *)LocalFileExtension, 3
        );

        CurrentEntry = GetDirectoryEntryOfFileInDirectory
        (
            Disk,
            MemoryArena,
            DiskParameters,
            CurrentEntry,
            LocalFileName,
            LocalFileExtension
        );

        if (CurrentEntry && !CurrentNode->ChildNode)
        {
            return CurrentEntry;
        }

        CurrentNode = CurrentNode->ChildNode;
    }

    return NULL;
}

void Fat12ListDirectorySector(sector *Sector, print_context *PrintContext)
{
    for
    (
        u32 DirectoryEntryIndex = 0;
        DirectoryEntryIndex < FAT12_DIRECTORY_ENTRIES_IN_SECTOR;
        DirectoryEntryIndex++
    )
    {
        directory_entry *DirectoryEntry = &Sector->DirectoryEntries[DirectoryEntryIndex];
        if (DirectoryEntry->FileName[0] == FAT12_FILENAME_EMPTY_SLOT)
        {
        }
        else if (DirectoryEntry->FileName[0] == FAT12_FILENAME_DELETED_SLOT)
        {
            PrintFormatted(PrintContext, "    > deleted file.\r\n");
        }
        else
        {
            if (DirectoryEntry->FileAttributes == FAT12_FILE_ATTRIBUTE_NORMAL)
            {
                char FileNameString[9];
                char FileExtensionString[4];

                MemoryCopy(FileNameString, DirectoryEntry->FileName, 8);
                MemoryCopy(FileExtensionString, DirectoryEntry->FileExtension, 3);

                FileNameString[8] = 0;
                FileExtensionString[3] = 0;

                PrintFormatted(PrintContext, "    FILE:   %s.%s\r\n", FileNameString, FileExtensionString);
            }
            else if (DirectoryEntry->FileAttributes == FAT12_FILE_ATTRIBUTE_DIRECTORY)
            {
                char FileNameString[9];

                MemoryCopy(FileNameString, DirectoryEntry->FileName, 8);
                FileNameString[8] = 0;
                PrintFormatted(PrintContext, "     DIR:   %s\r\n", FileNameString);
            }
        }
    }
}

void Fat12ListDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    print_context *PrintContext,
    char *DirectoryPath
)
{
    PrintFormatted(PrintContext, "\r\nlisting %ls:\r\n", DirectoryPath);

    if
    (
        (StringLength(DirectoryPath) == 1) &&
        (MemoryCompare(DirectoryPath, "\\", 1) == 0)
    )
    {
        for (u32 SectorIndex = 0; SectorIndex < FAT12_SECTORS_IN_ROOT_DIRECTORY; SectorIndex++)
        {
            Fat12ListDirectorySector(&Disk->RootDirectory.Sectors[SectorIndex], PrintContext);
        }
    }
    else
    {
        directory_entry *DirectoryEntry = GetDirectoryEntryOfFileByPath
        (
            Disk, MemoryArena, DiskParameters, DirectoryPath
        );

        u16 CurrentClusterNumber = DirectoryEntry->ClusterNumberLowWord;
        u16 CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);

        for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
        {
            sector *Sector = PushStruct(MemoryArena, sector);
            GetDiskSectorFromFatClusterNumber
            (
                DiskParameters,
                Sector,
                CurrentClusterNumber
            );
            Fat12ListDirectorySector(Sector, PrintContext);

            if (IsFatEntryEndOfFile(CurrentFatEntry))
            {
                break;
            }
            else
            {
                CurrentClusterNumber = CurrentFatEntry;
                CurrentFatEntry = GetFatEntryFromClusterNumber(Disk, CurrentClusterNumber);
            }
        }
    }
}