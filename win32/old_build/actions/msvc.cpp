
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
#include "win32\tools\build\actions\msvc.h"

b32 CompileShader(build_context *BuildContext)
{
    char CompilerCommand[1024];
    *CompilerCommand = {};
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "fxc.exe ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /Fo \"");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.OutputObjectPath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "\" ");
    FlattenStringList(BuildContext->CompilationInfo.Sources, CompilerCommand, ArrayCount(CompilerCommand));

    b32 Result = CreateProcessAndWait(CompilerCommand);
    if (!Result)
    {
        ConsolePrintColored
        (
            "ERROR: shader compilation failed.\n",
            FOREGROUND_RED
        );
    }

    return Result;
}

b32 CompileShader2(build_context *BuildContext)
{
    char CompilerCommand[1024];
    *CompilerCommand = {};
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "fxc.exe ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
    FlattenStringList(BuildContext->CompilationInfo.Sources, CompilerCommand, ArrayCount(CompilerCommand));

    b32 Result = CreateProcessAndWait(CompilerCommand);
    if (!Result)
    {
        ConsolePrintColored
        (
            "ERROR: shader compilation failed.\n",
            FOREGROUND_RED
        );
    }

    return Result;
}

b32 CompileWithMSVC(build_context *BuildContext)
{
    char CompilerCommand[KiloBytes(1)];
    *CompilerCommand = {};

    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "cl.exe /c ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /I");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerIncludePath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
    FlattenStringList(BuildContext->CompilationInfo.Sources, CompilerCommand, ArrayCount(CompilerCommand));
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /Fo");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.OutputObjectPath);

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

b32 LinkWithMSVC(build_context *BuildContext)
{
    char LinkerCommand[KiloBytes(4)] = {};
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), "link.exe ");
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), BuildContext->LinkingInfo.LinkerFlags);
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), " /OUT:\"");
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), BuildContext->LinkingInfo.OutputBinaryPath);
    StringCchCatA(LinkerCommand, ArrayCount(LinkerCommand), "\" ");
    FlattenStringList(BuildContext->LinkingInfo.LinkerInputs, LinkerCommand, ArrayCount(LinkerCommand));

    b32 Result = CreateProcessAndWait(LinkerCommand);
    if (!Result)
    {
        ConsolePrintColored
        (
            "ERROR: Linking failed.\n",
            FOREGROUND_RED
        );
    }

    return Result;
}

b32 CompileAndLinkWithMSVC(build_context *BuildContext)
{
    char CompilerCommand[KiloBytes(2)];
    *CompilerCommand = {};
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "cl.exe ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerFlags);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /I");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->CompilationInfo.CompilerIncludePath);
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " ");
    FlattenStringList(BuildContext->CompilationInfo.Sources, CompilerCommand, ArrayCount(CompilerCommand));
    if (StringLength(BuildContext->LinkingInfo.OutputBinaryPath) != 0)
    {
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /Fe:\"");
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->LinkingInfo.OutputBinaryPath);
        StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), "\"");
    }
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), " /link ");
    StringCchCatA(CompilerCommand, ArrayCount(CompilerCommand), BuildContext->LinkingInfo.LinkerFlags);

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