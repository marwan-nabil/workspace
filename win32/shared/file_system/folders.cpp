#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include <math.h>
#include <shlwapi.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\system\memory.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\strings\string_list.h"
#include "win32\shared\file_system\folders.h"

b32 CleanExtensionFromDirectory(const char *ExtensionToClean, const char *DirectoryPath, console_context *ConsoleContext)
{
    char FilesWildcard[MAX_STRING_LENGTH] = {};
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, DirectoryPath);
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, "\\*.");
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, ExtensionToClean);

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        DWORD LastError = GetLastError();
        if (LastError != ERROR_FILE_NOT_FOUND)
        {
            ConsolePrintColored("ERROR: FindFirstFileA() failed.\n", FOREGROUND_RED, ConsoleContext);
            return FALSE;
        }
    }
    else
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                char FoundFilePath[512] = {};
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, DirectoryPath);
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, "\\");
                StringCchCatA(FoundFilePath, 512, FindOperationData.cFileName);

                b32 DeleteResult = DeleteFile(FoundFilePath);
                if (DeleteResult == 0)
                {
                    DWORD LastError = GetLastError();
                    LPVOID ErrorMessageFromSystem;
                    FormatMessage
                    (
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        LastError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&ErrorMessageFromSystem,
                        0,
                        NULL
                    );

                    ConsoleSwitchColor(FOREGROUND_BLUE, ConsoleContext);
                    printf
                    (
                        "WARNING: Cannot delete the file %s. System error code for DeleteFile(): %lu == %s",
                        FoundFilePath, LastError, (const char *)ErrorMessageFromSystem
                    );
                    fflush(stdout);
                    ConsoleResetColor(ConsoleContext);

                    LocalFree(ErrorMessageFromSystem);
                }
                else
                {
                    printf("INFO: Deleted file: %s\n", FoundFilePath);
                }
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);

        DWORD LastErrorCode = GetLastError();
        if (LastErrorCode != ERROR_NO_MORE_FILES)
        {
            ConsoleSwitchColor(FOREGROUND_RED, ConsoleContext);
            printf("ERROR: cleanup process did not finish properly, please debug.\n");
            printf("ERROR: last error code is %d\n", LastErrorCode);
            printf("ERROR: extension with error is %s\n", ExtensionToClean);
            fflush(stdout);
            ConsoleResetColor(ConsoleContext);
            return FALSE;
        }
    }

    FindClose(FindHandle);
    return TRUE;
}

b32 DoesDirectoryExist(const char *DirectoryPath)
{
    DWORD FileAttributes = GetFileAttributes(DirectoryPath);
    if
    (
        (FileAttributes != INVALID_FILE_ATTRIBUTES) &&
        (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    )
    {
        return TRUE;
    }
    return FALSE;
}

void DeleteDirectoryCompletely(const char *DirectoryPath)
{
    SHFILEOPSTRUCT ShellOperation;
    ShellOperation.hwnd = NULL;
    ShellOperation.wFunc = FO_DELETE;
    ShellOperation.pFrom = DirectoryPath;
    ShellOperation.pTo = "";
    ShellOperation.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    ShellOperation.fAnyOperationsAborted = FALSE;
    ShellOperation.hNameMappings = 0;
    ShellOperation.lpszProgressTitle = "";

    SHFileOperation(&ShellOperation);
}

void EmptyDirectory(const char *DirectoryPath)
{
    char SearchPattern[1024] = {};
    StringCchCatA(SearchPattern, 1024, DirectoryPath);
    StringCchCatA(SearchPattern, 1024, "\\*");

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(SearchPattern, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if
                (
                    (strcmp(FindOperationData.cFileName, ".") != 0) &&
                    (strcmp(FindOperationData.cFileName, "..") != 0)
                )
                {
                    char SubDirectoryPath[1024] = {};
                    StringCchCatA(SubDirectoryPath, 1024, DirectoryPath);
                    StringCchCatA(SubDirectoryPath, 1024, "\\");
                    StringCchCatA(SubDirectoryPath, 1024, FindOperationData.cFileName);
                    DeleteDirectoryCompletely(SubDirectoryPath);
                }
            }
            else
            {
                char FoundFilePath[1024] = {};
                StringCchCatA(FoundFilePath, 1024, DirectoryPath);
                StringCchCatA(FoundFilePath, 1024, "\\");
                StringCchCatA(FoundFilePath, 1024, FindOperationData.cFileName);

                b32 DeleteResult = DeleteFile(FoundFilePath);
                if (DeleteResult == 0)
                {
                    DWORD LastError = GetLastError();
                    LPVOID ErrorMessageFromSystem;
                    FormatMessage
                    (
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        LastError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&ErrorMessageFromSystem,
                        0,
                        NULL
                    );

                    printf
                    (
                        "WARNING: Cannot delete the file %s. System error code for DeleteFile(): %lu == %s",
                        FoundFilePath, LastError, (const char *)ErrorMessageFromSystem
                    );
                    fflush(stdout);
                    LocalFree(ErrorMessageFromSystem);
                }
                else
                {
                    printf("INFO: Deleted file: %s\n", FoundFilePath);
                }
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
}

string_node *GetListOfFilesInFolder(char *DirectoryPath)
{
    string_node *Result = NULL;
    char FilesWildcard[MAX_STRING_LENGTH] = {};
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, DirectoryPath);
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, "\\*");

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                char FoundFilePath[MAX_STRING_LENGTH] = {};
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, DirectoryPath);
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, "\\");
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, FindOperationData.cFileName);
                PushStringNode(&Result, FoundFilePath);
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
    FindClose(FindHandle);
    return Result;
}

