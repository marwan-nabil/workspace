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
#include "win32\tools\build\actions\iverilog.h"

b32 CompileWithIVerilog(build_context *BuildContext)
{
    char CompilerCommand[1024];
    *CompilerCommand = {};
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "iverilog.exe ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "-I ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerIncludePath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " -o ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.OutputObjectPath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
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