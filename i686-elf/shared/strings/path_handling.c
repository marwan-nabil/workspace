#include "sources\i686-elf\libraries\base_types.h"
#include "sources\i686-elf\libraries\basic_defines.h"
#include "sources\i686-elf\libraries\math\integers.h"
#include "sources\i686-elf\libraries\memory\memory.h"
#include "sources\i686-elf\libraries\strings\print.h"
#include "sources\i686-elf\libraries\strings\strings.h"
#include "sources\i686-elf\libraries\memory\arena_allocator.h"
#include "sources\i686-elf\libraries\strings\path_handling.h"

file_path_node *
CreateFilePathSegmentList(char *FileFullPath, memory_arena *MemoryArena)
{
    char LocalPathBuffer[PATH_HANDLING_MAX_PATH];
    MemoryZero(LocalPathBuffer, ArrayCount(LocalPathBuffer));

    StringConcatenate(LocalPathBuffer, ArrayCount(LocalPathBuffer), FileFullPath);

    u32 PathLength = StringLength(LocalPathBuffer);

    file_path_node *CurrentFilePathNode = PushStruct(MemoryArena, file_path_node);
    MemoryZero(CurrentFilePathNode, sizeof(file_path_node));

    file_path_node *LastFilePathNode = NULL;

    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (LocalPathBuffer[CharIndex] == '\\')
        {
            char PathSegment[PATH_HANDLING_MAX_PATH];
            MemoryZero(PathSegment, ArrayCount(PathSegment));

            StringConcatenate(PathSegment, PATH_HANDLING_MAX_PATH, &LocalPathBuffer[CharIndex + 1]);
            MemoryZero(&LocalPathBuffer[CharIndex], StringLength(&LocalPathBuffer[CharIndex]));

            if (!CurrentFilePathNode)
            {
                CurrentFilePathNode = PushStruct(MemoryArena, file_path_node);
                MemoryZero(CurrentFilePathNode, sizeof(file_path_node));
            }

            MemoryCopy(CurrentFilePathNode->FileName, PathSegment, ArrayCount(CurrentFilePathNode->FileName));
            CurrentFilePathNode->ChildNode = LastFilePathNode;

            LastFilePathNode = CurrentFilePathNode;
            CurrentFilePathNode = NULL;
        }
    }

    return LastFilePathNode;
}

void RemoveLastSegmentFromPath(char *Path)
{
    u32 PathLength = StringLength(Path);
    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (Path[CharIndex] == '\\')
        {
            MemoryZero(&Path[CharIndex], StringLength(&Path[CharIndex]));
            return;
        }
    }
}

void GetFileNameAndExtensionFromString
(
    char *SourceString,
    char *FileName, u32 FileNameSize,
    char *FileExtension, u32 FileExtensionSize
)
{
    u32 SourceStringLength = StringLength(SourceString);
    u32 DotIndex = GetLastCharacterIndex(SourceString, SourceStringLength, '.');

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
        if (DotIndex == UINT32_MAX)
        {
            MemoryZero(FileExtension, FileExtensionSize);
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