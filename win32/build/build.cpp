#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#include <strsafe.h>

#include "win32\shared\base_types.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\file_system\folders.h"
#include "win32\build\build.h"

int main(int argc, char **argv)
{
    u32 BuildSuccess = FALSE;

    environment_info EnvironmentInfo = {};
    EnvironmentInfo.argc = argc;
    EnvironmentInfo.argv = argv;
    EnvironmentInfo.WorkspaceDirectoryPath = _getcwd(nullptr, 0);

    size_t PathSize = strlen(EnvironmentInfo.WorkspaceDirectoryPath) + 7;
    char *BuildDirectoryPath = (char *)malloc(PathSize);
    StringCchCatA(BuildDirectoryPath, PathSize, EnvironmentInfo.WorkspaceDirectoryPath);
    StringCchCatA(BuildDirectoryPath, PathSize, "\\build");
    BuildDirectoryPath[PathSize - 1] = 0;

    console_context ConsoleContext;
    InitializeConsole(&ConsoleContext);

    if (argc < 3)
    {
        ConsolePrintColored("ERROR: No build target.\n", FOREGROUND_RED, &ConsoleContext);
        BuildSuccess = FALSE;
        goto main_end;
    }

    if (strcmp(argv[2], "clean") == 0)
    {
        // TODO: loop over all targets, and invoke their clean functions
    }
    else if (strcmp(argv[2], "clean_all") == 0)
    {
        EmptyDirectory(BuildDirectoryPath);
        BuildSuccess = TRUE;
        goto main_end;
    }

main_end:
    if (BuildSuccess)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}