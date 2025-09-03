#pragma once

b8 IsCharacterLowerCase(char Character);
char ConvertCharacterToUpperCase(char Character);
u32 GetLastCharacterIndex(char *String, u32 StringLength, char Character);
const char *GetCharacterPointer(const char *String, char Character);
i16 StringCompare(char *String1, char *String2, u32 ComparisonRange);
void StringConcatenate(char *Destination, u32 Size, char *Source);
u32 StringLength(char *String);
void FillFixedSizeStringBuffer(char *Buffer, u32 BufferSize, char *SourceString);