// TODO:
// - make linting smarter by not having to touch all the files every time.
//     - only lint files that were modified since last lint time.
//     - this requires that we save the last lint time in a metadata .json file.
// - exclude the data, output, tools folders from liniting.
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <strsafe.h>
#include <fileapi.h>
#include <direct.h>
#include <io.h>
#include <time.h>
#include <shlwapi.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\file_system\files.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\strings\path_handling.h"
#include "lint.h"

// TODO: remove this global path, all functions should use absolute paths
char RootDirectoryPath[1024];

b32 ProcessDirectory(char *DirectoryRelativePath, directory_node **FoundDirectoriesList, file_node **FoundFilesList)
{
    b32 DirectoryContainsSourceCode = FALSE;
    b32 Result = TRUE;

    char DirectoryWildcard[1024] = {};
    StringCchCatA(DirectoryWildcard, ArrayCount(DirectoryWildcard), RootDirectoryPath);
    StringCchCatA(DirectoryWildcard, ArrayCount(DirectoryWildcard), DirectoryRelativePath);
    StringCchCatA(DirectoryWildcard, ArrayCount(DirectoryWildcard), "\\*");

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

                char FoundDirectoryRelativePath[1024] = {};
                StringCchCatA(FoundDirectoryRelativePath, 1024, DirectoryRelativePath);
                StringCchCatA(FoundDirectoryRelativePath, 1024, "\\");
                StringCchCatA(FoundDirectoryRelativePath, 1024, FindOperationData.cFileName);
                Result = ProcessDirectory(FoundDirectoryRelativePath, FoundDirectoriesList, FoundFilesList) && Result;
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
                    (strcmp(Extension, ".i") == 0) ||
                    (strcmp(Extension, ".txt") == 0)
                )
                {
                    if (!DirectoryContainsSourceCode)
                    {
                        DirectoryContainsSourceCode = TRUE;
                        directory_node *NewDirectoryNode = (directory_node *)malloc(sizeof(directory_node));
                        *NewDirectoryNode = {};
                        StringCchCatA(NewDirectoryNode->DirectoryRelativePath, ArrayCount(NewDirectoryNode->DirectoryRelativePath), DirectoryRelativePath);
                        NewDirectoryNode->NextDirectoryNode = *FoundDirectoriesList;
                        *FoundDirectoriesList = NewDirectoryNode;
                    }

                    file_node *NewFileNode = (file_node *)malloc(sizeof(file_node));
                    *NewFileNode = {};
                    StringCchCatA(NewFileNode->FileRelativePath, ArrayCount(NewFileNode->FileRelativePath), DirectoryRelativePath);
                    StringCchCatA(NewFileNode->FileRelativePath, ArrayCount(NewFileNode->FileRelativePath), "\\");
                    StringCchCatA(NewFileNode->FileRelativePath, ArrayCount(NewFileNode->FileRelativePath), FindOperationData.cFileName);
                    NewFileNode->NextFileNode = *FoundFilesList;
                    *FoundFilesList = NewFileNode;
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

b32 LintFile(char *FileRelativePath)
{
    char FilePath[MAX_PATH] = {};
    StringCchCatA(FilePath, MAX_PATH, RootDirectoryPath);
    StringCchCatA(FilePath, MAX_PATH, FileRelativePath);

    read_file_result SourceFile = ReadFileIntoMemory(FilePath);

    u8 *BufferMemory = (u8 *)VirtualAlloc
    (
        0,
        SourceFile.Size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    ZeroMemory(BufferMemory, SourceFile.Size);

    u8 *ScanPointer = (u8 *)SourceFile.FileMemory;
    u8 *WritePointer = BufferMemory;

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

    u64 OutputSize = WritePointer - BufferMemory;

    u8 *OutputFileMemory = (u8 *)VirtualAlloc
    (
        0,
        OutputSize,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    memcpy(OutputFileMemory, BufferMemory, OutputSize);

    b32 Result = WriteFileFromMemory(FilePath, OutputFileMemory, SafeTruncateUint64ToUint32(OutputSize));
    if (!Result)
    {
        return FALSE;
    }

    FreeFileMemory(SourceFile);
    VirtualFree(BufferMemory, 0, MEM_RELEASE);
    VirtualFree(OutputFileMemory, 0, MEM_RELEASE);

    return TRUE;
}

b32 LintDirectoryWithWildcard(char *DirectoryRelativePath, char *FilesWildcard)
{
    char SearchPattern[1024] = {};
    StringCchCatA(SearchPattern, 1024, RootDirectoryPath);
    StringCchCatA(SearchPattern, 1024, DirectoryRelativePath);
    StringCchCatA(SearchPattern, 1024, FilesWildcard);

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(SearchPattern, &FindOperationData);

    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                char FoundFilePath[MAX_PATH] = {};
                StringCchCatA(FoundFilePath, MAX_PATH, DirectoryRelativePath);
                StringCchCatA(FoundFilePath, MAX_PATH, "\\");
                StringCchCatA(FoundFilePath, 512, FindOperationData.cFileName);

                LintFile(FoundFilePath);
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);

        DWORD LastErrorCode = GetLastError();
        if (LastErrorCode != ERROR_NO_MORE_FILES)
        {
            printf("ERROR: linting process did not finish properly, please debug.\n");
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

b32 LintDirectory(char *DirectoryRelativePath)
{
    char FilesWildcard[MAX_PATH] = {};
    StringCchCatA(FilesWildcard, MAX_PATH, "\\*.cpp");

    b32 Result = LintDirectoryWithWildcard(DirectoryRelativePath, FilesWildcard);

    *FilesWildcard = {};
    StringCchCatA(FilesWildcard, MAX_PATH, "\\*.h");

    Result = LintDirectoryWithWildcard(DirectoryRelativePath, FilesWildcard) && Result;

    *FilesWildcard = {};
    StringCchCatA(FilesWildcard, MAX_PATH, "\\*.s");

    Result = LintDirectoryWithWildcard(DirectoryRelativePath, FilesWildcard) && Result;

    return Result;
}

void ProcessLintJob(lint_job *LintJob)
{
    LintJob->Result = TRUE;

    if (LintJob->Type == LJT_DIRECTORY)
    {
        LintJob->Result = LintDirectory(LintJob->DirectoryRelativePath);
    }
    else if (LintJob->Type == LJT_FILE_LIST)
    {
        for (u32 FileIndex = 0; FileIndex < LintJob->NumberOfFiles; FileIndex++)
        {
            LintJob->Result = LintFile(LintJob->FileRelativePaths[FileIndex]) && LintJob->Result;
        }
    }
    else if (LintJob->Type == LJT_FILE)
    {
        LintJob->Result = LintFile(LintJob->FileRelativePath);
    }
}

DWORD WINAPI
WorkerThreadEntry(void *Parameter)
{
    lint_job_queue *LintJobQueue = (lint_job_queue *)Parameter;

    while (LintJobQueue->TotalJobsDone < LintJobQueue->JobCount)
    {
        u64 LintJobIndex = InterlockedExchangeAdd64(&LintJobQueue->NextJobIndex, 1);
        if (LintJobIndex >= LintJobQueue->JobCount)
        {
            return TRUE;
        }

        ProcessLintJob(&LintJobQueue->Jobs[LintJobIndex]);

        InterlockedExchangeAdd64(&LintJobQueue->TotalJobsDone, 1);
    }

    return TRUE;
}

int main(int argc, char **argv)
{
    _getcwd(RootDirectoryPath, sizeof(RootDirectoryPath));

    directory_node *FoundDirectoriesList = 0;
    file_node *FoundFilesList = 0;

    ProcessDirectory("", &FoundDirectoriesList, &FoundFilesList);

    u32 DirectoryCount = 0;
    directory_node *DirectoryListIterator = FoundDirectoriesList;
    while (DirectoryListIterator)
    {
        DirectoryCount++;
        DirectoryListIterator = DirectoryListIterator->NextDirectoryNode;
    }

    u32 FileCount = 0;
    file_node *FileListIterator = FoundFilesList;
    while (FileListIterator)
    {
        FileCount++;
        FileListIterator = FileListIterator->NextFileNode;
    }

    b32 Result = TRUE;

    clock_t StartTime = clock();

    lint_job_queue LintJobQueue;
#if defined(JOB_PER_DIRECTORY)
    LintJobQueue.JobCount = DirectoryCount;
#endif
#if defined(JOB_PER_FILE)
    LintJobQueue.JobCount = FileCount;
#endif
    LintJobQueue.NextJobIndex = 0;
    LintJobQueue.TotalJobsDone = 0;
    LintJobQueue.Jobs = (lint_job *)malloc(LintJobQueue.JobCount * sizeof(lint_job));

    for (u32 JobIndex = 0; JobIndex < LintJobQueue.JobCount; JobIndex++)
    {
        lint_job *Job = &LintJobQueue.Jobs[JobIndex];
        ZeroMemory(Job, sizeof(lint_job));
#if defined(JOB_PER_DIRECTORY)
        Job->Type = LJT_DIRECTORY;
        Job->DirectoryRelativePath = FoundDirectoriesList->DirectoryRelativePath;
        FoundDirectoriesList = FoundDirectoriesList->NextDirectoryNode;
#endif
#if defined(JOB_PER_FILE)
        Job->Type = LJT_FILE;
        Job->FileRelativePath = FoundFilesList->FileRelativePath;
        FoundFilesList = FoundFilesList->NextFileNode;
#endif
        Job->Result = FALSE;
    }

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    u32 ThreadCount = (u8)SystemInfo.dwNumberOfProcessors;

// #if defined(JOB_PER_DIRECTORY)
//     u32 ThreadCount = LintJobQueue.JobCount;
// #endif
// #if defined(JOB_PER_FILE)
//     u32 ThreadCount = LintJobQueue.JobCount / 4;
// #endif

    printf("\nCreated %d threads.\n", ThreadCount);

    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
    {
        DWORD ThreadId;
        HANDLE ThreadHandle =
            CreateThread(0, 0, WorkerThreadEntry, &LintJobQueue, 0, &ThreadId);
        CloseHandle(ThreadHandle);
    }

    while (LintJobQueue.TotalJobsDone < LintJobQueue.JobCount) {}

    for (u32 ResultIndex = 0; ResultIndex < LintJobQueue.JobCount; ResultIndex++)
    {
        Result = Result && LintJobQueue.Jobs[ResultIndex].Result;
    }

    printf("\nLinting time: %ld ms\n", clock() - StartTime);

    if (Result)
    {
        printf("\nLint succeeded\n");
        return 0;
    }
    else
    {
        printf("\nLint failed\n");
        return 1;
    }
}