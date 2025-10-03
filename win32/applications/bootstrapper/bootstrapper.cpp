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
#include "win32\shared\system\processes.h"

int main(int argc, char **argv)
{
    char *WorkspaceDirectoryPath = _getcwd(NULL, 0);

    console_context ConsoleContext;
    InitializeConsole(&ConsoleContext);

    linear_allocator GlobalAllocator;
    GlobalAllocator.Size = MegaBytes(1);
    GlobalAllocator.Used = 0;
    GlobalAllocator.BaseAddress = (u8 *)malloc(GlobalAllocator.Size);

    char *CompilationCommandFormatString =
        "cl.exe /nologo /Od /Z7 /Oi /FC /GR- /EHa- "
        "/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4456 /wd4996 /wd4018 "
        "/D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DENABLE_ASSERTIONS /I%0 "
        "%0\\win32\\applications\\build\\build.cpp "
        "%0\\win32\\applications\\build\\targets.cpp "
        "%0\\win32\\applications\\build\\lint_target.cpp "
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
        ConsolePrintColored("INFO: bootstrapper.cpp: build succeeded.\n", FOREGROUND_GREEN, &ConsoleContext);
        return 0;
    }
    else
    {
        ConsolePrintColored("ERROR: bootstrapper.cpp: build failed.\n", FOREGROUND_RED, &ConsoleContext);
        return 1;
    }
}