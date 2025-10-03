#include <Windows.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\memory\linear_allocator.h"
#include "portable\shared\strings\cstrings.h"
#include "portable\shared\structures\singly_linked_list.h"

#include "win32\shared\file_system\folders.h"

singly_linked_list_node *GetListOfFilesInFolder(char *DirectoryPath, char *Extension, linear_allocator *Allocator)
{
    singly_linked_list_node *Result = NULL;
    char *FilesWildcard = Create3StringCombination(DirectoryPath, "\\*", Extension, Allocator);

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                Result = HeadPushSinglyLinkedListNode
                (
                    Result,
                    Create3StringCombination
                    (
                        DirectoryPath,
                        "\\",
                        FindOperationData.cFileName,
                        Allocator
                    ),
                    Allocator
                );
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
    FindClose(FindHandle);
    return Result;
}