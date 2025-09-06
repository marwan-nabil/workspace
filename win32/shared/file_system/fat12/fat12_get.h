#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\file_system\fat12\fat12.h"

u16 TranslateClusterNumberToSectorIndex(u16 ClusterNumber);
b32 IsFatEntryEndOfFile(u16 FatEntry);
sector *GetSectorFromClusterNumber(fat12_disk *Disk, u16 ClusterNumber);
u16 GetFatEntry(fat12_disk *Disk, u32 ClusterNumber);
u16 GetFirstFreeClusterNumber(fat12_disk *Disk);
u32 CalculateNumberOfFreeClusters(fat12_disk *Disk);
directory_entry *GetFirstFreeDirectoryEntryInSector(sector *Sector);
directory_entry *GetFirstFreeDirectoryEntryInDirectory(fat12_disk *Disk, directory_entry *Directory);
directory_entry *GetFirstFreeDirectoryEntryInRootDirectory(fat12_disk *Disk);
directory_entry *GetDirectoryEntryOfFileInSector(sector *Sector, char *FileName, char *Extension);
directory_entry *GetDirectoryEntryOfDirectoryInSector(sector *Sector, char *DirectoryName);
directory_entry *GetDirectoryEntryOfFileInDirectory
(
    fat12_disk *Disk, directory_entry *Directory,
    char *FileName, char *Extension
);
directory_entry *GetDirectoryEntryOfDirectoryInDirectory
(
    fat12_disk *Disk, directory_entry *Directory, char *DirectoryName
);
directory_entry *GetDirectoryEntryOfFileInRootDirectory(fat12_disk *Disk, char *FileName, char *Extension);
directory_entry *GetDirectoryEntryOfDirectoryInRootDirectory(fat12_disk *Disk, char *DirectoryName);