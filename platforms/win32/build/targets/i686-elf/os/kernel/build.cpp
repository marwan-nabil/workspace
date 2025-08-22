#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "sources\win32\libraries\base_types.h"
#include "sources\win32\libraries\basic_defines.h"
#include "sources\win32\libraries\strings\path_handling.h"
#include "sources\win32\libraries\strings\string_list.h"

#include "sources\win32\tools\build\actions\build_context.h"
#include "sources\win32\tools\build\actions\nasm.h"
#include "sources\win32\tools\build\actions\gcc.h"

b32 BuildKernelImage(build_context *BuildContext)
{
    PushSubTarget(BuildContext, "kernel");
    b32 BuildSuccess = FALSE;

    char *AssemblyFiles[] =
    {
        "\\sources\\i686-elf\\os\\kernel\\isr.s",
        "\\sources\\i686-elf\\libraries\\cpu\\gdt.s",
        "\\sources\\i686-elf\\libraries\\cpu\\idt.s",
        "\\sources\\i686-elf\\libraries\\cpu\\io.s",
        "\\sources\\i686-elf\\libraries\\cpu\\panic.s",
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
        "\\sources\\i686-elf\\os\\kernel\\main.c",
        "\\sources\\i686-elf\\os\\kernel\\isr.c",
        "\\sources\\i686-elf\\os\\kernel\\descriptor_tables.c",
        "\\sources\\i686-elf\\os\\kernel\\tests.c",
        "\\sources\\i686-elf\\libraries\\vga\\vga.c",
        "\\sources\\i686-elf\\libraries\\strings\\strings.c",
        "\\sources\\i686-elf\\libraries\\strings\\path_handling.c",
        "\\sources\\i686-elf\\libraries\\strings\\print.c",
        "\\sources\\i686-elf\\libraries\\cpu\\timing.c",
        "\\sources\\i686-elf\\libraries\\cpu\\gdt.c",
        "\\sources\\i686-elf\\libraries\\cpu\\idt.c",
        "\\sources\\i686-elf\\libraries\\memory\\arena_allocator.c",
        "\\sources\\i686-elf\\libraries\\memory\\memory.c",
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
    SetLinkerScriptPath(BuildContext, "\\sources\\i686-elf\\os\\kernel\\kernel.lds");

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