// TODO:
// - make linting smarter by not having to touch all the files every time.
//     - only lint files that were modified since last lint time.
//     - this requires that we save the last lint time in a metadata file.
#include <Windows.h>
#include <stdio.h>
#include <fileapi.h>
#include <direct.h>
#include <time.h>
#include <shlwapi.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\structures\singly_linked_list.h"
#include "portable\shared\strings\cstrings.h"
#include "portable\shared\memory\linear_allocator.h"

#include "win32\shared\file_system\files.h"
#include "win32\shared\math\scalar_conversions.h"

#include "lint.h"

b8 ScanDirectory
(
    char *DirectoryPath,
    singly_linked_list_node **FoundDirectoriesList,
    singly_linked_list_node **FoundFilesList,
    u32 *DirectoriesToLintCount,
    u32 *FilesToLintCount,
    linear_allocator *Allocator
)
{
    b8 DirectoryContainsSourceCode = FALSE;
    char *DirectoryWildcard = Create2StringCombination(DirectoryPath, "\\*", Allocator);

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(DirectoryWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if
                (
                    (strcmp(FindOperationData.cFileName, ".") == 0) ||
                    (strcmp(FindOperationData.cFileName, "..") == 0) ||
                    (strcmp(FindOperationData.cFileName, ".git") == 0) ||
                    (strcmp(FindOperationData.cFileName, ".vscode") == 0)
                )
                {
                    continue;
                }

                char *FoundDirectoryPath = Create3StringCombination
                (
                    DirectoryPath,
                    "\\",
                    FindOperationData.cFileName,
                    Allocator
                );
                ScanDirectory
                (
                    FoundDirectoryPath,
                    FoundDirectoriesList,
                    FoundFilesList,
                    DirectoriesToLintCount,
                    FilesToLintCount,
                    Allocator
                );
            }
            else
            {
                char *Extension = PathFindExtensionA(FindOperationData.cFileName);
                if
                (
                    (strcmp(Extension, ".cpp") == 0) ||
                    (strcmp(Extension, ".h") == 0) ||
                    (strcmp(Extension, ".c") == 0) ||
                    (strcmp(Extension, ".s") == 0) ||
                    (strcmp(Extension, ".asm") == 0) ||
                    (strcmp(Extension, ".i") == 0) ||
                    (strcmp(Extension, ".inc") == 0)
                )
                {
                    if (!DirectoryContainsSourceCode)
                    {
                        DirectoryContainsSourceCode = TRUE;
                        *FoundDirectoriesList = HeadPushSinglyLinkedListNode
                        (
                            *FoundDirectoriesList,
                            DirectoryPath,
                            Allocator
                        );
                        (*DirectoriesToLintCount)++;
                    }

                    char *FoundFilePath = Create3StringCombination
                    (
                        DirectoryPath,
                        "\\",
                        FindOperationData.cFileName,
                        Allocator
                    );

                    *FoundFilesList = HeadPushSinglyLinkedListNode
                    (
                        *FoundFilesList,
                        FoundFilePath,
                        Allocator
                    );
                    (*FilesToLintCount)++;
                }
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);

        DWORD LastErrorCode = GetLastError();
        if (LastErrorCode != ERROR_NO_MORE_FILES)
        {
            printf("ERROR: enumerating directories failed.\n");
            printf("ERROR: last error code is %d\n", LastErrorCode);
            return FALSE;
        }
    }
    else
    {
        DWORD LastError = GetLastError();
        if (LastError != ERROR_FILE_NOT_FOUND)
        {
            printf("ERROR: FindFirstFileA() failed.\n");
            return FALSE;
        }
    }

    FindClose(FindHandle);
    return TRUE;
}

