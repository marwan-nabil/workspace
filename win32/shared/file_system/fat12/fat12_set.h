#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\file_system\fat12\fat12.h"

void SetFatEntry(fat12_disk *Disk, u32 ClusterNumber, u16 FatEntry);
u16 AllocateDiskClusters(fat12_disk *Disk, void *Memory, u32 Size);
b32 AllocateFileToDirectoryEntry
(
    fat12_disk *Disk, directory_entry *DirectoryEntry,
    char *FileName, char *Extension, void *Memory, u32 Size
);
b32 AllocateDirectoryToDirectoryEntry
(
    fat12_disk *Disk, directory_entry *DirectoryEntry, char *DirectoryName
);
directory_entry *AddFileToRootDirectory
(
    fat12_disk *Disk,
    char *FileName, char *Extension,
    void *Memory, u32 Size
);
directory_entry *AddDirectoryToRootDirectory
(
    fat12_disk *Disk, char *DirectoryName
);
directory_entry *AddFileToDirectory
(
    fat12_disk *Disk, directory_entry *Directory,
    char *FileName, char *Extension, void *Memory, u32 Size
);
directory_entry *AddDirectoryToDirectory
(
    fat12_disk *Disk, directory_entry *Directory, char *DirectoryName
);