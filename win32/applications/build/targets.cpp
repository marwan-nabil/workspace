#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_structures\singly_linked_list.h"
#include "win32\shared\strings\cstring.h"
#include "win32\build\build.h"
#include "win32\build\targets.h"

target_configuration GlobalTargetConfigurations[NUMBER_OF_CONFIGURED_TARGETS] =
{
    {"lint", &BuildLintTarget},
};

b8 BuildLintTarget(environment_info *EnvironmentInfo)
{
    singly_linked_list_node *CompilerCommandListHead = HeadPushSinglyLinkedListNode
    (
        NULL,
        "cl.exe /nologo /Z7 /FC /Oi /GR- /EHa- /MTd /fp:fast /fp:except- "
        "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 "
        "/DENABLE_ASSERTIONS /D_CRT_SECURE_NO_WARNINGS /DJOB_PER_FILE /I",
        &EnvironmentInfo->GlobalMemoryAllocator
    );
    singly_linked_list_node *CompilerCommandListTail = TailPushSinglyLinkedListNode
    (
        CompilerCommandListHead,
        EnvironmentInfo->WorkspaceDirectoryPath,
        &EnvironmentInfo->GlobalMemoryAllocator
    );
    CompilerCommandListTail = TailPushSinglyLinkedListNode
    (
        CompilerCommandListTail,
        " \\win32\\lint\\lint.cpp "
        "\\win32\\shared\\file_system\\files.cpp "
        "\\win32\\shared\\strings\\path_handling.cpp "
        "/link /subsystem:console /incremental:no /opt:ref Shlwapi.lib "
        "/Fe:lint.exe",
        &EnvironmentInfo->GlobalMemoryAllocator
    );

    char *FlattenedCommand = FlattenLinkedListOfStrings
    (
        CompilerCommandListHead,
        &EnvironmentInfo->GlobalMemoryAllocator
    );
    printf("COMPILER COMMAND: %s\n", FlattenedCommand);
    return TRUE;
}