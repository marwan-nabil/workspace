#pragma once

#include <windows.h>
#include <string.h>
#include <strsafe.h>

#include "portable\shared\base_types.h"
#include "win32\shared\basic_structures\singly_linked_list.h"
#include "win32\shared\strings\cstring.h"
#include "win32\shared\memory\arena.h"

inline void
SeparateStringIntoFilenameAndExtension
(
    char *SourceString,
    char *FileName, u32 FileNameSize,
    char *FileExtension, u32 FileExtensionSize
)
{
    u32 SourceStringLength = strlen(SourceString);
    u32 DotIndex = GetLastCharacterIndex(SourceString, '.');

    u32 ReadIndex = 0;
    u32 WriteIndex = 0;

    while
    (
        (ReadIndex < DotIndex) &&
        (ReadIndex < SourceStringLength) &&
        (WriteIndex < FileNameSize)
    )
    {
        FileName[WriteIndex++] = SourceString[ReadIndex++];
    }

    if (ReadIndex == DotIndex)
    {
        WriteIndex = 0;
        ReadIndex++;

        while
        (
            (ReadIndex < SourceStringLength) &&
            (WriteIndex < FileExtensionSize)
        )
        {
            FileExtension[WriteIndex++] = SourceString[ReadIndex++];
        }
    }
    else if (ReadIndex == SourceStringLength)
    {
        return;
    }
    else if (WriteIndex == FileNameSize)
    {
        if (DotIndex == UINT64_MAX)
        {
            ZeroMemory(FileExtension, FileExtensionSize);
        }
        else
        {
            WriteIndex = 0;
            ReadIndex = DotIndex + 1;

            while
            (
                (ReadIndex < SourceStringLength) &&
                (WriteIndex < FileExtensionSize)
            )
            {
                FileExtension[WriteIndex++] = SourceString[ReadIndex++];
            }
        }
    }
}

void RemoveLastSegmentFromPath(char *Path, b8 KeepSlash, char Separator)
{
    u32 PathLength = strlen(Path);
    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (Path[CharIndex] == Separator)
        {
            if (KeepSlash)
            {
                ZeroMemory(&Path[CharIndex + 1], strlen(&Path[CharIndex]));
                return;
            }
            else
            {
                ZeroMemory(&Path[CharIndex], strlen(&Path[CharIndex]));
                return;
            }
        }
    }
}

char *GetPointerToLastSegmentFromPath(char *Path, char separator)
{
    u32 PathLength = strlen(Path);
    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (Path[CharIndex] == separator)
        {
            return &Path[CharIndex + 1];
        }
    }
    return NULL;
}

singly_linked_list_node *
CreateFilePathSegmentList(char *FileFullPath, memory_arena *ResultAllocator)
{
    size_t PathLength = strlen(FileFullPath);
    char *LocalPathBuffer = (char *)malloc(PathLength + 1);
    memcpy(LocalPathBuffer, FileFullPath, PathLength);
    LocalPathBuffer[PathLength] = NULL;

    singly_linked_list_node *CurrentFilePathNode = (singly_linked_list_node *)
        PushOntoMemoryArena(ResultAllocator, sizeof(singly_linked_list_node), TRUE);
    singly_linked_list_node *LastFilePathNode = NULL;

    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (LocalPathBuffer[CharIndex] == '\\')
        {
            size_t PathSegmentSize = strlen(&LocalPathBuffer[CharIndex + 1]);
            char *PathSegment = (char *)PushOntoMemoryArena(ResultAllocator, PathSegmentSize + 1, FALSE);
            memcpy(PathSegment, &LocalPathBuffer[CharIndex + 1], PathSegmentSize);
            PathSegment[PathSegmentSize] = NULL;

            ZeroMemory(&LocalPathBuffer[CharIndex], PathSegmentSize + 1);

            if (!CurrentFilePathNode)
            {
                CurrentFilePathNode = (singly_linked_list_node *)
                    PushOntoMemoryArena(ResultAllocator, sizeof(singly_linked_list_node), TRUE);
            }

            CurrentFilePathNode->Value = PathSegment;
            CurrentFilePathNode->NextNode = LastFilePathNode;

            LastFilePathNode = CurrentFilePathNode;
            CurrentFilePathNode = NULL;
        }
    }

    free(LocalPathBuffer);
    return LastFilePathNode;
}