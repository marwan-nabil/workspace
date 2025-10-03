#include <Windows.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\memory\linear_allocator.h"
#include "portable\shared\strings\format_strings.h"
#include "portable\shared\strings\cstrings.h"

#include "win32\shared\shell\console.h"
#include "win32\shared\file_system\folders.h"
#include "win32\shared\file_system\files.h"
#include "win32\shared\system\processes.h"

singly_linked_list_node *CollectAllBuildSystemDependencies
(
    char *WorkspaceDirectoryPath,
    linear_allocator *Allocator
)
{
    singly_linked_list_node *BuildSystemDependenciesList = GetListOfFilesInFolder
    (
        Create2StringCombination
        (
            WorkspaceDirectoryPath,
            "\\win32\\applications\\build",
            Allocator
        ),
        ".cpp",
        Allocator
    );

    char *ExternalBuildDependencies[] =
    {
        "\\win32\\shared\\shell\\console.h",
        "\\win32\\shared\\shell\\console.cpp",
        "\\win32\\shared\\system\\processes.h",
        "\\win32\\shared\\system\\processes.cpp",
    };

    for (u32 Index = 0; Index < ArrayCount(ExternalBuildDependencies); Index++)
    {
        BuildSystemDependenciesList = HeadPushSinglyLinkedListNode
        (
            BuildSystemDependenciesList,
            Create2StringCombination
            (
                WorkspaceDirectoryPath,
                ExternalBuildDependencies[Index],
                Allocator
            ),
            Allocator
        );
    }
    return BuildSystemDependenciesList;
}

u64 GetHashOfDependencies(singly_linked_list_node *BuildSystemDependenciesList)
{
    u64 Hash = 5381;
    singly_linked_list_node *CurrentNode = BuildSystemDependenciesList;
    while (CurrentNode)
    {
        read_file_result SourceFile = ReadFileIntoMemory((char *)CurrentNode->Value);
        char *FileMemory = (char *)SourceFile.FileMemory;
        for (u32 Index = 0; Index < SourceFile.Size; Index++)
        {
            Hash = ((Hash << 5) + Hash) + FileMemory[Index];
        }
        FreeFileMemory(SourceFile);
        CurrentNode = CurrentNode->NextNode;
    }
    return Hash;
}

int main(int argc, char **argv)
{
    char *WorkspaceDirectoryPath = _getcwd(NULL, 0);

    console_context ConsoleContext;
    InitializeConsole(&ConsoleContext);
    ConsolePrintColored("INFO: bootstrapper.cpp: rebuilding build system started.\n", FOREGROUND_BLUE, &ConsoleContext);

    linear_allocator GlobalAllocator;
    GlobalAllocator.Size = MegaBytes(1);
    GlobalAllocator.Used = 0;
    GlobalAllocator.BaseAddress = (u8 *)malloc(GlobalAllocator.Size);

    singly_linked_list_node *BuildSystemDependenciesList =
        CollectAllBuildSystemDependencies(WorkspaceDirectoryPath, &GlobalAllocator);
    u64 CurrentHash = GetHashOfDependencies(BuildSystemDependenciesList);

    u64 PreviousHash = 0;
    char *HashFilePath = Create2StringCombination
    (
        WorkspaceDirectoryPath,
        "\\build\\bootstrapper\\build_hash.bin",
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
        ConsolePrintColored("INFO: bootstrapper.cpp: rebuilding build system skipped.\n", FOREGROUND_BLUE, &ConsoleContext);
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
    char *TargetFolderString = Create2StringCombination
    (
        WorkspaceDirectoryPath,
        "\\build\\build",
        &GlobalAllocator
    );

    CreateDirectoryA(TargetFolderString, NULL);
    b8 Result = (b8)SetCurrentDirectoryA(TargetFolderString);
    if (Result)
    {
        Result = CreateProcessAndWait
        (
            CompilationCommandString,
            (HANDLE)_get_osfhandle(_fileno(stdout)),
            &ConsoleContext
        );
        SetCurrentDirectoryA(WorkspaceDirectoryPath);
    }

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