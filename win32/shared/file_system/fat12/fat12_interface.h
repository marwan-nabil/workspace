#pragma once

directory_entry *Fat12GetDirectoryEntryOfFile(fat12_disk *Disk, char *FullFilePath);
directory_entry *Fat12AddFile(fat12_disk *Disk, char *FullFilePath, void *Memory, u32 Size);
directory_entry *Fat12AddDirectory(fat12_disk *Disk, char *DirectoryPath);
void Fat12ListDirectorySector(sector *Sector);
void Fat12ListDirectory(fat12_disk *Disk, char *DirectoryPath);