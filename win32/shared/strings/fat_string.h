#pragma once

#include <windows.h>
#include <string.h>

#include "portable\shared\base_types.h"
#include "win32\shared\memory\arena.h"

struct fat_string
{
    char *String;
    size_t Size;
};

inline fat_string
CreateFatString(char *SourceString, memory_arena *ArenaAllocator)
{
    fat_string Result;
    Result.Size = strlen(SourceString);
    Result.String = (char *)PushOntoMemoryArena(ArenaAllocator, Result.Size, false);
    memcpy(Result.String, SourceString, Result.Size);
    return Result;
}

inline char *
CreateCStringFromFatString(fat_string FatString, memory_arena *ArenaAllocator)
{
    char *CString = (char *)PushOntoMemoryArena(ArenaAllocator, FatString.Size + 1, false);
    memcpy(CString, FatString.String, FatString.Size);
    CString[FatString.Size] = NULL;
    return CString;
}

inline i32
CompareFatStrings(fat_string *StringA, fat_string *StringB)
{
    if (StringA == StringB)
    {
        return 0;
    }
    else if (StringA->Size != StringB->Size)
    {
        return -1;
    }
    else
    {
        return memcmp(StringA->String, StringB->String, StringA->Size);
    }
}