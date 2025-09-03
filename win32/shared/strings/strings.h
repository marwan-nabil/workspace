
#pragma once

struct fat_string
{
    char *String;
    u32 Length;
};

inline u32
StringLength(char *String)
{
    u32 Count = 0;
    while (*String++)
    {
        ++Count;
    }
    return Count;
}

inline fat_string
CreateFatString(char *SourceString, memory_arena *MemoryArena)
{
    fat_string Result;
    Result.Length = StringLength(SourceString);
    Result.String = (char *)PushOntoMemoryArena(MemoryArena, Result.Length, false);
    memcpy(Result.String, SourceString, Result.Length);
    return Result;
}

inline char *
CreateCStringFromFatString(fat_string FatString, memory_arena *Memory)
{
    char *CString = (char *)PushOntoMemoryArena(Memory, FatString.Length + 1, true);
    memcpy(CString, FatString.String, FatString.Length);
    return CString;
}

inline i32
CompareFatStrings(fat_string *StringA, fat_string *StringB)
{
    if (StringA == StringB)
    {
        return 0;
    }
    else if (StringA->Length != StringB->Length)
    {
        return -1;
    }
    else
    {
        return memcmp(StringA->String, StringB->String, StringA->Length);
    }
}

inline void
CombineStrings
(
    size_t SourceASize, size_t SourceBSize, size_t DestinationSize,
    char *SourceA, char *SourceB, char *Destination
)
{
    Assert((SourceASize + SourceBSize) < DestinationSize);

    for (u32 Index = 0; Index < SourceASize; Index++)
    {
        *Destination++ = *SourceA++;
    }

    for (u32 Index = 0; Index < SourceBSize; Index++)
    {
        *Destination++ = *SourceB++;
    }

    *Destination++ = '\0';
}

inline void
InjectEscapeSlashes(char *SourceString, char *DestinationString, u32 DestinationSize)
{
    u32 WrittenCharacters = 0;
    while (*SourceString)
    {
        if (*SourceString == '\\')
        {
            DestinationString[WrittenCharacters++] = '\\';
            if (WrittenCharacters >= DestinationSize)
            {
                return;
            }
        }

        DestinationString[WrittenCharacters++] = *SourceString++;
        if (WrittenCharacters > DestinationSize)
        {
            return;
        }
    }
}

inline u32
GetFirstCharacterIndex(char *String, u32 StringLength, char Character)
{
    for (u32 CharacterIndex = 0; CharacterIndex < StringLength; CharacterIndex++)
    {
        if (String[CharacterIndex] == Character)
        {
            return CharacterIndex;
        }
    }

    return UINT32_MAX;
}

inline u32
GetLastCharacterIndex(char *String, u32 StringLength, char Character)
{
    Assert(StringLength);

    for (i32 CharacterIndex = StringLength - 1; CharacterIndex >= 0; CharacterIndex--)
    {
        if (String[CharacterIndex] == Character)
        {
            return CharacterIndex;
        }
    }

    return UINT32_MAX;
}

inline void
GetFileNameAndExtensionFromString
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

inline void
FillFixedSizeStringBuffer(char *Buffer, u32 BufferSize, char *SourceString)
{
    if (StringLength(SourceString) >= BufferSize)
    {
        memcpy(Buffer, SourceString, BufferSize);
    }
    else
    {
        StringCchCat(Buffer, BufferSize, SourceString);
    }
}