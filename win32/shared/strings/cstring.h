#pragma once

#include <windows.h>
#include <string.h>

#include "portable\shared\base_types.h"
#include "win32\shared\basic_structures\singly_linked_list.h"
#include "win32\shared\memory\arena.h"

inline u64
GetLastCharacterIndex(char *String, char Character)
{
    for (u64 CharacterIndex = strlen(String) - 1; CharacterIndex >= 0; CharacterIndex--)
    {
        if (String[CharacterIndex] == Character)
        {
            return CharacterIndex;
        }
    }
    return UINT64_MAX;
}

inline char *
FlattenLinkedListOfStrings
(
    singly_linked_list_node *StringList,
    memory_arena *ArenaAllocator
)
{
    u32 StringCount = 0;
    size_t ResultSize = 0;

    singly_linked_list_node *CurrentNode = StringList;
    while (CurrentNode)
    {
        if (CurrentNode->Value)
        {
            StringCount++;
            ResultSize += strlen((char *)CurrentNode->Value);
        }
        CurrentNode = CurrentNode->NextNode;
    }

    char *Result = (char *)PushOntoMemoryArena(ArenaAllocator, ResultSize + 1, FALSE);
    Result[ResultSize] = NULL;
    size_t WriteIndex = 0;

    CurrentNode = StringList;
    while (CurrentNode)
    {
        if (CurrentNode->Value)
        {
            size_t CurrentStringSize = strlen((char *)CurrentNode->Value);
            memcpy(Result + WriteIndex, (char *)CurrentNode->Value, CurrentStringSize);
            WriteIndex += CurrentStringSize;
        }
        CurrentNode = CurrentNode->NextNode;
    }

    return Result;
}