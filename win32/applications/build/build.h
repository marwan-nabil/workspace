#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\memory\arena.h"

struct environment_info
{
    memory_arena GlobalMemoryAllocator;
    i32 argc;
    char **argv;
    char *WorkspaceDirectoryPath;
};