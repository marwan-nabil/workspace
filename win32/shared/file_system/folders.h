#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\strings\cstrings.h"
#include "win32\shared\memory\arena.h"

b32 CleanExtensionFromDirectory(const char *ExtensionToClean, const char *DirectoryPath);
b32 DoesDirectoryExist(const char *DirectoryPath);
b32 DeleteDirectoryCompletely(const char *DirectoryPath);
void EmptyDirectory(const char *DirectoryPath);
string_node *GetListOfFilesInFolder(char *FolderPath);
string_node *GetListOfFilesWithNameInFolder(char *DirectoryPath, char *FileName);
void GetFilesByWildCard(char *DirectoryPath, char *WildCard, string_node **List);
void GetFilesByName(char *DirectoryPath, char *FileName, memory_arena *Memory, string_node **List);