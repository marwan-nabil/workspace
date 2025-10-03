#pragma once

#include <stddef.h>
#include "portable\shared\base_types.h"

struct read_file_result
{
    void *FileMemory;
    size_t Size;
};

void FreeFileMemory(read_file_result File);
read_file_result ReadFileIntoMemory(char *FilePath);
b32 WriteFileFromMemory(char *FilePath, void *DataToWrite, u32 DataSize);
b32 CreateEmptyFile(char *FilePath, u32 Size, u32 FillPattern);
b32 WriteBinaryFileOverAnother(char *DestinationBinaryFilePath, char *SourceBinaryFilePath, u32 WriteOffset);