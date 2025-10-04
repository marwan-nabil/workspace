#include <Windows.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\memory\linear_allocator.h"
#include "portable\shared\strings\format_strings.h"
#include "portable\shared\strings\cstrings.h"
#include "portable\shared\hashing\hashing.h"

#include "win32\shared\shell\console.h"
#include "win32\shared\file_system\folders.h"
#include "win32\shared\file_system\files.h"
#include "win32\shared\system\processes.h"

#define FILES_HASHTABLE_SIZE 32

struct file_hashtable_entry
{
    char *FileName;
    u64 ContentHash;
    file_hashtable_entry *NextEntry;
};

struct translation_unit_info
{
    char *SourceFilePath;
    u64 AccumulativeHash;
};

file_hashtable_entry GlobalFilesHashTable[FILES_HASHTABLE_SIZE];

singly_linked_list_node *
GetListOfIncludedHeaderPaths
(
    const char *SourceBuffer,
    size_t SourceBufferSize,
    char *IncludePathPrefix,
    linear_allocator *Allocator
)
{
    singly_linked_list_node *Result = NULL;
    for (size_t Index = 0; Index < SourceBufferSize;)
    {
        if (SourceBuffer[Index] == '#')
        {
            
        }
        else
        {
            Index++;
        }
    }
    return Result;
}

u64 HashHeaderDependencies
(
    read_file_result SourceFile,
    file_hashtable_entry *FilesHashTable,
    u32 FilesHashTableSize,
    char *IncludePathPrefix,
    linear_allocator *Allocator
)
{

}

singly_linked_list_node *
AddTranslationUnitInfo
(
    singly_linked_list_node *ListHead,
    char *SourceFilePath,
    file_hashtable_entry *FilesHashTable,
    u32 FilesHashTableSize,
    char *IncludePathPrefix,
    linear_allocator *Allocator
)
{
    linear_allocator LoaclAllocator;
    LoaclAllocator.Size = MegaBytes(1);
    LoaclAllocator.Used = 0;
    LoaclAllocator.BaseAddress = (u8 *)malloc(LoaclAllocator.Size);

    translation_unit_info *TranslationUnitInfo = (translation_unit_info *)PushOntoMemoryArena
    (
        Allocator,
        sizeof(translation_unit_info),
        FALSE
    );
    TranslationUnitInfo->SourceFilePath = SourceFilePath;
    read_file_result SourceFile = ReadFileIntoMemory(SourceFilePath);
    TranslationUnitInfo->AccumulativeHash = HashBuffer1((const u8 *)SourceFile.FileMemory, SourceFile.Size);
    TranslationUnitInfo->AccumulativeHash += HashHeaderDependencies
    (
        SourceFile,
        FilesHashTable,
        FilesHashTableSize,
        IncludePathPrefix,
        &LoaclAllocator
    );
    FreeFileMemory(SourceFile);
    free(LoaclAllocator.BaseAddress);

    singly_linked_list_node *Result = HeadPushSinglyLinkedListNode(ListHead, TranslationUnitInfo, Allocator);
    return Result;
}

