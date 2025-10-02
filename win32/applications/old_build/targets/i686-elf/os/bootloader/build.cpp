#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\strings\path_handling.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\nasm.h"
#include "win32\tools\build\actions\gcc.h"

b32 BuildBootloaderImage(build_context *BuildContext)
{
    PushSubTarget(BuildContext, "bootloader");
    b32 BuildSuccess = FALSE;

    char *AssemblyFiles[] =
    {
        "\\i686-elf\\os\\bootloader\\entry.s",
        "\\i686-elf\\shared\\cpu\\io.s",
        "\\i686-elf\\shared\\bios\\disk.s"
    };

    for (u32 Index = 0; Index < ArrayCount(AssemblyFiles); Index++)
    {
        AddCompilerSourceFile(BuildContext, AssemblyFiles[Index]);

        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, MAX_PATH, "\\");
        StringCchCatA(ObjectFileName, MAX_PATH, GetPointerToLastSegmentFromPath(AssemblyFiles[Index], '\\'));
        StringCchCatA(ObjectFileName, MAX_PATH, ".elf");
        SetCompilerOutputObject(BuildContext, ObjectFileName);

        AddCompilerFlags(BuildContext, "-f elf");
        SetCompilerIncludePath(BuildContext, "\\");
        BuildSuccess = AssembleWithNasm(BuildContext);
        ClearBuildContext(BuildContext);
        if (!BuildSuccess)
        {
            return FALSE;
        }
    }

    char *CFiles[] =
    {
        "\\i686-elf\\os\\bootloader\\main.c",
        "\\i686-elf\\os\\bootloader\\tests.c",
        "\\i686-elf\\shared\\vga\\vga.c",
        "\\i686-elf\\shared\\storage\\disk\\disk.c",
        "\\i686-elf\\shared\\strings\\strings.c",
        "\\i686-elf\\shared\\strings\\path_handling.c",
        "\\i686-elf\\shared\\strings\\print.c",
        "\\i686-elf\\shared\\cpu\\timing.c",
        "\\i686-elf\\shared\\storage\\fat12\\get.c",
        "\\i686-elf\\shared\\storage\\fat12\\set.c",
        "\\i686-elf\\shared\\memory\\arena_allocator.c",
        "\\i686-elf\\shared\\memory\\memory.c",
        "\\i686-elf\\shared\\storage\\file_io\\file_io.c",
    };

    for (u32 Index = 0; Index < ArrayCount(CFiles); Index++)
    {
        AddCompilerSourceFile(BuildContext, CFiles[Index]);

        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, MAX_PATH, "\\");
        StringCchCatA(ObjectFileName, MAX_PATH, GetPointerToLastSegmentFromPath(CFiles[Index], '\\'));
        StringCchCatA(ObjectFileName, MAX_PATH, ".elf");
        SetCompilerOutputObject(BuildContext, ObjectFileName);

        AddCompilerFlags(BuildContext, "-std=c99 -g -ffreestanding -nostdlib");
        SetCompilerIncludePath(BuildContext, "\\");
        BuildSuccess = CompileWithGCC(BuildContext);
        if (!BuildSuccess)
        {
            return FALSE;
        }
        ClearBuildContext(BuildContext);
    }

    AddLinkerFlags(BuildContext, "-nostdlib -Wl,-Map=bootloader.map");
    SetLinkerScriptPath(BuildContext, "\\i686-elf\\os\\bootloader\\bootloader.lds");

    for (u32 Index = 0; Index < ArrayCount(AssemblyFiles); Index++)
    {
        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, MAX_PATH, "\\");
        StringCchCatA(ObjectFileName, MAX_PATH, GetPointerToLastSegmentFromPath(AssemblyFiles[Index], '\\'));
        StringCchCatA(ObjectFileName, MAX_PATH, ".elf");
        AddLinkerInputFile(BuildContext, ObjectFileName);
    }
    for (u32 Index = 0; Index < ArrayCount(CFiles); Index++)
    {
        char ObjectFileName[MAX_PATH] = {};
        StringCchCatA(ObjectFileName, MAX_PATH, "\\");
        StringCchCatA(ObjectFileName, MAX_PATH, GetPointerToLastSegmentFromPath(CFiles[Index], '\\'));
        StringCchCatA(ObjectFileName, MAX_PATH, ".elf");
        AddLinkerInputFile(BuildContext, ObjectFileName);
    }

    SetLinkerOutputBinary(BuildContext, "\\bootloader.img");
    BuildSuccess = LinkWithGCC(BuildContext);
    ClearBuildContext(BuildContext);
    PopSubTarget(BuildContext);
    return BuildSuccess;
}