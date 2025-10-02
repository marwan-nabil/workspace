#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\file_system\fat12\fat12.h"

directory_entry *Fat12GetDirectoryEntryOfFile(fat12_disk *Disk, char *FullFilePath);
directory_entry *Fat12AddFile(fat12_disk *Disk, char *FullFilePath, void *Memory, u32 Size);
directory_entry *Fat12AddDirectory(fat12_disk *Disk, char *DirectoryPath);
void Fat12ListDirectorySector(sector *Sector);
void Fat12ListDirectory(fat12_disk *Disk, char *DirectoryPath);