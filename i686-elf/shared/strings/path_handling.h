#pragma once

#define PATH_HANDLING_SEGMENT_STRING_LENGTH 512
#define PATH_HANDLING_MAX_PATH 1024

typedef struct _file_path_node
{
    char FileName[PATH_HANDLING_SEGMENT_STRING_LENGTH];
    struct _file_path_node *ChildNode;
} file_path_node;

file_path_node *CreateFilePathSegmentList(char *FileFullPath, memory_arena *MemoryArena);
void RemoveLastSegmentFromPath(char *Path);
void GetFileNameAndExtensionFromString
(
    char *SourceString,
    char *FileName, u32 FileNameSize,
    char *FileExtension, u32 FileExtensionSize
);