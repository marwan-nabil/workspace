#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\strings\path_handling.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\nasm.h"
#include "win32\tools\build\actions\gcc.h"

b32 BuildKernelImage(build_context *BuildContext)
{
    PushSubTarget(BuildContext, "kernel");
    b32 BuildSuccess = FALSE;

    char *AssemblyFiles[] =
    {
        "\\i686-elf\\os\\kernel\\isr.s",
        "\\i686-elf\\shared\\cpu\\gdt.s",
        "\\i686-elf\\shared\\cpu\\idt.s",
        "\\i686-elf\\shared\\cpu\\io.s",
        "\\i686-elf\\shared\\cpu\\panic.s",
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
        "\\i686-elf\\os\\kernel\\main.c",
        "\\i686-elf\\os\\kernel\\isr.c",
        "\\i686-elf\\os\\kernel\\descriptor_tables.c",
        "\\i686-elf\\os\\kernel\\tests.c",
        "\\i686-elf\\shared\\vga\\vga.c",
        "\\i686-elf\\shared\\strings\\strings.c",
        "\\i686-elf\\shared\\strings\\path_handling.c",
        "\\i686-elf\\shared\\strings\\print.c",
        "\\i686-elf\\shared\\cpu\\timing.c",
        "\\i686-elf\\shared\\cpu\\gdt.c",
        "\\i686-elf\\shared\\cpu\\idt.c",
        "\\i686-elf\\shared\\memory\\arena_allocator.c",
        "\\i686-elf\\shared\\memory\\memory.c",
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

    AddLinkerFlags(BuildContext, "-nostdlib -Wl,-Map=kernel.map");
    SetLinkerScriptPath(BuildContext, "\\i686-elf\\os\\kernel\\kernel.lds");

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

    SetLinkerOutputBinary(BuildContext, "\\kernel.img");
    BuildSuccess = LinkWithGCC(BuildContext);
    ClearBuildContext(BuildContext);
    PopSubTarget(BuildContext);
    return BuildSuccess;
}