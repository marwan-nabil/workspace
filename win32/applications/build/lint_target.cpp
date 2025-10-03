#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#include <strsafe.h>
#include <io.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "portable\shared\memory\linear_allocator.h"
#include "portable\shared\structures\singly_linked_list.h"
#include "portable\shared\strings\cstrings.h"

#include "win32\shared\shell\console.h"
#include "win32\shared\system\processes.h"

#include "win32\applications\build\build.h"

b8 BuildLintTarget(environment_info *EnvironmentInfo)
{
    singly_linked_list_node *CompilerCommandListHead = HeadPushSinglyLinkedListNode
    (
        NULL,
        "cl.exe /nologo /Z7 /FC /Oi /GR- /EHa- /MTd /fp:fast /fp:except- "
        "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 "
        "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS /I",
        &EnvironmentInfo->GlobalAllocator
    );
    singly_linked_list_node *CompilerCommandListTail = TailPushSinglyLinkedListNode
    (
        CompilerCommandListHead,
        EnvironmentInfo->WorkspaceDirectoryPath,
        &EnvironmentInfo->GlobalAllocator
    );
    CompilerCommandListTail = TailPushSinglyLinkedListNode
    (
        CompilerCommandListTail,
        " ",
        &EnvironmentInfo->GlobalAllocator
    );

    char *SourceFiles[] = 
    {
        "\\win32\\applications\\lint\\lint.cpp ",
        "\\win32\\shared\\file_system\\files.cpp "
    };

    for (u32 Index = 0; Index < ArrayCount(SourceFiles); Index++)
    {
        char *SourceFilePath = Create2StringCombination
        (
            EnvironmentInfo->WorkspaceDirectoryPath,
            SourceFiles[Index],
            &EnvironmentInfo->GlobalAllocator
        );
        CompilerCommandListTail = TailPushSinglyLinkedListNode
        (
            CompilerCommandListTail,
            SourceFilePath,
            &EnvironmentInfo->GlobalAllocator
        );
    }

    CompilerCommandListTail = TailPushSinglyLinkedListNode
    (
        CompilerCommandListTail,
        "/link /subsystem:console /incremental:no /opt:ref Shlwapi.lib "
        "/Fe:lint.exe",
        &EnvironmentInfo->GlobalAllocator
    );

    char *CompilationCommandString = FlattenLinkedListOfStrings
    (
        CompilerCommandListHead,
        &EnvironmentInfo->GlobalAllocator
    );
    printf("COMPILER COMMAND: %s\n", CompilationCommandString);

    char *TargetFolderString = Create2StringCombination
    (
        EnvironmentInfo->WorkspaceDirectoryPath,
        "\\build\\lint",
        &EnvironmentInfo->GlobalAllocator
    );
    CreateDirectoryA(TargetFolderString, NULL);
    b8 Result = (b8)SetCurrentDirectoryA(TargetFolderString);
    if (Result)
    {
        Result = CreateProcessAndWait
        (
            CompilationCommandString,
            (HANDLE)_get_osfhandle(_fileno(stdout)),
            EnvironmentInfo->ConsoleContext
        );
        SetCurrentDirectoryA(EnvironmentInfo->WorkspaceDirectoryPath);
    }
    return Result;
}