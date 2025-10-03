#pragma once

#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\structures\singly_linked_list.h"
#include "portable\shared\memory\linear_allocator.h"

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
Create2StringCombination(char *FirstString, char *SecondString, linear_allocator *Allocator)
{
    size_t Size1 = strlen(FirstString);
    size_t Size2 = strlen(SecondString);
    char *Result = (char *)PushOntoMemoryArena(Allocator, Size1 + Size2 + 1, FALSE);
    memcpy(Result, FirstString, Size1);
    memcpy(Result + Size1, SecondString, Size2);
    Result[Size1 + Size2] = 0;
    return Result;
}

inline char *
Create3StringCombination(char *FirstString, char *SecondString, char *ThirdString, linear_allocator *Allocator)
{
    size_t Size1 = strlen(FirstString);
    size_t Size2 = strlen(SecondString);
    size_t Size3 = strlen(ThirdString);
    char *Result = (char *)PushOntoMemoryArena(Allocator, Size1 + Size2 + Size3 + 1, FALSE);
    memcpy(Result, FirstString, Size1);
    memcpy(Result + Size1, SecondString, Size2);
    memcpy(Result + Size1 + Size2, ThirdString, Size3);
    Result[Size1 + Size2 + Size3] = 0;
    return Result;
}

inline char *
FlattenLinkedListOfStrings
(
    singly_linked_list_node *StringList,
    linear_allocator *Allocator
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

    char *Result = (char *)PushOntoMemoryArena(Allocator, ResultSize + 1, FALSE);
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