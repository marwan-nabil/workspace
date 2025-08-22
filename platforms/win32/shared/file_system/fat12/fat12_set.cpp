#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\strings\path_handling.h"
#include "fat12.h"
#include "fat12_get.h"
#include "fat12_set.h"
#include "fat12_interface.h"

void SetFatEntry(fat12_disk *Disk, u32 ClusterNumber, u16 FatEntry)
{
    Assert(ClusterNumber >= 2);
    u32 StartingByteIndex = ClusterNumber * 3 / 2;

    if ((ClusterNumber % 2) == 0)
    {
        Disk->Fat1.Bytes[StartingByteIndex] = (u8)FatEntry;
        Disk->Fat1.Bytes[StartingByteIndex + 1] &= (u8)0xF0;
        Disk->Fat1.Bytes[StartingByteIndex + 1] |= (u8)((FatEntry >> 8) & 0x000F);
    }
    else
    {
        Disk->Fat1.Bytes[StartingByteIndex] &= (u8)0x0F;
        Disk->Fat1.Bytes[StartingByteIndex] |= (u8)((FatEntry & 0x000F) << 4);
        Disk->Fat1.Bytes[StartingByteIndex + 1] = (u8)((FatEntry >> 4) & 0x00FF);
    }
}

u16 AllocateDiskClusters(fat12_disk *Disk, void *Memory, u32 Size)
{
    u16 FirstAllocatedClusterNumber = 0;

    u32 ClustersNeeded = (Size + FAT12_CLUSTER_SIZE - 1) / FAT12_CLUSTER_SIZE;
    if (ClustersNeeded > CalculateNumberOfFreeClusters(Disk))
    {
        return 0;
    }
    Assert(ClustersNeeded <= FAT12_SECTORS_IN_DATA_AREA);

    u32 SizeLeft = Size;
    char *ReadPointer = (char *)Memory;
    u16 PreviousClusterNumber = 0;

    for (u32 ClusterIndex = 0; ClusterIndex < ClustersNeeded; ClusterIndex++)
    {
        u16 ClusterNumber = GetFirstFreeClusterNumber(Disk);
        Assert(ClusterNumber);

        SetFatEntry(Disk, ClusterNumber, FAT12_FAT_ENTRY_RESERVED_CLUSTER);

        if (PreviousClusterNumber)
        {
            SetFatEntry(Disk, PreviousClusterNumber, ClusterNumber);
        }
        else
        {
            FirstAllocatedClusterNumber = ClusterNumber;
        }

        u16 SectorIndex = TranslateClusterNumberToSectorIndex(ClusterNumber);

        u32 BytesToCopy = FAT12_SECTOR_SIZE;
        u32 BytesToZero = 0;

        if (SizeLeft < BytesToCopy)
        {
            BytesToCopy = SizeLeft;
            BytesToZero = FAT12_SECTOR_SIZE - SizeLeft;
        }

        if (ReadPointer)
        {
            memcpy(Disk->Sectors[SectorIndex].Bytes, ReadPointer, BytesToCopy);
            ReadPointer += BytesToCopy;
            ZeroMemory(Disk->Sectors[SectorIndex].Bytes + BytesToCopy, BytesToZero);
        }
        else
        {
            ZeroMemory(Disk->Sectors[SectorIndex].Bytes, BytesToCopy + BytesToZero);
        }

        SizeLeft -= BytesToCopy;

        if (SizeLeft == 0)
        {
            SetFatEntry(Disk, ClusterNumber, FAT12_FAT_ENTRY_END_OF_FILE_CLUSTER_RANGE_END);
        }

        PreviousClusterNumber = ClusterNumber;
    }

    return FirstAllocatedClusterNumber;
}

b32 AllocateFileToDirectoryEntry
(
    fat12_disk *Disk, directory_entry *DirectoryEntry,
    char *FileName, char *Extension, void *Memory, u32 Size
)
{
    u16 ClusterNumber = AllocateDiskClusters(Disk, Memory, Size);
    if (ClusterNumber)
    {
        *DirectoryEntry = {};
        memcpy(DirectoryEntry->FileName, FileName, 8);
        memcpy(DirectoryEntry->FileExtension, Extension, 3);
        DirectoryEntry->FileAttributes = FAT12_FILE_ATTRIBUTE_NORMAL;
        DirectoryEntry->FileSize = Size;
        DirectoryEntry->ClusterNumberLowWord = ClusterNumber;
        return TRUE;
    }

    return FALSE;
}

b32 AllocateDirectoryToDirectoryEntry
(
    fat12_disk *Disk, directory_entry *DirectoryEntry, char *DirectoryName
)
{
    u16 ClusterNumber = AllocateDiskClusters(Disk, 0, FAT12_CLUSTER_SIZE);
    if (ClusterNumber)
    {
        *DirectoryEntry = {};
        memcpy((void *)DirectoryEntry->FileName, DirectoryName, 8);
        DirectoryEntry->FileAttributes = FAT12_FILE_ATTRIBUTE_DIRECTORY;
        DirectoryEntry->FileSize = 0;
        DirectoryEntry->ClusterNumberLowWord = ClusterNumber;
        return TRUE;
    }

    return FALSE;
}

directory_entry *AddFileToRootDirectory
(
    fat12_disk *Disk,
    char *FileName, char *Extension,
    void *Memory, u32 Size
)
{
    directory_entry *FoundDirectoryEntry = GetFirstFreeDirectoryEntryInRootDirectory(Disk);
    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b32 Result = AllocateFileToDirectoryEntry
    (
        Disk,
        FoundDirectoryEntry,
        FileName,
        Extension,
        Memory,
        Size
    );

    if (Result)
    {
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *AddDirectoryToRootDirectory
(
    fat12_disk *Disk, char *DirectoryName
)
{
    directory_entry *FoundDirectoryEntry = GetFirstFreeDirectoryEntryInRootDirectory(Disk);
    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b32 Result = AllocateDirectoryToDirectoryEntry(Disk, FoundDirectoryEntry, DirectoryName);
    if (Result)
    {
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *AddFileToDirectory
(
    fat12_disk *Disk, directory_entry *Directory,
    char *FileName, char *Extension, void *Memory, u32 Size
)
{
    directory_entry *FoundDirectoryEntry = GetFirstFreeDirectoryEntryInDirectory(Disk, Directory);
    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b32 Result = AllocateFileToDirectoryEntry
    (
        Disk,
        FoundDirectoryEntry,
        FileName,
        Extension,
        Memory,
        Size
    );

    if (Result)
    {
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}

directory_entry *AddDirectoryToDirectory
(
    fat12_disk *Disk, directory_entry *Directory, char *DirectoryName
)
{
    directory_entry *FoundDirectoryEntry = GetFirstFreeDirectoryEntryInDirectory(Disk, Directory);
    if (!FoundDirectoryEntry)
    {
        return NULL;
    }

    b32 Result = AllocateDirectoryToDirectoryEntry(Disk, FoundDirectoryEntry, DirectoryName);
    if (Result)
    {
        return FoundDirectoryEntry;
    }
    else
    {
        return NULL;
    }
}