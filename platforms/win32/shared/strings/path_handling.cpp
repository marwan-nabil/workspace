#include <Windows.h>
#include <stdint.h>
#include <math.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\system\memory.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\strings\path_handling.h"

void RemoveLastSegmentFromPath(char *Path, b8 KeepSlash, char Separator)
{
    u32 PathLength = StringLength(Path);
    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (Path[CharIndex] == Separator)
        {
            if (KeepSlash)
            {
                ZeroMemory(&Path[CharIndex + 1], StringLength(&Path[CharIndex]));
                return;
            }
            else
            {
                ZeroMemory(&Path[CharIndex], StringLength(&Path[CharIndex]));
                return;
            }
        }
    }
}

char *GetPointerToLastSegmentFromPath(char *Path, char separator)
{
    u32 PathLength = StringLength(Path);
    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (Path[CharIndex] == separator)
        {
            return &Path[CharIndex + 1];
        }
    }
    return NULL;
}

file_path_segment_node *
CreateFilePathSegmentList(char *FileFullPath)
{
    char LocalPathBuffer[1024] = {};
    StringCchCat(LocalPathBuffer, ArrayCount(LocalPathBuffer), FileFullPath);

    u32 PathLength = StringLength(LocalPathBuffer);

    file_path_segment_node *CurrentFilePathNode =
        (file_path_segment_node *)malloc(sizeof(file_path_segment_node));
    *CurrentFilePathNode = {};
    file_path_segment_node *LastFilePathNode = 0;

    for (i32 CharIndex = PathLength - 1; CharIndex >= 0; CharIndex--)
    {
        if (LocalPathBuffer[CharIndex] == '\\')
        {
            char PathSegment[MAX_PATH] = {};
            StringCchCat(PathSegment, MAX_PATH, &LocalPathBuffer[CharIndex + 1]);
            ZeroMemory(&LocalPathBuffer[CharIndex], StringLength(&LocalPathBuffer[CharIndex]));

            if (!CurrentFilePathNode)
            {
                CurrentFilePathNode = (file_path_segment_node *)malloc(sizeof(file_path_segment_node));
                *CurrentFilePathNode = {};
            }

            memcpy(CurrentFilePathNode->SegmentName, PathSegment, ArrayCount(CurrentFilePathNode->SegmentName));
            CurrentFilePathNode->ChildNode = LastFilePathNode;

            LastFilePathNode = CurrentFilePathNode;
            CurrentFilePathNode = 0;
        }
    }

    return LastFilePathNode;
}

void
FreeFilePathSegmentList(file_path_segment_node *RootNode)
{
    file_path_segment_node *CurrentNode = RootNode;
    file_path_segment_node *ChildNode;

    while (CurrentNode)
    {
        ChildNode = CurrentNode->ChildNode;
        free(CurrentNode);
        CurrentNode = ChildNode;
    }
}

file_path_segment_node *SkipPrefixSubPath
(
    file_path_segment_node *Path,
    file_path_segment_node *PrefixSubPathToSkip
)
{
    file_path_segment_node *CurrentPathNode = Path;
    file_path_segment_node *CurrentSubPathNode = PrefixSubPathToSkip;
    while (CurrentPathNode && CurrentSubPathNode)
    {
        if (strcmp(CurrentPathNode->SegmentName, CurrentSubPathNode->SegmentName) == 0)
        {
            CurrentPathNode = CurrentPathNode->ChildNode;
            CurrentSubPathNode = CurrentSubPathNode->ChildNode;
        }
        else
        {
            CurrentPathNode = Path;
            break;
        }
    }

    if (!CurrentPathNode)
    {
        CurrentPathNode = Path;
    }

    return CurrentPathNode;
}

void FlattenPathSegmentList(file_path_segment_node *PathList, char *TargetStringBuffer, u32 TargetBufferLength, char Separator)
{
    char SeparatorString[2] = {};
    SeparatorString[0] = Separator;
    file_path_segment_node *CurrentSegmentNode = PathList;
    while (CurrentSegmentNode)
    {
        StringCchCatA(TargetStringBuffer, TargetBufferLength, SeparatorString);
        StringCchCatA(TargetStringBuffer, TargetBufferLength, CurrentSegmentNode->SegmentName);
        CurrentSegmentNode = CurrentSegmentNode->ChildNode;
    }
}