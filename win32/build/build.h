#pragma once

#include "win32\shared\base_types.h"

struct environment_info
{
    i32 argc;
    char **argv;
    char *WorkspaceDirectoryPath;
};