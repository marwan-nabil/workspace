#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\shell\console.h"
#include "win32\shared\system\processes.h"
#include "win32\shared\strings\strings.h"
#include "win32\shared\strings\string_list.h"

#include "win32\tools\build\actions\build_context.h"
#include "win32\tools\build\actions\gcc.h"

b32 CompileWithGCC(build_context *BuildContext)
{
    char CompilerCommand[KiloBytes(2)];
    *CompilerCommand = {};
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "i686-elf-gcc.exe -c ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " -I ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerIncludePath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
    if (StringLength(BuildContext->CompilationInfo.OutputObjectPath) != 0)
    {
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "-o ");
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.OutputObjectPath);
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
    }
    FlattenStringList(BuildContext->CompilationInfo.Sources, CompilerCommand, ArrayCount(CompilerCommand));

    b32 Result = CreateProcessAndWait(CompilerCommand);
    if (!Result)
    {
        ConsolePrintColored
        (
            "ERROR: compilation failed.\n",
            FOREGROUND_RED
        );
    }

    return Result;
}

b32 LinkWithGCC(build_context *BuildContext)
{
    char LinkerCommand[KiloBytes(5)];
    *LinkerCommand = {};

    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), "i686-elf-gcc.exe ");
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), BuildContext->LinkingInfo.LinkerFlags);
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), " -T ");
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), BuildContext->LinkingInfo.LinkerScriptPath);
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), " -o ");
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), BuildContext->LinkingInfo.OutputBinaryPath);
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), " ");
    FlattenStringList(BuildContext->LinkingInfo.LinkerInputs, LinkerCommand, ArrayCount(LinkerCommand));
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), " -lgcc");

    b32 Result = CreateProcessAndWait(LinkerCommand);
    if (!Result)
    {
        ConsolePrintColored
        (
            "ERROR: linking failed.\n",
            FOREGROUND_RED
        );
    }

    return Result;
}