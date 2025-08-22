#include <Windows.h>
#include <stdint.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "fat12.h"

// TODO: optimize fat12 driver primitives
u16 TranslateClusterNumberToSectorIndex(u16 ClusterNumber)
{
    Assert(ClusterNumber >= 2);
    u16 PhysicalSectorIndex = FAT12_DATA_AREA_START_SECTOR + ClusterNumber - 2;
    return PhysicalSectorIndex;
}

b32 IsFatEntryEndOfFile(u16 FatEntry)
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

sector *GetSectorFromClusterNumber(fat12_disk *Disk, u16 ClusterNumber)
{
    sector *Result = &Disk->Sectors[TranslateClusterNumberToSectorIndex(ClusterNumber)];
    return Result;
}

u16 GetFatEntry(fat12_disk *Disk, u32 ClusterNumber)
{
    Assert(ClusterNumber >= 2);
    u16 Result = 0;
    u32 StartingByteIndex = ClusterNumber * 3 / 2;

    if ((ClusterNumber % 2) == 0)
    {
        Result = (*(u16 *)&Disk->Fat1.Bytes[StartingByteIndex]) & 0x0FFF;
        // Result = (u16)Disk->Fat1.Bytes[StartingByteIndex];
        // Result |= ((u16)Disk->Fat1.Bytes[StartingByteIndex + 1] & 0x000F) << 8;
    }
    else
    {
        Result = (*(u16 *)&Disk->Fat1.Bytes[StartingByteIndex]) >> 4;
        // Result = (u16)(Disk->Fat1.Bytes[StartingByteIndex] & 0xF0) >> 4;
        // Result |= ((u16)Disk->Fat1.Bytes[StartingByteIndex + 1]) << 4;
    }

    return Result;
}

u16 GetFirstFreeClusterNumber(fat12_disk *Disk)
{
    for (u16 ClusterNumber = 2; ClusterNumber < FAT12_ENTRIES_PER_FAT; ClusterNumber++)
    {
        u16 FatEntry = GetFatEntry(Disk, ClusterNumber);
        if (FatEntry == FAT12_FAT_ENTRY_FREE_CLUSTER)
        {
            return ClusterNumber;
        }
    }
    return 0;
}

u32 CalculateNumberOfFreeClusters(fat12_disk *Disk)
{
    u32 Result = 0;

    for (u16 ClusterNumber = 2; ClusterNumber < FAT12_ENTRIES_PER_FAT; ClusterNumber++)
    {
        u16 FatEntry = GetFatEntry(Disk, ClusterNumber);
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
        DirectoryEntryIndex < FAT12_DIRECTORY_ENTRIES_PER_SECTOR;
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
GetFirstFreeDirectoryEntryInDirectory(fat12_disk *Disk, directory_entry *Directory)
{
    directory_entry *FirstFreeDirectoryEntry = NULL;

    u16 CurrentClusterNumber = Directory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        sector *Sector = GetSectorFromClusterNumber(Disk, CurrentClusterNumber);
        FirstFreeDirectoryEntry = GetFirstFreeDirectoryEntryInSector(Sector);

        if (FirstFreeDirectoryEntry)
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
                CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);
            }
        }
    }

    return FirstFreeDirectoryEntry;
}

directory_entry *
GetFirstFreeDirectoryEntryInRootDirectory(fat12_disk *Disk)
{
    directory_entry *FirstFreeDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < FAT12_SECTORS_IN_ROOT_DIRECTORY; SectorIndex++)
    {
        FirstFreeDirectoryEntry =
            GetFirstFreeDirectoryEntryInSector(&Disk->RootDirectory.Sectors[SectorIndex]);

        if (FirstFreeDirectoryEntry)
        {
            break;
        }
    }

    return FirstFreeDirectoryEntry;
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
            (memcmp(DirectoryEntry->FileName, FileName, 8) == 0) &&
            (memcmp(DirectoryEntry->FileExtension, Extension, 3) == 0)
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
        DirectoryEntryIndex < ArrayCount(Sector->DirectoryEntries);
        DirectoryEntryIndex++
    )
    {
        directory_entry *DirectoryEntry = &Sector->DirectoryEntries[DirectoryEntryIndex];
        if (memcmp(DirectoryEntry->FileName, DirectoryName, 8) == 0)
        {
            return DirectoryEntry;
        }
    }

    return NULL;
}

directory_entry *
GetDirectoryEntryOfFileInDirectory
(
    fat12_disk *Disk, directory_entry *Directory,
    char *FileName, char *Extension
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = Directory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        sector *PhysicalSector = GetSectorFromClusterNumber(Disk, CurrentClusterNumber);
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
                CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);
            }
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfDirectoryInDirectory
(
    fat12_disk *Disk, directory_entry *Directory, char *DirectoryName
)
{
    directory_entry *FoundDirectoryEntry = NULL;

    u16 CurrentClusterNumber = Directory->ClusterNumberLowWord;
    u16 CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);

    for (u32 LoopIndex = 0; LoopIndex < FAT12_SECTORS_IN_DATA_AREA; LoopIndex++)
    {
        sector *PhysicalSector = GetSectorFromClusterNumber(Disk, CurrentClusterNumber);
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
                CurrentFatEntry = GetFatEntry(Disk, CurrentClusterNumber);
            }
        }
    }

    return FoundDirectoryEntry;
}

directory_entry *
GetDirectoryEntryOfFileInRootDirectory(fat12_disk *Disk, char *FileName, char *Extension)
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
GetDirectoryEntryOfDirectoryInRootDirectory(fat12_disk *Disk, char *DirectoryName)
{
    directory_entry *FoundDirectoryEntry = NULL;

    for (u32 SectorIndex = 0; SectorIndex < ArrayCount(Disk->RootDirectory.Sectors); SectorIndex++)
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