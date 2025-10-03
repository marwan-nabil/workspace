#pragma once

#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"

inline u64 HashBuffer1(const u8 *Buffer, size_t BufferLength)
{
    u64 Hash = 5381;
    for (size_t Index = 0; Index < BufferLength; Index++)
    {
        Hash += (Hash << 5) + Buffer[Index];
    }
    return Hash;
}

inline u64 HashCString1(const char *CString)
{
    u64 Hash = 5381;
    while (*CString)
    {
        Hash += (Hash << 5) + *CString++;
    }
    return Hash;
}

inline u64 HashBuffer2(const u8 *Buffer, size_t BufferLength)
{
    u64 Hash = 14695981039346656037u;
    for (size_t Index = 0; Index < BufferLength; Index++)
    {
        Hash ^= Buffer[Index];
        Hash *= 1099511628211u;
    }
    return Hash;
}

inline u64 HashCString2(const char *CString)
{
    u64 Hash = 14695981039346656037u;
    while (*CString)
    {
        Hash ^= *CString++;
        Hash *= 1099511628211u;
    }
    return Hash;
}