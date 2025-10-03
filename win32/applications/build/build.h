#pragma once

#include "portable\shared\base_types.h"
#include "portable\shared\memory\linear_allocator.h"
#include "win32\shared\shell\console.h"

struct environment_info
{
    linear_allocator GlobalAllocator;
    i32 argc;
    char **argv;
    char *WorkspaceDirectoryPath;
    console_context *ConsoleContext;
};