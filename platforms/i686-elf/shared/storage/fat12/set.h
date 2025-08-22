#pragma once

void InitializeFat12RamDisk
(
    disk_parameters *DiskParameters,
    fat12_ram_disk *RamDisk
);

void SetFatEntry(fat12_ram_disk *Disk, u32 ClusterNumber, u16 FatEntry);

u16 AllocateDiskClusters
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    void *Memory,
    u32 Size
);

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
);

b8 AllocateDirectoryToDirectoryEntry
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *DirectoryEntry,
    char *DirectoryName
);

directory_entry *AddFileToRootDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FileName,
    char *Extension,
    void *Memory,
    u32 Size
);

directory_entry *AddDirectoryToRootDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *DirectoryName
);

directory_entry *AddFileToDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *ParentDirectory,
    char *FileName,
    char *Extension,
    void *Memory,
    u32 Size
);

directory_entry *AddDirectoryToDirectory
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    directory_entry *ParentDirectory,
    char *DirectoryName
);

directory_entry *Fat12AddFileByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *FullFilePath,
    void *Memory,
    u32 Size
);

directory_entry *Fat12AddDirectoryByPath
(
    fat12_ram_disk *Disk,
    memory_arena *MemoryArena,
    disk_parameters *DiskParameters,
    char *DirectoryPath
);