string_node *GetListOfFilesWithNameInFolder(char *DirectoryPath, char *FileName)
{
    string_node *Result = NULL;
    char FilesWildcard[MAX_STRING_LENGTH] = {};
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, DirectoryPath);
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, "\\");
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, FileName);

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                char FoundFilePath[MAX_STRING_LENGTH] = {};
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, DirectoryPath);
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, "\\");
                StringCchCatA(FoundFilePath, MAX_STRING_LENGTH, FindOperationData.cFileName);
                PushStringNode(&Result, FoundFilePath);
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
    FindClose(FindHandle);
    return Result;
}

void GetFilesByWildCard(char *DirectoryPath, char *WildCard, string_node **List)
{
    char FilesWildcard[MAX_STRING_LENGTH] = {};
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, DirectoryPath);
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, "\\*");

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if
                (
                    (strcmp(FindOperationData.cFileName, ".") == 0) ||
                    (strcmp(FindOperationData.cFileName, "..") == 0)
                )
                {
                    continue;
                }

                char SubDirectoryPath[MAX_STRING_LENGTH] = {};
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, DirectoryPath);
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, "\\");
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, FindOperationData.cFileName);
                GetFilesByWildCard(SubDirectoryPath, WildCard, List);
            }
            else if (PathMatchSpecA(FindOperationData.cFileName, WildCard))
            {
                string_node *NewStringNode = (string_node *)malloc(sizeof(string_node));
                *NewStringNode = {};
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), DirectoryPath);
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), "\\");
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FindOperationData.cFileName);
                NewStringNode->NextNode = *List;
                *List = NewStringNode;
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
}

void GetFilesByName(char *DirectoryPath, char *FileName, memory_arena *Memory, string_node **List)
{
    char FilesWildcard[MAX_STRING_LENGTH] = {};
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, DirectoryPath);
    StringCchCatA(FilesWildcard, MAX_STRING_LENGTH, "\\*");

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FilesWildcard, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if
                (
                    (strcmp(FindOperationData.cFileName, ".") == 0) ||
                    (strcmp(FindOperationData.cFileName, "..") == 0)
                )
                {
                    continue;
                }

                char SubDirectoryPath[MAX_STRING_LENGTH] = {};
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, DirectoryPath);
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, "\\");
                StringCchCatA(SubDirectoryPath, MAX_STRING_LENGTH, FindOperationData.cFileName);
                GetFilesByName(SubDirectoryPath, FileName, Memory, List);
            }
            else if (strcmp(FindOperationData.cFileName, FileName) == 0)
            {
                string_node *NewStringNode = (string_node *)PushOntoMemoryArena(Memory, sizeof(string_node), true);
                *NewStringNode = {};
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), DirectoryPath);
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), "\\");
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FindOperationData.cFileName);
                NewStringNode->NextNode = *List;
                *List = NewStringNode;
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
}

#if 0
string_node *GetListOfFilesWithNameInFolder3(char *FolderPath, char *FileName, char *FileNameWildCard, b8 Recursive)
{
    string_node *ResultList = NULL;
    char FileSearchString[MAX_STRING_LENGTH] = {};
    StringCchCatA(FileSearchString, MAX_STRING_LENGTH, FolderPath);
    StringCchCatA(FileSearchString, MAX_STRING_LENGTH, "\\*");

    WIN32_FIND_DATAA FindOperationData;
    HANDLE FindHandle = FindFirstFileA(FileSearchString, &FindOperationData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindOperationData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if
                (
                    (strcmp(FindOperationData.cFileName, ".") == 0) ||
                    (strcmp(FindOperationData.cFileName, "..") == 0)
                )
                {
                    continue;
                }

                char SubDirectoryPath[1024] = {};
                StringCchCatA(SubDirectoryPath, 1024, FolderPath);
                StringCchCatA(SubDirectoryPath, 1024, "\\");
                StringCchCatA(SubDirectoryPath, 1024, FindOperationData.cFileName);
                GetFilesByName(SubDirectoryPath, FileName, FileNameWildCard, List);
            }
            else if (FileName && (strcmp(FindOperationData.cFileName, FileName) == 0))
            {
                string_node *NewStringNode = (string_node *)malloc(sizeof(string_node));
                *NewStringNode = {};
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FolderPath);
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), "\\");
                StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FindOperationData.cFileName);
                NewStringNode->NextNode = *List;
                *List = NewStringNode;
            }
            else if (!FileName && FileNameWildCard)
            {
                // make sure the file name matches the wildcard
                BOOL Result = PathMatchSpecA(FindOperationData.cFileName, FileNameWildCard);
                if (Result)
                {
                    string_node *NewStringNode = (string_node *)malloc(sizeof(string_node));
                    *NewStringNode = {};
                    StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FolderPath);
                    StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), "\\");
                    StringCchCatA(NewStringNode->String, ArrayCount(NewStringNode->String), FindOperationData.cFileName);
                    NewStringNode->NextNode = *List;
                    *List = NewStringNode;
                }
            }
        } while (FindNextFileA(FindHandle, &FindOperationData) != 0);
    }
}
#endif