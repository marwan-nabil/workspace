#pragma once

b8 IsFatEntryEndOfFile(u16 FatEntry);
u16 TranslateFatClusterNumberToSectorIndex(u16 ClusterNumber);
u16 TranslateSectorIndexToFatClusterNumber(u16 SectorIndex);
void GetDiskSectorFromFatClusterNumber
(
    disk_parameters *DiskParameters,
    sector *SectorLocation,
    u16 ClusterNumber
);
u16 GetFatEntryFromClusterNumber(fat12_ram_disk *Disk, u32 ClusterNumber);
u16 GetFirstFreeClusterNumber(fat12_ram_disk *Disk);
u32 CalculateFreeClusterNumbers(fat12_ram_disk *Disk);
directory_entry *GetFirstFreeDirectoryEntryInSector(sector *Sector);
directory_entry *GetDirectoryEntryOfFileInSector(sector *Sector, char *FileName, char *Extension);
directory_entry *GetDirectoryEntryOfDirectoryInSector(sector *Sector, char *DirectoryName);
directory_entry *GetDirectoryEntryOfFileInDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *Directory,
    char *FileName,
    char *Extension
);
directory_entry *GetDirectoryEntryOfDirectoryInDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *Directory,
    char *DirectoryName
);
directory_entry *GetDirectoryEntryOfFileInRootDirectory
(
    fat12_ram_disk *Disk,
    char *FileName,
    char *Extension
);
directory_entry *GetDirectoryEntryOfDirectoryInRootDirectory
(
    fat12_ram_disk *Disk,
    char *DirectoryName
);
directory_entry *GetDirectoryEntryOfFileByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FullFilePath
);
void Fat12ListDirectorySector(sector *Sector, print_context *PrintContext);
void Fat12ListDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    print_context *PrintContext,
    char *DirectoryPath
);