int main(int argc, char **argv)
{
    char *WorkspaceDirectoryPath = _getcwd(NULL, 0);

    console_context ConsoleContext;
    InitializeConsole(&ConsoleContext);

    ConsolePrintColored
    (
        "INFO: bootstrapper.cpp: checking if build system has changed since last rebuild.\n",
        FOREGROUND_BLUE,
        &ConsoleContext
    );

    linear_allocator GlobalAllocator;
    GlobalAllocator.Size = MegaBytes(2);
    GlobalAllocator.Used = 0;
    GlobalAllocator.BaseAddress = (u8 *)malloc(GlobalAllocator.Size);

    singly_linked_list_node *TranslationUnitInfoList = NULL;

#if 1
    singly_linked_list_node *SourcesList = GetListOfFilesInFolder
    (
        Create2StringCombination
        (
            WorkspaceDirectoryPath,
            "\\win32\\applications\\build",
            &GlobalAllocator
        ),
        ".cpp",
        &GlobalAllocator
    );
    SourcesList = HeadPushSinglyLinkedListNode
    (
        SourcesList,
        Create2StringCombination
        (
            WorkspaceDirectoryPath,
            "\\win32\\shared\\shell\\console.cpp",
            &GlobalAllocator
        ),
        &GlobalAllocator
    );
    SourcesList = HeadPushSinglyLinkedListNode
    (
        SourcesList,
        Create2StringCombination
        (
            WorkspaceDirectoryPath,
            "\\win32\\shared\\system\\processes.cpp",
            &GlobalAllocator
        ),
        &GlobalAllocator
    );

    singly_linked_list_node *CurrentNode = SourcesList;
    while (CurrentNode)
    {
        TranslationUnitInfoList = AddTranslationUnitInfo
        (
            TranslationUnitInfoList,
            (char *)CurrentNode->Value,
            GlobalFilesHashTable,
            FILES_HASHTABLE_SIZE,
            WorkspaceDirectoryPath,
            &GlobalAllocator
        );
        CurrentNode = CurrentNode->NextNode;
    }

#else
    singly_linked_list_node *BuildSystemDependenciesList =
        CollectAllBuildSystemDependencies(WorkspaceDirectoryPath, &GlobalAllocator);

    u64 CurrentHash = GetHashOfDependencies(BuildSystemDependenciesList);

    u64 PreviousHash = 0;
    char *HashFilePath = Create2StringCombination
    (
        WorkspaceDirectoryPath,
        "\\build\\bootstrapper\\file_hashes.bin",
        &GlobalAllocator
    );

    read_file_result HashFile = ReadFileIntoMemory(HashFilePath);
    if (HashFile.FileMemory)
    {
        memcpy((void *)&PreviousHash, HashFile.FileMemory, HashFile.Size);
        FreeFileMemory(HashFile);
    }

    if (CurrentHash == PreviousHash)
    {
        ConsolePrintColored("INFO: bootstrapper.cpp: no change, skip rebuilding the build system.\n", FOREGROUND_BLUE, &ConsoleContext);
        return 0;
    }

    WriteFileFromMemory(HashFilePath, (void *)&CurrentHash, 8);

    char *CompilationCommandFormatString =
    "cl.exe /nologo /Od /Z7 /Oi /FC /GR- /EHa- "
    "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 "
    "/D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS /I%0 "
    "%0\\win32\\applications\\build\\*.cpp "
    "%0\\win32\\shared\\shell\\console.cpp "
    "%0\\win32\\shared\\system\\processes.cpp "
    "/link /subsystem:console /incremental:no /opt:ref Shlwapi.lib /Fe:build.exe";

    char *FormatStringArgs[] =
    {
        WorkspaceDirectoryPath
    };

    char *CompilationCommandString = ResolveFormatString
    (
        CompilationCommandFormatString,
        FormatStringArgs,
        ArrayCount(FormatStringArgs),
        &GlobalAllocator
    );
#endif

    char *TargetFolderString = Create2StringCombination
    (
        WorkspaceDirectoryPath,
        "\\build\\build",
        &GlobalAllocator
    );

    b32 Result = CreateDirectoryA(TargetFolderString, NULL);
    if (!Result) goto main_end;
    
    Result = SetCurrentDirectoryA(TargetFolderString);
    if (!Result) goto main_end;
    
    Result = CreateProcessAndWait
    (
        // CompilationCommandString,
        "cl.exe",
        (HANDLE)_get_osfhandle(_fileno(stdout)),
        &ConsoleContext
    );
    if (!Result) goto main_end;

    Result = SetCurrentDirectoryA(WorkspaceDirectoryPath);

main_end:
    if (Result)
    {
        ConsolePrintColored("INFO: bootstrapper.cpp: rebuilding build system succeeded.\n", FOREGROUND_GREEN, &ConsoleContext);
        return 0;
    }
    else
    {
        ConsolePrintColored("ERROR: bootstrapper.cpp: rebuilding build system failed.\n", FOREGROUND_RED, &ConsoleContext);
        return 1;
    }
}