b32 LintFile(char *FilePath)
{
    read_file_result SourceFile = ReadFileIntoMemory(FilePath);
    u8 *FileBuffer = (u8 *)VirtualAlloc
    (
        0,
        SourceFile.Size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    memset(FileBuffer, 0, SourceFile.Size);

    u8 *ScanPointer = (u8 *)SourceFile.FileMemory;
    u8 *WritePointer = FileBuffer;

    u8 *CopyStartPointer = ScanPointer;
    u8 *WhiteSpaceRegionPointer = ScanPointer;
    b32 InsideWhiteSpaceRegion = FALSE;

    while (1)
    {
        if (*ScanPointer == ' ')
        {
            if (!InsideWhiteSpaceRegion)
            {
                InsideWhiteSpaceRegion = TRUE;
                WhiteSpaceRegionPointer = ScanPointer;
            }
            ScanPointer++;
        }
        else if
        (
            ((*ScanPointer == '\r') && (*(ScanPointer + 1) == '\n')) ||
            (*ScanPointer == '\0')
        )
        {
            u8 *CopyEndPointer;

            if (InsideWhiteSpaceRegion)
            {
                CopyEndPointer = WhiteSpaceRegionPointer;
            }
            else
            {
                CopyEndPointer = ScanPointer;
            }

            while (CopyStartPointer != CopyEndPointer)
            {
                *WritePointer++ = *CopyStartPointer++;
            }

            if (*ScanPointer == '\0')
            {
                break;
            }
            else
            {
                *WritePointer++ = *ScanPointer++;
                *WritePointer++ = *ScanPointer++;

                CopyStartPointer = ScanPointer;
                WhiteSpaceRegionPointer = ScanPointer;
                InsideWhiteSpaceRegion = FALSE;
            }
        }
        else
        {
            InsideWhiteSpaceRegion = FALSE;
            ScanPointer++;
        }
    }

    WritePointer--;

    while (1)
    {
        if ((*WritePointer == '\n') || (*WritePointer == '\r'))
        {
            WritePointer--;
        }
        else
        {
            WritePointer += 1;
            break;
        }
    }

    size_t OutputSize = WritePointer - FileBuffer;
    u8 *OutputFileMemory = (u8 *)VirtualAlloc
    (
        0,
        OutputSize,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    memcpy(OutputFileMemory, FileBuffer, OutputSize);

    b32 Result = WriteFileFromMemory(FilePath, OutputFileMemory, SafeTruncateUint64ToUint32(OutputSize));
    if (!Result)
    {
        return FALSE;
    }

    FreeFileMemory(SourceFile);
    VirtualFree(FileBuffer, 0, MEM_RELEASE);
    VirtualFree(OutputFileMemory, 0, MEM_RELEASE);
    return TRUE;
}

DWORD WINAPI
WorkerThreadEntry(void *ThreadParameter)
{
    lint_job_queue *LintJobQueue = (lint_job_queue *)ThreadParameter;
    while (LintJobQueue->TotalJobsDone < LintJobQueue->JobCount)
    {
        u64 LintJobIndex = InterlockedExchangeAdd64(&LintJobQueue->NextJobIndex, 1);
        if (LintJobIndex >= LintJobQueue->JobCount)
        {
            return TRUE;
        }

        LintJobQueue->Jobs[LintJobIndex].Result = LintFile(LintJobQueue->Jobs[LintJobIndex].FilePath);
        InterlockedExchangeAdd64(&LintJobQueue->TotalJobsDone, 1);
    }
    return TRUE;
}

int main(int argc, char **argv)
{
    clock_t StartTime = clock();
    char *WorkspaceDirectoryPath = _getcwd(NULL, 0);

    linear_allocator GlobalAllocator;
    GlobalAllocator.Size = MegaBytes(4);
    GlobalAllocator.Used = 0;
    GlobalAllocator.BaseAddress = (u8 *)malloc(GlobalAllocator.Size);

    singly_linked_list_node *ListOfDirectoriesToScan = NULL;
    ListOfDirectoriesToScan = HeadPushSinglyLinkedListNode
    (
        ListOfDirectoriesToScan,
        Create2StringCombination(WorkspaceDirectoryPath, "\\hardware", &GlobalAllocator),
        &GlobalAllocator
    );
    ListOfDirectoriesToScan = HeadPushSinglyLinkedListNode
    (
        ListOfDirectoriesToScan,
        Create2StringCombination(WorkspaceDirectoryPath, "\\i686-elf", &GlobalAllocator),
        &GlobalAllocator
    );
    ListOfDirectoriesToScan = HeadPushSinglyLinkedListNode
    (
        ListOfDirectoriesToScan,
        Create2StringCombination(WorkspaceDirectoryPath, "\\portable", &GlobalAllocator),
        &GlobalAllocator
    );
    ListOfDirectoriesToScan = HeadPushSinglyLinkedListNode
    (
        ListOfDirectoriesToScan,
        Create2StringCombination(WorkspaceDirectoryPath, "\\win32", &GlobalAllocator),
        &GlobalAllocator
    );

    u32 DirectoriesToLintCount = 0;
    u32 FilesToLintCount = 0;
    singly_linked_list_node *DirectoriesToLint = NULL;
    singly_linked_list_node *FilesToLint = NULL;

    singly_linked_list_node *CurrentNode = ListOfDirectoriesToScan;
    while (CurrentNode)
    {
        ScanDirectory
        (
            (char *)CurrentNode->Value,
            &DirectoriesToLint,
            &FilesToLint,
            &DirectoriesToLintCount,
            &FilesToLintCount,
            &GlobalAllocator
        );
        CurrentNode = CurrentNode->NextNode;
    }

    lint_job_queue LintJobQueue;
    LintJobQueue.JobCount = FilesToLintCount;
    LintJobQueue.Jobs = (lint_job *)PushOntoMemoryArena(&GlobalAllocator, sizeof(lint_job), FALSE);
    LintJobQueue.NextJobIndex = 0;
    LintJobQueue.TotalJobsDone = 0;

    for (u32 JobIndex = 0; JobIndex < LintJobQueue.JobCount; JobIndex++)
    {
        lint_job *Job = &LintJobQueue.Jobs[JobIndex];
        Job->Result = FALSE;
        Job->FilePath = (char *)FilesToLint->Value;
        FilesToLint = FilesToLint->NextNode;
    }

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    u32 ThreadCount = (u8)SystemInfo.dwNumberOfProcessors;

    printf("INFO: Creating %d threads for linting.\n", ThreadCount);

    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
    {
        DWORD ThreadId;
        HANDLE ThreadHandle = CreateThread(0, 0, WorkerThreadEntry, &LintJobQueue, 0, &ThreadId);
        CloseHandle(ThreadHandle);
    }

    while (LintJobQueue.TotalJobsDone < LintJobQueue.JobCount) {}

    for (u32 ResultIndex = 0; ResultIndex < LintJobQueue.JobCount; ResultIndex++)
    {
        if (!LintJobQueue.Jobs[ResultIndex].Result)
        {
            printf("ERROR: Linting failed for file: %s\n", LintJobQueue.Jobs[ResultIndex].FilePath);
            return 1;
        }
    }

    printf("INFO: Linting succeeded in: %ld ms\n", clock() - StartTime);
    return 0;
}