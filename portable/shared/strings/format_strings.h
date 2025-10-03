#pragma once

#include <string.h>
#include <stdlib.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\memory\linear_allocator.h"

inline char *
ResolveFormatString(char *FormatString, char *FormatStringArgs[], u8 ArgCount, linear_allocator *Allocator)
{
    // TODO: create a test for this function
    size_t FormatStringSize = strlen(FormatString);
    size_t ResultStringSize = 0;

    char *CurrentChar = FormatString;
    while (*CurrentChar)
    {
        if (*CurrentChar == '%')
        {
            if (*(CurrentChar + 1) == '%')
            {
                ResultStringSize++;
                CurrentChar += 2;
            }
            else
            {
                char *EndPointer = NULL;
                u32 ArgumentIndex = strtol(CurrentChar + 1, &EndPointer, 10);
                if ((EndPointer == (CurrentChar + 1)) || (ArgumentIndex >= ArgCount))
                {
                    return NULL;
                }
                else
                {
                    ResultStringSize += strlen(FormatStringArgs[ArgumentIndex]);
                    CurrentChar = EndPointer;
                }
            }
        }
        else
        {
            ResultStringSize++;
            CurrentChar++;
        }
    }

    char *ResultString = (char *)PushOntoMemoryArena(Allocator, ResultStringSize + 1, FALSE);
    ResultString[ResultStringSize] = NULL;
    size_t WriteIndex = 0;
    CurrentChar = FormatString;
    while (*CurrentChar)
    {
        if (*CurrentChar == '%')
        {
            if (*(CurrentChar + 1) == '%')
            {
                ResultString[WriteIndex++] = '%';
                CurrentChar += 2;
            }
            else
            {
                char *EndPointer = NULL;
                u32 ArgumentIndex = strtol(CurrentChar + 1, &EndPointer, 10);

                size_t InjectedStringSize = strlen(FormatStringArgs[ArgumentIndex]);
                memcpy(ResultString + WriteIndex, FormatStringArgs[ArgumentIndex], InjectedStringSize);

                WriteIndex += InjectedStringSize;
                CurrentChar = EndPointer;
            }
        }
        else
        {
            ResultString[WriteIndex++] = *CurrentChar;
            CurrentChar++;
        }
    }

    return ResultString;
}