#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\memory\arena.h"
#include "win32\build\build.h"
#include "win32\build\targets.h"

int main(int argc, char **argv)
{
    b8 BuildSuccess = FALSE;
    b8 TargetFound = FALSE;

    environment_info EnvironmentInfo = {};
    EnvironmentInfo.argc = argc;
    EnvironmentInfo.argv = argv;
    EnvironmentInfo.WorkspaceDirectoryPath = _getcwd(NULL, 0);
    EnvironmentInfo.GlobalMemoryAllocator.Size = MegaBytes(1);
    EnvironmentInfo.GlobalMemoryAllocator.BaseAddress = (u8 *)malloc(EnvironmentInfo.GlobalMemoryAllocator.Size);

    console_context ConsoleContext;
    InitializeConsole(&ConsoleContext);

    if (argc < 2)
    {
        ConsolePrintColored("ERROR: build.cpp: no build target specified.\n", FOREGROUND_RED, &ConsoleContext);
        BuildSuccess = FALSE;
        goto main_end;
    }

    for (u32 TargetIndex = 0; TargetIndex < ArrayCount(GlobalTargetConfigurations); TargetIndex++)
    {
        if (strcmp(argv[1], GlobalTargetConfigurations[TargetIndex].TargetName) == 0)
        {
            TargetFound = TRUE;
            BuildSuccess = GlobalTargetConfigurations[TargetIndex].TargetEntryFunction(&EnvironmentInfo);
            break;
        }
    }

    if (!TargetFound)
    {
        ConsolePrintColored("ERROR: build.cpp: target not found.\n", FOREGROUND_RED, &ConsoleContext);
    }

main_end:
    if (BuildSuccess)
    {
        ConsolePrintColored("INFO: build.cpp: build succeeded.\n", FOREGROUND_GREEN, &ConsoleContext);
        return 0;
    }
    else
    {
        ConsolePrintColored("ERROR: build.cpp: build failed.\n", FOREGROUND_RED, &ConsoleContext);
        return -1;
    }
}